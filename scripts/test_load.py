#!/usr/bin/env python3
"""IWD2 auto-test: build, launch (auto-clicks Load Game via code hook), captures log."""
import subprocess, sys, os, time
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
DEBUG_LOG = REPO / "debug.log"
EXE = REPO / "build" / "Debug" / "iwd2-re.exe"
GAME_DIR = "C:/GOG Games/Icewind Dale 2"
AUTO_TEST = REPO / "auto_test"

def main():
    # 0. Kill old instance
    os.system("taskkill -f -im iwd2-re.exe 2>nul")
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

    # 2. Clear log
    if DEBUG_LOG.exists():
        DEBUG_LOG.unlink()

    # 3. Create trigger file (code hook auto-clicks Load Game)
    AUTO_TEST.write_text("1")

    # 4. Launch
    os.chdir(GAME_DIR)
    proc = subprocess.Popen([str(EXE)], cwd=GAME_DIR,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"PID: {proc.pid} — waiting 20s...")

    # 5. Wait for load to complete
    time.sleep(30)

    # 6. Check
    if proc.poll() is not None:
        print(f"Game exited with code {proc.returncode}")
    else:
        print("Game still running")

    # 7. Kill
    proc.kill()
    time.sleep(1)

    # 8. Cleanup
    if AUTO_TEST.exists():
        AUTO_TEST.unlink()

    # 9. Show log
    if DEBUG_LOG.exists():
        txt = DEBUG_LOG.read_text(errors='replace')
        key_lines = [l for l in txt.splitlines() 
                     if any(k in l for k in ["Unmarshal", "sprite", "member[", 
                                             "LoadGame:", "SetupChar", "CRE header",
                                             "OnLoadGameButtonClick", "ERROR", "ASSERT"])]
        if key_lines:
            print("\n=== KEY LOGS ===")
            for l in key_lines[-50:]:
                print(l)
        print(f"\n=== LAST 5 LINES ===")
        for l in txt.splitlines()[-5:]:
            print(l)
    else:
        print("No debug.log")

    return 0

if __name__ == "__main__":
    sys.exit(main())
