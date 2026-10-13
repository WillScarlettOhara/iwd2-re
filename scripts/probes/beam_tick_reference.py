"""Capture canonical beam AI numeric states with isolated engine collaborators.

The original stays suspended. Frame/direction/lock/start/sound callbacks replace
engine services; strike/clear periods are seeded so the STL branches are skipped.
The CPU executes the original timer gates, fixed-point and segment calculations.
This does not claim native target scanning, resource delivery or gameplay parity.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
from projectile_aim_reference import MD5, capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO, game_directory

JS=r"""
const inputs=__INPUTS__;
rpc.exports.run=function() {
  if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000))) throw Error('Wrong image');
  const kernel=Process.getModuleByName('kernel32.dll');
  const alloc=new NativeFunction(kernel.getExportByName('TlsAlloc'),'uint',[],'stdcall');
  const set=new NativeFunction(kernel.getExportByName('TlsSetValue'),'int',['uint','pointer'],'stdcall');
  const free=new NativeFunction(kernel.getExportByName('TlsFree'),'int',['uint'],'stdcall');
  const tlsIndex=ptr(0x8c23f0), oldIndex=tlsIndex.readU32(), index=alloc(), tls=Memory.alloc(0x74);
  if(index===0xffffffff) throw Error('TLS allocation failed');
  const chitin=Memory.alloc(0x2000), game=Memory.alloc(0x5000), object=Memory.alloc(0x2d6);
  const source=Memory.alloc(0x100), cell=Memory.alloc(0x100), vtable=Memory.alloc(0x90), sourceVT=Memory.alloc(0x20), cellVT=Memory.alloc(4);
  const global=ptr(0x8cf6dc), oldChitin=global.readPointer();
  const callbacks=[], replacements=[], events=[];
  let current=null, frames=0, removed=0, tlsCleared=false, tlsFreed=false;
  function callback(ret,args,fn) {const f=new NativeCallback(fn,ret,args,'thiscall'); callbacks.push(f); return f;}
  function replace(address,ret,args,fn) {const f=callback(ret,args,fn); Interceptor.replace(ptr(address),f); replacements.push(ptr(address));}
  try {
    if(!set(index,tls)) throw Error('TLS setup failed'); tlsIndex.writeU32(index);
    chitin.add(0x1c54).writePointer(game); global.writePointer(chitin);
    cellVT.writePointer(callback('void',['pointer'],()=>{++frames;})); cell.writePointer(cellVT);
    sourceVT.add(4).writePointer(callback('uchar',['pointer'],()=>0)); source.writePointer(sourceVT);
    vtable.add(0x48).writePointer(callback('void',['pointer'],()=>{++removed;}));
    replace(0x599c70,'uchar',['pointer','int','uchar','pointer','uint'],(self,id,thread,out,timeout)=>{
      if(current.missing) return 1; out.writePointer(source); return 0;
    });
    replace(0x59a010,'uchar',['pointer','int','uchar','uint'],()=>0);
    replace(0x59a530,'uchar',['pointer','int','uchar','pointer','uint'],()=>1);
    replace(0x52a480,'int16',['pointer','int','int'],()=>0);
    replace(0x579860,'void',['pointer','pointer'],()=>{});
    // GetStart is cdecl, unlike every member service above.
    const start=new NativeCallback((id,p,facing)=>{p.writeS32(current.origin[0]);p.add(4).writeS32(current.origin[1]);},'void',['int','pointer','uchar'],'mscdecl');
    callbacks.push(start); Interceptor.replace(ptr(0x52a640),start); replacements.push(ptr(0x52a640));
    replace(0x7aa5e0,'int',['pointer','int','int','int'],()=>1);
    Interceptor.flush();
    const ai=new NativeFunction(ptr(0x57b210),'void',['pointer'],'thiscall');
    for(let i=0;i<inputs.length;++i) {
      current=inputs[i]; object.writeByteArray(new Uint8Array(0x2d6)); frames=removed=0;
      object.writePointer(vtable); object.add(0x192).writePointer(cell);
      object.add(0x5c).writeS32(42); object.add(0x72).writeS32(current.missing?-1:1); object.add(0x76).writeS32(-1);
      [6,10,0x9c,0xa0,0xa4,0xa8,0x2b0].forEach((o,j)=>object.add(o).writeS32([10,20,-111,-222,-333,-444,99][j]));
      object.add(0xc8).writeS32(current.target[0]);object.add(0xcc).writeS32(current.target[1]);
      object.add(0x70).writeS16(current.speed);object.add(0x2c6).writeS32(current.tick);
      object.add(0x2ca).writeS32(3);object.add(0x2ce).writeS32(60);
      object.add(0x2ae).writeU8(current.life);object.add(0x2af).writeU8(123);
      game.add(0x4b40).writeS32(current.stop?1:0);game.add(0x4b44).writeS32(current.exempt?42:43);
      ai(object);
      events.push({case:i,state:[6,10,0x9c,0xa0,0xa4,0xa8,0x2b0,0x2c6].map(o=>object.add(o).readS32()),
        life:object.add(0x2ae).readU8(),segments:object.add(0x2af).readU8(),frames,removed});
    }
  } finally {
    global.writePointer(oldChitin);tlsIndex.writeU32(oldIndex);
    tlsCleared=set(index,ptr(0))!==0;tlsFreed=free(index)!==0;
    for(const address of replacements) Interceptor.revert(address);Interceptor.flush();
  }
  return {events,error:null,tlsRestored:tlsIndex.readU32()===oldIndex&&tlsCleared&&tlsFreed,
    chitinRestored:global.readPointer().equals(oldChitin)};
};
"""

def cases():
    result=[]
    def add(target=(256,96),origin=(128,96),speed=10,tick=1,life=110,stop=False,exempt=False,missing=False):
        result.append(dict(target=target,origin=origin,speed=speed,tick=tick,life=life,stop=stop,exempt=exempt,missing=missing))
    for target in ((128,96),(129,96),(127,96),(128,99),(128,95),(131,99),(125,93),(256,96),(1200,96),
                   (50128,96),(65664,96),(128,2147483647)):
        add(target)
    for speed in (1,-1,-10,32767): add(speed=speed)
    for origin in ((-1,-1),(-1025,-1367),(2147483647,2147483647),(-2147483648,-2147483648)):
        add((0,0),origin)
    for tick in (-1,2147483647,-2147483648): add(tick=tick)
    for life in (0,1,255): add(life=life)
    add(stop=True);add(stop=True,exempt=True);add(missing=True)
    return result

def header(inputs,events):
    def n(v): return '(-2147483647 - 1)' if v==-2147483648 else str(v)
    lines=['// Captured canonical beam AI numeric states; engine collaborators isolated.', '// MD5: '+MD5,
           '#ifndef BEAM_TICK_GOLDEN_H_', '#define BEAM_TICK_GOLDEN_H_',
           'struct BeamTickInput { LONG target[2], origin[2]; SHORT speed; LONG tick; BYTE life; BOOL stop, exempt, missing; };',
           'struct BeamTickOutput { LONG state[8]; BYTE life, segments; INT frames, removed; };',
           'static const BeamTickInput BEAM_TICK_INPUTS[] = {']
    for c in inputs:
        lines.append('    {{'+','.join(map(n,c['target']))+'},{'+','.join(map(n,c['origin']))+'},'
            +','.join(map(n,[c['speed'],c['tick'],c['life'],int(c['stop']),int(c['exempt']),int(c['missing'])]))+'},')
    lines+=['};','static const BeamTickOutput BEAM_TICK_OUTPUTS[] = {']
    for e in events:
        lines.append('    {{'+','.join(map(n,e['state']))+'},'+','.join(map(n,[e['life'],e['segments'],e['frames'],e['removed']]))+'},')
    return '\n'.join(lines+['};','#endif',''])

def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0]);ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path)
    args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);inputs=cases();script=args.out/'original.js'
    script.write_text(JS.replace('__INPUTS__',json.dumps(inputs)),encoding='utf-8')
    paths=(exe,Path(__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,45)
    good=not result['error'] and not result['cleanup_errors'] and len(result['events'])==len(inputs)
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    good=good and result.get('tlsRestored') and result.get('chitinRestored')
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    good=good and [e['case'] for e in result['events']]==list(range(len(inputs)))
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    (args.out/'inputs.json').write_text(json.dumps(inputs,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',inputs=len(inputs),original_md5=MD5,
                scope='Timer gates and numeric tail only; engine callbacks replaced; no strike/STL gameplay',
                fingerprints=hashes)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    if good and args.header:args.header.write_text(header(inputs,result['events']),encoding='utf-8')
    print(json.dumps(report,indent=2));return 0 if good else 2

if __name__=='__main__':raise SystemExit(main())
