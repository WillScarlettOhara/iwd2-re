# ProcessAI walking-to-idle routing

Corrected 0x72C1F0..0x72C21B inside ProcessAI (0x72B9A0), using its
shared full context bundle and original instructions. A null m_pPath and
SEQ_WALK invoke virtual GetVertListType at slot 0x24. Unless the result is
LIST_FLIGHT (2, PE constant 0x84C50E), SetIdleSequence is called.

The previous source instead compared animation.m_nSequence to SEQ_WALK
and added an animation-pointer guard. Since SetSequence normally changes
both sprite and animation sequences to WALK, that comparison prevented
the normal transition when a walking sprite lost its path. It also lacked
the flight-list exception. Both conditions are replaced by the binary gate.
SetIdleSequence and its area-dependent idle choice are unchanged.

Four typed AutoUI cases call ProcessAI with world time stopped and a WAIT
current action: front-list walking without path becomes GetIdleSequence;
flight-list walking remains WALK; front-list walking with an allocated
path remains WALK; READY without a path remains READY. The temporary path
is a real LONG allocation, removed before the next tick. List type, original
path and sequence are restored in the disposable process. No save is written.

The initial .arc/2026-10-05T19-33-13-090183-batch failed compilation:
AutoUI needed the CGameSprite qualification on sequence constants. This
was corrected before runtime; no runtime success is attributed to that run.
The second .arc/2026-10-05T19-36-06-420664-batch failed the earlier
periodic-sync walking assertion: that fixture set WALK without a path,
which now correctly transitions to idle and permits the stationary jump.
The walking sync fixture now supplies a real temporary path with stopped
world time and restores it before any engine tick. Its expected position
assertion is retained; no runtime success is attributed to the failed run.

Final validation: .arc/2026-10-05T19-39-14-896443-batch, PASS with one
warning and one skipped check. Eight source checks, class/layout audits
and Win32 build pass. All 98 case assertions and eleven UI steps pass;
CLEAN observation for 60 seconds. ProcessAI hits 93445 times and
SetIdleSequence 68806. Save/config paths are restored and hashed.
ProcessAI parity remains YELLOW (191/242 original/source calls), while
unchanged SetIdleSequence is GREEN. Offset analysis is unavailable;
complete static equivalence remains unproven.
This establishes the local animation gate, not full
ProcessAI equivalence or movement completion across every animation family.

The subsequent source-only marker tail is removed in
[combat-marker-tail-recovery.md](combat-marker-tail-recovery.md).
