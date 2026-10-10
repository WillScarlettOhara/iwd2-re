#include "CGameAIBase.h"

#include "CAIConditionResponse.h"
#include "CAIResponse.h"
#include "CAIUtil.h"
#include "CAIScript.h"
#include "CAITrigger.h"
#include "CBaldurChitin.h"
#include "CBaldurProjector.h"
#include "CGameArea.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameEffect.h"
#include "CGameJournal.h"
#include "CGameSpawning.h"
#include "CGameSprite.h"
#include "CGameStatic.h"
#include "CGameTiledObject.h"
#include "CGameTimer.h"
#include "CGameTrigger.h"
#include "CInfGame.h"
#include "CPathSearch.h"
#include "CProjectile.h"
#include "CScreenCharacter.h"
#include "CScreenChapter.h"
#include "CScreenWorld.h"
#include "CSpell.h"
#include "CTimerWorld.h"
#include "CUtil.h"
#include "CVariableHash.h"
#include "FileFormat.h"

// 0x8485C4
const SHORT CGameAIBase::ACTION_DONE = -1;

// 0x8485C6
const SHORT CGameAIBase::ACTION_INTERRUPTABLE = 1;

// 0x8485C8
const SHORT CGameAIBase::ACTION_NORMAL = 0;

// 0x8485CA
const SHORT CGameAIBase::ACTION_ERROR = -2;

// 0x8485CC
const SHORT CGameAIBase::ACTION_NO_ACTION = 2;

// 0x8485CE
const SHORT CGameAIBase::ACTION_STOPPED = -3;

// 0x8485E8
const BYTE CGameAIBase::EFFECT_LIST_TIMED = 1;

// 0x8485E9
const BYTE CGameAIBase::EFFECT_LIST_EQUIPED = 2;

// 0x8D1408
const CString CGameAIBase::DEAD_GLOBAL_PREFIX("_DEAD");

// 0x8D1810
CAIAction CGameAIBase::m_aiAction;

static void SplitScriptVariableName(const CString& sCombined, CString& sScope, CString& sName)
{
    // Ghidra 0x453840/0x45EDE0: script parser stores scope+name in string1.
    // The binary treats the first six chars as scope (GLOBAL/LOCALS/MYAREA/area resref).
    sScope = sCombined.Left(6);
    int nNameLength = sCombined.GetLength() - 6;
    sName = nNameLength > 0 ? sCombined.Right(nNameLength) : CString("");
}

// 0x44C4B0
CGameAIBase::CGameAIBase()
{
    m_nLastActionReturn = 0;
    m_objectType = CGameObject::TYPE_AIBASE;
    m_lAttacker.Set(CAIObjectType::NOONE);
    m_lOrderedBy.Set(CAIObjectType::NOONE);
    field_EA.Set(CAIObjectType::NOONE);
    field_126.Set(CAIObjectType::NOONE);
    field_162.Set(CAIObjectType::NOONE);
    m_lHitter.Set(CAIObjectType::NOONE);
    m_lHelp.Set(CAIObjectType::NOONE);
    m_lTrigger.Set(CAIObjectType::NOONE);
    m_lSeen.Set(CAIObjectType::NOONE);
    m_lTalkedTo.Set(CAIObjectType::NOONE);
    m_lHeard.Set(CAIObjectType::NOONE);
    field_306.Set(CAIObjectType::NOONE);
    field_342.Set(CAIObjectType::NOONE);
    field_37E.Set(CAIObjectType::NOONE);
    field_3BA.Set(CAIObjectType::NOONE);
    m_curResponseNum = -1;
    m_curResponseSetNum = -1;
    m_curScriptNum = -1;
    m_curAction = CAIAction::NULL_ACTION;
    m_interrupt = FALSE;
    m_actionCount = 0;
    field_54C = 0;
    field_550 = 0;
    field_552 = 0;
    field_44A = 0;
    m_overrideScript = NULL;
    m_special1Script = NULL;
    m_teamScript = NULL;
    m_special2Script = NULL;
    m_combatScript = NULL;
    m_special3Script = NULL;
    m_movementScript = NULL;
    m_inCutScene = FALSE;
    m_reactionRoll = 10;
    field_550 = rand() % 120;

    CAITrigger trigger(CAITrigger::ONCREATION, 0);
    m_pendingTriggers.AddTail(new CAITrigger(trigger));

    m_firstCall = TRUE;
    m_forceActionPick = FALSE;
    field_580 = 0;
    m_nLastActionReturn = -1;
    field_588 = 0;
    field_58C = 0;
    field_594 = 0;
    field_595 = 1;
    field_596 = 0;
    m_randValue = rand() & 0x7FFF;
}

// 0x44D160
CGameAIBase::~CGameAIBase()
{
    POSITION pos;

    pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if (pTrigger != NULL) {
            delete pTrigger;
        }
    }

    pos = m_queuedActions.GetHeadPosition();
    while (pos != NULL) {
        CAIAction* pAction = m_queuedActions.GetNext(pos);
        if (pAction != NULL) {
            delete pAction;
        }
    }

    pos = m_timers.GetHeadPosition();
    while (pos != NULL) {
        CGameTimer* pTimer = m_timers.GetNext(pos);
        if (pTimer != NULL) {
            delete pTimer;
        }
    }

    if (m_overrideScript != NULL) {
        delete m_overrideScript;
        m_overrideScript = NULL;
    }

    if (m_special1Script != NULL) {
        delete m_special1Script;
        m_special1Script = NULL;
    }

    if (m_teamScript != NULL) {
        delete m_teamScript;
        m_teamScript = NULL;
    }

    if (m_special2Script != NULL) {
        delete m_special2Script;
        m_special2Script = NULL;
    }

    if (m_combatScript != NULL) {
        delete m_combatScript;
        m_combatScript = NULL;
    }

    if (m_special3Script != NULL) {
        delete m_special3Script;
        m_special3Script = NULL;
    }

    if (m_movementScript != NULL) {
        delete m_movementScript;
        m_movementScript = NULL;
    }
}

// 0x44DAC0
// #guess name: same-area, target-state, search-grid range and LOS query.
BOOL CGameAIBase::CheckTargetVisibility(CGameObject* pTarget, BOOL bAllowDead)
{
    if (pTarget == NULL || pTarget->m_pArea != m_pArea) {
        return FALSE;
    }
    if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* sprite = static_cast<CGameSprite*>(pTarget);
        if (sprite->m_baseStats.m_bStealthMode != FALSE
            || (!bAllowDead && (sprite->m_derivedStats.m_generalState & STATE_DEAD) != 0)) {
            return FALSE;
        }
    }
    CPoint target = pTarget->GetPos();
    CPoint source = GetPos();
    INT dx = source.x / CPathSearch::GRID_SQUARE_SIZEX - target.x / CPathSearch::GRID_SQUARE_SIZEX;
    INT dy = source.y / CPathSearch::GRID_SQUARE_SIZEY - target.y / CPathSearch::GRID_SQUARE_SIZEY;
    BOOLEAN explored = FALSE;
    if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
        explored = static_cast<CGameSprite*>(pTarget)->Orderable(FALSE);
    }
    SHORT range = GetVisualRange();
    SHORT rangeAgain = GetVisualRange();
    if (dx * dx + dy * dy > range * rangeAgain) {
        return FALSE;
    }
    const BYTE* terrain = GetVisibleTerrainTable();
    source = GetPos();
    target = pTarget->GetPos();
    return m_pArea->CheckLOS(target, source, terrain, explored) != FALSE;
}

// 0x44D100
const BYTE* CGameAIBase::GetVisibleTerrainTable()
{
    return CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE;
}

// 0x44D110
const BYTE* CGameAIBase::GetTerrainTable()
{
    return CGameObject::DEFAULT_TERRAIN_TABLE;
}

// 0x44D120
BOOLEAN CGameAIBase::CompressTime(DWORD deltaTime)
{
    CheckTimers(deltaTime / CTimerWorld::TIMESCALE_MSEC_PER_SEC);
    return TRUE;
}

