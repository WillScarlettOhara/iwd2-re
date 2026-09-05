#!/usr/bin/env python3
"""Normalize and compare JSONL traces from the original and reconstructed game.

The generic Frida hook table emits one JSON object per event.  This tool makes
the noisy parts comparable (Frida message envelopes, process-local pointers,
timestamps and harness events), then reports frequency, order and argument
differences separately.

Examples:
  python scripts/frida_trace_diff.py compare orig.jsonl ours.jsonl
  python scripts/frida_trace_diff.py compare orig.jsonl ours.jsonl --ignore-order
  python scripts/frida_trace_diff.py normalize orig.jsonl -o orig.normalized.jsonl

Exit codes: 0 = match, 1 = behavioral difference, 2 = invalid/unproven input.
"""
from __future__ import annotations

import argparse
import fnmatch
import json
import math
import re
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


CONTROL_TAGS = {"HITS", "drive", "loaded", "ready", "sites"}
DEFAULT_IGNORED_FIELDS = {
    "bt", "elapsed", "elapsed_ms", "pid", "process_id", "thread",
    "thread_id", "threadid", "tid", "time", "timestamp", "ts",
}
TERMINAL_FAILURE_TAGS = {"assert", "assertion", "crash", "error", "exception", "fault"}
POINTER_NAMES = {"addr", "address", "eip", "returnaddress", "self", "this"}
HEX_POINTER = re.compile(r"^0x[0-9a-f]+$", re.IGNORECASE)
MISSING = object()


class TraceInputError(ValueError):
    pass


@dataclass(frozen=True)
class Event:
    tag: str
    fields: dict[str, Any]

    def json_object(self) -> dict[str, Any]:
        return {"tag": self.tag, **self.fields}


@dataclass
class Options:
    ignore_fields: tuple[str, ...] = ()
    include_tags: frozenset[str] = frozenset()
    ignore_tags: frozenset[str] = frozenset()
    keep_control: bool = False
    raw_pointers: bool = False
    tag_map: dict[str, str] | None = None


def _matches_field(path: str, leaf: str, patterns: Iterable[str]) -> bool:
    return any(fnmatch.fnmatchcase(path, pattern) or fnmatch.fnmatchcase(leaf, pattern)
               for pattern in patterns)


def _pointer_field(name: str) -> bool:
    folded = name.lower().replace("_", "")
    return (folded in POINTER_NAMES or name.lower().endswith(("_ptr", "_address"))
            or name.lower().startswith("ptr_"))


def _pointer_presence(value: Any) -> Any:
    if value is None:
        return "<null>"
    if isinstance(value, bool):
        return "<ptr>" if value else "<null>"
    if isinstance(value, (int, float)):
        return "<null>" if value == 0 else "<ptr>"
    if isinstance(value, str):
        folded = value.strip().lower()
        if folded in {"0", "null", "nullptr"}:
            return "<null>"
        if HEX_POINTER.fullmatch(folded):
            return "<null>" if int(folded, 16) == 0 else "<ptr>"
    return value


def _canonicalize(value: Any, path: str, options: Options) -> Any:
    if isinstance(value, dict):
        result = {}
        for key in sorted(value):
            leaf = str(key)
            child_path = f"{path}.{leaf}" if path else leaf
            if leaf.lower() in DEFAULT_IGNORED_FIELDS:
                continue
            if _matches_field(child_path, leaf, options.ignore_fields):
                continue
            child = value[key]
            if not options.raw_pointers and _pointer_field(leaf):
                child = _pointer_presence(child)
            result[leaf] = _canonicalize(child, child_path, options)
        return result
    if isinstance(value, list):
        return [_canonicalize(item, f"{path}[]", options) for item in value]
    return value


def _unwrap(row: Any, source: Path, line_number: int) -> dict[str, Any]:
    if not isinstance(row, dict):
        raise TraceInputError(f"{source}:{line_number}: expected a JSON object")
    if row.get("type") == "send" and isinstance(row.get("payload"), dict):
        return row["payload"]
    if set(row) == {"payload"} and isinstance(row["payload"], dict):
        return row["payload"]
    return row


