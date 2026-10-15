# Colored strike completion

Ctor5348C0, AI534C10, Render534DD0; CallLightning vt84E8C0,32 slots,
size29A, pack2 tail cell192/palette26C/render290/anim294/lifetime298 SHORT.
Full bundle/callsets/BG2 read; shared header reused.
ASM read covers ctor/AI/render and confirms counts, widths and release order.

Ctor non-FF branch was empty: now SetRange(0,BYTE color,game master bitmap),
THEN SetPalette. ColorFF skips both. Range palette construction, cell resource,
FrameSet0, sound ref, DWORD render/anim and SHORT lifetime preserved. No new
SequenceSet, height or sound configuration. CString-to-resref native ABI has
the same stored semantics as this source's existing CResRef interface.

AI own-ID Time Stop, wrapped SHORT decrement, zero removal/Delete then return
on either success/failure. Sequential end resets0; otherwise FrameAdvance.
Any nonzero anim flag uses BYTE sequence0 length, no rand when zero; otherwise
one rand modulo length. Single target share, missing/foreign RemoveSelf; foreign
branch saves target ID BEFORE removal, releases saved ID after possible deletion
(old source read the destroyed carrier). Coordinates last, only live same-area
or invalid target. No arrival or additional payload on countdown expiry.

Renderer clamps height-adjusted foot to areaY-1, keeps saved foot for clipping,
uses BYTE initial/repeated sequence count, signed-WORD visibility and all early
returns. Ordinary BAM header count is already BYTE; above255 synthetic callback
captures document ABI truncation only. Each visible sequence draws one slice,
next foot -= centerY, preserved frame, final sequence retained. Local flags OR
20101 when render field0, otherwise20280. Z enters bounds, then twice the top/
bottom clipping bounds; graphics foot excludes Z. Height/tile/bounds/negative-Z
arithmetic now explicitly wraps DWORD to match CPU instructions.

call_lightning_reference.py PASS266, canonical MD5
25cb3d8aa2ec648e94e328d8c467973e:36 constructors,168 AI,7 target/length AI,
55 render plans. Original frozen. Resources, palettes, arrays, sound, animation
predicates, FX and RNG values isolated/rooted. Failed deletion, zero/negative
lifetimes, stop owner, arbitrary flags, zero/255 lengths, removed-ID mutation,
0/1/2/3/255/256/257/-1 counts, lock/visibility/height/Z boundaries captured.
Repeated Frida replacements require revert+flush before replacing; fixed the
probe's initial access violation. Hooks/globals/dependency hashes restore.
No original real-HP/raster claim; generated CallLightningGolden.h is the oracle.

Typed tests exercise real palette helpers with a pinned 12x256 24-bit synthetic
master bitmap, including color127/254/FF and actual row RGB. The first game run
stopped on a shipped-palette bounds assertion at127; temporary header/data/stride
and demand restoration fixes the fixture, preserving native production behavior.
Replay native AI frames/WORD overflow, real CRT rand41/18467 consumption, pinned
foreign share causing failed deletion, missing/foreign targets. All test RNG
seeds reset to1 as existing disposable fixtures; no prior RNG-state claim.
Software full pixels use pinned/restored main/header/frame/sequence tables for
0/1/2/3/255 identical slices, both render branches, color73/FF and visibility.
Six color carriers deliver real6HP once and expire after3 updates. Restore actors,
objects, queues, resource pointers/metadata/demands, camera, pixels and clock.

Final19-52-30 ARC PASS: Win32 Debug,77 steps,790 strike/46265 nearest checks,
60 full pixel comparisons, six real6HP hits/3-update expiry, no repeat damage.
Ctor710/AI1588/Render60 hits,60s CLEAN; save/config/fixture hashes restored.
AI/Render parity GREEN; ctor YELLOW and arg-count advisory leave static proof
incomplete where oracles skip. Marker moved beside Render to gate all3 symbols.
Setup journal rename failed once; rollback clean, retry retained all gates.

School-hit factory/opcode233/Sunray completed: [school-hit-recovery.md](school-hit-recovery.md).
