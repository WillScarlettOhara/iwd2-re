#!/usr/bin/env python3
"""frida_orig.py - unattended Frida trace of the ORIGINAL IWD2.exe, with a verdict.

`vm.sh smoke` does this for our build: launch it, load a save, watch for a
fault, count hits, print RESULT and exit with a code.  This is the counterpart
for the original binary -- the one we have no source for and therefore the only
place a runtime question can actually be answered.

  scripts/vm.sh trace --hooks scripts/probes/store_ownership.json --load-slot 3
  python scripts/frida_orig.py --hooks h.json --load-slot 3 --out C:\\iwd2-re\\t.jsonl

It eats the same hook table as frida_probe.py (schema in frida_hooks.py) and
adds: getting the game into a loaded save without a human, a crash handler, hit
counting, and a machine-readable verdict.

WHY IT SPAWNS RATHER THAN ATTACHES: the interesting questions are usually about
startup and load order (when does m_idLocalPlayer become 1?), and attaching to a
game that already reached the main menu has missed all of it.

HOW IT GETS INTO A SAVE, unattended, with nobody at the keyboard:

  skip the intro movies -> wait for input to go live -> dismiss the network popup
  -> calibrate the cursor map -> click Load Game -> LoadGame(slot)
  -> click Done on Character Arbitration -> world engine activated

Every click is a REAL click: the cursor is moved onto the control and the left
button is pressed, then the engine's own poll (CChitin::AsynchronousUpdate:1537)
dispatches OnLButtonDown/Up exactly as it would for a hand.  That indirection is
not decoration -- calling the handlers directly deadlocks, see the comment on the
input block in DRIVER_JS.  Each step below it is there because a measured run
failed without it; the reasons are in the comments, and none of them are guessable
from the decompile.

--load-slot is a VISIBLE ROW of the load screen, the same number our build's
--slot takes (CScreenLoad.cpp:240-246), so the two sides stay comparable.
--load-slot -1 leaves the game at the menu, for startup/menu/network traces.

NOT for our build: its addresses are a different image entirely, and the debug
build's +0xA base shift breaks Frida member reads (53fe6dccc766).  Instrument
our side with Iwd2DebugLog and diff the two logs -- docs/frida-differential-tracing.md.

Output contract (what vm.sh trace parses, and what to read by hand):
  <out>          JSONL, one record per hook hit, fsync'd per line
  <out>.status   plain lines: `loaded: ...`, `HITS <name> <n>`, `RESULT: <verdict>`
Verdicts: CLEAN 0 | CRASH 1 | NOT-LOADED 2 | NOT-EXERCISED 2.
NOT-LOADED and NOT-EXERCISED both mean "ran, proved nothing" -- exit 2, never a
silent pass (see the --hit lesson behind vm.sh smoke).

The six gotchas of a session-1 payload all apply and are handled here; they are
enumerated in frida_probe.py's docstring.  Read them before editing this file.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
import time
from pathlib import Path

from runtime_paths import REPO, game_directory

import frida

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from frida_hooks import (  # noqa: E402
    PRELUDE,
    hit_counts_js,
    hooks_js,
    install_dump_js,
)

GAME_DIR = str(game_directory())
ORIG_EXE = os.path.join(GAME_DIR, "IWD2.exe")

# The load driver's call sites, all resolved from our source's // 0xADDR markers.
CONN_UPDATE     = 0x5FB3E0   # CScreenConnection::TimerAsynchronousUpdate
CONN_READY      = 0x601CB0   # CScreenConnection::AutoSelectServiceProvider
CONN_DISMISS    = 0x5FC850   # CScreenConnection::DismissPopup
UIMGR_UPDATE    = 0x4D3F00   # CUIManager::TimerAsynchronousUpdate
UIMGR_OFF       = 0x0030     # CBaldurEngine::m_cUIManager
UIMGR_GETPANEL  = 0x4D4000   # CUIManager::GetPanel(DWORD nID)
PANEL_GETCONTROL = 0x4D2CA0  # CUIPanel::GetControl(DWORD nID)
CONN_LBTNDOWN   = 0x5FB2B0   # CScreenConnection::OnLButtonDown(CPoint)
CONN_LBTNUP     = 0x5FB2F0   # CScreenConnection::OnLButtonUp(CPoint)
CONN_MOUSEMOVE  = 0x5FB330   # CScreenConnection::OnMouseMove(CPoint)
CHITIN_PTPOINTER = 0x1906    # CChitin::m_ptPointer
CHITIN_LBUTTON   = 0x0004    # CChitin::m_mouseLButton (the VK the engine polls)

# Load Game on GUICONN is panel 0 / control 7 (CUIControlFactory.cpp:316).
LOAD_PANEL, LOAD_CONTROL = 0, 7
# Binary-mirror layouts, pack(2): CUIPanel::m_ptOrigin +0x24, CUIControlBase
# m_ptOrigin +0x0E / m_size +0x16 / m_bActive +0x1E.
PANEL_ORIGIN, CTRL_ORIGIN, CTRL_SIZE, CTRL_ACTIVE = 0x24, 0x0E, 0x16, 0x1E
CONN_PROTOCOL   = 0x0466     # CScreenConnection::m_nProtocol      (0 = single player)
CONN_ALLOWINPUT = 0x04B2     # CScreenConnection::m_bAllowInput
CONN_EMWAITING  = 0x049A     # CScreenConnection::m_bEMWaiting -- gates OnLButtonDown
LOAD_BUTTON     = 0x5FCA00   # CScreenConnection::OnLoadGameButtonClick(BOOL bQuick)
LOAD_ACTIVATED  = 0x63B410   # CScreenLoad::EngineActivated
LOAD_GAME       = 0x63BE80   # CScreenLoad::LoadGame(INT nSlot)
WORLD_ACTIVATED = 0x6869C0   # CScreenWorld::EngineActivated -- the "loaded" signal
MP_ACTIVATED    = 0x648ED0   # CScreenMultiPlayer::EngineActivated (Character Arbitration)
MP_PANEL, MP_DONE = 0, 28    # GUIMP Done button (CUIControlFactory.cpp:583)
PLAY_MOVIE_INT  = 0x43F230   # CBaldurProjector::PlayMovieInternal(const CResRef&, BOOL)

# The intro movies are not cosmetic here, they are a hard block: the connection
# screen runs ONE update, then PlayMovieInternal does SelectEngine(projector)
# (CBaldurProjector.cpp:441-446) and the menu stops updating until the movie is
# over -- measured, 1 TimerAsynchronousUpdate in 32714 engine ticks.  Replacing
# that one chokepoint with a no-op means the projector is never selected at all,
# so the menu keeps ticking and the load driver gets its turn.  Every caller goes
# through it (PlayMovie only clears the queue first), and it is a cleaner skip
# than marking the movies played in the game's ini, which would change the user's
# install rather than this one run.
SKIP_MOVIES_JS = """
Interceptor.replace(ptr(%(play_movie)#x), new NativeCallback(function (thiz, resref, asynch) {
  send({ tag: 'drive', step: 'skipMovie' });
}, 'void', ['pointer', 'pointer', 'int'], 'thiscall'));
"""

# Hooking onLeave, never mid-function: Frida's 5-byte x86 trampoline corrupts a
# branch cluster and takes the game down with it (docs/frida-differential-tracing.md:101).
# Pure helpers are also exercised under Node by test_frida_calibration.py.
CURSOR_CALIBRATION_JS = """
function fitCursorMap(first, second) {
  const dx = second.desktop[0] - first.desktop[0];
  const dy = second.desktop[1] - first.desktop[1];
  if (dx <= 0 || dy <= 0) return null;
  if (first.game.concat(second.game).some(v => !Number.isFinite(v) || v < 0)) return null;
  const ax = (second.game[0] - first.game[0]) / dx;
  const ay = (second.game[1] - first.game[1]) / dy;
  // A stale first sample produced a NEGATIVE X scale on native Windows.
  // Neither negative, zero, nor implausibly large scales can describe a client.
  if (!(ax > 0 && ay > 0 && ax <= 16 && ay <= 16)) return null;
  return {ax: ax, ay: ay, bx: first.game[0] - ax * first.desktop[0],
          by: first.game[1] - ay * first.desktop[1]};
}
function mapMatchesSample(map, sample, tolerance) {
  return Math.abs(map.ax * sample.desktop[0] + map.bx - sample.game[0]) <= tolerance &&
         Math.abs(map.ay * sample.desktop[1] + map.by - sample.game[1]) <= tolerance;
}
function stableCursorSample(state, point, desktopReady, now) {
  state.polls++;
  if (!desktopReady || point.some(v => !Number.isFinite(v) || v < 0) ||
      state.polls < 4 || now - state.started < 150) {
    state.stable = 0; state.last = null;
    return null;
  }
  if (state.last && Math.abs(point[0] - state.last[0]) <= 1 && Math.abs(point[1] - state.last[1]) <= 1) {
    state.stable++;
  } else state.stable = 1;
  state.last = point;
  return state.stable >= 3 ? point : null;
}
"""

DRIVER_JS = """
const LOAD_SLOT = %(slot)d;
const SETTLE = %(settle)d;
const FALLBACK_TICKS = %(fallback)d;
let ticks = 0, settled = 0, inputReady = false, dismissed = false;
let loading = false, connThis = null, mpThis = null;