def _event_from_row(row: dict[str, Any], source: Path, line_number: int,
                    options: Options) -> Event | None:
    tag_key = next((key for key in ("tag", "event", "name", "t") if key in row), None)
    if tag_key is None or not isinstance(row[tag_key], (str, int)):
        raise TraceInputError(f"{source}:{line_number}: event has no scalar tag/name/event/t")
    tag = str(row[tag_key])
    # Failure evidence must survive tag aliases and selection filters.
    if tag.lower() in TERMINAL_FAILURE_TAGS:
        return Event(tag, _canonicalize(
            {key: value for key, value in row.items() if key != tag_key}, tag, options))
    if options.tag_map:
        tag = options.tag_map.get(tag, tag)
    if not options.keep_control and tag in CONTROL_TAGS:
        return None
    if options.include_tags and tag not in options.include_tags:
        return None
    if tag in options.ignore_tags:
        return None
    fields = {key: value for key, value in row.items() if key != tag_key}
    fields = _canonicalize(fields, tag, options)
    return Event(tag, fields)


def load_trace(path: Path, options: Options) -> list[Event]:
    if not path.is_file():
        raise TraceInputError(f"missing trace: {path}")
    events: list[Event] = []
    for line_number, raw in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
        if not raw.strip():
            continue
        try:
            row = json.loads(raw)
        except json.JSONDecodeError as exc:
            raise TraceInputError(f"{path}:{line_number}: invalid JSON: {exc.msg}") from exc
        event = _event_from_row(_unwrap(row, path, line_number), path, line_number, options)
        if event is not None:
            events.append(event)
    if not events:
        raise TraceInputError(f"trace has no comparable events: {path}")
    return events


def _equal(left: Any, right: Any, float_tolerance: float) -> bool:
    if isinstance(left, bool) or isinstance(right, bool):
        return left == right
    if isinstance(left, (int, float)) and isinstance(right, (int, float)):
        if isinstance(left, float) or isinstance(right, float):
            return math.isclose(float(left), float(right), rel_tol=0.0,
                                abs_tol=float_tolerance)
        return left == right
    if type(left) is not type(right):
        return False
    if isinstance(left, dict):
        return (left.keys() == right.keys()
                and all(_equal(left[key], right[key], float_tolerance) for key in left))
    if isinstance(left, list):
        return (len(left) == len(right)
                and all(_equal(a, b, float_tolerance) for a, b in zip(left, right)))
    return left == right


def _value_diffs(left: Any, right: Any, path: str,
                 float_tolerance: float) -> Iterable[tuple[str, Any, Any]]:
    if left is MISSING or right is MISSING:
        yield path, left, right
        return
    if isinstance(left, dict) and isinstance(right, dict):
        for key in sorted(set(left) | set(right)):
            yield from _value_diffs(left.get(key, MISSING), right.get(key, MISSING),
                                    f"{path}.{key}" if path else key, float_tolerance)
        return
    if isinstance(left, list) and isinstance(right, list):
        common = min(len(left), len(right))
        for index in range(common):
            yield from _value_diffs(left[index], right[index], f"{path}[{index}]",
                                    float_tolerance)
        if len(left) != len(right):
            yield f"{path}.length", len(left), len(right)
        return
    if not _equal(left, right, float_tolerance):
        yield path, left, right


def _display(value: Any) -> str:
    if value is MISSING:
        return "<missing>"
    return json.dumps(value, ensure_ascii=False, separators=(",", ":"))


