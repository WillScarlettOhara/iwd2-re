"""Frozen canonical level-up cancellation, reverse flow and done-button gates.

Targets and SetResRef execute unchanged. Numeric/string gate dependencies and
native skill/feat setters are real. UI, locks, object lookup, effects, stats
assignment/lifetime, MFC buffer allocation, script loading and spell-bank assignment are explicit service
doubles. Their receipts prove orchestration and operand identity, not callees.
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


def gate_cases():
    rows=[]
    for cls in [0,1,3,6,7,11,255]:
        for spec in [0,1,7,8,0x38,0x40,0x7fc0,0x8000,0xff8000,0x1000000,0xffffffff]:
            rows.append([6,cls,spec,0,0,0,0,255])
    for panel in [4,7,8,11,12,17,18,20,51,52,53,54,55,56,57]:
        for point in [-2147483648,-2,-1,0,1,2,2147483647]:
            for cls in [0,5]:
                rows.append([panel,cls,0,point,0,0,0,255])
    for small in [-1,0,1]:
        for large in [-1,0,1]: rows.append([19,0,0,0,small,large,0,255])
    for index in [0,3,7]:
        for race in [0,1,128,255]: rows.append([16,0,0,0,0,0,index,race])
    return rows


TEXTS=['',' ','\t\r\n','  Hero  ','\tHero\n','a b','\x01','\v\f',' aux ','con','prn',' A\tB ','x'*40]
PANELS=[4,6,7,8,9,10,11,12,13,16,17,18,19,20,50,51,52,53,54,55,56,57]

JS=r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],gates=[],texts=[],flows=[],cancels=[],scripts=[];
 let e=null,c=null,mode='gates',error=null;
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a,n){const p=ptr(a);globals.push([p,p.readByteArray(n)]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16)))]);Interceptor.replace(p,f);}
 function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
 function str(s){const p=Memory.allocUtf8String(s);roots.push(p);return p;}
 function ref(p,s){let b=Array.from(s.slice(0,8),x=>x.charCodeAt(0));while(b.length<8)b.push(0);p.writeByteArray(b);}
 function bytes(p,n){return Array.from(new Uint8Array(p.readByteArray(n)));}
 const screen=alloc(0x2000),globalScreen=alloc(0x2000),actor=alloc(0x9e00),base=alloc(0x37c),stats=alloc(0xe58),before=alloc(0xe58),game=alloc(0x6000),chitin=alloc(0x6000),panel=alloc(0x80),node=alloc(16),svt=alloc(0x100),avt=alloc(0x100),edit=alloc(128),evt=alloc(0x100),lvt=alloc(16),script=alloc(64),entries=alloc(16),zero=alloc(4);
 const gate=new NativeFunction(ptr(0x5e1f50),'int',['pointer','pointer'],'thiscall');
 const back=new NativeFunction(ptr(0x5e1b20),'int',['pointer','uint','pointer'],'thiscall');
 const cancel=new NativeFunction(ptr(0x5e3b80),'int',['pointer'],'thiscall');
 const setRef=new NativeFunction(ptr(0x514220),'void',['pointer','uint','uint'],'thiscall');
 const ctorString=new NativeFunction(ptr(0x7fcc88),'pointer',['pointer','pointer'],'thiscall');
 const dtorString=new NativeFunction(ptr(0x7fcc1a),'void',['pointer'],'thiscall');
 function reset(panelId){screen.writeByteArray(new Uint8Array(0x2000));actor.writeByteArray(new Uint8Array(0x9e00));base.writeByteArray(new Uint8Array(0x37c));stats.writeByteArray(new Uint8Array(0xe58));before.writeByteArray(new Uint8Array(0xe58));actor.writePointer(avt);screen.writePointer(svt);panel.add(0x20).writeU32(panelId);node.add(8).writePointer(panel);screen.add(0x632).writePointer(node);screen.add(0x85a).writePointer(base);screen.add(0x85e).writePointer(stats);screen.add(0x862).writePointer(before);game.add(0x3846).writeS16(1);game.add(0x382e).writeS32(123);actor.add(0x5c).writeS32(987);chitin.add(0x1033).writeU8(0);chitin.add(0x96e).writeU32(0);chitin.add(0x104c).writeU32(42);}
 try{
  save(0x8cf6dc,4).writePointer(chitin);save(0x8cf6d8,4).writePointer(chitin);chitin.add(0x1c54).writePointer(game);chitin.add(0x1c60).writePointer(globalScreen);
  replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(n,f,s)=>{throw Error('assert '+n+' '+s.readUtf8String());});
  replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
  replace(0x7fc984,'void',['pointer'],'mscdecl',p=>{if(mode==='cancel'){if(p.equals(base))e.frees.push('base');else if(p.equals(stats))e.frees.push('stats');else if(p.equals(before))e.frees.push('before');}});
  replace(0x7fca87,'void',['pointer','int'],'thiscall',(p,n)=>{const capacity=mode==='cancel'&&c.variant===2?Math.max(n,64):n;const b=alloc(capacity+13);b.writeS32(1);b.add(4).writeS32(n);b.add(8).writeS32(capacity);b.add(12).writeByteArray(new Uint8Array(capacity+1).fill(90));b.add(12+n).writeU8(0);p.writePointer(b.add(12));});
  replace(0x7fcb09,'void',['pointer'],'thiscall',()=>{});
  Interceptor.flush();
  for(const row of __GATES__){reset(row[0]);screen.add(0x106).writeU8(row[1]);screen.add(0x846).writeU32(row[2]);for(const off of [0x1778,0x7b6,0x7c2,0x812,0x798,0x646])screen.add(off).writeS32(row[3]);screen.add(0x80a).writeS32(row[4]);screen.add(0x806).writeS32(row[5]);screen.add(0x17a0).writeS32(row[6]);actor.add(0x7f7+row[6]).writeU8(row[7]);gates.push(gate(screen,actor));}
  replace(0x4d2ca0,'pointer',['pointer','uint'],'thiscall',(p,id)=>{if(!p.equals(panel)||id!==6)throw Error('edit identity');return edit;});
  edit.writePointer(evt);evt.add(0x6c).writePointer(cb('pointer',['pointer','pointer'],(p,out)=>{ctorString(out,str(c));return out;}));Interceptor.flush();
  for(c of __TEXTS__){reset(13);texts.push(gate(screen,actor));}
  for(const input of ['','ABCDEFGH','aBcDeFgH','ZRCHIT']){script.writeByteArray(new Uint8Array(64).fill(0xa5));const data=alloc(8);ref(data,input);setRef(script,data.readU32(),data.add(4).readU32());scripts.push({input,ref:bytes(script,8),tail:bytes(script.add(8),56)});}
  mode='flow';
  replace(0x445330,'pointer',['pointer','pointer'],'thiscall',(p,r)=>{e.assigns.push([p.equals(actor.add(0x920)),r.equals(stats)]);p.writeByteArray(r.readByteArray(0xe58));return p;});
  replace(0x72de60,'int',['pointer'],'thiscall',p=>{e.effects++;p.add(0x5c0).writeS16(-10);p.add(0x5c2).writeU16(12);p.add(0x924).writeS16(-14);p.add(0x7234).writeS32(16);return 1;});
  replace(0x5e0fd0,'int',['pointer','uint','pointer'],'thiscall',(p,id,s)=>{e.visited.push(id);return +(id===c.accept);});
  replace(0x5e0e70,'void',['pointer','uint','pointer','int'],'thiscall',(p,id,s,flag)=>e.popup.push([id,flag,s.equals(actor)]));
  replace(0x5e1df0,'void',['pointer','pointer'],'thiscall',(p,s)=>{e.dismiss.push([panel.add(0x20).readU32(),s.equals(actor)]);screen.add(0x632).writePointer(e.dismiss.length<c.stack?node:ptr(0));});
  replace(0x4f7500,'int',['pointer','pointer','int'],'thiscall',(p,msg,force)=>{e.messages.push([msg.add(4).readS32(),msg.add(8).readS32(),msg.add(12).readU32(),msg.add(16).readU8(),force]);return 1;});
  replace(0x518660,'void',['pointer','uint','uint8'],'thiscall',(p,id,value)=>e.ready.push([id,value]));Interceptor.flush();
  function record(){return {base:bytes(actor.add(0x5a4),0x37c),hp:[actor.add(0x5c0).readS16(),actor.add(0x5c2).readU16(),actor.add(0x924).readS16(),actor.add(0x7234).readS32()],state:actor.add(0x920).readU32()};}
  function initial(){base.add(0x18).writeU32(0x4000);base.add(0x1c).writeS16(-32768);base.add(0x1e).writeU16(65535);stats.add(4).writeS16(32767);stats.writeU32(0x1000);screen.add(0x866).writeS32(-77);actor.add(0x920).writeU32(c.dead?0x800:0);}
  for(const start of [6,7,55,56,8,16,54])for(const accept of [2,54,6,7,55,56,8,16,-1])for(const host of [0,1,2])for(const service of [0,1])for(const dead of [0,1]){
   c={start,accept,host,service,dead,stack:3};e={input:c,visited:[],popup:[],dismiss:[],assigns:[],effects:0,messages:[],ready:[]};reset(start);initial();chitin.add(0x1033).writeU8(host);chitin.add(0x96e).writeU32(service);e.result=back(screen,start,actor);Object.assign(e,record());flows.push(e);
  }
  mode='cancel';
  replace(0x5e1b20,'int',['pointer','uint','pointer'],'thiscall',(p,id,s)=>{e.back.push([id,s.equals(actor)]);return 1;});
  replace(0x7e6cbb,'void',['pointer','pointer','int'],'thiscall',()=>{});
  replace(0x7e6ce2,'int',['pointer','uint'],'thiscall',()=>1);
  replace(0x7e6cf7,'int',['pointer'],'thiscall',()=>{e.unlocks++;return 1;});
  replace(0x599c70,'uint8',['pointer','int','uint8','pointer','uint'],'thiscall',(p,id,thread,out)=>{e.denies.push([id,thread]);out.writePointer(actor);return c.lock===2&&e.denies.length<3?e.denies.length+3:c.lock===1?3:0;});
  replace(0x59a010,'uint8',['pointer','int','uint8','uint'],'thiscall',(p,id,thread)=>{e.release.push([id,thread]);return 0;});
  replace(0x4d45e0,'void',['pointer','pointer'],'thiscall',()=>{e.invalidates++;});
  replace(0x5d6650,'void',['pointer'],'thiscall',()=>{});
  lvt.add(4).writePointer(cb('pointer',['pointer','uint'],(p,flag)=>{e.deletedLists++;return p;}));
  avt.add(0xbc).writePointer(cb('void',['pointer','pointer','int','int'],(p,t,live,start)=>{e.ai=[t.equals(screen.add(0x12c)),live,start];}));
  avt.add(0x94).writePointer(cb('void',['pointer','int','pointer'],(p,level,s)=>{e.script=[level,bytes(s,8)];}));
  for(const address of [0x5f97f0,0x5f9a80])replace(address,'pointer',['pointer','int'],'thiscall',p=>p);
  replace(0x40f160,'void',['pointer','uint','uint','int'],'thiscall',(p,lo,hi,player)=>{e.scriptRead=[lo,hi,player];});
  for(const [a,off,n] of [[0x4450a0,0x4284,0x700],[0x444d10,0x4984,0x100]])replace(a,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{e.bankCopies.push([p.equals(actor.add(off)),r.equals(stats.add(off===0x4284?0x5ac:0xcac))]);p.writeByteArray(r.readByteArray(n));out.writeByteArray(new Uint8Array(n));return out;});
  replace(0x7e78de,'void',['pointer','uint','uint','pointer'],'stdcall',()=>{});
  replace(0x545b70,'uint',['pointer','uint'],'thiscall',(p,id)=>{e.featRows.push(id);return 74-id;});
  Interceptor.flush();
  for(const id of __PANELS__)for(const lock of [0,1,2])for(const variant of [0,1,2]){
   c={id,lock,variant,stack:1,dead:variant};e={input:c,assigns:[],effects:0,dismiss:[],back:[],messages:[],ready:[],denies:[],release:[],unlocks:0,invalidates:0,frees:[],deletedLists:0,ai:[],script:[],scriptRead:[],bankCopies:[],featRows:[]};reset(id);initial();screen.add(0x28).writeS32(variant?65536:1);screen.add(0x77c).writeU32(0x87654321);globalScreen.add(0x846).writeU32(0);actor.add(0x80c).writeU32(123);
   for(let j=0;j<8;j++)base.add(0x253+j).writeU8(j+10);
   for(let j=0;j<6;j++)base.add(0x25e+j).writeU8(j+20);
   for(let j=0;j<16;j++){screen.add(0x2c0+j*4).writeS32(j+3);actor.add(0x7b4+j).writeU8(90);}
   for(let j=0;j<75;j++)screen.add(0x650+j*4).writeS32(0);
   for(const off of [0x7be,0x7e2,0x80e,0x84a]){const list=alloc(32);list.writePointer(lvt);screen.add(off).writePointer(list);}
   for(let j=0;j<9;j++){const list=screen.add(0x1ba+j*0x1c);list.add(4).writePointer(entries);list.add(8).writePointer(entries.add(j%2?0:16));list.add(12).writePointer(entries.add(16));list.add(16).writeU32(100+j);list.add(20).writeU32(200+j);list.add(24).writeU32(300+j);}screen.add(0x2b6).writeU32(8);
   ref(screen.add(0x81b),'SMALL');ref(screen.add(0x823),'LARGE');ref(screen.add(0x84e),'SOUNDS');
   ctorString(screen.add(0x832),str('SCRIPT'));ctorString(screen.add(0x856),str(variant===1?'X'.repeat(40):'voice'));
   e.result=cancel(screen);Object.assign(e,record());e.spec=[actor.add(0x80c).readU32(),screen.add(0x846).readU32(),globalScreen.add(0x846).readU32()];e.snapshotNull=[0x85a,0x85e,0x862].map(o=>screen.add(o).readPointer().isNull());e.snapshotHP=screen.add(0x866).readS32();e.topSkill=screen.add(0x2ba).readU16();e.skills=bytes(actor.add(0x7b4),16);e.portraits=bytes(actor.add(0x5d0),16);e.tempPortraits=bytes(screen.add(0x836),16);e.sound=bytes(actor.add(0x7252),40);e.scriptName=bytes(screen.add(0x832).readPointer(),6);e.spellLists=Array.from({length:9},(_,j)=>{const p=screen.add(0x1ba+j*0x1c);return [p.add(8).readPointer().sub(p.add(4).readPointer()).toInt32()/16,p.add(16).readU32(),p.add(20).readU32(),p.add(24).readU32()];});e.highest=screen.add(0x2b6).readU32();cancels.push(e);dtorString(screen.add(0x832));dtorString(screen.add(0x856));
  }
 }catch(ex){error=mode+': '+String(ex);}
 finally{for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,b]of globals)p.writeByteArray(b);}
 return{gates,texts,flows,cancels,scripts,error,hooksRestored:patches.every(([p,b])=>JSON.stringify(bytes(p,16))===JSON.stringify(b)),globalsRestored:globals.every(([p,b])=>JSON.stringify(bytes(p,b.byteLength))===JSON.stringify(Array.from(new Uint8Array(b))))};
};
"""


