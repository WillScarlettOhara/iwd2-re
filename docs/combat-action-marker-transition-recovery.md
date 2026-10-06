# SetCurrAction destination marker transition

Recovered 0x733BEB..0x733C14 inside SetCurrAction (0x7338E0), from its
full context bundle and PE instructions. If either old or incoming action
is not NO_ACTION, ResolveTargetPoint receives the incoming action and the
remaining queue's head POSITION, before the current action is overwritten.
NO_ACTION to NO_ACTION preserves the marker. No unconditional tick-tail
update is reinstated in ProcessAI.

Six typed AutoUI cases call SetCurrAction directly: idle-to-idle preserves
a sentinel; WAIT-to-idle clears it; idle-to-move selects the destination;
WAIT-to-WAIT clears it; incoming move with a remaining PROTECTPOINT uses
the incoming destination without consuming the queue; group movement
with that queue preserves the marker. Every case verifies the current
action and queue size. Marker/group/action state is restored in a disposable
process and no save is written.

The previous paused-preview case with no command pause now expects the
queued PROTECTPOINT marker after action replacement, rather than the
earlier preview's current-move marker. The first preview remains accurate,
but SetCurrAction's previously missing write determines the final state.
The protected case still preserves the current-action preview. Earlier
archived test results are retained as historical evidence.

SetCurrAction remains partial: container/modal/spell and other transition
bookkeeping is not all recovered. ResolveTargetPoint is unchanged.
The first .arc/2026-10-06T18-53-11-151597-batch reports PASS with two
warnings and passes all 116 cases, but only ResolveTargetPoint was hooked.
A duplicate leading address marker before a long comment prevented the
runner from resolving SetCurrAction. That comment is converted to prose,
leaving the canonical marker immediately above its definition. This first
run is not sufficient all-target runtime evidence.

Final validation: .arc/2026-10-06T18-57-18-891913-batch, PASS with two
warnings. Eight source checks, class/layout audits and Win32 build pass.
All 116 case assertions and eleven UI steps pass; CLEAN observation for
60 seconds. SetCurrAction hits 41424 times and ResolveTargetPoint 41569.
Save/config paths are restored and hashed. Parity remains YELLOW (77/8
and 0/6 original/source calls), consistent with the partial SetCurrAction
body and recursive marker helper. Marker-helper offset analysis is
unavailable; complete static and differential equivalence are unproven.

Next target: remaining SetCurrAction transition bookkeeping before action copy.

Container closure is subsequently recovered in
[combat-action-container-transition-recovery.md](combat-action-container-transition-recovery.md).
