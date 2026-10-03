# ProcessAI dialogue pause recovery

2026-10-03. Recovered the dialogue pause body in ProcessAI (0x72B9A0), after
the preceding morale failure arc recovered its eligibility and shared morale
exit. Evidence: full tmp_processai_dialog.md context bundle, unchanged shared
sprite layout, and PE disassembly 0x72CD75..0x72CF66. Constants at 0x85BBB5,
0x85BBBB and 0x85BBBC are sequence 5/17/9 (die/sleep/twitch).

The eligible branch decrements m_dialogWait, then queues the idle sequence
unless already idle, dying, sleeping or twitching. It shares m_dialogWaitTarget
on THREAD_ASYNCH, computes the direction to its position, and queues a facing
message unless already aligned or in the sleep sequence. The target share is
always released on success; a missing target skips facing only. Finally a
non-null path queues DropPath. The branch skips normal timers/control/scripts
and joins common effect/action resolution. It does not change animation or
direction immediately. Morale exit can independently queue another DropPath.

The existing eligibility requires positive wait, EA below evil cutoff,
no sleeping state, allowed dialogue interruption and no ignore-dialog-pause
flag. Sleep animation and sleeping state are different conditions: an awake
sprite in sleep animation still decrements the wait, while sleeping state
bypasses this branch. Normal sprite-specific script scheduling remains
explicitly unrecovered; this is not a full ProcessAI recovery.

AutoUI dialogwait covers 13 cases synchronously with a party actor and target:
idle/facing messages, aligned idle, die/twitch/sleep sequence exceptions,
missing target, countdown completion, path drop, morale exit with two drops, interruption disabled,
ignore-dialog-pause, enemy cutoff and sleeping state. It checks actual message
types, payloads and countdown, then removes test messages. A temporary path
marker exists only during synchronous calls and is never processed as a path.
The actor's explicit test fields are reset; this is a disposable process, not
a complete snapshot of every ProcessAI side effect. The profile centers the
party and transactionally preserves save/config files without saving.

Runtime checks of our build and static parity are distinct from an original/
recovered differential. Actual asynchronous delivery and visual facing are
outside this bounded scenario.

Validation: .arc/2026-10-03T21-49-46-573320-batch, PASS with one warning.
Eight source checks, class/layout audits and Win32 build pass. Runtime CLEAN
for the 60-second hold, ProcessAI hit 92748 times, all 13 cases and nine UI
steps pass. Countdown completion is observed as wait=0; eligible cases emit
the expected sequence/facing/path message payloads. Cleanup restores and
hashes preserved save/config paths.

ProcessAI parity remains YELLOW (191 original / 145 detected source calls)
and retains the floating-point-sensitive-ops warning for its incomplete body.
The offset oracle cannot analyze this target; that static check is unproven.
No full ProcessAI or binary equivalence claim is made.

Initial run .arc/2026-10-03T21-47-02-317025-batch was UI-FAIL: the test wrongly
expected a SpriteUpdate in the local queue. Its GetCommType is BROADCAST_OTHERS;
AddMessage routes it through Broadcast with send-to-self FALSE, which deletes
it rather than adding it locally. The expectation was corrected to zero local
updates, keeping the latch, both panic bits and two path-drop assertions.
The complete validation above was rerun after the test correction.

Next recovery target: sprite-specific script scheduling in ProcessAI before
common effect/action resolution.
