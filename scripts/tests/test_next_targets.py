from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import next_targets


class TargetClassificationTest(unittest.TestCase):
    def test_reviewed_library_helpers_override_game_caller_consensus(self):
        callers = [0x410000, 0x410100]
        for target, (name, _role) in next_targets.REVIEWED_MFC_LISTS.items():
            with self.subTest(target=hex(target)):
                rows = next_targets.score_all(
                    {target: {"name": name, "num_callers": 2}}, set(callers),
                    {target: callers}, {}, {}, {target: 64},
                    {c: {"class": "CGameSprite"} for c in callers},
                    {"CGameSprite"}, facts_loader=lambda _addr: {})
                self.assertEqual(rows[0]["bucket"], "excluded")
                self.assertEqual(rows[0]["ownership"], "mfc")
                self.assertEqual(rows[0]["name"], name)
                self.assertEqual(rows[0]["recovered_callers"], 2)
                self.assertEqual(rows[0]["classification_reasons"][0]["code"],
                                 "reviewed-mfc-list")

    def test_library_review_is_address_and_export_name_specific(self):
        self.assertIsNone(next_targets.reviewed_exclusion(0x500000, "Find"))
        self.assertIsNone(next_targets.reviewed_exclusion(
            0x727DC0, "CGameSprite::Find"))
        self.assertIsNone(next_targets.reviewed_exclusion(
            0x727720, "FUN_00727720"))

    def test_named_runtime_families_have_stable_reason_codes(self):
        cases = {
            "AfxGetThread": ("mfc", "mfc-symbol"),
            "CRect": ("mfc", "mfc-symbol"),
            "std::vector": ("stl", "stl-symbol"),
            "memcpy": ("crt", "crt-symbol"),
            "Unwind@00801234": ("compiler", "compiler-funclet"),
        }
        for name, expected in cases.items():
            with self.subTest(name=name):
                self.assertEqual(next_targets.named_exclusion(name), expected)
        self.assertIsNone(next_targets.named_exclusion("CGameSprite::AIUpdate"))
        self.assertIsNone(next_targets.named_exclusion("Find"))

    def test_support_shape_is_reviewable_and_conservative(self):
        reason = next_targets.support_shape_reason(
            80, 12, ["CRes::Request", "CDimm::GetResObject"])
        self.assertIn("80 call sites", reason)
        self.assertIsNone(next_targets.support_shape_reason(
            8, 12, ["CRes::Request"]))
        self.assertIsNone(next_targets.support_shape_reason(
            80, 12, ["CGameSprite::AIUpdate"]))
        seh = "ExceptionList = &local_c;\npuStack_8 = &LAB_00801234;"
        self.assertIn("compiler SEH cleanup shape",
                      next_targets.support_shape_reason(
                          80, 12, ["CSound::~CSound", "FUN_007fcc1a"], seh))

    def test_probable_class_uses_unique_recovered_functions(self):
        callers = [0x410000, 0x410000, 0x410100, 0x410200]
        recovered = {0x410000, 0x410100, 0x410200}
        symbols = {
            0x410000: {"class": "CGameSprite"},
            0x410100: {"class": "CGameSprite"},
            0x410200: {"class": "CGameArea"},
        }
        self.assertEqual(
            next_targets.probable_caller_class(callers, recovered, symbols),
            ("CGameSprite", 2, 3),
        )

    def test_score_keeps_placeholder_name_and_audits_duplicate_sites(self):
        target = 0x500000
        callers = [0x410000, 0x410000, 0x410100]
        rows = next_targets.score_all(
            {target: {"name": "FUN_00500000", "num_callers": 3}},
            {0x410000, 0x410100},
            {target: callers},
            {}, {}, {target: 64},
            {
                0x410000: {"class": "CGameSprite"},
                0x410100: {"class": "CGameSprite"},
            },
            {"CGameSprite"},
            facts_loader=lambda _addr: {},
        )
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual(row["name"], "FUN_00500000")
        self.assertEqual(row["bucket"], "identify")
        self.assertEqual(row["probable_class"], "CGameSprite")
        self.assertEqual(row["num_callers"], 2)
        self.assertEqual(row["call_sites"], 3)
        self.assertEqual(row["recovered_callers"], 2)
        self.assertEqual(row["recovered_call_sites"], 3)
        self.assertEqual(row["classification_reasons"][0]["code"],
                         "caller-class-consensus")

    def test_vtable_is_direct_game_ownership_evidence(self):
        target = 0x500100
        rows = next_targets.score_all(
            {target: {"name": "FUN_00500100", "num_callers": 0}},
            set(), {target: []},
            {target: "CGameSprite"}, {"CGameSprite": 0.8}, {target: 64},
            facts_loader=lambda _addr: {},
        )
        self.assertEqual(rows[0]["bucket"], "identify")
        self.assertTrue(rows[0]["vtable_hole"])
        self.assertEqual(rows[0]["classification_reasons"][0]["code"],
                         "vtable-class")

    def test_vtable_evidence_overrides_support_shape(self):
        ownership, bucket, reasons = next_targets.classify(
            "FUN_00500200", placeholder=True, named_class=None,
            hole_class="CGameSprite", caller_class=None,
            support_reason="100 call sites, no callees", source_classes=set())
        self.assertEqual((ownership, bucket), ("game", "identify"))
        self.assertEqual(reasons[0]["code"], "vtable-class")


if __name__ == "__main__":
    unittest.main()
