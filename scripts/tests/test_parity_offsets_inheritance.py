"""Offsets inherited through multiple engine base classes must retain their names."""
import sys
from collections import Counter
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import parity_offsets


class InheritedOffsetsTest(unittest.TestCase):
    def test_sibling_control_base_cannot_replace_engine_member(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "Screen.h").write_text('''
class Screen;
// class Screen : public Wrong { };
class Screen : public Engine {
    /* 012C */ AIType m_typeAI;
    struct Nested { /* 0010 */ int m_nested; };
};
class SiblingControl : public Cell { /* 0010 */ int m_control; };
''')
            (root / "Engine.h").write_text('class Engine { /* 0028 */ int m_selected; };')
            (root / "Cell.h").write_text('class Cell { /* 012C */ int m_nNormalFrame; };')
            with patch.object(parity_offsets, "SRC", root):
                self.assertEqual(parity_offsets.header_offset_map("Screen"), {
                    "m_typeAI": 0x12C, "m_selected": 0x28,
                })
                # Narrowing the header scope must still flag a real wrong receiver.
                with patch.object(parity_offsets, "function_end", return_value=None), \
                     patch.object(parity_offsets, "binary_call_pairs", return_value=(Counter({(0x12C, "Set"): 1}), "ebp")), \
                     patch.object(parity_offsets, "source_pairs", return_value=Counter({("m_otherAI", "Set"): 1})), \
                     patch.object(parity_offsets, "_HDR_CACHE", {}):
                    result = parity_offsets.analyze(0, root / "Screen.cpp", 1, 3, "Screen", 10)
                    self.assertEqual(result[0], [("m_typeAI", "Set")])

    def test_transitive_base_without_unrelated_include(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "Sprite.h").write_text('class Sprite : public AI { /* 0120 */ int m_local; };')
            (root / "AI.h").write_text('#include "Other.h"\nclass AI : public Object { /* 0070 */ int m_ai; };')
            (root / "Object.h").write_text('class Object { /* 0020 */ Type m_typeAI; };')
            (root / "Other.h").write_text('class Other { /* 0020 */ int m_unrelated; };')
            with patch.object(parity_offsets, "SRC", root):
                self.assertEqual(parity_offsets.header_offset_map("Sprite"), {
                    "m_local": 0x120, "m_ai": 0x70, "m_typeAI": 0x20,
                })


if __name__ == "__main__":
    unittest.main()
