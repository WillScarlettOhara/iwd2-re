# Beam launch, ticks and strikes

Recovered IcewindCProjectileScorcher AIUpdate 57B210, Fire 57B1D0,
GatherTargets 57BE80 and Strike 57C1D0. Added folded empty DeliverEffects
799CA0 and DecodeProjectile 51EAF0 case 6D: native handler 51FF03 creates
the beam, sets TRA_17 sound, retains defaults and stores type 6C.
Class name remains provisional; no full-factory equivalence claim.

Full bundles/manifests: tmp_beam_{tick_gather,launch_strike,empty,factory}_context.md.
Shared header, whole factory lift, native bodies/slots/constants/STL inspected.
Ghidra created missing Fire/Strike before recovery.

AI increments wrapping LONG tick before Time Stop. Zero BYTE life removes
and deletes only after successful array deletion. Otherwise decrement life,
advance cell, share live target; missing/other-area targets RemoveSelf.
Virtual position and Z update endpoint/slope. Source deny retries SHARED
only; absent source retains clock/frame changes. Sprite sources turn toward
the endpoint. Update sequence, release deny, GetStart with facing offset.
Signed tick%clearPeriod == 1 clears struck IDs before tick%strikePeriod == 1
strikes initial ID with BYTE 1 if unseen, inserts even if rejected, then scans.

Numeric tail wraps shifts/products before signed division. Origin Y shifts
before multiplying by four/dividing by three. Endpoint Y scales by 4/3.
Squared distance wraps LONG; sqrt+0.5 uses __ftol low DWORD (NaN gives zero).
Zero distance arms expiry, retaining steps. Otherwise ceil by signed SHORT
velocity, narrow segments to BYTE, divide shifted deltas by zero-extended
count. Native zero-divisor faults preserved. Sound follows anchored head.

Gather builds four +/- facing-offset vertices, min/max bounds, ANYONE and
AREA visible-terrain table; includes back list. Exclude source and seen
DWORD IDs, strike with BYTE 0, then insert. Fire calls VFX base, stores
initial ID, calls virtual empty DeliverEffects; ordinary payload retained.
Strike shares victim once, rejects non-AI/exact TRUE immunity with feedback,
shares source, GetStart/CheckLOS with PROJECTILE terrain and explored FALSE.
BYTE == 1 loads EffAS1, all others EffAS2, level ONE. Override source resref
to SpWi217, transfer each effect to owned AddEffect message. Release victim
before source. Existing GetAllInPoly filter/table handling remains limited.

Native tmp_beam_tick_native_reference_v3/report.json PASS: 29 numeric states,
canonical MD5, isolated engine callbacks, no strike/STL proof. TLS/Chitin
restored; process killed; fingerprints stable. Initial capture UNPROVEN:
invalid Frida short return, corrected to int16. v2 also passed.
Golden replay isolates removal; real deletion tested separately. Oracle unchanged.

AutoUI beamchecks tests ticks, expiry, Time Stop, velocities, live IDs, facing,
anchor/sound, dedup/reset, actual polygon, virtual launch, factory, natural
EffAS payload provenance and actual defender HP through owned messages.
Pinned instant ZRCHIT tables give 3 initial or 2 secondary HP. Actor/resource,
list/message, Time Stop and clock restored before next ordinary tick.

Failed batches on 2026-10-13 retained:
14-26-16 compile: test FrameAdvance returned void rather than BOOL.
14-33-08 compile: derived stats copy constructor deleted; use assignment.
14-40-32 runtime: isolated native removal versus actual fixture removal;
one golden mismatch; four HP checks passed. Cleanup restored resources.
Final .arc/2026-10-13T14-51-37-446641-batch PASS: Win32 Debug, 68 UI steps,
88 beam checks, 29 golden states, 60s CLEAN. PDB AI/Fire/Gather/Strike/factory
hits 52/1/2/11/33. Melee -6 HP; bow/crossbow/sling -12 and one ammo each.
Beam initial -3, secondary -2. Cleanup restores hashes, no errors.
AI/Strike parity GREEN; other three YELLOW (short Fire, call counts).
Missing offset oracles leave full static equivalence unproven. No gates reduced.
Visual gameplay, blocked/nonzero-terrain LOS and immunity remain unproven.

Next: beam variant ctor 57C510 and virtuals.
