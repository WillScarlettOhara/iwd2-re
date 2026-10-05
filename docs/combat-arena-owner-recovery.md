# ProcessAI arena visibility and affiliation

Recovered 0x72BAD7..0x72BB9A inside ProcessAI (0x72B9A0), using its shared
full context bundle and original instructions. Game offset 0x43D8 resolves
to m_gameSave.m_bArenaMode (0x422C + 0x1AC). A non-null network provider
and foreign owner identify the arena opponent.

If the sprite has an area and its ID is on that area's visibility map,
RemoveCharacter receives m_pos, m_id and m_visibleTerrainTable. Independently
of area/map membership, a current EA <= GOODCUTOFF (30) becomes ARENAPC
(253). The binary directly writes three bytes: m_typeAI at 0x24,
m_liveTypeAI at 0x3DFE and m_startTypeAI at 0x3E3A. No SetAIType side effects
are added. The threshold is unsigned and inclusive. Outside arena mode,
with null provider or local ownership, neither operation occurs.

Eight typed AutoUI cases call actual ProcessAI: arena disabled, null
provider, local owner, foreign owner at EA 30, foreign owner at EA 31,
foreign owner at EA 0, null area and absent visibility membership. They
assert map membership and all three EA bytes, including preservation of
distinct live/start sentinel bytes when conversion is skipped. The null-area
case returns at the later foreign-owner gate before normal action handling.
Visibility IDs and the complete map byte buffer are restored from snapshots
between cases and before the next tick. Arena/provider/owner/EA/area state
is restored synchronously. The process is disposable; no save is written.

The first run .arc/2026-10-05T19-16-40-135543-batch passed all 94 case
assertions and 60-second observation, but its overall verdict is ERROR:
the layout header resolver selected CGameSprite.h's forward declaration
of CVisibilityMap rather than CVisibility.h's definition. The resolver
now requires a class/struct definition in fallback candidates. Two
regression tests and the fourteen existing batch tests pass. Only this
resolver hunk is staged from arc.py; unrelated workflow edits are retained.

Final validation: .arc/2026-10-05T19-19-45-629143-batch, PASS with one
warning and two skips. Eight source checks, class audits, sprite layout
audit and Win32 build pass. All 94 case assertions and eleven UI steps
pass; CLEAN observation for 60 seconds. ProcessAI hits 93441 times and
RemoveCharacter four times. Save/config paths are restored and hashed.
ProcessAI parity is YELLOW (191/241 original/source calls); unchanged
RemoveCharacter is GREEN. Offset analysis remains unavailable. Visibility
layout is skipped because the tool's annotation detector does not recognize
this header's bare hexadecimal offset comments; its complete layout is
therefore not established by this batch. This establishes local arena routing, not a real
multiplayer match or full original/recovered ProcessAI equivalence.

The subsequent walking/idle gate is corrected in
[combat-walk-idle-recovery.md](combat-walk-idle-recovery.md).
