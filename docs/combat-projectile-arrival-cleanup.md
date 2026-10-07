# Projectile arrival cleanup dispatch

Correct CProjectile::OnArrival (0x529FB0) attached-projectile cleanup:
the PE instruction at 0x52A118 is a direct call to CProjectile::RemoveSelf
(0x5551B0), not a virtual call. Use an explicitly qualified base call.
Existing recovered leaves do not override RemoveSelf, but dispatch must
remain faithful for subclasses. Other arrival behavior is unchanged.
Context bundle tmp_projectile_arrival_ctx.md and PE instructions were read;
the existing PlaySound helper matches the inlined arrival sound setup.

Nine typed cases combine absent/registered/missing callback and attached
projectile IDs. Disposable registered subclasses record callback, effect
delivery, removal and destruction order; their RemoveSelf override is a
canary that must not execute for attached cleanup. The old virtual call
would leave the attachment alive and fail these checks. A missing callback
returns before delivery/removal/destruction, even with a valid attachment;
a missing attachment does not block delivery. Successful arrival destroys
the attachment before delivering, then removes and destroys the main
projectile. A valid callback runs first. Survivors are explicitly cleaned
through the base RemoveSelf, and test objects never join area lists.

Earlier 457 assertions remain, for 466 total. No production test subclass,
new mapped method or header layout change is introduced. No save is written.

Validation: full batch 2026-10-07T20-52-25-518746-batch passes all eight
shared lints, class/layout audits and Win32 Debug build. Both parity
results are GREEN. All 466 typed assertions and 11 scenario steps pass;
60 seconds CLEAN with both functions executed. Fixtures removed and
preserved paths restored and hashed. Both member-offset audits skip as
not analysable; static evidence remains incomplete. This covers typed
arrival/lifetime behavior, not an original/recovered damage or visual
differential; generic visual notices are not claimed as verified.

Next: extend actual travelling melee/bow/crossbow/sling regression scenarios
and recover remaining projectile execution gaps.
