#!/usr/bin/env python3
"""Native Windows counterpart of vm.sh; no SSH, VM, or scheduled task required.

  python scripts/native.py build
  python scripts/native.py menu
  python scripts/native.py run --new-game --party 0
  python scripts/native.py smoke 3 60 --hit CGameSprite::Render
  python scripts/native.py smoke --new-game --party 0 --hold 60 --ui scripts/scenarios/newgame-reload.txt
  python scripts/native.py trace --hooks scripts/probes/hooks.json --load-slot 3 --hit NAME

Smoke attaches the existing crash oracle and PDB counters BEFORE resuming the
game, requires AutoLoad's loaded result, and composes UI, --hit and --expect gates.
Exit 0 CLEAN / 1 build or runtime failure / 2 unavailable or unproven. A bare
CLEAN without a gate proves only that loading and the observation interval ended.
"""
from __future__ import annotations

import argparse
import base64
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from pathlib import Path

from auto_start_game import add_launch_arguments, launch_environment, read_result
from runtime_paths import (REPO, build_directory, game_directory, python_executable,
                           stage_runtime_companions, utf8_stdio)

SCRIPTS = REPO / "scripts"
STATUS = REPO / "tmp_native_status.txt"
GUARD_LOG = REPO / "tmp_smoke_guard.log"


def read_text(path: Path) -> str:
    if not path.is_file():
        return ""
    raw = path.read_bytes()
    return raw.decode("utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8",
                      errors="replace")


def powershell(script: str, *, env: dict | None = None) -> subprocess.CompletedProcess:
    script = "$ProgressPreference = 'SilentlyContinue'\n" + script
    encoded = base64.b64encode(script.encode("utf-16le")).decode("ascii")
    return subprocess.run(["powershell.exe", "-NoProfile", "-NonInteractive", "-EncodedCommand", encoded],
                          env=env, capture_output=True, encoding="utf-8", errors="replace")


def stop_matching_exe(exe: Path) -> None:
    # Compare the full executable path: another checkout, the original, and
    # unrelated Python/Frida sessions belong to somebody else.
    env = os.environ.copy()
    env["IWD2_NATIVE_TARGET_EXE"] = str(exe.resolve())
    proc = powershell(r"""
$ErrorActionPreference = 'Stop'
$target = [IO.Path]::GetFullPath($env:IWD2_NATIVE_TARGET_EXE)
$name = [IO.Path]::GetFileNameWithoutExtension($target)
foreach ($candidate in @(Get-Process -Name $name -ErrorAction SilentlyContinue)) {
    if ($candidate.Path -and [IO.Path]::GetFullPath($candidate.Path) -eq $target) {
        $candidate.Kill()
        if (-not $candidate.WaitForExit(10000)) { throw 'Game did not exit' }
    }
}
exit 0
""", env=env)
    if proc.returncode:
        raise RuntimeError(proc.stderr.strip() or "cannot stop the previous game instance")


def clear_log(game_dir: Path) -> None:
    path = game_dir / "iwd2-re-debug.log"
    if path.exists():
        path.write_text("", encoding="utf-8")


