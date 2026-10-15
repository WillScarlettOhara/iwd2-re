"""Frozen native ember bounds, cloud rebinding and TString buffer ownership.

Real target bodies execute with rooted isolated BAM/resource/allocator/FX inputs.
Captures geometry, control, string bytes and ownership; no original HP/raster.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import call_lightning_reference as strike
from projectile_aim_reference import MD5, capture
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO, game_directory


def geometry_cases():
    frames = [[3,4,10,12],[-7,11,23,29],[17,-13,1,2],[-32768,32767,65535,65535],[0,0,0,0]]
    return [dict(main=a, other=b, shadow=s, z=z) for a in frames for b in frames
            for s in [False,True] for z in [-2147483648,-32769,-17,0,17,32768,2147483647]]


def bounds_cases():
    shapes = [([160,96],[3,4],[10,12],17), ([-1,-2],[-7,11],[23,29],-17),
              ([2147483647,-2147483648],[-32768,32767],[65535,65535],2147483647),
              ([-2147483648,2147483647],[2147483647,-2147483648],[-1,-2],-2147483648),
              ([0,0],[0,0],[0,0],0)]
    return [dict(pos=p,anchor=a,size=s,z=z,direction=d,threshold=t) for p,a,s,z in shapes
            for d in [-32768,-1,0,1,8,9,16,255,32767] for t in [0,8,128,255]]


JS = strike.JS[:strike.JS.index('    const ctor=new NativeFunction')] + r"""
    let mainCell=null,otherCell=null;
    const oldSeed=seed;seed=function(kind,input){oldSeed(kind,input);current.allocations=[];current.frees=[];};
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>{const p=alloc(Math.max(n,2));current.allocations.push(n);return p;});
    replace(0x7fc984,'void',['pointer'],'mscdecl',p=>current.frees.push(p.toString()));
    replace(0x7ad520,'int',['pointer','pointer','uchar'],'thiscall',(p,out)=>{const f=p.equals(mainCell)?current.input.main:current.input.other;out.writeS32(f[0]);out.add(4).writeS32(f[1]);return 1;});
    replace(0x7ad800,'int',['pointer','pointer','uchar'],'thiscall',(p,out)=>{const f=p.equals(mainCell)?current.input.main:current.input.other;out.writeS32(f[2]);out.add(4).writeS32(f[3]);return 1;});
    replace(0x7e8160,'int',[],'mscdecl',()=>{current.order.push('rand');return 41;});
    replace(0x7adf40,'int16',['pointer','uchar'],'thiscall',()=>current.input.count??3);
    replace(0x786df0,'pointer',['pointer','pointer','uint16','int'],'thiscall',(p,s,t,w)=>{current.order.push('resource:'+res(s)+':'+t);return current.input.missing?ptr(0):resource;});
    replace(0x40f940,'int',['pointer','pointer'],'thiscall',(p,s)=>res(p)===res(s)?1:0);
    replace(0x442990,'void',['pointer','pointer','int','int'],'thiscall',(p,s,a,w)=>{p.add(4).writePointer(current.input.missing?ptr(0):resource);p.add(8).writeByteArray(s.readByteArray(8));current.refs.push([res(s),a,w]);});
    const fx=new NativeFunction(ptr(0x55d690),'void',['pointer','pointer','pointer','int'],'thiscall');
    const bounds=new NativeFunction(ptr(0x6a33f0),'void',['pointer','pointer','pointer','pointer','int','int','int'],'thiscall');
    const change=new NativeFunction(ptr(0x55d800),'void',['pointer','int16'],'thiscall');
    const end=new NativeFunction(ptr(0x55d260),'int',['pointer'],'thiscall');
    const frame=new NativeFunction(ptr(0x55d210),'int16',['pointer'],'thiscall');
    const rebind=new NativeFunction(ptr(0x55dbd0),'void',['pointer','pointer'],'thiscall');
    const reserve=new NativeFunction(ptr(0x44bc20),'uchar',['pointer','uint','uchar'],'thiscall');
    const grow=new NativeFunction(ptr(0x44be10),'void',['pointer','uint'],'thiscall');
    const tidy=new NativeFunction(ptr(0x448d50),'void',['pointer','uchar'],'thiscall');
    function vec(v){const p=alloc(8);p.writeS32(v[0]);p.add(4).writeS32(v[1]);return p;}
    function storage(p,capacity,length,rc){p.writeU8(0x77);if(!capacity)return null;const b=alloc(capacity+2);b.writeU8(rc);b.add(1).writeUtf8String('O'.repeat(length));p.add(4).writePointer(b.add(1));p.add(8).writeS32(length);p.add(12).writeS32(capacity);return b;}
    function state(p,b){const d=p.add(4).readPointer();return {head:p.readU8(),length:p.add(8).readS32(),capacity:p.add(12).readS32(),null:d.isNull(),same:b!==null&&d.equals(b.add(1)),oldRc:b===null?-1:b.readU8(),rc:d.isNull()?-1:d.sub(1).readU8(),value:d.isNull()?'':d.readCString(),freed:b!==null&&current.frees.includes(b.toString())};}
    Interceptor.flush();
    for(const input of __GEOMETRY__){seed('geometry',input);const p=alloc(0x606),r=alloc(16),a=alloc(8);mainCell=p.add(0x406);otherCell=p.add(0x4e0);p.add(0x3fe).writePointer(mainCell);if(input.shadow)p.add(0x402).writePointer(otherCell);fx(p,r,a,input.z);current.rect=[...point(r),...point(r.add(8))];current.anchor=point(a);events.push(current);}
    for(const input of __BOUNDS__){seed('bounds',input);const p=alloc(0x606),r=alloc(16);p.add(0x5e4).writeS16(input.direction);p.add(0x5e6).writeU8(input.threshold);bounds(p,r,vec(input.pos),vec(input.anchor),input.z,...input.size);current.rect=[...point(r),...point(r.add(8))];events.push(current);}
    for(const direction of [-32768,-1,0,1,8,16,32767])for(const ended of [false,true])for(const alternate of [false,true]){seed('query',{direction,end:ended,alternate});const p=alloc(0x606);p.add(0x3fe).writePointer(p.add(alternate?0x4e0:0x406));p.add(0x5e2).writeS16(7);p.add(0x406+0xc4).writeS16(11);p.add(0x4e0+0xc4).writeS16(13);change(p,direction);current.state=[p.add(0x5e4).readS16(),p.add(0x5e2).readS16(),p.add(0x406+0xc4).readS16(),p.add(0x4e0+0xc4).readS16(),end(p),frame(p)];events.push(current);}
    const stores=[[0,0,0],[31,7,0],[63,40,0],[31,7,1],[63,40,254],[31,7,255]];
    for(const [capacity,length,rc] of stores)for(const count of [0,1,3,255,256,257,-1])for(const mode of [0,1,2])for(const name of ['', 'ICloudA','ICloudB','ZRABSENT']){
      const missing=name==='ZRABSENT';seed('rebind',{capacity,length,rc,count,mode,name,missing});const p=alloc(0x606),b=storage(p.add(0x5f6),capacity,length,rc);
      p.add(0x3fe).writePointer(p.add(0x4e0));p.add(0x402).writePointer(p.add(0x4e0));p.add(0x5e7).writeU8(mode);p.add(0x5e0).writeU8(2);p.add(0x5e4).writeS16(-123);
      p.add(0x406+0xa8).writePointer(resource);p.add(0x406+0xa4).writeS32(1);ref(p.add(0x406+0xac),'OLD');p.add(0x406+0xc4).writeS16(11);p.add(0x4e0+0xc4).writeS16(13);
      rebind(p,text(name));current.string=state(p.add(0x5f6),b);current.state=[p.add(0x5e2).readS16(),p.add(0x406+0xc4).readS16(),p.add(0x406+0xc6).readS16(),p.add(0x4e0+0xc4).readS16(),p.add(0x5e0).readU8(),p.add(0x5e4).readS16()];current.mainRef=res(p.add(0x406+0xac));events.push(current);
    }
    for(const [capacity,length,rc] of stores)for(const size of [0,1,7,31,32,64,100])for(const trim of [0,1,2]){seed('reserve',{capacity,length,rc,size,trim});const p=alloc(16),b=storage(p,capacity,length,rc);current.returned=reserve(p,size,trim);current.string=state(p,b);events.push(current);}
    for(const [capacity,length,rc] of stores)for(const size of [0,1,7,31,32,63,64,100]){seed('grow',{capacity,length,rc,size});const p=alloc(16),b=storage(p,capacity,length,rc);grow(p,size);current.string=state(p,b);events.push(current);}
    for(const [capacity,length,rc] of stores)for(const release of [0,1,2,255]){seed('tidy',{capacity,length,rc,release});const p=alloc(16),b=storage(p,capacity,length,rc);tidy(p,release);current.string=state(p,b);events.push(current);}
  }catch(e){error=String(e);if(current){current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);events.push(current);}}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events):
    def nums(v): return ','.join(str(int(x)) for x in v)
    def arr(v): return '{'+nums(v)+'}'
    lines=['// Native ember geometry/cloud/string capture; dependencies isolated.', '#ifndef EMBER_GEOMETRY_GOLDEN_H_', '#define EMBER_GEOMETRY_GOLDEN_H_',
           'struct EmberGeometryGolden { LONG main[4],other[4]; BOOL shadow; LONG z,rect[4],anchor[2]; };','static const EmberGeometryGolden EMBER_GEOMETRY[] = {']
    for e in events:
        if e['kind']=='geometry':
            c=e['input'];lines.append('    {'+arr(c['main'])+','+arr(c['other'])+','+nums([c['shadow'],c['z']])+','+arr(e['rect'])+','+arr(e['anchor'])+'},')
    lines+=['};','struct EmberBoundsGolden { LONG pos[2],anchor[2],size[2],z; SHORT direction; BYTE threshold; LONG rect[4]; };','static const EmberBoundsGolden EMBER_BOUNDS[] = {']
    for e in events:
        if e['kind']=='bounds':
            c=e['input'];lines.append('    {'+','.join(arr(c[k]) for k in ['pos','anchor','size'])+','+nums([c['z'],c['direction'],c['threshold']])+','+arr(e['rect'])+'},')
    lines+=['};','struct EmberQueryGolden { SHORT direction; BOOL end,alternate; LONG state[6]; };','static const EmberQueryGolden EMBER_QUERIES[] = {']
    for e in events:
        if e['kind']=='query':
            c=e['input'];lines.append('    {'+nums([c['direction'],c['end'],c['alternate']])+','+arr(e['state'])+'},')
    lines+=['};','struct EmberStringGolden { INT capacity,length; BYTE rc; UINT size; BYTE flag; BOOL returned; INT afterLength,afterCapacity; BOOL null,same,freed; INT oldRc,afterRc; const char* value; };']
    for kind in ['reserve','grow','tidy']:
        lines+=['static const EmberStringGolden EMBER_STRING_'+kind.upper()+'[] = {']
        for e in events:
            if e['kind']!=kind:continue
            c=e['input'];s=e['string'];lines.append('    {'+nums([c['capacity'],c['length'],c['rc'],c.get('size',0),c.get('trim',c.get('release',0)),e.get('returned',False),s['length'],s['capacity'],s['null'],s['same'],s['freed'],s['oldRc'],s['rc']])+','+json.dumps(s['value'])+'},')
        lines+=['};']
    lines+=['struct EmberRebindGolden { INT capacity,length; BYTE rc; SHORT count; BYTE mode; const char* name; LONG state[6]; INT afterCapacity,oldRc; BOOL same,freed; INT randomCalls; };','static const EmberRebindGolden EMBER_REBIND[] = {']
    for e in events:
        if e['kind']!='rebind':continue
        c=e['input'];s=e['string']
        if (not c['name'] or c['missing']) and c['count']!=0:continue
        lines.append('    {'+nums([c['capacity'],c['length'],c['rc'],c['count'],c['mode']])+','+json.dumps(c['name'])+','+arr(e['state'])+','+nums([s['capacity'],s['oldRc'],s['same'],s['freed'],e['order'].count('rand')])+'},')
    return '\n'.join(lines+['};','#endif','']).replace('-2147483648','(-2147483647 - 1)')


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__GEOMETRY__',json.dumps(geometry_cases())).replace('__BOUNDS__',json.dumps(bounds_cases())),encoding='utf-8')
    paths=(exe,Path(__file__),Path(strike.__file__),Path(strike.phase.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored')
    expected=len(geometry_cases())+len(bounds_cases())+28+504+126+48+24
    good=good and len(events)==expected and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        if e['kind'] in ['reserve','grow','tidy','rebind']:good=good and e['string']['head']==0x77
        if e['kind']=='rebind':good=good and e['string']['value']==e['input']['name'] and e['state'][-2:]==[2,-123]
        if e['kind']=='query':good=good and e['state']==[e['input']['direction'],7,11,13,int(e['input']['end']),-1]
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),expected=expected,error=result['error'],counts={k:sum(e['kind']==k for e in events) for k in sorted({e['kind'] for e in events})},fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
