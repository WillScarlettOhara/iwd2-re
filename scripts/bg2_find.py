#!/usr/bin/env python3
"""BG2EE PDB name lookup -- token-lean replacement for grepping the PDB dumps.

BG2EE and IWD2 share the Infinity engine; class/method/member NAMES carry over
(~70%) but OFFSETS DIFFER. Output is for *naming* recovered IWD2 code -- every
offset printed is BG2's, never IWD2's.

First run builds a SQLite index from ``data/pdb/Baldur.pdb`` via llvm-pdbutil,
or from the bundled ``bg2_pdb_extracted.zip`` / ``extracted/`` text dumps when
the PDB or LLVM is unavailable (~30 s, one-time). Queries afterwards are instant
and emit a handful of lines instead of a 45 MB grep.

Usage::

    bg2_find.py NAME                 substring search across everything
    bg2_find.py 'CProjectile::'      full class view (layout + methods)
    bg2_find.py 'CProjectile::Ren'   search scoped to one class
    bg2_find.py --enum NAME          enum values (name substring)
    bg2_find.py --build              force rebuild of the index
    -l N                             raise line cap (0 = no cap)
    --all                            include std::/libjingle noise
"""
from __future__ import annotations

import argparse
import io
import re
import shutil
import sqlite3
import subprocess
import sys
import zipfile
from contextlib import ExitStack
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
PDB = REPO / "data" / "pdb" / "Baldur.pdb"
DB = REPO / "data" / "pdb" / "bg2_pdb.sqlite"

NOISE_PREFIXES = (
    "std::", "talk_base::", "buzz::", "cricket::", "rtc::", "webrtc::",
    "sigslot::", "testing::", "google::", "google_breakpad::", "boost::",
    "Concurrency::", "ATL::", "<lambda", "__vc_attributes", "`anonymous",
    "_", "$",
)

# CodeView primitive type indices (fallback; the dump also prints these inline)
PRIMS = {
    0x0003: "void", 0x0008: "HRESULT", 0x0010: "signed char", 0x0011: "short",
    0x0012: "long", 0x0013: "__int64", 0x0020: "unsigned char",
    0x0021: "unsigned short", 0x0022: "unsigned long",
    0x0023: "unsigned __int64", 0x0030: "bool", 0x0040: "float",
    0x0041: "double", 0x0042: "long double", 0x0070: "char",
    0x0071: "wchar_t", 0x0074: "int", 0x0075: "unsigned",
}


def is_noise(name: str) -> bool:
    return name.startswith(NOISE_PREFIXES)


# ---------------------------------------------------------------- build

def dump_lines(pdb: Path, stream: str):
    """Read an LLVM stream on either host, including the checked-in UTF-16 dump."""
    llvm = shutil.which("llvm-pdbutil")
    if pdb.is_file() and llvm:
        with subprocess.Popen(
                [llvm, "dump", "-" + stream, str(pdb)],
                stdout=subprocess.PIPE, text=True, encoding="utf-8",
                errors="replace", bufsize=1 << 20) as proc:
            yield from proc.stdout
            if proc.wait():
                raise subprocess.CalledProcessError(proc.returncode, proc.args)
        return

    # The symbols dump also contains the module headers emitted by -modules.
    name = f"bg2_pdb_{'symbols' if stream == 'modules' else stream}.txt"
    extracted = pdb.parent / "extracted" / name
    archive = pdb.parent / "bg2_pdb_extracted.zip"
    with ExitStack() as stack:
        if extracted.is_file():
            raw = stack.enter_context(extracted.open("rb"))
        elif archive.is_file():
            zipped = stack.enter_context(zipfile.ZipFile(archive))
            raw = stack.enter_context(zipped.open(name))
        else:
            raise FileNotFoundError(
                f"need {pdb} and llvm-pdbutil, or {extracted}, or {archive}")
        bom = raw.read(2)
        raw.seek(0)
        encoding = "utf-16" if bom in (b"\xff\xfe", b"\xfe\xff") else "utf-8-sig"
        text = stack.enter_context(io.TextIOWrapper(raw, encoding=encoding))
        yield from text


RE_RECORD = re.compile(r"^\s+(0x[0-9A-F]+) \| (LF_\w+)(?: \[size = \d+\])?(?: `(.*)`)?\s*$")
RE_MEMBER = re.compile(
    r"- LF_MEMBER \[name = `(.*?)`, Type = (0x[0-9A-F]+)(?: \(([^)]*)\))?, "
    r"offset = (\d+), attrs = ([^\]]+)\]")
