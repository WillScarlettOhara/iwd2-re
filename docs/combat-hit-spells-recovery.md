# Combat hit spell payloads

Final validation: 2026-10-09T12-53-12-150353-batch PASS with seven warnings.
Win32 Debug build, eight shared lints and class audits pass. All 700 checks,
23 UI steps and nine symbol gates pass, followed by a 60-second CLEAN hold.
Target HP is 291 -> 285 -> 278 -> 266 -> 254 -> 242 -> 240: six melee damage,
seven for the melee-plus-self-effect round, twelve per bow/crossbow/sling shot,
then two from direct on-hit delivery. The attacker loses two HP to retaliation
and one to the direct self effect. Each ranged shot spends one charge.
Profile cleanup restores/hashes preserved paths, removes fixtures and reports
no errors. Static parity is GREEN for ApplyOnHitSpells and DeliverEffects;
the other seven targets are YELLOW (call counts or floating-point heuristic).
Operand-count advisories and incomplete/skipped offset/layout checks remain.
This is observed execution and HP evidence, not matched binary/runtime
differential equivalence or a complete recovery of combat.

Recover the connected BuildAbilityEffect (0x54A510), BuildSubSpellEffects
(0x586220) and ApplyOnHitSpells (0x761AB0) path. Full context bundle:
tmp_combat_hit_spells_ctx.md. All referenced shared blocks were read, with
the unchanged CGameSprite header reused from the preceding recovery.
PE disassembly confirms signed level comparisons, the SHORT ability cursor,
the extra GetAbility call, null-caster control flow, effect members, class
packing and unsigned averaging of the special saving-throw modifiers.

BuildAbilityEffect now includes the omitted SPIN126..SPIN132 block. Wizard
and sorcerer class levels plus knowledge of SPWI710 qualify their INT/CHR
modifiers; a cleric with specialization 0x40000 and the required domain spell
qualifies its WIS modifier. The original averages qualifying modifiers using
unsigned DIV, including wrapped negative values. The result changes saveMod,
not damage. Spell Focus and matching-school bonuses also change saveMod.
The old skeleton's Empower Spell/damage comments were incorrect.

Effect metadata uses named CGameEffect members instead of raw offsets.
SPL secondaryType goes to field_48; it is distinct from m_secondaryType.
The legacy m_firstCall field receives twice the Spell Penetration feat value.
A null caster returns after spell metadata, leaving decoded caster metadata
unchanged. A non-sprite receives level one; sprites receive the level byte.
The class byte and cleric specialization index are packed afterward.

BuildSubSpellEffects receives a signed INT level, as the PE does. It chooses
the last eligible ability and computes each effect's caster level separately.
ApplyOnHitSpells no longer truncates the recorded DWORD level to BYTE before
that selection. Effect order, source resource, target point and self/target
message routing remain as in the binary. The Call Lightning and effects-list
callers retain their pre-existing BYTE conversions; those call sites are
outside this recovered lot.

combat-hit-spells.json mounts the existing combat-payload fixtures and centers
the loaded save on the party. Typed tests pin ZRCHIT and substitute local
header/ability/effect tables, restoring every resource pointer and actor field
before the next tick. SPIN resrefs are transient aliases of the pinned helper,
so no installed SPIN resource is overridden. Cases cover spell metadata,
null/non-sprite casters, duration scaling, class packing, all four focus schools,
levels -1/0/1/2/3/255/256/257/65535/65537, empty abilities, full-width on-hit
message selection and special save modifiers (each class, all seven resrefs,
three-class average, unsigned negative average and missing eligibility).

The scenario repeats melee/bow/crossbow/sling Swing damage and real distant
projectile flight. A second melee Swing installs ZRCHIT on the defender and
must trigger ApplyOnHitSpells from the successful melee hit: seven damage on
the defender (six weapon plus one self effect), two retaliation damage on the
attacker. The transient list is restored after Swing queues its payload.
The scenario then queues ZRCHIT directly through ApplyOnHitSpells. Normal message
processing must inflict one damage on the source and two on the target. Test
damage exists only in the disposable process; the profile restores save,
configuration and fixture paths and never saves the altered actors.

The two ZRCHIT effects now deal magic damage. The loaded portrait 0 has four
points of crushing resistance: the original two-point physical retaliation is
correctly absorbed, so it cannot serve as a two-point HP-loss oracle. Magic
damage isolates delivery without changing either actor's resistance. This is
test fixture setup, not a change to recovered damage mechanics.

The initial standalone compile found a const-reference
error in the new block; it was corrected to use the static rule helper.
The preliminary three-target static run did not resolve two runtime symbols:
the CSpell marker was too far from its declaration and a dependency address
in CProjectile.h shadowed the global helper. Those markers were corrected.
That static-only run is not runtime proof and is not the final validation.

The first full run (2026-10-09T12-37-16-046209-batch) passes all 696 typed
construction/ownership/regression checks and the six-plus-one defender damage
case, then fails its retaliation HP expectation. Two dependency markers were
also too far from their definitions, leaving only seven symbol gates. They
were moved beside GetSpellAbilityValue and CGameEffectDamage::ApplyEffect.
The next full run (2026-10-09T12-46-55-150124-batch) exercises all nine symbols
and again fails that expectation. Typed diagnostics show portrait 0 has 349 HP,
four crushing resistance, zero magic resistance, no sphere and a vulnerable
animation. The two-point physical effect is correctly reduced to zero.
These runs are errors, not passes. ZRCHIT is changed to magic damage and the
entire nine-target validation is repeated without reducing any gate.

Next connected target: FUN_0056a5a0 (0x56A5A0), proven to be WhenStruck's
ApplyEffect by constructor 0x4A1B40 installing vtable 0x84C010 and that vtable's
slot +0x08 pointing to it. It installs the on-hit record in the sprite and,
outside local service-provider mode, queues the still-unidentified message
at vtable 0x84D45C (marshal/unmarshal/Run 0x516910/0x516A80/0x516B70).
Recover that installer and its message together; do not claim that the current
synthetic-list tests prove the missing opcode installer or network replication.
