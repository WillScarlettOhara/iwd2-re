# Sparkle cluster recovery

Lot: CSparkleCluster ctor 51AE40, dtor 51C330, AddToArea 51C3D0,
AIUpdate 51C6E0, Render 51CC00, GenerateExplosion 51CE50; travelling ctor
52AD60 and both trail branches in AIUpdate 52B900 / rebounding bolt 57D9F0.
Native vtable 84D624, allocation CA bytes. BG2 names corroborate the class;
this is an independent CGameObject emitter, not a projectile factory.

Evidence: full bundles tmp_sparkle_ctx.md, tmp_sparkle_spawn_ctx.md,
tmp_sparkle_life_ctx.md and tmp_sparkle_flight_ctx.md, shared headers/BG2
blocks, full reachable disassembly of all nine targets. Missing AI/render
definitions were created via address-map/create-functions. Pack(2), with
a compile-time assertion on the native 5C-byte derived member span; Debug's
CGameObject base differs from the original.

Constructor selects twelve SPARKLECOLOR table rows and parses five RGB
entries; caches the video mode, clears timer/bounds, records type/duration
and gravity 50. Native palette branches are factored into a row table and
loop. Unwritten fade heights/direction remain unwritten until their owner
sets them. Travelling ctor now clears the native DWORD trail flag at E2.
Native 52AD60 also leaves palette-resource 286 and field 298 unwritten;
the changed-constructor lint baseline records this checked omission.

AddToArea registers first, deletes on failed registration, then queues the
object in the requested area list. Fade heights are wrapped BYTE fractions
99/85/75/55 percent of height. Travel/conjure spawn fourteen particles with
native rand order, Y*4/3 fixed-point conversion, alternating X velocity,
gravity 50..180 and current fade/brightness adjustment. Explosion bit wins
over conjure. Explosion chooses one of eight headings by the native 27:16
and 7:38 comparisons, emits three fourteen-particle velocity streams, and
uses the native 100-by-100 bound. Its direction is an absolute target point.

AI updates color bands with strict comparisons and saturated channels,
randomly reverses travel X, integrates wrapped velocity/gravity/height and
removes ground particles. It does not advance particle clocks/tails.
Removing the last particle retires immediately, even with conjure time
left; delete failure leaves an empty emitter outside the area. Surviving
conjure ticks append fourteen particles at eight rotating spawn origins.
Destructor owns and frees all remaining particles.

Render uses the owning area's visibility/view, native FX preparation,
lock/clipping/blit order, exact type-specific Y offsets, adjusted particle
colors and restoration after each draw. It ignores the passed area/video.
Travelling AI emits after AimAtPoint; rebounding AI emits only on a surviving
cell crossing, after wall retraction and before tracker/sound updates.

AutoUI sparklechecks covers palettes, signed heights, all fade boundaries,
conjure duration/origins, explosion headings/streams, render color restoration,
successful/blocked final deletion and both virtual projectile spawn paths,
including Time Stop, arrival, disabled/same-cell and wall-bounce guards.
Fixtures restore search, objects, queues and clock in the same tick.
The full disposable profile also launches an arrow through opcode 430 with
test-only trail-flag activation, checks real payload damage and retirement
from area lists, and retains melee/bow/crossbow/sling tests. Original factory
2F now confirms zero-timer far expiry/near arrival; see
[projectile-launch-recovery.md](projectile-launch-recovery.md).

Validation: .arc/2026-10-11T21-37-18-936882-batch PASS (9 targets);
Win32 Debug, 663 particle checks/60 UI steps, nine symbols hit and 60 s
CLEAN. Three real trails retired; melee -6 HP, bow/crossbow/sling -12,
trail arrow -3/source 0. Offset oracles unanalysable; parity 1 GREEN/8 YELLOW.
Renderer invocation/color preservation is not pixel
equivalence; original/reconstruction differential capture is not claimed.
The five explosive/trap/spell-hit trail branches are tracked in
[sparkle-family-recovery.md](sparkle-family-recovery.md).
