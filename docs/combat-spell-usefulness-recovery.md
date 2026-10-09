# Scripted spell usefulness and selection

This lot restores two complete helpers and their original script branches:
CGameSprite::CheckSpellUseful (0x764880), CGameAIBase::CheckTargetVisibility
(0x44DAC0), IsSpellTargetValid in EvaluateStatusTrigger (0x731B30), and action
0x141 in ExecuteAction (0x728F80). The helper names are inferred and marked
#guess. The two large dispatchers still contain unrecovered cases.

Full recovery bundles were read before implementation:
tmp_combat_spell_permission_ctx.md and tmp_spell_los_ctx.md, then the named
bundles after address-map updates. The full caller functions, required call
sets and shared BG2 layouts were read. The previously read CGameSprite header
was reused with matching shared-block fingerprints; this lot adds only one
declaration. The complete CGameAIBase header was read. These fingerprints
establish context provenance, not parity.

PE instructions establish the argument widths and ordering. The usefulness
query returns with ret 0xC, reads spell level as a byte and caster type/failure
as signed shorts, and only treats disabledSpellTypes entries equal to one as
disabled. Silence checks the caster, with vocalize, feat 68 and numeric spell
2219 exemptions. Outdoor requirements read the caster's area. Resource and
level immunities read the target. Level-one entries in each of the innate,
song and shapeshift catalogues bypass level immunity.

The last eligible ability supplies effectCount, but PE 0x764DFA passes ability
zero to BuildAbilityEffect. This original behavior is preserved. Temporary
effects are decoded, inspected and discarded without applying them. The full
opcode/state table rejects redundant target effects, including general states,
spell states, either enfeeblement state, red/blue fire shields, both righteous
wrath states, arbitrary opcode-288 states and nonzero immunity selectors.
Unlisted effects remain eligible. Flags do not bypass the usefulness query.

Visibility uses same-area identity, the target's stealth and derived death
states, independently divided search-grid coordinates and two virtual visual
range calls. CheckLOS receives target then caster positions and the target's
Orderable(FALSE) result as its explored argument. Dead targets may be allowed;
stealth is still rejected. Full 0x44DAC0 instructions through ret 8 were read.

The trigger branch at 0x732C40..0x732C9D combines CheckSpellTarget and
CheckSpellUseful. It preserves the original unreleased ASYNC share; that
object-array thread does not change reader counts. The action table byte at
0x72B661 maps action 0x141 to case 0x73, beginning at 0x7297DC. Its flags are:

| Bit | Original behavior |
| --- | --- |
| 1 | Bypass visibility/range/LOS |
| 2 | Bypass target permission and usefulness |
| 4 | Start at a random list entry, wrapping once |
| 8 | Bypass memorized-spell availability |
| 0x20 | Permit processing without a resolved target |

Strings contain consecutive four-character decimal IDs. Malformed lengths
return ACTION_ERROR; useful available candidates populate field_588 and
field_3BA. An existing selection is retained. A null target requires both
target and visibility bypasses and stores CAIObjectType::NOONE. Failed
resref conversion exits directly without updating m_nLastActionReturn.

spellusechecks runs real dispatcher calls on two loaded party members. It
pins installed SPL resources and temporarily substitutes typed header,
ability and effect tables, testing every recovered opcode mapping and the
ability-zero quirk. It restores actor state, spell lists, catalogue pointers,
target markers, selection fields and resource tables in the same tick, and
checks HP, message count, active opcodes and resource reference balance.
The scenario then repeats the existing melee/bow/crossbow/sling HP and
ammunition regressions, plus opcode-430 projectile delivery. It centers the
loaded camera on the party and never saves the disposable process.

Run the complete gates with:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x764880 0x44DAC0 0x764810 0x731B30 0x728F80 --profile scripts/profiles/combat-spell-usefulness.json
```

The initial batch failed lint and compilation and is not passing evidence.
CString literals/casts and the test's missing CPathSearch include were fixed
before the final batch.

Final batch 2026-10-09T18-06-47-954078-batch passes with three parity warnings:
1,168 assertions (279 new), 35 scenario steps, five of five PDB symbol gates
and a 60-second CLEAN hold. CheckSpellUseful executes 254 times,
CheckTargetVisibility eight, CheckSpellTarget 48 and EvaluateStatusTrigger
four. ExecuteAction executes 93,068 times, including the isolated selection
cases. All 279 spelluse-case results pass. Native layout checks all 310
CGameSprite members; class/vtable audits find no issues. Eight shared source
checks and the complete Win32 Debug rebuild pass.

CheckTargetVisibility and CheckSpellTarget parity are GREEN. CheckSpellUseful
has 82 original versus 78 source calls (YELLOW); the incomplete trigger and
action dispatchers have 139/9 and 284/96 calls respectively (YELLOW). These
warnings and optional skips do not establish whole-function equivalence.

The melee fixture removes six target HP; arrow, bolt and sling fixtures each
remove twelve HP and one charge. On-hit melee removes seven target and two
source HP. Each of the instant/arrow/bolt/sling opcode-430 deliveries removes
three target HP and zero source HP through ordinary ticks. Transaction
cleanup reports restored=true and no errors, restoring saved/config paths
and removing fixture overrides. Evidence is retained under the batch's
profile-runtime directory, including ui.jsonl and transaction.json.

Limits: this is PE-guided recovery plus direct reconstructed-runtime coverage,
not a matched original/reconstructed differential capture. The original
caster-type indexing has no bounds guard; tests cover types one and two.
Invalid opcode-288 bit indices and empty random-start lists retain the
original exceptional/invalid-input behavior and are not runtime exercised.
Optional skipped binary-offset/layout analyses remain unproven. Continue
the remaining selected-spell trigger/action branches in EvaluateStatusTrigger
and ExecuteAction, then the connected casting path.
