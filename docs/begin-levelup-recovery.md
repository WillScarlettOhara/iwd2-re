# Character level-up entry

Targets: OnLevelUpButtonClick 5E5330, IsLevelUpButtonClickable 5E4E80,
IsLevelUpPanelNeeded 5E0FD0, AdvanceLevelUp 5E1910 (last two #guess),
and HasSpell 725330. Full bundles/manifests: tmp_beginlevelup_button,
tmp_beginlevelup, tmp_levelup_select, tmp_levelup_flow, tmp_has_spell,
all *_context.md. All ASM read; canonical MD5 25cb3d8aa2ec648e94e328d8c467973e.

Eligibility uses viewability, ownership (unless provider zero), XP level change
and STATE_DEAD. UpdateMainPanel enables control 37 accordingly. Click takes
the UI lock and character deny, stops actions, reports readiness, requests effect
processing, copies AI/specialization and allocates three independent snapshots.
It opens class selection (54). The virtual selected-portrait getter supplies the
actor; multiplayer messages separately use the member, both narrowed to SHORT.

Panel order: 2->54->6->7->55->56->8->16->2. Predicates handle first-class
specialization, every fourth total level, skill/feat counts, automatic cleric
domain/class feat minima, bard/sorcerer known spells and ranger enemies (/5,
signed subtraction, including negative remaining choices).
The spell cursor wraps as BYTE, skips missing resources, counts usable unknown
spells, examines at most 30 loaded entries and caps grants to availability.
Negative deltas survive. Invalid cleric specialization retains the native
sprite-pointer fallback column. Completion calls LevelUp, dismisses all popups
and clears readiness; host BYTE 0/1/other branches remain distinct.

HasSpell fixes three prior errors: the exact-level end bound, advancing domain
lookup, and checking domain counts instead of innate spells. Out-of-range UINT
levels scan all nine levels; only BOOLEAN 1 requests remaining-use checks.
field_2BC/64C are INT; field_77C is DWORD, removing field_77E. 105 compiled
member spacings PASS. Native ctor omissions remain documented in the baseline.

Reference: scripts/probes/begin_levelup_reference.py. Frozen private original:
tmp_begin_levelup_native6/report.json PASS: 486 selector, 72 eligibility,
216 flow, 12 click, 8,640 HasSpell cases; code/globals restored and PID killed.
Probe SHA256 c1db1f0989fcd67cf2f6c109aea8013a16ab6784cf2f71b8ad0ced71e6a61d6e.
HasSpell and catalogue/list searches run unchanged. Selector doubles resource
and usability services; eligibility doubles stats lifetime/XP. Flow doubles
predicates, UI/completion and message services. Click doubles locks/object
lookup, eligibility, effects, stats and downstream flow. These are orchestration
receipts, not independent proof of doubled callees.

Typed BeginLevelUpChecks replays 486+72+8,640 cases with real dependencies.
A private GUIREC dispatches control 37 for rejected/accepted clicks and checks
popup 54, snapshots, specialization, other-station stop message and effects.
Equipment snapshots are non-owning pointers. Path/search, owned current spell,
action queue/latches, animation, catalogues/tables/resources/network and engine
pointers restore in the same tick. No save. ZRCHIT plus existing SPIN260 are
pinned; the first run stopped safely because ZRCHIT2 was absent.

parity_offsets previously absorbed sibling classes and their bases, mistaking
m_typeAI+12C for CVidCell::m_nNormalFrame. It now scopes the requested class,
direct members and actual bases. Three tests PASS, including a real missing
receiver still being flagged. #guess stays in declarations, allowing the source
body reader to parse definitions; the address map was regenerated.

ARC 2026-10-18T15-18-57-842656-batch PASS (3 warnings): 89/89 steps,
9,696 checks, all five hits, 60s CLEAN; cleanup restored=true/errors[].
Melee 6HP, ranged 12HP/ammo 3->2; arrivals 3HP/source 0.
Generic static/differential unproven; manual scope above.
Cancellation, done gates and reverse progression: see cancel-levelup-recovery.md.
Done dispatch and skill/feat resets remain incomplete; retain the full profile.
