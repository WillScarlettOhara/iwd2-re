"""Canonical skills/feats panel resets and signed ability constraints.

Targets, skill costs/values, feat values/prerequisites/selection and ID parsing
run unchanged on private stats and numeric tables. Panel/control operations,
help, token/CString services, effects and race/class label services are explicit
doubles. Typed replay uses real GUIREC controls/tables and defers effects;
this does not establish effect recalculation or complete level-up finalization.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
from projectile_aim_reference import MD5, capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO, game_directory


def cases():
    panels=[dict(kind=k,profile=p,mapping=m,flag=f,budget=b)
            for k in [0,1] for p in range(4) for m in range(2)
            for f in [0,1,-1,2] for b in [-2,0,1,2]]
    abilities=[dict(kind=2,raceMin=r,raceMax='17ignored',adj=str(a),classMin=c)
               for r in ['0','1','3','255','256','-1']
               for a in [-258,-2,0,2,257,2147483647,-2147483648]
               for c in ['0','1','5','255']]
    return panels+abilities


JS=r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],events=[];
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a){const p=ptr(a);globals.push([p,p.readU32()]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16)))]);Interceptor.replace(p,f);}
 function str(s){const p=Memory.allocUtf8String(s);roots.push(p);return p;}
 function string(s){const p=alloc(4);p.writePointer(str(s));return p;}
 function array(p,n){return Array.from({length:n},(_,i)=>p.add(4*i).readS32());}
 function table(p,w,h,values){const data=alloc(w*h*4);values.forEach((v,i)=>data.add(i*4).writePointer(str(String(v))));p.add(0x18).writePointer(data);p.add(0x1c).writePointer(str('0'));p.add(0x20).writeS16(w);p.add(0x22).writeS16(h);}
 const screen=alloc(0x2000),actor=alloc(0x9e00),game=alloc(0x6000),chitin=alloc(0x6000),panel=alloc(0x80),vft=alloc(16),token=alloc(4);
 const buttons=Array.from({length:18},()=>alloc(64));buttons.forEach(p=>p.writePointer(vft));
 const skills=new NativeFunction(ptr(0x5f7850),'void',['pointer','pointer','pointer','int'],'thiscall');
 const feats=new NativeFunction(ptr(0x5da200),'void',['pointer','pointer','pointer','int'],'thiscall');
 const ability=new NativeFunction(ptr(0x5d9610),'int',['pointer','pointer','pointer','pointer','pointer','pointer','pointer','pointer','pointer'],'thiscall');
 const names=['MIN','MAX','MOD'].map(string),outputs=alloc(12);
 let c=null,e=null,error=null;
 try{
  save(0x8cf6dc).writePointer(chitin);chitin.add(0x1c54).writePointer(game);save(0x8f333c).writePointer(str('number'));
  replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(n,f,s)=>{throw Error('assert '+n+' '+s.readUtf8String());});
  replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
  replace(0x7faee8,'void',['pointer','pointer','int'],'mscdecl',(out,fmt,n)=>{if(fmt.readUtf8String()!=='%d')throw Error('format');out.writePointer(str(String(n)));});
  replace(0x7fc1a5,'pointer',['pointer','pointer'],'thiscall',(p,key)=>{if(key.readUtf8String()!=='number')throw Error('token key');return token;});
  replace(0x7fcd57,'pointer',['pointer','pointer'],'thiscall',(p,text)=>{e.token=text.readUtf8String();return p;});
  replace(0x5e6840,'void',['pointer','uint','uint','uint'],'thiscall',(p,id,control,ref)=>{e.help=[id,control,ref];});
  replace(0x72de60,'int',['pointer'],'thiscall',p=>{e.effects++;p.add(0x5c0).writeS16(-10);p.add(0x5c2).writeU16(12);p.add(0x924).writeS16(-14);p.add(0x7234).writeS32(16);return 1;});
  replace(0x4d2ca0,'pointer',['pointer','uint'],'thiscall',(p,id)=>{if(!p.equals(panel)||id<14||id>31)throw Error('control');e.order.push(id);return buttons[id-14];});
  replace(0x4d57c0,'void',['pointer','int'],'thiscall',(p,b)=>{const i=buttons.findIndex(x=>x.equals(p));if(i<0)throw Error('button');e.enabled[i]=b;});
  const active=new NativeCallback((p,b)=>{const i=buttons.findIndex(x=>x.equals(p));if(i<0)throw Error('active');e.active[i]=b;},'void',['pointer','uint8'],'thiscall');callbacks.push(active);vft.add(4).writePointer(active);
  replace(0x541940,'pointer',['pointer','pointer','uint8','uint8'],'thiscall',(p,out)=>{out.writePointer(str('HUMAN'));return out;});
  replace(0x541da0,'pointer',['pointer','pointer','int','uint'],'thiscall',(p,out)=>{out.writePointer(str('FIGHTER'));return out;});
  replace(0x403340,'pointer',['pointer','pointer','pointer'],'thiscall',(p,x,y)=>{
   const col=x.readPointer().readUtf8String();let value;
   if(p.equals(game.add(0x184)))value=col==='MIN'?c.raceMin:c.raceMax;
   else if(p.equals(game.add(0x1a8)))value=c.adj;
   else if(p.equals(game.add(0x1cc)))value=c.classMin;
   else throw Error('ability table');return string(value);
  });Interceptor.flush();
  const inputs=__CASES__;
  for(let i=0;i<inputs.length;i++){
   c=inputs[i];actor.writeByteArray(new Uint8Array(0x9e00));actor.add(0x962).writeU32(16);actor.add(0x966).writeU8(4);actor.add(0x96b).writeU8(4);
   if(c.kind===2){outputs.writeByteArray(new Uint8Array(12).fill(165));const result=ability(screen,actor,...names,outputs,outputs.add(1),outputs.add(2),outputs.add(4));events.push({op:'allocation-result',case:i,kind:2,result,ability:outputs.readU8(),minimum:outputs.add(1).readU8(),maximum:outputs.add(2).readU8(),modifier:outputs.add(4).readS32()});continue;}
   const n=c.kind?75:16,top=c.kind?0x64a:0x2ba,budget=c.kind?0x646:0x798,stored=c.kind?0x650:0x2c0;
   screen.writeByteArray(new Uint8Array(0x2000));screen.add(0x106).writeU8(5);screen.add(c.kind?0x64c:0x2bc).writeS32(c.budget);screen.add(budget).writeS32(c.budget-1);screen.add(top).writeU16(12);
   for(let j=0;j<n;j++)screen.add(stored+4*j).writeS32(900+j);
   for(let j=0;j<16;j++)actor.add(0x7b4+j).writeU8(c.profile===3?[0,1,127,255][j%4]:j%3);
   actor.add(0x75c).writeByteArray(new Uint8Array(12).fill(255));actor.add(0x774).writeByteArray(new Uint8Array(26).fill(255));
   for(const id of c.profile===1?[6]:c.profile===2?[34]:c.profile===3?[6,12,34,69]:[]){const p=actor.add(0x75c+4*(id>>>5));p.writeU32((p.readU32()&~(1<<(id&31)))>>>0);if(id===69)actor.add(0x780).writeU8(0);}
   table(game.add(0xac),1,75,Array.from({length:75},(_,j)=>c.mapping?74-j:j));table(game.add(0xd0),1,16,Array.from({length:16},(_,j)=>c.mapping?15-j:j));
   table(game.add(0xd78),11,16,Array.from({length:176},(_,j)=>j%11!==4?0:c.profile===0?0:c.profile===1?1:c.profile===2?(Math.floor(j/11)<10?0:2):1+Math.floor(j/11)%2));
   actor.add(0x5c0).writeS16(-32768);actor.add(0x5c2).writeU16(65535);actor.add(0x924).writeS16(32767);actor.add(0x7234).writeS32(-77);
   panel.add(0x20).writeU32(c.kind?56:55);e={enabled:Array(18).fill(-1),active:Array(18).fill(-1),order:[],effects:0};
   (c.kind?feats:skills)(screen,panel,actor,c.flag);
   if(JSON.stringify(e.order)!==JSON.stringify(Array.from({length:18},(_,j)=>j+14)))throw Error('control order');
   if(JSON.stringify(e.help)!==JSON.stringify([c.kind?56:55,92,c.kind?36476:17248]))throw Error('help receipt');
   events.push({op:'allocation-result',case:i,kind:c.kind,top:screen.add(top).readU16(),budget:screen.add(budget).readS32(),stored:array(screen.add(stored),n),enabled:e.enabled.reduce((v,b,j)=>v|(b<<j),0),active:e.active.reduce((v,b,j)=>v|(b<<j),0),effects:e.effects,dirty:actor.add(0x562c).readS32(),hp:[actor.add(0x5c0).readS16(),actor.add(0x5c2).readU16(),actor.add(0x924).readS16(),actor.add(0x7234).readS32()],token:e.token});
  }
 }catch(ex){error=String(ex);}
 finally{for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,v]of globals)p.writeU32(v);}
 return{events,error,hooksRestored:patches.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(16))))===JSON.stringify(b)),globalsRestored:globals.every(([p,v])=>p.readU32()===v)};
};
"""


