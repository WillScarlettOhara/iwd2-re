#include "CAIGroup.h"

#include <math.h>
#include <stdlib.h>

#include "CAITrigger.h"
#include "CBaldurChitin.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CMessage.h"
#include "CInfGame.h"
#include "CGameArea.h"
#include "CPathSearch.h"
#include "CSearchBitmap.h"

static const double PI = 3.14159265358979323846;
static const double TWO_PI = 6.28318530717958647692;

// 0x8479E4
const LONG CAIGroup::OFFSET_MULTIPLIER = 1000;

// 0x8479E8
const SHORT CAIGroup::FORMATION_NONE = 12;

// 0x8479EA
const SHORT CAIGroup::FORMATION_GATHER = CAIGROUP_FORMATION_GATHER;

// 0x8C6D98
const CPoint CAIGroup::FORMATION_GATHER_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(0, -3),
    CPoint(3, -2),
    CPoint(-3, -2),
    CPoint(3, 2),
    CPoint(-3, 2),
    CPoint(0, 3),
    CPoint(3, 5),
    CPoint(-3, 5),
    CPoint(0, 6),
    CPoint(3, 8),
    CPoint(-3, 8),
    CPoint(0, 9),
};

// 0x8479EC
const SHORT CAIGroup::FORMATION_GATHER_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    10,
    6,
    14,
    2,
    0,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847A04
const SHORT CAIGroup::FORMATION_T = CAIGROUP_FORMATION_T;

// 0x8C6DF8
const CPoint CAIGroup::FORMATION_T_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(0, 0),
    CPoint(4, 0),
    CPoint(-4, 0),
    CPoint(0, 4),
    CPoint(0, 7),
    CPoint(0, 10),
    CPoint(4, 4),
    CPoint(-4, 4),
    CPoint(4, 7),
    CPoint(-4, 7),
    CPoint(4, 10),
    CPoint(-4, 10),
};

// 0x847A08
const SHORT CAIGroup::FORMATION_T_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847A20
const SHORT CAIGroup::FORMATION_RANK = CAIGROUP_FORMATION_RANK;

// 0x8C66E0
const CPoint CAIGroup::FORMATION_RANK_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(-2, 0),
    CPoint(2, 0),
    CPoint(-6, 0),
    CPoint(6, 0),
    CPoint(-10, 0),
    CPoint(10, 0),
    CPoint(-2, 3),
    CPoint(2, 3),
    CPoint(-6, 3),
    CPoint(6, 3),
    CPoint(-10, 3),
    CPoint(10, 3),
};

// 0x847A24
const SHORT CAIGroup::FORMATION_RANK_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847A3C
const SHORT CAIGroup::FORMATION_3BY2 = CAIGROUP_FORMATION_3BY2;

// 0x8C6BA0
const CPoint CAIGroup::FORMATION_3BY2_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(0, 0),
    CPoint(4, 0),
    CPoint(-4, 0),
    CPoint(0, 4),
    CPoint(4, 4),
    CPoint(-4, 4),
    CPoint(0, 8),
    CPoint(4, 8),
    CPoint(-4, 8),
    CPoint(0, 12),
    CPoint(4, 12),
    CPoint(-1, 12),
};

// 0x847A40
const SHORT CAIGroup::FORMATION_3BY2_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847A58
const SHORT CAIGroup::FORMATION_2BY3 = CAIGROUP_FORMATION_2BY3;

// 0x8C6C08
const CPoint CAIGroup::FORMATION_2BY3_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(-2, 0),
    CPoint(2, 0),
    CPoint(-2, 4),
    CPoint(2, 4),
    CPoint(-2, 7),
    CPoint(2, 7),
    CPoint(-6, 0),
    CPoint(6, 0),
    CPoint(-6, 4),
    CPoint(6, 4),
    CPoint(-6, 7),
    CPoint(6, 7),
};

// 0x847A5C
const SHORT CAIGroup::FORMATION_2BY3_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847A74
const SHORT CAIGroup::FORMATION_PROTECT = CAIGROUP_FORMATION_PROTECT;

// 0x8C6740
const CPoint CAIGroup::FORMATION_PROTECT_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(0, -3),
    CPoint(4, 0),
    CPoint(-4, 0),
    CPoint(2, 4),
    CPoint(-2, 4),
    CPoint(0, 0),
    CPoint(0, -6),
    CPoint(4, -3),
    CPoint(-4, -3),
    CPoint(5, 5),
    CPoint(-5, 5),
    CPoint(0, 7),
};

// 0x847A78
const SHORT CAIGroup::FORMATION_PROTECT_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    10,
    6,
    14,
    2,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847A90
const SHORT CAIGroup::FORMATION_4AND2 = CAIGROUP_FORMATION_4AND2;

// 0x8C6D30
const CPoint CAIGroup::FORMATION_4AND2_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(-2, 0),
    CPoint(2, 0),
    CPoint(-6, 0),
    CPoint(6, 0),
    CPoint(-2, 4),
    CPoint(2, 4),
    CPoint(-6, 4),
    CPoint(6, 4),
    CPoint(-2, 8),
    CPoint(2, 8),
    CPoint(-6, 8),
    CPoint(6, 8),
};

// 0x847A94
const SHORT CAIGroup::FORMATION_4AND2_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847AAC
const SHORT CAIGroup::FORMATION_V = CAIGROUP_FORMATION_V;

// 0x8C6C68
const CPoint CAIGroup::FORMATION_V_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(-1, 0),
    CPoint(1, 0),
    CPoint(-2, 3),
    CPoint(2, 3),
    CPoint(-4, 6),
    CPoint(4, 6),
    CPoint(-1, 6),
    CPoint(1, 6),
    CPoint(-2, 9),
    CPoint(2, 9),
    CPoint(-4, 12),
    CPoint(4, 12),
};

// 0x847AB0
const SHORT CAIGroup::FORMATION_V_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847AC8
const SHORT CAIGroup::FORMATION_WEDGE = CAIGROUP_FORMATION_WEDGE;

// 0x8C6618
const CPoint CAIGroup::FORMATION_WEDGE_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(0, 0),
    CPoint(4, 3),
    CPoint(-4, 3),
    CPoint(-8, 6),
    CPoint(8, 6),
    CPoint(0, 6),
    CPoint(-8, 0),
    CPoint(8, 0),
    CPoint(-12, 9),
    CPoint(-6, 9),
    CPoint(6, 9),
    CPoint(12, 9),
};

// 0x847ACC
const SHORT CAIGroup::FORMATION_WEDGE_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847AE4
const SHORT CAIGroup::FORMATION_LINE = CAIGROUP_FORMATION_LINE;

