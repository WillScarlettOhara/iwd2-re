"""Frozen original level-up entry and panel selector, with explicit service doubles.

Selector and HasSpell targets execute unchanged; tables/resources are private.
HasSpell and its real catalogue/list searches run without service doubles.
Entry/flow tests
double UI, object locks, stats lifetime, effects and completion, recording their
ordering and payloads. These captures do not prove the doubled implementations.
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
    rows=[]
    for cls in range(1,12):
        for panel in [2,6,7,16,54,55,56]:
            for old,new,points in [(0,1,-1),(3,4,0),(4,3,1),(7,8,513),(255,0,1)]:
                rows.append(dict(cls=cls,panel=panel,old=old,new=new,points=points,spec=0x8000,
                                 previousClass=old,slots=min(new,8),spellLevel=0,before=0,after=0,known=0,count=0))
    for spec in [0,0x18000]+[0x8000<<i for i in range(9)]:
        rows.append(dict(cls=3,panel=56,old=0,new=1,points=1,spec=spec,previousClass=0,
                         slots=0,spellLevel=0,before=0,after=0,known=0,count=0))
    for cls in [2,10,5]:
        for level in [0,1,8,9,10,254]:
            for before,after,count,known in [(0,0,2,0),(3,1,2,0),(0,5,2,0),(0,5,2,1),(0,40,35,0)]:
                rows.append(dict(cls=cls,panel=8,old=1,new=10,points=0,spec=0x8000,
                                 previousClass=1,slots=0,spellLevel=level,before=before,after=after,known=known,count=count))
    return rows


JS=r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],events=[],eligibility=[],clicks=[],flows=[],hasSpells=[];
 let c=null,e=null,error=null,mode='selector';
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a,n){const p=ptr(a);globals.push([p,p.readByteArray(n)]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16))).join(',')]);Interceptor.replace(p,f);}
 function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
 function ref(p,s){const bytes=Array.from(s.slice(0,8),x=>x.charCodeAt(0));while(bytes.length<8)bytes.push(0);p.writeByteArray(bytes);}
 function string(s){const p=Memory.allocUtf8String(s);roots.push(p);return p;}
 const actor=alloc(0x9e00),game=alloc(0x6000),chitin=alloc(0x6000),screen=alloc(0x2000),old=alloc(0xe58),before=alloc(0xe58),svt=alloc(0x100),avt=alloc(0x100),resource=alloc(128),zero=alloc(4),node=alloc(16);
 const cells=alloc(23*75*4),master=alloc(35*8),entries=alloc(35*8),knownCells=alloc(9*10*4),knownEntry=alloc(16),queryMask=alloc(4),missing=alloc(8),strs={};
 for(let j=0;j<=40;j++)strs[j]=string(String(j));
 const selector=new NativeFunction(ptr(0x5e0fd0),'int',['pointer','uint','pointer'],'thiscall');
 const feat=new NativeFunction(ptr(0x762e20),'int',['pointer','uint'],'thiscall');
 const hasSpell=new NativeFunction(ptr(0x725330),'uint8',['pointer','pointer','pointer','uint','uint8'],'thiscall');
 function vector(p,current){knownEntry.writeU32(0);knownEntry.add(8).writeU32(current);p.add(4).writePointer(knownEntry);p.add(8).writePointer(knownEntry.add(16));p.add(12).writePointer(knownEntry.add(16));}
 function table(p,w,h,data){p.add(0x18).writePointer(data);p.add(0x1c).writePointer(strs[0]);p.add(0x20).writeS16(w);p.add(0x22).writeS16(h);}
 function reset(){actor.writeByteArray(new Uint8Array(0x9e00));screen.writeByteArray(new Uint8Array(0x2000));old.writeByteArray(new Uint8Array(0xe58));before.writeByteArray(new Uint8Array(0xe58));actor.writePointer(avt);screen.writePointer(svt);screen.add(0x85e).writePointer(old);screen.add(0x862).writePointer(before);game.add(0x3846).writeS16(1);game.add(0x382e).writeS32(123);}
 try{
   save(0x8cf6dc,4).writePointer(chitin);save(0x8cf6d8,4).writePointer(chitin);chitin.add(0x1c54).writePointer(game);
   replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(n,f,s)=>{throw Error('assert '+n+' '+s.readUtf8String());});
   for(let row=0;row<75;row++)for(let col=0;col<23;col++)cells.add((row*23+col)*4).writePointer(strs[(row+col*3)%11===0?1:0]);
   table(game.add(0xac),23,75,cells);table(game.add(0x550),9,10,knownCells);table(game.add(0x574),9,10,knownCells);
   game.add(0x4bf8).writePointer(master);game.add(0x4bfc).writeU32(35);
   for(let j=0;j<35;j++){ref(master.add(j*8),j%2?'SPIN260':'ZRCHIT');entries.add(j*8).writeU32(j);}
   replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
   for(const a of [0x78ad40,0x78ac30])replace(a,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{p.writeByteArray(r.readByteArray(8));out.writeByteArray(r.readByteArray(8));return out;});
   replace(0x4699f0,'void',['pointer','pointer','int','int'],'thiscall',(p,r)=>{p.add(4).writePointer(resource);p.add(8).writeByteArray(r.readByteArray(8));});
   replace(0x54a380,'int',['pointer'],'thiscall',()=>1);replace(0x54a3c0,'int',['pointer'],'thiscall',()=>1);
   replace(0x54aa40,'int',['pointer','pointer'],'thiscall',()=>1);
   replace(0x787ce0,'void',['pointer','pointer'],'thiscall',()=>{});replace(0x77e370,'void',['pointer'],'thiscall',()=>{});
   Interceptor.flush();ref(missing,'ZRABSENT');
   const masks=[2,4,8,64,128,512,1024,4];
   for(let bank=0;bank<8;bank++)for(const stored of [0,4,8])for(const level of [0,4,8,9,0xffffffff])for(let kind=0;kind<4;kind++)for(const current of [0,1,0xffffffff])for(const required of [0,1,2])for(const present of [0,1]){
     reset();vector(actor.add(bank===7?0x4984+stored*0x1c:0x4284+bank*0x100+stored*0x1c),current);
     const mask=[0,masks[bank],4,0x7ff][kind];queryMask.writeU32(mask);
     hasSpells.push(hasSpell(actor,present?master:missing,queryMask,level,required));
   }
   for(c of __CASES__){reset();screen.add(0x106).writeU8(c.cls);screen.add(0x2bc).writeS32(c.points);screen.add(0x64c).writeS32(c.points);screen.add(0x1776).writeU8(c.spellLevel);
     old.add(0x46).writeU8(c.old);old.add(0x47+c.cls-1).writeU8(1);before.add(0x47+c.cls-1).writeU8(c.previousClass);
     actor.add(0x966).writeU8(c.new);actor.add(0x967+c.cls-1).writeU8(c.new);actor.add(0x80c).writeU32(c.spec);
     for(let j=0;j<8;j++)actor.add(0x7f7+j).writeU8(j<c.slots?1:255);
     if(c.known)for(let cl=0;cl<7;cl++)for(let l=0;l<9;l++)vector(actor.add(0x4284+cl*0x100+l*0x1c),1);
     for(let row=0;row<10;row++)for(let col=0;col<9;col++)knownCells.add((row*9+col)*4).writePointer(strs[row===9?c.after:c.before]);
     for(let cl=0;cl<7;cl++)for(let l=0;l<9;l++){const list=game.add(0x4c18+cl*0x4c+l*8);list.writePointer(entries);list.add(4).writeU32(c.count);}
     const result=selector(screen,c.panel,actor);let hash=2166136261;
     for(let f=0;f<75;f++)hash=Math.imul(hash^feat(actor,f),16777619)>>>0;
     events.push({input:c,result,spellLevel:screen.add(0x1776).readU8(),extra:screen.add(0x7b6).readS32(),featHash:hash});
   }
   // Eligibility keeps the target unmodified; stats services are explicit doubles.
   replace(0x443b30,'pointer',['pointer'],'thiscall',p=>{p.writeByteArray(new Uint8Array(0xe58));return p;});
   replace(0x445330,'pointer',['pointer','pointer'],'thiscall',(p,r)=>{p.writeByteArray(r.readByteArray(0xe58));return p;});
   replace(0x5437e0,'void',['pointer','uint8','pointer','pointer'],'thiscall',(p,cl,stats,sprite)=>{e.next=[cl,sprite.equals(actor)];stats.add(0x46).writeU8(c.next);});
   for(const a of [0x7fb298,0x4e71b0,0x4e6e80,0x5f9870,0x5f99a0,0x5f98b0,0x443650,0x4435e0,0x443420,0x443570,0x443500,0x443490])replace(a,'void',['pointer'],'thiscall',()=>{});
   replace(0x5f9aa0,'pointer',['pointer','pointer','pointer'],'thiscall',()=>zero);
   replace(0x55bb40,'pointer',['pointer','pointer','uint'],'thiscall',()=>zero);
   replace(0x7fc984,'void',['pointer'],'mscdecl',()=>{});
   replace(0x7e78de,'void',['pointer','uint','uint','pointer'],'stdcall',()=>{});
   const eligible=new NativeFunction(ptr(0x5e4e80),'int',['pointer','pointer'],'thiscall');Interceptor.flush();
   for(const view of [0,1,2])for(const service of [0,1])for(const owner of [42,43])for(const state of [0,0x800,0x1000])for(const next of [1,2]){
     c={view,service,owner,state,next};e={input:c,next:[]};reset();screen.add(0x7fa).writeS32(view);chitin.add(0x96e).writeS32(service);chitin.add(0x104c).writeU32(42);actor.add(0x62).writeU32(owner);actor.add(0x920).writeU32(state);actor.add(0x966).writeU8(1);actor.add(0x3e3e).writeU8(5);
     e.result=eligible(screen,actor);eligibility.push(e);
   }
   // Flow entry tests record all panel skips, completions, stack pops and readiness.
   mode='flow';const flow=new NativeFunction(ptr(0x5e1910),'int',['pointer','uint','pointer'],'thiscall');
   replace(0x5e0fd0,'int',['pointer','uint','pointer'],'thiscall',(p,panel,s)=>{e.visited.push(panel);return +(panel===c.accept);});
   replace(0x5e0e70,'void',['pointer','uint','pointer','int'],'thiscall',(p,panel,s,flag)=>e.popup=[panel,flag,s.equals(actor)]);
   replace(0x5e56f0,'void',['pointer','pointer'],'thiscall',()=>{e.complete++;});
   replace(0x5e1df0,'void',['pointer','pointer'],'thiscall',()=>{e.pops++;screen.add(0x632).writePointer(e.pops<c.stack?node:ptr(0));});
   replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
   replace(0x4f7500,'int',['pointer','pointer','int'],'thiscall',(p,msg,force)=>{
     if(mode==='click')e.messages.push(msg.readU32()===0x847b9c?['stop',msg.add(4).readS32(),msg.add(8).readS32()]:['ready',msg.add(4).readS32(),msg.add(8).readS32(),msg.add(12).readU32(),msg.add(16).readU8()]);
     else e.messages.push([msg.add(4).readS32(),msg.add(8).readS32(),msg.add(12).readU32(),msg.add(16).readU8(),force]);return 1;});
   replace(0x518660,'void',['pointer','uint','uint8'],'thiscall',(p,id,value)=>e.ready.push([id,value]));Interceptor.flush();
   for(const start of [2,54,6,7,55,56,8,16])for(const accept of [2,54,6,7,55,56,8,16,-1])for(const host of [0,1,2]){
     c={start,accept,host,stack:3};e={input:c,visited:[],popup:[],complete:0,pops:0,messages:[],ready:[]};reset();screen.add(0x632).writePointer(node);chitin.add(0x1033).writeU8(host);chitin.add(0x96e).writeS32(1);chitin.add(0x104c).writeU32(42);e.result=flow(screen,start,actor);flows.push(e);
   }
   // Actual click target, with the downstream flow captured at its call boundary.
   mode='click';const click=new NativeFunction(ptr(0x5e5330),'void',['pointer'],'thiscall');
   replace(0x5e1910,'int',['pointer','uint','pointer'],'thiscall',(p,panel,s)=>{e.flow=[panel,s.equals(actor)];return 1;});
   replace(0x7e6cbb,'void',['pointer','pointer','int'],'thiscall',()=>{});
   replace(0x7e6ce2,'int',['pointer','uint'],'thiscall',()=>1);
   replace(0x7e6cf7,'int',['pointer'],'thiscall',()=>1);
   replace(0x599c70,'uint8',['pointer','int','uint8','pointer','uint'],'thiscall',(p,id,t,out)=>{e.denies.push(id);out.writePointer(actor);return 0;});
   replace(0x59a010,'uint8',['pointer','int','uint8','uint'],'thiscall',(p,id)=>{e.releases.push(id);return 0;});
   replace(0x5e4e80,'int',['pointer','pointer'],'thiscall',()=>c.allowed);
   for(const [a,name]of [[0x6ff2f0,'path'],[0x6ff320,'search'],[0x707170,'idle'],[0x72de60,'effects']])replace(a,'void',['pointer'],'thiscall',()=>e.order.push(name));
   replace(0x40b6e0,'void',['pointer','pointer'],'thiscall',(p,r)=>p.writeByteArray(r.readByteArray(0x3c)));
   svt.add(0xd0).writePointer(cb('int',['pointer'],()=>c.selected));
   avt.add(0x6c).writePointer(cb('void',['pointer','int'],()=>e.order.push('clear')));avt.add(0x90).writePointer(cb('void',['pointer','pointer'],()=>e.order.push('action')));
   Interceptor.flush();
   for(const allowed of [0,1])for(const host of [0,1,2])for(const selected of [0,1]){
     c={allowed,host,selected};e={input:c,order:[],denies:[],releases:[],messages:[],ready:[],flow:[]};reset();screen.add(0x85e).writePointer(ptr(0));screen.add(0x862).writePointer(ptr(0));screen.add(0x28).writeS32(0);actor.add(0x5c).writeS32(987);actor.add(0x80c).writeU32(0x87654321);actor.add(0x7234).writeS32(-77);chitin.add(0x1033).writeU8(host);
     click(screen);e.spec=[screen.add(0x77c).readU32(),screen.add(0x846).readU32()];e.hp=screen.add(0x866).readS32();e.target=[actor.add(0x5694).readS32(),actor.add(0x5698).readS32()];e.snapshots=[0x85a,0x85e,0x862].map(o=>!screen.add(o).readPointer().isNull());clicks.push(e);
   }
 }catch(ex){error=mode+': '+String(ex);}
 finally{for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,b]of globals)p.writeByteArray(b);}
 return{events,eligibility,flows,clicks,hasSpells,error,hooksRestored:patches.every(([p,b])=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===b),globalsRestored:globals.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(b.byteLength))))===JSON.stringify(Array.from(new Uint8Array(b))))};
};
"""


