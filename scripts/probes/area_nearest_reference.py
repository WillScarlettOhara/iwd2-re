"""Capture native area selection with isolated object/LOS/type services.

The canonical process stays suspended. Native list traversal, scalar filters,
distance arithmetic and mode gates execute; service callbacks are not parity.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from projectile_aim_reference import MD5, capture
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO, game_directory

JS = r"""
const inputs=__INPUTS__;
rpc.exports.run=function() {
  if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000))) throw Error('Wrong image');
  const area=Memory.alloc(0xb50), game=Memory.alloc(0x3800), type=Memory.alloc(60);
  const vt=Memory.alloc(0x28), avt=Memory.alloc(0xc0), anim=Memory.alloc(4);
  const objects=Array.from({length:4},()=>Memory.alloc(0x5200));
  const nodes=Array.from({length:5},()=>Memory.alloc(12));
  const callbacks=[], hooks=[], events=[]; let c, shares, locations;
  function cb(ret,args,fn) {const f=new NativeCallback(fn,ret,args,'thiscall'); callbacks.push(f);return f;}
  function replace(addr,ret,args,fn) {Interceptor.replace(ptr(addr),cb(ret,args,fn));hooks.push(ptr(addr));}
  function slot(p) {return objects.findIndex(o=>o.equals(p));}
  vt.add(4).writePointer(cb('uchar',['pointer'],()=>0x31));
  vt.add(0x10).writePointer(cb('pointer',['pointer'],p=>p.add(0x200)));
  vt.add(0x1c).writePointer(cb('pointer',['pointer'],p=>p.add(6)));
  vt.add(0x20).writePointer(cb('pointer',['pointer'],p=>p.add(0x100).readPointer()));
  vt.add(0x24).writePointer(cb('uchar',['pointer'],()=>0));
  avt.add(0xbc).writePointer(cb('int',['pointer'],()=>1));anim.writePointer(avt);
  try {
    replace(0x599a50,'uchar',['pointer','int','uchar','pointer','uint'],(p,id,t,out)=>{
      shares.push(id);if(id<0||id>3)return 1;out.writePointer(objects[id]);return 0;
    });
    replace(0x599e70,'uchar',['pointer','int','uchar','uint'],()=>0);
    replace(0x40ae80,'pointer',['pointer',...Array(11).fill('int')],p=>{p.writeByteArray(new Uint8Array(60));return p;});
    replace(0x40b6e0,'void',['pointer','pointer'],()=>{});
    replace(0x7fcc1a,'void',['pointer'],()=>{});
    replace(0x40afe0,'uchar',['pointer','pointer','int','int'],()=>1);
    replace(0x40d920,'int',['pointer','pointer'],(p,point)=>{locations.push([point.readS32(),point.add(4).readS32()]);return 1;});
    replace(0x727b80,'int',['pointer','int'],()=>1);
    replace(0x4e6fb0,'int',['pointer','pointer'],()=>0);
    replace(0x5af4e0,'int16',['pointer','int'],()=>0);
    replace(0x553c00,'uchar',['pointer','int'],()=>1);
    replace(0x553bd0,'uchar',['pointer','int'],()=>1);
    replace(0x46a820,'int',['pointer','pointer','pointer','pointer','uchar'],()=>1);
    Interceptor.flush();
    const nearest=new NativeFunction(ptr(0x46b160),'int',['pointer','int','pointer','int16','pointer','int','int','int','int','int'],'thiscall');
    const point=new NativeFunction(ptr(0x46dae0),'int',['pointer','int','int','pointer','int16','pointer','int','int','int','int'],'thiscall');
    for(let i=0;i<inputs.length;++i) {
      c=inputs[i];shares=[];locations=[]; area.writeByteArray(new Uint8Array(0xb50));type.writeByteArray(new Uint8Array(60));
      area.add(0x234).writePointer(game);area.add(0x866).writeS16(128);area.add(0x868).writeS16(128);
      for(let j=0;j<4;++j) {
        const o=objects[j], a=c.actors[j];o.writeByteArray(new Uint8Array(0x5200));o.writePointer(vt);
        o.add(6).writeS32(a[0]);o.add(10).writeS32(a[1]);o.add(0x920).writeU32(a[2]);o.add(0x838).writeU8(a[3]);
        o.add(0x206).writeU8(a[4]);o.add(0x50a6).writeS32(a[5]);o.add(0x50aa).writeS32(1);o.add(0x50ae).writeS32(1);
        o.add(0x50f0).writePointer(anim);o.add(0x100).writePointer(ptr(0));
      }
      for(let j=0;j<c.order.length;++j) {
        const id=c.order[j], n=nodes[j];n.writePointer(j+1<c.order.length?nodes[j+1]:ptr(0));
        n.add(4).writePointer(j?nodes[j-1]:ptr(0));n.add(8).writeS32(id);
        if(id>=0&&id<4&&c.actors[id][6]) objects[id].add(0x100).writePointer(n);
      }
      area.add(0x996).writePointer(nodes[0]);
      const result=c.point?point(area,c.center[0],c.center[1],type,c.range,area.add(0x40a),0,1,c.mode,c.include):
        nearest(area,0,type,c.range,area.add(0x40a),0,1,c.sleep,c.mode,c.include);
      events.push({case:i,result,shares,locations});
    }
  } finally {for(const p of hooks)Interceptor.revert(p);Interceptor.flush();}
  return {events,error:null,hooksRestored:true};
};
"""

def cases():
    result=[]
    def add(point=True, mode=0, range=50, center=(128,96), order=(1,2,3), actors=None, sleep=0, include=0):
        if actors is None: actors=[[128,96,0,0,0,1,1],[130,96,0,0,0,1,1],[140,96,0,0,0,1,1],[160,96,0,0,0,1,1]]
        result.append(dict(point=point,mode=mode,range=range,center=center,order=order,actors=actors,sleep=sleep,include=include))
    for mode in (0,1,2,255):add(mode=mode);add(False,mode,order=(1,0,2,3))
    for radius in (0,1,2,12,-1,-32768,32767):add(range=radius)
    add(center=(-1,96));add(center=(128,-1))
    add(order=(1,2,3),actors=[[128,96,0,0,0,1,1],[130,96,0,0,0,1,1],[126,96,0,0,0,1,1],[160,96,0,0,0,1,1]])
    add(False,order=(1,99,0,2,3),actors=[[128,96,0,0,0,1,1],[126,96,0,0,0,1,1],[130,96,0,0,0,1,1],[160,96,0,0,0,1,1]])
    add(False,order=(1,2,0,3),actors=[[128,96,0,0,0,1,1],[126,96,0,0,0,1,1],[127,96,0,0,0,1,0],[160,96,0,0,0,1,1]])
    for point in (False,True):
        for state,stealth,race,active in ((0x800,0,0,1),(1,0,0,1),(0,1,0,1),(0,0,190,1),(0,0,0,0)):
            for sleep in (0,1):
                add(point,sleep=sleep,order=(0,1,2,3) if not point else (1,2,3),
                    actors=[[128,96,0,0,0,1,1],[130,96,state,stealth,race,active,1],[140,96,0,0,0,1,1],[160,96,0,0,0,1,1]])
    for point in (False,True):
        for include in (1,2):add(point,include=include,order=(0,1,2,3) if not point else (1,2,3))
        add(point,include=1,order=(0,1,2,3) if not point else (1,2,3),
            actors=[[128,96,0,0,0,1,1],[130,96,0x10000000,0,0,1,1],[140,96,0,0,0,1,1],[160,96,0,0,0,1,1]])
    for point in (False,True):
        add(point,range=10,order=(0,1,2,3) if not point else (1,2,3),
            actors=[[128,96,0,0,0,1,1],[128,87,0,0,0,1,1],[128,104,0,0,0,1,1],[128,105,0,0,0,1,1]])
    add(actors=[[128,96,0,0,0,1,1],[65664,96,0,0,0,1,1],[140,96,0,0,0,1,1],[160,96,0,0,0,1,1]])
    return result

def header(inputs,events):
    lines=['// Canonical native area selection; engine services isolated. MD5 '+MD5,
           '#ifndef AREA_NEAREST_GOLDEN_H_', '#define AREA_NEAREST_GOLDEN_H_',
           'struct AreaNearestCase { BOOL point; LONG center[2]; SHORT range; INT mode,sleep,include; INT order[5],count; LONG actors[4][7]; LONG result; };',
           'static const AreaNearestCase AREA_NEAREST_CASES[] = {']
    for c,e in zip(inputs,events):
        order=list(c['order'])+[99]*(5-len(c['order']))
        lines.append('    {'+str(int(c['point']))+',{'+','.join(map(str,c['center']))+'},'
            +','.join(map(str,[c['range'],c['mode'],c['sleep'],c['include']]))+',{'+','.join(map(str,order))+'},'
            +str(len(c['order']))+',{'+','.join('{'+','.join(map(str,a))+'}' for a in c['actors'])+'},'+str(e['result'])+'},')
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
    good=not result['error'] and not result['cleanup_errors'] and len(result['events'])==len(inputs) and result.get('hooksRestored')
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    (args.out/'inputs.json').write_text(json.dumps(inputs,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',inputs=len(inputs),original_md5=MD5,
                scope='List/numeric/scalar gates; type, locks, animation, visibility and LOS callbacks isolated',fingerprints=hashes)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    if good and args.header:args.header.write_text(header(inputs,result['events']),encoding='utf-8')
    print(json.dumps(report,indent=2));return 0 if good else 2

if __name__=='__main__':raise SystemExit(main())
