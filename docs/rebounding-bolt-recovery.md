# Rebounding bolt recovery

Lot: ctor 57D390, Fire 57D700, AIUpdate 57D9F0, base ctor 578110 and
DecodeProjectile case CF. Guessed class CProjectileReboundingBolt, vtable 85101C.
The old CProjectileBAM::MoveUpdate export is wrong: 57D9F0 is slot 3
(AIUpdate), and +2B6 is IcewindCProjectileTargetMap, not CVidCell.

Evidence: full context bundles tmp_bounce_ctx.md and tmp_bounce_fire_ctx.md,
full reachable update/launch disassembly and tmp_ice_ctor_ctx.md. The missing
Fire export used address-map/create-functions. Entry 5291B4 (CF) -> 524F74
allocates 2EC bytes, calls 57D390 and sets +2B2 to 1.

Base ctor now clears sparkle flag E2, matching 578206; our inherited model
had left it indeterminate. Ctor: LightnT, doubled signed WORD velocity, render
flag 8, no tint/height offset/shadow, 16 directions, TRA_09b sound, duration
60, unlimited bounces (-1). Tracker: owner this, period 2, interval 33,
range 50. Pack(2); inherited by-name layout and terminal std::map drift remain.

Fire: third LONG argument goes to +72 (sourceId), targetId is always -1;
the second argument supplies GetStart. First share failure leaves the
object unregistered. Registration precedes the third-argument share; its
failure leaves a registered object outside the area, faithfully. Success
adds LIST_FRONT at the given height, flags 17C, plays the fire sound and
seeds X << 10, wrapped Y << 12 divided by 3. Direction uses the passed
point. Velocity uses wrapped signed squared distance, sqrt + 0.5 then
truncation. Zero distance retries against the third argument's centre;
a second zero arms duration zero without immediate removal. Steps are
wrapped (delta << 10) * signed WORD velocity, divided by rounded length.

AIUpdate: Time Stop precedes all mutation. Zero duration removes and deletes
only if ObjectArray::Delete succeeds; negative duration wraps. Other ticks
decrement duration, advance the main cell, update
fixed-point coordinates, then enforce strict area bounds. Same search cell
skips LOS/bounce work but still services the tracker and moves the sound.
On crossing, impassable cost OR terrain index 8 triggers collision. Positive
bounce counts decrement and retire at zero; zero/negative means unlimited.
The dominant velocity axis is tested first (ties X), retracting two steps
on each reflected axis. X-major fallback checks cost and index 8; Y-major's
second fallback checks ONLY cost (57DE8A), an intentional native asymmetry.
A bounce updates next target point and facing before tracker service/sound.

PARTIAL: optional sparkle trail factory 51AE40 remains unrecovered, as in
the travelling base. This leaf leaves it disabled. Slot 19 inherits 578480.
Hits use the existing proximity tracker, not OnArrival.

AutoUI reboundchecks: controlled terrain, golden wall choices across 3 axis
ratios, 4 signs, 8 masks, 4 budgets; launch/rounding/fallback, Time Stop,
expiry, wrapping, frame/service/sound order and Delete failure. Restores
search data, actor position, Time Stop, objects and queues in one tick.
Rebounddamage registers a stationary bolt at portrait 1 with range zero,
services its real gather/strike path, verifies one cloned damage message,
removes the bolt, then waits for ordinary message ticks to deliver 3 HP.
The disposable profile retains melee/bow/crossbow/sling HP checks; no saving.

Validation: .arc/2026-10-11T18-59-34-051163-batch, full four-target profile
PASS, three call-count warnings (one GREEN/three YELLOW). Eight source
checks, two class/layout audits, Win32 Debug build and cleanup PASS.
Hits: ctor 414, Fire 15, AIUpdate 398, base ctor 421. All 2168 checks and
55 UI steps PASS; 60-second hold CLEAN. Bolt payload: target -3 HP,
source unchanged; Swing: melee -6, arrow/bolt/sling -12 each. Offset
oracles are not analysable; no original/reconstruction differential.
Next: recover the shared sparkle emitter 51AE40 or its enclosing lifecycle.
