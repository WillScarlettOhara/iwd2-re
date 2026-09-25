# Original GroupAttack reference — 2026-09-25

The original executable and reconstruction agree on the fourteen named semantic
checks from `combatgroup`. This compares recorded predicates, not complete actor
state or damage calculations. No engine implementation changed in this work.

## Capture and comparison

`scripts/probes/group_attack_reference.py` verifies the canonical original MD5,
loads exactly `000000037-testSpells` through the existing original load driver,
and runs `group_attack_reference.js` on the world update thread. It uses original
functions and two actual party actors. It restores the current action, interrupt
flag, target AI type/activity flags and response indices, and deletes the test
actions before another update. The source queue must initially be empty. Only
the process spawned by this probe is terminated before profile file restoration.

Original pack(2) layout reads are checked against the source mirror and assembly:
action +0x476 (size 0xD6), target selector within action +0x3E, specifics
+0xB6/+0xBA/+0xBE, queue head +0x416/count +0x41E, activity flags
+0x50A6/+0x50AA/+0x50AE, interrupt +0x470. Native calls use thiscall and
asynchronous-thread constant 0. These offsets never read the debug reconstruction.

The reconstructed baseline is the successful direct test capture in
`.arc/2026-09-22T21-31-00-054410-batch/profile-runtime/ui.jsonl`. Records from
both sides are reduced to `{tag, case, passed}`. All fourteen records, their order
and boolean results are retained; IDs, raw type bytes and unobserved fields are
not compared. Both sides exercise null/inactive/interrupted guards, target
normalization, the two queued action IDs/order and their numeric specifics.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/probes/group_attack_reference.py --ours .arc/2026-09-22T21-31-00-054410-batch/profile-runtime/ui.jsonl --timeout 90
```

Final run: `.arc/2026-09-25T08-59-15-772349-group-reference/`.
`frida_trace_diff.py compare original.jsonl reconstructed.jsonl` reports MATCH,
14/14 events, matching frequency, order and fields. All checks are true. The
journal records restored=true and no cleanup errors. Script, baseline capture
and original executable hashes are in run.json; generated JavaScript and the
complete raw original log accompany the normalized traces. The existing VS2019
Win32 build and Python syntax compilation pass.

## Failed preparation retained

`.arc/2026-09-25T08-56-16-393077-group-reference/` timed out after entering
LoadGame, before any combat checks. It is UNPROVEN, not a mismatch. File
restoration completed without errors. The final driver selects the named save
directly at its native LoadGame call site instead of intercepting that nested
call. The exact cause of the first hang was not established.

Next scope: compare dispatch/target acquisition and damage under matched inputs;
these fourteen predicates do not establish those paths or full binary equivalence.
