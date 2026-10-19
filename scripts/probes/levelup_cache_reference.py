"""Bounded investigation of native level-stat HP cache stack residue.

5D8EA0 executes unchanged. An entry hook seeds its ASM-proven uninitialized
stack slot; stats lifetime/assignment, rule calculations and effects are explicit
service doubles. This proves the cache branch's stack dependence, not gameplay
or any doubled dependency. Private process/memory only; never resume or save.
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

JS=r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],events=[];
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a){const p=ptr(a);globals.push([p,p.readU32()]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),cb=new NativeCallback(fn,ret,args,abi);callbacks.push(cb);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16)))]);Interceptor.replace(p,cb);}
 const actor=alloc(0x9e00),screen=alloc(0x2000),base=alloc(0x37c),game=alloc(0x6000),chitin=alloc(0x6000);
 let marker=0,rolls=0,error=null,hook=null;
 const apply=new NativeFunction(ptr(0x5d8ea0),'void',['pointer','pointer'],'thiscall');
 try{
  save(0x8cf6dc).writePointer(chitin);chitin.add(0x1c54).writePointer(game);
  replace(0x443b30,'pointer',['pointer'],'thiscall',p=>{p.writeByteArray(new Uint8Array(0xe58));return p;});
  replace(0x445330,'pointer',['pointer','pointer'],'thiscall',(p,r)=>{p.writeByteArray(r.readByteArray(0xe58));return p;});
  replace(0x5437e0,'void',['pointer','uint8','pointer','pointer'],'thiscall',(p,cls,s)=>{s.add(0x46).writeU8(2);s.add(0x47+cls-1).writeU8(2);});
  replace(0x5429b0,'int',['pointer','pointer','pointer','uint8','pointer'],'thiscall',()=>0);
  replace(0x542fa0,'int',['pointer','pointer','pointer','pointer','uint8'],'thiscall',()=>{rolls++;return 7;});
  replace(0x546b60,'void',['pointer','pointer','pointer','pointer','pointer','int'],'thiscall',(p,s,a,b,c)=>{a.writeS32(9);b.writeS32(1);c.writeS32(6);});
  replace(0x544930,'int',['pointer','pointer','pointer'],'thiscall',()=>0);
  replace(0x72de60,'int',['pointer'],'thiscall',()=>1);
  for(const a of [0x4e71b0,0x4e6e80,0x7fb298,0x5f9870,0x5f99a0,0x5f98b0,0x443420,0x443650,0x4435e0,0x443570,0x443500,0x443490])
   replace(a,'void',['pointer'],'thiscall',()=>{});
  replace(0x7e78de,'void',['pointer','uint','uint','pointer'],'stdcall',()=>{});
  replace(0x5f9aa0,'pointer',['pointer','pointer'],'thiscall',(p,out)=>{out.writePointer(ptr(0));return out;});
  replace(0x55bb40,'pointer',['pointer','pointer','pointer','pointer'],'thiscall',(p,out)=>{out.writePointer(ptr(0));return out;});
  replace(0x7fc984,'void',['pointer'],'mscdecl',()=>{});
  hook=Interceptor.attach(ptr(0x5d8ea0),{onEnter(){this.context.esp.sub(0xe84).writeS32(marker);}});
  Interceptor.flush();
  for(const hit of [0,1])for(marker of [-32768,-10,0,31,62,32767]){
   actor.writeByteArray(new Uint8Array(0x9e00));screen.writeByteArray(new Uint8Array(0x2000));
   screen.add(0x106).writeU8(5);screen.add(0x85a).writePointer(base);
   actor.add(0x5c0).writeS16(100);actor.add(0x5c2).writeU16(100);
   actor.add(0x966).writeU8(1);actor.add(0x96b).writeU8(1);actor.add(0x62b).writeU8(1);
   actor.add(0x7286).writeU8(hit?2:0);actor.add(0x728e).writeS32(77);
   rolls=0;apply(screen,actor);
   events.push({hit,marker,rolls,hp:actor.add(0x5c0).readS16(),baseMax:actor.add(0x5c2).readU16(),screenDelta:screen.add(0x86a).readS32(),cached:actor.add(0x728e).readS32()});
  }
 }catch(ex){error=String(ex);}
 finally{if(hook)hook.detach();for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,v]of globals)p.writeU32(v);}
 return{events,error,hooksRestored:patches.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(16))))===JSON.stringify(b)),globalsRestored:globals.every(([p,v])=>p.readU32()===v)};
};
"""


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--out',required=True,type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script]
    hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    r=capture(frida,exe,game_directory(),script,45)
    good=not r['error'] and not r['cleanup_errors'] and not r['messages'] and r.get('hooksRestored') and r.get('globalsRestored') and len(r['events'])==12
    for e in r['events']:
        value=e['marker'] if e['hit'] else 7
        hp=((100+value+32768)%65536)-32768
        good=good and e['rolls']==(0 if e['hit'] else 1) and e['hp']==hp and e['baseMax']==(100+value)%65536 and e['screenDelta']==(77 if e['hit'] else 7)
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(r,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',error=r['error'],cases=len(r['events']),fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2));return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
