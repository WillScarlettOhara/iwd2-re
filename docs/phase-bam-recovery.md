# Phase BAM and visual-hit opcode 215

Six bodies: ctor535320, AI5357B0, Render535A40, Fire535D20,
VisualSpellHit ApplyEffect4BBCB0, SPBOOM AddVisualToArea530510.
Full bundles/callsets read; shared header reused, new declarations read.
ASM confirms ABI/widths/order. Export4 functions; unrelated443320 failure.

Phase carrier vt84E940 has32 slots, size332, distinct from ImpactBAM vt84E9C0
and BG2 SpellHit. Pack2 tail: cell192, bitmap26C, flags326 DWORD,
hasPalette32A BYTE, translucency32B BYTE, CString32C, phase330 BYTE.
Native dtor5356C0 is ordinary member/base destruction.
Ctor copies visual base, requests base+"2", seq/frame0, optional bitmap,
sound ref, caller flags/translucency, phase0. No sound setup or height flag.

AI own-object-ID Time Stop gate. Phase0: advance to end, switch to base+"1",
seq/frame0, phase1. Phases1..3: on end reset0/increment phase, then always
advance. Phase4: end switches to base+"2", seq0, BYTE length-1 frame, phase5.
Phase5: WORD frame0 calls virtual OnArrival and returns; otherwise wrapped
decrement, no FrameAdvance. Unknown BYTE phases only update sound coordinates.
Normal arrival retains callback/sound/empty grouped delivery/delete behavior.

Fire delivers payload BEFORE Add; failed registration deletes. GetStart uses
target ID with no facing offset; passed point/height/kind ignored. Wrapped Y-1,
front/Z0. Stop sound, optional launch sound/loop/channel15. No early callback.
Render uses owning area, signed-WORD tile arithmetic, terrain height minus Z,
BAM bounds and viewport rejection. Local flags: nonzero |80, zero101;
stored flags unchanged. Optional bitmap demand/palette/render/release, clipping,
unlock and blit; caller translucency retained.

Opcode215 no-area completes TRUE before reading selector. Flags32..34 create
SPBOOM with range colors43/41/45, direct visual registration at target Y-2/Z0;
no DeliverEffects. Flags35/36 create FlmStrk/HLYMITE sequential carriers,
colorFF, render0/1, lifetime2*BYTE world time scale. Flag38 creates DDoorH
phase carrier with zero flags/translucency/height. Others create ImpactBAM
(flags8/trans0), enqueue FireProjectile AND Fire the overlay directly.
Flags0..30 except low bits3 map76..8D and Air/Earth/Water palette rows;
37 maps93/Air7,39 mapsCF/SPFDEATH. Palette suffix arithmetic retains low BYTE.
Unmapped selectors preserve low WORD of incoming sprite pointer as message
argument (native slot reuse); the message constructor adds1. Low bits3 assert;
build fixture does not invoke assertion cases with a present area.

phase_bam_reference.py PASS324 against canonical MD5
25cb3d8aa2ec648e94e328d8c467973e:12 ctor,144 AI,16 Fire,48 render plans,
8 SPBOOM registrations,96 selector/no-area cases. All native target bodies
execute. Dependencies isolated/rooted; original frozen; hooks/globals/hashes
restored. Probe fixed CString operator+ stdcall ABI. No original HP/raster claim.
PhaseBAMGolden.h replays pinned/restored BAM counts, negative WORD wrapping,
all phases and Time Stop.

Typed fixtures decode/copy/apply supported and full-DWORD selectors, inspect
carrier placement, queued/empty group ownership, requested resource availability
and pointer-derived type. Software reference uses native render flags and frozen
frame anchor; all fixture resources/camera/visibility/pixels restored same tick.
Private party fixture tests real grouped six-HP launch, full animation retirement
and final callback with no repeat damage; every prior combat regression retained.

Final16-51-09 ARC PASS (six call-count YELLOWs; static equivalence unproven
where oracles/layouts skip). Win32 Debug,76 steps,942 phase checks/54 pixel
draws/99 native replays. Real6HP,33 ticks (both BAM lengths6), no repeat.
Six symbols hit,60s CLEAN; save/config/fixture hashes restored. Prior weapon
checks: melee6, bow/crossbow/sling12 each. First run fixed fixture type+1.

Next: school-hit dispatcher/factory560310 and its missing variant families;
ground spell-hit visual56BF30/56D0A0/56D730 remains scaffolded.
