# TString sharing and spell-immunity keys

Targets: cdecl construction wrapper44BA90, substring assignment45E870,
detach44BD40, length44BC00, compare4C5ED0 and max-size45EA70.
Full digest/context bundles/shared header/callsets and ASM read, including
45E870's entire self-erasure and sharing paths. TString remains pack2/16 bytes.

Construction accepts destination/source pointers and checks destination NULL
before reading source. It copies head byte0 only, preserving head padding.
Whole nonempty copies share while BYTE refcount<FE; FE/FF deep-copy. Assignment
preserves the destination head. Distinct strings can already share a buffer:
release the destination before incrementing the source count. Self assignment
performs two native erasures, each preceded by Detach, even for a whole copy.
Detach ignores stored logical length and copies strlen bytes from shared data;
owned/frozen/NULL buffers are unchanged. SetLength writes length and terminator
without inventing bounds checks. MaxSize returns FFFFFFFD.

Compare retains the int API but treats size/position bit patterns as UINT,
clips the left range and compares unsigned bytes, then unsigned lengths.
Embedded zeros are ordinary comparison bytes. Invalid position throws
std::out_of_range("invalid string position"): native7F9C0E message and ThrowInfo
869CB8 -> catch table869CA8 -> catch type869C88 -> RTTI8A80A8 establish the type.
Source catches verify that both strings remain unchanged. Native exception
execution is not claimed; huge allocation/failure injection is not covered.

tstring_sharing_reference.py: frozen canonical MD5
25cb3d8aa2ec648e94e328d8c467973e, tmp_tstring_sharing_native_final PASS6319:
29 constructions,903 assignments,24 distinct aliases,94 self assignments,
29 detach,126 lengths,5112 comparisons,NULL construction and max-size.
Counts0/1/2/127/253/254/255; empty/short/long/binary strings; high-bit sizes;
rounded capacities and released/retained references. Rooted alloc/free callbacks
retain released bytes for native observations; hooks/npos restored, PID killed.
Probe SHA256495628cd8e96d5d36d463ac83b4f7b8b4eab173a0d74551245735660cba0e2a1.
Self slicing with embedded zeros is excluded: native strlen detachment can make
the following stored-size subtraction underflow. Incoherent NULL/nonzero-length
objects and placement construction onto the source itself are also excluded.

TStringSharingChecks.h replays exact bytes, lengths, capacities, head/padding,
refcounts, return values, comparisons and retained identities. Freed addresses
may be reused by the CRT. Seeded external references are manually disposed only
after both real objects release their references. Actual CSpellImmunitySet
tests allocator copy construction, duplicate insertion, tree copy, key mutation,
membership and release. No original HP/raster claim from this isolated probe.

ARC13-36 failed compilation: fixture called nonexistent Pass; no runtime claim.
Fixed the fixture and expanded native construction/detach paths rather than
delegating their inlined bodies. ARC2026-10-16T13-46-54-645029-batch PASS3 warnings:
full Win32 Debug build,83/83 steps,81984 new checks,6/6 PDB gates,60s CLEAN.
Four parity GREEN; two tiny getters YELLOW. Third advisory: native inlined
PrepareAssign versus one source call in Detach. Missing layout/offset oracles
leave general static equivalence unproven; ASM and replay establish this scope.
Earlier2277 ember checks/eight cloud swaps,5348 particle checks/216 render calls
and1419 ground checks/72 render calls pass. Melee6HP; bow/bolt/sling12HP each
with charges3->2. Ordinary projectile arrivals each deliver3HP, source loss0.
Profile cleanup passed: temporary fixtures removed, preserved paths restored
and hashed; no game process left and no saves written.

Next: sprite stubs AddSpecialAbility71B9C0/RemoveSpecialAbility71BC60, used by
effects/Seven Eyes (digests only). Assemble full bundles first; keep the complete
combat profile. 55BAF0/55CC10 appear to be STL tree iterators.
