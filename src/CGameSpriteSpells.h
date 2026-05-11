#ifndef CGAMESPRITESPELLS_H_
#define CGAMESPRITESPELLS_H_

// ============================================================================
// CGameSpriteSpells - Spell lists - memorized, innate, bard songs, shapesifts
//
// Address: varies by method
// ============================================================================
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
    /* 0004 */ unsigned int nm_field_4;
    /* 0008 */ unsigned int nm_field_8;
    /* 000C */ unsigned int nfield_C;
};

class CGameSpriteSpellList {
public:
    CGameSpriteSpellList();
    ~CGameSpriteSpellList();
    BOOLEAN Add(const UINT& nID, const unsigned int& a2, const unsigned int& a3, const unsigned int& a4);
    BOOLEAN Remove(const UINT& nID, BOOLEAN a2, const unsigned& a3, const unsigned& a4);
    BOOLEAN AddCasts(const UINT& nID, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN RemoveCasts(const UINT& nID, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN AddAllCasts(const unsigned int& a1, const BOOLEAN& a2);
    BOOLEAN RemoveAllCasts(const unsigned int& a1, const BOOLEAN& a2);
    BOOLEAN Find(const UINT& nID, UINT& nIndex);

    BOOLEAN CheckF8(UINT nIndex);
    CGameSpriteSpellListEntry* Get(UINT nIndex);

    void Clear();

    /* 0000 */ std::vector<CGameSpriteSpellListEntry> m_List;
    /* 0010 */ unsigned int nm_field_10;
    /* 0014 */ unsigned int nm_field_14;
    /* 0018 */ unsigned int nm_field_18;
};

class CGameSpriteGroupedSpellList {
public:
    CGameSpriteGroupedSpellList();
    ~CGameSpriteGroupedSpellList();
    CGameSpriteSpellList* GetSpellsAtLevel(const UINT& nLevel);
    UINT GetNumSpells();
    UINT CountTotalCasts();
    BOOLEAN Add(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4, const unsigned int& a5);
    BOOLEAN Remove(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4, const unsigned int& a5);
    BOOLEAN AddCasts(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4);
    BOOLEAN RemoveCasts(const UINT& nID, const UINT& nLevel, const unsigned int& a3, const unsigned int& a4);
    BOOLEAN AddAllCasts(const UINT& nLevel, const unsigned int& a2, const BOOLEAN& a3);
    BOOLEAN RemoveAllCasts(const UINT& nLevel, const unsigned int& a2, const BOOLEAN& a3);
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
