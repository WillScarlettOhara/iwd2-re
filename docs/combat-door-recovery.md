# Door interaction and rendering recovery

Five door methods now replace inherited fallbacks or missing virtual slots:
`CompressTime` (0x487530, slot 0x2C), `OnActionButton` (0x487D10, 0x40),
`Render` (0x488B30, 0x4C), `RenderClosedPoly` (0x48B3C0, 0xB8) and
`RenderOpenPoly` (0x48B600, 0xBC). The last two names remain `#guess`; their
four stack arguments and open/closed geometry are established by assembly.

The click path sends UseDoor (action 142) to selected members and adds a
MoveToPoint order when every member is outside the weighted approach radius.
It retains LONG wrapping, BYTE selection count and seven-character area exceptions:
ar5004 divides the radius
by ten, ar5202 by five. Bash uses a leader action and a null primary action;
picklock/disarm use the primary action. Zero base skill ranks (CRE skill indices
10/5) leave targeting active. Ground fallbacks and magic targeting retain their
distinct state resets. The CAIAction copy bug (specificID2 into specificID3,
including the move's -1) is retained.

CompressTime subtracts the flash counter with signed-SHORT to unsigned-DWORD
comparison. Only a delta strictly greater than 4500 permits automatic closure
and clears flag 0x1000. The close path checks flag 0x20, occupied search cells and
rand()%5, updates position/tile/search masks, and posts DoorStatus only when the
host byte equals TRUE. It does not advance the world timer.

Render restores hover, tool/bashing, secret-door flash and Tab outlines, software
fills and clipped polygon grids. The owner area's viewport defines clipping;
the supplied area's Infinity performs the FX operations. Assembly also establishes
the previously reversed RenderConvexPoly argument order: polygon first, rectangle
second. Its declaration, definition and container/trigger callers are corrected
together; its body is unchanged.

## Evidence and limits

Context bundles were read completely, including call sets, BG2 layout and headers.
Final bundle `tmp_combat_doors_final_ctx.md` has
SHA-256 `47c36364614c3e2e59307448c0fe612a7c167f42766a65c45a8fad54537ac4b0`.
Its five function-export hashes match the previously read inputs; final shared
blocks were read again. Assembly was read from entry through return, excluding
OnActionButton's jump tables. ConvexPoly's complete context preceded its ABI fix.

`scripts/profiles/combat-door.json` centers the save, tests a loaded door,
inspects action messages, checks flash/range/skill
boundaries, both random outcomes, occupied-cell rejection and host messages,
then repeats target/movement/projectile and damage regressions.
The polygon checks draw two colors onto the software back surface and inspect
a changed interior pixel. They do not establish original/recovered visual parity.
Actions/messages are removed before ordinary ticks; actor, group, formation,
search pointers, geometry, host state and targeting are restored without saving.

VS2019/v142 Win32 clean build passed. Native run
`2026-10-09T23-12-42-587994-batch` passed 38 steps and 2796 assertions
(146 door checks), with all six symbol gates observed and a CLEAN crash guard.
Melee delivered 6 damage; bow, crossbow and sling delivered 12 each and consumed
one ammunition charge. Both polygon checks changed pixels. Profile cleanup
restored and hashed preserved files. CompressTime/Render parity is GREEN;
click/polygon helpers and ConvexPoly are YELLOW on call counts. Optional offset
and automatic layout checks were skipped; no differential equivalence is claimed.

The global class audit fails on the pre-existing CGameDoor::AddEffect omission,
0x486860 (slot 0x78), without suppression or placeholder override. The generic
handler's comment now identifies the gap.

## Continuation

Next: AddEffect (knock, traps, forced casting, summoning). Preliminary decompile/
callset `tmp_door_effect_ctx.md` needs a named marker and complete class/header
bundle before implementation. Formation executor 0x73EA80 (caller 0x729646)
remains a subsequent combat movement target.
