"""Native SPBRNHND constructor/factory, recursive cells, geometry and lifetime.

The canonical process remains suspended. Base/resources, object arrays, area
selection, sound and effect execution are isolated. Native dispatch, palette,
recursive construction/placement, Fire arithmetic/routing and AI execute.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
import acid_blob_reference as acid
from projectile_aim_reference import MD5,capture
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from runtime_paths import REPO,game_directory

PREFIX=acid.JS[:acid.JS.index('    for(const type of')]
PREFIX=PREFIX.replace('game=Memory.alloc(0x4000)','game=Memory.alloc(0x6000)')
PREFIX=PREFIX.replace('(p,v)=>ref(p,null,v)',"(p,v)=>{current.animation=v.readCString();return ref(p,null,v);}")
PREFIX=PREFIX.replace('p.add(0x16a).writeU8(0);',
    'p.add(0x16a).writeU8(0);p.add(0xe6).writeU32(0x20000);cell.add(0xac).writeU32(a);cell.add(0xb0).writeU32(b);cell.writePointer(cellTable);cellOwners.set(cell.toString(),p);')
PREFIX=PREFIX.replace('let current=null,error=null;',r"""
let current=null,error=null;
const cellOwners=new Map(),objectOwners=new Map(),cellTable=Memory.alloc(4),typeTable=Memory.alloc(8);
const advance=new NativeCallback(cell=>{const p=cellOwners.get(cell.toString());current.order.push(['frame',p.add(0x2a0).readS16()]);return 1;},'int',['pointer'],'thiscall');
const objectType=new NativeCallback(p=>p.add(0x60).readU8(),'uchar',['pointer'],'thiscall');
callbacks.push(advance,objectType);cellTable.writePointer(advance);typeTable.add(4).writePointer(objectType);
""")
JS=PREFIX+r"""
    function point(p) {return [p.readS32(),p.add(4).readS32()];}
    function seed(kind) {current={kind,pixels:[],demands:0,releases:0,order:[],children:[],messages:[],releasesIds:[],nextId:100,deliveries:0};}
    function ctorObject() {const p=Memory.alloc(0x2a2);ctor(p);return p;}
    replace(0x599a50,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(self,id,thread,out)=>{
      if(current.missing && id===0x1234)return 2;
      const p=objectOwners.get(id);if(!p)return 2;out.writePointer(p);return 0;
    });
    replace(0x599e70,'uchar',['pointer','int','uchar','uint'],'thiscall',(self,id)=>{current.releasesIds.push(id);return 0;});
    replace(0x52a640,'void',['int','pointer','uchar'],'mscdecl',(id,out)=>{out.writeS32(current.origin[0]);out.add(4).writeS32(current.origin[1]);});
    replace(0x59a0f0,'uchar',['pointer','pointer','pointer','uint'],'thiscall',(self,out,p)=>{
      if(current.failAdd)return 2;
      const id=++current.nextId;out.writeS32(id);objectOwners.set(id,p);return 0;
    });
    replace(0x4c7be0,'void',['pointer','pointer','pointer','int','uchar'],'thiscall',(p,area,pos,z,list)=>{
      p.add(0xea).writePointer(area);p.add(6).writeS32(pos.readS32());p.add(10).writeS32(pos.add(4).readS32());p.add(14).writeS32(z);
      current.children.push({vtable:p.readPointer().toString(),point:point(pos),z,list,life:p.add(0x2a0).readS16(),flags:p.add(0xe6).readU32(),facing:p.add(0x1dc).readS16(),
        color:p.add(0x192).readPointer().add(8+0x1c).readU8(),shared:p.add(0x192).readPointer().add(16).readPointer().equals(current.parent.add(0x19a+8).readPointer())});
    });
    replace(0x7aa910,'int',['pointer'],'thiscall',()=>1);
    replace(0x4ebf40,'void',['pointer','pointer','int','int'],'thiscall',()=>{});
    replace(0x7aa690,'int',['pointer','int'],'thiscall',()=>1);
    replace(0x7aa4b0,'int',['pointer','int','uint'],'thiscall',()=>1);
    replace(0x7a9db0,'int',['pointer','int','int','int','int'],'thiscall',()=>1);
    replace(0x7fb298,'void',['pointer'],'thiscall',()=>{});
    replace(0x46d7e0,'void',['pointer','pointer','pointer','int16','pointer','pointer','pointer','uchar'],'thiscall',(self,bounds,poly,n,type,terrain,list,back)=>{
      current.bounds=[0,4,8,12].map(o=>bounds.add(o).readS32());current.polygon=[0,8,16].map(o=>point(poly.add(o)));current.back=back;
      const ids=current.payload?[0x2345,0x2346,0x3456,0x7777]:[],nodes=Memory.alloc(Math.max(1,ids.length)*12);
      memory.push(nodes);
      list.add(4).writePointer(ids.length?nodes:ptr(0));
      for(let i=0;i<ids.length;i++){const p=nodes.add(i*12);p.writePointer(i+1<ids.length?p.add(12):ptr(0));p.add(8).writeS32(ids[i]);}
    });
    replace(0x4f7500,'int16',['pointer','pointer','uchar'],'thiscall',(self,p)=>{
      current.messages.push({target:p.add(4).readS32(),source:p.add(8).readS32(),copy:p.add(12).readPointer().toString()});return 1;
    });
    replace(0x52a1a0,'void',['pointer'],'thiscall',p=>{++current.deliveries;p.add(0x82).writePointer(ptr(0));});
    replace(0x4c7f60,'void',['pointer'],'thiscall',()=>{current.order.push(['remove']);});
    replace(0x59a530,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(self,id)=>{current.order.push(['delete',id]);return current.deleteFail?2:0;});
    replace(0x532dc0,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push(['destroy']);return p;});
    replace(0x533910,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push(['destroy']);return p;});
    Interceptor.flush();
    const ctor=new NativeFunction(ptr(0x532ca0),'pointer',['pointer'],'thiscall');
    const spawn=new NativeFunction(ptr(0x533600),'void',['pointer','pointer','int','uint','int'],'thiscall');
    const fire=new NativeFunction(ptr(0x532f00),'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');
    const tick=new NativeFunction(ptr(0x533a50),'void',['pointer'],'thiscall');
    const area=Memory.alloc(0x800),origin=Memory.alloc(8);
    for(const [id,type] of [[0x1234,0],[0x2345,1],[0x2346,1],[0x3456,0]]) {
      const p=Memory.alloc(0x100);p.writePointer(typeTable);p.add(0x60).writeU8(type);objectOwners.set(id,p);
    }
    const copy=new NativeCallback(()=>{const p=Memory.alloc(4);memory.push(p);return p;},'pointer',['pointer'],'thiscall'),effectTable=Memory.alloc(8);
    callbacks.push(copy);effectTable.add(4).writePointer(copy);
    function payload(p){const e=Memory.alloc(4),node=Memory.alloc(12);memory.push(e,node);e.writePointer(effectTable);node.add(8).writePointer(e);p.add(0x82).writePointer(node);}
    seed('ctor');let p=ctorObject();Object.assign(current,inspect(p),{life:p.add(0x2a0).readS16(),flags:p.add(0xe6).readU32(),span:[p.add(0x174).readS32(),p.add(0x178).readS32()]});events.push(current);
    for(const delay of [0,255]) {seed('factory');current.delay=delay;p=decode(0x1a,ptr(0),delay);Object.assign(current,inspect(p),{life:p.add(0x2a0).readS16(),flags:p.add(0xe6).readU32()});events.push(current);}
    for(let facing=0;facing<16;facing++)for(const oneSide of [0,1])for(const count of [-1,0,1,2,3,0x10002]) {
      seed('spawn');current.input=[count,facing,oneSide];current.parent=p=ctorObject();p.add(0xea).writePointer(area);p.add(14).writeS32(31);p.add(0x16a).writeU8(1);
      origin.writeS32(128);origin.add(4).writeS32(96);spawn(p,origin,count,facing,oneSide);events.push(current);
    }
    const directions=[[0,96],[-36,96],[-72,72],[-96,36],[-96,0],[-96,-36],[-72,-72],[-36,-96],[0,-96],[36,-96],[72,-72],[96,-36],[96,0],[96,36],[72,72],[36,96]];
    const cases=directions.map(d=>({origin:[128,96],target:[128+d[0],96+d[1]],height:31}));
    for(const x of [-11,-10,0,10,11]) cases.push({origin:[128,96],target:[128+x,96],height:-7});
    cases.push({origin:[128,96],target:[256,96],height:31,payload:true});
    cases.push({origin:[128,96],target:[256,96],height:31,missing:true});
    cases.push({origin:[128,96],target:[256,96],height:31,failAdd:true});
    for(const c of cases) {
      seed('fire');Object.assign(current,c);current.parent=p=ctorObject();if(c.payload)payload(p);
      fire(p,area,0x1234,0x4567,c.target[0],c.target[1],c.height,13);
      Object.assign(current,{launch:point(p.add(6)),accum:[p.add(0x9c).readS32(),p.add(0xa0).readS32()],facing:p.add(0x1dc).readS16(),retained:!p.add(0x82).readPointer().isNull()});events.push(current);
    }
    for(const life of [-32768,-1,0,1,2,55,32767])for(const stop of [0,1,2])for(const deleteFail of [false,true]) {
      seed('tick');current.input=[life,stop,deleteFail];p=ctorObject();p.add(0x5c).writeS32(44);p.add(0x2a0).writeS16(life);
      game.add(0x4b40).writeS32(stop);game.add(0x4b44).writeS32(stop===1?43:44);current.deleteFail=deleteFail;
      tick(p);current.life=p.add(0x2a0).readS16();events.push(current);
    }
  } catch(e) {error=String(e);}
  finally {global.writePointer(old);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:global.readPointer().equals(old),hooksRestored:addresses.every((p,i)=>
    Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""

def header(events):
    def ints(values):return ','.join(str(v) for v in values)
    def points(values,size):return '{'+','.join('{'+ints(v)+'}' for v in values+[[0,0]]*(size-len(values)))+'}'
    lines=['// Canonical SPBRNHND captures; array/resource/selection callbacks isolated.',
        '// MD5: '+MD5,'#ifndef SPRAY_GOLDEN_H_','#define SPRAY_GOLDEN_H_',
        'struct SprayFactoryGolden { BYTE delay,color,cached; SHORT life; DWORD flags; };',
        'static const SprayFactoryGolden SPRAY_FACTORY[] = {']
    for e in events:
        if e['kind']=='factory':lines.append(f"    {{{e['delay']},{e['color']},{e['cached']},{e['life']},{e['flags']}u}},")
    lines+=['};','struct SpraySpawnGolden { LONG count; SHORT facing; BOOL oneSide; INT size; LONG points[9][2]; };',
        'static const SpraySpawnGolden SPRAY_SPAWN[] = {']
    for e in events:
        if e['kind']=='spawn':
            coords=[c['point'] for c in e['children']]
            lines.append('    {'+ints(e['input'])+','+str(len(coords))+','+points(coords,9)+'},')
    lines+=['};','struct SprayFireGolden { LONG origin[2],target[2],height,launch[2],accum[2]; SHORT facing; INT size; LONG points[9][2],polygon[3][2],bounds[4]; };',
        'static const SprayFireGolden SPRAY_FIRE[] = {']
    for e in events:
        if e['kind']=='fire' and not any(e.get(k) for k in ('missing','failAdd','payload')):
            coords=[c['point'] for c in e['children'][1:]]
            lines.append('    {{'+ints(e['origin'])+'},{'+ints(e['target'])+'},'+str(e['height'])+',{'+ints(e['launch'])+'},{'+ints(e['accum'])+'},'+str(e['facing'])+','+str(len(coords))+','+points(coords,9)+','+points(e['polygon'],3)+',{'+ints(e['bounds'])+'}},')
    lines+=['};','struct SprayTickGolden { SHORT before; INT stop; BOOL deleteFail; SHORT after; INT frames; BOOL expired,destroyed; };',
        'static const SprayTickGolden SPRAY_TICK[] = {']
    for e in events:
        if e['kind']=='tick':
            life,stop,failed=e['input'];names=[o[0] for o in e['order']]
            lines.append(f"    {{{life},{stop},{int(failed)},{e['life']},{names.count('frame')},{int('remove' in names)},{int('destroy' in names)}}},")
    damage=next(e for e in events if e['kind']=='fire' and e.get('payload'))
    lines+=['};','static const LONG SPRAY_DAMAGE_POINTS[][2] = {']
    for c in damage['children'][1:]:lines.append('    {'+ints(c['point'])+'},')
    return '\n'.join(lines+['};','#endif',''])

def main():
    ap=argparse.ArgumentParser(description=__doc__.splitlines()[0]);ap.add_argument('--out',type=Path,required=True)
    ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(acid.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60)
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored')
    good=good and not any(m.get('type')=='error' for m in result['messages']) and len(result['events'])==261
    for e in result['events']:
        if e['kind'] in ('ctor','factory'):
            good=good and e['vtable']=='0x84e69c' and e['life']==55 and e['flags']==0x20008
            good=good and e['color']==(71 if e['kind']=='ctor' else 67) and e['cached']==71 and e['shared']
        elif e['kind']=='spawn':
            n=((e['input'][0]+32768)%65536)-32768;n=max(1,n)
            good=good and len(e['children'])==(n if e['input'][2] else n*(n+3)//2)
            good=good and all(c['vtable']=='0x84e724' and c['life']==55 and c['flags']==0x20000 and c['z']==31 and c['list']==0 and c['shared'] for c in e['children'])
        elif e['kind']=='fire':
            good=good and e['deliveries']==(0 if e.get('missing') or e.get('failAdd') else 1)
            if e.get('payload'):good=good and [(m['source'],m['target']) for m in e['messages']]==[(0x1234,0x2345),(0x1234,0x2346)]
        elif e['kind']=='tick':
            life,stop,failed=e['input'];expected=life if stop==1 else ((life-1+32768)%65536)-32768
            good=good and e['life']==expected and (bool(e['order'])==(stop!=1))
            if stop!=1:good=good and e['order'][0]==['frame',expected]
            good=good and any(o[0]=='destroy' for o in e['order'])==(stop!=1 and expected<=0 and not failed)
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(result['events']),original_md5=MD5,fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(result['events']),encoding='utf-8')
    return 0 if good else 2

if __name__=='__main__':raise SystemExit(main())
