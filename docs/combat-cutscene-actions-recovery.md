# Active cutscene actions

ProcessAI (0x72B9A0) no longer sends m_inCutScene sprites through the paused
action shortcut. Full ProcessAI context remains the shared recovery basis.
The original function references sprite offset 0x574 only to clear it at
0x72C5FB when no current action remains after pulling the queue; it does not
branch on that flag to bypass normal active processing. The same idle branch
zeros the internal command pause. The timer test at 0x72CA10 selects paused
processing; active action resolution remains at 0x72DAC5 onward.

The removed shortcut resolved an action regardless of caster hold, helpless
states, time-stop, death or dialogue gates, and skipped active counters and
control processing. Cutscene actors now use the existing recovered active
path. CheckMorale and ProcessPendingTriggers retain their own cutscene gates.
Stopped timers still use one instant ResolveInstants(FALSE) call.

AutoUI actiongates adds eight cutscene cases: ordinary and instant actions
under normal, Held, caster hold and other-caster time-stop states. One further
case checks that an idle sprite leaves cutscene mode and clears command pause.
The preceding 30 action/paused-preview assertions remain in the same run.
The fixture uses actual ProcessAI calls in a disposable process and never
writes a save; save/config preservation is transactional.

This removes a known incomplete shortcut, not all remaining gaps in ProcessAI.
Inactive-sprite routing, complete effect processing and original/recovered
differential equivalence remain outside this recovery.

Validation: .arc/2026-10-03T22-37-49-618714-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 39 assertions
and eight UI steps pass; CLEAN for 60 seconds, ProcessAI hit 93340 times and
ResolveInstants 93222. Profile cleanup restores and hashes preserved paths.
Parity remains YELLOW (original/source calls 191/166 and 17/35); ProcessAI
retains its floating-point warning. Offset analysis remains unavailable for
both targets, so full static equivalence is unproven.

Next target: inactive-sprite routing in ProcessAI.
