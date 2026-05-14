# World Screen Critical Stubs

Functions blocking CScreenWorld from rendering the game world. Ordered by priority.

## Address table

| Func | Address | File, Line | Complexity |
|------|---------|-----------|------------|
| `CInfGame::SaveGame` | 0x5AC430 | CInfGame.cpp:1987 | HIGH - writes GAM/ARE/CRE/save package |
| `CInfGame::Unmarshal` | 0x5A7E40 | CInfGame.cpp:1995 | HIGH - parses ICEWIND2.GAM and reconstructs areas/party |
| `CGameAIBase::ProcessAI` | ? (vtable 0x8C) | CGameAIBase.cpp:335 | HIGH - AI state machine |
| `CInfGame::SynchronousUpdate` | 0x5BE900 | CInfGame.cpp:4793 | HIGH - frame sync |
| `CScreenWorld::TogglePauseGame` | n/a | CScreenWorld.cpp:1449 | LOW - pause toggle |
| `CGameArea::GetTintColor` | 0x470F10 | CGameArea.cpp:1764 | MEDIUM - bilinear lum interp |
| `CGameSprite::RenderSpriteEffect` | n/a | CGameSprite.cpp:6107 | MEDIUM - vfx rendering |
| `CGameSprite::RenderDamageArrow` | n/a | CGameSprite.cpp:6101 | LOW - damage numbers |
| `CGameSprite::PlaySound(BYTE,...)` | n/a | CGameSprite.cpp:3761 | LOW - sound set play |
| `CGameArea::GetNearest` | n/a | CGameArea.cpp:499 | HIGH - AI targeting |

## New Game startup bypass (temporary)

Observed after BIFC fix:
- `SetupCharacters()` loads `AR1000`, sets `m_visibleArea=0`, adds 6 imported sprites.
- `CScreenSinglePlayer::OnMainDoneButtonClick()` then calls `SaveGame(0, 1, 0)`, `DestroyGame(FALSE, TRUE)`, `LoadGame(FALSE, TRUE)`.
- `SaveGame` is still a stub returning `FALSE`.
- `LoadGame` calls `Unmarshal(ICEWIND2.GAM...)`; `Unmarshal` is still a stub returning `FALSE`.
- Result: `DestroyGame` removes loaded area/party, `LoadGame` reconstructs nothing, then `WorldEngineActivated` sees missing visible area and loading screen sticks.

Temporary fix:
- In local single-player (`SERV_PROV_NULL`) only, when startup `SaveGame` returns `FALSE`, skip `DestroyGame/LoadGame` and keep `SetupCharacters()` state.
- Then select all party chars, select toolbar, start `CScreenChapter` with `CHPTXT0` and select chapter engine. This restores first prologue screen path until full `SaveGame`/`Unmarshal` implementation.

## Prologue/chapter text

- First new-game text resource: `CHPTXT0.2DA`.
- `CScreenChapter::StartChapter(CResRef("CHPTXT0"))` calls `CRuleTables::GetChapterText()` and uses current `CHAPTER` global.
- Original `NewGame()` initializes `CHAPTER` global `m_intValue = -1` (confirmed from decomp at 0x5ABA20). First chapter transition sets chapter to `1`, then `CHPTXT0` row `DEFAULT` gives title/text strrefs (`16202`, `15879`, ...).

## GetTintColor Analysis (0x470F10)

**Algorithm**: Bilinear interpolation of light map luminance values
- Divides world position by (16, 12) to get light grid coordinates
- Reads 4 nearby luminance pixels from `m_bmLum` (CVidBitmap at CGameArea+0x258)
- If night map exists (`m_pbmLumNight` at CGameArea+0x318), blends with night values
- Flag byte at CGameArea+0x624 controls day/night blending weight
- Returns RGB tint color for positional lighting

**Dependencies**: Needs a `CVidBitmap` method to read lum pixels at grid coords (currently `FUN_007b2f60`)

## SynchronousUpdate Analysis (0x5BE900)

Key pattern:
1. Guard: `m_bUpdateEnabled` field at CInfGame+0x366E
2. Thread sync: `EnterCriticalSection`/`LeaveCriticalSection` on area locks
3. Area iteration: processes each CGameArea's update state
4. Cleanup: unlock all held locks

~400 lines of decompiled code. Full decomp: `/tmp/syncupdate.txt`

## ProcessAI Analysis

Called from CGameAIbase::AIUpdate via vtable at offset 0x8C:
```
(**(code **)(*this + 0x8C))();
```

CGameSprite::AIUpdate is at 0x6F5FF0 (FUN_006f5ff0). ProcessAI address not directly in Ghidra DB because it's a virtual function resolved at runtime.

## Implementation Workflow

1. **Decompile**: `ghidrasql -q "SELECT text FROM pseudocode WHERE func_addr = 0xNNNNNN;"`
2. **Map fields**: Find matching C++ field names in the class header
3. **Translate**: Convert decompiled pseudocode to clean C++ using existing APIs
4. **Build**: `cmake --build build --config Debug` on Windows
5. **Commit**: Only after successful build
