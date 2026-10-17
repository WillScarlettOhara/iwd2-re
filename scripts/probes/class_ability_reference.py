"""Frozen original class/feat grants and level deltas on isolated sprites.

The three target bodies and class getters/table selection execute. Resource,
CString and effect delivery dependencies are receipts, not original HP evidence.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from projectile_aim_reference import MD5,capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO,game_directory

REFS=['SPIN231','SPIN197','SPIN111','SPIN245','SPIN246','SPIN232','SPIN275','SPIN276','SPIN277','SPIN278','SPIN279','SPIN236','SPIN260','SPIN152','SPIN970','ZRCHIT']
GLOBALS=[0x8f94c0,0x8f94b0,0x8f92a0,0x8f94a0,0x8f9488,0x8f94d8,0x8f9338,0x8f8e70,0x8f92f8,0x8f92e8,0x8f93d0,0x8f9410,0x8f8e20,0x8f9478,0x8f8e18]

def cases():
    result=[]
    def add(**kw):
        c=dict(mode=0,cls=1,delta=0,seed=0,hp=100,race=1,subrace=0,spec=0,active=1,action=0,
               feats=[],oldFeats=[],ranks=[0]*4,oldRanks=[0]*4,levels=[16]+[0]*10,
               oldLevels=[15]+[0]*10,tempLevels=[0]*11,total=16,oldTotal=15)
        c.update(kw);result.append(c)
    for feat in range(75):
        for old,current in [([], [feat]),([feat],[feat]),([feat],[])]:add(feats=current,oldFeats=old)
    for feat in [0,2,51,65,7,15,26,34,35,37,59]:
        for seed in [95,100,122,123,127,128,250,251,255]:add(feats=[feat],seed=seed)
    for feat,rank in [(69,0),(20,1),(22,2),(23,3)]:
        for old,new in [(0,3),(3,1),(2,2),(255,0),(0,255)]:
            ranks=[0]*4;previous=[0]*4;ranks[rank]=new;previous[rank]=old
            add(feats=[feat],ranks=ranks,oldRanks=previous,hp=32766)
    for old,current in [(0,0),(0,1),(3,4),(4,4),(4,5),(7,8),(8,7),(14,15),(15,16),(0,255)]:
        for fresh in [False,True]:add(feats=[67],oldFeats=[] if fresh else [67],levels=[current]+[0]*10,oldLevels=[old]+[0]*10)
    for old,current in [(14,14),(14,15),(15,15),(15,16)]:add(feats=[20],ranks=[0,3,0,0],levels=[current]+[0]*10,oldLevels=[old]+[0]*10)
    for cls in range(1,12):
        for level,delta in [(1,-1),(15,-1),(30,1),(30,0),(9,10),(15,-2)]:
            levels=[0]*11;levels[cls-1]=level
            add(mode=1,cls=cls,delta=delta,levels=levels,spec=0x400000 if cls==3 else 0)
    for active in [0,1]:
        add(mode=1,cls=1,delta=-1,levels=[30]+[0]*10,tempLevels=[14]+[0]*10,active=active,seed=127)
    for action in range(1,6):add(mode=1,cls=2,delta=-1,levels=[0,1]+[0]*9,action=action)
    for race,subrace in [(1,1),(2,0),(2,1),(6,0),(6,1)]:
        for total,old in [(0,0),(1,0),(30,14),(14,30),(255,0)]:
            add(mode=2,race=race,subrace=subrace,total=total,oldTotal=old,
                levels=[11,1,1,0,0,13,0,0,0,0,0],oldLevels=[10,0,0,0,0,12,0,0,0,0,0])
    return result

JS=r"""
rpc.exports.run=function(){
  const roots=[],callbacks=[],patches=[],globals=[],events=[];
  let current=null,error=null;
  function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
  function save(a,n){const p=ptr(a);globals.push([p,p.readByteArray(n)]);return p;}
  function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);
    patches.push([p,Array.from(new Uint8Array(p.readByteArray(16))).join(',')]);Interceptor.replace(p,f);}
  function text(s){const p=alloc(s.length+13);p.writeS32(1);p.add(4).writeS32(s.length);p.add(8).writeS32(s.length);p.add(12).writeUtf8String(s);return p.add(12);}
  function str(p){return p.readPointer().readUtf8String();}
  function ref(p,s){const b=Array.from(s.slice(0,8),c=>c.charCodeAt(0));while(b.length<8)b.push(0);p.writeByteArray(b);}
  function res(p){return Array.from(new Uint8Array(p.readByteArray(8))).map(c=>String.fromCharCode(c)).join('').split('\0')[0];}
  function masks(p,fs){fs.forEach(f=>{const q=p.add((f>>5)*4);q.writeU32((q.readU32()|(1<<(f&31)))>>>0);});}
  const actor=alloc(0x9e00),previous=alloc(0x37c),stats=alloc(0xe58),game=alloc(0x6000),chitin=alloc(0x6000),vtable=alloc(0x80);
  const refNames=__REFS__,refGlobals=__GLOBALS__;
  const statOffsets=[0x1c,0x1e,0x3e,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x54,0x55,0x56,0x57];
  function statsOut(){return statOffsets.map((o,i)=>i<3?actor.add(0x5a4+o).readU16():actor.add(0x5a4+o).readU8());}
  try{
    if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000)))throw Error('wrong image');
    save(0x8cf6dc,4).writePointer(chitin);chitin.add(0x1c54).writePointer(game);
    for(let i=0;i<refGlobals.length;i++)ref(save(refGlobals[i],8),refNames[i]);
    for(const [a,s] of [[0x8f9340,'FA_'],[0x8f9430,'GA_'],[0x8f94e0,'RA_'],[0x8f94c8,'AP_']])save(a,4).writePointer(text(s));
    replace(0x7fcd07,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(s.readPointer());return p;});
    replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
    replace(0x7e7a54,'int',['pointer','pointer'],'mscdecl',(a,b)=>a.readUtf8String().localeCompare(b.readUtf8String()));
    replace(0x7faa72,'pointer',['pointer','pointer','int'],'thiscall',(p,out,n)=>{out.writePointer(text(str(p).slice(0,Math.max(n,0))));return out;});
    replace(0x7fa9f6,'pointer',['pointer','pointer','int'],'thiscall',(p,out,n)=>{const s=str(p);out.writePointer(text(n<=0?'':s.slice(-n)));return out;});
    replace(0x403530,'pointer',['pointer','pointer'],'thiscall',(p,out)=>{out.writePointer(p.add(0x1c).readPointer());return out;});
    replace(0x78a970,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{ref(p,str(s).toUpperCase());return p;});
    replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
    replace(0x78ac30,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{p.writeByteArray(r.readByteArray(8));out.writeByteArray(r.readByteArray(8));return out;});
    replace(0x78acc0,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,str(s).toUpperCase());out.writeByteArray(p.readByteArray(8));return out;});
    replace(0x71b9c0,'void',['pointer','pointer','int'],'thiscall',(p,r,f)=>{current.grants.push([res(r),f]);});
    replace(0x71bc60,'void',['pointer','pointer'],'thiscall',(p,r)=>current.removals.push(res(r)));
    replace(0x7349a0,'void',['pointer','uint16','int','int','int','int','int','int'],'thiscall',(p,...a)=>current.feedback.push(a));
    replace(0x7e7b03,'int',['pointer'],'mscdecl',p=>parseInt(p.readUtf8String(),10)||0);
    // Two effects separated by a null list entry capture provenance, virtual
    // delivery flags and ownership of the temporary list nodes.
    replace(0x586220,'pointer',['pointer','pointer','pointer','int'],'mscdecl',(out,p,r,level)=>{
      current.ap.push([res(r),level]);const head=alloc(12),nodes=[alloc(12),alloc(12),alloc(12)];
      head.writePointer(nodes[0]);head.add(4).writePointer(nodes[2]);
      nodes.forEach((node,i)=>{node.writePointer(i===2?head:nodes[i+1]);node.add(4).writePointer(i===0?head:nodes[i-1]);
        node.add(8).writePointer(i===1?ptr(0):alloc(0x200));});
      out.writeU32(0);out.add(4).writePointer(head);out.add(8).writeU32(3);return out;});
    replace(0x44ad80,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,node)=>{
      const next=node.readPointer(),prev=node.add(4).readPointer();prev.writePointer(next);next.add(4).writePointer(prev);
      p.add(8).writeU32(p.add(8).readU32()-1);out.writePointer(next);current.erased++;return out;});
    replace(0x7fc984,'void',['pointer'],'mscdecl',()=>{});
    replace(0x723390,'void',['pointer'],'thiscall',()=>{current.monk++;});
    // Real HasFeat, Stunning Fist prerequisites and class-level getters execute.
    const type=new NativeCallback(p=>p.add(0x20),'pointer',['pointer'],'thiscall');callbacks.push(type);vtable.add(0x10).writePointer(type);
    const kind=new NativeCallback(()=>ptr(0x84c507).readU8(),'uint8',['pointer'],'thiscall');callbacks.push(kind);vtable.add(4).writePointer(kind);
    const deliver=new NativeCallback((p,e,list,noSave,immediate)=>{current.effects.push([res(e.add(0x90)),list,noSave,immediate]);},'void',['pointer','pointer','uint8','int','int'],'thiscall');
    callbacks.push(deliver);vtable.add(0x78).writePointer(deliver);
    const getTable=new NativeFunction(ptr(0x546030),'pointer',['pointer','uint8','uint'],'thiscall');
    const feat=new NativeFunction(ptr(0x71e240),'void',['pointer','pointer','pointer'],'thiscall');
    const klass=new NativeFunction(ptr(0x722d50),'void',['pointer','uint8','int16','int'],'thiscall');
    const all=new NativeFunction(ptr(0x71e760),'void',['pointer','pointer','int'],'thiscall');
    Interceptor.flush();
    for(const c of __CASES__){
      actor.writeByteArray(new Uint8Array(0x9e00));previous.writeByteArray(new Uint8Array(0x37c));stats.writeByteArray(new Uint8Array(0xe58));
      actor.writePointer(vtable);actor.add(0x72a4).writeS32(c.active);actor.add(0x26).writeU8(c.race);actor.add(0x27).writeU8(c.subrace);
      actor.add(0x80c).writeU32(c.spec);actor.add(0x5c0).writeU16(c.hp);actor.add(0x5c2).writeU16(c.hp);
      actor.add(0x5ec).writeU8(8);actor.add(0x978).writeS16(13);actor.add(0x97a).writeS16(13);
      for(let o=0x49;o<=0x58;o++)actor.add(0x5a4+o).writeU8(c.seed);
      masks(actor.add(0x75c),c.feats);masks(previous.add(0x1b8),c.oldFeats);
      [0x1dc,0x1e5,0x1e7,0x1e8].forEach((o,i)=>{actor.add(0x5a4+o).writeU8(c.ranks[i]);previous.add(o).writeU8(c.oldRanks[i]);});
      for(const [p,levels,total] of [[actor.add(0x920),c.levels,c.total],[actor.add(0x1778),c.tempLevels,0],[stats,c.oldLevels,c.oldTotal]]){
        p.add(0x42).writeU32(1);p.add(0x46).writeU8(total);levels.forEach((l,i)=>p.add(0x47+i).writeU8(l));}
      for(let cl=1;cl<=11;cl++){
        const t=getTable(game,cl,c.spec),values=alloc(30*4);for(let i=0;i<30;i++)values.add(i*4).writePointer(text('*'));
        const actions=['*','GA_ZRCHIT','RA_ZRCHIT','AP_ZRCHIT','FA_10453','*'];
        if(c.action)values.writePointer(text(actions[c.action]));
        t.add(0x18).writePointer(values);t.add(0x1c).writePointer(text('*'));t.add(0x20).writeS16(30);t.add(0x22).writeS16(1);
      }
      current={input:c,grants:[],removals:[],feedback:[],ap:[],effects:[],erased:0,monk:0};
      const before=Array.from(new Uint8Array(actor.add(0x5a4).readByteArray(0x37c)));
      if(c.mode===0)feat(actor,previous,stats);else if(c.mode===1)klass(actor,c.cls,c.delta,2);else all(actor,stats,2);
      current.stats=statsOut();current.feats=Array.from({length:3},(_,i)=>actor.add(0x75c+i*4).readU32());
      const after=Array.from(new Uint8Array(actor.add(0x5a4).readByteArray(0x37c)));
      current.changed=after.flatMap((v,i)=>v!==before[i]?[i]:[]);events.push(current);
    }
  }catch(e){error=String(e);}
  finally{for(const [p] of patches)Interceptor.revert(p);Interceptor.flush();for(const [p,data] of globals)p.writeByteArray(data);}
  return{events,error,globalsRestored:globals.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(b.byteLength))))===JSON.stringify(Array.from(new Uint8Array(b)))),
    hooksRestored:patches.every(([p,b])=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===b)};
};
"""

def header(events):
    def array(x):return '{'+','.join(str(int(v)) for v in x)+'}'
    def masks(fs):return [sum(1<<(f&31) for f in fs if f>>5==i) for i in range(3)]
    lines=['// Original class/feat state and grant receipts.','#ifndef CLASS_ABILITY_GOLDEN_H_','#define CLASS_ABILITY_GOLDEN_H_',
           'static const char* const CLASS_ABILITY_REFS[] = {'+','.join('"'+r+'"' for r in REFS)+'};',
           'struct ClassAbilityGolden { INT mode,cls,delta,seed,hp,race,subrace,spec,active,action,total,oldTotal; DWORD feats[3],oldFeats[3]; BYTE ranks[4],oldRanks[4],levels[11],oldLevels[11],tempLevels[11]; UINT stats[16]; DWORD afterFeats[3]; UINT grants[16]; UINT removes,feedback,ap; };',
           'static const ClassAbilityGolden CLASS_ABILITIES[] = {']
    for e in events:
        c=e['input'];scalar=[c[k] for k in ['mode','cls','delta','seed','hp','race','subrace','spec','active','action','total','oldTotal']]
        grants=[sum(1 for r,f in e['grants'] if r==name) for name in REFS]
        fields=[array(masks(c['feats'])),array(masks(c['oldFeats']))]+[array(c[k]) for k in ['ranks','oldRanks','levels','oldLevels','tempLevels']]+[array(e['stats']),array(e['feats']),array(grants)]
        lines.append('    {'+','.join(map(str,scalar))+','+','.join(fields)+f",{len(e['removals'])},{len(e['feedback'])},{len(e['ap'])}"+'},')
    return '\n'.join(lines+['};','#endif',''])

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js'
    script.write_text(JS.replace('__REFS__',json.dumps(REFS)).replace('__GLOBALS__',json.dumps(GLOBALS)).replace('__CASES__',json.dumps(cases())),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script];hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,90);events=result['events']
    allowed={0x1c,0x1d,0x1e,0x1f,0x3e,0x3f,*range(0x49,0x58),*range(0x1b8,0x1c4)}
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalsRestored') and len(events)==len(cases())
    good=good and not any(m.get('type')=='error' for m in result['messages']) and all(set(e['changed'])<=allowed for e in events)
    good=good and all(e['effects']==[['ZRCHIT',1,0,1]]*2*len(e['ap']) and e['erased']==3*len(e['ap']) for e in events)
    good=good and all(all(r in REFS and f==(0 if e['input']['mode']==0 else 2) for r,f in e['grants']) for e in events)
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),expected=len(cases()),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2

if __name__=='__main__':raise SystemExit(main())
