"""Individual parity heuristic signals — each returns a Finding or None."""
from __future__ import annotations

import json
import re
from collections import Counter
from collections.abc import Callable
from pathlib import Path

from re_agent.core.models import Finding, GhidraData, HookEntry, SourceMatch

SignalFn = Callable[..., Finding | None]


def check_missing_source(source: SourceMatch | None, **_kw: object) -> Finding | None:
    if source is None:
        return Finding(level="red", reason="Source function body not found")
    return None


def check_stub_markers(
    source: SourceMatch | None,
    stub_markers: tuple[str, ...] = ("NOTSA_UNREACHABLE",),
    **_kw: object,
) -> Finding | None:
    if source is not None and source.has_stub_marker:
        return Finding(level="red", reason=f"Source contains stub marker ({', '.join(stub_markers)})")
    return None


def check_trivial_stub(source: SourceMatch | None, **_kw: object) -> Finding | None:
    if source is None or source.plugin_call_count == 0:
        return None
    likely_trivial = (
        source.body_lines <= 14
        and source.non_plugin_call_count <= 1
        and source.control_flow_count == 0
    )
    if likely_trivial:
        return Finding(level="red", reason="Source appears to be a trivial plugin::Call* stub")
    return None


def check_large_asm_tiny_source(
    source: SourceMatch | None,
    ghidra: GhidraData | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    if source is None or ghidra is None or not ghidra.asm_ok or inline_skip:
        return None
    if ghidra.asm_instruction_count >= 80 and source.body_lines <= 12:
        return Finding(level="red", reason="Large ASM body but tiny source body, likely mismatch/stub")
    return None


def check_plugin_call_heavy(source: SourceMatch | None, **_kw: object) -> Finding | None:
    if source is None or source.plugin_call_count == 0:
        return None
    plugin_heavy = source.plugin_call_count >= max(2, source.non_plugin_call_count)
    trivial = (
        source.body_lines <= 14
        and source.non_plugin_call_count <= 1
        and source.control_flow_count == 0
    )
    if plugin_heavy and not trivial:
        return Finding(
            level="yellow",
            reason=(
                f"Source relies heavily on plugin::Call* "
                f"({source.plugin_call_count} plugin vs {source.non_plugin_call_count} non-plugin calls)"
            ),
        )
    return None


def check_short_body(source: SourceMatch | None, inline_skip: bool = False, **_kw: object) -> Finding | None:
    if source is None or inline_skip:
        return None
    if source.body_lines < 6:
        return Finding(level="yellow", reason=f"Very short body ({source.body_lines} lines), inspect manually")
    return None


def check_low_call_count(
    source: SourceMatch | None,
    ghidra: GhidraData | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    if source is None or ghidra is None or not ghidra.decompile_ok or inline_skip:
        return None
    if ghidra.callees is not None and ghidra.callees >= 6 and source.call_count <= 1:
        return Finding(
            level="yellow",
            reason=f"Source call count is very low ({source.call_count}) vs Ghidra callees ({ghidra.callees})",
        )
    return None


def check_fp_sensitivity(
    source: SourceMatch | None,
    ghidra: GhidraData | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    if source is None or ghidra is None or not ghidra.asm_ok or inline_skip:
        return None
    if ghidra.asm_has_fp_sensitive and not source.has_fp_token:
        return Finding(
            level="yellow",
            reason="ASM contains floating-point sensitive ops but source has no obvious math tokens",
        )
    return None


def check_call_count_mismatch(
    source: SourceMatch | None,
    ghidra: GhidraData | None = None,
    inline_skip: bool = False,
    call_count_warn_diff: int = 3,
    **_kw: object,
) -> Finding | None:
    if source is None or ghidra is None or not ghidra.asm_ok or inline_skip:
        return None
    call_diff = abs(ghidra.asm_call_count - source.call_count)
    if call_diff > call_count_warn_diff:
        return Finding(
            level="yellow",
            reason=(
                f"Call count mismatch: vanilla has {ghidra.asm_call_count} calls, "
                f"source has {source.call_count} calls (diff: {call_diff})"
            ),
        )
    return None


def check_nan_logic(
    source: SourceMatch | None,
    ghidra: GhidraData | None = None,
    **_kw: object,
) -> Finding | None:
    if source is None or ghidra is None or not ghidra.decompile_ok:
        return None
    if ghidra.decompile_has_nan and "isnan" not in source.body_no_comments and "NAN(" not in source.body_no_comments:
        return Finding(
            level="yellow",
            reason="Decompile includes NAN-sensitive logic; verify NaN behavior manually",
        )
    return None


def check_inline_wrapper(source: SourceMatch | None, **_kw: object) -> Finding | None:
    if source is not None and source.is_inline_internal_forwarder:
        return Finding(level="info", reason="Source is an inline forwarding wrapper to internal I_* implementation")
    return None


# --- member-offset parity: "right callee, right count, WRONG member" -----------------
# callee + call-count signals are blind to this whole class (both match); the only
# discriminating signal is the per-thiscall `this` offset, which the decompile masks but
# the Ghidra listing exposes: `LEA ECX,[ESI + 0xNNN]; CALL Class::Method`. Map the offset
# to a member via the header `/* 0xNNN */` comments and diff vs what the source names.

_MOV_THIS_RE = re.compile(r"\bMOV\s+([A-Z]{3}),ECX\b")
_ECX_WRITE_RE = re.compile(r"\b(?:MOV|LEA|XOR|AND|OR|ADD|SUB|POP|INC|DEC)\s+(?:ECX|CX|CL|CH)\b")
_ECX_OBJ_RE = re.compile(r"\b(?:LEA|MOV)\s+ECX,(?:[a-z]+ ptr )?\[([A-Z]{3}) \+ (0x[0-9a-fA-F]+)\]")
_ECX_OTHER_RE = re.compile(r"\bMOV\s+ECX,")
_CALL_TGT_RE = re.compile(r"\bCALL\s+(\S+)")
_DECL_RE = re.compile(r"/\*\s*([0-9A-Fa-f]+)\s*\*/\s*[^;{]*?\bm_(\w+)\s*(?:\[[^\]]*\])?\s*;")
_MEMBER_CALL_RE = re.compile(r"\bm_(\w+)\s*(?:\.|->)\s*(\w+)\s*\(")
_TYPE_OPEN_RE = re.compile(r"\b(?:class|struct)\s+(\w+)\b")
_FWD_DECL_RE = re.compile(r"^\s*(?:class|struct)\s+\w+\s*;")

_HEADER_CACHE: dict[str, dict[str, dict[str, int]]] = {}
_FNNAME_CACHE: dict[str, dict[str, str]] = {}


def build_member_offsets(source_root: Path) -> dict[str, dict[str, int]]:
    """{ClassName: {m_member: byte_offset}} from `/* 0xNNN */ ... m_x;` header comments —
    the recovery's own annotations, the bridge between C++ names and binary offsets.

    Keyed by the *declaring class*, not the file stem, and only members at that class's own
    top level are recorded. A header may hold several classes (e.g. CGameSpriteLastUpdate
    beside CGameSprite), and a nested struct/union re-bases its `/* 0xNNN */` offsets to its
    own 0x0000 — flattening either into one stem-keyed dict collides distinct members at the
    same offset (the m_pArea vs m_nSequence +0x12 false positive on sub_757B40)."""
    key = str(source_root)
    cached = _HEADER_CACHE.get(key)
    if cached is not None:
        return cached
    out: dict[str, dict[str, int]] = {}
    for hp in Path(source_root).rglob("*.h"):
        depth = 0
        stack: list[tuple[str, int]] = []  # (class_name, brace depth of its body)
        pending: str | None = None         # class/struct name awaiting its opening brace
        for line in hp.read_text(errors="replace").splitlines():
            # Record a member only when it sits directly in the innermost class body: a
            # nested struct/union is one brace deeper and its offsets are sub-struct-relative.
            md = _DECL_RE.search(line)
            if md and stack and depth == stack[-1][1]:
                out.setdefault(stack[-1][0], {}).setdefault(
                    f"m_{md.group(2)}", int(md.group(1), 16)
                )
            if not _FWD_DECL_RE.match(line):
                mt = _TYPE_OPEN_RE.search(line)
                if mt:
                    pending = mt.group(1)
            for ch in line:
                if ch == "{":
                    depth += 1
                    if pending is not None:
                        stack.append((pending, depth))
                        pending = None
                elif ch == "}":
                    if stack and stack[-1][1] == depth:
                        stack.pop()
                    depth -= 1
            if line.rstrip().endswith(";"):  # a finished statement is not a pending class
                pending = None
    _HEADER_CACHE[key] = out
    return out


def build_fn_names(export_dir: Path) -> dict[str, str]:
    """{normalized_8hex_addr: method_name} from the Ghidra address_map.json, so a raw
    `CALL 0x00xxxxxx` in the listing resolves to a method name. Ctors/dtors are dropped
    (they are never written in the source body). Missing map -> empty (signal degrades)."""
    key = str(export_dir)
    cached = _FNNAME_CACHE.get(key)
    if cached is not None:
        return cached
    out: dict[str, str] = {}
    fpath = Path(export_dir) / "address_map.json"
    if fpath.exists():
        try:
            raw = json.loads(fpath.read_text(errors="replace"))
        except (ValueError, OSError):
            raw = {}
        for addr, info in raw.items():
            name = info.get("name", "")
            cls = info.get("class", "")
            if name and not name.startswith("~") and name != cls:
                out[addr.lower().lstrip("0").rjust(1, "0")] = name
    _FNNAME_CACHE[key] = out
    return out


def _norm_addr(s: str) -> str:
    return s.lower().removeprefix("0x").lstrip("0").rjust(1, "0")


def _ghidra_call_pairs(asm: str, fn_names: dict[str, str]) -> Counter:
    """(this_offset, method) for each thiscall in a Ghidra dump-asm listing. CALL targets
    are raw addresses (`CALL 0x007acd60`) resolved via fn_names, or named `Class::Method`."""
    lines = asm.splitlines()
    this_reg = None
    for ln in lines[:32]:
        m = _MOV_THIS_RE.search(ln)
        if m and m.group(1) in ("ESI", "EDI", "EBX", "EBP"):
            this_reg = m.group(1)
            break
        if _ECX_WRITE_RE.search(ln) or re.search(r"\b(?:CALL|RET)\b", ln):
            break
    if this_reg is None:
        return Counter()
    pairs: Counter = Counter()
    pending: int | None = None
    for ln in lines:
        m = _ECX_OBJ_RE.search(ln)
        if m:
            pending = int(m.group(2), 16) if m.group(1) == this_reg else None
            continue
        if _ECX_OTHER_RE.search(ln):
            pending = None
            continue
        c = _CALL_TGT_RE.search(ln)
        if c:
            tgt = c.group(1)
            method = None
            if "::" in tgt:
                cls_part, _, m2 = tgt.rpartition("::")
                if not m2.startswith("~") and m2 != cls_part.rpartition("::")[2]:
                    method = m2
            elif tgt.startswith("0x"):
                method = fn_names.get(_norm_addr(tgt))
            if pending is not None and method:
                pairs[(pending, method)] += 1
            pending = None
    return pairs


def _method_adj(offsets: list[int], hvals: set[int]) -> int:
    """Per-method base-subobject shift (a base-class method's `this` is member+4, etc.)."""
    deltas: Counter = Counter()
    for off in offsets:
        for d in (0, 4, 8, 12, 16):
            if off - d in hvals:
                deltas[d] += 1
    return deltas.most_common(1)[0][0] if deltas else 0


def check_wrong_member(
    source: SourceMatch | None = None,
    ghidra: GhidraData | None = None,
    entry: HookEntry | None = None,
    member_offsets: dict[str, dict[str, int]] | None = None,
    fn_names: dict[str, str] | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    if (
        source is None
        or ghidra is None
        or not ghidra.asm_ok
        or not ghidra.asm_instructions
        or inline_skip
        or entry is None
        or not member_offsets
    ):
        return None
    hdr = member_offsets.get(entry.class_name)
    if not hdr:
        return None
    pairs = _ghidra_call_pairs(ghidra.asm_instructions, fn_names or {})
    if not pairs:
        return None
    off2name = {off: name for name, off in hdr.items()}
    hvals = set(hdr.values())
    by_method: dict[str, list[int]] = {}
    for (off, meth), n in pairs.items():
        by_method.setdefault(meth, []).extend([off] * n)
    adj = {meth: _method_adj(offs, hvals) for meth, offs in by_method.items()}

    bin_pairs: set[tuple[str, str]] = set()
    for off, meth in pairs:
        name = off2name.get(off - adj[meth])
        if name:
            bin_pairs.add((name, meth))

    src_pairs = {(f"m_{m.group(1)}", m.group(2)) for m in _MEMBER_CALL_RE.finditer(source.body)}
    src_methods = {meth for _, meth in src_pairs}
    # only the copy-paste / incomplete-member-set signature: the method IS used in the
    # source (on another member), just not on the member the binary calls it on. A method
    # entirely absent from the source = a stub/incomplete recovery, a different signal.
    missing = sorted(p for p in (bin_pairs - src_pairs) if p[1] in src_methods)
    if not missing:
        return None
    shown = ", ".join(f"{n}.{m}()" for n, m in missing[:6])
    more = f" (+{len(missing) - 6} more)" if len(missing) > 6 else ""
    return Finding(
        level="red",
        reason=(
            f"Wrong/missing member: binary calls {shown}{more} but the source never does "
            f"(likely a copy-paste on a sibling member)"
        ),
    )


# --- parameter-swap parity: "right callee, right count, WRONG argument" --------------
# Class of bug that callee/count/member signals are all blind to: the recovery feeds the
# wrong parameter into an expression (e.g. FXRenderClippingPolys used nPosZ where the
# binary's height test uses param_3 = nPosY; the Call Lightning canopy notch, lived 16
# days GREEN). A swap is structurally identical -- same callees, same counts, same members
# -- the only objective tell is reference frequency: the misplaced param is over-used and
# the starved param under-used by the same amount. We align the source's named params to
# the decompiler's param_N positions and flag a paired over/under imbalance.

_PARAM_TOK_RE = re.compile(r"\bparam_(\d+)\b")
_PARAM_IDENT_RE = re.compile(r"[A-Za-z_]\w*")


def _decompile_param_max(signature: str) -> int:
    """Highest param_N index in a decompiler signature -> the decompiler's parameter count
    (param_1..param_M; for a __thiscall param_1 is `this`)."""
    nums = [int(n) for n in _PARAM_TOK_RE.findall(signature)]
    return max(nums) if nums else 0


_SCALAR_TYPE_TOKENS = frozenset(
    {
        "int", "INT", "UINT", "long", "LONG", "ULONG", "short", "SHORT", "USHORT",
        "char", "CHAR", "byte", "BYTE", "bool", "BOOL", "BOOLEAN", "WORD", "DWORD",
        "QWORD", "COLORREF", "unsigned", "signed", "size_t", "float", "FLOAT",
    }
)


def _is_scalar_param(slot: str) -> bool:
    """A by-value scalar parameter -- the only kind a swap bug meaningfully mis-feeds.
    Excludes references/pointers (`&`/`*`) and class/struct types (CRect&, CPoint, T*),
    whose decompile-vs-source reference counts diverge for benign reasons (copied into
    locals, field access, member caching)."""
    decl = slot.split("=")[0]
    if "&" in decl or "*" in decl:
        return False
    type_toks = _PARAM_IDENT_RE.findall(decl)[:-1]  # drop the name (last identifier)
    if not type_toks:
        return False
    return all(t in _SCALAR_TYPE_TOKENS or t == "const" for t in type_toks)


def _source_param_names(signature: str) -> list[tuple[str, bool]]:
    """Ordered (name, is_scalar) for each parameter in a C++ definition
    `RetType Name(T a, U b, ...)`. Name = each top-level comma slot's last identifier
    (default-value tail stripped). Commas inside <>/()/[] are not separators."""
    lp = signature.find("(")
    if lp < 0:
        return []
    depth = 0
    rp = -1
    for i in range(lp, len(signature)):
        c = signature[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                rp = i
                break
    if rp < 0:
        return []
    inner = signature[lp + 1 : rp].strip()
    if not inner or inner == "void":
        return []
    slots: list[str] = []
    buf = ""
    d = 0
    for c in inner:
        if c in "<([":
            d += 1
        elif c in ">)]":
            d -= 1
        if c == "," and d == 0:
            slots.append(buf)
            buf = ""
        else:
            buf += c
    slots.append(buf)
    out: list[tuple[str, bool]] = []
    for slot in slots:
        ids = _PARAM_IDENT_RE.findall(slot.split("=")[0])
        out.append((ids[-1] if ids else "", _is_scalar_param(slot)))
    return out


# A comparison operator with the token as its LEFT operand: `tok <`, `tok >=`, `tok ==`...
# `<(?!<)` / `>(?!>)` exclude the shift operators; a trailing `->` never matches because the
# `>` there is preceded by `-`, not the token. Comparisons survive decompilation structurally
# (unlike arithmetic, which gets reassociated/cast), so this is the stable expression context.
_CMP_LEFT_RE = "{tok}\\s*(?:==|!=|<=|>=|<(?!<)|>(?!>))"


def _is_compared(token: str, body: str) -> bool:
    return re.search(_CMP_LEFT_RE.format(tok=rf"\b{re.escape(token)}\b"), body) is not None


def check_param_swap(
    source: SourceMatch | None = None,
    ghidra: GhidraData | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    if source is None or ghidra is None or not ghidra.decompiled or not source.signature or inline_skip:
        return None
    params = _source_param_names(source.signature)
    if len(params) < 2 or any(nm == "" for nm, _ in params):
        return None
    # The decompiler's param list (`int param_1,...,uint param_8`) lives in the head of the
    # decompiled body, NOT the `signature` field (which is often `...(void)`). Split on the
    # first `{`: head -> param count, body -> per-param reference frequency.
    full = ghidra.decompiled
    brace = full.find("{")
    dhead = full[:brace] if brace >= 0 else full
    dbody = full[brace:] if brace >= 0 else full
    m = _decompile_param_max(dhead)
    n = len(params)
    # Align source slot i to the decompiler's param index. Determine the offset from the
    # CALLING CONVENTION, not the param count: a __thiscall's param_1 is `this` (src[i] ==
    # param_(i+2)); otherwise src[i] == param_(i+1). Counting is unreliable -- the decompiler
    # silently drops trailing unused params, so a 3-arg __thiscall can show only param_1..3 and
    # a count heuristic would misread it as a free function and shift every alignment by one
    # (the CVidTile::ReadyTexture false positive). Sanity-bail only when the decompiler shows
    # too few real args to align src[0], or MORE params than the source declares.
    offset = 2 if "__thiscall" in dhead else 1
    if m < offset or m > n + offset - 1:
        return None
    sbody = source.body_no_comments
    # Scalar, by-value, non-field/index-accessed params are the only ones a swap meaningfully
    # mis-feeds; everything else diverges between source and decompile for benign reasons.
    considered = [
        (i, nm)
        for i, (nm, is_scalar) in enumerate(params)
        if is_scalar and not re.search(rf"\b{re.escape(nm)}\s*(?:\.|->|\[)", sbody)
    ]

    bin_freq = Counter(int(x) for x in _PARAM_TOK_RE.findall(dbody))
    deltas = {nm: len(re.findall(rf"\b{re.escape(nm)}\b", sbody)) - bin_freq.get(i + offset, 0) for i, nm in considered}

    # (1) EXPRESSION MATCHING -- structural, frequency-direction-confirmed. Does each param sit
    # on the left of a comparison? The binary's `param_(i+offset) <op>` must match the source's
    # `name_i <op>`. A clean transposition -- exactly one slot the binary compares but the
    # source does not (Y), and exactly one the source compares but the binary does not (Z) --
    # is a swapped comparison operand. Comparison structure alone fires on benign idioms
    # (asserts, member/derived comparisons), so confirm it with the reference shift the swap
    # must produce: Z is over-referenced (got Y's refs) and Y under-referenced. This is what
    # catches the nPosZ/nPosY height test regardless of how lopsided the counts are.
    bin_only: list[str] = []   # binary compares param at this slot, source's name does not
    src_only: list[str] = []   # source compares this name, binary's param at the slot does not
    for i, nm in considered:
        bin_cmp = _is_compared(f"param_{i + offset}", dbody)
        src_cmp = _is_compared(nm, sbody)
        if bin_cmp and not src_cmp:
            bin_only.append(nm)
        elif src_cmp and not bin_cmp:
            src_only.append(nm)
    if (
        len(bin_only) == 1
        and len(src_only) == 1
        and deltas.get(src_only[0], 0) >= 1
        and deltas.get(bin_only[0], 0) <= -1
    ):
        return Finding(
            level="yellow",
            reason=(
                f"Parameter swap in a comparison: source compares {src_only[0]} where the "
                f"binary compares {bin_only[0]} (aligned param positions, confirmed by the "
                f"reference shift) -- the wrong argument feeds the test"
            ),
        )

    # (2) REFERENCE-FREQUENCY fallback -- catches arithmetic-only swaps with no comparison.
    # Require exactly ONE over- and ONE under-used scalar param of near-balanced magnitude
    # (|over+under| <= 1): a swap moves the same refs from one slot to the other. Lopsided or
    # multi-param imbalance is decompiler-idiom noise (mangled __thiscall, member caching).
    over = sorted(((nm, d) for nm, d in deltas.items() if d >= 2), key=lambda x: -x[1])
    under = sorted(((nm, d) for nm, d in deltas.items() if d <= -2), key=lambda x: x[1])
    if len(over) != 1 or len(under) != 1 or abs(over[0][1] + under[0][1]) > 1:
        return None
    return Finding(
        level="yellow",
        reason=(
            f"Possible parameter swap: source over-references {over[0][0]}(+{over[0][1]}) and "
            f"under-references {under[0][0]}({under[0][1]}) at aligned param positions -- verify "
            f"the correct argument feeds each expression"
        ),
    )


def check_concat_swap(
    source: SourceMatch | None = None,
    entry: HookEntry | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    """Operand-ORDER faithfulness at CString operator+ sites. `a + b` and `b + a`
    call the same operator the same number of times, so call-count signals are
    blind to a swap (the CDimm::FindFileInDirectoryList dir/file bug that took a
    Frida diff to find). Delegates to scripts/arg_provenance.py --check, which
    diffs the binary push order against the source `A + B` order; SWAP? = the
    direct order fails AND the reversed order fits (PARAM kept strict)."""
    if source is None or entry is None or inline_skip:
        return None
    addr = getattr(entry, "address", None)
    if not addr:
        return None
    import subprocess
    import sys

    roots = [Path.cwd(), *Path(__file__).resolve().parents]
    script = next(
        (r / "scripts" / "arg_provenance.py" for r in roots if (r / "scripts" / "arg_provenance.py").is_file()),
        None,
    )
    if script is None:
        return None
    try:
        out = subprocess.run(
            [sys.executable, str(script), str(addr), "--check"],
            capture_output=True,
            text=True,
            timeout=60,
        ).stdout
    except Exception:
        return None
    swaps = [ln.strip() for ln in out.splitlines() if ln.strip().endswith("SWAP?")]
    if not swaps:
        return None
    return Finding(
        level="yellow",
        reason=(
            "Operand-order swap at a CString operator+ site (binary push order != source "
            "`A + B` order; call-count signals are blind to this): "
            + "; ".join(swaps[:3])
            + f" -- verify with `scripts/arg_provenance.py {addr} --check`"
        ),
    )


def check_never_advancing_loop(
    source: SourceMatch | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    """`while (*p <op> X) { ... }` whose body never advances `p` -- the dropped
    pointer-increment class. The condition variable is mutated by neither `p++`
    nor a by-ref pass, so the loop re-tests the same element forever (OOM/hang).
    Call-count signals are blind to it (the increment is a `ptr+4`, not a call),
    so this took a load-crash bisect to find: CGameEffectColorTintSolid /
    CGameEffectColorLightSolid ApplyEffect both walked a static `ranges[]` table
    with `while (*range != -1)` but forgot `range++`. Delegates to
    scripts/lint_infinite_loop.py and keeps only hits inside this function's
    line span (precision-first: unary-deref form, no break/return/goto)."""
    if source is None or inline_skip:
        return None
    path = getattr(source, "path", None)
    line = getattr(source, "line", None)
    if not path or not line or not Path(path).is_file():
        return None
    import subprocess
    import sys

    roots = [Path.cwd(), *Path(__file__).resolve().parents]
    script = next(
        (r / "scripts" / "lint_infinite_loop.py" for r in roots if (r / "scripts" / "lint_infinite_loop.py").is_file()),
        None,
    )
    if script is None:
        return None
    try:
        out = subprocess.run(
            [sys.executable, str(script), "--quiet", str(path)],
            capture_output=True,
            text=True,
            timeout=60,
        ).stdout
    except Exception:
        return None
    # The lint reports the absolute file line of the `while`; keep only the ones
    # that fall inside this function's body span.
    lo, hi = int(line), int(line) + int(getattr(source, "body_lines", 0)) + 2
    hits = []
    for ln in out.splitlines():
        m = re.search(r":(\d+):\s*while \(\*", ln)
        if m and lo <= int(m.group(1)) <= hi:
            hits.append(ln.strip())
    if not hits:
        return None
    return Finding(
        level="red",
        reason=(
            "Pointer-walk loop never advances its controlling pointer (dropped "
            "`p++`; call-count signals are blind to this -> infinite loop/OOM): "
            + "; ".join(hits[:3])
            + " -- verify vs binary, restore the missing advance"
        ),
    )


_UNINIT_CACHE: dict[tuple[str, float], list[tuple[int, str]]] = {}


def check_uninit_member(
    source: SourceMatch | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    """A constructor that leaves a member uninitialised (cppcheck uninitMemberVar).
    The MonsterLarge bug class (0x6B0A00 never set m_currentVidCellBase -> garbage
    deref on the first large monster, load crash): no call is involved so call-count
    signals are blind. Delegates to scripts/lint_uninit_member.py and keeps only hits
    inside this function's line span. YELLOW, not RED -- it is a faithfulness PROMPT:
    ~60 such members exist across src/, many uninit in the original ctor too (leaving
    them so is faithful), so confirm the BINARY ctor actually stores the member before
    adding an init (an unfaithful add is itself a defect)."""
    if source is None or inline_skip:
        return None
    path = getattr(source, "path", None)
    line = getattr(source, "line", None)
    if not path or not line or not Path(path).is_file():
        return None
    import shutil
    import subprocess
    import sys

    if shutil.which("cppcheck") is None:
        return None
    roots = [Path.cwd(), *Path(__file__).resolve().parents]
    script = next(
        (r / "scripts" / "lint_uninit_member.py" for r in roots if (r / "scripts" / "lint_uninit_member.py").is_file()),
        None,
    )
    if script is None:
        return None
    key = (str(path), Path(path).stat().st_mtime)
    if key not in _UNINIT_CACHE:  # one cppcheck run per file across a --filter sweep
        try:
            out = subprocess.run(
                [sys.executable, str(script), "--quiet", str(path)],
                capture_output=True,
                text=True,
                timeout=120,
            ).stdout
        except Exception:
            return None
        parsed: list[tuple[int, str]] = []
        for ln in out.splitlines():
            m = re.search(r":(\d+):\s*(Member variable .*?)\s*\[uninitMemberVar\]", ln)
            if m:
                parsed.append((int(m.group(1)), m.group(2)))
        _UNINIT_CACHE[key] = parsed
    lo, hi = int(line), int(line) + int(getattr(source, "body_lines", 0)) + 2
    hits = [msg for (ln, msg) in _UNINIT_CACHE[key] if lo <= ln <= hi]
    if not hits:
        return None
    return Finding(
        level="yellow",
        reason=(
            "Constructor leaves a member uninitialised (cppcheck uninitMemberVar; "
            "call-count signals are blind): "
            + "; ".join(hits[:3])
            + " -- confirm the binary ctor stores it before adding an init"
        ),
    )


def check_stuck_loop_index(
    source: SourceMatch | None = None,
    inline_skip: bool = False,
    **_kw: object,
) -> Finding | None:
    """A counter/index local a loop uses as a shift amount or array index but never
    advances -- stuck at its init value. The CSpawn::Read time-of-day bug
    (0x769BF0): `for (pos ...) timeOfDay |= 1 << bit;` with no `bit++`, so the mask
    collapses to bit 0 (binary does `inc ecx` each iteration). Call-count signals
    are blind (no call). Delegates to scripts/lint_stuck_loop_index.py and keeps
    only hits inside this function's line span. YELLOW: the class is FP-prone and
    the faithful loop may differ in more than the increment, so verify vs binary."""
    if source is None or inline_skip:
        return None
    path = getattr(source, "path", None)
    line = getattr(source, "line", None)
    if not path or not line or not Path(path).is_file():
        return None
    import subprocess
    import sys

    roots = [Path.cwd(), *Path(__file__).resolve().parents]
    script = next(
        (r / "scripts" / "lint_stuck_loop_index.py" for r in roots if (r / "scripts" / "lint_stuck_loop_index.py").is_file()),
        None,
    )
    if script is None:
        return None
    try:
        out = subprocess.run(
            [sys.executable, str(script), "--quiet", str(path)],
            capture_output=True,
            text=True,
            timeout=60,
        ).stdout
    except Exception:
        return None
    lo, hi = int(line), int(line) + int(getattr(source, "body_lines", 0)) + 2
    hits = []
    for ln in out.splitlines():
        m = re.search(r":(\d+):\s*((?:for|while)-loop .*?never advances[^\n]*)", ln)
        if m and lo <= int(m.group(1)) <= hi:
            hits.append(m.group(2).strip())
    if not hits:
        return None
    return Finding(
        level="yellow",
        reason=(
            "Loop counter/index never advances (stuck at its init value; call-count "
            "signals are blind): "
            + "; ".join(hits[:3])
            + " -- verify the binary advances it"
        ),
    )


ALL_SIGNALS: list[SignalFn] = [
    check_missing_source,
    check_stub_markers,
    check_trivial_stub,
    check_large_asm_tiny_source,
    check_plugin_call_heavy,
    check_short_body,
    check_low_call_count,
    check_fp_sensitivity,
    check_call_count_mismatch,
    check_nan_logic,
    check_inline_wrapper,
    check_wrong_member,
    check_param_swap,
    check_concat_swap,
    check_never_advancing_loop,
    check_uninit_member,
    check_stuck_loop_index,
]
