"""Member-offset audit must resolve classes for destructor targets too."""
import sys
import unittest
from collections import Counter
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import parity_offsets


class SourceResolutionTest(unittest.TestCase):
    def test_class_for_destructor_constructor_and_method(self):
        for method in ("~CProjectile", "CProjectile", "Fire"):
            with self.subTest(method=method):
                output = f"CProjectile::{method} src/CProjectile.cpp:119 0x51E9B0 [119-126]\n"
                with patch.object(parity_offsets, "run", return_value=output):
                    self.assertEqual(parity_offsets.resolve_source(0x51E9B0), (
                        parity_offsets.REPO / "src/CProjectile.cpp", 119, 126, "CProjectile"))


class ReceiverOriginTest(unittest.TestCase):
    def test_game_pointer_in_esi_is_not_the_unused_screen_receiver(self):
        asm=["0x0060be50: mov esi, dword ptr [eax + 0x1c54]",
             "0x0060be5b: lea ecx, [edi + 0x920]",
             "0x0060be61: call 0x447ad0 ; CDerivedStats::GetBestClass",
             "0x0060bee6: lea ecx, [esi + 0x1a8]",
             "0x0060bef0: call 0x403340 ; C2DArray::GetAt"]
        with patch.object(parity_offsets, "disasm", return_value=asm):
            self.assertEqual(parity_offsets.binary_call_pairs(0x60BE30,100,None),(Counter(),"unknown"))

    def test_explicit_receiver_still_records_member_calls(self):
        asm=["0x00400000: mov esi, ecx", "0x00400002: lea ecx, [esi + 0x120]",
             "0x00400008: call 0x500000 ; Type::Set"]
        with patch.object(parity_offsets, "disasm", return_value=asm):
            self.assertEqual(parity_offsets.binary_call_pairs(0x400000,20,None),(Counter({(0x120,"Set"):1}),"esi"))

    def test_copy_after_replacing_ecx_cannot_establish_receiver(self):
        asm=["0x00400000: mov ecx, dword ptr [eax + 0x20]", "0x00400004: mov esi, ecx",
             "0x00400006: lea ecx, [esi + 0x120]", "0x0040000c: call 0x500000 ; Type::Set"]
        with patch.object(parity_offsets, "disasm", return_value=asm):
            self.assertEqual(parity_offsets.binary_call_pairs(0x400000,20,None),(Counter(),"unknown"))

    def test_ghidra_signal_uses_the_same_receiver_evidence(self):
        from re_agent.parity.signals import _ghidra_call_pairs
        unrelated="MOV ESI,dword ptr [EAX + 0x1c54]\nLEA ECX,[EDI + 0x920]\nCALL Stats::Best\nLEA ECX,[ESI + 0x1a8]\nCALL Table::GetAt"
        self.assertEqual(_ghidra_call_pairs(unrelated,{}),Counter())
        self.assertEqual(_ghidra_call_pairs("MOV ESI,ECX\nLEA ECX,[ESI + 0x120]\nCALL Type::Set",{}),Counter({(0x120,"Set"):1}))


if __name__ == "__main__":
    unittest.main()
