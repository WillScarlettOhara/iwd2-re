#!/usr/bin/env python3
"""Resolve IWD2 resource names from asset StringRefs and dialog.tlk.

The compact output is intended for recovered-code comments and commit messages::

    python scripts/reagent_asset_names.py SPWI304 AR1000 60SPELLS
    Fireball (SPWI304.SPL, strref 6618)
    Targos Docks (AR1000.ARE, strref 10456)
    Sheemish's Spell Library (60SPELLS.STO, strref 37760)

Use ``--json`` when another script or an LLM driver needs structured fields. Areas
are resolved through WMP area entries because ARE files do not contain their world
map display names.
"""
from __future__ import annotations

import argparse
import json
import os
import struct
import sys
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Iterable

from runtime_paths import game_directory


REPO = Path(__file__).resolve().parent.parent
DEFAULT_ASSETS = REPO / "data" / "near_infinity_export"
SUPPORTED_TYPES = ("SPL", "ITM", "CRE", "STO", "ARE")
INVALID_STRREFS = {-1, 0xFFFFFFFF, 9999999}


@dataclass(frozen=True)
class TlkEntry:
    strref: int
    text: str
    flags: int
    sound: str


class TlkFile:
    ENTRY_SIZE = 26

    def __init__(self, path: Path, encoding: str = "cp1252") -> None:
        self.path = path
        self.encoding = encoding
        self._data = path.read_bytes()
        if len(self._data) < 18 or self._data[:8] != b"TLK V1  ":
            raise ValueError(f"not a TLK V1 file: {path}")
        self.language_id, self.count, self.string_offset = struct.unpack_from(
            "<HII", self._data, 8
        )
        entries_end = 18 + self.count * self.ENTRY_SIZE
        valid_offsets = entries_end <= self.string_offset <= len(self._data)
        if entries_end > len(self._data) or not valid_offsets:
            raise ValueError(f"invalid TLK offsets in {path}")

    def get(self, strref: int) -> TlkEntry | None:
        if strref in INVALID_STRREFS or not 0 <= strref < self.count:
            return None
        pos = 18 + strref * self.ENTRY_SIZE
        flags = struct.unpack_from("<H", self._data, pos)[0]
        sound = _resref(self._data[pos + 2 : pos + 10])
        text_offset, text_length = struct.unpack_from("<II", self._data, pos + 18)
        start = self.string_offset + text_offset
        end = start + text_length
        if not self.string_offset <= start <= end <= len(self._data):
            raise ValueError(f"invalid TLK entry {strref} in {self.path}")
        text = self._data[start:end].decode(self.encoding, errors="replace")
        return TlkEntry(strref=strref, text=text, flags=flags, sound=sound)

    def search(self, needle: str) -> Iterable[TlkEntry]:
        folded = needle.casefold()
        for strref in range(self.count):
            entry = self.get(strref)
            if entry and folded in entry.text.casefold():
                yield entry


@dataclass(frozen=True)
class FieldValue:
    field: str
    strref: int
    text: str | None


@dataclass
class AssetResult:
    resource: str
    type: str
    name: str | None
    strref: int | None
    source: str
    fields: list[FieldValue] = field(default_factory=list)
    metadata: dict[str, str] = field(default_factory=dict)

    @property
    def identifier(self) -> str:
        return f"{self.resource}.{self.type}"

    def compact(self) -> str:
        if self.name is None:
            return f"{self.identifier} (no valid StringRef)"
        name = _one_line(self.name)
        return f"{name} ({self.identifier}, strref {self.strref})"

    def to_json(self) -> dict:
        value = asdict(self)
        value["identifier"] = self.identifier
        return value


@dataclass(frozen=True)
class AssetSpec:
    signature: bytes
    fields: tuple[tuple[str, int], ...]
    preferred: tuple[str, ...]


ASSET_SPECS = {
    "SPL": AssetSpec(
        b"SPL ",
        (("name", 8), ("identified_name", 12), ("description", 80),
         ("identified_description", 84)),
        ("name", "identified_name"),
    ),
    "ITM": AssetSpec(
        b"ITM ",
        (("unidentified_name", 8), ("identified_name", 12),
         ("unidentified_description", 80), ("identified_description", 84)),
        ("identified_name", "unidentified_name"),
    ),
    "CRE": AssetSpec(
        b"CRE ",
        (("long_name", 8), ("tooltip", 12)),
        ("long_name", "tooltip"),
    ),
    "STO": AssetSpec(
        b"STOR",
        (("name", 12),),
        ("name",),
    ),
}


