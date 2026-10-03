# Matched final shots with empty/different reserves — 2026-10-03

Reconstructed runtime checks pass for bow, crossbow and sling, with no reserve
and with a different resource reserve. This adds test scaffolding only, with no
recovered engine body changes. **Differential evidence is MATCH 14/14 for each reserve mode.** The canonical
original agrees on the three ranged cancellation summaries and their delivered
damage, alongside eight base damage observations in each capture.

Each test starts with one unit in slot 11, clears slots 13/14 and installs either
no slot-12 reserve or seven units of ZRCARR2/ZRCBLT2/ZRCSTN2. Alternate resources
have identical fixture contents; this isolates resource-name replacement.
Controlled luck/strength and a forced hit use the real ExecuteAction action 3,
Attack, Swing, depletion and selection path. Direct frame progression stops on
the first ACTION_ERROR rather than continuing a canceled action.

| Reserve | Calls | Final frame | Return | Selected slot | Reserve count | HP loss |
|---|---:|---:|---:|---:|---:|---:|
| None | 7 | 6 | ACTION_ERROR (-2) | 10 (fists) | 0 (absent) | 12 |
| Different resource | 7 | 6 | ACTION_ERROR (-2) | 12 | 7 | 12 |

All three ranged families have each result. Slot 11 is null after destruction,
selected ability/set are 0 and the projectile delivers all 12 HP damage despite
the action error. A pending replacement projectile may remain at cancellation;
the harness deletes it during restoration. The resource keeper keeps ability
memory valid across item deletion; no deleted item is read. Equipment, owned
action, buttons, stats and animation are restored before subsequent world ticks.

Reconstruction evidence:
- Empty: `.arc/2026-10-03T16-05-28-034819-batch/`.
- Different: `.arc/2026-10-03T16-08-14-528288-batch/`.

Both arc batches PASS with four existing static warnings: eight source checks,
class/layout audits, Win32 build, 15 UI steps and 60-second CLEAN runtime pass.
All four required combat symbols execute. SelectLauncherAmmo offset analysis
remains unproven and existing call-count/provenance warnings remain. These
static limits are separate from the observed runtime results.

Original attempts from `.arc/2026-10-03T14-10-12-015567-weapon-reference/` through
`.arc/2026-10-03T14-18-35-827738-weapon-reference/` retain UNPROVEN reports. The
cursor stayed at (799,314) across fullscreen calibration retries. Windowed and
host-side cursor experiments did not resolve it; the host SetCursorPos returned
false. Those experimental driver/config changes were removed from the harness.
One attempt also encountered a transient transaction journal access error.
All transaction journals record restoration without cleanup errors. No game
process remains and no test damage was saved. Twenty-three comparator/fixture
tests pass, including rejection of incorrect cancellation return evidence.

The input blocker cleared on the follow-up without driver changes. The thread
and input desktops both report Default; this does not establish the cause of
the earlier failure. Two initial retries hit the transient journal access error;
subsequent runs succeeded:
- Empty original: `.arc/2026-10-03T15-00-03-771927-weapon-reference/`, MATCH 14/14.
- Different original: `.arc/2026-10-03T15-00-43-819145-weapon-reference/`, MATCH 14/14.

Both successful transactions restored save/config files with no cleanup errors,
and their spawned games stopped. Win32 build and 23 comparator/fixture tests
pass again. No source/probe changes were required to obtain these comparisons.

Reproduce the comparisons:

```powershell
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --finalshot --final-mode empty --ours .arc/2026-10-03T16-05-28-034819-batch/profile-runtime/ui.jsonl
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --finalshot --final-mode different --ours .arc/2026-10-03T16-08-14-528288-batch/profile-runtime/ui.jsonl
```

These scenarios establish original agreement for the controlled final shot,
item deletion, selected replacement and action return/frame. They do not establish normal AI cancellation or
next-round behavior, natural hit/miss/critical rolls, or Unusable trigger payloads
within Swing. Next: test resumed attacks using the selected replacement ammunition.