def header(result):
    lines=['// Canonical selector outputs; service boundaries documented in begin_levelup_reference.py.',
           '#ifndef BEGIN_LEVELUP_GOLDEN_H_', '#define BEGIN_LEVELUP_GOLDEN_H_',
           'struct BeginLevelUpGolden { INT cls,panel,oldLevel,newLevel,points; DWORD spec; INT previousClass,slots,spellLevel,before,after,known,count,result,level,extra; DWORD featHash; };',
           'static const BeginLevelUpGolden BEGIN_LEVELUP_CASES[] = {']
    for e in result['events']:
        c=e['input'];v=[str(c[k]) for k in ['cls','panel','old','new','points','spec','previousClass','slots','spellLevel','before','after','known','count']]
        v += [str(e[k]) for k in ['result','spellLevel','extra']]+[str(e['featHash'])+'u']
        lines.append('    {'+','.join(v)+'},')
    lines+=['};','struct LevelUpEligibilityGolden { INT view,service,owner,state,next,result; };','static const LevelUpEligibilityGolden LEVELUP_ELIGIBILITY_CASES[] = {']
    for e in result['eligibility']:
        lines.append('    {'+','.join([str(e['input'][k]) for k in ['view','service','owner','state','next']]+[str(e['result'])])+'},')
    lines+=['};','static const BYTE HAS_SPELL_RESULTS[] = {']
    values=result['hasSpells'];packed=[sum(value<<bit for bit,value in enumerate(values[start:start+8])) for start in range(0,len(values),8)]
    for start in range(0,len(packed),24):lines.append('    '+','.join(f'0x{b:02x}' for b in packed[start:start+24])+',')
    return '\n'.join(lines+['};','#endif',''])


