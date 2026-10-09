#include "CGameDoor.h"

#include "CAIScript.h"
#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CInfButtonArray.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CScreenWorld.h"
#include "CTimerWorld.h"
#include "CUtil.h"
#include "CVidPoly.h"
#include "CVidInf.h"

// 0x8D41D8
const LONG CGameDoor::RANGE_DOOR = 16
    * CPathSearch::GRID_SQUARE_SIZEX
    * CPathSearch::GRID_SQUARE_SIZEX;

// 0x485AC0
CGameDoor::CGameDoor(CGameArea* pArea, CAreaFileDoorObject* pDoorObject, CAreaPoint* pPoints, WORD maxPts)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 107
    UTIL_ASSERT(pArea != NULL && pDoorObject != NULL && pPoints != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 108
    UTIL_ASSERT(pDoorObject->m_openSelectionPointStart + pDoorObject->m_openSelectionPointCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 109
    UTIL_ASSERT(pDoorObject->m_closedSelectionPointStart + pDoorObject->m_closedSelectionPointCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 110
    UTIL_ASSERT(pDoorObject->m_openSearchSquaresStart + pDoorObject->m_openSearchSquaresCount <= maxPts);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 111
    UTIL_ASSERT(pDoorObject->m_closedSearchSquaresStart + pDoorObject->m_closedSearchSquaresCount <= maxPts);

    m_objectType = TYPE_DOOR;
    m_resID = pDoorObject->m_doorID;

    m_rOpenBounding.left = pDoorObject->m_openBoundingRectLeft;
    m_rOpenBounding.top = pDoorObject->m_openBoundingRectTop;
    m_rOpenBounding.right = pDoorObject->m_openBoundingRectRight;
    m_rOpenBounding.bottom = pDoorObject->m_openBoundingRectBottom;

    m_rClosedBounding.left = pDoorObject->m_closedBoundingRectLeft;
    m_rClosedBounding.top = pDoorObject->m_closedBoundingRectTop;
    m_rClosedBounding.right = pDoorObject->m_closedBoundingRectRight;
    m_rClosedBounding.bottom = pDoorObject->m_closedBoundingRectBottom;

    m_cursorType = pDoorObject->m_cursorType;
    m_dwFlags = pDoorObject->m_dwFlags;
    m_nOpenPolygon = pDoorObject->m_openSelectionPointCount;
    m_nClosedPolygon = pDoorObject->m_closedSelectionPointCount;

    m_ptDest1.x = pDoorObject->m_posXWalkTo1;
    m_ptDest1.y = pDoorObject->m_posYWalkTo1;

    m_ptDest2.x = pDoorObject->m_posXWalkTo2;
    m_ptDest2.y = pDoorObject->m_posYWalkTo2;

    m_strNotPickable = pDoorObject->m_strNotPickable;

    POSITION pos = pArea->m_lTiledObjects.AddTail(&m_tiledObject);
    WORD wInitialState = (m_dwFlags & 0x1) != 0
        ? CTiledObject::STATE_PRIMARY_TILE
        : CTiledObject::STATE_SECONDARY_TILE;
    m_tiledObject.Initialize(pArea->m_pResWED,
        m_resID,
        pos,
        wInitialState);

    memcpy(m_scriptRes, pDoorObject->m_script, RESREF_SIZE);

    strncpy(m_scriptName, pDoorObject->m_scriptName, SCRIPTNAME_SIZE);

    CAIScript* pScript = new CAIScript(CResRef(pDoorObject->m_script));
    SetScript(0, pScript);

    m_hitPoints = pDoorObject->m_hitPoints != 0 ? pDoorObject->m_hitPoints : 20;
    m_armourClass = pDoorObject->m_armourClass;
    m_openSound = pDoorObject->m_openSound;
    m_closeSound = pDoorObject->m_closeSound;
    m_trapDetectionDifficulty = pDoorObject->m_trapDetectionDifficulty;
    m_trapDisarmingDifficulty = pDoorObject->m_trapDisarmingDifficulty;
    m_trapActivated = pDoorObject->m_trapActivated;
    m_trapDetected = pDoorObject->m_trapDetected;
    m_posXTrapOrigin = pDoorObject->m_posXTrapOrigin;
    m_posYTrapOrigin = pDoorObject->m_posYTrapOrigin;
    m_keyType = pDoorObject->m_keyType;
    m_detectionDifficulty = pDoorObject->m_detectionDifficulty;
    m_lockDifficulty = pDoorObject->m_lockDifficulty;
    m_drawPoly = 0;

    if (m_nOpenPolygon != 0) {
        m_pOpenPolygon = new CPoint[m_nOpenPolygon];

        if (m_pOpenPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 170
            UTIL_ASSERT(FALSE);
        }

        WORD adjust = 0;
        for (WORD cnt = 0; cnt < m_nOpenPolygon; cnt++) {
            m_pOpenPolygon[cnt - adjust].x = pPoints[cnt + pDoorObject->m_openSelectionPointStart].m_xPos;
            m_pOpenPolygon[cnt - adjust].y = pPoints[cnt + pDoorObject->m_openSelectionPointStart].m_yPos;
            if (cnt >= 2) {
                int x2 = m_pOpenPolygon[cnt - adjust - 2].x;
                int y2 = m_pOpenPolygon[cnt - adjust - 2].y;

                int x1 = m_pOpenPolygon[cnt - adjust - 1].x;
                int y1 = m_pOpenPolygon[cnt - adjust - 1].y;

                int x0 = m_pOpenPolygon[cnt - adjust].x;
                int y0 = m_pOpenPolygon[cnt - adjust].y;

                if ((x2 == x1 && x1 == x0)
                    || (y2 == y1 && y1 == y0)
                    || (x2 != x1
                        && x1 != x0
                        && 1000 * (y2 - y1) / (x2 - x1) == 1000 * (y1 - y0) / (x1 - x0))) {
                    m_pOpenPolygon[cnt - adjust - 1] = m_pOpenPolygon[cnt - adjust];
                    adjust++;
                }
            }
        }

        m_nOpenPolygon -= adjust;
    } else {
        m_pOpenPolygon = NULL;
    }

    if (m_nClosedPolygon != 0) {
        m_pClosedPolygon = new CPoint[m_nClosedPolygon];

        if (m_pClosedPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 206
            UTIL_ASSERT(FALSE);
        }

        WORD adjust = 0;
        for (WORD cnt = 0; cnt < m_nClosedPolygon; cnt++) {
            m_pClosedPolygon[cnt - adjust].x = pPoints[cnt + pDoorObject->m_closedSelectionPointStart].m_xPos;
            m_pClosedPolygon[cnt - adjust].y = pPoints[cnt + pDoorObject->m_closedSelectionPointStart].m_yPos;
            if (cnt >= 2) {
                int x2 = m_pClosedPolygon[cnt - adjust - 2].x;
                int y2 = m_pClosedPolygon[cnt - adjust - 2].y;

                int x1 = m_pClosedPolygon[cnt - adjust - 1].x;
                int y1 = m_pClosedPolygon[cnt - adjust - 1].y;

                int x0 = m_pClosedPolygon[cnt - adjust].x;
                int y0 = m_pClosedPolygon[cnt - adjust].y;

                if ((x2 == x1 && x1 == x0)
                    || (y2 == y1 && y1 == y0)
                    || (x2 != x1
                        && x1 != x0
                        && 1000 * (y2 - y1) / (x2 - x1) == 1000 * (y1 - y0) / (x1 - x0))) {
                    m_pClosedPolygon[cnt - adjust - 1] = m_pClosedPolygon[cnt - adjust];
                    adjust++;
                }
            }
        }

        m_nClosedPolygon -= adjust;
    } else {
        m_pClosedPolygon = NULL;
    }

    m_nOpenSearch = pDoorObject->m_openSearchSquaresCount;
    if (m_nOpenSearch != 0) {
        m_pOpenSearch = new CPoint[m_nOpenSearch];

        if (m_pOpenSearch == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 243
            UTIL_ASSERT(FALSE);
        }

        for (WORD cnt = 0; cnt < m_nOpenSearch; cnt++) {
            m_pOpenSearch[cnt].x = pPoints[pDoorObject->m_openSearchSquaresStart + cnt].m_xPos;
            m_pOpenSearch[cnt].y = pPoints[pDoorObject->m_openSearchSquaresStart + cnt].m_yPos;
        }

        if ((m_dwFlags & 0x1) != 0) {
            pArea->m_search.AddDoor(m_pOpenSearch,
                m_nOpenSearch,
                (m_dwFlags & 0x400) != 0);
        }
    } else {
        m_pOpenSearch = NULL;
    }

    m_ptOpenDest.x = (pDoorObject->m_openBoundingRectLeft + pDoorObject->m_openBoundingRectRight) / 2;
    m_ptOpenDest.y = pDoorObject->m_openBoundingRectBottom;

    m_nClosedSearch = pDoorObject->m_closedSearchSquaresCount;
    if (m_nClosedSearch != 0) {
        m_pClosedSearch = new CPoint[m_nClosedSearch];

        if (m_pClosedSearch == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 270
            UTIL_ASSERT(FALSE);
        }

        for (WORD cnt = 0; cnt < m_nClosedSearch; cnt++) {
            m_pClosedSearch[cnt].x = pPoints[pDoorObject->m_closedSearchSquaresStart + cnt].m_xPos;
            m_pClosedSearch[cnt].y = pPoints[pDoorObject->m_closedSearchSquaresStart + cnt].m_yPos;
        }

        if ((m_dwFlags & 0x1) == 0) {
            pArea->m_search.AddDoor(m_pClosedSearch,
                m_nClosedSearch,
                (m_dwFlags & 0x400) != 0);
        }
    } else {
        m_pClosedSearch = NULL;
    }

    m_ptClosedDest.x = (pDoorObject->m_closedBoundingRectLeft + pDoorObject->m_closedBoundingRectRight) / 2;
    m_ptClosedDest.y = pDoorObject->m_closedBoundingRectBottom;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,
        this,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        if ((m_dwFlags & 0x1) != 0) {
            AddToArea(pArea, m_ptOpenDest, 0, LIST_FRONT);
        } else {
            AddToArea(pArea, m_ptClosedDest, 0, LIST_FRONT);
        }

        m_typeAI.SetName(CString(m_scriptName));

        CVariable name;
        name.SetName(CString(m_scriptName));
        name.m_intValue = m_id;
        pArea->GetNamedCreatures()->AddKey(name);

        SplitRectIntoGrid(&m_rClosedBounding, m_closedBoundingGrid);
        SplitRectIntoGrid(&m_rOpenBounding, m_openBoundingGrid);

        m_pClosedPolygonPoints = NULL;
        m_pOpenPolygonPoints = NULL;

        if (m_nClosedPolygon != 0) {
            m_pClosedPolygonPoints = new CAreaPoint[m_nClosedPolygon];

            for (WORD cnt = 0; cnt < m_nClosedPolygon; cnt++) {
                m_pClosedPolygonPoints[cnt].m_xPos = static_cast<WORD>(m_pClosedPolygon[cnt].x);
                m_pClosedPolygonPoints[cnt].m_yPos = static_cast<WORD>(m_pClosedPolygon[cnt].y);
            }
        } else {
            m_pClosedPolygonPoints = NULL;
        }

        if (m_nOpenPolygon != 0) {
            m_pOpenPolygonPoints = new CAreaPoint[m_nOpenPolygon];

            for (WORD cnt = 0; cnt < m_nOpenPolygon; cnt++) {
                m_pOpenPolygonPoints[cnt].m_xPos = static_cast<WORD>(m_pOpenPolygon[cnt].x);
                m_pOpenPolygonPoints[cnt].m_yPos = static_cast<WORD>(m_pOpenPolygon[cnt].y);
            }
        } else {
            m_pOpenPolygonPoints = NULL;
        }

        m_nAICounter = 0;
    } else {
        delete this;
    }
}

// 0x4866E0
CGameDoor::~CGameDoor()
{
    if (m_pOpenPolygon != NULL) {
        delete m_pOpenPolygon;
    }

    if (m_pClosedPolygon != NULL) {
        delete m_pClosedPolygon;
    }

    if (m_pOpenSearch != NULL) {
        delete m_pOpenSearch;
    }

    if (m_pClosedSearch != NULL) {
        delete m_pClosedSearch;
    }

    if (m_pClosedPolygonPoints != NULL) {
        delete m_pClosedPolygonPoints;
        m_pClosedPolygonPoints = NULL;
    }

    // When there is only one element its an unowned pointer to
    // `m_rClosedBounding`.
    if (m_closedBoundingGrid.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < m_closedBoundingGrid.GetCount(); nIndex++) {
            delete m_closedBoundingGrid[nIndex];
        }
    }
    m_closedBoundingGrid.SetSize(0);

    if (m_pOpenPolygonPoints != NULL) {
        delete m_pOpenPolygonPoints;
        m_pOpenPolygonPoints = NULL;
    }

    // When there is only one element its an unowned pointer to
    // `m_rOpenBounding`.
    if (m_openBoundingGrid.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < m_openBoundingGrid.GetCount(); nIndex++) {
            delete m_openBoundingGrid[nIndex];
        }
    }
    m_openBoundingGrid.SetSize(0);
}

