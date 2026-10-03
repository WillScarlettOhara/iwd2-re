# Matched final shots with identical reserves — 2026-10-03

The reconstructed and canonical original executables agree on 14 observations:
four base weapon descriptors, four base projectile HP deltas, three final-shot
round summaries and three delivered HP deltas. This arc adds test scaffolding;
no recovered engine body changes were needed.

Each ranged family starts with one unit in ammunition slot 11 and seven units
of the identical resource in slot 12. Other reserve slots are temporarily
cleared. The test executes Attack (3) through ExecuteAction and the real
Attack/Swing/depletion/selection path with controlled stats and forced hits.

| Pair | Delivered damage | Last unit | Selected reserve | Reserve count |
|---|---:|---|---:|---:|
| ZRCARROW / ZRCBOW | 12 | deleted, slot 11 null | 12 | 7 |
| ZRCBOLT / ZRCXBOW | 12 | deleted, slot 11 null | 12 | 7 |
| ZRCSTONE / ZRCSLING | 12 | deleted, slot 11 null | 12 | 7 |

All rounds take 102 direct ExecuteAction calls, end at frame -2, return
ACTION_INTERRUPTABLE (1), select ability 0/set 0 and leave no pending projectile.
Every intermediate call remains free of ACTION_ERROR. Selection of the reserve
therefore does not abort the remainder of the controlled round, and does not
consume another unit in that round. The queued projectile still delivers all
12 damage points after the last item has been destroyed.

The last unit is heap-allocated with the game's allocator, since the engine
owns its deletion. A separately demanded instance of the same resource keeps
the ability data alive for test cleanup. The test checks the live slot rather
than reading the deleted object. Assembly at 0x73B21B–0x73B24A confirms Release
before CheckItemDepleted, ACTION_ERROR on true and ACTION_NORMAL on false.
Existing depletion comparison establishes the identical-resource replacement
branch and its lack of an Unusable trigger.

Temporary reserve pointers, cached ammunition, quick buttons, source stats,
owned current action and combat state are restored before subsequent world
updates. The original equipment is reselected to restore equip/animation state.
The base checks first establish the same source/target positions and target
resistance/difficulty context in both executables despite different party order.

Reconstruction: `.arc/2026-10-03T15-50-21-979991-batch/`, PASS with four static
warnings. Eight source checks, class/layout audits and Win32 build pass. All
15 UI steps pass; the 60-second observation is CLEAN. Observed hits: Attack
306, Swing 306, CheckItemDepleted 3, SelectLauncherAmmo 3. Previous static
call-count/provenance warnings remain and SelectLauncherAmmo's offset check
is not analysable; runtime comparison is separate from those static limits.

Original: `.arc/2026-10-03T13-55-10-678439-weapon-reference/`, MATCH 14/14.
Generated JS, raw/normalized traces, input hashes and differential reports are
retained. Both transactions restore save/config state without cleanup errors,
both game processes are stopped and no test damage was saved. Twenty-two
comparator/fixture tests pass, including rejection of missing delivered damage,
undeleted last units, consumed/wrong reserves and an aborted round.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x737EF0 0x739B60 0x75D450 0x75E330 --profile scripts/profiles/combat-finalshot-reference.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --finalshot --ours .arc/2026-10-03T15-50-21-979991-batch/profile-runtime/ui.jsonl
```

This establishes final-shot delivery and completion of the current controlled
round with an identical reserve. It does not prove another round firing from
that reserve, normal AI timing, hit/miss/critical rolls, or final-shot behavior
without a reserve or with a different resource. Next: the latter two cancellation
paths, retaining damage delivery and the exact action return/frame evidence.

Follow-up: [empty/different reserve cancellation checks](combat-finalshot-cancellation.md) now match the original for each mode (14/14 observations), including delivered damage and the ACTION_ERROR cancellation frame.