// 0x8C6680
const CPoint CAIGroup::FORMATION_LINE_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(0, 0),
    CPoint(0, 3),
    CPoint(0, 6),
    CPoint(0, 9),
    CPoint(0, 12),
    CPoint(0, 15),
    CPoint(0, 18),
    CPoint(0, 21),
    CPoint(0, 24),
    CPoint(0, 27),
    CPoint(0, 30),
    CPoint(0, 33),
};

// 0x847AE8
const SHORT CAIGroup::FORMATION_LINE_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847B00
const SHORT CAIGroup::FORMATION_S = CAIGROUP_FORMATION_S;

// 0x8C6CC8
const CPoint CAIGroup::FORMATION_S_OFFSETS[CAIGROUP_FORMATION_MAX_SIZE] = {
    CPoint(1, 0),
    CPoint(-1, 2),
    CPoint(1, 4),
    CPoint(-1, 6),
    CPoint(1, 8),
    CPoint(-1, 10),
    CPoint(1, 12),
    CPoint(-1, 14),
    CPoint(1, 16),
    CPoint(-1, 18),
    CPoint(1, 20),
    CPoint(-1, 22),
};

// 0x847B04
const SHORT CAIGroup::FORMATION_S_FACE[CAIGROUP_FORMATION_MAX_SIZE] = {
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};

// 0x847B1C
const SHORT CAIGroup::FORMATION_FOLLOW = CAIGROUP_FORMATION_FOLLOW;

// 0x847B1E
const SHORT CAIGroup::DEADZONE = 12;

// 0x847B20
const SHORT CAIGroup::WAIT_BASE = 5;

// 0x847B22
const SHORT CAIGroup::FORMATION_MAX_SIZE = CAIGROUP_FORMATION_MAX_SIZE;

// NOTE: Inlined.
CAIGroup::CAIGroup(SHORT id)
{
    m_groupId = id;
    m_groupChanged = FALSE;
}

// FIXME: `action` should be reference.
//
// 0x404D00
//
// Per member: GetDeny-lock it, optionally interrupt its current action
// (`override`), reset its user-command pause and clear any pending
// travel-trigger tag, then queue either the leader's action (first member
// only, when `leaderAction` is given) or the primary `action` -- unless that
// action is MOVETOPOINT and the member is too encumbered to move
// (`m_derivedStats.m_nEncumberance == 2`), in which case it gets a feedback
// beep instead of a queued move. The leader additionally gets a
// "command accepted" sound when `override` is set, for any action other than
// GROUPATTACK (which already has its own feedback elsewhere).
void CAIGroup::GroupAction(CAIAction action, BOOL override, CAIAction* leaderAction)
{
    BOOL bLeader = TRUE;
    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pMember;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pMember),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        if (override) {
            pMember->ClearActions(FALSE);
            pMember->m_interrupt = TRUE;
        }
        pMember->m_userCommandPause = 75;
        pMember->m_triggerId = CGameObjectArray::INVALID_INDEX;

        BOOL bEncumberedMoveBlocked = (action.GetActionID() == CAIAction::MOVETOPOINT)
            && (pMember->GetDerivedStats()->m_nEncumberance == 2);

        if (bLeader && leaderAction != NULL) {
            if (leaderAction->GetActionID() != CAIAction::MOVETOPOINT
                || pMember->GetDerivedStats()->m_nEncumberance != 2) {
                CMessageAddAction* pMsg = new CMessageAddAction(*leaderAction, memberId, memberId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
            } else {
                pMember->FeedBack(15, 1, 0, 0, -1, 0, 0);
            }
        } else if (bEncumberedMoveBlocked) {
            pMember->FeedBack(15, 1, 0, 0, -1, 0, 0);
        } else {
            CMessageAddAction* pMsg = new CMessageAddAction(action, memberId, memberId);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
        }

        if (bLeader) {
            if (override && action.GetActionID() != CAIAction::GROUPATTACK) {
                pMember->PlaySound(11, TRUE, FALSE, FALSE);
            }
            bLeader = FALSE;
        }

        pMember->m_inFormation = TRUE;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x4052D0
void CAIGroup::SetGroupTriggerId(LONG triggerId)
{
    if (m_memberList.IsEmpty()) {
        return;
    }

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->m_triggerId = triggerId;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x405E00
void CAIGroup::RemoveFromSearch(CSearchBitmap* search)
{
    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CPoint pos = pSprite->GetPos();
            BYTE nPersonalSpace = pSprite->GetAnimation()->GetPersonalSpace();
            BYTE nEnemyAlly = pSprite->GetAIType().m_nEnemyAlly;

            pos.x /= CPathSearch::GRID_SQUARE_SIZEX;
            pos.y /= CPathSearch::GRID_SQUARE_SIZEY;
            search->RemoveObject(pos,
                nEnemyAlly,
                nPersonalSpace,
                pSprite->m_bBumpable,
                pSprite->m_bOnSearchMap);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x405F60
void CAIGroup::AddToSearch(CSearchBitmap* search)
{
    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CPoint pos = pSprite->GetPos();
            BYTE nPersonalSpace = pSprite->GetAnimation()->GetPersonalSpace();
            BYTE nEnemyAlly = pSprite->GetAIType().m_nEnemyAlly;

            pos.x /= CPathSearch::GRID_SQUARE_SIZEX;
            pos.y /= CPathSearch::GRID_SQUARE_SIZEY;
            search->AddObject(pos,
                nEnemyAlly,
                nPersonalSpace,
                pSprite->m_bBumpable,
                pSprite->m_bOnSearchMap);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4060C0
LONG CAIGroup::GetGroupLeader()
{
    if (m_memberList.GetCount() > 0) {
        return reinterpret_cast<LONG>(m_memberList.GetHead());
    }

    return CGameObjectArray::INVALID_INDEX;
}

// 0x4060E0
LONG* CAIGroup::GetGroupList()
{
    LONG* pList = new LONG[m_memberList.GetCount()];

    int index = 0;
    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        pList[index++] = reinterpret_cast<LONG>(m_memberList.GetNext(pos));
    }

    return pList;
}

// 0x406110
void CAIGroup::Add(CGameSprite* pSprite)
{
    if (pSprite->m_groupPosition != NULL) {
        // NOTE: Uninline.
        Remove(pSprite);
    }

    pSprite->m_groupPosition = Add(pSprite->m_id);
}

// 0x406170
POSITION CAIGroup::Add(LONG characterId)
{
    SHORT portraitNum = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(characterId);
    m_groupChanged = TRUE;
    if (portraitNum == -1) {
        return m_memberList.AddTail(reinterpret_cast<LONG*>(characterId));
    }

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetAt(pos));
        SHORT memberPortraitNum = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(memberId);
        if (memberPortraitNum == -1 || memberPortraitNum > portraitNum) {
            return m_memberList.InsertBefore(pos, reinterpret_cast<LONG*>(characterId));
        }
        m_memberList.GetNext(pos);
    }

    return m_memberList.AddTail(reinterpret_cast<LONG*>(characterId));
}

// 0x406210
void CAIGroup::Remove(CGameSprite* pSprite)
{
    m_groupChanged = TRUE;
    if (pSprite->m_groupPosition != NULL) {
        m_memberList.RemoveAt(pSprite->m_groupPosition);
        m_groupChanged = TRUE;
    }
    pSprite->m_groupPosition = NULL;
}

// 0x406250
BOOL CAIGroup::IsPartyLeader()
{
    if (m_memberList.GetCount() == 0) {
        return FALSE;
    }

    LONG memberId = reinterpret_cast<LONG>(m_memberList.GetHead());
    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(memberId) != -1) {
        return TRUE;
    }

    return g_pBaldurChitin->GetObjectGame()->m_familiars.Find(reinterpret_cast<int*>(memberId)) != NULL;
}