def receipts_match(r):
    cycle=[2,54,6,7,55,56,8,16]
    for e in r.get('flows',[]):
        c=e['input'];panel=c['start'];visited=[];restored=False
        while True:
            if panel==6:restored=True
            panel=cycle[(cycle.index(panel)-1)%len(cycle)];visited.append(panel)
            if panel==c['accept'] or panel==2:break
        popup=panel==c['accept']
        if e['visited']!=visited or e['result']!=int(popup):return False
        if e['popup']!=([[panel,0,True]] if popup else []):return False
        if len(e['dismiss'])!=(0 if popup else 3):return False
        if e['effects']!=int(restored) or e['assigns']!=([[True,True]] if restored else []):return False
        if restored and (e['hp']!=[-32768,65535,32767,-77] or e['state']!=(0x1000|(0x800 if c['dead'] else 0))):return False
        if e['messages']!=([[123,123,42,0,0]] if not popup and c['host']==0 and c['service'] else []):return False
        if e['ready']!=([[42,0]] if not popup and c['host']==1 else []):return False
    for e in r.get('cancels',[]):
        c=e['input'];ok=c['lock']!=1;pid=123 if c['variant'] else -1
        if e['result']!=int(ok) or e['release']!=([[pid,0]] if ok else []):return False
        if len(e['denies'])!=(3 if c['lock']==2 else 1) or e['invalidates']!=int(ok):return False
        if e['unlocks']!=2 or len(e['dismiss'])!=int(ok):return False
        if e['denies']!=[[pid,0]]*(3 if c['lock']==2 else 1):return False
        if e['back']!=([[c['id'],True]] if ok and c['id'] in [6,7,8,16,54,55,56] else []):return False
        if ok and c['id']==54 and (e['snapshotNull']!=[True]*3 or e['snapshotHP']!=0 or e['frees']!=['base','stats','before'] or e['ai']!=[True,1,1]):return False
        if ok and c['id']==54 and (e['hp']!=[-32768,65535,32767,-77] or e['state']!=(0x1000|(0x800 if c['variant'] else 0)) or e['spec'][2]!=0x87654321):return False
        if ok and c['id']==6 and e['spec'][:2]!=[0x87654321]*2:return False
        if ok and c['id']==8 and e['bankCopies']!=[[True,True],[True,True]]:return False
        if ok and c['id']==8 and (e['highest']!=0 or e['spellLists']!=[[0,100+j,200+j if j%2 else 0,300+j if j%2 else 0] for j in range(9)]):return False
        if ok and c['id']==55 and (e['skills']!=list(range(3,19)) or e['topSkill']!=16):return False
        if ok and c['id']==56 and e['featRows']!=list(range(75)):return False
        if ok and c['id'] in [11,13,19,20] and e['deletedLists']!=1:return False
        if ok and c['id']==17 and e['script']!=[3,list(b'SCRIPT\0\0')]:return False
        if ok and c['id'] in [17,20]:
            sound=list(b'SOUNDS\0\0')+list(b'X'*32 if c['variant']==1 else b'voice\0'+(b'Z'*26 if c['variant']==2 else b'\0'*26))
            if e['sound']!=sound:return False
        if ok and c['id'] in [17,18] and (e['portraits']!=list(b'SMALL\0\0\0LARGE\0\0\0') or e['tempPortraits']!=e['portraits']):return False
    for e in r.get('scripts',[]):
        if e['ref']!=list(e['input'].encode().ljust(8,b'\0')) or e['tail']!=[0xa5]*56:return False
    return True


