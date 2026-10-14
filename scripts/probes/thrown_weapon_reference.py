"""Native thrown-leaf/factory defaults and DWORD projectile immunity lookup.

The original remains suspended; base/resource callbacks reuse the acid probe.
Native constructors, dispatch, palette loop/copy and immunity lookup execute.
No original flight or rasterization claim.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
import acid_blob_reference as acid
from projectile_aim_reference import MD5,capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO,game_directory

prefix=acid.JS[:acid.JS.index('    for(const type of')]
prefix=prefix.replace("(p,v)=>ref(p,null,v)","(p,v)=>{current.animation=v.readCString();return ref(p,null,v);}")
prefix=prefix.replace("color:pal.add(0x1c).readU8(),", "range4:pal.add(0x20).readU8(),range5:pal.add(0x21).readU8(),color:pal.add(0x1c).readU8(),")
JS=prefix+r"""
    for(const type of [7,10,11,27,30,31,56,59,60,375]) for(const delay of [0,255]) {
      current={kind:'factory',type,delay,pixels:[],demands:0,releases:0};
      const p=decode(type,ptr(0),delay);Object.assign(current,inspect(p));events.push(current);
    }
    for(const address of [0x531570,0x531be0,0x531df0,0x532000]) {
      current={kind:'ctor',address,pixels:[],demands:0,releases:0};
      const ctor=new NativeFunction(ptr(address),'pointer',['pointer'],'thiscall');
      const p=Memory.alloc(0x2a0);ctor(p);Object.assign(current,inspect(p));events.push(current);
    }
    const lookup=new NativeFunction(ptr(0x4e7120),'int',['pointer','uint'],'thiscall');
    for(const [values,query] of [[[],0],[[0],0],[[7],7],[[7],8],
      [[1,0,0xffffffff,0x10000],0],[[1,0,0xffffffff,0x10000],0xffffffff],
      [[1,0,0xffffffff,0x10000],0x10000],[[1,0,0xffffffff,0x10000],1],
      [[7,7,8],8],[[7,7,8],9],[[0x10007],7],[[0x10007],0x10007]]) {
      const list=Memory.alloc(28),nodes=Memory.alloc(Math.max(1,values.length)*12);
      list.add(4).writePointer(values.length?nodes:ptr(0));
      for(let i=0;i<values.length;i++) {
        const node=nodes.add(i*12);node.writePointer(i+1<values.length?node.add(12):ptr(0));
        node.add(8).writeU32(values[i]);
      }
      events.push({kind:'immunity',values,query,result:lookup(list,query)});
    }
  } catch(e) {error=String(e);}
  finally {global.writePointer(old);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:global.readPointer().equals(old),hooksRestored:addresses.every((p,i)=>
    Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events):
    lines=['// Native thrown-leaf/factory and DWORD immunity capture; base/resources isolated.',
           '// MD5: '+MD5,'#ifndef THROWN_WEAPON_GOLDEN_H_','#define THROWN_WEAPON_GOLDEN_H_',
           'struct ThrownWeaponFactoryGolden { USHORT type; BYTE delay; const char* cell; SHORT speed; BOOL colored; };',
           'static const ThrownWeaponFactoryGolden THROWN_WEAPON_FACTORY[] = {']
    for e in events:
        if e['kind']=='factory':
            lines.append(f"    {{{e['type']},{e['delay']},\"{e['animation']}\",{e['speed']},{int(e['shared'])}}},")
    lines += ['};','struct ProjectileImmunityGolden { DWORD values[4]; INT count; DWORD query; BOOL found; };',
              'static const ProjectileImmunityGolden PROJECTILE_IMMUNITY[] = {']
    for e in events:
        if e['kind']=='immunity':
            values=','.join(str(v)+'u' for v in e['values']+[0]*(4-len(e['values'])))
            lines.append(f"    {{{{{values}}},{len(e['values'])},{e['query']}u,{e['result']}}},")
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0]);ap.add_argument('--out',type=Path,required=True)
    ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(acid.__file__),script)
    hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,45);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored') and len(events)==36
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        if e['kind']=='immunity':
            good=good and e['result']==int(e['query'] in e['values']);continue
        cell=e['animation'];vt,speed={'BOULDER':('0x84e1d4',30),'AXE':('0x84e36c',40),
            'SPEAR':('0x84e3f4',40),'DAGGER':('0x84e47c',40)}[cell]
        good=good and e['vtable']==vt and e['speed']==speed and e['sequence']==0 and e['tint']==1
        good=good and [e['offset'],e['mirror'],e['shadow'],e['dirs'],e['callback'],e['attached']]==[0,0,0,16,-1,-1]
        good=good and e['demands']==e['releases']==(2 if cell=='DAGGER' else 0) and e['fire']==[0]*8
        good=good and e['shared']==(cell=='DAGGER')
        if cell=='DAGGER':
            good=good and [e['range4'],e['range5']]==[30,27]
            good=good and e['pixels']==[[x,y,1] for y in (27,30) for x in range(12)]
        if e['kind']=='factory':
            expected='AXE' if e['type'] in (7,10,11) else 'DAGGER' if e['type'] in (27,30,31) else 'SPEAR' if e['type'] in (56,59,60) else 'BOULDER'
            good=good and cell==expected and e['height']==1 and e['stamped']==e['type']-1 and e['arrival']==[0]*8 and e['size']==0x2a0
        else:good=good and e['height']==0
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),original_md5=MD5,fingerprints=hashes,
        scope='Four native leaf constructors, twenty factory descriptors, real palette copy/loop and twelve DWORD list lookups; base/resources isolated')
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
