# Special-ability acquisition and removal

Targets AddSpecialAbility71B9C0 and RemoveSpecialAbility71BC60 were empty;
GetShapeshifts5F6800 incorrectly returned the innate catalogue.
Full bundles/shared headers/BG2/callsets and complete target ASM read. Global
catalogues are songs4C08, shapes4C10, innates4C00; actor lists4AA0/4ABC/4A84.
Getter ASM confirms lea eax,[ecx+4C10]; typed replay uses named members only.

Add first requires an SPL KEY entry (type1006, warning TRUE). Catalogue priority
is song, shapeshift, innate, with a repeated Find before each actual mutation.
Song/innate add max/current1 and shared0; shapes add zeros. Existing entries
preserve their shared field. Unknown catalogue entries can still emit feedback.
Only BOOL==1 enables feedback. Native WORDs85C0FA=36/85C116=56 select its ID;
FeedBack(id,0,0,0,name,0,0) uses a CSpell with balanced request/demand/object life.

Remove has no KEY gate. It uses the same catalogue priority/repeated lookup,
Remove(id,TRUE,zero,zero), then always updates quick buttons, even without a
catalogue/list match. Default32-byte CAbilityId gets itemType1/CString resref;
UpdateQuickButtons(-1,FALSE,FALSE). Full removal with zero count arguments
preserves m_nSharedCurrent even for an entry with uses: native quirk retained.

special_ability_reference.py: canonical MD5
25cb3d8aa2ec648e94e328d8c467973e, tmp_special_ability_native_layout PASS315,
native getter/timer-constructor bytes/save span.
Real outer bodies, catalogue Find and vector Add/Remove execute on rooted
preallocated buffers; resource/UI dependencies capture ABI and lifetime order.
Eight catalogue masks, missing/present KEY, empty/existing/duplicate entries,
six BOOL bit patterns. Three isolated post-KEY bind failures are native-only.
Hooks/global chitin pointers restored and verified; original PID killed.
Probe SHA256f9ab0967a188ef83ac177ed17b6a11f653fd73333381ad842b38410afd26785a.
No original HP, raster, huge vector allocation or UI-equivalence claim.

Typed replay312 cases: vector order, duplicate entries, aggregate fields,
balanced pinned ZRCHIT.SPL ownership, actual TLK token with generic-name10453.
Character engine suppresses audio; world text stays in the disposable process.
Native ability bytes drive reference UpdateQuickButtons; compare all banks,
selector mismatches, SHORT limits and weapon cache against actual Remove.
Actor lists/identity/buttons, catalogues, SPL header, token and engine restored
in the same tick. Never save. Three bind failures remain native-only.

Packing: CInfGame, CTimerWorld and CGameSave now explicitly pack2. Getter sizes
453050/45B630 prove save/options gap422C->43EA=446; native timer54EDD0 writes
DWORD0/BYTE4/BYTE5, leaving seeded bytes6/7 untouched. Full timer bundle/ASM
and parent59CC30 constructor prefix read. Replay verifies timer bytes/sizeof6
and save sizeof446. Standalone layout audits pass: CInfGame125, save12, timer3
members; four actionable breaks removed. Ten embedded-type size breaks remain
reviews, so full raw CInfGame ABI equivalence is not claimed.

ARC18-58: build passed, replay failed90 assertions due to the wrong getter.
Early UI failure cut off the2s hit report; zero counts did not prove no calls.
ARC19-12 passed84 steps/4111 new checks/three gates/60s before nested packing.
ARC19-26 rebuilt final packing; journal rename WinError5 prevented runtime.
Rollback hashes matched, cleanup_errors[]; retry uses unchanged code.
ARC2026-10-16T19-40-32-030929-batch PASS3 warnings: full Win32 Debug build,
84/84 steps,4113 new checks,three PDB gates288/24/142 hits,60s CLEAN.
Rollback verified, cleanup_errors[]. Two helper call-count advisories and the
tiny getter remain; missing offset oracles leave general static parity unproven.
Melee6HP, bow/bolt/sling12HP, ammo3->2 and ordinary arrivals3HP all pass.
Next: identify class-ability helpers71E240/722D50 and complete sprite71E760;
digests only read, full bundles/ASM required before editing.
