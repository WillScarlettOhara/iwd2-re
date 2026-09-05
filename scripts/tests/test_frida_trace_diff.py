"""Tests for the generic original/reconstruction JSONL trace comparator."""
from __future__ import annotations

import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))

import frida_trace_diff as trace_diff


class TraceDiffTest(unittest.TestCase):
    def test_failure_survives_filters_and_aliases(self):
        options = trace_diff.Options(include_tags=frozenset({'CALL'}),
                                     ignore_tags=frozenset({'crash'}),
                                     tag_map={'crash': 'ready'})
        event = trace_diff._event_from_row({'tag': 'crash'}, Path('test'), 1, options)
        self.assertIsNotNone(event)
        self.assertEqual(event.tag, 'crash')
        self.assertEqual(trace_diff.compare([event], [event])['verdict'], 'DIFFERENT')

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def write(self, name, rows):
        path = self.root / name
        path.write_text("".join(json.dumps(row) + "\n" for row in rows), encoding="utf-8")
        return path

    def run_main(self, *args):
        stdout, stderr = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            rc = trace_diff.main(list(args))
        return rc, stdout.getvalue(), stderr.getvalue()

    def test_normalization_unwraps_messages_and_removes_runtime_noise(self):
        path = self.write("raw.jsonl", [
            {"type": "send", "payload": {"tag": "ready", "hooks": 1}},
            {"type": "send", "payload": {
                "tag": "CALL", "this": "0x12345678", "a0_s32": 7,
                "target_ptr": "0x0", "timestamp": 123, "bt": ["0x401000"],
            }},
        ])
        events = trace_diff.load_trace(path, trace_diff.Options())
        self.assertEqual([event.json_object() for event in events], [{
            "tag": "CALL", "a0_s32": 7, "target_ptr": "<null>", "this": "<ptr>",
        }])

    def test_process_local_pointers_do_not_create_false_difference(self):
        original = self.write("orig.jsonl", [
            {"tag": "CALL", "this": "0x401000", "target_ptr": None, "a0": 7},
        ])
        ours = self.write("ours.jsonl", [
            {"tag": "CALL", "this": "0x991000", "target_ptr": "0x00000000", "a0": 7},
        ])
        rc, output, _ = self.run_main("compare", str(original), str(ours))
        self.assertEqual(rc, 0)
        self.assertIn("RESULT: MATCH", output)
        rc, _, _ = self.run_main("compare", str(original), str(ours), "--raw-pointers")
        self.assertEqual(rc, 1)

    def test_reports_frequency_order_and_argument_differences_separately(self):
        original = self.write("orig.jsonl", [
            {"tag": "A", "value": 1}, {"tag": "B", "value": 2},
            {"tag": "A", "value": 3},
        ])
        ours = self.write("ours.jsonl", [
            {"tag": "B", "value": 9}, {"tag": "A", "value": 1},
        ])
        rc, output, _ = self.run_main("compare", str(original), str(ours))
        self.assertEqual(rc, 1)
        self.assertIn("frequency: A 2|1", output)
        self.assertIn("order: first difference #1 A|B", output)
        self.assertIn("B[1].value: 2|9", output)

    def test_filters_aliases_float_tolerance_and_ignored_order(self):
        original = self.write("orig.jsonl", [
            {"tag": "OLD", "value": 1.0, "tick": 20}, {"tag": "B", "value": 2},
        ])
        ours = self.write("ours.jsonl", [
            {"tag": "B", "value": 2}, {"tag": "A", "value": 1.005, "tick": 90},
        ])
        aliases = self.root / "aliases.json"
        aliases.write_text('{"OLD":"A"}', encoding="utf-8")
        rc, output, _ = self.run_main(
            "compare", str(original), str(ours), "--tag-map", str(aliases),
            "--ignore-field", "tick", "--float-tolerance", "0.01", "--ignore-order")
        self.assertEqual(rc, 0)
        self.assertIn("order: ignored", output)

    def test_terminal_failure_never_becomes_match(self):
        original = self.write("orig.jsonl", [{"tag": "crash", "type": "access-violation"}])
        ours = self.write("ours.jsonl", [{"tag": "crash", "type": "access-violation"}])
        rc, output, _ = self.run_main("compare", str(original), str(ours))
        self.assertEqual(rc, 1)
        self.assertIn("terminal: crash", output)

    def test_invalid_or_empty_input_is_unproven(self):
        invalid = self.root / "invalid.jsonl"
        invalid.write_text("not json\n", encoding="utf-8")
        good = self.write("good.jsonl", [{"tag": "A"}])
        rc, _, error = self.run_main("compare", str(invalid), str(good))
        self.assertEqual(rc, 2)
        self.assertIn("TRACE-DIFF INVALID", error)
        controls = self.write("controls.jsonl", [{"tag": "ready"}])
        rc, _, error = self.run_main("compare", str(controls), str(good))
        self.assertEqual(rc, 2)
        self.assertIn("no comparable events", error)

    def test_normalize_writes_canonical_jsonl(self):
        source = self.write("source.jsonl", [{"event": "CALL", "z": 2, "a": 1}])
        output = self.root / "nested" / "normalized.jsonl"
        rc, _, error = self.run_main("normalize", str(source), "-o", str(output))
        self.assertEqual((rc, error), (0, ""))
        self.assertEqual(output.read_text(encoding="utf-8"),
                         '{"tag":"CALL","a":1,"z":2}\n')


if __name__ == "__main__":
    unittest.main()
