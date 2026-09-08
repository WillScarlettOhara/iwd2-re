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

### Attack hit and critical resolution (2026-09-07)

Recovered `CGameSprite::CheckAttackHit` (0x73B760), including the full original
decision path: offhand detection, automatic hits, blink, fallback ability,
normal/touch AC, iterative attacks, weapon/launcher and class/race/feat bonuses,
difficulty, concealment, critical threat and confirmation, feedback, screen
shake, Dirty Fighting, and consumption of missed unarmed special attacks.

The assembler fixes several easily misread details:

- Natural 1 is rejected after feedback/critical processing; natural 20 hits
  irrespective of AC. Confirmation has no special 1/20 rule: it compares its
  roll plus the signed 16-bit attack total with the signed 16-bit AC.
- Concealment runs before confirmation and may veto even a confirmed critical.
  The output critical flag is only written when a critical is accepted; early
  automatic-hit paths leave it alone.
- Derived THAC0 is zero-extended before summing, then the final comparison
  truncates the total to a signed short, as the original does.
- Dirty Fighting writes saving-throw mask 4 and save modifier 4, not dice 4d4.
  Its message uses the target as caller and the attacker as recipient in the
  original. The unarmed miss cleanup uses SPIN232/SPIN233.

The new `combathit` AutoUI command invokes the resolver directly using an
isolated party pair and the existing fixture items. It restores the modified
base stats, derived-stat template, equipment, AI fields, positions, resource
ability, time-stop and game options before releasing the objects. It reseeds
the test thread's CRT RNG and does not save. It does not run Swing, launch a
projectile or inflict base weapon damage.

Static batch `.arc/2026-09-07T17-03-38-batch/` passes source, member-offset,
argument and layout checks, with YELLOW parity for the call count (96 original
calls versus 112 source calls). The earlier static run's CString literal lint
was fixed. An initial test-harness build failure used a non-constant slot count
as an array bound; the arrays now derive their extent from the equipment array.

The Win32 build passes. The first native `combat-hit.txt` run passes 63
assertions and five UI steps, but the observer returns PROCESS-EXITED before
the full observation completes: CheckAttackHit x33,
CheckConcealment x27, FeedBack x36. Covered cases include exact AC boundaries,
natural 1/20, confirmation on 1, sword/keen/improved threat ranges, critical
immunity, concealment, source/target blink, automatic hits, fallback ability,
offhand/iterative attacks, touch attacks, NPC difficulty and all three launchers
with Rapid Shot. Evidence: `work/combat-hit-20260907/hit-first.jsonl` and
`smoke-first-exit.log`. Test fixtures were removed and the original INI restored with
a matching hash. No original/recovered differential capture was made. Screen
shake, Dirty Fighting and unarmed miss effect messages remain runtime-untested;
the scenario disables these side effects to restore party state in one tick.

A fresh rerun passes the same 63 assertions and finishes the full 60-second
observation CLEAN, with the same 33/27/36 function counts. Final evidence is
`hit-second.jsonl` and `smoke-pass.log` in the same directory. No application
crash event or new crash dump was found for the first exit; its cause is not
established. The rerun's 13 fixtures were removed and its INI restored.

### Base weapon damage (2026-09-07)

Recovered `CGameSprite::CreateWeaponDamage` (0x73D560). It builds the damage
effect from weapon and launcher dice, ability/derived/hand bonuses, strength,
Power Attack, favored enemies, conditional feats, special `ZZ` names, critical
multiplier and sneak damage. It also recovers the original side effects for
sneak-attack feats, unarmed special attacks, poison coating and Cleave.

Binary details retained:

- Kai maximizes weapon dice; launcher dice still roll with luck. The minimum
  is one only when the weapon has a nonzero dice count. Bonuses are added as
  zero-extended words, then the total is compared as a signed short.
- Positive offhand strength bonuses above one are halved; positive two-handed
  bonuses are multiplied by 1.5 and truncated, except the `FIST` item icon.
- Sneak damage is added after the critical multiplier. The sneak position
  exception checks movement scale zero, not the animation's CanLieDown method.
- Damage type 8 preserves the original crushing/slashing resistance comparison,
  including its asymmetric choice. The mixed-type boundary is covered in tests.
