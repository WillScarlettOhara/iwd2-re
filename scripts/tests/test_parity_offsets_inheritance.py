"""Offsets inherited through multiple engine base classes must retain their names."""
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import parity_offsets


class InheritedOffsetsTest(unittest.TestCase):
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
