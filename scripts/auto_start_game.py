#!/usr/bin/env python3
"""Launch iwd2-re and auto-load a save slot or premade new-game party."""
from __future__ import annotations

import argparse
import configparser
import os
import subprocess
import tempfile
import time
import uuid
from pathlib import Path

from runtime_paths import build_directory, game_directory, stage_runtime_companions

REPO = Path(__file__).resolve().parents[1]
GAME_DIR = game_directory()
EXE = build_directory() / "Debug" / "iwd2-re.exe"
PARTY_INI = GAME_DIR / "Party.ini"


def read_result(path: Path) -> dict[str, str]:
    values = {}
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        key, sep, value = line.partition("=")
        if sep:
            values[key] = value
    return values


def resolve_party(value: str, party_ini: Path = PARTY_INI) -> int:
    try:
        return int(value)
    except ValueError:
        pass

    parser = configparser.RawConfigParser()
    parser.read(party_ini)

    wanted = value.strip().lower()
    for section in parser.sections():
        if not section.lower().startswith("party "):
            continue
        name = parser.get(section, "Name", fallback="").strip().lower()
        if name == wanted:
            return int(section.split()[1])

    raise SystemExit(f"party not found in {party_ini}: {value!r}")


def add_launch_arguments(ap: argparse.ArgumentParser) -> None:
    ap.add_argument("--no-launch", action="store_true", help="attach to an already launched process is not implemented")
    ap.add_argument("--slot", type=int, default=0, help="visible load-screen slot to load, 0-4")
    ap.add_argument("--save-name", help="exact MPSave directory name to load")
    ap.add_argument("--new-game", action="store_true", help="start a new game instead of loading a save")
    ap.add_argument("--party", default="Lady's Lament", help="party index or Party.ini name for --new-game")
    ap.add_argument("--timeout", type=float, default=120.0, help="seconds to wait for world activation")
    ap.add_argument("--no-wait", action="store_true", help="launch and return without waiting for the result")
    ap.add_argument("--exe", default=str(EXE), help="iwd2-re.exe to launch (default: build/Debug; lets the scratch build+smoke gate point at an isolated exe)")
    ap.add_argument("--game-dir", type=Path, default=GAME_DIR, help="game assets/CWD (or IWD2_GAME_DIR)")
    ap.add_argument("--kill-after", action="store_true", help="terminate the launched process before returning (build+smoke gate use; interactive launches omit it to keep the window open)")
    ap.add_argument("--ui-script", help="AutoUI script to replay once the game is up (see src/AutoUI.h)")
    ap.add_argument("--ui-result", help="where AutoUI writes its JSONL (default: alongside the script)")
    ap.add_argument("--ui-go", help="gate file a `waitgo` step blocks on, so the scenario cannot act before the observer is attached")


def launch_environment(args: argparse.Namespace, result_path: Path) -> tuple[dict, str]:
    """Prepare the same AutoLoad/AutoUI environment for normal and guarded runs."""
    env = os.environ.copy()
    for key in list(env):
        if key.startswith(("IWD2_RE_AUTO_", "IWD2_RE_UI_")):
            del env[key]
    env["IWD2_RE_AUTO_RESULT"] = str(result_path)
    if args.new_game:
        env["IWD2_RE_AUTO_ACTION"] = "newgame"
        env["IWD2_RE_AUTO_PARTY"] = str(resolve_party(args.party, args.game_dir / "Party.ini"))
        description = f"new game with party {args.party!r}"
    else:
        env["IWD2_RE_AUTO_ACTION"] = "load"
        if args.save_name:
            env["IWD2_RE_AUTO_SAVE_NAME"] = args.save_name
            description = f"save {args.save_name!r}"
        else:
            env["IWD2_RE_AUTO_SLOT"] = str(args.slot)
            description = f"visible save slot {args.slot}"

    # AutoUI is inert unless IWD2_RE_UI_SCRIPT is set, so this is the only place
    # that arms it. Absolute paths: the game's cwd is the install dir, not the repo.
    if args.ui_script:
        ui_script = Path(args.ui_script).resolve()
        ui_result = Path(args.ui_result).resolve() if args.ui_result \
            else ui_script.with_suffix(".jsonl")
        if ui_result.exists():
            ui_result.unlink()
        env["IWD2_RE_UI_SCRIPT"] = str(ui_script)
        env["IWD2_RE_UI_RESULT"] = str(ui_result)
        if args.ui_go:
            env["IWD2_RE_UI_GO"] = str(Path(args.ui_go).resolve())
        description += f"; ui script {ui_script.name}"

    return env, description


def main() -> int:
    ap = argparse.ArgumentParser()
    add_launch_arguments(ap)
    args = ap.parse_args()
    if args.no_launch:
        raise SystemExit("--no-launch not implemented yet")
    result_path = Path(tempfile.gettempdir()) / f"iwd2-re-auto-{uuid.uuid4().hex}.txt"
    env, description = launch_environment(args, result_path)

    try:
        stage_runtime_companions(Path(args.exe), args.game_dir)
    except (OSError, RuntimeError) as exc:
        print(f"LAUNCH ERROR: {exc}")
        return 2
    proc = subprocess.Popen([str(Path(args.exe).resolve())], cwd=str(args.game_dir), env=env)
    print(f"pid={proc.pid}; auto {description}")
    if args.ui_script:
        print(f"ui_result={env['IWD2_RE_UI_RESULT']}")

    if args.no_wait:
        print(f"result={result_path}")
        return 0

    try:
        deadline = time.time() + args.timeout
        while time.time() < deadline:
            if result_path.exists():
                result = read_result(result_path)
                status = result.get("status", "unknown")
                detail = result.get("detail", "")
                print(f"{status}: {detail}")
                return 0 if status == "loaded" else 1
            if proc.poll() is not None:
                break
            time.sleep(0.25)

        if result_path.exists():
            result = read_result(result_path)
            print(f"{result.get('status', 'unknown')}: {result.get('detail', '')}")
            return 0 if result.get("status") == "loaded" else 1

        if proc.poll() is None:
            print(f"timeout: no world activation after {args.timeout:.1f}s")
        else:
            print(f"failed: process exited with code {proc.returncode} before world activation")

        return 1
    finally:
        # The build+smoke gate runs candidates back-to-back in one scratch worktree; a
        # leaked game keeps build/Debug/iwd2-re.exe open and fails the NEXT link. With
        # --kill-after we terminate the process we spawned -- by handle, so only this
        # exe, never a canonical game the user is playing -- on every exit path, which
        # also clears a load-time RTC/assert modal that would otherwise block forever.
        if args.kill_after and proc.poll() is None:
            try:
                proc.kill()
                proc.wait(timeout=10)
                print(f"killed pid={proc.pid} (--kill-after)")
            except Exception as exc:
                print(f"kill failed: {exc}")


if __name__ == "__main__":
    raise SystemExit(main())