// 0x453840
BOOL CGameAIBase::EvaluateStatusTrigger(const CAITrigger& trigger)
{
    switch (trigger.m_triggerID) {
    case CAITRIGGER_TRUE:
        return TRUE;

    case CAITRIGGER_FALSE:
        return FALSE;

    case CAITRIGGER_ALLEGIANCE: {
        CAITrigger cause(trigger);
        cause.Decode(this);
        return cause.GetCause().GetEnemyAlly() == cause.GetSpecifics();
    }

    case CAITRIGGER_ALIGNMENT:
    case CAITRIGGER_GENERAL:
    case CAITRIGGER_CLASS:
    case CAITRIGGER_RACE:
    case CAITRIGGER_SPECIFICS:
    case CAITRIGGER_GENDER: {
        CAITrigger cause(trigger);
        cause.Decode(this);
        const CAIObjectType& type = cause.GetCause();
        LONG value = cause.GetSpecifics();
        switch (cause.m_triggerID) {
        case CAITRIGGER_ALIGNMENT:
            switch (value) {
            case 1:
            case 2:
            case 3:
            case 0x10:
            case 0x20:
            case 0x30:
                return (type.GetAlignment() & value) != 0;
            default:
                return type.GetAlignment() == value;
            }
        case CAITRIGGER_GENERAL:
            return type.GetGeneral() == value;
        case CAITRIGGER_CLASS:
            return type.IsClassValid(static_cast<BYTE>(value));
        case CAITRIGGER_RACE:
            return type.GetRace() == value;
        case CAITRIGGER_SPECIFICS:
            return type.GetSpecific() == value;
        default:
            return type.GetGender() == value;
        }
    }

    case CAITRIGGER_SETLASTMARKEDOBJECT:
    case CAITRIGGER_SETSPELLTARGET: {
        CAITrigger cause(trigger);
        cause.Decode(this);
        CGameObject* pObject = cause.GetCause().GetObject(this, FALSE);
        const CAIObjectType& type = pObject != NULL ? pObject->GetAIType() : CAIObjectType::NOONE;
        if (cause.m_triggerID == CAITRIGGER_SETSPELLTARGET) {
            SetAIType3BA(type);
        } else {
            SetAIType342(type);
        }
        if (pObject != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        return TRUE;
    }

    case CAITRIGGER_HP:
    case CAITRIGGER_HPGT:
    case CAITRIGGER_HPLT:
    case CAITRIGGER_HPPERCENT:
    case CAITRIGGER_HPPERCENTGT:
    case CAITRIGGER_HPPERCENTLT:
    case CAITRIGGER_HPLOST:
    case CAITRIGGER_HPLOSTGT:
    case CAITRIGGER_HPLOSTLT:
    case CAITRIGGER_STATECHECK:
    case CAITRIGGER_NOTSTATECHECK:
    case CAITRIGGER_ISCREATUREAREAFLAG:
    case CAITRIGGER_CHECKSTAT:
    case CAITRIGGER_CHECKSTATGT:
    case CAITRIGGER_CHECKSTATLT: {
        CAITrigger cause(trigger);
        CGameSprite* pSprite = NULL;
        BOOL bQuick = QuickDecode(cause, pSprite);
        if (pSprite == NULL) {
            return FALSE;
        }
        LONG value;
        BOOL bHolds;
        switch (cause.m_triggerID) {
        case CAITRIGGER_STATECHECK:
            bHolds = (pSprite->GetDerivedStats()->m_generalState & cause.GetSpecifics()) != 0;
            break;
        case CAITRIGGER_NOTSTATECHECK:
            bHolds = (pSprite->GetDerivedStats()->m_generalState & cause.GetSpecifics()) == 0;
            break;
        case CAITRIGGER_ISCREATUREAREAFLAG:
            bHolds = (pSprite->GetBaseStats()->m_flags & cause.GetSpecifics()) != 0;
            break;
        default:
            switch (cause.m_triggerID) {
            case CAITRIGGER_CHECKSTAT:
            case CAITRIGGER_CHECKSTATGT:
            case CAITRIGGER_CHECKSTATLT:
                value = pSprite->GetDerivedStats()->GetAtOffset(static_cast<SHORT>(cause.GetInt1()));
                break;
            default:
                value = pSprite->GetBaseStats()->m_hitPoints;
                switch (cause.m_triggerID) {
                case CAITRIGGER_HPPERCENT:
                case CAITRIGGER_HPPERCENTGT:
                case CAITRIGGER_HPPERCENTLT:
                    value = value * 100 / pSprite->GetDerivedStats()->m_nMaxHitPoints;
                    break;
                case CAITRIGGER_HPLOST:
                case CAITRIGGER_HPLOSTGT:
                case CAITRIGGER_HPLOSTLT:
                    value = pSprite->GetDerivedStats()->m_nMaxHitPoints - value;
                    break;
                }
                break;
            }
            switch (cause.m_triggerID) {
            case CAITRIGGER_HPGT:
            case CAITRIGGER_HPPERCENTGT:
            case CAITRIGGER_HPLOSTGT:
            case CAITRIGGER_CHECKSTATGT:
                bHolds = value > cause.GetSpecifics();
                break;
            case CAITRIGGER_HPLT:
            case CAITRIGGER_HPPERCENTLT:
            case CAITRIGGER_HPLOSTLT:
            case CAITRIGGER_CHECKSTATLT:
                bHolds = value < cause.GetSpecifics();
                break;
            default:
                bHolds = value == cause.GetSpecifics();
                break;
            }
            break;
        }
        if (!bQuick) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pSprite->GetId(),
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        return bHolds;
    }

    case CAITRIGGER_EXISTS: {
        CAITrigger cause(trigger);
        cause.Decode(this);
        CGameObject* pObject = cause.GetCause().GetObjectFromId(this, TRUE);
        if (pObject == NULL) {
            return FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return TRUE;
    }

    case CAITRIGGER_GLOBAL:
    case CAITRIGGER_GLOBALGT:
    case CAITRIGGER_GLOBALLT: {
        CString sScope;
        CString sName;
        SplitScriptVariableName(trigger.GetString1(), sScope, sName);
        LONG nTriggerValue = trigger.GetSpecifics();
        CVariableHash* pHash = NULL;

        if (sScope == CString("GLOBAL")) {
            pHash = g_pBaldurChitin->GetObjectGame()->GetVariables();
        } else if (sScope == CString("LOCALS")) {
            if ((GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                pHash = static_cast<CGameSprite*>(this)->GetLocalVariables();
            }
        } else {
            CString sAreaName = sScope;
            if (sScope == CString("MYAREA") && m_pArea != NULL) {
                sAreaName = CResRef(reinterpret_cast<BYTE*>(m_pArea->m_header.m_areaName)).GetResRefStr();
            }
            CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sAreaName);
            if (pArea != NULL) {
                pHash = pArea->GetVariables();
            }
        }

        CVariable* pVar = pHash != NULL ? pHash->FindKey(sName) : NULL;
        LONG nValue = pVar != NULL ? pVar->m_intValue : 0;

        if (trigger.m_triggerID == CAITRIGGER_GLOBAL) {
            return nValue == nTriggerValue;
        }
        if (trigger.m_triggerID == CAITRIGGER_GLOBALGT) {
            return nValue > nTriggerValue;
        }
        return nValue < nTriggerValue;
    }

    case CAITRIGGER_ENTIREPARTYONMAP: {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        SHORT nCharacters = pGame->GetNumCharacters();
        if (nCharacters <= 0) {
            return FALSE;
        }

        for (SHORT nPortrait = 0; nPortrait < nCharacters; ++nPortrait) {
            LONG nCharacterId = pGame->GetCharacterId(nPortrait);
            CGameSprite* pSprite = NULL;
            BYTE rc = pGame->GetObjectArray()->GetShare(
                nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS || pSprite == NULL) {
                return FALSE;
            }

            BOOL bDead = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) != 0;
            BOOL bOnMap = bDead || pSprite->GetArea() == m_pArea;
            pGame->GetObjectArray()->ReleaseShare(
                nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            if (!bOnMap) {
                return FALSE;
            }
        }

        return TRUE;
    }

    case CAITRIGGER_INCUTSCENEMODE:
        return g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 322;

    case CAITRIGGER_ACTIONLISTEMPTY:
        // 0x455356: idle check used by the waypoint-patrol scripts (10ELDGUM
        // and friends) to wait for the previous MoveToPoint to finish.
        return m_queuedActions.GetCount() == 0
            && m_curAction.GetActionID() == CAIAction::NO_ACTION;

    case CAITRIGGER_NEARLOCATION: {
        // 0x458501: NearLocation(O:Object*,I:PointX,I:PointY,I:Range) --
        // squared search-grid distance of the object to (x,y) vs Range^2.
        // x/y of -1 resolve to the caller's position, -2 to its saved location.
        CAITrigger cause(trigger);
        cause.Decode(this);

        CGameObject* pObject = cause.GetCause().GetObjectWithType(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        LONG x = cause.GetSpecifics();
        LONG y = cause.GetInt1();
        if (x == -1) {
            x = GetPos().x;
        } else if (x == -2 && (GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
            x = static_cast<CGameSprite*>(this)->GetBaseStats()->m_savedLocationX;
        }
        if (y == -1) {
            y = GetPos().y;
        } else if (y == -2 && (GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
            y = static_cast<CGameSprite*>(this)->GetBaseStats()->m_savedLocationY;
        }

        LONG nRange = cause.GetInt2();
        BOOL bHolds = FALSE;
        if (x > 0 && y > 0 && nRange > 0) {
            CPoint posObject = pObject->GetPos();
            LONG dx = (posObject.x - x) / CPathSearch::GRID_SQUARE_SIZEX;
            LONG dy = (posObject.y - y) / CPathSearch::GRID_SQUARE_SIZEY;
            bHolds = dx * dx + dy * dy <= nRange * nRange;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return bHolds;
    }

    case CAITRIGGER_RANGE: {
        // 0x454851: Range(O:Object*,I:Range*,I:diffmode) -- squared search-grid
        // distance to the object vs (range+1)^2, compared per diffmode.
        CAITrigger cause(trigger);
        cause.Decode(this);

        CGameObject* pObject = cause.GetCause().GetObjectWithType(this, CGameObject::TYPE_AIBASE, FALSE);
        if (pObject == NULL) {
            return FALSE;
        }

        CPoint ptTarget;
        if (pObject->GetObjectType() == CGameObject::TYPE_DOOR) {
            // The PE passes this default-constructed point before writing it.
            ptTarget = static_cast<CGameDoor*>(pObject)->GetMoveDest(ptTarget);
        } else {
            ptTarget = pObject->GetPos();
        }
        CPoint gridTarget(ptTarget.x / CPathSearch::GRID_SQUARE_SIZEX,
            ptTarget.y / CPathSearch::GRID_SQUARE_SIZEY);

        CPoint ptSelf = GetPos();
        CPoint gridSelf(ptSelf.x / CPathSearch::GRID_SQUARE_SIZEX,
            ptSelf.y / CPathSearch::GRID_SQUARE_SIZEY);

        LONG nDistSq = (gridTarget.x - gridSelf.x) * (gridTarget.x - gridSelf.x)
            + (gridTarget.y - gridSelf.y) * (gridTarget.y - gridSelf.y);
        LONG nRangeSq = (cause.GetSpecifics() + 1) * (cause.GetSpecifics() + 1);

        BOOL bHolds;
        switch (cause.GetInt1()) {
        case 1: // EQUAL
            bHolds = nDistSq == nRangeSq;
            break;
        case 2: // LESS_THAN
            bHolds = nDistSq < nRangeSq;
            break;
        case 3: // GREATER_THAN
            bHolds = nRangeSq < nDistSq;
            break;
        default:
            bHolds = FALSE;
            break;
        }

        if (bHolds) {
            field_342.Set(pObject->GetAIType());
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pObject->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        return bHolds;
    }

    // 0x455E40: IsTeamBitOn(I:TeamFlag*TeamBit) -- mask test against the
    // team-allegiance bits written by SetTeamBit (0x729A3C).
    case CAITRIGGER_ISTEAMBITON:
        return trigger.GetSpecifics() & GetAICounter58C();

    default:
        return CGameObject::EvaluateStatusTrigger(trigger);
    }
}

// 0x44D4B0
void CGameAIBase::CheckTimers(LONG cycles)
{
    POSITION pos = m_timers.GetHeadPosition();
    while (pos != NULL) {
        POSITION oldPos = pos;
        CGameTimer* pTimer = m_timers.GetNext(pos);
        pTimer->m_time -= cycles;
        if (pTimer->m_time <= 0) {
            CAITrigger trigger(CAITrigger::TIMEREXPIRED, pTimer->m_id);

            // NOTE: Uninline.
            SetTrigger(trigger);

            m_timers.RemoveAt(oldPos);
            delete pTimer;
        }
    }
}

// 0x44D640
void CGameAIBase::ClearActions(BOOL a1)
{
    if (a1) {
        POSITION pos = m_queuedActions.GetHeadPosition();
        while (pos != NULL) {
            POSITION curr = pos;
            CAIAction* pAction = m_queuedActions.GetNext(pos);
            if (pAction != NULL) {
                if (pAction->m_actionID != CAIAction::SPELL
                    && pAction->m_actionID != CAIAction::SPELLPOINT
                    && pAction->m_actionID != CAIAction::FORCESPELL
                    && pAction->m_actionID != CAIAction::FORCESPELLPOINT
                    && pAction->m_actionID != CAIAction::APPLYSPELL
                    && pAction->m_actionID != CAIAction::REALLYFORCESPELL
                    && pAction->m_actionID != CAIAction::SPELLNODEC
                    && pAction->m_actionID != CAIAction::SPELLPOINTNODEC) {
                    m_queuedActions.RemoveAt(curr);
                    delete pAction;
                }
            }
        }
    } else {
        POSITION pos = m_queuedActions.GetHeadPosition();
        while (pos != NULL) {
            CAIAction* pAction = m_queuedActions.GetNext(pos);
            if (pAction != NULL) {
                delete pAction;
            }
        }
        m_queuedActions.RemoveAll();
    }

    if (m_queuedActions.GetCount() == 0) {
        m_curResponseNum = -1;
        m_curResponseSetNum = -1;
        m_curScriptNum = -1;
    }
}

// 0x799E60
// No-op base (3-byte stub in the binary); CGameSprite overrides this at 0x733660
// to update the current target id + target marker.
void CGameAIBase::UpdateTarget(CGameObject* pObject)
{
}

// 0x44D730
void CGameAIBase::ClearTriggers()
{
    ApplyTriggers();

    POSITION pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if (pTrigger != NULL) {
            delete pTrigger;
        }
    }
    m_pendingTriggers.RemoveAll();
}

// 0x44D780
void CGameAIBase::DoAction()
{
    SHORT actionReturn = ExecuteAction();

    if (actionReturn == ACTION_DONE
        || actionReturn == ACTION_ERROR
        || actionReturn == ACTION_STOPPED) {
        CAIAction action;
        SetCurrAction(GetNextAction(action));
        ResetCurrResponse();
    } else if (m_interrupt && actionReturn == ACTION_INTERRUPTABLE) {
        CAIAction action;
        m_actionCount++;
        SetCurrAction(GetNextAction(action));
        m_interrupt = FALSE;
    } else {
        m_actionCount++;
    }
}

// 0x44DC10
SHORT CGameAIBase::ExecuteAction()
{
    // TODO: Incomplete; target-object and message-heavy actions still need recovery.
    SHORT actionReturn = ACTION_DONE;

    // ActionOverride (id 1) is a queue marker.  The script compiler emits it
    // followed by the inner action with m_actorID pre-baked to the override
    // target, so dequeue it here and let the switch dispatch the real action
    // in the same tick.
    if (m_curAction.m_actionID == 1) {
        SetCurrAction(GetNextAction(m_aiAction));
    }

    if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
        actionReturn = ACTION_NO_ACTION;
    } else if (m_curAction.m_actionID == CAIAction::MOVEVIEWPOINT
        || m_curAction.m_actionID == CAIACTION_MOVEVIEWPOINTUNTILDONE) {
        actionReturn = MoveViewPoint();
    } else if (m_curAction.m_actionID == CAIAction::CLICKLBUTTONPOINT) {
        actionReturn = ClickLButtonPoint();
    } else if (m_curAction.m_actionID == CAIAction::CLICKRBUTTONPOINT) {
        actionReturn = ClickRButtonPoint();
    } else if (m_curAction.m_actionID == CAIAction::DOUBLECLICKLBUTTONPOINT) {
        actionReturn = DoubleClickLButtonPoint();
    } else if (m_curAction.m_actionID == CAIAction::DOUBLECLICKRBUTTONPOINT) {
        actionReturn = DoubleClickRButtonPoint();
    } else if (m_curAction.m_actionID == CAIAction::MOVECURSORPOINT) {
        actionReturn = MoveCursorPoint();
    } else if (m_curAction.m_actionID == CAIAction::CHANGEAISCRIPT) {
        actionReturn = ChangeAIScript();
    } else if (m_curAction.m_actionID == CAIAction::STARTTIMER) {
        actionReturn = StartTimer();
    } else if (m_curAction.m_actionID == CAIAction::WAIT
        || m_curAction.m_actionID == 0xCD) {
        actionReturn = Wait();
    } else if (m_curAction.m_actionID == CAIAction::SMALLWAIT) {
        actionReturn = SmallWait();
    } else if (m_curAction.m_actionID == CAIAction::SHOUT
        || m_curAction.m_actionID == CAIACTION_212) {
        actionReturn = Shout();
    } else if (m_curAction.m_actionID == 0x1E
        || m_curAction.m_actionID == 0x132) {
        // 0x1E = SetGlobal, 0x132 = SetGlobalRandom (ACTION.IDS).
        actionReturn = SetGlobal();
    } else if (m_curAction.m_actionID == 0x6D) {
        // 0x6D = IncrementGlobal (ACTION.IDS).
        actionReturn = IncrementGlobal();
    } else if (m_curAction.m_actionID == 0x143) {
        // 0x143 = WaitAnimation (ACTION.IDS).
        actionReturn = WaitAnimation();
    } else if (m_curAction.m_actionID == 0xE9) {
        // 0xE9 = HideCreature (ACTION.IDS).
        actionReturn = HideCreature();
    } else if (m_curAction.m_actionID == 0xF1) {
        // 0xF1 = FloatMessage (ACTION.IDS).
        actionReturn = FloatMessage();
    } else if (m_curAction.m_actionID == 0xB7) {
        // 0xB7 = MultiPlayerSync (ACTION.IDS).  Binary 0x466750 short-circuits
        // to ACTION_DONE in SP (DAT_008cf6dc+0x96e == DAT_0085e65c).  Skip
        // the MP handshake -- recovery deferred until MP path is restored.
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x78) {
        // 0x78 = StartCutScene (ACTION.IDS).
        actionReturn = StartCutScene();
    } else if (m_curAction.m_actionID == 0x7B) {
        // 0x7B = ClearAllActions (ACTION.IDS).  The intro area uses this
        // immediately before StartCutScene; clear party action queues without
        // touching the current script object's remaining response actions.
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); ++nPortrait) {
            LONG nCharacterId = pGame->GetCharacterId(nPortrait);
            CGameAIBase* pSprite = NULL;
            BYTE rc = pGame->GetObjectArray()->GetShare(
                nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                pSprite->ClearActions(FALSE);
                pGame->GetObjectArray()->ReleaseShare(
                    nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x7F) {
        // 0x7F = CutSceneId (ACTION.IDS).  StartCutScene consumes this as a
        // block-local actor switch; executing it directly is a no-op.
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xA1) {
        // 0xA1 = IncrementChapter (ACTION.IDS).
        actionReturn = IncrementChapter();
    } else if (m_curAction.m_actionID == 0xE8) {
        // 0xE8 = StartRandomTimer (ACTION.IDS).  Rolls a random time in
        // [m_specificID2, m_specificID3] inclusive into m_specificID2, then
        // dispatches the regular StartTimer handler (TimerID stays in
        // m_specificID).
        LONG lo = m_curAction.m_specificID2;
        LONG hi = m_curAction.m_specificID3;
        if (lo > hi) {
            LONG tmp = lo;
            lo = hi;
            hi = tmp;
        }
        LONG range = hi - lo + 1;
        if (range <= 1) {
            m_curAction.m_specificID2 = lo;
        } else {
            m_curAction.m_specificID2 = (rand() % range) + lo;
        }
        actionReturn = StartTimer();
    } else if (m_curAction.m_actionID == 0xFC) {
        // 0xFC = ChangeCurrentScript (ACTION.IDS).  Binary pokes
        // m_curScriptNum into m_specificID then falls into the 0x3C
        // (ChangeAIScript) handler, which reads m_specificID for the
        // script-slot index.
        m_curAction.m_specificID = static_cast<LONG>(m_curScriptNum);
        actionReturn = ChangeAIScript();
    } else if (m_curAction.m_actionID == 0xAC) {
        // 0xAC = ChangeTileState (ACTION.IDS).  Target is a CGameTiledObject.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_TILED_OBJECT);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_TILED_OBJECT) {
            actionReturn = ChangeTileState(static_cast<CGameTiledObject*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xB1) {
        // 0xB1 = TriggerActivation (ACTION.IDS).  Target is a CGameTrigger.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_TRIGGER);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_TRIGGER) {
            actionReturn = TriggerActivation(static_cast<CGameTrigger*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xC3) {
        // 0xC3 = Lock (ACTION.IDS).  Target is door or container.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        if (pObj != NULL) {
            actionReturn = Lock(static_cast<CGameAIBase*>(pObj));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xC4) {
        // 0xC4 = Unlock (ACTION.IDS).  Target is door or container.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        if (pObj != NULL) {
            actionReturn = Unlock(static_cast<CGameAIBase*>(pObj));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == CAIAction::FORCESPELL
        || m_curAction.m_actionID == CAIAction::REALLYFORCESPELL
        ) {
        // 0x71 = ForceSpell, 0xB5 = ReallyForceSpell. Normal Spell and
        // SpellNoDec actions are dispatched by CGameSprite::ExecuteAction.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        actionReturn = ForceSpellAction(pObj);
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == CAIAction::SPELLPOINT
        || m_curAction.m_actionID == CAIAction::FORCESPELLPOINT
        || m_curAction.m_actionID == CAIAction::SPELLPOINTNODEC) {
        // 0x5F = SpellPoint, 0x72 = ForceSpellPoint, 0xC0 = SpellPointNoDec.
        // Ghidra's binary jump table routes 0x72 to FUN_00461B80.  Sprites
        // never reach here for 0x5F/0xC0 anymore: CGameSprite::ExecuteAction
        // dispatches them to SpellPointSequence (jumptable case 0x2c), like
        // the binary.  Non-sprite AIBASEs keep the force path.
        actionReturn = ForceSpellPointAction();
    } else if (m_curAction.m_actionID == 0x10) {
        // 0x10 = GiveOrder (ACTION.IDS).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        if (pObj != NULL && (pObj->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            actionReturn = GiveOrder(static_cast<CGameAIBase*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x3E) {
        // 0x3E = SendTrigger (ACTION.IDS).
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL && (pObj->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            actionReturn = SendTrigger(static_cast<CGameAIBase*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x97) {
        // 0x97 = DisplayString (ACTION.IDS).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        if (pObj != NULL && (pObj->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            actionReturn = DisplayString(static_cast<CGameAIBase*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xC9) {
        // 0xC9 = DetectSecretDoor (ACTION.IDS).  Target is CGameDoor.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_DOOR);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_DOOR) {
            actionReturn = DetectSecretDoor(static_cast<CGameDoor*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xD1) {
        // 0xD1 = SpawnPtActivate (ACTION.IDS).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPAWNING);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_SPAWNING) {
            actionReturn = SpawnPtActivate(static_cast<CGameSpawning*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xD2) {
        // 0xD2 = SpawnPtDeactivate (ACTION.IDS).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPAWNING);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_SPAWNING) {
            actionReturn = SpawnPtDeactivate(static_cast<CGameSpawning*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xD3) {
        // 0xD3 = SpawnPtSpawn (ACTION.IDS).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPAWNING);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_SPAWNING) {
            actionReturn = SpawnPtSpawn(static_cast<CGameSpawning*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xD5 || m_curAction.m_actionID == 0xD6) {
        // 0xD5 = StartStatic, 0xD6 = StopStatic.  Binary pushes bStart = 1
        // for 0xD5 and bStart = 0 for 0xD6 (0x450171 vs 0x45019C).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_STATIC);
        if (pObj != NULL && pObj->GetObjectType() == CGameObject::TYPE_STATIC) {
            actionReturn = StaticStart(static_cast<CGameStatic*>(pObj),
                m_curAction.m_actionID == 0xD5 ? TRUE : FALSE);
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0xC5) {
        // 0xC5 = MoveGlobal (ACTION.IDS).  Target is a CGameSprite.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL && (pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
            actionReturn = MoveGlobal(static_cast<CGameSprite*>(pObj));
        }
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x84) {
        // 0x84 = VerbalConstant (ACTION.IDS).  Resolves m_acteeID and
        // broadcasts CMessageVerbalConstant so the target plays the named
        // voice line (m_specificID).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            CMessage* msg = new CMessageVerbalConstant(
                m_curAction.m_specificID,
                m_id,
                pObj->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x119) {
        // 0x119 = MarkObject (ACTION.IDS).  Stores the resolved target's
        // AI type in field_342 via SetAIType342; NOONE when unresolved.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj == NULL) {
            SetAIType342(CAIObjectType::NOONE);
        } else {
            SetAIType342(pObj->GetAIType());
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x131) {
        // 0x131 = SetMyTarget (ACTION.IDS).  Stores the resolved target's
        // AI type in field_37E via SetAIType37E; NOONE when unresolved.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj == NULL) {
            SetAIType37E(CAIObjectType::NOONE);
        } else {
            SetAIType37E(pObj->GetAIType());
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xA7) {
        // 0xA7 = StartMovie.
        actionReturn = StartMovie();
    } else if (m_curAction.m_actionID == 0xA9) {
        // 0xA9 = DestroyItem (ACTION.IDS).  Pure wire to existing helper.
        actionReturn = DestroyItem();
    } else if (m_curAction.m_actionID == 0xAA) {
        // 0xAA = RevealAreaOnMap.
        actionReturn = RevealAreaOnMap();
    } else if (m_curAction.m_actionID == 0xBB) {
        // 0xBB = StartMusic.
        actionReturn = StartMusic();
    } else if (m_curAction.m_actionID == 0xBE) {
        // 0xBE = FinalSave.
        actionReturn = FinalSave();
    } else if (m_curAction.m_actionID == 0xCA) {
        // 0xCA = FadeToColor.
        actionReturn = FadeToColor();
    } else if (m_curAction.m_actionID == 0xCB) {
        // 0xCB = FadeFromColor.
        actionReturn = FadeFromColor();
    } else if (m_curAction.m_actionID == 0xFD) {
        // 0xFD = FadeColorActivate.
        actionReturn = FadeColorActivate();
    } else if (m_curAction.m_actionID == 0x144) {
        // 0x144 = SetMusic.
        actionReturn = SetMusic();
    } else if (m_curAction.m_actionID == 0xEB) {
        // 0xEB = PlaySequence (ACTION.IDS).  Resolves m_acteeID, then
        // broadcasts a CMessageSetSequence so the target plays the
        // requested animation sequence (m_specificID).
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            CMessage* msg = new CMessageSetSequence(
                static_cast<BYTE>(m_curAction.m_specificID),
                pObj->m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x85) {
        // 0x85 = ClearActions (ACTION.IDS).  Resolves m_acteeID and forwards
        // to ClearActions(target), which queues a CMessageClearActions.
        // TODO: Binary also enqueues a NULL_ACTION terminator after clear.
        CGameObject* pObj = ResolveActionTarget();
        actionReturn = ClearActions(pObj);
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else if (m_curAction.m_actionID == CAIAction::TAKEPARTYGOLD) {
        actionReturn = TakePartyGold();
    } else if (m_curAction.m_actionID == CAIAction::GIVEPARTYGOLD
        || m_curAction.m_actionID == CAIAction::GIVEGOLDFORCE) {
        actionReturn = GivePartyGold();
    } else if (m_curAction.m_actionID == 0x1A) {
        // 0x1A = CallLightning (ACTION.IDS 26).  Binary case 0x1a:
        // resolves the target, draws the lightning bolt via
        // CInfinity::CallLightning, then queues a Damage(0x0D) effect
        // on the target through CMessageAddEffect.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            CPoint ptTarget = pObj->GetPos();
            CGameArea* pArea = pObj->GetArea();
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);

            // Draw the lightning bolt from the sky.
            if (pArea != NULL) {
                pArea->GetInfinity()->CallLightning(ptTarget.x, ptTarget.y);
            }

            // Queue the damage effect (opcode 0x0D = Damage).
            // TODO: the damage dice come from m_specificID (spell ability
            // header); using a placeholder for now.
            CGameEffect* pEffect = new CGameEffect();
            pEffect->m_effectID = 0x0D;        // Damage opcode
            pEffect->m_targetType = 9;          // target type
            pEffect->m_effectAmount = 0x100;    // TODO: read from action params

            CMessage* pMsg = new CMessageAddEffect(pEffect, m_id, pObj->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x32) {
        // 0x32 = MoveViewObject (ACTION.IDS 50).  Binary case 0x32.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            actionReturn = MoveViewObject(pObj);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x34) {
        // 0x34 = ClickLButtonObject (ACTION.IDS 52).  Binary case 0x34.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            actionReturn = ClickLButtonObject(pObj);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x36) {
        // 0x36 = ClickRButtonObject (ACTION.IDS 54).  Binary case 0x36.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            actionReturn = ClickRButtonObject(pObj);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x38) {
        // 0x38 = DoubleClickLButtonObject (ACTION.IDS 56).  Binary case 0x38.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            actionReturn = DoubleClickLButtonObject(pObj);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x3A) {
        // 0x3A = DoubleClickRButtonObject (ACTION.IDS 58).  Binary case 0x3a.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            actionReturn = DoubleClickRButtonObject(pObj);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    } else if (m_curAction.m_actionID == 0x24
        || m_curAction.m_actionID == 0xB2
        || m_curAction.m_actionID == 0xEA
        || m_curAction.m_actionID == 0xF6) {
        // 0x24=Continue, 0xB2=BreakInstants, 0xEA=Debug, 0xF6=Log.
        // Binary cases share switchD_0044DC95_caseD_24 which is
        // sVar7 = -1 (ACTION_DONE) -- no side effects.
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x7E) {
        // 0x7E = Activate (ACTION.IDS 126).  Binary case 0x7e posts a
        // CMessageSetActive(TRUE) on the resolved target.
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            CMessage* msg = new CMessageSetActive(TRUE, m_id, pObj->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x7D) {
        // 0x7D = Deactivate (ACTION.IDS 125).  Binary case 0x7d posts
        // CMessageSetActive(FALSE) -- but skips PCs (party-portrait check).
        CGameObject* pObj = ResolveActionTarget();
        if (pObj != NULL) {
            SHORT nPortrait = g_pBaldurChitin->GetObjectGame()
                                  ->GetCharacterPortraitNum(pObj->m_id);
            if (nPortrait == -1) {
                CMessage* msg = new CMessageSetActive(FALSE, m_id, pObj->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xA2) {
        // 0xA2 = ReputationSet (ACTION.IDS 162).  Binary computes the
        // delta from m_specificID (1-20 scale) and current m_nReputation
        // (10-200 internal), then calls ReputationAdjustment(delta*10).
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        SHORT delta = static_cast<SHORT>(m_curAction.m_specificID)
            - pGame->m_nReputation / CInfGame::REPUTATION_MULTIPLIER;
        pGame->ReputationAdjustment(
            static_cast<SHORT>(delta * CInfGame::REPUTATION_MULTIPLIER), FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xA3) {
        // 0xA3 = ReputationInc (ACTION.IDS 163).  Pure adjustment by
        // m_specificID * REPUTATION_MULTIPLIER.
        g_pBaldurChitin->GetObjectGame()->ReputationAdjustment(
            static_cast<SHORT>(m_curAction.m_specificID
                * CInfGame::REPUTATION_MULTIPLIER),
            FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x104) {
        // 0x104 = JumpToPointInstant (ACTION.IDS 260).  Binary case 0x104
        // resolves the target sprite and calls JumpToPoint(dest).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                static_cast<CGameSprite*>(pObj)->JumpToPoint(
                    m_curAction.m_dest, TRUE);
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x117) {
        // 0x117 = AdvanceTime (ACTION.IDS 279).  Pushes m_specificID into
        // the world timer and emits a FEEDBACK_TIMEPASS note.
        g_pBaldurChitin->GetObjectGame()->GetWorldTimer()
            ->AdvanceCurrentTime(m_curAction.m_specificID);
        // TODO: CInfGame::FeedBack(DAT_008518CA, m_specificID, ?) -- the
        // binary call site pushes only 2 args; the C++ signature has an
        // extra BOOLEAN that defaults via m_bShowQuestXP fallback.
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x83) {
        // 0x83 = Kill (ACTION.IDS 131).  Resolves target, skips work if
        // the derivedStats already carry STATE_DEAD, runs the sub_761650
        // prep, builds a CGameEffectDeath (inlined default ctor: effectID
        // 13 + m_deathType 1 over the CGameEffect base ctor's zeroing) with
        // m_dwFlags 4 (instant equip-style timing) and m_sourceID set to
        // the target itself (the binary writes target.m_id, not caster's,
        // into the source slot -- preserved as-is), then queues a
        // CMessageAddEffect on the target.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
            if ((pSprite->GetDerivedStats()->m_generalState & STATE_DEAD) == 0) {
                pSprite->sub_761650();
                CGameEffectDeath* pEffect = new CGameEffectDeath();
                pEffect->m_effectAmount = 0;
                pEffect->m_dwFlags = 4;
                pEffect->m_sourceID = pObj->m_id;
                CMessage* msg = new CMessageAddEffect(pEffect, m_id, pObj->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x102) {
        // 0x102 = RestUntilHealed (ACTION.IDS 258).  Binary asks
        // CanRestParty (passing bit 1 of action flags as the
        // ignore-encounters arg), then either toggles m_bHealPartyOnRest
        // around a RestParty call or displays the rejection strref the
        // out-param carried back, tinted green (the standard rest-feedback
        // color 0x00FF00).
        STRREF strError = 0;
        unsigned char ignoreEncounters
            = (m_curAction.GetFlags() & 0x2) != 0 ? 1 : 0;
        BOOL canRest = g_pBaldurChitin->GetObjectGame()->CanRestParty(
            strError, 0, 1, ignoreEncounters);
        if (canRest == TRUE) {
            CInfGame::m_bHealPartyOnRest = TRUE;
            g_pBaldurChitin->GetObjectGame()->RestParty(1, 1);
            CInfGame::m_bHealPartyOnRest = FALSE;
        } else {
            STR_RES strRes;
            if (g_pBaldurChitin->GetTlkTable().Fetch(strError, strRes)) {
                CString sName("");
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(
                    sName,
                    strRes.szText,
                    0x00FF00,
                    0x00FF00,
                    -1,
                    m_id,
                    m_id);
            }
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x10A) {
        // 0x10A = DisplayMessage (ACTION.IDS 266, signature
        // `DisplayMessage(I:StrRef*)`).  Binary fetches the TLK string for
        // m_specificID, then queues a CBaldurMessage::DisplayText with the
        // pre-fetched CString -- this routes through the TLK-override path
        // (CBaldurMessage::DisplayTextRef would skip overrides).  Name is
        // empty, marker is the -1 "no journal entry" sentinel, both color
        // slots use the standard NPC-chat tint 0xD7C8A0.
        STR_RES strRes;
        if (g_pBaldurChitin->GetTlkTable().Fetch(m_curAction.m_specificID, strRes)) {
            CString sName("");
            g_pBaldurChitin->GetBaldurMessage()->DisplayText(
                sName,
                strRes.szText,
                0xD7C8A0,
                0xD7C8A0,
                -1,
                m_id,
                m_id);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x10E) {
        // 0x10E = ClearSpriteEffects (ACTION.IDS 270).  Binary resolves
        // the target and calls ReapplyEquipmentEffects on it -- counter-
        // intuitive name but the action's job is to drop transient effects
        // and rebuild from equipment.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                static_cast<CGameSprite*>(pObj)->ReapplyEquipmentEffects();
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x40
        || m_curAction.m_actionID == 0x41) {
        // 0x40 = UndoExplore (ACTION.IDS 64), 0x41 = Explore (ACTION.IDS 65).
        // Binary posts CMessageSetAreaExplored on self -- 0x40 with FALSE
        // (unexplore), 0x41 with TRUE.
        CMessage* msg = new CMessageSetAreaExplored(
            m_curAction.m_actionID == 0x41 ? TRUE : FALSE,
            m_id,
            m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x88) {
        // 0x88 = UnlockScroll (ACTION.IDS 136).  Binary case 0x88 clears
        // CScreenWorld::m_scrollLockId (sets to -1).
        g_pBaldurChitin->m_pEngineWorld->m_scrollLockId = -1;
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x99
        || m_curAction.m_actionID == 0x9A
        || m_curAction.m_actionID == 0x9B
        || m_curAction.m_actionID == 0x9D
        || m_curAction.m_actionID == 0x9E
        || m_curAction.m_actionID == 0x9F) {
        // 0x99-0x9F = ChangeEnemyAlly / ChangeGeneral / ChangeRace /
        // ChangeSpecifics / ChangeGender / ChangeAlignment (ACTION.IDS
        // 153-155, 157-159).  Binary cases 0x99-0x9F share a uniform
        // shape: resolve target, build a copy of its AI type, mutate the
        // single byte field that names the action, call SetAIType with
        // updateLive=TRUE and updateStart=FALSE, then broadcast
        // CMessageSpriteUpdate (gated on SP or host-matches-target in
        // the binary; we always broadcast since observers expect the
        // update).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                CAIObjectType newType(pSprite->GetAIType());
                BYTE value = static_cast<BYTE>(m_curAction.m_specificID);
                switch (m_curAction.m_actionID) {
                case 0x99: newType.SetEnemyAlly(value); break;
                case 0x9A: newType.SetGeneral(value);   break;
                case 0x9B: newType.SetRace(value);      break;
                case 0x9D: newType.SetSpecific(value);  break;
                case 0x9E: newType.SetGender(value);    break;
                case 0x9F: newType.SetAlignment(value); break;
                }
                pSprite->SetAIType(newType, TRUE, FALSE);
                CMessage* msg = new CMessageSpriteUpdate(pSprite, m_id, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x9C) {
        // 0x9C = ChangeClass (ACTION.IDS 156).  Binary case 0x9C allocates
        // three local CAIObjectTypes and immediately destructs them
        // without any field write or SetAIType call -- the IWD2 build
        // shipped this action as an effective no-op.  Preserve that
        // behaviour (returns ACTION_DONE without mutating the target).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x103) {
        // 0x103 = SetVisualRange (ACTION.IDS 259).  Binary case 0x103
        // posts CMessage92(self, self, m_specificID) -- the message
        // class number is unresolved but the binary call site uses the
        // m_id/m_id/m_specificID triple.
        CMessage* msg = new CMessage92(
            m_id, m_id, m_curAction.m_specificID);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xFE) {
        // 0xFE = SetHP (ACTION.IDS 254).  Binary case 0xfe writes
        // min(m_specificID, target.maxHP) into target.m_hitPoints if the
        // target isn't already marked dead (state bit 0x800).  We need to
        // operate on the target sprite's m_baseStats.m_hitPoints
        // (offset 0x1c into CCreatureFileHeader == sprite + 0x5C0).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                if ((pSprite->m_baseStats.m_generalState & 0x800) == 0) {
                    LONG hp = m_curAction.m_specificID;
                    SHORT maxHP = pSprite->GetDerivedStats()->m_nMaxHitPoints;
                    if (hp > maxHP) {
                        pSprite->m_baseStats.m_hitPoints = maxHP;
                    } else {
                        pSprite->m_baseStats.m_hitPoints = static_cast<SHORT>(hp);
                    }
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xBA) {
        // 0xBA = EndCredits (ACTION.IDS 186).  Binary case 0xba jumps to
        // switchD_caseD_ba which calls CScreenWorld::ReadyEndCredits(FALSE).
        g_pBaldurChitin->m_pEngineWorld->ReadyEndCredits(FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x10F) {
        // 0x10F = StopJoinRequests (ACTION.IDS 271).  Saves the current
        // ListenToJoin flag into field_595 and disables join requests,
        // marking field_596 to allow a later ResetJoinRequests to
        // restore the prior state.
        if (field_596 == 0) {
            CMultiplayerSettings* pMP =
                g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings();
            field_595 = static_cast<unsigned char>(pMP->GetListenToJoinOption());
            pMP->SetListenToJoinOption(FALSE, FALSE);
            field_596++;
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x110) {
        // 0x110 = ResetJoinRequests (ACTION.IDS 272).  Restores the
        // saved ListenToJoin flag from field_595 and clears field_596.
        CMultiplayerSettings* pMP =
            g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings();
        pMP->SetListenToJoinOption(field_595 != 0 ? TRUE : FALSE, FALSE);
        field_596 = 0;
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xE6) {
        // 0xE6 = RestParty (ACTION.IDS 230).  Binary case 0xe6 calls
        // CInfGame::RestParty(1, 1).
        g_pBaldurChitin->GetObjectGame()->RestParty(1, 1);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xAD) {
        // 0xAD = AddJournalEntry (ACTION.IDS 173).  Binary case 0xad
        // calls m_cJournal.AddEntry(m_specificID, 0).
        g_pBaldurChitin->GetObjectGame()->m_cJournal.AddEntry(
            static_cast<STRREF>(m_curAction.m_specificID), 0);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xF4) {
        // 0xF4 = DeleteJournalEntry (ACTION.IDS 244).
        g_pBaldurChitin->GetObjectGame()->m_cJournal.DeleteEntry(
            static_cast<STRREF>(m_curAction.m_specificID));
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0xF5) {
        // 0xF5 = JournalEntryDone (ACTION.IDS 245).
        g_pBaldurChitin->GetObjectGame()->m_cJournal.SetQuestDone(
            static_cast<STRREF>(m_curAction.m_specificID));
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x10C) {
        // 0x10C = ClearPartyEffects (ACTION.IDS 268).  Iterates party
        // members; for each with m_baseStats.m_flags bit 0x800 set, calls
        // ReapplyEquipmentEffects to rebuild equipment-derived state.
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        SHORT nChars = pGame->GetNumCharacters();
        for (SHORT i = 0; i < nChars; ++i) {
            LONG nCharId = pGame->GetCharacterId(i);
            CGameSprite* pSprite = NULL;
            BYTE rc = pGame->GetObjectArray()->GetShare(
                nCharId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                if ((pSprite->m_baseStats.m_flags & 0x800) != 0) {
                    pSprite->ReapplyEquipmentEffects();
                }
                pGame->GetObjectArray()->ReleaseShare(
                    nCharId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x42) {
        // 0x42 = DayNight (ACTION.IDS 66).  Binary case 0x42 computes
        // m_specificID * byte[0x84EC0C] * byte[0x84EC0D] * byte[0x84EC0E]
        // == hours * MSEC_PER_SEC * SEC_PER_MIN * MIN_PER_HOUR
        // == hours * MSEC_PER_HOUR (15 * 60 * 5 == 4500) and feeds it
        // into CTimerWorld::AdvanceCurrentTime.
        g_pBaldurChitin->GetObjectGame()->GetWorldTimer()
            ->AdvanceCurrentTime(m_curAction.m_specificID
                * CTimerWorld::TIMESCALE_MSEC_PER_HOUR);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x11F) {
        // 0x11F = SetHPPercent (ACTION.IDS 287).  Binary case 0x11f
        // clamps m_specificID to [0,100], divides by 100 to get a
        // fraction, then:
        //   specifics2 == 0: heal-only set to max(curHP, maxHP * frac);
        //                    skips write entirely when maxHP*frac <= curHP.
        //   specifics2 != 0: scale curHP by frac.
        // Dead targets (derivedStats.generalState bit 0x800 OR
        // baseStats.m_generalState bit 0x800) are skipped.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                CDerivedStats* pDeriv = pSprite->GetDerivedStats();
                if ((pDeriv->m_generalState & 0x800) == 0
                    && (pSprite->m_baseStats.m_generalState & 0x800) == 0) {
                    float pct = static_cast<float>(m_curAction.m_specificID);
                    if (pct > 100.0f) pct = 100.0f;
                    else if (pct < 0.0f) pct = 0.0f;
                    float frac = pct / 100.0f;
                    SHORT curHP = pSprite->m_baseStats.m_hitPoints;
                    SHORT maxHP = pDeriv->m_nMaxHitPoints;
                    if (m_curAction.GetSpecifics2() == 0) {
                        if (static_cast<float>(maxHP) * frac
                            > static_cast<float>(curHP)) {
                            pSprite->m_baseStats.m_hitPoints =
                                static_cast<SHORT>(
                                    static_cast<float>(maxHP) * frac);
                        }
                    } else {
                        pSprite->m_baseStats.m_hitPoints =
                            static_cast<SHORT>(
                                static_cast<float>(curHP) * frac);
                    }
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x130) {
        // 0x130 = AddHP (binary case 0x130).  Resolves the target and,
        // unless flagged dead (state bit 0x800), adds m_specificID to
        // target.m_baseStats.m_hitPoints capped at m_nMaxHitPoints.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                if ((pSprite->m_baseStats.m_generalState & 0x800) == 0) {
                    LONG delta = m_curAction.m_specificID;
                    SHORT curHP = pSprite->m_baseStats.m_hitPoints;
                    SHORT maxHP = pSprite->GetDerivedStats()->m_nMaxHitPoints;
                    if (curHP + delta < maxHP) {
                        pSprite->m_baseStats.m_hitPoints =
                            curHP + static_cast<SHORT>(delta);
                    } else {
                        pSprite->m_baseStats.m_hitPoints = maxHP;
                    }
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x138) {
        // 0x138 = SetApparentName (binary case 0x138).  Writes the new
        // STRREF (m_specificID) into target.m_baseStats.m_apparentName
        // when it differs from the current value, then broadcasts
        // CMessageSpriteUpdate so observers refresh the displayed name.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                DWORD newRef = static_cast<DWORD>(m_curAction.m_specificID);
                if (pSprite->m_baseStats.m_apparentName != newRef) {
                    pSprite->m_baseStats.m_apparentName = newRef;
                    CMessage* msg = new CMessageSpriteUpdate(
                        pSprite, m_id, pSprite->m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(
                        msg, FALSE);
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x139) {
        // 0x139 = SetName (binary case 0x139).  Same shape as 0x138 but
        // writes target.m_baseStats.m_name (the canonical STRREF).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                DWORD newRef = static_cast<DWORD>(m_curAction.m_specificID);
                if (pSprite->m_baseStats.m_name != newRef) {
                    pSprite->m_baseStats.m_name = newRef;
                    CMessage* msg = new CMessageSpriteUpdate(
                        pSprite, m_id, pSprite->m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(
                        msg, FALSE);
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x13F) {
        // 0x13F = ForceMarkedSpell (ACTION.IDS 319).  Force-casts a
        // spell that was previously marked by a duration effect (e.g.
        // Call Lightning's periodic lightning strike).  Resolves the
        // target (the spell's designated victim) and dispatches through
        // the same FireSpell path as ReallyForceSpell.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            actionReturn = ForceSpellAction(pObj);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        } else {
            actionReturn = ACTION_DONE;
        }
    } else if (m_curAction.m_actionID == 0x13E) {
        // 0x13E = SetCreatureFlag (binary case 0x13e).  Resolves the
        // target sprite and applies AND-clear (specifics2 == 0) or
        // OR-set (specifics2 != 0) of mask m_specificID to its
        // m_baseStats.m_flags.  Returns ACTION_INTERRUPTABLE when the
        // target is unresolved (binary path LAB_00451451 -> sVar7 = -2).
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj == NULL) {
            actionReturn = ACTION_INTERRUPTABLE;
        } else {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                DWORD mask = static_cast<DWORD>(m_curAction.m_specificID);
                if (m_curAction.GetSpecifics2() == 0) {
                    pSprite->m_baseStats.m_flags &= ~mask;
                } else {
                    pSprite->m_baseStats.m_flags |= mask;
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            actionReturn = ACTION_DONE;
        }
    } else if (m_curAction.m_actionID == 0x145) {
        // 0x145 = SetAreaFlag (binary case 0x145).  Three-way bit-op on
        // m_pArea->m_header.m_flags selected by specifics2:
        //   1 -> AND  (clear masked bits)
        //   2 -> OR   (set masked bits)
        //   3 -> XOR  (toggle masked bits)
        //   other -> ACTION_INTERRUPTABLE.
        if (m_pArea != NULL) {
            DWORD mask = static_cast<DWORD>(m_curAction.m_specificID);
            switch (m_curAction.GetSpecifics2()) {
            case 1: m_pArea->m_header.m_flags &= mask; break;
            case 2: m_pArea->m_header.m_flags |= mask; break;
            case 3: m_pArea->m_header.m_flags ^= mask; break;
            default: actionReturn = ACTION_INTERRUPTABLE; break;
            }
            if (actionReturn != ACTION_INTERRUPTABLE) {
                actionReturn = ACTION_DONE;
            }
        } else {
            actionReturn = ACTION_INTERRUPTABLE;
        }
    } else if (m_curAction.m_actionID == 0x12F) {
        // 0x12F = SetRestEncounterChance (ACTION.IDS 303).  Binary case
        // 0x12f caps both probabilities at 100, writes them into the
        // area's rest-encounter header, and posts CMessageSetAreaRestEncounter
        // when the caster is in control.
        if (m_pArea != NULL) {
            CAreaFileRestEncounter* pRest = m_pArea->GetHeaderRestEncounter();
            if (pRest != NULL) {
                LONG dayProb = m_curAction.GetSpecifics();
                if (dayProb > 100) dayProb = 100;
                LONG nightProb = m_curAction.GetSpecifics2();
                if (nightProb > 100) nightProb = 100;
                pRest->m_probDay = static_cast<WORD>(dayProb);
                pRest->m_probNight = static_cast<WORD>(nightProb);
                if (InControl()) {
                    CMessage* msg = new CMessageSetAreaRestEncounter(
                        pRest->m_probDay,
                        pRest->m_probNight,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(
                        msg, FALSE);
                }
            }
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x12D) {
        // 0x12D = SetExtendedNight (ACTION.IDS 301).  Binary case 0x12d
        // toggles bit 0x40 of m_pArea->m_header.m_areaType based on
        // m_specificID (0 clears, nonzero sets) then propagates the new
        // areaType to the area's CInfinity render.
        if (m_pArea != NULL) {
            WORD areaType = m_pArea->m_header.m_areaType;
            if (m_curAction.m_specificID == 0) {
                areaType &= 0xFFBF;
            } else {
                areaType |= 0x40;
            }
            m_pArea->m_header.m_areaType = areaType;
            CInfinity* pInfinity = m_pArea->GetInfinity();
            if (pInfinity != NULL) {
                pInfinity->SetAreaType(areaType);
            }
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x11B) {
        // 0x11B = SetCriticalPathObject (ACTION.IDS 283).  Binary case
        // 0x11b toggles bit 0x2000 of the target sprite's
        // m_baseStats.m_flags based on m_specificID (0 clears, nonzero
        // sets), then posts CMessageSpriteUpdate in SP or matching-host
        // mode.  We perform the bit toggle; the SpriteUpdate broadcast
        // is a TODO until the MP-host comparison helper is recovered.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
                if (m_curAction.m_specificID == 0) {
                    pSprite->m_baseStats.m_flags &= ~0x2000u;
                } else {
                    pSprite->m_baseStats.m_flags |= 0x2000u;
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x120) {
        // 0x120 = SetDoorFlag (ACTION.IDS 288).  Binary case 0x120 reads
        // the resolved door's flags, OR-merges (when specifics2!=0) or
        // AND-NOT-clears (when 0) bit-mask specifics1, then writes back
        // and posts CMessageDoorStatus.
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_DOOR);
        if (pObj != NULL) {
            if (pObj->GetObjectType() == CGameObject::TYPE_DOOR) {
                CGameDoor* pDoor = static_cast<CGameDoor*>(pObj);
                DWORD flags = pDoor->GetFlags();
                DWORD mask = static_cast<DWORD>(m_curAction.GetSpecifics());
                if (m_curAction.GetSpecifics2() != 0) {
                    flags |= mask;
                } else {
                    flags &= ~mask;
                }
                pDoor->SetFlags(flags);
                CMessage* msg = new CMessageDoorStatus(pDoor, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x129) {
        // 0x129 = ScreenShake (ACTION.IDS 297).  Binary case 0x129 posts
        // CMessageScreenShake(duration=specifics1, magX=specifics2,
        // magY=specifics3, bOverride=TRUE) on self.
        CMessage* msg = new CMessageScreenShake(
            static_cast<WORD>(m_curAction.GetSpecifics()),
            static_cast<CHAR>(m_curAction.GetSpecifics2()),
            static_cast<CHAR>(m_curAction.GetSpecifics3()),
            TRUE,
            m_id,
            m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x111
        || m_curAction.m_actionID == 0x112) {
        // 0x111 = HideGUI (ACTION.IDS 273), 0x112 = UnhideGUI (274).
        // In SP the binary calls CScreenWorld::HideInterface / UnhideInterface
        // directly; in MP it posts CMessageToggleInterface.  We take the MP
        // path always -- the message handler routes to the same screen calls
        // and avoids the SP-only auto-hide bookkeeping branch.
        BOOLEAN bHide = m_curAction.m_actionID == 0x111 ? TRUE : FALSE;
        CMessage* msg = new CMessageToggleInterface(bHide, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x113) {
        // 0x113 = SaveGame(I:STRREF*) (ACTION.IDS 275).  Binary case 0x451282
        // only posts the message on SP or the MP host; clients no-op.
        if (!g_pChitin->cNetwork.GetSessionOpen()
            || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
            CMessage* msg = new CMessageSaveGame(
                static_cast<STRREF>(m_curAction.GetSpecifics()),
                m_id,
                m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x11E) {
        // 0x11E = AllowAreaResting (ACTION.IDS 286).  Binary case 0x11e
        // toggles the no-rest bit (0x2) in m_pArea->m_header.m_flags --
        // m_specificID == 0 sets it (disallow), nonzero clears it (allow).
        if (m_pArea != NULL) {
            if (m_curAction.m_specificID == 0) {
                m_pArea->m_header.m_flags |= 2;
            } else {
                m_pArea->m_header.m_flags &= ~2u;
            }
        }
        actionReturn = ACTION_DONE;
    } else if (m_curAction.m_actionID == 0x79
        || m_curAction.m_actionID == 0x7A) {
        // 0x79 = StartCutSceneMode (ACTION.IDS 121),
        // 0x7A = EndCutSceneMode  (ACTION.IDS 122).  Binary posts
        // CMessageCutSceneModeStatus -- 0x79 with TRUE, 0x7A with FALSE.
        CMessage* msg = new CMessageCutSceneModeStatus(
            m_curAction.m_actionID == 0x79 ? TRUE : FALSE,
            m_id,
            m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        actionReturn = ACTION_DONE;
    }

    SetLastActionReturn(actionReturn);
    return actionReturn;
}

// 0x45C300
void CGameAIBase::InsertResponse(CAIResponse& response, BOOL checkCurrentResponse, BOOL clearActions)
{
    CAIAction action;
    CAIObjectType actorType;

    if (checkCurrentResponse
        && m_curResponseSetNum >= 0
        && m_curScriptNum >= 0
        && m_curResponseSetNum == response.m_responseSetNum
        && m_curScriptNum == response.m_scriptNum) {
        return;
    }

    if (clearActions) {
        ClearActions(FALSE);
    }

    m_curResponseNum = response.m_responseNum;
    m_curResponseSetNum = response.m_responseSetNum;
    m_curScriptNum = response.m_scriptNum;
    m_interrupt = TRUE;

    POSITION pos = response.m_actionList.GetHeadPosition();
    while (pos != NULL) {
        CAIAction* node = response.m_actionList.GetNext(pos);
        action = *node;

        CAIAction* newNode = new CAIAction();
        *newNode = action;
        m_queuedActions.AddTail(newNode);
    }
}

// 0x45C730
void CGameAIBase::ProcessAI()
{
    if (!InControl()) {
        return;
    }

    if (m_curAction.GetActionID() == CAIAction::NO_ACTION) {
        if (!m_queuedActions.IsEmpty()) {
            SetCurrAction(GetNextAction(m_aiAction));
            m_interrupt = FALSE;
        }
        if (m_curAction.GetActionID() == CAIAction::NO_ACTION) {
            m_inCutScene = FALSE;
        }
    }
    m_randValue = rand() % 0x7FFF;

    if (!g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_active && !m_firstCall) {
        if (m_curAction.GetActionID() == CAIAction::NO_ACTION) {
            SetCurrAction(GetNextAction(m_aiAction));
            m_interrupt = FALSE;
        }
        while (g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lInstantActions.Find(m_curAction.GetActionID()) != NULL) {
            if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
                && m_curAction.GetActionID() == CAIAction::LEAVEAREALUA) {
                break;
            }
            DoAction();
        }
        return;
    }

    field_44A++;
    if (((field_44A ^ m_id) & 0xF) == 0) {
        CheckTimers(1);
    }
    if (((field_44A ^ m_id) & 0xF) == 0 || m_firstCall || m_forceActionPick) {
        field_54C++;
        m_firstCall = FALSE;
        m_forceActionPick = FALSE;
        if (m_curAction.GetActionID() != CAIAction::NO_ACTION
            && m_pendingTriggers.IsEmpty() && field_550 < 120 && field_552 <= 0) {
            field_550++;
        } else {
            if (!m_pendingTriggers.IsEmpty()) {
                field_552 = 5;
            }
            ProcessScripts();
            if (field_552 > 0) {
                field_552--;
            }
            field_550 = 0;
        }
    }

    if (m_curAction.GetActionID() == CAIAction::NO_ACTION) {
        SetCurrAction(GetNextAction(m_aiAction));
        m_interrupt = FALSE;
    }
    if (m_posVertList != NULL) {
        do {
            DoAction();
        } while (g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lInstantActions.Find(m_curAction.GetActionID()) != NULL
            && !(g_pChitin->cNetwork.GetSessionOpen() == TRUE
                && m_curAction.GetActionID() == CAIAction::LEAVEAREALUA));
    }
}

// 0x45D050
BOOL CGameAIBase::QuickDecode(CAITrigger& trigger, CGameSprite*& sprite)
{
    trigger.m_triggerCause.Decode(this);
    sprite = static_cast<CGameSprite*>(trigger.m_triggerCause.GetObjectWithType(this, TYPE_SPRITE, FALSE));
    if (sprite != NULL && sprite->GetAIType().Equal(CAIObjectType::NOT_SPRITE)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(sprite->GetId(),
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
        sprite = NULL;
    }
    return FALSE;
}

// 0x45CA10
void CGameAIBase::ProcessScripts()
{
    // TODO: 0x57 action sentinel meaning not yet identified.

    if (m_inCutScene) {
        return;
    }
    if (m_nLastActionReturn == 0) {
        return;
    }

    CAIResponse localResponse;

    ApplyTriggers();

    CAIResponse* found = NULL;
    if (m_overrideScript != NULL) {
        found = m_overrideScript->Find(m_pendingTriggers, this);
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BOOL bPlayerControl = pGame->m_bPartyAI == FALSE
        && pGame->GetCharacterPortraitNum(m_id) != -1;
    SHORT curID = m_curAction.m_actionID;
    BOOL bBypassChain = bPlayerControl
        || curID == 0x1B
        || curID == 0x57
        || (curID == CAIAction::MOVETOPOINT && m_curAction.m_specificID3 == 1);

    if (bBypassChain) {
        if (found != NULL) {
            if (!found->m_actionList.IsEmpty()) {
                found->m_scriptNum = 0;
                InsertResponse(*found, TRUE, TRUE);

                POSITION pos = m_pendingTriggers.GetHeadPosition();
                while (pos != NULL) {
                    CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
                    if (pTrigger != NULL) {
                        delete pTrigger;
                    }
                }
                m_pendingTriggers.RemoveAll();
            }
            delete found;
            return;
        }
    }

    BOOL bTryNextScript = TRUE;
    if (found != NULL) {
        localResponse.Add(*found);
        BOOL hasContinue = localResponse.InListEnd(CAIAction::CONTINUE);
        bTryNextScript = found->m_actionList.IsEmpty() || hasContinue;
    }
    if (found != NULL) {
        delete found;
        found = NULL;
    }

    CAIScript* chain[6] = {
        m_special1Script,
        m_teamScript,
        m_special2Script,
        m_combatScript,
        m_special3Script,
        m_movementScript,
    };

    SHORT scriptLevel = 0;
    for (int i = 0; i < 6 && bTryNextScript; ++i) {
        scriptLevel = (SHORT)(i + 1);
        if (chain[i] == NULL || chain[i]->IsEmpty()) {
            continue;
        }
        found = chain[i]->Find(m_pendingTriggers, this);
        if (found == NULL) {
            continue;
        }
        localResponse.Add(*found);
        BOOL hasContinue = localResponse.InListEnd(CAIAction::CONTINUE);
        if (!found->m_actionList.IsEmpty() && !hasContinue) {
            bTryNextScript = FALSE;
        }
        delete found;
        found = NULL;
    }

    if (!localResponse.m_actionList.IsEmpty()) {
        localResponse.m_scriptNum = scriptLevel;
        InsertResponse(localResponse, TRUE, TRUE);

        POSITION pos = m_pendingTriggers.GetHeadPosition();
        while (pos != NULL) {
            CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
            if (pTrigger != NULL) {
                delete pTrigger;
            }
        }
        m_pendingTriggers.RemoveAll();
    }
    if (found != NULL) {
        delete found;
    }


}

// 0x45D130
void CGameAIBase::RemoveFromArea()
{
    CGameObject::RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameAIBase.cpp
        // __LINE__: 6886
        UTIL_ASSERT(FALSE);
    }

    delete this;
}

// 0x44CC70
void CGameAIBase::AddAction(const CAIAction& action)
{
    CAIAction* copy = new CAIAction();
    copy->m_actionID = action.m_actionID;
    copy->m_specificID = action.m_specificID;
    copy->m_actorID.Set(action.m_actorID);
    copy->m_acteeID.Set(action.m_acteeID);
    copy->m_acteeID2.Set(action.m_acteeID2);
    copy->m_dest = action.m_dest;
    copy->m_specificID2 = action.m_specificID2;
    copy->m_specificID3 = action.m_specificID3;
    copy->m_string1 = action.m_string1;
    copy->m_string2 = action.m_string2;
    copy->m_internalFlags = action.m_internalFlags;
    m_queuedActions.AddHead(copy);
}

// 0x44CE20
void CGameAIBase::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    // The base handler discards effects on AI objects without a specific override.
    // CGameSprite overrides slot 0x78 (0x733050) with timed/equipped list management;
    // doors, containers and triggers also have distinct handlers in the binary.
    // CGameDoor's handler (0x486860) handles knock, trap detection, forced casting
    // and summoning; it does not store arbitrary sprite effects.
    // The earlier stopgap wrongly ran the sprite
    // list path in this base, static_cast-ing a non-sprite to CGameSprite and
    // AddTail-ing onto a garbage list -- the Fireball-on-non-sprite heap fault
    // (GatherTargets(ANYONE) strikes any blast object; the original discards here).
    if (pEffect != NULL) {
        delete pEffect;
    }
}

// 0x44CE40
void CGameAIBase::ClearAI(BOOLEAN a1)
{
    ClearActions(FALSE);
    ApplyTriggers();

    POSITION pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if (pTrigger != NULL) {
            delete pTrigger;
        }
    }
    m_pendingTriggers.RemoveAll();

    SetCurrAction(CAIAction::NULL_ACTION);

    m_curAction.m_actionID = CAIAction::NULL_ACTION.m_actionID;
    m_curAction.m_specificID = CAIAction::NULL_ACTION.m_specificID;
    m_curAction.m_actorID.Set(CAIAction::NULL_ACTION.m_actorID);
    m_curAction.m_acteeID.Set(CAIAction::NULL_ACTION.m_acteeID);
    m_curAction.m_acteeID2.Set(CAIAction::NULL_ACTION.m_acteeID2);
    m_curAction.m_dest = CAIAction::NULL_ACTION.m_dest;
    m_curAction.m_specificID2 = CAIAction::NULL_ACTION.m_specificID2;
    m_curAction.m_specificID3 = CAIAction::NULL_ACTION.m_specificID3;
    m_curAction.m_string1 = CAIAction::NULL_ACTION.m_string1;
    m_curAction.m_string2 = CAIAction::NULL_ACTION.m_string2;
    m_curAction.m_internalFlags = CAIAction::NULL_ACTION.m_internalFlags;
}

// 0x44CF50
void CGameAIBase::InsertAction(const CAIAction& action)
{
    CAIAction* copy = new CAIAction();
    copy->m_actionID = action.m_actionID;
    copy->m_specificID = action.m_specificID;
    copy->m_actorID.Set(action.m_actorID);
    copy->m_acteeID.Set(action.m_acteeID);
    copy->m_acteeID2.Set(action.m_acteeID2);
    copy->m_dest = action.m_dest;
    copy->m_specificID2 = action.m_specificID2;
    copy->m_specificID3 = action.m_specificID3;
    copy->m_string1 = action.m_string1;
    copy->m_string2 = action.m_string2;
    copy->m_internalFlags = action.m_internalFlags;
    m_queuedActions.AddTail(copy);
}

// 0x45D190
void CGameAIBase::SetCurrAction(const CAIAction& action)
{
    m_actionCount = 0;
    m_interrupt = FALSE;
    m_curAction.m_actionID = action.m_actionID;
    m_curAction.m_specificID = action.m_specificID;
    m_curAction.m_actorID.Set(action.m_actorID);
    m_curAction.m_acteeID.Set(action.m_acteeID);
    m_curAction.m_acteeID2.Set(action.m_acteeID2);
    m_curAction.m_dest = action.m_dest;
    m_curAction.m_specificID2 = action.m_specificID2;
    m_curAction.m_specificID3 = action.m_specificID3;
    m_curAction.m_string1 = action.m_string1;
    m_curAction.m_string2 = action.m_string2;
    m_curAction.m_internalFlags = action.m_internalFlags;
    if (action.m_actionID != CAIAction::NO_ACTION
        && g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lNoDecodeList.Find(action.m_actionID) == NULL) {
        m_curAction.Decode(this);
    }
}

// 0x45D280
void CGameAIBase::SetScript(SHORT level, CAIScript* script)
{
    switch (level) {
    case 0:
        if (m_overrideScript != NULL) {
            delete m_overrideScript;
        }
        m_overrideScript = script;
        break;
    case 1:
        if (m_special1Script != NULL) {
            delete m_special1Script;
        }
        m_special1Script = script;
        break;
    case 2:
        if (m_teamScript != NULL) {
            delete m_teamScript;
        }
        m_teamScript = script;
        break;
    case 3:
        if (m_special2Script != NULL) {
            delete m_special2Script;
        }
        m_special2Script = script;
        break;
    case 4:
        if (m_combatScript != NULL) {
            delete m_combatScript;
        }
        m_combatScript = script;
        break;
    case 5:
        if (m_special3Script != NULL) {
            delete m_special3Script;
        }
        m_special3Script = script;
        break;
    case 6:
        if (m_movementScript != NULL) {
            delete m_movementScript;
        }
        m_movementScript = script;
        break;
    }
}

// 0x45D6A0
void CGameAIBase::ApplyTriggers()
{
    CMessage* message;

    if ((g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime / 4) % 900 == 0) {
        message = new CMessageUpdateReaction(11, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    POSITION pos = m_pendingTriggers.GetHeadPosition();
    while (pos != NULL) {
        CAITrigger* pTrigger = m_pendingTriggers.GetNext(pos);
        if ((pTrigger->m_flags & 0x4) == 0) {
            pTrigger->m_flags |= 0x4;

            switch (pTrigger->m_triggerID) {
            case CAITRIGGER_ATTACKEDBY:
                AutoPause(2);
                if (!m_lAttacker.Equal(pTrigger->GetCause())
                    && (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1
                        || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pTrigger->GetCause().GetInstance()) != -1)) {
                    m_lAttacker.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_ATTACKEDBY,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_HELP:
                if (!m_lHelp.Equal(pTrigger->GetCause())) {
                    m_lHelp.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_HELP,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_RECEIVEDORDER:
                if (!m_lOrderedBy.Equal(pTrigger->GetCause())) {
                    m_lOrderedBy.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(), 6, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_SAID:
                if (!m_lTalkedTo.Equal(pTrigger->GetCause())) {
                    m_lTalkedTo.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_SAID,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_HITBY:
                m_lAttackStyle = pTrigger->m_specificID;
                if (!m_lHitter.Equal(pTrigger->GetCause())) {
                    m_lHitter.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_HITBY,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            case CAITRIGGER_HEARD:
                if (!m_lHeard.Equal(pTrigger->GetCause())) {
                    m_lHeard.Set(pTrigger->GetCause());
                    message = new CMessageSetLastObject(pTrigger->GetCause(),
                        CAITRIGGER_HEARD,
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
                break;
            }

            if (g_pBaldurChitin->GetObjectGame()->m_saveObjectList.Find(pTrigger->m_triggerID) != NULL
                && !m_lTrigger.Equal(pTrigger->GetCause())) {
                m_lTrigger.Set(pTrigger->GetCause());
                message = new CMessageSetLastObject(pTrigger->GetCause(), 0, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    }
}

// 0x45DED0
BOOL CGameAIBase::CheckAppropriateTarget(CSpell* pSpell, CGameAIBase* pTarget)
{

    pSpell->Demand();
    if (pTarget->GetObjectType() != TYPE_SPRITE) {
        pSpell->Release();
        return FALSE;
    }

    SPELL_ABILITY* ability = pSpell->GetAbility(0);
    if (ability != NULL
        && (ability->actionType != 1
            || (static_cast<CGameSprite*>(pTarget)->GetDerivedStats()->m_generalState & 0x800) == 0
            || pSpell->GetGenericName() == 12117
            || pSpell->GetGenericName() == 25765
            || pSpell->GetGenericName() == 32393)) {
        pSpell->Release();
        return TRUE;
    }

    pSpell->Release();
    return FALSE;
}

// 0x45DF70
BOOL CGameAIBase::PartyHasItem(const CResRef& resRef)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); nPortrait++) {
        LONG nCharacterId = pGame->GetCharacterId(nPortrait);

        CGameSprite* pSprite;
        BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            break;
        }

        CString sName;
        resRef.CopyToString(sName);

        if (pSprite->FindItemPersonal(sName, 0, FALSE) != -1) {
            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return TRUE;
        }

        if (pSprite->FindItemBags(sName, 0, FALSE) != -1) {
            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return TRUE;
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
    return FALSE;
}

// 0x45E100
void CGameAIBase::ApplyEffectToParty(CGameEffect* pEffect)
{
    CMessage* message;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); nPortrait++) {
        LONG nCharacterId = pGame->GetCharacterId(nPortrait);
        if (pGame->GetGameSave()->m_bArenaMode) {
            CGameSprite* pSprite;
            BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if (pSprite->InControl()) {
                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    message = new CMessageAddEffect(pEffect->Copy(), m_id, nCharacterId);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                } else {
                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        } else {
            message = new CMessageAddEffect(pEffect->Copy(), m_id, nCharacterId);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }
}

// 0x4C83F0
SHORT CGameAIBase::GiveItemCreate(CGameSprite* pTarget)
{
    // Unrecovered.  Transfers this object's items to pTarget: when this is a
    // container (TYPE_CONTAINER) it drains the container's contents, and when
    // this is a sprite (TYPE_SPRITE) it hands over the relevant inventory,
    // delivering each item with a CMessageContainerAddItem (target container)
    // or the give-to-sprite message.  The GiveItemCreate action (ExecuteAction)
    // builds the items into a temporary container, then calls this to deliver
    // them to the action's target.  Body left unimplemented.
    return ACTION_DONE;
}

// 0x799E60
void CGameAIBase::AutoPause(DWORD type)
{
}

// 0x49FC40
BOOL CGameAIBase::GetCanSeeInvisible()
{
    return TRUE;
}

// 0x45F2A0
SHORT CGameAIBase::MoveView(CPoint dest, int speed)
{
    CGameArea* pVisibleArea = g_pBaldurChitin->GetObjectGame()->GetVisibleArea();

    INT x;
    INT y;
    pVisibleArea->GetInfinity()->GetViewPosition(x, y);

    if (dest.x < 0) {
        dest.x = 0;
    }

    if (dest.y < 0) {
        dest.y = 0;
    }

    int maxX = pVisibleArea->GetInfinity()->rViewPort.left
        - pVisibleArea->GetInfinity()->rViewPort.right
        + pVisibleArea->GetInfinity()->nAreaX;
    if (dest.x > maxX) {
        dest.x = maxX;
    }

    int maxY = pVisibleArea->GetInfinity()->rViewPort.top
        - pVisibleArea->GetInfinity()->rViewPort.bottom
        + pVisibleArea->GetInfinity()->nAreaY;
    if (dest.y > maxY) {
        dest.y = maxY;
    }

    if (m_curAction.m_actionID == CAIACTION_MOVEVIEWPOINTUNTILDONE) {
        if (!field_594) {
            CMessageStartScroll* pMessage = new CMessageStartScroll(pVisibleArea,
                CPoint(x, y),
                dest,
                static_cast<BYTE>(speed),
                m_id,
                m_id);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            field_594 = TRUE;
        }

        if (x != dest.x || y != dest.y) {
            return ACTION_NORMAL;
        }

        field_594 = FALSE;
    } else {
        CMessageStartScroll* pMessage = new CMessageStartScroll(pVisibleArea,
            CPoint(x, y),
            dest,
            static_cast<BYTE>(speed),
            m_id,
            m_id);

        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x45F5D0
SHORT CGameAIBase::MoveViewPoint()
{
    CPoint dest(m_curAction.m_dest);

    CRect viewPort(GetArea()->GetInfinity()->rViewPort);
    dest.x -= viewPort.Width() / 2;
    dest.y -= viewPort.Height() / 2;

    if (dest.x < 0) {
        dest.x = 0;
    }

    if (dest.x > GetArea()->GetInfinity()->nAreaX) {
        dest.x = GetArea()->GetInfinity()->nAreaX;
    }

    if (dest.y < 0) {
        dest.y = 0;
    }

    if (dest.y > GetArea()->GetInfinity()->nAreaY) {
        dest.y = GetArea()->GetInfinity()->nAreaY;
    }

    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    return MoveView(dest, speed);
}

// 0x45F660
SHORT CGameAIBase::MoveViewObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest(target->GetPos());

    CRect viewPort(GetArea()->GetInfinity()->rViewPort);
    dest.x -= viewPort.Width() / 2;
    dest.y -= viewPort.Height() / 2;

    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    return MoveView(dest, speed);
}

// 0x45F6D0
SHORT CGameAIBase::MoveCursor(CPoint dest, SHORT speed)
{
    CInfinity* pInfinity = GetArea()->GetInfinity();

    INT x;
    INT y;
    pInfinity->GetViewPosition(x, y);

    CPoint pt;
    GetCursorPos(&pt);

    const SHORT nCursorStep = 4;
    const SHORT nDiagonalStep = static_cast<SHORT>(3 * nCursorStep / 4);
    const SHORT nDiagonalTolerance = static_cast<SHORT>(nDiagonalStep * speed);

    if (m_curAction.m_specificID == 0) {
        pInfinity->SetViewPosition(dest.x, dest.y, TRUE);
        return ACTION_DONE;
    }

    const SHORT nStraightStep = static_cast<SHORT>(nCursorStep * speed);
    const SHORT nHalfStep = static_cast<SHORT>(nCursorStep / 2 * speed);
    const INT dx = dest.x - pt.x;
    const INT dy = dest.y - pt.y;

    if (dx <= nStraightStep) {
        if (dx >= -nStraightStep
            && dy <= nDiagonalTolerance
            && dy >= -nDiagonalTolerance) {
            return ACTION_DONE;
        }

        if (dx >= -nStraightStep) {
            if (dy > 0) {
                pt.y += nDiagonalStep * speed;
            } else if (dy < 0) {
                pt.y -= nDiagonalStep * speed;
            }

            pInfinity->SetViewPosition(pt.x, pt.y, TRUE);
            return ACTION_INTERRUPTABLE;
        }
    }

    if (dx > 0 && dy <= 6 && dy >= -nDiagonalTolerance) {
        pt.x += nCursorStep * speed;
    } else if (dx < 0 && dy <= 6 && dy >= -nDiagonalTolerance) {
        pt.x -= nCursorStep * speed;
    } else if (dx > 0 && dy > 0) {
        pt.x += nDiagonalStep * speed;
        pt.y += nHalfStep;
    } else if (dx < 0 && dy > 0) {
        pt.x -= nDiagonalStep * speed;
        pt.y += nHalfStep;
    } else if (dx > 0 && dy < 0) {
        pt.x += nDiagonalStep * speed;
        pt.y -= nHalfStep;
    } else if (dx < 0 && dy < 0) {
        pt.x -= nDiagonalStep * speed;
        pt.y -= nHalfStep;
    }

    pInfinity->SetViewPosition(pt.x, pt.y, TRUE);

    return ACTION_INTERRUPTABLE;
}

// 0x45F900
SHORT CGameAIBase::MoveCursorPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    return MoveCursor(dest, speed);
}

// NOTE: Inlined.
SHORT CGameAIBase::ClickLButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnLButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonUp(pt);
    return ACTION_DONE;
}

// 0x45F920
SHORT CGameAIBase::ClickLButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickLButton(dest);
}

// 0x45F980
SHORT CGameAIBase::ClickLButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickLButton(dest);
}

// NOTE: Inlined.
SHORT CGameAIBase::DoubleClickLButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnLButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonUp(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonDblClk(pt);
    g_pBaldurChitin->m_pEngineWorld->OnLButtonUp(pt);
    return ACTION_DONE;
}

// 0x45F9F0
SHORT CGameAIBase::DoubleClickLButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickLButton(dest);
}

// 0x45FA70
SHORT CGameAIBase::DoubleClickLButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickLButton(dest);
}

// NOTE: Inlined.
SHORT CGameAIBase::ClickRButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnRButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonUp(pt);
    return ACTION_DONE;
}

// 0x45FB10
SHORT CGameAIBase::ClickRButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickRButton(dest);
}

// 0x45FB70
SHORT CGameAIBase::ClickRButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return ClickRButton(dest);
}

// NOTE: Inlined.
SHORT CGameAIBase::DoubleClickRButton(CPoint pt)
{
    g_pBaldurChitin->m_pEngineWorld->OnRButtonDown(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonUp(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonDblClk(pt);
    g_pBaldurChitin->m_pEngineWorld->OnRButtonUp(pt);
    return ACTION_DONE;
}

// 0x45FBF0
SHORT CGameAIBase::DoubleClickRButtonPoint()
{
    CPoint dest = m_curAction.m_dest;
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickRButton(dest);
}

// 0x45FC80
SHORT CGameAIBase::DoubleClickRButtonObject(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CPoint dest = target->GetPos();
    SHORT speed = static_cast<SHORT>(m_curAction.m_specificID);

    SHORT moveReturn = MoveCursor(dest, speed);
    if (moveReturn != ACTION_DONE) {
        return moveReturn;
    }

    // NOTE: Uninline.
    return DoubleClickRButton(dest);
}

// 0x45FD20
SHORT CGameAIBase::ChangeAIScript()
{
    CAIScript* script = new CAIScript(CResRef(m_curAction.GetString1()));
    SetScript(static_cast<SHORT>(m_curAction.m_specificID), script);

    if (GetObjectType() == TYPE_CONTAINER) {
        static_cast<CGameContainer*>(this)->SetScriptRes(m_curAction.GetString1());
    }

    return ACTION_DONE;
}

// 0x45FED0
SHORT CGameAIBase::StartTimer()
{
    BYTE id = static_cast<BYTE>(m_curAction.m_specificID);
    LONG time = m_curAction.m_specificID2;

    POSITION pos = m_timers.GetHeadPosition();
    while (pos != NULL) {
        CGameTimer* pTimer = m_timers.GetNext(pos);
        if (pTimer->m_id == id) {
            pTimer->m_time = time;
            return ACTION_DONE;
        }
    }

    CGameTimer* pTimer = new CGameTimer();
    pTimer->m_time = time;
    pTimer->m_id = id;
    m_timers.AddTail(pTimer);

    return ACTION_DONE;
}

// 0x45FF40
SHORT CGameAIBase::SendTrigger(CGameAIBase* sprite)
{
    if (sprite == NULL) {
        return ACTION_ERROR;
    }

    CAITrigger trigger(CAITrigger::TRIGGER, m_typeAI, m_curAction.m_specificID);

    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
        m_id,
        sprite->GetId());

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4600B0
SHORT CGameAIBase::Wait()
{
    if (m_actionCount == 0) {
        m_curAction.m_specificID *= 15;
    }

    m_curAction.m_specificID--;

    if (m_curAction.m_specificID > 0) {
        return ACTION_INTERRUPTABLE;
    }

    return ACTION_DONE;
}

// 0x4600F0
SHORT CGameAIBase::SmallWait()
{
    m_curAction.m_specificID--;

    if (m_curAction.m_specificID > 0) {
        return ACTION_INTERRUPTABLE;
    }

    return ACTION_DONE;
}

// 0x460110
SHORT CGameAIBase::Shout()
{
    CTypedPtrList<CPtrList, LONG*> targets;

    SHORT range = m_curAction.m_actionID != CAIACTION_212
        ? GetVisualRange()
        : SHORT_MAX;

    m_pArea->GetAllInRange(m_pos,
        CAIObjectType::ANYONE,
        range,
        GetTerrainTable(),
        targets,
        FALSE,
        FALSE);

    CAITrigger trigger(CAITrigger::HEARD, m_typeAI, m_curAction.m_specificID);

    POSITION pos = targets.GetHeadPosition();
    while (pos != NULL) {
        LONG nId = reinterpret_cast<LONG>(targets.GetNext(pos));

        CMessage* message = new CMessageSetTrigger(trigger, m_id, nId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return ACTION_DONE;
}

// 0x463310
void CGameAIBase::PutItemGround(CItem* pItem)
{
    LONG nContainerId = g_pBaldurChitin->GetObjectGame()->GetGroundPile(m_id);
    if (nContainerId != CGameObjectArray::INVALID_INDEX) {
        CGameContainer* pContainer;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            pContainer->PlaceItemInBlankSlot(pItem, TRUE, SHORT_MAX);
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nContainerId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x463740
SHORT CGameAIBase::ClearActions(CGameObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CMessage* message = new CMessageClearActions(m_id, target->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    return ACTION_DONE;
}

// 0x45EDE0 - handles SetGlobal (0x1E) and SetGlobalRandom (0x132).
// TODO: Multiplayer broadcast (CMessage at vtable PTR_FUN_0084882c) skipped.
SHORT CGameAIBase::SetGlobal()
{
    CString sScope;
    CString sName;
    SplitScriptVariableName(m_curAction.GetString1(), sScope, sName);
    LONG nValue = m_curAction.m_specificID;

    if (m_curAction.m_actionID == 0x132) {
        LONG lo = m_curAction.m_specificID;
        LONG hi = m_curAction.m_specificID2;
        if (lo < hi) {
            nValue = rand() % (hi - lo + 1) + lo;
        } else if (lo > hi) {
            nValue = rand() % (lo - hi + 1) + hi;
        } else {
            nValue = lo;
        }
        m_curAction.m_specificID = nValue;
    }

    if (sScope == CString("GLOBAL")) {
        CVariableHash* pHash = g_pBaldurChitin->GetObjectGame()->GetVariables();
        CVariable* pVar = pHash->FindKey(sName);
        if (pVar != NULL) {
            pVar->m_intValue = nValue;
        } else {
            CVariable v;
            v.SetName(sName);
            v.m_intValue = nValue;
            pHash->AddKey(v);
        }
        return ACTION_DONE;
    }

    if (sScope == CString("LOCALS")) {
        if ((GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariableHash* pHash = pSprite->GetLocalVariables();
            CVariable* pVar = pHash->FindKey(sName);
            if (pVar != NULL) {
                pVar->m_intValue = nValue;
            } else {
                CVariable v;
                v.SetName(sName);
                v.m_intValue = nValue;
                pHash->AddKey(v);
            }
        }
        return ACTION_DONE;
    }

    CString sAreaName = sScope;
    if (sScope == CString("MYAREA") && m_pArea != NULL) {
        sAreaName = CResRef(reinterpret_cast<BYTE*>(m_pArea->m_header.m_areaName)).GetResRefStr();
    }

    CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sAreaName);
    if (pArea != NULL) {
        CVariableHash* pHash = pArea->GetVariables();
        CVariable* pVar = pHash->FindKey(sName);
        if (pVar != NULL) {
            pVar->m_intValue = nValue;
        } else {
            CVariable v;
            v.SetName(sName);
            v.m_intValue = nValue;
            pHash->AddKey(v);
        }
    }

    return ACTION_DONE;
}

// Decodes a numeric spell id into a resref string of the form
// "SP" + family + "%03d" where family is one of:
//   1xxx -> SPPR (priest)
//   2xxx -> SPWI (wizard / arcane)
//   3xxx -> SPIN (innate)
// When spellId is 0 the helper falls back to field_588 (caster's pending
// spell id); if both are zero the output is the empty string.  Invalid
// family ids fall through to an empty prefix and the binary logs an
// "Invalid or missing spell ID" assert -- skipped here.
// 0x45D3E0
void CGameAIBase::SpellIdToResRef(int spellId, CString& outResRef)
{
    if (spellId == 0 && field_588 == 0) {
        outResRef = "";
        return;
    }
    if (spellId == 0) {
        spellId = field_588;
    }

    outResRef = "SP";
    int family = spellId / 1000;
    if (family == 1) {
        outResRef += "PR";
    } else if (family == 2) {
        outResRef += "WI";
    } else if (family == 3) {
        outResRef += "IN";
    }

    CString suffix;
    suffix.Format("%03d", spellId % 1000);
    outResRef += suffix;
}

// 0x461190 - ForceSpell (0x71) and ReallyForceSpell (0xB5) action handler.
// This step wires the cast-animation state machine -- the binary's three
// visual stages now drive CMessageSetSequence on the caster so the sprite
// raises hands during the wind-up and fires the cast-burst pose on the
// release tick.  ApplyCastingEffect moves from the fire branch to the
// first cast tick to match the binary (pre-cast SPL feature blocks run
// once at cast start, not at completion).
//
// Recovered field offsets on caster (param_1 in the decomp):
//   +0x25  -> m_typeAI.m_nGeneral (binary compares against DAT_00847C48,
//             the G_DEAD marker written by 0x4ACFB2 / 0x70D4F6 when
//             m_baseStats.m_generalState has STATE_DEAD bit 0x800)
//   +0x33  -> m_typeAI.m_nSpecific (used as effect.casterParty in case 6)
//   +0x11D -> m_actionCount (cast-frame counter; CGameAIBase 0x474)
//   +0x14D2-> m_nSequence (CGameSprite animation sequence; 0x5348)
//   +0x283 -> m_derivedStats.m_spellStates dword0 (bit 0x100000 == lost
//             concentration; CGameSprite 0xA0C)
//   +0x476 -> m_curAction.m_actionID (only the 0xB5 fast-path check uses it)
//   +0x966 -> m_derivedStats.m_nLevel (CGameSprite 0x920+0x46)
//
// Sequence ids match the DAT_0085BBB2/DAT_0085BBB3 byte constants:
//   SEQ_CAST    = 2 (DAT_0085BBB2, cast-burst pose)
//   SEQ_CONJURE = 3 (DAT_0085BBB3, raise-hands wind-up)
//
// Helpers still pending (see memory/project_forcespell_recovery.md):
//   FUN_0051EAF0 - projectile/visual-effect factory (~5400 lines).  Until
//                  recovered the binary's projectile launch path
//                  (CMessageFireProjectile + CProjectile::Launch vfn at
//                  +0x6c) cannot fire.  targetType 2 effects are applied
//                  directly to explicit object targets as a gameplay
//                  fallback, but the visible bolt is absent.
//   FUN_00727720 - per-class memorization lookup; supplies the byte the
//                  binary parks at projectile +0x186 (caster-class index
//                  used by saving-throw + damage scaling).
//   FUN_00727B80 - silence/spell-state filter consulted on the first cast
//                  tick before queueing the concentration / dispel
//                  ITEM_EFFECT broadcasts.
SHORT CGameAIBase::ForceSpellAction(CGameObject* target)
{
    // Dead casters short-circuit (binary 0x4611BA).
    if (m_typeAI.GetGeneral() == CAIObjectType::G_DEAD) {
        return ACTION_DONE;
    }
    if (target == NULL) {
        return ACTION_INTERRUPTABLE;
    }

    // The caster move-to-range and orient gates run AFTER the spell + ability
    // are resolved (the chosen ability's range drives the approach distance) --
    // see the combined gate below, just before the cast-time machine.

    // Resolve resref + cast level.  Script can pass either a CString in
    // m_string1 (with cast level in m_specificID) or a numeric spell id in
    // m_specificID alone (cast level falls back to caster's m_nLevel).
    SHORT specificLevel;
    CResRef resRef;
    CString sStr1 = m_curAction.GetString1();
    if (sStr1.IsEmpty()) {
        CString sFromId;
        SpellIdToResRef(m_curAction.m_specificID, sFromId);
        if (sFromId.IsEmpty()) {
            return ACTION_INTERRUPTABLE;
        }
        resRef = sFromId;
        if (GetObjectType() == CGameObject::TYPE_SPRITE) {
            specificLevel = static_cast<CGameSprite*>(this)
                                ->GetDerivedStats()
                                ->m_nLevel;
        } else {
            specificLevel = 1;
        }
    } else {
        resRef = sStr1;
        // Spell (0x1F) / SpellPoint (0x5F) cast from the UI carry the resref in
        // String1 with specificID == 0; their caster level is the sprite's own
        // level. The real Spell handlers (ExecuteAction case 0x9e -> the cast
        // state machine) are unrecovered, so this stopgap -- which routes Spell
        // through the ForceSpell handler -- derives the level here. ForceSpell
        // (0x71) / ForceSpellPoint (0x72) and their Really/NoDec variants keep
        // the script-supplied level in specificID.
        if (m_curAction.m_actionID == CAIAction::FORCESPELL
            || m_curAction.m_actionID == CAIAction::REALLYFORCESPELL
            || m_curAction.m_actionID == CAIAction::FORCESPELLPOINT) {
            specificLevel = static_cast<SHORT>(m_curAction.m_specificID);
        } else if (GetObjectType() == CGameObject::TYPE_SPRITE) {
            specificLevel = static_cast<CGameSprite*>(this)
                                ->GetDerivedStats()
                                ->m_nLevel;
        } else {
            specificLevel = 1;
        }
    }
    if (specificLevel < 2) {
        specificLevel = 1;
    }

    // Binary heap-allocates a 16-byte CResHelper<CResSpell,1006> (== CSpell)
    // with SetResRef + auto-request, then Demand() to make sure the .SPL is
    // resident.
    CSpell* pSpell = new CSpell();
    if (pSpell == NULL) {
        return ACTION_INTERRUPTABLE;
    }
    pSpell->SetResRef(resRef, TRUE, TRUE);
    if (!pSpell->Demand()) {
        delete pSpell;
        return ACTION_INTERRUPTABLE;
    }

    // 0xB5 (ReallyForceSpell) skips the target-appropriate gate.
    if (m_curAction.m_actionID != 0xB5
        && (target->GetObjectType() & CGameObject::TYPE_AIBASE) != 0
        && !CheckAppropriateTarget(pSpell, static_cast<CGameAIBase*>(target))) {
        pSpell->Release();
        delete pSpell;
        return ACTION_INTERRUPTABLE;
    }

    // Pick the ability whose minCasterLevel <= specificLevel.  Binary scans
    // forward, counts qualifying entries, then re-fetches at (count - 1).
    LONG abilityCount = pSpell->GetAbilityCount();
    SHORT nAbilityIndex = -1;
    for (LONG i = 0; i < abilityCount; ++i) {
        SPELL_ABILITY* pCheck = pSpell->GetAbility(i);
        if (pCheck == NULL
            || pCheck->minCasterLevel > static_cast<WORD>(specificLevel)) {
            break;
        }
        nAbilityIndex++;
    }
    SPELL_ABILITY* pAbility = pSpell->GetAbility(nAbilityIndex);
    if (pAbility == NULL) {
        pSpell->Release();
        delete pSpell;
        return ACTION_INTERRUPTABLE;
    }

    // Cast-time gate.  speedFactor is the per-ability cast time in 1/10ths
    // of an AI tick; the binary computes castTime = speedFactor*100/10 ==
    // *10 and walks the caster through three visual stages:
    //   m_actionCount == 0           : first tick -- queue the SPL pre-cast
    //                                  feature blocks (ApplyCastingEffect)
    //                                  and emit concentration/dispel
    //                                  ITEM_EFFECT broadcasts if the caster
    //                                  is silenced or has lost concentration
    //   m_actionCount < castTime - 4 : raise-hands (SEQ_CONJURE)
    //   m_actionCount < castTime     : cast-burst (SEQ_CAST) +
    //                                  CGameSprite::ApplyCastingEffectPost
    //   otherwise                    : fire and complete.
    // The stage-1/2 branches return ACTION_INTERRUPTABLE so the action
    // executor re-enters this case next tick.  ReallyForceSpell (0xB5)
    // and non-sprite callers skip the cast-time machine entirely and
    // resolve in a single tick (binary 0x46139A check on +0x476).
    BOOL isSprite = (GetObjectType() & CGameObject::TYPE_SPRITE) != 0;
    BOOL bInstantCast = !isSprite || m_curAction.m_actionID == 0xB5;
    CGameSprite* pSprite = isSprite ? static_cast<CGameSprite*>(this) : NULL;
    CPoint targetPos = target->GetPos();

    // --- Move-to-range + caster orient gate (binary FUN_00740270). ----------
    // Only the normal UI casts (Spell 0x1F / SpellNoDec 0xBF) walk and turn;
    // the force actions (ForceSpell 0x71 / ReallyForceSpell 0xB5 /
    // ForceSpellPoint 0x72) deliberately fire in place.  Both gates run here --
    // after the ability is chosen, before the cast-time machine -- in approach-
    // then-face order: the walk takes priority so the orient gate never preempts
    // the path (a turn-vs-walk fight that would never make progress).  The
    // original runs face-then-range with a free-running cast counter; this
    // stopgap pins the counter (see below), so the order is swapped to keep the
    // walk alive.
    if (isSprite
        && m_curAction.m_actionID != CAIAction::FORCESPELL
        && m_curAction.m_actionID != CAIAction::REALLYFORCESPELL
        && m_curAction.m_actionID != CAIAction::FORCESPELLPOINT) {
        // (1) Move-to-range + line-of-sight.  FUN_00740270 fires only when the
        // target is BOTH within the chosen ability's range
        // (CGameSprite::CheckCastingRange, 0x7408B6) AND in line of sight
        // (CGameArea::CheckLOS, 0x7408DC); a spell
        // whose header flag 0x800 is set bypasses the LOS half (0x7408E5).  When
        // neither holds it walks the caster toward the target via FUN_0073EDD0
        // (== CGameSprite::MoveToObject, 0x740A37) and re-enters next tick.
        // CheckLOS fails not only on blocking terrain but also when the target is
        // beyond the caster's sight range (its leading distance gate,
        // CGameArea.cpp:498) -- so a target sitting inside a spell's generous range
        // but past sight is still approached first (the runtime-traced gap: a
        // creature inside Magic Missile's 50-square range yet past sight still
        // made the original walk while our range-only gate cast in place).  range
        // == 0xFFFF (-1) is unbounded (matches CheckCastingRange's -1 short test); the
        // grid-square distance carries a +2 slack; self casts never walk.  The
        // whole decision is latched to m_actionCount <= 0 -- the binary's +0x54EA
        // "casting begun" flag, set once the cast machine runs -- so a target that
        // steps out of sight mid-cast cannot re-trigger the walk and restart the
        // cast.
        if (m_actionCount <= 0
            && target != static_cast<CGameObject*>(this)) {
            CPoint selfPos = pSprite->GetPos();
            LONG dx = selfPos.x / CPathSearch::GRID_SQUARE_SIZEX
                - targetPos.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG dy = selfPos.y / CPathSearch::GRID_SQUARE_SIZEY
                - targetPos.y / CPathSearch::GRID_SQUARE_SIZEY;
            LONG slack = pAbility->range + 2;
            BOOL bInRange = pAbility->range == 0xFFFF
                || dx * dx + dy * dy <= slack * slack;
            BOOL bCanCast = bInRange
                && (m_pArea->CheckLOS(m_pos, targetPos, GetTerrainTable(), FALSE)
                    || (pSpell->GetItemFlags() & 0x800) != 0);
            if (!bCanCast) {
                pSpell->Release();
                delete pSpell;
                // Pin the action counter (DoAction post-increments -1 -> 0) so
                // the cast-time machine still sees m_actionCount == 0 on its
                // first tick once the approach finishes -- a counter inflated by
                // the walk would skip ApplyCastingEffect and fizzle the cast.
                m_actionCount = -1;
                return pSprite->MoveToObject(target);
            }
            // In range and in sight: drop the approach path that MoveToObject
            // submitted (it paths all the way to personal space, CGameSprite.cpp
            // :14060) so the caster does not coast into melee after the spell
            // fires.  The binary posts the same CMessageDropPath right before it
            // casts (0x7408ED, vtable 0x84C44C, just ahead of LAB_00740922).
            CMessage* pDropPath = new CMessageDropPath(m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pDropPath, FALSE);
        }

        // (2) Orient.  The SEQ_CAST executor compares m_nDirection against
        // GetDirection(the target's live GetPos()) and, while they differ, posts
        // a gradual CMessageSetDirection (-> CGameSprite::SetDirection) and
        // re-enters WITHOUT advancing the cast, so the casting glow is born
        // already facing the target.  The m_actionCount <= 0 guard disables the
        // gate once the cast is counting: otherwise a target that moves mid-cast
        // re-fires the turn and re-pins the counter every tick, restarting the
        // cast forever (the un-guarded 0d1347bf loop).
        if (m_actionCount <= 0
            && pSprite->m_nDirection != pSprite->GetDirection(targetPos)) {
            pSpell->Release();
            delete pSpell;
            pSprite->SetDirection(targetPos);
            m_actionCount = -1;
            return ACTION_INTERRUPTABLE;
        }
    }

    if (!bInstantCast) {
        WORD castTime = static_cast<WORD>(pAbility->speedFactor) * 10;
        SHORT currentSeq = pSprite->m_nSequence;


        // First-tick pre-cast hook.  Binary 0x46139A: queue the SPL's
        // pre-cast feature blocks (visuals, chant, projectile-spawn) before
        // any animation runs so they overlap with the cast-time wind-up.
        if (m_actionCount == 0) {
            pSprite->ApplyCastingEffect(pSpell, pAbility, targetPos);
            // TODO (multi-session): FUN_00727B80 silence/spell-state gate ->
            // concentration ITEM_EFFECT (opcode 0x88) broadcast against
            // target sprite, and m_spellStates bit 0x100000 -> dispel
            // ITEM_EFFECT (opcode 0xA0) on self.  Both queue a
            // CMessageAddEffect via PTR_FUN_008487CC.  No gameplay impact
            // while missing -- the caster simply doesn't visibly lose
            // concentration when struck mid-cast.
        }

        if (m_actionCount < static_cast<SHORT>(castTime - 4)) {
            // Stage 1: raise hands.  Self-queued CMessageSetSequence so the
            // animation system picks up SEQ_CONJURE on the next render
            // (binary 0x461795 path, PTR_FUN_008488C4 vtable -> CMessageSetSequence).
            if (currentSeq != CGameSprite::SEQ_CONJURE) {
                CMessage* msg = new CMessageSetSequence(
                    CGameSprite::SEQ_CONJURE, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            pSpell->Release();
            delete pSpell;
            return ACTION_INTERRUPTABLE;
        }

        if (m_actionCount < static_cast<SHORT>(castTime)) {
            // Stage 2: cast burst.  On entry transition send SEQ_CAST and
            // play the burst sound cue (ApplyCastingEffectPost).
            if (currentSeq != CGameSprite::SEQ_CAST) {
                CMessage* msg = new CMessageSetSequence(
                    CGameSprite::SEQ_CAST, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
                pSprite->ApplyCastingEffectPost(pSpell, pAbility);
            }
            pSpell->Release();
            delete pSpell;
            return ACTION_INTERRUPTABLE;
        }

        // Stage 3 entry: cast-time exhausted.  If the cast was instant
        // enough to skip stage 2 (speedFactor*10 <= 4 + first tick), the
        // burst sequence + sound still need to fire here.
        if (currentSeq != CGameSprite::SEQ_CAST) {
            CMessage* msg = new CMessageSetSequence(
                CGameSprite::SEQ_CAST, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            pSprite->ApplyCastingEffectPost(pSpell, pAbility);
        }
    } else if (isSprite) {
        // ReallyForceSpell single-tick path: still queue the SPL pre-cast
        // feature blocks so visuals and the gameplay 8-way dispatch see a
        // consistent state, but skip every sequence-change message.
        pSprite->ApplyCastingEffect(pSpell, pAbility, targetPos);
    }

    // Fire path -- ability's gameplay-payload effects.  The 8-way targetType
    // dispatch matches binary FUN_00461190 lines 262-339.  Non-sprite
    // callers fall back to the legacy stub (FireSpell) since they don't
    // have an area pointer for the AoE / party cases.
    if (isSprite) {
        BYTE nClass = static_cast<BYTE>(m_curAction.m_specificID2 & 0xFF);
        DWORD nSpec = pSprite->m_baseStats.m_specialization;
        BYTE nLevel = static_cast<BYTE>(specificLevel);

        // Pre-build the projectile (binary FUN_00461190 @0x46137c: DecodeProjectile
        // with the ability's missileType) before the effect dispatch, so the
        // targetType-2 gameplay effects can ride the missile (CProjectile::AddEffect)
        // and deliver on arrival instead of at cast.  Fired directly after the loop.
        CProjectile* pProj = CProjectile::DecodeProjectile(pAbility->missileType, this, 0);

        BYTE nSpellClass = 0;
        DWORD nSpellSpecialization = 0;
        BYTE nSpellLevel = 0;
        BOOL bFoundSpell = pSprite->FindAvailableSpell(resRef, nSpellClass, nSpellSpecialization, nSpellLevel);
        if (pProj != NULL) {
            if (bFoundSpell) {
                pProj->m_nSpellLevel = nSpellLevel;
            }
            pProj->m_casterResRef = resRef;
        }

        for (LONG e = 0; e < static_cast<LONG>(pAbility->effectCount); ++e) {
            CGameEffect* pEffect = pSpell->BuildAbilityEffect(
                nAbilityIndex, e, this, nClass, nSpec, nLevel);
            if (pEffect == NULL) {
                continue;
            }
            pEffect->m_source = pSprite->GetPos();
            pEffect->m_sourceID = m_id;
            pEffect->m_target = targetPos;
            switch (pEffect->m_targetType) {
            case 1: {
                CMessage* msg = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
                continue;
            }
            case 2:
                // targetType 2 (preset target): the effect rides the missile and
                // delivers on arrival (binary case 2 @0x461953: CProjectile::AddEffect
                // on the pre-built projectile).  If the projectile type is unrecovered
                // (pProj NULL), fall back to a direct cast-time application so the
                // spell still has gameplay impact even without the missile.
                if (pProj != NULL) {
                    pProj->AddEffect(pEffect);
                    continue;
                }
                if (target != NULL) {
                    CMessage* msg = new CMessageAddEffect(pEffect, m_id, target->m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
                    continue;
                }
                break;
            case 3:
                pSprite->ApplyEffectToParty(pEffect);
                break;
            case 4:
                pSprite->m_pArea->ApplyEffect(pEffect, FALSE, FALSE, 0, NULL);
                break;
            case 5:
                pSprite->m_pArea->ApplyEffect(pEffect, TRUE, FALSE, 0, NULL);
                break;
            case 6:
                pSprite->m_pArea->ApplyEffect(
                    pEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
                break;
            case 7:
                pSprite->m_pArea->ApplyEffect(
                    pEffect, FALSE, TRUE, GetAIType().m_nSpecific, NULL);
                break;
            case 8:
                pSprite->m_pArea->ApplyEffect(pEffect, FALSE, FALSE, 0, pSprite);
                break;
            default:
                break;
            }
            delete pEffect;
        }

        // Cast-success feedback line -- "<caster> casts <SpellName>".
        // Binary 0x4619E5: only sprite casters emit FEEDBACK_SPELL.
        STRREF strSpellName = pSpell->GetGenericName();
        pSprite->FeedBack(CGameSprite::FEEDBACK_SPELL, 0, 0, 0,
            static_cast<LONG>(strSpellName), 0, 0);

        // Projectile launch (binary FUN_00461190 @0x46146d): fire the pre-built
        // projectile directly via CProjectile::Fire (vtable slot 0x6c, height 0x1e),
        // carrying the targetType-2 effects attached above so they deliver on
        // arrival rather than at cast.  The original ALSO queues a broadcast
        // CMessageFireProjectile here, but its Run() skips on the casting owner and
        // only fires the visual on remote machines -- that MP replication is deferred
        // with the rest of MP. The spell-level stamp at +0x186 is set above.
        // Fire ignores nHeight.
        if (pProj != NULL) {
            LONG nMissileTarget = (target != NULL)
                ? target->m_id : CGameObjectArray::INVALID_INDEX;
            pProj->Fire(pSprite->m_pArea, m_id, nMissileTarget, targetPos, 0x1E, 0);
        }
    } else {
        FireSpell(resRef, target);
    }

    // Consume the memorized slot.  Binary path lives in the
    // FUN_00740270 / FUN_00742840 quickspell wrappers (still TODO) -- here
    // we try the three slot kinds the sprite can own (per-class memorized,
    // cleric/paladin domain, innate) and stop at the first match.
    if (isSprite) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(this);
        SHORT nSpellLevel = pSpell->GetLevel();
        if (nSpellLevel >= 1) {
            UINT nLvl = static_cast<UINT>(nSpellLevel - 1);
            BOOLEAN consumed = FALSE;
            for (UINT nIdx = 0; nIdx < CSPELLLIST_NUM_CLASSES && !consumed; ++nIdx) {
                BYTE nCasterClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nIdx);
                if (nCasterClass == 0) { continue; }
                if (pSprite->SubtractFromSpellCount(nCasterClass, nLvl, resRef, 1, 0)) {
                    consumed = TRUE;
                }
            }
            if (!consumed && pSprite->SubtractFromDomainSpellCount(nLvl, resRef, 1, 0)) {
                consumed = TRUE;
            }
            if (!consumed) {
                pSprite->SubtractFromInnateSpellCount(resRef, 1, 0);
            }
        }
    }

    pSpell->Release();
    delete pSpell;
    return ACTION_DONE;
}

// 0x461B80
SHORT CGameAIBase::ForceSpellPointAction()
{
    if (m_typeAI.GetGeneral() == CAIObjectType::G_DEAD) {
        return ACTION_DONE;
    }

    SHORT specificLevel;
    CResRef resRef;
    CString sStr1 = m_curAction.GetString1();
    if (sStr1.IsEmpty()) {
        CString sFromId;
        SpellIdToResRef(m_curAction.m_specificID, sFromId);
        if (sFromId.IsEmpty()) {
            return ACTION_INTERRUPTABLE;
        }
        resRef = sFromId;
        if (GetObjectType() == CGameObject::TYPE_SPRITE) {
            specificLevel = static_cast<CGameSprite*>(this)
                                ->GetDerivedStats()
                                ->m_nLevel;
        } else {
            specificLevel = 1;
        }
    } else {
        resRef = sStr1;
        // Spell (0x1F) / SpellPoint (0x5F) cast from the UI carry the resref in
        // String1 with specificID == 0; their caster level is the sprite's own
        // level. The real Spell handlers (ExecuteAction case 0x9e -> the cast
        // state machine) are unrecovered, so this stopgap -- which routes Spell
        // through the ForceSpell handler -- derives the level here. ForceSpell
        // (0x71) / ForceSpellPoint (0x72) and their Really/NoDec variants keep
        // the script-supplied level in specificID.
        if (m_curAction.m_actionID == CAIAction::FORCESPELL
            || m_curAction.m_actionID == CAIAction::REALLYFORCESPELL
            || m_curAction.m_actionID == CAIAction::FORCESPELLPOINT) {
            specificLevel = static_cast<SHORT>(m_curAction.m_specificID);
        } else if (GetObjectType() == CGameObject::TYPE_SPRITE) {
            specificLevel = static_cast<CGameSprite*>(this)
                                ->GetDerivedStats()
                                ->m_nLevel;
        } else {
            specificLevel = 1;
        }
    }
    if (specificLevel < 2) {
        specificLevel = 1;
    }

    CSpell* pSpell = new CSpell();
    if (pSpell == NULL) {
        return ACTION_INTERRUPTABLE;
    }
    pSpell->SetResRef(resRef, TRUE, TRUE);
    if (!pSpell->Demand()) {
        delete pSpell;
        return ACTION_INTERRUPTABLE;
    }

    LONG abilityCount = pSpell->GetAbilityCount();
    SHORT nAbilityIndex = -1;
    for (LONG i = 0; i < abilityCount; ++i) {
        SPELL_ABILITY* pCheck = pSpell->GetAbility(i);
        if (pCheck == NULL
            || pCheck->minCasterLevel > static_cast<WORD>(specificLevel)) {
            break;
        }
        nAbilityIndex++;
    }
    SPELL_ABILITY* pAbility = pSpell->GetAbility(nAbilityIndex);
    if (pAbility == NULL) {
        pSpell->Release();
        delete pSpell;
        return ACTION_INTERRUPTABLE;
    }

    BOOL isSprite = (GetObjectType() & CGameObject::TYPE_SPRITE) != 0;
    BOOL bInstantCast = !isSprite;
    CGameSprite* pSprite = isSprite ? static_cast<CGameSprite*>(this) : NULL;
    CPoint targetPos = m_curAction.m_dest;

    if (!bInstantCast) {
        WORD castTime = static_cast<WORD>(pAbility->speedFactor) * 10;
        SHORT currentSeq = pSprite->m_nSequence;


        if (m_actionCount == 0) {
            pSprite->ApplyCastingEffect(pSpell, pAbility, targetPos);
        }

        if (m_actionCount < static_cast<SHORT>(castTime - 4)) {
            if (currentSeq != CGameSprite::SEQ_CONJURE) {
                CMessage* msg = new CMessageSetSequence(
                    CGameSprite::SEQ_CONJURE, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            pSpell->Release();
            delete pSpell;
            return ACTION_INTERRUPTABLE;
        }

        if (m_actionCount < static_cast<SHORT>(castTime)) {
            if (currentSeq != CGameSprite::SEQ_CAST) {
                CMessage* msg = new CMessageSetSequence(
                    CGameSprite::SEQ_CAST, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
                pSprite->ApplyCastingEffectPost(pSpell, pAbility);
            }
            pSpell->Release();
            delete pSpell;
            return ACTION_INTERRUPTABLE;
        }

        if (currentSeq != CGameSprite::SEQ_CAST) {
            CMessage* msg = new CMessageSetSequence(
                CGameSprite::SEQ_CAST, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            pSprite->ApplyCastingEffectPost(pSpell, pAbility);
        }
    }

    if (isSprite) {
        BYTE nClass = static_cast<BYTE>(m_curAction.m_specificID2 & 0xFF);
        DWORD nSpec = pSprite->m_baseStats.m_specialization;
        BYTE nLevel = static_cast<BYTE>(specificLevel);

        // Pre-build the projectile (binary 0x462102: DecodeProjectile with the
        // ability's missileType) before the effect dispatch so the targetType-2
        // gameplay effects can ride the missile (CProjectile::AddEffect) and
        // deliver on arrival instead of at cast.  Fired directly after the loop.
        // This mirrors the object-target twin ForceSpellAction (0x461190); the
        // earlier stopgap here dropped case 2 (no projectile) so projectile
        // spells -- Fireball: damage targetType-2 + missile 38 -- never fired.
        CProjectile* pProj = CProjectile::DecodeProjectile(pAbility->missileType, this, 0);

        BYTE nSpellClass = 0;
        DWORD nSpellSpecialization = 0;
        BYTE nSpellLevel = 0;
        BOOL bFoundSpell = pSprite->FindAvailableSpell(resRef, nSpellClass, nSpellSpecialization, nSpellLevel);
        if (pProj != NULL) {
            if (bFoundSpell) {
                pProj->m_nSpellLevel = nSpellLevel;
            }
            pProj->m_casterResRef = resRef;
        }

        for (LONG e = 0; e < static_cast<LONG>(pAbility->effectCount); ++e) {
            CGameEffect* pEffect = pSpell->BuildAbilityEffect(
                nAbilityIndex, e, this, nClass, nSpec, nLevel);
            if (pEffect == NULL) {
                continue;
            }
            pEffect->m_source = pSprite->GetPos();
            pEffect->m_sourceID = m_id;
            pEffect->m_target = targetPos;
            switch (pEffect->m_targetType) {
            case 1: {
                CMessage* msg = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
                continue;
            }
            case 2:
                // targetType 2 (preset target): the effect rides the missile and
                // delivers on arrival (binary case 2 @0x461953: CProjectile::AddEffect).
                // A point cast has no object to address, so when the projectile type
                // is unrecovered (pProj NULL) the effect is dropped rather than fired
                // at cast.
                if (pProj != NULL) {
                    pProj->AddEffect(pEffect);
                    continue;
                }
                break;
            case 3:
                pSprite->ApplyEffectToParty(pEffect);
                break;
            case 4:
                pSprite->m_pArea->ApplyEffect(pEffect, FALSE, FALSE, 0, NULL);
                break;
            case 5:
                pSprite->m_pArea->ApplyEffect(pEffect, TRUE, FALSE, 0, NULL);
                break;
            case 6:
                pSprite->m_pArea->ApplyEffect(
                    pEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
                break;
            case 8:
                pSprite->m_pArea->ApplyEffect(pEffect, FALSE, FALSE, 0, pSprite);
                break;
            case 7:
            default:
                // targetType 7 (binary @0x46213b): no application, just release.
                break;
            }
            delete pEffect;
        }

        STRREF strSpellName = pSpell->GetGenericName();
        pSprite->FeedBack(CGameSprite::FEEDBACK_SPELL, 0, 0, 0,
            static_cast<LONG>(strSpellName), 0, 0);

        // Projectile launch (binary 0x46146d region): fire the pre-built
        // projectile via CProjectile::Fire (vtable slot 0x6c, height 0x1e),
        // carrying the targetType-2 effects attached above.  A point cast has no
        // target object, so the missile homes on the destination point with an
        // invalid target id.  The original ALSO queues a broadcast
        // CMessageFireProjectile (its Run() only fires the visual on remote
        // machines), deferred with the rest of MP. The spell level and resref
        // are stamped above (binary 0x46214E-0x46217B). Fire ignores nHeight.
        if (pProj != NULL) {
            pProj->Fire(pSprite->m_pArea, m_id, CGameObjectArray::INVALID_INDEX,
                targetPos, 0x1E, 0);
        }
    } else {
        FireSpellPoint(resRef, targetPos);
    }

    if (isSprite) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(this);
        SHORT nSpellLevel = pSpell->GetLevel();
        if (nSpellLevel >= 1) {
            UINT nLvl = static_cast<UINT>(nSpellLevel - 1);
            BOOLEAN consumed = FALSE;
            for (UINT nIdx = 0; nIdx < CSPELLLIST_NUM_CLASSES && !consumed; ++nIdx) {
                BYTE nCasterClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nIdx);
                if (nCasterClass == 0) { continue; }
                if (pSprite->SubtractFromSpellCount(nCasterClass, nLvl, resRef, 1, 0)) {
                    consumed = TRUE;
                }
            }
            if (!consumed && pSprite->SubtractFromDomainSpellCount(nLvl, resRef, 1, 0)) {
                consumed = TRUE;
            }
            if (!consumed) {
                pSprite->SubtractFromInnateSpellCount(resRef, 1, 0);
            }
        }
    }

    pSpell->Release();
    delete pSpell;
    return ACTION_DONE;
}

// 0x45C290 - resolves m_acteeID, dropping sprites whose immunity list names
// this object's CAIObjectType (e.g. spell-immune creature scripted against
// by a matching caster type), and publishes the result through UpdateTarget.
// Returns the target with an ACTIVE GetObjectArray share -- caller must
// ReleaseShare on the returned id.  The dispatcher inlines this body in
// several CGameAIBase::ExecuteAction cases (0x32-0x3A, 0x3E, 0x7D, 0x7E,
// 0x85); src calls the helper there instead.
CGameObject* CGameAIBase::ResolveActionTarget()
{
    CGameObject* pObj = m_curAction.m_acteeID.GetObject(this, FALSE);
    if (pObj != NULL
        && pObj->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pObj)->GetDerivedStats()->m_cImmunitiesAIType.OnList(m_typeAI)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
            pObj->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObj = NULL;
    }

    UpdateTarget(pObj);

    return pObj;
}

// 0x45BDD0 - typed resolver used by most dispatcher cases: same resolve +
// immunity drop as the untyped overload, then filters by nObjectType
// (TYPE_NONE accepts anything, TYPE_AIBASE requires only the AIBASE bit,
// any other value must equal GetObjectType exactly) and caps the targeting
// range of blinded non-party sprites at GetPersonalSpace() / 2 + 4 grid
// squares.  Rejected targets are released and re-published as NULL.
CGameObject* CGameAIBase::ResolveActionTarget(BYTE nObjectType)
{
    CGameObject* pObj = m_curAction.m_acteeID.GetObject(this, FALSE);
    if (pObj != NULL
        && pObj->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pObj)->GetDerivedStats()->m_cImmunitiesAIType.OnList(m_typeAI)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
            pObj->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObj = NULL;
    }

    UpdateTarget(pObj);

    if (pObj != NULL) {
        if (pObj->GetObjectType() != nObjectType
            && nObjectType != CGameObject::TYPE_AIBASE
            && nObjectType != CGameObject::TYPE_NONE) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            UpdateTarget(NULL);
            return NULL;
        }

        if (nObjectType == CGameObject::TYPE_AIBASE
            && (pObj->GetObjectType() & CGameObject::TYPE_AIBASE) == 0) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            UpdateTarget(NULL);
            return NULL;
        }

        if (GetObjectType() == CGameObject::TYPE_SPRITE
            && g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1
            && (static_cast<CGameSprite*>(this)->GetDerivedStats()->m_generalState & STATE_BLIND) != 0
            && pObj->GetObjectType() == CGameObject::TYPE_SPRITE) {
            const CPoint& targetPos = pObj->GetPos();
            CPoint ptTarget(
                targetPos.x / CPathSearch::GRID_SQUARE_SIZEX,
                targetPos.y / CPathSearch::GRID_SQUARE_SIZEY);
            CPoint ptThis(
                m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
            LONG nSquares = CAIUtil::CountSquares(ptThis, ptTarget);

            if (nSquares > (static_cast<CGameSprite*>(this)->m_animation.GetPersonalSpace() >> 1) + 4) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                    pObj->m_id,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                UpdateTarget(NULL);
                return NULL;
            }
        }
    }

    UpdateTarget(pObj);

    return pObj;
}

// 0x45C030 - same resolve and same filters as the overload above, but the
// object comes from a caller-supplied type instead of m_curAction.m_acteeID.
// The binary emits both bodies in full rather than delegating.
CGameObject* CGameAIBase::ResolveActionTarget(const CAIObjectType& type, BYTE nObjectType)
{
    CGameObject* pObj = type.GetObject(this, FALSE);
    if (pObj != NULL
        && pObj->GetObjectType() == CGameObject::TYPE_SPRITE
        && static_cast<CGameSprite*>(pObj)->GetDerivedStats()->m_cImmunitiesAIType.OnList(m_typeAI)) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
            pObj->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        pObj = NULL;
    }

    UpdateTarget(pObj);

    if (pObj != NULL) {
        if (pObj->GetObjectType() != nObjectType
            && nObjectType != CGameObject::TYPE_AIBASE
            && nObjectType != CGameObject::TYPE_NONE) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            UpdateTarget(NULL);
            return NULL;
        }

        if (nObjectType == CGameObject::TYPE_AIBASE
            && (pObj->GetObjectType() & CGameObject::TYPE_AIBASE) == 0) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            UpdateTarget(NULL);
            return NULL;
        }

        if (GetObjectType() == CGameObject::TYPE_SPRITE
            && g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1
            && (static_cast<CGameSprite*>(this)->GetDerivedStats()->m_generalState & STATE_BLIND) != 0
            && pObj->GetObjectType() == CGameObject::TYPE_SPRITE) {
            const CPoint& targetPos = pObj->GetPos();
            CPoint ptTarget(
                targetPos.x / CPathSearch::GRID_SQUARE_SIZEX,
                targetPos.y / CPathSearch::GRID_SQUARE_SIZEY);
            CPoint ptThis(
                m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
            LONG nSquares = CAIUtil::CountSquares(ptThis, ptTarget);

            if (nSquares > (static_cast<CGameSprite*>(this)->m_animation.GetPersonalSpace() >> 1) + 4) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                    pObj->m_id,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                UpdateTarget(NULL);
                return NULL;
            }
        }
    }

    UpdateTarget(pObj);

    return pObj;
}

// 0x44DC10 case 0xA1 - IncrementChapter(S:Chapter*).
SHORT CGameAIBase::IncrementChapter()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    CString sChapter = m_curAction.GetString1();
    if (sChapter.IsEmpty()) {
        sChapter = "CHAPTERS";
    }

    BYTE resRef[RESREF_SIZE];
    CResRef(sChapter).GetResRef(resRef);

    // TODO: Incomplete. The MP client request path is not recovered here yet.
    // The binary SP/host path opens the chapter screen through
    // CScreenChapter::StartChapterMultiplayerHost.
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        BYTE nChapter = static_cast<BYTE>(pGame->GetCurrentChapter() + 1);

        g_pBaldurChitin->m_pEngineChapter->StartChapterMultiplayerHost(
            nChapter,
            resRef);

        CList<STRREF, STRREF>* pTextList = pGame->GetRuleTables().GetChapterText(CResRef(resRef), nChapter);
        if (pTextList != NULL) {
            if (pTextList->GetCount() > 1) {
                POSITION pos = pTextList->GetHeadPosition();
                pTextList->GetNext(pos);
                pGame->GetJournal()->AddEntry(pTextList->GetNext(pos), 0);
            }

            delete pTextList;
        }
    }

    return ACTION_DONE;
}

// 0x44DC10 case 0x78 - StartCutScene(S:CutScene*).
SHORT CGameAIBase::StartCutScene()
{
    CString sScript = m_curAction.GetString1();
    if (sScript.IsEmpty()) {
        return ACTION_ERROR;
    }

    CAIScript script((CResRef(sScript)));
    LONG queuedCount = 0;

    POSITION pos = script.m_caList.GetHeadPosition();
    while (pos != NULL) {
        CAIConditionResponse* pConditionResponse = script.m_caList.GetNext(pos);
        if (pConditionResponse == NULL) {
            continue;
        }

        POSITION responsePos = pConditionResponse->m_responseSet.m_responseList.GetHeadPosition();
        if (responsePos == NULL) {
            continue;
        }

        CAIResponse* pResponse = pConditionResponse->m_responseSet.m_responseList.GetNext(responsePos);
        if (pResponse == NULL || pResponse->m_actionList.GetCount() == 0) {
            continue;
        }

        POSITION actionPos = pResponse->m_actionList.GetHeadPosition();
        CAIAction* pActorAction = pResponse->m_actionList.GetNext(actionPos);
        if (pActorAction == NULL) {
            continue;
        }

        CAIAction actorAction(*pActorAction);
        actorAction.Decode(this);

        CGameObject* pObject = actorAction.m_acteeID.GetObjectWithType(this,
            CGameObject::TYPE_AIBASE,
            FALSE);
        if (pObject == NULL) {
            continue;
        }

        CAIResponse response;
        response.m_weight = pResponse->m_weight;
        response.m_responseNum = pResponse->m_responseNum;
        response.m_responseSetNum = pResponse->m_responseSetNum;
        response.m_scriptNum = pResponse->m_scriptNum;

        while (actionPos != NULL) {
            CAIAction* pAction = pResponse->m_actionList.GetNext(actionPos);
            if (pAction != NULL) {
                response.Add(*pAction);
                queuedCount++;
            }
        }

        CMessage* responseMsg = new CMessageInsertResponse(response,
            FALSE,
            FALSE,
            FALSE,
            m_id,
            pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(responseMsg, FALSE);

        if ((pObject->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            CMessage* cutSceneMsg = new CMessageSetInCutScene(TRUE,
                m_id,
                pObject->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(cutSceneMsg, FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObject->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
    }


    return ACTION_DONE;
}

// Case body for FloatMessage (0xF1), at 0x4507E0 inside ExecuteAction --
// not a function of its own, so no address marker.
// Resolves m_acteeID, queues a CMessageFloatText so the strref (m_specificID)
// is displayed above the target sprite.  Binary takes an SP fast path
// calling FUN_004C80E0 directly; routing through CMessageFloatText::Run
// reaches the same display routine on both SP and MP.
SHORT CGameAIBase::FloatMessage()
{
    CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
    if (pObj == NULL) {
        return ACTION_DONE;
    }

    LONG targetId = pObj->m_id;
    CMessage* msg = new CMessageFloatText(m_id,
        targetId,
        m_curAction.m_specificID,
        FALSE);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
        targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
    return ACTION_DONE;
}

// Case body for HideCreature (0xE9), at 0x452020 inside ExecuteAction --
// not a function of its own, so no address marker.
// Resolves target via m_acteeID and calls SetStealthState(m_specificID).
// TODO: Skips the FUN_0045BDD0 immunity/distance filter and the MP
// broadcast (CMessage90) -- SP semantics are preserved.
SHORT CGameAIBase::HideCreature()
{
    CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
    if (pObj == NULL) {
        return ACTION_DONE;
    }

    if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
        pSprite->SetStealthState(m_curAction.m_specificID);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
        pObj->m_id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
    return ACTION_DONE;
}

// 0x4525CC - case body for WaitAnimation (0x143).
// Returns ACTION_INTERRUPTABLE while the resolved target's current
// animation sequence matches m_specificID, ACTION_DONE otherwise.
// TODO: Binary uses FUN_0045BDD0 which adds an immunity check and a
// PC-distance cutoff (param_1[0x248] & 0x40000U).  Skipped here -- both
// are filters; the core semantics match.
SHORT CGameAIBase::WaitAnimation()
{
    CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
    if (pObj == NULL) {
        return ACTION_DONE;
    }

    SHORT result = ACTION_DONE;
    if ((pObj->GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pObj);
        if (pSprite->GetSequence() == m_curAction.m_specificID) {
            result = ACTION_INTERRUPTABLE;
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
        pObj->m_id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
    return result;
}

// 0x460300 - handles IncrementGlobal (0x6D).  Like SetGlobal but adds
// m_specificID to existing value; if variable absent, creates it with
// value = m_specificID.
// TODO: Multiplayer broadcast (same CMessage as SetGlobal) skipped.
SHORT CGameAIBase::IncrementGlobal()
{
    CString sScope;
    CString sName;
    SplitScriptVariableName(m_curAction.GetString1(), sScope, sName);
    LONG nDelta = m_curAction.m_specificID;

    if (sScope == CString("GLOBAL")) {
        CVariableHash* pHash = g_pBaldurChitin->GetObjectGame()->GetVariables();
        CVariable* pVar = pHash->FindKey(sName);
        if (pVar != NULL) {
            pVar->m_intValue += nDelta;
        } else {
            CVariable v;
            v.SetName(sName);
            v.m_intValue = nDelta;
            pHash->AddKey(v);
        }
        return ACTION_DONE;
    }

    if (sScope == CString("LOCALS")) {
        if ((GetObjectType() & CGameObject::TYPE_SPRITE) != 0) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(this);
            CVariableHash* pHash = pSprite->GetLocalVariables();
            CVariable* pVar = pHash->FindKey(sName);
            if (pVar != NULL) {
                pVar->m_intValue += nDelta;
            } else {
                CVariable v;
                v.SetName(sName);
                v.m_intValue = nDelta;
                pHash->AddKey(v);
            }
        }
        return ACTION_DONE;
    }

    CString sAreaName = sScope;
    if (sScope == CString("MYAREA") && m_pArea != NULL) {
        sAreaName = CResRef(reinterpret_cast<BYTE*>(m_pArea->m_header.m_areaName)).GetResRefStr();
    }

    CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sAreaName);
    if (pArea != NULL) {
        CVariableHash* pHash = pArea->GetVariables();
        CVariable* pVar = pHash->FindKey(sName);
        if (pVar != NULL) {
            pVar->m_intValue += nDelta;
        } else {
            CVariable v;
            v.SetName(sName);
            v.m_intValue = nDelta;
            pHash->AddKey(v);
        }
    }

    return ACTION_DONE;
}

// 0x465110
SHORT CGameAIBase::TakePartyGold()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    DWORD gold = m_curAction.m_specificID;
    if (gold > pGame->GetGameSave()->m_nPartyGold) {
        gold = pGame->GetGameSave()->m_nPartyGold;
    }

    if (m_objectType == TYPE_SPRITE) {
        static_cast<CGameSprite*>(this)->GetBaseStats()->m_gold += gold;
        static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold += gold;
    }

    CMessagePartyGold* pMessage = new CMessagePartyGold(TRUE,
        TRUE,
        -static_cast<LONG>(gold),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4651A0
SHORT CGameAIBase::GivePartyGold()
{
    DWORD gold = m_curAction.m_specificID;

    if (m_objectType == TYPE_SPRITE && m_curAction.m_actionID == CAIAction::GIVEPARTYGOLD) {
        if (gold > static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold) {
            gold = static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold;
        }

        static_cast<CGameSprite*>(this)->GetBaseStats()->m_gold -= gold;
        static_cast<CGameSprite*>(this)->GetDerivedStats()->m_nGold -= gold;
    }

    CMessagePartyGold* pMessage = new CMessagePartyGold(TRUE,
        TRUE,
        gold,
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4654C0
SHORT CGameAIBase::GiveOrder(CGameAIBase* sprite)
{
    if (sprite == NULL) {
        return ACTION_ERROR;
    }

    CAITrigger trigger(CAITrigger::RECEIVEDORDER, m_typeAI, m_curAction.m_specificID);

    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
        m_id,
        sprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x465630
SHORT CGameAIBase::DisplayString(CGameAIBase* sprite)
{
    STR_RES strRes;
    COLORREF rgbColor = RGB(0, 0, 0);
    STRREF name;

    if (sprite == NULL) {
        return ACTION_ERROR;
    }

    if (sprite->GetObjectType() == TYPE_SPRITE) {
        rgbColor = CVidPalette::RANGE_COLORS[static_cast<CGameSprite*>(sprite)->GetBaseStats()->m_colors[2]];
        name = static_cast<CGameSprite*>(sprite)->GetNameRef();
    } else {
        name = -1;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(m_curAction.m_specificID, strRes);
    strRes.szText.TrimLeft();

    if (sprite->m_typeAI.GetEnemyAlly() == CAIObjectType::EA_PC) {
        if (strRes.cSound.m_nLooping == 0) {
            strRes.cSound.SetFireForget(TRUE);
        }

        if (sprite->GetObjectType() == TYPE_SPRITE) {
            strRes.cSound.SetChannel(static_cast<CGameSprite*>(sprite)->GetChannel(),
                reinterpret_cast<DWORD>(m_pArea));
        } else {
            strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
        }

        strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);

        if (strRes.szText != "") {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(name,
                m_curAction.m_specificID,
                rgbColor,
                RGB(160, 200, 215),
                -1,
                m_id,
                sprite->GetId());
        }
    } else {
        if (strRes.cSound.m_nLooping == 0) {
            strRes.cSound.SetFireForget(TRUE);
        }

        if (sprite->GetObjectType() == TYPE_SPRITE) {
            strRes.cSound.SetChannel(static_cast<CGameSprite*>(sprite)->GetChannel(),
                reinterpret_cast<DWORD>(m_pArea));
        } else {
            strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
        }

        strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);

        if (strRes.szText != "") {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(name,
                m_curAction.m_specificID,
                rgbColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                sprite->GetId());
        }
    }

    return ACTION_DONE;
}

// 0x465F30
SHORT CGameAIBase::StartMovie()
{
    CString sMovieFileName;

    CBaldurProjector* pProjector = g_pBaldurChitin->m_pEngineProjector;
    if (!pProjector->ResolveMovieFileName(CResRef(m_curAction.GetString1()), sMovieFileName)) {
        return ACTION_ERROR;
    }

    g_pBaldurChitin->m_pEngineWorld->ReadyMovie(CResRef(m_curAction.GetString1()), FALSE);

    return ACTION_DONE;
}

// 0x466030
SHORT CGameAIBase::RevealAreaOnMap()
{
    CString sArea = m_curAction.GetString1();

    CWorldMap* pWorldMap = g_pBaldurChitin->GetObjectGame()->GetWorldMap(sArea);
    pWorldMap->EnableArea(pWorldMap->GetCurrentMapIndex(),
        CResRef(sArea),
        TRUE);

    g_pBaldurChitin->GetObjectGame()->FeedBack(CInfGame::FEEDBACK_WORLDMAP_UPDATE, 0, TRUE);

    g_pBaldurChitin->GetBaldurMessage()->SendMapWorldRevealArea(m_curAction.GetString1());

    return ACTION_DONE;
}

// 0x466120
SHORT CGameAIBase::ChangeTileState(CGameTiledObject* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    if (m_curAction.m_specificID != 0) {
        if ((target->m_dwFlags & 0x1) != 0) {
            target->ToggleState();
        }
    } else {
        if ((target->m_dwFlags & 0x1) == 0) {
            target->ToggleState();
        }
    }

    return ACTION_DONE;
}

// 0x466170
SHORT CGameAIBase::TriggerActivation(CGameTrigger* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    if (m_curAction.m_specificID != 0) {
        target->m_dwFlags &= ~0x100;
    } else {
        target->m_dwFlags |= 0x100;
    }

    CMessageTriggerStatus* pMessage = new CMessageTriggerStatus(target->m_dwFlags,
        target->m_trapActivated,
        target->m_trapDetected,
        m_id,
        target->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return ACTION_DONE;
}

// 0x4668B0
SHORT CGameAIBase::StartMusic()
{
    if (m_pArea == NULL) {
        return ACTION_ERROR;
    }

    // TODO: Check cast.
    m_pArea->PlaySong(static_cast<SHORT>(m_curAction.m_specificID), m_curAction.m_specificID2);

    return ACTION_DONE;
}

// 0x4668E0
SHORT CGameAIBase::SetMusic()
{
    if (m_pArea == NULL) {
        return ACTION_ERROR;
    }

    if (!m_pArea->SetSong(static_cast<SHORT>(m_curAction.m_specificID), static_cast<BYTE>(m_curAction.m_specificID2))) {
        return ACTION_ERROR;
    }

    return ACTION_DONE;
}

// 0x466A00
SHORT CGameAIBase::FinalSave()
{
    if (g_pChitin->cNetwork.GetSessionOpen()
        && g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return ACTION_DONE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE) != CGameObjectArray::SUCCESS) {
        return ACTION_DONE;
    }

    // NOTE: Looks like inlining.
    if (1) {
        CString sSaveName("000000002-Final-Save");
        g_pBaldurChitin->GetObjectGame()->m_sSaveGame = sSaveName;
        CScreenCharacter::SAVE_NAME = sSaveName;
    }

    g_pBaldurChitin->GetObjectGame()->SaveGame(1, 0, 1);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return ACTION_ERROR;
    }

    return ACTION_DONE;
}

// 0x466B30
SHORT CGameAIBase::Unlock(CGameAIBase* pObject)
{
    CMessage* message;

    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    if (pObject->GetObjectType() != TYPE_DOOR
        && pObject->GetObjectType() != TYPE_CONTAINER) {
        return ACTION_ERROR;
    }

    message = new CMessageSetForceActionPick(TRUE, m_id, pObject->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    CAITrigger trigger(CAITrigger::NO_TRIGGER, 0);

    if (pObject->GetObjectType() == TYPE_DOOR
        && (static_cast<CGameDoor*>(pObject)->m_dwFlags & 0x2) != 0) {
        trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

        message = new CMessageSetTrigger(trigger, m_id, pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        static_cast<CGameDoor*>(pObject)->m_dwFlags &= ~0x2;

        message = new CMessageUnlock(static_cast<CGameDoor*>(pObject)->m_dwFlags,
            m_id,
            pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    if (pObject->GetObjectType() == TYPE_CONTAINER
        && (static_cast<CGameContainer*>(pObject)->m_dwFlags & 0x1) != 0) {
        trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

        message = new CMessageSetTrigger(trigger, m_id, pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        message = new CMessageUnlock(static_cast<CGameContainer*>(pObject)->m_dwFlags & ~0x1,
            m_id,
            pObject->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return ACTION_DONE;
}

// 0x466F90
SHORT CGameAIBase::MoveGlobal(CGameSprite* pSprite)
{
    if (pSprite == NULL) {
        return ACTION_ERROR;
    }

    CString sArea = m_curAction.GetString1();
    sArea.MakeUpper();

    CMessage* message = new CMessageMoveGlobal(sArea,
        m_curAction.m_dest,
        m_id,
        pSprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    return ACTION_DONE;
}

// 0x467110
SHORT CGameAIBase::Lock(CGameAIBase* pObject)
{
    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    if (pObject->GetObjectType() != TYPE_DOOR
        && pObject->GetObjectType() != TYPE_CONTAINER) {
        return ACTION_ERROR;
    }

    CMessageSetForceActionPick* pSetForceActionPick = new CMessageSetForceActionPick(TRUE,
        m_id,
        pObject->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetForceActionPick, FALSE);

    CAITrigger trigger(CAITrigger::NO_TRIGGER, 0);

    if (pObject->GetObjectType() == TYPE_DOOR) {
        CGameDoor* pDoor = static_cast<CGameDoor*>(pObject);
        if ((pDoor->m_dwFlags & 0x2) == 0) {
            trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

            CMessageSetTrigger* pSetTrigger = new CMessageSetTrigger(trigger,
                m_id,
                pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetTrigger, FALSE);

            pDoor->m_dwFlags |= 0x2;

            CMessageUnlock* pUnlock = new CMessageUnlock(pDoor->m_dwFlags,
                m_id,
                pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pUnlock, FALSE);
        }
    }

    if (pObject->GetObjectType() == TYPE_CONTAINER) {
        CGameContainer* pContainer = static_cast<CGameContainer*>(pObject);
        if ((pContainer->m_dwFlags & 0x1) == 0) {
            trigger = CAITrigger(CAITrigger::UNLOCKED, m_typeAI, 0);

            CMessageSetTrigger* pSetTrigger = new CMessageSetTrigger(trigger,
                m_id,
                pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetTrigger, FALSE);

            CMessageUnlock* pUnlock = new CMessageUnlock(pContainer->m_dwFlags | 0x1,
                m_id,
                pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pUnlock, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x467550
SHORT CGameAIBase::DestroyItem()
{
    if (GetObjectType() == TYPE_SPRITE) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(this);
        SHORT slotNum = pSprite->FindItemPersonal(m_curAction.GetString1(), 0, FALSE);
        if (slotNum != -1) {
            pSprite->Unequip(slotNum);
            g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pSprite->GetEquipment()->m_items[slotNum]);
            pSprite->GetEquipment()->m_items[slotNum] = NULL;
            return ACTION_DONE;
        }

        if (pSprite->TakeItemBags(m_curAction.GetString1(), 1, -1) > 0) {
            return ACTION_DONE;
        }

        return ACTION_ERROR;
    }

    if (GetObjectType() == TYPE_CONTAINER) {
        CGameContainer* pContainer = static_cast<CGameContainer*>(this);
        SHORT slotNum = pContainer->FindItemSlot(CResRef(m_curAction.GetString1()));
        if (slotNum != -1) {
            pContainer->GetItem(slotNum);
            pContainer->SetItem(slotNum, NULL);
            pContainer->CompressContainer();
        }
        return ACTION_DONE;
    }

    return ACTION_ERROR;
}

// 0x467720
SHORT CGameAIBase::DetectSecretDoor(CGameDoor* target)
{
    if (target == NULL) {
        return ACTION_ERROR;
    }

    CGameDoor* pDoor;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(target->GetId(),
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pDoor),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return ACTION_ERROR;
    }

    if ((pDoor->m_dwFlags & 0x80) != 0) {
        if ((pDoor->m_dwFlags & 0x100) == 0) {
            pDoor->SetDrawPoly(400);

            pDoor->m_dwFlags |= 0x100;

            CMessageDoorStatus* pDoorStatus = new CMessageDoorStatus(pDoor,
                m_id,
                target->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pDoorStatus, FALSE);
        } else {
            pDoor->SetDrawPoly(400);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(target->GetId(),
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return ACTION_DONE;
}

// 0x467880
SHORT CGameAIBase::FadeToColor()
{
    CMessageFadeColor* pFadeColor = new CMessageFadeColor(TRUE,
        static_cast<BYTE>(m_curAction.m_dest.x),
        static_cast<BYTE>(m_curAction.m_dest.y),
        static_cast<BYTE>(m_curAction.m_specificID),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFadeColor, FALSE);

    return ACTION_DONE;
}

// 0x467900
SHORT CGameAIBase::FadeFromColor()
{
    CMessageFadeColor* pFadeColor = new CMessageFadeColor(FALSE,
        static_cast<BYTE>(m_curAction.m_dest.x),
        static_cast<BYTE>(m_curAction.m_dest.y),
        static_cast<BYTE>(m_curAction.m_specificID),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFadeColor, FALSE);

    return ACTION_DONE;
}

// 0x467970
SHORT CGameAIBase::FadeColorActivate()
{
    CMessageFadeColor* pFadeColor = new CMessageFadeColor(255,
        static_cast<BYTE>(m_curAction.m_dest.x),
        static_cast<BYTE>(m_curAction.m_dest.y),
        static_cast<BYTE>(m_curAction.m_specificID),
        m_id,
        m_id);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pFadeColor, FALSE);

    return ACTION_DONE;
}

// 0x4679E0
SHORT CGameAIBase::SpawnPtActivate(CGameSpawning* target)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_SPAWNING) {
        return ACTION_ERROR;
    }

    if (!target->m_spawningObject.m_activated) {
        CMessageSpawnPtActivate* pMessage = new CMessageSpawnPtActivate(TRUE,
            m_id,
            target->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x467A80
SHORT CGameAIBase::SpawnPtDeactivate(CGameSpawning* target)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_SPAWNING) {
        return ACTION_ERROR;
    }

    if (target->m_spawningObject.m_activated) {
        CMessageSpawnPtActivate* pMessage = new CMessageSpawnPtActivate(FALSE,
            m_id,
            target->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x467B10
SHORT CGameAIBase::SpawnPtSpawn(CGameSpawning* target)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_SPAWNING) {
        return ACTION_ERROR;
    }

    if (target->m_spawningObject.m_activated) {
        CMessageSpawnPtSpawn* pMessage = new CMessageSpawnPtSpawn(m_pos,
            m_id,
            target->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return ACTION_DONE;
}

// 0x467BB0
SHORT CGameAIBase::StaticStart(CGameStatic* target, BOOL bStart)
{
    if (target != NULL) {
        return ACTION_ERROR;
    }

    if (target->GetObjectType() != TYPE_STATIC) {
        return ACTION_ERROR;
    }

    // TODO: Check, not sure if that's right.
    if (bStart == (target->m_header.m_dwFlags & 0x8)) {
        CMessageStaticStart* pMessage = new CMessageStaticStart(bStart,
            m_id,
            target->GetId());
    }

    return ACTION_DONE;
}

// 0x460D60
SCRIPTNAME& CGameAIBase::GetScriptName()
{
    return m_scriptName;
}

// 0x50A400
void CGameAIBase::SetTrigger(const CAITrigger& trigger)
{
    m_pendingTriggers.AddTail(new CAITrigger(trigger));
}

// NOTE: Inlined.
void CGameAIBase::SetDefaultScript(CAIScript* script)
{
    if (m_movementScript != NULL) {
        delete m_movementScript;
    }

    m_movementScript = script;
}

// 0x6F2C20
SHORT CGameAIBase::GetVisualRange()
{
    return m_pArea->m_visibility.m_nSearchRangeH * 32;
}

// 0x6F2C30
SHORT CGameAIBase::GetHelpRange()
{
    return m_pArea->m_visibility.m_nSearchRangeH * 48;
}

// NOTE: Inlined.
void CGameAIBase::ResetCurrResponse()
{
    if (m_curAction.m_actionID == CAIAction::NO_ACTION) {
        m_curResponseNum = -1;
        m_curResponseSetNum = -1;
        m_curScriptNum = -1;
    }
}

// 0x45B970
CAIAction& CGameAIBase::GetNextAction(CAIAction& action)
{
    while (!m_queuedActions.IsEmpty()) {
        CAIAction* node = m_queuedActions.RemoveHead();
        if (node->GetActionID() != CAIAction::NO_ACTION) {
            action = *node;

            CAIObjectType actorType(node->m_actorID);
            if (actorType.OfType(CAIObjectType::ANYONE, FALSE, FALSE)
                && actorType.GetName() == ""
                && actorType.m_SpecialCase[0] == 0) {
                delete node;
                return action;
            }

            actorType.Decode(this);
            CGameAIBase* actor = static_cast<CGameAIBase*>(actorType.GetObjectWithType(this, CGameObject::TYPE_AIBASE, FALSE));
            if (actor != NULL) {
                action.m_actorID = CAIObjectType::ANYONE;
                action.m_internalFlags |= 0x1;

                CMessage* message = new CMessageInsertAction(action, m_id, actor->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(actor->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        delete node;
    }

    action = CAIAction::NULL_ACTION;
    return action;
}

// 0x481890
void CGameAIBase::SplitRectIntoGrid(CRect* r, CArray<CRect*>& ary)
{
    const INT width = r->right - r->left;
    const INT height = r->bottom - r->top;

    INT xCount = 1;
    INT yCount = 1;
    if (width > 512) {
        xCount = width / 512 + 1;
    }
    if (height > 512) {
        yCount = height / 512 + 1;
    }

    if (width <= 512 && height <= 512) {
        ary.SetSize(0, -1);
        ary.Add(r);
        return;
    }

    ary.SetSize(0, 4);
    for (INT y = 0; y < yCount; y++) {
        const INT top = r->top + 512 * y;
        if (top > r->bottom) {
            return;
        }

        for (INT x = 0; x < xCount; x++) {
            const INT left = r->left + 512 * x;
            if (left > r->right) {
                break;
            }

            CRect* pRect = new CRect();
            pRect->left = left;
            pRect->top = top;
            pRect->right = left + min(r->right - left, 512);
            pRect->bottom = top + min(r->bottom - top, 512);
            ary.Add(pRect);
        }
    }
}

// 0x467C50
void CGameAIBase::FireSpell(const CResRef& resRef, CGameObject* target)
{
    // TODO: Incomplete.
}

// 0x4681E0
void CGameAIBase::FireSpellPoint(const CResRef& resRef, const CPoint& ptTarget)
{
    // TODO: Incomplete.
}

// 0x4686C0
CVariable* CGameAIBase::GetGlobalVariable(const CString& sScope, const CString& sName, int a3)
{
    CVariable* pVariable;

    if (sScope == CString("GLOBAL")) {
        pVariable = g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
        if (pVariable != NULL) {
            return pVariable;
        }
    } else {
        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(sScope);
        if (pArea != NULL) {
            pVariable = pArea->GetVariables()->FindKey(sName);
            if (pVariable != NULL) {
                return pVariable;
            }
        }
    }

    CVariable variable;
    variable.SetName(sName);
    g_pBaldurChitin->GetObjectGame()->GetVariables()->AddKey(variable);

    return g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(sName);
}

// 0x4530F0
void CGameAIBase::SetAIType342(const CAIObjectType& type)
{
    field_342.Set(type);
}

// 0x453110
void CGameAIBase::SetAIType37E(const CAIObjectType& type)
{
    field_37E.Set(type);
}

// 0x453130
void CGameAIBase::SetLastActionReturn(SHORT returnValue)
{
    m_nLastActionReturn = returnValue;
}

// 0x45B6D0
int CGameAIBase::GetAICounter58C()
{
    return field_58C;
}

// 0x45B6E0
void CGameAIBase::SetAIType3BA(const CAIObjectType& type)
{
    field_3BA.Set(type);
}

// -----------------------------------------------------------------------------

// 0x45E250
CGameAIArea::CGameAIArea()
{
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x45E2D0
CGameAIArea::~CGameAIArea()
{
}

// 0x47C830
BOOLEAN CGameAIArea::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x766660
BOOLEAN CGameAIArea::CompressTime(DWORD deltaTime)
{
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x45E2E0
CGameAIGame::CGameAIGame()
{
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x45E2D0
CGameAIGame::~CGameAIGame()
{
}

// 0x47C830
BOOLEAN CGameAIGame::CanSaveGame(STRREF& strError)
{
    strError = -1;
    return TRUE;
}

// 0x766660
BOOLEAN CGameAIGame::CompressTime(DWORD deltaTime)
{
    return TRUE;
}
