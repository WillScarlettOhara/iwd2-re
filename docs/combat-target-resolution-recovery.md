# Sprite target fixation and formation movement

Next lot: [script target and health predicates](combat-target-trigger-recovery.md).

Recover the complete sprite-specific untyped target resolver at 0x733340 and
the formation movement executor at 0x745450. The names
ResolveCurrentActionTarget and Formation are marked #guess; BG2's PDB names
GetTarget/GetTargetShare do not establish which one corresponds to the first
function. Formation is supported by the BG2 method signature and action 47.
No virtual slots or data members change.

ExecuteAction (0x728F80) now dispatches seven more action IDs: 14 resolves and
publishes a target, 47 invokes Formation even for NULL, 48 jumps to the action's
absolute point, 317 jumps to a resolved object's position, and 180/208/263
invoke the existing MoveToObject executor with the base typed AIBASE resolver.
All these branches store m_nLastActionReturn and release a non-NULL returned
share. The dispatcher remains partial. Ordinary MoveToObject action 22 and
its door/non-sprite branch are not claimed recovered by this lot.

Context was assembled before implementation: tmp_combat_cast_target_ctx.md,
then the named two-function tmp_combat_target_formation_ctx.md bundle and
manifest (bundle SHA256 8d4fb971). Both complete functions and required call
sets were read. The shared BG2 sprite layout 47cbd251 matches the complete
block read in the earlier spell lots. The header differs from the previously
read complete header only by the two new nonvirtual declarations, which were
read. ExecuteAction's full export 5dee122f and entry-based disassembly are
reused from the earlier complete dispatcher bundle. Input hashes establish
provenance, not parity or runtime coverage.

Valid entry-based PE disassembly establishes the resolver through its final
ret at 0x733657 and Formation through ret 4 at 0x745946. ExecuteAction's
selector bytes at 0x72B520 and table at 0x72B348 establish the action IDs and
call sites. Resolver calls are at 0x729531, 0x729B2F, 0x729B6E and 0x72B23E;
the last, action 22, remains outside the implemented dispatcher branches.
The typed movement variants call 0x45BDD0 at 0x72960F and MoveToObject at
0x72961E. No disassembly of jump-table bytes is instruction evidence.

The untyped resolver calls GetObject(FALSE). With a resolved object and an
instance of exactly -1, it copies the action selector, changes only its
instance, and copies it back. It does not replace the selector with the
target's AI type. On a changed target it acquires the previous deny, removes
one previous sprite reticle owner when selected and orderable, releases the
deny, adds one new reticle owner under the same condition, and publishes the
new ID. Failure to acquire a stale previous object leaves the old ID and
markers, but still checks the new object's sprite immunity.

Immunity runs after selector fixation and marker publication. It releases
the resolved share and returns NULL without rolling those changes back.
Non-sprites skip marker additions and immunity. A missing target clears the
stored ID without acquiring the previous object when unselected/unorderable;
when selected and orderable it first removes a previous sprite marker owner,
and acquisition failure leaves the stored ID unchanged. Unlike UpdateTarget,
this function does not extend the force-render countdown.

Formation interprets the action point as full-width grid-cell offsets from
the leader's current destination. Either -1 destination coordinate uses the
leader's actual position. An already active identical destination resets the
SHORT throttle and continues. Close-enough movement also resets the phase.
The two personal spaces are combined and truncated to BYTE before adding
one for the radius; zero and very small spaces therefore preserve the PE's
large wrapped radius. CountSquares is the existing engine helper; the following
32-bit distance square also preserves the PE's IMUL wrapping.

Outside range, the destination is published and the phase cleared. Every
signed SHORT throttle multiple of eight allocates a real search request with
two exclusive world-point goals: the offset destination, then the leader's
unoffset destination. It carries the source ID/side, terrain, path smoothing
and an owned selected-party list. Both sides of the 16-cell distance split
build the same request; the far side sets the phase before SetTarget and the
near side afterwards. The final increment wraps as SHORT. Successful movement
returns ACTION_INTERRUPTABLE, including the close and same-destination cases.

The inert AutoUI targetresolvechecks command snapshots both actors' selectors,
markers, target state, animation spaces and affected movement fields. It runs
the helper and real dispatcher with explicit, nearest, NULL, stale and immune
targets; checks marker ownership and lack of force-render extension; exercises
all seven new action IDs; and inspects formation requests across the distance,
signed throttle, sentinel, full-width offset and literal BYTE-radius cases.
Existing immunity entry pointers are restored, without deleting them.

Request inspection holds the search critical section and temporarily directs
SetTarget's semaphore signal to a private semaphore. The worker stays on its
original handle. Each request must remain waiting in the real queue and signal
the private semaphore; it is removed and deleted before restoring the handle
and unlocking. This avoids waking the worker with an empty queue after test
cleanup. Deny counts and the queue/message baselines are checked. No contention
or allocation-failure behavior is fabricated.

Validation command:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts/arc.py batch 0x733340 0x745450 0x728F80 --profile scripts/profiles/combat-target-resolution.json
```

The first runtime batch, 2026-10-09T18-53-54-759702-batch, passes 1,543
assertions (190 new), all 36 steps, all three symbol gates and a 60-second
CLEAN hold. It executes the target resolver 45 times, Formation 34 times and
ExecuteAction 93,490 times. It restores all preserved paths with hashes and
no cleanup errors. A subsequent test refinement adds a literal 32-bit square
overflow case and checks party-list contents as well as counts. Final-source
validation of that refinement is recorded below.

Final batch 2026-10-09T18-59-38-871442-batch passes 1,544 assertions (191
new), all 36 scenario steps, all three PDB symbol gates and the 60-second
CLEAN hold. ResolveCurrentActionTarget executes 45 times, Formation 35 and
ExecuteAction 93,438. Eight shared source checks, class/vtable audits, the
310-member native sprite layout and Win32 Debug build pass. Save/config
hashes are restored and cleanup_errors is empty. The melee fixture removes
six target HP; arrow/bow, bolt/crossbow and stone/sling each remove twelve
and consume exactly one charge. On-hit retaliation and all four opcode-430
projectile damage paths retain their earlier passing results.

Parity remains YELLOW: the resolver has 22/29 original/source calls,
Formation 25/19 and the partial ExecuteAction dispatcher 284/123. The resolver
uses engine accessors that the PE inlines, while Formation shares the two
request-building branches; these counts are not equivalence evidence. The
optional offset analyses for the two initially unnamed exports are skipped,
so arc explicitly records static evidence as unproven for them. No warning
suppression or baseline exception is added. The initial static batch's native
compile error was the Windows near macro colliding with a local variable;
renaming it preceded the successful full build and both runtime batches.

Limits: the reconstructed executable is exercised, not a matched original/
reconstruction capture. Original allocation failures and shared/denied retry
contention are not forced. The jump dispatcher tests include NULL and area-less
exits; existing attackplace exercises actual JumpToPoint separately. The target
helper's optional automated offset analysis cannot classify its unknown export;
manual offsets above and native layout checks are separate evidence. The base
typed resolver and UpdateTarget retain their existing behavior.

Next bounded combat dependency: SetSpellTarget (trigger ID 0x40F4) in the
partial CGameAIBase::EvaluateStatusTrigger at 0x453840, followed by the
untyped MoveToObject action 22 and its door destination helper at 0x48B2C0.
