# Swing recovery — 2026-09-17

`CGameSprite::Swing` (0x739B60) now dispatches animation combat frames,
melee damage/effects, ranged hit payloads and projectile firing, ammunition
consumption, depletion checks, hostility and retaliation. The initial target
is party-on-party melee, bow, crossbow and sling testing.

Full context: `tmp_swing_current.md`; assembly: `tmp_swing_ready_asm.txt`.
Preserved binary details include low-byte ability types, temporary/derived
speed selection, offhand selection, special E000/F100 animation timing,
victim-facing then attack-direction arguments to CreateWeaponDamage,
projectile Fire type 6, and `victim.ApplyOnHitSpells(attacker)`.

Final profile: `.arc/2026-09-17T13-52-22-227687-batch/`.
Win32 build passed. The entire 60-second observation was CLEAN with 408
Swing calls. Four rounds use the real combat-frame bitmap, forced hit and
controlled damage stats, restoring temporary equipment/stats after each round:

| Fixture | Calls | Target HP | Charges |
|---|---:|---|---|
| Melee | 102 | 291 -> 285 | 3 -> 3 (unlimited) |
| Bow/arrow | 102 | 285 -> 273 | 3 -> 2 |
| Crossbow/bolt | 102 | 273 -> 261 | 3 -> 2 |
| Sling/stone | 102 | 261 -> 249 | 3 -> 2 |

The scenario centers the camera and asserts both party members are visible.
The profile removed its fixtures and hash-verified save/config restoration.
Static offsets and class/layout checks pass. Parity is YELLOW (201 original
calls / 139 source calls; duplicate branches are factored), and the string
provenance auditor flags one count mismatch for review. Neither static checks
nor this test establish full binary equivalence; no paired original capture
exists. Critical/miss resolution, special animation families and retaliation
are not covered by this round scenario. Prior helper diagnostics cover some
of their components separately.

The first build failed on C++ API signatures (height helper, damage modifier
namespace and quick-button reference); the first static run also failed its
compile-dependent layout gate. Those reports are retained in
`.arc/2026-09-17T13-48-22-608648-batch/`. The corrected full profile passed.

Follow-up: Attack (0x737EF0) and its ExecuteAction dispatch are now recovered;
see [combat-attack.md](combat-attack.md) for action-dispatch rounds and a
normal UI attack that damages another party member. This Swing diagnostic
still calls Swing directly.
