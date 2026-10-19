# Level-up allocation panels

Targets: ResetSkillsPanel 5F7850, ResetFeatsPanel 5DA200, ResetAbility 5D9610;
signed skill-read/cost corrections 7642D0/764310 (tmp_skill_signed_context.md).
Full context/shared blocks: tmp_allocation_context.md (+manifest). ASM in
tmp_reset_skills.asm, tmp_reset_feats.asm, tmp_reset_ability.asm and tail
9776..97AA. Canonical MD5 25cb3d8aa2ec648e94e328d8c467973e.

Every nonzero init flag preserves signed HP, base/derived maxima and full CON
bonus across ProcessEffectList, copies the budget and snapshots all 16 skills
or 75 feats. Top scans may finish at 16/75 without clamping. No-init only resets
top to zero; budgets and stored ranks survive. Both write signed number token
and help control 92 (skills 17248, feats 36476), then enable/activate both
buttons for each of nine rows. Skills require nonzero cost and signed budget
>= cost. Feats ignore the budget. Both button loops ignore the top offset.
Native feat scan calls GetFeatId but discards its return, testing the raw row;
buttons use mapped IDs. Preserve this discrepancy. ResetAbility keeps full
signed racial adjustment through the max comparison, then narrows minimum to
BYTE and clamps at one. Race/class minima narrow first; max is always 50,
modifier remains INT and the caller's ability byte is untouched.

allocation_reference.py capture tmp_allocation_native3: PASS 424 cases (256
panels, 168 ability constraints). Targets, costs, rank getters, native feat
selection/prerequisites and numeric ID parsing execute unchanged. Private
stats/tables, rooted callbacks; UI/help/token/CString, effects and race/class
labels are explicit doubles. Effects deliberately perturb HP before restoration.
Help/control order receipts checked separately. No save or main-thread resume;
hooks/globals restored, callback and cleanup errors empty, private PID retired.
Input cells/cases, PE, driver and generated JS fingerprinted in report.json.

AllocationGolden.h and AllocationChecks.h replay with actual GUIREC controls,
real numeric C2DArray lookups and real help displays. Single fighter, costs
0/1/2, signed skill/high-byte feat ranks, selectable simple feats, unavailable/all-owned
feats, reversed maps, budgets -2/0/1/2, flags 0/1/-1/2. This is not a full
feat-prerequisite audit. Effects are deferred in the typed fixture. Actor,
tables, TLK token, engine routing, queues and clock restore in one outer tick;
the active/render engine never becomes the private GUIREC. Equipment untouched.
allocation_compare.py requires the full 92-step profile and CLEAN; compares all
424 ordered allocation-result events, only op->tag rename, no ignored fields.

ARC 2026-10-19T13-22-20-423150-batch PASS: 92/92, 1961 checks, 72s CLEAN.
Hits (target order above): 130/132/168/1536/1746. Exact 424-event pair MATCH;
fingerprints in allocation-pair/. GetSkillCost YELLOW (7 vs 13 calls);
offset/layout skips leave static evidence unproven. Melee 6HP, ranged 12HP each.
Cleanup restored hashes, errors[].
Initial fixture read base enable flags; corrected to button flags. Subsequent
native mismatch exposed signed skill ranks: MOVSX at 7642F6 and 7644EA. The
getter and cost threshold now both interpret raw BYTE 255 as -1, not 255.

Separate investigation: levelup_cache_reference.py, tmp_levelup_cache_native1,
PASS 12 private cases. Unchanged 5D8EA0 reads uninitialized [frame ESP+24] on
cache hit; entry hook seeds ESP_entry-E84. Misses add doubled HP result seven.
Hits with cached 77 skip HP calculation yet add -32768/-10/0/31/62/32767 to
HP 100 (signed WORD wrap); cache stays 77. Code/globals restored, no callback
or cleanup errors. This proves stack dependence, not a deterministic HP policy.
No source recovery of 5D8EA0 or OnDoneButtonClick 5E2210 in this lot; do not
invent a cache-hit value or introduce an uninitialized C++ read. Next work:
remaining level-up stat/final dispatch and class-selection panel reset. Native
cache anomaly is established; do not repeat the same investigation next turn.
