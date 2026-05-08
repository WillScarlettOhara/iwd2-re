#include "CMoveList.h"

#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CInfGame.h"

// 0x5170A0
CMoveListEntry::CMoveListEntry()
{
    m_nSpriteId = CGameObjectArray::INVALID_INDEX;
    m_areaResRef = "";
    m_ptDestination.x = 0;
    m_ptDestination.y = 0;
    m_nFacing = 0;
    m_nDelay = 0;
}

// NOTE: Inlined.
CMoveList::CMoveList()
{
}

// 0x59EAD0
CMoveList::~CMoveList()
{
    ClearAll();
}

// 0x4E71B0
void CMoveList::ClearAll()
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CMoveListEntry* pEntry = GetNext(pos);
        delete pEntry;
    }
    RemoveAll();
}

// 0x517210
void CMoveList::CheckLoad(CGameArea* pArea)
{
    CResRef areaResRef = pArea->m_resRef;

    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CMoveListEntry* pNode = GetNext(pos);
        if (pNode->m_areaResRef == areaResRef) {
            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pNode->m_nSpriteId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                pSprite->MoveOntoArea(pArea, pNode->m_ptDestination, pNode->m_nFacing);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pNode->m_nSpriteId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }
}

// 0x517300
void CMoveList::AIUpdate()
{
    // TODO: Incomplete.
}

// Phase 1-2: Scaffold functions
// 0x516CD0
void FUN_00516cd0() {
    // TODO: Incomplete.
}

// 0x516D50
void FUN_00516d50() {
    // TODO: Incomplete.
}

// 0x516E90
void FUN_00516e90() {
    // TODO: Incomplete.
}

// 0x516F10
void FUN_00516f10() {
    // TODO: Incomplete.
}

// 0x516FC0
void FUN_00516fc0() {
    // TODO: Incomplete.
}

// 0x516FD0
void FUN_00516fd0() {
    // TODO: Incomplete.
}

// 0x517000
void FUN_00517000() {
    // TODO: Incomplete.
}

// 0x5170F0
void FUN_005170f0() {
    // TODO: Incomplete.
}

// 0x59E980
void FUN_0059e980() {
    // TODO: Incomplete.
}

