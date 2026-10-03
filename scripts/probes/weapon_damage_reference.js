// Original executable only. Match AutoUI's weapondamage on real party actors.
const wdNative = (address, ret, args) => new NativeFunction(ptr(address), ret,
    ['pointer'].concat(args), 'thiscall');
const wdCtor = wdNative('0x4e7e90', 'pointer', ['uint32','uint32','uint16','uint16','uint16','int','uint32']);
const wdDtor = wdNative('0x4e8180', 'void', []);
const wdDemand = wdNative('0x4e82b0', 'int', []);
const wdRelease = wdNative('0x4e82f0', 'int', []);
const wdAbility = wdNative('0x4e9610', 'pointer', ['int']);
const wdDamage = wdNative('0x73d560', 'pointer', ['pointer','pointer','int','int','pointer','int16','int16','pointer','int']);
const wdDecode = new NativeFunction(ptr('0x51eaf0'), 'pointer', ['uint16','pointer','uint8'], 'mscdecl');
const wdAdd = wdNative('0x51ea90', 'void', ['pointer']);
const wdHeight = wdNative('0x529f40', 'int', ['pointer']);
const wdDeny = wdNative('0x599c70', 'uint8', ['int','uint8','pointer','uint']);
const wdUnlock = wdNative('0x59a010', 'uint8', ['int','uint8','uint']);
const wdCases = [['ZRCMELEE','',1,5], ['ZRCARROW','ZRCBOW',2,9],
    ['ZRCBOLT','ZRCXBOW',12,9], ['ZRCSTONE','ZRCSLING',17,9]];
let wdLoaded = false, wdDone = false, wdTicks = 0, wdIndex = 0, wdWaiting = null, wdMatched = false;
Interceptor.attach(ptr('0x4a7900'), {
    onEnter(args) {
        if (!wdWaiting) return;
        this.effect = this.context.ecx; this.target = args[0];
        if (this.effect.add(0x10c).readS32() !== wdWaiting.ids[0]) { this.effect = null; return; }
        const game = ptr('0x8cf6dc').readPointer().add(0x1c54).readPointer();
        send({tag:'damage-context', phase:'apply-entry', item:wdWaiting.input[0],
            amount:this.effect.add(0x18).readS32(), special:this.effect.add(0x44).readS32(),
            derivedCrushing:this.target.add(0x952).readS16(), tempCrushing:this.target.add(0x17aa).readS16(),
            baseCrushing:this.target.add(0x5f9).readS8(), barbarianLevel:this.target.add(0x967).readU8(),
            nameRef:this.target.add(0x5a4).readU32(), x:this.target.add(6).readS32(), y:this.target.add(0xa).readS32(),
            name:this.target.add(0x5a0).readPointer().readCString(),
            tempMaxHP:this.target.add(0x177c).readS16(), difficulty:game.add(0x4456).readU32(),
            multiplier:game.add(0x445a).readS32(), mpMultiplier:game.add(0x445e).readS32()});
    },
    onLeave() {
        if (this.effect) send({tag:'damage-context', phase:'apply-exit', amount:this.effect.add(0x18).readS32()});
    }
});

function gaSelectSave(screen) {
    const count = screen.add(0x1ca).readS32(), slots = screen.add(0x1d6).readPointer();
    for (let i = 0; i < count && i < 10000; ++i) {
        const slot = slots.add(i * 4).readPointer();
        if (slot.readPointer().readCString() === '000000037-testSpells') {
            screen.add(0x1c6).writeS32(i); wdMatched = true;
            send({tag:'reference-save', name:'000000037-testSpells'}); return;
        }
    }
    throw new Error('Exact save missing');
}
Interceptor.attach(ptr('0x6869c0'), {onLeave() { wdLoaded = true; }});

