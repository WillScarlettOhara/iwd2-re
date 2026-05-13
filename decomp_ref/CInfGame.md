# CInfGame — BG2EE PDB Field Mapping

## BG2 ↔ IWD2 layout differences (m_lastTarget area)

| BG2 (CInfGame, sizeof 24376) | IWD2 (CInfGame, sizeof 19124) |
|---|---|
| `m_nLastSaveTime` @ 23936 (ULONG) | `m_nLastSaveTime` @ 4AA2 (ULONG) |
| `m_gameTime` @ 23940 (ULONG) | **NOT IN IWD2** |
| `m_zoomLevel` @ 23944 (ULONG) | **NOT IN IWD2** |
| `m_zoomEnabled` @ 23948 (ULONG) | **NOT IN IWD2** |
| `m_lastClick` @ 23952 (CPoint) | `m_lastClick` @ 4AA6 (CPoint) |
| `m_lastTarget` @ 23960 (LONG) | `m_lastTarget` @ 4AAE (LONG) |
| `m_bTriggerOutline` @ 23964 (unsigned char) | `m_bTriggerOutline` @ 4AB2 (BOOLEAN) |
| `m_entanglePalette` @ 23968 (CVidPalette) | `m_entanglePalette` @ 4AB4 (CVidPalette) |

## BG2 ↔ IWD2 layout mapping (save screen area)

| BG2 (CInfGame) | IWD2 (CInfGame) |
|---|---|
| `m_cVRamPool` @ 15464 | `m_cVRamPool` @ 3662 |
| `m_bSaveScreen` @ 15472 (int) | `m_bSaveScreen` @ 366E (int) |
| `m_sSaveScreenArea` @ 15476 (CString) | **NOT IN IWD2 at this location** |
| `m_rgbMasterBitmap` @ 15480 | `m_rgbMasterBitmap` @ 3672 |

## BG2 ↔ IWD2 layout mapping (party voice prompts)

| BG2 (CInfGame) | IWD2 (CInfGame) |
|---|---|
| `m_variables` @ 23496 | `m_variables` @ 47FC |
| `m_namedCreatures` @ 23504 | `m_namedCreatures` @ 4808 |
| `m_soundNeedParty` @ 23512 | `m_soundNeedParty` @ 4814 (`STR_RES`) |
| `m_soundAreaTransitionRefused` @ 23536 | `m_soundAreaTransitionRefused` @ 487C (`STR_RES`) |
| `m_bForceDither` @ 23560 | `m_bForceDither` @ 48E4 |

## Key findings

- **field_4AB2** was declared as `BOOL` (4 bytes in Win32) but Ghidra decomp confirms 1-byte access: `*(undefined1*)(this + 0x4AB2)`. Corrected to `BOOLEAN` (1 byte).
- BG2EE added `m_gameTime`, `m_zoomLevel`, `m_zoomEnabled` between `m_nLastSaveTime` and `m_lastClick` — these don't exist in IWD2.
- 1 byte of padding at 0x4AB3 before `m_entanglePalette` (alignment).
- Field only initialized to 0 in constructor and `InitGame`; no reads found in current C++ source or Ghidra decomp.
- Ghidra calls it `m_bTriggerOutline` in BG2EE PDB — same relative position, same type (1 byte). Name adopted.
- **field_366E** maps to BG2EE `m_bSaveScreen`: same relative position between `m_cVRamPool` and `m_rgbMasterBitmap`, and PDB type is `int`. IWD2 does not have BG2EE's adjacent `m_sSaveScreenArea` field at this location.
- **field_4814** and **field_487C** map to BG2EE `m_soundNeedParty` and `m_soundAreaTransitionRefused`: same relative position between `m_namedCreatures` and `m_bForceDither`. IWD2 stores them as `STR_RES` and initializes them via `CTlkTable::Fetch(16484, ...)` and `Fetch(15307, ...)`.

## Methodology

1. Decompile IWD2 function with GhidraSQL → identify access width (`*(undefined1*)` = 1 byte)
2. Extract BG2EE CInfGame field list from `bg2_pdb_types.txt` (LF_CLASS @ 0x11C8E, field list @ 0x11C8D)
3. Match by neighboring fields (m_lastTarget → m_entanglePalette sandwich)
4. Rename in C++ source (fields not stored in Ghidra DB)
