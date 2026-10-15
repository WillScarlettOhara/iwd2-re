"""Native SunrayX constructor, three-phase AI, four-slice renderer and Fire ABI.

Frozen original with rooted isolated resources, arrays, FX, sound and RNG.
No original HP or raster claim.
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
    replace(0x57eab0,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x7ae010,'int',['pointer','uchar'],'thiscall',p=>p.add(0xc4).readS16()>=(current.input.length??4)-1?1:0);
    target.writePointer(actorTable);actorTable.add(0x1c).writePointer(cb('pointer',['pointer'],p=>p.add(6)));target.add(6).writeS32(160);target.add(10).writeS32(96);
    const parent=alloc(4),parentVT=alloc(0x80);parent.writePointer(parentVT);parentVT.add(0x7c).writePointer(cb('void',['pointer'],()=>current.order.push('callback')));
    replace(0x599c70,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(p,id,t,out)=>{current.order.push('deny:'+id);out.writePointer(parent);return 0;});
    replace(0x59a010,'uchar',['pointer','int','uchar','uint'],'thiscall',(p,id)=>{current.order.push('release-deny:'+id);return 0;});
    replace(0x52a4e0,'void',['pointer','uint','uint','int','int'],'thiscall',(p,a,b,loop,forget)=>current.order.push('sound:'+loop+':'+forget));
    replace(0x5ce280,'int',['pointer','pointer','int','int','uint','int'],'thiscall',(p,c,x,y,f,t)=>{current.draws.push([c.add(0xc6).readS16(),x,y,f,t]);return 1;});
    const ctor=new NativeFunction(ptr(0x57e860),'pointer',['pointer','pointer','pointer','uchar','int','int','int16'],'thiscall');
    const ai=new NativeFunction(ptr(0x57eb30),'void',['pointer'],'thiscall');
    const render=new NativeFunction(ptr(0x57ed30),'void',['pointer','pointer','pointer','int'],'thiscall');
    const fire=new NativeFunction(ptr(0x57f050),'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');
    const oldSeed=seed;seed=function(kind,input){oldSeed(kind,input);current.plans=[];current.draws=[];};
    function carrier(color=255,flags=0,anim=0,life=20){const p=alloc(0x29e);ctor(p,string('SunrayX'),string('EFF_P19'),color,flags,anim,life);p.add(0x5c).writeS32(44);p.add(0xea).writePointer(area);p.add(0x76).writeS32(-1);p.add(6).writeS32(160);p.add(10).writeS32(96);p.add(14).writeS32(17);return p;}
    function finish(p){current.state=[p.add(0x298).readS16(),p.add(0x256).readS16(),p.add(0x258).readS16(),p.add(0x29a).readS32()];events.push(current);}
    Interceptor.flush();
    for(const color of [0,1,73,127,254,255])for(const flags of [0,1,-1])for(const anim of [0,-1]){
      seed('ctor',{color,flags,anim});const p=carrier(color,flags,anim,-32768);current.vtable=p.readPointer().toString();current.trail=p.add(0xe2).readS32();finish(p);
    }
    for(const lifetime of [-32768,-1,0,1,2,20,32767])for(const stop of [0,1,2])for(const anim of [0,-1])for(const phase of [0,1,2,3,-1,0x10000])for(const frame of [0,3]){
      seed('ai',{lifetime,stop,anim,phase,frame,length:4});const p=carrier(255,0,anim,lifetime);p.add(0x29a).writeS32(phase);p.add(0x256).writeS16(frame);current.order=[];
      game.add(0x4b40).writeS32(stop);game.add(0x4b44).writeS32(stop===1?43:44);ai(p);finish(p);
    }
    game.add(0x4b40).writeS32(0);
    for(const input of [{length:0,anim:1},{length:1,anim:1},{length:255,anim:1,random:32767},{missing:true},{foreign:true},{missing:true,anim:1},{foreign:true,anim:1}]){
      seed('target-ai',input);const p=carrier(255,0,input.anim??0);p.add(0x76).writeS32(22);target.add(0x12).writePointer(input.foreign?foreign:area);current.order=[];ai(p);finish(p);
    }
    target.add(0x12).writePointer(area);
    for(const targetId of [-1,22])for(const missing of [false,true])for(const failAdd of [false,true])for(const callback of [-1,99])for(const y of [96,2147483647]){
      seed('fire',{targetId,missing,failAdd,callback,y});const p=carrier();p.add(0x7a).writeS32(callback);p.add(0x15a).writeS32(1);target.add(10).writeS32(y);current.order=[];
      fire(p,area,11,targetId,256,y,31,13);current.point=point(p.add(6));current.z=p.add(14).readS32();finish(p);
    }
    area.add(0x866).writeS16(128);area.add(0x514).writeS32(0);area.add(0x518).writeS32(0);area.add(0x51c).writeS32(320);area.add(0x520).writeS32(240);area.add(0x550).writeS32(10000);
    for(const phase of [0,1,2,-1,0x4000,0x10001])for(const flags of [0,1,-1])for(const hidden of [false,true])for(const lockFail of [false,true]){
      seed('render',{phase,flags,hidden,lockFail});const p=carrier(255,flags);p.add(0x29a).writeS32(phase);current.order=[];render(p,ptr(0),ptr(1),1);finish(p);
    }
    for(const input of [{y:-10},{y:2147483647},{y:10001},{centerY:4},{lockFail:true},{z:-17},{z:2147483647}]){
      seed('render',input);const p=carrier();p.add(10).writeS32(input.y??96);p.add(14).writeS32(input.z??17);current.order=[];render(p,ptr(0),ptr(1),1);finish(p);
    }
  }catch(e){error=String(e);if(current){current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);events.push(current);}}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""

def header(events):
    lines=['// Native SunrayX control capture; dependencies isolated.','#ifndef SUNRAY_GOLDEN_H_','#define SUNRAY_GOLDEN_H_',
      'struct SunrayAIGolden { SHORT lifetime,stop; BOOL random; LONG phase; SHORT frame,afterLife,afterFrame,afterSequence; LONG afterPhase; BOOL retired; };','static const SunrayAIGolden SUNRAY_AI[] = {']
    for e in events:
        if e['kind']!='ai':continue
        c=e['input'];v=[c['lifetime'],c['stop'],int(bool(c['anim'])),c['phase'],c['frame'],*e['state'],int('destroy' in e['order'])]
        lines.append('    {'+','.join(map(str,v))+'},')
    return '\n'.join(lines+['};','#endif',''])

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(strike.__file__),Path(strike.phase.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored')
    good=good and not any(m.get('type')=='error' for m in result['messages']) and len(events)==36+504+7+32+72+7
    for e in events:
        if e['kind']=='ctor':good=good and e['vtable']=='0x8512bc' and e['trail']==0
        if e['kind']=='target-ai' and e['input'].get('foreign'):good=good and e['order'][-2:]==['remove-self','release-share:22']
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
