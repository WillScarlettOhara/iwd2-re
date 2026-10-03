# Periodic fatigue

CGameSprite::CheckFatigue (0x71D0B0), named by BG2 PDB, is recovered from its
full context bundle and original instructions 0x71D0B0..0x71D307. Sprite/base/
derived layouts are shared with the preceding ProcessAI recoveries. The
original time-scale bytes 15/60/5 give a four-hour period of 18000 game ticks.

Unsigned elapsed-time subtraction/division is converted to LONG before signed
comparison with base fatigue. Increasing fatigue constructs opcode 93 with
amount equal to the elapsed periods, flags 1 and permanent duration type 1.
Effect immunity and the active-opcode set can suppress submission. Otherwise
CMessageAddEffect receives ownership, signed-byte atoi of fatigue-table column
0 adjusts derived luck, nonzero modifiers add portrait icon 39 only once,
and fatigue above 6 queues sound 3 with text and circle enabled.

Lower elapsed fatigue corrects the rest epoch to current time minus base
fatigue times the period; base fatigue above 6 still queues the sound. Equal
fatigue does nothing. The original suppression branch does not delete its
decoded effect; this original ownership quirk is retained, not silently repaired.

ProcessAI invokes CheckFatigue for a party portrait at matching unsigned
frame/id modulo 2051, before timer-stopped and inactive routing. The subsequent
counter/action gates retain their ordering.

AutoUI fatiguechecks adds seven cases: equality, just below the period boundary,
increase, sound thresholds 6 and 7, and timestamp correction with/without sound.
It inspects queued effect/sound payloads, applies the recovered fatigue effect
to observe the base value, checks luck/icon behavior and restores base fatigue,
epoch, luck, game time and any newly added icon. It deletes fixture messages
before normal dispatch. The process is disposable and no save is written;
profile preservation restores save/config files. Immunity suppression and
network delivery are not covered by these cases. Previous 52 action assertions
remain in the same scenario.

Two additional ProcessAI cases force matching and nonmatching modulo-2051
phases with stopped time. Epoch correction proves the periodic call occurs
only at the matching phase, independently of direct CheckFatigue fixture calls.

Initial .arc/2026-10-03T23-09-28-101767-batch passed the seven direct fatigue
cases and 52 previous assertions, but CheckFatigue's seven hits were all direct
fixture calls. Dispatch assertions were added and complete validation rerun.

The first dispatch run, .arc/2026-10-03T23-16-38-113760-batch, passed all nine
fatigue assertions but subsequently crashed in GetVisualRange via
UpdateBerserkActive with a null area. The phase fixture had left its synthetic
time at 1000000 during a complete ProcessAI call, potentially expiring unrelated
effects. It now restores actual saved game time before those calls and only
changes the fatigue epoch. This failure is not a clean runtime validation;
the crash's underlying broader AI behavior is not declared fully diagnosed.

Static checks, recovered-build cases and original/recovered differential
equivalence remain distinct evidence. Full ProcessAI is still partial.

Final validation: .arc/2026-10-03T23-20-11-426812-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. The 52 previous
cases, seven direct fatigue cases, two phase cases and nine UI steps pass.
CLEAN for 60 seconds; ProcessAI hit 53462 times and CheckFatigue eight,
including the matching-phase call. Preserved paths are restored and hashed.
Both functions remain parity YELLOW: original/source calls 191/202 and 14/23;
ProcessAI retains its floating-point warning and unavailable offset analysis.
Complete equivalence remains unproven.

Next target: remaining early ProcessAI action replacement and movement bookkeeping.