// 0x487460
void CGameDoor::AIUpdate()
{
    if (m_drawPoly > 0) {
        m_drawPoly--;
    }

    ProcessAI();

    if (g_pBaldurChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (m_nAICounter++ == 225) {
            m_nAICounter = 0;

            CMessageDoorStatus* pMessage = new CMessageDoorStatus(this, m_id, m_id);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
}

// 0x47C830
BOOLEAN CGameDoor::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x4876E0
void CGameDoor::DebugDump(const CString& message, BOOLEAN bEchoToScreen)
{
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    CString sTemp;

    if (bEchoToScreen) {
        pWorld->DisplayText(CString(""),
            CString("DEBUG DUMP: CGameDoor"),
            -1,
            FALSE);

        pWorld->DisplayText(CString(""),
            message,
            -1,
            FALSE);

        sTemp.Format("Current Area: %.*s", RESREF_SIZE, m_pArea->m_resRef.GetResRef());
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Current Position: x=%d y=%d", m_pos.x, m_pos.y);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Locked: %s\n", (m_dwFlags & 0x2) != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Lock Difficulty: %d%%\n", m_lockDifficulty);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Key Type: %.*s", RESREF_SIZE, m_keyType.GetResRef());
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trapped: %s", m_trapActivated != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Detected: %s", m_trapDetected != 0 ? "TRUE" : "FALSE");
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Detection Difficulty: %d%%", m_trapDetectionDifficulty);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Removal Difficulty: %d%%", m_trapDisarmingDifficulty);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Script: %.*s", RESREF_SIZE, m_scriptRes);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);

        sTemp.Format("Trap Launching Point: x=%d y=%d", m_posXTrapOrigin, m_posYTrapOrigin);
        pWorld->DisplayText(CString(""),
            sTemp,
            -1,
            FALSE);
    }
}

