# Projectile ownership and packing — 2026-10-03

The base `CProjectile` now uses pack(2), matching the original's two-byte
alignment. This removes the actionable alignment drift between `field_70` and
`m_sourceId`, and between `field_17C` and `field_17E`. The header still contains
unmodelled regions and fields kept in derived classes; this does not establish
an exact complete binary layout.

The missing destructor at 0x51E9B0 deletes every remaining owned effect and
clears its MFC list. Member and base destructors then perform the ordinary
CString, CSound, list and CGameObject cleanup. The original scalar-deleting
wrapper at 0x51EA70 calls that destructor and frees the object when requested;
the C++ virtual destructor supplies that wrapper automatically.

Evidence comes from complete context bundles `tmp_projectile_cleanup_ctx.md`
and `tmp_projectile_dtor_ctx.md`, and assembly at 0x51E9B0/0x51EA70. The binary
walks the list at +0x7E, deletes non-null effect pointers through their virtual
destructor, clears the list, destroys members and calls the base area removal.
The implementation has no additional projectile arrival/delivery behavior.

The packing audit now passes with four review gaps, and the vtable audit has
one remaining missing override: CompressTime, slot 0x2C, original 0x536F40.
Its complete context and assembly were inspected: it requires a delta greater
than one game minute, removes the object-array entry, deletes the projectile
only when deletion succeeds and returns false. It remains unrecovered in this
change. These gaps are retained rather than added to the accepted baseline.

Base-impact validation: `.arc/2026-10-03T13-34-25-869312-batch/`.
VS2019 Win32 build and eight source checks pass. The 12-step scenario passes,
with 5/9/9/9 HP loss, and the full 60-second observation is CLEAN. PDB counters
record four calls each to the new destructor, CreateWeaponDamage and
DetermineHeight. Layout audits pass with the review gaps retained.

The overall batch remains FAIL: the CompressTime virtual is missing, and the
offset tool cannot infer a class for the destructor. An explicit manual
`parity_offsets.py 0x51E9B0 --class CProjectile` reports not analysable.
Destructor parity is YELLOW (seven original calls vs three source calls;
member/base destruction and inlined iteration account for additional binary
calls). Neither the offset skip nor the YELLOW score proves full parity.
Pending-effect ownership is established from the assembly; the impact scenario
exercises destruction after delivery, not every pending-effect cancellation.

Attack/Swing regression: `.arc/2026-10-03T13-40-09-488084-batch/`, PASS with
static warnings. All 22 UI steps pass, both symbols execute 442 times and the
90-second observation is CLEAN. Controlled action IDs 3/98/105 produce HP losses
6/12/12/12, including fixture payloads. Unlimited melee usage remains 3;
each ranged stack goes from 3 to 2. The ordinary weapon-button/target click
also produces a real attack and HP loss (249 to 242). These rounds and UI
behavior were observed in reconstruction only; paired original Attack/Swing
and ammunition comparison remains outstanding.

After the engine change, the matched base-impact comparison is still MATCH,
8/8: `.arc/2026-10-03T11-43-33-539196-weapon-reference/`. The probe now includes
the sprite/projectile headers in its source hashes, so packing changes are
recorded alongside method changes. Five baseline-input harness tests pass.
All three runtime transactions restore save/config state without cleanup
errors; no damage is saved and fixtures are removed.

Next: recover/compare projectile CompressTime, then extend the matched original
probe to controlled Attack/Swing rounds and ammunition use.
