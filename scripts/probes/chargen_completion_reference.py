"""Frozen original character completion: racial flags/grants, spell refill and equipment.

Target bodies, RemoveItems, catalogue lookup and spell count helpers execute.
Resource/GUI/equipment dependencies are captured receipts, not combat damage proof.
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


def cases():
    rows = []
    for race, subs in [(1,3),(2,3),(3,1),(4,3),(5,3),(6,2),(7,1)]:
        for sub in range(subs):
            for table in range(3):
                for seed in range(2):
                    rows.append(dict(mode=0,race=race,sub=sub,table=table,seed=seed,gold=17,max=3,bonus=2,cls=1,buttons=1,slot=43))
    for seed in range(3):
        for gold in [0,17]:
            for maximum in [0,3]:
                rows.append(dict(mode=1,race=6,sub=1,table=0,seed=seed,gold=gold,max=maximum,bonus=2,cls=1,buttons=1,slot=43))
    for cls in [1,6]:
        for buttons in [0,1,3]:
            for slot in [-1,10,11,14,15,43]:
                rows.append(dict(mode=2,race=1,sub=0,table=0,seed=0,gold=17,max=3,bonus=2,cls=cls,buttons=buttons,slot=slot))
    return rows


def getter_cases():
    rows=[dict(mode=0,parsed=p,demands=d,type=t,index=i)
          for p in [0,1] for d in [1,7] for t in [0,1,2,0x100,0x101,0xffff]
          for i in [-1,0,1,2,0x7fffffff]]
    rows += [dict(mode=m,parsed=1,demands=1,type=1,index=i) for m in [1,2] for i in [0,1]]
    return rows


JS = r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],events=[],getterEvents=[];let c=null,e=null,error=null;
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function text(s){const p=alloc(s.length+13);p.writeS32(1);p.add(4).writeS32(s.length);p.add(8).writeS32(s.length);p.add(12).writeUtf8String(s);return p.add(12);}
 function str(p){return p.readPointer().readUtf8String();}
 function ref(p,s){const b=Array.from(s.slice(0,8),x=>x.charCodeAt(0));while(b.length<8)b.push(0);p.writeByteArray(b);}
 function res(p){return Array.from(new Uint8Array(p.readByteArray(8))).map(x=>String.fromCharCode(x)).join('').split('\0')[0];}
 function save(a,n){const p=ptr(a);globals.push([p,p.readByteArray(n)]);return p;}
 function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16))).join(',')]);Interceptor.replace(p,f);}
 const actor=alloc(0x9e00),game=alloc(0x6000),chitin=alloc(0x6000),vtable=alloc(0x100),catalogue=alloc(16),screen=alloc(0x2000);
 let innateData=null;const names=['ZRCHIT','SPIN231'];names.forEach((s,i)=>ref(catalogue.add(i*8),s));
 const racial=new NativeFunction(ptr(0x722820),'void',['pointer'],'thiscall');
 const wrap=new NativeFunction(ptr(0x613310),'void',['pointer','pointer'],'thiscall');
 const skills=new NativeFunction(ptr(0x612de0),'void',['pointer','pointer'],'thiscall');
 function list(p,n,seed){const data=alloc(n*16+128);p.add(4).writePointer(data);p.add(8).writePointer(data.add(n*16));p.add(12).writePointer(data.add(n*16+128));
   p.add(16).writeU32(seed);p.add(20).writeU32(9+seed);p.add(24).writeU32(19+seed);
   for(let j=0;j<n;j++){const d=data.add(j*16);d.writeU32(j);d.add(4).writeU32(j===0?0:3+seed);d.add(8).writeU32(j+seed);d.add(12).writeU32(29+seed+j);}return data;}
 function digest(){let h=2166136261;function word(v){h=Math.imul(h^(v>>>0),16777619)>>>0;}
   function one(p){[16,20,24].forEach(o=>word(p.add(o).readU32()));const a=p.add(4).readPointer(),b=p.add(8).readPointer();
     word(b.sub(a).toInt32()/16);for(let q=a;q.compare(b)<0;q=q.add(4))word(q.readU32());}
   for(let cl=0;cl<7;cl++)for(let l=0;l<9;l++)one(actor.add(0x4284+cl*0x100+l*0x1c));
   for(let l=0;l<9;l++)one(actor.add(0x4984+l*0x1c));[0x4a84,0x4aa0,0x4abc].forEach(o=>one(actor.add(o)));return h;}
 try{
   if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000)))throw Error('wrong image');
   save(0x8cf6dc,4).writePointer(chitin);chitin.add(0x1c54).writePointer(game);
   save(0x8c1758,4).writePointer(text(''));save(0x8def6c,4).writePointer(text('SPECIAL_ABILITIES_FILE'));
   game.add(0x4c00).writePointer(catalogue);game.add(0x4c04).writeU32(2);
   replace(0x7fcd07,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(s.readPointer());return p;});
   replace(0x7fc98f,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(s.readPointer());return p;});
   replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
   replace(0x7e7a54,'int',['pointer','pointer'],'mscdecl',(a,b)=>a.readUtf8String().localeCompare(b.readUtf8String()));
   replace(0x7e7b03,'int',['pointer'],'mscdecl',p=>parseInt(p.readUtf8String(),10)||0);
   replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
   replace(0x78a970,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{ref(p,str(s).toUpperCase());return p;});
   replace(0x78a990,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{ref(p,s.readUtf8String().toUpperCase());return p;});
   replace(0x78ad40,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{p.writeByteArray(r.readByteArray(8));out.writeByteArray(r.readByteArray(8));return out;});
   replace(0x78ac30,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{p.writeByteArray(r.readByteArray(8));out.writeByteArray(r.readByteArray(8));return out;});
   replace(0x78aa30,'pointer',['pointer','pointer'],'thiscall',(p,out)=>{out.writePointer(text(res(p)));return out;});
   replace(0x541940,'pointer',['pointer','pointer','uint8','uint8'],'thiscall',(p,out,r,s)=>{e.races.push([r,s]);out.writePointer(text('RACE'));return out;});
   replace(0x403340,'pointer',['pointer','pointer','pointer'],'thiscall',(p,x,y)=>{if(str(x)!=='SPECIAL_ABILITIES_FILE')throw Error('wrong column');const out=alloc(4);out.writePointer(text(c.table===0?'*':'RACESPAB'));return out;});
   replace(0x402b70,'void',['pointer','pointer'],'thiscall',(p,r)=>{if(res(r)!=='RACESPAB')throw Error('wrong table');e.loads++;
     const rows=[['ZRCHIT','1'],['ZRCHIT','1'],['SPIN231','2'],['*','3'],['MISSING','1'],['ZRCHIT','0'],['SPIN231','1']];
     const width=c.table===1?1:2,data=alloc(rows.length*width*4);rows.forEach((row,i)=>{for(let x=0;x<width;x++)data.add((i*width+x)*4).writePointer(text(row[x]));});
     p.add(0x18).writePointer(data);p.add(0x1c).writePointer(text('*'));p.add(0x20).writeS16(width);p.add(0x22).writeS16(rows.length);});
   replace(0x71b9c0,'void',['pointer','pointer','int'],'thiscall',(p,r,f)=>{const name=res(r);e.grants.push([name,f]);const id=names.indexOf(name);if(id<0)return;
     let end=actor.add(0x4a8c).readPointer(),found=false;for(let q=innateData;q.compare(end)<0;q=q.add(16)){if(q.readU32()===id){q.add(4).writeU32(q.add(4).readU32()+1);q.add(8).writeU32(q.add(8).readU32()+1);found=true;break;}}
     if(!found){end.writeU32(id);end.add(4).writeU32(1);end.add(8).writeU32(1);actor.add(0x4a8c).writePointer(end.add(16));}});
   replace(0x7fc984,'void',['pointer'],'mscdecl',p=>{if(e.buttons.some(b=>b.equals(p)))e.deletedButtons++;});
   replace(0x7e78de,'void',['pointer','uint','uint','pointer'],'stdcall',()=>{});
   replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
   replace(0x45fe00,'pointer',['pointer','uint','uint'],'thiscall',(p,lo,hi)=>{p.writeU32(lo);p.add(4).writeU32(hi);return p;});
   vtable.add(0x94).writePointer(cb('void',['pointer','int16','pointer'],(p,slot,s)=>{e.script=[slot,res(s)];}));
   vtable.add(0x10).writePointer(cb('pointer',['pointer'],p=>p.add(0x20)));
   replace(0x544bf0,'int8',['pointer','uint8'],'thiscall',(p,a)=>{e.alignment=a;return 12;});
   replace(0x5431e0,'int',['pointer','pointer','pointer','pointer','uint','uint','pointer'],'thiscall',(p,cl,t,d,s,l,b)=>{
     e.maxCalls.push([cl.readU8(),l,s,t.add(0x18).readU32(),d.equals(actor.add(0x920))]);b.writeS32(c.max?c.bonus:0);return c.max;});
   replace(0x542fe0,'int',['pointer','pointer'],'thiscall',()=>{e.goldCalls++;return 37;});
   replace(0x74f830,'void',['pointer','uint8','uint8','uint8','uint8'],'thiscall',(p,...args)=>{e.selection.push(args);});
   replace(0x71b460,'void',['pointer','int'],'thiscall',()=>{e.unequip++;});
   replace(0x4e7e90,'pointer',['pointer','uint','uint','uint16','uint16','uint16','int','uint'],'thiscall',(p,lo,hi,...args)=>{p.writeU32(lo);p.add(4).writeU32(hi);e.item=[res(p),...args];return p;});
   replace(0x717850,'pointer',['pointer','int16','uint16','int16'],'thiscall',()=>{
     const list=alloc(28),vt=alloc(8);list.writePointer(vt);vt.add(4).writePointer(cb('void',['pointer','uint'],()=>{e.listDeleted++;}));let next=ptr(0);
     for(let j=c.buttons-1;j>=0;j--){const node=alloc(12),button=alloc(60);for(let k=0;k<60;k++)button.add(k).writeU8(k+1);button.add(0x1c).writeS16(c.slot);
       node.writePointer(next);node.add(8).writePointer(button);next=node;e.buttons.push(button);}list.add(4).writePointer(next);return list;});
   replace(0x7fb278,'void',['pointer'],'thiscall',()=>{e.listCleared++;});
   replace(0x443b30,'pointer',['pointer'],'thiscall',p=>{p.writeByteArray(new Uint8Array(0xe58));return p;});
   replace(0x71e240,'void',['pointer','pointer','pointer'],'thiscall',(p,b,d)=>{e.previousZero=Array.from(new Uint8Array(b.readByteArray(0x37c))).every(x=>x===0);e.feats++;});
   replace(0x72de60,'int',['pointer'],'thiscall',()=>{e.process++;return 1;});
   for(const a of [0x4e71b0,0x7fb298,0x443810,0x4436c0,0x5f9870,0x448580,0x5f99a0,0x5f98b0,0x443650,0x4435e0,0x443420,0x443570,0x443500,0x443490])replace(a,'void',['pointer'],'thiscall',()=>{});
   Interceptor.flush();
   for(c of __CASES__){
     if(events.length%12===0)send({phase:'case',index:events.length,input:c});
     e={input:c,grants:[],races:[],loads:0,maxCalls:[],goldCalls:0,selection:[],unequip:0,buttons:[],deletedButtons:0,listDeleted:0,listCleared:0,feats:0,process:0};
     actor.writeByteArray(new Uint8Array(0x9e00));actor.writePointer(vtable);actor.add(0x28).writeU8(c.cls);actor.add(0x3e3c).writeU8(c.race);actor.add(0x3e3d).writeU8(c.sub);actor.add(0x3e4b).writeU8(0x12);actor.add(0x3e4e).writeU32(0x202);
     actor.add(0x5bc).writeU32(0x40000000);actor.add(0x5e2).writeS16(-7);actor.add(0x5b8).writeU32(c.gold);actor.add(0x3d3a).writeU8(99);
     for(let cl=0;cl<7;cl++)for(let l=0;l<9;l++)list(actor.add(0x4284+cl*0x100+l*0x1c),2,c.seed+l);
     for(let l=0;l<9;l++)list(actor.add(0x4984+l*0x1c),2,c.seed+l);
     innateData=list(actor.add(0x4a84),c.seed?1:0,c.seed);if(c.seed){innateData.writeU32(0);innateData.add(4).writeU32(7);innateData.add(8).writeU32(2);}
     list(actor.add(0x4aa0),2,c.seed);list(actor.add(0x4abc),2,c.seed);
     if(c.mode===0)racial(actor);else if(c.mode===1)wrap(screen,actor);else skills(screen,actor);
     e.state=actor.add(0x5bc).readU32();e.field3e=actor.add(0x5e2).readS16();e.gold=actor.add(0x5b8).readU32();e.reputation=actor.add(0x5e0).readU8();e.hash=digest();
     e.cache=actor.add(0x3d3a).readU8();e.button=Array.from(new Uint8Array(actor.add(0x342c).readByteArray(60)));e.dirty=actor.add(0x562c).readU32();e.created=!actor.add(0x4b84).readPointer().isNull();
     delete e.buttons;events.push(e);
   }
   const getAbility=new NativeFunction(ptr(0x4e9610),'pointer',['pointer','int'],'thiscall');
   const item=alloc(0x100),resource=alloc(0x100),header=alloc(0x100),abilities=alloc(0x70);
   resource.add(0x50).writePointer(abilities);resource.add(0x58).writePointer(header);header.add(0x68).writeU16(2);
   for(const row of __GETTERS__){
     ref(item.add(0xc),row.mode===1?'':'ZRCHIT');item.add(8).writePointer(row.mode===2?ptr(0):resource);
     resource.add(0x5c).writeS32(row.parsed);resource.add(0x40).writeS32(row.demands);
     abilities.writeU16(row.type);abilities.add(0x38).writeU16(2);
     const result=getAbility(item,row.index);
     getterEvents.push({input:row,result:result.isNull()?-1:result.sub(abilities).toInt32()/0x38,demands:resource.add(0x40).readS32()});
   }
 }catch(ex){error=String(ex);}
 finally{for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,b]of globals)p.writeByteArray(b);}
 return{events,getterEvents,error,hooksRestored:patches.every(([p,b])=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===b),globalsRestored:globals.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(b.byteLength))))===JSON.stringify(Array.from(new Uint8Array(b))))};
};
"""


