# Matched resumed shots with identical reserves — 2026-10-03

The reconstructed and canonical original executables MATCH on 17 observations:
eight base damage observations, three first-round summaries, three second-round
summaries and three total HP deltas. Test scaffolding only; no recovered engine
body changes were needed.

For bow, crossbow and sling, the first Attack (3) uses the last unit in slot 11,
destroys it and selects the identical seven-unit reserve in slot 12. The harness
then resets attackFrame to -1 and actionCount to 0, keeping the action and engine
selected equipment. It drives a second ExecuteAction cycle without reinstalling
or reselecting ammunition. Fixed stats and forced hits are retained.

Each first cycle takes 102 calls, returns ACTION_INTERRUPTABLE (1), ends at frame
-2 and leaves reserve count 7. Each second cycle also takes 102 calls, returns 1,
ends at -2 with no pending projectile, and consumes exactly one reserve unit
(count 6). Selected slot/ability/set remain 12/0/0 and the two projectiles deliver
24 HP damage in total (12 per fixture hit). Original equipment, stats, action,
buttons and animation are restored before subsequent world updates. No test
state or damage is saved.

Reconstructed evidence: `.arc/2026-10-03T17-03-49-122790-batch/`, PASS with four
existing static warnings. Eight source checks, class/layout audits, Win32 build,
15 UI steps and 60-second CLEAN runtime pass; all four required combat symbols
execute. SelectLauncherAmmo offset analysis remains unproven; existing static
call-count/provenance warnings remain separate from runtime evidence.

Original evidence: `.arc/2026-10-03T15-10-03-532905-weapon-reference/`, MATCH 17/17.
The preceding `.arc/2026-10-03T15-08-44-200993-weapon-reference/` was DIFFERENT on
three total HP deltas: the original collector ended on the first positive delta,
before the second projectile arrived. In resume mode it now waits for aggregate
expected damage, bounded to 600 world ticks; timeout still emits the observed
delta for comparison. The raw traces and failed report are retained.

Both final transactions restored save/config state without cleanup errors;
processes stopped. Twenty-four comparator/fixture tests pass, including rejection
of early reserve consumption and failure to consume the reserve on the second
cycle.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x737EF0 0x739B60 0x75D450 0x75E330 --profile scripts/profiles/combat-finalshot-resume-same.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --finalshot --final-mode resume-same --ours .arc/2026-10-03T17-03-49-122790-batch/profile-runtime/ui.jsonl
```

This proves controlled second-cycle use of an identical replacement reserve.
It does not establish natural AI round timing/cancellation, separate observed HP
deltas for each projectile, or restart after different-resource cancellation.
Next: controlled restart with a different resource reserve, then natural UI
attack progression.
