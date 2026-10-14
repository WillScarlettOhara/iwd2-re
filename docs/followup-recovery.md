# MAGMISS follow-up carrier

Ctor533CB0/vtable84E7B0; leaf534600/vtable84E838, size2FA, pack2, separate
classes/inherited travelling render. Fire533F40, AI534140, callback533DE0.
Dtor533E10 destroys CAI type/bases, not unused child; fixtures free it.

Tail2A0..2F8: eleven annotated members in CProjectile.h, spacing PASS.
Ctor seq0, stores args/filter, Decode(type+1), callback/attached-1, empty
trail. Leaf: count input, interval300, child16,
keep-sourceTRUE, mirror/clamp10,7/render128, dirs1, no tint/offset/shadow/
fire, GOODCUTOFF31, LOSTRUE; second argument unused.

Factory51..5B sets counts1..11, stamp type-1; caster EA>199 => filter199,
otherwise200. Caster is required, as native. Factory17 creates CallLiH
CallLightning child: no recolorFF, render1, sequential0, life20, EFF_P19.
Its unfinished non-FF recolor branch is outside this recovery.
Installed KEY: MAGMISS is WAV4, no BAM1000; parent cell clears missing ref.
Fixture checks resource availability, not an assumed retained BAM name.

Fire source/target/supplied point/facing0. Copies BEFORE Add, stamps
COPY.slotNum with child's WORD type, preserves effect.projectileType and
original payload. Registers front at supplied point/height, sound15, binds
callback/height, clears ready, fires; synchronous callback may set ready.
Creates next child, copies spell level/resource ONLY to new prototype,
decrements WORD count, resets delay. Kind2F2 stays0: initial uses caller
kind, follow-ups use0 (native omission).

AI gates own-ID Time Stop/ready. Positive delay decrements/returns, action
next update (301 for300). Remaining>0 binds callback, retries previous share;
missing/foreign target RemoveSelf, saved ID released after removal. Front
GetNearest / point FindObjectNear: WORD(searchRangeH<<5), default visible
terrain, LOS field, see-invisible1, nearest0, sleep/dead/all0. Release share;
keep-sourceFALSE uses previous target as source. New share retries; failure
retains mutated IDs. Success copies effects, fires/replaces child/metadata,
decrements count/resets delay. Exhausted/no-next notifies parent via deny
retry; failed deny keeps carrier. Remove/Delete, self-delete only on success.

Child Fire535100: DeliverEffects BEFORE registration; single target share,
no retry/fallback, failure stays registered/unplaced. Live position (supplied
for target-1), wrapped Y+13, front/Z0. Stop/launch sound, immediate parent
callback with deny retry/release. Replaces incorrect arrival-sound dispatch.

Five-body + child Fire/AI bundles read, shared header reused/new declarations
read. ASM settles widths/order/ABI; lift merges args. Export created3 functions
(unrelated443320 error). Stubs gone.
followup_reference.py PASS342, canonical MD5 25cb3d8aa2ec648e94e328d8c467973e:
7 ctor,88 factory,112 Fire,128 AI,7 child Fire cases. Native control flow
executes; resources/base, child dispatch, arrays, selectors, effects/sound
isolated. Probe needed child-delete callback; hooks/globals restored. Golden
data from final capture, hashes in tmp_followup_reference_golden. No native
actual-HP/raster claim.

Typed fixtures: factories/copies/WORD overflow/delay/Time Stop/retirement,
real child share/callback. All11 counts + point-search/changing-source: six HP
per strike,301-update cadence,20-update markers, parent retirement. Private
area disables LOS; factory flag/selector ABI checked separately. Actors,
queues, areas, resources/clock restore in same tick; no save.
Initial batches fixed compact ctor heuristic, fixture syntax and absent-BAM
assumption, without gate changes. Third journal rename failed; rollback clean.
Final15-30-29 PASS: Win32 Debug,75 steps,693 follow-up/46134 nearest checks,
69 six-HP hits/markers retired,9 symbols,60s CLEAN; backup hashes restored.
Fire/GetNearest GREEN; seven YELLOW (counts/tiny callback). Missing offset
oracles/fieldless layouts leave full static equivalence unproven.

Next: visual ctor535320/vtable84E940, Fire535D20, AI5357B0 (missing exports).
