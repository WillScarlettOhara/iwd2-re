#!/usr/bin/env python3
"""src_find.py - symbol index over src/ (token-cheap replacement for grep->sed->Read chains).

Usage:
  src_find.py Render                     # all symbols matching 'Render' (substring, case-sens)
  src_find.py CGameSprite::Render        # exact qualified lookup
  src_find.py 0x703700                   # address -> symbol
  src_find.py CGameSprite::Render --body # print exact function body with line numbers
  src_find.py Render --around 20         # 20 lines of context around the definition line
  src_find.py CGameSprite:: -l           # list all members of a class (names only)
  src_find.py --file src/CProjectile.cpp # outline one file (all indexed symbols)
  src_find.py --rebuild                  # force full reindex

Index cache: .ghidra-exports/src_index.json (per-file mtime, incremental refresh).
"""

import argparse
import json
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(REPO, "src")
CACHE_DIR = os.path.join(REPO, ".ghidra-exports")
CACHE = os.path.join(CACHE_DIR, "src_index.json")

ADDR_RE = re.compile(r"^//\s*(0x[0-9A-Fa-f]{5,8})\b")
# Definition / declaration line at column 0. Captures optional Class:: qualifier and name.
DEF_RE = re.compile(
    r"^(?!//|#|\s)"                                   # col 0, not comment/preproc
    r"(?P<pre>[\w:<>,~\*&\s]*?)"                      # return type & qualifiers
    r"(?P<qual>(?:[A-Za-z_]\w*::)+)?"                  # Class:: (possibly nested)
    r"(?P<name>~?[A-Za-z_]\w*|operator[^\s(]{1,8})"    # name / dtor / operator
    r"\s*\("
)
CLASS_RE = re.compile(r"^(?:class|struct)\s+([A-Za-z_]\w*)\s*(?::[^;{]*)?\{?\s*(?://.*)?$")


def parse_file(path):
    """Return list of symbol dicts for one source file."""
    try:
        with open(path, errors="replace") as f:
            lines = f.readlines()
    except OSError:
        return []
    rel = os.path.relpath(path, REPO).replace("\\", "/")
    is_header = path.endswith(".h")
    syms = []
    pending_addr = None
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        m = ADDR_RE.match(line)
        if m:
            pending_addr = m.group(1)
            i += 1
            continue
        if pending_addr and line.lstrip().startswith("//"):
            # doc-comment block between the addr tag and the definition
            i += 1
            continue
        cm = CLASS_RE.match(line)
        if cm:
            syms.append({
                "name": cm.group(1), "qual": "", "kind": "class",
                "file": rel, "line": i + 1, "addr": None, "sig": line.strip()[:160], "end": i + 1,
            })
        dm = DEF_RE.match(line)
        if dm and "(" in line:
            name = dm.group("name")
            qual = dm.group("qual") or ""
            pre = dm.group("pre").strip()
            # skip control flow & macro-ish lines
            if name in ("if", "while", "for", "switch", "return", "sizeof", "defined") or pre.startswith(("if", "else", "return")):
                i += 1
                continue
            # gather signature until '{' or ';' (max 8 lines)
            sig_lines = []
            j = i
            kind = None
            while j < n and j < i + 8:
                sig_lines.append(lines[j].rstrip("\n"))
                body_start = lines[j].find("{")
                semi = lines[j].find(";")
                if body_start != -1 and (semi == -1 or body_start < semi):
                    kind = "fn"
                    break
                if semi != -1:
                    kind = "decl"
                    break
                j += 1
            if kind is None:
                i += 1
                continue
            addr = pending_addr
            sig = " ".join(s.strip() for s in sig_lines)
            sig = re.sub(r"\s*\{.*$", "", sig)[:200]
            end = i + 1
            if kind == "fn":
                # brace matching from the line containing '{'
                depth = 0
                k = j
                started = False
                while k < n:
                    for ch in lines[k]:
                        if ch == "{":
                            depth += 1
                            started = True
                        elif ch == "}":
                            depth -= 1
                    if started and depth <= 0:
                        end = k + 1
                        break
                    k += 1
                else:
                    end = j + 1
            if kind == "fn" or (kind == "decl" and is_header and qual == ""):
                # cpp definitions; header method declarations (inside class -> no qual at col 0 is rare, keep cpp focus)
                syms.append({
                    "name": name, "qual": qual.rstrip(":"), "kind": kind,
                    "file": rel, "line": i + 1, "addr": addr, "sig": sig, "end": end,
                })
            pending_addr = None
            i = (end if kind == "fn" else j) if kind == "fn" else i + 1
            continue
        # addr comment annotating data: `// 0xADDR` then a declaration ending with ';'
        if pending_addr and line.strip().endswith(";") and not line.lstrip().startswith("//"):
            syms.append({
                "name": line.strip().rstrip(";").split("=")[0].strip().split()[-1].lstrip("*&"),
                "qual": "", "kind": "data",
                "file": rel, "line": i + 1, "addr": pending_addr, "sig": line.strip()[:160], "end": i + 1,
            })
            pending_addr = None
        elif not line.lstrip().startswith("//"):
            pending_addr = None
        i += 1
    return syms


