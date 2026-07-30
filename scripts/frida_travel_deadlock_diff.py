#!/usr/bin/env python3
"""Live differential trace against the ORIGINAL IWD2.exe for the travel-trigger
LoadArea deadlock investigated on branch feat/travel-trigger-arrival.

Our own build (Debug AND Release) reliably deadlocks: the AI thread holds a
GetDeny(sprite, THREAD_ASYNCH) lock across the whole ArriveAtTravelTrigger ->
LoadArea -> WaitForEngine call chain, while CGameArea::Render (main thread)
busy-spins forever on GetShare(sameSprite, THREAD_1) == DENIED. Every piece of
that mechanism (WaitForEngine, the GetDeny wrap in CGameArea::AIUpdate,
ArriveAtTravelTrigger's LoadArea-before-RemoveFromArea order, Render's first
vert-sort loop) was independently re-verified against the real disassembly/
decompile this session and found to match our recovery exactly -- yet the
original game demonstrably completes this same transition (confirmed earlier
this arc via a live click). This script hooks the same mechanism on the
ORIGINAL exe to see whether the same GetShare/DENIED contention ever happens
there, and if not, what's different at the moment WaitForEngine is called.

Hooks (function ENTRIES, absolute addresses, ImageBase 0x400000, no ASLR):
  0x4CED10  CGameTrigger::OnActionButton     -- fires on the staircase click
  0x74CDB0  CGameSprite::ArriveAtTravelTrigger -- reads this+0x5C (m_id)
  0x59FA00  CInfGame::WaitForEngine          -- reads g_pChitin (0x8CF6D8)
                                                 +0x193A (m_bDisplayStale) and
                                                 +0x193E (m_bInSynchronousUpdate)
  0x477740  CGameArea::Render                -- entry/exit, to see if/when it
                                                 runs relative to the AI thread
  0x599A50  CGameObjectArray::GetShare       -- args: index, threadNum, ptr,
                                                 dwTimeout (__thiscall, this=ecx
                                                 unused here).
  0x599C70  CGameObjectArray::GetDeny        -- same signature as GetShare.
  0x59A010  CGameObjectArray::ReleaseDeny    -- args: index, threadNum,
                                                 dwTimeout.
  GetShare/GetDeny/ReleaseDeny are filtered in JS to only the sprite id first
  seen entering ArriveAtTravelTrigger (targetId) -- otherwise these fire
  constantly for every object in the game and drown the trace. Return codes:
  SUCCESS=0, TIMEOUT=1, DELETED=2, BAD_INDEX=3, SHARED=4, DENIED=5.
  Thread slots: THREAD_ASYNCH=0 (AI thread), THREAD_1=1 (Render), THREAD_
  SEARCH=2.

  GetDeny/ReleaseDeny are call-depth/sequence paired (added to resolve the
  prior run's "same-millisecond ReleaseDeny -- is it really the SAME lock?"
  ambiguity): every GetDeny that actually SUCCEEDS (rc==0) for this object
  gets a monotonic seq# pushed onto a per-threadNum stack; every ReleaseDeny
  pops the most recent matching-threadNum entry (LIFO) and reports exactly
  which seq# it closes (pairedSeq) plus the resulting nesting depth
  (depthAfter). GetDeny_enter/_exit also report `depth`, the nesting level
  at that moment. This makes it possible to tell, unambiguously, whether the
  outer per-tick GetDeny(threadNum=0) genuinely stays open across the whole
  ArriveAtTravelTrigger/LoadArea/WaitForEngine chain, or whether the
  same-millisecond ReleaseDeny seen previously was really an unrelated,
  earlier lock on this object.

Run:  python scripts/frida_travel_deadlock_diff.py
Then: load the SAME save used on our build ("000000004-Staircase Transition"
      under MPSave, via the in-game Load Game screen -- the original exe has
      no IWD2_RE_AUTO_ACTION env-var support, so this has to be manual) and
      click the same staircase trigger (id 3342387, AR1000 -> AR1100).
Stop: Ctrl+C. Full JSON also written to tmp_travel_deadlock_diff.log
"""
from __future__ import annotations

import json
import sys
import time
from pathlib import Path

