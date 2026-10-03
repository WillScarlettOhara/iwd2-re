// Original IWD2.exe only; ExecuteAction's GroupAttack branch at 0x729E07.
// Match AutoUI combatgroup mode 2 after discarding the two direct-test actions.
function gaDispatchChecks(source, target, check) {
    const execute = gaNative('0x728f80', 'int16', []);
    const snapshots = [[source, 0x12, 4], [source, 0x24, 1], [target, 0x24, 1],
        [source, 0x592, 2], [source, 0x5690, 4],
        [source, 0x5648, 0x24], [target, 0x5648, 0x24]]
        .map(([p, offset, size]) => [p.add(offset), p.add(offset).readByteArray(size)]);
    const action = source.add(0x476), type = target.add(0x20);
    const lastReturn = source.add(0x592);
    const queueCount = () => source.add(0x41e).readS32();
    try {
        gaClear(source, 0);
        source.add(0x5690).writeS32(-1);
        source.add(0x24).writeU8(2); target.add(0x24).writeU8(255);
        const expected = gaType(type); expected[8] = -1;
        action.writeS16(94);
        gaTypeSet(action.add(0x3e), source.add(0x20));
        action.add(0xb6).writeS32(7); action.add(0xba).writeS32(8); action.add(0xbe).writeS32(9);
        source.add(0x12).writePointer(ptr(0));
        lastReturn.writeS16(123);
        let result = execute(source);
        source.add(0x12).writeByteArray(snapshots[0][1]);
        check('dispatch-no-target', result === -1 && lastReturn.readS16() === result && queueCount() === 0);
        source.add(0x470).writeS32(1); lastReturn.writeS16(123);
        result = execute(source);
        check('dispatch-interrupted', result === 1 && lastReturn.readS16() === result && queueCount() === 0);
        source.add(0x470).writeS32(0); lastReturn.writeS16(123);
        result = execute(source);
        check('dispatch-active', result === -1 && lastReturn.readS16() === result);
        check('dispatch-enemy-selector', gaEq(action.add(0x3e), expected) && gaEmpty(action.add(0x3e)));
        check('dispatch-two-actions', queueCount() === 2);
        if (queueCount() !== 2) throw new Error('Unexpected dispatch queue length');
        const head = source.add(0x416).readPointer();
        const attack = head.add(8).readPointer(), repeat = head.readPointer().add(8).readPointer();
        check('dispatch-silent-attack', attack.readS16() === 98 && gaEq(attack.add(0x3e), expected) && gaEmpty(attack.add(0x3e)));
        check('dispatch-repeat', repeat.readS16() === 94 && gaEq(repeat.add(0x3e), expected)
            && [0xb6,0xba,0xbe].every((o,i) => repeat.add(o).readS32() === 7+i));
    } finally {
        gaClear(source, 0);
        for (const [p, bytes] of snapshots) p.writeByteArray(bytes);
    }
}
