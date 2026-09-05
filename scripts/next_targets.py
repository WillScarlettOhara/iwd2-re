#!/usr/bin/env python3
"""Rank the ~18.5k functions of IWD2.exe that are not recovered yet.

Until now "what next?" was answered once per session, by reasoning over
`gb unimplemented` + caller counts + judgement, and the answer survived only as
a sentence at the end of a commit message. This computes it instead, from data
the repo already has, and writes a top-N that survives a clone.

  scripts/next_targets.py                       top 25 to stdout
  scripts/next_targets.py --near CGameSprite    the holes in one class
  scripts/next_targets.py --write               refresh docs/next-targets.md
  scripts/next_targets.py --check               exit 1 if a listed target is now recovered
  scripts/next_targets.py --json out.json       the full ranking, every score term

Ranking (weights are constants below; every term lands in the JSON so a
ranking can be audited rather than trusted):

  +40 log2(1+recovered_callers)  a hole our own C++ already calls -- the only
                                 term backed by an observable defect
  +25 vtable_hole                an unrecovered slot in a mostly-done class.
                                 NOT decorative: _index.json reports
                                 num_callers 0 for every virtual-dispatch-only
                                 method (CGameSprite::Render included), so the
                                 call graph is structurally blind to virtuals,
                                 i.e. to most of this engine. vtable_map.json
                                 is the only signal that sees them.
  +15 class_completion           finishing a class makes its audits clean and
                                 reuses context already in the author's head
  +10 size_band                  byte-weighted .text is the headline metric, but
                                 a 12KB function is a session and a 20-byte thunk
                                 is worth nothing -- a band, not a monotone
   +8 log2(1+num_callers)        global hub-ness, as a tiebreak
  -10 log2(1+unrecovered_callees) drags in N new stubs; prefer near-leaves

Caller scores use unique caller functions. Ghidra exports one entry per call
site, so treating their length as a function count made helpers called several
times by one constructor look like global hubs.

There is no placeholder penalty: a FUN_ target is not a worse recovery target,
it is a different task, so it gets its own bucket instead of a fudge weight.

Five target buckets, plus incomplete source sites, because they are different
kinds of work:

  Recover   a NAMED game-owned function with no `// 0xADDR` marker in src/
  Identify  a FUN_/sub_ placeholder tied to a game class by vtable or caller
            evidence. It still needs a naming pass first.
  Triage    code whose ownership is not established yet
  Support   likely inline/template/resource plumbing, held for a cheap audit
  Excluded  named MFC/ATL, CRT, STL, or compiler code, with a reason code
  Complete  a `// TODO: Incomplete.` / `// PARTIAL:` site: a lie already in the
            tree, relevance guaranteed, but completing is not creating

Scope note. _index.json holds 29,664 Ghidra "functions", but 19,599 of them are
`Unwind@...` SEH funclets and 33 are `Catch@...` blocks -- compiler-generated
exception plumbing that nobody recovers. Excluding those (and thunks) the real
population is ~10,018, of which ~8,386 are recovered. So the honest remaining
backlog is ~1,632 functions, not the ~18.5k that 29,664 minus the marker count
suggests -- and the 37% headline in README/project_status.py is measured against
a denominator that is two thirds SEH funclets.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
EXPORTS = REPO / ".ghidra-exports"
INDEX = EXPORTS / "_index.json"
VTABLE_MAP = EXPORTS / "vtable_map.json"
CALLGRAPH = EXPORTS / "_callgraph.json"
ADDR_MAP = EXPORTS / "address_map.json"
OUT_MD = REPO / "docs" / "next-targets.md"
OUT_JSON = EXPORTS / "next_targets.json"

W_RECOVERED_CALLERS = 40
W_VTABLE_HOLE = 25
W_CLASS_COMPLETION = 15
W_SIZE_BAND = 10
W_CALLERS = 8
W_UNRECOVERED_CALLEES = -10
W_CLASS_EVIDENCE = 12

# A class counts as "mostly done" -- and therefore its remaining slots as holes
# worth filling -- above this recovered-slot ratio.
VTABLE_HOLE_MIN_COMPLETION = 0.60

# Unwind@/Catch@ are SEH funclets and thunk_ is a jump stub: compiler output,
# not code anyone recovers. They are 19,639 of the 29,664 index entries, so
# leaving them in makes the backlog look twelve times larger than it is.
MIN_SIZE = 16

# These rules only classify symbols that already carry a meaningful name. They
# never rename FUN_/sub_ placeholders. Keep them ordered from narrowest to
# broadest so each exclusion has one stable, auditable reason code.
NAMED_EXCLUSION_RULES = (
    ("compiler", "compiler-funclet", re.compile(
        r"^(?:thunk_|FID_|Catch|Unwind|__Init|_Init|\?\?_|__|_)")),
    ("stl", "stl-symbol", re.compile(
        r"^(?:std::|ATL::|_?basic_(?:string|ostream|istream)|allocator<)")),
    ("crt", "crt-symbol", re.compile(
        r"^(?:operator (?:new|delete)|mem(?:cpy|move|set|cmp)|"
        r"str(?:cmp|cpy|ncpy|len|chr)|wcs(?:cmp|cpy|len)|"
        r"malloc|calloc|realloc|free|qsort|bsearch|atexit|abort)$")),
    ("mfc", "mfc-symbol", re.compile(
        r"^(?:Afx\w*|NormalizeRect|~?(?:CCmdUI|CWnd|CFrameWnd|CView|CWinApp|CDocument|"
        r"CDocTemplate|CArchive|CFile|CFileFind|CException|CMemoryException|"
        r"CNotSupportedException|CObject|CRuntimeClass|CString|CPoint|CRect|"
        r"CSize|CMapPtrToPtr|CMapStringToPtr|CPtrArray|CPtrList|CObArray|"
        r"CObList|CTypedPtr|CCriticalSection|CSingleLock|CEvent|CMutex|"
        r"CSemaphore|CSyncObject))(?:$|::)")),
)

# A high-fan-in, short placeholder whose only callees are these helpers is much
# more likely to be an emitted inline/template body than missing game logic.
# This is intentionally a review bucket, not a hard exclusion.
SUPPORT_CALLEE_RE = re.compile(
    r"^(?:operator|CRes(?:::|$)|CResRef(?:::|$)|CDimm(?:::|$)|GetResObject|"
    r"CTypedPtr|CObArray|CObList|CString|CPtrList|std::|ATL::|_+|"
    r"mem(?:cpy|set|move|cmp)|str(?:cmp|cpy|ncpy|len)|malloc|free|"
    r"FUN_0078(?:ab40|abb0|ac30|ad40)|FUN_007e78de|"
    r"FUN_007fc984|FUN_007fcc1a)"
)
SUPPORT_MIN_CALL_SITES = 40
SUPPORT_MAX_DECOMPILED_LINES = 50
CALLER_CLASS_MIN_FUNCTIONS = 2
CALLER_CLASS_MIN_SHARE = 0.60


def as_addr(v) -> int | None:
    """Export refs are not all plain addresses: imports appear as
    `EXTERNAL:00000122`. Anything that is not an in-image address is dropped."""
    if not isinstance(v, str) or ":" in v:
        return None
    try:
        n = int(v, 16)
    except ValueError:
        return None
    return n if 0x400000 <= n < 0x1000000 else None


def sh(*args: str) -> str:
    try:
        return subprocess.run(["git", *args], cwd=REPO, capture_output=True,
                              text=True, timeout=120).stdout
    except (OSError, subprocess.TimeoutExpired):
        return ""


# --------------------------------------------------------------------------
# inputs
# --------------------------------------------------------------------------

def recovered_addrs() -> set[int]:
    """Addresses carrying a `// 0xADDR` marker in src/. git grep over the
    working tree, so it cannot disagree with what is actually checked in."""
    out = sh("grep", "-ohE", r"//\s*0x[0-9A-Fa-f]{5,8}", "--", "src")
    return {int(m, 16) for m in re.findall(r"0x([0-9A-Fa-f]{5,8})", out)}


def load_index() -> dict[int, dict]:
    data = json.loads(INDEX.read_text(encoding="utf-8"))
    return {int(k, 16): v for k, v in data.items()}


def load_source_symbols() -> tuple[dict[int, dict], set[str]]:
    """Return recovered symbol metadata and the classes represented in src/.

    ``address_map.json`` is generated from source markers. It therefore gives
    stronger class evidence than a class guessed from a decompiler placeholder.
    """
    if not ADDR_MAP.exists():
        return {}, set()
    try:
        raw = json.loads(ADDR_MAP.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}, set()
    symbols: dict[int, dict] = {}
    classes: set[str] = set()
    for key, value in raw.items():
        if not isinstance(value, dict):
            continue
        try:
            addr = int(key, 16)
        except ValueError:
            continue
        symbols[addr] = value
        cls = value.get("class")
        if isinstance(cls, str) and cls:
            classes.add(cls)
    return symbols, classes


def load_callgraph(index: dict[int, dict], rebuild: bool = False) -> dict[int, list[int]]:
    """addr -> list of caller addrs.

    _index.json only carries counts; the caller LISTS live in the 29k
    per-function exports, so this walks them once and caches the result.
    """
    if CALLGRAPH.exists() and not rebuild:
        try:
            raw = json.loads(CALLGRAPH.read_text(encoding="utf-8"))
            if len(raw) >= len(index) * 0.9:
                return {int(k, 16): [int(a, 16) for a in v] for k, v in raw.items()}
        except (json.JSONDecodeError, OSError, ValueError):
            pass

    print(f"building call graph from {len(index)} exports (once; cached in "
          f"{CALLGRAPH.relative_to(REPO)}) ...", file=sys.stderr)
    graph: dict[str, list[str]] = {}
    for addr in index:
        key = f"{addr:08x}"
        p = EXPORTS / f"{key}.json"
        if not p.exists():
            continue
        try:
            d = json.loads(p.read_text(encoding="utf-8"))
        except (json.JSONDecodeError, OSError):
            continue
        callers = [f"{a:08x}" for a in
                   (as_addr(c.get("addr")) for c in (d.get("callers") or []))
                   if a is not None]
        graph[key] = callers
    CALLGRAPH.write_text(json.dumps(graph), encoding="utf-8")
    return {int(k, 16): [int(a, 16) for a in v] for k, v in graph.items()}


def load_export_facts(addr: int) -> dict:
    p = EXPORTS / f"{addr:08x}.json"
    if not p.exists():
        return {}
    try:
        return json.loads(p.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}


def load_callees(addr: int) -> list[int]:
    d = load_export_facts(addr)
    return [a for a in (as_addr(c.get("addr")) for c in (d.get("callees") or []))
            if a is not None]


def load_vtables() -> tuple[dict[int, str], dict[str, float]]:
    """(addr -> owning class for unrecovered slots, class -> recovered ratio)."""
    if not VTABLE_MAP.exists():
        return {}, {}
    data = json.loads(VTABLE_MAP.read_text(encoding="utf-8"))
    holes: dict[int, str] = {}
    completion: dict[str, float] = {}
    for cls, info in data.items():
        slots = info.get("slots") or {}
        if not slots:
            continue
        done = sum(1 for s in slots.values() if s.get("recovered"))
        completion[cls] = done / len(slots)
        for s in slots.values():
            if s.get("recovered") or not s.get("addr"):
                continue
            try:
                holes[int(s["addr"], 16)] = cls
            except ValueError:
                continue
    return holes, completion


def sizes_from_index(index: dict[int, dict]) -> dict[int, int]:
    """No export carries a size, so approximate it by the gap to the next entry
    address -- the same trick project_status.py validates its byte coverage on."""
    ordered = sorted(index)
    out = {}
    for i, a in enumerate(ordered):
        nxt = ordered[i + 1] if i + 1 < len(ordered) else a
        out[a] = max(0, min(nxt - a, 65536))
    return out


def stub_sites() -> list[dict]:
    """`// TODO: Incomplete.` / `// PARTIAL:` -- known-incomplete code already in
    the tree. Guaranteed relevant, but completing is not creating, so these are
    ranked separately and never blended into the main list."""
    out = []
    raw = sh("grep", "-n", "-E", r"//\s*(TODO: Incomplete\.|PARTIAL:)", "--", "src")
    for line in raw.splitlines():
        parts = line.split(":", 2)
        if len(parts) < 3:
            continue
        out.append({"file": parts[0], "line": int(parts[1]),
                    "text": parts[2].strip()[:100]})
    return out


# --------------------------------------------------------------------------
# classification and scoring
# --------------------------------------------------------------------------

def named_exclusion(name: str) -> tuple[str, str] | None:
    """Return ``(ownership, reason-code)`` for known non-game symbols."""
    for ownership, code, pattern in NAMED_EXCLUSION_RULES:
        if pattern.search(name):
            return ownership, code
    return None


def decompiled_line_count(text: str) -> int:
    """Count non-empty decompiler lines; formatting changes should not matter."""
    return sum(1 for line in text.splitlines() if line.strip())


def support_shape_reason(call_sites: int, decompiled_lines: int,
                         callee_names: list[str], decompiled: str = "") -> str | None:
    """Explain why an unnamed function looks emitted from support code.

    This is deliberately conservative and merely moves a function to a review
    queue. A reviewer can promote it without first inventing a symbol name.
    """
    if call_sites < SUPPORT_MIN_CALL_SITES:
        return None
    if not decompiled_lines or decompiled_lines > SUPPORT_MAX_DECOMPILED_LINES:
        return None
    shape = f"{call_sites} call sites, {decompiled_lines} non-empty decompiler lines"
    if ("ExceptionList" in decompiled
            and re.search(r"\bLAB_[0-9A-Fa-f]+", decompiled)
            and any(name.startswith("FUN_007fc") for name in callee_names)):
        return f"{shape}, compiler SEH cleanup shape"
    if not callee_names:
        return f"{shape}, no callees"
    if all(SUPPORT_CALLEE_RE.search(name or "") for name in callee_names):
        return f"{shape}, only resource/container/runtime callees"
    return None


def probable_caller_class(callers: list[int], recovered: set[int],
                          source_symbols: dict[int, dict]) -> tuple[str | None, int, int]:
    """Infer a class affinity from unique recovered caller functions.

    The returned class is evidence for grouping only. It is never used as a
    replacement name for a FUN_ symbol.
    """
    counts: dict[str, int] = {}
    for addr in set(callers) & recovered:
        cls = (source_symbols.get(addr) or {}).get("class")
        if isinstance(cls, str) and cls:
            counts[cls] = counts.get(cls, 0) + 1
    if not counts:
        return None, 0, 0
    cls, count = min(counts.items(), key=lambda item: (-item[1], item[0]))
    total = sum(counts.values())
    if count < CALLER_CLASS_MIN_FUNCTIONS or count / total < CALLER_CLASS_MIN_SHARE:
        return None, count, total
    return cls, count, total


def classify(name: str, *, placeholder: bool, named_class: str | None,
             hole_class: str | None, caller_class: str | None,
             support_reason: str | None, source_classes: set[str]) -> tuple[str, str, list[dict]]:
    """Return ownership, work bucket, and machine-readable evidence."""
    excluded = named_exclusion(name) if not placeholder else None
    if excluded:
        ownership, code = excluded
        return ownership, "excluded", [{"code": code, "detail": name}]
    if hole_class:
        return "game", "identify" if placeholder else "recover", [
            {"code": "vtable-class", "detail": hole_class}]
    if support_reason:
        return "support", "support", [
            {"code": "support-shape", "detail": support_reason}]
    if named_class and named_class in source_classes:
        return "game", "identify" if placeholder else "recover", [
            {"code": "source-class", "detail": named_class}]
    if caller_class:
        return "game", "identify" if placeholder else "recover", [
            {"code": "caller-class-consensus", "detail": caller_class}]
    return "unknown", "triage", [
        {"code": "ownership-unproven", "detail": "no vtable/source/caller-class evidence"}]

def size_band(n: int) -> float:
    if n < 32:
        return 0.15
    if n < 200:
        return 0.6
    if n <= 2000:
        return 1.0
    if n <= 6000:
        return 0.75
    return 0.5


def score_all(index, recovered, graph, holes, completion, sizes,
              source_symbols=None, source_classes=None,
              facts_loader=load_export_facts) -> list[dict]:
    source_symbols = source_symbols or {}
    source_classes = source_classes or set()
    rows = []
    for addr, meta in index.items():
        if addr in recovered:
            continue
        name = meta.get("name") or f"FUN_{addr:08x}"
        hard_exclusion = named_exclusion(name)
        # The ~19k compiler funclets would swamp the useful JSON. Their stable
        # classifier and aggregate count remain available through --scope.
        if hard_exclusion and hard_exclusion[0] == "compiler":
            continue
        size = sizes.get(addr, 0)
        hole_cls = holes.get(addr)
        if size < MIN_SIZE and not hole_cls and not hard_exclusion:
            continue

        callers = graph.get(addr, [])
        unique_callers = set(callers)
        rec_callers = len(unique_callers & recovered)
        recovered_call_sites = sum(1 for c in callers if c in recovered)
        call_sites = meta.get("num_callers") or len(callers)
        is_fun = name.startswith(("FUN_", "sub_"))

        named_cls = name.split("::")[0] if "::" in name else None
        caller_cls, caller_cls_count, classed_callers = probable_caller_class(
            callers, recovered, source_symbols)
        cls = hole_cls or named_cls or caller_cls
        comp = completion.get(cls, 0.0) if cls else 0.0
        vhole = 1.0 if (hole_cls and comp >= VTABLE_HOLE_MIN_COMPLETION) else 0.0

        facts = {}
        support_reason = None
        if is_fun and call_sites >= SUPPORT_MIN_CALL_SITES:
            facts = facts_loader(addr) or {}
            support_reason = support_shape_reason(
                call_sites,
                decompiled_line_count(facts.get("decompiled") or ""),
                [c.get("name") or "" for c in (facts.get("callees") or [])],
                facts.get("decompiled") or "",
            )
        ownership, bucket, reasons = classify(
            name, placeholder=is_fun, named_class=named_cls,
            hole_class=hole_cls, caller_class=caller_cls,
            support_reason=support_reason, source_classes=source_classes)

        # Only paid for candidates that could plausibly rank; reading 18.5k
        # export files for the callee term would cost more than the term is worth.
        unrec_callees = 0
        prelim = (W_RECOVERED_CALLERS * math.log2(1 + rec_callers)
                  + W_VTABLE_HOLE * vhole
                  + W_CLASS_COMPLETION * comp
                  + W_SIZE_BAND * size_band(size)
                  + W_CALLERS * math.log2(1 + len(unique_callers))
                  + W_CLASS_EVIDENCE * math.log2(
                      1 + (caller_cls_count if caller_cls else 0)))
        if prelim > 30 and bucket not in ("excluded", "support"):
            if not facts:
                facts = facts_loader(addr) or {}
            callees = [a for a in
                       (as_addr(c.get("addr")) for c in (facts.get("callees") or []))
                       if a is not None]
            unrec_callees = sum(1 for c in callees if c not in recovered)
        score = prelim + W_UNRECOVERED_CALLEES * math.log2(1 + unrec_callees)

        rows.append({
            "address": f"0x{addr:06x}" if addr < 0x1000000 else f"0x{addr:08x}",
            "addr_int": addr,
            "bucket": bucket,
            "ownership": ownership,
            "name": name,
            "class": cls,
            "probable_class": caller_cls,
            "dominant_class_callers": caller_cls_count,
            "classed_recovered_callers": classed_callers,
            "size": size,
            "num_callers": len(unique_callers),
            "call_sites": call_sites,
            "recovered_callers": rec_callers,
            "recovered_call_sites": recovered_call_sites,
            "unrecovered_callees": unrec_callees,
            "vtable_hole": bool(vhole),
            "vtable_class_completion": round(comp, 3),
            "is_placeholder": is_fun,
            "classification_reasons": reasons,
            "score": round(score, 2),
        })
    # Deterministic: score desc, then address asc -- so regenerating without a
    # data change produces a byte-identical file.
    rows.sort(key=lambda r: (-r["score"], r["addr_int"]))
    return rows


# --------------------------------------------------------------------------
# output
# --------------------------------------------------------------------------

def fmt_table(rows: list[dict], limit: int) -> list[str]:
    out = ["| score | address | size | callers | symbol | evidence |",
           "|------:|---------|-----:|---------|--------|----------|"]
    for r in rows[:limit]:
        why = []
        if r["vtable_hole"]:
            why.append(f"vtable `{r['class']}`")
        elif r["probable_class"]:
            why.append(f"caller class `{r['probable_class']}`")
        if r["recovered_callers"]:
            why.append(f"{r['recovered_callers']} recovered caller function(s)")
        if r["bucket"] in ("support", "excluded"):
            why.extend(reason["code"] for reason in r["classification_reasons"])
        out.append(
            f"| {r['score']:.2f} | `{r['address']}` | {r['size']} | "
            f"{r['num_callers']} fn / {r['call_sites']} sites "
            f"({r['recovered_callers']}R) | "
            f"`{r['name']}` | {', '.join(why) or '-'} |")
    return out


def scope_stats(index: dict[int, dict], recovered: set[int]) -> dict:
    """The honest denominator: what is actually recoverable, funclets excluded."""
    real = rec = funclets = libraries = other_compiler = 0
    for a, m in index.items():
        name = m.get("name") or ""
        # Keep the historical scope denominator stable. Other recognized
        # compiler/library symbols are reported, but were already part of this
        # denominator and removing recovered ones would change the headline.
        if name.startswith(("Unwind", "Catch", "thunk_")):
            funclets += 1
            continue
        exclusion = named_exclusion(name)
        if exclusion and exclusion[0] == "compiler":
            other_compiler += 1
        elif exclusion:
            libraries += 1
        real += 1
        if a in recovered:
            rec += 1
    return {"real": real, "recovered": rec, "remaining": real - rec,
            "funclets": funclets, "named_libraries": libraries,
            "other_compiler": other_compiler,
            "index_total": len(index)}


def write_markdown(rows, stubs, recovered, index, path: Path, top: int) -> None:
    head_sha = sh("rev-parse", "--short", "HEAD").strip() or "unknown"
    mtime = ""
    if INDEX.exists():
        import datetime
        mtime = datetime.date.fromtimestamp(INDEX.stat().st_mtime).isoformat()
    st = scope_stats(index, recovered)
    recov = [r for r in rows if r["bucket"] == "recover"]
    ident = [r for r in rows if r["bucket"] == "identify"]
    triage = [r for r in rows if r["bucket"] == "triage"]
    support = [r for r in rows if r["bucket"] == "support"]
    excluded = [r for r in rows if r["bucket"] == "excluded"]

    lines = [
        "<!-- GENERATED by scripts/next_targets.py -- do not edit by hand.",
        f"     commit {head_sha}   _index.json {mtime}   "
        f"markers {len(recovered)}, matched to index entries {st['recovered']}",
        f"     weights: recovered_callers={W_RECOVERED_CALLERS} "
        f"vtable_hole={W_VTABLE_HOLE} class_completion={W_CLASS_COMPLETION} "
        f"size={W_SIZE_BAND} callers={W_CALLERS} "
        f"class_evidence={W_CLASS_EVIDENCE} "
        f"unrec_callees={W_UNRECOVERED_CALLEES}",
        "     Refresh: scripts/next_targets.py --write   Staleness: --check -->",
        "",
        "# Next targets",
        "",
        f"**Scope.** `_index.json` holds {st['index_total']:,} Ghidra functions, "
        f"but {st['funclets']:,} of them are `Unwind@`/`Catch@` SEH funclets and "
        f"thunks -- compiler-generated exception plumbing nobody recovers. The "
        f"real population is **{st['real']:,}**, of which **{st['recovered']:,} "
        f"are recovered ({100 * st['recovered'] / max(1, st['real']):.1f}%)**, "
        f"leaving **{st['remaining']:,}**.",
        "",
        "> The ~37% figure in `README.md` / `project_status.py` is measured "
        "against the {:,}-entry denominator, which is two thirds SEH funclets."
        .format(st["index_total"]),
        "",
        "Ownership is kept separate from priority. Full ranking, ownership, "
        "class evidence, unique callers, call-site counts, and reason codes: "
        "`.ghidra-exports/next_targets.json` (gitignored).",
        "See `docs/target-selection.md` for the rules and promotion workflow.",
        "",
        "Regenerate after a recovery arc -- deliberately not wired to a hook, "
        "which would put an unrelated file in every commit and hard-fail for "
        "anyone without `.ghidra-exports/`.",
        "",
        f"## Recover: game-owned names ({len(recov)})",
        "",
        "Named functions with no `// 0xADDR` marker in `src/`.",
        "",
    ]
    lines += fmt_table(recov, top)
    lines += [
        "",
        f"## Identify: game-owned placeholders ({len(ident)})",
        "",
        "`FUN_`/`sub_` placeholders. You cannot recover what you have not "
        f"identified, so these need a Ghidra naming pass first. Top {top}:",
        "",
    ]
    lines += fmt_table(ident, top)
    lines += [
        "",
        f"## Triage: ownership unproven ({len(triage)})",
        "",
        "These retain their exported name. Gather vtable or caller-class "
        "evidence before spending a recovery arc:",
        "",
    ]
    lines += fmt_table(triage, top)
    lines += [
        "",
        f"## Support-code review ({len(support)})",
        "",
        "Short high-fan-in placeholders with only resource/container/runtime "
        "callees. This is a review queue, not a permanent exclusion:",
        "",
    ]
    lines += fmt_table(support, min(top, 20))
    lines += [
        "",
        f"## Excluded named libraries ({len(excluded)})",
        "",
        f"MFC/ATL, CRT, and STL symbols recognized by explicit rules "
        f"({st['funclets']:,} compiler funclets are aggregated above):",
        "",
    ]
    lines += fmt_table(excluded, min(top, 20))
    lines += [
        "",
        f"## Complete ({len(stubs)})",
        "",
        "`// TODO: Incomplete.` / `// PARTIAL:` sites -- code already in the "
        "tree that knowingly diverges. First 20:",
        "",
        "| file:line | marker |",
        "|-----------|--------|",
    ]
    for s in stubs[:20]:
        lines.append(f"| `{s['file']}:{s['line']}` | {s['text']} |")
    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def cmd_check(rows_path: Path, recovered: set[int]) -> int:
    """Exit 1 when a listed target has since been recovered -- a 0.3s probe that
    tells `arc status` the file is out of date."""
    if not OUT_MD.exists():
        print("docs/next-targets.md missing -> scripts/next_targets.py --write")
        return 1
    text = OUT_MD.read_text(encoding="utf-8")
    listed = {int(m, 16) for m in re.findall(r"`0x([0-9a-fA-F]{5,8})`", text)}
    done = listed & recovered
    if done:
        print(f"{len(done)} listed target(s) are now recovered "
              f"-> scripts/next_targets.py --write")
        for a in sorted(done)[:10]:
            print(f"  0x{a:06x}")
        return 1
    print(f"docs/next-targets.md is current ({len(listed)} targets, none recovered)")
    return 0


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--top", type=int, default=25)
    ap.add_argument("--near", help="only this class (and its vtable holes)")
    ap.add_argument("--write", action="store_true", help="refresh docs/next-targets.md")
    ap.add_argument("--check", action="store_true", help="exit 1 if the md is stale")
    ap.add_argument("--json", nargs="?", const=str(OUT_JSON),
                    help="write the full ranking as JSON")
    ap.add_argument("--rebuild", action="store_true", help="rebuild the call-graph cache")
    ap.add_argument("--scope", action="store_true",
                    help="print the honest recoverable-scope counts as JSON and exit")
    args = ap.parse_args(argv)

    if not INDEX.exists():
        print(f"{INDEX.relative_to(REPO)} missing -- needs .ghidra-exports/",
              file=sys.stderr)
        return 2

    recovered = recovered_addrs()
    if args.check:
        return cmd_check(OUT_MD, recovered)
    if args.scope:
        print(json.dumps(scope_stats(load_index(), recovered)))
        return 0

    index = load_index()
    graph = load_callgraph(index, args.rebuild)
    holes, completion = load_vtables()
    if not VTABLE_MAP.exists():
        print("warning: vtable_map.json missing; run "
              "scripts/reagent_vtable_map.py for direct class evidence",
              file=sys.stderr)
    source_symbols, source_classes = load_source_symbols()
    sizes = sizes_from_index(index)
    rows = score_all(index, recovered, graph, holes, completion, sizes,
                     source_symbols, source_classes)
    stubs = stub_sites()

    if args.json:
        Path(args.json).write_text(json.dumps(rows, indent=1), encoding="utf-8")
        print(f"wrote {args.json} ({len(rows)} classified)")

    if args.write:
        write_markdown(rows, stubs, recovered, index, OUT_MD, args.top or 50)
        print(f"wrote {OUT_MD.relative_to(REPO)} "
              f"(top {args.top or 50} of {len(rows)}, {len(stubs)} stub sites)")
        return 0

    st = scope_stats(index, recovered)
    cls_stubs = []
    if args.near:
        cls = args.near
        rows = [r for r in rows if r["class"] == cls]
        comp = completion.get(cls)
        nholes = sum(1 for r in rows if r["vtable_hole"])
        if comp is not None:
            print(f"{cls}  {comp * 100:.0f}% of vtable slots recovered, "
                  f"{nholes} hole(s) left")
        else:
            print(f"{cls}  (no vtable in vtable_map.json)")
        cls_stubs = [s for s in stubs if Path(s["file"]).stem == cls]
    else:
        print(f"real scope {st['recovered']:,}/{st['real']:,} recovered "
              f"({100 * st['recovered'] / max(1, st['real']):.1f}%), "
              f"{st['remaining']:,} left "
              f"-- {st['funclets']:,} SEH funclets/thunks excluded")

    for bucket, title in (
            ("recover", "RECOVER (game-owned name)"),
            ("identify", "IDENTIFY (game-owned FUN_/sub_; name it first)"),
            ("triage", "TRIAGE (ownership unproven)"),
            ("support", "SUPPORT REVIEW (inline/template shape)"),
            ("excluded", "EXCLUDED (named MFC/ATL/CRT/STL)")):
        sel = [r for r in rows if r["bucket"] == bucket]
        if not sel:
            continue
        print(f"\n  == {title}: {len(sel)} ==")
        print(f"  {'score':>6}  {'address':<10} {'size':>5}  {'fn/sites':<13} what")
        shown = args.top if bucket not in ("support", "excluded") else min(args.top, 10)
        for r in sel[:shown]:
            why = "vtable hole" if r["vtable_hole"] else ""
            if not why and r["probable_class"]:
                why = f"caller class {r['probable_class']}"
            if bucket in ("support", "excluded"):
                why = r["classification_reasons"][0]["code"]
            callers = (f"{r['num_callers']}/{r['call_sites']} "
                       f"({r['recovered_callers']}R)")
            print(f"  {r['score']:6.1f}  {r['address']:<10} {r['size']:5}  "
                  f"{callers:<13} {r['name']}  {why}".rstrip())
    if not rows:
        print("  (nothing matched)")
    if cls_stubs:
        print(f"\n  COMPLETE: {len(cls_stubs)} `TODO: Incomplete` in {args.near} "
              f"-> lines {', '.join(str(s['line']) for s in cls_stubs[:8])}")
    actionable = sum(r["bucket"] in ("recover", "identify", "triage") for r in rows)
    print(f"\n  {actionable} actionable candidate(s), {len(rows)} classified; "
          "full audit: --json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
