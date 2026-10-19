# Hit-point calculation prerequisites

Targets: RollHitPoints 542A90 (numeric), 542B40 (table), 542CB0 (class),
GetStartingHitPoints 542FA0, and caller CompleteCharacterAbilities 612930.
Full bundles/manifests: tmp_hitpoints_context.md, tmp_hp_caller_context.md.
ASM: tmp_hitpoints.asm, tmp_startinghp.asm, tmp_createhp.asm plus remaining
542DE8..542E85 and jump table; full target/header/shared context read.
Canonical MD5 25cb3d8aa2ec648e94e328d8c467973e.

Numeric: assert nonzero sides; class level <=1 gets sides*rolls. Later positive
roll counts consume three rand calls, then either use minimum or consume three
more and use the last result without another minimum clamp. Zero rolls use the
modifier; negative rolls do not. Preserve signed arithmetic and RNG order.
Table: unsigned BYTE endpoints, INT induction through level 255, SIDES/ROLLS/
MODIFIER lookups in order, optional sides override, every nonzero MaxHP bypasses
random rolls, signed division then per-level minimum one. Aggregate guard kept.
Class: old/new GetClassLevel receipts, zero-new early return, five table families;
barbarian uses warrior table with twelve sides, not the barbarian table.
GetStartingHitPoints actually takes AI, old stats, new stats and CON. AI/CON are
unused; sum all eleven class deltas with min-roll/divisor one. CON bonus remains
separate in GetHPCONBonusTotal. CRuleTables now explicitly uses pack(2).

Creation caller now forwards its zero-level snapshot plus current stats, uses
DEX/WIS for Reflex/Will arguments, and clears nonempty innate banks' max/total
while keeping current; empty-bank aggregates survive. Full ASM confirmed these.
The saving-throw dependency currently ignores its ability argument.

Reference scripts/probes/hitpoints_reference.py: tmp_hitpoints_native1/report.json
PASS, 1,145 cases. Targets, native GetClassLevel, atol and CRT rand unchanged;
table lookup and CString format/destruction are explicit doubles. Private
stats/game/TLS, rooted callbacks; code/globals/TLS restored, no callback errors,
private PID retired. Inputs, generated JS, canonical binary and drivers hashed.
Generated HitPointsGolden.h contains captured outputs and exact supplied cells.

Typed replay uses actual C2DArray lookup/labels, all six borrowed banks and private
stats. Four creation-caller cases use a loaded actor, empty fighter grants and
deferred effects: HP forwarding, aggregate clearing and two effect requests.
Restore stats/AI/innates/options/table ownership, queues and clock in one tick.
No equipment copies or saving; RNG explicitly resets to seed one afterwards.

ARC 2026-10-19T12-22-33-840702-batch PASS (two call-count warnings): BUILD OK,
91/91 steps, 2,304 HP checks, 70s CLEAN. Hits: RollHitPoints 3,624 pooled,
GetStartingHitPoints 52, creation caller 4. Cleanup restored hashes/errors[].
hitpoints_compare.py --reference tmp_hitpoints_native1 --run .arc/RUN_ID:
MATCH for 1,145 ordered HP/next-RNG outputs, no differences or tolerances;
only op->tag renamed. Artifacts/fingerprints in run's hitpoints-pair/.
Melee 6HP; bow/crossbow/sling 12HP; earlier level-up/cancel checks still pass.
Overloaded RollHitPoints PDB counters pool three methods; typed kind 0/1/2
receipts distinguish executed overloads. Static/runtime/differential are separate.
Static evidence remains unproven: skipped offset/layout checks. Numeric/table/
aggregate parity GREEN; class 17-vs-5 calls and caller 46-vs-27 YELLOW (native
switch duplication and automatic lifetime/inlined operations need manual scope).
Source lints and class audits pass; the caller is not a complete paired capture.

Next: character level-stat helper 5D8EA0, then OnDoneButtonClick 5E2210. The native
HP-cache-hit branch reads an uninitialized local; investigate without inventing
deterministic HP. Skill/feat panel reset bodies remain incomplete.
