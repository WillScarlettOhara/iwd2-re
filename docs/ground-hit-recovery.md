# Ground impact recovery

Visual ctor56BF30, destructor body56CF00 (scalar wrapper56CEE0), AI56D0A0,
Render56D730, Remove56D9B0. Full target/helper/cleanup bundles, callsets and
shared CProjectile header read. ASM overrides a bad lift: first arc DX is
cursorX-centerX, not its negation; distance is trunc(sqrt(wrapped DX²+DY²)+0.5).
Grid16x12, fixed shift10; first arc origin Y uses wrapped (Y<<2)/3. Second arc
duplicate-axis test deliberately reads first-arc vy[i]. Four quadrants retain
native zero-axis flags. Own tail stays pack2 / native234 bytes.

Constructor now retains four VC6 TString buffers (byte share count before
characters); >=FE copies into owned rounded-capacity storage. Destruction
releases them in reverse order. Descriptor allocator bytes are NOT copied;
native sub-ctors seed them from uninitialized stack, excluded from value parity.
Other visual/animation/density/pool fields copied individually. Uses operator
new/delete for all three fan buffers; failed allocation/registration destroys
the visual. Host initializes flag-buffer pointer before possible failure to
make partial cleanup defined. Null BAM names pass through native CString's
empty fallback. Registration uses qualified base AddToArea/front/Z0.

AI wraps DWORD age, position/velocity, density-square and spawn-count math;
BYTE countdown wraps independently. Exact bamLoaded==1 drives animation;
Render accepts any nonzero loaded byte. Collision reflects only crossed grid
axes; coverage consumes one BYTE per pixel. Stationary cooldown decrements per
eligible cell; failed density rolls still append a flag0 cell, successful ones
append flag1/update index/spawn/reload cooldown. Pool cells are10 bytes with
BYTE flag plus padding, not SHORT flag. Stationary life WORD(duration+2*count),
moving life count; signed Y*3/4 truncates toward zero after DWORD multiplication.
Successful self-removal returns before the native epilogue's freed-this reread.

Render restores mode assertion, native visibility clamp and wrapped geometry.
Foot includes terrain height, not Z. Temporary bounds include Z, which is
removed again before clipping; clip Y=footY-Z, clip Z=Z. FX prep flags|80,
render stored flags/transparency0, blit stored flags|1.

ground_hit_reference.py:465 cases, canonical MD5
25cb3d8aa2ec648e94e328d8c467973e, capture tmp_ground_native_signed.
108 ctor fans,72 countdown/Time Stop gates,54 collision cells,128 emission
cases,96 render plans,5 name-sharing/detach/destruction cases,2 registration
results. Rooted isolated dependencies, frozen original; hooks/globals and
dependency hashes restored. No original HP or raster claim. GroundHitGolden.h
stores fan arrays, gates, collisions and spawn outcomes.

GroundHitChecks.h replays typed state and real particle launches, verifies
four name refs, drives real Fireball OnArrival after destroying its carrier,
and compares full software pixel patches with/without a synthetic WED obstacle.
Private search bitmap uses passable terrain; resource frame/sequence metadata,
search, actors/objects/pools/queues, camera/visibility/pixels/clock restored in
the same outer tick. Pending area insertion IDs removed before fixture deletion.

First ARC10-50: build passed, numeric/particle/name cases passed; harness used
one-frame=end render sequence and impassable synthetic terrain, causing render
and lifecycle failures. Corrected fixture to two frames/passable costs. Initial
destructor source mapping stale/tiny-body advisory corrected; full gates retained.

ARC10-58: Win32 Debug,80/80 UI steps,1419 ground checks,72 software pixel
comparisons,88 particle launch checks; parent-independent detonation27 objects
retired in54 ticks, zero left. Earlier melee6HP/bow/bolt/sling12HP checks pass.
Final ARC11-06 PASS4 warnings:5/5 symbols,60s CLEAN,rollback restored/errors[].
Missing offset oracles/call-count advisories leave static equivalence unproven.
Next: particle ctors56DF00/56E280, AI56E650, Render56EA90.
