"""Ground impact fan, scheduling and renderer control from the frozen original.

Native constructor/AI/render execute with rooted isolated area, allocation,
resource, particle and FX dependencies. No original HP or raster claim.
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
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
    replace(0x7fc984,'void',['pointer'],'mscdecl',()=>{});
    replace(0x56cee0,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x56d9b0,'void',['pointer'],'thiscall',()=>current.order.push('retire'));
    replace(0x547a00,'uchar',['pointer','pointer','pointer','pointer','uchar'],'thiscall',(p,pos,t,out)=>{
      current.tiles.push(point(pos));out.writeS16(0);return current.input.wall?255:0;});
    for(const [address,kind] of [[0x56df00,'moving'],[0x56e280,'stationary']])
      replace(address,'pointer',['pointer','pointer','pointer','pointer','int','pointer','int16','uchar','uchar'],'thiscall',(p,d,a,pos,z,v,life,b,c)=>{
        current.particles.push([kind,point(pos),point(v),life,b,c]);return p;});
    const engine=alloc(8),mode=alloc(4),modeVT=alloc(0xc4);chitin.add(0x3c4).writePointer(engine);engine.add(4).writePointer(mode);mode.writePointer(modeVT);
    modeVT.add(0xc0).writePointer(ptr(0x799aa0));
    const ctor=new NativeFunction(ptr(0x56bf30),'pointer',['pointer','pointer','pointer','pointer','pointer','pointer','int16','uchar','uchar','int16'],'thiscall');
    const ai=new NativeFunction(ptr(0x56d0a0),'void',['pointer'],'thiscall');
    const render=new NativeFunction(ptr(0x56d730),'void',['pointer','pointer','pointer','int'],'thiscall');
    const oldSeed=seed;seed=function(kind,input){oldSeed(kind,input);current.tiles=[];current.particles=[];current.plans=[];current.draws=[];};
    function descriptor(n){const d=alloc(n);d.add(0x2e).writeS32(2147483647);return d;}
    function name(d,offset,s,count=0){const b=alloc(s.length+2);b.writeU8(count);b.add(1).writeUtf8String(s);d.add(offset+4).writePointer(b.add(1));d.add(offset+8).writeS32(s.length);d.add(offset+12).writeS32(s.length|31);return b;}
    function carrier(range=200,speed=10,x=160,y=96){const p=alloc(0x2a2),e0=descriptor(0x32),e1=descriptor(0x32),e2=descriptor(0x46),pos=alloc(8);
      e2.add(0x40).writeS32(30);pos.writeS32(x);pos.add(4).writeS32(y);ctor(p,e0,e1,e2,area,pos,range,speed,0,20);return p;}
    function finish(p){current.state=[p.add(0x28c).readS32(),p.add(0x18a).readU8(),p.add(0x29c).readS32(),p.add(0x288).readS32(),p.add(0x8a+0xc4).readS16()];events.push(current);}
    Interceptor.flush();
    for(const range of [1,15,16,17,31,32,33,100,199,200,201,511])for(const speed of [1,10,255])for(const y of [96,-97,2147483647]){
      seed('ctor',{range,speed,y});const p=carrier(range,speed,160,y),n=(p.add(0x198).readS32()+p.add(0x19c).readS32())*4;
      current.fan=Array.from({length:n*4},(_,i)=>p.add(0x1a0).readPointer().add(i*4).readS32());
      current.flags=Array.from(new Uint8Array(p.add(0x1a4).readPointer().readByteArray(n)));
      current.geometry=[p.add(0x18c).readS32(),p.add(0x190).readS32(),p.add(0x198).readS32(),p.add(0x19c).readS32()];finish(p);
    }
    for(const count of [0,1,2,255])for(const loaded of [0,1,2])for(const end of [false,true])for(const stop of [0,1,2]){
      seed('gate',{count,loaded,end,stop});const p=carrier();p.add(0x18a).writeU8(count);p.add(0x2a0).writeU8(loaded);p.add(0x198).writeS32(0);p.add(0x19c).writeS32(0);
      game.add(0x4b40).writeS32(stop);game.add(0x4b44).writeS32(stop===1?43:44);ai(p);finish(p);
    }
    game.add(0x4b40).writeS32(0);
    for(const wall of [false,true])for(const collision of [0,1,2])for(const flag of [0,1,2])for(const velocity of [[17000,19000],[-17000,-19000],[1,-1]]){
      seed('fan',{wall,collision,flag,velocity});const p=carrier();p.add(0x2a0).writeU8(1);p.add(0x18b).writeU8(collision);p.add(0x198).writeS32(1);p.add(0x19c).writeS32(0);
      const f=p.add(0x1a0).readPointer(),flags=p.add(0x1a4).readPointer();f.writeS32(-1025);f.add(4).writeS32(-1367);f.add(8).writeS32(velocity[1]);f.add(12).writeS32(velocity[0]);flags.writeByteArray([flag,2,2,2]);
      ai(p);current.cell=Array.from({length:4},(_,i)=>f.add(i*4).readS32());current.flag=flags.readU8();finish(p);
    }
    for(const density of [-1,0,41,10000])for(const cooldown of [-1,0,1,6])for(const age of [0,10,46340,2147483647])for(const moving of [false,true]){
      seed('spawn',{density,cooldown,age,moving});const p=carrier();p.add(0x2a0).writeU8(1);p.add(0x18a).writeU8(3);p.add(0x198).writeS32(1);p.add(0x19c).writeS32(0);p.add(0x28c).writeS32(age);p.add(0x29c).writeS32(cooldown);
      const f=p.add(0x1a0).readPointer(),flags=p.add(0x1a4).readPointer();f.writeByteArray(Array(64).fill(0));f.add(8).writeS32(-3);f.add(12).writeS32(-1);flags.writeByteArray([0,2,2,2]);
      const pool=alloc(20);p.add(0x26e).writePointer(pool);name(p,0x242,'FirebaR');p.add(0x27a).writeS32(density);p.add(0x27e).writeS32(6);if(moving)name(p,0x210,'RNG_M03');
      ai(p);const start=pool.add(4).readPointer(),last=pool.add(8).readPointer();current.pool=start.isNull()?[]:Array.from({length:last.sub(start).toInt32()/10},(_,i)=>[...point(start.add(i*10)),start.add(i*10+8).readU8()]);finish(p);
    }
    area.add(0x866).writeS16(128);area.add(0x54c).writeS32(10000);area.add(0x550).writeS32(10000);
    for(const loaded of [0,1,2])for(const end of [false,true])for(const hidden of [false,true])for(const lockFail of [false,true])for(const z of [-17,0,17,2147483647]){
      seed('render',{loaded,end,hidden,lockFail,z});const p=carrier();p.add(0x2a0).writeU8(loaded);p.add(14).writeS32(z);p.add(0x298).writeU32(0x20000);render(p,ptr(0),ptr(1),1);finish(p);
    }
    for(const address of [0x7ad0f0,0x7beee0,0x7a8e20])replace(address,'void',['pointer'],'thiscall',()=>{});
    const destroy=new NativeFunction(ptr(0x56cf00),'void',['pointer'],'thiscall');Interceptor.flush();
    for(const count of [0,1,253,254,255]){
      seed('ownership',{count});const p=alloc(0x2a2),e0=descriptor(0x32),e1=descriptor(0x32),e2=descriptor(0x46),pos=alloc(8);
      const blocks=[name(e1,0,'RNG_M03',count),name(e1,16,'RNG_M03',count),name(e2,0,'RNG_M03',count),name(e2,16,'RNG_M03',count)];
      e1.writeU8(0x77);e2.add(16).writeU8(0x88);ctor(p,e0,e1,e2,area,pos,200,10,0,20);
      current.names=[0x214,0x224,0x246,0x256].map((offset,i)=>[p.add(offset).readPointer().equals(blocks[i].add(1)),blocks[i].readU8(),p.add(offset).readPointer().readCString()]);
      current.flags=[p.add(0x210).readU8(),p.add(0x252).readU8()];destroy(p);current.released=blocks.map(b=>b.readU8());events.push(current);
    }
    for(const failAdd of [false,true]){
      seed('registration',{failAdd});const p=carrier();finish(p);
    }
  }catch(e){error=String(e);if(current){current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);events.push(current);}}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""

def header(events):
    lines=['// Canonical ground-impact control capture.','#ifndef GROUND_HIT_GOLDEN_H_','#define GROUND_HIT_GOLDEN_H_',
      'struct GroundFanGolden { SHORT range; BYTE speed; LONG y,geometry[4]; const LONG* fan; const BYTE* flags; };']
    rows=[]
    for i,e in enumerate(events):
        if e['kind']!='ctor':continue
        lines+=['static const LONG GROUND_FAN_'+str(i)+'[] = {'+','.join(map(str,e['fan']))+'};',
                'static const BYTE GROUND_FLAGS_'+str(i)+'[] = {'+','.join(map(str,e['flags']))+'};']
        c=e['input'];rows.append('    {'+f"{c['range']},{c['speed']},{c['y']},"+'{'+','.join(map(str,e['geometry']))+'},GROUND_FAN_'+str(i)+',GROUND_FLAGS_'+str(i)+'},')
    lines+=['static const GroundFanGolden GROUND_FANS[] = {',*rows,'};',
      'struct GroundGateGolden { BYTE count,loaded; BOOL end; SHORT stop; LONG age; BYTE after; SHORT frame; BOOL retired; };',
      'static const GroundGateGolden GROUND_GATES[] = {']
    for e in events:
        if e['kind']!='gate':continue
        c=e['input'];s=e['state'];lines.append('    {'+','.join(map(str,[c['count'],c['loaded'],int(c['end']),c['stop'],s[0],s[1],s[4],int('retire' in e['order'])]))+'},')
    lines+=['};','struct GroundCollisionGolden { BOOL wall; BYTE collision,flag; LONG velocity[2],cell[4]; BYTE afterFlag; BOOL retired; };',
      'static const GroundCollisionGolden GROUND_COLLISIONS[] = {']
    for e in events:
        if e['kind']!='fan':continue
        c=e['input'];lines.append('    {'+f"{int(c['wall'])},{c['collision']},{c['flag']},"+'{'+','.join(map(str,c['velocity']))+'},{'+','.join(map(str,e['cell']))+'},'+f"{e['flag']},{int('retire' in e['order'])}"+'},')
    lines+=['};','struct GroundSpawnGolden { LONG density,cooldown,age; BOOL moving; LONG afterAge,afterCooldown,movingCount; BOOL stationary; INT poolFlag; };',
      'static const GroundSpawnGolden GROUND_SPAWNS[] = {']
    for e in events:
        if e['kind']!='spawn':continue
        c=e['input'];s=e['state'];lines.append('    {'+','.join(map(str,[c['density'],c['cooldown'],c['age'],int(c['moving']),s[0],s[2],s[3],int(any(p[0]=='stationary' for p in e['particles'])),e['pool'][0][2] if e['pool'] else -1]))+'},')
    # MSVC parses bare -2147483648 as an unsigned literal and warns on the
    # aggregate narrowing, even though the stored DWORD is the desired value.
    return '\n'.join(lines+['};','#endif','']).replace('-2147483648','(-2147483647 - 1)')

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(strike.__file__),Path(strike.phase.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored')
    good=good and not any(m.get('type')=='error' for m in result['messages']) and len(events)==108+72+54+128+96+5+2
    for e in events:
        if e['kind']=='ownership':
            n=e['input']['count'];good=good and all(v==[n<254,n+(n<254),'RNG_M03'] for v in e['names']) and e['released']==[n]*4
            # The +0 allocator bytes are seeded from uninitialized VC6 stack
            # bytes by the descriptor ctors, not copied from 0x77/0x88.
        if e['kind']=='render':
            c=e['input'];draw=c['loaded']!=0 and not c['end'] and not c['hidden'] and not c['lockFail']
            good=good and ('render' in e['order'])==draw
            if draw:good=good and e['bounds']==[157,101,167,113] and e['draws']==[[3,0,131072,0]]
        if e['kind']=='registration':good=good and ('destroy' in e['order'])==e['input']['failAdd'] and ('area' in e['order'])!=e['input']['failAdd']
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