def build(args: argparse.Namespace) -> int:
    cmake = os.environ.get("IWD2_CMAKE") or shutil.which("cmake")
    if not cmake:
        print("BUILD FAILED: CMake not found (set IWD2_CMAKE)")
        return 2
    build_dir = args.build_dir.resolve()
    cache = read_text(build_dir / "CMakeCache.txt")
    if cache:
        if "CMAKE_GENERATOR:INTERNAL=Visual Studio 16 2019" not in cache or \
                "CMAKE_GENERATOR_PLATFORM:INTERNAL=Win32" not in cache:
            print("BUILD FAILED: build directory must use Visual Studio 16 2019 / Win32; "
                  "choose a fresh --build-dir")
            return 2
    commands = []
    stamp = build_dir / ".native-header-state.json"
    headers = {str(p.relative_to(REPO)): hashlib.sha256(p.read_bytes()).hexdigest()
               for p in (REPO / "src").rglob("*.h")}
    try:
        previous = json.loads(read_text(stamp))
    except (ValueError, OSError):
        previous = None
    if not cache:
        commands.append([cmake, "-S", str(REPO), "-B", str(build_dir),
                         "-G", "Visual Studio 16 2019", "-A", "Win32", "-T", "v142"])
    commands.append([cmake, "--build", str(build_dir), "--config", "Debug",
                     "--parallel", str(args.jobs)])
    if previous != headers:
        commands[-1].append("--clean-first")
        print("Header state changed or unknown: rebuilding all translation units", flush=True)
    log = REPO / "tmp_native_build.log"
    rc = 0
    with log.open("w", encoding="utf-8") as output:
        for command in commands:
            output.write(subprocess.list2cmdline(command) + "\n")
            output.flush()
            rc = subprocess.run(command, cwd=REPO, stdout=output, stderr=subprocess.STDOUT).returncode
            if rc:
                break
    text = read_text(log)
    diagnostics = list(dict.fromkeys(line.strip() for line in text.splitlines()
                                    if re.search(r"(?:warning|error|avertissement|erreur)\s+[A-Z]+\d+", line, re.I)))
    print("\n".join(diagnostics[:60])) if diagnostics else None
    print(f"BUILD {'FAILED' if rc else 'OK'}  (full log: {log.name})")
    if rc:
        if not diagnostics:
            print("\n".join(text.splitlines()[-20:]))
        return 1
    stamp.write_text(json.dumps(headers, sort_keys=True), encoding="utf-8")
    stage_runtime_companions(build_dir / "Debug" / "iwd2-re.exe", game_directory())
    if args.run:
        return main(["run", "--exe", str(build_dir / "Debug" / "iwd2-re.exe")])
    return 0


def normalize_launch(args: argparse.Namespace) -> None:
    if args.no_launch:
        raise ValueError("--no-launch is not implemented")
    args.game_dir = args.game_dir.resolve()
    args.exe = str(Path(args.exe).resolve())
    if args.slot_arg is not None:
        args.slot = args.slot_arg
    if not args.game_dir.is_dir():
        raise ValueError(f"game directory missing: {args.game_dir}")
    if not Path(args.exe).is_file():
        raise ValueError(f"executable missing: {args.exe}")
    if args.ui_script and not Path(args.ui_script).is_file():
        raise ValueError(f"UI scenario missing: {args.ui_script}")
    stage_runtime_companions(Path(args.exe), args.game_dir)


def run_game(args: argparse.Namespace) -> int:
    normalize_launch(args)
    stop_matching_exe(Path(args.exe))
    clear_log(args.game_dir)
    command = [python_executable(), str(SCRIPTS / "auto_start_game.py"),
               "--game-dir", str(args.game_dir), "--exe", args.exe,
               "--slot", str(args.slot), "--timeout", str(args.timeout)]
    if args.new_game:
        command += ["--new-game", "--party", args.party]
    if args.save_name:
        command += ["--save-name", args.save_name]
    for attr, flag in (("ui_script", "--ui-script"), ("ui_result", "--ui-result"), ("ui_go", "--ui-go")):
        if getattr(args, attr):
            command += [flag, getattr(args, attr)]
    for attr, flag in (("no_wait", "--no-wait"), ("kill_after", "--kill-after")):
        if getattr(args, attr):
            command.append(flag)
    proc = subprocess.run(command, cwd=REPO, capture_output=True, encoding="utf-8", errors="replace")
    STATUS.write_text(proc.stdout + proc.stderr, encoding="utf-8")
    print(proc.stdout + proc.stderr, end="")
    return proc.returncode