def receipts_match(result):
    for e in result.get('eligibility',[]):
        c=e['input'];controlled=bool(c['view']) and (not c['service'] or c['owner']==42)
        if e['result']!=int(controlled and c['next']!=1 and not c['state']&0x800):return False
        if e['next']!=([5,True] if controlled else []):return False
    cycle=[2,54,6,7,55,56,8,16]
    for e in result.get('flows',[]):
        c=e['input'];visited=[];panel=c['start']
        while True:
            panel=cycle[(cycle.index(panel)+1)%len(cycle)];visited.append(panel)
            if panel==c['accept'] or panel==2:break
        popup=panel==c['accept']
        if e['visited']!=visited or e['result']!=int(popup):return False
        if e['popup']!=([panel,1,True] if popup else []):return False
        if e['complete']!=int(not popup) or e['pops']!=(0 if popup else c['stack']):return False
        if e['messages']!=([[123,123,42,0,0]] if not popup and c['host']==0 else []):return False
        if e['ready']!=([[42,0]] if not popup and c['host']==1 else []):return False
    for e in result.get('clicks',[]):
        c=e['input'];allowed=bool(c['allowed']);target=123 if c['selected']==0 else -1
        if e['denies']!=[target] or e['releases']!=[target]:return False
        if e['order']!=(['path','search','clear','action','idle','effects'] if allowed else []):return False
        messages=[['stop',987,987]] if allowed else []
        if allowed and c['host']==0:messages.append(['ready',123,123,42,1])
        if e['messages']!=messages or e['ready']!=([[42,1]] if allowed and c['host']==1 else []):return False
        if e['flow']!=([2,True] if allowed else []):return False
        if e['snapshots']!=[allowed]*3 or e['spec']!=([0x87654321]*2 if allowed else [0,0]):return False
        if e['hp']!=(-77 if allowed else 0) or e['target']!=([-1,-1] if allowed else [0,0]):return False
    return True


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(cases())),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script];hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,90)
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalsRestored') and len(result['events'])==len(cases())
    good=good and len(result.get('eligibility',[]))==72 and len(result.get('flows',[]))==216 and len(result.get('clicks',[]))==12
    good=good and len(result.get('hasSpells',[]))==8640 and all(value in (0,1) for value in result['hasSpells'])
    good=good and not any(m.get('type')=='error' for m in result['messages']) and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    good=good and receipts_match(result)
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(result['events']),hasSpells=len(result.get('hasSpells',[])),eligibility=len(result.get('eligibility',[])),flows=len(result.get('flows',[])),clicks=len(result.get('clicks',[])),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(result),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
