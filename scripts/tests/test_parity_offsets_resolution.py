"""Member-offset audit must resolve classes for destructor targets too."""
import sys
import unittest
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


if __name__ == "__main__":
    unittest.main()