const LoadGame = new NativeFunction(ptr(%(load_game)#x), 'void',
                                    ['pointer', 'int'], 'thiscall');
const DismissPopup = new NativeFunction(ptr(%(conn_dismiss)#x), 'void',
                                        ['pointer'], 'thiscall');
const GetPanel = new NativeFunction(ptr(%(getpanel)#x), 'pointer',
                                    ['pointer', 'uint32'], 'thiscall');
const GetControl = new NativeFunction(ptr(%(getcontrol)#x), 'pointer',
                                      ['pointer', 'uint32'], 'thiscall');

// The engine dispatches its own clicks: CChitin::AsynchronousUpdate:1537-1577
// polls GetAsyncKeyState(m_mouseLButton) and calls pActiveEngine->OnLButtonDown/Up
// with m_ptPointer -- but only `if (m_ptPointer.x != -1)`, and m_ptPointer comes
// from the real cursor (:1439/:1488), which reads (-1,-1) whenever the pointer is
// off the window. Unattended, the engine therefore never dispatches anything.
// Calling the handlers ourselves does not work either: OnLoadGameButtonClick opens
// by taking the UI manager's critical section (CScreenConnection.cpp:1142-1143)
// and never returns from it, from any of the three hook positions tried. So we
// move the real cursor onto the control and let the engine's own poll see the
// press -- same code path a hand takes, right thread, right lock state. This is
// the "polite physical hijack" tier, which CLAUDE.md allows for unattended runs.
// Real OS input rather than an Interceptor on GetAsyncKeyState: the engine polls
// that API from several threads every tick, and interposing on it for the whole
// run is a lot of hot-path overhead for the same effect.
// (Module.getExportByName's two-argument form is gone in Frida 17 -- go through
// the module object.)
const user32 = Process.getModuleByName('user32.dll');
const SetCursorPos = new NativeFunction(user32.getExportByName('SetCursorPos'),
                                        'int', ['int', 'int'], 'stdcall');
const mouse_event = new NativeFunction(user32.getExportByName('mouse_event'), 'void',
                                       ['uint32', 'uint32', 'uint32', 'uint32', 'pointer'],
                                       'stdcall');
const MOUSEEVENTF_LEFTDOWN = 0x0002, MOUSEEVENTF_LEFTUP = 0x0004;
const EnumWindows = new NativeFunction(user32.getExportByName('EnumWindows'), 'int', ['pointer', 'pointer'], 'stdcall');
const GetWindowThreadProcessId = new NativeFunction(user32.getExportByName('GetWindowThreadProcessId'), 'uint32', ['pointer', 'pointer'], 'stdcall');
const IsWindowVisible = new NativeFunction(user32.getExportByName('IsWindowVisible'), 'int', ['pointer'], 'stdcall');
const GetClientRect = new NativeFunction(user32.getExportByName('GetClientRect'), 'int', ['pointer', 'pointer'], 'stdcall');
const ClientToScreen = new NativeFunction(user32.getExportByName('ClientToScreen'), 'int', ['pointer', 'pointer'], 'stdcall');
const GetCursorPos = new NativeFunction(user32.getExportByName('GetCursorPos'), 'int', ['pointer'], 'stdcall');
const SetForegroundWindow = new NativeFunction(user32.getExportByName('SetForegroundWindow'), 'int', ['pointer'], 'stdcall');
// IWD2 is x86: POINT is two 32-bit integers passed by value on the stack.
const WindowFromPoint = new NativeFunction(user32.getExportByName('WindowFromPoint'), 'pointer', ['int', 'int'], 'stdcall');
let driveFailed = false, gameWindow = null;

function readPointer() {
  const c = ptr(0x8CF6D8).readPointer().add(%(ptpointer)#x);
  return [c.readS32(), c.add(4).readS32()];
}

function desktopPointer() {
  const point = Memory.alloc(8);
  if (!GetCursorPos(point)) return null;
  return [point.readS32(), point.add(4).readS32()];
}
function ownedPoint(point) {
  if (!point) return false;
  const hwnd = WindowFromPoint(point[0], point[1]);
  if (hwnd.isNull()) return false;
  const pid = Memory.alloc(4);
  GetWindowThreadProcessId(hwnd, pid);
  return pid.readU32() === Process.id;
}
function clientBounds(hwnd) {
  const rect = Memory.alloc(16), origin = Memory.alloc(8), pid = Memory.alloc(4);
  GetWindowThreadProcessId(hwnd, pid);
  if (pid.readU32() !== Process.id || !IsWindowVisible(hwnd) || !GetClientRect(hwnd, rect)) return null;
  const width = rect.add(8).readS32() - rect.readS32();
  const height = rect.add(12).readS32() - rect.add(4).readS32();
  if (width < 100 || height < 100) return null;
  origin.writeS32(0); origin.add(4).writeS32(0);
  if (!ClientToScreen(hwnd, origin)) return null;
  return {hwnd: hwnd, left: origin.readS32(), top: origin.add(4).readS32(), width: width, height: height};
}
function findGameWindow() {
  let best = null;
  const callback = new NativeCallback(function(hwnd, unused) {
    const rect = clientBounds(hwnd);
    if (rect && (!best || rect.width * rect.height > best.width * best.height)) best = rect;
    return 1;
  }, 'int', ['pointer', 'pointer'], 'stdcall');
  EnumWindows(callback, ptr(0));
  return best;
}
function insideClient(point) {
  const rect = gameWindow ? clientBounds(gameWindow.hwnd) : null;
  return rect && point[0] >= rect.left && point[0] < rect.left + rect.width &&
         point[1] >= rect.top && point[1] < rect.top + rect.height;
}
function newSample() { return {started: Date.now(), polls: 0, stable: 0, last: null}; }
function pollSample(state, target) {
  const desktop = desktopPointer();
  const ready = desktop && Math.abs(desktop[0] - target[0]) <= 1 &&
                Math.abs(desktop[1] - target[1]) <= 1 && ownedPoint(desktop) && insideClient(desktop);
  return stableCursorSample(state, readPointer(), ready, Date.now());
}

// Probe actual client-relative interior points only after inputReady. Each
// position settles for >=150ms/four polls and three stable engine samples.
// A third point validates the fitted map; no negative/stale map reaches a click.
let calibrated = false, calStep = 0, calAttempts = 0, calPoints = [], calSamples = [], calSample = null;
let mapAx = 1, mapBx = 0, mapAy = 1, mapBy = 0;
function calibrationFailed(why) {
  send({tag: 'drive', step: 'calibrationRetry', attempt: ++calAttempts, why: why});
  calStep = 0;
  if (calAttempts >= 3) {
    driveFailed = true;
    send({tag: 'drive', step: 'clickFailed', why: 'cursor calibration failed: ' + why});
  }
  return false;
}
function calibrate() {
  if (calStep === 0) {
    gameWindow = findGameWindow();
    if (!gameWindow) return calibrationFailed('no visible client belonging to original PID');
    SetForegroundWindow(gameWindow.hwnd);
    calPoints = [0.25, 0.75, 0.5].map(fraction =>
      [gameWindow.left + Math.round(gameWindow.width * fraction),
       gameWindow.top + Math.round(gameWindow.height * fraction)]);
    calSamples = []; calSample = newSample(); calStep = 1;
    send({tag: 'drive', step: 'calibrationWindow', pid: Process.id, hwnd: gameWindow.hwnd.toString(),
          left: gameWindow.left, top: gameWindow.top, width: gameWindow.width, height: gameWindow.height});
    // DDrawCompat 0.5.4 reports FALSE from the process-local SetCursorPos hook
    // even when its virtual cursor moves. The stable GetCursorPos, HWND-owner,
    // and engine m_ptPointer checks below are the authoritative result.
    SetCursorPos(calPoints[0][0], calPoints[0][1]);
    return false;
  }
  const g = pollSample(calSample, calPoints[calStep - 1]);
  if (!g) {
    if (Date.now() - calSample.started > 3000) return calibrationFailed('cursor/client sample never stabilized');
    return false;
  }
  const sample = {desktop: calPoints[calStep - 1], game: g};
  calSamples.push(sample);
  send({tag: 'drive', step: 'calibrationSample', sample: calStep, desktop: sample.desktop, game: sample.game});
  if (calStep < 3) {
    const next = calPoints[calStep++];
    calSample = newSample();
    SetCursorPos(next[0], next[1]);
    return false;
  }
  const map = fitCursorMap(calSamples[0], calSamples[1]);
  if (!map || !mapMatchesSample(map, calSamples[2], 2)) return calibrationFailed('nonpositive or inconsistent cursor map');
  mapAx = map.ax; mapBx = map.bx; mapAy = map.ay; mapBy = map.by;
  calibrated = true;
  send({ tag: 'drive', step: 'calibrated', ax: mapAx, bx: mapBx, ay: mapAy, by: mapBy });
  return false;
}
function aimAt(x, y, dx, dy) {
  const point = [Math.round((x - mapBx) / mapAx), Math.round((y - mapBy) / mapAy)];
  if (dx || dy) {
    const current = desktopPointer();
    if (!current) return null;
    point[0] = Math.round(current[0] + dx / mapAx);
    point[1] = Math.round(current[1] + dy / mapAy);
  }
  if (!insideClient(point)) return null;
  SetCursorPos(point[0], point[1]);
  return point;
}

// One click job at a time: {thiz, panel, ctrl, label}. The manager update of the
// owning screen runs it, so a job posted for a screen that is not up yet simply
// waits until that screen starts ticking.
let job = null;

function runJob() {
  if (job.state === 'resolve') {
    // The control origin is panel-relative, so the panel origin is added back --
    // same arithmetic as src/AutoUI.cpp:743-745.
    const panel = GetPanel(job.thiz.add(%(uimgr_off)#x), job.panel);
    if (panel.isNull()) { finishJob('no panel'); return; }
    const ctrl = GetControl(panel, job.ctrl);
    if (ctrl.isNull()) { finishJob('no control'); return; }
    // A control the engine has disabled swallows the click silently; wait for it
    // rather than clicking into the void (the arbitration Done button only wakes
    // up once the server has finished loading).
    if (ctrl.add(%(ctrl_active)#x).readU8() === 0) {
      if (++job.waited > 600) { finishJob('control never became active'); }
      return;
    }
    job.x = panel.add(%(panel_origin)#x).readS32()
          + ctrl.add(%(ctrl_origin)#x).readS32()
          + (ctrl.add(%(ctrl_size)#x).readS32() >> 1);
    job.y = panel.add(%(panel_origin)#x + 4).readS32()
          + ctrl.add(%(ctrl_origin)#x + 4).readS32()
          + (ctrl.add(%(ctrl_size)#x + 4).readS32() >> 1);
    job.desktop = aimAt(job.x, job.y, 0, 0);
    if (!job.desktop) { finishJob('target maps outside the game client'); return; }
    job.sample = newSample();
    job.state = 'aim';
    send({ tag: 'drive', step: 'aim', target: job.label, x: job.x, y: job.y });
    return;
  }

  if (job.state === 'aim') {
    const at = pollSample(job.sample, job.desktop);
    if (!at) {
      if (Date.now() - job.sample.started > 3000) finishJob('cursor aim never stabilized inside the game');
      return;
    }
    if (Math.abs(at[0] - job.x) > 2 || Math.abs(at[1] - job.y) > 2) {
      // Linear map, so one correction is normally enough; retry a few times to
      // absorb rounding and a cursor the desktop clamped.
      if (++job.aimTries > 4) { finishJob('cursor never reached the control'); return; }
      job.desktop = aimAt(job.x, job.y, job.x - at[0], job.y - at[1]);
      if (!job.desktop) { finishJob('corrected aim lies outside the game client'); return; }
      job.sample = newSample();
      return;
    }
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
    job.state = 'press'; job.pressTicks = 0;
    send({ tag: 'drive', step: 'press', target: job.label, x: job.x, y: job.y, ptPointer: at });
    return;
  }

  // Hold a few ticks before releasing: the engine samples the button state once
  // per tick, so a one-tick press can fall between two polls.
  if (job.state === 'press') {
    if (++job.pressTicks < 4) return;
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
    send({ tag: 'drive', step: 'release', target: job.label });
    finishJob(null);
  }
}

function finishJob(why) {
  if (why && job && job.state === 'press') mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
  if (why) send({ tag: 'drive', step: 'clickFailed', target: job.label, why: why });
  job = null;
}
function postJob(thiz, panel, ctrl, label) {
  job = { thiz: thiz, panel: panel, ctrl: ctrl, label: label,
          state: 'resolve', waited: 0, aimTries: 0, pressTicks: 0, x: 0, y: 0 };
}

// The connection screen comes up with popup 19 ("Finding the network devices on
// this host...") and m_bAllowInput FALSE, and stays that way until the service
// provider enumeration counts down (CScreenConnection.cpp:701-746). Clicking
// before that FREEZES the game -- measured, twice, identically.
// AutoSelectServiceProvider is called on the very line that sets
// m_bAllowInput = TRUE, so it is the "input is live" event. (DismissPopup is NOT:
// the original skips it when byte_8F376C is set, and it never fired in any run.)
Interceptor.attach(ptr(%(conn_ready)#x), {
  onEnter() { if (!inputReady) { inputReady = true; send({ tag: 'drive', step: 'inputReady', ticks: ticks }); } }
});

Interceptor.attach(ptr(%(conn_update)#x), {
  onEnter() { connThis = this.context.ecx; ticks++; }
});

Interceptor.attach(ptr(%(mp_activated)#x), {
  onEnter() {
    mpThis = this.context.ecx;
    if (LOAD_SLOT < 0) return;
    // Loading through the connection screen is the multiplayer HOST path: it ends
    // in SetArbitrationLockStatus (CScreenConnection.cpp:1285-1287), so the game
    // parks on Character Arbitration until Done is pressed. Done is panel 0 /
    // control 28 on GUIMP (CUIControlFactory.cpp:583).
    send({ tag: 'drive', step: 'arbitration' });
    postJob(mpThis, %(mp_panel)d, %(mp_done)d, 'Done');
  }
});

// WHERE the click is driven from matters: the engine's own auto-load call site is
// mid-body (CScreenConnection.cpp:816), right after m_cUIManager.TimerAsynchronousUpdate()
// at :813 -- a position no hook on the enclosing function's boundaries can reach.
// Hooking that inner call puts us exactly there. m_cUIManager is CBaldurEngine+0x30,
// which is how we tell one screen's manager from another's.
Interceptor.attach(ptr(%(uimgr_update)#x), {
  onEnter() { this.mgr = this.context.ecx; },
  onLeave() {
    if (LOAD_SLOT < 0 || driveFailed) return;
    if (dismissed) {
      if (job === null || !this.mgr.equals(job.thiz.add(%(uimgr_off)#x))) return;
      if (!calibrated) calibrate();
      else runJob();
      return;
    }
    if (connThis === null || !this.mgr.equals(connThis.add(%(uimgr_off)#x))) return;
    // FALLBACK_TICKS keeps a config that never reaches AutoSelectServiceProvider
    // from hanging here forever; it is a backstop, not the normal path.
    if (!inputReady && ticks >= FALLBACK_TICKS && connThis.add(%(allow_input)#x).readU32() !== 0) {
      inputReady = true;
      send({tag: 'drive', step: 'inputReadyField', ticks: ticks});
    }
    if (!inputReady || connThis.add(%(allow_input)#x).readU32() === 0) { settled = 0; return; }
    if (++settled < Math.max(4, SETTLE)) return;
    // Popup 19 has to go before the click. The engine dismisses it itself only
    // when byte_8F376C is clear, and that flag is set the moment the intro movies
    // are QUEUED (CScreenConnection.cpp:688), not when they finish -- so skipping
    // the movies leaves the engine's own DismissPopup (:704) unreached and the
    // popup owning the screen. Calling it is the branch the no-movie path takes.
    dismissed = true;
    send({ tag: 'drive', step: 'DismissPopup', ticks: ticks });
    DismissPopup(connThis);
    // Load Game is panel 0 / control 7 on GUICONN (CUIControlFactory.cpp:316).
    postJob(connThis, %(load_panel)d, %(load_control)d, 'LoadGame');
  }
});

Interceptor.attach(ptr(%(load_activated)#x), {
  onEnter() { this.thiz = this.context.ecx; },
  onLeave() {
    if (loading || LOAD_SLOT < 0) return;
    loading = true;
    send({ tag: 'drive', step: 'LoadGame', slot: LOAD_SLOT });
    LoadGame(this.thiz, LOAD_SLOT);
  }
});

Interceptor.attach(ptr(%(world_activated)#x), {
  onEnter() { send({ tag: 'loaded', detail: 'world engine activated' }); }
});
"""

# Lifted from frida_crash_guard.py: same EBP walk, same 'system' pass-through so
# an OutputDebugString exception does not read as a fault, same `return false`
# so the crash stays faithful.  The original has no PDB, so frames go out raw
# and vm.sh symbolizes them host-side through sym.py addr2fn.
CRASH_JS = """
Process.setExceptionHandler(function (d) {
  if (d.type === 'system') return false;
  const ctx = d.context;
  send({ tag: 'crash', type: d.type, address: d.address.toString(),
         memory: d.memory ? { operation: d.memory.operation,
                              address: d.memory.address.toString() } : null,
         eip: ctx.eip.toString(), frames: guard(() => ebpwalk(ctx.ebp, 24)) });
  return false;
});
"""


class Trace:
    def __init__(self, out: Path, status: Path):
        self.out = open(out, "w", encoding="utf-8", errors="replace")
        self.status = open(status, "w", encoding="utf-8", errors="replace")
        self.loaded = False
        self.loaded_detail = ""
        self.crash = None
        self.detached = False
        self.errors = []
        self.counts: dict[str, int] = {}

    def _write(self, fp, line: str) -> None:
        if fp.closed:
            return
        fp.write(line + "\n")
        fp.flush()
        os.fsync(fp.fileno())   # gotcha 3: a crash is the interesting case

    def record(self, payload) -> None:
        self._write(self.out, json.dumps(payload))

    def say(self, line: str) -> None:
        """A status line -- this is what the host polls, so it goes to both."""
        self._write(self.status, line)
        print(line, flush=True)

    def on_message(self, message, data) -> None:
        if message["type"] != "send":
            self.errors.append(message)
            self.record({"tag": "ERROR", "message": message})
            return
        payload = message["payload"]
        tag = payload.get("tag")
        if tag == "HITS":
            self.counts = payload.get("counts", {})
            return          # the timer fires every 2s; only the totals matter
        self.record(payload)
        if tag == "loaded" and not self.loaded:
            self.loaded = True
            self.loaded_detail = payload.get("detail", "")
            self.say(f"loaded: {self.loaded_detail}")
        elif tag == "crash" and self.crash is None:
            self.crash = payload

    def finish(self, drove: bool, hit: str | None, hit_min: int) -> int:
        for name, n in sorted(self.counts.items()):
            self.say(f"HITS {name} {n}")

        if self.crash is not None:
            c = self.crash
            self.say(f"EXCEPTION {c.get('type')} @ {c.get('address')} eip={c.get('eip')}")
            for frame in c.get("frames") or []:
                self.say(f"  frame {frame}")
            verdict, rc = "CRASH", 1
        elif self.detached:
            verdict, rc = "PROCESS-EXITED", 1
        elif self.errors:
            verdict, rc = "OBSERVER-ERROR", 2
        elif drove and not self.loaded:
            verdict, rc = "NOT-LOADED", 2
        elif hit and self.counts.get(hit, 0) < hit_min:
            verdict, rc = "NOT-EXERCISED", 2
        else:
            verdict, rc = "CLEAN", 0

        note = ""
        if hit:
            note = f"  ({hit} hit x{self.counts.get(hit, 0)})"
        elif verdict == "CLEAN" and not self.counts:
            note = "  (no hits -- nothing proved the path ran)"
        self.say(f"RESULT: {verdict}{note}")
        self.out.close()
        self.status.close()
        return rc


def build_js(spec: dict, slot: int, settle: int, skip_movies: bool = True) -> str:
    hooks = spec["hooks"]
    blocks = ["'use strict';", PRELUDE, CRASH_JS, CURSOR_CALIBRATION_JS]
    if skip_movies:
        blocks.append(SKIP_MOVIES_JS % {"play_movie": PLAY_MOVIE_INT})
    # Always installed: with --load-slot -1 the driving halves self-gate, but the
    # "loaded" signal is still worth having.
    blocks.append(DRIVER_JS % {
        "slot": slot, "settle": settle, "fallback": max(600, settle * 30),
        "conn_update": CONN_UPDATE, "conn_ready": CONN_READY,
        "conn_dismiss": CONN_DISMISS,
        "uimgr_update": UIMGR_UPDATE, "uimgr_off": UIMGR_OFF,
        "getpanel": UIMGR_GETPANEL, "getcontrol": PANEL_GETCONTROL,
        "mousemove": CONN_MOUSEMOVE, "lbtndown": CONN_LBTNDOWN, "lbtnup": CONN_LBTNUP,
        "load_panel": LOAD_PANEL, "load_control": LOAD_CONTROL,
        "mp_activated": MP_ACTIVATED, "mp_panel": MP_PANEL, "mp_done": MP_DONE,
        "panel_origin": PANEL_ORIGIN, "ctrl_origin": CTRL_ORIGIN,
        "ctrl_size": CTRL_SIZE, "ctrl_active": CTRL_ACTIVE,
        "allow_input": CONN_ALLOWINPUT, "protocol": CONN_PROTOCOL,
        "em_waiting": CONN_EMWAITING,
        "ptpointer": CHITIN_PTPOINTER, "lbutton_off": CHITIN_LBUTTON,
        "load_button": LOAD_BUTTON,
        "load_activated": LOAD_ACTIVATED, "load_game": LOAD_GAME,
        "world_activated": WORLD_ACTIVATED,
    })
    blocks.append(hooks_js(hooks))
    blocks.append(install_dump_js(hooks))
    blocks.append(hit_counts_js(hooks))
    blocks.append("send({ tag: 'ready', hooks: %d });" % len(hooks))
    return "\n".join(blocks)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--hooks", required=True, help="hook spec JSON (schema: frida_hooks.py)")
    ap.add_argument("--out", default=str(REPO / "tmp_orig_trace.jsonl"))
    ap.add_argument("--status", help="verdict file (default: <out>.status)")
    ap.add_argument("--load-slot", type=int, default=3,
                    help="visible load-screen row to load; -1 leaves the game at the menu")
    ap.add_argument("--settle-ticks", type=int, default=10,
                    help="menu update ticks to wait AFTER the enumeration popup clears")
    ap.add_argument("--post-load", type=float, default=20.0,
                    help="seconds to keep tracing once the save is up")
    ap.add_argument("--timeout", type=float, default=180.0, help="hard cap, from spawn")
    ap.add_argument("--hit", help="hook name that must fire, else NOT-EXERCISED")
    ap.add_argument("--hit-min", type=int, default=1)
    ap.add_argument("--no-skip-movies", dest="skip_movies", action="store_false",
                    help="let the intro movies play (they block the menu -- see SKIP_MOVIES_JS)")
    ap.add_argument("--exe", default=ORIG_EXE)
    ap.add_argument("--game-dir", default=GAME_DIR, help="asset directory/CWD (or IWD2_GAME_DIR)")
    ns = ap.parse_args()

    spec = json.loads(Path(ns.hooks).read_text())
    out = Path(ns.out)
    status = Path(ns.status) if ns.status else Path(str(out) + ".status")
    trace = Trace(out, status)
    trace.say(f"=== spawning {ns.exe} (slot {ns.load_slot}, {len(spec['hooks'])} hooks) ===")

    pid = frida.spawn(ns.exe, cwd=ns.game_dir)
    session = None
    try:
        session = frida.attach(pid)
        session.on("detached", lambda *_: setattr(trace, "detached", True))
        script = session.create_script(
            build_js(spec, ns.load_slot, ns.settle_ticks, ns.skip_movies))
        script.on("message", trace.on_message)
        script.load()
        frida.resume(pid)
        trace.say(f"ARMED pid={pid}")

        deadline = time.monotonic() + ns.timeout
        load_deadline = None
        while time.monotonic() < deadline:
            if trace.crash is not None or trace.detached:
                break
            if trace.loaded and load_deadline is None:
                load_deadline = time.monotonic() + ns.post_load
            if load_deadline is not None and time.monotonic() >= load_deadline:
                break
            time.sleep(0.5)

        return trace.finish(drove=ns.load_slot >= 0, hit=ns.hit, hit_min=ns.hit_min)
    finally:
        # This trace owns only its spawned original, including setup failures.
        try:
            frida.kill(pid)
        except Exception:
            pass
        if session is not None:
            try:
                session.detach()
            except Exception:
                pass
        trace.out.close()
        trace.status.close()


if __name__ == "__main__":
    raise SystemExit(main())