def header(events,getters):
    lines=['// Generated from canonical IWD2.exe by chargen_completion_reference.py.',
           '#ifndef CHARGEN_COMPLETION_GOLDEN_H_', '#define CHARGEN_COMPLETION_GOLDEN_H_',
           'struct ChargenGolden { INT mode,race,sub,table,seed,gold,maximum,bonus,cls,buttons,slot; DWORD state; SHORT field3e; DWORD afterGold,hash; BYTE cache; UINT grants[2]; };',
           'static const ChargenGolden CHARGEN_CASES[] = {']
    for e in events:
        c=e['input']; values=[str(c[k]) for k in ['mode','race','sub','table','seed','gold','max','bonus','cls','buttons','slot']]
        values += [str(e['state'])+'u',str(e['field3e']),str(e['gold'])+'u',str(e['hash'])+'u',str(e['cache'])]
        grants=[sum(name==r for r,f in e['grants']) for name in ['ZRCHIT','SPIN231']]
        lines.append('    {'+','.join(values)+',{'+','.join(map(str,grants))+'}},')
    lines += ['};','struct ItemAbilityGolden { INT mode,parsed,demands,type,index,result; };',
              'static const ItemAbilityGolden ITEM_ABILITY_CASES[] = {']
    for e in getters:
        c=e['input'];lines.append('    {'+','.join(str(c[k]) for k in ['mode','parsed','demands','type','index'])+','+str(e['result'])+'},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(cases())).replace('__GETTERS__',json.dumps(getter_cases())),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script];hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,90);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalsRestored') and len(events)==len(cases())
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    getters=result.get('getterEvents',[])
    good=good and len(getters)==len(getter_cases()) and all(e['demands']==e['input']['demands'] and e['result'] in [-1,0,1] for e in getters)
    for e in events:
        c=e['input']
        if c['mode']==1:
            good=good and e.get('script')==[6,'DPLAYER3'] and len(e['maxCalls'])==18 and e['goldCalls']==(c['gold']==0) and e['reputation']==120
            good=good and e['maxCalls']==[[cls,l,0,0x202,True] for cls in [2,10] for l in range(1,10)]
        if c['mode']==2:
            good=good and e['feats']==e['process']==1 and e['previousZero'] and e['dirty']==1 and e['created']==(c['cls']!=6)
            good=good and e['deletedButtons']==(c['buttons'] if c['cls']!=6 else 0)
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),expected=len(cases()),getter_cases=len(getters),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events,getters),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
