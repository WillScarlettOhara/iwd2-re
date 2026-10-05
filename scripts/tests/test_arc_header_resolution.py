"""Layout audits must resolve definitions rather than forward declarations."""
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / "arc.py"
import sys
sys.path.insert(0, str(SCRIPT.parent))
spec = importlib.util.spec_from_file_location("arc_header_resolution", SCRIPT)
arc = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = arc
spec.loader.exec_module(arc)


class HeaderResolutionTests(unittest.TestCase):
    def test_visibility_skips_sprite_forward_declaration(self):
        with patch.object(arc, "git", return_value="src/CGameSprite.h\nsrc/CVisibility.h\n"):
            self.assertEqual(arc.header_for_class("CVisibilityMap"), arc.REPO / "src/CVisibility.h")

    def test_direct_sprite_header(self):
        self.assertEqual(arc.header_for_class("CGameSprite"), arc.REPO / "src/CGameSprite.h")


if __name__ == "__main__":
    unittest.main()
