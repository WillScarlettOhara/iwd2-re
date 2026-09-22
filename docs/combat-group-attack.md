# GroupAttack target and queued action — 2026-09-22

`CGameSprite::GroupAttack` (0x74FC50) had two recovery errors: it derived the
next target selector from the attacker, and queued Attack (3) instead of
AttackNoSound (98). The function now copies the resolved target's AI type,
clears its name and instance, requeues the current group order, then inserts
AttackNoSound at the head so it executes first.

## Binary evidence

The full recovery bundle is `tmp_groupattack_ctx.md` with its provenance
manifest. At 0x74FC6F ECX receives the target argument; after the three activity
guards, 0x74FCBB calls its GetAIType vtable slot +0x10. ESI still holds the
attacker. At 0x74FD1D the destination selector is attacker+0x4B4; 0x74FD26
sets the copied selector's instance to -1. The preceding CString operations
clear its name.

0x74FD3E queues the current action. 0x74FD41 reads the WORD at 0x847838;
`sym.py bytes 0x847838 2` returns `62 00` (98, AttackNoSound). The second
AddAction call is at 0x74FE67. AddAction inserts at the head of the queue.
The constants at 0x8485C4/0x8485C6 are -1 (done) / 1 (interruptible).

## Runtime test scope

`combat-group.json` loads the preserved test save, recenters the camera and
requires two visible party members. `combatgroup` calls GroupAttack directly
with a deliberately distinct target classification, name and instance.
It checks null/inactive/AI-inactive/imprisoned/interrupted targets, guard
precedence, the normalized target on both queued actions, their order and
specific arguments. It restores all temporarily changed fields and removes
only the newly queued actions before any further game tick.

This direct test does not execute those queued attacks or prove damage. At this
stage the GroupAttack branch in ExecuteAction was still missing; the follow-up
below restores it. No matched original/reconstruction runtime capture is attached.

## Validation

The preliminary run `.arc/2026-09-22T21-27-52-981955-batch/` is retained as
FAIL: its initial lint pass caught two narrow CString literals in the new
test. They were corrected before compilation. The build and runtime passed
(14 assertions, eight scenario steps, seven GroupAttack entries, CLEAN 60 s),
but that does not override the initial lint failure. A full rerun validates
the final unchanged source.

Call-count parity reports YELLOW (24 binary calls versus nine source calls).
The member-offset audit is non-analysable; neither this warning nor a clean
runtime is evidence of complete binary equivalence.

Final run `.arc/2026-09-22T21-31-00-054410-batch/`: PASS with the parity
warning above. All eight shared source checks, class/layout audits and the
VS2019 Win32 build passed. All 14 assertions and eight scenario steps passed;
GroupAttack executed seven times during a CLEAN 60-second world observation.
Profile cleanup restored the preserved paths and verified their hashes.

## ExecuteAction dispatch follow-up

The full bundle `tmp_group_dispatch_ctx.md` and its manifest cover ExecuteAction
(0x728F80). The action-94 mapping byte at 0x72B57E is 0x2B; the corresponding
jump-table entry at 0x72B3F4 points to 0x729E07. That block copies ANYONE
(0x8C7748), then replaces only its enemy/ally byte with m_typeAI.GetEnemyOf's
result. It resolves a sprite (type byte 0x31 at 0x84C507) via 0x45C030 and
calls GroupAttack at 0x729E70, even when resolution returns NULL.

The new branch follows that sequence. A non-NULL target takes the ReleaseShare
path at 0x72B2F6, not ReleaseDeny; the epilogue stores the returned SHORT in
m_nLastActionReturn (+0x592) before returning. The rest of ExecuteAction remains
partial.

`combat-group-dispatch.json` runs the original 14 assertions plus seven dispatch
assertions. It temporarily classifies the companion as an enemy and deliberately
puts the attacker itself in the action's target selector: the enemy search must
still choose the companion. The test also checks absent-area/no-target and
interrupted results, the stored return value, and the two queued actions. All
classification changes, the temporary area pointer, return state and actions
are restored synchronously. Queued attacks are inspected and discarded; this
does not assert damage or demonstrate an entire group-combat AI loop.

The next useful step is to exercise the queued AttackNoSound through normal AI
ticks, then compare an equivalent original-binary scenario.

Validation run `.arc/2026-09-22T21-43-57-408325-batch/`: build, source checks,
class/layout audits, 21 assertions and eight UI steps passed. The 60-second
world observation was CLEAN, with 65,946 ExecuteAction entries (all world
actors/actions combined) and ten GroupAttack entries (seven direct plus three
dispatch calls). Preserved paths were restored and their hashes checked.
Parity remains YELLOW: ExecuteAction has 284 binary calls versus 70 source
calls because it is still partial; GroupAttack remains 24 versus nine, with
its member-offset check non-analysable. No differential equivalence is claimed.
