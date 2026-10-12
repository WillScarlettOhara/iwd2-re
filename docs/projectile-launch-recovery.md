# Travelling launch and sparkle expiry

Targets: CProjectileTravelling::Fire 52C050,
IcewindCProjectileTravellingVFX::Fire 5791D0 and
CProjectile::GetDirection 52A480.

Evidence: tmp_sparkle_launch_ctx.md and tmp_projectile_direction_ctx.md,
their provenance manifests, complete shared CProjectile.h, BG2 names for
the direction helper, required calls and full reachable native disassembly.
Factory-2F jump entry 528F34 -> 520288, its common tail 528E1C and the
Sparkle constructor/vtable/AI confirm that no launch stage arms its timer.

Both launches now wrap the weighted LONG distance at native multiplication,
division and addition boundaries. An armed timer rounds sqrt(distance) by
adding 0.5, narrows the result to signed SHORT, then divides by signed speed
and adds one. Unarmed timers retain the leaf default, including zero.
The origin accumulators wrap before Y's signed division by three; direction
scaling wraps before division too. Target coordinates are assigned after
registration, area insertion and launch sound, as in the original.

The VFX launch now uses the actual animation rectangle's half-height,
correct default rectangle/reference point, one target/source share attempt,
SHARED-only target deny retry, the pointer returned by that deny, checked
registration with owned deletion on failure, qualified CGameObject insertion
and the projectile's launch-loop flag. Both families add at cast height;
previous comments claiming a ground-level base launch were incorrect.

scripts/probes/sparkle_launch_reference.py loads exact save
000000037-testSpells in the canonical original (MD5
25cb3d8aa2ec648e94e328d8c467973e). It uses the existing real-click load driver,
resolves ANSI save names in the engine's own array, invokes the real factory
and virtual Fire on the world thread, then observes ordinary area ticks.
No effects, equipment changes or save writes. It kills only its own process;
the profile journal verifies restoration of save/config/temp paths.

Original run .arc/sparkle-launch-original-20261012-r6: 12 checks PASS,
six travelling AI hits, CLEAN, restored=true. Factory-2F's velocity is 40,
distanceLifetime=0 and lifetime=0 at factory return, after Fire and at first
AI. A far point expires with lifetime=-1 before OnArrival. Near-point and
self-target launches reach OnArrival with lifetime=0, then delete directly;
arrival guards precede the timer. Arrow control starts at 32767, moves on
two ticks and arrives. This confirms native early expiry, not a missing
timer assignment to repair. Earlier observer failures remain archived
and do not count as successful traces. This is a targeted original runtime
observation, not full original/reconstruction differential or visual parity.

AutoUI projectilelaunchchecks invokes both production Fire methods through
registered typed fixtures. It covers independent numeric boundary cases,
live-target positions, half-height, qualified area insertion and registration
order, near snapping, signed origins, absent source/target, forced Add
NO_MEMORY and loop/channel state. Two Sparkle virtual-AI fixtures check the
arrival/zero-timer precedence. Objects and queues retire before another tick;
actor position, ID limit, Time Stop, messages and world clock are restored.

Validation: .arc/2026-10-12T14-04-23-297898-batch PASS with one warning.
Win32 Debug build, 172 new checks/62 UI steps, 60 s CLEAN; hits:
base Fire 61, VFX Fire 29, direction 354. Melee loses 6 HP, bow/crossbow/sling
12 each, trail arrow 3/source 0; clusters retire. Save/config/resources restored,
cleanup errors empty. Parity 2 GREEN/1 YELLOW (VFX calls 35 native/49 source).
Offset oracles unavailable and layout review gaps remain: absolute layout and
pixel equivalence are unproven. First batch failed on Windows' near macro;
renamed the fixture variable and reran the full profile without reduced gates.

Next: recover VFX AimAtPoint 578ED0, which still delegates to the base integrator.
