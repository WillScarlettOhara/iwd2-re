"""Frozen native spell panel, button binding/click and specialist scan.

Target bodies, catalogue access and click list Add/Remove execute unchanged.
UI/locks/resource/CString services, usability and known-spell queries are explicit
doubles. Typed replay uses loaded resources, real GUIREC and real spell lists.
Race text and rendering have separate typed UI checks, without a native raster claim.
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
    rows=[dict(kind=0,cls=c,spec=s,level=l,count=n,profile=p,known=k)
          for c in [2,10,11] for s in [0,0x4000] for l in [1,9]
          for n in [0,1,4,49] for p in [0,1] for k in [0,1]]
    rows += [dict(kind=1,ref=s) for s in ['ZRCHIT','ZRCHIT','SPIN260','','','ZRCHIT']]
    rows += [dict(kind=2,spec=s,level=l,count=n) for s in [0,0x40,0x80,0x4000]
             for l in [1,9] for n in [0,1,3,255]]
    rows += [dict(kind=3,cls=c,spec=s,extra=e,selected=b) for c in [2,10,11]
             for s in [0,0x4000] for e in [-1,0,1,3] for b in [0,1]]
    return rows


JS=r"""
rpc.exports.run=function(){
 const roots=[],callbacks=[],patches=[],globals=[],events=[];
 function alloc(n){const p=Memory.alloc(n);roots.push(p);return p;}
 function save(a){const p=ptr(a);globals.push([p,p.readU32()]);return p;}
 function replace(a,ret,args,abi,fn){const p=ptr(a),f=new NativeCallback(fn,ret,args,abi);callbacks.push(f);patches.push([p,Array.from(new Uint8Array(p.readByteArray(16)))]);Interceptor.replace(p,f);}
 function ref(p,s){const b=Array.from(s.toUpperCase().slice(0,8),x=>x.charCodeAt(0));while(b.length<8)b.push(0);p.writeByteArray(b);}
 function res(p){return Array.from(new Uint8Array(p.readByteArray(8)),x=>String.fromCharCode(x)).join('').split('\0')[0];}
 function str(s){const p=Memory.allocUtf8String(s);roots.push(p);return p;}
 function string(s){const p=alloc(s.length+13);p.writeS32(1);p.add(4).writeS32(s.length);p.add(8).writeS32(s.length);p.add(12).writeUtf8String(s);return p.add(12);}
 function cs(s){const p=alloc(4);p.writePointer(string(s));return p;}
 function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
 const chitin=alloc(0x6000),game=alloc(0x6000),screen=alloc(0x2000),actor=alloc(0x9e00),panel=alloc(0x100),svt=alloc(0x100),bvt=alloc(0x100),master=alloc(24),entries=alloc(255*8),known=alloc(32),knownData=alloc(64),cells=alloc(9*10*4),token=alloc(4),creator=alloc(0x2000);
 const buttons=Array.from({length:30},()=>alloc(0x700)),resources=[alloc(128),alloc(128)];
 const names=['ZRCHIT','SPIN260',''];names.forEach((s,i)=>ref(master.add(i*8),s));
 function id(p){return names.indexOf(res(p));}
 function spellId(p){return id(p.add(8));}
 function vector(p,selected){const data=alloc(256);if(selected){data.writeU32(0);data.add(4).writeU32(3);}p.writeU32(0);p.add(4).writePointer(data);p.add(8).writePointer(data.add(selected?16:0));p.add(12).writePointer(data.add(256));p.add(16).writeU32(0);p.add(20).writeU32(selected?3:0);p.add(24).writeU32(selected?3:0);}
 function lists(p){const begin=p.add(4).readPointer(),end=p.add(8).readPointer(),n=end.sub(begin).toInt32()/16;if(n<0||n>4)throw Error('vector length');return{entries:Array.from({length:n},(_,i)=>Array.from({length:4},(_,j)=>begin.add(i*16+4*j).readU32())),totals:[16,20,24].map(o=>p.add(o).readU32())};}
 function clsIndex(c){return c===2?0:c===10?5:6;}
 let c=null,e=null,error=null,mode='panel';
 const reset=new NativeFunction(ptr(0x5d9970),'void',['pointer','pointer'],'thiscall');
 const bind=new NativeFunction(ptr(0x5ebdd0),'void',['pointer','uint','uint'],'thiscall');
 const specialist=new NativeFunction(ptr(0x618d10),'int',['pointer','uint8','pointer'],'thiscall');
 const click=new NativeFunction(ptr(0x5ebfa0),'void',['pointer','int','int'],'thiscall');
 try{
  save(0x8cf6dc).writePointer(chitin);save(0x8cf6d8).writePointer(chitin);save(0x8f333c).writePointer(str('number'));
  chitin.add(0x1c54).writePointer(game);chitin.add(0x1c60).writePointer(screen);chitin.add(0x1c64).writePointer(creator);
  game.add(0x4bf8).writePointer(master);game.add(0x4bfc).writeU32(3);screen.writePointer(svt);panel.add(0x20).writeU32(8);
  buttons.forEach(p=>{p.writePointer(bvt);p.add(6).writePointer(panel);});
  svt.add(0xd0).writePointer(cb('int16',['pointer'],()=>0));
  bvt.add(0x44).writePointer(cb('void',['pointer','int','int','int'],(p,a,b,d)=>{p.add(0x3e).writeS32(a);if(b!==-1||d!==-1)throw Error('tooltip args');}));
  replace(0x780c00,'void',['int','pointer','pointer','int'],'mscdecl',(n,f,s)=>{throw Error('assert '+n+' '+s.readUtf8String());});
  replace(0x78a960,'pointer',['pointer'],'thiscall',p=>{ref(p,'');return p;});
  for(const a of [0x78ac30,0x78ac50])replace(a,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,r)=>{p.writeByteArray(r.readByteArray(8));out.writeByteArray(p.readByteArray(8));return out;});
  replace(0x78ad40,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,s.readUtf8String());out.writeByteArray(p.readByteArray(8));return out;});
  replace(0x78acc0,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,s)=>{ref(p,s.readPointer().readUtf8String());out.writeByteArray(p.readByteArray(8));return out;});
  replace(0x78a9f0,'void',['pointer','pointer'],'thiscall',(p,out)=>out.writePointer(string(res(p))));
  replace(0x7fd114,'void',['pointer','int','uint8'],'thiscall',(p,n,b)=>{const data=p.readPointer();if(n<0||n>=data.sub(8).readS32())throw Error('SetAt range');data.add(n).writeU8(b);});
  replace(0x7fcc1a,'void',['pointer'],'thiscall',()=>{});
  replace(0x7faee8,'void',['pointer','pointer','int'],'mscdecl',(p,fmt,n)=>p.writePointer(string(String(n))));
  replace(0x7fc1a5,'pointer',['pointer','pointer'],'thiscall',(p,key)=>{if(key.readUtf8String()!=='number')throw Error('token');return token;});
  replace(0x7fcd57,'pointer',['pointer','pointer'],'thiscall',(p,s)=>{e.token=s.readUtf8String();return p;});
  replace(0x427b60,'pointer',['pointer','uint'],'mscdecl',(out,n)=>{out.writePointer(string('tlk'+n));return out;});
  replace(0x427fe0,'void',['pointer','pointer','uint','pointer','pointer','pointer','int'],'mscdecl',(p,pan,n,fmt,a,b,l)=>{e.label=[n,fmt.readUtf8String(),a.readUtf8String(),b.readUtf8String(),l];});
  replace(0x4d4000,'pointer',['pointer','uint'],'thiscall',(p,n)=>{if(n!==8)throw Error('panel');return panel;});
  replace(0x4d2ca0,'pointer',['pointer','uint'],'thiscall',(p,n)=>{if(n>=30)throw Error('button index');return buttons[n];});
  replace(0x4d57c0,'void',['pointer','int'],'thiscall',(p,b)=>{p.add(0x646).writeS32(b);p.add(0x1e).writeU8(b?1:0);});
  replace(0x4d5ae0,'void',['pointer','int'],'thiscall',(p,b)=>p.add(0x66a).writeS32(b));
  replace(0x5e6840,'void',['pointer','uint','uint','uint'],'thiscall',(p,pan,n,t)=>{e.help=[pan,n,t];});
  replace(0x5e0b20,'void',['pointer','uint','pointer'],'thiscall',()=>{e.updates++;});
  replace(0x4699f0,'void',['pointer','pointer','int','int'],'thiscall',(p,r,a,b)=>{const j=id(r);if(a!==1||b!==1)throw Error('resource flags');p.writeS32(j>=0&&j<2?1:0);p.add(4).writePointer(j>=0&&j<2?resources[j]:ptr(0));p.add(8).writeByteArray(r.readByteArray(8));});
  replace(0x54a380,'int',['pointer'],'thiscall',p=>+(spellId(p)>=0&&spellId(p)<2));
  replace(0x54a3c0,'int',['pointer'],'thiscall',()=>1);
  replace(0x54a430,'int',['pointer','pointer'],'thiscall',(p,out)=>{ref(out,spellId(p)===0?'GUIBTNA':'GUIBTNB');return 0;});
  replace(0x54acd0,'int',['pointer'],'thiscall',p=>p.add(4).readPointer().isNull()?0:10453+spellId(p));
  replace(0x54ad30,'int',['pointer'],'thiscall',()=>7192);
  replace(0x54b000,'uint8',['pointer'],'thiscall',p=>spellId(p)+1);
  replace(0x54aa40,'int',['pointer','pointer'],'thiscall',p=>mode!=='panel'||c.cls!==11||!c.profile||spellId(p)===0?1:0);
  replace(0x77e370,'int',['pointer'],'thiscall',()=>1);replace(0x787ce0,'void',['pointer','pointer'],'thiscall',()=>{});
  replace(0x724790,'pointer',['pointer','pointer','pointer'],'thiscall',()=>known);
  replace(0x725330,'uint8',['pointer','pointer','pointer','uint','uint8'],'thiscall',(p,r)=>+(c.known&&res(r)==='ZRCHIT'));
  replace(0x599c70,'uint8',['pointer','int','uint8','pointer','uint'],'thiscall',(p,n,t,out)=>{out.writePointer(actor);return 0;});
  replace(0x59a010,'uint8',['pointer','int','uint8','uint'],'thiscall',()=>{e.releases++;return 0;});
  replace(0x5431e0,'int',['pointer','pointer','pointer','pointer','uint','uint','pointer'],'thiscall',(p,cl,a,d,s,l,out)=>{out.writeS32(2);return 3;});
  const setPatch=ptr(0x5ebdd0);replace(0x5ebdd0,'void',['pointer','uint','uint'],'thiscall',(p,a,b)=>{p.add(0x676).writeU32(a);p.add(0x67a).writeU32(b);});
  Interceptor.flush();
  let previousKind=-1;
  for(let i=0;i<__CASES__.length;i++){
   c=__CASES__[i];e={updates:0,releases:0};mode=c.kind===0?'panel':c.kind===1?'bind':c.kind===2?'helper':'click';
   if(c.kind!==previousKind&&c.kind===1){Interceptor.revert(setPatch);Interceptor.flush();ref(buttons[0].add(0x676),'');ref(buttons[0].add(0x66e),'');buttons[0].add(0x3e).writeS32(-999);}previousKind=c.kind;
   if(c.kind===1){const r=alloc(8);ref(r,c.ref);bind(buttons[0],r.readU32(),r.add(4).readU32());events.push({op:'spell-selection-result',case:i,kind:1,spell:res(buttons[0].add(0x676)),icon:res(buttons[0].add(0x66e)),tooltip:buttons[0].add(0x3e).readS32()});continue;}
   actor.writeByteArray(new Uint8Array(0x9e00));actor.add(0x80c).writeU32((c.spec||0)>>>0);
   for(let j=0;j<255;j++)entries.add(j*8).writeU32(c.kind===2&&c.count===255?(j===254?0:2):j%3);
   for(let cl=0;cl<7;cl++)for(let l=0;l<9;l++){const p=game.add(0x4c18+cl*0x4c+l*8);p.writePointer(entries);p.add(4).writeU32(c.kind===2?(cl===6&&l===c.level-1?c.count:0):c.kind===3?1:c.count);}
   if(c.kind===2){events.push({op:'spell-selection-result',case:i,kind:2,result:specialist(creator,c.level,actor)});continue;}
   screen.add(0x106).writeU8(c.cls);screen.add(0x1776).writeU8(c.level||1);screen.add(0x846).writeU32(c.spec);actor.add(0x967+c.cls-1).writeU8(5);
   if(c.kind===0){
    const data=string('5');for(let j=0;j<90;j++)cells.add(4*j).writePointer(data);
    for(const off of [0x550,0x574]){game.add(off+0x18).writePointer(cells);game.add(off+0x1c).writePointer(data);game.add(off+0x20).writeS16(9);game.add(off+0x22).writeS16(10);}
    known.add(4).writePointer(knownData);known.add(8).writePointer(knownData.add(c.known*16));
    buttons.forEach(p=>{ref(p.add(0x676),'OLD');p.add(0x66a).writeS32(1);});
    reset(screen,actor);const mask=buttons.reduce((v,p,j)=>v|(p.add(0x646).readS32()?1<<j:0),0)>>>0;
    if(buttons.some(p=>p.add(0x66a).readS32()!==0))throw Error('selection reset');
    if(JSON.stringify(e.help)!==JSON.stringify([8,30,c.cls===11?17250:119]))throw Error('help');
    events.push({op:'spell-selection-result',case:i,kind:0,budget:screen.add(0x7b6).readS32(),enabled:mask,spells:buttons.map(p=>res(p.add(0x676))),token:e.token});
   }else{
    screen.add(0x7b6).writeS32(c.extra);buttons[0].add(0x66a).writeS32(c.selected);ref(buttons[0].add(0x676),'ZRCHIT');
    const group=actor.add(0x4284+clsIndex(c.cls)*0x100),list=group;
    vector(list,c.selected);group.add(0xfc).writeU32(c.selected?1:0);game.add(0x3846).writeS16(1);game.add(0x382e).writeS32(123);
    click(buttons[0],0,0);if(e.releases!==1||e.updates!==1)throw Error('click cleanup');
    events.push({op:'spell-selection-result',case:i,kind:3,budget:screen.add(0x7b6).readS32(),selected:buttons[0].add(0x66a).readS32(),...lists(list),highest:group.add(0xfc).readU32()});
   }
  }
 }catch(ex){error=mode+': '+String(ex);}
 finally{for(const[p]of patches)Interceptor.revert(p);Interceptor.flush();for(const[p,v]of globals)p.writeU32(v);}
 return{events,error,hooksRestored:patches.every(([p,b])=>JSON.stringify(Array.from(new Uint8Array(p.readByteArray(16))))===JSON.stringify(b)),globalsRestored:globals.every(([p,v])=>p.readU32()===v)};
};
"""


def header(inputs,events):
    lines=['// Generated from canonical IWD2.exe by spell_selection_reference.py.','#ifndef SPELL_SELECTION_GOLDEN_H_','#define SPELL_SELECTION_GOLDEN_H_',
           'struct SpellSelectionGolden { INT kind,cls,spec,level,count,profile,known,extra,selected; INT budget,result; DWORD enabled; const char* spells[30]; const char *spell,*icon; INT tooltip; UINT entryCount,entries[4][4],totals[3],highest; };',
           'static const SpellSelectionGolden SPELL_SELECTION_CASES[] = {']
    for c,e in zip(inputs,events):
        vals=[c.get(k,0) for k in ['kind','cls','spec','level','count','profile','known','extra','selected']]+[e.get('budget',0),e.get('result',0)]
        quoted=lambda s:json.dumps(s)
        entries='{'+','.join('{'+','.join(map(str,r))+'}' for r in e.get('entries',[]))+'}'
        lines.append('    {'+','.join(map(str,vals))+','+str(e.get('enabled',0))+'u,{'+','.join(map(quoted,e.get('spells',[])))+'},'+quoted(e.get('spell',c.get('ref','')))+','+quoted(e.get('icon',''))+','+str(e.get('tooltip',0))+','+str(len(e.get('entries',[])))+','+entries+',{'+','.join(map(str,e.get('totals',[0,0,0])))+'},'+str(e.get('highest',0))+'},')
    return '\n'.join(lines+['};','#endif',''])


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--out',required=True,type=Path);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(exist_ok=False);inputs=cases();script=args.out/'original.js';script.write_text(JS.replace('__CASES__',json.dumps(inputs)),encoding='utf-8');(args.out/'inputs.json').write_text(json.dumps(inputs),encoding='utf-8')
    paths=[exe,Path(__file__),Path(sys.modules[capture.__module__].__file__),script,args.out/'inputs.json'];hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,90);events=result['events']
    good=not result['error'] and not result['messages'] and not result['cleanup_errors'] and result['hooksRestored'] and result['globalsRestored'] and len(events)==len(inputs)
    good=good and all(e['case']==i and e['kind']==c['kind'] for i,(e,c) in enumerate(zip(events,inputs))) and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8');report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),error=result['error'],fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');(args.out/'original.jsonl').write_text(''.join(json.dumps(e)+'\n' for e in events),encoding='utf-8')
    if good and args.header:args.header.write_text(header(inputs,events),encoding='utf-8')
    print(json.dumps(report,indent=2));return 0 if good else 2


if __name__=='__main__':raise SystemExit(main())
