# Projectile time compression — 2026-10-03

`CProjectile::CompressTime` (0x536F40, virtual slot 0x2C) is recovered. It asserts
that deltaTime exceeds 15*60 ticks, asks the object array to delete the projectile
ID on the asynchronous thread, deletes itself only when that operation succeeds,
and returns false in both cases. It does not deliver effects or advance time.

The complete context is `tmp_projectile_cleanup_ctx.md`. Assembly confirms the
unsigned comparison, object ID at +0x5C, Delete arguments (thread ASYNCH, null
output, INFINITE), conditional scalar-deleting destructor and byte-zero return.
The assertion's original file string is `CProjectile.cpp`, line 10139.

The `projectilecompress` AutoUI test creates two instant projectiles without
placing them in an area or firing them. The first has INVALID_INDEX: compression
returns false and the object remains available for explicit cleanup. The second
is registered through the real object array: compression returns false and a
subsequent lookup returns DELETED. The test never dereferences that object after
successful compression. It runs after the four base-damage checks.

`weapon_damage_reference.py --cleanup` repeats both branches in the canonical
original, using decoded instant projectiles, real Add/CompressTime/GetDeny and
the inherited virtual slot. It compares ten records: eight base-damage/HP
observations and two compression results. Actor positions and target statistics
remain matched before the damage tests. Missing cleanup records or failed
retention/removal are rejected by the baseline parser. Seven harness tests pass.

The base projectile vtable audit now has no missing overrides. Its packing
audit passes with the same four review gaps for unmodelled regions; this does
not claim an exact complete binary layout. CompressTime static parity is GREEN.
Pending-effect destruction, area removal, failed deletion due to foreign-thread
shares and assertion failures are outside the two-branch comparison.

Reconstruction: `.arc/2026-10-03T14-09-34-294262-batch/`, PASS with one warning
and one static skip. Eight source checks, class audits, layout audits and Win32
build pass. The 13-step scenario passes and the 60-second observation is CLEAN:
CompressTime executes twice and CreateWeaponDamage four times. The offset tool
cannot analyse CompressTime and visual confirmation is outstanding; these are
retained as limits. CreateWeaponDamage retains its previous parity warnings.

Original: `.arc/2026-10-03T12-15-33-124174-weapon-reference/`, MATCH, 10/10.
Generated JS, input hashes, raw/normalized records and comparison are retained.
Both transactions restore save/config state with no cleanup errors. No damaged
party is saved, and fixtures are removed.

```powershell
.\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x536F40 0x73D560 --profile scripts/profiles/combat-projectile-compress.json
.\.venv-reagent\Scripts\python.exe scripts/probes/weapon_damage_reference.py --cleanup --ours .arc/2026-10-03T14-09-34-294262-batch/profile-runtime/ui.jsonl
```

Next combat scope remains matched original Attack/Swing rounds, payloads and
ammunition. The previously missing base CompressTime override is now closed.
