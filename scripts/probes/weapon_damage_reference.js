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
const wdSwing = wdNative('0x739b60', 'int16', ['pointer']);
const wdExecute = wdNative('0x728f80', 'int16', []);
const wdActionCtor = wdNative('0x403a40', 'pointer', []);
const wdActionAssign = wdNative('0x405090', 'pointer', ['pointer']);
const wdActionDtor = wdNative('0x403b40', 'void', []);
const wdUsage = wdNative('0x4e84b0', 'uint16', ['int']);
const wdGameNew = new NativeFunction(ptr('0x7fc95b'), 'pointer', ['uint'], 'mscdecl');
const wdSelect = wdNative('0x74f830', 'void', ['uint8','uint8','uint8','uint8']);
const wdButtonCtor = wdNative('0x593f80', 'pointer', []);
const wdCases = [['ZRCMELEE','',1,5], ['ZRCARROW','ZRCBOW',2,9],
    ['ZRCBOLT','ZRCXBOW',12,9], ['ZRCSTONE','ZRCSLING',17,9]];
let wdLoaded = false, wdDone = false, wdTicks = 0, wdIndex = 0, wdWaiting = null, wdMatched = false;
function wdCompressChecks(objects, source) {
    const compress = projectile => wdNative(projectile.readPointer().add(0x2c).readPointer(),
        'uint8', ['uint'])(projectile, 901);
    let projectile = wdDecode(1, source, 0);
    if (projectile.isNull()) throw new Error('Instant projectile missing');
    try {
        const result = compress(projectile), retained = projectile.add(0x5c).readS32() === -1;
        send({tag:'projectile-compress', case:'unregistered', returned:result, retained:retained ? 1 : 0});
    } finally { wdDelete(projectile); }
    projectile = wdDecode(1, source, 0);
    if (projectile.isNull()) throw new Error('Instant projectile missing');
    const add = wdNative('0x59a0f0', 'uint8', ['pointer','pointer','uint']);
    if (add(objects, projectile.add(0x5c), projectile, 0xffffffff) !== 0) {
        wdDelete(projectile); throw new Error('Registration failed');
    }
    const id = projectile.add(0x5c).readS32(), result = compress(projectile);
    // On success the projectile is deleted: never dereference it afterwards.
    const denyResult = wdDeny(objects, id, 0, Memory.alloc(4), 0xffffffff);
    const removed = denyResult === ptr('0x85186e').readU8();
    if (denyResult === 0) {
        wdUnlock(objects, id, 0, 0xffffffff);
        wdNative('0x59a530', 'uint8', ['int','uint8','pointer','uint'])(objects, id, 0, ptr(0), 0xffffffff);
        wdDelete(projectile);
    }
    send({tag:'projectile-compress', case:'registered', returned:result, removed:removed ? 1 : 0});
}
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

