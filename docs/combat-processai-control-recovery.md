# ProcessAI forced action recovery

2026-10-03. Partial recovery of `CGameSprite::ProcessAI` (0x72B9A0).

Recovered blocks: helpless (general state 0x20), active berserk (state 2,
`m_berserkActive`), feebleminded (0x100000), and the normal-state exit latch.
Interruption copies the shared `NULL_ACTION`, clears queued actions without
setting the sequence, drops the path, sets `m_interrupt`, queues the action,
and applies triggers. The idle case still sets the latch and applies triggers.
Berserk preserves ATTACK/ATTACKREEVALUATE and otherwise selects
ATTACKREEVALUATE with LONG_MAX. It deliberately preserves the shared scratch
object selectors, as the original does. The exit cancels the previous forced
action for one tick before normal script/modal processing resumes.

Evidence: full hashed context `tmp_processai_recovery.md`; PE disassembly at
0x72D0F5..0x72D32A and 0x72D816..0x72D917. Constants resolved from the original
PE and existing headers (0x20 is STATE_HELPLESS, not STATE_STUNNED; charm exit
uses EA_CHARMED_PC at 0x847C41). CAIAction assignment retains CString ownership.

Validation run: `.arc/2026-10-03T17-41-15-760412-batch/`, combat-base-reference
profile, ProcessAI and UpdateBerserkActive hooks. ProcessAI parity is YELLOW:
191 original calls versus 85 source calls, and missing floating-point handling.
Offset analysis for ProcessAI is unavailable, not a pass. Full ProcessAI
recovery remains incomplete: morale/dialogue handling, confusion, animal rage,
charm actions, sleeping sequence and sprite script scheduling remain missing.
The ordinary runtime scenario is not branch-specific proof of these forced
states, and no original/recovered differential claim is made for them.

Next: recover the remaining forced-action block and the sprite-specific script
scheduler directly from ProcessAI, then validate forced-state execution.

Final gate: PASS with one warning; Win32 build passed, 60-second runtime hold
CLEAN with both symbols exercised; fixture cleanup and preserved-path hashes
passed. This confirms ordinary runtime integration, not forced-state parity.
