"""School-hit factory, overlay lifetime and opcode 233 native control capture.

Original frozen; resources, arrays, sound, animation and effect/message ownership
use rooted isolated callbacks. This is an ABI/state oracle, not HP or raster.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import call_lightning_reference as strike
from projectile_aim_reference import MD5,capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO,game_directory

JS=strike.JS[:strike.JS.index('    const ctor=new NativeFunction')]+r"""
    const glows=new Map(),actor=alloc(0x160),actorVT=alloc(0x80);
    actor.writePointer(actorVT);actor.add(6).writeS32(160);actor.add(10).writeS32(96);actor.add(0x5c).writeS32(22);actor.add(0x12).writePointer(area);
    actorVT.add(0x1c).writePointer(cb('pointer',['pointer'],p=>p.add(6)));
    actorVT.add(0x78).writePointer(cb('void',['pointer','pointer','uchar','int','int'],(p,e,l,a,b)=>current.glows.push(['direct',glows.get(e.toString()),l,a,b])));
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
    replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(line)=>current.order.push('assert:'+line));
    replace(0x578110,'pointer',['pointer','uint','uint'],'thiscall',(p,a,b)=>{
      const cell=alloc(0xda);cell.writePointer(cellTable);p.add(0x192).writePointer(cell);
      const rr=alloc(8);rr.writeU32(a);rr.add(4).writeU32(b);ref(cell.add(8),res(rr));
      p.add(0x72).writeS32(0);p.add(0x76).writeS32(0);p.add(0x7a).writeS32(-1);p.add(0x1de).writeS32(1);
      current.constructed=res(rr);return p;
    });
    replace(0x530790,'pointer',['pointer'],'thiscall',p=>{current.constructed='';p.writePointer(fakeTable);return p;});
    replace(0x585cc0,'pointer',['uchar','uchar','uchar','uchar'],'mscdecl',(r,g,b,t)=>{const e=alloc(4);glows.set(e.toString(),[r,g,b,t]);return e;});
    replace(0x4f7500,'int',['pointer','pointer','int'],'thiscall',(p,m,f)=>{
      if(m.readPointer().equals(ptr(0x8487cc)))current.glows.push(['message',glows.get(m.add(12).readPointer().toString()),m.add(4).readS32(),m.add(8).readS32(),f]);
      else current.order.push('message');return 1;
    });
    replace(0x7ae010,'int',['pointer','uchar'],'thiscall',p=>p.add(0xc4).readS16()>=(current.input.length??4)-1?1:0);
    replace(0x7adf40,'int16',['pointer','uchar'],'thiscall',()=>current.input.sequences??current.input.count??3);
    replace(0x7a9ae0,'int',['pointer'],'thiscall',()=>current.input.sound?1:0);
    replace(0x579b10,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x52a4e0,'void',['pointer','uint','uint','int','int'],'thiscall',(p,a,b,loop,forget)=>current.order.push('sound:'+loop+':'+forget));
    const factory=new NativeFunction(ptr(0x560310),'pointer',['int','pointer','uchar'],'mscdecl');
    const ctor=new NativeFunction(ptr(0x57e490),'pointer',['pointer','pointer','pointer'],'thiscall');
    const ai=new NativeFunction(ptr(0x57e580),'void',['pointer'],'thiscall');
    const fire=new NativeFunction(ptr(0x57e710),'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');
    const oldSeed=seed;seed=function(kind,input){oldSeed(kind,input);current.glows=[];};
    function finish(p){const c=p.add(0x192).readPointer();current.state=[c.add(0xc4).readS16(),c.add(0xc6).readS16(),p.add(0x2b4).readS32(),p.add(0x1de).readS32()];events.push(current);}
    function overlay(){const p=alloc(0x2ba),v=alloc(12);v.add(8).writeU32(0x20000);ctor(p,text('InvocH'),v);p.add(0x5c).writeS32(44);p.add(6).writeS32(160);p.add(10).writeS32(96);p.add(14).writeS32(17);return p;}
    Interceptor.flush();
    for(let type=0;type<112;++type)for(const positive of [0,1,2,255])for(const caster of [false,true]){
      seed('factory',{type,positive,caster});const p=factory(type,caster?actor:ptr(0),positive);current.vtable=p.readPointer().toString();
      current.visual=current.constructed??current.refs[0]?.[0]??'';current.fire=res(p.add(0x152));current.arrival=res(p.add(0x15e));
      if(current.vtable==='0x851234')current.config=[p.add(0x2a2).readU8(),p.add(0x2a3).readU8(),p.add(0x2aa).readU32(),p.add(0x2ae).readU8(),p.add(0x2af).readU8(),p.add(0x2b0).readS32(),p.add(0x2b4).readS32(),p.add(0x2b8).readU8(),p.add(0x2b9).readU8()];
      events.push(current);
    }
    for(const stop of [0,1,2])for(const single of [0,1,2])for(const looping of [0,1,2])for(const sequence of [0,1,2,-1])for(const count of [0,1,2])for(const frame of [1,2,3])for(const sound of [false,true]){
      seed('ai',{stop,single,looping,sequence,count,frame,sound,length:4,sequences:3});const p=overlay(),c=p.add(0x192).readPointer();
      p.add(0x2af).writeU8(looping);p.add(0x2b0).writeS32(1);p.add(0x2b4).writeS32(count);p.add(0x2b9).writeU8(single);
      c.add(0xc4).writeS16(frame);c.add(0xc6).writeS16(sequence);game.add(0x4b40).writeS32(stop);game.add(0x4b44).writeS32(stop===1?43:44);
      current.order=[];ai(p);finish(p);
    }
    game.add(0x4b40).writeS32(0);
    for(const random of [0,1,2])for(const count of [0,1,2,3,255,256])for(const offset of [0,1,2])for(const failAdd of [false,true]){
      seed('fire',{random,count,offset,failAdd});const p=overlay();p.add(0x2b8).writeU8(random);p.add(0x2ae).writeU8(offset);p.add(0x15a).writeS32(1);
      p.add(0x72).writeS32(71);p.add(0x76).writeS32(72);current.order=[];fire(p,area,11,22,256,-2147483648,31,13);
      current.point=point(p.add(6));current.z=p.add(14).readS32();current.ids=[p.add(0x72).readS32(),p.add(0x76).readS32()];finish(p);
    }
    // Isolate the factory after its complete branch capture to expose dispatcher ABI.
    fakeTable.add(0x6c).writePointer(cb('void',['pointer','pointer','int','int','int','int','int','int16'],(p,a,s,t,x,y,z,k)=>current.fires.push([s,t,x,y,z,k])));
    replace(0x560310,'pointer',['int','pointer','uchar'],'mscdecl',(type,p,positive)=>{current.decoded=[type,p.equals(actor),positive];const out=alloc(0x192);out.writePointer(fakeTable);return out;});
    replace(0x500470,'pointer',['pointer','uint16','int','pointer','int','int','int','uchar'],'thiscall',(p,t,target,pos,z,s,d,k)=>{current.messages.push([t,target,point(pos),z,s,d,k]);p.writePointer(ptr(1));return p;});
    const dispatch=new NativeFunction(ptr(0x5601e0),'int',['pointer','pointer'],'thiscall');Interceptor.flush();
    for(const type of [0,1,3,45,57,73,98,111,0x10003,0xfffffffd])for(const school of ['SPPTEST','SPpTEST','SPWTEST',''])for(const missing of [false,true]){
      seed('dispatch',{type,school,missing});const e=alloc(0x114);e.add(0x1c).writeU32(type);e.add(0x90).writeByteArray(Array.from(school.padEnd(8,'\0'),c=>c.charCodeAt(0)));actor.add(0x12).writePointer(missing?ptr(0):area);
      current.result=dispatch(e,actor);current.done=e.add(0x110).readS32();events.push(current);
    }
    actorVT.add(4).writePointer(cb('uchar',['pointer'],()=>1));
    replace(0x599c70,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(p,id,t,out)=>{current.order.push('deny:'+id);if(current.input.missing)return 2;out.writePointer(actor);return 0;});
    replace(0x59a010,'uchar',['pointer','int','uchar','uint'],'thiscall',(p,id)=>{current.order.push('release-deny:'+id);return 0;});
    replace(0x51eaf0,'pointer',['uint16','pointer','uchar'],'mscdecl',(type,p,delay)=>{current.decoded=[type,p.equals(actor),delay];const out=alloc(0x192);out.writePointer(fakeTable);return out;});
    Interceptor.flush();const run=new NativeFunction(ptr(0x5007f0),'void',['pointer'],'thiscall');
    for(const provider of [0,1])for(const owned of [false,true])for(const noArea of [false,true])for(const missing of [false,true]){
      seed('replicate',{provider,owned,noArea,missing});chitin.add(0x96e).writeS32(provider);chitin.add(0x104c).writeS32(17);actor.add(0x62).writeS32(owned?17:18);actor.add(0x12).writePointer(noArea?ptr(0):area);
      const m=alloc(0x24);m.add(4).writeS32(22);m.add(0xc).writeU16(0x1004);m.add(0xe).writeS32(33);m.add(0x12).writeS32(160);m.add(0x16).writeS32(96);m.add(0x1a).writeS32(31);m.add(0x1e).writeU8(255);run(m);events.push(current);
    }
  }catch(e){error=String(e);if(current){current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);events.push(current);}}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""

