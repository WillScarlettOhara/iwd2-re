# ProcessAI movement scale override

Recovered 0x72BBC3..0x72C0CF inside ProcessAI (0x72B9A0), using the
shared full context bundle and original instruction reads. Game save mode
0x142 saves the animation's current BYTE movement scale in field_72A8,
latches field_72AA and imposes scale 6. A latched sprite whose scale drifts
is reset to 6. Leaving the mode clears the latch, restores the saved scale
and sets field_562C to 1.

The delta is recalculated only when the replaced/restored scale differs
from 6. The original signed integer metric is dx*dx + dy*dy*16/9;
sqrt is rounded with +0.5 before conversion to integer. The decompile
omits that addition; PE instructions and constant 0x848DD0 confirm it
in all three branches. Zero distance clears both delta components.
Otherwise each component reads the current animation scale separately;
the vertical numerator applies dy*4/3 before the EXACT_SCALE shift.
No new clamping or overflow correction is introduced.

Nine typed AutoUI cases extend tickcounters: initial override, initial
scale already six, latched six, latched drift, zero destination, restoration
to four, restoration to six with unchanged delta, negative displacement
and integer vertical scaling at (1,1). They call actual ProcessAI with
world time stopped and WAIT current action. Mode, animation scale, latch,
saved scale, destination, delta and dirty field are restored synchronously.
No party position or save is written. The profile centers the camera.

Validation: .arc/2026-10-04T21-57-10-214053-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 82 case
assertions and eleven UI steps pass, followed by CLEAN observation for
60 seconds. ProcessAI hits 93429 times and ResolveInstants 93282.
Save/config paths are restored and hashed. Parity remains YELLOW (191/238
and 17/35 original/source calls); offset analysis is unavailable for both.
Full ProcessAI recovery and original/recovered differential equivalence
remain unproven.

Next target: missing network ownership return before dialog/action processing.
