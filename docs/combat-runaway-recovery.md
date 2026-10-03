# RunAwayFrom executor recovery

2026-10-03. Recovered CGameSprite::RunAwayFrom(CGameAIBase*) at 0x744C00
and ExecuteAction case 0x18 (0x7296D2), shared by actions 23 and 184.
The byte dispatch table confirms both entries. BG2 PDB supplies RunAwayFrom
and RUNAWAYFROM / RUNAWAYFROMNOINTERRUPT names; IWD2 assembly confirms
SHORT return and ret 4. Named action constants added without fabricated PE
addresses. Dispatcher resolves TYPE_SPRITE, releases its share and records
the return. Shared class layout reused from the preceding recovery context.

The executor rejects missing/different-area targets, then post-increments
SHORT field_5614. Expiry compares the signed previous count to LONG specifics,
resets count and queues DropPath plus idle sequence unless already ready.
Action 184 returns NORMAL while running; action 23 returns INTERRUPTABLE.

At more than eight grid squares, with no path/search request, it attempts
120 points away, 240 sideways and 120 toward the target, using the original
rand call order and offsets of grid size plus rand()%200. Points clamp to
inclusive area dimensions. The first point with LOS becomes the destination;
it issues SetTarget when different from current position and returns early.
A target within eight squares causes reflected movement every fifteenth count.

The fallthrough can retaliate with a usable non-ranged selected weapon.
It searches GetEnemyOf at a SHORT radius of (BYTE personal space + SHORT
weapon range plus one)*16, shares the nearest object, calls Swing and releases
it. Sanctuary is checked on the original fleeing-from target, not the selected
nearest enemy; Swing's return is intentionally ignored. Ranged weapons never
fire through this routine. Both quirks match assembly.

Evidence: complete tmp_action184.md bundle and sequential PE disassembly
0x744C00..0x745447, plus dispatcher 0x7296D2 and shared release/return blocks.
Constants at 0x85BC88/8A/8C are SHORT 8/120/200; ready sequence at 0x85BBB9 is 7.
Consolidated loops preserve phases, RNG order, early returns and fallthrough.

AutoUI combatrunaway tests guards, both action variants, signed counter wrap,
expiry and queued message types, while restoring action by assignment,
counter, return, position, target id, sequence, weapon and area pointers.
Tests deliberately avoid fifteenth-count movement and weapon retaliation;
those paths are recovered from assembly but not runtime-proven by this profile.
Profile centers the party and preserves save/config transactionally.

Validation: .arc/2026-10-03T20-47-26-765060-batch.
RunAwayFrom parity GREEN; offset analysis OK. All eight shared source checks, class/layout audits, Win32 build and
60-second runtime hold pass. RunAwayFrom fired 33 times and ExecuteAction
was instrumented and hit. All 34 AutoUI checks passed; 8 UI steps PASS.
Profile cleanup restored and hashed preserved paths.

Global verdict PASS with two warnings: argument provenance reports a source
site count mismatch (binary=0, source=1), retained for review; ExecuteAction
parity is YELLOW (284 original / 77 detected source calls) and remains partial.
GREEN function parity and this bounded runtime scenario do not establish
full original/recovered equivalence. Next combat validation should cover
flee destinations and nearest-enemy retaliation.

Follow-up: [runtime path coverage](combat-runaway-paths-validation.md) proves
nearby periodic fleeing and melee retaliation for both actions 23 and 184.
The random-point phases, sanctuary skip and ranged rejection remain untested.