def header(events):
    lines=['// Canonical school-hit ABI/state capture; dependencies isolated.','#ifndef SCHOOL_HIT_GOLDEN_H_','#define SCHOOL_HIT_GOLDEN_H_',
      'struct SchoolHitFactoryGolden { INT type; BYTE positive; BOOL caster; const char *visual,*fire; BYTE tint; DWORD flags; BYTE offset,loop; LONG loopIndex,loopCount; BYTE random,single; BOOL glow,queued; BYTE r,g,b; };',
      'static const SchoolHitFactoryGolden SCHOOL_HIT_FACTORY[] = {']
    for e in events:
        if e['kind']!='factory' or e['input']['positive'] not in (0,1):continue
        c=e['input'];v=e.get('config',[0,0,0,0,0,0,0,0,0]);glow=e['glows'][0] if e['glows'] else None
        rgb=glow[1][:3] if glow else [0,0,0]
        values=[c['type'],c['positive'],int(c['caster']),json.dumps(e['visual']),json.dumps(e['fire']),v[0],v[2],*v[3:],int(bool(glow)),int(bool(glow) and glow[0]=='message'),*rgb]
        lines.append('    {'+','.join(map(str,values))+'},')
    lines+=['};','struct SchoolHitAIGolden { BYTE stop,single,loop; SHORT sequence; LONG count; SHORT frame; BOOL sound; SHORT afterFrame,afterSequence; LONG afterCount,visible; BOOL retired; };','static const SchoolHitAIGolden SCHOOL_HIT_AI[] = {']
    for e in events:
        if e['kind']!='ai' or e['input']['sequence']<0:continue
        c=e['input'];values=[c[k] for k in ('stop','single','looping','sequence','count','frame','sound')]+e['state']+[int('destroy' in e['order'])]
        lines.append('    {'+','.join(str(int(v)) for v in values)+'},')
    return '\n'.join(lines+['};','#endif',''])

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(strike.__file__),Path(strike.phase.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored')
    good=good and not any(m.get('type')=='error' for m in result['messages']) and len(events)==896+1944+108+80+16
    for e in events:
        c=e['input']
        if e['kind']=='fire':good=good and e['ids']==[71,72] and e['order'][0]=='deliver' and e['z']==(17 if c['failAdd'] else 0)
        if e['kind']=='dispatch' and not c['missing']:good=good and e['decoded'][2]==int(len(c['school'])>2 and c['school'][2] in 'Pp')
        if e['kind']=='replicate':good=good and bool(e['fires'])==bool(c['provider'] and not c['owned'] and not c['noArea'] and not c['missing'])
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
