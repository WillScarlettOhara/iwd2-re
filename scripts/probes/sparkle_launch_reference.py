"""Trace factory-2F expiry in the canonical original, in a disposable loaded game.

No effects are attached and no save is written. Factory, virtual Fire and ordinary
area ticks are native; only intro movies and the load driver are instrumented.
The original offsets must never be used on the reconstructed Debug executable.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import frida
from arc_profile import ProfileSession, load_profile
from frida_orig import Trace, build_js
from runtime_paths import REPO, game_directory


JS = r"""
const wantedSave = '000000037-testSpells';
let selectedSave = false, worldReady = false, launched = false;
const tracked = new Map();
const getShare = new NativeFunction(ptr(0x599a50), 'uint8',
  ['pointer', 'int', 'uint8', 'pointer', 'uint'], 'thiscall');
const factory = new NativeFunction(ptr(0x51eaf0), 'pointer',
  ['uint16', 'pointer', 'uint8'], 'mscdecl');
function state(p) {
  return { id: p.add(0x5c).readS32(), type: p.add(0x6e).readU16(),
    velocity: p.add(0x70).readS16(), lifetime: p.add(0x29e).readS16(),
    distanceLifetime: p.add(0x29d).readU8(), trail: p.add(0xe2).readS32(),
    x: p.add(6).readS32(), y: p.add(10).readS32(), z: p.add(14).readS32(),
    targetX: p.add(0xc8).readS32(), targetY: p.add(0xcc).readS32(),
    metric: p.add(0x170).readS32(), vtable: p.readPointer().toString() };
}
function event(p, phase) {
  const entry = tracked.get(p.toString());
  if (entry) send({ tag: 'sparkle-reference', case: entry, phase: phase, state: state(p) });
}
// Resolve the exact name in the original engine's own array. LoadGame reads
// top + visible row at 63BF0D and the array pointer at +1D6 (pack(2)).
function loadExactSave(load) {
    const count = load.add(0x1ca).readS32();
    const slots = load.add(0x1d6).readPointer();
    for (let i = 0; i < count && i < 1000; i++) {
      const slot = slots.add(i * 4).readPointer();
      // Save names are Windows ANSI; unrelated French names are not UTF-8.
      const name = slot.isNull() ? ptr(0) : slot.readPointer();
      let matches = !name.isNull();
      for (let j = 0; matches && j <= wantedSave.length; j++)
        matches = name.add(j).readU8() === (j === wantedSave.length ? 0 : wantedSave.charCodeAt(j));
      if (matches) {
        load.add(0x1c6).writeS32(Math.max(0, Math.min(i, count - 5)));
        const row = i - load.add(0x1c6).readS32();
        selectedSave = true;
        send({ tag: 'selected-save', name: wantedSave, index: i, row: row });
        LoadGame(load, row);
        return;
      }
    }
    throw new Error('Exact save missing from original load screen');
}
Interceptor.attach(ptr(0x6869c0), { onLeave() { worldReady = true; } });
Interceptor.attach(ptr(0x52b900), { onEnter() { event(this.context.ecx, 'ai'); } });
Interceptor.attach(ptr(0x529fb0), { onEnter() { event(this.context.ecx, 'arrival'); } });
Interceptor.attach(ptr(0x52cbc0), {
  onEnter() {
    event(this.context.ecx, 'destructor');
    tracked.delete(this.context.ecx.toString());
  }
});
Interceptor.attach(ptr(0x5551b0), {
  onEnter() {
    const p = this.context.ecx;
    event(p, 'remove');
    tracked.delete(p.toString()); // Never inspect freed memory onLeave.
  }
});
// Launch after a complete world tick, on the engine thread. No synthetic AI
// call or clock adjustment: ordinary area registration/ticks own retirement.
Interceptor.attach(ptr(0x68c3d0), {
  onLeave() {
    if (!worldReady || !selectedSave || launched) return;
    launched = true;
    try {
      const game = ptr(0x8cf6dc).readPointer().add(0x1c54).readPointer();
      const objects = game.add(0x372c), source = game.add(0x3816).readS32();
      const out = Memory.alloc(4);
      if (getShare(objects, source, 0, out, 0xffffffff) !== 0)
        throw new Error('Party source unavailable');
      const actor = out.readPointer(), area = actor.add(0x12).readPointer();
      if (area.isNull()) throw new Error('Party area unavailable');
      const x = actor.add(6).readS32(), y = actor.add(10).readS32();
      const cases = [ ['sparkle-far', 0x2f, -1, x + 300, y],
        ['sparkle-near', 0x2f, -1, x, y],
        ['sparkle-self', 0x2f, source, x + 300, y],
        ['arrow-control', 2, -1, x + 300, y] ];
      for (const c of cases) {
        const p = factory(c[1], actor, 0);
        if (p.isNull()) throw new Error('Factory returned null');
        tracked.set(p.toString(), c[0]);
        event(p, 'factory');
        const fire = new NativeFunction(p.readPointer().add(0x6c).readPointer(),
          'void', ['pointer','pointer','int','int','int','int','int','int16'], 'thiscall');
        fire(p, area, source, c[2], c[3], c[4], 0, 0);
        event(p, 'fire');
      }
      send({ tag: 'reference-launched', cases: cases.length });
    } catch (e) { send({ tag: 'reference-error', error: String(e) }); }
  }
});
"""


class ReferenceTrace(Trace):
    def __init__(self, out):
        super().__init__(out / "trace.jsonl", out / "trace.status")
        self.records = []

    def on_message(self, message, data):
        super().on_message(message, data)
        if message["type"] == "send":
            payload = message["payload"]
            self.records.append(payload)
            if payload.get("tag") == "reference-error":
                self.errors.append(payload)

    def check_cases(self):
        checks = {}
        for name in ("sparkle-far", "sparkle-near", "sparkle-self", "arrow-control"):
            events = [r for r in self.records if r.get("case") == name]
            states = {r["phase"]: r["state"] for r in events}
            checks[name + "-launched"] = all(p in states for p in ("factory", "fire", "ai"))
            if name.startswith("sparkle"):
                checks[name + "-native-zero-lifetime"] = all(
                    states.get(p, {}).get("lifetime") == 0 and
                    states.get(p, {}).get("distanceLifetime") == 0 for p in ("factory", "fire", "ai"))
                if name == "sparkle-far":
                    checks[name + "-expires-before-arrival"] = (
                        "arrival" not in states and states.get("remove", {}).get("lifetime") == -1)
                else:
                    checks[name + "-arrives-before-expiry"] = (
                        states.get("arrival", {}).get("lifetime") == 0 and "destructor" in states)
            else:
                checks[name + "-survives-first-tick"] = (
                    states.get("factory", {}).get("lifetime") == 32767 and
                    len([r for r in events if r["phase"] == "ai"]) >= 2)
        checks["exact-save"] = any(r.get("tag") == "selected-save" for r in self.records)
        self.record({"tag": "reference-checks", "checks": checks})
        for name, passed in checks.items():
            self.say(f"CHECK {name}: {'PASS' if passed else 'FAIL'}")
        if not all(checks.values()):
            self.errors.append({"checks": checks})
        return checks


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", required=True, type=Path)
    ap.add_argument("--timeout", type=float, default=180)
    args = ap.parse_args()
    game = game_directory()
    exe = game / "IWD2.exe"
    if hashlib.md5(exe.read_bytes()).hexdigest() != "25cb3d8aa2ec648e94e328d8c467973e":
        raise SystemExit("Canonical original MD5 mismatch")
    if any(p.name.lower() in ("iwd2.exe", "iwd2-re.exe") for p in frida.get_local_device().enumerate_processes()):
        raise SystemExit("Close the running game before this disposable trace")
    args.out.mkdir(parents=True, exist_ok=False)
    profile = load_profile(REPO / "scripts/profiles/combat-messages.json", REPO)
    profile["fixtures"] = []
    spec = {"hooks": [{"addr": "0x52b900", "name": "TravellingAI", "conv": "thiscall", "max": 1}]}
    trace = ReferenceTrace(args.out)
    pid, session = None, None
    checks, rc = {}, 2
    try:
        with ProfileSession(profile, REPO, game, args.out) as transaction:
            transaction.start()
            try:
                pid = frida.spawn(str(exe), cwd=str(game))
                session = frida.attach(pid)
                session.on("detached", lambda *_: setattr(trace, "detached", True))
                driver = build_js(spec, 0, 10).replace(
                    "LoadGame(this.thiz, LOAD_SLOT);", "loadExactSave(this.thiz);")
                script = session.create_script(driver + JS)
                script.on("message", trace.on_message)
                script.load()
                frida.resume(pid)
                trace.say(f"ARMED pid={pid}")
                deadline, loaded_deadline = time.monotonic() + args.timeout, None
                while time.monotonic() < deadline and not trace.crash and not trace.detached and not trace.errors:
                    if trace.loaded and loaded_deadline is None:
                        loaded_deadline = time.monotonic() + 8
                    if loaded_deadline is not None and time.monotonic() >= loaded_deadline:
                        break
                    time.sleep(0.5)
                checks = trace.check_cases()
                rc = trace.finish(drove=True, hit="TravellingAI", hit_min=1)
            finally:
                if pid is not None:
                    try:
                        frida.kill(pid)
                    except frida.ProcessNotFoundError:
                        pass
                if session is not None:
                    session.detach()
        report = {"schema": 1, "original_md5": hashlib.md5(exe.read_bytes()).hexdigest(),
                  "checks": checks, "result": "PASS" if rc == 0 else "FAIL",
                  "restored": transaction.state["restored"]}
        (args.out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        return rc
    finally:
        trace.out.close()
        trace.status.close()


if __name__ == "__main__":
    raise SystemExit(main())