- Special attack message recipients and portrait-icon removal follow the
  original. These side-effect branches remain outside the current runtime
  matrix; the diagnostic clears their enabling feats/states.

`combathit` now contains 144 assertions (63 hit/critical checks plus 81 damage
checks). `weapondamage` constructs the base damage effect, then sends it through
the existing projectile delivery path. It excludes ITM payload effects so the
observed HP change is attributable to base damage. The fixture weapons carry
1d4+1, and launchers add 1d2+2; the delivery diagnostic temporarily fixes luck
high enough to obtain 5 and 9 respectively. Source statistics are restored
before firing. The target's HP loss persists for this unsaved test session.

Run on the isolated save with two party members and more than 32 HP on portrait
1, without damage reductions or other combat:

```powershell
$py = '.\.venv-reagent\Scripts\python.exe'
& $py scripts/combat_payload_fixture.py install
try {
    & $py scripts/native.py smoke --save 000000037-testSpells --hold 60 `
        --ui scripts/scenarios/combat-damage.txt `
        --hit 'CGameSprite::CreateWeaponDamage,CGameEffectDamage::ApplyEffect,CProjectileBolt::CProjectileBolt,CProjectileSlingStone::CProjectileSlingStone'
} finally {
    & $py scripts/combat_payload_fixture.py remove
}
```

Static batch `.arc/2026-09-07T21-56-47-batch/` passes with YELLOW parity:
133 original calls versus 112 source calls (repeated special-state handling is
shared), plus floating-point/NaN heuristics. The 1.5 multiplier is explicitly
implemented as float; NaNs in the decompile are mis-typed CPoint(-1,-1) values.
The initial member-offset failure was an auditor bug: it stopped at the direct
base class and mislabeled inherited `m_typeAI` as `m_nFacing`. The auditor now
walks transitive bases, with a regression test that excludes unrelated includes.
The three fixture safety tests and this new auditor test pass.

Native validation passes all 144 numeric assertions and eight scenario steps,
then finishes the full 60-second observation CLEAN. Target HP changes are
291 -> 286 (melee) -> 277 (arrow) -> 268 (bolt) -> 259 (sling stone).
Hits: CreateWeaponDamage x31, damage ApplyEffect x4, bolt constructor x1,
sling-stone constructor x1. The Win32 build succeeds. Evidence is under
`work/combat-damage-20260907/`: `damage-first.jsonl` and `smoke-pass.log`.
All 13 test resources were removed and the pre-run INI restored with matching
SHA-256. There is no original/recovered differential trace for this batch.

### Next recovery batch

Camera validation (2026-09-08): `000000037-testSpells` contains six party
members in AR2000. It loads at scroll (0,2447), placing portrait 0 at screen
(232,-1589). `center` moves the scroll to (0,642); all six members then lie
inside the 800x433 viewport. The new `combat-camera.txt` scenario verifies each
member with `expect partyvisible <slot>` and completes 15 steps, with
`CInfinity::SetViewPosition` observed 12 times and a CLEAN 30-second smoke.
A separate negative scenario omitting `center` reports UI-FAIL at portrait 0,
as intended. Evidence: `work/combat-targeting-20260908/{scout,camera,camera-negative}.jsonl`.
Win32 build succeeds; the INI was restored with SHA-256
`1E7BF0A7526C8FDB85BC42AE9998ACD3C4CFEF42CCABBB11EABCF104BFA07DA6`.
The damage and payload scenarios now require portraits 0 and 1 on screen.
This verifies a targeting precondition, not mouse attacks or new damage paths.

Weapon-click recovery (2026-09-08): the 0x3C..0x43 branch of
`CInfButtonArray::OnLButtonPressed` now follows 0x592D92..0x592ED4. The active
engine virtual call at +0x44 is **GetShiftKey**, not a weapon-swap capability.
Shift resets targeting and opens state 0x65 with the clicked weapon ability
index. A normal click clears modal state, selects the effective weapon button,
and calls `ReadyQuickSlotByMode(index,1)`. This reaches the existing
`SetSelectedWeaponButton`; selection resets to 100 if it cannot ready targeting.
Clicking the selected weapon cancels targeting. The existing offhand collapse
rules are preserved. The enclosing dispatcher remains incomplete.

