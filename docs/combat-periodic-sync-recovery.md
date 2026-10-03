# Periodic sprite synchronization

ProcessAI (0x72B9A0) restores synchronization at 0x72C608..0x72C717, using
the previously read full ProcessAI context, original instructions and the
existing JumpToPoint (0x745950) implementation.

Party sprites synchronize on their unsigned frame/id phase modulo 16;
all sprites additionally qualify on modulo 2048. Service provider NULL or local
ownership permits synchronization. Party sprites not in WALK sequence call
JumpToPoint at their current position with spriteUpdate FALSE. Then the branch
sends CMessageSpriteUpdate. It occurs before timer-stopped and inactive routing,
so it is not restricted to normal active action processing.

AutoUI actiongates adds three stopped-timer canaries: eligible party phase,
off-phase, and eligible phase while walking. A temporary stale m_posExact tests
whether JumpToPoint actually recalculates exact position, rather than merely
observing a clean process. The synchronous fixture restores exact position,
frame counter and sequence before further tests. No save is written; the
profile preserves save/config files. Packet delivery to another peer and the
nonparty modulo-2048 path are not runtime-tested here.

The next periodic call at 0x71D0B0 is named CGameSprite::CheckFatigue by BG2
PDB. Its decompile creates opcode 93 effects and fatigue feedback; it is a
separate unrecovered body, not part of this synchronization change. Complete
ProcessAI and original/recovered differential equivalence remain unproven.

Initial .arc/2026-10-03T22-56-38-222124-batch built successfully but returned
NOT-LOADED after 120 seconds, with no UI scenario output and zero symbol hits.
It is unproven runtime evidence, not a successful recovery validation.

Validation: .arc/2026-10-03T23-01-12-730170-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 52 assertions
and eight UI steps pass, including the three exact-position canaries. CLEAN
for 60 seconds; ProcessAI hit 13271 times and JumpToPoint 51. Profile cleanup
restores and hashes preserved paths. Both targets remain parity YELLOW,
original/source calls 191/200 and 50/36; ProcessAI's floating-point warning
remains. ProcessAI offset analysis is unavailable; complete static equivalence
is not established.

CheckFatigue and its periodic dispatch are covered by
[the fatigue recovery](combat-fatigue-recovery.md).
Next target: remaining early ProcessAI action replacement and movement bookkeeping.
