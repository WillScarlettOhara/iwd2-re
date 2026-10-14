# Thrown weapons and projectile immunity

Four fieldless Travelling leaves (#guess class names), native size 2A0:
BOULDER ctor 531570/vtable 84E1D4/speed 30; AXE 531BE0/84E36C/40;
SPEAR 531DF0/84E3F4/40; DAGGER 532000/84E47C/40.
All 34-slot tables inherit gameplay/render virtuals, own dtor deferred.
Seq0, tint on, height offset/mirror/shadow off, directions16, empty fire
sound, callback/attached -1, empty trailing CString. Boulder signed SHORT
speed*3/2 truncates toward zero then narrows; others double/narrow WORD.
Dagger colors range5=27, then range4=30, BEFORE binding the cell palette.
Other three retain the BAM resource palette.

Factory AXE 7/A/B, DAGGER 1B/1E/1F, SPEAR 38/3B/3C, BOULDER 177:
height true, fire/arrival empty, common type-1 stamp. Table 528E7C indexed
by type-1 verified; BOULDER's old 51F2E4 handler is reused by type177,
not type16 (that is an existing cone). Body cases and allocations checked.

4E7120 identified as existing CImmunitiesProjectile::OnList; body already
matched, added address marker. Native head +4, next +0, node data +8,
full DWORD equality without dereferencing stored values. Null data matches
query0; high WORD matters. CProjectile's spell-level gate reads low BYTE,
only indices0..8; projectile immunity takes precedence. No invented rule.

Full bundles tmp_weapon_leaves_context.md, tmp_magic_leaves_context.md,
tmp_impact_helper_context.md. Shared unchanged Travelling header/bodies and
factory reused from prior arc; new declarations and CImmunities classes
read separately. Ctors/helper/factory/vtables checked against disassembly.
SPBRNHND 532CA0 was inspected but not implemented: distinct field/AI/Fire.

thrown_weapon_reference.py PASS36, tmp_thrown_weapon_reference_final:
four native constructors, twenty type/delay descriptors, twelve raw DWORD
list lookups. Uses acid probe's isolated base/resources; real native palette
loop/copy, factory and leaf writes execute. Capture hashes include that
dependency. Canonical MD5, original suspended/killed, globals/hooks restored.
ThrownWeaponGolden.h generated from captured values. No native flight or
rasterization claim.

Typed thrownweaponchecks replays list order/membership without mutation,
every factory class/animation/speed/flag, dagger binding/metadata, normal
visible pixels plus hidden/offscreen full surface comparisons. Accesses real
objects with typed member pointers, no offset reads or test-subclass cast.
Frame anchor acquired after resource construction; manual rendering seeds
both area pointers and uses Aim south to initialize the unwritten facing.
Restores master/resource demands, camera/WED, visibility/search byte/pixels.
nearestchecks now launches ten new types plus four acid variants, real
six-HP payload at arrival only, Time Stop and automatic retirement. Tests
full-DWORD immunity, discarded blocked payload (no queued hit), spell levels
0/8/9 and dirty high words; owns one group and the defender damage bark.
Full actor/stats/queues/private area/clock restored same tick. No save.

Final .arc/2026-10-14T08-02-36-394832-batch PASS (5 warnings): Win32 Debug,
73 steps, 282 weapon checks/60 renders, 374 nearest checks, nine symbols
hit, 60s CLEAN, cleanup hashes restored. Every new type lost six HP after
three updates and retired; blocked payloads caused no queued hit/HP loss.
Melee/bow/crossbow/sling 6/12/12/12 plus ammo/acid/chain/beam tests pass.
Dagger/OnList/Fire/Deliver parity GREEN; three other ctors/factory/AI
YELLOW call counts. Missing offset oracles and fieldless layout skips leave
full static equivalence unproven; source pixel tests are not native raster.
Continuation: [SPBRNHND spray recovery](spray-recovery.md), including
its separate visual cells, recursive placement and shared lifetime.
