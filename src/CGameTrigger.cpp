#include "CGameTrigger.h"

#include "CAIAction.h"
#include "CAIObjectType.h"
#include "CAIScript.h"
#include "CAITrigger.h"
#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameEffect.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CMessage.h"
#include "CInfButtonArray.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CScreenWorld.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVidMode.h"
#include "CVidPoly.h"

// 0x8D71DC
const LONG CGameTrigger::RANGE_TRIGGER = 16
    * CPathSearch::GRID_SQUARE_SIZEX
    * CPathSearch::GRID_SQUARE_SIZEX;

// 0x8D71C4
const LONG CGameTrigger::RANGE_EDGE = (CSearchBitmap::TRAVEL_WIDTH + 10)
    * (CSearchBitmap::TRAVEL_WIDTH + 10)
    * CPathSearch::GRID_SQUARE_SIZEX
    * CPathSearch::GRID_SQUARE_SIZEX;

// 0x4CCAB0
CGameTrigger::CGameTrigger(CGameArea* pArea, CAreaFileTriggerObject* pTriggerObject, CAreaPoint* pPoints, WORD maxPts)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 110
    UTIL_ASSERT(pArea != NULL && pTriggerObject != NULL && pPoints != NULL && pTriggerObject->m_pickPointStart + pTriggerObject->m_pickPointCount <= maxPts);

    m_objectType = TYPE_TRIGGER;
    m_triggerType = pTriggerObject->m_triggerType;
    m_rBounding.left = pTriggerObject->m_boundingRectLeft;
    m_rBounding.top = pTriggerObject->m_boundingRectTop;
    m_rBounding.right = pTriggerObject->m_boundingRectRight + 1;
    m_rBounding.bottom = pTriggerObject->m_boundingRectBottom + 1;
    m_cursorType = pTriggerObject->m_cursorType != 30 ? pTriggerObject->m_cursorType : 42;
    memcpy(m_newArea, pTriggerObject->m_newArea, RESREF_SIZE);
    memcpy(m_newEntryPoint, pTriggerObject->m_newEntryPoint, SCRIPTNAME_SIZE);
    memcpy(m_scriptRes, pTriggerObject->m_script, RESREF_SIZE);
    m_dwFlags = pTriggerObject->m_dwFlags;
    m_description = pTriggerObject->m_description;
    m_nPolygon = pTriggerObject->m_pickPointCount;
    strncpy(m_scriptName, pTriggerObject->m_scriptName, SCRIPTNAME_SIZE);

    CAIScript* pScript = new CAIScript(CResRef(pTriggerObject->m_script));
    SetScript(0, pScript);

    m_trapDetectionDifficulty = pTriggerObject->m_trapDetectionDifficulty;
    m_trapDisarmingDifficulty = pTriggerObject->m_trapDisarmingDifficulty;
    m_trapActivated = pTriggerObject->m_trapActivated;
    m_trapDetected = pTriggerObject->m_trapDetected;
    m_posTrapOrigin.x = pTriggerObject->m_posXTrapOrigin;
    m_posTrapOrigin.y = pTriggerObject->m_posYTrapOrigin;
    m_keyType = pTriggerObject->m_keyType;
    m_drawPoly = 0;

    if (m_nPolygon != 0) {
        m_pPolygon = new CPoint[m_nPolygon];

        if (m_pPolygon == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
            // __LINE__: 149
            UTIL_ASSERT(FALSE);
        }

        WORD adjust = 0;
        for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
            m_pPolygon[cnt - adjust].x = pPoints[cnt + pTriggerObject->m_pickPointStart].m_xPos;
            m_pPolygon[cnt - adjust].y = pPoints[cnt + pTriggerObject->m_pickPointStart].m_yPos;
            if (cnt >= 2) {
                int x2 = m_pPolygon[cnt - adjust - 2].x;
                int y2 = m_pPolygon[cnt - adjust - 2].y;

                int x1 = m_pPolygon[cnt - adjust - 1].x;
                int y1 = m_pPolygon[cnt - adjust - 1].y;

                int x0 = m_pPolygon[cnt - adjust].x;
                int y0 = m_pPolygon[cnt - adjust].y;

                if ((x2 == x1 && x1 == x0)
                    || (y2 == y1 && y1 == y0)
                    || (x2 != x1
                        && x1 != x0
                        && 1000 * (y2 - y1) / (x2 - x1) == 1000 * (y1 - y0) / (x1 - x0))) {
                    m_pPolygon[cnt - adjust - 1] = m_pPolygon[cnt - adjust];
                    adjust++;
                }
            }
        }

        m_nPolygon -= adjust;
    } else {
        m_pPolygon = NULL;
    }

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,
        this,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        CPoint pos;
        pos.x = (pTriggerObject->m_boundingRectLeft + pTriggerObject->m_boundingRectRight) / 2;
        pos.y = (pTriggerObject->m_boundingRectTop + pTriggerObject->m_boundingRectBottom) / 2;

        AddToArea(pArea,
            pos,
            0,
            m_triggerType == 0 ? LIST_FRONT : LIST_BACK);

        m_boundingRange = static_cast<WORD>(max((pTriggerObject->m_boundingRectRight - pTriggerObject->m_boundingRectLeft) / 2,
            2 * (pTriggerObject->m_boundingRectBottom - pTriggerObject->m_boundingRectTop) / 3));

        m_typeAI.SetName(CString(m_scriptName));

        CVariable name;
        name.SetName(CString(m_scriptName));
        name.m_intValue = m_id;
        pArea->GetNamedCreatures()->AddKey(name);

        if ((m_dwFlags & 0x200) != 0) {
            m_ptUsePoint.x = pTriggerObject->field_88;
            m_ptUsePoint.y = pTriggerObject->field_8C;
        }

        SplitRectIntoGrid(&m_rBounding, m_boundingGrid);

        // FIXME: One assignment is usually enough.
        m_pPolygonPoints = NULL;
        m_pPolygonPoints = NULL;

        if (m_nPolygon != 0) {
            m_pPolygonPoints = new CAreaPoint[m_nPolygon];

            for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
                m_pPolygonPoints[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
                m_pPolygonPoints[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
            }
        }
    } else {
        delete this;
    }
}

