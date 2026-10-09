# Marked spell state and preparation delay

Recover four cases in CGameSprite::EvaluateStatusTrigger (0x731B30) and three
in ExecuteAction (0x728F80). Both dispatchers remain partial. The existing
CGameAIBase::Wait (0x4600B0) and SpellIdToResRef (0x45D3E0) are dependencies,
not newly recovered functions. The latter's address marker was moved beside
its definition so arc can resolve and instrument it; its behavior is unchanged.

The full tmp_combat_selected_cast_ctx.md bundle was assembled before edits.
Its complete original dispatcher exports match the input hashes previously
read in tmp_combat_spell_permission_ctx.md (f2d27494 for 0x731B30, 5dee122f
for 0x728F80); their full bodies are reused, and this lot's blocks and required
call sets were read again with current resolved names. The shared BG2 layout
47cbd251 and CGameSprite header (file SHA256 46e67910) match the complete
previously read blocks. No class header/member layout changes are made.

PE disassembly from each function entry establishes valid boundaries.
HaveSpell begins at 0x731C09, IsMarkedSpell at 0x731C8F, SetMarkedSpell at
0x731CA6 and ForceMarkedSpell at 0x731D36. The selector bytes at 0x72B655,
0x72B657 and 0x72B65F select cases 0x6E, 0x70 and 0x72. Their jump-table
entries lead to 0x7296FA, 0x72AEE4 and 0x7297AC. The misaligned prefix of
a separate quick view at 0x72B100 was discarded; the entry-based disassembly
establishes 0x72B104 and subsequent instructions. No misaligned instructions
are recovery evidence.

HaveSpell uses the caller's memorized spell lists, class mask 0xFFF, level
selector nine and availability flag true. ID zero resolves field_588; with
both zero it returns false. IsMarkedSpell compares the full numeric ID,
including zero. SetMarkedSpell always returns true: zero clears the marker,
an existing marker is retained, and an empty marker is populated only when
the requested spell is available. ForceMarkedSpell replaces it without an
availability check. Actions 0x135 and 0x13F implement the same conditional
and forced writes respectively, returning and storing ACTION_DONE. Numeric
marking does not alter the stored target in field_3BA or consume a spell.

Action 0x137 fills a zero numeric spell argument from field_588, resolves the
SPL, demands it and chooses the last ordered ability whose minimum caster
level is no greater than the signed SHORT caster level. It stops at the first
ineligible ability. Empty resref returns ACTION_ERROR directly without
updating m_nLastActionReturn; demand failure or no eligible ability completes.

The WORD ability speed and signed SHORT derived mental speed (+0x9E4) use
the exact 0x72B166..0x72B18D arithmetic:

```
delay = max(abilitySpeed - mentalSpeed, 0)
wait = SHORT(delay - 1 - mentalSpeed)
```

Only a positive wrapped SHORT changes the current action ID directly to Wait
(63), replaces its numeric argument with wait and calls the existing Wait
executor. SetCurrAction is deliberately not involved: the original only
writes those two action fields. Wait multiplies by fifteen on actionCount
zero, then decrements; otherwise it only decrements. Resource demand,
request and reference counts return to baseline.

spellusechecks adds 185 assertions to the earlier 279. It covers zero,
unavailable, retained, forced and full-width markers; known-but-exhausted
spells; all nine list levels and all seven casting classes; silence and
target-state independence; selected resource/target decoding; seventeen
speed/mental-speed boundary pairs; explicit versus selected IDs; no/too-low
abilities, ordered selection and fourteen normal Wait dispatches. Expected
boundary results are a literal table, not a second implementation of the
production formula. Actor spell lists, marker/target, action/count, mental
speed and pinned SPL tables are restored before the tick returns.

Validation command:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x731B30 0x728F80 0x4600B0 0x45D3E0 --profile scripts/profiles/combat-marked-spell.json
```

The first batch 2026-10-09T18-24-21-782008-batch passes 1,353 assertions and
35 scenario steps, including actual melee/bow/crossbow/sling HP/ammo and
projectile regressions. It observes three symbols for 60 seconds CLEAN, but
does not instrument SpellIdToResRef because it captured its old address-marker
position. It is not complete four-target runtime evidence. A final batch
with the moved marker and final source is required and recorded below.

Final batch 2026-10-09T18-29-26-293624-batch passes 1,353 assertions (185 new),
35 scenario steps, all four symbol gates and a 60-second CLEAN hold.
EvaluateStatusTrigger executes 78 times, ExecuteAction 93,105, Wait 3,577 and
SpellIdToResRef 354. All 464 spellusechecks results pass. Eight shared source
checks, class/vtable audits, the 310-member native sprite layout and Win32
Debug build pass. Transaction cleanup restores all preserved paths, removes
fixtures and reports no errors. Melee removes six target HP; bow, crossbow
and sling each remove twelve and consume one charge, retaining the earlier
on-hit and opcode-430 damage results.

Wait parity is GREEN. The partial sprite dispatchers remain YELLOW with
139/15 and 284/108 original/source calls. SpellIdToResRef remains YELLOW
(14/1 calls and low-call warning); Ghidra's unqualified name also differs
from the source's class-qualified name. No suppression or parity-baseline
exception was added. These warnings and skipped analyses are not proof of
whole-function equivalence.

Limits: runtime coverage is on the reconstruction. No matched original and
reconstructed capture is claimed. The dispatchers remain incomplete and the
existing conversion helper still omits invalid-family diagnostic assertions;
forced invalid numbers are only tested as marker values, not converted.
Missing-resource demand failure is not exercised. Optional skipped offset
analyses remain unproven. The subsequent [target-resolution lot](combat-target-resolution-recovery.md)
recovers 0x733340, target-marker cleanup, sprite immunity and formation movement.
SetSpellTarget (0x40F4) in the base status-trigger dispatcher remains next.
