"""Capture the original beam's control flow with isolated rendering collaborators.

The canonical process stays suspended: no game/save/graphics initialization.
Only Render 0x57B780 executes unchanged. Private area/cell buffers and replacement
callbacks supply height, geometry, visibility and FX services. This proves native
anchors/masks/rejection/arithmetic; it does not prove original rasterization.
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
const inputs = __INPUTS__;
rpc.exports.run = function () {
  if (Process.arch !== 'ia32' || !Process.enumerateModules()[0].base.equals(ptr(0x400000)))
    throw new Error('Expected canonical Win32 image');
  if (ptr(0x85eb66).readU8() !== 10) throw new Error('Fixed-point constant mismatch');
  const object = Memory.alloc(0x2d6), area = Memory.alloc(0x900), cell = Memory.alloc(0x100);
  const callbacks = [], addresses = [], events = [];
  let current = null, result = null;
  const point = p => [p.readS32(), p.add(4).readS32()];
  function replace(address, ret, args, callback) {
    const f = new NativeCallback(callback, ret, args, 'thiscall');
    callbacks.push(f); addresses.push(ptr(address)); Interceptor.replace(ptr(address), f);
  }
  try {
    replace(0x470d80,'int',['pointer','pointer','uchar'],(self,p,list) => {
      result.heightCalls.push({point:point(p),list}); return current.height;
    });
    replace(0x7ad520,'int',['pointer','pointer','uchar'],(self,p,demanded) => {
      p.writeS32(1); p.add(4).writeS32(9); return 1;
    });
    replace(0x7ad800,'int',['pointer','pointer','uchar'],(self,p,demanded) => {
      p.writeS32(3); p.add(4).writeS32(18); return 1;
    });
    replace(0x553c00,'uchar',['pointer','int'],(self,tile) => {
      result.tiles.push(tile); return current.hidden.includes(tile) ? 0 : 1;
    });
    replace(0x5ce130,'int',['pointer','pointer','uint','int','pointer','pointer'],(self,r,flags,surface,p,ref) => {
      result.preps.push({point:point(p),flags,reference:point(ref),surface}); return 1;
    });
    replace(0x5ce1f0,'int',['pointer','pointer','uint'],(self,r,flags) => current.lock ? 1 : 0);
    replace(0x5ce280,'int',['pointer','pointer','int','int','uint','int'],(self,c,x,y,flags,trans) => {
      result.renders.push({reference:[x,y],flags,trans}); return 1;
    });
    replace(0x5ce350,'int',['pointer','int','int','int','pointer','pointer','int','uint'],(self,x,y,z,ref,bounds,dither,flags) => {
      result.clips.push({point:[x,y],z,flags,dither,bounds:[0,4,8,12].map(o=>bounds.add(o).readS32())}); return 1;
    });
    replace(0x5ce930,'int',['pointer','uint','pointer','pointer'],(self,flags,r,p) => {
      result.unlocks.push({flags,nullRect:r.isNull(),reference:point(p)}); return 1;
    });
    replace(0x5ce0a0,'int',['pointer','int','pointer','int','int','int','int','uint'],(self,surface,r,x,y,rx,ry,flags) => {
      result.blits.push({point:[x,y],reference:[rx,ry],flags,surface}); return 1;
    });
    Interceptor.flush();
    const render = new NativeFunction(ptr(0x57b780),'void',['pointer','pointer','pointer','int'],'thiscall');
    for (let i=0; i<inputs.length; ++i) {
      current = inputs[i];
      object.writeByteArray(new Uint8Array(0x2d6)); area.writeByteArray(new Uint8Array(0x900));
      [6,10,14,0x9c,0xa0,0xa4,0xa8,0x2b0].forEach((o,j)=>object.add(o).writeS32(current.state[j]));
      object.add(0x1a).writeU8(2); object.add(0x1da).writeS16(current.direction);
      object.add(0xea).writePointer(area); object.add(0x192).writePointer(cell); object.add(0x2af).writeU8(current.count);
      area.add(0x866).writeS16(100);
      [0x514,0x518,0x51c,0x520].forEach((o,j)=>area.add(o).writeS32([80,80,400,320][j]));
      area.add(0x55c).writeS32(current.view[0]); area.add(0x560).writeS32(current.view[1]);
      result={case:i,preps:[],renders:[],clips:[],unlocks:[],blits:[],tiles:[],heightCalls:[]};
      render(object,ptr(0),ptr(1),1); events.push(result);
    }
    return {events,error:null};
  } finally { for (const address of addresses) Interceptor.revert(address); Interceptor.flush(); }
};
"""


