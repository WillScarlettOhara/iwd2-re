"""Stage the installed renderer without replacing a custom runtime DLL."""
from __future__ import annotations

import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import auto_start_game
import native
from runtime_paths import stage_runtime_companions


class RuntimeCompanionsTest(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.game = self.root / "Game assets"
        self.game.mkdir()
        self.exe = self.root / "build" / "Debug" / "iwd2-re.exe"
        self.environment = patch.dict(os.environ, {"IWD2_SKIP_RUNTIME_COMPANIONS": "0"})
        self.environment.start()
        self.addCleanup(self.environment.stop)

    def install(self, name="ddraw.dll", data=b"installed renderer"):
        (self.game / name).write_bytes(data)

    def prepare_exe(self):
        self.exe.parent.mkdir(parents=True, exist_ok=True)
        self.exe.write_bytes(b"test executable")

    def test_copies_installed_companion_set_only(self):
        for name in ("ddraw.dll", "libwine.dll", "wined3d.dll", "unrelated.dll"):
            self.install(name, name.encode())
        copied = stage_runtime_companions(self.exe, self.game)
        self.assertEqual({p.name for p in copied}, {"ddraw.dll", "libwine.dll", "wined3d.dll"})
        for path in copied:
            self.assertEqual(path.read_bytes(), (self.game / path.name).read_bytes())
        self.assertFalse((self.exe.parent / "unrelated.dll").exists())

    def test_identical_destination_is_not_rewritten(self):
        self.install()
        stage_runtime_companions(self.exe, self.game)
        destination = self.exe.parent / "ddraw.dll"
        os.utime(destination, ns=(1_500_000_000_000_000_000, 1_500_000_000_000_000_000))
        before = destination.stat().st_mtime_ns
        self.assertEqual(stage_runtime_companions(self.exe, self.game), [])
        self.assertEqual(destination.stat().st_mtime_ns, before)

    def test_conflict_preserves_existing_dll_and_copies_nothing(self):
        self.install()
        self.install("wined3d.dll", b"installed Wine")
        self.prepare_exe()
        destination = self.exe.parent / "wined3d.dll"
        destination.write_bytes(b"custom Wine")
        with self.assertRaisesRegex(RuntimeError, "IWD2_SKIP_RUNTIME_COMPANIONS=1"):
            stage_runtime_companions(self.exe, self.game)
        self.assertEqual(destination.read_bytes(), b"custom Wine")
        self.assertFalse((self.exe.parent / "ddraw.dll").exists())

    def test_explicit_skip_preserves_custom_runtime(self):
        self.install()
        self.prepare_exe()
        destination = self.exe.parent / "ddraw.dll"
        destination.write_bytes(b"custom renderer")
        with patch.dict(os.environ, {"IWD2_SKIP_RUNTIME_COMPANIONS": "1"}):
            self.assertEqual(stage_runtime_companions(self.exe, self.game), [])
        self.assertEqual(destination.read_bytes(), b"custom renderer")

    def test_missing_assets_or_wrapper_needs_no_staging(self):
        self.assertEqual(stage_runtime_companions(self.exe, self.root / "missing game"), [])
        self.install("libwine.dll")
        self.assertEqual(stage_runtime_companions(self.exe, self.game), [])
        self.assertFalse(self.exe.parent.exists())

    def test_executable_inside_installation_needs_no_copy(self):
        self.install()
        self.assertEqual(stage_runtime_companions(self.game / "iwd2-re.exe", self.game), [])

    def test_native_run_and_smoke_prepare_executable_directory(self):
        self.install()
        self.prepare_exe()
        for command in ("run", "smoke"):
            with self.subTest(command=command):
                destination = self.exe.parent / "ddraw.dll"
                destination.unlink(missing_ok=True)
                args = native.parser().parse_args([
                    command, "--exe", str(self.exe), "--game-dir", str(self.game)])
                native.normalize_launch(args)
                self.assertEqual(destination.read_bytes(), b"installed renderer")

    def test_direct_auto_start_stages_before_launch(self):
        self.install()
        self.prepare_exe()

        def launch(*args, **kwargs):
            self.assertEqual((self.exe.parent / "ddraw.dll").read_bytes(), b"installed renderer")
            return Mock(pid=123)

        with patch.object(sys, "argv", ["auto_start_game.py", "--no-wait", "--exe", str(self.exe),
                                        "--game-dir", str(self.game)]), \
                patch.object(auto_start_game.subprocess, "Popen", side_effect=launch):
            self.assertEqual(auto_start_game.main(), 0)

    def test_menu_stages_and_clears_automation_without_stopping_a_game(self):
        self.install()
        self.prepare_exe()
        args = native.parser().parse_args([
            "menu", "--exe", str(self.exe), "--game-dir", str(self.game)])

        def launch(command, *, cwd, env):
            self.assertEqual(command, [str(self.exe)])
            self.assertEqual(cwd, str(self.game))
            self.assertEqual((self.exe.parent / "ddraw.dll").read_bytes(), b"installed renderer")
            self.assertFalse(any(key.startswith(("IWD2_RE_AUTO_", "IWD2_RE_UI_")) for key in env))
            self.assertEqual(env["KEEP_CUSTOM_SETTING"], "yes")
            return Mock(pid=123)

        with patch.dict(os.environ, {"IWD2_RE_AUTO_ACTION": "newgame", "IWD2_RE_UI_SCRIPT": "old.txt",
                                     "KEEP_CUSTOM_SETTING": "yes"}), \
                patch.object(native.subprocess, "Popen", side_effect=launch), \
                patch.object(native, "stop_matching_exe") as stop:
            self.assertEqual(native.menu(args), 0)
            stop.assert_not_called()


if __name__ == "__main__":
    unittest.main()
