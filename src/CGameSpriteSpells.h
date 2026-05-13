#ifndef CGAMESPRITESPELLS_H_
#define CGAMESPRITESPELLS_H_

#include <vector>

#include "FileFormat.h"

// NOTE: For unknown reason almost all function params are references instead of
// plain values. I've added `const` qualifier to make their use a little bit
// easier and to identify which references are actually out params. That in
// turn resulted in pretty ugly interface.
//
// Also since many fields remains unclear, there is mess with signness.

class CGameSpriteSpellListEntry {
public:
    /* 0000 */ UINT m_nID;
    /* 0004 */ unsigned int m_nMax;
    /* 0008 */ unsigned int m_nCurrent;
    /* 000C */ unsigned int field_C;
};

class CGameSpriteSpellList {
public:
    CGameSpriteSpellList();
    ~CGameSpriteSpellList();
    BOOLEAN Add(const UINT& nID, const unsigned int& a2, const unsigned int& a3, const unsigned int& a4);
    BOOLEAN Remove(const UINT& nID, BOOLEAN a2, const unsigned& a3, const unsigned& a4);
    BOOLEAN AddToCurrentCount(const UINT& nID, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN SubtractFromCurrentCount(const UINT& nID, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN AddToSharedCurrentCount(const unsigned int& a1, const BOOLEAN& a2);
    BOOLEAN SubtractFromSharedCurrentCount(const unsigned int& a1, const BOOLEAN& a2);
    BOOLEAN Find(const UINT& nID, UINT& nIndex);

    BOOLEAN CheckF8(UINT nIndex);
    CGameSpriteSpellListEntry* Get(UINT nIndex);

    void Clear();

    /* 0000 */ std::vector<CGameSpriteSpellListEntry> m_List;
    /* 0010 */ unsigned int field_10;
    /* 0014 */ unsigned int field_14;
    /* 0018 */ unsigned int field_18;
};

class CGameSpriteGroupedSpellList {
public:
    CGameSpriteGroupedSpellList();
    ~CGameSpriteGroupedSpellList();
    CGameSpriteSpellList* GetSpellsAtLevel(const UINT& nLevel);
    UINT GetNumSpells();
    UINT GetTotalCurrentCount();
    BOOLEAN Add(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4, const unsigned int& a5);
    BOOLEAN Remove(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4, const unsigned int& a5);
    BOOLEAN AddToCurrentCountAtLevel(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4);
    BOOLEAN SubtractFromCurrentCountAtLevel(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4);
    BOOLEAN AddToSharedCurrentCountAtLevel(const UINT& nLevel, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN SubtractFromSharedCurrentCountAtLevel(const UINT& nLevel, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN Find(const UINT& nID, const UINT& nLevel, UINT& nIndex);

    void Clear();

    /* 0000 */ CGameSpriteSpellList m_lists[CSPELLLIST_MAX_LEVELS];
    /* 0048 */ UINT m_nHighestLevel;
};

class CGameSpriteSpells {
public:
    CGameSpriteGroupedSpellList* Get(const UINT& nClassIndex);
    CGameSpriteSpellList* GetSpellsAtLevel(UINT nClassIndex, UINT nLevel);

    /* 0000 */ CGameSpriteGroupedSpellList m_spellsByClass[CSPELLLIST_NUM_CLASSES];
};

#endif /* CGAMESPRITESPELLS_H_ */
