"""Canonical phase BAM and opcode 215 dispatch, with isolated dependencies.

The original remains suspended. Native constructors, AI, Fire, render planning
and dispatcher execute; resources, animation predicates, arrays, sound and FX
use rooted callbacks. This captures control/ABI, not original HP or raster.
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
  const callbacks=[],addresses=[],before=[],memory=[],events=[];
  function alloc(n){const p=Memory.alloc(n);memory.push(p);return p;}
  function text(s){const p=alloc(12+s.length+1);p.writeS32(1);p.add(4).writeS32(s.length);p.add(8).writeS32(s.length);p.add(12).writeUtf8String(s);return p.add(12);}
  function string(s){const p=alloc(4);p.writePointer(text(s));return p;}
  function str(p){return p.readPointer().readCString();}
  function ref(p,s){const b=Array.from(s.toUpperCase().slice(0,8),c=>c.charCodeAt(0));while(b.length<8)b.push(0);p.writeByteArray(b);}
  function res(p){return Array.from(new Uint8Array(p.readByteArray(8))).map(c=>String.fromCharCode(c)).join('').split('\0')[0];}
  function point(p){return [p.readS32(),p.add(4).readS32()];}
  function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
  function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);addresses.push(p);
    before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));Interceptor.replace(p,f);}
  const g=ptr(0x8cf6dc),gc=ptr(0x8cf6d8),old=g.readPointer(),oldc=gc.readPointer();
  const chitin=alloc(0x6000),game=alloc(0x6000),area=alloc(0xb20),resource=alloc(0x100),colors=alloc(1024),cellTable=alloc(4),actorTable=alloc(0x28),fakeTable=alloc(0x8c);
  let current=null,error=null;
  function seed(kind,input){current={kind,input,order:[],refs:[],fires:[],constructors:[],messages:[]};}
  try {
    if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000)))throw Error('Wrong image');
    chitin.add(0x1c54).writePointer(game);g.writePointer(chitin);gc.writePointer(chitin);
    replace(0x4c7b30,'pointer',['pointer'],'thiscall',p=>p);
    replace(0x5371a0,'pointer',['pointer','int'],'thiscall',p=>p);
    replace(0x7a8bb0,'pointer',['pointer'],'thiscall',p=>p);
    replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
    replace(0x7b2a40,'pointer',['pointer'],'thiscall',p=>p);
    cellTable.writePointer(cb('int',['pointer'],p=>{current.order.push('advance');p.add(0xc4).writeS16(p.add(0xc4).readS16()+1);return 1;}));
    replace(0x7acd70,'pointer',['pointer'],'thiscall',p=>{p.writePointer(cellTable);return p;});
    replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
    replace(0x7fcc88,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(text(s.readCString()));return p;});
    replace(0x7fcd57,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(text(s.readCString()));return p;});
    replace(0x7fcd07,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(s.readPointer());return p;});
    replace(0x7fce63,'pointer',['pointer','pointer','pointer'],'stdcall',(out,s,t)=>{out.writePointer(text(str(s)+t.readCString()));return out;});
    replace(0x7fd114,'void',['pointer','int','char'],'thiscall',(s,i,c)=>s.readPointer().add(i).writeU8(c&255));
    replace(0x78a970,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{ref(p,str(s));return p;});
    replace(0x78acc0,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,str(s));out.writeByteArray(p.readByteArray(8));return out;});
    replace(0x78ad40,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,s.readCString());out.writeByteArray(p.readByteArray(8));return out;});
    replace(0x78ac30,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{p.writeByteArray(s.readByteArray(8));out.writeByteArray(p.readByteArray(8));return out;});
    replace(0x78abb0,'int',['pointer','pointer'],'thiscall',(p,s)=>res(p)===s.readCString().toUpperCase()?1:0);
    replace(0x78ab40,'int',['pointer','pointer'],'thiscall',(p,s)=>res(p)!==s.readCString().toUpperCase()?1:0);
    for(const address of [0x4428a0,0x442990])replace(address,'void',['pointer','pointer','int','int'],'thiscall',(p,s,a,w)=>{
      p.add(4).writePointer(resource);p.add(8).writeByteArray(s.readByteArray(8));current.refs.push([res(s),a,w]);});
    replace(0x786df0,'pointer',['pointer','pointer','uint16','int'],'thiscall',(p,s,type,w)=>{current.order.push('resource:'+res(s)+':'+type);return resource;});
    replace(0x77e610,'void',['pointer'],'thiscall',()=>current.order.push('request'));
    replace(0x77e370,'void',['pointer'],'thiscall',()=>current.order.push('cancel'));
    replace(0x7ae290,'int',['pointer','int16'],'thiscall',(p,s)=>{p.add(0xc6).writeS16(s);current.order.push('sequence');return 1;});
    replace(0x7adff0,'int',['pointer','int16'],'thiscall',(p,s)=>{p.add(0xc4).writeS16(s);current.order.push('frame:'+s);return 1;});
    replace(0x7ae010,'int',['pointer','uchar'],'thiscall',()=>current.input.end?1:0);
    replace(0x7adae0,'uchar',['pointer','uint16','uchar'],'thiscall',()=>current.input.length??7);
    replace(0x529fb0,'void',['pointer'],'thiscall',()=>current.order.push('arrival'));
    replace(0x7aa5e0,'void',['pointer','int','int','int'],'thiscall',(p,x,y,z)=>current.order.push('coordinates:'+x+','+y+','+z));
    replace(0x5356a0,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x52a1a0,'void',['pointer'],'thiscall',()=>current.order.push('deliver'));
    replace(0x59a0f0,'uchar',['pointer','pointer','pointer','uint'],'thiscall',(p,out,o)=>{current.order.push('add');out.writeS32(44);return current.input.failAdd?2:0;});
    replace(0x52a640,'void',['int','pointer','uchar'],'mscdecl',(id,p,facing)=>{current.order.push('start:'+id+':'+facing);p.writeS32(160);p.add(4).writeS32(current.input.y??96);});
    replace(0x4c7be0,'void',['pointer','pointer','pointer','int','uchar'],'thiscall',(p,a,pos,z,list)=>{current.order.push('area');p.add(6).writeS32(pos.readS32());p.add(10).writeS32(pos.add(4).readS32());p.add(14).writeS32(z);p.add(0x12).writePointer(a);p.add(0x1a).writeU8(list);});
    replace(0x7aa910,'int',['pointer'],'thiscall',()=>{current.order.push('stop');return 1;});
    replace(0x7aa690,'int',['pointer','int'],'thiscall',()=>{current.order.push('loop');return 1;});
    replace(0x7aa4b0,'int',['pointer','int','uint'],'thiscall',(p,c)=>{current.order.push('channel:'+c);return 1;});
    replace(0x7a9db0,'int',['pointer','int','int','int','int'],'thiscall',(p,x,y,z,loop)=>{current.order.push('play:'+x+','+y+','+z+','+loop);return 1;});
    replace(0x553c00,'uchar',['pointer','int16'],'thiscall',(p,t)=>{current.tile=t;return current.input.hidden?0:1;});
    replace(0x470d80,'int',['pointer','pointer','uchar'],'thiscall',()=>5);
    replace(0x7ad520,'int',['pointer','pointer','uchar'],'thiscall',(p,out)=>{out.writeS32(3);out.add(4).writeS32(4);return 1;});
    replace(0x7ad800,'int',['pointer','pointer','uchar'],'thiscall',(p,out)=>{out.writeS32(10);out.add(4).writeS32(12);return 1;});
    replace(0x5ce130,'int',['pointer','pointer','uint','int','pointer','pointer'],'thiscall',(p,r,f,s,pos,anchor)=>{current.plan=[f,s,point(pos),point(anchor)];return 1;});
    replace(0x5ce1f0,'int',['pointer','pointer','uint'],'thiscall',()=>current.input.lockFail?0:1);
    replace(0x77ec60,'pointer',['pointer'],'thiscall',p=>{current.order.push('demand');return p;});
    replace(0x77ecc0,'int',['pointer'],'thiscall',()=>256);
    replace(0x77ece0,'pointer',['pointer'],'thiscall',()=>colors);
    replace(0x7ae2b0,'void',['pointer','pointer','int','uint16'],'thiscall',()=>current.order.push('palette'));
    replace(0x77f190,'int',['pointer'],'thiscall',()=>{current.order.push('release');return 0;});
    replace(0x5ce280,'int',['pointer','pointer','int','int','uint','int'],'thiscall',(p,c,x,y,f,t)=>{current.draw=[x,y,f,t];current.order.push('render');return 1;});
    replace(0x5ce350,'int',['pointer','int','int','int','pointer','pointer','int','uint'],'thiscall',(p,x,y,z,a,b)=>{current.bounds=[b.readS32(),b.add(4).readS32(),b.add(8).readS32(),b.add(12).readS32()];current.order.push('clip');return 1;});
    replace(0x5ce930,'int',['pointer','uint','pointer','pointer'],'thiscall',()=>{current.order.push('unlock');return 1;});
    replace(0x5ce0a0,'int',['pointer','int','pointer','int','int','int','int','uint'],'thiscall',()=>{current.order.push('blit');return 1;});
    Interceptor.flush();
    const ctor=new NativeFunction(ptr(0x535320),'pointer',['pointer','pointer','pointer','pointer','uint','uchar'],'thiscall');
    const ai=new NativeFunction(ptr(0x5357b0),'void',['pointer'],'thiscall');
    const fire=new NativeFunction(ptr(0x535d20),'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');
    const render=new NativeFunction(ptr(0x535a40),'void',['pointer','pointer','pointer','int'],'thiscall');
    function carrier(palette='',flags=0,t=0){const p=alloc(0x332);ctor(p,string('DDoorH'),string(''),string(palette),flags,t);p.add(0x5c).writeS32(44);p.add(0xea).writePointer(area);p.add(6).writeS32(160);p.add(10).writeS32(96);p.add(14).writeS32(17);return p;}
    function finish(p){current.state=[p.add(0x330).readU8(),p.add(0x256).readS16(),p.add(0x326).readU32(),p.add(0x32a).readU8(),p.add(0x32b).readU8()];events.push(current);}
    for(const palette of ['', 'SHAir7'])for(const flags of [0,8,0x80000000])for(const t of [0,255]){seed('ctor',{palette,flags,t});const p=carrier(palette,flags,t);current.vtable=p.readPointer().toString();current.base=str(p.add(0x32c));current.height=p.add(0x16a).readU8();finish(p);}
    for(const phase of [0,1,2,3,4,5,6,255])for(const frame of [0,6,-32768])for(const end of [false,true])for(const stop of [0,1,2]){
      seed('ai',{phase,frame,end,stop,length:7});const p=carrier();current.order=[];current.refs=[];p.add(0x330).writeU8(phase);p.add(0x256).writeS16(frame);
      game.add(0x4b40).writeS32(stop);game.add(0x4b44).writeS32(stop===1?43:44);ai(p);finish(p);}
    game.add(0x4b40).writeS32(0);
    for(const failAdd of [false,true])for(const sound of ['', 'ARE_M06'])for(const loop of [false,true])for(const y of [96,-2147483648]){
      seed('fire',{failAdd,sound,loop,y});const p=carrier();ref(p.add(0x152),sound);p.add(0x15a).writeS32(loop?1:0);current.order=[];fire(p,area,11,22,256,400,31,13);current.point=point(p.add(6));current.z=p.add(14).readS32();finish(p);}
    area.add(0x866).writeS16(128);area.add(0x514).writeS32(0);area.add(0x518).writeS32(0);area.add(0x51c).writeS32(320);area.add(0x520).writeS32(240);
    for(const flags of [0,8,0x80000000])for(const palette of ['', 'SHAir7'])for(const t of [0,255])for(const gate of [0,1,2,3]){
      seed('render',{flags,palette,t,hidden:gate===1,lockFail:gate===3});const p=carrier(palette,flags,t);current.order=[];
      area.add(0x55c).writeS32(gate===2?10000:0);area.add(0x560).writeS32(0);render(p,ptr(0),ptr(1),1);finish(p);}
    fakeTable.writePointer(cb('pointer',['pointer','uint'],p=>{current.order.push('destroy');return p;}));
    const addVisual=new NativeFunction(ptr(0x530510),'void',['pointer','pointer','int','int'],'thiscall');
    for(const failAdd of [false,true])for(const sound of ['', 'ARE_M06'])for(const loop of [false,true]){
      seed('boom-add',{failAdd,sound,loop});const p=alloc(0x2a0);p.writePointer(fakeTable);ref(p.add(0x152),sound);p.add(0x15a).writeS32(loop?1:0);
      addVisual(p,area,160,96);current.point=point(p.add(6));current.z=p.add(14).readS32();events.push(current);}
    // Dispatcher dependencies are isolated separately; its native selection,
    // constructor arguments, parallel message and virtual launch still execute.
    fakeTable.add(0x6c).writePointer(cb('void',['pointer','pointer','int','int','int','int','int','int16'],(p,a,s,t,x,y,z,k)=>current.fires.push([s,t,x,y,z,k])));
    fakeTable.add(0x88).writePointer(cb('void',['pointer','pointer','int','int'],(p,a,x,y)=>current.fires.push(['visual',x,y])));
    actorTable.add(0x1c).writePointer(cb('pointer',['pointer'],p=>p.add(6)));
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
    replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',()=>current.order.push('assert'));
    replace(0x530300,'pointer',['pointer'],'thiscall',p=>{p.writePointer(fakeTable);current.constructors.push(['boom']);return p;});
    replace(0x52b150,'void',['pointer','uint16','uint16'],'thiscall',(p,r,v)=>current.order.push('range:'+r+':'+v));
    replace(0x5348c0,'pointer',['pointer','pointer','pointer','uchar','int','int','int16'],'thiscall',(p,v,s,c,f,a,l)=>{p.writePointer(fakeTable);current.constructors.push(['lightning',str(v),str(s),c,f,a,l]);return p;});
    for(const [address,kind] of [[0x535320,'phase'],[0x535f20,'impact']])replace(address,'pointer',['pointer','pointer','pointer','pointer','uint','uchar'],'thiscall',(p,v,s,c,f,t)=>{p.writePointer(fakeTable);current.constructors.push([kind,str(v),str(s),str(c),f,t]);return p;});
    replace(0x529f40,'int',['pointer','pointer'],'thiscall',()=>19);
    replace(0x500470,'pointer',['pointer','uint16','int','pointer','int','int','int','int16'],'thiscall',(p,t,target,pos,z,s,d,k)=>{current.messages.push([t,target,point(pos),z,s,d,k]);return p;});
    replace(0x4f7500,'int',['pointer','pointer','int'],'thiscall',()=>{current.order.push('message');return 1;});
    Interceptor.flush();
    const dispatch=new NativeFunction(ptr(0x4bbcb0),'int',['pointer','pointer'],'thiscall');
    for(const flag of [...Array(40).keys(),40,41,42,0x400,0x401,0x402,0xfffffffc,0xfffffffe])for(const missing of [false,true]){
      seed('dispatch',{flag,missing});const e=alloc(0x114),actor=alloc(0x160);e.add(0x1c).writeU32(flag);actor.writePointer(actorTable);actor.add(6).writeS32(160);actor.add(10).writeS32(96);actor.add(0x5c).writeS32(22);actor.add(0x12).writePointer(missing?ptr(0):area);
      current.pointerType=actor.toUInt32()&65535;current.result=dispatch(e,actor);current.done=e.add(0x110).readS32();events.push(current);}
  }catch(e){error=String(e);}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events):
    lines=['// Canonical phase-carrier capture; resource and animation callbacks isolated.',
           '#ifndef PHASE_BAM_GOLDEN_H_', '#define PHASE_BAM_GOLDEN_H_',
           'struct PhaseBAMGateGolden { BYTE phase; SHORT frame; BOOL end; SHORT stop; BYTE after; SHORT nextFrame; BOOL arrival; };',
           'static const PhaseBAMGateGolden PHASE_BAM_GATE[] = {']
    for e in events:
        if e['kind']=='ai':
            c=e['input'];lines.append(f"    {{{c['phase']},{c['frame']},{int(c['end'])},{c['stop']},{e['state'][0]},{e['state'][1]},{int('arrival' in e['order'])}}},")
    lines+=['};','struct PhaseBAMRenderFlagsGolden { DWORD input,output; };',
            'static const PhaseBAMRenderFlagsGolden PHASE_BAM_RENDER_FLAGS[] = {']
    seen=set()
    for e in events:
        if e['kind']=='render' and e.get('plan') and e['input']['flags'] not in seen:
            seen.add(e['input']['flags']);lines.append(f"    {{{e['input']['flags']}u,{e['plan'][0]}u}},")
    lines+=['};','struct VisualSpellHitGolden { DWORD flag; const char* kind; const char* visual; const char* palette; USHORT type; BOOL pointerType; };',
            'static const VisualSpellHitGolden VISUAL_SPELL_HIT[] = {']
    for e in events:
        if e['kind']!='dispatch' or e['input']['missing'] or 'assert' in e['order']:continue
        c=e['constructors'][0];kind=c[0];visual=c[1] if len(c)>1 else 'SPBOOM';pal=c[3] if kind in ('phase','impact') else ''
        # Discard synthetic palette strings containing embedded NUL/high bytes.
        if any(ord(ch)<32 or ord(ch)>126 for ch in pal):continue
        pointer=e['input']['flag']>39
        lines.append(f"    {{{e['input']['flag']}u,\"{kind}\",\"{visual}\",\"{pal}\",{e['messages'][0][0] if e['messages'] and not pointer else 0},{int(pointer)}}},")
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0]);ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path)
    args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored') and len(events)==324
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        c=e['input']
        if e['kind']=='ctor':good=good and e['vtable']=='0x84e940' and e['base']=='DDoorH' and e['height']==0 and e['refs'][0][0]=='DDOORH2'
        elif e['kind']=='fire':
            good=good and e['order'][:2]==['deliver','add'] and ('destroy' in e['order'])==c['failAdd']
            if not c['failAdd']:
                y=((c['y']-1+2**31)%2**32)-2**31
                good=good and e['point']==[160,y] and e['z']==0 and 'start:22:0' in e['order']
                good=good and ('channel:15' in e['order'])==bool(c['sound']) and ('loop' in e['order'])==(bool(c['sound']) and c['loop'])
        elif e['kind']=='boom-add':good=good and e['point']==[160,96] and ('destroy' in e['order'])==c['failAdd']
        elif e['kind']=='render':
            good=good and e['state'][2]==c['flags']
            if e.get('plan'):good=good and e['plan'][0]==(c['flags']|128 if c['flags'] else 257)
        elif e['kind']=='dispatch':
            good=good and e['result']==e['done']==1 and len(e['constructors'])==(0 if c['missing'] else 1)
            if c['flag']>39 and not c['missing']:good=good and e['messages'][0][0]==e['pointerType']
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],original_md5=MD5,fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
