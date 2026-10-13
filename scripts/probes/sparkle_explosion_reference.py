"""Capture original sparkle-explosion factory descriptors and virtual dispatch.

Original remains suspended. Constructors, resource strings, allocation and
palette services are isolated; native factory field writes and Explode execute.
This does not establish BAM rendering, resource lifetime or damage parity.
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

JS=r"""
rpc.exports.run=function() {
  const replacements=[], callbacks=[], memory=[], before=[], events=[];
  let current=null,error=null;
  function replace(address,ret,args,abi,body) {
    const p=ptr(address), f=new NativeCallback(body,ret,args,abi);
    before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));
    callbacks.push(f); replacements.push(p); Interceptor.replace(p,f);
  }
  function bytes(p) {return Array.from(new Uint8Array(p.readByteArray(8)));}
  function resref(p) {return String.fromCharCode(...bytes(p).filter(b=>b!==0));}
  function assign(self,out,value) {
    const b=Array.from(value.readCString().slice(0,8)).map(c=>c.charCodeAt(0));
    while(b.length<8)b.push(0);self.writeByteArray(b);out.writeByteArray(b);return out;
  }
  try {
    if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000)))throw Error('Wrong image');
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',size=>{
      const p=Memory.alloc(size);memory.push(p);current.size=size;return p;
    });
    replace(0x586a40,'pointer',['pointer'],'thiscall',p=>p);
    replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
    for(const address of [0x7fcc88,0x7fcd57])
      replace(address,'pointer',['pointer','pointer'],'thiscall',(self,value)=>{self.writePointer(value);return self;});
    replace(0x78a990,'pointer',['pointer','pointer'],'thiscall',(self,value)=>{
      const b=Array.from(value.readCString().slice(0,8)).map(c=>c.charCodeAt(0));while(b.length<8)b.push(0);
      self.writeByteArray(b);return self;
    });
    replace(0x78ad40,'pointer',['pointer','pointer','pointer'],'thiscall',assign);
    replace(0x529660,'void',['pointer','pointer'],'thiscall',(self,value)=>{current.palette=value.readCString();});
    replace(0x5297b0,'pointer',['pointer','uint','uint'],'thiscall',(self,a,b)=>{
      const value=Memory.alloc(8);memory.push(value);value.writeU32(a);value.add(4).writeU32(b);
      current.kind='sparkle';current.cell=resref(value);current.resWords=[a,b];
      self.add(0xe6).writeU32(0x12340000);return self;
    });
    replace(0x535f20,'pointer',['pointer','pointer','pointer','pointer','int','uchar'],'thiscall',(self,visual,sound,palette,flags,height)=>{
      current.kind='impact';current.cell=visual.readPointer().readCString();
      current.sound=sound.readPointer().readCString();current.palette=palette.readPointer().readCString();
      current.flags=flags;current.height=height;return self;
    });
    Interceptor.flush();
    const decode=new NativeFunction(ptr(0x51eaf0),'pointer',['uint16','pointer','uchar'],'mscdecl');
    for(const type of [0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0xb6,0xb7,
                      0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb]) {
      current={type,palette:''};const object=decode(type,ptr(0),0);
      current.fire=resref(object.add(0x152));current.arrival=resref(object.add(0x15e));
      current.stamped=object.add(0x6e).readU16();
      if(current.kind==='sparkle') {
        current.color=object.add(0x9a).readS16();current.child=object.add(0x2a4).readS16();
        current.flags=object.add(0xe6).readU32();current.trail=object.add(0xe2).readS32();current.height=object.add(0x16a).readU8();
      }
      events.push(current);
    }
    const object=Memory.alloc(0x4b2),child=Memory.alloc(0x192),vt=Memory.alloc(0x90);
    child.writePointer(vt);
    const fire=new NativeCallback((self,area,source,target,x,y,height,type)=>{
      current.fireArgs=[source,target,x,y,height,type];current.area=area.toString();
    },'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');callbacks.push(fire);vt.add(0x6c).writePointer(fire);
    replace(0x51eaf0,'pointer',['uint16','pointer','uchar'],'mscdecl',(type,caster,delay)=>{
      current.decoded=[type,caster.toString(),delay];return current.nullChild?ptr(0):child;
    });Interceptor.flush();
    const explode=new NativeFunction(ptr(0x52e160),'void',['pointer'],'thiscall');
    for(const color of [-32768,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,32767]) {
      current={color,nullChild:false};object.add(0x9a).writeS16(color);
      object.add(0x72).writeS32(-123);object.add(0x76).writeS32(456);
      object.add(0xc8).writeS32(-2147483648);object.add(0xcc).writeS32(2147483647);
      object.add(0xea).writePointer(ptr(0x12345678));explode(object);events.push(current);
    }
    current={color:2,nullChild:true};object.add(0x9a).writeS16(2);explode(object);events.push(current);
  } catch(e) {error=String(e);}
  finally {for(const p of replacements)Interceptor.revert(p);Interceptor.flush();}
  const hooksRestored=replacements.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i]);
  return {events,error,hooksRestored};
};
"""


def header(events):
    quote=lambda s: json.dumps(s)
    lines=['// Canonical native factory descriptors and Explode dispatch; services isolated.',
           '// MD5: '+MD5,'#ifndef SPARKLE_EXPLOSION_GOLDEN_H_','#define SPARKLE_EXPLOSION_GOLDEN_H_',
           'struct SparkleExplosionFactoryGolden { USHORT type; BOOL sparkle; const char *cell, *palette, *fire, *arrival; SHORT color, child; };',
           'static const SparkleExplosionFactoryGolden SPARKLE_EXPLOSION_FACTORY[] = {']
    for e in events[:19]:
        lines.append('    {'+str(e['type'])+','+('TRUE' if e['kind']=='sparkle' else 'FALSE')+','+
                     ','.join(quote(e[k]) for k in ('cell','palette','fire','arrival'))+
                     ','+str(e.get('color',0))+','+str(e.get('child',0))+'},')
    lines += ['};','struct SparkleExplosionDispatchGolden { SHORT color; USHORT type; };',
              'static const SparkleExplosionDispatchGolden SPARKLE_EXPLOSION_DISPATCH[] = {']
    for e in events[19:-1]:
        lines.append('    {'+str(e['color'])+','+str(e.get('decoded',[0])[0])+'},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0]);ap.add_argument('--out',type=Path,required=True)
    ap.add_argument('--header',type=Path)
    args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,45);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and len(events)==37
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    if good:
        types=[0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0xb6,0xb7,*range(0xc4,0xcc)]
        good=all(e['type']==t and e['stamped']==t-1 and e['cell']==('TRAVEL' if i<11 else 'SHAREA')
                 and e['kind']==('sparkle' if i<11 else 'impact') and e['size']==(0x4b2 if i<11 else 0x32c)
                 and (e['flags']==0x12340008 and e['trail']==1 and e['height']==1 if i<11 else
                      e['flags']==8 and e['height']==0 and e['sound']=='' and e['fire']==e['arrival']=='')
                 for i,(e,t) in enumerate(zip(events[:19],types)))
        good=good and [e['color'] for e in events[19:-1]]==[-32768,-1,0,*range(1,14),32767]
        for e in events[19:-1]:
            good=good and (('decoded' in e)==(1<=e['color']<=12))
            if 'decoded' in e:
                good=good and 0xc4<=e['decoded'][0]<=0xcb and e['decoded'][1:]==['0x0',0]
                good=good and e['area']=='0x12345678' and e['fireArgs']==[-123,456,-2**31,2**31-1,30,e['color']]
        good=good and events[-1]==dict(color=2,nullChild=True,decoded=[0xc5,'0x0',0])
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),original_md5=MD5,fingerprints=hashes,
                scope='Native factory field writes and Explode arguments; constructors/resources/child Fire isolated')
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
