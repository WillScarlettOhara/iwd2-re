# GroupAttack on normal AI ticks — 2026-09-22

This scenario follows the recovered GroupAttack dispatcher through normal game
updates, rather than calling ExecuteAction or Swing from the test harness.
The engine implementation is unchanged by this follow-up.

`groupdamage` temporarily classifies the second party member as an enemy and
issues one GroupAttack order to the first. It waits for AttackNoSound with the
expected normalized target, a queued GroupAttack repeat with the same selector,
and actual target HP loss. It rejects competing party attacks. No hit roll,
combat stat or attack-frame progression is forced.

On success or timeout it clears the source's order/queue/path, deletes only
its unfired pending projectile, and restores the target's previous enemy/ally
classification. Each run starts with idle actors in the same area and target
HP above 60. An inaccessible actor fails the scenario; the enclosing profile
still owns process cleanup and preserved-file restoration.

`combat-group-live.json` runs the equipped weapon from the test save, then bow,
crossbow and sling fixtures through the shared rangedready/rangedrestore
helpers. Those helpers retain original item pointers, verify ammunition was
spent, and restore the original equipment and quick slots. The camera is
centered and both actors must be visible before testing. Save/config files
are preserved transactionally; the scenario never saves the game.

This uses a party companion with temporary enemy classification, not a complete
NPC AI scenario. It proves one damaging attack after GroupAttack scheduling;
it does not prove that the repeated group order later reacquires another enemy.
There is no matched original/reconstruction capture, so runtime success must
not be interpreted as complete binary equivalence.

## Observed result

Run `.arc/2026-09-22T21-52-43-552075-batch/` passed all 29 scenario steps.
The equipped weapon reduced HP from 291 to 281; bow from 281 to 270;
crossbow from 270 to 262; sling from 262 to 252. Each ranged case spent one
round (40 to 39), and all equipment restorations passed. Every case observed
AttackNoSound plus the expected queued GroupAttack, with no competing party
attack. All four classification restorations reported EA_PC (2).

The VS2019 Win32 build, shared source checks and class/layout audits passed.
The 120-second world observation was CLEAN: GroupAttack executed four times,
Attack 61 times and Swing 61 times. Cleanup restored preserved paths and
verified their hashes. Results were reviewed and committed on September 24.

Static limitations remain: call-count parity is YELLOW for all three functions
(GroupAttack 24/9, Attack 124/79, Swing 201/139, binary/source). GroupAttack's
member-offset audit is non-analysable, and Swing retains its string-provenance
count warning. These do not invalidate the observed scenario, but leave full
binary equivalence unproven.
