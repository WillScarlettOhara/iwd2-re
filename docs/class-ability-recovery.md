# Class and feat ability recovery

Targets: 71E760 AddNewSpecialAbilities, 722D50 AddClassAbilities (BG2 PDB names),
71E240 ApplyNewFeatBonuses (descriptive #guess). Bundle/shared blocks read:
tmp_class_ability_context.md + manifest.
Target ASM read with sym.py; PE MD5
25cb3d8aa2ec648e94e328d8c467973e.

71E760 dispatches classes1..11 from derived stats, even zero/negative gains for
classes currently nonzero. Delta is narrowed to SHORT in AddClassAbilities.
It adds one BYTE magic resistance per positive total-level gain for race2/6,
subrace1. It reads derived stats even when active stats select the temporary set.

722D50 selects the specialization table, reads active class level, and processes
columns [SHORT(current-delta),current); delta=-1 starts at zero. Table width
stops the entire level loop. Negative columns use the table default but still
execute class bonuses. Prefixes are exact/case-sensitive: FA_ feeds back even
when feedBack!=TRUE, GA_ adds, RA_ removes, AP_ builds level1 subspell effects,
assigns sourceRes and delivers nonnull effects virtually with list1/FALSE/TRUE.
Temporary std::list nodes are destroyed; effect pointers transfer to AddEffect.
Barbarian resistance increments start11/every3; cleric Mask level1 grants Blind
Fight; monk2/9 grants Deflect Arrows/Improved Evasion, magic resistance +23 at13
then +1, field_3E increments every fifth level. SetMonkAbilities and combat
attack-count recalculation run even with no new levels. Attack count narrows to
BYTE before the upper clamp5. The eight barbarian resistances are not clamped.

71E240 compares old/base feat bits; current bit gains affect resistance/saves
and grant abilities. Resistance first wraps a signed BYTE after +5, then clamps
0..100; save bytes wrap without a clamp. Stunning Fist requires real HasFeat,
uses inclusive previous-total..current-total (or1 when newly acquired), granting
at multiples4, including0 for an already acquired feat at total0. Toughness rank
loss subtracts HP, rank bonuses wrap WORDs. Extra Rage selects SPIN260 only when
current barbarian>=15 and previous<15; otherwise SPIN236. Positive rank deltas
grant rage/smite/turning repeatedly; losses do not remove abilities.

Probe scripts/probes/class_ability_reference.py: 466 isolated original cases,
tmp_class_ability_native4/report.json PASS. Real target bodies, class table
selection, HasFeat/prerequisites, GetClassLevel/GetClassMaskLevel, SetFeatValue
and base combat calculation execute. CString/resource dependencies are receipts;
AP uses two nonnull effects separated by null, captures sourceRes/list1/0/1 and
three node erasures. Rooted inputs/hooks; globals/code restored; spawned PID
killed. No original HP claim.
SHA256 2360c40a9617bdc1e2e797877ff2a4eba991a9d41182dff3d58d1d3e8e88bdc3.

ClassAbilityGolden.h:466 typed party replays.
Tables borrow cells through a scoped friend fixture; original storage retained.
Grants require KEY entries. GA/RA use real sprite lists, FA real
TLK feedback, AP real pinned SPL with two delayed effects; timed-list ownership,
immediate duration conversion/provenance and resource balance checked. Restore
actor stats/identity/lists/shortcuts, tables, resource metadata, token, engine,
clock and queues in the same outer tick. Never save. Fixture AI class mask0 skips
monk fist replacement. LevelUp and CompleteCharacterSkills remain stubs.

ARC20-21 rejected a stale namespace; fixed to global BuildSubSpellEffects.
ARC2026-10-17T10-45-13-916737-batch PASS3 warnings, full combat-messages profile:
Win32 Debug build,310-member layout,85/85 steps,466 cases/10271 new checks,
three PDB gates31/181/375 hits,60s CLEAN; restored=true/cleanup_errors[].
Static parity remains unproven for71E760 (missing offset oracle); all three
call-count advisories remain44->4/40->35/21->47. No reduced gates.
Melee6HP, bow/bolt/sling12HP, ammo3->2 and ordinary arrivals3HP pass.
Next: recover CompleteCharacterSkills 612DE0, then scope LevelUp 5E56F0.
