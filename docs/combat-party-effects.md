# Party effect ownership — 2026-09-17

Combat continuation: `Swing` (0x739B60) calls `ApplyEffectToParty` for
weapon effects targeting the party, then deletes the input effect. The
existing implementation put that same pointer into every owning message.

`CGameAIBase::ApplyEffectToParty` (0x45E100) now calls `pEffect->Copy()`
for each recipient, both outside arena mode and for locally controlled arena
members. The original's virtual call at 0x45E1E7 uses effect vtable slot +4;
the returned pointer becomes `CMessageAddEffect::m_effect` at 0x45E20D.
The message destructor owns that copy; the caller retains its input.

## Evidence

Full pre-edit context and assembly are archived in
`.arc/2026-09-17T13-38-46-011010-batch/`. This initial run built successfully
but stopped before exercising the method: the diagnostic incorrectly rejected
the local null-provider session because its connection flag was set.
The failed report remains unchanged.

The corrected `combatparty` diagnostic requires an offline world, temporarily
disables transport, and tests ordinary party distribution, offline arena,
matching owner, empty party, and foreign owner. It checks distinct live effect
pointers, payload/provenance, recipient order and retained input ownership.
Copies are mutated independently and discarded without applying damage.
All temporary party/network state is restored within the same UI step.
The profile additionally restores the selected save and configuration and
verifies their hashes. The scenario centers the camera before checking both
party members are visible.

Final run: `.arc/2026-09-17T13-40-35-175437-batch/` (see its `run.json`).
Win32 build passed; 119 diagnostic assertions passed; the full 60-second
observation was CLEAN with five entries into `ApplyEffectToParty`.
Profile cleanup restored and hash-verified the preserved files. Two builds
and two runtime attempts were used, including the failed precondition attempt.
Static parity reports YELLOW for call counts (6 original / 19 source);
the offset auditor cannot analyse this method, so static evidence remains
unproven. There is no paired original runtime capture for this correction.
This fixes an existing method, not a newly recovered function or playable
Attack/Swing integration; it is not counted as a new recovery lot.

## Next combat work

Resume `Swing` at 0x739B60, then `Attack` at 0x737EF0 and its action dispatch.
`tmp_swing_current.md` is the complete refreshed context;
`tmp_swing_ready_asm.txt` contains the original disassembly. No Swing body
has been added yet. Two ambiguities are now resolved:

- At 0x73B127–0x73B12A, the victim calls `ApplyOnHitSpells(attacker)`:
  this dispatches retaliation effects, not the attacker's on-hit list.
- The depleted-item vtable call at 0x73B23B is `AutoPause(1)` (slot +0xB0),
  not `ClearActions`.

Keep the unrelated workflow changes already present in the working tree.
