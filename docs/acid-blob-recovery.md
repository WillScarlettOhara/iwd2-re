# Acid travelling leaf and range palette

CProjectileAcidBlob (#guess): native ctor 532260, vtable 84E504,
size 2A0, no added fields. All gameplay/render slots inherit Travelling;
own destructor deferred (532360/532380). ACIDBLOB, sequence 0,
range 0 ACID=73, shared cell palette, tint enabled, no height offset,
mirror or shadow, 16 directions. Speed 20 inherited, callback/attached -1,
empty fire sound/trailing CString. Factory 6C/C0/C1/C2 enables height,
clears fire/arrival sound and stamps type-1. C0/C1/C2 range colors 86/110/26.

Recovered Travelling::SetRange 52B150 (BG2 PDB BAM::SetRange signature):
cell SetPalette BEFORE main palette SetRange. Native 7AE300/7BF3B0 copies
range metadata but aliases the RGB buffer; changed RGB is visible while
the cell's cached range remains old. Ctor instead colors before binding.
7BF7D0 narrows both arguments to WORD, stores BYTE color, copies 12 master
bitmap pixels to 4+12*range, balances Demand/Release, clears subrange cache.
No reversal of the native helper order.

Full bundles tmp_acid_travel_context.md, tmp_acid_leaf_context.md and
tmp_travel_set_range_context.md; unchanged shared headers/factory reused
from prior arc, new bodies/cases/declarations read. Constructor/helper,
factory 520134/524760/5247D9/524852, palette copy/loop checked in asm.
Inherited Fire 52C050, AI 52B900 and Render 52B190 reread completely.

acid_blob_reference.py PASS13, canonical MD5, tmp_acid_blob_reference_final.
Native ctor/factory/palette-copy/range-loop execute; base, allocation,
bitmap/resources isolated callbacks. Eight type/delay descriptors, five
range/value probes including dirty high WORDs. Real loop observes all
12 pixels. Original suspended/killed, hooks/global restored, stable hashes.
AcidBlobGolden.h generated from this capture. No native flight/raster proof.

AutoUI acidblobchecks replays defaults, alias/order/WORD cases and each
master bitmap pixel, then eight factory/delay variants against direct
CVidCell/FX rendering for normal, hidden and offscreen states. Frame anchor,
palette/resource demands, camera/WED, visibility/search byte and full pixel
patch restored. Both area pointers seeded on manual renderer fixtures.
nearestchecks launches all four actual factories via virtual Fire/AI,
retains damage during movement/Time Stop, overrides supplied target point
with live party position, checks launch Z32 and ordinary arrival retirement.
Real instant six-HP payload, party stats/queues/private area restored in
same outer tick. No save; previous weapon, chain and beam tests retained.

First batch 20-56-10-832036 ERROR/UI-INCOMPLETE: all six symbols ran,
four live variants each lost six HP after five updates and retired, but
the render fixture stopped on its first variant. Test setup now aims the
unlaunched object south (native ctor leaves facing unwritten), restores
its position, builds palettes before frame-anchor locks, and requires
nonempty visible reference pixels and owned arrival messages.
Second batch 21-06-09-974394 UI-FAIL: added queue ownership assertion
rejected the legitimate defender damage bark after group Run. Fixture now
checks that exact owned sound and disposes it without altering speech;
requires one group plus one bark. All four six-HP flights remained correct.
Final .arc/2026-10-13T21-12-08-533351-batch PASS (3 warnings):
Win32 Debug, 72 steps, 149 acid checks/24 renders, 142 nearest checks,
all six symbols hit, 60s CLEAN. Four real six-HP hits after five updates;
melee/bow/crossbow/sling 6/12/12/12 plus ammo/chain/beam regressions pass.
Cleanup hashes restored. Ctor/helper/Fire parity GREEN; factory/AI/Render
YELLOW. Fieldless layout skipped; missing offset oracles leave full static
equivalence unproven. Pixel tests compare recovered paths, not native raster.
Continuation: [thrown leaves and immunity helper 4E7120](thrown-weapon-recovery.md).
