"""Fixture cleanup must never remove an unrelated or edited game resource."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / "combat_payload_fixture.py"


class CombatPayloadFixtureTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.game = Path(self.temp.name)
        self.override = self.game / "override"

    def run_fixture(self, operation):
        return subprocess.run([sys.executable, str(SCRIPT), operation,
                               "--game-dir", str(self.game)], capture_output=True)

    def test_round_trip_preserves_other_resources(self):
        self.override.mkdir()
        other = self.override / "OTHER.ITM"
        other.write_bytes(b"unrelated")
        self.assertEqual(self.run_fixture("install").returncode, 0)
        self.assertEqual(len(list(self.override.glob("ZRC*.ITM"))), 13)
        self.assertEqual(len(list(self.override.glob("ZZ*.ITM"))), 3)
        self.assertEqual(self.run_fixture("remove").returncode, 0)
        self.assertEqual(other.read_bytes(), b"unrelated")
        self.assertEqual(list(self.override.iterdir()), [other])

    def test_existing_resource_aborts_before_any_install(self):
        self.override.mkdir()
        existing = self.override / "ZRCBOLT.ITM"
        existing.write_bytes(b"user resource")
        self.assertNotEqual(self.run_fixture("install").returncode, 0)
        self.assertEqual(list(self.override.iterdir()), [existing])
        self.assertEqual(existing.read_bytes(), b"user resource")

    def test_edited_fixture_aborts_before_any_removal(self):
        self.assertEqual(self.run_fixture("install").returncode, 0)
        edited = self.override / "ZRCBOLT.ITM"
        edited.write_bytes(b"edited")
        self.assertNotEqual(self.run_fixture("remove").returncode, 0)
        self.assertEqual(len(list(self.override.iterdir())), 17)
        self.assertEqual(edited.read_bytes(), b"edited")


if __name__ == "__main__":
    unittest.main()