def _resref(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("ascii", errors="replace").strip().upper()


def _one_line(text: str) -> str:
    return " ".join(text.replace("\x00", "").split())


def _read_u32(data: bytes, offset: int, source: Path) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ValueError(f"offset {offset:#x} outside {source}")
    return struct.unpack_from("<I", data, offset)[0]


def default_tlk_candidates() -> list[Path]:
    candidates: list[Path] = []
    if value := os.environ.get("IWD2_TLK"):
        candidates.append(Path(value).expanduser())
    candidates.extend(
        [
            DEFAULT_ASSETS / "dialog.tlk",
            game_directory() / "dialog.tlk",
            Path.home() / "Games" / "Heroic" / "Icewind Dale 2" / "dialog.tlk",
            Path.home() / "Games" / "Heroic" / "Icewind Dale 2.backup" / "dialog.tlk",
        ]
    )
    return candidates


def find_default_tlk() -> Path:
    for path in default_tlk_candidates():
        if path.is_file():
            return path
    checked = "\n  ".join(str(path) for path in default_tlk_candidates())
    raise FileNotFoundError(
        "dialog.tlk not found; pass --tlk or set IWD2_TLK. Checked:\n  " + checked
    )


def read_asset(path: Path, asset_type: str, tlk: TlkFile) -> AssetResult:
    spec = ASSET_SPECS[asset_type]
    data = path.read_bytes()
    if len(data) < 8 or data[:4] != spec.signature:
        raise ValueError(f"expected {spec.signature!r} signature in {path}")

    fields: list[FieldValue] = []
    by_name: dict[str, FieldValue] = {}
    for field_name, offset in spec.fields:
        strref = _read_u32(data, offset, path)
        entry = tlk.get(strref)
        value = FieldValue(field_name, strref, entry.text if entry else None)
        fields.append(value)
        by_name[field_name] = value

    chosen = next(
        (by_name[name] for name in spec.preferred if by_name[name].text),
        None,
    )
    return AssetResult(
        resource=path.stem.upper(),
        type=asset_type,
        name=chosen.text if chosen else None,
        strref=chosen.strref if chosen else None,
        source=str(path),
        fields=fields,
        metadata={"version": data[4:8].decode("ascii", errors="replace").strip()},
    )


def read_wmp_areas(path: Path, tlk: TlkFile) -> list[AssetResult]:
    data = path.read_bytes()
    if len(data) < 16 or data[:4] != b"WMAP":
        raise ValueError(f"expected WMAP signature in {path}")
    map_count = _read_u32(data, 8, path)
    map_offset = _read_u32(data, 12, path)
    results: list[AssetResult] = []

    for map_index in range(map_count):
        pos = map_offset + map_index * 184
        if pos + 184 > len(data):
            raise ValueError(f"world map entry {map_index} outside {path}")
        map_resref = _resref(data[pos : pos + 8])
        area_count = _read_u32(data, pos + 32, path)
        area_offset = _read_u32(data, pos + 36, path)
        for area_index in range(area_count):
            area_pos = area_offset + area_index * 240
            if area_pos + 240 > len(data):
                raise ValueError(f"area entry {area_index} outside {path}")
            current = _resref(data[area_pos : area_pos + 8])
            original = _resref(data[area_pos + 8 : area_pos + 16])
            name_ref = _read_u32(data, area_pos + 64, path)
            tooltip_ref = _read_u32(data, area_pos + 68, path)
            name = tlk.get(name_ref)
            tooltip = tlk.get(tooltip_ref)
            chosen = name or tooltip
            fields = [
                FieldValue("name", name_ref, name.text if name else None),
                FieldValue("tooltip", tooltip_ref, tooltip.text if tooltip else None),
            ]
            aliases = sorted({value for value in (current, original) if value})
            for resource in aliases:
                results.append(
                    AssetResult(
                        resource=resource,
                        type="ARE",
                        name=chosen.text if chosen else None,
                        strref=chosen.strref if chosen else None,
                        source=str(path),
                        fields=fields,
                        metadata={
                            "world_map": map_resref,
                            "current_area": current,
                            "original_area": original,
                        },
                    )
                )
    return results


class AssetResolver:
    def __init__(self, assets: Path, tlk: TlkFile) -> None:
        self.assets = assets
        self.tlk = tlk
        self._area_index: dict[str, list[AssetResult]] | None = None

    def resolve(self, query: str, type_hint: str | None = None) -> list[AssetResult]:
        supplied = Path(query).expanduser()
        if supplied.is_file():
            asset_type = type_hint or supplied.suffix.lstrip(".").upper()
            if asset_type == "ARE":
                return self._resolve_area(supplied.stem)
            if asset_type not in ASSET_SPECS:
                raise ValueError(f"unsupported asset type: {asset_type or '(none)'}")
            return [read_asset(supplied, asset_type, self.tlk)]

        raw = supplied.name
        suffix = supplied.suffix.lstrip(".").upper()
        resource = supplied.stem.upper() if suffix else raw.upper()
        asset_type = (type_hint or suffix).upper()
        if asset_type:
            return self._resolve_typed(resource, asset_type)

        matches: list[AssetResult] = []
        for candidate_type in SUPPORTED_TYPES:
            try:
                matches.extend(self._resolve_typed(resource, candidate_type))
            except FileNotFoundError:
                continue
        if not matches:
            raise FileNotFoundError(f"resource not found: {query}")
        return matches

    def _resolve_typed(self, resource: str, asset_type: str) -> list[AssetResult]:
        if asset_type == "ARE":
            return self._resolve_area(resource)
        if asset_type not in ASSET_SPECS:
            raise ValueError(f"unsupported asset type: {asset_type}")
        path = self.assets / asset_type / f"{resource}.{asset_type}"
        if not path.is_file():
            raise FileNotFoundError(f"resource not found: {resource}.{asset_type}")
        return [read_asset(path, asset_type, self.tlk)]

    def _resolve_area(self, resource: str) -> list[AssetResult]:
        if self._area_index is None:
            self._area_index = {}
            seen: set[tuple[str, int | None, str, str, str]] = set()
            wmp_dir = self.assets / "WMP"
            for path in sorted(wmp_dir.glob("*.WMP")):
                for result in read_wmp_areas(path, self.tlk):
                    key = (
                        result.resource,
                        result.strref,
                        result.metadata["world_map"],
                        result.metadata["current_area"],
                        result.metadata["original_area"],
                    )
                    if key in seen:
                        continue
                    seen.add(key)
                    self._area_index.setdefault(result.resource, []).append(result)
        matches = self._area_index.get(resource.upper(), [])
        if not matches:
            raise FileNotFoundError(f"area not found in WMP files: {resource}.ARE")
        return matches


def _print_verbose(result: AssetResult) -> None:
    print(result.compact())
    print(f"  source: {result.source}")
    for value in result.fields:
        text = _one_line(value.text) if value.text is not None else "<invalid>"
        print(f"  {value.field}: [{value.strref}] {text}")
    for key, value in result.metadata.items():
        print(f"  {key}: {value}")


def _integer(value: str) -> int:
    try:
        return int(value, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"invalid integer: {value}") from exc


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("resources", nargs="*", help="resource names, filenames, or paths")
    parser.add_argument("--type", choices=SUPPORTED_TYPES, help="type for extensionless resources")
    parser.add_argument(
        "--strref", type=_integer, action="append", default=[], help="resolve a raw StringRef"
    )
    parser.add_argument("--search", help="case-insensitive substring search in dialog.tlk")
    parser.add_argument(
        "--limit", type=int, default=20, help="maximum --search results (default: 20)"
    )
    parser.add_argument("--tlk", type=Path, help="path to dialog.tlk (or set IWD2_TLK)")
    parser.add_argument(
        "--assets", type=Path, default=DEFAULT_ASSETS, help="NearInfinity export root"
    )
    parser.add_argument("--encoding", default="cp1252", help="TLK text encoding (default: cp1252)")
    parser.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    parser.add_argument("--verbose", action="store_true", help="show every StringRef field")
    args = parser.parse_args(argv)

    if not args.resources and not args.strref and args.search is None:
        parser.error("provide a resource, --strref, or --search")

    try:
        tlk_path = args.tlk.expanduser() if args.tlk else find_default_tlk()
        tlk = TlkFile(tlk_path, args.encoding)
        resolver = AssetResolver(args.assets.expanduser(), tlk)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    payload: list[dict] = []
    failures = 0
    for query in args.resources:
        try:
            for result in resolver.resolve(query, args.type):
                payload.append(result.to_json())
                if not args.json:
                    _print_verbose(result) if args.verbose else print(result.compact())
        except (OSError, ValueError) as exc:
            failures += 1
            if args.json:
                payload.append({"query": query, "error": str(exc)})
            else:
                print(f"error: {exc}", file=sys.stderr)

    for strref in args.strref:
        entry = tlk.get(strref)
        item = {
            "strref": strref,
            "text": entry.text if entry else None,
            "sound": entry.sound if entry else None,
        }
        payload.append(item)
        if not args.json:
            text = _one_line(entry.text) if entry else "<invalid>"
            print(f"[{strref}] {text}")

    if args.search is not None:
        matches = list(tlk.search(args.search))[: max(args.limit, 0)]
        for entry in matches:
            payload.append(asdict(entry))
            if not args.json:
                print(f"[{entry.strref}] {_one_line(entry.text)}")

    if args.json:
        json.dump(payload, sys.stdout, ensure_ascii=False, indent=2)
        sys.stdout.write("\n")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
