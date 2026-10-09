# Impact delivery and BAM lifecycle

Final validation: 2026-10-09T12-04-11-878720-batch PASS with eight warnings.
Win32 Debug build, eight shared lints and class audits pass. All 598 typed
assertions, 18 UI steps and ten symbol hit gates pass, with six AimAtPoint
entries during the distant shots and a 60-second CLEAN hold. HP loss remains
6/12/12/12 for melee/bow/crossbow/sling; each ranged round spends one charge.
Profile cleanup restores and hashes preserved paths, removes fixtures and
reports no errors. Static parity is GREEN for DeliverEffects, AimAtPoint and
the message destructor; the remaining targets are YELLOW (call counts or the
short Run body). Operand-count advisories and incomplete/skipped layout/offset
checks remain; this is observed runtime evidence, not binary equivalence.

Recover one connected projectile lot: CProjectile::DeliverEffects (0x52A1A0),
CProjectileBAM constructor (0x57CAC0), AIUpdate (0x57CEE0), Render (0x57CFB0)
and Fire (0x57D230). Validate the existing CMessage105 constructor/destructor/
Run (0x5152C0/0x515430/0x5157F0) together with AimAtPoint (0x52BD20) and
Swing (0x739B60), using combat-impact-flight.json.

Full bundles: tmp_projectile_delivery_lot_ctx.md, tmp_message105_lot_ctx.md,
tmp_bam_motion_lot_ctx.md, tmp_bam_ctor_ctx.md and tmp_bam_render_ctx.md.
PE disassembly decides target acquisition attempts, message ownership, field
widths, constructor members, sound arguments and render gates. BG2 names are
supporting evidence only; its BAM layout differs from this IWD2 class.

DeliverEffects now sends one subtype-105 wrapper containing ordered
CMessageAddEffect children, including an empty wrapper for an empty successful
impact. The wrapper copies list nodes and owns the children. The projectile
empties its effect list, queues once, releases the target share, then frees
only its temporary nodes. Missing targets retain the payload; non-AI and
immune targets discard it. This replaces the earlier individual-message
approximation; CMessage105 itself was already recovered.

BAM Fire acquires the live target once and returns if that acquisition fails.
It records the live target position, obtains the facing-adjusted source
origin, plays the prepared sound, registers and adds itself to the area,
seeds the fixed-point motion fields, then delivers its effects. Failed
registration deletes the projectile. Both BAM and travelling Fire write
the common base motion/target fields, so their declarations move to CProjectile.

The BAM constructor includes the bitmap at +0x26C and visual effect at +0x32E,
clears its three raw fields, prepares the sound on channel 4 and stores the
sound reference in m_fireSoundRef, as the binary does. Delay arithmetic uses
BYTE sequence length and repeats the length lookup after SequenceSet(0).
AIUpdate respects Time Stop except for its caster. Render uses its own area,
checks tile visibility, intersects frame bounds with the current viewport,
and preserves the original FX preparation/render/blit chain.

The scenario adds 41 typed cases to the prior 557: six wrapper ownership/order
cases, twelve impact deliveries (0/1/3 effects across sprite/missing/non-AI/
immune targets), three constructors, four render calls, four launches and
twelve delay/Time Stop updates. Render calls cover initial delay, invisible
off-map position, a displaced viewport and a visible frame. They exercise
the entry and check unchanged projectile state; they do not establish pixel
equivalence or count every graphics branch. Constructor cases check resource
binding, sound channel, copied visual parameters, members and delay boundaries.

Four subsequent Swing rounds use the existing deterministic fixtures for
melee, bow, crossbow and sling, with forced hit and fixed damage. Normal engine
ticks advance ranged projectiles and apply real HP loss. After melee the
existing attackplace primitive moves the shooter to (232,1002), the verified
combat-pursuit setup for this save, before the three ranged rounds. This requires
actual travel beyond the arrival radius; movement and damage exist only in the
disposable process. This directly tests
Swing and impact delivery, not attack-order UI or original/recovered differential
equivalence. The profile centers the party, mounts temporary equipment,
restores save/config/fixture paths transactionally and never saves test damage.

The first extended run (2026-10-09T11-48-34-994312-batch) passes all 598 typed
assertions and 16 scenario steps. Actual target HP is 291 -> 285 -> 273 ->
261 -> 249 (6/12/12/12 damage), with one charge spent per ranged shot. All
nine resolved symbols execute and the 60-second hold is CLEAN. AimAtPoint's
marker was outside the symbol resolver's comment window; it was moved next
to the declaration and the complete ten-target batch is rerun. The nine-symbol
run does not establish the missing AimAtPoint hit gate.

The ten-symbol run (2026-10-09T11-55-56-443140-batch) again passes every
assertion and damage case, but reports NOT-EXERCISED for AimAtPoint. The party
starts too close together: missiles arrive on their first AI update without
the integrator. This is an ERROR, not a pass. The scenario is extended with
the distant placement and a normal tick wait; the full ten-target batch is
repeated without reducing gates.

An earlier six-target attempt (2026-10-09T11-09-14-166306-batch) reached a build harness
timeout and has no runtime verdict; it is not a pass. The subsequent standalone
full Win32 Debug build succeeds, including the changed header.

Remaining scope includes original/recovered runtime differential captures,
network delivery/marshalling, explicit resource-immunity feedback and target
lock contention. Current asynchronous typed cases do not prove contention.
The generated target list's CProjectileBAM::MoveUpdate at 0x57D9F0 is a stale
class association: its lifetime/bounce/shadow fields belong to the travelling
VFX branch. Establish the constructor/vtable identity before recovering it;
do not add this method to CProjectileBAM using the current export's name.
