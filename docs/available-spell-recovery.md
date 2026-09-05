# Available spell lookup — 0x727720

`CGameSprite::FindAvailableSpell` is a descriptive recovery name, not an
original symbol recovered from a PDB. The signature is `BOOL` with a resref
reference and three in/out references: byte class, DWORD specialization,
byte spell level. Assembly was checked against canonical `.bin/IWD2.exe`.

## Binary evidence

- `0x72773B–0x72775C`: resolve the resref through the game's `m_spells.Find`;
  a missing resref returns FALSE without writing the output arguments.
- `0x72776C–0x727795`: a level in 1–9 restricts the search to that level;
  zero or a value above 9 searches all nine levels.
- `0x727795–0x7277C4`: nonzero specialization skips class lists; otherwise a
  class in 1–11 supplies the starting index through `GetSpellcasterIndex`.
  The search continues through later classes, rather than restricting itself
  to the requested class. Invalid non-caster classes retain the existing
  helper's assertion behavior.
- Class lists start at sprite `+0x4284`, stride `0x100`; level lists have stride
  `0x1C`. Entries have stride `0x10`, ID at `+0`, current count at `+8`.
  The first matching entry must have a nonzero current count. A depleted first
  match does not cause a search for duplicate IDs in that same level.
- `0x727995–0x7279C5`: successful class lookup returns the caster class,
  one-based level and zero specialization.
- Domain list starts at `+0x4984`; `0x727AF8–0x727B19` returns cleric class 3,
  one-based level and base specialization from sprite `+0x80C`.

Existing `Find`, `Get`, `GetSpellsAtLevel` and `Get` class-list methods express
the inlined container traversal and assertions. Binary helper `0x5F66F0` is
the vector-size computation used by the entry accessor's bounds assertion.

## Caller integration and limits

`ForceSpellAction` now looks up the spell after projectile creation. Assembly
`0x461488–0x4614C6` supplies zeroed lookup outputs and, on success, stamps
the **spell level** at projectile `+0x186`. `0x4614CC–0x4614DC` copies the
spell resref to projectile `+0x18A`. The existing null-projectile fallback in
this partial source caller is retained. The other five binary callers remain
outside this recovery; this does not complete the casting state machines.

## Validation

- `.arc/2026-09-05T19-20-32-verify/`: lookup parity GREEN; offsets, argument
  provenance, constructor/vtable, source lints and compiled layout PASS.
- `.arc/2026-09-05T19-23-37-batch/`: lookup and edited caller checked together.
  Caller parity YELLOW: 92 binary calls versus 77 source calls. Its incomplete
  implementation remains a material limitation; the lookup itself is GREEN.
- Native v142 Win32 full rebuild succeeds, with existing unrelated warnings.

The reusable probe `scripts/probes/frida_available_spell_probe.py` invokes the
lookup on live sprites from their AI callback, using PDB function addresses
and reference arguments, never debug-build member offsets. It checks failure
output preservation, successful output ranges, and repeatability with explicit
class/specialization/level. It also attempts domain-only queries. These are
injected method calls, not evidence of casting UI coverage or a matched trace
against the original executable. A domain-only miss does not prove the domain
success branch. Logs are written to `tmp_available_spell.jsonl`.

Final native run: CLEAN, AI callback hit 15,960 times; 1,960 completed lookup
calls, eight successes (cleric and sorcerer lists, including explicit-hint
repeats), 1,952 misses. No domain success was observed. Prologue UI scenario
PASS, eight steps. Early runs returned NOT-EXERCISED because Frida suppresses
recursive entry callbacks during the injected NativeFunction call; the final
harness counts the driving AI callback and independently gates lookup return
values. It does not report injected calls as ordinary hit-counter observations.
Evidence and pre-test save/config backups: `work/available-spell-20260905/`.

Next related task: examine the `ForceSpellPointAction` caller's projectile
metadata path, reusing this lookup and checking its own binary call site.
