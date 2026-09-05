# ForceSpellPoint projectile metadata

The metadata block in `CGameAIBase::ForceSpellPointAction` (`0x461B80`)
now matches the object-target twin's recovered lookup. Canonical assembly:

- `0x46210C`: decode the projectile from the selected ability's missile type.
- `0x462127–0x46214E`: for a sprite, call `0x727720` with zeroed class,
  specialization and spell-level reference arguments.
- `0x462153–0x462165`: success writes the returned byte level, zero-extended
  to a DWORD, to projectile `+0x186` (`m_nSpellLevel`).
- `0x46216B–0x46217B`: copy the spell resref to `+0x18A` (`m_casterResRef`).

The existing null-projectile fallback is retained. This is a narrow repair of
an already partial caller, not a recovery of its entire casting state machine
or its multiplayer message path.

Validation: native v142 Win32 build OK. Static arc evidence is under
`.arc/2026-09-05T19-33-10-verify/`: lints, offsets, argument provenance and
vtable checks pass. Parity remains YELLOW (79 binary calls vs 55 source calls).
The layout step skips the base header's unsupported bare-offset comment format;
no layout or header change is introduced by this repair.

`force-spell-point.txt` uses the opt-in AutoUI `forcepoint` command to assign
an action to the first party member with the requested spell available.
It uses `GetDeny`/`ReleaseDeny` and lets the AI execute the action later.
`frida_forcepoint_probe.py` observes a successful lookup specifically nested
inside `ForceSpellPointAction`, excluding the scenario's caster-selection
queries. The probe uses PDB symbols and argument references, not member offsets.

An early harness version used a zero timeout for the MFC critical-section lock;
the debug runtime asserted in `afxmt.inl`. It was corrected to `INFINITE`, as
required by this MFC lock implementation. This was a test-harness defect.

The opening dialogue keeps the new game paused. Initial runs queued the action
but never executed the handler; the hit gate correctly returned NOT-EXERCISED.
Normal unpause was rejected while the dialogue panel was active, and numeric
reply keys did not complete it. The isolated scenario therefore explicitly calls
the engine's `EndDialog(TRUE, TRUE)` through opt-in AutoUI `enddialog`, then
uses `resume` before queuing the action. This setup is not a dialogue fix or
proof of normal prologue completion. Save/config backups and runtime evidence
are kept under `work/forcepoint-20260905/`.

Final native gate: CLEAN; `ForceSpellPointAction` executed 51 times and
`FindAvailableSpell` five times. Exactly one lookup occurred inside the handler
and succeeded with class 3, specialization 0, level 1. The probe also observed
a non-null `DecodeProjectile` return in that handler before the lookup, proving
that the metadata block's null guard is passed. The eleven-step scenario passed.
This is execution evidence for the repaired block, not a differential capture
of projectile immunity behavior against the original executable.

Next investigation: the opening dialogue's reply/continue path. Its persistent
pause blocked the ordinary test setup even after numeric response keys. Keep
that issue separate from the corrected projectile metadata.
