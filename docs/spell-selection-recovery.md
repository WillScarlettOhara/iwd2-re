# Level-up spell selection

Recovered: ResetSpellsPanel 5D9970; spell button SetSpell 5EBDD0,
OnLButtonClick 5EBFA0, Render 5EC420; CreateChar::HasSpecialistSpell 618D10;
ResetRaceDescriptionPanel 5D8B50 (descriptive names marked #guess).
Full bundles/shared context: tmp_spellreset_context.md,
tmp_spellbutton_context.md, tmp_schoolhelper_context.md (+manifests).
ASM: tmp_spellreset.asm, tmp_spellclick.asm, tmp_specialist_helper.asm,
tmp_racedescription.asm and sym.py binding/render tails. The shared exported
button header was stale; actual source and native ctor confirm 3State ancestry.
Canonical MD5: 25cb3d8aa2ec648e94e328d8c467973e.

ResetSpells takes one sprite argument (ret 4); the old second argument was
spurious. Bard/sorcerer budget is known-table value minus list entry count;
other classes get 2 universal / 3 specialist slots. Missing catalogue resources
are skipped without consuming a button; at most 30 loaded entries are bound.
Usability must equal TRUE and the actor must not already know the spell.
Budget clamps to enabled count; selection, number token, label and help reset.
SetSpell caches equal references, converts the final icon character to B and
updates the tooltip even if the resource is missing (generic name zero).
Render preserves base 3State gates, clipping, pressed offsets and gray flag 2.

Clicks deny-lock the selected actor, toggle the real spell list and budget,
then update help/popup and release. Bard/sorcerer list maxima include the
GetMaxKnownSpells output bonus. The native specialist scan result is ignored
by this click caller: preserve that behavior. The helper uses a BYTE index;
255 entries are covered, but a no-match catalogue over 255 can wrap forever.
Race text loads SRFILE by base race, narrows the subrace-table ID to BYTE and
uses SRLIST column 3 with the actual display colors.

spell_selection_reference.py -> tmp_spell_selection_native2: PASS 278 cases:
192 panels, six sequential bindings, 32 specialist scans, 48 clicks. Target
bodies/catalogue access/list Add/Remove execute unchanged. UI, locks, resource,
CString, usability and known-spell services are explicit doubles. Rooted
private allocations/callbacks; original main stays suspended; hooks/globals
restored and private PID retired, no callback or cleanup errors. PE, driver,
script, generated JS and inputs are fingerprinted in report.json.

SpellSelectionGolden.h and SpellSelectionChecks.h replay with loaded spells,
real GUIREC controls, real spell lists and table lookups. Eight software pixel
cases check enabled/pressed/clipped icons, plus inactive/clean gates; three
racial text cases use real 2DA loads and TLK strings. These are separate typed
UI checks, without a paired native raster/text claim. Actor, resources, tables,
catalogues, token, engine route, sampled pixels, clock and queues restore in
one outer tick. Native 2DA's extra request is retired by the fixture only.
spell_selection_compare.py requires the full 93-step profile and CLEAN,
compares all 278 ordered outputs, only op->tag rename, no ignored fields.

ARC 2026-10-19T17-24-40-586724-batch: source/build/class audits pass;
click/render GREEN, other parity YELLOW (call counts), offset/layout skips
leave static evidence unproven. Initial smoke NOT-LOADED, zero hits; ERROR
report retained. Unchanged-input runtime-only retry under ProfileSession:
2026-10-19T17-38-37-250145-spell-runtime-retry, 93/93, 6113 new checks,
60s CLEAN. Hits in target order: 192/5766/48/10/56/3. Exact 278-event pair
MATCH in spell-selection-pair; fingerprints unchanged, cleanup errors[].
Melee loses 6HP; bow/crossbow/sling each lose 12HP. No full-ARC PASS claim.

Next: finish OnDoneButtonClick 5E2210, UpdatePopupPanel 5E0B20's level-up
routes (currently only information panel 57 is implemented), and stat updates.
5D8EA0's uninitialized HP-cache-hit read is already established by
tmp_levelup_cache_native1 (see allocation-recovery.md); do not invent a
deterministic replacement or repeat the same investigation.
