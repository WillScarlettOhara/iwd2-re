"""Frozen canonical HP calculations with exact native RNG consumption.

The four target bodies, GetClassLevel, atol and CRT rand run unchanged.
Table lookup and CString formatting/destruction are explicit service doubles;
inputs use private stats/game/TLS memory, without starting or saving a game.
Only original-binary offsets are used. Typed reconstruction replay uses C++.
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


def cells(profile, bank, level):
    if profile == 0:
        return [str([10, 4, 8, 6, 12, 8][bank]), '1' if level <= 10 else '0', '0' if level <= 10 else '3']
    if profile == 1:
        return [str(3+2*bank), '0' if level % 3 == 0 else '2', '-9' if level % 2 else '15']
    if profile == 2:
        return ['-8', '-1' if level % 3 else '0', '-13' if level % 2 else '17']
    return ['  +6tail', 'garbage' if level % 2 else '1ignored', ' -19x']


def cases():
    rows=[]
    def add(kind, args, seed=1, maximum=0, profile=0, old=None, new=None):
        rows.append(dict(kind=kind,args=list(args)+[0]*(7-len(args)),seed=seed,maximum=maximum,
                         profile=profile,old=old or [0]*11,new=new or [0]*11))
    for sides in [1,2,6,12,-6]:
        for rolls in [-1,0,1,3]:
            for level in [0,1,2,10]:
                for minimum in [-2,1,5]:
                    for seed in [1,42]:add(0,[sides,rolls,level,minimum,-9],seed)
    for sides in [2147483647,-2147483648]:
        for rolls in [0,1,2]:
            for level in [1,2]:add(0,[sides,rolls,level,1,2147483647],0xffffffff)
    pairs=[(0,0),(0,1),(1,4),(8,11),(10,12),(254,255),(255,0),(5,2)]
    for profile in range(4):
        for old,new in pairs:
            for maximum in [0,1,2]:
                for divisor in [1,-3]:
                    for override in [0,2]:add(1,[old,new,5,divisor,override,12,3],42,maximum,profile)
    # Full BYTE range without a BYTE induction-variable wrap.
    add(1,[0,255,1,2,0,0,0],0,0,0)
    for profile in [0,1]:
        for cls in range(1,12):
            for before,after in [(0,0),(0,1),(1,3),(5,2),(254,255)]:
                for maximum in [0,2]:
                    old=[0]*11;new=[0]*11;old[cls-1]=before;new[cls-1]=after
                    add(2,[cls,1,1],1,maximum,profile,old,new)
    plans=[([0]*11,[0]*11),([0]*11,[1]*11),([1]*11,[2]*11),([3]*11,[1]*11),
           ([0]*11,[1,2,3,0,4,1,0,1,0,2,1]),([0]*11,[0]*10+[255])]
    for profile in [0,1]:
        for old,new in plans:
            for maximum in [0,2]:
                for con in [0,255]:add(3,[con],42,maximum,profile,old,new)
    return rows


JS=r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],saved=[],events=[],lookups=[];
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a){const p=ptr(a);saved.push([p,p.readU32()]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16)))]);Interceptor.replace(p,f);}
 const kernel=Process.getModuleByName('kernel32.dll');
 const tlsAlloc=new NativeFunction(kernel.getExportByName('TlsAlloc'),'uint',[],'stdcall');
 const tlsSet=new NativeFunction(kernel.getExportByName('TlsSetValue'),'int',['uint','pointer'],'stdcall');
 const tlsFree=new NativeFunction(kernel.getExportByName('TlsFree'),'int',['uint'],'stdcall');
 const index=tlsAlloc(),tls=alloc(0x400),rules=alloc(0x2000),oldStats=alloc(0xe58),newStats=alloc(0xe58),game=alloc(0x6000),chitin=alloc(0x6000),type=alloc(0x60);
 if(index===0xffffffff)throw Error('TlsAlloc failed');
 const numeric=new NativeFunction(ptr(0x542a90),'int',['pointer','int','int','int','int','int'],'thiscall');
 const table=new NativeFunction(ptr(0x542b40),'int',['pointer','pointer','uint8','uint8','int','int','int','int'],'thiscall');
 const perClass=new NativeFunction(ptr(0x542cb0),'int',['pointer','int','pointer','pointer','int','int'],'thiscall');
 const total=new NativeFunction(ptr(0x542fa0),'int',['pointer','pointer','pointer','pointer','uint8'],'thiscall');
 const random=new NativeFunction(ptr(0x7e8160),'int',[],'mscdecl');
 const offsets=[0x310,0x334,0x358,0x37c,0x3a0,0x3c4];
 function string(text){const data=alloc(text.length+13);data.writeS32(1);data.add(4).writeS32(text.length);data.add(8).writeS32(text.length);data.add(12).writeUtf8String(text);const s=alloc(4);s.writePointer(data.add(12));return s;}
 const values=__CELLS__.map(banks=>banks.map(rows=>rows.map(row=>row.map(string))));
 let current=null,error=null,cleared=false,released=false;
 try{
  if(!tlsSet(index,tls))throw Error('TlsSetValue failed');save(0x8c23f0).writeU32(index);
  save(0x8cf6dc).writePointer(chitin);chitin.add(0x1c54).writePointer(game);
  replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(line,file,msg)=>{throw Error('native assertion '+line+' '+msg.readUtf8String());});
  replace(0x7faee8,'void',['pointer','pointer','int'],'mscdecl',(out,fmt,level)=>{if(fmt.readUtf8String()!=='%d')throw Error('format');out.writePointer(string(String(level)).readPointer());});
  replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
  replace(0x403340,'pointer',['pointer','pointer','pointer'],'thiscall',(p,x,y)=>{
   const bank=offsets.findIndex(o=>p.equals(rules.add(o))),column=[0x8dee58,0x8df150,0x8df094].findIndex(a=>x.equals(ptr(a)));
   const level=Number(y.readPointer().readUtf8String());
   if(bank<0||column<0||!Number.isInteger(level)||level<1||level>255)throw Error('table lookup identity');
   lookups.push([events.length,bank,level,column]);return values[current.profile][bank][level-1][column];
  });Interceptor.flush();
  const inputs=__CASES__;
  for(let i=0;i<inputs.length;i++){
   current=inputs[i];const a=current.args;tls.add(0x14).writeU32(current.seed);game.add(0x448a).writeS32(current.maximum);
   oldStats.add(0x47).writeByteArray(current.old);newStats.add(0x47).writeByteArray(current.new);
   let result;
   if(current.kind===0)result=numeric(rules,...a.slice(0,5));
   else if(current.kind===1)result=table(rules,rules.add(offsets[a[6]]),...a.slice(0,6));
   else if(current.kind===2)result=perClass(rules,a[0],oldStats,newStats,a[1],a[2]);
   else result=total(rules,type,oldStats,newStats,a[0]);
   events.push({op:'hitpoint-result',case:i,kind:current.kind,result,nextRandom:random()});
  }
 }catch(ex){error=String(ex);}
 finally{
  for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,v]of saved)p.writeU32(v);
  cleared=!!tlsSet(index,ptr(0));released=!!tlsFree(index);
 }
 return{events,lookups,error,tlsCleared:cleared,tlsReleased:released,
  hooksRestored:patches.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(16))))===JSON.stringify(b)),
  globalsRestored:saved.every(([p,v])=>p.readU32()===v)};
};
"""