RE_STMEMBER = re.compile(
    r"- LF_STMEMBER \[name = `(.*?)`, type = (0x[0-9A-F]+)(?: \(([^)]*)\))?, "
    r"attrs = ([^\]]+)\]")
RE_ONEMETHOD = re.compile(r"- LF_ONEMETHOD \[name = `(.*)`\]\s*$")
RE_METHOD = re.compile(
    r"- LF_METHOD \[name = `(.*)`, # overloads = (\d+), overload list = (0x[0-9A-F]+)\]")
RE_ENUMERATE = re.compile(r"- LF_ENUMERATE \[(.+) = (-?\d+)\]")
RE_INDEX = re.compile(r"- LF_INDEX \[type = (0x[0-9A-F]+)\]")
RE_MLIST_ENTRY = re.compile(
    r"- Method \[type = (0x[0-9A-F]+), vftable offset = (-?\d+), attrs = ([^\]]+)\]")
RE_TI_INLINE = re.compile(r"(0x[0-9A-F]{4,5}) \(([^()]*)\)")
RE_ARG_LINE = re.compile(r"^\s+(0x[0-9A-F]+)(?: \([^)]*\))?: `(.*)`\s*$")


def parse_types(pdb: Path):
    """One streaming pass over `llvm-pdbutil dump -types`."""
    named = {}       # ti -> (kind, name)  for class/struct/union/enum incl fwd refs
    classes = {}     # name -> dict(kind, size, fl)   non-fwd only
    enums = {}       # name -> fl ti                  non-fwd only
    fieldlists = {}  # ti -> list of entry tuples
    methodlists = {}
    arglists = {}    # ti -> [pretty arg, ...]
    mfuncs = {}      # ti -> (ret_ti, ret_inline, arglist_ti, conv)
    ptrs = {}        # ti -> (referent_ti, suffix)
    mods = {}        # ti -> (referent_ti, mods)
    arrays = {}      # ti -> (elem_ti, byte_size)
    bitfields = {}   # ti -> (base_ti, bits, bitoff)
    prims = dict(PRIMS)

    kind = None
    ti = 0
    name = None
    pending = None   # (method_name,) waiting for its detail line
    cur_list = None  # active fieldlist/methodlist accumulator
    cur_class = None  # dict being filled from continuation lines

    for line in dump_lines(pdb, "types"):
        m = RE_RECORD.match(line)
        if m:
            ti, kind, name = int(m.group(1), 16), m.group(2), m.group(3)
            pending = None
            cur_class = None
            cur_list = None
            if kind in ("LF_CLASS", "LF_STRUCTURE", "LF_UNION", "LF_ENUM") and name:
                named[ti] = ("enum" if kind == "LF_ENUM" else kind[3:].lower(), name)
                cur_class = {"name": name, "kind": named[ti][0], "fl": 0,
                             "size": 0, "fwd": False}
            elif kind == "LF_FIELDLIST":
                cur_list = fieldlists[ti] = []
            elif kind == "LF_METHODLIST":
                cur_list = methodlists[ti] = []
            elif kind == "LF_ARGLIST":
                cur_list = arglists[ti] = []
            continue

        # ---- continuation lines of the current record
        if cur_class is not None:
            m = re.search(r"field list: (0x[0-9A-F]+|<no type>)", line)
            if m:
                cur_class["fl"] = 0 if m.group(1) == "<no type>" else int(m.group(1), 16)
            m = re.search(r"options: (.*?), sizeof (\d+)", line)
            if m:
                cur_class["fwd"] = "forward ref" in m.group(1)
                cur_class["size"] = int(m.group(2))
                if not cur_class["fwd"] and not is_noise(cur_class["name"]):
                    if cur_class["kind"] == "enum":
                        enums[cur_class["name"]] = cur_class["fl"]
                    else:
                        classes[cur_class["name"]] = dict(cur_class)
            elif "options:" in line and cur_class["kind"] == "enum":
                if "forward ref" not in line and not is_noise(cur_class["name"]):
                    enums[cur_class["name"]] = cur_class["fl"]
            continue

        if kind == "LF_POINTER":
            m = re.search(r"referent = (0x[0-9A-F]+)", line)
            if m:
                suffix = "&" if " ref" in line or "mode = ref" in line else "*"
                ptrs[ti] = (int(m.group(1), 16), suffix)
            continue
        if kind == "LF_MODIFIER":
            m = re.search(r"referent = (0x[0-9A-F]+)(?: \(([^)]*)\))?, modifiers = (.+)", line)
            if m:
                mods[ti] = (int(m.group(1), 16), m.group(3).strip())
            continue
        if kind == "LF_ARRAY":
            m = re.search(r"size: (\d+), index type: .*?, element type: (0x[0-9A-F]+)", line)
            if m:
                arrays[ti] = (int(m.group(2), 16), int(m.group(1)))
            continue
        if kind == "LF_BITFIELD":
            m = re.search(r"type = (0x[0-9A-F]+)(?: \([^)]*\))?, bit offset = (\d+), # bits = (\d+)", line)
            if m:
                bitfields[ti] = (int(m.group(1), 16), int(m.group(3)), int(m.group(2)))
            continue
        if kind in ("LF_MFUNCTION", "LF_PROCEDURE"):
            m = re.search(r"return type = (0x[0-9A-F]+)(?: \(([^)]*)\))?, # args = \d+, param list = (0x[0-9A-F]+)", line)
            if m:
                mfuncs[ti] = [int(m.group(1), 16), m.group(2), int(m.group(3), 16), ""]
            elif "calling conv = " in line and ti in mfuncs:
                mfuncs[ti][3] = re.search(r"calling conv = ([\w ]+?),", line).group(1)
            continue

        if cur_list is None:
            continue

        # ---- fieldlist / methodlist / arglist entries
        if kind == "LF_ARGLIST":
            m = RE_ARG_LINE.match(line)
            if m:
                cur_list.append(m.group(2))
            continue
        if kind == "LF_METHODLIST":
            m = RE_MLIST_ENTRY.search(line)
            if m:
                cur_list.append((int(m.group(1), 16), int(m.group(2)), m.group(3).strip()))
            continue

        if pending is not None:
            m = re.search(r"type = (0x[0-9A-F]+), vftable offset = (-?\d+), attrs = (.+)", line)
            if m:
                cur_list.append(("method", pending, int(m.group(1), 16),
                                 int(m.group(2)), m.group(3).strip()))
                pending = None
                continue
            m = re.search(r"type = (0x[0-9A-F]+), offset = (\d+)", line)
            if m and pending == "\x00base":
                cur_list.append(("base", int(m.group(1), 16), int(m.group(2))))
                pending = None
                continue
            pending = None

        m = RE_MEMBER.search(line)
        if m:
            cur_list.append(("member", m.group(1), int(m.group(2), 16),
                             m.group(3), int(m.group(4)), 0))
            continue
        m = RE_STMEMBER.search(line)
        if m:
            cur_list.append(("member", m.group(1), int(m.group(2), 16),
                             m.group(3), -1, 1))
            continue
        m = RE_ONEMETHOD.search(line)
        if m:
            pending = m.group(1)
            continue
        m = RE_METHOD.search(line)
        if m:
            cur_list.append(("overloads", m.group(1), int(m.group(3), 16)))
            continue
        m = RE_ENUMERATE.search(line)
        if m:
            cur_list.append(("enumval", m.group(1), int(m.group(2))))
            continue
        m = RE_INDEX.search(line)
        if m:
            cur_list.append(("chain", int(m.group(1), 16)))
            continue
        if "- LF_BCLASS" in line:
            pending = "\x00base"
            continue

    # opportunistic primitive names seen inline anywhere are already in PRIMS;
    # pointer-to-primitive CV indices (0x04XX/0x06XX) resolve via low byte
    def resolve(t, depth=0):
        if depth > 8:
            return f"0x{t:X}"
        if t < 0x1000:
            if t in prims:
                return prims[t]
            base = prims.get(t & 0xFF)
            if base and (t & 0xF00) in (0x400, 0x600):
                return base + "*"
            return f"0x{t:X}"
        if t in named:
            return named[t][1]
        if t in ptrs:
            ref, suf = ptrs[t]
            return resolve(ref, depth + 1) + suf
        if t in mods:
            ref, mm = mods[t]
            return mm + " " + resolve(ref, depth + 1)
        if t in arrays:
            elem, nbytes = arrays[t]
            return f"{resolve(elem, depth + 1)}[{nbytes}b]"
        if t in bitfields:
            base, bits, _ = bitfields[t]
            return f"{resolve(base, depth + 1)}:{bits}"
        return f"0x{t:X}"

    def sig(ft):
        rec = mfuncs.get(ft)
        if not rec:
            return "(?)"
        ret_ti, ret_inline, arg_ti, conv = rec
        args = ", ".join(arglists.get(arg_ti, []))
        ret = ret_inline or resolve(ret_ti)
        s = f"({args})"
        if ret != "void":
            s += f" -> {ret}"
        if conv and conv not in ("thiscall", "cdecl"):
            s += f" [{conv}]"
        return s

    def fl_entries(fl):
        out = []
        while fl:
            nxt = 0
            for e in fieldlists.get(fl, []):
                if e[0] == "chain":
                    nxt = e[1]
                else:
                    out.append(e)
            fl = nxt
        return out

    members, methods, bases, enumvals, classrows = [], [], [], [], []
    for cname, c in classes.items():
        ents = fl_entries(c["fl"])
        nm = nf = 0
        for e in ents:
            if e[0] == "member":
                _, mname, mti, minline, off, static = e
                members.append((cname, mname, minline or resolve(mti), off, static))
                nm += 1
            elif e[0] == "method":
                _, mname, fti, vft, attrs = e
                methods.append((cname, mname, sig(fti), attrs, vft))
                nf += 1
            elif e[0] == "overloads":
                _, mname, mlist = e
                for fti, vft, attrs in methodlists.get(mlist, []):
                    methods.append((cname, mname, sig(fti), attrs, vft))
                    nf += 1
            elif e[0] == "base":
                bases.append((cname, resolve(e[1]), e[2]))
        classrows.append((cname, c["kind"], c["size"], nm, nf))

    for ename, fl in enums.items():
        for e in fl_entries(fl):
            if e[0] == "enumval":
                enumvals.append((ename, e[1], e[2]))

    return classrows, bases, members, methods, enumvals


