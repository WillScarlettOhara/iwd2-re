# SPBRNHND spray and animation lifetime

Parent ctor532CA0/vtable84E69C/size2A2; distinct cell vtable84E724/size2A2
adds slot34 AddVisualToArea533AE0, retains travelling Fire27. Both use
AI533A50 and signed WORD lifetime2A0. Source-only common base expresses
shared code/tail, without inferring a third native vtable. Child ctor is
inline at533729/533806, kept in header. Pack2; native base LONG174/178
remain unwritten by base ctor, spray sets23/50. Names/semantics remain raw.

Ctor: SPBRNHND seq0, range0=71 then bind, flags20000 OR8, no tint/offset/
mirror/shadow, dirs16, life55, callback/attached-1, empty fire/trailing.
Factory1A/51F843: height1, bind BEFORE range0=67, EFF_M20, empty arrival,
stamp19. Parent cached range stays71, RGB becomes67. Child caches67,
aliases RGB, owns no payload, default flags20000, copied height.

Fire532F00 retries source shared/denied, GetStart(facing1), adds +/-10
only outside inclusive[-10,10]. Uses supplied point, not live target.
Registers before front-list insertion; supplied LONG height, sound15.
Seeds X<<10/Y<<12 divided3, scaled direction, wrapped DWORD metric,
sqrt truncation then/32. Spawn533600 emits1 for oneSide, otherwise2;
recurses iff signed-WORD(count)-1>0, passes full DWORD count-1. Order:
left,right,recurseleft(TRUE),recurseright(FALSE). Eight WORD direction
tables84D6B8..84D798; 14/32 offsets divided1000, SHORT narrow, wrapped add.
Fire triangle is asymmetric (native), fields23/50, exact maximum bounds.
GetAllInPoly: ANYONE, visible terrain, back-listTRUE. Each selected AIBASE
gets independent effect.Copy AddEffect(source,candidate). Release source,
virtual DeliverEffects sends original to explicit target: overlap hits twice.

AI: own-ID Time Stop gate, WORD decrement BEFORE virtual FrameAdvance;
post<=0 removes area, deletes array entry, self-deletes only on success.
No OnArrival/repeated delivery. AddVisual registers/front-inserts, failed
Add self-deletes. Existing wave SetResRef helper matches native4EBF40.

Export created two functions (unrelated443320 error). Five full bodies/
call sets + children bundle read, shared header reused. ASM settles widths,
lists/order; lift misreads oneSide/null and stack args. Stubs replaced.
Canonical MD5 25cb3d8aa2ec648e94e328d8c467973e.

spray_reference.py PASS261: ctor1, factory2, spawn192, Fire24, AI42.
Native constructor, palette copy/loop, placement, geometry/routing and AI
execute; base/resources/sound/arrays/selection/effect execution isolated by
callbacks. No native actual-HP/raster claim. Pointer/GC probe bugs fixed;
hooks/globals restored. SprayGolden.h from final capture, dependency hashes
in tmp_spray_reference_golden/report.json.

Typed fixtures replay placement, geometry, countdown order. Party fixture:
two polygon copies plus original target, six HP each, nine cells, 55-tick
retirement without repeated damage. Pixel fixture compares parent20008 /
child20000, cached palette, normal/hidden/offscreen. All actors/queues,
resources, pixels/camera restored in same tick; no save.

First13-09-56 failed lint/layout/build; fixed inline ctor, sound wrapper,
base-tail annotation and handler, without gate changes. Second13-18-49
fixture placed target inside triangle (12 HP); moved outside bounds.
Native implementation preserved. Final13-32-00 PASS: Win32 Debug,74 UI
steps,1854 spray checks/12 pixel comparisons,1500 nearest checks,9 symbols
hit,60s CLEAN, cleanup hashes restored. Ctor/AI/Deliver parity GREEN;
six YELLOW warnings (counts/render), missing offset oracles and fieldless
layout skips leave full static equivalence unproven. Pixel tests are source
renderer comparisons, not native raster proof.

Continuation: [MAGMISS follow-up recovery](followup-recovery.md), including
the immediate CallLiH callback and native count/interval scheduler.
