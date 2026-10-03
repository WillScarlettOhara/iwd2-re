# Berserk action executor recovery

2026-10-03. `CGameSprite::Berserk` recovered at 0x75A4B0 and connected to
ExecuteAction action 124 (byte map 0x72B59C = case 0x3C, target 0x72A3E3).
BG2 PDB supplies the matching short-returning Berserk method and BERSERK action
name. PE value at 0x84786C is 124. The source address map registers these names.

The executor decrements action specifics before checking berserk-active state
and the remaining positive duration. In 25% of active calls it seeks ANYONE;
when found it preserves the berserk action in the queue and adds an ATTACK.
If not found it adds the default NO_ACTION. Otherwise it seeks the enemy type,
preserves the berserk action and adds ATTACK or WAIT(6). AddAction inserts at
the head: the attack/wait executes before the preserved continuation. No HP
or projectile behavior was invented in this selection helper.

The constructor CAIAction(SHORT,LONG), 0x75AAC0, initializes the three object
selectors through their constructors, retains the original unused temporary
default action, then sets action id, specifics and flags. Its destination is
not explicitly initialized, matching the PE. The unused temporary is retained
for original lifetime/exception behavior. Typed assignment owns the strings.

Evidence: full ExecuteAction context tmp_execute_control.md plus
 tmp_berserk_executor.md with both complete decompiles and their manifests;
CGameSprite header/layout reused from the ExecuteAction bundle. Assembly
0x75A4B0..0x75AAB6 and 0x75AAC0..0x75ACA6 confirms widths, queue order, nearest
arguments, decrement, constructor ret 8 and return SHORT. GetCanSeeInvisible
and GetVisualRange are virtual slots B4 and 98, and GetNearest gets the typed
terrain table and the original TRUE/visibility/FALSE/0/FALSE tail.

The bounded AutoUI combatberserk command exercises ExecuteAction with inactive,
expired and twelve active cases. It temporarily detaches the original queue,
inspects created action kinds, deletes those test actions, then restores the
original queue, current action, last return and berserk flag in the same tick.
It also validates the WAIT(6) constructor fields. Profile owns the save/config
transaction; no fixture items and no save writes are required.

Remaining requested dispatch/executors: action 134 -> 0x738E70 (413 decompile
lines); action 184 -> 0x744C00 (367 lines). They are not implemented or claimed
functional by this recovery. ProcessAI remains partial.

Validation: initial run .arc/2026-10-03T19-19-41-264309-batch failed due to
the provisional sub_ name, stale runtime hook name and constructor parity.
Its UI result nevertheless passed all 14 cases; the missing hook was corrected
by rebuilding the source address map with Berserk. Full rerun:
.arc/2026-10-03T19-26-21-515394-batch. Constructor parity remains RED under the
large-ASM/tiny-source heuristic (body <=12 lines, >=80 assembly instructions),
with zero source calls detected versus 13 original calls. Those are primarily
member constructors/destructors and the unused temporary. This static finding
is retained, not baseline-suppressed; constructor offsets are not analyzable.
Berserk parity is YELLOW (54/16 calls). A clean runtime test cannot replace an
original/recovered differential capture or resolve that static verdict.

Final rerun: source lints, class audits, Win32 build and 60-second runtime hold
passed. All three symbol gates were hit; combatberserk reports 12 active cases
and 2 early-exit cases valid=true, 8 UI steps passed. Cleanup restored preserved
paths. The global arc verdict remains FAIL for constructor parity; no full
static or differential PASS is claimed.

Follow-up: action 134 is now recovered and tested; see
[AttackReevaluate recovery](combat-reevaluate-recovery.md). Action 184 remains.

Follow-up: actions 23/184 now use the recovered RunAwayFrom executor; see
[RunAwayFrom recovery](combat-runaway-recovery.md). ProcessAI remains partial.
