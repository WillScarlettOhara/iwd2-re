# Travelling VFX rendering

Refined IcewindCProjectileTravellingVFX::Render 578480: IMUL CX / MOVSX DX,CX
truncate visibility indices to signed WORD; LONG wrongly rejects wrapped rows.
Render/bounds 578970/sequence 579860 markers now adjoin their bodies. A false
bare 578480 header marker previously overwrote the map with CProjectile::Render;
ordinary prose fixes it without changing the spell-hit forwarding slot.

Evidence: full tmp_vfx_render_context.md and tmp_vfx_bounds_direction_context.md
with manifests/shared CProjectile.h; complete native bodies 578480..57895F,
578970..578AA1 and 579860..579A0D.

VFX flags/transparency come directly from m_visualEffect, with no virtual
GetRenderFlags call or travelling mirror/tint input. Visible/mobile-cost gates
precede sequence updates. Shadow adds bit 4; Z is subtracted from position
without a shadow, otherwise carried into the main-cell anchor and negated in
the W..E window. The shadow reference flips in that window independently of
the actual blit mask. East == 1 ORs horizontal bit 10; north == 1 XORs that SAME
bit, never bit 20. Tint/copy == 1 fetch area tint, but only tint == 1 assigns it
to the main cell. The shadow keeps its tint. Only FXPrep adds COPYFROMBACK.
Palette swap demands the travel bitmap, assigns its palette and releases it.

Direction helper: north NONZERO folds 16 facings; counts 8/2 use other tables.
Unsupported counts still update the main-cell owner facing. Count 1 or unchanged
owner facing returns even for an alternate cell. Selection preserves the frame.
Bounds union the Z-lifted chosen cell and ground shadow, without changing owner.

AutoUI vfxrenderchecks uses explicit mirror/sequence outcome tables, independent
rectangle unions and complete software back-patch comparisons. It exercises
512 mask/direction cases, signed shadow height, exact tint/copy bytes,
transparency, palette swap, guards, wrapped rows, sequences and ownership. Symmetric
BAM/header fixtures restore full records and demands under service locks.
Camera/WED, map bytes, pixels, queues and clock are restored before ticking.

Full batch .arc/2026-10-13T08-30-20-085521-batch: PASS with 3 warnings.
Win32 Debug, 66 UI steps, 3593 VFX checks plus the previous 208 explosion
checks, 60-second hold CLEAN. PDB hits Render=588 / bounds=1180 / sequence=672.
Melee loses 6 HP; bow/crossbow/sling lose 12 each and consume ammunition.
Payloads lose 3 target HP / zero source HP; particles retire. Cleanup restored
all hashes with no errors. Parity bounds GREEN, Render/sequence YELLOW calls
22/27 and 12/21; operand-count warnings and unavailable offset oracles leave
full static proof unproven.

First run
.arc/2026-10-13T08-06-13-289310-batch remains FAIL: stale map caused a false
source-missing parity RED; an older fake-network fixture let the already-active
message thread fetch through NULL DirectPlay at step 8, before VFX checks.
The native 424180 thread loop checks local player zero only at its INITIAL
wait, so zero alone cannot isolate an already-active loop. AutoUI now holds
both transport locks and pauses that thread around all three simulated network
scopes, resumes after restoration and asserts the original suspension count.
No production transport behavior changed. Cleanup succeeded. 08-17-17 remains
FAIL (missing CUtil include, no runtime); 08-23-00 remains ERROR (seven test
assertions expected absent BAM sequences to survive GetFrame normalization).
The reference now records the terminal draw sequence; raw helper selection is
still checked independently. Both failures were fixed without reducing gates.

Pixels are bounded regression evidence, not an original/reconstruction pixel
capture. Accelerated rendering, actual wall occlusion, forced lock failure and
nonzero terrain-height pixels remain outside this fixture's proof.

Next: recover missing Render 57B780 of the beam leaf (ctor 57AEB0, vtable
850E84 slot 19); its identity/layout need recovery before source declaration.