// 0x4CD150
CGameTrigger::~CGameTrigger()
{
    if (m_pPolygon != NULL) {
        delete m_pPolygon;
    }

    if (m_pPolygonPoints != NULL) {
        delete m_pPolygonPoints;
        m_pPolygonPoints = NULL;
    }

    // When there is only one element its an unowned pointer to `m_rBounding`.
    if (m_boundingGrid.GetCount() > 1) {
        for (INT nIndex = 0; nIndex < m_boundingGrid.GetCount(); nIndex++) {
            delete m_boundingGrid[nIndex];
        }
    }

    m_boundingGrid.SetSize(0);
}

// 0x4CD630
void CGameTrigger::AIUpdate()
{
    if (m_drawPoly > 0) {
        m_drawPoly--;
    }

    ProcessAI();
}

// 0x4CD650
void CGameTrigger::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    switch (pEffect->m_effectID) {
    case CGAMEEFFECT_DETECTTRAPS:
        if (m_trapActivated != 0 && (m_dwFlags & 0x100) == 0) {
            if (m_trapDetected == 0
                    && m_trapDetectionDifficulty < 100
                    && (m_dwFlags & 0x8) != 0) {
                CAITrigger trigger(CAITRIGGER_DETECTED, m_typeAI, 0);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(
                    new CMessageSetTrigger(trigger, m_id, m_id), FALSE);

                m_trapDetected = 1;
                g_pBaldurChitin->GetMessageHandler()->AddMessage(
                    new CMessageTriggerStatus(m_dwFlags, m_trapActivated, m_trapDetected, m_id, m_id), FALSE);

                // If the object bound to this trap (m_typeAI) resolves to a bare
                // sprite-type AI marker, fire its trap-detected auto-pause. The id
                // passed as caller is the trigger's own; it is only used when the
                // bound type has no fixed instance.
                CGameObject* pObject = m_typeAI.GetObject(reinterpret_cast<CGameAIBase*>(m_id), FALSE);
                if (pObject != NULL) {
                    CAIObjectType spriteType(CGameObject::TYPE_SPRITE, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0);
                    if (pObject->GetAIType().Equal(spriteType)) {
                        static_cast<CGameAIBase*>(pObject)->AutoPause(0x80);
                    }
                }
            }

            if (m_trapDetected != 0) {
                if (m_drawPoly != 400
                        && *(reinterpret_cast<char*>(g_pBaldurChitin) + 0x497E) == 0) {
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(
                        new CMessageSetDrawPoly(400, m_id, m_id), FALSE);
                }
                m_drawPoly = 400;
            }
        }
        break;

    case CGAMEEFFECT_CASTSPELL: {
        CAIObjectType acteeType;
        acteeType.Set(GetAIType());

        CAIAction forceSpell;
        forceSpell.m_actionID = CAIAction::FORCESPELL;
        forceSpell.m_acteeID.Set(acteeType);

        CString spellRes;
        pEffect->m_res.CopyToString(spellRes);
        forceSpell.SetString1(spellRes);
        forceSpell.m_specificID = pEffect->m_effectAmount;

        g_pBaldurChitin->GetMessageHandler()->AddMessage(
            new CMessageAddAction(forceSpell, m_id, pEffect->m_sourceID), FALSE);
        break;
    }

    case CGAMEEFFECT_SUMMON:
        // HACK: CCreatureFile/CGameSprite spawn unrecovered (FindNearbyPassablePoint sig, CGameSprite
        // ctor args, CProjectile dispatch unknown) — replaces 0x4CDBDA
        break;

    default:
        break;
    }

    delete pEffect;
}