RE_SYM = re.compile(r"^\s+\d+ \| (S_\w+) \[size = \d+\] `(.*)`\s*$")


def parse_globals(pdb: Path, modules: dict):
    funcs, globs = [], []
    skind = sname = None
    for line in dump_lines(pdb, "globals"):
        m = RE_SYM.match(line)
        if m:
            skind, sname = m.group(1), m.group(2)
            continue
        if not skind or sname is None or is_noise(sname):
            continue
        if skind in ("S_PROCREF", "S_LPROCREF"):
            m = re.search(r"module = (\d+)", line)
            if m:
                funcs.append((sname, modules.get(int(m.group(1)), "")))
                skind = None
        elif skind in ("S_GDATA32", "S_LDATA32"):
            m = re.search(r"type = 0x[0-9A-F]+ \((.*)\), addr = (\d+:\d+)", line)
            if m:
                globs.append((sname, m.group(1), m.group(2), "data"))
                skind = None
        elif skind == "S_CONSTANT":
            m = re.search(r"type = 0x[0-9A-F]+ \((.*)\), value = (.+)", line)
            if m:
                globs.append((sname, m.group(1), m.group(2).strip(), "const"))
                skind = None
    return funcs, globs


def parse_modules(pdb: Path) -> dict:
    out = {}
    for line in dump_lines(pdb, "modules"):
        m = re.search(r"Mod (\d+) \| `(.*?)`", line)
        if m:
            stem = Path(m.group(2).replace("\\", "/")).stem
            out[int(m.group(1))] = stem
    return out


