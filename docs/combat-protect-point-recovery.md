# Hostile ProtectPoint replacement

ProcessAI (0x72B9A0) restores the early branch at 0x72C358..0x72C5B6, using
its previously read full context and original instructions. Action data
0x8477B8 is ProtectPoint (27); 0x847C42 is the hostile cutoff 200.

For current ProtectPoint and virtual GetAIType EA >=200, ClearActions(FALSE)
removes the queue while preserving the current action. The branch queues
CMessageDropPath followed by CMessageAddAction carrying NO_ACTION. Its payload
has specifics 0/-1/0, destination (-1,-1), default selectors/strings and zero
internal flags. The original uses assignment (0x405090), so the unrelated
copy-constructor specific3 quirk does not apply. Current action and interruption
are changed later by normal message delivery, not synchronously by this branch.

AutoUI protectchecks uses stopped time to isolate this path. Four cases cover
EA199, EA200, EA255, and a different action at EA255. Assertions inspect the
remaining queue, retained current action, drop-path count and complete relevant
NO_ACTION payload. Fixture messages are deleted before dispatch and actor EA,
selection, marker/target/group-move state and timer state are restored. The
process is disposable, with no save writes and transactional profile cleanup.

This does not establish network delivery, later message execution or complete
ProcessAI equivalence. Existing combat/action/fatigue assertions remain in the
same profile.

Validation: .arc/2026-10-04T21-30-14-824819-batch. Build, eight source checks,
class audits and runtime pass; all 65 assertions and ten UI steps pass. CLEAN
for 60 seconds; ProcessAI hit 92845 times and AddAction's constructor twice.
Profile cleanup restores and hashes preserved paths.

Overall arc verdict remains FAIL because the unchanged AddAction constructor
is parity RED: large ASM/tiny source, four-line-body warning and 10/0 detected
calls. Complete disassembly 0x4F4F90..0x4F5128 shows base/member initialization
and inline assignment, corresponding to the existing CMessage base initializer,
CAIAction member construction and m_action = action. No padding or suppression
was added to appease the heuristic. This manual inspection and observed payloads
do not convert the static oracle into a PASS. Ghidra additionally names this
address CMessage. ProcessAI remains YELLOW (191/212 calls and floating-point
warning); its offset analysis is unavailable. The message layout audit is skipped.

Next target: remaining initial movement and animation bookkeeping in ProcessAI.