// 0x4CFA60
void CGameTrigger::Render(CGameArea* pArea, CVidMode* pVidMode, INT nSurface)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1020
    UTIL_ASSERT(pVidMode != NULL);

    COLORREF color;
    if (m_triggerType == 0
        && m_trapActivated != 0
        && m_trapDetected != 0
        && m_id == m_pArea->m_iPicked
        && pGame->m_nState == 2
        && pGame->m_iconIndex == '$') {
        color = RGB(0x00, 0xFA, 0x00);
    } else if (m_triggerType == 0 && m_drawPoly > 0) {
        color = RGB(0xFF, 0x00, 0x00);
    } else {
        if (!pGame->m_bTriggerOutline) {
            return;
        }
        color = RGB(0x00, 0x00, 0xFF);
    }

    if (CInfinity::TRANSLUCENT_BLTS_ON && !g_pChitin->cVideo.Is3dAccelerated()) {
        RenderClippedPoly(pArea, pVidMode, nSurface,
            g_pChitin->GetCurrentVideoMode()->GetColor(color));
    }

    m_pArea->GetInfinity()->OutlinePoly(m_pPolygon, m_nPolygon, m_rBounding, color);
}

// 0x4D02C0
void CGameTrigger::RenderClippedPoly(CGameArea* pArea, CVidMode* pVidMode, INT nSurface, COLORREF color)
{
    CInfinity* pInfinity = pArea->GetInfinity();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1101
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);

    CRect rViewport;
    rViewport.left = pInfinity->nCurrentX;
    rViewport.top = pInfinity->nCurrentY;
    rViewport.right = rViewport.left + pInfinity->rViewPort.Width();
    rViewport.bottom = rViewport.top + pInfinity->rViewPort.Height();

    CPoint ptReference(0, 0);

    CVidPoly poly;
    poly.SetPoly(reinterpret_cast<WORD*>(m_pPolygonPoints), m_nPolygon);

    for (int i = 0; i < m_boundingGrid.GetSize(); i++) {
        CRect* pGridRect = m_boundingGrid[i];
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

        if (pInfinity->FXPrep(rFXRect, CInfinity::FXPREP_COPYFROMBACK, nSurface, ptPos, ptReference)) {
            if (pInfinity->FXLock(rFXRect, 0)) {
                static_cast<CVidInf*>(pVidMode)->RenderConvexPoly(rClip, &poly, color, CInfinity::MIRROR_FX, ptPos, FALSE);

                CPoint ptZero(0, 0);
                if (pInfinity->FXUnlock(0, NULL, ptZero)) {
                    pInfinity->FXBltFrom(nSurface, rFXRect, ptPos.x, ptPos.y, ptReference.x, ptReference.y, 0);
                }
            }
        }
    }
}

