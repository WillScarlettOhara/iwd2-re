#ifndef CMOVELIST_H_
#define CMOVELIST_H_

#include "mfc.h"

#include "CResRef.h"

class CGameArea;

// IE packs binary-mirror classes to 2 (see CLAUDE.md); without it m_nDelay
// would silently 4-align to 0x18 instead of the binary's 0x16.
#pragma pack(push)
#pragma pack(2)

class CMoveListEntry {
public:
    CMoveListEntry();

    /* 0000 */ LONG m_nSpriteId;
    /* 0004 */ CResRef m_areaResRef;
    /* 000C */ CPoint m_ptDestination;
    /* 0014 */ BYTE m_nFacing;
    /* 0016 */ LONG m_nDelay;
};

#pragma pack(pop)

class CMoveList : public CTypedPtrList<CPtrList, CMoveListEntry*> {
public:
    CMoveList();
    ~CMoveList() override;
    void ClearAll();
    void CheckLoad(CGameArea* pArea);
    void AIUpdate();
};

#endif /* CMOVELIST_H_ */
