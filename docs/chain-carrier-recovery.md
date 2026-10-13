# Chain carrier recovery

Recovered IcewindCProjectileChainLightning (#guess), native vtable 850F14,
size 2BE. Constructor 57C510 takes const std::set<LONG>& target history,
not a beam/projectile copy. DeliverEffects 57C7B0 owns the chain pass.
Initial construction is DecodeProjectile case D2 at 5250B0, recovered in
[launch continuation](chain-launch-recovery.md). The prior 70AAA5 pointer
scan was an instruction-byte coincidence, not a vtable assignment. All flight/render/fire slots inherit the already recovered VFX base.

Full bundles/manifests tmp_beam_variant_context.md,
tmp_chain_delivery_context.md, tmp_chain_recovery_context.md; shared header
reused, new class read. Native ctor 57C510..57C652/delivery 57C7B0..57CAB2
inspected. Vtable 34 slots; next starts 850F9C. Tree find 570620/insert 4C4690
compare signed IDs. VS2019/VC6 set-size drift is confined to final member.

Constructor rebuilds CLightT VFX base, deep copies IDs, sets cell sequence 0,
16 headings, TRA_09, doubles SHORT velocity with WORD wrap (20 -> 40), enables
copy-from-back, disables height flags. It does not copy source/caster metadata.

Delivery inserts current target before GetShare. Successful target share gets
its CGameObject area (assert nonnull), releases it, queries nearest with ANYONE,
SHORT 7FFF, PROJECTILE-owner area's visible terrain, flags TRUE/TRUE/TRUE,
BYTE selection mode starting 0, ignoreDead FALSE. Repeated result, invalid ID or
mode >20 terminates via qualified CProjectile::DeliverEffects. Increment
mode before signed-set membership check; first unseen candidate creates
fresh child from current history, then shares it, reads virtual position/id,
releases, and copies ordered payloads. Damage opcode 12 with history size 1
logically shifts native +34 (m_numDice, not flat amount) by one. Low BYTE of
native +C4 (m_casterLevel) <= DWORD cardinality aborts child. Native abort leaks
the unattached copy; preserved, test captures/cleans that pointer explicitly.
Child Fire receives owner area, PARENT projectile ID as source, live candidate
ID/position, parent Z, type 0. Qualified parent delivery runs on all paths.

AutoUI chainchecks appends temporary nodes for two party sprites at one point,
pins visibility, invokes actual GetNearest and real child Fire/registration.
Checks signed deep copy/defaults, missing-target insertion/retained payload,
first-hop dice 9 -> 4 / 80000001 -> 40000000, later-hop unchanged dice/flat
amount, BYTE limits 1/256/258, non-damage bypass, visited target rejection,
owned grouped parent effects and source/target provenance. Retires children
and parent, drains own messages, restores sprite positions/list nodes/flags,
visibility and clock in same tick. No save. Existing beam/weapon tests retained.

First batch 2026-10-13T15-32-06-606884-batch FAIL: 69 steps/57 chain checks
PASS, 60s CLEAN, ctor/delivery hits 20/11. Static ctor heuristic RED (generated
tree copy versus <=12 source lines); native body inspected, documented ctor
semantics only, no behavioral changes. Layout error fixed with inherited-tail
anchor; missing base evidence still fails, packing drift still actionable.
Seven auditor unit tests PASS. Second 15-38-38-006132 FAIL: layout PASS;
mixed fixture called base CGameEffect::Copy's assertion, UI-INCOMPLETE despite
CLEAN guard. Replace test prefix with copiable fixture; cleanup restored hashes.
Final .arc/2026-10-13T15-43-03-139897-batch PASS: Win32 Debug, 69 steps,
58 chain/88 beam checks, 60s CLEAN, ctor/delivery hits 20/11. Layout PASS,
delivery parity GREEN, ctor YELLOW calls 11/2. Missing offset oracles leave
full static equivalence unproven. Cleanup restores hashes. No gates reduced.

The native argument is a selection mode, not an ordinal. The front-list
nonzero-mode rejection is native behavior, not a missing nth-target feature.
FindObjectNear and traversal corrected in [continuation](area-nearest-recovery.md).
Initial factory/flight recovery: [continuation](chain-launch-recovery.md).