// 0x4CD220
BOOLEAN CGameTrigger::DoAIUpdate(BOOLEAN active, LONG counter)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if ((m_dwFlags & 0x100) != 0) {
        return FALSE;
    }

    if (m_triggerType == 0) {
        if ((m_dwFlags & 0x20) != 0
            && *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pGame) + 0x4446) == 0) {
            return FALSE;
        }

        if (m_trapActivated != 0) {
            CTypedPtrList<CPtrList, LONG*> targets(10);
            m_pArea->GetCloseObjects(m_posVertList, m_pos, CAIObjectType::ANYONE,
                m_boundingRange, m_pArea->m_visibleTerrainTable, targets, FALSE, FALSE);

            POSITION pos = targets.GetHeadPosition();
            while (pos != NULL) {
                LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));

                CGameObject* pObject;
                if (pGame->GetObjectArray()->GetShare(nId, CGameObjectArray::THREAD_ASYNCH,
                        &pObject, INFINITE)
                    == CGameObjectArray::SUCCESS) {
                    CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
                    if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE
                        && ((m_dwFlags & 0x40) != 0
                            || pGame->GetCharacterPortraitNum(pSprite->m_id) != -1
                            || pGame->m_familiars.Find(reinterpret_cast<int*>(pSprite->m_id)) != NULL)) {
                        // Skip the edit-mode trigger-placement preview (CChitin +0x1032/+0x1033,
                        // CGameSprite +0x580); always 0 in normal play, so the trap is processed.
                        if (!(*(reinterpret_cast<BYTE*>(g_pChitin) + 0x1032) == 1
                              && *(reinterpret_cast<BYTE*>(g_pChitin) + 0x1033) == 1
                              && *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pSprite) + 0x580) == 1)) {
                            if (IsOverActivate(pSprite->GetPos())) {
                                CPoint ptLast(pSprite->field_536A, pSprite->field_536E);
                                if (!IsOverActivate(ptLast)) {
                                    BYTE rc;
                                    do {
                                        rc = pGame->GetObjectArray()->GetDeny(nId,
                                            CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
                                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                                    if (rc == CGameObjectArray::SUCCESS) {
                                        CAITrigger trigger(CAITRIGGER_ENTERED, pSprite->GetAIType(), 0);
                                        g_pBaldurChitin->GetMessageHandler()->AddMessage(
                                            new CMessageSetTrigger(trigger, m_id, m_id), FALSE);
                                        pGame->GetObjectArray()->ReleaseDeny(nId,
                                            CGameObjectArray::THREAD_ASYNCH, INFINITE);
                                    }

                                    if ((m_dwFlags & 0x2) == 0) {
                                        m_trapActivated = 0;
                                    }
                                }
                            }

                            CPoint ptNow = pSprite->GetPos();
                            pSprite->field_536A = ptNow.x;
                            pSprite->field_536E = ptNow.y;
                        }
                    }

                    pGame->GetObjectArray()->ReleaseShare(nId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }
            }

            targets.RemoveAll();
            return TRUE;
        }
    }

    return CResRef(m_scriptRes) != CResRef();
}

// 0x4CE180
BOOLEAN CGameTrigger::CompressTime(DWORD deltaTime)
{
    if (m_triggerType == 0) {
        // NOTE: Unsigned compare.
        if (deltaTime > static_cast<DWORD>(m_drawPoly)) {
            m_drawPoly = 0;
        } else {
            m_drawPoly = static_cast<SHORT>(m_drawPoly - deltaTime);
        }
    }

    return TRUE;
}

