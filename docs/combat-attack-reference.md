# Matched ExecuteAction/Attack rounds — 2026-10-03

The reconstructed and canonical original executables agree on 16 observations:
four base damage descriptors, four base projectile HP deltas, four controlled
ExecuteAction round summaries and four HP deltas including weapon payloads.
This arc changes test scaffolding and comparison tools, not recovered engine
bodies. AutoUI now records action ID, return value and ending attack frame.

| Fixture pair | Action | Return | HP loss | Remaining weapon usage |
|---|---|---:|---:|---:|
| ZRCMELEE | Attack (3) | 1 | 6 | 3 (unlimited melee ability) |
| ZRCARROW / ZRCBOW | AttackNoSound (98) | 1 | 12 | 2, from 3 |
| ZRCBOLT / ZRCXBOW | AttackOneRound (105) | -1 | 12 | 2, from 3 |
| ZRCSTONE / ZRCSLING | Attack (3) | 1 | 12 | 2, from 3 |

Every round takes 102 direct ExecuteAction calls, ends at attack frame -2 and
leaves no pending projectile. The action dispatcher resolves the real target
and enters Attack, which checks reach/LOS and calls Swing. Action 105 returns
ACTION_DONE (-1); actions 3 and 98 return ACTION_INTERRUPTABLE (1).
Original assembly confirms the action-count read at +0x474, current action at
+0x476, comparison against 0x69 at 0x738BA4, ACTION_DONE at 0x8485C4 and last
action ID at +0x5606. ACTION_NORMAL is 0 and ACTION_ERROR is -2.

Actor matching, target context checks and forced-hit source stats are inherited
from [the Swing comparison](combat-swing-reference.md). Four base damage tests
run before the action rounds. The target selector uses its real object-array
ID, not a party slot. On the original, temporary actions and their backup are
constructed at 0x403A40, copied with operator= at 0x405090 and destroyed at
0x403B40. This preserves owned CString references and avoids raw action copies.
The target instance sits at action +0x48 (actee +0x3E, instance +0x0A).
Current action, action count, last action ID, temporary equipment, value stats
and attack state are restored synchronously before queued damage executes.

Reconstruction: `.arc/2026-10-03T15-09-42-152191-batch/`, PASS with three static
warnings. Eight source checks, class/layout audits and Win32 build pass. All
16 UI steps pass; the 60-second observation is CLEAN. Runtime hits are
ExecuteAction 93837 (including ordinary idle dispatch), Attack 408 and Swing
408. Static parity retains the previous call-count warnings for the partial
ExecuteAction body, Attack and Swing; Swing also retains its provenance warning.
This comparison does not establish parity of unrelated dispatcher branches.

Original: `.arc/2026-10-03T13-13-32-476423-weapon-reference/`, MATCH 16/16.
Input hashes, generated JS, raw/normalized captures and differential report
are retained. Both transactions restored save/config files without cleanup
errors. Neither process remains running and no damage was saved. Thirteen
harness tests pass, including rejection of missing action evidence, incorrect
AttackOneRound completion, wrong action IDs and unfinished frames.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x728F80 0x737EF0 0x739B60 --profile scripts/profiles/combat-attack-reference.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --attack --ours .arc/2026-10-03T15-09-42-152191-batch/profile-runtime/ui.jsonl
```

These are forced-hit rounds with direct frame progression. Ordinary AI timing,
hit/miss rolls, critical hits, last-unit ammunition depletion, retaliation and
nonempty immunity lists remain outside this comparison. Next: final-unit
depletion and automatic weapon/ammunition selection, then matched AI-driven
attacks with ordinary hit checks.
