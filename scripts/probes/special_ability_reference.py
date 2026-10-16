"""Frozen original special-ability add/remove, with real catalogues and lists.

Native Find, sprite-list Add/Remove and the catalogue getter execute on rooted storage.
Resource/UI dependencies capture ABI and ownership; no original HP/raster claim.
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


def cases():
    result=[]
    for mask in range(8):
        for seed in range(3):
            for key in [False,True]:
                for feedback in [0,1,2,-1,257,-2147483648]:
                    result.append(dict(op=0,mask=mask,seed=seed,key=key,feedback=feedback,resource=True))
            result.append(dict(op=1,mask=mask,seed=seed,key=True,feedback=0,resource=True))
    # The resource lookup can fail after an existing KEY entry. These isolated
    # dependency-failure captures are separate from the loaded-resource replay.
    for mask in [0,2,4]:result.append(dict(op=0,mask=mask,seed=0,key=True,feedback=1,resource=False))
    return result


JS=r"""
rpc.exports.run=function(){
  const memory=[],callbacks=[],addresses=[],before=[],events=[];
  const globals=[ptr(0x8cf6dc),ptr(0x8cf6d8)],saved=globals.map(p=>p.readPointer());
  let current=null,error=null,getterCorrect=false,timerBytes=[],saveSpan=0;
  function alloc(n){const p=Memory.alloc(n);memory.push(p);return p;}
  function ref(p,s){const a=Array.from(s.toUpperCase().slice(0,8),c=>c.charCodeAt(0));while(a.length<8)a.push(0);p.writeByteArray(a);}
  function res(p){return Array.from(new Uint8Array(p.readByteArray(8))).map(c=>String.fromCharCode(c)).join('').split('\0')[0];}
  function string(s){const p=alloc(s.length+13);p.writeS32(1);p.add(4).writeS32(s.length);p.add(8).writeS32(s.length);p.add(12).writeUtf8String(s);return p.add(12);}
  function replace(address,ret,args,abi,fn){const p=ptr(address),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);addresses.push(p);
    before.push(Array.from(new Uint8Array(p.readByteArray(16))).join(','));Interceptor.replace(p,f);}
  const chitin=alloc(0x6000),game=alloc(0x6000),actor=alloc(0x9e00),resource=alloc(128),key=alloc(32),input=alloc(8);
  const offsets=[0x4a84,0x4aa0,0x4abc],ids=[3,1,2],catalogues=[0x4c00,0x4c08,0x4c10];ref(input,'ZRCHIT');
  function lists(){return offsets.map(o=>{const p=actor.add(o),b=p.add(4).readPointer(),end=p.add(8).readPointer(),n=end.sub(b).toInt32()/16;
    if(n<0||n>3)throw Error('Bad vector size '+n);return{entries:Array.from({length:n},(_,i)=>Array.from({length:4},(_,j)=>b.add(i*16+j*4).readU32())),totals:[p.add(16).readU32(),p.add(20).readU32(),p.add(24).readU32()]};});}
  try{
    if(Process.arch!=='ia32'||!Process.enumerateModules()[0].base.equals(ptr(0x400000)))throw Error('Wrong original');
    chitin.add(0x1c54).writePointer(game);globals.forEach(p=>p.writePointer(chitin));
    replace(0x78b410,'pointer',['pointer','pointer','uint16','int'],'thiscall',(p,r,t,w)=>{current.keyArgs=[res(r),t,w];return current.input.key?key:ptr(0);});
    replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
    replace(0x78ad40,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,s.readCString());out.writeByteArray(p.readByteArray(8));return out;});
    replace(0x78acc0,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,s.readPointer().readCString());out.writeByteArray(p.readByteArray(8));return out;});
    replace(0x78ab40,'int',['pointer','pointer'],'thiscall',(p,s)=>res(p)!==s.readCString().toUpperCase()?1:0);
    replace(0x7fcd57,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{p.writePointer(string(s.readCString()));return p;});
    replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
    replace(0x4699f0,'void',['pointer','pointer','int','int'],'thiscall',(p,r,a,w)=>{current.order.push('bind');current.bind=[res(r),a,w];p.writeS32(1);p.add(4).writePointer(current.input.resource?resource:ptr(0));ref(p.add(8),res(r));});
    replace(0x54a380,'int',['pointer'],'thiscall',()=>{current.order.push('demand');return current.input.resource?1:0;});
    replace(0x54a3c0,'int',['pointer'],'thiscall',()=>{current.order.push('release');return current.input.resource?1:0;});
    replace(0x54acd0,'int',['pointer'],'thiscall',()=>{current.order.push('name');return 10453;});
    replace(0x7349a0,'void',['pointer','uint16','int','int','int','int','int','int'],'thiscall',(p,id,a,b,c,d,e,f)=>{current.order.push('feedback');current.feedback=[id,a,b,c,d,e,f];});
    replace(0x77e370,'void',['pointer'],'thiscall',()=>current.order.push('cancel'));
    replace(0x787ce0,'void',['pointer','pointer'],'thiscall',()=>current.order.push('release-object'));
    replace(0x71aba0,'void',['pointer','pointer','int16','int','int'],'thiscall',(p,a,d,r,e)=>{current.quick=Array.from(new Uint8Array(a.readByteArray(32)));current.quickArgs=[d,r,e];});
    Interceptor.flush();
    const add=new NativeFunction(ptr(0x71b9c0),'void',['pointer','pointer','int'],'thiscall');
    const remove=new NativeFunction(ptr(0x71bc60),'void',['pointer','pointer'],'thiscall');
    const shapes=new NativeFunction(ptr(0x5f6800),'pointer',['pointer'],'thiscall');
    getterCorrect=shapes(game).equals(game.add(0x4c10));
    const save=new NativeFunction(ptr(0x453050),'pointer',['pointer'],'thiscall');
    const options=new NativeFunction(ptr(0x45b630),'pointer',['pointer'],'thiscall');
    saveSpan=options(game).sub(save(game)).toInt32();
    const timer=alloc(8);timer.writeByteArray(Array(8).fill(0xcc));
    new NativeFunction(ptr(0x54edd0),'pointer',['pointer'],'thiscall')(timer);
    timerBytes=Array.from(new Uint8Array(timer.readByteArray(8)));
    for(const c of __CASES__){
      current={input:c,order:[],feedback:[],quick:[],quickArgs:[]};
      for(let i=0;i<3;i++){
        const catalogue=game.add(catalogues[i]),refs=alloc(32);for(let j=0;j<4;j++)ref(refs.add(j*8),'ZZCAT'+i+j);
        if(c.mask&(1<<i))ref(refs.add(ids[i]*8),'ZRCHIT');catalogue.writePointer(refs);catalogue.add(4).writeU32(4);
        const p=actor.add(offsets[i]),storage=alloc(128),rows=c.seed===0?[]:c.seed===1?[[ids[i],4,2,9],[99,7,3,8]]:[[ids[i],4,2,9],[ids[i],6,1,7]];
        rows.forEach((row,j)=>row.forEach((v,k)=>storage.add(j*16+k*4).writeU32(v)));
        p.writeU32(0);p.add(4).writePointer(storage);p.add(8).writePointer(storage.add(rows.length*16));p.add(12).writePointer(storage.add(128));
        p.add(16).writeU32(rows.reduce((n,r)=>n+r[2],0));p.add(20).writeU32(17);p.add(24).writeU32(19);
      }
      if(c.op===0)add(actor,input,c.feedback);else remove(actor,input);
      current.lists=lists();events.push(current);
    }
  }catch(e){error=String(e);}
  finally{for(const p of addresses)Interceptor.revert(p);Interceptor.flush();globals.forEach((p,i)=>p.writePointer(saved[i]));}
  return{events,error,getterCorrect,timerBytes,saveSpan,globalsRestored:globals.every((p,i)=>p.readPointer().equals(saved[i])),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""


def header(events,timer_bytes,save_span):
    def arr(values):return '{'+','.join(str(int(v)) for v in values)+'}'
    lines=['// Original special-ability control and real native catalogue/list state.','#ifndef SPECIAL_ABILITY_GOLDEN_H_','#define SPECIAL_ABILITY_GOLDEN_H_',
        'static const BYTE SPECIAL_TIMER_BYTES[] = '+arr(timer_bytes)+';',
        'static const UINT SPECIAL_SAVE_SPAN = '+str(save_span)+';',
        'struct SpecialListGolden { UINT count; UINT entries[3][4]; UINT totals[3]; };',
        'struct SpecialAbilityGolden { INT op,mask,seed; BOOL key; INT feedback; BOOL resource; SpecialListGolden lists[3]; INT feedbackId; BYTE quick[32]; };',
        'static const SpecialAbilityGolden SPECIAL_ABILITIES[] = {']
    for e in events:
        c=e['input'];values=[c[k] for k in ['op','mask','seed','key','feedback','resource']]
        states=['{'+str(len(s['entries']))+',{'+','.join(arr(r) for r in s['entries'])+'},'+arr(s['totals'])+'}' for s in e['lists']]
        literals=[str(int(v)) if int(v)!=-2147483648 else '(-2147483647-1)' for v in values]
        lines.append('    {'+','.join(literals)+',{'+','.join(states)+'},'+str(e['feedback'][0] if e['feedback'] else 0)+','+arr(e['quick'])+'},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(cases())),encoding='utf-8')
    paths=(exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('globalsRestored') and result.get('hooksRestored') and result.get('getterCorrect') and len(events)==len(cases()) and not any(m.get('type')=='error' for m in result['messages'])
    good=good and result.get('timerBytes')==[0,0,0,0,0,0,204,204] and result.get('saveSpan')==446
    for e in events:
        c=e['input'];feedback=c['op']==0 and c['key'] and c['feedback']==1
        expected=[] if not feedback else ['bind','demand']+(['name','feedback'] if c['resource'] else [])+['release']+(['cancel','release-object'] if c['resource'] else [])
        good=good and e['order']==expected
        if e['feedback']:good=good and e['feedback']==[56 if c['mask']&2 else 36,0,0,0,10453,0,0]
        if c['op']==1:good=good and len(e['quick'])==32 and e['quickArgs']==[-1,0,0]
        else:good=good and e['keyArgs']==['ZRCHIT',1006,1] and not e['quick']
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),expected=len(cases()),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events,result['timerBytes'],result['saveSpan']),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
