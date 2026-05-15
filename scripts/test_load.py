#!/usr/bin/env python3
"""Automated IWD2 test: launch, click Load Game, click save, capture log."""
import subprocess, sys, os, time, ctypes
from pathlib import Path
from ctypes import wintypes

REPO = Path(__file__).resolve().parents[1]
DEBUG_LOG = REPO / "debug.log"
EXE = REPO / "build" / "Debug" / "iwd2-re.exe"
GAME_DIR = "C:/GOG Games/Icewind Dale 2"

# In-game coordinates (800x600 space)
# These will be auto-mapped by iterating through the menu
LOAD_GAME_Y = None  # determined at runtime
FIRST_SAVE = (120, 305)

user32 = ctypes.windll.user32

# ---- Window helpers ----
def find_window():
    hwnd = user32.FindWindowW(None, None)
    results = []
    while hwnd:
        buf = ctypes.create_unicode_buffer(256)
        user32.GetWindowTextW(hwnd, buf, 255)
        name = buf.value
        if "Icewind" in name:
            rect = wintypes.RECT()
            user32.GetWindowRect(hwnd, ctypes.byref(rect))
            results.append((hwnd, name, rect.left, rect.top, rect.right, rect.bottom))
        hwnd = user32.GetWindow(hwnd, 2)
    return results[0] if results else None

def click_game(hwnd, game_x, game_y):
    """Click at GAME coordinate (0-800,0-600), accounting for centering."""
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    ww = rect.right - rect.left
    wh = rect.bottom - rect.top
    offset_x = (ww - 800) // 2
    offset_y = (wh - 600) // 2
    abs_x = rect.left + offset_x + game_x
    abs_y = rect.top + offset_y + game_y
    print(f"  click game({game_x},{game_y}) -> screen({abs_x},{abs_y}) off=({offset_x},{offset_y})")
    user32.SetCursorPos(abs_x, abs_y)
    time.sleep(0.02)
    user32.mouse_event(0x0002, 0, 0, 0, 0)
    time.sleep(0.02)
    user32.mouse_event(0x0004, 0, 0, 0, 0)
    time.sleep(0.02)
    # Also send WM_LBUTTONDOWN/UP for games using Windows messages
    lparam = (game_y << 16) | game_x
    user32.PostMessageW(hwnd, 0x0201, 1, lparam)
    time.sleep(0.02)
    user32.PostMessageW(hwnd, 0x0202, 0, lparam)

def log_new_lines(skip_bytes):
    """Return new bytes since skip_bytes, or empty string."""
    try:
        with open(DEBUG_LOG, 'rb') as f:
            f.seek(skip_bytes)
            return f.read().decode(errors='replace')
    except:
        return ""

def main():
    # 0. Kill any existing instance
    os.system("taskkill -f -im iwd2-re.exe 2>nul")

    # 1. Build
    if "--skip-build" not in sys.argv:
        print("Building...")
        rc = subprocess.run(["cmake", "--build", str(REPO / "build"), "--config", "Debug"],
                          capture_output=True, timeout=240)
        if rc.returncode != 0:
            print("BUILD FAILED\n" + rc.stderr.decode(errors='replace')[-500:])
            return 1
        print("Build OK")

    # 2. Clear log
    if DEBUG_LOG.exists():
        DEBUG_LOG.unlink()

    # 3. Launch
    os.chdir(GAME_DIR)
    proc = subprocess.Popen([str(EXE)], cwd=GAME_DIR,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"PID: {proc.pid}")

    # 4. Wait for main menu
    print("Waiting for main menu...")
    for i in range(15):
        time.sleep(1)
        hwnd_info = find_window()
        if hwnd_info:
            break
    if not hwnd_info:
        print("ERROR: window not found")
        proc.kill(); return 1
    hwnd, name, left, top, right, bottom = hwnd_info
    print(f"Window: '{name}' {right-left}x{bottom-top}")

    # Bring to foreground
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.5)

    # 5. Map Load Game button position by trying Y values
    if DEBUG_LOG.exists():
        log_pos = DEBUG_LOG.stat().st_size
    else:
        log_pos = 0
    load_y = None

    # Common main menu button Y positions — try each in separate runs
    # y=262 = New Game (confirmed). Load Game should be a few slots below.
    candidates = [330, 364, 296, 398, 432, 262]
    load_y = None
    for y in candidates:
        click_game(hwnd, 629, y)
        time.sleep(2.0)
        new_txt = log_new_lines(log_pos)
        log_pos = DEBUG_LOG.stat().st_size if DEBUG_LOG.exists() else log_pos
        if "OnLoadGameButtonClick" in new_txt or "StartLoad: begin" in new_txt or "FreeGameSlots" in new_txt:
            load_y = y
            print(f"  -> LOAD GAME at y={y}")
            break
        elif "OnNewGameButtonClick" in new_txt or "NewGame: before Unmarshal" in new_txt:
            print(f"  -> New Game (y={y}) — RESTARTING...")
            # New Game was started, kill and restart for next attempt
            proc.kill()
            time.sleep(2)
            if DEBUG_LOG.exists():
                DEBUG_LOG.unlink()
            proc = subprocess.Popen([str(EXE)], cwd=GAME_DIR,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(8)
            hwnd_info = find_window()
            if not hwnd_info:
                print("ERROR: window lost after restart")
                proc.kill(); return 1
            hwnd, name, left, top, right, bottom = hwnd_info
            user32.SetForegroundWindow(hwnd)
            time.sleep(0.5)
            log_pos = 0
            continue

    if not load_y:
        print("FAIL: Load Game button not found")
        proc.kill(); return 1

    # 6. Click Load Game again to open save list
    print(f"Opening save list...")
    click_game(hwnd, 629, load_y)
    time.sleep(3)

    # 7. Click first save
    save_idx = 0
    for arg in sys.argv:
        if arg.startswith("--save="):
            save_idx = int(arg.split("=")[1])
    save_y = FIRST_SAVE[1] + save_idx * 30
    print(f"Clicking save #{save_idx} at ({FIRST_SAVE[0]},{save_y})")
    click_game(hwnd, FIRST_SAVE[0], save_y)

    # 8. Wait for load
    print("Waiting for game to load...")
    time.sleep(12)

    # 9. Check status
    if proc.poll() is not None:
        print(f"Game exited with code {proc.returncode}")
    else:
        print("Game still running")

    # 10. Kill and show log
    proc.kill()
    time.sleep(1)

    if DEBUG_LOG.exists():
        txt = DEBUG_LOG.read_text(errors='replace')
        
        # Show Unmarshal-related lines
        key_lines = [l for l in txt.splitlines() 
                     if any(k in l for k in ["Unmarshal", "sprite", "member[", 
                                             "LoadGame:", "SetupChar", "CRE header"])]
        if key_lines:
            print("\n=== KEY LOGS ===")
            for l in key_lines[-40:]:
                print(l)
        
        # Always show last few lines
        all_lines = txt.splitlines()
        print(f"\n=== LAST 10 LINES ===")
        for l in all_lines[-10:]:
            print(l)
    else:
        print("No debug.log")

    return 0

if __name__ == "__main__":
    sys.exit(main())