// 0x4062B0
void CAIGroup::Sort()
{
    CAIGroup tempGroup;
    POSITION pos;

    m_groupChanged = TRUE;

    pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));
        tempGroup.Add(memberId);
    }

    m_memberList.RemoveAll();

    pos = tempGroup.m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(tempGroup.m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->m_groupPosition = m_memberList.AddTail(reinterpret_cast<LONG*>(memberId));

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4058E0
void CAIGroup::RotateOffsets(CPoint* offsets, SHORT count, SHORT rotationDegrees)
{
    double angleRadians = (rotationDegrees * 2.0 * PI) / 360.0;

    for (int i = 0; i < count; i++) {
        LONG x = offsets[i].x;
        LONG y = offsets[i].y;

        double dist = sqrt((double)(x * x + y * y));
        if (dist == 0.0) {
            continue;
        }

        LONG absX = (x ^ (x >> 31)) - (x >> 31);
        double baseAngle = acos((double)absX / dist);

        if (x < 0) {
            if (y < 0) {
                baseAngle = baseAngle + PI;
            } else {
                baseAngle = PI - baseAngle;
            }
        } else {
            if (y < 0) {
                baseAngle = TWO_PI - baseAngle;
            }
        }

        double newAngle = baseAngle + angleRadians;
        offsets[i].x = (LONG)(dist * cos(newAngle));
        offsets[i].y = (LONG)(dist * sin(newAngle));
    }
}

// 0x405370
CPoint* CAIGroup::GetFormationOffsets(SHORT formationType, SHORT rotationDegrees, BYTE bExtend)
{
    BYTE count;
    BYTE memberCount = static_cast<BYTE>(m_memberList.GetCount());

    if (bExtend == 0) {
        count = memberCount;
        if (memberCount < 2) {
            count = 1;
        }
    } else {
        if (memberCount < 7) {
            count = 6;
        } else {
            count = memberCount;
        }
    }

    CPoint* offsets = new CPoint[count];
    for (int i = 0; i < count; i++) {
        offsets[i].x = 0;
        offsets[i].y = 0;
    }

    if (bExtend == 1) {
        RotateOffsets(offsets, count, rotationDegrees);
        return offsets;
    }

    CPoint leaderPos(-1, -1);

    if (m_memberList.GetHeadPosition() != NULL) {
        LONG leaderId = reinterpret_cast<LONG>(m_memberList.GetHead());

        CGameSprite* pLeader;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(leaderId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pLeader),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CPoint& ptLeaderPos = pLeader->GetPos();
            leaderPos.x = ptLeaderPos.x;
            leaderPos.y = ptLeaderPos.y;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(leaderId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            return offsets;
        }
    }

    LONG* memberIds = NULL;

    if (memberCount > CAIGROUP_FORMATION_MAX_SIZE) {
        memberIds = new LONG[memberCount];
        POSITION pos = m_memberList.GetHeadPosition();
        int idx = 0;
        while (pos != NULL) {
            memberIds[idx++] = reinterpret_cast<LONG>(m_memberList.GetNext(pos));
        }
    }

    if (formationType == FORMATION_NONE) {
        if (memberIds != NULL) {
            delete[] memberIds;
        }

        if (memberCount == 0) {
            return offsets;
        }

        memberIds = new LONG[memberCount];
        POSITION pos = m_memberList.GetHeadPosition();
        int idx = 0;
        while (pos != NULL) {
            memberIds[idx++] = reinterpret_cast<LONG>(m_memberList.GetNext(pos));
        }
    }

    offsets[0].x = 0;
    offsets[0].y = 0;

    LONG* pMemberId = memberIds;

    for (int i = 0; i < count; i++) {
        if (i > 11) {
            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*pMemberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                CPoint& ptPos = pSprite->GetPos();
                offsets[i].x = (ptPos.x - leaderPos.x) / CPathSearch::GRID_SQUARE_SIZEX;

                CPoint& ptPos2 = pSprite->GetPos();
                offsets[i].y = (ptPos2.y - leaderPos.y) / CPathSearch::GRID_SQUARE_SIZEY;

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*pMemberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                offsets[i].x = offsets[i].x * OFFSET_MULTIPLIER;
                offsets[i].y = offsets[i].y * OFFSET_MULTIPLIER;
            }

            pMemberId++;
            continue;
        }

        switch (formationType) {
        case CAIGROUP_FORMATION_FOLLOW:
        case CAIGROUP_FORMATION_T:
            offsets[i] = FORMATION_T_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_GATHER:
            offsets[i] = FORMATION_GATHER_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_4AND2:
            offsets[i] = FORMATION_4AND2_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_3BY2:
            offsets[i] = FORMATION_3BY2_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_PROTECT:
            offsets[i] = FORMATION_PROTECT_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_2BY3:
            offsets[i] = FORMATION_2BY3_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_RANK:
            offsets[i] = FORMATION_RANK_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_V:
            offsets[i] = FORMATION_V_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_WEDGE:
            offsets[i] = FORMATION_WEDGE_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_S:
            offsets[i] = FORMATION_S_OFFSETS[i];
            break;
        case CAIGROUP_FORMATION_LINE:
            offsets[i] = FORMATION_LINE_OFFSETS[i];
            break;
        case 12: {
            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*pMemberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                pMemberId++;
                continue;
            }

            CPoint& ptPos = pSprite->GetPos();
            offsets[i].x = (ptPos.x - leaderPos.x) / CPathSearch::GRID_SQUARE_SIZEX;

            CPoint& ptPos2 = pSprite->GetPos();
            offsets[i].y = (ptPos2.y - leaderPos.y) / CPathSearch::GRID_SQUARE_SIZEY;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*pMemberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            break;
        }
        }

        offsets[i].x = offsets[i].x * OFFSET_MULTIPLIER;
        offsets[i].y = offsets[i].y * OFFSET_MULTIPLIER;

        pMemberId++;
    }

    RotateOffsets(offsets, count, rotationDegrees);

    if (memberIds != NULL) {
        delete[] memberIds;
    }

    return offsets;
}