The first character's observed bar has type 60 at control 7 and type 61 at
control 8. `combat-weapon-click.txt` exercises ready/cancel/re-ready, asserts
game state/icon/button as (2,12,60), (0,12,100), (2,12,60), and then clicks
portrait 1's ground position (232,168). Before clicking it asserts that the
picker identifies object 8847495, and afterwards checks (0,12,100).
The equivalent captured 20-step scenario passes with a CLEAN 30-second smoke:
ReadyQuickSlotByMode x2, SetSelectedWeaponButton x2, OnActionButton x2,
GroupSetTarget x1. Evidence: `work/combat-weapon-click-20260908/target.jsonl`.
The earlier 15-step ready/cancel-only run also passes (`click.jsonl`).
The Win32 build succeeds. Static batch `.arc/2026-09-08T02-52-36-batch/`
passes with YELLOW call-count parity (444 original vs 372 source calls for the
whole incomplete dispatcher). Shift/picker and offhand branches have assembly
evidence only, not runtime assertions; there is no original/recovered trace.
This proves mouse targeting reaches GroupSetTarget, not attack execution or
damage. The INI is restored after the runs. Full context and assembly are in
`tmp_combat_weapon_click_full.md` and `tmp_weapon_click_asm.txt`.

Prioritize these over the generic size-ranked target list:

1. `Attack` candidate 0x737EF0: target validity, approach/range/LOS and dispatch
   into 0x739B60. Connect action IDs 3/98/105 through 0x728F80 only when its
   executor can actually run; preserve the original target/lock handling.
2. `Swing` candidate 0x739B60: attack-frame progression, projectile creation,
   to-hit and damage calculation, effects, ammo and feedback.
3. Hit/critical resolution (0x73B760) and base damage (0x73D560) are recovered;
   the normal attack executor still needs to call them. Current tests invoke
   them directly and therefore do not prove an ordinary mouse attack works.
4. Dependencies: 0x75D450 depleted ammunition/equipment cleanup; 0x761AB0
   on-hit subspell effects. Full context bundles for the
   main chain and these dependencies are available as `tmp_combat_*.md` locally.
5. Drive a real forced attack on another party member in both executables,
   compare captures, and test normal/critical/missed blows, ammunition exhaustion,
   target movement and death. The payload-only test cannot prove those behaviors.

### Ammunition selection and melee fallback (2026-09-08)

Recovered `CGameSprite::SelectLauncherAmmo` (0x75E330) and replaced the stub
`EquipMostDamagingMelee` (0x75DFD0). The first is a dependency of depleted-item
handling at 0x75D450, and originally also of SetWeaponSet. The current
SetWeaponSet implementation still contains its own approximate ammunition
search; it has not been replaced in this batch. Attack/Swing remain unrecovered.

SelectLauncherAmmo scans slots 11..14 for ability 0 with low-byte type 2 and
the exact launcher pointer returned by GetLauncher. It selects the greatest
positive usage count, retaining the first equal stack. The original compares
an unsigned candidate count against a signed saved SHORT; the wrap at 32768
is preserved. With no candidate, only fallback flag exactly 1 invokes melee
selection and returns ACTION_DONE; otherwise it returns ACTION_ERROR. On
success it selects ability 0 and updates the first matching launcher quick
button in slots 43..50 from GetItemUsages, remembers the ammunition slot, and
refreshes quick buttons. Returned button/list ownership follows the binary.

EquipMostDamagingMelee rejects an occupied slot 42. Across all eight weapon
slots it scores low-byte type-1 abilities as signed SHORT(dice count * dice
size + bonus), accepts scores >= the current maximum, and therefore retains
the last equal candidate. It selects the candidate's weapon set, not its
scored ability; if no candidate reaches zero, it selects fists. For the selected
PC leader it updates the bar. The original's two GetGroupList calls and leak
of the first allocated list are preserved (0x75E0EC..0x75E10E).

`combat-ammo.txt` invokes the actual helpers on temporarily substituted ZRC
fixtures. For each of bow, crossbow and sling it checks: larger compatible
stack, equal stacks, depleted later stack, no compatible nonempty stack, and
fallback to melee in slot 45/set 1. Each selection is also checked against
the quick button and remembered ammo slot. A larger incompatible stack is
present throughout. Original equipment pointers, selection, weapon set,
quick weapon buttons, remembered ammo and base attack count are restored.
No damage or saving occurs. Fixtures now declare weapon quick-slot type 1,
so GetItemUsages can build their buttons; payload and damage values are unchanged.

