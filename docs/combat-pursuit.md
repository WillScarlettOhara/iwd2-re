# Movement recovery — 2026-09-19

Completed the two deferred branches in CGameSprite::MoveToObject (0x73EDD0):
MoveToObjectUntilSee (263) records the newly seen object and visibility delay,
and MoveToObjectFollow (180) posts DropPath when already close to its target.
The ordinary attack approach path was already implemented.

Full context: `tmp_moveobject_ctx.md` and its provenance manifest. PE assembly
at 0x73EE3B–0x73F00A proves the world-coordinate half-visual-range test, LOS
argument order, same-area check, m_lSeen comparison/Set, visibility delay and
CMessageSetLastObject with SEE (0x401C), caller and recipient both the actor.
The vtable at 0x848998 is SetLastObject, not a follow message. The vtable at
0x84C44C identifies DropPath. Its allocation at 0x73F1BD and dispatch at
0x73F1ED complete the follow-stop branch. CAIObjectType::Equal matches the
inlined field comparison exactly. The two existing near/far path-request
branches were checked against `tmp_moveobject_path_asm.txt` and remain merged.

The combat-pursuit scenario checks 12 conditions, including null target,
newly/already seen target, the SEE message identity and absence of duplicates,
and follow-stop messages with/without a path. These synchronous checks restore
actor state and discard their messages; they do not prove message delivery.

First run `.arc/2026-09-19T23-20-37-294734-batch/` built and passed those
checks, but its pursuit test failed: the preparatory ground click at (400,216)
did not move the actor. Attacks then damaged the companion without movement.
The reason that ground destination did not produce movement is not established.
The failure and its logs are retained.

Second run `.arc/2026-09-19T23-24-07-812627-batch/` uses JumpToPoint to place
the attacker at world (232,1002), with zero encumbrance, before clicking the
normal weapon button and party member 1. Both party members were first checked
visible after camera centering; the ground picker must identify the target.
The attack moves the source to (232,852), then reduces target HP 291 -> 273.
All 21 steps pass. Preparation uses a test operation, but pursuit and damage
run through the normal UI order and AI ticks. No attack-frame forcing or hit
forcing is used. The scenario stops the attack after movement and damage.
VS2019 Win32 build passed, followed by 90 seconds CLEAN. Save/config restoration
passed hash verification with no cleanup errors.
MoveToObject ran 31 times and Attack 57 times.

Static limitations: MoveToObject offset auditing is non-analysable, string
provenance warns, and call-count parity is YELLOW (42 original / 36 source).
Attack retains its prior call-count warning. No matched original/reconstruction
capture exists. Guard tests cover the restored branches directly; ExecuteAction
dispatch for actions 180 and 263 is outside this change.

Next: ordinary UI attacks with bow, crossbow and sling separately; original
comparison for target visibility and pursuit; remaining action dispatches.
