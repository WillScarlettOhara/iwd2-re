# Combat effect controller recovery

## Recovered scope

`CVisualEffect` constructor (554870), destructor (5549D0), empty Render
(554B60), AIUpdate (554B70), Move (554F00), SetAllDelete (555200).
`CVEFVidCell` FrameAdvance (5513F0), CalculatePos (551490), SetDirection
(551520), GetDirection (551670). Complete CGameSprite::HandleEffects (734550).

The controller owns object IDs and event records. HandleEffects marks its
children for deletion before every pass, captures equipped-list retry before
the timed list runs, resets bonuses per pass, folds signed skill bytes and
refreshes the action bar for the picked portrait (including -1 == -1).
Its return value retains the original BOOL bit mask.

## Evidence and important details

Full context bundles: tmp_effect_controller_ctx.md, tmp_visual_lifecycle_ctx.md,
tmp_visual_update_ctx.md, tmp_visual_motion_ctx.md, tmp_visual_direction_ctx.md.
Complete reachable assembly and BG2 class references were read. The native
controller vtable is 84EDFC; its binary size is 2C2. Base layouts are 1C0
(controller), 1E4 (animation), E0 (event), packed to two bytes.

555200 is SetAllDelete, not the nearby source-index projectile function.
The four rotation tables at 84ED40..84EDA0 and frame rate 30 at 84EDF8 were
read from the PE. Move uses signed 32-bit wrapping, fixed-point shift 10,
strict stop distance and sqrt rounding by +0.5 (omitted by the decompiler).
Negative Y conversion divides before shifting. Child direction precedence,
SHORT sign extension and modulo cadence follow assembly.

AIUpdate retains a native quirk: the flagged-delete branch at 554C42 skips
updating the saved removal POSITION. Consecutive flagged children can therefore
reuse a stale node position. The regression fixture avoids that unsafe native
case; this recovery does not claim to repair it.

Only the CVEFVidCell prefix through newDirection is recovered. Its full
constructor/destructor, resource loading, renderer, sound and remaining tail
are still missing. Explicit AutoUI fixtures provide child setup and retirement;
they do not substitute for the production resource loader.

## Validation

The combat-messages profile runs effectcontrollerchecks in the loaded world,
restoring stats ownership, actor/effect lists, action bar and temporary object
registrations in the same tick. It covers every valid cadence with positive
and negative counters, duration/end/loop gates, all rotations, direction
precedence, movement rounding/wrapping/decay, controller deletion and repeated
HandleEffects passes. The profile also retains actual melee, bow, crossbow,
sling, on-hit and network/projectile damage checks. No save is written.

Static batch: eight GREEN parity results; SetAllDelete and SetDirection are
YELLOW solely for source/binary call-count differences (6 vs 2). Class audits
are clean. Compiled member spacing passes for 12 controller, 6 animation-prefix
and 310 sprite members. The offset oracle cannot analyse the new classes with
its current header parser; these layout checks and fixture assertions cover a
different question and do not replace that oracle or differential evidence.

Run `.arc/2026-10-11T11-07-43-341505-batch`: Win32 Debug build PASS;
50-step UI scenario PASS, 133 controller assertions PASS, 135 locator assertions
PASS. Actual target HP loss: melee 6, bow/crossbow/sling 12 each; received effect
3, grouped effects 10, four opcode-430 launches 3 each. Every target executed.
60-second crash guard CLEAN; profile cleanup PASS. No matched differential
capture exists for this lot.

## Next recovery

Identify and recover the VEF resource wrapper and GetVEF (5545E0), then the
controller event/child factory at 554D20. The latter currently has an unrelated
source marker naming a message constructor; verify and repair that identity
before implementing it. Full animation loading is required to exercise these
children outside the explicit test fixture.