// 0x405A10
SHORT* CAIGroup::GetFormationFaces(SHORT formationType, SHORT rotationDegrees)
{
    BYTE memberCount = static_cast<BYTE>(m_memberList.GetCount());

    SHORT* faces = new SHORT[memberCount];
    for (int i = 0; i < memberCount; i++) {
        faces[i] = 0;
    }

    if (m_memberList.GetHeadPosition() == NULL) {
        return faces;
    }

    LONG leaderId = reinterpret_cast<LONG>(m_memberList.GetHead());

    LONG* memberIds = NULL;

    if (memberCount > CAIGROUP_FORMATION_MAX_SIZE) {
        memberIds = new LONG[memberCount];
        POSITION pos = m_memberList.GetHeadPosition();
        int idx = 0;
        while (pos != NULL) {
            memberIds[idx++] = reinterpret_cast<LONG>(m_memberList.GetNext(pos));
        }
    }

    if (formationType == FORMATION_NONE) {
        if (memberIds != NULL) {
            delete[] memberIds;
        }

        memberIds = new LONG[memberCount];
        POSITION pos = m_memberList.GetHeadPosition();
        int idx = 0;
        while (pos != NULL) {
            memberIds[idx++] = reinterpret_cast<LONG>(m_memberList.GetNext(pos));
        }
    }

    LONG* pMemberId = memberIds;

    for (int i = 0; i < memberCount; i++) {
        if (i > 11) {
            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*pMemberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                faces[i] = pSprite->m_nDirection;

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*pMemberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                SHORT rotatedFace = (SHORT)((rotationDegrees * 16 / 360 + faces[i]) & 0xF);
                if (rotatedFace < 0) {
                    rotatedFace = (rotatedFace - 1 | ~0xF) + 1;
                }
                faces[i] = rotatedFace;
            }

            pMemberId++;
            continue;
        }

        switch (formationType) {
        case CAIGROUP_FORMATION_T:
            faces[i] = FORMATION_T_FACE[i];
            break;
        case CAIGROUP_FORMATION_GATHER:
            faces[i] = FORMATION_GATHER_FACE[i];
            break;
        case CAIGROUP_FORMATION_4AND2:
            faces[i] = FORMATION_4AND2_FACE[i];
            break;
        case CAIGROUP_FORMATION_3BY2:
            faces[i] = FORMATION_3BY2_FACE[i];
            break;
        case CAIGROUP_FORMATION_PROTECT:
            faces[i] = FORMATION_PROTECT_FACE[i];
            break;
        case CAIGROUP_FORMATION_2BY3:
            faces[i] = FORMATION_2BY3_FACE[i];
            break;
        case CAIGROUP_FORMATION_RANK:
            faces[i] = FORMATION_RANK_FACE[i];
            break;
        case CAIGROUP_FORMATION_V:
            faces[i] = FORMATION_V_FACE[i];
            break;
        case CAIGROUP_FORMATION_WEDGE:
            faces[i] = FORMATION_WEDGE_FACE[i];
            break;
        case CAIGROUP_FORMATION_S:
            faces[i] = FORMATION_S_FACE[i];
            break;
        case CAIGROUP_FORMATION_LINE:
            faces[i] = FORMATION_LINE_FACE[i];
            break;
        case 12: {
            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(*pMemberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                pMemberId++;
                continue;
            }

            faces[i] = pSprite->m_nDirection;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(*pMemberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            break;
        }
        }

        SHORT rotatedFace = (SHORT)((rotationDegrees * 16 / 360 + faces[i]) & 0xF);
        if (rotatedFace < 0) {
            rotatedFace = (rotatedFace - 1 | ~0xF) + 1;
        }
        faces[i] = rotatedFace;

        pMemberId++;
    }

    CGameSprite* pLeader;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(leaderId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pLeader),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pLeader->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (memberIds != NULL) {
        delete[] memberIds;
    }

    return faces;
}

