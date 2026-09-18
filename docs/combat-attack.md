# Attack recovery — 2026-09-18

`CGameSprite::Attack` (0x737EF0) now connects target validation, weapon range,
line of sight, movement and Swing. The existing partial ExecuteAction
(0x728F80) dispatches Attack (3), AttackNoSound (98) and AttackOneRound (105).
Other missing ExecuteAction branches remain outside this recovery.

Full contexts: `tmp_attack_current.md`, `tmp_attack_dispatch_full.md`.
Assembly: `tmp_attack_asm.txt`, `tmp_attack_dispatch_asm.txt`.
Binary details checked include byte/short range arithmetic, target-side
Orderable for LOS, case-sensitive animation names, unusable trigger string1,
party fallback targeting, and the one-round completion condition.

Final profile: `.arc/2026-09-17T17-06-41-168494-batch/`.
VS2019 Win32 build passed; 90 seconds CLEAN; Attack and Swing each hit 442
times. All 22 UI steps passed. The camera is centered on the party and both
members must be visible before testing.

| Controlled round through ExecuteAction | Action | Target HP | Charges |
|---|---:|---|---|
| Melee | 3 | 291 -> 285 | 3 -> 3 (unlimited) |
| Bow/arrow | 98 | 285 -> 273 | 3 -> 2 |
| Crossbow/bolt | 105 | 273 -> 261 | 3 -> 2 |
| Sling/stone | 3 | 261 -> 249 | 3 -> 2 |

These rounds force successful hits and controlled damage, advance combat
frames explicitly, and restore temporary equipment/stats. The final test
then uses the save's real equipment: select party member 0, click the normal
weapon button, pick party member 1 and click. The ordinary AI loop executes
action 3 and reduces the target from 249 to 242 HP after 64 scenario ticks.
The scenario stops that attack afterward. No save is written; fixture removal
and save/config restoration passed hash verification.

Static lints, offsets and class/layout checks pass. Call-count parity remains
YELLOW (Attack 124 original / 79 source; Swing 201 / 139), and Swing's string
provenance check remains a review warning. No matched original/reconstruction
capture exists: these are runtime observations, not full equivalence proof.
Out-of-range pursuit, invalid targets and special animation timing are not
covered by this scenario. The normal UI click was tested with one equipped
weapon, while all four weapon families were tested through action dispatch.

Earlier attempt `.arc/2026-09-17T16-59-39-774506-batch/` is retained: a CString
cast lint was corrected; the game did not reach loading (zero target hits,
cause undetermined). The subsequent full profile above succeeded.

Next combat work: exercise ordinary UI attacks for each ranged family and
out-of-range pursuit, then build a matched original/reconstruction capture.
Ranked remaining recovery targets are in `docs/next-targets.md`.
