"""Frozen original TString construction, substring assignment and COW detach.

Real six native bodies execute; rooted allocation/free callbacks retain released
bytes for ownership observations. No game-state, HP or raster equivalence claim.
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


def stores():
    result=[dict(data=[],length=0,capacity=0,rc=0)]
    for data in [[],list(b'ICloudA'),list(b'A'*40),[255,128,127,0,65,98,67]]:
        for rc in [0,1,2,127,253,254,255]:
            result.append(dict(data=data,length=len(data),capacity=len(data)|31,rc=rc))
    return result


def cases():
    all_stores=stores();out=[]
    for s in all_stores:
        out.extend([dict(kind='construct',source=s),dict(kind='detach',source=s)])
        if s['capacity']:
            for n in sorted({0,1,s['length']//2,s['length'],s['capacity']}):out.append(dict(kind='length',source=s,size=n))
        targets=[all_stores[i] for i in [0,1,8,10,15,22,28]]
        slices=sorted({(0,0),(0,0xffffffff),(s['length'],0),(s['length']//2,1),(0,s['length']//2)})
        for t in targets:
            for pos,count in slices:out.append(dict(kind='assign',source=s,target=t,pos=pos,count=count))
        if 0 not in s['data']:
            for pos,count in slices:out.append(dict(kind='self',source=s,pos=pos,count=count))
        for pos in sorted({0,s['length']//2,s['length']}):
            for count in [0,1,7,0x7fffffff,0x80000000,0xffffffff]:
                for rhs in [[],list(b'ICloudA'),list(b'ICloudB'),[128],[255],s['data']]:
                    for length in [len(rhs),0xffffffff]:out.append(dict(kind='compare',source=s,pos=pos,count=count,rhs=rhs,rhsLength=length))
    for rc in [1,2,253,254]:
        for data in [list(b'ICloudA'),list(b'A'*40)]:
            s=dict(data=data,length=len(data),capacity=len(data)|31,rc=rc)
            for pos,count in [(0,0xffffffff),(1,len(data)-1),(len(data),0)]:out.append(dict(kind='alias',source=s,pos=pos,count=count))
    out.extend([dict(kind='null-construct'),dict(kind='limit')])
    return out


JS=r"""
rpc.exports.run=function(){
  const memory=[],callbacks=[],addresses=[],before=[],events=[];let current=null,error=null;
  function alloc(n){const p=Memory.alloc(Math.max(n,2));memory.push(p);return p;}
  function replace(address,ret,args,fn){const p=ptr(address),f=new NativeCallback(fn,ret,args,'mscdecl');callbacks.push(f);addresses.push(p);before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));Interceptor.replace(p,f);}
  function obj(input,head){const p=alloc(16);p.writeByteArray([head,0xa1,0xb2,0xc3]);if(!input.capacity)return [p,null];const b=alloc(input.capacity+2);b.writeU8(input.rc);b.add(1).writeByteArray(Array(input.capacity+1).fill(0xcc));b.add(1).writeByteArray(input.data);b.add(1+input.length).writeU8(0);p.add(4).writePointer(b.add(1));p.add(8).writeS32(input.length);p.add(12).writeS32(input.capacity);return[p,b];}
  function state(p,src,old){const d=p.add(4).readPointer(),length=p.add(8).readU32();return {head:p.readU8(),padding:Array.from(new Uint8Array(p.add(1).readByteArray(3))),length,capacity:p.add(12).readU32(),origin:d.isNull()?0:src!==null&&d.equals(src.add(1))?1:old!==null&&d.equals(old.add(1))?2:3,rc:d.isNull()?-1:d.sub(1).readU8(),data:d.isNull()?[]:Array.from(new Uint8Array(d.readByteArray(length))),terminated:d.isNull()||d.add(length).readU8()===0};}
  function oldState(b){return b===null?[-1,0]:[b.readU8(),current.frees.includes(b.toString())?1:0];}
  try{
    if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000))||ptr(0x8485c0).readU32()!==0xffffffff)throw Error('Wrong original');
    replace(0x7fc95b,'pointer',['uint'],n=>{current.allocations.push(n);return alloc(n);});
    replace(0x7fc984,'void',['pointer'],p=>current.frees.push(p.toString()));Interceptor.flush();
    const construct=new NativeFunction(ptr(0x44ba90),'void',['pointer','pointer'],'mscdecl');
    const assign=new NativeFunction(ptr(0x45e870),'pointer',['pointer','pointer','uint','uint'],'thiscall');
    const detach=new NativeFunction(ptr(0x44bd40),'void',['pointer'],'thiscall');
    const length=new NativeFunction(ptr(0x44bc00),'void',['pointer','uint'],'thiscall');
    const compare=new NativeFunction(ptr(0x4c5ed0),'int',['pointer','uint','uint','pointer','uint'],'thiscall');
    const limit=new NativeFunction(ptr(0x45ea70),'uint',['pointer'],'thiscall');
    const tidy=new NativeFunction(ptr(0x448d50),'void',['pointer','uchar'],'thiscall');
    for(const c of __CASES__){
      current={kind:c.kind,input:c,allocations:[],frees:[]};
      if(c.kind==='null-construct'){construct(ptr(0),ptr(0));events.push(current);continue;}
      if(c.kind==='limit'){current.value=limit(alloc(16));events.push(current);continue;}
      const [s,sb]=obj(c.source,0x77);let t=s,tb=null;
      if(c.kind==='construct'){t=alloc(16);t.writeByteArray([0x99,0xa1,0xb2,0xc3]);construct(t,s);}
      else if(c.kind==='assign'){[t,tb]=obj(c.target,0x99);current.returned=assign(t,s,c.pos,c.count).equals(t);}
      else if(c.kind==='alias'){t=alloc(16);t.writeByteArray(s.readByteArray(16));t.writeU8(0x99);current.returned=assign(t,s,c.pos,c.count).equals(t);}
      else if(c.kind==='self')current.returned=assign(s,s,c.pos,c.count).equals(s);
      else if(c.kind==='detach')detach(s);
      else if(c.kind==='length')length(s,c.size);
      else if(c.kind==='compare'){const rhs=alloc(128);rhs.writeByteArray(Array(128).fill(0xcc));rhs.writeByteArray(c.rhs);current.value=compare(s,c.pos,c.count,rhs,c.rhsLength);}
      current.target=state(t,sb,tb);current.source=state(s,sb,tb);current.oldSource=oldState(sb);current.oldTarget=oldState(tb);
      if(c.kind==='construct'){tidy(t,1);current.releasedSource=oldState(sb);}
      events.push(current);
    }
  }catch(e){error=String(e);if(current){current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);events.push(current);}}
  finally{for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return{events,error,constantRestored:ptr(0x8485c0).readU32()===0xffffffff,hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events):
    # Compact JSON is retained alongside the capture; C++ rows keep binary bytes,
    # lengths and source/destination ownership rather than assuming C strings.
    def arr(values):return '{'+','.join(str(int(v)) for v in values)+'}'
    def store(s):return '{'+arr(s['data'])+f",{s['length']},{s['capacity']},{s['rc']}"+'}'
    def state(s):return '{'+','.join(map(str,[s['head'],s['length'],s['capacity'],s['origin'],s['rc'],int(s['terminated'])]))+','+arr(s['data'])+'}'
    lines=['// Native TString copy/assignment/detach/compare capture; allocator isolated.','#ifndef TSTRING_SHARING_GOLDEN_H_','#define TSTRING_SHARING_GOLDEN_H_',
      'struct SharingStorage { BYTE data[64]; UINT length,capacity; BYTE rc; };',
      'struct SharingState { INT head; UINT length,capacity; INT origin,rc; BOOL terminated; BYTE data[64]; };',
      'struct SharingGolden { INT kind; SharingStorage source,target; UINT pos,count,size; SharingState afterSource,afterTarget; INT oldSource[2],oldTarget[2],releasedSource[2]; BOOL returned; LONGLONG value; BYTE rhs[128]; UINT rhsLength,rhsBytes; };',
      'static const SharingGolden TSTRING_SHARING[] = {']
    kinds=['construct','assign','alias','self','detach','length','compare','null-construct','limit']
    empty=dict(data=[],length=0,capacity=0,rc=0);es=dict(head=0,length=0,capacity=0,origin=0,rc=-1,terminated=True,data=[])
    for e in events:
        c=e['input'];kind=kinds.index(c['kind'])
        lines.append('    {'+str(kind)+','+store(c.get('source',empty))+','+store(c.get('target',empty))+','+','.join(str(c.get(k,0))+'u' for k in ['pos','count','size'])+','+state(e.get('source',es))+','+state(e.get('target',es))+','+arr(e.get('oldSource',[-1,0]))+','+arr(e.get('oldTarget',[-1,0]))+','+arr(e.get('releasedSource',[-1,0]))+','+str(int(e.get('returned',False)))+','+str(e.get('value',0))+','+arr(c.get('rhs',[]))+','+str(c.get('rhsLength',0))+'u,'+str(len(c.get('rhs',[])))+'u},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(cases())),encoding='utf-8')
    paths=(exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events'];good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('constantRestored')
    good=good and len(events)==len(cases()) and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        if e['kind']=='limit':good=good and e['value']==0xfffffffd
        if 'target' in e:good=good and e['target']['terminated'] and e['target']['padding']==[0xa1,0xb2,0xc3]
        if e['kind']=='construct':good=good and e['source']['data']==e['input']['source']['data']
        if e['kind']=='compare':good=good and e['value'] in [-1,0,1]
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),expected=len(cases()),error=result['error'],counts={k:sum(e['kind']==k for e in events) for k in sorted({e['kind'] for e in events})},fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