def header(r):
    lines=['// Generated by cancel_levelup_reference.py from canonical IWD2.exe.','#ifndef CANCEL_LEVELUP_GOLDEN_H_','#define CANCEL_LEVELUP_GOLDEN_H_',
           'struct DoneGateGolden { DWORD panel; BYTE cls; DWORD spec; INT points,smallPortrait,largePortrait,index; BYTE race; BOOL result; };','static const DoneGateGolden DONE_GATE_CASES[] = {']
    for row,result in zip(gate_cases(),r['gates']):
        vals=[str(v) for v in row]+[str(result)];vals[2]+='u';vals[3]='(-2147483647-1)' if row[3]==-2147483648 else vals[3]
        lines.append('    {'+','.join(vals)+'},')
    quoted=[json.dumps(s).replace('\\u0001','\\001').replace('\\u000b','\\v') for s in TEXTS]
    lines+=['};','static const char* const DONE_GATE_TEXTS[] = {']+[f'    {s},' for s in quoted]+['};','static const BOOL DONE_GATE_TEXT_RESULTS[] = { '+','.join(map(str,r['texts']))+' };','#endif','']
    return '\n'.join(lines)


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);script=args.out/'original.js'
    script.write_text(JS.replace('__GATES__',json.dumps(gate_cases())).replace('__TEXTS__',json.dumps(TEXTS)).replace('__PANELS__',json.dumps(PANELS)),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script];hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    r=capture(frida,exe,game_directory(),script,90)
    good=not r['error'] and not r['cleanup_errors'] and not r['messages'] and r.get('hooksRestored') and r.get('globalsRestored')
    good=good and len(r.get('gates',[]))==len(gate_cases()) and len(r.get('texts',[]))==len(TEXTS) and len(r.get('flows',[]))==756 and len(r.get('cancels',[]))==198 and len(r.get('scripts',[]))==4
    good=good and receipts_match(r) and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(r,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',gates=len(r.get('gates',[])),texts=len(r.get('texts',[])),flows=len(r.get('flows',[])),cancels=len(r.get('cancels',[])),scripts=len(r.get('scripts',[])),error=r['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good:
        records=[dict(op='native-done-gate',case=i,input=row,result=value) for i,(row,value) in enumerate(zip(gate_cases(),r['gates']))]
        records += [dict(op='native-done-text',case=i,result=value) for i,value in enumerate(r['texts'])]
        (args.out/'done-gates.jsonl').write_text(''.join(json.dumps(row)+'\n' for row in records),encoding='utf-8')
    if good and args.header:args.header.write_text(header(r),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
