#!/usr/bin/env python3
"""Annotate magic ``switch``/``case`` constants in Ghidra output with their IDS
symbol names, so a first-pass reverse can recognise engine dispatch tables.

IWD2 scripts, actions, triggers, stats and spell-states are all keyed by small
integers whose *names* live in the game's ``.IDS`` tables (extracted to
``data/near_infinity_export/IDS``). A decompiled action dispatcher reads as a bare
jump table::

    switch (action_id) {
    case 0x73: ...        // <- 115 = ForceSpell, but Ghidra cannot know that

deepseek has no tools and cannot open those tables, so the candidate names must be
INJECTED. This pass scans the decompiler text for ``case`` labels (the strongest,
lowest-noise signal -- a real jump table over engine ids), looks each value up
ACROSS every IDS table, and emits the candidates as a comment block. It does NOT
pick one: a value can appear in several tables, so the model disambiguates by
context (an ``EXECUTE``-shaped fn -> ACTION.IDS; a stat read -> STATS.IDS). Values
that match too many tables (generic 0/1/2...) are dropped as noise.

Purely mechanical lookup -- no guessing, no value invented.

Usage::

    reagent_ids_constants.py --address 0x4d1c20      # decompile via bridge, annotate
    reagent_ids_constants.py --input dump.c          # annotate a file
    cat dump.c | reagent_ids_constants.py            # annotate stdin
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

from runtime_paths import venv_tool

REPO = Path(__file__).resolve().parent.parent
DEFAULT_IDS_DIR = REPO / "data" / "near_infinity_export" / "IDS"
DEFAULT_BRIDGE = venv_tool("ghidra-bridge")
DEFAULT_CONFIG = REPO / "ghidra-bridge.host.yaml"

# A data row: VALUE (dec or 0xhex) then whitespace then a SYMBOL. The header lines
# (a lone count ``267``, a label ``slo188``, or blank) lack the value+symbol pair
# and are rejected for free.
ROW_RE = re.compile(r"^\s*(0x[0-9a-fA-F]+|-?\d+)\s+(\S+)")
# A C ``case`` label over an integer constant.
CASE_RE = re.compile(r"\bcase\s+(0x[0-9a-fA-F]+|-?\d+)\s*:")


def load_ids(ids_dir: Path) -> dict[int, dict[str, str]]:
    """Build ``{value: {IDS_STEM: symbol}}`` from every ``*.IDS`` table."""
    index: dict[int, dict[str, str]] = {}
    for path in sorted(ids_dir.glob("*.IDS")):
        stem = path.stem
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            m = ROW_RE.match(line)
            if not m:
                continue
            try:
                val = int(m.group(1), 16) if m.group(1).lower().startswith("0x") else int(m.group(1))
            except ValueError:
                continue
            # Keep just the name token (drop an ACTION/TRIGGER ``(args)`` signature).
            sym = m.group(2).split("(", 1)[0]
            if sym:
                index.setdefault(val, {}).setdefault(stem, sym)
    return index


def _candidates(tables: dict[str, str]) -> list[tuple[str, list[str]]]:
    """Invert ``{IDS: symbol}`` to ``[(symbol, [IDS, ...]), ...]`` sorted by support.

    Several tables naming a value the SAME (ACTION/INSTANT/DLGINST/SCRINST all
    ``SetGlobalTimer``) is consensus, not noise -- grouping by symbol surfaces it,
    and the symbol with the widest support leads.
    """
    by_sym: dict[str, list[str]] = {}
    for ids, sym in tables.items():
        by_sym.setdefault(sym, []).append(ids)
    return sorted(by_sym.items(), key=lambda kv: (-len(kv[1]), kv[0]))


def annotate(text: str, index: dict[int, dict[str, str]], max_symbols: int) -> tuple[str, int]:
    """Return ``(inject_block, n_values)`` of IDS candidates for the case labels in *text*.

    The block is a standalone comment header (not interleaved into the code, so it
    never corrupts the decompiler text). Noise is gauged by the number of DISTINCT
    symbols a value maps to: a value resolving to more than *max_symbols* different
    names is too generic (small ints like 0/1/2) and is dropped.
    """
    values: list[int] = []
    seen: set[int] = set()
    for m in CASE_RE.finditer(text):
        tok = m.group(1)
        try:
            val = int(tok, 16) if tok.lower().startswith("0x") else int(tok)
        except ValueError:
            continue
        if val in seen:
            continue
        seen.add(val)
        if val in index and len(set(index[val].values())) <= max_symbols:
            values.append(val)

    if not values:
        return "", 0

    lines = ["// IDS candidates for switch/case constants (engine data tables -- pick ONE by context):"]
    for val in sorted(values):
        cands = "  ".join(f"{sym} [{','.join(sorted(ids))}]" for sym, ids in _candidates(index[val]))
        lines.append(f"//   case {val} (0x{val & 0xffffffff:x}): {cands}")
    return "\n".join(lines), len(values)


def fetch_decompile(address: str, bridge: str, config: str) -> str:
    """Shell the ghidra-bridge to decompile *address* and return its text."""
    proc = subprocess.run(
        [bridge, "--config", config, "decompile", address],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        encoding="utf-8", errors="replace", check=False,
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr or proc.stdout)
        raise SystemExit(f"ghidra-bridge decompile {address} failed (exit {proc.returncode})")
    return proc.stdout


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = ap.add_mutually_exclusive_group()
    src.add_argument("--address", help="decompile this address via ghidra-bridge, then annotate")
    src.add_argument("--input", type=Path, help="annotate this file (default: stdin)")
    ap.add_argument("--ids-dir", type=Path, default=DEFAULT_IDS_DIR, help="IDS table directory")
    ap.add_argument("--max-symbols", type=int, default=5,
                    help="drop a value resolving to more than this many DISTINCT symbols (generic-noise cut)")
    ap.add_argument("--prepend", action="store_true",
                    help="emit the block followed by the original text (default: block only)")
    ap.add_argument("--bridge", default=str(DEFAULT_BRIDGE), help="ghidra-bridge executable")
    ap.add_argument("--config", default=str(DEFAULT_CONFIG), help="ghidra-bridge.yaml path")
    args = ap.parse_args()

    if not args.ids_dir.is_dir():
        sys.stderr.write(f"ERROR: IDS dir not found: {args.ids_dir}\n")
        return 1
    index = load_ids(args.ids_dir)

    if args.address:
        text = fetch_decompile(args.address, args.bridge, args.config)
    elif args.input:
        text = args.input.read_text(encoding="utf-8", errors="replace")
    else:
        text = sys.stdin.read()

    block, n = annotate(text, index, args.max_symbols)
    if args.prepend:
        sys.stdout.write((block + "\n\n" if block else "") + text)
    else:
        sys.stdout.write(block + ("\n" if block else ""))
    if args.address:
        sys.stderr.write(f"\n[ids] {n} case constant(s) annotated from {len(index)} IDS values\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
