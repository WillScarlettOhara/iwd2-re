# Character completion recovery

Targets: 612DE0 CompleteCharacterSkills, 613310 CompleteCharacterWrapup,
722820 AddRacialAbilities (#guess), 4E9610 CItem::GetAbility (correction).
Full bundles/shared blocks: tmp_chargen_completion_context.md and
tmp_item_ability_context.md + manifests; all target ASM read with sym.py.
Canonical original MD5 25cb3d8aa2ec648e94e328d8c467973e.

Skills removes items, computes gold, creates 00STAF01 for live class !=6,
selects43/0/1/1 and requests usages43/1/0.
Only the first button is copied; ammo cache keeps SHORT slots11..14, else0.
Empty list/monk preserve button/cache; all buttons/list are deleted. Zero base
and default derived stats feed feat helper/no-op71E750, then dirty1/effect pass.
ApplyNewSkillBonuses (#guess) aliases the no-op COMDAT shared with UI methods.

Wrapup installs DPLAYER3/script6, sets start alignment reputation*10, grants
racial abilities and refills7 caster lists/9 levels. Indices0/5 replace max with
GetMaxKnownSpells+bonus and fill shared uses; other lists fill each ID with
amount0/TRUE. Domains/innates/songs fill IDs; shapeshifts fill their shared pool.
Zero-max entries retain current count. Gold recalculates only if zero.

Racial helper uses start race/subrace, ORs infravision for human1/2, elf1,
all dwarves and half-orcs; deep gnome ORs infravision/nondetection and writes
WORD field_3E=14. RACESPAB column
SPECIAL_ABILITIES_FILE selects a 2DA, '*' skips it. Column0 is resref, column1
unsigned atol count. Exactly1 requires innate catalogue membership and absence
of the ID in the sprite list; other counts grant repeatedly, feedback FALSE.
Missing columns use table default.

GetAbility requires GetDemands()>0; it must NOT Demand() as the old source did.
It rejects ability type's low BYTE0, including WORD0x100. Missing ref/resource,
unparsed resources and invalid indices return NULL; counts remain unchanged.

CScreenCreateChar now packs2; missing field_4FA added and initialized (native
store605E8A). Compiled spacing matches all82 comments, sprite all310.

Probe scripts/probes/chargen_completion_reference.py:144 completion +124 getter
cases, tmp_chargen_native5/report.json PASS. Rooted inputs/hooks, restored code
and globals, only spawned PID killed. Vector-destructor callback ABI is stdcall.
SHA256 95db28b1b8555c9106b697ccdbd26fde5b497db68b904c88c56b0c1a4103ef90.
Real target bodies, RemoveItems, catalogue lookup, class-index mapping and spell
count helpers execute. Resource/GUI/equipment/stat-constructor/feat dependencies
are receipts. Racial callbacks update membership for later dedup; grant receipts
are the oracle. Wrapup's '*' racial table isolates its real spell-count digest.

ChargenCompletionChecks uses typed party state, real pinned text/item resources,
table loading, ability grants, script installation, item selection and counters.
112 completion +124 getter replays. Arbitrary native receipt buttons remain
separate: real43/1/0 exposes at most one slot43 button. Hidden originals and
disposable fist/old-slot items exercise removal. Effect pass uses deferred gate.
Restore actor, tables, catalogues, resources and queues in one tick; never save.

ARC19-21 built but rejected fixture cleanup: owning equipment snapshot freed
restored items; replaced with non-owning pointers. Demands exposed the GetAbility
bug above. Native2DA Load requests twice/destructor cancels once (ASM402C32/402C4F,
722CBF); fixture now observes and cancels its extra request. Raw resource helper
pins text without calling Load. All actor/stat/spell checks already passed.
ARC2026-10-17T19-35-59-105262-batch PASS3 warnings: build170s,86/86 steps,
1192 new checks, hits4/12/108/17951,60s CLEAN; restored=true/cleanup_errors[].
GetAbility GREEN; other call counts55->18/47->34/31->26. Offset oracles skipped;
general static/differential parity remains unproven in ARC. Native fixtures
above cover their stated scope. Melee6HP, ranged12HP/ammo3->2 still pass.
Next: LevelUp 5E56F0 (604 lines/56 callees); full bundle required.
