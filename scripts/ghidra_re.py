#!/usr/bin/env python3
"""
Small GhidraSQL HTTP helper for IWD2 RE.

Default target: http://127.0.0.1:8081/query

Examples:
  python scripts/ghidra_re.py decomp 0x5D2DE0
  python scripts/ghidra_re.py func 0x5D2DE0 RenderFogOfWar
  python scripts/ghidra_re.py local 0x5D2DE0 local_8 --name pArea --type "CGameArea *"
  python scripts/ghidra_re.py param 0x5D2DE0 0 --name pVidMode --type "CVidMode *"
  python scripts/ghidra_re.py comment 0x5D2DE0 plate "Renders fog of war overlay."
  python scripts/ghidra_re.py bookmark 0x5D2DE0 review "Check blend flags."
  python scripts/ghidra_re.py source-notes --replace
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any, Iterable

DEFAULT_URL = os.environ.get("GHIDRASQL_URL", "http://127.0.0.1:8081")
REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_DIR = REPO_ROOT / "src"

ADDR_RE = re.compile(r"//\s*0x([0-9A-Fa-f]{6,8})")
NOTE_RE = re.compile(r"\b(TODO|FIXME)\b[: ]", re.IGNORECASE)


def sql_quote(value: str) -> str:
    return "'" + value.replace("'", "''") + "'"


def parse_addr(value: str | int) -> int:
    if isinstance(value, int):
        return value
    value = value.strip()
    return int(value, 16) if value.lower().startswith("0x") else int(value, 10)


def addr_sql(value: str | int) -> str:
    return f"0x{parse_addr(value):X}"


class GhidraSQL:
    def __init__(self, url: str, timeout: int = 60, dry_run: bool = False, verbose: bool = False) -> None:
        self.url = url.rstrip("/")
        self.timeout = timeout
        self.dry_run = dry_run
        self.verbose = verbose

    def query(self, sql: str) -> dict[str, Any]:
        if self.verbose or self.dry_run:
            print(sql.rstrip(";") + ";")
        if self.dry_run:
            return {"success": True, "results": []}

        data = sql.encode("utf-8")
        req = urllib.request.Request(
            self.url + "/query",
            data=data,
            method="POST",
            headers={"Content-Type": "text/plain; charset=utf-8"},
        )
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as resp:
                payload = json.loads(resp.read().decode("utf-8", errors="replace"))
        except urllib.error.URLError as exc:
            raise SystemExit(f"GhidraSQL query failed: {exc}") from exc

        if not payload.get("success", False):
            raise SystemExit(json.dumps(payload, indent=2))

        for result in payload.get("results", []):
            if not result.get("success", False):
                raise SystemExit(json.dumps(result, indent=2))

        return payload

    def save(self) -> None:
        self.query("SELECT save_database();")


def first_result(payload: dict[str, Any]) -> dict[str, Any] | None:
    results = payload.get("results") or []
    return results[0] if results else None


def print_table(payload: dict[str, Any]) -> None:
    result = first_result(payload)
    if not result:
        return
    columns = result.get("columns") or []
    rows = result.get("rows") or []
    if columns:
        print("\t".join(str(c) for c in columns))
    for row in rows:
        print("\t".join("" if v is None else str(v) for v in row))


def run_decomp(db: GhidraSQL, args: argparse.Namespace) -> bool:
    payload = db.query(f"SELECT text FROM pseudocode WHERE func_addr = {addr_sql(args.address)};")
    result = first_result(payload)
    rows = result.get("rows", []) if result else []
    if rows:
        print(rows[0][0])
    return False


def run_locals(db: GhidraSQL, args: argparse.Namespace) -> bool:
    payload = db.query(
        "SELECT local_id, name, type "
        f"FROM decomp_lvars WHERE func_addr = {addr_sql(args.address)} ORDER BY local_id;"
    )
    print_table(payload)
    return False


def run_params(db: GhidraSQL, args: argparse.Namespace) -> bool:
    payload = db.query(
        "SELECT ordinal, param_name, param_type "
        f"FROM function_params WHERE func_addr = {addr_sql(args.address)} ORDER BY ordinal;"
    )
    print_table(payload)
    return False


def run_func(db: GhidraSQL, args: argparse.Namespace) -> bool:
    sets = [f"name = {sql_quote(args.name)}"]
    if args.signature:
        sets.append(f"signature = {sql_quote(args.signature)}")
    db.query(f"UPDATE funcs SET {', '.join(sets)} WHERE address = {addr_sql(args.address)};")
    print_table(db.query(f"SELECT name, signature FROM funcs WHERE address = {addr_sql(args.address)};"))
    return True


def run_signature(db: GhidraSQL, args: argparse.Namespace) -> bool:
    db.query(f"UPDATE funcs SET signature = {sql_quote(args.prototype)} WHERE address = {addr_sql(args.address)};")
    print_table(db.query(f"SELECT name, signature FROM funcs WHERE address = {addr_sql(args.address)};"))
    return True


def run_param(db: GhidraSQL, args: argparse.Namespace) -> bool:
    sets: list[str] = []
    if args.name is not None:
        sets.append(f"param_name = {sql_quote(args.name)}")
    if args.type is not None:
        sets.append(f"param_type = {sql_quote(args.type)}")
    if not sets:
        raise SystemExit("param needs --name and/or --type")
    db.query(
        f"UPDATE function_params SET {', '.join(sets)} "
        f"WHERE func_addr = {addr_sql(args.func_addr)} AND ordinal = {int(args.ordinal)};"
    )
    print_table(
        db.query(
            "SELECT ordinal, param_name, param_type FROM function_params "
            f"WHERE func_addr = {addr_sql(args.func_addr)} ORDER BY ordinal;"
        )
    )
    return True


def run_local(db: GhidraSQL, args: argparse.Namespace) -> bool:
    if args.name is None and args.type is None:
        raise SystemExit("local needs --name and/or --type")
    if args.name is not None:
        db.query(f"SELECT rename_local({addr_sql(args.func_addr)}, {sql_quote(args.local_id)}, {sql_quote(args.name)});")
    if args.type is not None:
        db.query(f"SELECT set_local_type({addr_sql(args.func_addr)}, {sql_quote(args.local_id)}, {sql_quote(args.type)});")
    print_table(
        db.query(
            "SELECT local_id, name, type FROM decomp_lvars "
            f"WHERE func_addr = {addr_sql(args.func_addr)} ORDER BY local_id;"
        )
    )
    return True


def run_comment(db: GhidraSQL, args: argparse.Namespace) -> bool:
    repeatable = 1 if args.kind == "repeatable" else 0
    if args.replace:
        db.query(f"DELETE FROM comments WHERE address = {addr_sql(args.address)} AND source = {sql_quote(args.kind)};")
    db.query(
        "INSERT INTO comments (address, comment, repeatable, source) VALUES "
        f"({addr_sql(args.address)}, {sql_quote(args.text)}, {repeatable}, {sql_quote(args.kind)});"
    )
    return True


def run_decomp_comment(db: GhidraSQL, args: argparse.Namespace) -> bool:
    if args.replace:
        db.query(
            "DELETE FROM decomp_comments "
            f"WHERE func_addr = {addr_sql(args.func_addr)} AND address = {addr_sql(args.address)} "
            f"AND source = {sql_quote(args.kind)};"
        )
    db.query(
        "INSERT INTO decomp_comments (func_addr, address, comment, source) VALUES "
        f"({addr_sql(args.func_addr)}, {addr_sql(args.address)}, {sql_quote(args.text)}, {sql_quote(args.kind)});"
    )
    return True


def run_bookmark(db: GhidraSQL, args: argparse.Namespace) -> bool:
    if args.replace:
        db.query(
            "DELETE FROM bookmarks "
            f"WHERE address = {addr_sql(args.address)} AND type = {sql_quote(args.type)} "
            f"AND category = {sql_quote(args.category)};"
        )
    db.query(
        "INSERT INTO bookmarks (address, type, category, comment) VALUES "
        f"({addr_sql(args.address)}, {sql_quote(args.type)}, {sql_quote(args.category)}, {sql_quote(args.text)});"
    )
    return True


def run_tag(db: GhidraSQL, args: argparse.Namespace) -> bool:
    count_payload = db.query(f"SELECT COUNT(*) FROM function_tags WHERE name = {sql_quote(args.tag)};")
    result = first_result(count_payload)
    rows = result.get("rows", []) if result else []
    exists = bool(rows and int(rows[0][0]) > 0)
    if not exists:
        db.query(
            "INSERT INTO function_tags (name, comment) VALUES "
            f"({sql_quote(args.tag)}, {sql_quote(args.comment or '')});"
        )
    db.query(
        "DELETE FROM function_tag_mappings "
        f"WHERE func_addr = {addr_sql(args.func_addr)} AND tag_name = {sql_quote(args.tag)};"
    )
    db.query(
        "INSERT INTO function_tag_mappings (func_addr, tag_name) VALUES "
        f"({addr_sql(args.func_addr)}, {sql_quote(args.tag)});"
    )
    return True


def iter_source_files(src: Path) -> Iterable[Path]:
    for path in src.rglob("*"):
        if path.suffix.lower() in {".cpp", ".h"}:
            yield path


def collect_source_notes(src: Path, max_per_addr_kind: int = 12) -> dict[tuple[int, str], list[str]]:
    notes: dict[tuple[int, str], list[str]] = {}
    for path in sorted(iter_source_files(src)):
        rel = path.relative_to(REPO_ROOT).as_posix()
        current_addr: int | None = None
        try:
            lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            continue
        for line_no, line in enumerate(lines, 1):
            addr_match = ADDR_RE.search(line)
            if addr_match:
                current_addr = int(addr_match.group(1), 16)

            note_match = NOTE_RE.search(line)
            if not note_match or current_addr is None:
                continue

            kind = note_match.group(1).upper()
            text = line.strip()
            entry = f"{rel}:{line_no}: {text}"
            bucket = notes.setdefault((current_addr, kind), [])
            if len(bucket) < max_per_addr_kind:
                bucket.append(entry)
            elif len(bucket) == max_per_addr_kind:
                bucket.append("...")
    return notes


def run_source_notes(db: GhidraSQL, args: argparse.Namespace) -> bool:
    src = Path(args.src).resolve() if args.src else SRC_DIR
    notes = collect_source_notes(src)
    if args.kind:
        wanted = args.kind.upper()
        notes = {k: v for k, v in notes.items() if k[1] == wanted}

    if args.limit:
        notes = dict(list(sorted(notes.items()))[: int(args.limit)])

    print(f"source notes: {len(notes)} address/category bookmarks")
    if args.replace:
        db.query("DELETE FROM bookmarks WHERE category IN ('TODO', 'FIXME') AND comment LIKE '[src] %';")

    for (address, kind), entries in sorted(notes.items()):
        comment = "[src] " + " | ".join(entries)
        if len(comment) > args.max_comment:
            comment = comment[: args.max_comment - 3] + "..."
        db.query(
            "INSERT INTO bookmarks (address, type, category, comment) VALUES "
            f"(0x{address:X}, 'Note', {sql_quote(kind)}, {sql_quote(comment)});"
        )
    return True


def load_batch(path: Path) -> list[dict[str, Any]]:
    text = path.read_text(encoding="utf-8")
    stripped = text.lstrip()
    if stripped.startswith("["):
        data = json.loads(text)
        if not isinstance(data, list):
            raise SystemExit("batch JSON must be a list")
        return data
    ops = []
    for line_no, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        try:
            obj = json.loads(line)
        except json.JSONDecodeError as exc:
            raise SystemExit(f"{path}:{line_no}: invalid JSONL: {exc}") from exc
        ops.append(obj)
    return ops


def op_namespace(op: dict[str, Any]) -> argparse.Namespace:
    return argparse.Namespace(**op)


def run_batch(db: GhidraSQL, args: argparse.Namespace) -> bool:
    dispatch = {
        "func": run_func,
        "signature": run_signature,
        "param": run_param,
        "local": run_local,
        "comment": run_comment,
        "decomp-comment": run_decomp_comment,
        "bookmark": run_bookmark,
        "tag": run_tag,
    }
    changed = False
    for i, op in enumerate(load_batch(Path(args.file)), 1):
        name = op.pop("op", None)
        if name not in dispatch:
            raise SystemExit(f"batch op #{i}: unknown op {name!r}")
        print(f"[{i}] {name}")
        # Defaults shared by comment/bookmark operations.
        op.setdefault("replace", False)
        op.setdefault("kind", "plate")
        op.setdefault("type", "Note")
        op.setdefault("category", "re")
        changed = dispatch[name](db, op_namespace(op)) or changed
    return changed


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Rename/annotate the IWD2 Ghidra project through ghidrasql HTTP.")
    parser.add_argument("--url", default=DEFAULT_URL, help=f"GhidraSQL base URL (default: {DEFAULT_URL})")
    parser.add_argument("--timeout", type=int, default=60)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--no-save", action="store_true", help="Do not run SELECT save_database() after mutations")

    sub = parser.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("decomp", help="Print pseudocode for a function")
    p.add_argument("address")
    p.set_defaults(func=run_decomp)

    p = sub.add_parser("locals", help="List decompiler locals with local_id")
    p.add_argument("address")
    p.set_defaults(func=run_locals)

    p = sub.add_parser("params", help="List function parameters")
    p.add_argument("address")
    p.set_defaults(func=run_params)

    p = sub.add_parser("func", help="Rename a function and optionally set signature")
    p.add_argument("address")
    p.add_argument("name")
    p.add_argument("--signature")
    p.set_defaults(func=run_func)

    p = sub.add_parser("signature", help="Set full function prototype/signature")
    p.add_argument("address")
    p.add_argument("prototype")
    p.set_defaults(func=run_signature)

    p = sub.add_parser("param", help="Rename/retype a function parameter")
    p.add_argument("func_addr")
    p.add_argument("ordinal", type=int)
    p.add_argument("--name")
    p.add_argument("--type")
    p.set_defaults(func=run_param)

    p = sub.add_parser("local", help="Rename/retype a decompiler local")
    p.add_argument("func_addr")
    p.add_argument("local_id")
    p.add_argument("--name")
    p.add_argument("--type")
    p.set_defaults(func=run_local)

    p = sub.add_parser("comment", help="Add code-unit comment")
    p.add_argument("address")
    p.add_argument("kind", choices=["plate", "pre", "post", "eol", "repeatable"])
    p.add_argument("text")
    p.add_argument("--replace", action="store_true")
    p.set_defaults(func=run_comment)

    p = sub.add_parser("decomp-comment", help="Add decompiler-token comment")
    p.add_argument("func_addr")
    p.add_argument("address")
    p.add_argument("kind", choices=["pre", "post", "eol", "plate", "repeatable"])
    p.add_argument("text")
    p.add_argument("--replace", action="store_true")
    p.set_defaults(func=run_decomp_comment)

    p = sub.add_parser("bookmark", help="Add bookmark")
    p.add_argument("address")
    p.add_argument("category")
    p.add_argument("text")
    p.add_argument("--type", default="Note")
    p.add_argument("--replace", action="store_true")
    p.set_defaults(func=run_bookmark)

    p = sub.add_parser("tag", help="Attach a function tag")
    p.add_argument("func_addr")
    p.add_argument("tag")
    p.add_argument("--comment")
    p.set_defaults(func=run_tag)

    p = sub.add_parser("source-notes", help="Import source TODO/FIXME as Ghidra bookmarks")
    p.add_argument("--src", default=str(SRC_DIR))
    p.add_argument("--replace", action="store_true", help="Delete previous [src] TODO/FIXME bookmarks first")
    p.add_argument("--kind", choices=["TODO", "FIXME"])
    p.add_argument("--limit", type=int)
    p.add_argument("--max-comment", type=int, default=900)
    p.set_defaults(func=run_source_notes)

    p = sub.add_parser("batch", help="Apply JSON array or JSONL batch")
    p.add_argument("file")
    p.set_defaults(func=run_batch)

    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    db = GhidraSQL(args.url, timeout=args.timeout, dry_run=args.dry_run, verbose=args.verbose)
    changed = args.func(db, args)
    if changed and not args.no_save:
        db.save()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
