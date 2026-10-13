"""Capture native acid-leaf construction, factory variants and SetRange order.

Original stays suspended. The travelling base/resources are isolated callbacks;
the native leaf constructor, factory branches, palette bind/copy and range loop
execute. Synthetic bitmap pixels make order and WORD narrowing observable.
This does not prove original resource loading, flight or rasterization.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
from projectile_aim_reference import MD5,capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO,game_directory

JS=r"""
rpc.exports.run=function() {
  const callbacks=[],addresses=[],before=[],memory=[],events=[];
  const global=ptr(0x8cf6dc),old=global.readPointer(),chitin=Memory.alloc(0x2000),game=Memory.alloc(0x4000);
  let current=null,error=null;
  function replace(a,ret,args,abi,fn) {
    const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);addresses.push(p);
    before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));Interceptor.replace(p,f);
  }
  function ref(self,out,value) {
    const b=Array.from(value.readCString().slice(0,8)).map(c=>c.charCodeAt(0));while(b.length<8)b.push(0);
    self.writeByteArray(b);if(out)out.writeByteArray(b);return out||self;
  }
  function inspect(p) {
    const pal=p.add(0x19a),cell=p.add(0x192).readPointer();
    return {vtable:p.readPointer().toString(),speed:p.add(0x70).readS16(),height:p.add(0x16a).readU8(),
      tint:p.add(0x1be).readS32(),offset:p.add(0x1c2).readS32(),mirror:p.add(0x1c6).readS32(),
      shadow:p.add(0x1d4).readS32(),dirs:p.add(0x1d8).readS16(),callback:p.add(0x7a).readS32(),attached:p.add(0x182).readS32(),
      color:pal.add(0x1c).readU8(),cached:cell.add(8+0x1c).readU8(),
      shared:pal.add(8).readPointer().equals(cell.add(16).readPointer()),stamped:p.add(0x6e).readU16(),
      fire:Array.from(new Uint8Array(p.add(0x152).readByteArray(8))),arrival:Array.from(new Uint8Array(p.add(0x15e).readByteArray(8)))};
  }
  try {
    if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000)))throw Error('Wrong image');
    chitin.add(0x1c54).writePointer(game);global.writePointer(chitin);
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',size=>{const p=Memory.alloc(size);memory.push(p);current.size=size;return p;});
    replace(0x586a40,'pointer',['pointer'],'thiscall',p=>p);
    replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
    replace(0x7fcd57,'pointer',['pointer','pointer'],'thiscall',(p,v)=>{p.writePointer(v);return p;});
    replace(0x78a9d0,'pointer',['pointer','pointer'],'thiscall',(p,v)=>ref(p,null,v));
    replace(0x78ad40,'pointer',['pointer','pointer','pointer'],'thiscall',ref);
    replace(0x52ad60,'pointer',['pointer','uint','uint'],'thiscall',(p,a,b)=>{
      current.cell=[a,b];const cell=Memory.alloc(0xda),colors=Memory.alloc(1024),res=Memory.alloc(0x100);
      memory.push(cell,colors,res);p.add(0x192).writePointer(cell);p.add(0x70).writeS16(20);
      p.add(0x19a+8).writePointer(colors);p.add(0x19a+12).writeS32(256);p.add(0x19a+0x14).writeU16(1);
      game.add(0x3672+0xa4).writePointer(res);p.add(0x16a).writeU8(0);
      return p;
    });
    replace(0x7ae290,'int16',['pointer','int16'],'thiscall',(p,s)=>{current.sequence=s;return s;});
    replace(0x77ec60,'pointer',['pointer'],'thiscall',p=>{++current.demands;return p;});
    replace(0x77f190,'int',['pointer'],'thiscall',()=>{++current.releases;return 0;});
    replace(0x7b2f60,'int',['pointer','pointer','int','int','uchar'],'thiscall',(self,out,x,y,loaded)=>{
      current.pixels.push([x,y,loaded]);out.writeU8(y>>>8);out.add(1).writeU8(x+1);out.add(2).writeU8(y);out.add(3).writeU8(0);return 1;
    });Interceptor.flush();
    const decode=new NativeFunction(ptr(0x51eaf0),'pointer',['uint16','pointer','uchar'],'mscdecl');
    for(const type of [0x6c,0xc0,0xc1,0xc2]) for(const delay of [0,255]) {
      current={type,delay,pixels:[],demands:0,releases:0};const p=decode(type,ptr(0),delay);
      Object.assign(current,inspect(p));events.push(current);
    }
    const ctor=new NativeFunction(ptr(0x532260),'pointer',['pointer'],'thiscall');
    const range=new NativeFunction(ptr(0x52b150),'void',['pointer','uint','uint'],'thiscall');
    for(const input of [[0,86],[1,110],[6,26],[0x10003,0x10049],[0xffff0000,0xffff006e]]) {
      current={input,pixels:[],demands:0,releases:0};const p=Memory.alloc(0x2a0);ctor(p);
      current.pixels=[];current.demands=current.releases=0;
      range(p,input[0],input[1]);Object.assign(current,inspect(p));
      const cell=p.add(0x192).readPointer(),pal=p.add(0x19a);
      current.range=pal.add(0x1c+(input[0]&0xffff)).readU8();
      current.cachedRange=cell.add(8+0x1c+(input[0]&0xffff)).readU8();events.push(current);
    }
  } catch(e) {error=String(e);}
  finally {global.writePointer(old);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:global.readPointer().equals(old),hooksRestored:addresses.every((p,i)=>
    Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events):
    lines=['// Canonical acid-leaf/factory/range capture; resource/base callbacks isolated.',
           '// MD5: '+MD5,'#ifndef ACID_BLOB_GOLDEN_H_','#define ACID_BLOB_GOLDEN_H_',
           'struct AcidBlobFactoryGolden { USHORT type; BYTE delay, color, cached; };',
           'static const AcidBlobFactoryGolden ACID_BLOB_FACTORY[] = {']
    for e in events[:8]: lines.append(f"    {{{e['type']},{e['delay']},{e['color']},{e['cached']}}},")
    lines += ['};','struct AcidBlobRangeGolden { DWORD range,value; BYTE color,cached; USHORT row; };',
              'static const AcidBlobRangeGolden ACID_BLOB_RANGE[] = {']
    for e in events[8:]:
        lines.append(f"    {{{e['input'][0]}u,{e['input'][1]}u,{e['range']},{e['cachedRange']},{e['pixels'][0][1]}}},")
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0]);ap.add_argument('--out',type=Path,required=True)
    ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,45);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored') and len(events)==13
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        good=good and e['vtable']=='0x84e504' and e['speed']==20 and e['tint']==1
        good=good and [e['offset'],e['mirror'],e['shadow'],e['dirs'],e['callback'],e['attached']]==[0,0,0,16,-1,-1]
        good=good and e['sequence']==0 and e['shared'] and e['demands']==e['releases']
        if 'type' in e:
            color={0x6c:73,0xc0:86,0xc1:110,0xc2:26}[e['type']]
            good=good and e['color']==color and e['cached']==73 and e['stamped']==e['type']-1
            good=good and e['height']==1 and e['fire']==e['arrival']==[0]*8 and e['size']==0x2a0
            good=good and e['demands']==(1 if e['type']==0x6c else 2)
        else:
            good=good and e['range']==(e['input'][1]&255) and e['cachedRange']==(73 if e['input'][0]&0xffff==0 else 0)
            good=good and e['pixels']==[[i,e['input'][1]&0xffff,1] for i in range(12)] and e['demands']==1
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),original_md5=MD5,fingerprints=hashes,
                scope='Post-base constructor, factory and real palette copy/range loop; bitmap/resource/base callbacks isolated')
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