// 0x4063E0
void CAIGroup::GroupSetTarget(CPoint target, BOOL additive, SHORT formationType, CPoint cursor)
{
    if (m_memberList.IsEmpty()) {
        return;
    }

    CTypedPtrList<CPtrList, CAIAction*> actions;

    if (formationType == 0) {
        FollowLeader(target, additive);
        return;
    }

    LONG absX = (target.x - cursor.x);
    absX = (absX ^ (absX >> 31)) - (absX >> 31);
    LONG absY = (target.y - cursor.y);
    absY = (absY ^ (absY >> 31)) - (absY >> 31);

    if (absX < DEADZONE && absY < DEADZONE * 3 / 4) {
        cursor.x = -1;
        cursor.y = -1;
    }

    BOOL bFirst = TRUE;
    if (m_memberList.GetCount() == 0) {
        return;
    }

    if (m_memberList.GetCount() == 1) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetHead());

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->m_curAction.m_actionID == 2) {
                pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION, TRUE, FALSE, FALSE);
            } else {
                CAIAction* moveAction = new CAIAction(CAIAction::MOVETOPOINT, target, 0, -1);
                actions.AddTail(moveAction);

                pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;

                if (additive == 0) {
                    pSprite->m_interrupt = TRUE;
                    while (actions.GetCount() != 0) {
                        CAIAction* action = actions.RemoveTail();
                        pSprite->AddAction(*action);
                        delete action;
                    }
                } else {
                    if (pSprite->m_curAction.m_actionID != CAIAction::MOVETOPOINT) {
                        pSprite->m_interrupt = TRUE;
                    }
                    while (actions.GetCount() != 0) {
                        CAIAction* action = actions.RemoveTail();
                        pSprite->InsertAction(*action);
                        delete action;
                    }
                }

                pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION, TRUE, FALSE, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        return;
    }

    RemoveFromSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);

    CPoint* offsets = NULL;
    SHORT* faces = NULL;
    int offsetIndex = 0;
    int faceIndex = 0;

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        if (bFirst) {
            bFirst = FALSE;
            pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION, TRUE, FALSE, FALSE);

            int cursorX = cursor.x;
            int cursorY = cursor.y;
            if (cursor.x == -1 && cursor.y == -1) {
                CPoint& ptPos = pSprite->GetPos();
                cursorX = ptPos.x;
                cursorY = ptPos.y;
            }

            LONG dx = target.x - cursorX;
            LONG dy = target.y - cursorY;
            double dist = sqrt((double)(dx * dx + dy * dy));
            SHORT rotationDegrees = 0;

            if (dist >= 1.0) {
                LONG absDx = (dx ^ (dx >> 31)) - (dx >> 31);
                double baseAngle = acos((double)absDx / dist);

                if (dx < 0) {
                    if (dy < 0) {
                        baseAngle = baseAngle + PI;
                    } else {
                        baseAngle = PI - baseAngle;
                    }
                } else {
                    if (dy < 0) {
                        baseAngle = TWO_PI - baseAngle;
                    }
                }

                SHORT angleDegrees = (SHORT)(baseAngle * -360.0 / TWO_PI);
                rotationDegrees = (SHORT)(90 - angleDegrees) % 360;
            }

            offsets = GetFormationOffsets(formationType, rotationDegrees, 0);
            faces = GetFormationFaces(formationType, rotationDegrees);

            LONG gridTargetX = target.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG gridTargetY = target.y / CPathSearch::GRID_SQUARE_SIZEY;

            CPoint& ptLeaderPos = pSprite->GetPos();
            LONG leaderGridX = ptLeaderPos.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG leaderGridY = ptLeaderPos.y / CPathSearch::GRID_SQUARE_SIZEY;

            if (leaderGridX != gridTargetX || leaderGridY != gridTargetY) {
                // 0x46A3D0
                BYTE personalSpace = pSprite->GetAnimation()->GetPersonalSpace();
                POINT targetGrid = { gridTargetX, gridTargetY };
                BOOL passable = g_pBaldurChitin->GetObjectGame()->GetArea(0)->AdjustTarget(
                    CPoint(leaderGridX, leaderGridY), &targetGrid, personalSpace, 10);
                if (!passable) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);
                    if (offsets != NULL) {
                        delete[] offsets;
                    }
                    if (faces != NULL) {
                        delete[] faces;
                    }
                    return;
                }
                gridTargetX = targetGrid.x;
                gridTargetY = targetGrid.y;
            }

            if (target.x / CPathSearch::GRID_SQUARE_SIZEX == gridTargetX
                && target.y / CPathSearch::GRID_SQUARE_SIZEY == gridTargetY) {
                // Keep world coordinates as-is.
            } else {
                target.x = CPathSearch::GRID_SQUARE_SIZEX * gridTargetX;
                target.y = CPathSearch::GRID_SQUARE_SIZEY * gridTargetY;
            }
        }

        LONG memberOffsetX = (CPathSearch::GRID_SQUARE_SIZEX * offsets[offsetIndex].x) / OFFSET_MULTIPLIER;
        LONG memberOffsetY = (CPathSearch::GRID_SQUARE_SIZEY * offsets[offsetIndex].y) / OFFSET_MULTIPLIER;

        CAIAction* faceAction = new CAIAction(CAIAction::FACE, CPoint(-1, -1), faces[faceIndex], -1);
        actions.AddTail(faceAction);

        CAIAction* moveAction = new CAIAction(CAIAction::MOVETOPOINT, CPoint(-1, -1), 0, -1);
        actions.AddTail(moveAction);

        LONG memberDestGridX = (target.x + memberOffsetX) / CPathSearch::GRID_SQUARE_SIZEX;
        LONG memberDestGridY = (target.y + memberOffsetY) / CPathSearch::GRID_SQUARE_SIZEY;

        CPoint& ptPos = pSprite->GetPos();
        LONG spriteGridX = ptPos.x / CPathSearch::GRID_SQUARE_SIZEX;
        LONG spriteGridY = ptPos.y / CPathSearch::GRID_SQUARE_SIZEY;

        CPoint memberDest;
        if (spriteGridX == memberDestGridX && spriteGridY == memberDestGridY) {
            memberDest.x = memberOffsetX + target.x;
            memberDest.y = memberOffsetY + target.y;
        } else {
            // 0x46A3D0
            BYTE personalSpace = pSprite->GetAnimation()->GetPersonalSpace();
            POINT destGrid = { memberDestGridX, memberDestGridY };
            BOOL passable = g_pBaldurChitin->GetObjectGame()->GetArea(0)->AdjustTarget(
                CPoint(spriteGridX, spriteGridY), &destGrid, personalSpace, 10);
            if (passable) {
                memberDest.x = memberOffsetX + target.x;
                memberDest.y = memberOffsetY + target.y;
            } else {
                memberDest.x = target.x;
                memberDest.y = target.y;
            }
        }

        if (pSprite->m_curAction.m_actionID == 2) {
            pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION, TRUE, FALSE, FALSE);
        } else {
            moveAction->m_dest = memberDest;

            if (g_pBaldurChitin->GetObjectGame()->m_worldTime.m_active) {
                SHORT waitFrames = (SHORT)(rand() % 7);
                CAIAction* waitAction = new CAIAction(CAIAction::SMALLWAIT, CPoint(-1, -1), waitFrames, -1);
                actions.AddTail(waitAction);
            }

            pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
            pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;

            if (additive == 0) {
                pSprite->m_interrupt = TRUE;
                while (actions.GetCount() != 0) {
                    CAIAction* action = actions.RemoveHead();
                    pSprite->AddAction(*action);
                    delete action;
                }
            } else {
                SHORT curActionId = pSprite->m_curAction.m_actionID;
                if (curActionId != CAIAction::MOVETOPOINT
                    && curActionId != CAIAction::FACE
                    && curActionId != CAIAction::SMALLWAIT) {
                    pSprite->m_interrupt = TRUE;
                }
                while (actions.GetCount() != 0) {
                    CAIAction* action = actions.RemoveTail();
                    pSprite->InsertAction(*action);
                    delete action;
                }
            }
        }

        while (actions.GetCount() != 0) {
            CAIAction* action = actions.RemoveTail();
            delete action;
        }
        actions.RemoveAll();

        pSprite->m_inFormation = TRUE;
        pSprite->m_groupMove = FALSE;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        offsetIndex++;
        faceIndex++;
    }

    if (offsets != NULL) {
        delete[] offsets;
    }
    if (faces != NULL) {
        delete[] faces;
    }

    AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);
}

