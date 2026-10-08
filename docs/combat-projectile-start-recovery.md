# Projectile launch origin

Recover CProjectile::GetStart (0x52A640), replacing the simplified file-local
GetProjectileSourcePosition helper. Name and static void signature are
confirmed by BG2 PDB GetStart(long,CPoint&,unsigned char). Complete recovery
bundle: tmp_projectile_source_ctx.md. Read projectile, door, container and
trigger declarations; PE verifies type dispatch, member widths, facing flag,
16-bit arithmetic and a single GetShare attempt.

Ordinary object kinds use virtual GetPos. Containers and triggers use their
CPoint trap origin, while doors zero-extend the two WORD trap coordinates.
Unknown kinds assert. Only nonzero facing flags on sprites add animation
ellipse offsets. Facing pairs 0/1, 4/5, 8/9, 12/13 use cardinal offsets;
the other four pairs use the diagonal square root. The diagonal squares are
signed SHORT products, the division is integer, and the square root result
is truncated to SHORT. Preserve the binary's lack of a zero-denominator
guard. Missing objects leave the caller's point unchanged. All successful
shares are released. Existing projectile callers now use the real helper;
the launch source existence check precedes GetStart in Travelling::Fire.

65 typed cases: all 16 directions with flag 0, 1 and 255; missing source with
sentinel coordinates; and all 16 directions with a temporary 256-by-12
ellipse. The wide case has a squared horizontal radius that truncates to
zero, so all diagonal offsets must be zero. Cardinal expectations remain
explicit constants. Direction and ellipse are restored inside the same
AutoUI step. Existing launch and combat assertions remain. Trap-origin
branches and unsupported-kind assertions are verified statically, without
constructing fake derived objects in the runtime fixture.

No visual differential equivalence is claimed. Follow-up: trailing projectile
object construction and actual weapon flight regressions.
Validation: full batch 2026-10-08T21-59-42-475494-batch PASS with one
warning. Eight shared lints, class audits and Win32 Debug build pass.
All 557 typed assertions and 11 scenario steps pass. GetStart executes 77
times and Travelling::Fire 14 times, followed by 60 seconds CLEAN. GetStart
parity is YELLOW (55 binary calls, 24 source calls; repeated ellipse reads
are consolidated), Fire parity GREEN. Offset analysis is not analysable;
layout evidence remains incomplete/skipped. Fixtures removed and preserved
paths restored and hashed by the profile.
