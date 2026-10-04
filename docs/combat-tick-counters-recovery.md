# Poison sound and command pause counters

ProcessAI (0x72B9A0) corrects the early decrement at 0x72BBAD..0x72BBC3.
Its shared full context bundle and original instructions load/test/store the
signed SHORT at sprite offset 0x72A0, m_poisonSoundTimer. Only positive values
decrement. The previous source incorrectly decremented m_nCommandPause instead.

The early poison timer runs before stopped-time and inactive routing. Command
pause retains its single decrement on the normal active path at 0x72CA2C.
Consequently a command pause of 5 remains 5 in paused/inactive cases and becomes
4 in normal active processing, rather than becoming 3 from two decrements.

AutoUI tickcounters adds seven cases: positive, zero and negative poison timers
with active/stopped time, plus an inactive sprite with positive poison timer.
Actual ProcessAI calls check both typed counters. Script scheduling is blocked
to isolate the decrements; a WAIT current action avoids idle-command reset.
Counter, activity, state/spell and timer fields are restored in a disposable
process. No save is written; profile cleanup restores save/config files.
The preceding 65 action/fatigue assertions remain in the same scenario.

This does not establish poison damage or all sound playback behavior. Full
ProcessAI and original/recovered differential equivalence remain unproven.

The first run .arc/2026-10-04T21-41-31-908704-batch passed all 72 assertions
but crashed during observation in GetVisualRange via UpdateBerserkActive on
a null area. The original branch 0x72C125..0x72C1EB was still absent: dead
sprites without an area process effects, then either advance an initial
ordinary action and clear interruption or resolve their instant chain with
FALSE. They return before normal control processing. This branch is recovered
in the same lot, without adding a general invented null-area guard.

An eighth new case temporarily detaches a dead test actor from its area during
a synchronous ProcessAI call. It asserts queued WAIT(200) selection without
execution or interruption and restores the area and living animation/state
before the next engine tick. Instant and network exceptions have instruction
evidence but are not separately runtime-tested in the detached fixture.

Final validation: .arc/2026-10-04T21-45-27-222341-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 73 case
assertions and eleven UI steps pass; CLEAN for 60 seconds. ProcessAI hit 88331
times and ResolveInstants 87833. Preserved save/config paths are restored and
hashed. Both targets remain parity YELLOW (191/226 and 17/35 original/source
calls); ProcessAI retains its floating-point warning. Offset analysis is
unavailable for both, so full static equivalence remains unproven.

Next target: missing movement/animation bookkeeping before the action-control path.
