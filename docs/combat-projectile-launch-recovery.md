# Travelling projectile registration and launch sound

Recover the registration/sound block of CProjectileTravelling::Fire
(0x52C050). Full context: tmp_travel_fire_ctx.md; unchanged shared projectile
header/layout reused from the preceding arc. PE instructions from 0x52C650
confirm the Add result check, virtual deleting destructor on failure, direct
CGameObject::AddToArea on success, and the loop flag at +0x15A.

On failed object-array registration, delete the projectile and return before
area insertion, sound or position initialization. Use the qualified base area
insertion, matching the direct call at 0x52C6B5. Launch sound uses
m_loopFireSound through the existing PlaySound helper instead of hardcoding
FALSE: Stop, nonempty resource setup, optional looping, channel 15 with area,
and positioned Play with fire-and-forget disabled.

Four disposable heap projectiles exercise empty/nonempty TRA_02 sound crossed
with looping off/on. They execute the actual Fire, register in the object
array and enter the loaded party area. Check identity, area, zero launch
height, loop state, channel, area sound tag and sound coordinates. Each is
removed immediately through base RemoveSelf, cancelling its sound and
unregistering it. No save data is modified. Registration failure is proven
statically, not forced by this runtime fixture.

Full batch: 2026-10-07T21-48-40-317701-batch. All 482 typed
assertions and 11 scenario steps pass, with four actual Fire entries and
60 seconds CLEAN. Eight shared lints, class audits and Win32 Debug build
pass. Parity remains YELLOW (49 binary calls versus 38 source calls);
offset analysis is not analysable and layout is skipped. Fixtures are
removed and preserved paths restored and hashed.
The first run passed runtime but lint rejected a block comment resembling a
function marker; corrected that comment and reran the full profile.

Fire remains partial: target bounds/height initialization, contention policy
and trailing-object factory still require recovery. No full projectile or
visual equivalence is claimed.
