# Segmented beam recovery

Recovered ctor 57AEB0, static offset initializer 57BBC0 and Render 57B780 of
vtable 850E84. IcewindCProjectileScorcher is a provisional IWD2 name; BG2's
CProjectileScorcher only supplies role names, not inheritance or offsets.
Native base IcewindCProjectileTravellingVFX, size 2D6: lifespan
BYTE 2AE, segments BYTE 2AF, delta-Z LONG 2B0, glow BYTE 2B4, struck-target
set 2B6, tick/strike/clear clocks 2C6/2CA/2CE and initial target 2D2.
Pack(2); the VS2019 set differs by 4 bytes. Named accesses do not prove
absolute native layout equivalence.

Evidence: tmp_beam_full_context.md/manifest/shared CProjectile.h, constructor,
helper/vector bundles, complete native bodies, constants and BG2 layout.
Ghidra created the previously undefined renderer, then re-exported it.

Ctor uses AScorcT, frame zero, sixteen directions, TRA_M04, frame-height
times 3/4 narrowed to SHORT velocity, invalid callbacks, lifespan 110,
zero segments/delta/glow/tick/initial target and strike/clear periods 3/60.
The static vector grows/truncates to sixteen; only flag == 1 skips setup.
Preserve its unusual negative X offsets for directions 0 and 13..15.

Render ignores the argument area, using the projectile owner. Flags begin
200, OR 10 beyond north and 20 strictly between west/east. The main cell
alone is measured and drawn, with unchanged reference and zero transparency.
No inherited visible/search/tint/palette/shadow/sequence gate applies.
Height is always queried at m_pos; initial anchor is (x,y-Z+height).
Visibility uses the height-adjusted anchor and signed WORD tile wrapping.
Rejected visibility or viewport intersections increment the BYTE index but
freeze the anchor. Accepted segments OR COPYFROMBACK for every FX call;
lock failure still advances the anchor. Next X wraps step*index+origin before
SAR 10. Y wraps multiplication by three, divides by four toward zero, then
SAR 10; subtract Z, add original-position terrain height and clamp the slope
to index. Negative INT_MIN delta negates with DWORD wrapping. Render changes
no projectile motion/frame/tint state and the viewport remains constant.

Native reference: scripts/probes/beam_render_reference.py executes 57B780 in a
suspended process with private buffers and replaced height/cell/visibility/FX
collaborators. tmp_beam_native_reference/report.json
PASS: 49 cases, MD5 25cb3d8aa2ec648e94e328d8c467973e, process killed.
Captures cover directions, 0..255 counts, fractional/overflow arithmetic,
signed heights, hidden tiles, viewport rejection and failed FXLock.
BeamRenderGolden.h retains 43 cases compatible with the real software fixture.
This is native control-flow/anchor evidence, not native rasterization.

AutoUI beamrenderchecks compares complete pixels to explicit samples and
native captured anchors/masks, checks ctor defaults/vector branches, and
restores table state, complete BAM/header frames and demands under service
locks, visibility, camera/WED, pixels, queues and clock in the same tick.

Full batch .arc/2026-10-13T09-35-22-706073-batch: PASS with 3 warnings.
Win32 Debug, 67 UI steps, 261 beam checks, 83 virtual renders, 60-second
hold CLEAN. PDB ctor/init/render hits 1/5/83. Melee loses 6 HP;
bow/crossbow/sling lose 12 each and one ammo charge. Payloads lose 3 target
HP / zero source HP; particles retire. Cleanup restores all hashes, no errors.
Parity YELLOW calls 15/23, 12/3, 5/24; operand-count warning and unavailable
offset oracles leave full static equivalence unproven. Initial run
.arc/2026-10-13T09-28-26-937211-batch remains FAIL: missing address marker
on the explicit deferred AI stub. Build/game passed (40 renders, 131 checks,
67 UI steps, CLEAN); marker fixed before final rerun. No gates reduced.

Next: recover AIUpdate 57B210, Fire 57B1D0 and strike services 57BE80/57C1D0
before factory integration. AI/Fire are explicit stubs; this lot does not yet
enable beam damage in ordinary gameplay. Software walls/nonzero terrain,
accelerated pixels and forced lock failure in our build remain unproven.