def load_index(force=False):
    os.makedirs(CACHE_DIR, exist_ok=True)
    cache = {"files": {}, "syms": {}}
    if not force and os.path.exists(CACHE):
        try:
            cache = json.load(open(CACHE))
        except Exception:
            cache = {"files": {}, "syms": {}}
    current = {}
    for root, _dirs, files in os.walk(SRC):
        for fn in files:
            if fn.endswith((".cpp", ".h")):
                p = os.path.join(root, fn)
                current[os.path.relpath(p, REPO).replace("\\", "/")] = os.path.getmtime(p)
    dirty = False
    # drop deleted files
    for rel in list(cache["files"]):
        if rel not in current:
            del cache["files"][rel]
            cache["syms"].pop(rel, None)
            dirty = True
    # reparse changed files
    for rel, mt in current.items():
        if cache["files"].get(rel) != mt:
            cache["files"][rel] = mt
            cache["syms"][rel] = parse_file(os.path.join(REPO, rel))
            dirty = True
    if dirty:
        with open(CACHE, "w") as f:
            json.dump(cache, f)
    return cache


def all_syms(cache):
    for rel in sorted(cache["syms"]):
        for s in cache["syms"][rel]:
            yield s


def fullname(s):
    return (s["qual"] + "::" + s["name"]) if s["qual"] else s["name"]


def print_body(s, around=0):
    path = os.path.join(REPO, s["file"])
    lines = open(path, errors="replace").readlines()
    if around:
        lo = max(0, s["line"] - 1 - around)
        hi = min(len(lines), s["line"] + around)
    else:
        lo, hi = s["line"] - 1, s["end"]
        if s["line"] > 1 and ADDR_RE.match(lines[s["line"] - 2]):
            lo -= 1
    for idx in range(lo, hi):
        sys.stdout.write(f"{idx + 1}\t{lines[idx].rstrip()}\n" if lines[idx].strip() else f"{idx + 1}\t\n")


def main():
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("query", nargs="?", help="name, Class::Method, Class::, or 0xADDR")
    ap.add_argument("--body", action="store_true", help="print full body of the (single) match")
    ap.add_argument("--around", type=int, default=0, help="N context lines around def instead of body")
    ap.add_argument("-l", "--list", action="store_true", help="names only")
    ap.add_argument("--file", help="outline all symbols of one file")
    ap.add_argument("--rebuild", action="store_true")
    ap.add_argument("--max", type=int, default=40)
    args = ap.parse_args()

    cache = load_index(force=args.rebuild)
    if args.rebuild and not args.query and not args.file:
        n = sum(len(v) for v in cache["syms"].values())
        print(f"indexed {len(cache['files'])} files, {n} symbols")
        return

    matches = []
    if args.file:
        rel = os.path.relpath(os.path.abspath(args.file), REPO) if os.path.isabs(args.file) else args.file
        rel = rel.replace("\\", "/")
        matches = list(cache["syms"].get(rel, []))
    elif args.query:
        q = args.query
        if re.fullmatch(r"0[xX][0-9A-Fa-f]+", q):
            want = int(q, 16)
            matches = [s for s in all_syms(cache) if s["addr"] and int(s["addr"], 16) == want]
        elif q.endswith("::"):
            cls = q[:-2]
            matches = [s for s in all_syms(cache) if s["qual"] == cls]
        elif "::" in q:
            matches = [s for s in all_syms(cache) if fullname(s) == q]
            if not matches:
                matches = [s for s in all_syms(cache) if q in fullname(s)]
        else:
            matches = [s for s in all_syms(cache) if s["name"] == q]
            if not matches:
                matches = [s for s in all_syms(cache) if q.lower() in s["name"].lower()]
    else:
        ap.print_help()
        return

    if not matches:
        print("no match")
        sys.exit(1)

    fns_first = sorted(matches, key=lambda s: (s["kind"] != "fn", s["file"], s["line"]))
    if (args.body or args.around) and len([s for s in fns_first if s["kind"] == "fn"]) >= 1:
        target = next(s for s in fns_first if s["kind"] == "fn") if any(s["kind"] == "fn" for s in fns_first) else fns_first[0]
        if len(matches) > 1:
            print(f"# {len(matches)} matches, showing {fullname(target)}  {target['file']}:{target['line']}")
        print_body(target, around=args.around)
        return

    for s in fns_first[: args.max]:
        if args.list:
            print(fullname(s))
        else:
            addr = s["addr"] or "-"
            span = f"[{s['line']}-{s['end']}]" if s["kind"] == "fn" else s["kind"]
            print(f"{fullname(s):44s} {s['file']}:{s['line']:<6} {addr:10s} {span:12s} {s['sig'][:110]}")
    if len(fns_first) > args.max:
        print(f"... {len(fns_first) - args.max} more (use --max)")


if __name__ == "__main__":
    main()
