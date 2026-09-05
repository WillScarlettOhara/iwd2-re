"""Coverage gates must not turn idle runs or missing instrumentation into PASS."""
from __future__ import annotations

import io
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))

import native
import runtime_paths
import arc
from auto_start_game import launch_environment


class NativeWorkflowTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.args = native.parser().parse_args(["smoke", "3", "60", "--hit", "CGameSprite::Render",
                                               "--game-dir", str(self.root)])
        self.observation = native.Observation(io.StringIO())

    def instrument(self, hits=1):
        self.observation.message({"type": "send", "payload": {"tag": "SYM", "name": self.args.hit, "n": 1}})
        self.observation.message({"type": "send", "payload": {"tag": "HITS", "name": self.args.hit, "n": hits}})

    def test_windows_and_linux_venv_paths(self):
        self.assertEqual(runtime_paths.venv_tool("re-agent", windows=True, repo=self.root),
                         self.root / ".venv-reagent" / "Scripts" / "re-agent.exe")
        self.assertEqual(runtime_paths.venv_tool("python", windows=False, repo=self.root),
                         self.root / ".venv-reagent" / "bin" / "python")

    def test_windows_lint_paths_preserve_findings(self):
        report = arc.parse_lint("re_lint", 1, "src\\CInfGame.cpp:120: RE001: bad call\n", "")
        self.assertEqual(len(report.findings), 1)
        self.assertEqual(report.findings[0].file, "src/CInfGame.cpp")
        self.assertEqual(report.findings[0].line, 120)
        self.assertEqual(report.findings[0].code, "RE001")

    def test_missing_capability_not_reported_as_a_clean_lint(self):
        report = arc.fold([arc.Report(tool="lint_uninit_member", status="skip", summary="cppcheck missing")],
                          "lint", "1 check")
        self.assertEqual(report.status, "skip")
        self.assertIn("1 skipped", report.summary)
        self.assertIn("cppcheck missing", report.notes[0])

    def test_missing_instrumentation_is_not_clean(self):
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "NOT-INSTRUMENTED"))

    def test_zero_hits_is_not_clean(self):
        self.instrument(hits=0)
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "NOT-EXERCISED"))

    def test_minimum_hits_enforced(self):
        self.instrument(hits=1)
        self.args.hit_min = 2
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "NOT-EXERCISED"))
        self.args.hit_min = 1
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (0, "CLEAN"))

    def test_every_comma_separated_hit_is_required(self):
        self.args.hit = "COne::Tick, CTwo::Tick,COne::Tick"
        self.assertEqual(native.hit_symbols(self.args.hit), ["COne::Tick", "CTwo::Tick"])
        for name in native.hit_symbols(self.args.hit):
            self.observation.message({"type": "send",
                                      "payload": {"tag": "SYM", "name": name, "n": 1}})
        self.observation.message({"type": "send",
                                  "payload": {"tag": "HITS", "name": "COne::Tick", "n": 4}})
        self.assertEqual(native.evidence_verdict(self.args, self.observation),
                         (2, "NOT-EXERCISED"))
        self.observation.message({"type": "send",
                                  "payload": {"tag": "HITS", "name": "CTwo::Tick", "n": 1}})
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (0, "CLEAN"))

    def test_arc_parses_multi_symbol_smoke_counts(self):
        report = arc.parse_smoke(
            0,
            "RESULT: CLEAN (COne::Tick hit x4; CTwo::Tick hit x1)\n",
            "")
        self.assertEqual(report.status, "pass")
        self.assertEqual(report.counts["symbols"], 2)
        self.assertEqual(report.counts["hits_by_symbol"],
                         {"COne::Tick": 4, "CTwo::Tick": 1})
        self.assertIn("2/2 symbols", report.summary)

    def test_process_exit_and_crash_fail_even_with_hits(self):
        self.instrument()
        self.observation.detached = True
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (1, "PROCESS-EXITED"))
        self.observation.crash = True
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (1, "CRASH"))

    def test_script_error_cannot_silently_pass(self):
        self.instrument()
        self.observation.message({"type": "error", "description": "hook failed"})
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "OBSERVER-ERROR"))

    def test_ui_and_hit_gates_both_required(self):
        result = self.root / "ui.jsonl"
        self.args.ui_script = "scenario.txt"
        self.args.ui_result = str(result)
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "UI-NO-RESULT"))
        result.write_text('{"op":"verdict","verdict":"PASS"}\n')
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "NOT-INSTRUMENTED"))
        self.instrument()
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (0, "CLEAN"))
        result.write_text('{"op":"click","status":"fail"}\n{"op":"verdict","verdict":"PASS"}\n')
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "UI-FAIL"))

    def test_incomplete_ui_cannot_pass(self):
        result = self.root / "ui.jsonl"
        result.write_text('{"op":"click","status":"ok"}\n')
        self.assertEqual(native.ui_verdict(result), "UI-INCOMPLETE")

    def test_debug_log_gate_still_checked_when_hit_gate_passes(self):
        self.instrument()
        self.args.expect = r"PROLOGUE done"
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (2, "NOT-EXERCISED"))
        (self.root / "iwd2-re-debug.log").write_text("PROLOGUE done\n", encoding="utf-16")
        self.assertEqual(native.evidence_verdict(self.args, self.observation), (0, "CLEAN"))

    def test_launch_clears_stale_auto_and_ui_environment(self):
        with patch.dict(os.environ, {"IWD2_RE_AUTO_SAVE_NAME": "old save", "IWD2_RE_UI_SCRIPT": "old script"}):
            env, description = launch_environment(self.args, self.root / "load.txt")
        self.assertNotIn("IWD2_RE_AUTO_SAVE_NAME", env)
        self.assertNotIn("IWD2_RE_UI_SCRIPT", env)
        self.assertEqual(env["IWD2_RE_AUTO_SLOT"], "3")


if __name__ == "__main__":
    unittest.main()
