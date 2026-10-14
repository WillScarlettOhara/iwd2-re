"""Native colored-strike construction, AI and layered render plans.

Original frozen; resources, palette helpers, arrays, animation/FX and RNG inputs
isolated with rooted callbacks. No original HP or raster claim.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import phase_bam_reference as phase
from projectile_aim_reference import MD5,capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO,game_directory

PREFIX=phase.JS[:phase.JS.index('    const ctor=new NativeFunction')]
PREFIX=PREFIX.replace('const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);',
    "const p=ptr(a),f=new NativeCallback(function(...v){if(current){if(!current.calls)current.calls=[];current.calls.push(a.toString(16));}return fn(...v);},ret,args,abi);")
PREFIX=PREFIX.replace("callbacks.push(f);addresses.push(p);\n    before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));Interceptor.replace(p,f);",
    "callbacks.push(f);const index=addresses.findIndex(v=>v.equals(p));if(index>=0){Interceptor.revert(p);Interceptor.flush();}else{addresses.push(p);before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));}Interceptor.replace(p,f);")
JS=PREFIX+r"""
    replace(0x7beea0,'pointer',['pointer','uint16'],'thiscall',(p,t)=>{current.order.push('palette-type:'+t);p.add(8).writePointer(colors);p.add(0x14).writeU16(t);return p;});
    replace(0x7bf7d0,'void',['pointer','uint16','uint16','pointer'],'thiscall',(p,r,c,b)=>{current.range=[r,c,b.equals(game.add(0x3672))];p.add(0x1c+r).writeU8(c);current.order.push('range');});
    replace(0x7ae300,'void',['pointer','pointer'],'thiscall',(p,pal)=>{p.add(8+8).writePointer(pal.add(8).readPointer());p.add(8+0x1c).writeU8(pal.add(0x1c).readU8());current.order.push('bind');});
    replace(0x534b10,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x4c7f60,'void',['pointer'],'thiscall',()=>current.order.push('remove'));
    replace(0x59a530,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',()=>{current.order.push('delete');return current.input.failDelete?2:0;});
    const target=alloc(0x160),foreign=alloc(0xb20);target.add(0x12).writePointer(area);
    replace(0x599a50,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(p,id,t,out)=>{current.order.push('share:'+id);if(current.input.missing)return 2;out.writePointer(target);return 0;});
    replace(0x599e70,'uchar',['pointer','int','uchar','uint'],'thiscall',(p,id)=>{current.order.push('release-share:'+id);return 0;});
    replace(0x5551b0,'void',['pointer'],'thiscall',p=>{current.order.push('remove-self');p.add(0x76).writeS32(-777);});
    replace(0x7e8160,'int',[],'mscdecl',()=>{current.order.push('rand');return current.input.random??41;});
    replace(0x7adf40,'int16',['pointer','uchar'],'thiscall',()=>current.input.count??3);
    replace(0x7ad520,'int',['pointer','pointer','uchar'],'thiscall',(p,out)=>{out.writeS32(3);out.add(4).writeS32(current.input.centerY??0);return 1;});
    replace(0x5ce130,'int',['pointer','pointer','uint','int','pointer','pointer'],'thiscall',(p,r,f,s,pos,a)=>{current.plans.push([f,s,point(pos),point(a)]);return 1;});
    replace(0x5ce280,'int',['pointer','pointer','int','int','uint','int'],'thiscall',(p,c,x,y,f,t)=>{current.draws.push([x,y,f,t]);current.order.push('render');return 1;});
    Interceptor.flush();
    const ctor=new NativeFunction(ptr(0x5348c0),'pointer',['pointer','pointer','pointer','uchar','int','int','int16'],'thiscall');
    const ai=new NativeFunction(ptr(0x534c10),'void',['pointer'],'thiscall');
    const render=new NativeFunction(ptr(0x534dd0),'void',['pointer','pointer','pointer','int'],'thiscall');
    const oldSeed=seed;seed=function(kind,input){oldSeed(kind,input);current.plans=[];current.draws=[];};
    function carrier(color=255,flags=0,anim=0,life=20){const p=alloc(0x29a);ctor(p,string('CallLiH'),string('EFF_P19'),color,flags,anim,life);p.add(0x5c).writeS32(44);p.add(0xea).writePointer(area);p.add(0x76).writeS32(-1);p.add(6).writeS32(160);p.add(10).writeS32(96);p.add(14).writeS32(17);return p;}
    function finish(p){current.state=[p.add(0x298).readS16(),p.add(0x256).readS16(),p.add(0x258).readS16(),p.add(0x290).readS32(),p.add(0x294).readS32()];events.push(current);}
    for(const color of [0,1,73,127,254,255])for(const flags of [0,1,-1])for(const anim of [0,-1]){
      seed('ctor',{color,flags,anim});const p=carrier(color,flags,anim,-32768);current.vtable=p.readPointer().toString();current.height=p.add(0x16a).readU8();finish(p);}
    for(const lifetime of [-32768,-1,0,1,2,20,32767])for(const stop of [0,1,2])for(const failDelete of [false,true])for(const anim of [0,-1])for(const end of [false,true]){
      seed('ai',{lifetime,stop,failDelete,anim,end,length:7});const p=carrier(255,0,anim,lifetime);p.add(0x256).writeS16(3);current.order=[];
      game.add(0x4b40).writeS32(stop);game.add(0x4b44).writeS32(stop===1?43:44);ai(p);finish(p);}
    game.add(0x4b40).writeS32(0);
    for(const input of [{length:0,anim:1},{length:1,anim:1},{length:255,anim:1,random:32767},{missing:true},{foreign:true},{missing:true,anim:1},{foreign:true,anim:1}]){
      seed('target-ai',input);const p=carrier(255,0,input.anim??0,20);p.add(0x76).writeS32(22);target.add(0x12).writePointer(input.foreign?foreign:area);current.order=[];ai(p);finish(p);}
    area.add(0x866).writeS16(128);area.add(0x514).writeS32(0);area.add(0x518).writeS32(0);area.add(0x51c).writeS32(320);area.add(0x520).writeS32(240);area.add(0x550).writeS32(10000);
    for(const count of [0,1,2,3,255,256,257,-1])for(const flags of [0,1,-1])for(const hidden of [false,true]){
      seed('render',{count,flags,hidden});const p=carrier(255,flags);current.order=[];render(p,ptr(0),ptr(1),1);finish(p);}
    for(const input of [{y:-10},{y:2147483647},{y:10001},{centerY:4,count:3},{lockFail:true,count:3},{count:3,z:-17},{count:3,z:2147483647}]){
      seed('render',input);const p=carrier();p.add(10).writeS32(input.y??96);p.add(14).writeS32(input.z??17);current.order=[];render(p,ptr(0),ptr(1),1);finish(p);}
  }catch(e){error=String(e);current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);current.context=e.context;
    current.module=e.address?Process.findModuleByAddress(e.address)?.name:null;current.symbol=e.address?String(DebugSymbol.fromAddress(e.address)):null;
    current.backtrace=e.context?Thread.backtrace(e.context,Backtracer.ACCURATE).map(p=>String(DebugSymbol.fromAddress(p))):[];events.push(current);}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events):
    lines=['// Native colored-strike ctor/AI capture; dependencies isolated.',
           '#ifndef CALL_LIGHTNING_GOLDEN_H_','#define CALL_LIGHTNING_GOLDEN_H_',
           'struct CallLightningAIGolden { SHORT lifetime,stop; BOOL failed,random,end; SHORT after,frame; BOOL retired; };',
           'static const CallLightningAIGolden CALL_LIGHTNING_AI[] = {']
    for e in events:
        if e['kind']!='ai':continue
        c=e['input'];lines.append(f"    {{{c['lifetime']},{c['stop']},{int(c['failDelete'])},{int(bool(c['anim']))},{int(c['end'])},{e['state'][0]},{e['state'][1]},{int('destroy' in e['order'])}}},")
    lines+=['};','struct CallLightningRenderGolden { BYTE count; LONG flags; BOOL hidden; INT draws; };',
            'static const CallLightningRenderGolden CALL_LIGHTNING_RENDER[] = {']
    for e in events:
        if e['kind']=='render' and set(e['input'])=={'count','flags','hidden'} and e['input']['count'] in [0,1,2,3,255]:
            c=e['input'];lines.append(f"    {{{c['count']},{c.get('flags',0)},{int(c.get('hidden',False))},{len(e['draws'])}}},")
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(phase.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored') and len(events)==266
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        c=e['input']
        if e['kind']=='ctor':good=good and e['vtable']=='0x84e8c0' and ('range' in e['order'])==(c['color']!=255) and e['height']==0
        if e['kind']=='target-ai' and c.get('foreign'):good=good and e['order'][-2:]==['remove-self','release-share:22']
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8');report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