def build_db():
    print("parsing types stream (~30 s)...", file=sys.stderr)
    classrows, bases, members, methods, enumvals = parse_types(PDB)
    print(f"  {len(classrows)} classes, {len(members)} members, "
          f"{len(methods)} methods, {len(enumvals)} enum values", file=sys.stderr)
    modules = parse_modules(PDB)
    funcs, globs = parse_globals(PDB, modules)
    print(f"  {len(funcs)} compiled functions, {len(globs)} globals/constants",
          file=sys.stderr)

    DB.unlink(missing_ok=True)
    con = sqlite3.connect(DB)
    con.executescript("""
        CREATE TABLE classes(name TEXT, kind TEXT, size INT, nmembers INT, nmethods INT);
        CREATE TABLE bases(class TEXT, base TEXT, off INT);
        CREATE TABLE members(class TEXT, name TEXT, type TEXT, off INT, static INT);
        CREATE TABLE methods(class TEXT, name TEXT, sig TEXT, attrs TEXT, vft INT);
        CREATE TABLE enumvals(enum TEXT, name TEXT, value INT);
        CREATE TABLE funcs(name TEXT, module TEXT);
        CREATE TABLE globals(name TEXT, type TEXT, addr TEXT, kind TEXT);
    """)
    con.executemany("INSERT INTO classes VALUES(?,?,?,?,?)", classrows)
    con.executemany("INSERT INTO bases VALUES(?,?,?)", bases)
    con.executemany("INSERT INTO members VALUES(?,?,?,?,?)", members)
    con.executemany("INSERT INTO methods VALUES(?,?,?,?,?)", methods)
    con.executemany("INSERT INTO enumvals VALUES(?,?,?)", enumvals)
    con.executemany("INSERT INTO funcs VALUES(?,?)", set(funcs))
    con.executemany("INSERT INTO globals VALUES(?,?,?,?)", set(globs))
    for t, c in (("classes", "name"), ("members", "name"), ("members", "class"),
                 ("methods", "name"), ("methods", "class"), ("enumvals", "name"),
                 ("funcs", "name"), ("globals", "name")):
        con.execute(f"CREATE INDEX idx_{t}_{c} ON {t}({c} COLLATE NOCASE)")
    con.commit()
    con.close()
    print(f"wrote {DB} ({DB.stat().st_size >> 20} MB)", file=sys.stderr)


