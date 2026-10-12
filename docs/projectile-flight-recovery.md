# Travelling flight recovery

Targets: CProjectileTravelling::AimAtPoint 52BD20 / AIUpdate 52B900 and
IcewindCProjectileTravellingVFX::AimAtPoint 578ED0 / AIUpdate 578AB0.
Both VFX delegates are now replaced with their own recovered native bodies.

Evidence: tmp_{travel,vfx}_{aim,ai}_context.md and provenance manifests,
unchanged shared CProjectile.h and complete native assembly. Aim calls
CGameSprite::GetDirection in the older family, CProjectile::GetDirection in
VFX; both call sites are preserved.

Aim uses wrapped LONG Y*4 before signed division by three, wrapped distance
squares, sqrt+0.5, signed SHORT velocity and wrapped fixed-point numerators.
__ftol converts to a QWORD and the caller takes its low DWORD: a negative
wrapped distance yields NaN then ZERO, not the INT_MIN of a direct int cast.
Zero distance yields steps (1,1), retaining the old target and skipping
drift/jitter contribution and random draws. Otherwise it updates the target,
adds drift, draws X then Y only for positive wrapped spread widths, advances
both accumulators and decays each drift by the unsigned WORD decrement.
Y reconstruction wraps multiplication by three before truncating division
by four and arithmetic shift ten. The associated object receives only Z;
GetDeny retries SHARED and DENIED, then releases only a successful acquisition.

AI advances the BAM before signed-cell/inclusive-radius/zero-metric arrival
guards and wrapped SHORT countdown; only an old zero retires immediately.
Time Stop compares projectile ID. Homing takes one share, checks stored area,
snapshots virtual GetPos and interpolates height from the PREVIOUS target
distance before re-aiming. Height product/Z addition wrap. Sprite drift uses
the animation rectangle and restores old Z after its temporary step; its cast
uses the pointer returned by GetDeny. Trails precede sound-coordinate update.

scripts/probes/projectile_aim_reference.py calls the numeric branch in a
suspended original with attached ID -1; no engine/constructor/vtable/area/save
access. Private CRT-rand TLS is restored before killing only the spawned PID.
Native offsets are never used on Debug classes.
Run .arc/projectile-aim-original-20261012-r3: PASS, canonical MD5
25cb3d8aa2ec648e94e328d8c467973e; 124 inputs / 204 states per method,
both methods agree, cleanup errors empty. src/ProjectileAimGolden.h contains
captured states/next rand returns; repeated generation matches exactly.

AutoUI projectileaimchecks replays each state through both typed virtual Aims,
including three-step trajectories/random consumption, registered VEF Z and
absent/stale IDs. projectiletickchecks exercises both virtual AI bodies with
intercepted frame/arrival/removal, real aiming/live targets, numeric boundaries,
Time Stop, sound and fourteen-particle trails. Objects retire before another
tick; actors/queues/messages/clock restore. Test RNG is reseeded to one.

Validation: .arc/2026-10-12T22-22-18-133457-batch PASS with four warnings.
Win32 Debug build, 3119 motion + 64 tick checks, all 64 UI steps and 60 s CLEAN.
Hits: base Aim 235 / VFX Aim 221 / base AI 62 / VFX AI 23. Melee loses 6 HP;
bow/crossbow/sling lose 12 each; trail arrow loses 3/source 0; clusters retire.
Save/config/resources restored=true, cleanup errors empty. Parity 1 GREEN /
3 YELLOW (VFX Aim calls 7 native/11 source; both AIs 22/31). Aim operand-count
warnings and unavailable offset oracles keep static evidence unproven;
VFX own-member spacing passes, base layout audit skips. Numeric golden replay
is bounded behavioral evidence; full AI/gameplay differential and pixels remain
unproven. Earlier journal replacement and NaN mismatch runs remain ERROR,
with successful restoration; no reduced gates were used to obtain this pass.

Next: the CProjectileExploding::Render override at 52CF80 is now recovered;
see projectile-explosion-render-recovery.md for its validation and next target.
