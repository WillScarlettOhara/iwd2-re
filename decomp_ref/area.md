# CGameArea / CGameDoor / CGameTrigger — ARE File Format Mapping

## Source of truth
- **NearInfinity**: `AreResource.java` — IWD2 ARE v9.1 format parser
- **IESDP**: `/tmp/iesdp/iesdp-gh-pages/opcodes/iwd2.htm`

## ARE v9.1 Header (CAreaFileHeader, 0x5C bytes)

IWD2 inserts 16 extra bytes at offset 0x54 (after `m_windSpeed` and before the section offset table):

| Offset | Type | Field | Proposed Name | ARE v9.1 Meaning |
|--------|------|-------|---------------|------------------|
| 0x4C | DWORD | `field_4C` | `m_areaDifficulty2` | BYTE at ARE+0x54 (area difficulty 2) |
| 0x50 | DWORD | `field_50` | `m_areaDifficulty3` | BYTE at ARE+0x55 (area difficulty 3) |
| 0x54 | DWORD | `field_54` | `m_areaCurrentDifficulty` | WORD at ARE+0x56 (current difficulty) |
| 0x58 | DWORD | `field_58` | `m_notUsed` | Start of 12-byte Unknown block at ARE+0x58 |

## CGameArea unnamed fields

| Offset | Type | Field | Evidence | Proposed Name |
|--------|------|-------|----------|---------------|
| 0x1FC | CRITICAL_SECTION | `field_1FC` | Enter/LeaveCriticalSection in AddObject, ClearMarshal | `m_csObjectListLock` |
| 0x214 | CRITICAL_SECTION | `field_214` | Init/Delete called but never Entered in implemented code | `m_csSecondaryLock` |
| 0x241 | unsigned char | `field_241` | Set to TIMESCALE_MSEC_PER_SEC on mouse-down, decremented each frame | `m_nMouseDownTimer` |
| 0x3EC | short | `field_3EC` | Only zero-initialized, never used | `field_3EC` (unknown) |
| 0x436 | unsigned char | `field_436` | Only zero-initialized | `field_436` (unused) |
| 0x438 | int | `field_438` | Only zero-initialized | `field_438` (unused) |
| 0xAE6 | unsigned char | `field_AE6` | Set to 1 in ctor, never read | `field_AE6` (unused flag) |
| 0xB0E | int | `field_B0E` | Only zero-initialized | `field_B0E` (unused) |
| 0xB16 | BOOL | `field_B16` | Set by CInfGame::UpdateAreaSaveStatus() to !CanSaveGame(); read by IsAreaSaveable() | `m_bSaveBlocked` |

## CGameDoor unnamed fields

| Offset | Type | Field | Purpose | Proposed Name |
|--------|------|-------|---------|---------------|
| 0x6DA | CArray<CRect*> | `field_6DA` | Closed door bounding rects for IsOver detection | `m_aClosedBoundingRects` |
| 0x6EE | CAreaPoint* | `field_6EE` | Marshal helper: converts m_pClosedPolygon to CAreaPoint | `m_pClosedAreaPoints` |
| 0x6F2 | CArray<CRect*> | `field_6F2` | Open door bounding rects for IsOver | `m_aOpenBoundingRects` |
| 0x706 | CAreaPoint* | `field_706` | Marshal helper for open polygon | `m_pOpenAreaPoints` |

## CGameTrigger unnamed fields

| Offset | Type | Field | Purpose | Proposed Name |
|--------|------|-------|---------|---------------|
| 0x628 | CPoint | `field_628` | Likely the "use point" (override/alternate activation point) | `m_usePoint` |
| 0x630 | CArray<CRect*> | `field_630` | Bounding rects for IsOver detection | `m_aBoundingRects` |
| 0x644 | CAreaPoint* | `field_644` | Marshal helper: polygon vertices as CAreaPoint | `m_pAreaPoints` |

## CAreaFileTriggerObject (FileFormat.h) unnamed fields

| Offset | Type | Field | IESDP Trigger Offset | Proposed Name |
|--------|------|-------|---------------------|---------------|
| 0x88 | LONG | `field_88` | 0x88 (IWD2 alternate point X) | `m_alternatePointX` |
| 0x8C | LONG | `field_8C` | 0x8C (IWD2 alternate point Y) | `m_alternatePointY` |

Note: IWD2 triggers also have override points at offset 0x84-0x87 (two WORDs), currently missing from CAreaFileTriggerObject — there's a gap between fields at 0x84-0x88.

## IWD2 ARE v9.1 Section Offsets (post-header)

After the 16-byte IWD2 extension at 0x54, the offset table shifts by +16:

| Offset | Type | Field |
|--------|------|-------|
| 0x64+16 | DWORD | Actors offset |
| 0x68+16 | WORD | # actors |
| 0x6A+16 | WORD | # triggers |
| 0x6C+16 | DWORD | Triggers offset |
| ... | ... | ... |

References: `AreResource.java` methods `read()` and sub-structures `Door.java`, `ITEPoint.java`, `Song.java`.
