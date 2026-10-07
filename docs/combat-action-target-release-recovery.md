# SetCurrAction target-marker release

Recovered 0x733E31..0x733F00 in SetCurrAction (0x7338E0), using its
previously read full context bundle and PE instructions. After action-ID
notification, a noninvalid m_targetId is always cleared, including an idle
incoming action. Only a selected, Orderable(FALSE) actor acquires its target
under an ASYNC deny lock, retrying SHARED/DENIED. A successful sprite lookup
calls its marker's SetType(ELLIPSE); every successful lookup releases the
deny lock, including nonsprite targets. Failed lookups still clear the ID.

Existing CMarker::SetType (0x7670D0) is unchanged. It releases one reticle
reference: count two becomes one and stays a reticle, count one becomes zero
and changes to ellipse. A zero-count reticle stays a reticle. The recovery
uses this helper rather than forcing the type/counter directly.

Nine new typed cases use party actor 1 as target: idle transition releasing
the final reference; two references; zero references; unselected actor;
helpless actor; invalid sentinel; missing object ID; self-target; and a real
registered empty ground container. Checks cover Orderable, current action,
cleared target ID, both sprite markers' counters and types. The existing
temporary container is retained until these checks and then removed. Source
and target state is restored and no save is written. Earlier 267 assertions
remain in the same scenario. Nested same-thread deny locks exercise self
and party target access; static instructions establish the locking protocol.

SetCurrAction remains partial, especially spell/projectile resource cleanup
and later action setup. No full differential equivalence is claimed.

Validation: .arc/2026-10-07T07-54-54-288098-batch, PASS with one warning.
Eight source checks, class audits, sprite layout and Win32 build pass.
All 276 typed assertions and eleven UI steps pass; CLEAN observation for
60 seconds. SetCurrAction hits 41515 times and CMarker::SetType 110.
Profile paths are restored and hashed. SetCurrAction parity remains YELLOW
(77/39 original/source calls); SetType is GREEN. Marker offset analysis
is unavailable and its layout audit skips because it does not recognize
bare hexadecimal header comments. Full static and differential equivalence
remain unproven.

Next target: SetCurrAction current spell/resource cleanup from 0x733F00.

That cleanup is subsequently recovered in
[combat-action-resource-cleanup-recovery.md](combat-action-resource-cleanup-recovery.md).
