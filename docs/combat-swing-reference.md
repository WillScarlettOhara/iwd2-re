# Matched Swing rounds — 2026-10-03

The reconstructed and canonical original executables agree on 16 observations:
four base weapon damage descriptors, four base projectile HP deltas, four direct
Swing round summaries and four HP deltas including weapon/launcher payloads.
This arc adds a comparison harness and scenario; no engine body changes.

| Fixture pair | Base HP loss | Swing HP loss | Remaining weapon usage |
|---|---:|---:|---:|
| ZRCMELEE | 5 | 6 | 3 (unlimited melee ability) |
| ZRCARROW / ZRCBOW | 9 | 12 | 2, from 3 |
| ZRCBOLT / ZRCXBOW | 9 | 12 | 2, from 3 |
| ZRCSTONE / ZRCSLING | 9 | 12 | 2, from 3 |

Every round takes 102 direct calls, ends with attack frame -2, returns
ACTION_INTERRUPTABLE and leaves no pending projectile. The fixtures add one
damage point through the weapon ability and two through ranged launchers.
Original ApplyEffect diagnostics retain these separate amounts; the normalized
comparison checks their total delivered HP loss.

The scenario centers the loaded save before selecting the actors. The original
party order differs, so the probe matches world positions, then requires the
same target resistance, level, maximum HP and difficulty context. Source stats
are controlled: luck 19, strength 10, one attack, no feats/rogue level/class mask,
and SHORT_MAX hit bonus. The original target's retaliation list and weapon
immunity list must already be empty. Temporary equipment pointers, value-only
stats, buttons and attack state are restored before queued effects execute.
Owned strings and STL containers are never restored through raw byte copies.

Binary checks confirm Swing at 0x739B60, equipment selectors +0x4BA4/+0x4BA6,
combat state offsets, attack count +0x93A, RecordWeaponUse's four 0x0E entries,
and ApplyOnHitSpells' sentinel at +0xDB4. CheckAttackHit at 0x73B760 compares
ability +0x14 against SHORT_MAX at 0x84CEC6 and returns true without rolling.
The action constants at 0x8485C6/CA are 1/-2.

Reconstruction: `.arc/2026-10-03T14-26-06-452181-batch/`, PASS with two warnings
and one static skip. Source checks, class/layout audits and Win32 build pass.
All 16 UI steps pass; the 60-second runtime observation is CLEAN. Observed hits:
Swing 408, CreateWeaponDamage 8, ApplyWeaponEffects 3. Swing and damage factory
retain previous parity warnings; ApplyWeaponEffects parity is GREEN but its
offset check is not analysable. These are distinct from runtime comparison.

Original: `.arc/2026-10-03T12-29-46-282976-weapon-reference/`, MATCH 16/16.
Input hashes, generated original JS, raw/normalized captures and differential
report are retained. Both transactions restored files without cleanup errors.
No test damage was saved and both spawned game processes were stopped.
Ten harness tests pass, including rejection of missing rounds, unchanged
ammunition and missing payload damage.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x739B60 0x73D560 0x71F860 --profile scripts/profiles/combat-swing-reference.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --rounds --ours .arc/2026-10-03T14-26-06-452181-batch/profile-runtime/ui.jsonl
```

This establishes controlled direct Swing behavior. It does not establish
ordinary hit/miss rolls, critical hits, normal AI timing, ExecuteAction/Attack
parity, depletion of the final ammunition unit, or nonempty retaliation and
immunity lists. Next: matched ExecuteAction/Attack rounds on these same actors
and fixtures, followed by ordinary AI-driven attacks and final-unit depletion.

Follow-up: [ExecuteAction/Attack comparison](combat-attack-reference.md) now
matches 16 observations, including AttackOneRound completion and ammunition.