def menu(args: argparse.Namespace) -> int:
    """Open the ordinary menus without inherited AutoLoad or AutoUI actions."""
    game_dir = args.game_dir.resolve()
    exe = args.exe.resolve()
    if not game_dir.is_dir():
        raise ValueError(f"game directory missing: {game_dir}")
    if not exe.is_file():
        raise ValueError(f"executable missing: {exe}")
    stage_runtime_companions(exe, game_dir)
    env = {key: value for key, value in os.environ.items()
           if not key.startswith(("IWD2_RE_AUTO_", "IWD2_RE_UI_"))}
    proc = subprocess.Popen([str(exe)], cwd=str(game_dir), env=env)
    print(f"Started game menus (pid={proc.pid})")
    return 0


class Observation:
    """Crash oracle events plus actual successful symbol instrumentation/hits."""
    def __init__(self, logfile):
        self.logfile = logfile
        self.lock = threading.Lock()
        self.crash = False
        self.detached = False
        self.errors: list[str] = []
        self.instrumented: set[str] = set()
        self.hits: dict[str, int] = {}

    def message(self, message, data=None):
        if message.get("type") != "send":
            self.errors.append(str(message))
            self.write("OBSERVER_ERROR " + str(message))
            return
        payload = message.get("payload", {})
        tag = payload.get("tag")
        if tag == "CRASH":
            self.crash = True
            self.write(payload["body"])
        elif tag == "SYM":
            if payload["n"] > 0:
                self.instrumented.add(payload["name"])
            self.write(f"INSTRUMENTED {payload['name']} {payload['n']}")
        elif tag == "NOSYM":
            self.write("NOT_INSTRUMENTED " + payload["name"])
        elif tag == "HITS":
            self.hits[payload["name"]] = payload["n"]
            self.write(f"HITS {payload['name']} {payload['n']}")

    def write(self, text: str):
        with self.lock:
            if not self.logfile.closed:
                self.logfile.write(text + "\n")
                self.logfile.flush()

    def detach(self, *args):
        self.detached = True
        self.write("########## DETACHED " + str(args))


def ui_verdict(path: Path) -> str | None:
    text = read_text(path)
    if not text.strip():
        return "UI-NO-RESULT"
    try:
        rows = [json.loads(line) for line in text.splitlines() if line.strip()]
    except json.JSONDecodeError:
        return "UI-INCOMPLETE"
    verdicts = [r for r in rows if r.get("op") == "verdict"]
    if not verdicts:
        return "UI-INCOMPLETE"
    if verdicts[-1].get("verdict") != "PASS" or any(r.get("status") == "fail" for r in rows):
        return "UI-FAIL"
    return None


def evidence_verdict(args: argparse.Namespace, observation: Observation) -> tuple[int, str]:
    if observation.crash or observation.detached:
        return 1, "CRASH" if observation.crash else "PROCESS-EXITED"
    if observation.errors:
        return 2, "OBSERVER-ERROR"
    if args.ui_script:
        verdict = ui_verdict(Path(args.ui_result))
        if verdict:
            return 2, verdict
    if args.hit:
        if args.hit not in observation.instrumented:
            return 2, "NOT-INSTRUMENTED"
        if observation.hits.get(args.hit, 0) < args.hit_min:
            return 2, "NOT-EXERCISED"
    if args.expect:
        count = sum(bool(re.search(args.expect, line)) for line in
                    read_text(args.game_dir / "iwd2-re-debug.log").splitlines())
        if count < args.expect_min:
            return 2, "NOT-EXERCISED"
    return 0, "CLEAN"


