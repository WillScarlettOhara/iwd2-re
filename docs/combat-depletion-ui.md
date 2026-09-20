# Last-ammunition recovery check — 2026-09-20

Fixed CheckItemDepleted (0x75D450): Unusable carries the item resref in
CAITrigger::m_string1, not m_string2. Full context/provenance is in
`tmp_depletion_current.md`; PE assembly in `tmp_depletion_asm.txt`.
At 0x75DBF5 the local trigger starts at stack +0x34. At 0x75DC8E a push moves
the stack by four bytes, so the assignment destination [esp+0x86] at 0x75DC8F
is the original stack +0x82, or trigger +0x4E (string1). The missing-ability
path at 0x75D7F6 likewise uses trigger +0x4E. Existing combatammo checks now
assert the resource string and empty string2 for all three ammunition types.

New `combat-depletion-ui` profile uses the same transient fixture equipment as
the ranged UI test, with one round per pile. Each weapon is tested without a
reserve and with a second identical one-round pile. The ordinary weapon button
and picked companion initiate the attack; no hit or combat-frame forcing.
The test requires consumption, not damage, because misses also spend ammo.

Observed in `.arc/2026-09-20T19-54-22-660690-batch/`:

| Weapon | No spare | Identical spare |
|---|---|---|
| Bow | slot 11 deleted, fists (10), action 0 | slot 11 deleted, slot 12 selected, action 3 |
| Crossbow | slot 11 deleted, fists (10), action 0 | slot 11 deleted, slot 12 selected, action 3 |
| Sling | slot 11 deleted, fists (10), action 0 | slot 11 deleted, slot 12 selected, action 3 |

The spare retains its single round; the test stops the continuing attack before
another shot. All 94 scenario steps passed, including the existing ammunition
helper checks and equipment restoration. Camera centering and ground-picker
assertions guard against clicking another object.
VS2019 Win32 build and the full 120-second observation passed CLEAN. Profile
cleanup removed its fixtures and hash-verified save/config restoration.

The binary passes !IsPC to SelectLauncherAmmo's melee-fallback flag. For the
tested player character, depletion without spare ammo therefore does not select
the strongest melee weapon. This behavior was preserved. NPC melee fallback,
different-resource replacement ammo and used-up-item transformations are not
covered by this UI scenario. No paired original/reconstruction runtime capture
exists. Offset checks and source lints passed; call-count parity remains YELLOW
(CheckItemDepleted 110 original / 61 source; Swing 201 / 139), with Swing's
existing string-provenance warning. These remain separate from runtime evidence.

Next: original comparison of depletion transitions and NPC melee fallback;
different-resource replacement and attack-order behavior on target loss.