def header(inputs,events):
    def integer(n):return '(-2147483647-1)' if n==-2147483648 else str(n)
    lines=['// Generated by allocation_reference.py from canonical IWD2.exe.',
           '#ifndef ALLOCATION_GOLDEN_H_','#define ALLOCATION_GOLDEN_H_',
           'struct AllocationGolden { INT kind,profile,mapping,flag,budget,top,remaining; INT stored[75]; DWORD enabled,active; };',
           'static const AllocationGolden ALLOCATION_CASES[] = {']
    for c,e in zip(inputs,events):
        if c['kind']==2:continue
        values=[c[k] for k in ['kind','profile','mapping','flag','budget']]+[e['top'],e['budget']]
        lines.append('    {'+','.join(map(integer,values))+',{'+','.join(map(integer,e['stored']))+'},'+str(e['enabled'])+'u,'+str(e['active'])+'u},')
    lines+=['};','struct AbilityConstraintGolden { const char *raceMin,*raceMax,*adj,*classMin; INT result,ability,minimum,maximum,modifier; };',
            'static const AbilityConstraintGolden ABILITY_CONSTRAINT_CASES[] = {']
    for c,e in zip(inputs,events):
        if c['kind']!=2:continue
        lines.append('    {'+','.join(json.dumps(c[k]) for k in ['raceMin','raceMax','adj','classMin'])+','+
                     ','.join(integer(e[k]) for k in ['result','ability','minimum','maximum','modifier'])+'},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args()
    import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);inputs=cases();script=args.out/'original.js'
    script.write_text(JS.replace('__CASES__',json.dumps(inputs)),encoding='utf-8');(args.out/'inputs.json').write_text(json.dumps(inputs),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script,args.out/'inputs.json']
    hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths};result=capture(frida,exe,game_directory(),script,90)
    events=result['events'];good=not result['error'] and not result['messages'] and not result['cleanup_errors'] and result['hooksRestored'] and result['globalsRestored']
    good=good and len(events)==len(inputs) and all(e['case']==i and e['kind']==c['kind'] for i,(e,c) in enumerate(zip(events,inputs)))
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8');report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');(args.out/'original.jsonl').write_text(''.join(json.dumps(e)+'\n' for e in events),encoding='utf-8')
    if good and args.header:args.header.write_text(header(inputs,events),encoding='utf-8')
    print(json.dumps(report,indent=2));return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
