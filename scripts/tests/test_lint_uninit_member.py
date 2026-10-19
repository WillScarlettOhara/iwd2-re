"""Incremental analysis must retain findings and invalidate source/header edits."""
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lint_uninit_member as lint


@unittest.skipUnless(lint.cppcheck_executable(), "cppcheck not installed")
class IncrementalConstructorTests(unittest.TestCase):
    def test_findings_survive_reuse_and_follow_source_and_header_edits(self):
        with tempfile.TemporaryDirectory(prefix="iwd2-uninit-") as directory:
            root = Path(directory).resolve()
            source_dir = root / "src"
            source_dir.mkdir()
            header = source_dir / "Widget.h"
            source = source_dir / "Widget.cpp"
            header.write_text("class Widget { public: Widget(); int value; };\n")
            source.write_text('#include "Widget.h"\nWidget::Widget() {}\n')
            with patch.object(lint, "ROOT", root), patch.object(lint, "SRC", source_dir):
                first = lint._cppcheck([source])
                self.assertTrue(any("value" in message for _, _, message in first))
                self.assertTrue(any((root / ".cache" / "cppcheck-uninit").iterdir()))
                self.assertEqual(first, lint._cppcheck([source]), "reuse must not hide an existing warning")

                source.write_text('#include "Widget.h"\nWidget::Widget() : value(1) {}\n')
                self.assertEqual([], lint._cppcheck([source]), "source edit must invalidate analysis")

                header.write_text("class Widget { public: Widget(); int value; int other; };\n")
                updated = lint._cppcheck([source])
                self.assertTrue(any("other" in message for _, _, message in updated),
                                "a header-only edit must invalidate the cached clean result")


if __name__ == "__main__":
    unittest.main()
