# Matched ammunition depletion transitions — 2026-10-03

This scenario compares direct CheckItemDepleted transitions for arrow/bow,
bolt/crossbow and stone/sling, after the four matched base-damage tests. It
does not fire the final shot or exercise action cancellation. The existing
depletion scaffold in the workspace was completed and validated; no engine
body changes were necessary.

Each ranged family runs these five cases:

| Case | Return | Selected slot / set | Unusable trigger |
|---|---:|---|---|
| PC, no reserve | true | fist 10 / set 0 | depleted resource |
| PC, identical reserve | false | reserve 12 / set 0 | none |
| PC, different resource | true | reserve 12 / set 0 | depleted resource |
| Ally, no reserve | true | melee 45 / set 1 | depleted resource |
| PC, count 65535, identical reserve | false | reserve 12 / set 0 | none |

Every exhausted item is deleted and its slot becomes null. Reserve count stays
7, selected ability stays 0, and emitted triggers have ID 9, the depleted
resource in string1, empty string2 and the source actor as caller/target. The
wrapped-count case confirms normalization to zero before deletion. The ally
case changes EA classification on the same party actor; it isolates the non-PC
branch rather than representing a complete NPC AI scenario. Two available melee
fixtures confirm the stronger weapon is selected when falling back.

Original destroyed items use the game's operator new at 0x7FC95B; remaining
fixtures use explicit constructors/destructors and never get passed to the
game's deleting destructor. The probe reads the live equipment slot after
deletion and never dereferences the deleted item. Original slots, classification,
weapon set, quick buttons, cached ammunition and attack count are restored
before the next world update. The original equipment is reselected to restore
the animation/equip state. Temporary fixture files and save/config state are
owned by the profile transaction.

The first original captures missed nested AddMessage observations during the
direct engine call. Reading the newly appended portion of the actual message
queue instead establishes the emitted payloads; the negative captures remain
preserved. Assembly confirms CPtrList tail +8, node previous +4/message +8
(AddTail 0x7FBE4E), message vtable 0x847BB8, trigger +0x0C and string offsets
+0x5A/+0x5E. The baseline parser requires all 15 transitions and explicit state
restoration; tests reject a consumed reserve and the wrong Unusable resource.

The initial reconstruction run `.arc/2026-10-03T15-26-07-101739-batch/` passed
all runtime checks but failed the source lint for an LPCSTR cast in the test
emitter. That cast was corrected to LPCTSTR. It is retained as a failed run.
Original queue capture `.arc/2026-10-03T13-35-35-400195-weapon-reference/`
matches all 23 observations against that runtime capture: eight base-damage
records and fifteen depletion transitions.

Final reconstruction: `.arc/2026-10-03T15-36-13-739486-batch/`, PASS with three
static warnings. Eight source checks, class/layout audits and Win32 build pass.
All 13 UI steps pass and the 60-second observation is CLEAN: CheckItemDepleted
15 hits, SelectLauncherAmmo 15 hits, EquipMostDamagingMelee 3 hits. Static parity
retains the existing call-count warnings; the offset checks for the latter two
methods are not analysable. Those static limits are separate from runtime proof.

Final original: `.arc/2026-10-03T13-40-13-615204-weapon-reference/`, MATCH 23/23
against the final reconstruction capture. Raw and normalized traces, generated
JS, input hashes and differential reports are retained. Both final transactions
restore save/config files without cleanup errors. No test damage or fixture
state was saved; both spawned game processes are stopped. Nineteen comparator
and fixture tests pass. Temporary actor lock contention now retries on the next
world tick for SHARED/DENIED, with partial locks released and the existing outer
capture deadline still enforced; missing/deleted actors remain errors.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x75D450 0x75E330 0x75DFD0 --profile scripts/profiles/combat-depletion-reference.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --depletion --ours .arc/2026-10-03T15-36-13-739486-batch/profile-runtime/ui.jsonl
```

Next: fire the final arrow, bolt and stone through real Attack/Swing and compare
delivered damage, depletion, replacement and action continuation with the
original. The direct transitions above do not establish those combined paths.

Follow-up: [final shots with identical reserves](combat-finalshot-reference.md)
now match 14 observations. No-reserve/different-resource final shots remain open.
