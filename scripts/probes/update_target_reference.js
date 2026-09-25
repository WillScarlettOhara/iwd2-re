// Optional original-only extension to group_attack_reference.js.
// Layout and thresholds checked against 0x733660; never used on our build.
function gaTargetChecks(source, target, ids, check) {
    const update = gaNative('0x733660', 'void', ['pointer']);
    const orderable = gaNative('0x728460', 'int', ['int']);
    const setType = gaNative('0x7670d0', 'uint8', ['uint8']);
    const snapshots = [[source, 0x5690, 4], [source, 0x5648, 0x24],
        [target, 0x5648, 0x24], [source, 0x50b2, 4], [source, 0x474, 2],
        [source, 0x5576, 2], [source, 0x724e, 4]]
        .map(([p, offset, size]) => [p.add(offset), p.add(offset).readByteArray(size)]);
    const marker = target.add(0x5648);
    const count = () => marker.add(2).readS16();
    const force = () => marker.add(5).readU8();
    const targetId = () => source.add(0x5690).readS32();
    try {
        source.add(0x5690).writeS32(-1);
        check('target-orderable-precondition', orderable(source, 0) !== 0);
        source.add(0x50b2).writeS32(1);
        source.add(0x474).writeS16(0);
        source.add(0x5576).writeS16(73);
        marker.writeU8(1); marker.add(2).writeS16(0); marker.add(5).writeU8(0);
        update(source, ptr(0));
        check('target-empty-clear', targetId() === -1);
        update(source, target);
        check('target-select-threshold', targetId() === ids[1] && marker.readU8() === 0 && count() === 1 && force() === 0);
        source.add(0x5576).writeS16(74);
        update(source, target);
        check('target-duplicate-no-op', count() === 1 && force() === 0);
        update(source, ptr(0));
        check('target-clear', targetId() === -1 && marker.readU8() === 1 && count() === 0);
        update(source, target);
        check('target-force-render', force() === 15);
        source.add(0x5648).writeU8(1); source.add(0x564a).writeS16(0);
        update(source, source);
        check('target-switch', targetId() === ids[0] && source.add(0x564a).readS16() === 1
            && count() === 0 && marker.readU8() === 1);
        update(source, ptr(0));
        marker.add(5).writeU8(0); source.add(0x474).writeS16(1);
        update(source, target);
        check('target-action-count', count() === 1 && force() === 0);
        setType(marker, 0);
        update(source, ptr(0));
        check('target-shared-marker', count() === 1 && marker.readU8() === 0);
        source.add(0x50b2).writeS32(0);
        update(source, target);
        check('target-unselected', targetId() === ids[1] && count() === 1);
        update(source, ptr(0));
        check('target-unselected-clear', targetId() === -1 && count() === 1);
        source.add(0x50b2).writeS32(1); source.add(0x724e).writeS32(1);
        update(source, target);
        check('target-unorderable', targetId() === ids[1] && count() === 1);
        update(source, ptr(0));
        check('target-unorderable-clear', targetId() === -1 && count() === 1);
        source.add(0x724e).writeByteArray(snapshots[6][1]);
        source.add(0x5690).writeS32(0x8000);
        update(source, target);
        check('target-missing-old-preserved', targetId() === 0x8000 && count() === 1);
        update(source, ptr(0));
        check('target-missing-old-clear-preserved', targetId() === 0x8000);
    } finally {
        for (const [p, bytes] of snapshots) p.writeByteArray(bytes);
    }
}
