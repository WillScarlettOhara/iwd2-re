"""Frozen canonical LevelUp, CheckDropSlot and OnKeyDown, with bounded dependency doubles.

The target bodies execute unchanged. Resource, allocator, UI, and inventory service
boundaries are replaced, recorded and restored before the spawned process is killed.
This proves orchestration/widths/branches, not the doubled callees themselves.
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
    for cls in range(1, 12):
        for spec in [0,1,2,4,8,16,32,7,56,0x8001,0x8010,0x800000]:
            for flags in [0,12]:
                rows.append(dict(cls=cls,spec=spec,flags=flags,points=513,oldCleric=0,
                                 seed=0,oldMax=3,newMax=3,high=0))
    for cls in [2,10,4]:
        for previous,current in [(0,0),(0,4),(4,1),(2,5)]:
            for seed in [0,1,2]:
                rows.append(dict(cls=cls,spec=0,flags=0,points=-1,oldCleric=1,
                                 seed=seed,oldMax=previous,newMax=current,high=0))
    for cls in range(1,12):
        rows.append(dict(cls=cls,spec=0x800000,flags=0,points=256,oldCleric=1,
                         seed=0,oldMax=3,newMax=3,high=7))
    for current in [0,2,32768,65535]:
        for caster in [0,1,5,255]:
            rows.append(dict(cls=1,spec=0,flags=0,points=0,oldCleric=0,seed=0,
                             oldMax=3,newMax=3,high=0,rage=current+1,caster=caster))
    for alignment in [0x11,0x12,0x13,0x21,0x22,0x23,0x31,0x32,0x33]:
        for flag in [0,1,2,4,8,16,32,63,0x4000003f]:
            rows.append(dict(cls=3,spec=0x8000,flags=0,points=0,oldCleric=1,
                             seed=0,oldMax=3,newMax=3,high=0,domainFlag=flag,alignment=alignment))
    for row in rows:
        if row['cls']==3 and not row['spec'] & 0xff8000:
            row['spec'] |= 0x8000
        row.setdefault('rage',0);row.setdefault('caster',1)
        row.setdefault('domainFlag',-1);row.setdefault('alignment',0x22)
    return rows


def drop_cases():
    return [dict(slot=slot,present=present,usable=usable,slots=slots,swap=swap,inventory=inventory)
            for slot in [0,11,12,13,14,43,44,49,50]
            for present,usable,slots,swap,inventory in [(0,1,2,1,1),(1,1,8,1,1),
                (1,0,2,0,1),(1,0,2,1,1),(1,0,2,1,0)]]


def key_cases():
    rows=[dict(key=k,popup=p,handled=h,ctrl=0,binding=-1,count=1)
          for p in [-1,18,51] for h in [0,1] for k in range(256)]
    rows += [dict(key=80,popup=-1,handled=0,ctrl=ctrl,binding=i,count=1)
             for i in range(30) for ctrl in [0,1]]
    rows += [dict(key=9,popup=-1,handled=0,ctrl=0,binding=-1,count=n) for n in [-32768,-1,0]]
    return rows


JS = r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],events=[],drops=[],keys=[];let c=null,e=null,error=null;
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a,n){const p=ptr(a);globals.push([p,p.readByteArray(n)]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16))).join(',')]);Interceptor.replace(p,f);}
 function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
 function ref(p,s){let b=Array.from(s.slice(0,8),x=>x.charCodeAt(0));while(b.length<8)b.push(0);p.writeByteArray(b);}
 const actor=alloc(0x9e00),game=alloc(0x6000),chitin=alloc(0x6000),screen=alloc(0x2000),vtable=alloc(0x100),old=alloc(0xe58),before=alloc(0xe58),base=alloc(0x400),item=alloc(0x100),master=alloc(16),innateRefs=alloc(16),resource=alloc(128),abilities=alloc(80);
 const levelup=new NativeFunction(ptr(0x5e56f0),'void',['pointer','pointer'],'thiscall');
 const drop=new NativeFunction(ptr(0x5e6a30),'void',['pointer','int'],'thiscall');
 function list(p,n){const data=alloc(256);p.add(4).writePointer(data);p.add(8).writePointer(data.add(n*16));p.add(12).writePointer(data.add(256));
   p.add(16).writeU32(2);p.add(20).writeU32(9);p.add(24).writeU32(19);
   for(let j=0;j<n;j++){const d=data.add(j*16);d.writeU32(j);d.add(4).writeU32(7+j);d.add(8).writeU32(2+j);d.add(12).writeU32(12+j);}return data;}
 function snapshot(){const cl=c.cls===2?0:5;return Array.from({length:9},(_,l)=>{const p=actor.add(0x4284+cl*0x100+l*0x1c),a=p.add(4).readPointer(),b=p.add(8).readPointer();
   return [p.add(16).readU32(),p.add(20).readU32(),p.add(24).readU32(),...Array.from(new Uint32Array(a.readByteArray(b.sub(a).toInt32())))];});}
 try{
   save(0x8cf6dc,4).writePointer(chitin);chitin.add(0x1c54).writePointer(game);
   if(ptr(0x85186c).readU8()!==0 || ptr(0x85bd86).readU8()!==43)throw Error('unexpected constants');
   game.add(0x4bf8).writePointer(master);game.add(0x4bfc).writeU32(2);ref(master,'ZRCHIT');ref(master.add(8),'ZRCHIT2');
   ref(save(0x8f9410,8),'SPIN236');ref(save(0x8f8e20,8),'SPIN260');ref(innateRefs,'SPIN236');ref(innateRefs.add(8),'SPIN260');
   game.add(0x4c00).writePointer(innateRefs);game.add(0x4c04).writeU32(2);
   game.add(0x3846).writeS16(1);game.add(0x382e).writeS32(123);
   replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(n,f,s)=>{throw Error('assert '+n+' '+s.readUtf8String());});
   replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
   for(const a of [0x78ad40,0x78ac30])replace(a,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{p.writeByteArray(r.readByteArray(8));out.writeByteArray(r.readByteArray(8));return out;});
   replace(0x7fc984,'void',['pointer'],'mscdecl',()=>{});
   replace(0x7fc95b,'pointer',['uint'],'mscdecl',n=>alloc(n));
   replace(0x5d6650,'void',['pointer'],'thiscall',()=>{});
   for(const a of [0x71e240])replace(a,'void',['pointer','pointer','pointer'],'thiscall',()=>{e.pre++;});
   replace(0x71e760,'void',['pointer','pointer','int'],'thiscall',()=>{e.special++;});
   replace(0x71e9f0,'void',['pointer','pointer'],'thiscall',(p,cl)=>e.divine.push(cl.readU8()));
   replace(0x5e6950,'void',['pointer'],'thiscall',()=>{e.drop++;});
   replace(0x72de60,'int',['pointer'],'thiscall',()=>{e.process++;return 1;});
   vtable.add(0x10).writePointer(cb('pointer',['pointer'],()=>actor.add(0x20)));
   replace(0x5431e0,'int',['pointer','pointer','pointer','pointer','uint','uint','pointer'],'thiscall',(p,cl,t,d,s,l,b)=>{e.max.push([cl.readU8(),l,s,t.equals(actor.add(0x20)),d.equals(actor.add(0x920))]);b.writeS32(0);return d.equals(old)?c.oldMax:c.newMax;});
   replace(0x71aba0,'void',['pointer','pointer','int','int','int'],'thiscall',()=>{});
   replace(0x543770,'int',['pointer','pointer','int'],'thiscall',(p,b,l)=>b.equals(base)?c.oldMax:c.newMax);
   replace(0x547130,'uint',['pointer','pointer','pointer'],'thiscall',()=>c.domainFlag>=0?1:0);
   replace(0x5ca910,'uint',['pointer','pointer'],'thiscall',()=>0);
   replace(0x71b460,'void',['pointer','int'],'thiscall',()=>{e.unequip++;});
   replace(0x71b5e0,'void',['pointer','int'],'thiscall',()=>{e.equip++;});
   replace(0x6a1800,'pointer',['uint','pointer','uint16'],'mscdecl',(id)=>{e.animation=id;return ptr(0);});
   replace(0x4699f0,'void',['pointer','pointer','int','int'],'thiscall',(p,r)=>{p.add(4).writePointer(resource);p.add(8).writeByteArray(r.readByteArray(8));});
   replace(0x54a380,'int',['pointer'],'thiscall',()=>1);replace(0x54a3c0,'int',['pointer'],'thiscall',()=>1);
   replace(0x54a3e0,'int',['pointer'],'thiscall',()=>2);replace(0x54a4a0,'pointer',['pointer','int'],'thiscall',(p,i)=>abilities.add(i*40));
   replace(0x54acd0,'int',['pointer'],'thiscall',()=>10453);replace(0x54a9e0,'uint',['pointer'],'thiscall',()=>c.domainFlag>>>0);
   replace(0x75e940,'uint',['pointer','pointer','int','int'],'thiscall',()=>c.caster);
   replace(0x7fcd57,'pointer',['pointer','pointer'],'thiscall',p=>p);replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
   replace(0x78acc0,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out)=>{ref(p,'RAGEICON');ref(out,'RAGEICON');return out;});
   replace(0x787ce0,'void',['pointer','pointer'],'thiscall',()=>{});replace(0x77e370,'void',['pointer'],'thiscall',()=>{});
   replace(0x724b10,'uint8',['pointer','pointer','pointer','pointer','pointer'],'thiscall',(p,r,mx,cur,sh)=>{const l=actor.add(0x4a84),end=l.add(8).readPointer();end.writeU32(1);end.add(4).writeU32(mx.readU32());end.add(8).writeU32(cur.readU32());end.add(12).writeU32(sh.readU32());l.add(8).writePointer(end.add(16));return 1;});
   replace(0x725210,'uint8',['pointer','pointer','pointer','pointer'],'thiscall',(p,r,n)=>{const d=actor.add(0x4a88).readPointer().add(16);d.add(8).writeU32(d.add(8).readU32()+n.readU32());return 1;});
   replace(0x725270,'uint8',['pointer','pointer','pointer','pointer'],'thiscall',()=>1);
   replace(0x724ea0,'uint8',['pointer','pointer','pointer','pointer','pointer'],'thiscall',()=>{const l=actor.add(0x4a84),d=l.add(4).readPointer();d.writeByteArray(d.add(16).readByteArray(16));l.add(8).writePointer(d.add(16));return 1;});
   replace(0x724a40,'uint8',['pointer','pointer','pointer','pointer','pointer','pointer'],'thiscall',()=>{e.domain++;return 1;});
   for(const [a,kind]of [[0x584e30,0],[0x584eb0,1],[0x585190,2],[0x5851b0,3]])replace(a,'uint8',['pointer'],'mscdecl',()=>+(kind===0?c.alignment%16===1:kind===1?c.alignment%16===3:kind===2?(c.alignment>>4)===1:(c.alignment>>4)===3));
   Interceptor.flush();
   for(c of __CASES__){
     e={input:c,pre:0,special:0,divine:[],drop:0,process:0,max:[],unequip:0,equip:0,animation:-1,domain:0};
     actor.writeByteArray(new Uint8Array(0x9e00));screen.writeByteArray(new Uint8Array(0x2000));old.writeByteArray(new Uint8Array(0xe58));before.writeByteArray(new Uint8Array(0xe58));
     actor.writePointer(vtable);screen.add(0x106).writeU8(c.cls);screen.add(0x846).writeU32(c.spec);actor.add(0x80c).writeU32(c.spec);actor.add(0x89f).writeU8(c.flags);
     screen.add(0x798).writeS32(c.points);screen.add(0x85a).writePointer(base);screen.add(0x85e).writePointer(old);screen.add(0x862).writePointer(before);screen.add(0x866).writeS32(77);screen.add(0x86a).writeS32(88);
     actor.add(0x920+0x47+c.cls-1).writeU8(10);actor.add(0x920+0x47+5).writeU8(c.cls===6?10:1);actor.add(0x920+0x47+6).writeU8(c.cls===7?10:1);
     before.add(0x47+2).writeU8(c.oldCleric);actor.add(0x5f3).writeU8(126);
     const bits=[0,0x100,0x300,0,0,0x100,0x100,0x100,0x100,0x300,0x200,0x200][c.cls];actor.add(0x5c4).writeU32(0x6000|bits|(c.high<<16));
     for(let cl=0;cl<7;cl++)for(let l=0;l<9;l++)list(actor.add(0x4284+cl*0x100+l*0x1c),c.seed);
     list(actor.add(0x4abc),2);
     const inn=list(actor.add(0x4a84),c.rage?1:0);if(c.rage){actor.add(0x967).writeU8(15);inn.add(4).writeU32(65536);inn.add(8).writeU32(c.rage-1);inn.add(12).writeU32(12);actor.add(0x5c4).writeU32(0x6100);for(let j=0;j<9;j++)ref(actor.add(0x38dc+j*60+0x20),j%2?'OTHER':'SPIN236');}
     abilities.writeByteArray(new Uint8Array(80));abilities.add(2).writeU16(4);abilities.add(12).writeU8(7);abilities.add(16).writeU16(1);abilities.add(42).writeU16(4);abilities.add(52).writeU8(9);abilities.add(56).writeU16(5);
     actor.add(0x1778+0xe54).writeU32(0x102);const domain=alloc(8);domain.writeU32(0);game.add(0x4e2c).writePointer(domain);game.add(0x4e30).writeU32(1);
     levelup(screen,actor);
     e.points=actor.add(0x8a3).readU8();e.flags=actor.add(0x89f).readU8();e.electric=actor.add(0x5f3).readU8();e.animationWord=actor.add(0x5c4).readU32();e.dirty=actor.add(0x562c).readS32();
     e.cleared=[0x85a,0x85e,0x862,0x866,0x86a].every(o=>screen.add(o).readU32()===0);e.spells=snapshot();const sh=actor.add(0x4abc);e.shape=[sh.add(16).readU32(),sh.add(20).readU32(),sh.add(24).readU32(),...Array.from(new Uint32Array(sh.add(4).readPointer().readByteArray(32)))];
     e.rageButton=[actor.add(0x38dc+0x18).readS16(),actor.add(0x38dc+0x3a).readU8(),actor.add(0x38dc+0x28).readS16(),actor.add(0x38dc+0x1a).readS16()];events.push(e);
   }
   replace(0x5bb600,'void',['pointer','uint16','int16','pointer','pointer','pointer','pointer','pointer','int'],'thiscall',(p,portrait,slot,out)=>{e.info=[portrait,slot];out.writePointer(c.present?item:ptr(0));});
   replace(0x5bace0,'int16',['pointer','uint16'],'thiscall',()=>c.slots);
   replace(0x5b9c60,'int',['pointer','uint16','pointer','pointer','int'],'thiscall',()=>{e.usability++;return c.usable;});
   replace(0x599c70,'uint8',['pointer','int','uint8','pointer','uint'],'thiscall',(p,id,t,out)=>{e.deny++;out.writePointer(actor);return 0;});
   replace(0x59a010,'uint8',['pointer','int','uint8','uint'],'thiscall',()=>{e.release++;return 0;});
   replace(0x7187e0,'int16',['pointer','int16','int16'],'thiscall',(p,slot,a)=>{e.launcher=[slot,a];return 45;});
   replace(0x5b81c0,'int',['pointer','uint16','int16','pointer','pointer','uint16','uint8'],'thiscall',(p,portrait,slot,out)=>{e.swap=[portrait,slot];out.writePointer(c.swap?item:ptr(0));return c.swap;});
   replace(0x5bad70,'int',['pointer','uint16','pointer','int16','pointer','uint16','uint8','int'],'thiscall',(p,portrait,out,slot)=>{e.inventory=[portrait,slot];return c.inventory;});
   replace(0x5b75c0,'int',['pointer','int'],'thiscall',(p,id)=>{e.pile=id;return 456;});
   replace(0x5b7850,'int',['pointer','int','int16','pointer','pointer','uint16','uint8'],'thiscall',(p,id,slot)=>{e.ground=[id,slot];return 1;});
   replace(0x4e7840,'pointer',['pointer'],'thiscall',p=>{p.writeByteArray(new Uint8Array(0xee));return p;});
   replace(0x4e8350,'void',['pointer','pointer','int'],'thiscall',()=>{});replace(0x4e8180,'void',['pointer'],'thiscall',()=>{});
   replace(0x4f7500,'int',['pointer','pointer','int'],'thiscall',(p,msg)=>{e.message=[msg.add(0xfa).readU16(),msg.add(0xfc).readU8(),msg.add(4).readS32(),msg.add(8).readS32()];return 1;});
   Interceptor.flush();
   for(c of __DROPS__){e={input:c,usability:0,deny:0,release:0};actor.writeByteArray(new Uint8Array(0x9e00));screen.add(0x28).writeS32(0);for(let j=0;j<8;j++){actor.add(0x342c+j*60).writeU32(0x11223344);actor.add(0x3d3a+j).writeU8(99);}drop(screen,c.slot);
     e.buttons=Array.from({length:8},(_,j)=>actor.add(0x342c+j*60).readU32());e.cache=Array.from(new Uint8Array(actor.add(0x3d3a).readByteArray(8)));drops.push(e);}
   const keyboard=new NativeFunction(ptr(0x5d72d0),'void',['pointer','int16'],'thiscall');
   const manager=alloc(256),panel=alloc(128),node=alloc(16),control=alloc(128),controlVt=alloc(256),active=alloc(512),video=alloc(256),videoVt=alloc(256);
   node.add(8).writePointer(panel);control.writePointer(controlVt);active.add(4).writePointer(video);video.writePointer(videoVt);
   chitin.add(0x3c4).writePointer(active);chitin.add(0x1c88).writePointer(actor);
   vtable.add(0x2c).writePointer(cb('void',['pointer','pointer'],()=>e.actions.push(102)));
   vtable.add(0xcc).writePointer(cb('pointer',['pointer'],()=>manager));
   vtable.add(0xe0).writePointer(cb('void',['pointer','uint'],(p,n)=>e.actions.push(200+n)));
   vtable.add(0xf0).writePointer(cb('void',['pointer'],()=>e.actions.push(105)));
   controlVt.add(0x68).writePointer(cb('void',['pointer','int','int'],()=>e.actions.push(400+e.control)));
   videoVt.add(0xec).writePointer(cb('void',['pointer'],()=>e.actions.push(104)));
   replace(0x5e3b80,'int',['pointer'],'thiscall',()=>{e.actions.push(100);return 1;});
   replace(0x5e2210,'void',['pointer'],'thiscall',()=>e.actions.push(101));
   replace(0x4d4510,'int',['pointer','int16'],'thiscall',(p,k)=>{e.managed.push(k);return c.handled;});
   replace(0x4d46d0,'void',['pointer'],'thiscall',()=>e.actions.push(103));
   replace(0x4d4000,'pointer',['pointer','uint'],'thiscall',(p,n)=>{if(n!==18)throw Error('wrong keyboard panel');return panel;});
   replace(0x4d2ca0,'pointer',['pointer','uint'],'thiscall',(p,n)=>{e.control=n;return control;});
   replace(0x4282c0,'void',['pointer','uint'],'thiscall',(p,n)=>e.actions.push(300+n));
   Interceptor.flush();
   for(c of __KEYS__){e={input:c,actions:[],managed:[]};screen.writeByteArray(new Uint8Array(0x2000));screen.writePointer(vtable);
     screen.add(0x5d0).writeU8(c.key);screen.add(0x7b4).writeU8(c.ctrl);screen.add(0x632).writePointer(c.popup<0?ptr(0):node);panel.add(0x20).writeS32(c.popup);
     game.add(0x4514).writeByteArray(new Uint8Array(372));game.add(0x4688).writeByteArray(new Uint8Array(372));
     if(c.binding>=0){game.add(0x4514+c.binding).writeU8(c.key);game.add(0x4688+c.binding).writeU8(1);}
     keyboard(screen,c.count);keys.push(e);
   }
 }catch(ex){error=String(ex);}
 finally{for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,b]of globals)p.writeByteArray(b);}
 return{events,drops,keys,error,hooksRestored:patches.every(([p,b])=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===b),globalsRestored:globals.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(b.byteLength))))===JSON.stringify(Array.from(new Uint8Array(b))))};
};
"""


