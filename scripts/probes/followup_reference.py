"""Canonical follow-up constructors, factories, Fire, scheduling and child callback.

The original stays suspended. Base/resources, child construction and Fire,
selection, arrays, effects and sound use isolated callbacks. Native parent
dispatch/control flow and CallLiH Fire execute. No actual HP/raster claim.
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
JS=PREFIX+r"""
    const owners=new Map(),area=Memory.alloc(0xb20),foreign=Memory.alloc(0xb20);
    memory.push(area,foreign);area.add(0x86e).writeS16(16);
    function alloc(n){const p=Memory.alloc(n);memory.push(p);return p;}
    function seed(kind,input={}){current={kind,input,pixels:[],demands:0,releases:0,order:[],copies:[],fires:[],searches:[],decoded:[]};owners.clear();}
    function point(p){return [p.readS32(),p.add(4).readS32()];}
    const table=alloc(0x88),effectTable=alloc(8),actorTable=alloc(0x28);
    function cb(ret,args,fn){const f=new NativeCallback(fn,ret,args,'thiscall');callbacks.push(f);return f;}
    const notify=new NativeFunction(ptr(0x533de0),'void',['pointer'],'thiscall');
    table.add(0x6c).writePointer(cb('void',['pointer','pointer','int','int','int','int','int','int16'],(p,a,s,t,x,y,z,k)=>{
      const copied=[];let node=p.add(0x82).readPointer();while(!node.isNull()){const e=node.add(8).readPointer();copied.push([e.add(0x98).readU32(),e.add(0x9c).readS32()]);node=node.readPointer();}
      current.fires.push([s,t,x,y,z,k,p.add(0x7a).readS32(),p.add(0x186).readS32(),copied]);
      current.order.push('fire');if(current.input.sync)notify(current.parent);
    }));
    table.writePointer(cb('pointer',['pointer','uint'],p=>{current.order.push('child-destroy');return p;}));
    table.add(0x7c).writePointer(cb('void',['pointer'],()=>current.order.push('parent-callback')));
    table.add(0x10).writePointer(cb('pointer',['pointer'],p=>p.add(0x100)));
    actorTable.add(0x1c).writePointer(cb('pointer',['pointer'],p=>p.add(6)));
    actorTable.add(0x24).writePointer(cb('uchar',['pointer'],p=>p.add(0x1a).readU8()));
    effectTable.add(4).writePointer(cb('pointer',['pointer'],p=>{const e=alloc(0xa0);e.writeByteArray(p.readByteArray(0xa0));current.copies.push(e);return e;}));
    function child(type){const p=alloc(0x29a);p.writePointer(table);p.add(0x6e).writeU16((type-1)&65535);p.add(0x7a).writeS32(-1);current.decoded.push(type);return p;}
    const decodeAddress=ptr(0x51eaf0);addresses.push(decodeAddress);before.push(Array.from(new Uint8Array(decodeAddress.readByteArray(16))).join(','));
    const fakeDecode=new NativeCallback(type=>child(type),'pointer',['uint16','pointer','uchar'],'mscdecl');callbacks.push(fakeDecode);
    const originalDecode=new NativeFunction(Interceptor.replaceFast(decodeAddress,fakeDecode),'pointer',['uint16','pointer','uchar'],'mscdecl');
    replace(0x40ae80,'pointer',['pointer','uint','uint','uint','uint','uint','uint','uint','uint','int','uint','int'],'thiscall',p=>{p.writeByteArray(new Uint8Array(60));p.add(10).writeS32(-1);return p;});
    replace(0x40b6e0,'void',['pointer','pointer'],'thiscall',(p,v)=>p.writeByteArray(v.readByteArray(60)));
    replace(0x78ac30,'pointer',['pointer','pointer','pointer'],'thiscall',(p,out,v)=>{const bytes=v.readByteArray(8);p.writeByteArray(bytes);out.writeByteArray(bytes);return out;});
    replace(0x7fbe4e,'pointer',['pointer','pointer'],'thiscall',(list,e)=>{const node=alloc(12);node.add(8).writePointer(e);const head=list.add(4).readPointer();
      if(head.isNull())list.add(4).writePointer(node);else{let last=head;while(!last.readPointer().isNull())last=last.readPointer();last.writePointer(node);}return node;});
    replace(0x59a0f0,'uchar',['pointer','pointer','pointer','uint'],'thiscall',(self,out,p)=>{current.order.push('add');if(current.input.failAdd)return 2;out.writeS32(44);owners.set(44,p);return 0;});
    replace(0x4c7be0,'void',['pointer','pointer','pointer','int','uchar'],'thiscall',(p,a,pos,z,list)=>{current.order.push('area');p.add(6).writeS32(pos.readS32());p.add(10).writeS32(pos.add(4).readS32());p.add(14).writeS32(z);p.add(0x12).writePointer(a);p.add(0x1a).writeU8(list);});
    replace(0x599a50,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(self,id,thread,out)=>{
      current.order.push('share:'+id);if(current.input.retry && !current.retried){current.retried=true;return 4;}
      if((current.input.missing && id===22)||(current.input.newMissing && id===33))return 2;
      const p=owners.get(id);if(!p)return 2;out.writePointer(p);return 0;});
    replace(0x599e70,'uchar',['pointer','int','uchar','uint'],'thiscall',(self,id)=>{current.order.push('release:'+id);return 0;});
    replace(0x599c70,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',(self,id,thread,out)=>{
      current.order.push('deny:'+id);if(current.input.denyFail)return 2;const p=owners.get(id);if(!p)return 2;out.writePointer(p);return 0;});
    replace(0x59a010,'uchar',['pointer','int','uchar','uint'],'thiscall',(self,id)=>{current.order.push('release-deny:'+id);return 0;});
    replace(0x4c7f60,'void',['pointer'],'thiscall',()=>current.order.push('remove'));
    replace(0x5551b0,'void',['pointer'],'thiscall',()=>current.order.push('remove-self'));
    replace(0x59a530,'uchar',['pointer','int','uchar','pointer','uint'],'thiscall',()=>{current.order.push('delete');return current.input.deleteFail?2:0;});
    for(const address of [0x533df0,0x534770,0x534b10])replace(address,'pointer',['pointer','uint'],'thiscall',p=>{current.order.push('destroy');return p;});
    replace(0x46b160,'int',['pointer','int','pointer','int16','pointer','int','int','int','uchar','int'],'thiscall',(p,id,type,range,terrain,los,see,sleep,mode,dead)=>{
      current.searches.push(['front',id,range,terrain.toString(),los,see,sleep,mode,dead]);return current.input.noNext?-1:33;});
    replace(0x46dae0,'int',['pointer','int','int','pointer','int16','pointer','int','int','int','int'],'thiscall',(p,x,y,type,range,terrain,los,see,mode,all)=>{
      current.searches.push(['point',x,y,range,terrain.toString(),los,see,mode,all]);return current.input.noNext?-1:33;});
    replace(0x7aa910,'int',['pointer'],'thiscall',()=>{current.order.push('sound-stop');return 1;});
    replace(0x4ebf40,'void',['pointer','pointer','int','int'],'thiscall',()=>current.order.push('sound-ref'));
    replace(0x7aa690,'int',['pointer','int'],'thiscall',()=>1);replace(0x7aa4b0,'int',['pointer','int','uint'],'thiscall',()=>1);
    replace(0x7a9db0,'int',['pointer','int','int','int','int'],'thiscall',()=>{current.order.push('sound-play');return 1;});
    replace(0x52a1a0,'void',['pointer'],'thiscall',()=>current.order.push('deliver'));
    Interceptor.flush();
    const ctor=new NativeFunction(ptr(0x534600),'pointer',['pointer','int','uint'],'thiscall');
    const fire=new NativeFunction(ptr(0x533f40),'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');
    const tick=new NativeFunction(ptr(0x534140),'void',['pointer'],'thiscall');
    const childFire=new NativeFunction(ptr(0x535100),'void',['pointer','pointer','int','int','int','int','int','int16'],'thiscall');
    function parent(interval=1){const p=alloc(0x2fa);ctor(p,interval,65535);current.parent=p;p.add(0x5c).writeS32(44);p.add(0xea).writePointer(area);p.add(0x72).writeS32(11);p.add(0x76).writeS32(22);p.add(0x186).writeS32(7);p.add(0x18a).writeByteArray([84,69,83,84,0,0,0,0]);return p;}
    function actor(id,list,a=area){const p=alloc(0x160);p.writePointer(actorTable);p.add(6).writeS32(id===22?160:190);p.add(10).writeS32(id===22?96:110);p.add(0x12).writePointer(a);p.add(0x1a).writeU8(list);owners.set(id,p);return p;}
    function payload(p){for(let i=0;i<2;++i){const e=alloc(0xa0);e.writePointer(effectTable);e.add(0x98).writeU32(123+i);e.add(0x9c).writeS32(900+i);const node=alloc(12);node.add(8).writePointer(e);
      if(i===0)p.add(0x82).writePointer(node);else p.add(0x82).readPointer().writePointer(node);}}
    function snapshot(p){current.state=[p.add(0x2a0).readS16(),p.add(0x2a2).readS16(),p.add(0x2f8).readS16(),p.add(0x2ae).readS32(),p.add(0x72).readS32(),p.add(0x76).readS32(),p.add(0xc8).readS32(),p.add(0xcc).readS32()];
      const next=p.add(0x2aa).readPointer();current.next=[next.add(0x7a).readS32(),next.add(0x186).readS32(),Array.from(new Uint8Array(next.add(0x18a).readByteArray(8)))];
      current.copySlots=current.copies.map(e=>[e.add(0x98).readU32(),e.add(0x9c).readS32()]);delete current.copies;delete current.parent;events.push(current);}
    for(const interval of [-32768,-1,0,1,11,32767,65538]){seed('ctor',{interval});const p=parent(interval);current.defaults=[p.add(0x2a0).readS16(),p.add(0x2a2).readS16(),p.add(0x2a4).readS32(),p.add(0x2a8).readU16(),p.add(0x2b6).readU8(),p.add(0x2f4).readS32()];current.vtable=p.readPointer().toString();snapshot(p);}
    for(let type=0x51;type<=0x5b;++type)for(const ea of [0,199,200,255])for(const delay of [0,255]){
      seed('factory',{type,ea,delay});const caster=alloc(0x160);caster.writePointer(table);caster.add(0x104).writeU8(ea);const p=originalDecode(type,caster,delay);
      current.defaults=[p.add(0x2a0).readS16(),p.add(0x2a2).readS16(),p.add(0x2a4).readS32(),p.add(0x2a8).readU16(),p.add(0x2b6).readU8(),p.add(0x2f4).readS32()];current.stamp=p.add(0x6e).readU16();current.vtable=p.readPointer().toString();snapshot(p);}
    for(const remaining of [-32768,-1,0,1,2,300,32767])for(const interval of [-1,0,1,11])for(const sync of [false,true])for(const failAdd of [false,true]){
      seed('fire',{remaining,interval,sync,failAdd});const p=parent(interval);p.add(0x2a0).writeS16(remaining);p.add(0x2a2).writeS16(interval);payload(p);current.order=[];fire(p,area,11,22,256,96,31,13);snapshot(p);}
    const cases=[];
    for(const remaining of [-1,0,1,300])for(const delay of [-1,0,1,32767])for(const stop of [0,1,2])for(const ready of [false,true])cases.push({remaining,delay,stop,ready,gate:true});
    for(const list of [0,1])for(const keep of [0,1])for(const width of [16,1024,-1])for(const noNext of [false,true])cases.push({list,keep,width,noNext,remaining:2,delay:0,ready:true,sync:true});
    for(const flags of [{missing:true},{foreign:true},{newMissing:true},{retry:true},{noNext:true,callback:true},{remaining:0,callback:true},{remaining:0,callback:true,denyFail:true},{remaining:0,deleteFail:true}])cases.push({remaining:2,delay:0,ready:true,...flags});
    for(const input of cases){seed('tick',input);const p=parent(3);p.add(0x2a0).writeS16(input.remaining);p.add(0x2f8).writeS16(input.delay);p.add(0x2ae).writeS32(input.ready?1:0);p.add(0x2a4).writeS32(input.keep??1);p.add(0x2ee).writeS32(31);
      actor(22,input.list??0,input.foreign?foreign:area);actor(33,0);if(input.callback){const owner=alloc(0x160);owner.writePointer(table);owners.set(99,owner);p.add(0x7a).writeS32(99);}
      area.add(0x86e).writeS16(input.width??16);payload(p);current.order=[];game.add(0x4b40).writeS32(input.stop??0);game.add(0x4b44).writeS32(input.stop===1?43:44);
      if(input.gate && input.remaining>0 && input.delay<=0 && input.ready && input.stop!==1)current.input.noNext=true;
      tick(p);snapshot(p);}
    game.add(0x4b40).writeS32(0);
    for(const input of [{target:22},{target:-1},{target:22,missing:true},{target:22,retry:true},{target:22,failAdd:true},{target:22,callback:true},{target:22,callback:true,denyFail:true}]){
      seed('child-fire',input);const p=child(23);p.writePointer(ptr(0x84e8c0));p.add(0x7a).writeS32(input.callback?99:-1);actor(22,0);
      const owner=alloc(0x160);owner.writePointer(table);owners.set(99,owner);childFire(p,area,11,input.target,256,2147483647,31,13);
      current.point=point(p.add(6));current.id=p.add(0x5c).readS32();events.push(current);}
  } catch(e){error=String(e);}
  finally{global.writePointer(old);for(const p of addresses)Interceptor.revert(p);Interceptor.flush();}
  return {events,error,globalRestored:global.readPointer().equals(old),hooksRestored:addresses.every((p,i)=>Array.from(new Uint8Array(p.readByteArray(16))).join(',')===before[i])};
};
"""

def header(events):
    lines=['// Canonical follow-up capture; selection/resources/children isolated.',
        '#ifndef FOLLOWUP_GOLDEN_H_','#define FOLLOWUP_GOLDEN_H_',
        'struct FollowupFactoryGolden { USHORT type; BYTE ea,delay,filter; SHORT count,interval; };',
        'static const FollowupFactoryGolden FOLLOWUP_FACTORY[] = {']
    for e in events:
        if e['kind']=='factory':
            c=e['input'];lines.append(f"    {{{c['type']},{c['ea']},{c['delay']},{e['defaults'][4]},{e['defaults'][0]},{e['defaults'][1]}}},")
    lines+=['};','struct FollowupFireGolden { SHORT remaining,interval; BOOL sync; SHORT after,delay; };',
        'static const FollowupFireGolden FOLLOWUP_FIRE[] = {']
    for e in events:
        c=e['input']
        if e['kind']=='fire' and not c['failAdd']:
            lines.append(f"    {{{c['remaining']},{c['interval']},{int(c['sync'])},{e['state'][0]},{e['state'][2]}}},")
    lines+=['};','struct FollowupGateGolden { SHORT remaining,delay,stop; BOOL ready; SHORT after; BOOL retired; };',
        'static const FollowupGateGolden FOLLOWUP_GATE[] = {']
    for e in events:
        c=e['input']
        if e['kind']=='tick' and c.get('gate'):
            # Selector callbacks are not an oracle for actual area selection.
            if c['remaining']>0 and c['delay']<=0 and c['ready'] and c['stop']!=1:continue
            lines.append(f"    {{{c['remaining']},{c['delay']},{c['stop']},{int(c['ready'])},{e['state'][2]},{int('destroy' in e['order'])}}},")
    return '\n'.join(lines+['};','#endif',''])

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--out',type=Path,required=True);ap.add_argument('--header',type=Path);args=ap.parse_args();import frida
    exe=REPO/'.bin/IWD2.exe'
    if hashlib.md5(exe.read_bytes()).hexdigest()!=MD5:raise SystemExit('Canonical MD5 mismatch')
    args.out.mkdir(parents=True,exist_ok=False);script=args.out/'original.js';script.write_text(JS,encoding='utf-8')
    paths=(exe,Path(__file__),Path(acid.__file__),script);hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    result=capture(frida,exe,game_directory(),script,60);events=result['events']
    good=not result['error'] and not result['cleanup_errors'] and result.get('hooksRestored') and result.get('globalRestored') and len(events)==342
    good=good and not any(m.get('type')=='error' for m in result['messages'])
    for e in events:
        c=e['input']
        if e['kind'] in ('ctor','factory'):
            count=c['type']-0x50 if e['kind']=='factory' else ((c['interval']+32768)%65536)-32768
            good=good and e['vtable']=='0x84e838' and e['defaults'][:2]==[count,300] and e['defaults'][2:4]==[1,22] and e['defaults'][5]==1
        elif e['kind']=='fire':
            good=good and len(e['copySlots'])==2 and e['copySlots']==[[123,22],[124,22]] and len(e['fires'])==(0 if c['failAdd'] else 1)
        elif e['kind']=='child-fire':
            good=good and e['order'][0]=='deliver' and ('parent-callback' in e['order'])==(c.get('callback',False) and not c.get('denyFail',False))
    good=good and hashes=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    (args.out/'capture.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    report=dict(result='PASS' if good else 'UNPROVEN',cases=len(events),original_md5=MD5,fingerprints=hashes,scope=__doc__)
    (args.out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
    if good and args.header:args.header.write_text(header(events),encoding='utf-8')
    return 0 if good else 2
if __name__=='__main__':raise SystemExit(main())
