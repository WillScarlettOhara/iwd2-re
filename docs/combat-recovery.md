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

### Attack-rule prerequisites (2026-09-06)

Recovered `GetTouchAC` (0x723DB0), `GetDualWieldPenalty` (0x73D080), and
`GetSituationalAttackBonus` (0x73D1B0). Renamed the previously recovered
0x723F60 from `GetAttacksPerRound` to `GetMonkACBonus`, including character-sheet
callers: the binary returns an unarmored monk's positive Wisdom AC modifier,
not an attack count. No object-layout fields were added.

PE details deliberately retained:

- Touch AC includes natural, deflection and dodge bonuses, plus capped Dexterity
  and monk Wisdom. Blindness removes only positive Dexterity without Blind Fight.
- Dual wield starts at -6/-10; Ambidexterity, Two Weapon Fighting, light offhand
  types 16/19 and an eligible ranger adjust these. Shields bypass the penalty.
  The binary compares the selected weapon to literal 42; `SLOT_FIST` is 10 and
  cannot replace that comparison. The first runtime matrix caught this mistake.
- Range uses squared search-grid distance minus an unsigned byte of personal
  space, with thresholds 3/32/64. Negative range modifiers are **zero-extended
  WORDs** in the INT return (65528/65534/65531). The invisible-source branch reads
  a **DWORD** at 0x85BB86 (0xFFFB0002), overlapping the adjacent range constant.
  Blind Fight is queried on the target. These widths follow assembly rather than
  normalizing the result to the apparent D&D rule; the hit resolver subsequently
  uses its own widths.

`scripts/scenarios/combat-rules.txt` exercises these helpers directly on loaded
party sprites. `combatrules` temporarily changes stats, equipment, feats and
positions under deny locks, restores them before returning to the engine, and
does not save or inflict damage. The fixture installer also provides a dagger,
armor and shield; it now manages ten resources with the same no-overwrite and
verified-removal protections. This is prerequisite coverage, **not an Attack or
Swing test**. It does not establish original/reconstruction differential parity.

The corrected runtime matrix passes 27 numeric assertions: AC and blindness,
monk Wisdom/shield exclusion, offhand types, feats, ranger and selection 42,
invisibility/target Blind Fight, and four range cases including Precise Shot.
The first matrix failed selection 42 (preserved as `rules-first-fail.jsonl`);
the correction was rebuilt before rerunning. All nine Python fixture/batch tests
also pass after extending the expected fixture inventory from seven to ten.

Final batch `.arc/2026-09-06T20-07-43-batch/`: PASS, four GREEN parity checks,
clean lints/layout audit, VS2019 Win32 build, and CLEAN 60-second observation.
Runtime hits: touch AC x6, monk AC x8, dual wield x9, situational bonus x10.
The remaining name warning is the old Ghidra export label for 0x723F60.
Local evidence is in `work/combat-rules-20260906/rules-pass.jsonl`,
`smoke-pass.log` and `smoke-pass.txt`. Test resources were removed and the original
INI restored after the run. Reproduce with the fixture installer followed by
`native.py smoke --save 000000037-testSpells --hold 60 --ui scripts/scenarios/combat-rules.txt`
and a `--hit` list of the four names above, then remove the fixtures.

### Proficiency, concealment and iterative attacks (2026-09-07)

Recovered `GetWeaponProficiencyHitBonus` (0x73CF10), `CheckConcealment`
(0x73C490), and `CRuleTables::GetIterativeAttackBonus` (0x547020).

- Proficiency takes the higher weapon/launcher rank and reads WSPECIAL column 0.
  Untrained party members (or character creation) receive -4 except for ability
  flag 0x10000; other creatures use the table's rank-zero entry.
- Concealment is 50% for attacker blindness or target invisibility combined with
  the creature's visible flag, otherwise 20% for state 0x20000000 or `m_bHiding == 1`.
  Without either it returns success without consuming a random roll. Blind Fight
  retries only a failure and emits the original localized world feedback.
- Iterative attack bonus is base minus step times index, clamped to zero.
  The character sheet now calls these recovered proficiency/iteration helpers
  and the preceding batch's dual-wield helper. Its removed duplicate incorrectly
  granted the light-armor two-weapon exception to rogues rather than rangers.

`combat-rules.txt` now runs 58 numeric assertions and opens the character sheet's
Equipment tab via the real UI button. Cases include all three launchers, higher
rank selection in either direction, untrained PC/NPC/touch behavior, threshold
boundaries and exact random-roll consumption. Concealment cases deliberately
reseed the test thread's CRT RNG; stats, IDs, equipment and positions are restored,
but this is an isolated diagnostic and must not be used as a save-producing run.

Static batch `.arc/2026-09-07T02-10-13-batch/` passes with one warning:
concealment and iteration are GREEN; proficiency is YELLOW (12 binary calls vs
17 source calls, including un-inlined accessors/table lookup). The rule-table
layout audit is skipped by the existing auditor; no members were added.
VS2019 Debug Win32 compilation passes. A first 60-second native run passed all
58 cases and reached all three new functions. Final batch
`.arc/2026-09-07T02-12-21-batch/` passed the shared checks and a fresh 60-second
native observation with nine UI steps, including the Equipment tab. Parity was
not repeated in this final batch; the preceding warning still applies.
Hits: proficiency x13, concealment x12, iteration x21, equipment information x2.
Evidence: `work/combat-hit-prereqs-20260907/rules-pass.jsonl`, `smoke-pass.log`
and `smoke-pass.txt`. All ten fixtures were removed and the saved INI restored.
No paired original/reconstruction capture or visual parity comparison was made.