# ---------------------------------------------------------------- query

CAVEAT = "[BG2EE 2.5 PDB -- offsets/sizes are BG2's, NOT IWD2's]"


def cap(rows, limit):
    if limit and len(rows) > limit:
        return rows[:limit], len(rows) - limit
    return rows, 0


def fmt_method(name, sig_, attrs, vft):
    flags = [w for w in ("virtual", "pure", "static", "intro") if w in attrs]
    if vft >= 0:
        flags.append(f"vft+{vft}")
    tail = ("  [" + " ".join(flags) + "]") if flags else ""
    return f"{name}{sig_}{tail}"


def class_view(con, cname, limit):
    row = con.execute(
        "SELECT name, kind, size FROM classes WHERE name = ? COLLATE NOCASE",
        (cname,)).fetchone()
    if not row:
        rows = con.execute(
            "SELECT name FROM classes WHERE name LIKE ? COLLATE NOCASE "
            "ORDER BY length(name) LIMIT 10", (f"%{cname}%",)).fetchall()
        if not rows:
            print(f"no BG2 class matching '{cname}'")
            return
        print(f"no exact class '{cname}'; close: " + ", ".join(r[0] for r in rows))
        return
    name, kindw, size = row
    print(f"{name}  {kindw}  sizeof {size}  {CAVEAT}")
    for b, off in con.execute(
            "SELECT base, off FROM bases WHERE class = ? ORDER BY off", (name,)):
        print(f"  : {b} @{off}")
    mem = con.execute(
        "SELECT off, type, name, static FROM members WHERE class = ? "
        "ORDER BY static, off", (name,)).fetchall()
    mem, more = cap(mem, limit)
    for off, ty, n, static in mem:
        loc = "static" if static else f"@{off}"
        print(f"  {loc:>6}  {ty}  {n}")
    if more:
        print(f"  ... +{more} members (-l 0 for all)")
    met = con.execute(
        "SELECT name, sig, attrs, vft FROM methods WHERE class = ? "
        "ORDER BY (vft < 0), vft, name", (name,)).fetchall()
    met, more = cap(met, limit)
    for n, s, attrs, vft in met:
        print(f"  {fmt_method(n, s, attrs, vft)}")
    if more:
        print(f"  ... +{more} methods (-l 0 for all)")


