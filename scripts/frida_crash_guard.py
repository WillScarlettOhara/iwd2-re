#!/usr/bin/env python3
"""Crash oracle for OUR build: arm a Frida exception handler on the live game so a
silent cast-time crash (window vanishes = unhandled access violation) is caught
with a symbolized EBP backtrace BEFORE the process dies.

This is the permanent promotion of the throwaway tmp_crash_catch.py that caught the
two Fireball crashes (abort-on-cast 11ef54f6, UAF 0xDDDDDDDD 1ac84b92). The lesson
those bugs taught: parity GREEN + a Frida trace on the *original* is not enough --
nothing exercised our recovered code on our own build, so cast-time faults shipped
as "done". `vm.sh smoke` arms this guard while you drive the recovered path.

Channel: print to stdout. Run as a host-side background ssh child
(`ssh win11vm python scripts/frida_crash_guard.py <pid>`); its stdout returns over
ssh reliably -- unlike the vm.sh-frida VBS payload, whose VM-side file writes are
dropped once the parent exits. Attach BY PID (from the running iwd2-re.exe), not by
spawning ptr(0x400000) hooks. Stays alive until the crash (or until killed).

  python scripts/frida_crash_guard.py <pid|name>     # default target: iwd2-re.exe
  python scripts/frida_crash_guard.py <pid> --count CGameSprite::Render

--count also answers the OTHER half of the question. "No fault for 90s" does not
distinguish "the recovered code is correct" from "the recovered code never ran",
and that gap is exactly what let both Fireball crashes ship past parity GREEN.
Counting entries to the recovered symbol turns an idle smoke into a real gate,
and it needs no source change -- we are already attached and already symbolizing.

Exit / output contract (parsed by vm.sh smoke):
  "ARMED"                  -> handler installed, safe to drive the feature
  "########## EXCEPTION ..."-> a fault fired; the block that follows is the backtrace
  "########## DETACHED"    -> target went away (crash death or normal close)
  "INSTRUMENTED <sym> <n>" -> --count resolved <sym> to n address(es)
  "NOT_INSTRUMENTED <sym>" -> --count resolved <sym> to nothing (inlined? folded
                              by /OPT:ICF? wrong name?) -- a caller MUST treat
                              this as a failure, never as a silent pass
  "HITS <sym> <n>"         -> entry count, re-emitted every 2s. Periodic, not
                              on teardown, because vm.sh kills this process and
                              a teardown handler would never run.
"""
import frida, sys, os, time, json

CRASH_TAG = "########## EXCEPTION"
DETACH_TAG = "########## DETACHED"


def out(s):
    print(s, flush=True)   # -> host-side background-task stdout (the reliable channel over ssh)


# The exception-handler + EBP-walk JS is kept verbatim from tmp_crash_catch.py: it is
# the version proven to capture both Fireball faults. Do not "improve" without a repro.
JS = r"""
function sym(a){
  try { var d = DebugSymbol.fromAddress(a);
    var s = (d.moduleName||"?") + "!" + (d.name||"?");
    if (d.fileName) s += "  [" + d.fileName + ":" + d.lineNumber + "]";
    return a + "  " + s;
  } catch(e){ return a + "  <sym err>"; }
}
function ebpwalk(ctx){
  var out = [], ebp = ctx.ebp, sp = ctx.sp;
  out.push("  EIP  " + sym(ctx.pc));
  for (var i = 0; i < 80; i++){
    if (ebp.compare(sp) < 0 || ebp.compare(sp.add(0x80000)) > 0) break;
    var ret, nxt;
    try { ret = ptr(ebp.add(4).readU32()); nxt = ptr(ebp.readU32()); } catch(e){ break; }
    out.push("  ret  " + sym(ret));
    if (nxt.compare(ebp) <= 0) break;
    ebp = nxt;
  }
  return out.join("\n");
}
Process.setExceptionHandler(function(d){
  // Ignore benign, continuable exceptions Frida reports as 'system': chiefly
  // DBG_PRINTEXCEPTION_C raised by OutputDebugString (our Iwd2DebugLog) and the
  // MS_VC thread-name exception. They are NOT crashes -- reporting them aborts the
  // smoke at the first debug-log line (e.g. the CAST marker) before the real fault.
  // Real faults are access-violation / breakpoint / illegal-instruction / etc.
  if (d.type === 'system') { return false; }   // pass through, don't report
  var hdr = "%TAG% " + d.type + " @ " + d.address;
  if (d.memory) hdr += "  mem " + d.memory.operation + " " + d.memory.address;
  hdr += " ##########";
  send({tag:"CRASH", body: hdr + "\n" + ebpwalk(d.context)});
  Thread.sleep(2);   // let the host receive the backtrace before we let it die
  return false;      // do NOT swallow -> process still crashes (faithful)
});
""".replace("%TAG%", CRASH_TAG)