def header(inputs, events):
    def integer(n):return '(-2147483647-1)' if n == -2147483648 else str(n)
    def array(row):return '{'+','.join(map(integer,row))+'}'
    lines=['// Generated by hitpoints_reference.py from canonical IWD2.exe.',
           '#ifndef HITPOINTS_GOLDEN_H_','#define HITPOINTS_GOLDEN_H_',
           'struct HitPointGolden { INT kind,args[7]; DWORD seed; INT maximum,profile; BYTE oldLevels[11],newLevels[11]; INT result,nextRandom; };',
           'static const HitPointGolden HITPOINT_CASES[] = {']
    for c,e in zip(inputs,events):
        lines.append('    {'+','.join([str(c['kind']),array(c['args']),str(c['seed'])+'u',str(c['maximum']),str(c['profile']),
                                      array(c['old']),array(c['new']),integer(e['result']),str(e['nextRandom'])])+'},')
    lines+=['};','static const char* const HITPOINT_TABLE_CELLS[4][6][255][3] = {']
    for profile in range(4):
        lines.append('    {')
        for bank in range(6):
            lines.append('        {'+','.join('{'+','.join(map(json.dumps,cells(profile,bank,level)))+'}' for level in range(1,256))+'},')
        lines.append('    },')
    lines+=['};','#endif','']
    return '\n'.join(lines)


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args()
    import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);inputs=cases();script=args.out/'original.js'
    all_cells=[[[cells(p,b,l) for l in range(1,256)] for b in range(6)] for p in range(4)]
    script.write_text(JS.replace('__CASES__',json.dumps(inputs)).replace('__CELLS__',json.dumps(all_cells)),encoding='utf-8')
    (args.out/'inputs.json').write_text(json.dumps(inputs),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script,args.out/'inputs.json']
    hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,90)
    events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and not result['messages'] and all(result.get(k) for k in ['tlsCleared','tlsReleased','hooksRestored','globalsRestored'])
    good=good and len(events)==len(inputs) and all(e['case']==i and e['kind']==c['kind'] for i,(e,c) in enumerate(zip(events,inputs)))
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    (args.out/'original.jsonl').write_text(''.join(json.dumps(e)+'\n' for e in events),encoding='utf-8')
    if good and args.header:args.header.write_text(header(inputs,events),encoding='utf-8')
    print(json.dumps(report,indent=2));return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
