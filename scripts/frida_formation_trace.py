#!/usr/bin/env python3
"""Differential trace of the ORIGINAL IWD2.exe formation pipeline via Frida.

IWD2.exe has no ASLR (ImageBase 0x400000), so Ghidra addresses are absolute.
We hook the formation entry + the offset rotation and dump their args/outputs as
JSON lines. Run our own iwd2-re.exe build with its FORMDBG log under the SAME
click, then diff the two logs to find any divergence the static review missed.

Hooks (all addresses are IWD2.exe absolute, == Ghidra):
  0x4063e0  CAIGroup::GroupSetTarget(CPoint target, BOOL additive,
                                      SHORT formationType, CPoint cursor)  __thiscall
  0x4058e0  RotateOffsets(int* offsets, SHORT count, SHORT rotationDegrees) __cdecl

Usage:
  python scripts/frida_formation_trace.py          # spawn IWD2.exe, hook, log
  python scripts/frida_formation_trace.py --attach  # attach to a running IWD2.exe

Then in-game: load the dock save, click a formation toolbar button + ground.
Log: tmp_frida_formation.log (repo root). Ctrl-C / kill to stop.
"""
import frida
import sys
import os
import json

from runtime_paths import game_directory

GAME_DIR = str(game_directory())
EXE = os.path.join(GAME_DIR, "IWD2.exe")
LOG = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "tmp_frida_formation.log")

JS = r"""
'use strict';

// IWD2.exe, no ASLR -> Ghidra addresses are absolute.
const GroupSetTarget = ptr(0x4063e0);
const RotateOffsets  = ptr(0x4058e0);
const GetDeny        = ptr(0x599c70);   // CGameObjectArray::GetDeny
// GroupSetTarget spans [0x4063e0, 0x407280); used to filter GetDeny by call origin.
const GST_LO = ptr(0x4063e0);
const GST_HI = ptr(0x407280);
const M_POS  = 0x06;                     // CGameObject::m_pos (x@+0x06, y@+0x0a)

function s16(v) { return (v.toInt32() << 16) >> 16; }

// CAIGroup::GroupSetTarget(CPoint target, BOOL additive, SHORT fType, CPoint cursor)
// __thiscall: ecx = this; stack args start at [esp+4] == args[0].
Interceptor.attach(GroupSetTarget, {
  onEnter(args) {
    const thiz = this.context.ecx;
    // Embedded CPtrList @+0x06 (vtable@+6) -> m_pNodeHead@+0x0a; node {next@0,prev@4,data@8}.
    const members = [];
    try {
      let node = thiz.add(0x0a).readPointer();
      let guard = 0;
      while (!node.isNull() && guard < 16) {
        members.push(node.add(8).readU32());
        node = node.readPointer();   // pNext @0 = forward (head order)
        guard++;
      }
    } catch (e) { members.push('ERR:' + e); }
    send({
      tag: 'GST',
      this: thiz.toString(),
      target: [args[0].toInt32(), args[1].toInt32()],
      additive: args[2].toInt32(),
      formationType: s16(args[3]),
      cursor: [args[4].toInt32(), args[5].toInt32()],
      members: members,
    });
  }
});

// RotateOffsets(int* offsets, SHORT count, SHORT rotationDegrees) __cdecl
// offsets stride = 8 bytes (x:int, y:int) per slot.
function dumpOffsets(p, n) {
  const a = [];
  for (let i = 0; i < n; i++) {
    a.push([p.add(i * 8).readS32(), p.add(i * 8 + 4).readS32()]);
  }
  return a;
}
Interceptor.attach(RotateOffsets, {
  onEnter(args) {
    this.p = args[0];
    this.n = s16(args[1]);
    this.rot = s16(args[2]);
    if (this.n < 0 || this.n > 64) { this.n = 0; return; }  // guard bad reads
    send({ tag: 'ROT_in', count: this.n, rotDeg: this.rot, offsets: dumpOffsets(this.p, this.n) });
  },
  onLeave() {
    if (this.n > 0) {
      send({ tag: 'ROT_out', count: this.n, rotDeg: this.rot, offsets: dumpOffsets(this.p, this.n) });
    }
  }
});

// CGameObjectArray::GetDeny(LONG index, BYTE threadNum, CGameObject** ptr, DWORD timeout)
// __thiscall: ecx = this; args[0]=index, args[2]=out CGameObject**. SUCCESS == 0.
// GST resolves each member through GetDeny; filter by return address so we only
// capture the members the formation code touches, then read m_pos off the sprite.
Interceptor.attach(GetDeny, {
  onEnter(args) {
    const ra = this.returnAddress;
    this.inGST = ra.compare(GST_LO) >= 0 && ra.compare(GST_HI) < 0;
    if (this.inGST) {
      this.mid = args[0].toInt32() >>> 0;
      this.outpp = args[2];
      this.ra = ra;
    }
  },
  onLeave(retval) {
    if (!this.inGST) return;
    if ((retval.toInt32() & 0xff) !== 0) return;   // only SUCCESS
    try {
      const sprite = this.outpp.readPointer();
      if (sprite.isNull()) return;
      send({
        tag: 'MEMBER_POS',
        id: this.mid,
        pos: [sprite.add(M_POS).readS32(), sprite.add(M_POS + 4).readS32()],
        ra: this.ra.toString(),
      });
    } catch (e) { send({ tag: 'MEMBER_POS_ERR', err: '' + e }); }
  }
});

send({ tag: 'ready' });
"""


def main():
    attach = "--attach" in sys.argv
    open(LOG, "w").close()

    def on_message(message, data):
        if message["type"] == "send":
            line = json.dumps(message["payload"])
        else:
            line = "ERROR " + json.dumps(message)
        print(line, flush=True)
        with open(LOG, "a") as f:
            f.write(line + "\n")

    if attach:
        session = frida.attach("IWD2.exe")
        pid = None
        print("[*] attached to running IWD2.exe", flush=True)
    else:
        pid = frida.spawn(EXE, cwd=GAME_DIR)
        session = frida.attach(pid)
        print(f"[*] spawned IWD2.exe pid={pid}", flush=True)

    script = session.create_script(JS)
    script.on("message", on_message)
    script.load()
    if pid is not None:
        frida.resume(pid)

    print("[*] hooks live. In-game: load dock save, click formation button + ground.", flush=True)
    print(f"[*] logging to {LOG}", flush=True)
    sys.stdout.flush()
    # Stay alive so the hooks persist while the user plays.
    import threading
    threading.Event().wait()


if __name__ == "__main__":
    main()