def cases():
    result = []
    def add(*, direction=0, count=5, pos=(232,1002,17), accum=(232*1024,1336*1024),
            step=(8*1024,0), delta=0, height=0, view=(72,882), hidden=(), lock=True):
        result.append(dict(state=[*pos,*accum,*step,delta], direction=direction,count=count,
                           height=height,view=view,hidden=list(hidden),lock=lock))
    for direction in range(16):
        add(direction=direction)
    for delta in (-2147483648,-100,-2,0,2,100,2147483647):
        add(delta=delta)
    for count in (0,1,2,127,128,255):
        add(count=count,step=(0,0))
    add(step=(8*1024,4*1024))
    add(step=(8*1024,-4*1024))
    add(pos=(232,1002,-17))
    add(count=2,accum=(232*1024,1336*1024+1),step=(8*1024,-2))
    add(hidden=(3007,))
    add(hidden=(3008,))
    add(hidden=(3107,))
    add(view=(10072,882))
    add(view=(235,882))
    add(view=(72,995))
    add(step=(200*1024,0))
    for wrap in (32*65536,-32*65536):
        add(count=1,pos=(232,1002+wrap,17),view=(72,882+wrap))
    add(height=7)
    add(height=-7)
    add(accum=(2147483647,1336*1024),step=(1,0))
    add(accum=(-2147483648,1336*1024),step=(-1,0))
    add(accum=(232*1024,715827883))
    add(accum=(232*1024,-715827883))
    add(lock=False)
    return result


def header(inputs,events):
    def n(v):
        return '(-2147483647 - 1)' if v == -2147483648 else str(v)
    lines=['// Captured from canonical Render 0x57B780 with isolated collaborators.',
           '// Original MD5: '+MD5, '#ifndef BEAM_RENDER_GOLDEN_H_', '#define BEAM_RENDER_GOLDEN_H_',
           'struct BeamRenderInput { LONG state[8]; SHORT direction; BYTE count; LONG height, view[2]; };',
           'struct BeamRenderSample { INT input; LONG x, y; DWORD flags; };',
           'static const BeamRenderInput BEAM_RENDER_INPUTS[] = {']
    selected=[(c,e) for c,e in zip(inputs,events) if c['height']==0 and not c['hidden'] and c['lock']]
    for c,e in selected:
        lines.append('    {{'+','.join(map(n,c['state']))+'},'+','.join(map(n,[c['direction'],c['count'],c['height']]))
                     +',{'+','.join(map(n,c['view']))+'}},')
    lines += ['};','static const BeamRenderSample BEAM_RENDER_SAMPLES[] = {']
    for i,(c,e) in enumerate(selected):
        for sample in e['blits']:
            lines.append('    {'+','.join(map(n,[i,*sample['point'],sample['flags']]))+'u},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--out',type=Path,required=True)
    ap.add_argument('--header',type=Path)
    args=ap.parse_args()
    import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:
        raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False)
    inputs=cases(); script=args.out/'original.js'
    script.write_text(JS.replace('__INPUTS__',json.dumps(inputs)),encoding='utf-8')
    paths=(exe,Path(__file__),script)
    hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,45)
    good=not result['error'] and not result['cleanup_errors'] and len(result['events'])==len(inputs)
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    for e,c in zip(result['events'],inputs):
        good=good and len(e['heightCalls'])==1+len(e['preps']) and len(e['tiles'])==c['count']
        good=good and len(e['renders'])==len(e['clips'])==len(e['unlocks'])==len(e['blits'])
        good=good and (len(e['blits'])==len(e['preps']) if c['lock'] else not e['blits'])
        good=good and all(r['reference']==[1,9] and r['trans']==0 for r in e['renders'])
        good=good and all(h==dict(point=c['state'][:2],list=2) for h in e['heightCalls'])
        good=good and all(u['nullRect'] and u['reference']==[0,0] for u in e['unlocks'])
    good=good and [e['case'] for e in result['events']]==list(range(len(inputs)))
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    (args.out/'inputs.json').write_text(json.dumps(inputs,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',inputs=len(inputs),original_md5=MD5,
                scope='Original renderer with isolated collaborators; no native rasterization or gameplay',fingerprints=hashes)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    if good and args.header:
        args.header.write_text(header(inputs,result['events']),encoding='utf-8')
    print(json.dumps(report,indent=2)); return 0 if good else 2


if __name__=='__main__':
    raise SystemExit(main())
