# Exploding sparkle and impact BAM

New CProjectileSparkleExplosion (#guess): ctor 5297B0, Explode 52E160,
vtable 84D938, native size 4B2. No added fields; inherits exploding flight,
render and strike passes. Separate from CProjectileExplodingWeapon/Flame.
Explode slot 34 maps signed color 1/9/10 -> C4, 2 -> C5, 3/11 -> C8,
4 -> C6, 5 -> C7, 6 -> C9, 7/8 -> CA, 12 -> CB. Other WORD colors return.
Decode(NULL caster, delay 0), virtual child Fire(owner area, source/target IDs,
recorded target point, height 30, original color). No payload copying here.

Factory 95..9D/B6/B7 builds TRAVEL, trail/height, flags OR 8. Exact
color/strike-child/palette/sound descriptors: SparkleExplosionGolden.h.

New distinct CProjectileImpactBAM (#guess), native size 32C, vtable 84E9C0:
535F20 ctor, 536380 AI, 5363F0 Render, 5366F0 Fire. Not CProjectileBAM 57CAC0:
native vtables/fields/lifetime differ. Pack 2, cell 192, bitmap 26C, DWORD
render flags 326, palette byte 32A, translucency byte 32B. Factory C4..CB
builds SHAREA, empty sound, flags 8, translucency 0, optional SHAREA1..6
palette. C4/C6 retain cell palette. Fire/arrival sound overrides remain empty.

Ctor seeds resources, palette, sequence/frame, sound channel 14 and height.
AI gates Time Stop by own object ID, tests end before advancing,
calls virtual arrival when ended, otherwise advances and sets sound coords.
Fire records IDs/area and delivers effects BEFORE object registration; failed
Add deletes self. Live target GetStart(FALSE) overrides supplied point; sound
plays at target before Y+1, then base AddToArea(front) with supplied height.
Missing live target leaves native local point uninitialized; not invented.

Render: owner area, signed-WORD visibility, height minus Z, viewport rejection.
Nonzero flags persist OR 80, zero stays zero. Balanced palette demand,
BYTE translucency, clipping Z0; prep/lock/unlock/blit use resulting flags.

Full bundles tmp_sparkle_explosion_named_context.md/tmp_impact_bam_context.md;
unchanged header reused, new declarations read. Bodies/factory/vtables checked
in assembly. Missing exports created from addressed stubs: 4 created, unrelated
443320 creation error. All stubs replaced.

sparkle_explosion_reference.py PASS37, canonical MD5. Native factory field
writes/common tail and Explode run; constructors/resources/allocation/child
Fire are isolated. Captures 19 descriptors, 17 signed colors plus null child.
Original suspended/killed, hooks restored, stable hashes, clean cleanup.

AutoUI sparkleexplosionchecks tests all factories, registered colored child
source/target/type and live placement, unsupported colors, constructor state,
Time Stop/end-before-advance, 24 complete pixel comparisons (eight BAMs with
normal/hidden/offscreen states), zero/high flags/translucency and resource
balance. nearestchecks adds immediate real six-HP BAM payload and actual
animation expiry without a repeated hit. Same-tick full restoration; no save.
First 20-26-33-834391 batch FAIL: two CString lints (fixed), UI abort after
BAM's correct six-HP hit/expiry because fixture left its empty owned group
queued. Common DeliverEffects emits that group even with no payload; fixture
now verifies ownership/count zero and consumes it. No gameplay workaround.
Final .arc/2026-10-13T20-34-12-919989-batch PASS: Win32 Debug, 71 steps,
182 impact checks/33 renders, 86 nearest checks, seven symbols hit, 60s CLEAN.
Six-HP BAM hit, expiry after six updates, no repeat; chain/beam/weapon tests
pass. Hash cleanup restored. Impact member spacing PASS; fieldless leaf skip.
Explode/AI/Fire parity GREEN; ctor/render/factory YELLOW. Missing offset
oracles leave static equivalence unproven; no native rasterization claim.
Next: identify travelling leaf 532260 and factory C0..C2.
