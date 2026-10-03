# Inactive sprite actions

ProcessAI (0x72B9A0) restores the branch at 0x72C7A7..0x72CA0B when any of
m_active, m_activeAI or m_activeImprisonment is false. The full shared ProcessAI
bundle and sprite layout remain the recovery context; original instructions
confirm all three BOOL tests at offsets 0x50A6/0x50AA/0x50AE.

With the world timer active, inactive sprites use the original script scheduler:
unsigned field_44A/id phase modulo 16, nonpositive pauses, low field_3428 bits
clear, and living/dead-latch eligibility. Visible, idle, triggered, deferred or
grace cases select ProcessPendingTriggers; offscreen cases increment deferral.
The dead latch and grace/deferral updates match the active scheduler, but the
frame counter itself is not incremented on this path.

An idle current action pulls the queue. If the initial current action is not
instant (or is network LeaveAreaLua), it advances once and clears interruption
without executing the newly selected action. Otherwise ResolveInstants(FALSE)
processes the instant chain until a noninstant action or network LeaveAreaLua.
It returns before active counters, morale/control processing and common action
resolution. The timer-stopped branch still takes precedence.

AutoUI actiongates adds six cases disabling each activity flag independently,
with ordinary and instant actions. Assertions check actual ordinary suppression,
instant marker execution, queued WAIT(200) selection, cleared interruption and
unchanged frame counter. All activity flags are restored in the disposable
fixture. The scheduler's inactive branch has instruction evidence; this fixture
blocks script scheduling and does not claim inactive-script runtime coverage.
Previous active, paused and cutscene cases remain in the same profile. No save
is written; profile cleanup preserves save/config files.

ProcessAI remains partial. Original/recovered differential equivalence and
network-session behavior are not established.

Validation: .arc/2026-10-03T22-44-02-607310-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 45 assertions
and eight UI steps pass; CLEAN for 60 seconds. ProcessAI hit 93495 times and
ResolveInstants 92657. Profile cleanup restores and hashes preserved paths.
Both targets remain parity YELLOW: ProcessAI's floating-point warning remains;
ResolveInstants reports 17 original/35 source calls. Offset analysis remains
unavailable for both targets, so full static equivalence is unproven.

Next target: earlier ProcessAI bookkeeping and synchronization branches.