// 0x487C10
BOOL CGameDoor::IsOver(const CPoint& pt)
{
    if ((m_dwFlags & 0x2000) != 0) {
        return FALSE;
    }

    if ((m_dwFlags & 0x1) != 0) {
        if (!m_rOpenBounding.PtInRect(pt)) {
            return FALSE;
        }

        if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
            return FALSE;
        }

        if (m_pOpenPolygon != NULL) {
            return CVidPoly::IsPtInPoly(m_pOpenPolygon, m_nOpenPolygon, pt);
        }
    } else {
        if (!m_rClosedBounding.PtInRect(pt)) {
            return FALSE;
        }

        if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
            return FALSE;
        }

        if (m_pClosedPolygon != NULL) {
            return CVidPoly::IsPtInPoly(m_pClosedPolygon, m_nClosedPolygon, pt);
        }
    }

    return TRUE;
}

// 0x4892B0
void CGameDoor::RemoveFromArea()
{
    m_pArea->m_lTiledObjects.RemoveAt(m_tiledObject.m_posAreaList);

    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1269
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x489330
void CGameDoor::SetCursor(LONG nToolTip)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if ((m_dwFlags & 0x80) == 0 || (m_dwFlags & 0x100) != 0) {
        switch (pGame->GetState()) {
        case 0:
            if (pGame->GetGroup()->GetCount() != 0) {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(m_cursorType, FALSE);
            } else {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);
            }
            break;
        case 1:
        case 3:
            CGameObject::SetCursor(nToolTip);
            break;
        case 2:
            switch (pGame->GetIconIndex()) {
            case 12:
                if ((m_dwFlags & 0x1) == 0 && (m_dwFlags & 0x2) != 0) {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(12, FALSE);
                } else {
                    CGameObject::SetCursor(nToolTip);
                }
                break;
            case 18:
            case 40:
            case 255:
                CGameObject::SetCursor(nToolTip);
                break;
            case 36:
                if (1) {
                    INT nNewCursor;
                    if (m_trapActivated != 0 && m_trapDetected != 0) {
                        nNewCursor = 38;
                    } else {
                        if ((m_dwFlags & 0x2) == 0) {
                            CGameObject::SetCursor(nToolTip);
                            break;
                        }

                        nNewCursor = 26;
                    }

                    SHORT nPortrait = g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter();

                    // NOTE: Uninline.
                    LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                    CGameSprite* pSprite;

                    BYTE rc;
                    do {
                        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        if (nNewCursor == 26) {
                            if (pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] == 0) {
                                g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                            }
                        } else {
                            if (pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 0) {
                                g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                            }
                        }

                        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
                // __LINE__: 1370
                UTIL_ASSERT(FALSE);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
            // __LINE__: 1380
            UTIL_ASSERT(FALSE);
        }
    } else {
        CGameObject::SetCursor(nToolTip);
    }
}

// 0x48AC80
void CGameDoor::OnDoorStatusUpdate(BOOLEAN bDoorOpened, DWORD dwFlags, WORD nTrapActivated, WORD nTrapDetected)
{
    if ((m_dwFlags & 0x1) == 0 || bDoorOpened) {
        if ((m_dwFlags & 0x1) == 0 && bDoorOpened == TRUE) {
            m_dwFlags |= 0x1;
            m_pos = m_ptClosedDest;
            m_tiledObject.m_wAIState = CTiledObject::STATE_PRIMARY_TILE;

            if (m_pClosedSearch != NULL) {
                m_pArea->m_search.RemoveDoor(m_pClosedSearch, m_nClosedSearch);
            }

            if (m_pOpenSearch != NULL) {
                m_pArea->m_search.AddDoor(m_pOpenSearch,
                    m_nOpenSearch,
                    (m_dwFlags & 0x400) != 0);
            }
        }
    } else {
        m_dwFlags &= ~0x1;
        m_pos = m_ptOpenDest;
        m_tiledObject.m_wAIState = CTiledObject::STATE_SECONDARY_TILE;

        if (m_pOpenSearch != NULL) {
            m_pArea->m_search.RemoveDoor(m_pOpenSearch, m_nOpenSearch);
        }

        if (m_pClosedSearch != NULL) {
            m_pArea->m_search.AddDoor(m_pClosedSearch,
                m_nClosedSearch,
                (m_dwFlags & 0x400) != 0);
        }
    }

    m_dwFlags = dwFlags;
    m_trapDetected = nTrapDetected;
    m_trapActivated = nTrapActivated;
}

// 0x48ADC0
void CGameDoor::Marshal(CAreaFileDoorObject** pDoorObject)
{
    CAreaPoint* pPoints;
    DWORD cnt;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 1904
    UTIL_ASSERT(pDoorObject != NULL);

    *pDoorObject = new CAreaFileDoorObject();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
    // __LINE__: 1910
    UTIL_ASSERT(*pDoorObject != NULL);

    // FIXME: Redundant, memset is a part of constructor.
    memset(*pDoorObject, 0, sizeof(CAreaFileDoorObject));

    if (m_nClosedSearch > 0) {
        pPoints = new CAreaPoint[m_nClosedSearch];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1915
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nClosedSearch; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pClosedSearch[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pClosedSearch[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_closedSearchSquaresStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_closedSearchSquaresCount = m_nClosedSearch;
    }

    if (m_nOpenSearch > 0) {
        pPoints = new CAreaPoint[m_nOpenSearch];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1927
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nOpenSearch; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pOpenSearch[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pOpenSearch[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_openSearchSquaresStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_openSearchSquaresCount = m_nOpenSearch;
    }

    if (m_nClosedPolygon > 0) {
        pPoints = new CAreaPoint[m_nClosedPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1939
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nClosedPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pClosedPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pClosedPolygon[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_closedSelectionPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_closedSelectionPointCount = m_nClosedPolygon;
    }

    if (m_nOpenPolygon > 0) {
        pPoints = new CAreaPoint[m_nOpenPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameDoor.cpp
        // __LINE__: 1951
        UTIL_ASSERT(pPoints != NULL);

        for (cnt = 0; cnt < m_nOpenPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pOpenPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pOpenPolygon[cnt].y);
        }

        // FIXME: Unsafe x64 conversion.
        (*pDoorObject)->m_openSelectionPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pDoorObject)->m_openSelectionPointCount = m_nOpenPolygon;
    }

    m_resID.GetResRef((*pDoorObject)->m_doorID);
    (*pDoorObject)->m_closedBoundingRectLeft = static_cast<WORD>(m_rClosedBounding.left);
    (*pDoorObject)->m_closedBoundingRectTop = static_cast<WORD>(m_rClosedBounding.top);
    (*pDoorObject)->m_closedBoundingRectRight = static_cast<WORD>(m_rClosedBounding.right) - 1;
    (*pDoorObject)->m_closedBoundingRectBottom = static_cast<WORD>(m_rClosedBounding.bottom) - 1;
    (*pDoorObject)->m_openBoundingRectLeft = static_cast<WORD>(m_rOpenBounding.left);
    (*pDoorObject)->m_openBoundingRectTop = static_cast<WORD>(m_rOpenBounding.top);
    (*pDoorObject)->m_openBoundingRectRight = static_cast<WORD>(m_rOpenBounding.right) - 1;
    (*pDoorObject)->m_openBoundingRectBottom = static_cast<WORD>(m_rOpenBounding.bottom) - 1;
    (*pDoorObject)->m_cursorType = m_cursorType;
    (*pDoorObject)->m_dwFlags = m_dwFlags;
    (*pDoorObject)->m_posXWalkTo1 = static_cast<WORD>(m_ptDest1.x);
    (*pDoorObject)->m_posYWalkTo1 = static_cast<WORD>(m_ptDest1.y);
    (*pDoorObject)->m_posXWalkTo2 = static_cast<WORD>(m_ptDest2.x);
    (*pDoorObject)->m_posYWalkTo2 = static_cast<WORD>(m_ptDest2.y);
    memcpy((*pDoorObject)->m_script, m_scriptRes, RESREF_SIZE);
    strncpy((*pDoorObject)->m_scriptName, m_scriptName, SCRIPTNAME_SIZE);
    (*pDoorObject)->m_hitPoints = m_hitPoints;
    (*pDoorObject)->m_armourClass = m_armourClass;
    m_openSound.GetResRef((*pDoorObject)->m_openSound);
    m_closeSound.GetResRef((*pDoorObject)->m_closeSound);
    (*pDoorObject)->m_trapDetectionDifficulty = m_trapDetectionDifficulty;
    (*pDoorObject)->m_trapDisarmingDifficulty = m_trapDisarmingDifficulty;
    (*pDoorObject)->m_trapActivated = m_trapActivated;
    (*pDoorObject)->m_trapDetected = m_trapDetected;
    (*pDoorObject)->m_posXTrapOrigin = m_posXTrapOrigin;
    (*pDoorObject)->m_posYTrapOrigin = m_posYTrapOrigin;
    m_keyType.GetResRef((*pDoorObject)->m_keyType);
    (*pDoorObject)->m_detectionDifficulty = m_detectionDifficulty;
    (*pDoorObject)->m_lockDifficulty = m_lockDifficulty;
    (*pDoorObject)->m_strNotPickable = m_strNotPickable;
}

// 0x487530
BOOLEAN CGameDoor::CompressTime(DWORD deltaTime)
{
    if (static_cast<DWORD>(m_drawPoly) < deltaTime) {
        m_drawPoly = 0;
    } else {
        m_drawPoly = static_cast<SHORT>(m_drawPoly - deltaTime);
    }
    if (deltaTime > static_cast<DWORD>(CTimerWorld::TIMESCALE_MSEC_PER_SEC
            * CTimerWorld::TIMESCALE_SEC_PER_MIN * CTimerWorld::TIMESCALE_MIN_PER_HOUR)) {
        if ((m_dwFlags & 1) != 0 && (m_dwFlags & 0x20) == 0
            && m_pArea->m_search.CanToggleDoor(m_pClosedSearch, m_nClosedSearch)
            && rand() % 5 != 0) {
            m_dwFlags &= ~1;
            m_pos = m_ptOpenDest;
            m_tiledObject.m_wAIState = CTiledObject::STATE_SECONDARY_TILE;
            if (m_pOpenSearch != NULL) {
                m_pArea->m_search.RemoveDoor(m_pOpenSearch, m_nOpenSearch);
            }
            if (m_pClosedSearch != NULL) {
                m_pArea->m_search.AddDoor(m_pClosedSearch, m_nClosedSearch, (m_dwFlags & 0x400) != 0);
            }
            if (g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE) {
                g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessageDoorStatus(this, m_id, m_id), FALSE);
            }
        }
        m_dwFlags &= ~0x1000;
    }
    return TRUE;
}

// 0x487D10
void CGameDoor::OnActionButton(const CPoint& pt)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CAIGroup* pGroup = pGame->GetGroup();
    if ((m_dwFlags & 0x80) != 0 && (m_dwFlags & 0x100) == 0) {
        pGame->SetLastClick(CPoint(-1, -1));
        CGameObject::OnActionButton(pt);
        return;
    }

    switch (pGame->m_nState) {
    case 0: {
        pGame->SetLastClick(CPoint(-1, -1));
        if (static_cast<BYTE>(pGroup->GetCount()) == 0) {
            return;
        }
        CAIObjectType target(0, 0, 0, 0, 0, 0, 0, 0, m_id);
        CAIAction action(CAIAction::USEDOOR, target, 0, 0, 0);
        pGroup->GroupAction(action, TRUE, NULL);

        LONG* pMembers = pGroup->GetGroupList();
        UTIL_ASSERT(pMembers != NULL);
        if (pMembers == NULL) {
            return;
        }
        LONG range = RANGE_DOOR;
        CPoint dest;
        SHORT member = 0;
        for (; member < static_cast<BYTE>(pGroup->GetCount()); ++member) {
            CGameObject* pMember;
            BYTE rc = pGame->GetObjectArray()->GetShare(pMembers[member],
                CGameObjectArray::THREAD_ASYNCH, &pMember, INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                UTIL_ASSERT(FALSE);
                continue;
            }
            CPoint pos = pMember->GetPos();
            dest = GetMoveDest(pos);
            const char* areaName = reinterpret_cast<const char*>(m_pArea->m_header.m_areaName);
            if (_strnicmp(areaName, "ar5004", 7) == 0) {
                range = RANGE_DOOR / 10;
            } else if (_strnicmp(areaName, "ar5202", 7) == 0) {
                range = RANGE_DOOR / 5;
            }
            LONG dx = pos.x - dest.x;
            LONG dy = pos.y - dest.y;
            BOOL close = dx * dx + dy * dy * 16 / 9 <= range;
            pGame->GetObjectArray()->ReleaseShare(pMembers[member],
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (close) {
                break;
            }
        }
        if (member == static_cast<BYTE>(pGroup->GetCount())) {
            CAIAction move(CAIAction::MOVETOPOINT, dest, 0, -1);
            move.m_specificID3 = 0;
            pGroup->GroupAction(move, FALSE, NULL);
        }
        delete[] pMembers;
        return;
    }
    case 2:
        pGame->SetLastClick(CPoint(-1, -1));
        switch (static_cast<BYTE>(pGame->m_iconIndex)) {
        case 12: {
            if ((m_dwFlags & 1) != 0 || (m_dwFlags & 2) == 0) {
                CGameObject::OnActionButton(pt);
                return;
            }
            CAIAction action(CAIAction::BASHDOOR, m_typeAI, 0, 0, 0);
            pGroup->GroupAction(CAIAction::NULL_ACTION, TRUE, &action);
            break;
        }
        case 18:
        case 40:
        case 255:
            CGameObject::OnActionButton(pt);
            return;
        case 20:
            pGame->UseMagicOnObject(m_id);
            break;
        case 36: {
            BOOL disarm = m_trapActivated != 0 && m_trapDetected != 0;
            if (!disarm && (m_dwFlags & 2) == 0) {
                CGameObject::OnActionButton(pt);
                return;
            }
            LONG id = pGame->GetCharacterId(g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter());
            CGameObject* pObject;
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS) {
                BYTE skill = static_cast<CGameSprite*>(pObject)->m_baseStats.m_skills[disarm ? 5 : 10];
                pGame->GetObjectArray()->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                if (skill == 0) {
                    return;
                }
            }
            CAIAction action(disarm ? CAIAction::REMOVETRAPS : CAIAction::PICKLOCK, m_typeAI, 0, 0, 0);
            pGroup->GroupAction(action, TRUE, NULL);
            break;
        }
        default:
            UTIL_ASSERT(FALSE);
            return;
        }
        pGame->m_nState = 0;
        pGame->GetButtonArray()->m_nSelectedButton = 100;
        pGame->GetButtonArray()->UpdateState();
        return;
    case 3:
        pGame->SetLastClick(CPoint(-1, -1));
        CGameObject::OnActionButton(pt);
        return;
    default:
        UTIL_ASSERT(FALSE);
    }
}

// 0x488B30
void CGameDoor::Render(CGameArea* pArea, CVidMode* pVidMode, int nSurface)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    SHORT state = pGame->m_nState;
    UTIL_ASSERT(pVidMode != NULL);
    if ((m_dwFlags & 0x2000) != 0) {
        return;
    }
    BOOL drewFill = FALSE;
    auto draw = [&](COLORREF color, BOOL fill) {
        if (fill && CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.Is3dAccelerated()) {
            COLORREF surfaceColor = g_pChitin->GetCurrentVideoMode()->GetColor(color);
            if ((m_dwFlags & 1) != 0) {
                RenderOpenPoly(pArea, pVidMode, nSurface, surfaceColor);
            } else {
                RenderClosedPoly(pArea, pVidMode, nSurface, surfaceColor);
            }
            drewFill = TRUE;
        }
        if ((m_dwFlags & 1) != 0) {
            m_pArea->GetInfinity()->OutlinePoly(m_pOpenPolygon, m_nOpenPolygon, m_rOpenBounding, color);
        } else {
            m_pArea->GetInfinity()->OutlinePoly(m_pClosedPolygon, m_nClosedPolygon, m_rClosedBounding, color);
        }
    };
    if (m_id == m_pArea->m_iPicked
        && m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))
        && (m_dwFlags & 0x80) == 0) {
        if (static_cast<BYTE>(pGame->GetGroup()->GetCount()) != 0 && state == 0) {
            draw(RGB(0x20, 0x40, 0xA0), TRUE);
        }
        if (pGame->m_nState == 2) {
            if (pGame->m_iconIndex == 36
                && ((m_trapActivated != 0 && m_trapDetected != 0) || (m_dwFlags & 2) != 0)) {
                draw(RGB(0, 0xFA, 0), TRUE);
            }
            if (pGame->m_iconIndex == 12 && (m_dwFlags & 1) == 0 && (m_dwFlags & 2) != 0) {
                draw(RGB(0, 0xFA, 0), TRUE);
            }
        }
    }
    COLORREF color;
    if (m_drawPoly > 0) {
        color = (m_dwFlags & 0x180) == 0x180 ? RGB(0xFF, 0, 0xFF) : RGB(0xFF, 0, 0);
    } else {
        if ((m_dwFlags & 0x80) != 0
            || !m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(m_pos))
            || g_pBaldurChitin->pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || g_pBaldurChitin->m_pEngineWorld->GetMenuKey() != TRUE) {
            return;
        }
        color = RGB(0xFF, 0, 0);
    }
    if (((m_dwFlags & 1) != 0 ? m_nOpenPolygon : m_nClosedPolygon) != 0) {
        draw(color, !drewFill);
    }
}