// 0x407280
void CAIGroup::GroupProtectPoint(CPoint target, SHORT formationType, CPoint cursor, LONG range)
{
    CTypedPtrList<CPtrList, CAIAction*> actions;

    LONG absX = (target.x - cursor.x);
    absX = (absX ^ (absX >> 31)) - (absX >> 31);
    LONG absY = (target.y - cursor.y);
    absY = (absY ^ (absY >> 31)) - (absY >> 31);

    if (absX < DEADZONE && absY < DEADZONE * 3 / 4) {
        cursor.x = -1;
        cursor.y = -1;
    }

    BOOL bFirst = TRUE;
    if (m_memberList.GetCount() == 0) {
        return;
    }

    if (m_memberList.GetCount() == 1) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetHead());

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CAIAction* protectAction = new CAIAction(CAIAction::PROTECTPOINT, target, range, -1);
            actions.AddTail(protectAction);

            pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
            pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;
            pSprite->m_interrupt = TRUE;

            while (actions.GetCount() != 0) {
                CAIAction* action = actions.RemoveTail();
                pSprite->AddAction(*action);
                delete action;
            }

            pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION, TRUE, FALSE, FALSE);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        return;
    }

    RemoveFromSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);

    CPoint* offsets = NULL;
    SHORT* faces = NULL;
    int offsetIndex = 0;
    int faceIndex = 0;

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        if (bFirst) {
            bFirst = FALSE;
            pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION, TRUE, FALSE, FALSE);

            int cursorX = cursor.x;
            int cursorY = cursor.y;
            if (cursor.x == -1 && cursor.y == -1) {
                CPoint& ptPos = pSprite->GetPos();
                cursorX = ptPos.x;
                cursorY = ptPos.y;
            }

            LONG dx = target.x - cursorX;
            LONG dy = target.y - cursorY;
            double dist = sqrt((double)(dx * dx + dy * dy));
            SHORT rotationDegrees = 0;

            if (dist >= 1.0) {
                LONG absDx = (dx ^ (dx >> 31)) - (dx >> 31);
                double baseAngle = acos((double)absDx / dist);

                if (dx < 0) {
                    if (dy < 0) {
                        baseAngle = baseAngle + PI;
                    } else {
                        baseAngle = PI - baseAngle;
                    }
                } else {
                    if (dy < 0) {
                        baseAngle = TWO_PI - baseAngle;
                    }
                }

                SHORT angleDegrees = (SHORT)(baseAngle * -360.0 / TWO_PI);
                rotationDegrees = (SHORT)(90 - angleDegrees) % 360;
            }

            offsets = GetFormationOffsets(formationType, rotationDegrees, 0);
            faces = GetFormationFaces(formationType, rotationDegrees);

            LONG gridTargetX = target.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG gridTargetY = target.y / CPathSearch::GRID_SQUARE_SIZEY;

            CPoint& ptLeaderPos = pSprite->GetPos();
            LONG leaderGridX = ptLeaderPos.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG leaderGridY = ptLeaderPos.y / CPathSearch::GRID_SQUARE_SIZEY;

            if (leaderGridX != gridTargetX || leaderGridY != gridTargetY) {
                // 0x46A3D0
                BYTE personalSpace = pSprite->GetAnimation()->GetPersonalSpace();
                POINT targetGrid = { gridTargetX, gridTargetY };
                BOOL passable = g_pBaldurChitin->GetObjectGame()->GetArea(0)->AdjustTarget(
                    CPoint(leaderGridX, leaderGridY), &targetGrid, personalSpace, 10);
                if (!passable) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);
                    if (offsets != NULL) {
                        delete[] offsets;
                    }
                    if (faces != NULL) {
                        delete[] faces;
                    }
                    return;
                }
                gridTargetX = targetGrid.x;
                gridTargetY = targetGrid.y;
            }

            if (target.x / CPathSearch::GRID_SQUARE_SIZEX == gridTargetX
                && target.y / CPathSearch::GRID_SQUARE_SIZEY == gridTargetY) {
                // Keep world coordinates.
            } else {
                target.x = CPathSearch::GRID_SQUARE_SIZEX * gridTargetX;
                target.y = CPathSearch::GRID_SQUARE_SIZEY * gridTargetY;
            }
        }

        LONG memberOffsetX = (CPathSearch::GRID_SQUARE_SIZEX * offsets[offsetIndex].x) / OFFSET_MULTIPLIER;
        LONG memberOffsetY = (CPathSearch::GRID_SQUARE_SIZEY * offsets[offsetIndex].y) / OFFSET_MULTIPLIER;

        CAIAction* faceAction = new CAIAction(CAIAction::FACE, CPoint(-1, -1), faces[faceIndex], -1);
        actions.AddTail(faceAction);

        LONG memberDestGridX = (target.x + memberOffsetX) / CPathSearch::GRID_SQUARE_SIZEX;
        LONG memberDestGridY = (target.y + memberOffsetY) / CPathSearch::GRID_SQUARE_SIZEY;

        CPoint& ptPos = pSprite->GetPos();
        LONG spriteGridX = ptPos.x / CPathSearch::GRID_SQUARE_SIZEX;
        LONG spriteGridY = ptPos.y / CPathSearch::GRID_SQUARE_SIZEY;

        CPoint memberDest;
        if (spriteGridX == memberDestGridX && spriteGridY == memberDestGridY) {
            memberDest.x = memberOffsetX + target.x;
            memberDest.y = memberOffsetY + target.y;
        } else {
            // 0x46A3D0
            BYTE personalSpace = pSprite->GetAnimation()->GetPersonalSpace();
            POINT destGrid = { memberDestGridX, memberDestGridY };
            BOOL passable = g_pBaldurChitin->GetObjectGame()->GetArea(0)->AdjustTarget(
                CPoint(spriteGridX, spriteGridY), &destGrid, personalSpace, 10);
            if (passable) {
                memberDest.x = memberOffsetX + target.x;
                memberDest.y = memberOffsetY + target.y;
            } else {
                memberDest.x = target.x;
                memberDest.y = target.y;
            }
        }

        CAIAction* protectAction = new CAIAction(CAIAction::PROTECTPOINT, memberDest, range, -1);
        actions.AddTail(protectAction);

        // NOTE: Original adds SMALLWAIT(rand()%7) stagger when
        // multiplayer flag at CInfGame+0x1B7C is set.
        // TODO: Add back when multiplayer flag is recovered.

        pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
        pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;
        pSprite->m_interrupt = TRUE;

        while (actions.GetCount() != 0) {
            CAIAction* action = actions.RemoveTail();
            pSprite->AddAction(*action);
            delete action;
        }
        actions.RemoveAll();

        pSprite->m_inFormation = TRUE;
        pSprite->m_groupMove = FALSE;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        offsetIndex++;
        faceIndex++;
    }

    if (offsets != NULL) {
        delete[] offsets;
    }
    if (faces != NULL) {
        delete[] faces;
    }

    AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);
}