// 0x4CE1C0
void CGameTrigger::DebugDump(const CString& message, BOOLEAN bEchoToScreen)
{
    STR_RES strRes;
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    CString sTemp;

    switch (m_triggerType) {
    case 0:
        if (bEchoToScreen) {
            pWorld->DisplayText(CString(""),
                CString("DEBUG DUMP: CGameTrigger - Proximity"),
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

            sTemp.Format("Reset Trap: %s", (m_dwFlags & 0x2) != 0 ? "TRUE" : "FALSE");
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

            sTemp.Format("Trap Launching Point: x=%d y=%d", m_posTrapOrigin.x, m_posTrapOrigin.y);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);
        }
        break;
    case 1:
        if (bEchoToScreen) {
            pWorld->DisplayText(CString(""),
                CString("DEBUG DUMP: CGameTrigger - Info"),
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

            sTemp.Format("STRREF: %d", m_description);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            // FIXME: `strRes` is never fetched (always empty string)
            sTemp.Format("Description: %s", (LPCSTR)strRes.szText);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);
        }
        break;
    case 2:
        if (bEchoToScreen) {
            pWorld->DisplayText(CString(""),
                CString("DEBUG DUMP: CGameTrigger - Travel"),
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

            sTemp.Format("Target Area: %.*s", RESREF_SIZE, m_newArea);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            sTemp.Format("Entry Point Name: %.*s", SCRIPTNAME_SIZE, m_newEntryPoint);
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);

            sTemp.Format("Party Required: %s", (m_dwFlags & 0x4) != 0 ? "TRUE" : "FALSE");
            pWorld->DisplayText(CString(""),
                sTemp,
                -1,
                FALSE);
        }
        break;
    }
}

// 0x4CEBE0
BOOL CGameTrigger::IsOver(const CPoint& pt)
{
    if ((m_dwFlags & 0x100) != 0) {
        return FALSE;
    }

    if (m_triggerType == 0) {
        if (m_trapActivated == 0) {
            return FALSE;
        }

        if (m_trapDetected == 0) {
            return FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetState() != 2) {
            return FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetIconIndex() != 36) {
            return FALSE;
        }
    }

    if (!m_rBounding.PtInRect(pt)) {
        return FALSE;
    }

    if (!g_pBaldurChitin->GetObjectGame()->GetGroup()->IsPartyLeader()) {
        return FALSE;
    }

    if (m_pPolygon != NULL) {
        return CVidPoly::IsPtInPoly(m_pPolygon, m_nPolygon, pt);
    }

    return TRUE;
}

// 0x4CECB0
BOOL CGameTrigger::IsOverActivate(const CPoint& pt)
{
    if (!m_rBounding.PtInRect(pt)) {
        return FALSE;
    }

    if (m_pPolygon != NULL) {
        return CVidPoly::IsPtInPoly(m_pPolygon, m_nPolygon, pt);
    }

    return TRUE;
}

