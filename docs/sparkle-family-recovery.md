# Sparkle trails in explosive projectiles

Targets: CProjectileExploding::AIUpdate 52DD60,
CProjectileExplodingFlame::AIUpdate 52EC80,
CProjectileExplodingWeapon::AIUpdate 52E4D0,
CProjectileSkullTrap::AIUpdate 52F9E0 and
IcewindCProjectileSpellHit::AIUpdate 56FAF0.

Evidence: tmp_sparkle_families_ctx.md and its provenance manifest,
complete shared CProjectile.h block, required calls and full reachable
PE disassembly of all five functions. The existing flying/linger bodies
were checked against that evidence before replacing their omitted trail
allocations. Pack(2) now covers the exploding base and its three leaves:
the compiler audit found a two-byte drift before m_bCheckNonSprites and
a padded Skull Trap CString. No factory behavior is changed.

Each surviving flight tick with a nonzero trail flag constructs a
CSparkleCluster(color, TYPE_TRAVEL, DEFAULT_CONJURE_DURATION) and adds it to
LIST_FRONT at the projectile's position/height AFTER AimAtPoint and BEFORE
updating the sound coordinates. Flame and weapon variants keep their three
CGameTemporal puffs before the independent sparkle spawn. The flame follows
a live target; the weapon/base/trap/spell-hit branches use the saved point.
Missing or other-area flame targets remove the projectile before spawning.

The distance comparison retains native DWORD wrap at subtraction,
square/weighted multiplication and addition, then signed division/comparison.
The flame-puff scaled steps likewise wrap before signed division by three.
Spell-hit strike/lifetime counters wrap as LONGs before checking expiry.
Arrival, Time Stop, detonation, strike and removal branches retain their
separate native behavior: the base reloads seven on an empty pre-scan, the
flame/weapon reload then decrement to six, and the trap stays at zero.

AutoUI sparklefamilychecks dispatches all five actual AIUpdate functions
using registered typed fixtures. Only motion/arrival/strike callbacks are
intercepted to keep diagnostic side effects local. Real cluster/puff
construction, area queues and empty AreaEffect scans execute. It checks
post-motion spawn positions, particle counts/type/duration, flame jitter
and signed height, simultaneous puff/sparkle allocation, disabled trails,
Time Stop and owner exemption, radius equality/negative velocity, wrapped
X/Y distances, live/missing/other-area targets, SHORT/LONG countdown wraps,
no-cell snap, gather/strike ordering and final-tick removal. Temporary
objects, queues, Time Stop and list cursor are restored before the next tick;
the world clock and message count must be unchanged.

Run the complete combat-messages profile, including the real melee, bow,
crossbow, sling and trail-arrow damage checks. The scenario centers on the
party and never saves. Static parity and diagnostic fixtures do not establish
original/reconstruction differential or pixel equivalence. The original
factory-2F trace now confirms zero-timer far expiry and near arrival; see
[projectile-launch-recovery.md](projectile-launch-recovery.md).

Validation: the first batch failed compilation on a misnamed type constant;
the second built and passed 176 new checks/61 UI steps, all five symbol gates
and a CLEAN 60-second observation, but FAILED the exploding layout audit.
Final batch .arc/2026-10-12T12-52-23-680699-batch PASS with one call-count
warning: Win32 Debug build, all five layout audits, 176 new checks/61 UI
steps and five symbols exercised during 60 s CLEAN. Hits: base 11, flame 14,
weapon 11, trap 11, spell-hit 10. Melee loses 6 HP; arrow/bolt/stone lose 12
each; trail arrow delivers 3 HP/source 0 and every emitted cluster retires.
Save/config/resources are restored, cleanup errors empty. Parity is four
GREEN/one YELLOW (base call count 16 binary/21 source); offset oracles are
unanalysable, so static layout spacing is not full absolute-offset proof.