# Entry counters for --count. Kept separate from the crash JS above, which is
# verbatim from the version proven to catch both Fireball faults.
COUNT_JS = r"""
var WANTED = %SYMS%;
var counts = {};

function resolve(name){
  var hits = [];
  try { hits = DebugSymbol.findFunctionsNamed(name); } catch(e){}
  if (!hits.length) {
    try { hits = DebugSymbol.findFunctionsMatching("*" + name); } catch(e){}
  }
  // Keep only addresses inside our own module: a same-named symbol pulled in
  // from a CRT or system DLL would inflate the count into a false pass.
  var self = Process.enumerateModules()[0];
  var lo = self.base, hi = self.base.add(self.size);
  return hits.filter(function(a){ return a.compare(lo) >= 0 && a.compare(hi) < 0; });
}

WANTED.forEach(function(name){
  var addrs = resolve(name);
  if (!addrs.length) {
    send({tag:"NOSYM", name:name});
    return;
  }
  counts[name] = 0;
  var attached = 0;
  addrs.forEach(function(a){
    try {
      Interceptor.attach(a, { onEnter: function(){ counts[name]++; } });
      attached++;
    } catch(e){ }
  });
  if (attached) send({tag:"SYM", name:name, n:attached});
  else send({tag:"NOSYM", name:name});
});

// Re-emit on a timer rather than at teardown: vm.sh kills this process, so an
// onDetach/atexit report would never be written.
setInterval(function(){
  Object.keys(counts).forEach(function(k){
    send({tag:"HITS", name:k, n:counts[k]});
  });
}, 2000);
rpc.exports.getcounts = function(){ return counts; };
"""


def on_message(msg, data):
    try:
        if msg.get("type") == "send" and isinstance(msg.get("payload"), dict):
            p = msg["payload"]
            tag = p.get("tag")
            if tag == "CRASH":
                out(p["body"])
                out("")
            elif tag == "SYM":
                out("INSTRUMENTED %s %d" % (p["name"], p["n"]))
            elif tag == "NOSYM":
                out("NOT_INSTRUMENTED %s" % p["name"])
            elif tag == "HITS":
                out("HITS %s %d" % (p["name"], p["n"]))
        else:
            out("[msg] " + repr(msg))
    except Exception as e:
        out("[on_message err] " + repr(e))


def main():
    argv = sys.argv[1:]
    symbols = []
    if "--count" in argv:
        i = argv.index("--count")
        symbols = [s for s in argv[i + 1].split(",") if s.strip()]
        del argv[i:i + 2]
    target = argv[0] if argv else "iwd2-re.exe"
    out("=== attaching to %s, arming exception handler ===" % target)
    try:
        session = frida.attach(int(target) if target.isdigit() else target)
    except Exception as e:
        out("ATTACH_FAILED: " + repr(e))
        return 2
    # When the target dies (crash or normal close) the host loop needs an end marker.
    session.on("detached", lambda *_: (out(DETACH_TAG), os._exit(0)))
    script = session.create_script(JS)
    script.on("message", on_message)
    script.load()
    if symbols:
        counter = session.create_script(
            COUNT_JS.replace("%SYMS%", json.dumps(symbols)))
        counter.on("message", on_message)
        counter.load()
    out("ARMED; drive the recovered path now (cast the spell). Waiting for a crash...")
    while True:
        time.sleep(0.5)


if __name__ == "__main__":
    raise SystemExit(main())
