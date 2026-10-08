# Travelling projectile initial height

Recover the target bounds and height initialization in
CProjectileTravelling::Fire (0x52C050), using the complete
tmp_travel_fire_ctx.md bundle from the preceding launch arc. The binary,
shared header/layout and decompile are unchanged. PE checks:
0x52C0BA-0x52C1F2 (target bounds), 0x52C244-0x52C274 (source failure),
0x52C59F onward (distance), 0x52C60F-0x52C657 (height initialization).

Target GetShare is one attempt; failure returns. A sprite target obtains
GetDeny, retrying SHARED only, reads CalculateFxRect at its current posZ,
and releases deny before share. Deny failure retains the default rectangle
(0,0,64,64) and reference (32,50). Source GetShare is also one attempt;
failure returns before resolving the launch origin or registering.

After calculating flight distance and optional lifetime, sprite targets seed
both m_nDeltaZ and m_nDeltaZLast with the signed SHORT launch height minus
half the rectangle height. The PE uses rectangle bottom minus top, followed
by SAR 1; this is not bottom minus the reference point. Non-sprite or point
targets set both deltas to zero and overwrite flight distance with 1. Lifetime
must already have been calculated from the actual distance.

Ten typed heap projectile cases poison both height terms and distance before
calling Fire: six self-target creature cases cross target posZ -13/0/19
with height disabled/enabled; two point cases cover near and distant ground;
two missing-object cases cover target and source failure. Tests check both
height terms, position Z, target point, distance, lifetime and registration.
Expected animation bounds come from the loaded actor's actual animation.
The actor posZ is restored before leaving the step. All registered fixtures
are removed immediately and unregistered fixtures deleted. Save files are
not written. Contention and deny-failure defaults are established statically,
not forced by these runtime cases.

Trailing-object construction remains explicitly unrecovered. No complete
visual/flight equivalence is claimed.

Validation: full batch 2026-10-08T14-23-31-055124-batch PASS. Eight
shared lints, class audits and Win32 Debug build pass. All 492 typed
assertions across 11 steps pass, with 14 Fire entries and 60 seconds CLEAN.
Parity is GREEN. Offset analysis is not analysable and layout is skipped;
these remain evidence gaps. Fixtures are removed and preserved paths
restored and hashed by the profile.