// 0x407FC0
void CAIGroup::GroupDrawMove(CPoint target, SHORT formationType, CPoint cursor)
{
    if (formationType == 0) {
        if (m_memberList.GetCount() != 0) {
            LONG leaderId = reinterpret_cast<LONG>(m_memberList.GetHead());

            CGameSprite* pLeader;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(leaderId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pLeader),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                pLeader->m_targetPoint.x = target.x;
                pLeader->m_targetPoint.y = target.y;
                pLeader->m_groupMove = TRUE;

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(leaderId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        return;
    }

    LONG absX = (target.x - cursor.x);
    absX = (absX ^ (absX >> 31)) - (absX >> 31);
    LONG absY = (target.y - cursor.y);
    absY = (absY ^ (absY >> 31)) - (absY >> 31);

    if (absX < DEADZONE && absY < DEADZONE * 3 / 4) {
        cursor.x = -1;
        cursor.y = -1;
    }

    BOOL bFirst = TRUE;
    if (m_memberList.GetCount() == 0) {
        return;
    }

    RemoveFromSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);

    if (m_memberList.GetHeadPosition() == NULL) {
        AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);
        return;
    }

    int offsetIndex = 0;
    CPoint* offsets = NULL;

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        if (bFirst) {
            CGameSprite* pSprite;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(memberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                bFirst = FALSE;
                int cursorX = cursor.x;
                int cursorY = cursor.y;
                if (cursor.x == -1 && cursor.y == -1) {
                    CPoint& ptPos = pSprite->GetPos();
                    cursorX = ptPos.x;
                    cursorY = ptPos.y;
                }

                LONG dx = target.x - cursorX;
                LONG dy = target.y - cursorY;
                double dist = sqrt((double)(dx * dx + dy * dy));
                SHORT rotationDegrees = 0;

                if (dist >= 1.0) {
                    LONG absDx = (dx ^ (dx >> 31)) - (dx >> 31);
                    double baseAngle = acos((double)absDx / dist);

                    if (dx < 0) {
                        if (dy < 0) {
                            baseAngle = baseAngle + PI;
                        } else {
                            baseAngle = PI - baseAngle;
                        }
                    } else {
                        if (dy < 0) {
                            baseAngle = TWO_PI - baseAngle;
                        }
                    }

                    SHORT angleDegrees = (SHORT)(baseAngle * -360.0 / TWO_PI);
                    rotationDegrees = (SHORT)(90 - angleDegrees) % 360;
                }

                offsets = GetFormationOffsets(formationType, rotationDegrees, 0);

                LONG gridTargetX = target.x / CPathSearch::GRID_SQUARE_SIZEX;
                LONG gridTargetY = target.y / CPathSearch::GRID_SQUARE_SIZEY;

                CPoint& ptLeaderPos = pSprite->GetPos();
                LONG leaderGridX = ptLeaderPos.x / CPathSearch::GRID_SQUARE_SIZEX;
                LONG leaderGridY = ptLeaderPos.y / CPathSearch::GRID_SQUARE_SIZEY;

                if (leaderGridX != gridTargetX || leaderGridY != gridTargetY) {
                    // 0x46A3D0
                    BYTE personalSpace = pSprite->GetAnimation()->GetPersonalSpace();
                    POINT targetGrid = { gridTargetX, gridTargetY };
                    BOOL passable = g_pBaldurChitin->GetObjectGame()->GetArea(0)->AdjustTarget(
                        CPoint(leaderGridX, leaderGridY), &targetGrid, personalSpace, 10);
                    if (!passable) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(memberId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);
                        if (offsets != NULL) {
                            delete[] offsets;
                        }
                        return;
                    }
                    gridTargetX = targetGrid.x;
                    gridTargetY = targetGrid.y;
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(memberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                if (target.x / CPathSearch::GRID_SQUARE_SIZEX == gridTargetX
                    && target.y / CPathSearch::GRID_SQUARE_SIZEY == gridTargetY) {
                    // Keep world coordinates as-is.
                } else {
                    target.x = CPathSearch::GRID_SQUARE_SIZEX * gridTargetX;
                    target.y = CPathSearch::GRID_SQUARE_SIZEY * gridTargetY;
                }
            }
        }

        if (offsets == NULL) {
            continue;
        }

        LONG memberOffsetX = (CPathSearch::GRID_SQUARE_SIZEX * offsets[offsetIndex].x) / OFFSET_MULTIPLIER;
        LONG memberOffsetY = (CPathSearch::GRID_SQUARE_SIZEY * offsets[offsetIndex].y) / OFFSET_MULTIPLIER;

        LONG memberDestGridX = (target.x + memberOffsetX) / CPathSearch::GRID_SQUARE_SIZEX;
        LONG memberDestGridY = (target.y + memberOffsetY) / CPathSearch::GRID_SQUARE_SIZEY;

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CPoint& ptPos = pSprite->GetPos();
            LONG spriteGridX = ptPos.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG spriteGridY = ptPos.y / CPathSearch::GRID_SQUARE_SIZEY;

            if (spriteGridX == memberDestGridX && spriteGridY == memberDestGridY) {
                pSprite->m_targetPoint.x = target.x + memberOffsetX;
                pSprite->m_targetPoint.y = target.y + memberOffsetY;
            } else {
                // 0x46A3D0
                BYTE personalSpace = pSprite->GetAnimation()->GetPersonalSpace();
                POINT destGrid = { memberDestGridX, memberDestGridY };
                BOOL passable = g_pBaldurChitin->GetObjectGame()->GetArea(0)->AdjustTarget(
                    CPoint(spriteGridX, spriteGridY), &destGrid, personalSpace, 10);
                if (passable) {
                    pSprite->m_targetPoint.x = target.x + memberOffsetX;
                    pSprite->m_targetPoint.y = target.y + memberOffsetY;
                } else {
                    pSprite->m_targetPoint.x = target.x;
                    pSprite->m_targetPoint.y = target.y;
                }
            }

            pSprite->m_groupMove = TRUE;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            offsetIndex++;
        }
    }

    AddToSearch(&g_pBaldurChitin->GetObjectGame()->GetArea(0)->m_search);

    if (offsets != NULL) {
        delete[] offsets;
    }
}