// 0x48B3C0
void CGameDoor::RenderClosedPoly(CGameArea* pArea, CVidMode* pVidMode, int nSurface, COLORREF color)
{
    CInfinity* pInfinity = pArea->GetInfinity();
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);
    CInfinity* pOwnerInfinity = m_pArea->GetInfinity();
    CRect rViewport(pOwnerInfinity->nCurrentX, pOwnerInfinity->nCurrentY,
        pOwnerInfinity->nCurrentX + pOwnerInfinity->rViewPort.Width(),
        pOwnerInfinity->nCurrentY + pOwnerInfinity->rViewPort.Height());
    const INT count = m_closedBoundingGrid.GetSize();
    CPoint ptReference(0, 0);
    CVidPoly poly;
    poly.SetPoly(reinterpret_cast<WORD*>(m_pClosedPolygonPoints), m_nClosedPolygon);
    for (INT i = 0; i < count; ++i) {
        CRect* pGridRect = m_closedBoundingGrid[i];
        if (pGridRect == NULL) {
            continue;
        }
        CRect rClip;
        rClip.IntersectRect(&rViewport, pGridRect);
        if (rClip.left == 0 && rClip.right == 0 && rClip.top == 0 && rClip.bottom == 0) {
            continue;
        }
        CPoint ptPos(rClip.left, rClip.top);
        CRect rFXRect(rClip);
        rFXRect.OffsetRect(-rClip.left, -rClip.top);
        if (pInfinity->FXPrep(rFXRect, CInfinity::FXPREP_COPYFROMBACK, nSurface, ptPos, ptReference)
            && pInfinity->FXLock(rFXRect, 0)) {
            static_cast<CVidInf*>(pVidMode)->RenderConvexPoly(&poly, rClip, color, CInfinity::MIRROR_FX, ptPos, FALSE);
            if (pInfinity->FXUnlock(0, NULL, CPoint(0, 0))) {
                pInfinity->FXBltFrom(nSurface, rFXRect, ptPos.x, ptPos.y, ptReference.x, ptReference.y, 0);
            }
        }
    }
}

