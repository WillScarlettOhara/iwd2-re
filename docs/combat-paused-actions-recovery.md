# Paused action preview

ProcessAI (0x72B9A0) restores destination preview selection in the timer-stopped
path at 0x72DD1B..0x72DE32. Full ProcessAI context and the ResolvePausedAction
(0x72B670) bundle were read; the latter's existing body is unchanged.

Without interruption, with protected base flags bit 0x80000000, or with both
command pauses nonpositive, ResolvePausedAction receives the current action
and queue head. Otherwise a nonempty queue supplies its first action and the
remaining queue position, without consuming it. An empty queue skips preview.
Subsequent interruption advances the actual current action unless protected.
The final unconditional destination recalculation is removed from this path:
the binary returns after optionally resolving one instant action with FALSE.

AutoUI's disposable actiongates fixture adds five stopped-timer execution
cases and three preview cases, alongside its previous 22 active-timer cases.
It checks ordinary action suppression, instant execution with held/caster-hold/
time-stop states, queued preview, protected current preview, and current preview
without positive command pause. Real ProcessAI and ResolvePausedAction calls
drive marker, current-action and interruption assertions. Marker and group-move
state are restored. No save is written; profile cleanup preserves save/config.

Initial .arc/2026-10-03T22-27-48-899217-batch failed one fixture assertion.
The test incorrectly used userCommandPause, which clears protection at the
start of ProcessAI. It now uses the internal command pause to retain protection.
This failure was not a crash and is not successful runtime evidence.

The active cutscene fast path was subsequently removed by
[the cutscene action recovery](combat-cutscene-actions-recovery.md).
This recovery does not establish all paused-action effects, network behavior,
or original/recovered differential equivalence.

Validation: .arc/2026-10-03T22-30-38-012573-batch, PASS with two warnings.
All eight source checks, class/layout audits and Win32 build pass. All 30
assertions and eight UI steps pass; CLEAN for 60 seconds. ProcessAI hit 93279
times and ResolvePausedAction 109. Preserved paths were restored and hashed.
Parity remains YELLOW: original/source call counts 191/165 and 11/21;
ProcessAI retains its floating-point warning. Offset analysis remains unavailable,
so static equivalence is unproven.

Next target: inactive-sprite routing in ProcessAI.