The first 12-case run passed its assertions but terminated before its 30-second
observation completed (PROCESS-EXITED, no captured exception); this is not a
clean runtime result and its cause remains unidentified. The subsequent
15-case run including melee fallback passes all assertions and the complete
60-second observation CLEAN. Hits: SelectLauncherAmmo 15,
EquipMostDamagingMelee 3, SetWeaponSet 15 (including save loading).
Evidence is under `work/combat-ammo-20260908/`: `ammo.jsonl`, `first-guard.log`,
`ammo-fallback.jsonl`, `fallback-guard.log`. Win32 build and three fixture
safety tests pass. All 13 fixtures were removed and the INI restored with
SHA-256 `1E7BF0A7526C8FDB85BC42AE9998ACD3C4CFEF42CCABBB11EABCF104BFA07DA6`.
Signed-overflow counts, melee ties, fist fallback and the slot-42 rejection
have assembly evidence only. There is no original/recovered differential trace.
Static batch `.arc/2026-09-08T06-12-15-batch/` passes source, member and class
audits. Both parity results are YELLOW: 21 original vs 13 source calls for
SelectLauncherAmmo (inlined button/default-ability construction), and 12 vs 18
for EquipMostDamagingMelee (accessors expressed as source calls).

### Depleted items and ranged selection (2026-09-08)

Recovered CheckItemDepleted (0x75D450, BOOL with SHORT slot and ability) and
EquipRangedWeapon (0x75E150). The former notifies the active engine's
UpdatePersonalItemStatus, checks usage counts, and applies the original usage
flags: 1 deletes the item, 2 replaces its resource with the used-up item (or
deletes it if none), other flags retain the depleted item. Unusable events
carry the resource in trigger **string2**, not string1. A replacement ammo
stack with the same eight-byte resource returns FALSE so the attack can
continue; otherwise the helper returns TRUE. It does not decrement charges;
that remains the caller's job. Invalid ability 0 has its separate deletion
path; nonzero missing abilities send Unusable and release all demands.
The original slot bound is `> NUM_SLOT`, preserving its slot-51 edge case.

EquipRangedWeapon retains an already selected ranged ability for a party
member, otherwise scans ammunition then weapon slots for a compatible ranged
ability, selects the corresponding weapon set, or falls back to melee.
It deliberately does not rank or check charges. Its existing callers have
not been connected in this batch. The original party-member entry demands
the selected item before its null test, so a valid selected item is a precondition.

The ammo scenario now executes 36 checks: the previous 15, plus deletion of
an exhausted heap-owned stack and replacement by the identical resource,
remaining charges, invalid slot, empty slot, missing ability, unlimited melee
ability, and switching from fists back to ranged, for bow/crossbow/sling.
The first test failed because CItem construction with count 0 initializes a
charge; explicitly SetUsageCount(0,0) now establishes the depleted precondition.
No recovery-code change was needed for that failed test. The corrected run
passes all checks and a complete 60-second CLEAN observation: CheckItemDepleted
x18, EquipRangedWeapon x3, SelectLauncherAmmo x18, CMessageSetTrigger::Run x10.
Evidence: `work/combat-depletion-20260908/depletion-retry.jsonl` and
`guard-pass.log`; the failed preparation is retained in `depletion.jsonl`.
The ammo fixtures now declare stack limit 40 and consumable usage flags/max
charges. Other fixture payload/damage values are unchanged. All 13 fixtures
were removed and the original INI hash restored; three fixture safety tests pass.

Win32 build passes. Static batch `.arc/2026-09-08T06-20-23-batch/` gave GREEN
parity for EquipRangedWeapon and YELLOW for CheckItemDepleted (110 original
calls vs 61 source calls, repeated trigger/destructor handling shared), but
failed two narrow-CString-literal lints. After replacing those with empty
constructors, `.arc/2026-09-08T06-23-22-batch/` passes all static checks with
`--no-parity`; earlier parity evidence remains applicable. Resource transformation,
missing ability 0, special slot 42 and non-party fallback branches are not yet
runtime-covered. No original/recovered differential trace. Attack/Swing still
need recovery; their on-hit subspell dependency 0x761AB0 remains next.