// 0x48B600
void CGameDoor::RenderOpenPoly(CGameArea* pArea, CVidMode* pVidMode, int nSurface, COLORREF color)
{
    CInfinity* pInfinity = pArea->GetInfinity();
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);
    CInfinity* pOwnerInfinity = m_pArea->GetInfinity();
    CRect rViewport(pOwnerInfinity->nCurrentX, pOwnerInfinity->nCurrentY,
        pOwnerInfinity->nCurrentX + pOwnerInfinity->rViewPort.Width(),
        pOwnerInfinity->nCurrentY + pOwnerInfinity->rViewPort.Height());
    const INT count = m_openBoundingGrid.GetSize();
    CPoint ptReference(0, 0);
    CVidPoly poly;
    poly.SetPoly(reinterpret_cast<WORD*>(m_pOpenPolygonPoints), m_nOpenPolygon);
    for (INT i = 0; i < count; ++i) {
        CRect* pGridRect = m_openBoundingGrid[i];
        if (pGridRect == NULL) {
            continue;
        }
        CRect rClip;
        rClip.IntersectRect(&rViewport, pGridRect);
        if (rClip.left == 0 && rClip.right == 0 && rClip.top == 0 && rClip.bottom == 0) {
            continue;
        }
        CPoint ptPos(rClip.left, rClip.top);
        CRect rFXRect(rClip);
        rFXRect.OffsetRect(-rClip.left, -rClip.top);
        if (pInfinity->FXPrep(rFXRect, CInfinity::FXPREP_COPYFROMBACK, nSurface, ptPos, ptReference)
            && pInfinity->FXLock(rFXRect, 0)) {
            static_cast<CVidInf*>(pVidMode)->RenderConvexPoly(&poly, rClip, color, CInfinity::MIRROR_FX, ptPos, FALSE);
            if (pInfinity->FXUnlock(0, NULL, CPoint(0, 0))) {
                pInfinity->FXBltFrom(nSurface, rFXRect, ptPos.x, ptPos.y, ptReference.x, ptReference.y, 0);
            }
        }
    }
}

