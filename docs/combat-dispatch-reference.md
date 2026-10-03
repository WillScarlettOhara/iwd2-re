# Original target and enemy-dispatch reference — 2026-10-03

The original executable and reconstruction agree on all 36 named predicates
from `combatgroup 2`: fifteen UpdateTarget checks, fourteen direct GroupAttack
checks, and seven ExecuteAction dispatch checks. This validates those observed
predicates, including their order. It does not compare complete actor state or
damage calculations. No engine implementation changed in this follow-up.

## Capture

`group_attack_reference.py --dispatch` includes the target checks, then calls
the original ExecuteAction on the world update thread through
`group_dispatch_reference.js`. The two direct-test queued actions are discarded
first. The source must initially be idle with an empty queue.

The dispatch test temporarily sets the attacker to EA_PC and the companion to
EA_ENEMY, while the action's selector identifies the attacker itself. It checks
the absent-area result, interrupted result, stored last return, enemy selector,
two queued actions, AttackNoSound and repeated GroupAttack with specifics 7/8/9.
Target IDs, markers, area pointer, classification bytes and last return are
restored synchronously. The enclosing harness restores actions, interrupt,
target type/activity and response indices before releasing the actor locks.

Original offsets are checked against the source mirror and assembly: area
+0x12, AI type +0x20 / enemy-ally byte +4, last return SHORT +0x592, target ID
+0x5690 and markers +0x5648 (size 0x24). The dispatch block at 0x729E07 copies
ANYONE, sets only its enemy-ally byte from GetEnemyOf and resolves a sprite via
0x45C030. GroupAttack is called at 0x729E70. The shared exit releases the share
and writes the return value at 0x72B314. No original offsets read our debug build.

## Evidence

Reconstruction baseline:
`.arc/2026-09-25T17-38-41-882078-batch/profile-runtime/ui.jsonl`.
The engine source is unchanged since that successful 36-assertion capture.

Original target/direct-only capture:
`.arc/2026-10-03T10-57-12-805889-group-reference/`: MATCH, 29/29.

Original target/direct/dispatch capture:
`.arc/2026-10-03T10-58-48-922706-group-reference/`: MATCH, 36/36.
All predicates are true. Both transactions record restored=true and no cleanup
errors. The complete original logs, generated JavaScript, normalized traces,
comparison report and input SHA-256 hashes remain in each run directory.
Both executables' original MD5 is verified before capture.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/probes/group_attack_reference.py --dispatch --ours .arc/2026-09-25T17-38-41-882078-batch/profile-runtime/ui.jsonl --timeout 90
```

The prior cursor-calibration failure did not recur today. Its cause remains
unestablished; the load driver now includes desktop/engine positions and window
ownership in calibration failure records. The comparison harness stops early
on a terminal click failure and retains UNPROVEN, instead of waiting out the
whole timeout. Successful cursor calibration and native loading still use the
existing driver. Six calibration tests and complete generated-script syntax
checks pass; VS2019 Win32 compilation passes.

Next useful scope: matched original/reconstruction melee and projectile damage
inputs, including the normal Attack/Swing path and ammunition depletion.
