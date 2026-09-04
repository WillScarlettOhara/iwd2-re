from __future__ import annotations

import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from bg2_find import dump_lines, parse_globals, parse_modules, parse_types


TYPES = """
    0x1000 | LF_ARGLIST [size = 8]
    0x1001 | LF_MFUNCTION [size = 28]
             return type = 0x0003 (void), # args = 0, param list = 0x1000
             calling conv = thiscall, options = None
    0x1002 | LF_FIELDLIST [size = 40]
             - LF_MEMBER [name = `m_count`, Type = 0x0074 (int), offset = 4, attrs = public]
             - LF_ONEMETHOD [name = `ApplyTriggers`]
               type = 0x1001, vftable offset = 172, attrs = public intro virtual
    0x1003 | LF_CLASS [size = 48] `CGameAIBase`
             field list: 0x1002
             options: None, sizeof 8
"""
GLOBALS = """
  123 | S_PROCREF [size = 24] `ApplyTriggers`
           module = 12
  456 | S_GDATA32 [size = 32] `g_count`
           type = 0x0074 (int), addr = 0003:16
"""
SYMBOLS = "Mod 0012 | `C:\\build\\CGameAIBase.obj`:\n"


class Bg2DumpFallbackTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.pdb = self.root / "Baldur.pdb"

    def tearDown(self):
        self.temp.cleanup()

    def test_bundled_utf16_archive_preserves_names_layout_and_module(self):
        with zipfile.ZipFile(self.root / "bg2_pdb_extracted.zip", "w") as archive:
            for stream, text in (("types", TYPES), ("globals", GLOBALS),
                                 ("symbols", SYMBOLS)):
                archive.writestr(f"bg2_pdb_{stream}.txt", text.encode("utf-16"))

        classes, _, members, methods, _ = parse_types(self.pdb)
        self.assertEqual(classes, [("CGameAIBase", "class", 8, 1, 1)])
        self.assertEqual(members, [("CGameAIBase", "m_count", "int", 4, 0)])
        self.assertEqual(methods, [("CGameAIBase", "ApplyTriggers", "()",
                                    "public intro virtual", 172)])
        modules = parse_modules(self.pdb)
        self.assertEqual(modules, {12: "CGameAIBase"})
        self.assertEqual(parse_globals(self.pdb, modules),
                         ([("ApplyTriggers", "CGameAIBase")],
                          [("g_count", "int", "0003:16", "data")]))

    def test_extracted_utf8_dump_is_supported(self):
        extracted = self.root / "extracted"
        extracted.mkdir()
        (extracted / "bg2_pdb_types.txt").write_text(TYPES, encoding="utf-8-sig")
        self.assertEqual("".join(dump_lines(self.pdb, "types")), TYPES)

    def test_missing_sources_reports_recoverable_inputs(self):
        with self.assertRaisesRegex(FileNotFoundError, "bg2_pdb_extracted.zip"):
            list(dump_lines(self.pdb, "types"))


if __name__ == "__main__":
    unittest.main()
