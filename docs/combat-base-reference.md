# Base weapon damage reference — 2026-10-03

The canonical original executable and reconstruction match eight observations:
four CreateWeaponDamage descriptors and four actual projectile HP losses.
Melee gives 5; arrow/bow, bolt/crossbow and stone/sling each give 9.
These are controlled fixture values, not the damage of the party's actual weapons.
No engine implementation changed in this arc.

## Comparable inputs

The profile loads `000000037-testSpells`, centers the camera, checks two visible
companions, and constructs temporary ZRC items. Source luck is 19, strength 10,
the derived template and base feats are zeroed, and rogue level/class/mask are
zero during the damage factory call. Source fields are restored before firing.
The source must be idle with an empty action queue and selective damage list.

The original probe runs on the world update thread, constructs CItem with its
eight-byte CResRef passed by value, calls the original factory, transfers the
effect to the decoded projectile, releases actor locks, then calls virtual Fire.
It observes actual target HP changes; it never applies damage directly.
The reconstruction's AutoUI executes the corresponding factory/projectile path.
Attack rolls, Attack/Swing, extra weapon/launcher effects and ammo expenditure
are outside this comparison. All fixture damage is crushing with flags zero.

Party array order differs in the two loaded executions. Selecting original
party indices 0/1 initially reversed the actors: the original target was Dahel
Bloody-Four at (232,858), a level-20 barbarian with crushing resistance 4 and
maximum HP 349. Reconstruction targeted Harbesh Carver at (232,810), resistance
0 and maximum HP 291. Consequently the earlier original losses were 1/5/5/5,
versus 5/9/9/9 in reconstruction. This is an input mismatch, not evidence of a
damage application defect. The suspected class-ability reload cause was not
confirmed; no resistance or class implementation was changed.

The corrected harness finds unique actors at the world positions emitted by
the reconstruction capture. Missing or ambiguous positions are UNPROVEN. It
checks target base/derived/temp crushing resistance, barbarian level, maximum
HP, difficulty and both difficulty multipliers against the baseline before
every update. It records original actor names and context. No original binary
member offsets are applied to the debug build.

## Evidence and limits

Reconstruction: `.arc/2026-10-03T13-24-28-669886-batch/`.
VS2019 Win32 build and eight shared source checks pass. The 12-step UI scenario
passes, both required symbols execute four times, and the 60-second smoke is
CLEAN. CreateWeaponDamage static parity is YELLOW; DetermineHeight is GREEN
with visual confirmation still outstanding.

The batch overall verdict remains **FAIL**: existing CProjectile audits report
missing virtual overrides at slots 0 and 0x2C and two actionable packing/layout
findings. Runtime agreement on these fixtures does not clear those findings.

Original corrected captures:
`.arc/2026-10-03T11-28-44-026723-weapon-reference/` and
`.arc/2026-10-03T11-29-29-672374-weapon-reference/`: **MATCH, 8/8**.
The second removes exploratory load/class hooks and retains damage context.
Original source is Dahel; target is Harbesh. Both captures verify the canonical
MD5 and retain generated JS, raw/normalized traces, comparison and input hashes.
The Python baseline extraction was subsequently factored into `actor_inputs`
and tested without changing the generated actor/context values.

Earlier `.arc/*-weapon-reference/` DIFFERENT and UNPROVEN runs remain preserved
as diagnostics. They do not establish a like-for-like engine discrepancy.
Native and original transactions record restored=true and no cleanup errors;
no damaged party is saved and temporary fixtures are removed.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x73D560 0x529F40 --profile scripts/profiles/combat-base-reference.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --ours .arc/2026-10-03T13-24-28-669886-batch/profile-runtime/ui.jsonl
.\.venv-reagent\Scripts\python.exe -m unittest scripts/tests/test_weapon_damage_reference.py
```

Five harness tests reject missing/inconsistent contexts, overlapping actor
positions and PASS captures without the expected damage observations. Next
combat comparison: matched actors through normal Attack/Swing, including weapon
payloads and ammo depletion. CProjectile packing/virtual findings remain a
separate recovery task requiring binary context and runtime validation.