// 0x408660
void CAIGroup::GroupCancelMove()
{
    if (m_memberList.GetCount() == 0) {
        return;
    }

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->m_targetPoint.x = -1;
            pSprite->m_targetPoint.y = -1;
            pSprite->m_groupMove = FALSE;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x408720
void CAIGroup::ClearActions()
{
    if (m_memberList.IsEmpty()) {
        return;
    }

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->DropPath();

            CMessageStopActions* pMessage = new CMessageStopActions(memberId,
                memberId,
                pSprite);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4088C0
void CAIGroup::GroupSetTarget(LONG iObject)
{
    if (m_memberList.IsEmpty()) {
        return;
    }

    CAIObjectType target(0, 0, 0, 0, 0, 0, 0, 0, iObject, 0, 0);
    CAIObjectType actee(0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0);

    if (m_memberList.GetCount() > 1) {
        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(iObject,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            actee.Set(pSprite->GetAIType());
            actee.m_nInstance = -1;
            actee.m_sName = CString("");

            GroupAction(CAIAction(CAIAction::GROUPATTACK, actee, 0, 0, 0),
                TRUE,
                NULL);

            // FIXME: Both params immediately destroyed after call which means
            // they are constructed just for the call which leads to pretty
            // ugly invocation.
            GroupAction(CAIAction(CAIAction::ATTACK, target, 0, 0, 0),
                TRUE,
                &CAIAction(CAIAction::ATTACK, target, 0, 0, 0));

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(iObject,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else {
        GroupAction(CAIAction(CAIAction::ATTACK, target, 0, 0, 0),
            TRUE,
            NULL);
    }
}

// 0x4091B0
void CAIGroup::AddTrigger(CAITrigger& trigger)
{
    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
                memberId,
                memberId);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x4093E0
BOOL CAIGroup::InList(LONG characterId)
{
    return m_memberList.Find(reinterpret_cast<LONG*>(characterId)) != NULL;
}

// 0x409400
void CAIGroup::FollowLeader(CPoint target, BOOL additive)
{
    CTypedPtrList<CPtrList, CAIAction*> actions;

    if (m_memberList.IsEmpty()) {
        return;
    }

    if (m_memberList.GetHeadPosition() != NULL) {
        CAIAction* action = new CAIAction(CAIAction::LEADER, target, additive, -1);
        actions.AddTail(action);

        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetHead());

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->PlaySoundA(CGameSprite::SOUND_SELECT_ACTION,
                TRUE,
                FALSE,
                FALSE);

            if (additive) {
                pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;

                if (pSprite->m_curAction.m_actionID != 23
                    && pSprite->m_curAction.m_actionID != 84
                    && pSprite->m_curAction.m_actionID != 83
                    && pSprite->m_curAction.m_actionID != 88) {
                    pSprite->m_interrupt = TRUE;
                }

                while (actions.GetCount() != 0) {
                    CAIAction* action = actions.RemoveTail();
                    pSprite->InsertAction(*action);
                    delete action;
                }
            } else {
                pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;
                pSprite->m_interrupt = TRUE;

                while (actions.GetCount() != 0) {
                    CAIAction* action = actions.RemoveTail();
                    pSprite->AddAction(*action);
                    delete action;
                }
            }

            actions.RemoveAll();

            pSprite->m_inFormation = TRUE;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            POSITION pos = m_memberList.GetHeadPosition();
            while (pos != NULL) {
                LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

                CMessageStopFollow* pMessage = new CMessageStopFollow(memberId, memberId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
    }
}

// 0x409820
void CAIGroup::HandleFollowPath(LONG* pPath, INT nPath, CPoint target, CPoint start, BOOL additive)
{
    CTypedPtrList<CPtrList, CAIAction*> actions;

    BOOL bFirst = TRUE;
    int delay = 0;

    POSITION pos = m_memberList.GetHeadPosition();
    while (pos != NULL) {
        POSITION curPos = pos;
        LONG memberId = reinterpret_cast<LONG>(m_memberList.GetNext(pos));

        CGameSprite* pSprite;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(memberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        if (bFirst) {
            bFirst = FALSE;

            pSprite->GetPos();

            if (pos != NULL) {
                LONG nextMemberId = reinterpret_cast<LONG>(m_memberList.GetAt(pos));

                CMessageStopFollow* pMessage = new CMessageStopFollow(memberId, nextMemberId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        } else {
            CAIAction* followAction = new CAIAction(CAIAction::FOLLOW, target, (SHORT)delay, -1);
            followAction->m_specificID2 = start.y;
            actions.AddTail(followAction);

            if (additive == 0) {
                CAIAction* waitAction = new CAIAction(CAIAction::SMALLWAIT, CPoint(-1, -1), 5, -1);
                actions.AddTail(waitAction);

                CAIAction* moveAction = new CAIAction(CAIAction::MOVETOPOINTNORECTICLE, CPoint(-1, -1), 0, -1);
                actions.AddTail(moveAction);

                if (pos != NULL) {
                    POSITION nextPos = pos;
                    LONG nextMemberId = reinterpret_cast<LONG>(m_memberList.GetAt(nextPos));
                    pSprite->m_followLeaderNext = nextMemberId;
                }

                pSprite->ClearStoredPaths();

                pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;
                pSprite->m_interrupt = TRUE;

                while (actions.GetCount() != 0) {
                    CAIAction* action = actions.RemoveTail();
                    pSprite->AddAction(*action);
                    delete action;
                }
                actions.RemoveAll();

                LONG* pathCopy = new LONG[nPath];
                memcpy(pathCopy, pPath, nPath * sizeof(LONG));

                pSprite->m_pPathTemp.AddTail(reinterpret_cast<int*>(pathCopy));
                pSprite->m_nPathTemp.AddTail(reinterpret_cast<int*>(nPath));
            } else {
                pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;

                SHORT curActionId = pSprite->m_curAction.m_actionID;
                if (curActionId != CAIAction::MOVETOPOINT
                    && curActionId != CAIAction::FACE
                    && curActionId != CAIAction::SMALLWAIT
                    && curActionId != CAIAction::FOLLOW
                    && curActionId != CAIAction::MOVETOPOINTNORECTICLE) {

                    CAIAction* waitAction = new CAIAction(CAIAction::SMALLWAIT, CPoint(-1, -1), 5, -1);
                    actions.AddTail(waitAction);

                    CAIAction* moveAction = new CAIAction(CAIAction::MOVETOPOINTNORECTICLE, CPoint(-1, -1), 0, -1);
                    actions.AddTail(moveAction);

                    if (pos != NULL) {
                        LONG nextMemberId = reinterpret_cast<LONG>(m_memberList.GetAt(pos));
                        pSprite->m_followLeaderNext = nextMemberId;
                    }

                    pSprite->ClearStoredPaths();
                    pSprite->m_interrupt = TRUE;
                }

                while (actions.GetCount() != 0) {
                    CAIAction* action = actions.RemoveTail();
                    pSprite->InsertAction(*action);
                    delete action;
                }
                actions.RemoveAll();

                LONG* pathCopy = new LONG[nPath];
                memcpy(pathCopy, pPath, nPath * sizeof(LONG));

                pSprite->m_pPathTemp.AddTail(reinterpret_cast<int*>(pathCopy));
                pSprite->m_nPathTemp.AddTail(reinterpret_cast<int*>(nPath));
            }
        }

        pSprite->m_inFormation = TRUE;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(memberId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        delay++;
    }
}

// 0x68C000
BYTE CAIGroup::GetCount()
{
    return static_cast<BYTE>(m_memberList.GetCount());
}
