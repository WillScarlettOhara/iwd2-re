# Separate ProcessAI and ResolveInstants action scratch

ProcessAI (0x72B9A0) now uses its own function-local static CAIAction,
matching 0x8F9928 in the shared full context bundle and PE instructions.
It previously reused CGameSprite::m_aiDoAction, mapped to 0x8F9A18.
All ProcessAI GetNextAction outputs and controlled-action construction
now use the separate scratch. This includes inactive/dead/paused selection,
morale and imposed actions, and normal active queue selection.

ResolveInstants (0x728BC0) correctly retains m_aiDoAction. Its full bundle
and instructions at 0x728DB3/0x728DDB pass 0x8F9A18 to GetNextAction after
interruption or terminal execution. PE scan finds its constructor at
0x7281F0 calling CAIAction's default constructor. ProcessAI's separate
inline initializer at 0x72B9D9..0x72BAA4 initializes a distinct static
action. Both are output buffers, not fallback actions: GetNextAction
overwrites its argument from the queue or NULL_ACTION.

Four typed AutoUI cases call actual ProcessAI with queued WAIT(200):
paused idle selection and paused interruption preserve a 901/902/903
canary in m_aiDoAction; active WAIT interruption and completion update
that scratch to WAIT(200). Every case verifies the selected WAIT remains
unexecuted and interruption is cleared. The canary is saved/restored
using assignment to avoid the original action-copy specific3 quirk.
Rule/timer/action state is restored in the disposable process, without
advancing game time or writing a save.

Validation: .arc/2026-10-05T21-04-18-483351-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 106
case assertions and eleven UI steps pass; CLEAN observation for 60 seconds.
ProcessAI hits 93395 times and ResolveInstants 93228. Save/config paths
are restored and hashed. Parity remains YELLOW (191/240 and 17/35
original/source calls); offset analysis is unavailable. This establishes separation of these two action
buffers, not full ProcessAI or original/recovered differential equivalence.

Next target: remaining direct action interruption and terminal return
cases in ResolveInstants.
