# AttackReevaluate executor recovery

2026-10-03. Recovered CGameSprite::AttackReevaluate at 0x738E70 and
ExecuteAction action 134 (jump-table case 0x40 at 0x72A33E).
BG2 PDB provides the method name, but IWD2's ret has no stack arguments:
the recovered signature is SHORT AttackReevaluate(), not the BG2 int overload.

The executor decodes the original selector on the first tick and stores the
resolved selector in acteeID2. It validates visibility, sanctuary, death,
flight, self targeting and the sleeping/unarmed restriction. Invalid targets
are released before nearest matching target selection; the replacement is
stored and validated, with an interruptible return before attacking.

The valid target path recovers attack feedback/sound, unusable-weapon trigger,
launcher range, personal space, LOS, combat music, path drop, Swing and
MoveToObject. The signed action count budget is checked after either path.
Unlike Attack, failed movement is not mapped to DONE; autopause tests frame
exactly zero. The original exceptional AttackOneRound exit skips ReleaseShare
at 0x739A75; that behavior is retained. No alternate damage formula was added.

Evidence: full tmp_action134.md context and shared CGameSprite layout from the
previous ExecuteAction context. PE assembly checked across 0x738E70..0x739B60;
SHORT range, BYTE personal space, target Orderable(FALSE), release ordering,
and dispatcher call are grounded in assembly rather than malformed decompile
locals. Source address map regenerated before runtime instrumentation.

Profile scripts/profiles/combat-reevaluate.json centers the camera and reuses
temporary combat-payload fixtures. Four attackdamage cases use action 134 via
ExecuteAction for melee, arrow/bow, bolt/crossbow, and stone/sling. The harness
forces a hit and restores source fields; the profile preserves save/config
files and never saves test damage. This verifies valid-target damage, not
fallback retargeting, missing weapons, or movement failure at runtime.

Initial validation: .arc/2026-10-03T19-49-43-393475-batch returned UI-FAIL:
the harness used a zero duration but expected INTERRUPTABLE, whereas this
executor correctly returns DONE once actionCount exceeds the budget. Melee
has unlimited usage (remaining=3 is expected); no damage conclusion was
available from that failed run. The test now uses a duration of 1000, restores
m_nLastActionReturn, and snapshots the action by assignment to preserve all
three specifics despite the original copy-constructor quirk.

Final validation: .arc/2026-10-03T19-57-43-700947-batch.
AttackReevaluate static parity is YELLOW: 110 original versus 75 detected
source calls; offset analysis passes. ExecuteAction parity is also YELLOW (284/72 calls); it remains partial.
All eight shared source checks, class/layout audits, Win32 build, and the
60-second hold pass. Both symbols were hit (AttackReevaluate x408). All 12
UI steps pass: melee 291->285 (6 HP), bow 285->273, crossbow 273->261,
sling 261->249 (12 HP each). Ranged charges decrease 3->2. Preserved files
were restored and hashed; fixtures removed. Global verdict PASS with two
static warnings; this is not a differential equivalence claim.
ProcessAI and ExecuteAction remain partial; action 184 remains a recovery target.
