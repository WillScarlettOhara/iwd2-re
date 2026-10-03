# RunAwayFrom runtime path coverage

2026-10-03. Follow-up to combat-runaway-recovery.md; engine code is unchanged.
The new combat-runaway-paths profile uses temporary combat-payload fixtures,
centers the camera, and tests movement plus nearest-enemy retaliation.

AutoUI runawaymove requires an idle actor without a path/search request,
detaches and preserves the existing action queue, installs action 184 with
counter 14, and executes it. RunAwayFrom increments to 15, reflects the
threat position and issues SetTarget. Subsequent real engine ticks must move
the actor with positive displacement away from the threat. The test cancels
its path/search, restores previous action/queue/counters/destination/interrupt
and sequence through engine APIs, and leaves the actually moved position for
this disposable runtime session. The save is never written.

attackdamage now supports actions 23 and 184. It marks the target temporarily
as an enemy, checks GetNearest selects that exact party member, and places
the actor at the target position only during the synchronous combat cycle.
The existing harness forces hit context and checks actual target HP after
message processing. Resetting the flee counter before each simulated combat
frame isolates retaliation from fifteenth-count movement requests. Source
position/counter/type and target type are restored before an engine tick;
normal melee damage and payload production still run through RunAwayFrom,
Swing and the message handler. Action 184 expects NORMAL, action 23 expects
INTERRUPTABLE, independently of Swing's ignored return.

Initial .arc/2026-10-03T20-58-29-776557-batch: build passed, both symbols hit,
but the UI verdict failed at the retaliation HP check. Movement was proven:
start (232,858), destination (232,906), real end (232,864) after 3 ticks,
positive away product 288. The melee target remained 291 rather than 285.
The initial test changed the actor allegiance, which qualifies all allied
party members as enemies; it did not prove the selected victim. The revised
harness marks only the intended target and asserts its GetNearest ID.

This covers the nearby periodic movement path, not the three random-point
search phases used beyond eight grid squares. Sanctuary skip and ranged
weapon rejection are still not runtime-covered by this scenario.

Final .arc/2026-10-03T21-04-05-099182-batch: all shared source checks,
class/layout audits, Win32 build, 60-second hold, both symbol gates, and
12 UI steps pass. RunAwayFrom x240 includes the 34 guard checks, movement
and two 102-call retaliation cycles. Actual movement repeats (232,858) to
(232,864) in 3 ticks, toward (232,906). GetNearest selects 8847495, matching
the intended victim, with range 512 and sanctuary=0 in both cases.
Action 184 deals 291->285 (6 HP) and returns NORMAL; action 23 deals
285->279 (6 HP) and returns INTERRUPTABLE. Save/config restored and hashed;
temporary fixtures removed. Global PASS with the same two static warnings:
argument provenance count mismatch on RunAwayFrom, and partial ExecuteAction
call-count parity. No differential equivalence is claimed. The initial
failure is retained; its victim selection was unproven, so no confirmed
engine defect or root cause is attributed to it.

Next recovery: remaining morale/panic control in ProcessAI, using the now
recovered flee executor. Far-target random destinations still need coverage.
