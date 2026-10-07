# SetCurrAction current resource cleanup

Recovered 0x733F00..0x733FA7 in SetCurrAction (0x7338E0), using its
previously read full context bundle and PE instructions. Delete the current
CSpell and clear the pointer on every transition. Its existing CResHelper
destructor cancels an outstanding request when bLoading, releases the
resource object reference when pRes/resref are valid, and clears pRes,
matching the original inlined destruction. Clear the borrowed current-item
pointer without deleting its item.

NO_ACTION returns here: it retains the current projectile and fields 710C
and 5618. Other actions clear 710C, virtually delete a current projectile,
clear its pointer and 5618, then conditionally Decode. Decode now precedes
the previously recovered movement feedback, as in the original. Random
phase, no-action counter, idle sequence and spell-action setup remain
unrecovered later blocks.

Eight typed cases combine NO_ACTION/WAIT with absent spell, empty spell,
registered spell without request, and registered spell with active request.
A guard holds SPWI112's resource alive; reference and request counts must
return exactly to baseline. A disposable projectile subclass records its
virtual destruction; idle actions retain it and active actions delete it.
Null projectiles are also tested. The borrowed item remains readable and
is deleted only by the fixture. Cases check pointer and phase-field state.
Original spell/item/projectile pointers and phase values are preserved
around the scenario, which writes no save. Earlier 276 assertions remain.

The offset-audit resolver failed to identify CProjectile's destructor because
its class regex excluded '~'. Accept the destructor prefix and add regression
coverage for destructor, constructor and ordinary method source resolution.
Both offset-tool tests pass; no runner gates are reduced.

Validation: full batch 2026-10-07T09-06-01-107268-batch passes with two
call-count warnings (SetCurrAction 77/39, projectile destructor 7/3).
Eight shared lint checks, class/layout audits and Win32 Debug build pass.
The profile passes 284 typed assertions and all 11 steps, then observes
60 seconds CLEAN: SetCurrAction executes 41,451 times and the projectile
destructor six times. Fixtures are removed and preserved paths restored
and hashed. Two offset-tool regression tests also pass.

The first full batch (09-01-11-924462) ran clean and passed the runtime
cases but returned ERROR on destructor class resolution; the corrected
batch reruns every gate. Static destructor evidence remains incomplete
where the offset audit cannot resolve member metadata. Runtime lifetime
assertions establish the tested cleanup cases, not complete differential
equivalence of SetCurrAction or visual projectile behavior.

Next target: SetCurrAction post-decode random phase and idle-sequence setup.

Subsequently recovered: [idle phase](combat-action-idle-phase-recovery.md).
