# Explosive projectile rendering

Recovered CProjectileExploding::Render 52CF80 (vtable 84DBDC, slot 19).
Refined CProjectileTravelling::Render 52B190's signed-WORD visibility index.
Explosive flame/weapon and Skull Trap inherit this override.

Evidence: full context/manifest tmp_exploding_render_context.md, shared header,
native disassembly 52CF80..52D421 and 52B190..52B6A2. Ghidra now exports the
previously missing 52CF80. BG2 supplies names, not the renderer/layout.

The renderer calls qualified travelling Render first. A
nonzero linger state AND active first explosion cell enable the second pass;
an active second cell alone does nothing. The second pass independently calls
virtual GetRenderFlags, uses the OWNER area, sign-extends the low WORD tile
index and checks mobile cost with personal space three / bump checks enabled.

It deliberately measures the TRAVEL cell with GetCellBounds(..., NULL), whose
shadow/mirror bounds remain those of the travelling fields. Explosion cell 2
controls bit 4 and the explosion shadow draw, independently of the travel
shadow flag. Mirror bit 200 and tint bit 10000 are preserved. With no second
explosion cell, the non-mirrored OR accelerated path subtracts Z from position;
otherwise the draw carries a separate Z offset. Height offset is optional.
Signed facing thresholds N=8, W=4, E=12 mirror the bounds/reference and blit.
Only FXPrep adds COPYFROMBACK; lock/render/clip/unlock/blit keep the other flags.
Tint goes to cell 1 only; cell 2 draws first with transparency zero, then cell 1
at reference Y minus signed Z with transparency 128. Lock failure returns.

AutoUI explodingrenderchecks compares complete software back-surface patches
against explicit-anchor BAM paints, with distinct owned palettes and tints.
The test pins the BAM, temporarily centers its first frame and cached header,
and restores both records and demand counts under resource-service locks.
Both draw anchors must fit the measured FX bounds before reference painting.
It covers flight-first/inactive cells, sixteen facing flags, software mirror
and second-cell combinations, signed Z, tint preservation, invisible/blocked/
off-viewport rejection and +/-65536-row tile wrapping in BOTH renderers.
All 47 cases reached the virtual explosive renderer. Wall polygons are
isolated with a temporary NULL WED; their production clipping call still runs.
Frames, locks, WED, camera, visibility/search bytes, sampled pixels, queues,
messages and world clock are checked/restored before another ordinary tick.

Full arc batch .arc/2026-10-12T23-09-37-644186-batch: PASS with 2 warnings.
Win32 Debug build, all 65 UI steps, 208 new renderer checks, 60-second hold
CLEAN; PDB hits explosive=47 / travelling=76. Melee loses 6 HP; bow/crossbow/
sling lose 12 each and consume ammunition. Projectile/trail payloads lose
3 target HP / zero source HP; particles retire. Cleanup restored paths/hashes,
no errors. Parity YELLOW calls 16/26 and 23/28; operand-count warnings remain.
Offset oracle skips both classes
because their declarations share CProjectile.h; explosive layout passes
13 members, travelling has no own annotations. Full static proof is unproven.

Earlier 22-40-44/22-46-58/22-54-32/23-02-38 runs remain FAIL: nonexistent
ARBOLT, then invalid synthetic arrow anchors (18-pixel frame, center Y=17)
overran the reference FX surface. Only fixtures changed to fix this.
23-06-46 remains ERROR: transient transaction rename denial, restoration
succeeded. Inline guess annotations moved after semicolons; the travelling
address marker now adjoins its signature so both symbol gates resolve.

Pixel comparisons are bounded software regression evidence, not a captured
original/reconstruction pixel differential. Accelerated rendering, actual wall
occlusion and forced FXLock failure remain unproven by these fixtures.

Next: travelling VFX rendering is refined and tested; see vfx-render-recovery.md
for its independent masks, bounds, sequences and the next recovery target.
