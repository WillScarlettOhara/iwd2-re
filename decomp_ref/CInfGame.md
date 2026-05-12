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

## Key findings

- **field_4AB2** was declared as `BOOL` (4 bytes in Win32) but Ghidra decomp confirms 1-byte access: `*(undefined1*)(this + 0x4AB2)`. Corrected to `BOOLEAN` (1 byte).
- BG2EE added `m_gameTime`, `m_zoomLevel`, `m_zoomEnabled` between `m_nLastSaveTime` and `m_lastClick` — these don't exist in IWD2.
- 1 byte of padding at 0x4AB3 before `m_entanglePalette` (alignment).
- Field only initialized to 0 in constructor and `InitGame`; no reads found in current C++ source or Ghidra decomp.
- Ghidra calls it `m_bTriggerOutline` in BG2EE PDB — same relative position, same type (1 byte). Name adopted.

## Methodology

1. Decompile IWD2 function with GhidraSQL → identify access width (`*(undefined1*)` = 1 byte)
2. Extract BG2EE CInfGame field list from `bg2_pdb_types.txt` (LF_CLASS @ 0x11C8E, field list @ 0x11C8D)
3. Match by neighboring fields (m_lastTarget → m_entanglePalette sandwich)
4. Rename in C++ source (fields not stored in Ghidra DB)