// 0x4CED10
void CGameTrigger::OnActionButton(const CPoint& pt)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CAIGroup* pGroup = pGame->GetGroup();

    // Fire a Clicked() script trigger (caused by the party leader) so area
    // scripts can react to the click before it is otherwise handled.
    if (pGroup->GetCount() != 0) {
        CAIObjectType leaderType;

        LONG leaderId = pGroup->GetGroupLeader();
        if (leaderId != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pLeader;

            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(leaderId,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pLeader,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                leaderType.Set(pLeader->GetAIType());
                pGame->GetObjectArray()->ReleaseShare(leaderId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        CMessage* message = new CMessageSetTrigger(
            CAITrigger(CAITrigger::CLICKED, leaderType, 0),
            m_id,
            m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    switch (pGame->GetState()) {
    case 0:
        pGame->SetLastClick(CPoint(-1, -1));
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        if (pGroup->GetCount() == 0) {
            return;
        }

        if (m_triggerType == 2) {
            // Travel region: walk the whole party onto the trigger, then move
            // them to the trigger's destination point.
            // travelAction.m_specificID = m_id (this trigger's own object id)
            // -- live Frida trace against the original exe (a hook on
            // CGameSprite::ArriveAtTravelTrigger, the confirmed action-93
            // handler, reading this.returnAddress + m_curAction.m_specificID
            // during a real staircase click) shows m_specificID equal to the
            // clicked trigger's id at the moment this action executes.
            // ArriveAtTravelTrigger resolves the CGameTrigger via exactly this
            // field, so it must carry m_id, not 0.
            CAIObjectType travelType(0, 0, 0, 0, 0, 0, 0, 0, m_id, 0, 0);
            CAIAction travelAction(93 /* 0x84782E: travel action, not in ACTION.IDS */,
                travelType,
                m_id,
                0,
                0);
            pGroup->GroupAction(travelAction, TRUE, NULL);

            CPoint destination = (m_dwFlags & 0x200) != 0 ? m_ptUsePoint : m_pos;
            CAIAction moveAction(CAIAction::MOVETOPOINT, destination, 0, -1);
            pGroup->GroupAction(moveAction, FALSE, NULL);

            pGroup->SetGroupTriggerId(m_id);

            if (m_description != -1 && m_description != 0) {
                STR_RES strRes;
                g_pBaldurChitin->GetTlkTable().Fetch(m_description, strRes);
                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    strRes.szText,
                    0xBED7D7,
                    0xFFB4B4,
                    -1,
                    FALSE);
            }
        } else if (m_triggerType == 1) {
            // Info point: float the trigger's info string above it (duration 10,
            // begin-fade 5) when m_description is valid.  Like the original, an
            // info point does not walk the party.
            if (m_description != -1) {
                FloatText(m_description, 0xA, 5);
            }
        } else {
            CGameObject::OnActionButton(pt);
        }
        break;

    case 2:
        pGame->SetLastClick(CPoint(-1, -1));
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        switch (pGame->GetIconIndex()) {
        case 0x24: {
            // Disarm a known, active floor trap.
            if (m_triggerType != 0 || m_trapActivated == 0 || m_trapDetected == 0) {
                CGameObject::OnActionButton(pt);
                return;
            }

            // Abort unless the selected character actually has the
            // disable-device skill.
            SHORT nPortrait = g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter();
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
                BYTE nSkill = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE];
                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                if (nSkill == 0) {
                    return;
                }
            }

            CAIAction removeTraps(CAIAction::REMOVETRAPS, m_typeAI, 0, 0, 0);
            pGroup->GroupAction(removeTraps, TRUE, NULL);

            pGame->SetState(0);
            pGame->GetButtonArray()->SetSelectedButton(100);
            pGame->GetButtonArray()->UpdateState();
            return;
        }

        case 0x0C:
        case 0x12:
        case 0x14:
        case 0x28:
        case 0xFF:
            CGameObject::OnActionButton(pt);
            return;

        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
            // __LINE__: 973
            UTIL_ASSERT(FALSE);
        }
        break;

    case 3:
        pGame->SetLastClick(CPoint(-1, -1));
        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);
        CGameObject::OnActionButton(pt);
        break;

    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
        // __LINE__: 984
        UTIL_ASSERT(FALSE);
    }
}

// 0x4CFC20
void CGameTrigger::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
        // __LINE__: 111
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x4CFC80
void CGameTrigger::SetCursor(LONG nToolTip)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if ((m_dwFlags & 0x100) == 0) {
        switch (pGame->GetState()) {
        case 0:
            if (pGame->GetGroup()->GetCount() != 0) {
                if (m_triggerType != 0) {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(m_cursorType, FALSE);
                } else {
                    CGameObject::SetCursor(nToolTip);
                }
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
            case 18:
            case 40:
            case 255:
                CGameObject::SetCursor(nToolTip);
                break;
            case 36:
                if (m_triggerType == 0
                    && m_trapActivated != 0
                    && m_trapDetected != 0) {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(38, FALSE);

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
                        if (pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 0) {
                            g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                        }

                        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                } else {
                    CGameObject::SetCursor(nToolTip);
                }
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
                // __LINE__: 1200
                UTIL_ASSERT(FALSE);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
            // __LINE__: 1210
            UTIL_ASSERT(FALSE);
        }
    } else {
        CGameObject::SetCursor(nToolTip);
    }
}

