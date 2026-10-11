# Weapon swing sound selection

2026-10-10. Replace the empty sprite helper 756930 with DecodeSwingSound.
BG2 PDB supplies this exact method name and two CItem* arguments. Rename the
function in the Ghidra project and re-export its decompile/index before changing
the declaration, definition, AIUpdate caller and AutoUI calls. Keep all member
names/layouts unchanged.

## Binary evidence

Full context tmp_weapon_wear_ctx.md includes the 491-line decompile, required
call set, BG2 layout and full CGameSprite header. Full reachable assembly
756930-7574CA (both arguments popped) and PE strings 8B8A78-8B8AAC checked with
sym.py. The discarded decompile blocks are impossible zero-count branches.
Existing AIUpdate 2181 selects the pending sound; its attack/shoot cases play
field_7408 at frame field_740C. This helper only chooses those fields.

Verified library helpers: 7FCC88 CString narrow constructor, 7FCD57 assignment
from text, 7FCD07 assignment from CString, 7FCFAA append text, 7FAA72 Left,
7F79D7 decimal _itoa, 7E7A54 case-sensitive MBCS comparison. No invented raw
prototypes. STR_RES/CSound and CTlkTable::Fetch are existing recovered types.

## Preserved decisions

- Derived silence bit 1000 returns before item or animation access.
- Truthy field_70FB selects custom lists 72F0/730C/7328 for animation sequences
  11/12/13, or 7344 otherwise. Choose rand()%count, advance once with wrap if
  equal to the pending sound, then install text and full signed frame. Even an
  empty custom sound replaces the selection. Empty lists fall through.
- Item resrefs WYVERN1/2 and WOLFWI1/2 take fixed sound exemptions before launcher
  lookup. Otherwise launcher type overrides item type; type zero means 20.
- Preserve every switch family: Ms/Slg/Swd have five zero-padded variants;
  Sm seven, Lg six. Flal/Xbow use single digits 0..4, Fist 0..2. Choose randomly
  and avoid the current sound by advancing once with wrap. Comparisons are
  case-sensitive. Literal spellings and variant bounds come from PE/assembly.
- Unknown types or prefix Sw_Fist consult base speech slots 40/41/42 for
  animation sequences 11/12/13, if not -1. Fetch the TLK sound resref. Empty
  fallback leaves both old fields unchanged; any nonempty default/TLK sound
  installs frame zero. Animation sequence, not sprite sequence, controls this.

Explicit pack(2) already applies to CGameSprite. No new fields or offsets.

## Runtime coverage

weaponsoundchecks uses the centered disposable combat-messages profile, real
party actor, pinned ZRCMELEE/ZRCBOW headers and copied/restored custom lists.
Exercise 20 type cases with 16 choices each, no-repeat/range/format assertions,
launcher priority, four creature exemptions, unknown types, early silence,
four custom lists, signed frame extrema, empty record and TLK fallback slots.
Restore actor state, speech, lists, pending selection, animation sequence and
item types in the same tick. No queued messages or clock advancement. Keep all
received/direct on-hit and melee/bow/crossbow/sling/projectile HP regressions.

Native batch .arc/2026-10-10T21-37-09-917852-batch PASS: source lints,
zero vtable findings, 310 compiled member spacings, full clean build (169s),
48 UI steps, all 347 sound checks, CLEAN 60s, DecodeSwingSound hit 1291 times.
Profile cleanup restores fixture/config/save paths. HP regressions: melee -6;
bow/bolt/sling -12 each; received/direct on-hit source -1 and target -2;
single/grouped received effects -3/-10; four opcode-430 projectile cases -3.
Refresh address_map/hooks after the Ghidra rename, then repeat isolated oracles:
arg_provenance has zero operator+ sites; parity YELLOW (99 binary calls vs 27
source calls, repeated branches/STL collapsed). parity_offsets is not analysable
for this header; full assembly review and compiled layout cover member accesses.
No original/recovered differential claim.
Next: RenderDamageArrow 708FC0, a remaining no-op in the combat feedback path.
