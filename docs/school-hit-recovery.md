# School-hit and Sunray recovery

Factory560310, opcode233 Apply5601E0, SummonVFX ctor57E490/AI57E580/Fire57E710,
Sunray ctor57E860/AI57EB30/Render57ED30/Fire57F050. Full bundles/callsets/ASM
read; shared headers reused. Sunray virtuals exported using addressed stubs,
address map + create-functions. No build-map.

SummonVFX vt851234/34slots/size2BA inherits travelling VFX, not CProjectile.
Removed independent embedded cell/Render. Inherits renderer578480 and ownership.
Pack2 tail: offset2AE BYTE, loop2AF BYTE, sequence2B0 LONG, count2B4 LONG,
random2B8 BYTE, single2B9 BYTE. Ctor clears offset/loop/index/count/shadow/height/
height offset; dirCount1, random/single1; copies all12 visual-effect bytes.
Fire delivers FIRST using EXISTING payload IDs; ignores caller source/target.
BYTE sequence count, exact random==1 gate, wrapped Y(-100 if offset==1, else+1),
registration/frontZ0, loop-aware sound. Failed Add destroys; IDs/Z unchanged
before successful insertion. AI own-ID Time Stop; advance then loop timer
(ticks on matching unsigned sequence, not repetitions), sequential progression,
sound wait hiding the cell, then removal/Delete. Failed Delete updates sound.
No arrival/callback/repeated payload at expiry.

Factory all112 cases: Sunray57, corrected PWStunH/EFF_M38 at18, common-tail copy
flags, offsets69/104..108, random off21, loop sequence1/count165,265,965 at73..76/
98 with random/single off. Missing healing/poison/paralysis/strength halos added.
Fire63 alone queues glow; others AddEffect directly. Positive exact low-BYTE==1.
Opcode233 queues WORD(type+1000) (message ctor adds1), then local Fire. Existing
message Run/InControl prevents duplicate owner/SP launch. No-area FALSE/done
before selector. Invalid selectors assert.

Sunray vt8512BC/32slots/size29E, distinct from CallLightning. Cell192/palette26C/
render290/anim294/life298 SHORT/phase29A LONG. Ctor clears trail/phase, recolors
range then binds. AI wrapped SHORT decrement; zero switches phase2/frame0;
opening end -> phase1, phase1 loops, phase2 end removes. Random mode never
retires by phase end. Missing/foreign target removes, saves ID before release.
Fire: payload first, single target share or pointY+13, immediate parent callback.
Render always four slices, WORD(phase<<2)+BYTE slice, preserved frame, normal
strike visibility/clipping/Z/wrapped geometry; no sequence-count gate.

school_hit_reference.py PASS3044:896 factory,1944 AI,108 Fire,80 dispatch,
16 replication. sunray_reference.py PASS658:36 ctor,504 AI,7 target,32 Fire,
79 render plans. Canonical MD5 25cb3d8aa2ec648e94e328d8c467973e. Final captures
tmp_school_native_final and tmp_sunray_native_verified. Frozen original, rooted
isolated resources/arrays/FX/sound/RNG. Hooks/globals/dependency hashes restored.
No original HP/raster claim. SchoolHitGolden.h/SunrayGolden.h store oracles.

SchoolHitChecks.h: all factory/glow ownership paths,1458 coherent overlay AI
and504 Sunray AI replays using restored BAM/header tables; fake COM sound-status
buffer detached before destruction; Fire failures/RNG/IDs/offsets, opcode decode/
Copy/local launch/owner packets, real recoloring and60 four-slice software pixel
comparisons. AreaNearestChecks adds12 real6HP carriers/full expiry/no repeats
(Sunray343 ticks, loop carriers212/320/992). Restore actors/objects/queues/sound/
resources/camera/pixels/clock same outer tick. RNG reseeded1, no prior-state claim.

First ARC08-09: new fixture left pending insertion IDs -> count failures then
SortLists UAF. Both retire helpers now remove pending IDs before deletion;
production area sorting unchanged. Rollback restored/errors[].

Final ARC08-18 PASS5 warnings: Win32 Debug,79 steps,5658 school/1726 Sunray/
46325 nearest checks,60 new pixels,9/9 symbols,60s CLEAN; hashes restored/errors[].
AI/overlay Fire/opcode/render parity GREEN. Four call-count YELLOWs, render arg
count advisory and missing offset oracles leave static equivalence unproven.
Next: ground impact visual56BF30/56D0A0/56D730.