def compare(original: list[Event], reconstructed: list[Event], *,
            ignore_order: bool = False, count_tolerance: int = 0,
            float_tolerance: float = 0.0, max_diffs: int = 8) -> dict[str, Any]:
    original_counts = Counter(event.tag for event in original)
    reconstructed_counts = Counter(event.tag for event in reconstructed)
    count_diffs = []
    for tag in sorted(set(original_counts) | set(reconstructed_counts)):
        left, right = original_counts[tag], reconstructed_counts[tag]
        if abs(left - right) > count_tolerance:
            count_diffs.append({"tag": tag, "original": left, "reconstructed": right})

    original_order = [event.tag for event in original]
    reconstructed_order = [event.tag for event in reconstructed]
    order_match = original_order == reconstructed_order
    order_index = None
    if not order_match:
        common = min(len(original_order), len(reconstructed_order))
        order_index = next((i for i in range(common)
                            if original_order[i] != reconstructed_order[i]), common)

    by_tag_original: dict[str, list[Event]] = {}
    by_tag_reconstructed: dict[str, list[Event]] = {}
    for event in original:
        by_tag_original.setdefault(event.tag, []).append(event)
    for event in reconstructed:
        by_tag_reconstructed.setdefault(event.tag, []).append(event)

    argument_diffs = []
    argument_diff_count = 0
    for tag in sorted(set(by_tag_original) & set(by_tag_reconstructed)):
        pairs = zip(by_tag_original[tag], by_tag_reconstructed[tag])
        for occurrence, (left, right) in enumerate(pairs, 1):
            for field, left_value, right_value in _value_diffs(
                    left.fields, right.fields, "", float_tolerance):
                argument_diff_count += 1
                if len(argument_diffs) < max_diffs:
                    argument_diffs.append({
                        "tag": tag, "occurrence": occurrence, "field": field,
                        "original": "<missing>" if left_value is MISSING else left_value,
                        "reconstructed": "<missing>" if right_value is MISSING else right_value,
                    })

    terminal = sorted({event.tag for event in original + reconstructed
                       if event.tag.lower() in TERMINAL_FAILURE_TAGS})
    different = bool(count_diffs or argument_diff_count or terminal
                     or (not ignore_order and not order_match))
    return {
        "verdict": "DIFFERENT" if different else "MATCH",
        "events": {"original": len(original), "reconstructed": len(reconstructed)},
        "frequencies": {"match": not count_diffs, "differences": count_diffs},
        "order": {
            "checked": not ignore_order,
            "match": order_match,
            "first_difference": order_index,
            "original": None if order_index is None or order_index >= len(original_order)
                        else original_order[order_index],
            "reconstructed": None if order_index is None or order_index >= len(reconstructed_order)
                             else reconstructed_order[order_index],
        },
        "arguments": {"match": argument_diff_count == 0,
                      "difference_count": argument_diff_count,
                      "differences": argument_diffs},
        "terminal_failures": terminal,
    }


def print_summary(result: dict[str, Any]) -> None:
    events = result["events"]
    print(f"TRACE-DIFF {result['verdict']} events={events['original']}/{events['reconstructed']}")
    frequencies = result["frequencies"]
    if frequencies["match"]:
        print("  frequency: match")
    else:
        preview = ", ".join(
            f"{item['tag']} {item['original']}|{item['reconstructed']}"
            for item in frequencies["differences"][:6])
        extra = len(frequencies["differences"]) - 6
        print(f"  frequency: {preview}" + (f" (+{extra})" if extra > 0 else ""))
    order = result["order"]
    if not order["checked"]:
        print("  order: ignored")
    elif order["match"]:
        print("  order: match")
    else:
        print(f"  order: first difference #{order['first_difference'] + 1} "
              f"{order['original'] or '<end>'}|{order['reconstructed'] or '<end>'}")
    arguments = result["arguments"]
    if arguments["match"]:
        print("  arguments: match")
    else:
        print(f"  arguments: {arguments['difference_count']} difference(s)")
        for item in arguments["differences"]:
            print(f"    {item['tag']}[{item['occurrence']}].{item['field']}: "
                  f"{_display(item['original'])}|{_display(item['reconstructed'])}")
    if result["terminal_failures"]:
        print("  terminal: " + ", ".join(result["terminal_failures"]))
    print(f"RESULT: {result['verdict']}")