import frida

SCRIPT_DIR = Path(__file__).resolve().parent
GAME_DIR = Path(r"C:\Juegos\Icewind Dale 2")
ORIG_EXE = GAME_DIR / "IWD2.exe"
LOG_PATH = SCRIPT_DIR.parent / "tmp_travel_deadlock_diff.log"

JS = r"""
const OnActionButton = ptr(0x4CED10);
const ArriveAtTravelTrigger = ptr(0x74CDB0);
const WaitForEngine = ptr(0x59FA00);
const Render = ptr(0x477740);
const GetShare = ptr(0x599A50);
const GetDeny = ptr(0x599C70);
const ReleaseDeny = ptr(0x59A010);
const SpriteAIUpdate = ptr(0x6F5FF0);
const LoadArea = ptr(0x5A24D0);
const CHITIN = ptr(0x8CF6D8);

function u32(p){ try { return p.readU32() >>> 0; } catch(e){ return null; } }
function s32(p){ try { return p.readS32(); } catch(e){ return null; } }

function safeAttach(addr, callbacks, label) {
  try {
    Interceptor.attach(addr, callbacks);
    send({tag: 'HOOK_OK', label: label});
  } catch (e) {
    send({tag: 'HOOK_FAIL', label: label, error: e.toString()});
  }
}

// Set on the first ArriveAtTravelTrigger entry; used to filter the very
// noisy GetDeny/GetShare traffic down to just the one sprite we care about.
let targetId = null;

safeAttach(OnActionButton, {
  onEnter(args) {
    const t = this.context.ecx;
    send({tag: 'OnActionButton', t: Date.now(), id: s32(t.add(0x5C)), triggerType: s32(t.add(0x598))});
  }
}, 'OnActionButton');

safeAttach(ArriveAtTravelTrigger, {
  onEnter(args) {
    const t = this.context.ecx;
    const id = s32(t.add(0x5C));
    targetId = id;
    send({tag: 'ArriveAtTravelTrigger', t: Date.now(), id: id, ret: this.returnAddress.toString()});
  }
}, 'ArriveAtTravelTrigger');

safeAttach(WaitForEngine, {
  onEnter(args) {
    const g = u32(CHITIN);
    const bDisplayStale = g ? s32(ptr(g).add(0x193A)) : null;
    const bInSynchronousUpdate = g ? s32(ptr(g).add(0x193E)) : null;
    send({tag: 'WaitForEngine_enter', t: Date.now(), a1: args[0].toInt32(),
      bDisplayStale: bDisplayStale, bInSynchronousUpdate: bInSynchronousUpdate,
      ret: this.returnAddress.toString()});
  },
  onLeave(retval) {
    const g = u32(CHITIN);
    const bDisplayStale = g ? s32(ptr(g).add(0x193A)) : null;
    const bInSynchronousUpdate = g ? s32(ptr(g).add(0x193E)) : null;
    send({tag: 'WaitForEngine_exit', t: Date.now(), bDisplayStale: bDisplayStale,
      bInSynchronousUpdate: bInSynchronousUpdate});
  }
}, 'WaitForEngine');

let renderCount = 0;
safeAttach(Render, {
  onEnter(args) {
    renderCount++;
    this.t0 = Date.now();
    this.pArea = args[0];
    send({tag: 'Render_enter', t: this.t0, pArea: this.pArea.toString(), count: renderCount});
  },
  onLeave(retval) {
    const t1 = Date.now();
    send({tag: 'Render_exit', t: t1, ms: t1 - this.t0, count: renderCount});
  }
}, 'Render');

safeAttach(GetShare, {
  onEnter(args) {
    const index = args[0].toInt32();
    if (targetId === null || index !== targetId) return;
    this.isTarget = true;
    this.threadNum = args[1].toInt32() & 0xff;
    send({tag: 'GetShare_enter', t: Date.now(), index: index, threadNum: this.threadNum});
  },
  onLeave(retval) {
    if (!this.isTarget) return;
    send({tag: 'GetShare_exit', t: Date.now(), threadNum: this.threadNum, rc: retval.toInt32() & 0xff});
  }
}, 'GetShare');

// Call-depth-aware GetDeny/ReleaseDeny pairing (UPDATE 13's open ambiguity):
// a monotonic seq is assigned to each GetDeny call that actually SUCCEEDS
// (rc==0) for this object, pushed onto a per-threadNum-aware stack. Each
// ReleaseDeny pops the most recent matching-threadNum entry (LIFO -- correct
// for a normal nested call stack) and reports exactly which GetDeny seq# it
// closes, plus the resulting nesting depth. This removes the "same
// millisecond, but is it really the SAME lock?" ambiguity that a bare
// timestamp comparison couldn't resolve.
let denySeq = 0;
let denyStack = []; // {seq, threadNum}

safeAttach(GetDeny, {
  onEnter(args) {
    const index = args[0].toInt32();
    if (targetId === null || index !== targetId) return;
    this.isTarget = true;
    this.threadNum = args[1].toInt32() & 0xff;
    send({tag: 'GetDeny_enter', t: Date.now(), index: index, threadNum: this.threadNum, depth: denyStack.length});
  },
  onLeave(retval) {
    if (!this.isTarget) return;
    const rc = retval.toInt32() & 0xff;
    let seq = null;
    if (rc === 0) {
      seq = ++denySeq;
      denyStack.push({seq: seq, threadNum: this.threadNum});
    }
    send({tag: 'GetDeny_exit', t: Date.now(), threadNum: this.threadNum, rc: rc, seq: seq, depth: denyStack.length});
  }
}, 'GetDeny');

safeAttach(ReleaseDeny, {
  onEnter(args) {
    const index = args[0].toInt32();
    if (targetId === null || index !== targetId) return;
    const threadNum = args[1].toInt32() & 0xff;
    let pairedSeq = null;
    for (let i = denyStack.length - 1; i >= 0; i--) {
      if (denyStack[i].threadNum === threadNum) {
        pairedSeq = denyStack[i].seq;
        denyStack.splice(i, 1);
        break;
      }
    }
    send({tag: 'ReleaseDeny', t: Date.now(), index: index, threadNum: threadNum, pairedSeq: pairedSeq, depthAfter: denyStack.length});
  }
}, 'ReleaseDeny');

// CGameSprite::AIUpdate itself -- filtered by `this` (ecx) == targetId's
// sprite pointer is not known ahead of time (targetId is an object-array
// index, not a raw pointer), so instead we read this+0x5C (m_id, same offset
// ArriveAtTravelTrigger uses) and compare against targetId. The key thing we
// want is `this.returnAddress` -- whichever function actually calls this for
// our sprite, since that's the not-yet-identified per-tick caller.
safeAttach(SpriteAIUpdate, {
  onEnter(args) {
    const t = this.context.ecx;
    const id = (function(){ try { return t.add(0x5C).readS32(); } catch(e){ return null; } })();
    if (targetId === null || id !== targetId) return;
    send({tag: 'SpriteAIUpdate_enter', t: Date.now(), id: id, ret: this.returnAddress.toString()});
  },
  onLeave(retval) {
  }
}, 'CGameSprite::AIUpdate');

// CInfGame::LoadArea itself -- settles definitively whether it's called
// from WITHIN the seq-paired GetDeny/ReleaseDeny bracket already captured
// around ArriveAtTravelTrigger's last logged entry, or from some later,
// uncaptured re-entry into ArriveAtTravelTrigger. Not filtered by id (LoadArea
// is a CInfGame method, not a CGameSprite one -- this+0x5C isn't meaningful
// here); only fires once per real area transition so noise isn't a concern.
safeAttach(LoadArea, {
  onEnter(args) {
    send({tag: 'LoadArea_enter', t: Date.now(), ret: this.returnAddress.toString()});
  },
  onLeave(retval) {
    send({tag: 'LoadArea_exit', t: Date.now()});
  }
}, 'CInfGame::LoadArea');
"""


