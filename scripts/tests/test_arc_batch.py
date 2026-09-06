"""The economical batch gate must preserve per-target evidence and shared work."""
from __future__ import annotations

import io
import json
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))

import arc


def resolved(address: str, symbol: str, cls: str = "CTest") -> dict:
    return {"address": address, "file": "src/CTest.cpp", "line": 10,
            "symbol": symbol, "class": cls}


class FakeRunner:
    calls: list[arc.Step] = []

    def __init__(self, caps, baseline, rundir, progress=False):
        self.ran_tools: set[str] = set()

    def run(self, step: arc.Step) -> arc.Report:
        self.calls.append(step)
        report = arc.Report(tool=step.id, label=step.label, status="pass", summary="clean")
        self.ran_tools.add(report.tool)
        return report

    def run_parallel(self, steps: list[arc.Step]) -> list[arc.Report]:
        return [self.run(step) for step in steps]


class ArcBatchTest(unittest.TestCase):
    def test_unexercised_batch_is_not_success(self):
        report = arc.parse_batch_smoke(2, 'RESULT: NOT-EXERCISED', '')
        self.assertEqual(arc.verdict_of([report])[1], 2)

    def setUp(self):
        FakeRunner.calls = []
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)

    def test_addresses_are_normalized_and_deduplicated(self):
        addresses, error = arc.batch_addresses(["0x442990", "0X442990", "0x4428A0"])
        self.assertIsNone(error)
        self.assertEqual(addresses, ["0x442990", "0x4428a0"])
        self.assertIsNotNone(arc.batch_addresses(["442990"])[1])

    def test_layout_shared_header_does_not_audit_empty_leaf_as_sibling(self):
        root = Path(self.temp.name)
        header = root / "Family.h"
        header.write_text("class Base {\n/* 0x04 */ int member;\n};\n"
                          "class Leaf : public Base {\npublic:\nLeaf();\n};\n")
        with patch.object(arc, "header_for_class", return_value=header):
            step = arc.layout_step("Leaf")
        self.assertIn("no own offset-annotated members", step.skip_reason)
        self.assertEqual(step.argv, [])

    def test_layout_shared_header_passes_resolved_header_and_source(self):
        root = Path(self.temp.name)
        header = root / "Family.h"
        header.write_text("class Leaf {\n/* 0x04 */ int one;\n/* 0x08 */ int two;\n};\n")
        source = header.with_suffix(".cpp")
        source.write_text('#include "Family.h"\n')
        with patch.object(arc, "REPO", root), \
             patch.object(arc, "header_for_class", return_value=header):
            step = arc.layout_step("Leaf")
        self.assertIsNone(step.skip_reason)
        self.assertEqual(step.argv[-4:], ["--header", str(header), "--source", "Family.cpp"])

    def test_default_and_explicit_hit_lists_are_stable(self):
        infos = [resolved("0x442990", "CTest::One"),
                 resolved("0x4428a0", "CTest::Two")]
        self.assertEqual(arc.batch_hit_symbols(SimpleNamespace(hit=None), infos),
                         ["CTest::One", "CTest::Two"])
        args = SimpleNamespace(hit=["CTest::Two,CTest::One", "CTest::Two"])
        self.assertEqual(arc.batch_hit_symbols(args, infos),
                         ["CTest::Two", "CTest::One"])

    def test_one_build_and_one_multi_symbol_smoke_keep_per_target_steps(self):
        infos = {"0x442990": resolved("0x442990", "CTest::One"),
                 "0x4428a0": resolved("0x4428a0", "CTest::Two")}
        stdout = io.StringIO()
        rundir = Path(self.temp.name) / "batch"
        with patch.object(arc, "resolve_address", side_effect=lambda addr: infos[addr]), \
             patch.object(arc, "Runner", FakeRunner), \
             patch.object(arc, "new_rundir", return_value=rundir), \
             patch.object(arc, "write_run_json"), \
             patch.object(arc, "runtime_command", side_effect=lambda *args: ["runtime", *args]), \
             patch.object(arc, "ghidra_name", return_value=None), \
             patch.object(arc, "layout_step", side_effect=lambda cls: arc.Step(
                 id="layout", label="layout", argv=[], parse=arc.parse_struct_layout,
                 skip_reason="test")), \
             patch("sys.stdout", stdout):
            rc = arc.main(["batch", "0x442990", "0x4428A0",
                           "--no-parity", "--json"])

        self.assertEqual(rc, 0)
        payload = json.loads(stdout.getvalue())
        self.assertEqual(payload["cmd"], "batch")
        self.assertEqual(payload["hit_symbols"], ["CTest::One", "CTest::Two"])
        self.assertEqual([len(target["steps"]) for target in payload["targets"]], [2, 2])
        build = [step for step in FakeRunner.calls if step.id == "build"]
        smoke = [step for step in FakeRunner.calls if step.id == "smoke"]
        self.assertEqual(len(build), 1)
        self.assertEqual(len(smoke), 1)
        self.assertEqual(smoke[0].argv,
                         ["runtime", "smoke", "3", "90", "--hit",
                          "CTest::One,CTest::Two"])


if __name__ == "__main__":
    unittest.main()
