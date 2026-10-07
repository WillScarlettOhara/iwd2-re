# SetCurrAction idle phase

Recovered 0x733FCE..0x734037 in SetCurrAction (0x7338E0), from the
previously read full context bundle and verified PE instructions. After
conditional Decode, active actions set field_5616 to UtilRandInt(8, -luck)
and clear m_noActionCount. The range is the signed SHORT at 0x85BBA4;
luck is read as signed SHORT at sprite+0x95C and the result stored as SHORT.
The entry reset of field_5616 remains, including for NO_ACTION.

Sequences DIE, SLEEP, TWITCH and WALK are preserved. PE bytes at 0x85BBBB
are 17 (SLEEP), distinct from AWAKE (1). All other sequences
call the existing SetIdleSequence (0x707170). Its PE was checked: it chooses
READY in a loaded saveable area, otherwise HEAD_TURN, and updates the
sequence when different or at animation end. The helper is unchanged.

Thirty-six typed cases cover nine sequences, active WAIT with luck 8,
-8 and 0, and NO_ACTION. Luck 8 clamps phase to zero; -8 clamps to seven;
zero must stay in [0,7]. Active transitions clear the inactivity counter;
NO_ACTION retains it and the sequence while retaining the entry phase reset.
The fixture restores luck, phase and inactivity counter alongside its
existing sequence/resource restoration, without reseeding RNG or advancing
the world clock. AWAKE is explicitly tested as a non-preserved sequence.
Earlier 284 assertions remain.

Validation: full final batch 2026-10-07T12-51-19-237906-batch passes:
eight shared lints, class/layout audits, Win32 Debug build, all 320 typed
assertions and 11 scenario steps, then 60 seconds CLEAN with both symbols
executed. Preserved paths are restored and hashed, fixtures removed.
SetIdleSequence parity is GREEN; SetCurrAction remains YELLOW (77/41 calls).
The helper offset audit skips as not analysable (no offsets or thiscalls);
static evidence is incomplete and no original/recovered differential is claimed.

The first batch detected an address-like block comment as a false function
marker. A second batch passed, but the final rerun above includes the
subsequent SLEEP/AWAKE correction consistently in static and runtime checks.

Next target: command-pause marker at 0x734037..0x734056, followed by the
spell-action resource setup. Full SetCurrAction equivalence remains unproven.

Subsequently recovered: [command-pause marker](combat-action-pause-marker-recovery.md).
