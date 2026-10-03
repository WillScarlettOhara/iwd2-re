# Common combat action gates

ProcessAI (0x72B9A0) now restores the common action-resolution branches at
0x72DB06..0x72DD79, verified against the original instructions. This extends
the script scheduling recovery; it does not declare all of ProcessAI recovered.

Ordinary resolution requires a living sprite, no IsHelpless spell state,
no caster hold, no petrification/frozen death, and either zero time-stop or
the sprite being its caster. Any nonzero time-stop value blocks other actors.
Eligible actors resolve once with TRUE and then continue through instant actions.
Restricted actors resolve only instant actions with FALSE. Dead actors discard
an initial ordinary action and clear interruption without executing the next
queued action in that pass; an allowed instant action follows the FALSE path.
The network LeaveAreaLua exceptions preserve the binary's initial helpless
check and its unconditional stop after subsequent iterations.

AutoUI actiongates calls actual ProcessAI on a centered, idle party actor.
WAIT's decrement proves ordinary execution; SetStartPos's position marker
proves instant execution. Twenty-two cases cover both action classes across
normal, caster hold, Held, Hopelessness, Resilient Sphere, petrification, frozen
death, other/caster time-stop, negative time-stop and death. A queued WAIT
terminates instant chains and verifies dead-action advancement.

The instant canary is temporarily classified in the in-memory rule list;
membership and lookup mode are restored before the next engine tick. The
fixture restores state/spell bits, time-stop globals and the position marker,
but uses a disposable process rather than a complete AI-state snapshot.
It never writes a save. Profile cleanup preserves save/config files.

Network sessions are not exercised by this fixture. Original/recovered
differential equivalence is not established; instruction evidence and observed
recovered-build cases remain separate.

Initial .arc/2026-10-03T22-16-55-612390-batch stopped at test-only compile errors:
the rule accessor is const. The fixture now explicitly obtains mutable access
to the underlying nonconst game's table for its scoped mutation.

Validation: .arc/2026-10-03T22-21-31-664330-batch, PASS with two warnings.
All eight source checks, class/layout audits and Win32 build pass. Runtime
CLEAN for 60 seconds, ProcessAI hit 93376 times and ResolveInstants 93266.
All 22 assertions and eight UI steps pass; profile cleanup restores and hashes
preserved paths. Both targets remain parity YELLOW: original/source call counts
191/164 and 17/35 respectively; ProcessAI also retains its floating-point warning.
Offset analysis is unavailable for both targets, so static equivalence remains
unproven.

Paused preview is covered by [the paused action recovery](combat-paused-actions-recovery.md).
Next target: active cutscene/inactive-sprite routing in ProcessAI.
