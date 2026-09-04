#!/usr/bin/env python3
"""Structural faithfulness gate: set-diff the CALLEES of a recovered C++ function
against the callees in IWD2.exe's real disassembly.

re-agent's objective verifier only compares call COUNTS (with a tolerance of 3), so
it cannot tell *which* call is missing and waves through a function that calls three
wrong things. This is stronger and deterministic: it names every function the binary
calls (resolving ``CALL 0xADDR`` through our recovered names + Ghidra's index) and
diffs that SET against the functions our ``src/`` calls. A callee the binary makes
but our source does not is concrete evidence of under-implementation -- the exact
"missing > wrong" failure the build+smoke gate cannot see statically.

NB a literal asm line-by-line diff is infeasible (a different compiler / register
allocator emits different instructions for identical behaviour); the faithful,
compiler-independent invariant is the call graph -- *what* the function calls.

KNOWN FALSE POSITIVES (this is a LINT, not an oracle -- a human confirms RED):
  * INLINING -- the dominant one. If the binary inlined a helper your source still
    calls as one function, that helper's OWN calls appear as direct calls in the
    caller's asm, so they read as "missing" from your (faithful) source. Real case:
    C2DArray::Load calls ``SetResRef(ref, TRUE, TRUE)``; the binary inlined
    CRes::SetResRef, whose CDimm::GetResObject + CRes::CancelRequest then surface as
    Load's own calls. The source is correct; the diff still flags them. Inlined base
    methods often aren't even named in the export, so the call graph can't suppress
    this -- only a human can. So read RED as "named calls the binary makes that your
    source does not -- confirm these are not just an inlined helper's calls".
  * LEAF COLLISION -- callees are matched on the bare leaf (``Method``) because source
    calls through ``this``/members lose the class; two classes sharing a method name
    can mis-match. Widens matches (favours GREEN), never invents a miss.

Buckets the binary's calls into:
  * game  -- a recovered/named game function -> diffed against src (the real signal);
  * crt   -- a compiler/CRT helper (SEH, security cookie, chkstk...) -> ignored
             (MSVC inserts these; they never appear in our source);
  * indirect -- ``CALL [reg+off]`` / ``CALL reg`` virtual or pointer dispatch ->
             counted, not name-diffed (resolve via vtable_map separately);
  * unresolved -- ``CALL 0xADDR`` with no name anywhere -> reported, not failed.

Verdict: RED if any *named game* callee is missing from src; YELLOW if the src body
can't be found or only indirect/unresolved uncertainty remains; else GREEN.

Usage::

    python scripts/reagent_asm_verify.py --address 0x402b70
    python scripts/reagent_asm_verify.py --address 0x402b70 --json
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

from runtime_paths import venv_tool

sys.path.insert(0, str(Path(__file__).resolve().parent))
from reagent_resolve_symbols import (  # noqa: E402
    load_names, load_vtable_map, lookup_class, slots_for_class, virtual_placement,
)
from reagent_build_smoke import locate_function, src_file_for  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
DEFAULT_MAP = REPO / ".ghidra-exports" / "address_map.json"
DEFAULT_INDEX = REPO / ".ghidra-exports" / "_index.json"
DEFAULT_GLOBALS = REPO / ".ghidra-exports" / "_globals.json"
DEFAULT_VTABLE = REPO / ".ghidra-exports" / "vtable_map.json"
DEFAULT_BRIDGE = venv_tool("ghidra-bridge")
DEFAULT_CONFIG = REPO / "ghidra-bridge.host.yaml"

# A disassembly line: ``00403334 CALL <operand>``.
ASM_CALL_RE = re.compile(r"^\s*[0-9a-fA-F]{8}\s+CALL\s+(.+?)\s*$", re.IGNORECASE)
# Operand shapes.
DIRECT_ADDR_RE = re.compile(r"^0x([0-9a-fA-F]+)$")
DIRECT_NAME_RE = re.compile(r"^[A-Za-z_][\w:@?$.]*$")
# A register-indirect dispatch ``[EAX + 0x1c]`` / ``dword ptr [ECX]`` -- an x86 virtual
# call loads the vtable pointer into a GP register, then calls a slot off it, so the
# base is a GP reg (NOT EBP/ESP, which address the stack frame: locals, args and plain
# fn-pointers -- never a vtable). The captured offset is the vtable slot (absent = 0x0).
INDIRECT_MEM_RE = re.compile(
    r"\[\s*(E(?:AX|CX|DX|BX|SI|DI))\s*(?:\+\s*(0x[0-9a-fA-F]+))?\s*\]", re.IGNORECASE)
# x86 GP registers -- a bare-register CALL operand (``CALL EAX``) is an indirect
# fn-pointer dispatch, not a named callee; route it to indirect, not the game set.
X86_REGS = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"}
# A C++ call token in our source.
SRC_CALL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_:]*)\s*\(")

# Tokens that are not real calls in C++ source.
SRC_SKIP = {
    "if", "for", "while", "switch", "return", "sizeof", "alignof", "decltype",
    "static_cast", "reinterpret_cast", "const_cast", "dynamic_cast", "catch",
    "new", "delete", "do", "else", "operator",
}
# MSVC / CRT helpers the compiler injects -- never in our source, so excluding them
# keeps the diff from raising false under-implementation findings.
CRT_HELPER_RE = re.compile(
    r"(?i)(^_+chkstk|^__alloca|alloca_probe|^__SEH_|security_(check_)?cookie|"
    r"report_gsfailure|CxxFrameHandler|_EH_|except_handler|^__GSHandler|"
    r"^_+ftol|^_+allmul|^_+aulldiv|^_+alldiv|^_+aullrem|^_+allrem|^_+allshl|^_+aullshr|^_+allshr)"
)


def leaf(name: str) -> str:
    """Reduce ``CClass::Method`` / ``obj.Method`` to the bare leaf ``Method``."""
    for sep in ("::", "->", "."):
        if sep in name:
            name = name.rsplit(sep, 1)[-1]
    return name


def dump_asm(address: str, bridge: str, config: str) -> str:
    """Shell ``ghidra-bridge dump-asm`` (boots PyGhidra headless) -> listing text."""
    with tempfile.NamedTemporaryFile(suffix=".asm", delete=False) as tmp:
        out_path = tmp.name
    try:
        proc = subprocess.run(
            [bridge, "--config", config, "dump-asm", address, out_path],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            encoding="utf-8", errors="replace", check=False,
        )
        if proc.returncode != 0:
            sys.stderr.write(proc.stdout)
            raise SystemExit(f"dump-asm {address} failed (exit {proc.returncode})")
        return Path(out_path).read_text(encoding="utf-8", errors="replace")
    finally:
        try:
            Path(out_path).unlink()
        except OSError:
            pass


def parse_asm_callees(asm: str, names: dict[int, str]) -> dict:
    """Classify the CALL targets in *asm* into game / crt / indirect / unresolved.

    ``indirect_slots`` additionally records, for each GP-register memory-indirect
    dispatch, the vtable slot offset it uses (normalised ``0x%04x``, absent = ``0x0000``)
    so the caller can name it through the containing class's vtable. A bare-register or
    EBP/ESP-relative dispatch has no resolvable slot and only bumps ``indirect``.
    """
    game: dict[str, str] = {}     # leaf -> full name (for reporting)
    crt: set[str] = set()
    unresolved: set[str] = set()
    indirect = 0
    indirect_slots: list[str] = []
    for line in asm.splitlines():
        m = ASM_CALL_RE.match(line)
        if not m:
            continue
        op = m.group(1).strip()
        am = DIRECT_ADDR_RE.match(op)
        if am:
            addr = int(am.group(1), 16)
            name = names.get(addr)
            if name is None:
                unresolved.add(f"0x{addr:06x}")
            elif CRT_HELPER_RE.search(name):
                crt.add(name)
            else:
                game[leaf(name)] = name
        elif DIRECT_NAME_RE.match(op) and op.lower() not in X86_REGS:
            if CRT_HELPER_RE.search(op):
                crt.add(op)
            else:
                game[leaf(op)] = op
        else:
            indirect += 1   # CALL [reg+off] / CALL reg -- virtual or fn-pointer
            mem = INDIRECT_MEM_RE.search(op)
            if mem:
                off = int(mem.group(2), 16) if mem.group(2) else 0
                indirect_slots.append(f"0x{off:04x}")
    return {"game": game, "crt": sorted(crt), "indirect": indirect,
            "indirect_slots": indirect_slots, "unresolved": sorted(unresolved)}


def src_callees(address: int, map_path: Path, src_override: Path | None) -> tuple[set[str], str | None]:
    """Return ``(leaf call-token set, src_path)`` for the recovered function body."""
    if src_override:
        src_path = src_override
    else:
        rel = src_file_for(address, map_path)
        if rel is None:
            return set(), None
        src_path = REPO / "src" / rel
    if not src_path.is_file():
        return set(), None
    text = src_path.read_text(encoding="utf-8", errors="replace")
    span = locate_function(text, address)
    body = text[span[0]:span[1]] if span else text
    body = re.sub(r"//.*", "", re.sub(r"/\*.*?\*/", "", body, flags=re.S))
    leaves = {leaf(m.group(1)) for m in SRC_CALL_RE.finditer(body)
              if m.group(1) not in SRC_SKIP}
    return leaves, str(src_path)


def resolve_vtable_candidates(slots: list[str], class_name: str | None,
                              vmap: dict, src_leaves: set[str]) -> list[dict]:
    """Name GP-register indirect dispatches via the containing class's vtable slots.

    A static asm pass cannot PROVE the dispatched object is ``this`` (it may be a member
    of another class), so each is a CANDIDATE, not a fact -- it assumes dispatch through
    the containing class's vtable, the common ``this->Virtual()`` case. The value is
    turning an opaque ``indirect: N`` into named hints (``slot -> CClass::Method``, with
    whether ``src`` already calls it); it never flips the verdict to RED. Deduped by
    slot, in first-seen order. Entry: ``{slot, name, recovered, in_src}``.
    """
    cslots = slots_for_class(vmap, class_name)
    out: list[dict] = []
    seen: set[str] = set()
    for slot in slots:
        if slot in seen:
            continue
        seen.add(slot)
        info = cslots.get(slot)
        name = info.get("name") if isinstance(info, dict) else None
        if not name:
            continue
        out.append({
            "slot": slot, "name": name,
            "recovered": bool(info.get("recovered")),
            "in_src": leaf(name) in src_leaves,
        })
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--address", required=True, help="target function address (e.g. 0x402b70)")
    ap.add_argument("--src", type=Path, help="override src file (else resolved via address_map)")
    ap.add_argument("--map", type=Path, default=DEFAULT_MAP)
    ap.add_argument("--index", type=Path, default=DEFAULT_INDEX)
    ap.add_argument("--globals", dest="globals_", type=Path, default=DEFAULT_GLOBALS)
    ap.add_argument("--vtable", type=Path, default=DEFAULT_VTABLE, help="vtable_map.json for indirect-call naming")
    ap.add_argument("--bridge", default=str(DEFAULT_BRIDGE))
    ap.add_argument("--config", default=str(DEFAULT_CONFIG))
    ap.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    args = ap.parse_args()

    try:
        addr = int(args.address, 16)
    except ValueError:
        print(f"bad address: {args.address}")
        return 2

    names = load_names(args.map, args.index, args.globals_)
    asm = dump_asm(args.address, args.bridge, args.config)
    buckets = parse_asm_callees(asm, names)
    src_leaves, src_path = src_callees(addr, args.map, args.src)

    vmap = load_vtable_map(args.vtable)
    this_class = lookup_class(args.map, args.address)
    vcands = resolve_vtable_candidates(buckets["indirect_slots"], this_class, vmap, src_leaves)
    vplace = virtual_placement(addr, vmap, this_class, names.get(addr))   # the is_virtual fact
    vmissing = [c for c in vcands if not c["in_src"]]

    asm_game = buckets["game"]
    missing = sorted({full for lf, full in asm_game.items() if lf not in src_leaves})

    if src_path is None:
        verdict = "YELLOW"
        reason = "source body not found (address not in src/ or no // marker)"
    elif missing:
        verdict = "RED"
        reason = f"{len(missing)} named callee(s) in asm not in src (confirm not inlined helpers)"
    elif buckets["indirect"] or buckets["unresolved"]:
        verdict = "YELLOW"
        named = f"{len(vcands)} indirect named via vtable, {len(vmissing)} not in src; " if vcands else ""
        reason = f"{named}only indirect/unresolved calls remain to confirm"
    else:
        verdict = "GREEN"
        reason = "every named asm callee is present in src"

    result = {
        "address": args.address, "src": src_path, "verdict": verdict, "reason": reason,
        "missing": missing, "asm_game_calls": sorted(set(asm_game.values())),
        "indirect": buckets["indirect"], "crt_ignored": buckets["crt"],
        "unresolved": buckets["unresolved"], "src_call_leaves": sorted(src_leaves),
        "this_class": this_class, "is_virtual": bool(vplace),
        "virtual_placement": vplace, "vtable_candidates": vcands,
    }
    if args.json:
        print(json.dumps(result, indent=2))
        return 0

    print(f"{verdict}  {args.address}  ({reason})")
    print(f"  src: {src_path}")
    if vplace:
        fold = (f" (COMDAT-folded across {vplace['n_placements']} vtables; "
                f"slot reliable, name not)" if vplace["folded"] else "")
        print(f"  is_virtual: {vplace['name'] or '?'} @ {vplace['class']} "
              f"slot {vplace['slot']}{fold}")
    print(f"  asm named-game callees: {len(asm_game)}  | indirect: {buckets['indirect']}  "
          f"| crt-ignored: {len(buckets['crt'])}  | unresolved: {len(buckets['unresolved'])}")
    if missing:
        print("  MISSING in src (binary calls, recovered fn does not):")
        for n in missing:
            print(f"    - {n}")
        print("  ^ confirm by hand: a common benign cause is the binary inlining a helper")
        print("    your source still calls (the helper's own calls then read as missing).")
    if vcands:
        print(f"  indirect calls named via {this_class} vtable (candidates, assume dispatch through this):")
        for c in vcands:
            mark = "in src" if c["in_src"] else "NOT in src"
            print(f"    - slot {c['slot']}: {c['name']}  [{mark}]")
    if buckets["unresolved"]:
        print(f"  unresolved asm calls: {', '.join(buckets['unresolved'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