def header(events,keys):
    lines=['// Canonical IWD2.exe LevelUp orchestration; see levelup_reference.py for doubled boundaries.',
           '#ifndef LEVELUP_GOLDEN_H_', '#define LEVELUP_GOLDEN_H_',
           'struct LevelUpGolden { INT cls; DWORD spec; INT flags,points,oldCleric,seed,oldMax,newMax,high,rage,caster,domainFlag,alignment; BYTE afterPoints,afterFlags,electric; DWORD animation,spellHash; UINT shape[11]; INT domain,rageButton[4]; };',
           'static const LevelUpGolden LEVELUP_CASES[] = {']
    for e in events:
        c=e['input'];v=[str(c[k]) for k in ['cls','spec','flags','points','oldCleric','seed','oldMax','newMax','high','rage','caster','domainFlag','alignment']]
        v += [str(e['points']),str(e['flags']),str(e['electric']),str(e['animationWord'])+'u']
        h=2166136261
        for row in e['spells']:
            for n in row:h=((h^n)*16777619)&0xffffffff
        v += [str(h)+'u','{'+','.join(str(n)+'u' for n in e['shape'])+'}',str(e['domain']),'{'+','.join(map(str,e['rageButton']))+'}']
        lines.append('    {'+','.join(v)+'},')
    lines += ['};','struct CharacterKeyGolden { INT key,popup,handled,ctrl,binding,count,action; };',
              'static const CharacterKeyGolden CHARACTER_KEY_CASES[] = {']
    for e in keys:
        c=e['input'];values=[str(c[k]) for k in ['key','popup','handled','ctrl','binding','count']]
        if c['popup']!=-1 or c['handled'] or (e['actions'] and e['actions'][0] not in [102,103,105,200,201,202,203,204,205]):
            continue
        values.append(str(e['actions'][0] if e['actions'] else -1));lines.append('    {'+','.join(values)+'},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(cases())).replace('__DROPS__',json.dumps(drop_cases())).replace('__KEYS__',json.dumps(key_cases())),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script];hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,90);events=result['events'];drops=result.get('drops',[]);keys=result.get('keys',[])
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalsRestored') and len(events)==len(cases()) and len(drops)==len(drop_cases())
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:good=good and e['cleared'] and e['dirty']==1 and e['pre']==1 and e['special']==1 and e['divine']==[3,4,7,8] and e['drop']==e['process']==1
    for e in drops:good=good and e['deny']==e['release']
    good=good and len(keys)==len(key_cases()) and all(len(e['actions'])<=1 for e in keys)
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),expected=len(cases()),drops=len(drops),keys=len(keys),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events,keys),encoding='utf-8')
    return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
