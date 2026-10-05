# ProcessAI destination marker tail

Removed the source-only ResolveTargetPoint call after normal action
resolution in ProcessAI (0x72B9A0). Its shared full context bundle and
original tail instructions confirm that 0x72DE1A restores registers/SEH
and returns; no destination-marker call occurs there. Normal action
resolution also reaches this common return.

The existing paused ResolvePausedAction calls remain unchanged. They
select the current action or queued preview according to interruption
and command protection, then resolve the marker unless m_groupMove is set.
The removed tail had erased a sentinel marker on an active ordinary WAIT,
despite neither the binary tail nor WAIT requiring that write.

Four typed AutoUI cases call actual ProcessAI with current WAIT(100):
active time preserves the marker with either group setting; paused time
clears it to (-1,-1) without group movement; paused group movement preserves
it. Every case also verifies that the WAIT action remains current. Script
scheduling is blocked to isolate the call, without changing game time.
Existing command-protection preview assertions remain in the same scenario.
The fixture restores marker/group/timer state and never writes a save.

Validation: .arc/2026-10-05T20-49-08-772473-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 102
case assertions and eleven UI steps pass, followed by CLEAN observation
for 60 seconds. Both ProcessAI and ResolvePausedAction are hit. Save/config
paths are restored and hashed. Parity remains YELLOW (191/240 and 11/21
original/source calls); offset analysis is unavailable, so full static
equivalence remains unproven. This establishes the local marker-write timing, not
full ProcessAI equivalence or every destination-marker lifecycle.

Next target: remaining active-action scheduling and interruption branches
inside ProcessAI.