function wdItem(name, usage = 1) {
    // CItem's original constructor takes CResRef BY VALUE: two stack DWORDs.
    // RET 0x1C at 0x4E817B; usage words follow the eight-byte value.
    const ref = Memory.allocUtf8String(name.padEnd(8, '\0'));
    const item = Memory.alloc(0xf0); // Original CItem ends at +0xEE.
    wdCtor(item, ref.readU32(), ref.add(4).readU32(), usage, 0, 0, 0, 0);
    return item;
}
function wdDelete(object) {
    if (!object || object.isNull()) return;
    wdNative(object.readPointer().readPointer(), 'pointer', ['uint'])(object, 1);
}
function wdRound(source, target, ids, input) {
    // Original-only offsets; never apply these reads to the reconstructed build.
    // ApplyOnHitSpells 0x761AB0 walks the sentinel at +0xDB4.
    const sentinel = target.add(0xdb4).readPointer();
    if (source.add(0x41e).readS32() !== 0 || source.add(0x476).readS16() !== 0
        || source.add(0xb64).readS32() !== 0 || target.add(0xb2c).readS32() !== 0
        || sentinel.isNull() || !sentinel.readPointer().equals(sentinel)
        || !source.add(0x5592).readPointer().isNull())
        throw new Error('Swing requires idle source and empty bonus, immunity, retaliation and pending projectile state');
    const hp = target.add(0x5c0).readS16();
    if (hp <= 14 || !source.add(0x12).readPointer().equals(target.add(0x12).readPointer()))
        throw new Error('Swing target area/HP precondition failed');
    // Value-only buffers. Never shallow-copy owned action strings or STL containers.
    const sourceFields = [[0x920,0x188], [0x5a4,0x37c], [0x28,1], [0x38,4],
        [0x342c,8*0x3c], [0x4c12,4*0x0e], [0x4ad8+11*4,4], [0x4ad8+43*4,8],
        [0x4ba4,1], [0x4ba6,2], [0x4c68,1], [0x5592,4], [0x5604,2],
        [0x5622,2], [0x5630,1], [0x5632,4], [0x5636,1], [0x7118,4], [0x72a4,4], [0x70ee,4]];
    if (wdTestAttack || wdTestFinalShot) sourceFields.push([0x474,2], [0x5606,2]);
    if (wdTestFinalShot) sourceFields.push([0x4ad8+12*4,12], [0x3d3a,8]);
    const snapshots = sourceFields.map(([offset,size]) => [source.add(offset), source.add(offset).readByteArray(size)]);
    snapshots.push([target.add(0x58c), target.add(0x58c).readByteArray(4)]);
    let weapon = null, launcher = null, ability = null, demanded = false, hitBonus = 0;
    let savedAction = null, testAction = null;
    let lastUnit = null, reserve = null, installed = false;
    const selected = source.add(0x4ba4).readU8(), selectedAbility = source.add(0x4ba6).readU16();
    const action = wdTestFinalShot ? 3 : [3,98,105,3][wdIndex-wdCases.length];
    try {
        weapon = wdItem(input[0], 3); launcher = wdItem(input[1]);
        demanded = wdDemand(weapon) !== 0;
        if (!demanded) throw new Error('Swing fixture demand failed');
        ability = wdAbility(weapon, 0);
        if (ability.isNull()) throw new Error('Swing fixture ability missing');
        if (wdTestFinalShot) {
            reserve = wdItem(wdFinalMode === 2 ? {ZRCARROW:'ZRCARR2',ZRCBOLT:'ZRCBLT2',ZRCSTONE:'ZRCSTN2'}[input[0]] : input[0], 7);
            // Only a game-allocated object may be deleted by CheckItemDepleted.
            lastUnit = wdGameNew(0xf0);
            if (lastUnit.isNull()) throw new Error('Final unit allocation failed');
            const ref = Memory.allocUtf8String(input[0].padEnd(8,'\0'));
            wdCtor(lastUnit, ref.readU32(), ref.add(4).readU32(), 1,0,0,0,0);
        }
        hitBonus = ability.add(0x14).readS16(); ability.add(0x14).writeS16(32767);
        source.add(0x920).writeByteArray(new Uint8Array(0x188));
        source.add(0x95c).writeS16(19); source.add(0x974).writeS16(10); source.add(0x93a).writeS16(1);
        source.add(0x75c).writeByteArray(new Uint8Array(12)); source.add(0x62f).writeU8(0);
        source.add(0x28).writeU8(0); source.add(0x38).writeU32(0);
        target.add(0x58c).writeU32(0x200000);
        if (input[1]) source.add(0x4ad8+11*4).writePointer(wdTestFinalShot ? lastUnit : weapon);
        installed = true;
        source.add(0x4ad8+43*4).writePointer(input[1] ? launcher : weapon);
        source.add(0x4ad8+44*4).writePointer(ptr(0));
        source.add(0x4ba4).writeU8(input[1] ? 11 : 43); source.add(0x4ba6).writeU16(0);
        source.add(0x4c68).writeU8(0); source.add(0x72a4).writeS32(1);
        source.add(0x5592).writePointer(ptr(0)); source.add(0x5622).writeS16(-1);
        source.add(0x5630).writeU8(0); source.add(0x5632).writeS32(0); source.add(0x5636).writeU8(0);
        if (wdTestFinalShot) {
            source.add(0x4ad8+12*4).writePointer(wdFinalMode === 1 ? ptr(0) : reserve);
            source.add(0x4ad8+13*4).writePointer(ptr(0)); source.add(0x4ad8+14*4).writePointer(ptr(0));
            source.add(0x3d3a).writeByteArray(new Uint8Array(8));
            for (let i=0; i<8; ++i) {
                const button = source.add(0x342c+i*0x3c); wdButtonCtor(button);
                button.add(0x1c).writeS16(43+i); button.add(0x1e).writeS16(0);
            }
        }
        if (wdTestAttack || wdTestFinalShot) {
            // Own the backup through real construction/assignment, including CString refcounts.
            savedAction = Memory.alloc(0xd6); wdActionCtor(savedAction);
            wdActionAssign(savedAction, source.add(0x476));
            testAction = Memory.alloc(0xd6); wdActionCtor(testAction);
            testAction.writeS16(action);
            // CAIAction actee at +0x3E; CAIObjectType instance at +0x0A.
            testAction.add(0x48).writeS32(ids[1]);
            wdActionAssign(source.add(0x476), testAction); source.add(0x474).writeS16(0);
        }
        const execute = () => wdTestAttack || wdTestFinalShot ? wdExecute(source) : wdSwing(source, target);
        let result = execute(), calls = 1, valid = result !== -2;
        for (let frame = 1; frame <= 101 && source.add(0x5622).readS16() !== -2 && !(wdTestFinalShot && wdFinalMode !== 0 && wdFinalMode !== 3 && result === -2); ++frame) {
            source.add(0x5622).writeS16(frame);
            if (wdTestAttack || wdTestFinalShot) source.add(0x474).writeS16(frame);
            result = execute();
            valid = valid && (result !== -2 || (wdTestFinalShot && wdFinalMode !== 0 && wdFinalMode !== 3)); ++calls;
        }
        if (wdTestFinalShot && wdFinalMode === 3) {
            valid = valid && result === 1 && source.add(0x5622).readS16() === -2
                && wdUsage(reserve,0) === 7 && source.add(0x4ad8+11*4).readPointer().isNull()
                && source.add(0x4ba4).readU8() === 12;
            send({tag:'resume-first', item:input[0], calls, remaining:wdUsage(reserve,0),
                returned:result, frame:source.add(0x5622).readS16(), valid});
            source.add(0x5622).writeS16(-1); source.add(0x474).writeS16(0);
            result = execute(); calls = 1;
            for (let frame=1; frame<=101 && source.add(0x5622).readS16() !== -2; ++frame) {
                source.add(0x5622).writeS16(frame); source.add(0x474).writeS16(frame);
                result = execute(); valid = valid && result !== -2; ++calls;
            }
        }
        const remaining = wdTestFinalShot && wdFinalMode === 1 ? 0 : wdUsage(wdTestFinalShot ? reserve : weapon, 0);
        // ACTION_INTERRUPTABLE=1, ACTION_ERROR=-2 (binary constants 0x8485C6/CA).
        valid = valid && result === (wdTestFinalShot && wdFinalMode !== 0 && wdFinalMode !== 3 ? -2 : wdTestAttack && action === 105 ? -1 : 1)
            && (wdTestFinalShot && wdFinalMode !== 0 && wdFinalMode !== 3 ? source.add(0x5622).readS16() >= 0 : source.add(0x5622).readS16() === -2)
            && (wdTestFinalShot && wdFinalMode !== 0 && wdFinalMode !== 3 || source.add(0x5592).readPointer().isNull()) && remaining === (wdTestFinalShot ? (wdFinalMode === 1 ? 0 : (wdFinalMode === 3 ? 6 : 7)) : (input[1] ? 2 : 3));
        const tag = wdTestFinalShot ? 'weapon-finalshot' : (wdTestAttack ? 'weapon-attack' : 'weapon-swing');
        const record = {tag, stage:'round', item:input[0], calls, remaining, valid};
        if (wdTestAttack || wdTestFinalShot) Object.assign(record, {action, returned:result, frame:source.add(0x5622).readS16()});
        if (wdTestFinalShot) {
            Object.assign(record, {deleted:Number(source.add(0x4ad8+11*4).readPointer().isNull()),
                slot:source.add(0x4ba4).readU8(), ability:source.add(0x4ba6).readU16(), set:source.add(0x4c68).readU8()});
            record.valid = record.valid && record.deleted === 1 && record.slot === (wdFinalMode === 1 ? 10 : 12) && record.ability === 0 && record.set === 0;
        }
        send(record);
        return {ids, hp, input:[input[0],input[1],input[2],input[1] ? (wdFinalMode === 3 ? 24 : 12) : 6], ticks:0, round:true, tag};
    } finally {
        if (savedAction) { wdActionAssign(source.add(0x476), savedAction); wdActionDtor(savedAction); }
        if (testAction) wdActionDtor(testAction);
        if (lastUnit && !lastUnit.isNull()) {
            const live = installed ? source.add(0x4ad8+11*4).readPointer() : lastUnit;
            if (!live.isNull()) wdDelete(live); // Never read the deleted lastUnit itself.
        }
        if (weapon) {
            wdDelete(source.add(0x5592).readPointer());
            if (ability && !ability.isNull()) ability.add(0x14).writeS16(hitBonus);
        }
        for (const [p,bytes] of snapshots) p.writeByteArray(bytes);
        if (wdTestFinalShot) {
            source.add(0x4ba4).writeU8(255); wdSelect(source,selected,selectedAbility & 255,0,1);
            for (const [p,bytes] of snapshots) p.writeByteArray(bytes);
        }
        if (demanded) wdRelease(weapon);
        if (launcher) wdDtor(launcher);
        if (weapon) wdDtor(weapon);
        if (reserve) wdDtor(reserve);
    }
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
            const lock = wdDeny(objects, id, 0, ref, 0xffffffff);
            if (lock === 4 || lock === 5) return; // SHARED/DENIED: retry after rendering releases it.
            if (lock !== 0) throw new Error('Party lock failed: '+lock);
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
            // Two immediately driven rounds can deliver on different world ticks.
            // Keep a bounded wait for their aggregate; timeout still emits the actual delta.
            if ((wdTestFinalShot && wdFinalMode === 3 ? damage < wdWaiting.input[3] : damage === 0) && ++wdWaiting.ticks < 600) return;
            send({tag:wdWaiting.round ? wdWaiting.tag : 'weapon-base', stage:'delivery', item:wdWaiting.input[0],
                damage:damage, expectedDamage:wdWaiting.input[3]});
            if (damage <= 0) throw new Error('No HP loss or projectile timeout');
            wdWaiting = null;
            if (++wdIndex === (wdTestFinalShot ? 7 : wdCases.length * (wdTestRounds || wdTestAttack ? 2 : 1))) {
                if (wdTestCleanup) wdCompressChecks(objects, source);
                if (wdTestDepletion) wdDepletionChecks(source);
                wdDone = true; send({tag:'reference-done'}); return;
            }
        }
        launch = wdIndex < wdCases.length
            ? wdLaunch(game, objects, source, target, ids, wdCases[wdIndex])
            : wdRound(source, target, ids, wdCases[wdIndex-wdCases.length+(wdTestFinalShot ? 1 : 0)]);
    } catch (e) { wdDone = true; send({tag:'reference-error', error:String(e)}); }
    finally { for (const id of held.reverse()) wdUnlock(objects, id, 0, 0xffffffff); }
    if (launch && !wdDone) {
        wdWaiting = launch;
        if (launch.round) return; // Swing already queued effects/fired the projectile.
        try {
            // Virtual Fire slot +0x6C; CPoint is eight BY-VALUE stack bytes.
            wdNative(launch.projectile.readPointer().add(0x6c).readPointer(), 'void',
                ['pointer','int','int','int','int','int','int16'])(launch.projectile,
                launch.area, ids[0], ids[1], launch.pos[0], launch.pos[1], launch.height, 0);
            // Fire transfers ownership; instant projectiles delete themselves here.
        } catch (e) { wdDone = true; send({tag:'reference-error', error:'fire: '+String(e)}); }
    }
}});
