// Loaded after weapon_damage_reference.js, original executable only.
const dpCheck = wdNative('0x75d450', 'int', ['int16','int16']);
const dpSetUsage = wdNative('0x4e8760', 'void', ['int','uint16']);
const dpSelect = wdNative('0x74f830', 'void', ['uint8','uint8','uint8','uint8']);
const dpButtonCtor = wdNative('0x593f80', 'pointer', []);
const dpNew = new NativeFunction(ptr('0x7fc95b'), 'pointer', ['uint'], 'mscdecl');
function dpReadMessages(list, previousTail, id) {
    const capture = {triggers:0,triggerId:0,string1:'',string2:'',triggerActors:1};
    // MFC CPtrList tail +8; node previous +4, message +8 (AddTail 0x7FBE4E).
    let node = list.add(8).readPointer(), inspected = 0;
    while (!node.equals(previousTail)) {
        if (node.isNull() || ++inspected > 1024) throw new Error('Message queue tail boundary lost');
        const message = node.add(8).readPointer();
        // CheckItemDepleted constructs this message inline: vtable 0x847BB8,
        // CAITrigger at +0x0C, CString members at trigger +0x4E/+0x52.
        if (!message.isNull() && message.readPointer().equals(ptr('0x847bb8'))) {
            capture.triggers++;
            capture.triggerId = message.add(0xc).readU16();
            capture.string1 = message.add(0x5a).readPointer().readCString();
            capture.string2 = message.add(0x5e).readPointer().readCString();
            capture.triggerActors &= Number(message.add(4).readS32() === id && message.add(8).readS32() === id);
        }
        node = node.add(4).readPointer();
    }
    return capture;
}

function wdDepletionChecks(source) {
    if (source.add(0x41e).readS32() !== 0 || source.add(0x476).readS16() !== 0
        || source.add(0x24).readU8() !== 2)
        throw new Error('Depletion requires idle PC and empty action queue');
    // Equipment pointers and value-only records. CButtonData owns no strings.
    const fields = [[0x4ad8,51*4], [0x342c,8*0x3c], [0x3d3a,8], [0x4ba4,1],
        [0x4ba6,2], [0x4c68,1], [0x5ed,1], [0x24,1]];
    const saved = fields.map(([offset,size]) => [source.add(offset), source.add(offset).readByteArray(size)]);
    const selected = source.add(0x4ba4).readU8(), ability = source.add(0x4ba6).readU16();
    const fist = source.add(0x4ad8+10*4).readPointer();
    const cases = ['pc-empty','pc-same','pc-different','ally-empty','pc-wrapped'];
    const alternates = ['ZRCARR2','ZRCBLT2','ZRCSTN2'];
    try {
        for (let kind = 0; kind < 3; ++kind) {
            for (let trial = 0; trial < cases.length; ++trial) {
                const [name,launcherName] = wdCases[kind+1], owned = [];
                let exhausted = null, installed = false;
                try {
                    const make = (name, count=1) => { const p=wdItem(name,count); owned.push(p); return p; };
                    const launcher = make(launcherName), melee = make('ZRCMELEE'), weaker = make('ZRCDAGGR');
                    const reserve = make(trial === 2 ? alternates[kind] : name, 7);
                    // The game deletes an exhausted CItem: allocate it with the game's
                    // operator new, never Frida Memory.alloc (a different allocator).
                    exhausted = dpNew(0xf0);
                    if (exhausted.isNull()) throw new Error('CItem allocation failed');
                    const ref = Memory.allocUtf8String(name.padEnd(8,'\0'));
                    wdCtor(exhausted, ref.readU32(), ref.add(4).readU32(), 1,0,0,0,0);
                    dpSetUsage(exhausted, 0, trial === 4 ? 65535 : 0);
                    const hasReserve = [1,2,4].includes(trial);
                    source.add(0x4ad8).writeByteArray(new Uint8Array(51*4));
                    for (let i=0; i<8; ++i) {
                        const button = source.add(0x342c+i*0x3c);
                        dpButtonCtor(button);
                        button.add(0x1c).writeS16(43+i); button.add(0x1e).writeS16(0);
                    }
                    source.add(0x3d3a).writeByteArray(new Uint8Array(8));
                    for (const [slot,item] of [[10,fist],[11,exhausted],[12,hasReserve ? reserve : ptr(0)],
                            [43,launcher],[45,melee],[47,weaker]])
                        source.add(0x4ad8+slot*4).writePointer(item);
                    installed = true;
                    source.add(0x4c68).writeU8(0); source.add(0x4ba4).writeU8(11);
                    source.add(0x4ba6).writeU16(0); source.add(0x24).writeU8(trial === 3 ? 4 : 2);
                    const messages = ptr('0x8cf6dc').readPointer().add(0x4980);
                    const previousTail = messages.add(8).readPointer();
                    const returned = dpCheck(source,11,0);
                    const capture = dpReadMessages(messages, previousTail, source.add(0x5c).readS32());
                    const record = {tag:'weapon-depletion', item:name, case:cases[trial], returned,
                        deleted:Number(source.add(0x4ad8+11*4).readPointer().isNull()),
                        slot:source.add(0x4ba4).readU8(), set:source.add(0x4c68).readU8(),
                        ability:source.add(0x4ba6).readU16(), reserve:hasReserve ? wdUsage(reserve,0) : -1};
                    for (const key of ['triggers','triggerId','string1','string2','triggerActors'])
                        record[key] = capture[key];
                    send(record);
                } finally {
                    if (installed) {
                        // Read the live slot, not the deleted object's storage.
                        const live = source.add(0x4ad8+11*4).readPointer();
                        if (!live.isNull()) wdDelete(live);
                    } else if (exhausted) wdDelete(exhausted);
                    for (const [p,bytes] of saved) p.writeByteArray(bytes);
                    for (const item of owned.reverse()) wdDtor(item);
                }
            }
        }
    } finally {
        for (const [p,bytes] of saved) p.writeByteArray(bytes);
        source.add(0x4ba4).writeU8(255);
        dpSelect(source,selected,ability & 255,0,1);
        for (const [p,bytes] of saved) p.writeByteArray(bytes);
    }
}