def main() -> int:
    pid = frida.spawn(str(ORIG_EXE), cwd=str(GAME_DIR))
    session = frida.attach(pid)
    log_f = open(LOG_PATH, "w", encoding="utf-8")
    t_start = [None]

    def rel(t):
        if t_start[0] is None:
            t_start[0] = t
        return t - t_start[0]

    def on_message(message, data):
        if message.get("type") != "send":
            if message.get("type") == "error":
                print("[JS ERROR]", message.get("description"))
            return
        p = message["payload"]
        log_f.write(json.dumps(p) + "\n")
        log_f.flush()
        tag = p.get("tag")
        if tag == "HOOK_OK":
            print(f"[hook ok] {p['label']}")
        elif tag == "HOOK_FAIL":
            print(f"[hook FAILED] {p['label']}: {p['error']}")
        elif tag == "OnActionButton":
            print(f"[+{rel(p['t'])}ms] [OnActionButton] id={p['id']} triggerType={p['triggerType']}")
        elif tag == "ArriveAtTravelTrigger":
            print(f"[+{rel(p['t'])}ms] [ArriveAtTravelTrigger] id={p['id']} ret={p['ret']}")
        elif tag == "WaitForEngine_enter":
            print(f"[+{rel(p['t'])}ms] [WaitForEngine enter] a1={p['a1']} bDisplayStale={p['bDisplayStale']} "
                  f"bInSynchronousUpdate={p['bInSynchronousUpdate']} ret={p['ret']}")
        elif tag == "WaitForEngine_exit":
            print(f"[+{rel(p['t'])}ms] [WaitForEngine exit] bDisplayStale={p['bDisplayStale']} "
                  f"bInSynchronousUpdate={p['bInSynchronousUpdate']}")
        elif tag == "Render_enter":
            print(f"[+{rel(p['t'])}ms] [Render enter] pArea={p['pArea']} (call #{p['count']})")
        elif tag == "Render_exit":
            print(f"[+{rel(p['t'])}ms] [Render exit] took {p['ms']}ms (call #{p['count']})")
        elif tag == "GetShare_enter":
            print(f"[+{rel(p['t'])}ms] [GetShare enter] index={p['index']} threadNum={p['threadNum']}")
        elif tag == "GetShare_exit":
            print(f"[+{rel(p['t'])}ms] [GetShare exit] threadNum={p['threadNum']} rc={p['rc']}")
        elif tag == "GetDeny_enter":
            print(f"[+{rel(p['t'])}ms] [GetDeny enter] index={p['index']} threadNum={p['threadNum']} depth={p['depth']}")
        elif tag == "GetDeny_exit":
            print(f"[+{rel(p['t'])}ms] [GetDeny exit] threadNum={p['threadNum']} rc={p['rc']} seq={p['seq']} depth={p['depth']}")
        elif tag == "ReleaseDeny":
            print(f"[+{rel(p['t'])}ms] [ReleaseDeny] index={p['index']} threadNum={p['threadNum']} "
                  f"pairedSeq={p['pairedSeq']} depthAfter={p['depthAfter']}")
        elif tag == "SpriteAIUpdate_enter":
            print(f"[+{rel(p['t'])}ms] [CGameSprite::AIUpdate enter] id={p['id']} CALLER_RET={p['ret']}  <== who calls this?")
        elif tag == "LoadArea_enter":
            print(f"[+{rel(p['t'])}ms] [CInfGame::LoadArea ENTER] CALLER_RET={p['ret']}  <== should be 0x74de7c (ArriveAtTravelTrigger)")
        elif tag == "LoadArea_exit":
            print(f"[+{rel(p['t'])}ms] [CInfGame::LoadArea EXIT]")

    script = session.create_script(JS)
    script.on("message", on_message)
    script.load()
    frida.resume(pid)

    print("[*] Original IWD2.exe spawned under Frida.")
    print("[*] Load the staircase-transition save via the in-game Load Game menu,")
    print("[*] then click the same staircase trigger (AR1000 -> AR1100).")
    print("[*] Watching for: WaitForEngine enter/exit state, Render duration, and any")
    print("[*] THREAD_1 GetShare DENIED (would mean the SAME contention as our build).")
    print("[*] Ctrl+C to stop.\n")
    try:
        while True:
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("\n[*] stopping")
    finally:
        log_f.close()
        try:
            session.detach()
        except Exception:
            pass
        try:
            frida.kill(pid)
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
