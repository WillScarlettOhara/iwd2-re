"""Capture native AimAtPoint states without starting the game or loading a save.

Only the ASM-reviewed numeric branch is called: no vtable reads, constructors,
area access or object-array calls, because attached ID is INVALID_INDEX. Native
offsets are used ONLY in the canonical original. A private TLS block supplies
the original CRT's rand state on the Frida worker; the TLS index is restored.
Generate golden typed-fixture data only after both original methods agree.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
import threading
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO, game_directory

MD5 = "25cb3d8aa2ec648e94e328d8c467973e"
JS = r"""
const inputs = __INPUTS__;
rpc.exports.run = function () {
  if (Process.arch !== 'ia32' || !Process.enumerateModules()[0].base.equals(ptr(0x400000)))
    throw new Error('Expected canonical Win32 image');
  if (ptr(0x85eb66).readU8() !== 10 || ptr(0x851878).readS32() !== -1)
    throw new Error('Unexpected fixed-point or invalid-ID constants');
  const kernel = Process.getModuleByName('kernel32.dll');
  const alloc = new NativeFunction(kernel.getExportByName('TlsAlloc'), 'uint', [], 'stdcall');
  const set = new NativeFunction(kernel.getExportByName('TlsSetValue'), 'int', ['uint','pointer'], 'stdcall');
  const free = new NativeFunction(kernel.getExportByName('TlsFree'), 'int', ['uint'], 'stdcall');
  const indexAddress = ptr(0x8c23f0), previous = indexAddress.readU32(), index = alloc();
  if (index === 0xffffffff) throw new Error('TlsAlloc failed');
  const tls = Memory.alloc(0x74), object = Memory.alloc(0x2ae);
  const rand = new NativeFunction(ptr(0x7e8160), 'int', [], 'mscdecl');
  const offsets = [6,10,0x9c,0xa0,0xac,0xb0,0xb4,0xb8,0xbc,0xc0,0xc8,0xcc];
  const outputOffsets = [6,10,0x9c,0xa0,0xa4,0xa8,0xac,0xb0,0xc8,0xcc];
  const events = [];
  let error = null;
  try {
    if (!set(index, tls)) throw new Error('TlsSetValue failed');
    indexAddress.writeU32(index);
    for (const address of [0x52bd20,0x578ed0]) {
      const aim = new NativeFunction(ptr(address), 'void', ['pointer','int','int'], 'thiscall');
      for (let i = 0; i < inputs.length; i++) {
        const c = inputs[i];
        object.writeByteArray(new Uint8Array(0x2ae));
        offsets.forEach((offset,j) => object.add(offset).writeS32(c.state[j]));
        object.add(0x70).writeS16(c.speed);
        object.add(0xe0).writeU16(c.decay);
        object.add(0xc4).writeS32(c.flag);
        object.add(0x182).writeS32(-1);
        tls.add(0x14).writeU32(c.seed);
        for (let tick = 0; tick < c.ticks; tick++) {
          aim(object, c.target[0], c.target[1]);
          events.push({address, case: i, tick, state: outputOffsets.map(o => object.add(o).readS32()),
            facing: object.add(0x1dc).readS16(), seed: tls.add(0x14).readU32()});
        }
        events[events.length-1].nextRandom = rand();
      }
    }
  } catch (e) { error = String(e); }
  finally {
    indexAddress.writeU32(previous);
    const cleared = set(index, ptr(0)), released = free(index);
    if (!cleared || !released) error = 'Private TLS cleanup failed';
  }
  return {events, error, tlsRestored: indexAddress.readU32() === previous};
};
"""


def capture(frida, exe, cwd, script_path, timeout):
    device=frida.get_local_device()
    cancel=frida.Cancellable()
    timer=threading.Timer(timeout,cancel.cancel)
    timer.daemon=True
    pid=session=None
    messages=[]
    result=dict(events=[],error=None,cleanup_errors=[])
    timer.start()
    try:
        with cancel:
            pid=device.spawn([str(exe)],cwd=str(cwd))
            session=device.attach(pid)
            script=session.create_script(script_path.read_text(encoding="utf-8"))
            script.on("message",lambda message,data:messages.append(message))
            script.load()
            result.update(script.exports_sync.run())
    except (Exception,KeyboardInterrupt) as error:
        result["error"]=f"{type(error).__name__}: {error}"
    finally:
        timer.cancel()
        if pid is not None:
            try:
                device.kill(pid)
            except frida.ProcessNotFoundError:
                pass
            except Exception as error:
                result["cleanup_errors"].append(f"kill PID {pid}: {error}")
        if session is not None:
            try:
                session.detach()
            except frida.InvalidOperationError:
                pass
            except Exception as error:
                result["cleanup_errors"].append(f"detach: {error}")
        result.update(pid=pid,messages=messages)
    return result


def cases():
    result = []
    def add(target, *, pos=(0, 0), accum=(0, 0), speed=10, drift=(0, 0),
            low=(0, 0), high=(0, 0), decay=0, seed=1, ticks=1, flag=0):
        result.append(dict(target=target, state=[*pos, *accum, *drift, *low, *high, -1111, -2222],
                           speed=speed, decay=decay, seed=seed, ticks=ticks, flag=flag))
    for target in ((0,0),(1,0),(-1,0),(0,1),(0,-1),(3,3),(-3,-3),(29,5),
                   (9,3),(32768,0),(65536,3),(50000,0),(0,2147483647)):
        for speed in (0,1,10,-10,-32768,32767):
            add(target, speed=speed)
    add((0,0), drift=(99,-99), low=(-20,-20), high=(20,20), decay=10, ticks=3)
    add((100,0), pos=(2147483647,2147483647), accum=(2147483647,-2147483648))
    add((100,75), accum=(2147483647,2147483647), drift=(2147483647,-2147483648))
    for decay in (0,1,10,65535):
        for drift in ((0,0),(9,-9),(10,-10),(11,-11),(65536,-65536),(-2147483648,2147483647)):
            add((100,75), drift=drift, decay=decay, ticks=3, flag=1)
    for low, high in (((-11,-23),(7,5)), ((2,3),(2,3)), ((8,9),(2,3)),
                      ((-2147483648,0),(2147483647,19)), ((2147483647,0),(-2147483648,19))):
        for seed in (1,42,0xffffffff):
            add((100,75), low=low, high=high, seed=seed, ticks=3)
    for accum in ((-1,-1),(-1025,-1367),(715827883,715827883),(-2147483648,-2147483648)):
        add((0,0), accum=accum)
    return result


def header(inputs, events):
    # Native captures are the oracle: no reconstructed arithmetic here.
    lines = ["// Generated by scripts/probes/projectile_aim_reference.py from canonical IWD2.exe.",
             "// Original MD5: " + MD5, "#ifndef PROJECTILE_AIM_GOLDEN_H_", "#define PROJECTILE_AIM_GOLDEN_H_",
             "struct ProjectileAimInput { LONG state[12]; LONG x, y; SHORT speed; USHORT decay; DWORD seed; INT ticks, flag; };",
             "struct ProjectileAimOutput { INT input, tick; LONG state[10]; SHORT facing; DWORD seed; INT nextRandom; };",
             "static const ProjectileAimInput PROJECTILE_AIM_INPUTS[] = {"]
    def signed(v):
        return "(-2147483647 - 1)" if v == -2147483648 else str(v)
    for c in inputs:
        lines.append("    {{" + ",".join(map(signed,c["state"])) + "}," +
                     ",".join(map(signed,[*c["target"],c["speed"],c["decay"]])) +
                     f",{c['seed']}u,{c['ticks']},{c['flag']}" + "},")
    lines += ["};", "static const ProjectileAimOutput PROJECTILE_AIM_OUTPUTS[] = {"]
    for e in events:
        lines.append(f"    {{{e['case']},{e['tick']},{{" + ",".join(map(signed,e["state"])) +
                     f"}},{e['facing']},{e['seed']}u,{e.get('nextRandom',-1)}" + "},")
    return "\n".join(lines + ["};", "#endif", ""])


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--header", type=Path)
    args = ap.parse_args()
    import frida
    exe = REPO / ".bin/IWD2.exe"
    if hashlib.md5(exe.read_bytes()).hexdigest() != MD5:
        raise SystemExit("Canonical MD5 mismatch")
    args.out.mkdir(parents=True, exist_ok=False)
    for address,count in (("0x52BD20",240),("0x578ED0",220),("0x7E8160",12),("0x7EDB13",42)):
        asm=subprocess.run([sys.executable,str(REPO/"scripts/sym.py"),"disasm",address,str(count)],
                           cwd=REPO,capture_output=True,text=True,check=True,timeout=30)
        (args.out/f"{address}.asm").write_text(asm.stdout,encoding="utf-8")
    inputs = cases()
    script = args.out / "original.js"
    script.write_text(JS.replace("__INPUTS__",json.dumps(inputs)), encoding="utf-8")
    paths=(exe,Path(__file__),script,REPO/"scripts/sym.py")
    fingerprints={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result = capture(frida,exe,game_directory(),script,45)
    (args.out / "capture.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
    (args.out / "inputs.json").write_text(json.dumps(inputs,indent=2),encoding="utf-8")
    events = result["events"]
    count = sum(c["ticks"] for c in inputs)
    good = not result["error"] and not result["cleanup_errors"] and result.get("tlsRestored")
    good = good and fingerprints=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    good = good and not any(m.get("type") == "error" for m in result["messages"])
    good = good and len(events) == count*2
    if good:
        order=[(i,tick) for i,c in enumerate(inputs) for tick in range(c["ticks"])]
        good=all((e.get("case"),e.get("tick"))==order[j%count]
                 and e.get("address")== (0x52bd20 if j<count else 0x578ed0)
                 and len(e.get("state",[]))==10
                 and all(type(n) is int and -2**31<=n<2**31 for n in e["state"])
                 and type(e.get("facing")) is int and 0<=e["facing"]<=15
                 and type(e.get("seed")) is int and 0<=e["seed"]<2**32
                 and ("nextRandom" in e)==(e["tick"]==inputs[e["case"]]["ticks"]-1)
                 and ("nextRandom" not in e or type(e["nextRandom"]) is int and 0<=e["nextRandom"]<=32767)
                 for j,e in enumerate(events))
        left = [{k:v for k,v in e.items() if k != "address"} for e in events[:count]]
        right = [{k:v for k,v in e.items() if k != "address"} for e in events[count:]]
        good = good and left == right
    report = dict(result="PASS" if good else "UNPROVEN", original_md5=MD5,
                  inputs=len(inputs), states_per_method=count, scope="Numeric branch only; attached ID invalid",
                  fingerprints=fingerprints)
    (args.out / "report.json").write_text(json.dumps(report,indent=2),encoding="utf-8")
    if good and args.header:
        args.header.write_text(header(inputs,events[:count]),encoding="utf-8")
    print(json.dumps(report,indent=2))
    return 0 if good else 2


if __name__ == "__main__":
    raise SystemExit(main())
