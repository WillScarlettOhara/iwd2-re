# Combat recovery: weapon effect delivery

The first combat batch restores `CGameSprite::ApplyWeaponEffects` (0x71F860)
and ordinary crossbow-bolt / sling-stone projectile creation. It also fixes the
uninitialized result in `CAIObjectType::GetObjectFromId` (0x40D0F0), exposed by
an `Exists` trigger in the test save before combat testing could start.

**Normal weapon attacks are not connected yet.** The diagnostic scenario below
calls the recovered payload helper directly. It does not establish attack-roll,
base weapon damage, animation timing, ammunition consumption, or full combat
parity with the original executable.

## Binary evidence

- 0x71F860: stack arguments are projectile, item, signed 16-bit ability index,
  target point reference (`ret 0x10`). Effects of the selected item precede
  ability-zero effects of its launcher. Both get source position / ID and target
  position, then `IcewindMisc::ApplyDamageModifiers`. Target 1 posts a self effect,
  2 transfers ownership to the projectile, 3-6/8 broadcast then delete the
  template, and 7 discards it. The selected item's demand is released before
  launcher effect extraction; the launcher's demand is released afterwards.
- 0x531360, vtable 0x84E14C: BOLT travelling leaf, 16 directions, speed x5.
- 0x5319A0, vtable 0x84E2E4: MAGICSTN travelling leaf, one direction, palette
  range 0 = 0x1B, speed x5. Both vtables inherit the travelling update, render,
  fire and arrival slots; neither leaf adds fields.
- Factory 0x51EAF0: bolt IDs 0xC/0xF/0x10 and stone IDs 0x11/0x14/0x15 set
  height enabled, empty launch/arrival sounds, and the normal projectile ID.
- 0x40D114 explicitly zeroes the output pointer used by `GetShare`. Failure can
  leave that output untouched; returning stack garbage caused the observed crash.

Full context bundles were assembled before edits; constructor arguments,
effect ownership, member offsets and factory branches were checked in PE assembly.

## Validation (2026-09-06)

- VS2019/v142 Debug Win32 build passed.
- Static batch `.arc/2026-09-06T19-52-09-batch/` passed with one warning:
  the three new functions are GREEN; `GetObjectFromId` remains YELLOW because
  its existing recovery un-inlines the object search (16 binary calls vs 5 source
  calls). Only the missing initialization is fixed here.
- The final native smoke observed 60 seconds, returned CLEAN, and reached all
  five requested symbols: payload helper x12 (four each of null-item,
  invalid-ability and normal calls), bolt constructor x1, stone constructor x1,
  damage application x7, and object lookup x1806.
- AutoUI passed all 12 steps. Portrait 1 HP: 291 -> 290 (instant), 290 -> 287
  (arrow), 287 -> 284 (bolt), 284 -> 281 (stone). Every assembled effect passed
  source ID, source/target position, resource, amount, order and count assertions.
- Nine Python tests passed for fixture installation/removal and batch planning.
  `arc` now passes a resolved shared header to the layout auditor and skips
  memberless leaves explicitly instead of looking for nonexistent leaf headers.
- Earlier runs remain failures: the first crashed on the uninitialized lookup;
  the second passed the payload assertions but the process exited during the
  observation window. That exit was not reproduced by the final 60-second run;
  its cause is not established. The second run also exposed the fixed layout
  harness error. Neither earlier run is presented as a full PASS.

Local runtime evidence: `work/combat-20260905/payload-pass.jsonl`,
`smoke-pass.log`, `smoke-pass.txt`. Fixtures were removed and the backed-up INI
restored. No matched original/reconstruction runtime capture or visual comparison
was produced; GREEN static checks and exact diagnostic HP deltas do not prove
full original-game combat or rendering parity.

## Reproduce the diagnostic

Use the repository venv and an isolated, settled save with two party members,
more than 12 HP on portrait 1, no damage modifiers/reduction or combat in progress.
The test inflicts 10 total damage without saving. Restore gameplay settings after
the run if the runtime adapter changed them.

```powershell
$py = '.\.venv-reagent\Scripts\python.exe'
& $py scripts/combat_payload_fixture.py install
try {
    & $py scripts/native.py smoke --save 000000037-testSpells --hold 50 `
        --ui scripts/scenarios/combat-payload.txt `
        --hit 'CGameSprite::ApplyWeaponEffects,CProjectileBolt::CProjectileBolt,CProjectileSlingStone::CProjectileSlingStone,CGameEffectDamage::ApplyEffect'
} finally {
    & $py scripts/combat_payload_fixture.py remove
}
```

The fixture installer refuses existing resources and its remover checks exact
bytes. `ZRC*` ITMs contain deterministic opcode-12 payloads: 1 from the item,
2 from the launcher. No base weapon damage is fabricated. AutoUI temporarily
substitutes a launcher only while calling the helper, restores equipment before
the next engine tick, checks effect provenance, and fires the actual projectile
at portrait 1. Melee delivery uses the immediate projectile; ranged tests use
arrow 2, bolt 12, and sling stone 17. HP deltas are assertions, not just hit counters.

## Remaining combat chain

Prioritize these over the generic size-ranked target list:

1. `Attack` candidate 0x737EF0: target validity, approach/range/LOS and dispatch
   into 0x739B60. Connect action IDs 3/98/105 through 0x728F80 only when its
   executor can actually run; preserve the original target/lock handling.
2. `Swing` candidate 0x739B60: attack-frame progression, projectile creation,
   to-hit and damage calculation, effects, ammo and feedback.
3. 0x73B760: hit/critical resolution; 0x73D560: base weapon damage and modifiers.
   These are large unrecovered functions, not substitutes for opcode-12 delivery.
4. Dependencies: 0x75D450 depleted ammunition/equipment cleanup; 0x761AB0
   on-hit subspell effects. Additional unresolved calls remain in the bundles.
5. Drive a real forced attack on another party member in both executables,
   compare captures, and test normal/critical/missed blows, ammunition exhaustion,
   target movement and death. The payload-only test cannot prove those behaviors.
