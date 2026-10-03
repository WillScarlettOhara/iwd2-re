# Combat interruption of dialogue waits

ProcessAI (0x72B9A0) restores two early bookkeeping rules using its shared full
context bundle and original instructions at 0x72C32E..0x72C358 and
0x72C767..0x72C78E.

A positive dialogue wait is canceled when the head of m_pendingTriggers is
AttackedBy (2) or HitBy (32). Only the first trigger is tested; a later combat
trigger does not cancel the wait here. The existing action/CanAct cancellation
checks run before this test. Trigger objects are neither consumed nor reordered.

The positive non-selection counter decrements whenever the world timer is
active, before inactive-sprite routing. The previous source incorrectly placed
it exclusively inside active-sprite processing. Stopped timers leave it alone.

AutoUI actiongates extends all previous execution cases with a typed counter
assertion (5 to 4 when time runs, unchanged when stopped). Four further cases
use stopped time to isolate dialogue cancellation: AttackedBy head, HitBy head,
unrelated head, and unrelated head followed by HitBy. Existing pending trigger
objects are temporarily moved out and restored; only fixture triggers are
deleted. The disposable test process never writes a save; profile cleanup
preserves save/config files transactionally.

These are bounded recovered rules. Complete synchronization, network ownership
and original/recovered differential equivalence remain unproven.

Validation: .arc/2026-10-03T22-50-25-191373-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 49 case
assertions and eight UI steps pass, including counter checks within the
execution cases. CLEAN for 60 seconds; ProcessAI hit 93397 times and
ResolveInstants 92556. Preserved paths are restored and hashed. Both targets
remain parity YELLOW, with ProcessAI's floating-point warning and
ResolveInstants' original/source call count 17/35. Offset analysis remains
unavailable; full static equivalence is unproven.

Next target: periodic sprite synchronization in ProcessAI and its 0x71D0B0 callee.
