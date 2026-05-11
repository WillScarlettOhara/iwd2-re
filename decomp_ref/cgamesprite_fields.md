# CGameSprite Field Analysis

## Fields with high-confidence mappings

### field_54A8 → m_bBumpable (BOOL)
- Type: `int` (BOOL)
- Passed as `bBumpable` parameter to `CSearchBitmap::AddObject/RemoveObject`
- Set to 0 in constructor, read during AddToArea/RemoveFromArea/AIUpdate
- Signature context: `AddObject(point, sourceSide, personalSpaceRange, field_54A8, field_7430)`
- Cross-referenced from: CGameSprite.cpp, CAIGroup.cpp, CMessage.cpp, CInfGame.cpp, CSearchBitmap.cpp

### field_7430 → m_bOnSearchMap (BOOLEAN&)
- Type: `unsigned char`
- Passed as `bOnSearchMap` in-out ref to `CSearchBitmap::AddObject/RemoveObject`
- The function updates this to indicate whether the object was (or is) on the search map
- Set to 0 in second constructor (line 1087)
- Cross-referenced from: CGameSprite.cpp, CAIGroup.cpp, CMessage.cpp, CInfGame.cpp

### field_722A → m_dwLastUpdateTime (ULONG)
- Type: `ULONG`
- Initialized in constructor from `g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime`
- Timestamp of last sprite update
- Cross-referenced from: CGameSprite.h, CGameSprite.cpp

### field_9D15 → m_bMirrorImage (BOOLEAN)
- Type: `unsigned char`
- Set to 1 in IcewindCGameEffects for: lich touch, soul eater, dragon gem, and general effect application
- Set to 0 in CGameSprite constructor
- Used to track special visual effect state (likely mirror images)
- Cross-referenced from: CGameSprite.cpp, CGameEffect.cpp, CInfGame.cpp, IcewindCGameEffects.cpp

## Fields needing more analysis

### field_562C → flag (BOOL-like, 7 cross-refs)
- Set to 0 in first constructor, TRUE in second constructor, 1 in AddToArea
- sub_4531B0() sets it to 1 unconditionally
- Cross-referenced: CBounceList.cpp, CGameEffect.cpp, CScreenCreateChar.cpp, CScreenWorld.cpp, CScreenStore.cpp

### field_5640 → counter/flag (3 cross-refs)
- Set to 0 in constructor, between m_hasColorRangeEffects and m_removeFromArea
- Read in CGameEffect.cpp (effect application)

### field_5304 → sprite effect flag (int)
- Set to 0 in constructor, near m_spriteEffectFlags
- Likely related to sprite effects rendering

### field_3D14[9] → feat modes?? (9 ints)
- Between quick spells (0x3D0C) and m_nLastSpellbookClassIndex (0x3D38)
- Size 9 = matches quick spell slot count
- May be prepared spell counts per class

### field_3D3A[8] → ability indices?? (8 bytes)
- Between m_nLastSpellbookClassLevel (0x3D39) and m_nLastSong (0x3D42)
- Size 8 = matches number of classes

### field_4C54[5] → feat slots?? (5 ints)
- Between m_nModalState (0x4C53) and m_nWeaponSet (0x4C68)
- Size 5 = matches MAX_SELECTABLE_FEAT_USE_LEVELS

## Priority for future renaming
1. field_54A8 (m_bBumpable) - 6 refs, clear meaning
2. field_7430 (m_bOnSearchMap) - 5 refs, clear meaning
3. field_722A (m_dwLastUpdateTime) - 2 refs, clear meaning
4. field_9D15 (m_bMirrorImage) - 5 refs, needs PDB confirmation
5. field_562C - 7 refs, needs more analysis
6. field_5640 - 3 refs, needs analysis
7. Arrays need analysis via feat/skill handling functions
