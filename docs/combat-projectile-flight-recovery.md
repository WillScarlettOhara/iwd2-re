# Travelling projectile target handling

Recover missing details in CProjectileTravelling::AIUpdate (0x52B900),
using full context bundle tmp_travel_ai_ctx.md and PE instructions.
Shared CProjectile header/layout is unchanged from the preceding arc.

GetShare is a single attempt (0x52BA91), with any non-success removing the
missile; the recovered source incorrectly retried. GetDeny in the drift
height branch retries SHARED only (0x52BB89), not DENIED. When the target
has left the area, save its ID before RemoveSelf (0x52BAC8), then release
that saved ID: RemoveSelf may delete this. For drift, CalculateFxRect and
ReleaseDeny precede the height comparison. The PE writes height +/-1 then
restores the old EAX at 0x52BC2A; preserve that quirk instead of retaining
the adjustment. The target height uses arithmetic shift, not division.

Twelve typed cases use an ARARROW travelling subclass whose arrival,
removal and aim virtuals record behavior without deleting the fixture or
moving it. Cases cover linear height interpolation with both delta signs,
drift height retention with both signs, missing target, wrong area,
untargeted aim, expired lifetime, same-cell arrival, Time Stop exclusion
and owner bypass, and zero-distance arrival. Fixed expected positions,
height, lifetime and virtual outcomes are asserted. Time Stop is restored.
Earlier 466 assertions remain, for 478 total. Fixtures never join area
lists or write saves. Contended object-array retry outcomes and actual
self-deletion are established statically, not simulated by these cases.

Validation: final full batch 2026-10-07T21-16-41-020899-batch passes eight
shared lints, class audits and Win32 Debug build. All 478 typed assertions
and 11 steps pass, followed by 60 seconds CLEAN and 12 AIUpdate entries.
Fixtures removed and preserved paths restored and hashed. Parity is YELLOW
(22/30 calls). Argument provenance reports binary=0/source=1 CString concat
sites: the body has only integer additions, no CString concatenation;
arithmetic was checked against PE. Offset and layout evidence is incomplete
(not analysable/skipped). No visual differential is claimed.

The first run passed cases but failed to resolve the symbol for hit counters
because its address marker was too far above the declaration. Relocate the
existing marker directly above AIUpdate and rerun every gate; the final run
does instrument and exercise the symbol. Sparkle-trail factory remains
explicitly unrecovered; no full flight/damage equivalence is claimed.

Next: actual weapon flight/damage regression scenarios and remaining
projectile execution gaps.
