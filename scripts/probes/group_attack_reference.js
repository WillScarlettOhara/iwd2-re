// Original IWD2.exe only. Run synchronously on the world update thread.
// Offsets are binary pack(2) layouts, never applied to the reconstructed build.
const gaNative = (a, ret, args) => new NativeFunction(ptr(a), ret, ['pointer'].concat(args), 'thiscall');
const ga = gaNative('0x74fc50', 'int16', ['pointer']);
const gaCtor = gaNative('0x403a40', 'void', []);
const gaAssign = gaNative('0x405090', 'pointer', ['pointer']);
const gaDtor = gaNative('0x403b40', 'void', []);
const gaTypeSet = gaNative('0x40b6e0', 'void', ['pointer']);
const gaStringCtor = gaNative('0x7fcc88', 'void', ['pointer']);
const gaStringSet = gaNative('0x7fcd07', 'void', ['pointer']);
const gaStringDtor = gaNative('0x7fcc1a', 'void', []);
const gaClear = gaNative('0x44d640', 'void', ['int']);
const gaDeny = gaNative('0x599c70', 'uint8', ['int', 'uint8', 'pointer', 'uint']);
const gaRelease = gaNative('0x59a010', 'uint8', ['int', 'uint8', 'uint']);
let gaLoaded = false, gaDone = false, gaTicks = 0, gaSaveMatched = false;

// Select the exact named save, keeping the driver's visible row at zero.
function gaSelectSave(screen) {
    const count = screen.add(0x1ca).readS32();
    const slots = screen.add(0x1d6).readPointer();
    for (let i = 0; i < count && i < 10000; ++i) {
        const slot = slots.add(i * 4).readPointer();
        if (slot.readPointer().readCString() === '000000037-testSpells') {
            screen.add(0x1c6).writeS32(i);
            gaSaveMatched = true;
            send({tag:'reference-save', name:'000000037-testSpells'});
            return;
        }
    }
    throw new Error('Exact save missing');
}
Interceptor.attach(ptr('0x6869c0'), {onLeave() { gaLoaded = true; }});

function gaType(p) {
    return [p.add(4).readU8(), p.add(5).readU8(), p.add(6).readU8(),
        p.add(7).readU8(), p.add(0x18).readS32(), p.add(0x13).readU8(),
        p.add(0x15).readU8(), p.add(0x14).readU8(), p.add(0xa).readS32()];
}
function gaEq(p, expected) { return JSON.stringify(gaType(p)) === JSON.stringify(expected); }
function gaEmpty(p) { return p.readPointer().readCString() === ''; }

Interceptor.attach(ptr('0x686690'), {onLeave() {
    if (!gaLoaded || gaDone || ++gaTicks < 15) return;
    gaDone = true;
    const game = ptr('0x8cf6dc').readPointer().add(0x1c54).readPointer();
    const objects = game.add(0x372c);
    const ids = [game.add(0x3816).readS32(), game.add(0x381a).readS32()];
    const refs = [Memory.alloc(4), Memory.alloc(4)];
    const held = [];
    const owned = [];
    let source, target, savedAction, savedType, oldInterrupt, oldActivity, oldResponses;
    const check = (name, passed) => send({tag:'combatgroup-case', case:name, passed:!!passed});
    try {
        if (!gaSaveMatched) throw new Error('Named save was not selected');
        for (let i = 0; i < 2; ++i) {
            if (gaDeny(objects, ids[i], 0, refs[i], 0xffffffff) !== 0)
                throw new Error('Party lock failed');
            held.push(ids[i]);
        }
        source = refs[0].readPointer(); target = refs[1].readPointer();
        if (source.add(0x41e).readS32() !== 0 || source.add(0x476).readS16() !== 0)
            throw new Error('Expected idle source and empty action queue');
        const action = source.add(0x476), type = target.add(0x20);
        savedAction = Memory.alloc(0xd6); gaCtor(savedAction); owned.push(savedAction);
        gaAssign(savedAction, action);
        savedType = Memory.alloc(0xd6); gaCtor(savedType); owned.push(savedType);
        gaTypeSet(savedType.add(2), type);
        oldInterrupt = source.add(0x470).readS32();
        oldActivity = target.add(0x50a6).readByteArray(12);
        oldResponses = source.add(0x46a).readByteArray(6);
        const name = Memory.alloc(4);
        gaStringCtor(name, Memory.allocUtf8String('ZRCTARGET'));
        gaStringSet(type, name); gaStringDtor(name);
        type.add(0x13).writeU8(source.add(0x33).readU8() ^ 1);
        type.add(0xa).writeS32(ids[1]);
        const expected = gaType(type); expected[8] = -1;
        action.writeS16(94);
        gaTypeSet(action.add(0x3e), source.add(0x20));
        action.add(0xb6).writeS32(7); action.add(0xba).writeS32(8); action.add(0xbe).writeS32(9);
        const originalTarget = gaType(action.add(0x3e));
        for (const offset of [0x50a6,0x50aa,0x50ae]) target.add(offset).writeS32(1);
        source.add(0x470).writeS32(0);
        check('null-target', ga(source, ptr(0)) === -1);
        target.add(0x50a6).writeS32(0);
        check('inactive-target', ga(source, target) === -1);
        target.add(0x50a6).writeS32(1); target.add(0x50aa).writeS32(0);
        check('inactive-ai', ga(source, target) === -1);
        target.add(0x50aa).writeS32(1); target.add(0x50ae).writeS32(0);
        check('imprisoned-target', ga(source, target) === -1);
        target.add(0x50ae).writeS32(1); source.add(0x470).writeS32(1);
        check('interrupted', ga(source, target) === 1);
        check('null-before-interrupt', ga(source, ptr(0)) === -1);
        check('guards-preserve-order', source.add(0x41e).readS32() === 0 && gaEq(action.add(0x3e), originalTarget));
        source.add(0x470).writeS32(0);
        check('active-target', ga(source, target) === -1);
        check('current-target', gaEq(action.add(0x3e), expected) && gaEmpty(action.add(0x3e)));
        const count = source.add(0x41e).readS32();
        check('two-actions', count === 2);
        if (count !== 2) throw new Error('Unexpected queue length');
        const head = source.add(0x416).readPointer();
        const attack = head.add(8).readPointer();
        const repeat = head.readPointer().add(8).readPointer();
        check('attack-target', attack.readS16() === 98 && gaEq(attack.add(0x3e), expected) && gaEmpty(attack.add(0x3e)));
        check('attack-specifics', [0xb6,0xba,0xbe].every(o => attack.add(o).readS32() === 0));
        check('repeat-target', repeat.readS16() === 94 && gaEq(repeat.add(0x3e), expected) && gaEmpty(repeat.add(0x3e)));
        check('repeat-specifics', [0xb6,0xba,0xbe].every((o,i) => repeat.add(o).readS32() === 7+i));
    } catch (e) { send({tag:'reference-error', error:String(e)}); }
    finally {
        try {
            if (oldResponses !== undefined) {
                gaClear(source, 0);
                gaAssign(source.add(0x476), savedAction);
                source.add(0x470).writeS32(oldInterrupt);
                source.add(0x46a).writeByteArray(oldResponses);
                gaTypeSet(target.add(0x20), savedType.add(2));
                target.add(0x50a6).writeByteArray(oldActivity);
            }
            for (const action of owned) gaDtor(action);
            for (const id of held.reverse()) gaRelease(objects, id, 0, 0xffffffff);
            send({tag:'reference-done'});
        } catch (e) { send({tag:'reference-error', error:'restore: '+String(e)}); }
    }
}});
