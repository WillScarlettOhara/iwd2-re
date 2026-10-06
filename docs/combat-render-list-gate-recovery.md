# ProcessAI render-list action gate

Corrected the field tested before normal action resolution in ProcessAI
(0x72B9A0). Its shared full context bundle and PE instructions at
0x72DAD4..0x72DAD7 compare sprite offset 0x16 against zero. The existing
CGameObject layout resolves that field to POSITION m_posVertList;
m_pArea is the different pointer at 0x12. The prior source tested m_pArea.

After ProcessEffectList succeeds, the sprite must have a render-list
position before the dialog/incapacity/time-stop gates permit ResolveInstants.
Keeping an area pointer alone does not satisfy the original condition.
No list insertion/removal or generic null-area guard is added.

Four typed AutoUI cases call actual ProcessAI with the same loaded area:
registered and unregistered WAIT, then registered and unregistered instant
SetStartPos. The WAIT decrements only when registered; the instant action
updates its start-position canary only when registered. The existing
instant-rule fixture and action assertions are retained. The saved POSITION
is temporarily replaced by NULL during a synchronous call and restored
immediately, before any engine tick; the actual area list is not edited.
No fake POSITION is supplied, game time is not advanced, and no save is written.

Validation: .arc/2026-10-06T18-47-15-637524-batch, PASS with two warnings.
Eight source checks, class/layout audits and Win32 build pass. All 110
case assertions and eleven UI steps pass; CLEAN observation for 60 seconds.
ProcessAI hits 93457 times and ResolveInstants 93288. Save/config paths
are restored and hashed. Parity remains YELLOW (191/240 and 17/35
original/source calls); offset analysis is unavailable. This establishes this field/branch routing, not full
ProcessAI or original/recovered differential equivalence.

The subsequent action-transition marker call is recovered in
[combat-action-marker-transition-recovery.md](combat-action-marker-transition-recovery.md).
