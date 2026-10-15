"""Frozen original particle motion, shared-cell respawn and ember animation ABI.

Real native particle/animation constructors, AI and renderers; rooted isolated
resources, arrays, animation predicates, FX and RNG. No original HP/raster claim.
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

def cases():
    rows=[]
    def add(**values):
        row=dict(life=10,stop=0,end=False,wall=False,collision=0,cloud=0,respawn=0,tick=0,
                 position=[15,11],exact=[15360,11264],velocity=[2048,2048],visible=1,
                 name='ICloudA',flags=[1,0,0,0,0],index=0,seed=1)
        row.update(values);rows.append(row)
    for life in [-32768,-1,0,1,2,32767]:
        for stop in [0,1,2]:
            for end in [False,True]:add(life=life,stop=stop,end=end)
    for pos,exact,velocity in [([15,11],[15360,11264],[2048,2048]),([0,0],[-1,-1],[-2048,-2048]),
                              ([15,11],[15360,11264],[0,0]),([15,11],[15360,11264],[2048,0]),
                              ([2097151,2097151],[2147483647,2147483647],[1,2]),([1,1],[1024,1024],[-2147483648,-2147483648])]:
        for wall in [False,True]:
            for collision in [0,1,2,255]:
                for visible in [0,1]:add(position=pos,exact=exact,velocity=velocity,wall=wall,collision=collision,visible=visible)
    for tick in [-101,-1,0,98,99,2147483647]:
        for cloud in [0,1,2]:
            for end in [False,True]:
                for name in ['ICloudA','ICloudB','FirebaR']:add(tick=tick,cloud=cloud,end=end,name=name)
    for flags in [[1,0,0,0,1],[1,2,255,0,0],[0],[1,0],[1,0,1,1,0]]:
        for seed in [1,2,42,4294967295]:
            for respawn in [0,1,2]:add(flags=flags,seed=seed,respawn=respawn,end=True)
    return rows

JS=strike.JS[:strike.JS.index('    const ctor=new NativeFunction')]+r"""
    replace(0x6a15f0,'pointer',['pointer'],'thiscall',p=>p);
    replace(0x78a990,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{ref(p,s.readCString());return p;});
    replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
    replace(0x7fc984,'void',['pointer'],'mscdecl',()=>{});
    replace(0x56e260,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x56ecf0,'void',['pointer'],'thiscall',()=>current.order.push('retire'));
    replace(0x547a00,'uchar',['pointer','pointer','pointer','pointer','uchar'],'thiscall',(p,pos,t,out)=>{current.tiles.push(point(pos));out.writeS16(0);return current.input.wall?255:0;});
    replace(0x55dbd0,'void',['pointer','pointer'],'thiscall',(p,s)=>{const n=s.readCString();current.overrides.push(n);name(p,0x5f6,n);});
    let rng=1;function random(){rng=(Math.imul(rng,214013)+2531011)>>>0;return(rng>>>16)&32767;}
    replace(0x7e8160,'int',[],'mscdecl',()=>{current.randomCalls++;return random();});
    const table=alloc(0x100);table.add(0x84).writePointer(cb('int',['pointer'],()=>current.input.end?1:0));
    table.add(0x88).writePointer(cb('void',['pointer'],()=>current.increments++));
    table.add(0xc).writePointer(cb('void',['pointer','int16'],(p,d)=>{current.direction=d;}));
    table.add(4).writePointer(cb('void',['pointer','pointer','pointer','int'],(p,r,a,z)=>{r.writeByteArray(new Int32Array([0,0,10,12]).buffer);a.writeS32(3);a.add(4).writeS32(4);}));
    table.add(8).writePointer(cb('void',['pointer','pointer','pointer','pointer','int','int','int'],(p,out,pos,a,z,w,h)=>{
      const [x,y]=point(pos),[cx,cy]=point(a);current.geometry=[x,y,cx,cy,z,w,h];
      out.writeByteArray(new Int32Array([(x-cx)|0,(y-cy+z)|0,(x-cx+w)|0,(y-cy+z+h)|0]).buffer);}));
    table.add(0x90).writePointer(cb('void',['pointer','pointer','pointer','int','pointer','pointer','pointer','uint','uint','pointer','int','int','int','uchar'],(p,inf,mode,s,fx,pos,a,flags,tint,bounds,dither,fade,z,trans)=>{
      current.animationDraw=[...point(pos),...point(a),flags,tint,...Array.from(new Int32Array(bounds.readByteArray(16))),dither,fade,z,trans];}));
    replace(0x470f10,'uint',['pointer','pointer','uchar'],'thiscall',(p,pos,list)=>{current.tintPoint=[...point(pos),list];return 0xffffff;});
    replace(0x5ce930,'int',['pointer','uint','pointer','pointer'],'thiscall',(p,f,r,pos)=>{current.unlock=[f,!r.isNull(),...point(pos)];return 1;});
    function name(d,offset,s){const b=alloc(s.length+2);b.add(1).writeUtf8String(s);d.add(offset+4).writePointer(b.add(1));d.add(offset+8).writeS32(s.length);d.add(offset+12).writeS32(s.length|31);}
    function vec(v){const p=alloc(8);p.writeS32(v[0]);p.add(4).writeS32(v[1]);return p;}
    function descriptor(form,n='FirebaR',mode=0){const d=alloc(0x46);if(n!==null)name(d,0,n);else d.add(8).writeS32(7);d.add(form?0x35:0x2c).writeU8(mode);return d;}
    const oldSeed=seed;seed=function(kind,input){oldSeed(kind,input);current.tiles=[];current.overrides=[];current.increments=0;current.direction=-999;current.randomCalls=0;current.plans=[];current.draws=[];rng=input.seed??1;};
    const ctors=[0x56df00,0x56e280].map(a=>new NativeFunction(ptr(a),'pointer',['pointer','pointer','pointer','pointer','int','pointer','int16','uchar','uchar'],'thiscall'));
    const animCtors=[0x55cd70,0x55d3a0].map(a=>new NativeFunction(ptr(a),'pointer',['pointer','pointer','uint16'],'thiscall'));
    const ai=new NativeFunction(ptr(0x56e650),'void',['pointer'],'thiscall');
    const render=new NativeFunction(ptr(0x56ea90),'void',['pointer','pointer','pointer','int'],'thiscall');
    const advance=new NativeFunction(ptr(0x55d810),'void',['pointer'],'thiscall');
    const draw=new NativeFunction(ptr(0x55d950),'void',['pointer','pointer','pointer','int','pointer','pointer','pointer','uint','uint','pointer','int','int','int','uchar'],'thiscall');
    function rawParticle(input){const p=alloc(0x112),anim=alloc(0x606);p.writePointer(ptr(0x84f14c));p.add(0x7e).writePointer(ptr(0x84f1b8));p.add(0x82).writePointer(anim);anim.writePointer(table);name(anim,0x5f6,input.name??'ICloudA');
      p.add(0x5c).writeS32(44);p.add(0x12).writePointer(area);p.add(6).writeByteArray(new Int32Array(input.position??[160,96]).buffer);p.add(14).writeS32(input.z??17);return p;}
    Interceptor.flush();
    for(const form of [0,1])for(const n of ['', 'ICloudB',null])for(const mode of [0,1,2])for(const count of [0,1,3,255,256,257])for(const facing of [0,65535]){
      seed('anim-ctor',{form,n,mode,count,facing});const d=descriptor(form,n,mode),p=alloc(0x606);d.add(0x36).writeU8(2);animCtors[form](p,d,facing);
      current.state=[p.add(0x5e2).readS16(),p.add(0x5e4).readU16(),p.add(0x5e7).readU8(),p.add(0x5e8).readU8(),p.add(0x5e0).readU8()];
      current.name=p.add(0x5fa).readPointer().isNull()?'':p.add(0x5fa).readPointer().readCString();events.push(current);
    }
    for(const form of [0,1])for(const failAdd of [false,true])for(const life of [-32768,0,7])for(const collision of [0,1,255]){
      seed('ctor',{form,failAdd,life,collision});const p=alloc(0x112),d=descriptor(form),pool=alloc(20);pool.add(16).writeS32(9);if(form){d.add(0x2c).writePointer(pool);d.add(0x30).writeS32(3);d.add(0x34).writeU8(2);d.add(0x44).writeU8(255);}
      ctors[form](p,d,area,vec([-1025,98302]),-17,vec([-1024,2048]),life,255,collision);current.position=point(p.add(6));current.poolReferences=pool.add(16).readS32();current.direction=p.add(0x82).readPointer().add(0x5e4).readU16();events.push(current);
    }
    for(const input of __CASES__){
      seed('ai',input);const p=rawParticle(input),anim=p.add(0x82).readPointer();p.add(0x8c).writeByteArray(new Int32Array([...input.exact,...input.velocity]).buffer);p.add(0x9c).writeS16(input.life);p.add(0x9f).writeU8(input.collision);p.add(0x88).writeS32(input.visible);p.add(0x10c).writeU8(input.respawn);p.add(0x10d).writeU8(input.cloud);p.add(0x10e).writeS32(input.tick);
      const pool=alloc(20),cells=alloc(input.flags.length*10);pool.add(4).writePointer(cells);pool.add(8).writePointer(cells.add(input.flags.length*10));p.add(0x104).writePointer(pool);p.add(0x108).writeS32(input.index);
      input.flags.forEach((flag,i)=>{cells.add(10*i).writeS32(i*1024-1025);cells.add(10*i+4).writeS32(i*2048-1);cells.add(10*i+8).writeU8(flag);cells.add(10*i+9).writeU8(0xab);});
      game.add(0x4b40).writeS32(input.stop);game.add(0x4b44).writeS32(input.stop===1?43:44);ai(p);
      current.state=[...point(p.add(6)),...point(p.add(0x8c)),...point(p.add(0x94)),p.add(0x9c).readS16(),p.add(0x88).readS32(),p.add(0x108).readS32(),p.add(0x10e).readS32()];current.flags=input.flags.map((_,i)=>cells.add(i*10+8).readU8());current.name=anim.add(0x5fa).readPointer().readCString();current.nextRandom=random();events.push(current);
    }
    game.add(0x4b40).writeS32(0);
    for(const count of [0,1,2,255,256,257,-1])for(const sequence of [-32768,-1,0,1,32767])for(const mode of [0,1,2])for(const single of [0,1,2])for(const end of [false,true])for(const shadow of [false,true]){
      seed('advance',{count,sequence,mode,single,end,shadow});const p=alloc(0x606),other=alloc(0xda);other.writePointer(cellTable);other.add(0xc4).writeS16(end?3:0);p.add(0x3fe).writePointer(other);p.add(0x406).writePointer(cellTable);p.add(0x4e0).writePointer(cellTable);p.add(0x406+0xc4).writeS16(7);p.add(0x4e0+0xc4).writeS16(9);if(shadow)p.add(0x402).writePointer(p.add(0x4e0));p.add(0x5e0).writeU8(1);p.add(0x5e2).writeS16(sequence);p.add(0x5e7).writeU8(mode);p.add(0x5e8).writeU8(single);advance(p);
      current.state=[p.add(0x5e2).readS16(),p.add(0x5e0).readU8(),other.add(0xc4).readS16(),p.add(0x406+0xc4).readS16(),p.add(0x406+0xc6).readS16(),p.add(0x4e0+0xc4).readS16(),p.add(0x4e0+0xc6).readS16()];current.nextRandom=random();events.push(current);
    }
    area.add(0x866).writeS16(128);area.add(0x55c).writeS32(80);area.add(0x560).writeS32(80);area.add(0x514).writeS32(0);area.add(0x518).writeS32(0);area.add(0x51c).writeS32(320);area.add(0x520).writeS32(240);
    for(const position of [[-1,96],[0,0],[79,96],[80,96],[399,96],[5120,3840],[5121,96],[160,3841]])for(const visible of [0,1,2])for(const hidden of [false,true])for(const life of [-32768,0,1,255])for(const fade of [0,1,255]){
      seed('render',{position,visible,hidden,life,fade});const p=rawParticle({position});p.add(0x88).writeS32(visible);p.add(0x9c).writeS16(life);p.add(0x9e).writeU8(fade);p.add(0x1a).writeU8(3);render(p,ptr(0),ptr(1),1);events.push(current);
    }
    for(const copy of [0,1,2])for(const transparent of [0,1,2])for(const tint of [0,1,2])for(const extra of [0,1])for(const shadow of [false,true])for(const fade of [false,true]){
      seed('anim-render',{copy,transparent,tint,extra,shadow,fade});const p=alloc(0x606);p.add(0x3fe).writePointer(p.add(0x406));if(shadow)p.add(0x402).writePointer(p.add(0x4e0));p.add(0x5e0).writeU8(1);p.add(0x5ea).writeU8(tint);p.add(0x5eb).writeU8(copy);p.add(0x5ec).writeU8(extra);p.add(0x5ed).writeU8(transparent);p.add(0x5ee).writeS32(73);p.add(0x5f2).writeS32(0x20000);
      const rect=alloc(16),bounds=alloc(16);rect.writeByteArray(new Int32Array([0,0,10,12]).buffer);bounds.writeByteArray(new Int32Array([157,109,167,121]).buffer);draw(p,area.add(0x4cc),ptr(1),1,rect,vec([160,96]),vec([3,4]),0x20000,0x123456,bounds,0,fade?1:0,17,255);events.push(current);
    }
  }catch(e){error=String(e);if(current){current.exception=Object.getOwnPropertyNames(e).map(k=>[k,String(e[k])]);events.push(current);}}
  finally{g.writePointer(old);gc.writePointer(oldc);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:g.readPointer().equals(old)&&gc.readPointer().equals(oldc),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""

def header(events):
    def nums(values):return ','.join(str(int(v)) for v in values)
    def arr(values):return '{'+nums(values)+'}'
    lines=['// Canonical native particle/animation capture; dependencies isolated.','#ifndef HIT_PARTICLE_GOLDEN_H_','#define HIT_PARTICLE_GOLDEN_H_',
      'struct HitParticleGolden { SHORT life,stop; BOOL end,wall; BYTE collision,cloud,respawn; LONG tick,position[2],exact[2],velocity[2],visible; const char* name; INT count,index; BYTE flags[5]; DWORD seed; LONG state[10]; BYTE afterFlags[5]; const char* afterName; BOOL retired; INT increments; SHORT direction; INT randomCalls,nextRandom; };',
      'static const HitParticleGolden HIT_PARTICLE_AI[] = {']
    for e in events:
        if e['kind']!='ai':continue
        c=e['input'];lines.append('    {'+nums([c['life'],c['stop'],c['end'],c['wall'],c['collision'],c['cloud'],c['respawn'],c['tick']])+','+','.join(arr(c[k]) for k in ['position','exact','velocity'])+f",{c['visible']},\"{c['name']}\",{len(c['flags'])},{c['index']},"+arr(c['flags'])+f",{c['seed']}u,"+arr(e['state'])+','+arr(e['flags'])+f",\"{e['name']}\","+nums(['retire' in e['order'],e['increments'],e['direction'],e['randomCalls'],e['nextRandom']])+'},')
    lines+=['};','struct HitAnimationGolden { SHORT count,sequence; BYTE mode,single; BOOL end,shadow; LONG state[7]; INT randomCalls,nextRandom; };','static const HitAnimationGolden HIT_ANIMATION_ADVANCE[] = {']
    for e in events:
        if e['kind']!='advance':continue
        c=e['input'];lines.append('    {'+nums([c[k] for k in ['count','sequence','mode','single','end','shadow']])+','+arr(e['state'])+','+nums([e['randomCalls'],e['nextRandom']])+'},')
    lines+=['};','struct HitRenderGolden { LONG position[2],visible; BOOL hidden; SHORT life; BYTE fade; BOOL draws; LONG draw[14]; };','static const HitRenderGolden HIT_PARTICLE_RENDER[] = {']
    for e in events:
        if e['kind']!='render':continue
        c=e['input'];lines.append('    {'+arr(c['position'])+','+nums([c['visible'],c['hidden'],c['life'],c['fade'],'animationDraw' in e])+','+arr(e.get('animationDraw',[0]*14))+'},')
    lines+=['};','struct HitCtorGolden { BOOL ranged; const char* name; BYTE mode; SHORT count; USHORT facing; SHORT sequence; USHORT direction; BYTE afterMode,single,visible; INT randomCalls; };','static const HitCtorGolden HIT_ANIMATION_CTORS[] = {']
    for e in events:
        if e['kind']!='anim-ctor':continue
        c=e['input']
        if c['n'] is None and c['count']!=0:continue # An absent resource cannot supply sequences in our engine.
        name='NULL' if c['n'] is None else json.dumps(c['n'])
        lines.append('    {'+f"{c['form']},{name},{c['mode']},{c['count']},{c['facing']},"+nums(e['state'])+f",{e['randomCalls']}"+'},')
    lines+=['};','struct HitEmberGolden { BYTE copy,transparent,tint,extra; BOOL shadow,fade; DWORD prep; INT transparency; };','static const HitEmberGolden HIT_EMBER_RENDER[] = {']
    for e in events:
        if e['kind']!='anim-render':continue
        c=e['input'];lines.append('    {'+nums([c[k] for k in ['copy','transparent','tint','extra','shadow','fade']])+','+nums([e['plans'][0][0],e['draws'][-1][3]])+'},')
    lines+=['};','struct HitParticleCtorGolden { BOOL ranged; SHORT life; BYTE collision; LONG position[2]; USHORT direction; LONG references; };','static const HitParticleCtorGolden HIT_PARTICLE_CTORS[] = {']
    for e in events:
        if e['kind']!='ctor' or e['input']['failAdd']:continue
        c=e['input'];lines.append('    {'+nums([c['form'],c['life'],c['collision']])+','+arr(e['position'])+','+nums([e['direction'],e['poolReferences']])+'},')
    return '\n'.join(lines+['};','#endif','']).replace('-2147483648','(-2147483647 - 1)')

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(cases())),encoding='utf-8')
    paths=(exe,Path(__file__),Path(strike.__file__),Path(strike.phase.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored')
    expected=216+36+len(cases())+1260+576+216
    good=good and not any(m.get('type')=='error' for m in result['messages']) and len(events)==expected
    for e in events:
        c=e['input']
        if e['kind']=='ctor':
            good=good and ('destroy' in e['order'])==c['failAdd'] and e['poolReferences']==(10 if c['form'] and not c['failAdd'] else 9)
            if not c['failAdd']:good=good and e['position']==[-2,95]
        if e['kind']=='anim-ctor':good=good and e['name']==(c['n'] or '') and e['state'][1]==c['facing']
        if e['kind']=='anim-render':good=good and e['plans'][0][0]==(0x20100 if not c['copy'] and not c['transparent'] and not c['extra'] and not c['shadow'] else 0x20080) and e['draws'][-1][3]==(73 if c['transparent']==1 else 0)
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    counts={kind:sum(e['kind']==kind for e in events) for kind in {e['kind'] for e in events}}
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),counts=counts,error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