function wdItem(name) {
    // CItem's original constructor takes CResRef BY VALUE: two stack DWORDs.
    // RET 0x1C at 0x4E817B; usage words follow the eight-byte value.
    const ref = Memory.allocUtf8String(name.padEnd(8, '\0'));
    const item = Memory.alloc(0xf0); // Original CItem ends at +0xEE.
    wdCtor(item, ref.readU32(), ref.add(4).readU32(), 1, 0, 0, 0, 0);
    return item;
}
function wdDelete(object) {
    if (!object || object.isNull()) return;
    wdNative(object.readPointer().readPointer(), 'pointer', ['uint'])(object, 1);
}
function wdLaunch(game, objects, source, target, ids, input) {
    const [itemName, launcherName, expectedMissile, expectedDamage] = input;
    if (source.add(0x41e).readS32() !== 0 || source.add(0x476).readS16() !== 0
        || source.add(0xb64).readS32() !== 0)
        throw new Error('Expected idle source, empty queue and selective damage list');
    const area = source.add(0x12).readPointer(), hp = target.add(0x5c0).readS16();
    if (area.isNull() || !target.add(0x12).readPointer().equals(area) || hp <= 60)
        throw new Error('Expected same area and sufficient HP');
    const pos = [target.add(6).readS32(), target.add(0xa).readS32()];
    const snapshots = [[0x920,0x188], [0x75c,12], [0x62f,1], [0x28,1], [0x38,4]]
        .map(([offset,size]) => [source.add(offset), source.add(offset).readByteArray(size)]);
    let weapon = null, launcher = null, projectile = null, effect = null, demanded = false;
    let transferred = false;
    try {
        weapon = wdItem(itemName); launcher = wdItem(launcherName);
        demanded = wdDemand(weapon) !== 0;
        if (!demanded) throw new Error('Fixture demand failed');
        const ability = wdAbility(weapon, 0);
        if (ability.isNull()) throw new Error('Fixture ability missing');
        const missile = ability.add(0x2a).readU16();
        if (missile !== expectedMissile) throw new Error('Unexpected fixture missile');
        projectile = wdDecode(missile, source, 0);
        if (projectile.isNull()) throw new Error('Projectile missing');
        source.add(0x920).writeByteArray(new Uint8Array(0x188));
        source.add(0x95c).writeS16(19); source.add(0x974).writeS16(10);
        source.add(0x75c).writeByteArray(new Uint8Array(12));
        source.add(0x62f).writeU8(0); source.add(0x28).writeU8(0); source.add(0x38).writeS32(0);
        effect = wdDamage(source, weapon, launcherName ? launcher : ptr(0), 0, 0,
            target.add(0x20), 0, 0, target, 0);
        if (effect.isNull()) throw new Error('Damage effect missing');
        const amount = effect.add(0x18).readS32();
        const valid = effect.add(0xc).readU32() === 12 && amount === expectedDamage
            && effect.add(0x1c).readU32() === 0 && effect.add(0x10c).readS32() === ids[0];
        send({tag:'weapon-base', stage:'effect', item:itemName, damage:amount, missile:missile, valid:valid});
        if (!valid) throw new Error('Unexpected damage descriptor');
        wdAdd(projectile, effect); effect = null;
        for (const [p, bytes] of snapshots) p.writeByteArray(bytes);
        const height = wdHeight(projectile, source);
        transferred = true;
        return {projectile, area, ids, pos, height, hp, input, ticks:0};
    } finally {
        for (const [p, bytes] of snapshots) p.writeByteArray(bytes);
        if (effect && !effect.isNull()) wdDelete(effect);
        if (!transferred && projectile && !projectile.isNull()) wdDelete(projectile);
        if (demanded) wdRelease(weapon);
        if (launcher) wdDtor(launcher);
        if (weapon) wdDtor(weapon);
    }
}
Interceptor.attach(ptr('0x686690'), {onLeave() {
    if (!wdLoaded || wdDone || ++wdTicks < 15) return;
    const game = ptr('0x8cf6dc').readPointer().add(0x1c54).readPointer(), objects = game.add(0x372c);
    const ids = [], actors = [], held = [];
    let launch = null;
    try {
        if (!wdMatched) throw new Error('Named save was not selected');
        for (let i = 0; i < 6; ++i) {
            const id = game.add(0x3816 + i * 4).readS32();
            if (id === -1) continue;
            const ref = Memory.alloc(4);
            if (wdDeny(objects, id, 0, ref, 0xffffffff) !== 0) throw new Error('Party lock failed');
            held.push(id);
            const actor = ref.readPointer();
            for (let n = 0; n < 2; ++n) {
                if (actor.add(6).readS32() === wdActorPositions[n][0]
                    && actor.add(0xa).readS32() === wdActorPositions[n][1]) {
                    if (actors[n]) throw new Error('Ambiguous actor position');
                    ids[n] = id; actors[n] = actor;
                }
            }
        }
        if (!actors[0] || !actors[1] || ids[0] === ids[1]) throw new Error('Matched actors missing');
        const [source, target] = actors;
        const context = {x:target.add(6).readS32(), y:target.add(0xa).readS32(),
            baseCrushing:target.add(0x5f9).readS8(), barbarianLevel:target.add(0x967).readU8(),
            tempMaxHP:target.add(0x177c).readS16(), derivedCrushing:target.add(0x952).readS16(),
            tempCrushing:target.add(0x17aa).readS16(), difficulty:game.add(0x4456).readU32(),
            multiplier:game.add(0x445a).readS32(), mpMultiplier:game.add(0x445e).readS32()};
        for (const key of Object.keys(wdExpectedContext)) {
            if (context[key] !== wdExpectedContext[key]) throw new Error('Target context differs: '+key);
        }
        if (wdIndex === 0 && !wdWaiting) send({tag:'matched-actors', positions:wdActorPositions,
            sourceName:source.add(0x5a0).readPointer().readCString(),
            targetName:target.add(0x5a0).readPointer().readCString(), context:context});
        if (wdWaiting) {
            const damage = wdWaiting.hp - target.add(0x5c0).readS16();
            if (damage === 0 && ++wdWaiting.ticks < 600) return;
            send({tag:'weapon-base', stage:'delivery', item:wdWaiting.input[0],
                damage:damage, expectedDamage:wdWaiting.input[3]});
            if (damage <= 0) throw new Error('No HP loss or projectile timeout');
            wdWaiting = null;
            if (++wdIndex === wdCases.length) { wdDone = true; send({tag:'reference-done'}); return; }
        }
        launch = wdLaunch(game, objects, source, target, ids, wdCases[wdIndex]);
    } catch (e) { wdDone = true; send({tag:'reference-error', error:String(e)}); }
    finally { for (const id of held.reverse()) wdUnlock(objects, id, 0, 0xffffffff); }
    if (launch && !wdDone) {
        wdWaiting = launch;
        try {
            // Virtual Fire slot +0x6C; CPoint is eight BY-VALUE stack bytes.
            wdNative(launch.projectile.readPointer().add(0x6c).readPointer(), 'void',
                ['pointer','int','int','int','int','int','int16'])(launch.projectile,
                launch.area, ids[0], ids[1], launch.pos[0], launch.pos[1], launch.height, 0);
            // Fire transfers ownership; instant projectiles delete themselves here.
        } catch (e) { wdDone = true; send({tag:'reference-error', error:'fire: '+String(e)}); }
    }
}});
