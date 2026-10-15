# Ember geometry and cloud names

Targets: FX55D690, bounds6A33F0, rebind55DBD0, direction55D800, end55D260,
frame55D210; TString preparation44BC20, growth44BE10, tidy448D50.
Full bundles/shared headers/callsets and ASM read, including Grow's continuation
44BE90. That address is a tail inside the same native SEH function, not a
separate C++ call. Native getters retain the current-cell end predicate and
constant frame -1; facing only stores a SHORT, without selecting a sequence.

FX geometry now explicitly wraps DWORD height/union math before signed maxima.
Shadow changes the main center before rectangle union; final reference Y is
max(wrapped shifted-mainY-Z, shadowY+Z), independently of union height.
Bounds retains signed SHORT direction versus zero-extended BYTE threshold,
mirrored left=(posX-width)+referenceX and wrapped top/right/bottom.

Cloud rebind operates on m_g1VidCell even when current/shadow point elsewhere.
It preserves facing, render gate, pointers and shadow frame; narrows sequence
count to BYTE, randomizes only mode==1/count!=0, then sets main sequence/frame0.
Missing/empty BAMs still keep their input TString name. Native NULL input is
invalid at the final strlen; no invented NULL-name behavior.

TString previously modeled only owned storage. New PrepareAssign (role name,
old export mislabeled Assign) takes size/trim BYTE and returns a BYTE predicate.
Shared buffers detach via Grow; empty assignment decrements a shared reference
or frees owned/frozen storage. Trim retains only sufficient capacity<32; larger
buffers are released before replacement. Grow rounds capacity|31, retries exact
size after allocation failure, copies min(old length,new capacity), releases the
old buffer and restores length/terminator. Tidy respects BYTE release and leaves
the four-byte union head untouched. Methods moved out of line for source/PDB
coverage; CMake registers TString.cpp. Layout stays16 bytes, now explicit pack2.
Copy construction/assignment still deep-copy: separate native44BA90 sharing is
not claimed recovered. Allocation-failure injection and huge buffers not tested.

ember_geometry_reference.py: frozen canonical MD5
25cb3d8aa2ec648e94e328d8c467973e, tmp_ember_geometry_native_1 PASS1260:
350 FX geometries,180 bounds,28 query contracts,504 rebinds,126 storage gates,
48 growth and24 tidy cases. Rooted callbacks/allocations; hooks/globals and
dependency hashes restored. No original HP/raster claim. Golden replay excludes
empty/missing resources with impossible nonzero synthetic sequence counts.

EmberGeometryChecks.h uses real pinned BAM/header frames and sequence metadata,
checks string contents/capacity/refcounts and coherent pointer identities, and
drives eight actual ICloudA/ICloudB swaps through registered particle AI. Owned
allocation identity is not compared after free (allocator reuse is allowed).
Resources/KEY counters, metadata, demands, strings, objects, queues, Time Stop
and clock restored before the next outer tick. Full combat profile, never saves.

ARC21-27: full build passed,2248 new checks/eight real cloud changes passed.
Existing opcode430 payload fixture then failed on its first arrow launch;
later bolt/sling/wrapped-arrow cases passed. Added bounded failure diagnostics;
expected assertions unchanged. Explicit base-frame call covers the folded native
55D210 body represented by separate source symbols. First-arrow cause unproven;
no weapon implementation changed and failure did not recur in the full rerun.

ARC21-38 PASS4 warnings: full Win32 Debug build,82/82 steps,2277 new checks,
eight cloud changes,9/9 PDB gates,60s CLEAN. Earlier5348 particle checks/216
pixels and1419 ground checks/72 pixels pass; melee6HP, bow/bolt/sling12HP,
ammunition and ordinary projectile arrival damage pass. Rollback restored,
cleanup_errors[]. Four class audits clean. Static equivalence unproven: missing
layout/offset oracles, two call-count and two tiny-body advisories (ASM reviewed).
Next: native TString sharing44BA90 and detach44BD40 (digests read only).