def _load_tag_map(path: str | None) -> dict[str, str] | None:
    if not path:
        return None
    try:
        value = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise TraceInputError(f"invalid tag map {path}: {exc}") from exc
    if not isinstance(value, dict) or not all(isinstance(k, str) and isinstance(v, str)
                                              for k, v in value.items()):
        raise TraceInputError("tag map must be a JSON object of string aliases")
    return value


def _options(ns: argparse.Namespace) -> Options:
    return Options(
        ignore_fields=tuple(ns.ignore_field),
        include_tags=frozenset(ns.include_tag),
        ignore_tags=frozenset(ns.ignore_tag),
        keep_control=ns.keep_control,
        raw_pointers=ns.raw_pointers,
        tag_map=_load_tag_map(ns.tag_map),
    )


def _add_normalization_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--ignore-field", action="append", default=[], metavar="GLOB",
                        help="ignore a leaf name or TAG.field glob (repeatable)")
    parser.add_argument("--include-tag", action="append", default=[], metavar="TAG")
    parser.add_argument("--ignore-tag", action="append", default=[], metavar="TAG")
    parser.add_argument("--keep-control", action="store_true",
                        help="include ready/sites/drive/loaded/HITS harness events")
    parser.add_argument("--raw-pointers", action="store_true",
                        help="compare process-local pointer values instead of null/present")
    parser.add_argument("--tag-map", metavar="JSON",
                        help="JSON object mapping source tag names to canonical names")


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = result.add_subparsers(dest="command", required=True)
    compare_parser = sub.add_parser("compare", help="compare two JSONL traces")
    compare_parser.add_argument("original", type=Path)
    compare_parser.add_argument("reconstructed", type=Path)
    compare_parser.add_argument("--ignore-order", action="store_true")
    compare_parser.add_argument("--count-tolerance", type=int, default=0)
    compare_parser.add_argument("--float-tolerance", type=float, default=0.0)
    compare_parser.add_argument("--max-diffs", type=int, default=8)
    compare_parser.add_argument("--json", action="store_true", dest="json_output")
    _add_normalization_arguments(compare_parser)
    normalize_parser = sub.add_parser("normalize", help="write one canonical JSONL trace")
    normalize_parser.add_argument("trace", type=Path)
    normalize_parser.add_argument("-o", "--out", type=Path,
                                  help="output file; stdout when omitted")
    _add_normalization_arguments(normalize_parser)
    return result


def main(argv: list[str] | None = None) -> int:
    ns = parser().parse_args(argv)
    if getattr(ns, "count_tolerance", 0) < 0 or getattr(ns, "float_tolerance", 0.0) < 0:
        print("TRACE-DIFF INVALID: tolerances must be non-negative", file=sys.stderr)
        return 2
    if getattr(ns, "max_diffs", 1) < 0:
        print("TRACE-DIFF INVALID: --max-diffs must be non-negative", file=sys.stderr)
        return 2
    try:
        options = _options(ns)
        if ns.command == "normalize":
            events = load_trace(ns.trace, options)
            text = "".join(json.dumps(event.json_object(), ensure_ascii=False,
                                      separators=(",", ":")) + "\n" for event in events)
            if ns.out:
                ns.out.parent.mkdir(parents=True, exist_ok=True)
                ns.out.write_text(text, encoding="utf-8")
            else:
                print(text, end="")
            return 0
        original = load_trace(ns.original, options)
        reconstructed = load_trace(ns.reconstructed, options)
        result = compare(original, reconstructed, ignore_order=ns.ignore_order,
                         count_tolerance=ns.count_tolerance,
                         float_tolerance=ns.float_tolerance, max_diffs=ns.max_diffs)
    except (OSError, TraceInputError) as exc:
        print(f"TRACE-DIFF INVALID: {exc}", file=sys.stderr)
        return 2
    if ns.json_output:
        print(json.dumps(result, ensure_ascii=False, separators=(",", ":")))
    else:
        print_summary(result)
    return 0 if result["verdict"] == "MATCH" else 1


if __name__ == "__main__":
    raise SystemExit(main())