def smoke(args: argparse.Namespace) -> int:
    normalize_launch(args)
    if args.no_wait:
        raise ValueError("smoke must wait for its verdict; use run --no-wait to launch without a gate")
    try:
        import frida
        from frida_crash_guard import JS, COUNT_JS
    except ImportError as exc:
        print(f"RESULT: NOT-INSTRUMENTED ({exc}; use .venv-reagent/Scripts/python.exe)")
        return 2
    seconds = args.seconds
    if seconds is None and (args.save_name or args.new_game) and args.slot_arg is not None:
        seconds = args.slot_arg
    hold = args.hold if args.hold is not None else (seconds if seconds is not None else 90.0)
    if hold < 0 or args.hit_min < 1 or args.expect_min < 1:
        raise ValueError("hold must be nonnegative; minimum hit counts must be positive")
    if args.expect:
        re.compile(args.expect)
    stop_matching_exe(Path(args.exe))
    clear_log(args.game_dir)
    result_path = REPO / "tmp_native_load.txt"
    result_path.unlink(missing_ok=True)
    if args.ui_script:
        args.ui_result = str(Path(args.ui_result).resolve()) if args.ui_result else str(REPO / "tmp_ui_result.jsonl")
        args.ui_go = str(Path(args.ui_go).resolve()) if args.ui_go else str(REPO / "tmp_ui_go.txt")
        Path(args.ui_go).unlink(missing_ok=True)
    env, description = launch_environment(args, result_path)
    device = frida.get_local_device()
    pid = None
    session = None
    rc, verdict = 2, "NOT-LOADED"
    with GUARD_LOG.open("w", encoding="utf-8") as logfile:
        observation = Observation(logfile)
        try:
            pid = device.spawn([args.exe], cwd=str(args.game_dir), env=env)
            session = device.attach(pid)
            session.on("detached", observation.detach)
            guard = session.create_script(JS)
            guard.on("message", observation.message)
            guard.load()
            counter = None
            if args.hit:
                counter = session.create_script(COUNT_JS.replace("%SYMS%", json.dumps([args.hit])))
                counter.on("message", observation.message)
                counter.load()
            observation.write(f"ARMED pid={pid} before resume")
            print(f"ARMED pid={pid}; auto {description}", flush=True)
            if args.ui_script:
                Path(args.ui_go).write_text("go\n", encoding="utf-8")
            device.resume(pid)
            deadline = time.monotonic() + args.timeout
            loaded_at = None
            while True:
                if observation.crash or observation.detached:
                    rc, verdict = evidence_verdict(args, observation)
                    break
                now = time.monotonic()
                if loaded_at is None:
                    if result_path.exists():
                        result = read_result(result_path)
                        if result.get("status") == "loaded":
                            loaded_at = now
                            print(f"loaded: {result.get('detail', '')}; observing {hold:g}s", flush=True)
                        elif result.get("status") == "failed":
                            rc, verdict = 1, "LOAD-FAILED"
                            print(read_text(result_path))
                            break
                    if now >= deadline:
                        break
                elif now - loaded_at >= hold:
                    # Flush counts synchronously; do not lose the last <2 seconds.
                    if counter is not None:
                        for name, count in counter.exports_sync.getcounts().items():
                            observation.hits[name] = count
                            observation.write(f"HITS {name} {count}")
                    rc, verdict = evidence_verdict(args, observation)
                    break
                time.sleep(0.1)
        except Exception as exc:
            rc, verdict = 2, "HARNESS-ERROR"
            observation.write(repr(exc))
            print(f"{type(exc).__name__}: {exc}")
        finally:
            if pid is not None:
                try:
                    device.kill(pid)
                except Exception:
                    pass
            if session is not None:
                try:
                    session.detach()
                except Exception:
                    pass
        suffix = f" ({args.hit} hit x{observation.hits.get(args.hit, 0)})" if args.hit else ""
        output = f"RESULT: {verdict}{suffix}\n"
        if verdict == "CLEAN" and not (args.hit or args.expect):
            output += "No symbol or debug-log gate requested; this does not prove a specific recovered function ran.\n"
        if observation.crash:
            output += read_text(GUARD_LOG)
        STATUS.write_text(output, encoding="utf-8")
        print(output, end="")
    return rc


def parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)
    b = sub.add_parser("build", help="VS2019/v142 Debug Win32; never kills a process")
    b.add_argument("--build-dir", type=Path, default=build_directory())
    b.add_argument("--jobs", type=int, default=4)
    b.add_argument("--run", action="store_true")
    menus = sub.add_parser("menu", help="open the normal game menus without AutoLoad or AutoUI")
    menus.add_argument("--game-dir", type=Path, default=game_directory())
    menus.add_argument("--exe", type=Path, default=build_directory() / "Debug" / "iwd2-re.exe")
    for name in ("run", "smoke"):
        launch = sub.add_parser(name)
        add_launch_arguments(launch)
        launch.set_defaults(slot=3)
        launch.add_argument("slot_arg", nargs="?", type=int)
        launch.add_argument("--save", dest="save_name")
        launch.add_argument("--ui", dest="ui_script")
        if name == "smoke":
            launch.add_argument("seconds", nargs="?", type=float)
            launch.add_argument("--hold", type=float, help="observation seconds after loading (default 90)")
            launch.add_argument("--hit")
            launch.add_argument("--hit-min", type=int, default=1)
            launch.add_argument("--expect")
            launch.add_argument("--expect-min", type=int, default=1)
    for name in ("log", "tail", "clearlog"):
        log = sub.add_parser(name)
        if name == "log":
            log.add_argument("regex")
        if name == "tail":
            log.add_argument("lines", nargs="?", type=int, default=50)
        log.add_argument("-n", type=int, default=50)
        log.add_argument("-f", "--file", type=Path, default=game_directory() / "iwd2-re-debug.log")
    sub.add_parser("status")
    sub.add_parser("ps")
    kill = sub.add_parser("kill")
    kill.add_argument("target", nargs="?", choices=("ours", "orig", "all"), default="ours")
    kill.add_argument("--game-dir", type=Path, default=game_directory())
    kill.add_argument("--exe", type=Path, default=build_directory() / "Debug" / "iwd2-re.exe")
    sub.add_parser("trace", help="frida_orig.py arguments; original trace with a verdict", add_help=False)
    sub.add_parser("frida", help="run a probe in this desktop session (foreground)", add_help=False)
    return ap


def main(argv: list[str] | None = None) -> int:
    utf8_stdio()
    argv = sys.argv[1:] if argv is None else argv
    # Forward whole argument lists, including --help, to the trace/probe driver.
    if argv and argv[0] in ("trace", "frida"):
        if argv[0] == "trace":
            command = [python_executable(), str(SCRIPTS / "frida_orig.py"), *argv[1:]]
        elif len(argv) > 1:
            command = [python_executable(), str(Path(argv[1]).resolve()), *argv[2:]]
        else:
            print("usage: native.py frida <script.py> [args...]")
            return 2
        return subprocess.run(command, cwd=REPO).returncode
    args = parser().parse_args(argv)
    if os.name != "nt":
        print("native.py requires Windows; use scripts/vm.sh on Linux")
        return 2
    try:
        if args.command == "build":
            return build(args)
        if args.command == "run":
            return run_game(args)
        if args.command == "menu":
            return menu(args)
        if args.command == "smoke":
            return smoke(args)
        if args.command == "status":
            print(read_text(STATUS), end="")
        elif args.command in ("log", "tail"):
            lines = read_text(args.file).splitlines()
            if args.command == "log":
                lines = [line for line in lines if re.search(args.regex, line)]
            count = args.lines if args.command == "tail" else args.n
            print("\n".join(lines[-count:]))
        elif args.command == "clearlog":
            if args.file.exists():
                args.file.write_text("", encoding="utf-8")
        elif args.command == "kill":
            if args.target in ("ours", "all"):
                stop_matching_exe(args.exe)
            if args.target in ("orig", "all"):
                stop_matching_exe(args.game_dir / "IWD2.exe")
        elif args.command == "ps":
            proc = powershell("Get-Process iwd2-re,IWD2 -ErrorAction SilentlyContinue | Select-Object Id,ProcessName,Path | Format-Table -AutoSize")
            print(proc.stdout, end="")
        return 0
    except (OSError, ValueError, RuntimeError) as exc:
        print(f"HARNESS ERROR: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
