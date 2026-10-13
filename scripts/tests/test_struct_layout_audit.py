"""Layout evidence must come from the selected compiler, never a failed compile."""
from __future__ import annotations

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import struct_layout_audit as audit


DUMP = """
  class CExample size(16):
        +---
   0    | +--- (base class CBase)
   0    | | inherited
        | +---
   8    | first
  12    | second
        +---
  class COther size(4):
   0    | unrelated
"""


class StructLayoutAuditTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def test_dump_uses_only_direct_members_of_exact_class(self):
        self.assertEqual(audit.parse_dump(DUMP, "CExample"), {"first": 8, "second": 12})

    def test_missing_layout_is_a_harness_error(self):
        with self.assertRaisesRegex(audit.AuditError, "no '/d1reportSingleClassLayoutCMissing'"):
            audit.parse_dump(DUMP, "CMissing")

    def test_one_member_leaf_checks_annotated_base_boundary(self):
        header = self.root / "leaf.h"
        header.write_text("class CBase {\n/* 0000 */ WORD tail;\n};\n"
                          "class CLeaf /*#guess*/ : public CBase {\n/* 0002 */ LONG only;\n};\n")
        dump = "class CLeaf size(6):\n 0 | | WORD tail\n 2 | LONG only\n"
        common = [("only", 2, "LONG")]
        self.assertEqual(audit.single_member_anchor(header, "CLeaf", dump, common),
                         [("tail", 0, "WORD"), ("only", 2, "LONG")])
        drifted = dump.replace("2 | LONG", "4 | LONG")
        offsets = audit.parse_dump(drifted, "CLeaf", include_inherited=True)
        self.assertEqual(offsets["only"] - offsets["tail"], 4)
        gap_drift = (offsets["only"] - offsets["tail"]) - (common[0][1] - 0)
        self.assertEqual(audit.classify("WORD", common[0][1], gap_drift), "actionable")

    def test_one_member_leaf_missing_base_evidence_stays_unproven(self):
        header = self.root / "leaf.h"
        header.write_text("class CLeaf : public CMissing {\n/* 0004 */ LONG only;\n};\n")
        common = [("only", 4, "LONG")]
        self.assertEqual(audit.single_member_anchor(header, "CLeaf", DUMP, common), common)

    def test_x64_build_cannot_supply_x86_layout_evidence(self):
        (self.root / "iwd2-re.vcxproj").touch()
        (self.root / "CMakeCache.txt").write_text(
            "CMAKE_GENERATOR:INTERNAL=Visual Studio 16 2019\n"
            "CMAKE_GENERATOR_PLATFORM:INTERNAL=x64\n", encoding="utf-8")
        with self.assertRaisesRegex(audit.AuditError, "Win32"):
            audit.native_msbuild(self.root)

    def test_failed_native_compile_rejects_even_a_partial_layout(self):
        source = self.root / "source.cpp"
        source.touch()
        failed = subprocess.CompletedProcess([], 1, DUMP, "error C1083: missing include\n")
        with patch.object(audit, "HOST_REPO", self.root), \
                patch.object(audit, "native_msbuild", return_value=self.root / "MSBuild.exe"), \
                patch.object(audit.subprocess, "run", return_value=failed):
            with self.assertRaisesRegex(audit.AuditError, "native MSVC compile failed"):
                audit.run_native_layout_dump("CExample", str(source), self.root)
        log = self.root / ".arc" / "layout" / "CExample.log"
        self.assertIn("error C1083", log.read_text(encoding="utf-8"))
        self.assertEqual(list(log.parent.glob("compile-*")), [])

    def test_linux_keeps_remote_route_and_rejects_remote_failure(self):
        failed = subprocess.CompletedProcess([], 255, "", "ssh: no connection")
        with patch.object(audit, "WINDOWS", False), \
                patch.object(audit.subprocess, "run", return_value=failed) as run:
            with self.assertRaisesRegex(audit.AuditError, "VM layout compile failed"):
                audit.run_layout_dump("CExample", "src/CExample.cpp")
        self.assertEqual(run.call_args.args[0][:2], ["ssh", "win11vm"])


if __name__ == "__main__":
    unittest.main()