### Legacy `ZZ` weapon bonuses (2026-09-07)

Recovered `GetSpecialWeaponHitBonus` (0x73CD00), the last standalone modifier
called by the hit resolver. It examines the weapon and then its launcher; a
launcher whose resref begins with `ZZ` overrides the ammunition/weapon code.
The third character selects a race (`A` through `Z`) or one of six hard-coded
alignment mask/value pairs. The fourth character maps `0..4` to `-5..-1` and
`5..9` to `+1..+5`.

The exact 26-entry race map and six alignment pairs come from PE data at
0x8B8548 and 0x8B85B0. The implementation keeps the original shared scratch
buffer and launcher demand/release behavior. The high-nibble alignment codes
`0..2` compare `(alignment & 0xF0)` with `1..3`, which cannot match the normal
`0x11..0x33` values; codes `3..5` compare the low nibble and do match. This
historical behavior is retained rather than corrected.

The combat fixture set now contains 13 resources, adding race and alignment
`ZZ` items. The runtime matrix contains 75 cases, including a matching race,
nonmatching race, the high-mask behavior, a working low-mask alignment and a
nonmatching alignment, plus launcher override of the weapon/ammunition code.
It also checks both bonus endpoints (-5/+5) and target codes `6..9`: the
original reads adjacent PE constants for these codes, whose comparison values
cannot match a BYTE alignment. The recovered function returns zero for them.
Fixture cleanup tests still enforce all-or-nothing
installation/removal without touching unrelated resources.

Static batch `.arc/2026-09-07T02-22-06-batch/` passes with a YELLOW call-count
warning (21 binary calls vs 26 source calls from un-inlined accessors/CRT use).
The first runtime exposed the impossible high-mask case; the second exposed an
incorrect expected value after changing `ZZ05` to `ZZ35`. Both are retained as
failure evidence. A corrected 63-case run was CLEAN, then the final launcher
override run passed all 64 assertions and nine UI steps and remained CLEAN for
60 seconds. Runtime hit count: special weapon bonus x6. Evidence is in
`work/combat-zz-20260907/`: `rules-pass.jsonl`, `smoke-pass.log`,
`rules-first-fail.jsonl`, and `rules-second-fail.jsonl`. No original differential
capture was made.

`CItem::GetCriticalRangeBonus` (0x4E9840) is also recovered and replaces the
local copy in `CScreenCharacter`. Item types 16/19/20/23/27/31/57/69 contribute
one point; ability flag `0x20000` contributes one more. A null ability still
allows the item-type bonus. Five additional matrix cases cover a sword, bow,
null ability, and the flag on both weapon types.

The updated static batch `.arc/2026-09-07T16-46-35-batch/` passes: critical
range GREEN, special weapon bonus YELLOW (21 binary calls versus 25 source
calls). The CItem layout audit is unavailable; no fields were added. The
Win32 build succeeds with existing warnings in unrelated files. The fixture
installer/remover's three Python tests pass, including preservation of unrelated
resources and cleanup of all three new `ZZ` fixtures.

The first 75-case run passed its assertions but the observer returned
NOT-INSTRUMENTED: it was given the nonexistent UI symbol
`CScreenCharacter::UpdateCharacterEquipmentInfo`. Its evidence is preserved as
`rules-uninstrumented.jsonl` and `smoke-uninstrumented.log`; the correct symbol
is `CScreenCharacter::UpdateEquipmentInformation`.

The corrected run passed all 75 assertions and nine UI steps, with 60 seconds
of CLEAN observation: special weapon bonus x12, critical range x7, equipment
information x2. Final captures are `rules-final.jsonl` and `smoke-final.log`
in the same evidence directory. All 13 fixtures were removed and the pre-run
INI restored with matching SHA-256. This still does not exercise normal attack
resolution or base weapon damage.

### Next recovery batch

Prioritize these over the generic size-ranked target list:

1. `Attack` candidate 0x737EF0: target validity, approach/range/LOS and dispatch
   into 0x739B60. Connect action IDs 3/98/105 through 0x728F80 only when its
   executor can actually run; preserve the original target/lock handling.
2. `Swing` candidate 0x739B60: attack-frame progression, projectile creation,
   to-hit and damage calculation, effects, ammo and feedback.
3. 0x73B760: hit/critical resolution; 0x73D560: base weapon damage and modifiers.
   These are large unrecovered functions, not substitutes for opcode-12 delivery.
4. Dependencies: 0x75D450 depleted ammunition/equipment cleanup; 0x761AB0
   on-hit subspell effects. Full context bundles for the
   main chain and these dependencies are available as `tmp_combat_*.md` locally.
5. Drive a real forced attack on another party member in both executables,
   compare captures, and test normal/critical/missed blows, ammunition exhaustion,
   target movement and death. The payload-only test cannot prove those behaviors.
