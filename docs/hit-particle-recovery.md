# Hit particle recovery

Particle ctors56DF00/56E280, AI56E650, Render56EA90, Remove56ECF0;
ember animator ctors55CD70/55D3A0, IncrementFrame55D810, Render55D950.
Full target/helper/cleanup bundles, shared headers, callsets, relevant ASM and
native ABI read. Native particle vtable84F14C, embedded wrapper84F1B8 at7E;
animator84EEB8. Animator classes now pack2; parent layout audit16 fields clean.
Explicit default destructor maps deleting wrapper55D2C0; member cleanup55D2E0
is supplied by C++ destruction (TString, palette, cells, base).

Both particle ctors use signed facing remainder, qualified base insertion and
self-delete on failed registration. Ranged ctor clears its pool pointer before
possible partial destruction and acquires the shared ref only after success.
AI preserves native SHORT lifetime wrap, DWORD age/motion/rebound arithmetic,
exact cloud/respawn BYTE==1 gates, occupied-cell retry RNG and 10-byte pool cells.
Animator constructors retain BAM names, enabling ICloudA/ICloudB alternation;
null strings use CString's empty fallback. Sequential scheduler narrows sequence
count to BYTE and wraps its SHORT sequence. Render uses terrain-adjusted tint
position and complete GC bounds; intersection only gates drawing. Ember render
wraps height/shadow/fade-point math and preserves descriptor blend decisions.

hit_particle_reference.py: canonical MD5 25cb3d8aa2ec648e94e328d8c467973e,
capture tmp_hit_particle_native_verified PASS2604:216 animator constructors,
36 particle constructors (including failed registration),300 AI,1260 sequence
updates,576 particle render plans,216 ember render plans. Frozen original with
rooted isolated dependencies; hooks/globals and dependency hashes restored.
No original HP/raster claim. Golden header excludes incoherent missing-resource
constructors with nonzero synthetic sequence counts; both real ctor forms run.

HitParticleChecks.h replays typed state, flags/padding, RNG, names, constructors,
full renderer arguments and software pixels using captured blend plans. Height
and luminosity bitmaps make the terrain tint coordinate observable. Actual BAM
resources/metadata are pinned under service locks. Pending insertion IDs removed
before deleting fixtures. Private search, objects/pools/queues, resources, camera,
visibility, pixels and clock restored in the same outer tick. Never saves.

First ARC15-36 failed link: destructor declaration changed while objects were
being compiled, mixing implicit/explicit definitions. Stable full rebuild follows;
no runtime pass claimed for that failed build. Vtable auditor now clean.

ARC15-45: full Win32 build passed; all numeric/control/pixel checks passed.
Two restoration checks failed because main/shadow share FirebaR and nested
anchors were unwound forwards. Fixture now releases shadow before main; no
production change was needed. Full gates retained for the rerun.

ARC15-53 PASS7 warnings: Win32 Debug,81/81 UI steps,5348 checks,216 complete
software pixel comparisons,7/7 unique PDB symbols exercised (both constructor
forms replayed),60s CLEAN. Ground lifecycle1419 checks/72 pixels still passes;
real melee6HP and bow/bolt/sling12HP plus ammunition and arrival checks pass.
Rollback restored/errors[]. Static equivalence remains unproven: missing
offset oracles and seven call-count advisories; no original raster equivalence.
Next: animator geometry55D690 and cloud rebind55DBD0, then their shared helpers.
