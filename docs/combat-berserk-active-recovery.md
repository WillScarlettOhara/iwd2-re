# Recovered UpdateBerserkActive (0x72FBF0) — 2026-10-03

A previously missing engine function now updates CGameSprite::m_berserkActive
from the normal ProcessAI loop, immediately before CheckModal. This is actual
engine recovery, not another differential-test extension. The recovered name
is descriptive, based on the existing destination field and verified behavior.

The complete body follows the original assembly:
- CanAct false clears the flag, but does not return.
- Spell state 33 (SPLSTATE_BERSERK_ALWAYS_ACTIVE), byte +0xA10 bit 1, forces it
  true and returns, even after CanAct false. This is not general-state BERSERK.
- Otherwise, only update when unsigned frame counter +0x44A modulo 100 equals
  unsigned object id +0x5C modulo 100. Other ticks retain the previous value.
- Non-front-list sprites clear the flag on the aligned tick. The compared byte
  constant at 0x84C50C is LIST_FRONT (0), not an object type.
- Front-list sprites query the nearest enemy from their current AI type, using
  virtual visibility capability, visual range, terrain table and LOS. Set the
  flag iff the result differs from INVALID_INDEX.

Assembly confirms virtual slots 0x24 GetVertListType, 0xB4 GetCanSeeInvisible and
0x98 GetVisualRange. Decompiler argument lists for the last two virtual calls
are misleading: the pushed values are subsequent GetNearest arguments. The
function's EAX values are incidental; its caller ignores them, so its interface
is void. The ProcessAI call sites are 0x72D90B followed by CheckModal at 0x72D912.
CAIObjectType uses normal RAII for its owned CString.

Context: full bundle `tmp_combat_enemy_full.md` and provenance manifest, built
with a temporary name mapping after the original placeholder-only bundle omitted
the class context. Assembly was inspected with sym.py. No BG2 offsets were used
as IWD2 offsets. No remaining active ProcessAI branches were invented.

Final validation `.arc/2026-10-03T17-24-56-422969-batch/`: PASS with one existing
CheckModal warning. New function parity is GREEN and its offset check passes.
Eight source checks, class/layout audits, Win32 build and a 60-second CLEAN
runtime pass. The new function executes 93,142 times through ordinary ProcessAI.
The existing centered base-damage scenario passes all 12 steps. Save/config and
fixtures restore without cleanup errors, processes stop and test damage is never
saved. No new original differential capture was required for this recovery.

Limitations: the argument-provenance tool prints the stale neighboring export
name UpdateCharacterStatus with no source; it does not provide independent ABI
proof. The assembly inspection supplies the call/argument evidence. CheckModal
has an existing 228/115 call-count warning and skipped offset check; only the new
function is a required runtime symbol. Full ProcessAI still contains unrecovered
morale, panic, berserk action selection and interruption branches. This change
restores their enemy-presence state prerequisite; it does not make those branches
complete or claim the full combat loop has been recovered.

Initial `.arc/2026-10-03T17-19-37-825224-batch/` was FAIL because a call-site comment
was incorrectly parsed as a function marker. The comment was corrected; the
final complete arc rerun above passes. Runtime in that initial run was also
CLEAN. Next recovery: the missing active ProcessAI action-selection branches.
