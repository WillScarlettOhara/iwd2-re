# Attack sequence validation

Recovered CGameSprite::CheckSequence (0x720310), previously an empty stub.
Full context bundle tmp_checksequence_ctx.md plus PE disassembly establish
the two branches: only SHOOT (8) and ATTACK (0) are inspected. The selected
slot is zero-extended BYTE and ability index zero-extended WORD. Demand
the selected item when present, retain the requested sequence if GetAbility
returns nonnull, otherwise choose the idle sequence. Release the item on
both present-item paths. Missing items also select idle. The existing
GetIdleSequence helper represents the inlined loaded/saveable-area test:
READY (7) there, otherwise HEAD_TURN (6). Other sequence bytes are untouched.

CMessageSetSequence::Run already calls this helper before SetSequence
(except animation ID 0xEC0B). Its existing implementation is unchanged.
This closes an actual missing check on melee/ranged animation messages,
not a change to hit/damage calculations.

Sixty-four typed cases combine four sequence bytes (ATTACK, SHOOT, CAST,
255), four weapon states (null, empty item, valid fist ability, invalid
0xFFFF ability), and four area states (null, unloaded, loaded unsaveable,
loaded saveable). The save's fist ability is checked before testing.
Each case has a fixed expected sequence. Equipment slot/pointers,
selection/ability, area pointer/load/save flags and game loading flag are
restored synchronously before returning to the engine tick. Earlier 393
assertions remain, for 457 total. No save is written.

Validation: full batch 2026-10-07T19-30-34-192397-batch passes eight shared
lints, class/layout audits and Win32 Debug build. Parity is GREEN. All
457 typed assertions and 11 scenario steps pass, followed by 60 seconds
CLEAN with CheckSequence executed 1,486 times. Fixtures removed and
preserved paths restored and hashed. Member-offset audit skips as not
analysable; static evidence remains incomplete despite GREEN parity.
No original/recovered visual differential is claimed.

Next: combat animation/message dispatch and remaining projectile execution
gaps, with actual melee/bow/crossbow/sling damage scenarios as regression gates.