// 0x4D0230
void CGameTrigger::SetDrawPoly(SHORT time)
{
    if (m_drawPoly != time) {
        if (!g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly) {
            CMessageSetDrawPoly* pMessage = new CMessageSetDrawPoly(time, GetId(), GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }
    m_drawPoly = time;
}

// 0x4D02A0
CPoint& CGameTrigger::GetPos()
{
    return (m_dwFlags & 0x200) != 0 ? m_ptUsePoint : m_pos;
}

// 0x45B950
BOOL CGameTrigger::IsTrapActive()
{
    return m_trapActivated && (m_dwFlags & 0x100) == 0;
}

// 0x4CFF70
void CGameTrigger::Marshal(CAreaFileTriggerObject** pTriggerObject)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1235
    UTIL_ASSERT(pTriggerObject != NULL);

    *pTriggerObject = new CAreaFileTriggerObject();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
    // __LINE__: 1241
    UTIL_ASSERT(*pTriggerObject != NULL);

    memset(*pTriggerObject, 0, sizeof(CAreaFileTriggerObject));

    if (m_nPolygon != 0) {
        CAreaPoint* pPoints = new CAreaPoint[m_nPolygon];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameTrigger.cpp
        // __LINE__: 1246
        UTIL_ASSERT(pPoints != NULL);

        for (WORD cnt = 0; cnt < m_nPolygon; cnt++) {
            pPoints[cnt].m_xPos = static_cast<WORD>(m_pPolygon[cnt].x);
            pPoints[cnt].m_yPos = static_cast<WORD>(m_pPolygon[cnt].y);
        }

        (*pTriggerObject)->m_pickPointStart = reinterpret_cast<DWORD>(pPoints);
        (*pTriggerObject)->m_pickPointCount = m_nPolygon;
    }

    (*pTriggerObject)->m_triggerType = m_triggerType;
    (*pTriggerObject)->m_boundingRectLeft = static_cast<WORD>(m_rBounding.left);
    (*pTriggerObject)->m_boundingRectTop = static_cast<WORD>(m_rBounding.top);
    (*pTriggerObject)->m_boundingRectRight = static_cast<WORD>(m_rBounding.right) - 1;
    (*pTriggerObject)->m_boundingRectBottom = static_cast<WORD>(m_rBounding.bottom) - 1;

    if ((m_dwFlags & 0x200) == 0) {
        (*pTriggerObject)->m_transitionWalkToX = static_cast<WORD>(m_pos.x);
        (*pTriggerObject)->m_transitionWalkToY = static_cast<WORD>(m_pos.y);
    } else {
        (*pTriggerObject)->m_transitionWalkToX = static_cast<WORD>(m_ptUsePoint.x);
        (*pTriggerObject)->m_transitionWalkToY = static_cast<WORD>(m_ptUsePoint.y);
    }

    (*pTriggerObject)->m_cursorType = m_cursorType;
    memcpy((*pTriggerObject)->m_newArea, m_newArea, RESREF_SIZE);
    memcpy((*pTriggerObject)->m_newEntryPoint, m_newEntryPoint, SCRIPTNAME_SIZE);
    (*pTriggerObject)->m_dwFlags = m_dwFlags;
    (*pTriggerObject)->m_description = m_description;
    (*pTriggerObject)->m_posXTrapOrigin = static_cast<WORD>(m_posTrapOrigin.x);
    (*pTriggerObject)->m_posYTrapOrigin = static_cast<WORD>(m_posTrapOrigin.y);
    memcpy((*pTriggerObject)->m_scriptName, m_scriptName, SCRIPTNAME_SIZE);
    (*pTriggerObject)->m_trapDetectionDifficulty = m_trapDetectionDifficulty;
    (*pTriggerObject)->m_trapDisarmingDifficulty = m_trapDisarmingDifficulty;
    (*pTriggerObject)->m_trapActivated = m_trapActivated;
    (*pTriggerObject)->m_trapDetected = m_trapDetected;
    m_keyType.GetResRef((*pTriggerObject)->m_keyType);
    memcpy((*pTriggerObject)->m_script, m_scriptRes, RESREF_SIZE);
    (*pTriggerObject)->field_88 = m_ptUsePoint.x;
    (*pTriggerObject)->field_8C = m_ptUsePoint.y;
}