// 0x48B2C0
const CPoint& CGameDoor::GetMoveDest(const CPoint& pt)
{
    LONG dx1 = pt.x - m_ptDest1.x;
    LONG dy1 = pt.y - m_ptDest1.y;
    LONG dx2 = pt.x - m_ptDest2.x;
    LONG dy2 = pt.y - m_ptDest2.y;
    LONG distance1 = dx1 * dx1 + dy1 * dy1 * 16 / 9;
    LONG distance2 = dx2 * dx2 + dy2 * dy2 * 16 / 9;
    return distance1 < distance2 ? m_ptDest1 : m_ptDest2;
}

// 0x48B350
void CGameDoor::SetDrawPoly(SHORT time)
{
    if (m_drawPoly != time) {
        if (!g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly) {
            CMessageSetDrawPoly* pMessage = new CMessageSetDrawPoly(time, GetId(), GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
    m_drawPoly = time;
}

// 0x453140
DWORD CGameDoor::GetFlags()
{
    return m_dwFlags;
}

// 0x453150
void CGameDoor::SetFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

// 0x45B700
BOOL CGameDoor::IsOpen()
{
    return m_dwFlags & 0x1;
}

// 0x47D7F0
BOOLEAN CGameDoor::DoAIUpdate(BOOLEAN active, LONG counter)
{
    // Folded with CGameContainer::DoAIUpdate at 0x47D7F0.
    return active && (counter & m_AISpeed) == (m_AISpeed & m_id);
}
