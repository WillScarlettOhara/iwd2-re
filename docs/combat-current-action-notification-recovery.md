# SetCurrAction current-action-ID notification

Recovered 0x733D77..0x733E31 in SetCurrAction (0x7338E0), using its
previously read full context bundle and PE instructions. Compare the
incoming action with m_currentActionId (+0x7296), not the previous
m_curAction ID. Equal values emit nothing. For actions 108, 176, DIALOGUE
(8), STARTDIALOG (137), and STARTDIALOGUENOSET (198), enqueue their SHORT ID
in CMessageSetCurrentActionId(id, spriteId, spriteId). Other differing
actions enqueue zero only if the tracked ID was nonzero. Dispatch uses
AddMessage(FALSE); the tracked field changes when the message runs.

The original inline action copy precedes this notification. Use the existing
CAIAction assignment operator at that point, preserving all three specifics,
flags, strings and object selectors. Per-action resets remain at entry;
conditional Decode stays later, replacing the previous base-class call
which combined copying and decoding at the tail. The intervening still
missing target/resource blocks are not declared recovered. Existing
CMessageSetCurrentActionId::Run (0x505B60) is unchanged and its deny-lock,
sprite gate and SHORT write are confirmed by PE instructions.

28 new typed cases cover the five special actions with tracked IDs zero,
same and other; ordinary NO_ACTION, WAIT, ATTACK and PLAYERDIALOG (139)
with zero/same/other IDs (NO_ACTION also checks a negative tracked ID);
and REMOVETRAPS-to-DIALOGUE. Every case checks count, delayed tracked-field
write, current action copy, three distinct specifics, flags and cleared
encumbrance latches. Emitted messages check subtype, caller, target,
payload and actual Run result. The combined transition verifies the trap
message precedes the action-ID notification. Fixture state and messages
are restored; no save is written. Earlier 239 cases remain in the scenario.

SetCurrAction remains partial, especially target-marker reservation and
spell/projectile resource cleanup. No full differential equivalence is claimed.

Validation: .arc/2026-10-06T21-39-26-872772-batch, PASS with two warnings.
Eight source checks, class audits, sprite layout and Win32 build pass.
All 267 typed assertions and eleven UI steps pass; CLEAN observation for
60 seconds. SetCurrAction hits 39275 times and the message Run 16.
Profile paths are restored and hashed. Parity remains YELLOW (77/32 and
3/7 original/source calls). Message offset analysis is unavailable; its
layout audit does not recognize bare hexadecimal header comments.
Full static and original/recovered differential equivalence remain unproven.

Next target: SetCurrAction target-marker release at 0x733E31..0x733F00.

That release is subsequently recovered in
[combat-action-target-release-recovery.md](combat-action-target-release-recovery.md).
