#!/usr/bin/env python3
"""Lint: a constructor that leaves a member uninitialised (cppcheck uninitMemberVar).

Wraps `cppcheck --enable=warning` and keeps ONLY the `uninitMemberVar` id -- a ctor
exists but skips a member -- and NOT `uninitMemberVarNoCtor` (a POD struct with no
ctor at all, which is expected and noisy, 87 of them across src/).

Exactly the MonsterLarge bug class (fixed 2e1851f8): the
CGameAnimationTypeMonsterLarge ctor (0x6B0A00) never set m_currentVidCellBase
(+0x406), so ChangeDirection dereferenced garbage on the first large monster ->
load crash. cppcheck flags it statically; `re-agent parity` / call-count signals
are blind to it (no call is involved, just a missing field store).

Caveat -- this is a FAITHFULNESS prompt, not a proof of a bug. ~60 `uninitMemberVar`
hits exist across src/ today; many are members the original ctor ALSO leaves
uninitialised (faithful -> not a bug). So:
  * default scans only CHANGED lines (staged+unstaged src/ diff) -- a ctor you just
    recovered -- so the pre-existing 60 don't drown a fresh mistake;
  * --all audits the whole tree (periodic, expect noise);
  * every hit must be checked against the BINARY ctor (does it init the member?)
    before "fixing" -- adding an init the original omits is itself an infidelity.

  python3 scripts/lint_uninit_member.py            # changed src/ lines, exit 1 on hit
  python3 scripts/lint_uninit_member.py --all       # whole tree audit
  python3 scripts/lint_uninit_member.py src/Foo.cpp # explicit file(s)
  python3 scripts/lint_uninit_member.py --quiet      # only print hits
"""
from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

from runtime_paths import cppcheck_executable

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

# cppcheck default line: `path:line:col: severity: message [id]`
_CC_LINE = re.compile(r"^(?P<file>.*?):(?P<line>\d+):\d+:\s+\w+:\s+(?P<msg>.*?)\s+\[(?P<id>\w+)\]\s*$")
WANT_ID = "uninitMemberVar"  # NOT uninitMemberVarNoCtor (POD-without-ctor noise)


def _ctor_span(path: Path, start_line: int) -> tuple[int, int]:
    """[first line, last line] of the ctor whose signature cppcheck flagged --
    brace-balanced from its opening `{` (the missing init can be anywhere in the
    body, not near the signature line). Falls back to (start, start) on parse miss."""
    try:
        lines = path.read_text(errors="replace").splitlines()
    except OSError:
        return (start_line, start_line)
    depth = 0
    started = False
    for n in range(start_line - 1, len(lines)):
        for ch in lines[n]:
            if ch == "{":
                depth += 1
                started = True
            elif ch == "}":
                depth -= 1
                if started and depth == 0:
                    return (start_line, n + 1)
    return (start_line, start_line)


def _run_git(args: list[str]) -> str:
    return subprocess.run(
        ["git", *args], cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True
    ).stdout


def _changed_lines() -> dict[Path, set[int]]:
    """new-side line numbers touched in the staged+unstaged src/ diff, per file."""
    changed: dict[Path, set[int]] = {}
    diff = _run_git(["diff", "--cached", "--unified=0", "--no-ext-diff", "--", "src"])
    diff += _run_git(["diff", "--unified=0", "--no-ext-diff", "--", "src"])
    cur: Path | None = None
    new_no = 0
    for ln in diff.splitlines():
        if ln.startswith("+++ b/"):
            cur = (ROOT / ln[6:]).resolve()
        elif ln.startswith("@@"):
            m = re.search(r"\+(\d+)(?:,(\d+))?", ln)
            new_no = int(m.group(1)) if m else 0
        elif cur is not None and ln.startswith("+") and not ln.startswith("+++"):
            changed.setdefault(cur, set()).add(new_no)
            new_no += 1
        elif cur is not None and not ln.startswith("-"):
            new_no += 1
    return changed


def _cppcheck(files: list[Path]) -> list[tuple[Path, int, str]]:
    """Run cppcheck once over `files`; return [(file, line, member-message)] for
    uninitMemberVar only."""
    if not files:
        return []
    import os

    # Reuse cppcheck's own dependency-aware analysis between ARC and pre-commit.
    # Findings are still read and filtered every time; this is not a cached PASS.
    build_dir = ROOT / ".cache" / "cppcheck-uninit"
    build_dir.mkdir(parents=True, exist_ok=True)
    proc = subprocess.run(
        [
            cppcheck_executable(), "--enable=warning", "--std=c++17", "--quiet",
            f"--cppcheck-build-dir={build_dir}",
            f"-j{os.cpu_count() or 1}", f"-I{SRC}", *[str(f) for f in files],
        ],
        cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True,
    )
    out: list[tuple[Path, int, str]] = []
    for ln in proc.stderr.splitlines():
        m = _CC_LINE.match(ln)
        if not m or m.group("id") != WANT_ID:
            continue
        out.append(((ROOT / m.group("file")).resolve(), int(m.group("line")), m.group("msg")))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--all", action="store_true", help="scan all tracked src/ files")
    ap.add_argument("--quiet", action="store_true", help="only print hits")
    ap.add_argument("paths", nargs="*", help="explicit files (default: changed src/ lines)")
    args = ap.parse_args()

    if cppcheck_executable() is None:
        print("cppcheck not found on PATH -- install it to run this lint.", file=sys.stderr)
        return 0  # absent tool must not break a commit hook

    changed: dict[Path, set[int]] | None = None
    if args.paths:
        files = [Path(p).resolve() for p in args.paths]
    elif args.all:
        files = sorted(SRC.rglob("*.cpp"))
    else:
        changed = _changed_lines()
        files = sorted({f for f in changed if f.suffix == ".cpp" and f.exists()})

    hits = _cppcheck(files)
    if changed is not None:
        # default mode: keep a hit only if a line of its ctor body is in the diff.
        # cppcheck points at the ctor signature; the missing init can be anywhere
        # in the body, so test the whole brace-balanced span against changed lines.
        def touched(f: Path, line: int) -> bool:
            lo, hi = _ctor_span(f, line)
            return any(lo <= c <= hi for c in changed.get(f, set()))

        hits = [(f, l, m) for (f, l, m) in hits if touched(f, l)]

    for f, line, msg in hits:
        rel = f.relative_to(ROOT) if ROOT in f.parents else f
        print(f"{rel}:{line}: {msg} [uninitMemberVar] -- verify the binary ctor inits it before adding")

    if not args.quiet:
        scope = "all tracked" if args.all else ("explicit" if args.paths else "changed")
        print(
            f"\n{len(hits)} uninitialised-member ctor(s) in {scope} src/ -- check each vs the binary ctor."
            if hits
            else f"OK: no uninitMemberVar in {scope} src/ ({len(files)} file(s) scanned)."
        )
    return 1 if hits else 0


if __name__ == "__main__":
    raise SystemExit(main())