def search(con, q, limit, include_noise):
    like = f"%{q}%"
    noise = "" if include_noise else " AND NOT noise"
    total = 0

    def emit(label, rows, fmt):
        nonlocal total
        rows, more = cap(rows, limit)
        for r in rows:
            print(f"{label:6} {fmt(r)}")
        if more:
            print(f"{label:6} ... +{more} more (-l 0 for all)")
        total += len(rows)

    emit("class", con.execute(
        "SELECT name, kind, size FROM classes WHERE name LIKE ? COLLATE NOCASE "
        "ORDER BY length(name)", (like,)).fetchall(),
        lambda r: f"{r[0]}  {r[1]} sizeof {r[2]}   ({r[0]}:: for layout)")
    emit("method", con.execute(
        "SELECT class, name, sig, attrs, vft FROM methods "
        "WHERE name LIKE ? COLLATE NOCASE ORDER BY class, name", (like,)).fetchall(),
        lambda r: f"{r[0]}::{fmt_method(r[1], r[2], r[3], r[4])}")
    emit("member", con.execute(
        "SELECT class, name, type, off, static FROM members "
        "WHERE name LIKE ? COLLATE NOCASE ORDER BY class, off", (like,)).fetchall(),
        lambda r: f"{r[0]}::{r[1]}  {r[2]}  " + ("static" if r[4] else f"@{r[3]}"))
    emit("fn", con.execute(
        "SELECT name, module FROM funcs WHERE name LIKE ? COLLATE NOCASE "
        "AND name NOT LIKE '%::%' ORDER BY name", (like,)).fetchall(),
        lambda r: f"{r[0]}  [{r[1]}]")
    emit("global", con.execute(
        "SELECT name, type, addr, kind FROM globals WHERE name LIKE ? COLLATE NOCASE "
        "ORDER BY name", (like,)).fetchall(),
        lambda r: f"{r[0]}  {r[1]}  " + (f"= {r[2]}" if r[3] == "const" else ""))
    emit("enum", con.execute(
        "SELECT enum, name, value FROM enumvals WHERE name LIKE ? OR enum LIKE ? "
        "COLLATE NOCASE ORDER BY enum, value", (like, like)).fetchall(),
        lambda r: f"{r[0]}::{r[1]} = {r[2]}")
    if total == 0:
        print(f"no BG2 symbol matching '{q}'")
    else:
        print(CAVEAT)


def class_scoped(con, cname, sub, limit):
    like = f"%{sub}%"
    rows = con.execute(
        "SELECT name, sig, attrs, vft FROM methods WHERE class = ? COLLATE NOCASE "
        "AND name LIKE ? COLLATE NOCASE ORDER BY name", (cname, like)).fetchall()
    for n, s, attrs, vft in rows:
        print(f"{cname}::{fmt_method(n, s, attrs, vft)}")
    mrows = con.execute(
        "SELECT name, type, off, static FROM members WHERE class = ? COLLATE NOCASE "
        "AND name LIKE ? COLLATE NOCASE ORDER BY off", (cname, like)).fetchall()
    for n, ty, off, static in mrows:
        print(f"{cname}::{n}  {ty}  " + ("static" if static else f"@{off}"))
    if not rows and not mrows:
        print(f"nothing matching '{sub}' in BG2 {cname} (try '{cname}::' for full view)")
    else:
        print(CAVEAT)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("query", nargs="?", help="NAME | Class:: | Class::sub")
    ap.add_argument("--build", action="store_true", help="rebuild index from PDB or bundled dumps")
    ap.add_argument("--enum", help="enum values by enum-name substring")
    ap.add_argument("-l", type=int, default=25, metavar="N",
                    help="line cap per section (0 = unlimited, default 25)")
    ap.add_argument("--all", action="store_true", help="include std::/libjingle noise")
    a = ap.parse_args()

    if a.build or not DB.exists():
        build_db()
        if not a.query and not a.enum:
            return 0
    con = sqlite3.connect(DB)

    if a.enum:
        rows = con.execute(
            "SELECT enum, name, value FROM enumvals WHERE enum LIKE ? COLLATE NOCASE "
            "ORDER BY enum, value", (f"%{a.enum}%",)).fetchall()
        rows, more = cap(rows, a.l)
        for e, n, v in rows:
            print(f"{e}::{n} = {v}")
        if more:
            print(f"... +{more} more (-l 0 for all)")
        return 0

    if not a.query:
        ap.print_help()
        return 1

    q = a.query
    if q.endswith("::"):
        class_view(con, q[:-2], a.l if a.l else 0)
    elif "::" in q:
        cname, sub = q.split("::", 1)
        class_scoped(con, cname, sub, a.l)
    else:
        search(con, q, a.l, a.all)
    return 0


if __name__ == "__main__":
    sys.exit(main())
