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

## Continuation: confusion, animal rage and charm

Recovered the remaining forced-action branches from 0x72D32A..0x72D816.
Confusion renews the shared scratch action on the unsigned id/frame modulo-100
phase: rand()%10 selects action 184 for 0..2, NULL_ACTION for 3..5, or GROUPATTACK
with ANYONE for 6..9. Off-phase ticks still latch control and apply triggers.
The GROUPATTACK case deliberately keeps the existing scratch destination.

Animal rage exempts action 149, preserves a suitable action 134 target, and
otherwise queues action 134 against the opposite EA cutoff with specifics
45/-1/0. Neutral EA values between the two cutoffs retain their action. The
spell-state-179 and charmed-PC transitions use the NULL_ACTION cancellation
path. The normal-state latch exit now follows the complete forced-state chain.

Corrected the previous berserk block: the original reads 124 at 0x84786C,
not CAIAction::ATTACKREEVALUATE (134 at 0x847880). Existing constants elsewhere
are unchanged. Action IDs 124, 149 and 184 remain numeric until their names
are established; no name is inferred from another engine's action table.

Evidence: unchanged full-context bundle hash
32fe38a26e888dc387147a33e2479e63e4e97e253a1d021291316b1bc282aa33;
PE disassembly and two-byte action values, existing CAIObjectType::ANYONE
at 0x8C7748, existing EA cutoffs, and typed action/object ownership.
The remaining ProcessAI gaps include morale/dialogue handling, sprite-specific
script scheduling and sleeping sequence handling. Ordinary smoke execution
is not a differential proof of forced-state behavior.

Next dependency: ExecuteAction remains partial. Its dispatch for actions
124, 134 and 184 must be recovered before claiming end-to-end berserk, animal
rage or confused wandering. GROUPATTACK dispatch already exists.

Continuation validation: .arc/2026-10-03T18-10-30-824159-batch, PASS with one
warning. Win32 build OK, 60-second hold CLEAN, both symbols exercised, cleanup
restored preserved paths. ProcessAI parity YELLOW (191/105 calls), offset
analysis skipped; forced-state branches remain dynamically unproven.

Executor continuation: see combat-berserk-executor-recovery.md. Action 124 now
has a recovered Berserk executor and a named BERSERK constant. Actions 134/184
remain missing.
