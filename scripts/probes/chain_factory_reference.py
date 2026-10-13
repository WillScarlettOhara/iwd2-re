"""Capture original chain factory dispatch and temporary empty-set ownership.

The original remains suspended. Allocation, locking, visual initialization,
CString destruction and the chain constructor are isolated callbacks. The CPU
executes DecodeProjectile's dispatch/common tail and the actual VC6 empty-set
constructor/destructor. This is not a native resource or flight oracle.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from projectile_aim_reference import MD5, capture
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO, game_directory

JS = r"""
rpc.exports.run = function () {
  if (Process.arch !== 'ia32' || !Process.enumerateModules()[0].base.equals(ptr(0x400000)))
    throw Error('Expected canonical Win32 image');
  const replacements = [], callbacks = [], allocations = [], events = [], codeBefore = [];
  const count = ptr(0x8d48b0), nil = ptr(0x8d48b4);
  const oldCount = count.readU32(), oldNil = nil.readPointer();
  let current = null, error = null;
  function replace(address, ret, args, abi, body) {
    const callback = new NativeCallback(body, ret, args, abi);
    const p = ptr(address);
    codeBefore.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));
    callbacks.push(callback); Interceptor.replace(p, callback); replacements.push(p);
  }
  try {
    count.writeU32(0); nil.writePointer(ptr(0));
    replace(0x7fc95b, 'pointer', ['uint'], 'mscdecl', size => {
      const p = Memory.alloc(size); allocations.push(p);
      current.allocated.push({pointer:p.toString(), size}); return p;
    });
    replace(0x7fc984, 'void', ['pointer'], 'mscdecl', p => current.freed.push(p.toString()));
    replace(0x7f9a11, 'pointer', ['pointer'], 'thiscall', p => p);
    replace(0x7f9aad, 'void', ['pointer'], 'thiscall', () => {});
    replace(0x586a40, 'pointer', ['pointer'], 'thiscall', p => p);
    replace(0x7fcc1a, 'void', ['pointer'], 'thiscall', () => {});
    replace(0x57c510, 'pointer', ['pointer','pointer'], 'thiscall', (self, history) => {
      const head = history.add(4).readPointer();
      current.constructorCalls++;
      current.history = {size:history.add(12).readU32(), left:head.readPointer().equals(head),
        right:head.add(8).readPointer().equals(head), parent:head.add(4).readPointer().equals(nil.readPointer())};
      self.add(0x6e).writeU16(0); return self;
    });
    Interceptor.flush();
    const decode = new NativeFunction(ptr(0x51eaf0), 'pointer', ['uint16','pointer','uchar'], 'mscdecl');
    const caster = Memory.alloc(0x100);
    for (const hasCaster of [false,true]) for (const delay of [0,17,255]) {
      current = {hasCaster,delay,allocated:[],freed:[],constructorCalls:0};
      const result = decode(0xd2, hasCaster ? caster : ptr(0), delay);
      current.type = result.add(0x6e).readU16(); current.result = result.toString();
      current.treeRestored = count.readU32() === 0 && nil.readPointer().isNull();
      events.push(current);
    }
  } catch (e) { error = String(e); }
  finally {
    count.writeU32(oldCount); nil.writePointer(oldNil);
    for (const p of replacements) Interceptor.revert(p);
    Interceptor.flush();
  }
  const hooksRestored = replacements.every((p,i) =>
    Array.from(new Uint8Array(p.readByteArray(16))).join(',') === codeBefore[i]);
  return {events,error,hooksRestored,globalsRestored:count.readU32()===oldCount && nil.readPointer().equals(oldNil)};
};
"""


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()
    import frida

    exe = REPO / ".bin/IWD2.exe"
    if hashlib.md5(exe.read_bytes()).hexdigest() != MD5:
        raise SystemExit("Canonical MD5 mismatch")
    args.out.mkdir(parents=True, exist_ok=False)
    for address, length in (("0x51EAF0", 45), ("0x5250B0", 34), ("0x537010", 55),
                            ("0x5370C0", 87), ("0x528E1C", 23)):
        asm = subprocess.run([sys.executable, str(REPO / "scripts/sym.py"), "disasm", address, str(length)],
                             cwd=REPO, capture_output=True, text=True, check=True, timeout=30)
        (args.out / (address + ".asm")).write_text(asm.stdout, encoding="utf-8")
    script = args.out / "original.js"
    script.write_text(JS, encoding="utf-8")
    paths = (exe, Path(__file__), script)
    hashes = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result = capture(frida, exe, game_directory(), script, 45)
    good = not result["error"] and not result["cleanup_errors"] and result.get("globalsRestored") and result.get("hooksRestored")
    good = good and not any(m.get("type") == "error" for m in result["messages"])
    events = result["events"]
    good = good and len(events) == 6
    for index, e in enumerate(events):
        allocated = e["allocated"]
        good = good and e["hasCaster"] == (index >= 3) and e["delay"] == (0, 17, 255)[index % 3]
        good = good and e["constructorCalls"] == 1 and e["type"] == 0xD1 and e["treeRestored"]
        good = good and e["history"] == dict(size=0, left=True, right=True, parent=True)
        good = good and [a["size"] for a in allocated] == [0x2BE, 0x14, 0x14]
        good = good and e["result"] == allocated[0]["pointer"]
        good = good and e["freed"] == [allocated[2]["pointer"], allocated[1]["pointer"]]
    good = good and hashes == {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out / "capture.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    report = dict(result="PASS" if good else "UNPROVEN", original_md5=MD5, cases=len(events),
                  scope="Factory dispatch/common tail and native empty-set lifetime; constructor/resources isolated",
                  fingerprints=hashes)
    (args.out / "report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if good else 2


if __name__ == "__main__":
    raise SystemExit(main())
