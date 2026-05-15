#!/usr/bin/env python3
"""IWD2 auto-test: build, create auto_test trigger, launch, capture log.
The game's CChitin::SynchronousUpdate detects auto_test and auto-loads the first save."""
import subprocess, sys, os, time
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
DEBUG_LOG = REPO / "debug.log"
AUTO_TEST = REPO / "auto_test"
EXE = REPO / "build" / "Debug" / "iwd2-re.exe"
GAME_DIR = "C:/GOG Games/Icewind Dale 2"

def main():
    # 0. Kill old
    os.system("taskkill -f -im iwd2-re.exe 2>nul")
    os.system("taskkill -f -im AutoHotkey*.exe 2>nul")
    time.sleep(1)

    # 1. Build
    if "--skip-build" not in sys.argv:
        print("Building...")
        rc = subprocess.run(["cmake", "--build", str(REPO / "build"), "--config", "Debug"],
                          capture_output=True, timeout=300)
        if rc.returncode != 0:
            print("BUILD FAILED\n" + rc.stderr.decode(errors='replace')[-500:])
            return 1
        print("Build OK")

    # 2. Clear log and create trigger
    if DEBUG_LOG.exists():
        DEBUG_LOG.unlink()
    AUTO_TEST.write_text("1")
    print(f"Created {AUTO_TEST}")

    # 3. Launch game (the code hook handles everything)
    os.chdir(GAME_DIR)
    proc = subprocess.Popen([str(EXE)], cwd=GAME_DIR,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"Game PID: {proc.pid} — waiting 40s...")

    # 4. Wait
    time.sleep(40)

    # 5. Check
    if proc.poll() is not None:
        print(f"Game exited with code 0x{proc.returncode:08X}")
    else:
        print("Game still running — killing")
        proc.kill()
    time.sleep(1)

    # 6. Cleanup
    if AUTO_TEST.exists():
        AUTO_TEST.unlink()

    # 7. Show log
    if DEBUG_LOG.exists():
        txt = DEBUG_LOG.read_text(errors='replace')
        for tag in ["AutoTest", "Unmarshal", "sprite", "member[", "LoadGame:", "SetupChar", "CRE header", "ERROR", "ASSERT"]:
            lines = [l for l in txt.splitlines() if tag in l]
            if lines:
                print(f"\n--- {tag} ---")
                for l in lines[-20:]:
                    print(l)
        print(f"\n=== LAST 5 ===")
        for l in txt.splitlines()[-5:]:
            print(l)
    else:
        print("No debug.log")

    return 0

if __name__ == "__main__":
    sys.exit(main())
