// TEST SCAFFOLDING -- not a mirror of IWD2.exe. See AutoUI.h.
//
// Drives the UI the way the engine drives it. CChitin::AsynchronousUpdate polls
// the mouse with GetAsyncKeyState and then calls pActiveEngine->OnLButtonDown /
// OnLButtonUp with a screen point (CChitin.cpp:1544-1567) -- IWD2 never handles
// a WM_LBUTTONDOWN. So a synthetic click is just those same two virtual calls
// with a point we computed instead of one the OS gave us: real hit-testing,
// real capture handshake, real click sound.
//
// That single primitive covers menus, the action bar (which is panel 1,
// controls 6..17 of CScreenWorld's manager -- CInfButtonArray.cpp:598,627),
// portraits and ground clicks. Only the addressing differs: a control is
// resolved to its centre, a ground click is given in screen coordinates.
//
// Script format -- one command per line, '#' starts a comment:
//
//   waitgo [ticks]               block until IWD2_RE_UI_GO exists -- the observer
//                                handshake. vm.sh smoke only attaches the crash
//                                guard after the save reports loaded, so a
//                                scenario that clicks before that is invisible
//                                to --hit. Put this before the first action.
//   wait <screen> [ticks]        block until that screen is active
//   dump                         whole UI tree of the active screen
//   center                       scroll the view onto the party. A save stores
//                                its own camera, so the party can start off
//                                screen and every ground click then misses.
//   ground                       visible area's render list, each object with
//                                the screen point that clicks it. The world
//                                twin of `dump`: it answers "what is out there
//                                and where" so a scenario can click a door or a
//                                creature without a human reading pixels.
//   bar                          action-bar state + the twelve slots' button
//                                types. `dump` cannot see these: every cell is
//                                the same always-active control whatever the
//                                bar shows.
//   screen                       active screen identity only
//   click <panel> <control>      resolve the control centre, then click it
//   rclick <panel> <control>     same, with the right button. The action bar's
//                                customise menu and every quick-slot picker are
//                                reachable only this way.
//   clickxy <x> <y>              move the cursor there, then click it (ground,
//                                world). The move matters: world picking reads
//                                m_ptMousePos, not the click point.
//   hover <x> <y>                move the cursor without clicking
//   key <vk>                     stage the VK and run one OnKeyDown tick
//   goto <screen>                CBaldurEngine::OnLeftPanelButtonClick
//   expect screen <screen>
//   expect controls <panel> <min>
//   expect control <panel> <control>
//   expect partyvisible <slot>  require that party member's ground position to
//                                be inside the visible area's viewport.
//   expect targeting <state> <icon> <button>  check game targeting and bar selection.
//   expect picked <slot>        require the ground picker to identify that party member.
//   sleep <ticks>
//   resume                      unpause the world if paused
//   enddialog                   end the current dialogue for isolated engine tests
//   forcepoint <resref>          queue ForceSpellPoint at the first party
//                                caster's own position (requires available spell).
//                                Engine-action test, not a casting UI click.
//   weaponpayload <item> [launcher]  ZRC fixtures only; verify weapon/launcher
//                                effect provenance, fire at portrait 1, check HP.
//                                Bypasses Swing and attack/base-damage rolls.
//   combatrules                 assert recovered AC/dual-wield/range calculations;
//                                transient state restored in the same tick.
//   combathit                   exercise attack/critical resolution on portraits
//                                0 and 1; no damage or save, reseeds thread RNG.
//   combatammo                  select compatible fixture ammunition and verify
//                                quick buttons; restores the original equipment.
//   weapondamage <item> [launcher]  calculate base damage and deliver by projectile;
//                                ZRC fixtures only, no saving, check actual HP loss.
//
// Results are one JSON object per line to IWD2_RE_UI_RESULT, last line the
// verdict. A failed assertion stops the script.

#include "AutoUI.h"

#include "mfc.h"

#include "BalDataTypes.h"
#include "CAIAction.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CGameArea.h"
#include "CGameObject.h"
#include "CGameObjectArray.h"
#include "CGameOptions.h"
#include "CGameSprite.h"
#include "CGameEffect.h"
#include "CItem.h"
#include "CProjectile.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CScreenWorld.h"
#include "CUIControlBase.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CWarp.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Iwd2AutoUI {

namespace {

const int MAX_STEPS = 128;
const int MAX_TOKEN = 48;
const int MAX_STRS = 2;
const int MAX_NUMS = 3;
// Ticks to let the engine settle after a click before the next step runs.
const int CLICK_SETTLE_TICKS = 2;
const int DEFAULT_WAIT_TICKS = 3000;

struct Step {
    char op[MAX_TOKEN];
    char s[MAX_STRS][MAX_TOKEN];
    int n[MAX_NUMS];
    int nCount;
};

bool s_loaded = false;
bool s_finished = false;
Step s_steps[MAX_STEPS];
int s_count = 0;
int s_pc = 0;
int s_waited = 0;
int s_failed = 0;
int s_clickPhase = 0;
bool s_clickRight = false;
CPoint s_clickPt(0, 0);
FILE* s_out = NULL;
LONG s_payloadTarget = -1;
SHORT s_payloadHP = 0;
INT s_payloadDamage = 0;

DWORD GetEnv(const char* name, char* value, DWORD size)
{
    DWORD result = GetEnvironmentVariableA(name, value, size);
    if (result >= size && size > 0) {
        value[size - 1] = '\0';
    }
    return result;
}

void Emit(const char* fmt, ...)
{
    if (s_out == NULL) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(s_out, fmt, args);
    va_end(args);
    fputc('\n', s_out);
    // Flushed per line: if the game faults mid-scenario the lines already
    // written are the diagnosis.
    fflush(s_out);
}

int Num(const Step& step, int nIndex, int nDefault)
{
    return nIndex < step.nCount ? step.n[nIndex] : nDefault;
}

// Identity of the active screen, by pointer comparison against the slots
// CBaldurChitin holds (CBaldurChitin.h:163-185). Pointer identity beats any
// heuristic: there is exactly one instance of each screen.
const char* ScreenName(const CWarp* pEngine)
{
    if (pEngine == NULL || g_pBaldurChitin == NULL) {
        return "none";
    }

    const void* p = pEngine;
    if (p == g_pBaldurChitin->m_pEngineWorld) return "world";
    if (p == g_pBaldurChitin->m_pEngineSave) return "save";
    if (p == g_pBaldurChitin->m_pEngineLoad) return "load";
    if (p == g_pBaldurChitin->m_pEngineOptions) return "options";
    if (p == g_pBaldurChitin->m_pEngineConnection) return "connection";
    if (p == g_pBaldurChitin->m_pEngineSinglePlayer) return "singleplayer";
    if (p == g_pBaldurChitin->m_pEngineMultiPlayer) return "multiplayer";
    if (p == g_pBaldurChitin->m_pEngineChapter) return "chapter";
    if (p == g_pBaldurChitin->m_pEngineCharacter) return "character";
    if (p == g_pBaldurChitin->m_pEngineCreateChar) return "createchar";
    if (p == g_pBaldurChitin->m_pEngineInventory) return "inventory";
    if (p == g_pBaldurChitin->m_pEngineJournal) return "journal";
    if (p == g_pBaldurChitin->m_pEngineMap) return "map";
    if (p == g_pBaldurChitin->m_pEngineWorldMap) return "worldmap";
    if (p == g_pBaldurChitin->m_pEngineSpellbook) return "spellbook";
    if (p == g_pBaldurChitin->m_pEngineStore) return "store";
    if (p == g_pBaldurChitin->m_pEngineStart) return "start";
    if (p == g_pBaldurChitin->m_pEngineMovies) return "movies";
    if (p == g_pBaldurChitin->m_pEngineKeymaps) return "keymaps";
    if (p == g_pBaldurChitin->m_pEngineProjector) return "projector";
    if (p == g_pBaldurChitin->m_pEngineDM) return "dm";
    return "other";
}

// Every screen that owns a UI is a CBaldurEngine; GetManager returns
// this + 0x30 (CBaldurEngine.cpp:468).
CUIManager* ManagerOf(CWarp* pEngine)
{
    if (pEngine == NULL) {
        return NULL;
    }

    return static_cast<CBaldurEngine*>(pEngine)->GetManager();
}

CUIPanel* FindPanel(CWarp* pEngine, DWORD nPanelId)
{
    CUIManager* pManager = ManagerOf(pEngine);
    return pManager != NULL ? pManager->GetPanel(nPanelId) : NULL;
}

int CountControls(CUIPanel* pPanel)
{
    if (pPanel == NULL) {
        return -1;
    }

    int nControls = 0;
    POSITION pos = pPanel->m_lControls.GetHeadPosition();
    while (pos != NULL) {
        pPanel->m_lControls.GetNext(pos);
        nControls++;
    }
    return nControls;
}

// Dump the live tree. This is the point of the module: it answers "does this
// screen actually have controls, and are they active" without a human reading
// pixels.
void DumpTree(CWarp* pEngine, int nStep)
{
    CUIManager* pManager = ManagerOf(pEngine);
    if (pManager == NULL) {
        Emit("{\"step\":%d,\"op\":\"dump\",\"status\":\"error\",\"detail\":\"no manager\"}",
            nStep);
        return;
    }

    char szResRef[16];
    memset(szResRef, 0, sizeof(szResRef));
    pManager->m_cResRef.CopyToString(szResRef);

    Emit("{\"step\":%d,\"op\":\"dump\",\"screen\":\"%s\",\"chu\":\"%s\","
         "\"initialized\":%d,\"hidden\":%d}",
        nStep, ScreenName(pEngine), szResRef,
        pManager->m_bInitialized != 0 ? 1 : 0,
        pManager->m_bHidden != 0 ? 1 : 0);

    POSITION panelPos = pManager->m_lPanels.GetHeadPosition();
    while (panelPos != NULL) {
        CUIPanel* pPanel = pManager->m_lPanels.GetNext(panelPos);
        if (pPanel == NULL) {
            continue;
        }

        Emit("{\"step\":%d,\"op\":\"panel\",\"id\":%lu,\"x\":%ld,\"y\":%ld,"
             "\"cx\":%ld,\"cy\":%ld,\"active\":%d,\"enabled\":%d,\"hidden\":%d,"
             "\"controls\":%d}",
            nStep, pPanel->m_nID,
            pPanel->m_ptOrigin.x, pPanel->m_ptOrigin.y,
            pPanel->m_size.cx, pPanel->m_size.cy,
            pPanel->m_bActive != 0 ? 1 : 0,
            pPanel->m_bEnabled != 0 ? 1 : 0,
            pPanel->m_bHidden != 0 ? 1 : 0,
            CountControls(pPanel));

        POSITION ctrlPos = pPanel->m_lControls.GetHeadPosition();
        while (ctrlPos != NULL) {
            CUIControlBase* pControl = pPanel->m_lControls.GetNext(ctrlPos);
            if (pControl == NULL) {
                continue;
            }

            Emit("{\"step\":%d,\"op\":\"control\",\"panel\":%lu,\"id\":%lu,"
                 "\"x\":%ld,\"y\":%ld,\"cx\":%ld,\"cy\":%ld,"
                 "\"type\":%d,\"active\":%d,\"enabled\":%d}",
                nStep, pPanel->m_nID, pControl->m_nID,
                pControl->m_ptOrigin.x, pControl->m_ptOrigin.y,
                pControl->m_size.cx, pControl->m_size.cy,
                pControl->m_nControlType,
                pControl->m_bActive != 0 ? 1 : 0,
                pControl->m_bEnabled != 0 ? 1 : 0);
        }
    }
}

// The world twin of DumpTree. m_lVertSort is what the area actually renders,
// so walking it lists exactly the objects a click could hit, and CInfinity
// turns each world position into the screen point that hits it. Without this a
// world-side scenario has to guess coordinates and silently misses.
int CountVertSort(CGameArea* pArea)
{
    int nObjects = 0;
    POSITION pos = pArea->m_lVertSort.GetHeadPosition();
    while (pos != NULL) {
        pArea->m_lVertSort.GetNext(pos);
        nObjects++;
    }
    return nObjects;
}

// The action bar's contents are not visible in `dump`: its twelve cells are the
// same twelve always-active CUIControlButtons whatever the bar shows.  What a
// cell IS lives in CInfButtonArray::m_buttonTypes, driven by m_nState.  So this
// is the diagnostic for "which menu am I in and what is in each slot" -- the
// only way to write a scenario that clicks a submenu entry by role.
void DumpButtonArray(int nStep)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame == NULL) {
        Emit("{\"step\":%d,\"op\":\"bar\",\"status\":\"error\",\"detail\":\"no game\"}",
            nStep);
        return;
    }

    CInfButtonArray* pBar = &pGame->m_cButtonArray;
    Emit("{\"step\":%d,\"op\":\"bar\",\"state\":%d,\"selected\":%d,"
         "\"customizeSlot\":%d,\"pickerPage\":%d}",
        nStep, pBar->m_nState, pBar->m_nSelectedButton,
        pBar->m_nCustomizeSlot, pBar->m_nListStartIndex);

    for (int i = 0; i < 12; i++) {
        Emit("{\"step\":%d,\"op\":\"barslot\",\"slot\":%d,\"control\":%d,"
             "\"type\":%d,\"greyout\":%d}",
            nStep, i, i + 6, pBar->m_buttonTypes[i],
            pBar->m_buttonArray[i].m_bGreyOut != 0 ? 1 : 0);
    }
}

void DumpGround(int nStep)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CGameArea* pArea = pGame != NULL ? pGame->GetVisibleArea() : NULL;
    if (pArea == NULL) {
        Emit("{\"step\":%d,\"op\":\"ground\",\"status\":\"error\","
             "\"detail\":\"no visible area\"}", nStep);
        return;
    }

    CInfinity* pInfinity = pArea->GetInfinity();

    // Report the scroll origin and viewport with the list. GetScreenCoordinates
    // collapses anything off-screen to (-1,-1), which cannot be told apart from
    // a bad walk, so emit the unclipped point and let the reader clip.
    char szArea[16];
    memset(szArea, 0, sizeof(szArea));
    pArea->m_resRef.CopyToString(szArea);

    Emit("{\"step\":%d,\"op\":\"ground\",\"area\":\"%s\",\"objects\":%d,"
         "\"scrollx\":%d,"
         "\"scrolly\":%d,\"left\":%ld,\"top\":%ld,\"right\":%ld,\"bottom\":%ld}",
        nStep, szArea, CountVertSort(pArea),
        pInfinity->nNewX, pInfinity->nNewY,
        pInfinity->rViewPort.left, pInfinity->rViewPort.top,
        pInfinity->rViewPort.right, pInfinity->rViewPort.bottom);

    // The party is the reference point: the camera follows it, so if the party
    // does not land inside the viewport the list being walked is not the one
    // the screen shows.
    for (SHORT nSlot = 0; nSlot < pGame->GetNumCharacters(); nSlot++) {
        LONG characterId = pGame->GetFixedOrderCharacterId(nSlot);
        if (characterId == CGameObjectArray::INVALID_INDEX) {
            continue;
        }

        CGameObject* pMember;
        if (pGame->GetObjectArray()->GetShare(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                &pMember,
                INFINITE)
            != CGameObjectArray::SUCCESS) {
            continue;
        }

        char szMemberArea[16];
        memset(szMemberArea, 0, sizeof(szMemberArea));
        if (pMember->m_pArea != NULL) {
            pMember->m_pArea->m_resRef.CopyToString(szMemberArea);
        }

        Emit("{\"step\":%d,\"op\":\"party\",\"slot\":%d,\"id\":%ld,"
             "\"area\":\"%s\",\"same\":%d,"
             "\"wx\":%ld,\"wy\":%ld,\"x\":%ld,\"y\":%ld}",
            nStep, nSlot, characterId,
            szMemberArea, pMember->m_pArea == pArea ? 1 : 0,
            pMember->m_pos.x, pMember->m_pos.y,
            pMember->m_pos.x - pInfinity->nNewX + pInfinity->rViewPort.left,
            pMember->m_pos.y - pInfinity->nNewY + pInfinity->rViewPort.top);

        pGame->GetObjectArray()->ReleaseShare(characterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    POSITION pos = pArea->m_lVertSort.GetHeadPosition();
    while (pos != NULL) {
        LONG objectId = reinterpret_cast<LONG>(pArea->m_lVertSort.GetNext(pos));

        CGameObject* pObject;
        BYTE rc = pGame->GetObjectArray()->GetShare(objectId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        CPoint ptScreen;
        ptScreen.x = pObject->m_pos.x - pInfinity->nNewX + pInfinity->rViewPort.left;
        ptScreen.y = pObject->m_pos.y - pInfinity->nNewY + pInfinity->rViewPort.top;

        Emit("{\"step\":%d,\"op\":\"object\",\"id\":%ld,\"type\":%d,"
             "\"wx\":%ld,\"wy\":%ld,\"x\":%ld,\"y\":%ld,\"onscreen\":%d}",
            nStep, objectId, pObject->GetObjectType(),
            pObject->m_pos.x, pObject->m_pos.y,
            ptScreen.x, ptScreen.y,
            pInfinity->rViewPort.PtInRect(ptScreen) ? 1 : 0);

        pGame->GetObjectArray()->ReleaseShare(objectId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// A save restores the camera it was written with, which need not be looking at
// the party -- slot 3 loads with the view 1600 pixels below them. Nothing on
// screen then corresponds to anything in the area, so centre first and let the
// coordinates ground reports mean what they say.
void CenterOnParty(int nStep)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CGameArea* pArea = pGame != NULL ? pGame->GetVisibleArea() : NULL;
    if (pArea == NULL) {
        Emit("{\"step\":%d,\"op\":\"center\",\"status\":\"error\","
             "\"detail\":\"no visible area\"}", nStep);
        return;
    }

    LONG characterId = pGame->GetNumCharacters() > 0
        ? pGame->GetFixedOrderCharacterId(0)
        : CGameObjectArray::INVALID_INDEX;
    if (characterId == CGameObjectArray::INVALID_INDEX) {
        Emit("{\"step\":%d,\"op\":\"center\",\"status\":\"error\","
             "\"detail\":\"no party\"}", nStep);
        return;
    }

    CGameObject* pMember;
    if (pGame->GetObjectArray()->GetShare(characterId,
            CGameObjectArray::THREAD_ASYNCH,
            &pMember,
            INFINITE)
        != CGameObjectArray::SUCCESS) {
        Emit("{\"step\":%d,\"op\":\"center\",\"status\":\"error\","
             "\"detail\":\"party member locked\"}", nStep);
        return;
    }

    CPoint ptMember = pMember->m_pos;
    pGame->GetObjectArray()->ReleaseShare(characterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    CInfinity* pInfinity = pArea->GetInfinity();
    pInfinity->SetViewPosition(ptMember.x - pInfinity->rViewPort.Width() / 2,
        ptMember.y - pInfinity->rViewPort.Height() / 2,
        TRUE);

    Emit("{\"step\":%d,\"op\":\"center\",\"wx\":%ld,\"wy\":%ld,"
         "\"scrollx\":%d,\"scrolly\":%d,\"status\":\"ok\"}",
        nStep, ptMember.x, ptMember.y, pInfinity->nNewX, pInfinity->nNewY);
}

void Fail(int nStep, const char* op, const char* detail)
{
    Emit("{\"step\":%d,\"op\":\"%s\",\"status\":\"fail\",\"detail\":\"%s\"}",
        nStep, op, detail);
    s_failed++;
    s_pc = s_count;
}

// One line -> one Step. Tokens after the op are sorted into up to two string
// slots and up to three integer slots, so every command form below parses with
// the same code: `wait world 3000`, `click 0 3`, `expect controls 0 5`.
void ParseLine(char* p)
{
    Step& step = s_steps[s_count];
    memset(&step, 0, sizeof(step));

    int nStrs = 0;
    char* token = strtok(p, " \t\r\n");
    if (token == NULL) {
        return;
    }

    strncpy(step.op, token, MAX_TOKEN - 1);

    while ((token = strtok(NULL, " \t\r\n")) != NULL) {
        if ((token[0] >= '0' && token[0] <= '9') || token[0] == '-') {
            if (step.nCount < MAX_NUMS) {
                step.n[step.nCount++] = atoi(token);
            }
        } else if (nStrs < MAX_STRS) {
            strncpy(step.s[nStrs++], token, MAX_TOKEN - 1);
        }
    }

    s_count++;
}

void LoadScript()
{
    s_loaded = true;

    char szPath[MAX_PATH];
    if (GetEnv("IWD2_RE_UI_SCRIPT", szPath, sizeof(szPath)) == 0) {
        return;
    }

    char szResult[MAX_PATH];
    if (GetEnv("IWD2_RE_UI_RESULT", szResult, sizeof(szResult)) > 0) {
        s_out = fopen(szResult, "wb");
    }

    FILE* fp = fopen(szPath, "rb");
    if (fp == NULL) {
        Emit("{\"op\":\"load\",\"status\":\"error\",\"detail\":\"cannot open script\"}");
        return;
    }

    char szLine[256];
    while (s_count < MAX_STEPS && fgets(szLine, sizeof(szLine), fp) != NULL) {
        char* hash = strchr(szLine, '#');
        if (hash != NULL) {
            *hash = '\0';
        }
        ParseLine(szLine);
    }

    fclose(fp);
    Emit("{\"op\":\"load\",\"status\":\"ok\",\"steps\":%d}", s_count);
}

} // namespace

bool IsEnabled()
{
    char szPath[MAX_PATH];
    return GetEnv("IWD2_RE_UI_SCRIPT", szPath, sizeof(szPath)) > 0;
}

void Tick(CWarp* pActiveEngine)
{
    if (!s_loaded) {
        LoadScript();
    }

    if (s_finished || pActiveEngine == NULL) {
        return;
    }

    if (s_pc >= s_count) {
        s_finished = true;
        Emit("{\"op\":\"verdict\",\"verdict\":\"%s\",\"steps\":%d,\"failed\":%d}",
            s_failed != 0 ? "FAIL" : "PASS", s_count, s_failed);
        if (s_out != NULL) {
            fclose(s_out);
            s_out = NULL;
        }
        return;
    }

    // A click spans ticks: down, then up, then settle -- as the engine's own
    // edge detection would deliver it.
    if (s_clickPhase != 0) {
        if (s_clickPhase == 1) {
            if (s_clickRight) {
                pActiveEngine->OnRButtonUp(s_clickPt);
            } else {
                pActiveEngine->OnLButtonUp(s_clickPt);
            }
            s_clickPhase = 2;
            return;
        }
        if (s_clickPhase < CLICK_SETTLE_TICKS + 2) {
            s_clickPhase++;
            return;
        }
        s_clickPhase = 0;
        s_pc++;
        return;
    }

    const Step& step = s_steps[s_pc];
    const int nStep = s_pc + 1;

    if (strcmp(step.op, "combatrules") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, "combatrules", "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0);
        LONG targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, "combatrules", "source unavailable");
            return;
        }
        if (objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, "combatrules", "target unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(sourceObject);
        CGameSprite* target = static_cast<CGameSprite*>(targetObject);
        auto savedBase = source->m_baseStats;
        auto savedTargetBase = target->m_baseStats;
        BYTE targetRace = target->m_typeAI.m_nRace;
        BYTE targetAlignment = target->m_typeAI.m_nAlignment;
        CPoint sourcePos = source->m_pos, targetPos = target->m_pos;
        DWORD sourceState = source->m_derivedStats.m_generalState;
        DWORD targetState = target->m_derivedStats.m_generalState;
        SHORT dex = source->m_derivedStats.m_nDEX, wis = source->m_derivedStats.m_nWIS;
        SHORT luck = source->m_derivedStats.m_nLuck;
        BOOL hiding = target->m_bHiding;
        bool tenser = source->m_derivedStats.m_spellStates[SPLSTATE_TENSERS_TRANSFORMATION];
        SHORT natural = source->m_derivedStats.m_nACNaturalBonus;
        SHORT deflection = source->m_derivedStats.m_nACDeflectionBonus;
        SHORT dodge = source->m_derivedStats.m_nACDodgeBonus;
        auto ranger = source->m_derivedStats.m_nClassLevels[CAIOBJECTTYPE_C_RANGER - 1];
        BYTE weaponSet = source->m_nWeaponSet, selected = source->m_equipment.m_selectedWeapon;
        CItem* armor = source->m_equipment.m_items[1];
        CItem* mainhand = source->m_equipment.m_items[43];
        CItem* offhand = source->m_equipment.m_items[44];
        CItem sword(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
        CItem dagger(CResRef("ZRCDAGGR"), 1, 0, 0, 0, 0);
        CItem plate(CResRef("ZRCARMOR"), 1, 0, 0, 0, 0);
        CItem shield(CResRef("ZRCSHLD"), 1, 0, 0, 0, 0);
        BOOL valid = sword.GetItemType() == 20 && dagger.GetItemType() == 16
            && plate.GetItemType() == 63 && shield.GetItemType() == 41;
        INT checks = 0;
        auto check = [&](const char* name, INT actual, INT expected) {
            Emit("{\"step\":%d,\"op\":\"combatrules-case\",\"case\":\"%s\",\"actual\":%d,\"expected\":%d}",
                nStep, name, actual, expected);
            valid = valid && actual == expected;
            ++checks;
        };
        if (valid) {
            memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
            memset(target->m_baseStats.m_feats, 0, sizeof(target->m_baseStats.m_feats));
            source->m_baseStats.m_monkLevel = 0;
            source->m_derivedStats.m_nClassLevels[CAIOBJECTTYPE_C_RANGER - 1] = 0;
            source->m_derivedStats.m_generalState = target->m_derivedStats.m_generalState = 0;
            source->m_derivedStats.m_nDEX = 18;
            source->m_derivedStats.m_nWIS = 16;
            source->m_derivedStats.m_nACNaturalBonus = 2;
            source->m_derivedStats.m_nACDeflectionBonus = 3;
            source->m_derivedStats.m_nACDodgeBonus = 1;
            source->m_nWeaponSet = 0;
            source->m_equipment.m_selectedWeapon = 43;
            source->m_equipment.m_items[1] = NULL;
            source->m_equipment.m_items[43] = &sword;
            source->m_equipment.m_items[44] = NULL;
            check("touch-unarmored", source->GetTouchAC(), 20);
            source->m_equipment.m_items[1] = &plate;
            check("touch-plate-dex-cap", source->GetTouchAC(), 16);
            source->m_equipment.m_items[1] = NULL;
            source->m_derivedStats.m_generalState = 0x40000;
            check("touch-blind", source->GetTouchAC(), 16);
            source->m_baseStats.m_feats[0] |= 1 << CGAMESPRITE_FEAT_BLIND_FIGHT;
            check("touch-blind-fight", source->GetTouchAC(), 20);
            source->m_derivedStats.m_nDEX = 8;
            check("touch-negative-dex", source->GetTouchAC(), 15);
            source->m_derivedStats.m_nDEX = 18;
            source->m_derivedStats.m_generalState = 0;
            source->m_baseStats.m_monkLevel = 1;
            check("monk-wisdom", source->GetMonkACBonus(), 3);
            check("touch-monk", source->GetTouchAC(), 23);
            source->m_equipment.m_items[44] = &shield;
            check("monk-shield", source->GetMonkACBonus(), 0);
            check("shield-is-not-dual-wield", source->GetDualWieldPenalty(1), 0);
            source->m_equipment.m_items[44] = NULL;
            check("no-offhand", source->GetDualWieldPenalty(0), 0);
            source->m_equipment.m_items[44] = &sword;
            check("dual-main", source->GetDualWieldPenalty(0), -6);
            check("dual-off", source->GetDualWieldPenalty(1), -10);
            source->m_equipment.m_items[44] = &dagger;
            check("dual-light", source->GetDualWieldPenalty(1), -8);
            source->m_baseStats.m_feats[0] |= 1 << CGAMESPRITE_FEAT_AMBIDEXTERITY;
            source->m_baseStats.m_feats[2] |= 1 << (CGAMESPRITE_FEAT_TWO_WEAPON_FIGHTING - 64);
            check("dual-feats-main", source->GetDualWieldPenalty(0), -2);
            check("dual-feats-off", source->GetDualWieldPenalty(1), -2);
            memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
            source->m_derivedStats.m_nClassLevels[CAIOBJECTTYPE_C_RANGER - 1] = 1;
            check("dual-ranger", source->GetDualWieldPenalty(1), -2);
            source->m_equipment.m_selectedWeapon = 42;
            check("dual-selection-42", source->GetDualWieldPenalty(1), 0);

            ITEM_ABILITY ability = {};
            ability.type = 1;
            INT baseline = source->m_derivedStats.m_cToHitBonusList.GetBonus(target->GetAIType());
            check("situational-melee", source->GetSituationalAttackBonus(target, &ability), baseline);
            target->m_derivedStats.m_generalState = 8;
            check("situational-stunned", source->GetSituationalAttackBonus(target, &ability), baseline + 2);
            target->m_derivedStats.m_generalState = 0;
            source->m_derivedStats.m_generalState = 0x10;
            check("situational-invisible-dword", source->GetSituationalAttackBonus(target, &ability), baseline - 327678);
            target->m_baseStats.m_feats[0] |= 1 << CGAMESPRITE_FEAT_BLIND_FIGHT;
            check("situational-target-blind-fight", source->GetSituationalAttackBonus(target, &ability), baseline);
            source->m_derivedStats.m_generalState = 0;
            source->m_pos = CPoint(0, 0);
            // Far from each threshold, independent of the actors' personal space.
            ability.type = 2;
            target->m_pos = CPoint(0, 0);
            check("ranged-close-word", source->GetSituationalAttackBonus(target, &ability), baseline + 65528);
            source->m_baseStats.m_feats[1] |= 1 << (CGAMESPRITE_FEAT_PRECISE_SHOT - 32);
            check("ranged-precise-shot", source->GetSituationalAttackBonus(target, &ability), baseline + 65532);
            source->m_derivedStats.m_generalState = 0x10;
            check("ranged-bypasses-blind-fight", source->GetSituationalAttackBonus(target, &ability), baseline - 262146);
            source->m_derivedStats.m_generalState = 0;
            target->m_pos = CPoint(16 * 20, 0);
            check("ranged-normal", source->GetSituationalAttackBonus(target, &ability), baseline);
            target->m_pos = CPoint(16 * 50, 0);
            check("ranged-medium-word", source->GetSituationalAttackBonus(target, &ability), baseline + 65534);
            target->m_pos = CPoint(16 * 90, 0);
            check("ranged-long-word", source->GetSituationalAttackBonus(target, &ability), baseline + 65531);

            const CRuleTables& rules = game->GetRuleTables();
            check("iterative-first", rules.GetIterativeAttackBonus(16, 5, 0), 16);
            check("iterative-second", rules.GetIterativeAttackBonus(16, 5, 1), 11);
            check("iterative-fourth", rules.GetIterativeAttackBonus(16, 5, 3), 1);
            check("iterative-floor", rules.GetIterativeAttackBonus(16, 5, 4), 0);
            check("iterative-no-step", rules.GetIterativeAttackBonus(7, 0, 3), 7);

            memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
            source->m_derivedStats.m_spellStates[SPLSTATE_TENSERS_TRANSFORMATION] = false;
            ability.type = 1;
            check("proficiency-untrained-party", source->GetWeaponProficiencyHitBonus(&sword, &ability), -4);
            ability.abilityFlags = 0x10000;
            check("proficiency-touch-exempt", source->GetWeaponProficiencyHitBonus(&sword, &ability), 0);
            ability.abilityFlags = 0;
            source->m_id = -1;
            check("proficiency-untrained-npc", source->GetWeaponProficiencyHitBonus(&sword, &ability),
                atol(rules.m_tWeaponSpecialization.GetAt(CPoint(0, 0))));
            source->m_id = sourceId;
            source->m_baseStats.m_feats[1] |= 1 << (CGAMESPRITE_FEAT_MARTIAL_LARGESWORD - 32);
            source->m_baseStats.m_featLargeSword = 2;
            INT rank2 = atol(rules.m_tWeaponSpecialization.GetAt(CPoint(0, 2)));
            check("proficiency-sword-focus", source->GetWeaponProficiencyHitBonus(&sword, &ability), rank2);
            source->m_baseStats.m_featLargeSword = 3;
            INT rank3 = atol(rules.m_tWeaponSpecialization.GetAt(CPoint(0, 3)));
            check("proficiency-sword-specialization", source->GetWeaponProficiencyHitBonus(&sword, &ability), rank3);

            CItem specialRace(CResRef("ZZA5TST"), 1, 0, 0, 0, 0);
            CItem specialAlignmentBroken(CResRef("ZZ05TST"), 1, 0, 0, 0, 0);
            CItem specialAlignment(CResRef("ZZ35TST"), 1, 0, 0, 0, 0);
            ability.type = 1;
            target->m_typeAI.m_nRace = 156;
            check("special-race-match", source->GetSpecialWeaponHitBonus(target, &specialRace, &ability), 1);
            // The melee helper reads the resref without demanding the weapon.
            // Keep the resource binding intact and restore its name afterwards.
            specialRace.cResRef = CResRef("ZZA0TST");
            check("special-negative-endpoint", source->GetSpecialWeaponHitBonus(target, &specialRace, &ability), -5);
            specialRace.cResRef = CResRef("ZZA9TST");
            check("special-positive-endpoint", source->GetSpecialWeaponHitBonus(target, &specialRace, &ability), 5);
            const char* unusedCodes[] = { "ZZ65TST", "ZZ75TST", "ZZ85TST", "ZZ95TST" };
            for (INT i = 0; i < 4; ++i) {
                specialRace.cResRef = CResRef(unusedCodes[i]);
                check(unusedCodes[i], source->GetSpecialWeaponHitBonus(target, &specialRace, &ability), 0);
            }
            specialRace.cResRef = CResRef("ZZA5TST");
            ability.abilityFlags = 0;
            check("critical-sword", specialRace.GetCriticalRangeBonus(&ability), 1);
            check("critical-bow", specialAlignment.GetCriticalRangeBonus(&ability), 0);
            check("critical-null-ability", specialRace.GetCriticalRangeBonus(NULL), 1);
            ability.abilityFlags = 0x20000;
            check("critical-keen-sword", specialRace.GetCriticalRangeBonus(&ability), 2);
            check("critical-keen-bow", specialAlignment.GetCriticalRangeBonus(&ability), 1);
            ability.abilityFlags = 0;
            target->m_typeAI.m_nRace = 157;
            check("special-race-miss", source->GetSpecialWeaponHitBonus(target, &specialRace, &ability), 0);
            target->m_typeAI.m_nAlignment = CAIOBJECTTYPE_LAWFUL_GOOD;
            check("special-alignment-high-mask-bug", source->GetSpecialWeaponHitBonus(target, &specialAlignmentBroken, &ability), 0);
            check("special-alignment-low-mask-match", source->GetSpecialWeaponHitBonus(target, &specialAlignment, &ability), 1);
            target->m_typeAI.m_nAlignment = CAIOBJECTTYPE_LAWFUL_NEUTRAL;
            check("special-alignment-miss", source->GetSpecialWeaponHitBonus(target, &specialAlignment, &ability), 0);
            target->m_typeAI.m_nRace = 157;
            target->m_typeAI.m_nAlignment = CAIOBJECTTYPE_LAWFUL_GOOD;
            source->m_equipment.m_items[43] = &specialAlignment;
            ability.type = 2;
            ability.launcherType = 1;
            check("special-launcher-overrides-weapon", source->GetSpecialWeaponHitBonus(target, &specialRace, &ability), 1);
            source->m_equipment.m_items[43] = &sword;

            const char* ammunition[] = { "ZRCARROW", "ZRCBOLT", "ZRCSTONE" };
            const char* launchers[] = { "ZRCBOW", "ZRCXBOW", "ZRCSLING" };
            const UINT feats[] = { CGAMESPRITE_FEAT_MARTIAL_BOW, CGAMESPRITE_FEAT_SIMPLE_CROSSBOW, CGAMESPRITE_FEAT_SIMPLE_MISSILE };
            source->m_baseStats.m_featLargeSword = 2;
            for (INT i = 0; i < 3; ++i) {
                CItem ammo(CResRef(ammunition[i]), 1, 0, 0, 0, 0);
                CItem launcher(CResRef(launchers[i]), 1, 0, 0, 0, 0);
                source->m_equipment.m_items[43] = &launcher;
                source->m_baseStats.m_feats[feats[i] >> 5] |= 1 << (feats[i] & 31);
                source->m_baseStats.m_featBow = 2;
                source->m_baseStats.m_featCrossbow = 2;
                source->m_baseStats.m_featMissile = 2;
                ability.type = 2;
                ability.launcherType = static_cast<BYTE>(i + 1);
                check(ammunition[i], source->GetWeaponProficiencyHitBonus(&ammo, &ability), rank2);
                source->m_baseStats.m_featBow = 1;
                source->m_baseStats.m_featCrossbow = 1;
                source->m_baseStats.m_featMissile = 1;
                check("proficiency-keeps-higher-weapon-rank", source->GetWeaponProficiencyHitBonus(&sword, &ability), rank2);
                source->m_equipment.m_items[43] = &sword;
            }

            // Fixed CRT seeds assert thresholds AND consumption of the reroll.
            // This isolated scenario deliberately reseeds the thread's RNG.
            memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
            source->m_derivedStats.m_generalState = 0;
            target->m_derivedStats.m_generalState = 0;
            target->m_bHiding = FALSE;
            target->m_baseStats.m_visible = 0;
            source->m_derivedStats.m_nLuck = 0;
            srand(1);
            check("concealment-none", source->CheckConcealment(target), TRUE);
            check("concealment-none-no-roll", rand(), 41);
            target->m_derivedStats.m_generalState = 0x20000000;
            source->m_derivedStats.m_nLuck = 19;
            srand(0);
            check("concealment-blur-below-20", source->CheckConcealment(target), FALSE);
            source->m_derivedStats.m_nLuck = 20;
            srand(0);
            check("concealment-blur-at-20", source->CheckConcealment(target), TRUE);
            target->m_derivedStats.m_generalState = 0;
            target->m_bHiding = TRUE;
            source->m_derivedStats.m_nLuck = 19;
            srand(0);
            check("concealment-hiding", source->CheckConcealment(target), FALSE);
            target->m_bHiding = 2;
            check("concealment-hiding-exact-one", source->CheckConcealment(target), TRUE);
            target->m_bHiding = FALSE;
            target->m_derivedStats.m_generalState = 0x10;
            check("concealment-invisible-without-visible-flag", source->CheckConcealment(target), TRUE);
            target->m_baseStats.m_visible = 1;
            source->m_derivedStats.m_nLuck = 49;
            srand(0);
            check("concealment-invisible-below-50", source->CheckConcealment(target), FALSE);
            source->m_derivedStats.m_nLuck = 50;
            srand(0);
            check("concealment-invisible-at-50", source->CheckConcealment(target), TRUE);
            target->m_derivedStats.m_generalState = 0;
            source->m_derivedStats.m_generalState = 0x40000;
            source->m_derivedStats.m_nLuck = 49;
            srand(0);
            check("concealment-blind-source", source->CheckConcealment(target), FALSE);
            source->m_derivedStats.m_nLuck = 0;
            source->m_baseStats.m_feats[0] |= 1 << CGAMESPRITE_FEAT_BLIND_FIGHT;
            srand(1);
            check("blind-fight-reroll-success", source->CheckConcealment(target), TRUE);
            check("blind-fight-consumed-two-rolls", rand(), 6334);
            srand(0);
            check("blind-fight-reroll-failure", source->CheckConcealment(target), FALSE);
            source->m_derivedStats.m_nLuck = 50;
            srand(1);
            check("blind-fight-first-roll-success", source->CheckConcealment(target), TRUE);
            check("blind-fight-no-unneeded-reroll", rand(), 18467);
        }
        source->m_baseStats = savedBase;
        target->m_baseStats = savedTargetBase;
        target->m_typeAI.m_nRace = targetRace;
        target->m_typeAI.m_nAlignment = targetAlignment;
        source->m_pos = sourcePos;
        target->m_pos = targetPos;
        source->m_derivedStats.m_generalState = sourceState;
        target->m_derivedStats.m_generalState = targetState;
        source->m_derivedStats.m_nDEX = dex;
        source->m_derivedStats.m_nWIS = wis;
        source->m_derivedStats.m_nLuck = luck;
        target->m_bHiding = hiding;
        source->m_derivedStats.m_spellStates[SPLSTATE_TENSERS_TRANSFORMATION] = tenser;
        source->m_derivedStats.m_nACNaturalBonus = natural;
        source->m_derivedStats.m_nACDeflectionBonus = deflection;
        source->m_derivedStats.m_nACDodgeBonus = dodge;
        source->m_derivedStats.m_nClassLevels[CAIOBJECTTYPE_C_RANGER - 1] = ranger;
        source->m_nWeaponSet = weaponSet;
        source->m_equipment.m_selectedWeapon = selected;
        source->m_equipment.m_items[1] = armor;
        source->m_equipment.m_items[43] = mainhand;
        source->m_equipment.m_items[44] = offhand;
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, "combatrules", "missing fixtures or numeric mismatch (state restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"combatrules\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combathit") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, "combathit", "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, "combathit", "source unavailable");
            return;
        }
        if (objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, "combathit", "target unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(sourceObject);
        CGameSprite* target = static_cast<CGameSprite*>(targetObject);
        CDerivedStatsTemplate sourceStats = source->m_derivedStats, targetStats = target->m_derivedStats;
        auto sourceBase = source->m_baseStats, targetBase = target->m_baseStats;
        BYTE sourceRace = source->m_typeAI.m_nRace, targetRace = target->m_typeAI.m_nRace;
        BYTE sourceEA = source->m_typeAI.m_nEnemyAlly, targetEA = target->m_typeAI.m_nEnemyAlly;
        BYTE sourceClass = source->m_typeAI.m_nClass;
        auto sourceClassMask = source->m_typeAI.m_nClassMask;
        BYTE sourceSet = source->m_nWeaponSet, targetSet = target->m_nWeaponSet;
        auto sourceSelected = source->m_equipment.m_selectedWeapon;
        auto sourceAbility = source->m_equipment.m_selectedWeaponAbility;
        CPoint sourcePos = source->m_pos, targetPos = target->m_pos;
        BOOL hiding = target->m_bHiding;
        auto timeStop = game->m_nTimeStop, timeStopCaster = game->m_nTimeStopCaster;
        auto difficulty = game->GetOptions()->m_nDifficultyLevel;
        BOOL shake = game->GetOptions()->m_bCriticalHitScreenShake;
        CItem* sourceItems[sizeof(source->m_equipment.m_items) / sizeof(CItem*)];
        CItem* targetItems[sizeof(target->m_equipment.m_items) / sizeof(CItem*)];
        memcpy(sourceItems, source->m_equipment.m_items, sizeof(sourceItems));
        memcpy(targetItems, target->m_equipment.m_items, sizeof(targetItems));
        CItem sword(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
        sword.Demand();
        ITEM_ABILITY* ability = sword.GetAbility(0);
        BOOL valid = ability != NULL && source->m_derivedStats.m_cToHitBonusList.IsEmpty();
        INT checks = 0;
        auto check = [&](const char* name, INT actual, INT expected) {
            Emit("{\"step\":%d,\"op\":\"combathit-case\",\"case\":\"%s\",\"actual\":%d,\"expected\":%d}", nStep, name, actual, expected);
            valid = valid && actual == expected;
            ++checks;
        };
        if (valid) {
            ITEM_ABILITY savedAbility = *ability;
            static_cast<CDerivedStatsTemplate&>(source->m_derivedStats) = CDerivedStatsTemplate{};
            static_cast<CDerivedStatsTemplate&>(target->m_derivedStats) = CDerivedStatsTemplate{};
            source->m_derivedStats.m_nSTR = source->m_derivedStats.m_nDEX = source->m_derivedStats.m_nCHR = 10;
            target->m_derivedStats.m_nDEX = 10;
            source->m_derivedStats.m_nClassLevels[CAIOBJECTTYPE_C_FIGHTER - 1] = 10;
            source->m_derivedStats.m_nNumberOfAttacks = 2;
            source->m_typeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
            source->m_typeAI.m_nClassMask = CLASSMASK_FIGHTER;
            source->m_typeAI.m_nRace = target->m_typeAI.m_nRace = CAIOBJECTTYPE_R_HUMAN;
            source->m_typeAI.m_nEnemyAlly = target->m_typeAI.m_nEnemyAlly = CAIOBJECTTYPE_EA_PC;
            memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
            memset(target->m_baseStats.m_feats, 0, sizeof(target->m_baseStats.m_feats));
            memset(source->m_baseStats.m_classLevels, 0, sizeof(source->m_baseStats.m_classLevels));
            memset(target->m_baseStats.m_classLevels, 0, sizeof(target->m_baseStats.m_classLevels));
            source->m_baseStats.m_fighterLevel = 10;
            source->m_baseStats.m_featLargeSword = 0;
            source->m_baseStats.m_specFlags = target->m_baseStats.m_specFlags = 0;
            source->m_baseStats.m_attackBase = 10;
            target->m_baseStats.m_visible = 0;
            target->m_bHiding = FALSE;
            memset(source->m_equipment.m_items, 0, sizeof(sourceItems));
            memset(target->m_equipment.m_items, 0, sizeof(targetItems));
            source->m_nWeaponSet = target->m_nWeaponSet = 0;
            source->m_equipment.m_items[43] = &sword;
            source->m_equipment.m_selectedWeapon = 43;
            source->m_equipment.m_selectedWeaponAbility = 0;
            source->m_pos = CPoint(1000, 1000);
            target->m_pos = CPoint(1120, 1000);
            game->m_nTimeStop = 0;
            game->GetOptions()->m_bCriticalHitScreenShake = FALSE;
            game->GetOptions()->m_nDifficultyLevel = 3;
            ability->abilityFlags = 0;
            ability->thac0Bonus = 0;
            ability->damageType = 2;
            auto attack = [&](const char* name, UINT seed, INT expectedHit, INT expectedCritical, INT index = 0, INT offhand = 0, BYTE count = 1) {
                srand(seed);
                BOOL critical = 0;
                BOOLEAN hit = source->CheckAttackHit(target, &sword, index, &critical, offhand, count);
                check(name, hit, expectedHit);
                check("critical-result", critical, expectedCritical);
            };
            // Fighter 10, STR 10, untrained sword: +6 against the controlled AC.
            target->m_derivedStats.m_nACNaturalBonus = 6;
            source->m_derivedStats.m_nLuck = 9;
            attack("melee-equals-ac", 0, 1, 0);
            target->m_derivedStats.m_nACNaturalBonus = 7;
            attack("melee-one-below-ac", 0, 0, 0);
            target->m_derivedStats.m_nACNaturalBonus = -3;
            source->m_derivedStats.m_nLuck = 0;
            attack("natural-one-fails", 0, 0, 0);
            attack("confirmation-one-can-succeed", 9563, 1, 1);
            target->m_derivedStats.m_nACNaturalBonus = -2;
            attack("confirmation-one-below-ac", 9563, 1, 0);
            target->m_derivedStats.m_nACNaturalBonus = 1000;
            attack("natural-twenty-hits-high-ac", 9563, 1, 0);
            target->m_derivedStats.m_nACNaturalBonus = -3;
            attack("sword-nineteen-threat", 9063, 1, 1);
            source->m_derivedStats.m_nLuck = 17;
            attack("sword-eighteen-not-threat", 0, 1, 0);
            source->m_baseStats.m_feats[0] |= 1 << CGAMESPRITE_FEAT_IMPROVED_CRITICAL;
            attack("improved-critical-eighteen", 0, 1, 1);
            source->m_baseStats.m_feats[0] = 0;
            ability->abilityFlags = 0x20000;
            attack("keen-eighteen", 0, 1, 1);
            ability->abilityFlags = 0;
            target->m_baseStats.m_specFlags = 2;
            source->m_derivedStats.m_nLuck = 0;
            attack("target-immune-to-critical", 9563, 1, 0);
            target->m_baseStats.m_specFlags = 0;
            target->m_derivedStats.m_generalState = 0x20000000;
            attack("concealment-overrides-natural-twenty", 9563, 0, 0);
            target->m_derivedStats.m_generalState = 0;
            target->m_derivedStats.m_nACNaturalBonus = 1000;
            target->m_derivedStats.m_generalState = 0x20;
            attack("helpless-autohit", 0, 1, 0);
            target->m_derivedStats.m_generalState = 0;
            target->m_derivedStats.m_spellStates[SPLSTATE_TORTOISE_SHELL] = true;
            attack("tortoise-autohit", 0, 1, 0);
            target->m_derivedStats.m_spellStates.reset();
            game->m_nTimeStop = 1;
            game->m_nTimeStopCaster = sourceId;
            attack("time-stop-autohit", 0, 1, 0);
            game->m_nTimeStopCaster = targetId;
            attack("time-stop-caster-not-autohit", 0, 0, 0);
            game->m_nTimeStop = 0;
            ability->thac0Bonus = SHORT_MAX;
            attack("weapon-autohit-sentinel", 0, 1, 0);
            ability->thac0Bonus = 0;
            target->m_derivedStats.m_nACNaturalBonus = -3;
            target->m_derivedStats.m_spellStates[SPLSTATE_BLINK] = true;
            attack("target-blink-miss", 1, 0, 0);
            target->m_derivedStats.m_spellStates.reset();
            source->m_derivedStats.m_spellStates[SPLSTATE_BLINK] = true;
            attack("source-blink-miss", 23, 0, 0);
            // MSVC rand seed 1 starts at 41 (passes the source's 20% check).
            source->m_derivedStats.m_nLuck = 19;
            attack("source-blink-pass", 1, 1, 1);
            source->m_derivedStats.m_spellStates.reset();
            source->m_derivedStats.m_nLuck = 9;
            target->m_derivedStats.m_nACNaturalBonus = 6;
            attack("second-attack-penalty", 0, 0, 0, 0, 0, 2);
            attack("missing-ability-fallback", 0, 1, 0, 99);
            source->m_equipment.m_items[44] = &sword;
            target->m_derivedStats.m_nACNaturalBonus = -4;
            attack("offhand-resets-attack-count", 0, 1, 0, 0, 0, 2);
            source->m_equipment.m_items[44] = NULL;
            target->m_derivedStats.m_nACNaturalBonus = 6;
            target->m_derivedStats.m_nACArmorBonus = 10;
            attack("armor-blocks-normal-hit", 0, 0, 0);
            ability->abilityFlags = 0x10000;
            attack("touch-ignores-armor", 0, 1, 0);
            ability->abilityFlags = 0;
            target->m_derivedStats.m_nACArmorBonus = 0;
            source->m_typeAI.m_nEnemyAlly = 255;
            game->GetOptions()->m_nDifficultyLevel = 1;
            attack("easy-npc-penalty", 0, 0, 0);
            game->GetOptions()->m_nDifficultyLevel = 5;
            attack("hard-npc-bonus", 0, 1, 0);
            source->m_typeAI.m_nEnemyAlly = CAIOBJECTTYPE_EA_PC;
            source->m_pos = CPoint(0, 0);
            target->m_pos = CPoint(320, 0);
            const char* ammunition[] = { "ZRCARROW", "ZRCBOLT", "ZRCSTONE" };
            const char* launchers[] = { "ZRCBOW", "ZRCXBOW", "ZRCSLING" };
            for (INT i = 0; i < 3; ++i) {
                CItem ammo(CResRef(ammunition[i]), 1, 0, 0, 0, 0);
                CItem launcher(CResRef(launchers[i]), 1, 0, 0, 0, 0);
                source->m_equipment.m_items[43] = &launcher;
                srand(0);
                BOOL critical = FALSE;
                check(ammunition[i], source->CheckAttackHit(target, &ammo, 0, &critical, 0, 1), TRUE);
                check("ranged-no-critical", critical, FALSE);
                source->m_derivedStats.m_spellStates[SPLSTATE_FEAT_RAPID_SHOT] = true;
                srand(0);
                check("rapid-shot-penalty", source->CheckAttackHit(target, &ammo, 0, &critical, 0, 1), FALSE);
                source->m_derivedStats.m_spellStates[SPLSTATE_FEAT_RAPID_SHOT] = false;
                source->m_equipment.m_items[43] = &sword;
                ammo.ReleaseAll();
                launcher.ReleaseAll();
            }
            auto damage = [&](const char* name, INT expected, DWORD flags = 0, BOOL critical = FALSE, INT offhand = 0, INT index = 0) {
                srand(1);
                CGameEffect* effect = source->CreateWeaponDamage(&sword, NULL, index, critical,
                    target->GetAIType(), 0, 0, target, offhand);
                check(name, effect->m_effectAmount, expected);
                check("damage-type", effect->m_dwFlags, flags);
                check("damage-source", effect->m_sourceID, sourceId);
                delete effect;
            };
            ability->type = 1;
            ability->abilityFlags = 0;
            ability->damageDice = 6;
            ability->damageDiceCount = 2;
            ability->damageDiceBonus = 3;
            ability->damageType = 2;
            source->m_derivedStats.m_nLuck = 0;
            damage("base-two-dice-plus-bonus", 8);
            damage("base-critical-doubles", 16, 0, TRUE);
            source->m_derivedStats.m_nLuck = 19;
            damage("luck-caps-each-die", 15);
            source->m_derivedStats.m_spellStates[SPLSTATE_KAI_64] = true;
            source->m_derivedStats.m_nLuck = -20;
            damage("kai-maximizes-despite-negative-luck", 15);
            source->m_derivedStats.m_spellStates.reset();
            source->m_derivedStats.m_nLuck = 0;
            source->m_derivedStats.m_nSTR = 18;
            ability->abilityFlags = 1;
            damage("strength-mainhand", 12);
            DWORD savedFlags = sword.pRes->m_pHeader->itemFlags;
            sword.pRes->m_pHeader->itemFlags |= 2;
            damage("strength-two-handed", 14);
            source->m_derivedStats.m_nSTR = 20;
            damage("strength-two-handed-rounds-down", 15);
            sword.pRes->m_pHeader->itemFlags = savedFlags;
            source->m_derivedStats.m_nSTR = 18;
            source->m_equipment.m_items[44] = &sword;
            damage("strength-offhand-halved", 10, 0, FALSE, 1);
            source->m_derivedStats.m_nSTR = 6;
            damage("negative-strength-not-halved", 6, 0, FALSE, 1);
            source->m_equipment.m_items[44] = NULL;
            ability->abilityFlags = 0;
            source->m_derivedStats.m_nSTR = 10;
            source->m_derivedStats.m_DamageBonusRight = 0x10002;
            damage("right-hand-bonus-low-word", 10);
            source->m_derivedStats.m_DamageBonusRight = 0;
            source->m_derivedStats.m_nDamageBonus = -20;
            damage("negative-total-floors-at-one", 1);
            source->m_derivedStats.m_nDamageBonus = 0;
            ability->damageDiceBonus = -2;
            damage("signed-weapon-bonus-final-word", 3);
            ability->damageDiceBonus = 0;
            ability->damageDiceCount = 0;
            damage("zero-dice-has-zero-floor", 0);
            source->m_derivedStats.m_nDamageBonus = 10;
            damage("no-damage-ignores-derived-bonus", 0);
            source->m_derivedStats.m_nDamageBonus = 0;
            ability->damageDiceCount = 2;
            ability->damageDiceBonus = 3;
            const DWORD damageTypes[] = { 0x100000, 0, 0x1000000, 0x2000000, 0x8000000, 0x100000, 0x100000, 0x1000000, 0x4000000 };
            for (INT i = 0; i < 9; ++i) {
                ability->damageType = i + 1;
                damage("base-damage-descriptor", 8, damageTypes[i]);
            }
            target->m_derivedStats.m_nResistPiercing = 10;
            ability->damageType = 6;
            damage("mixed-piercing-crushing", 8, 0);
            ability->damageType = 7;
            damage("mixed-piercing-slashing", 8, 0x1000000);
            target->m_derivedStats.m_nResistCrushing = 10;
            ability->damageType = 8;
            damage("mixed-crushing-slashing-original-order", 8, 0);
            target->m_derivedStats.m_nResistPiercing = target->m_derivedStats.m_nResistCrushing = 0;
            damage("base-fallback-ability", 1, 0, FALSE, 0, 99);
            *ability = savedAbility;
        }
        sword.ReleaseAll();
        static_cast<CDerivedStatsTemplate&>(source->m_derivedStats) = sourceStats;
        static_cast<CDerivedStatsTemplate&>(target->m_derivedStats) = targetStats;
        source->m_baseStats = sourceBase;
        target->m_baseStats = targetBase;
        source->m_typeAI.m_nRace = sourceRace;
        target->m_typeAI.m_nRace = targetRace;
        source->m_typeAI.m_nEnemyAlly = sourceEA;
        target->m_typeAI.m_nEnemyAlly = targetEA;
        source->m_typeAI.m_nClass = sourceClass;
        source->m_typeAI.m_nClassMask = sourceClassMask;
        source->m_nWeaponSet = sourceSet;
        target->m_nWeaponSet = targetSet;
        source->m_equipment.m_selectedWeapon = sourceSelected;
        source->m_equipment.m_selectedWeaponAbility = sourceAbility;
        memcpy(source->m_equipment.m_items, sourceItems, sizeof(sourceItems));
        memcpy(target->m_equipment.m_items, targetItems, sizeof(targetItems));
        source->m_pos = sourcePos;
        target->m_pos = targetPos;
        target->m_bHiding = hiding;
        game->m_nTimeStop = timeStop;
        game->m_nTimeStopCaster = timeStopCaster;
        game->GetOptions()->m_nDifficultyLevel = difficulty;
        game->GetOptions()->m_bCriticalHitScreenShake = shake;
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, "combathit", "fixture/precondition or numeric mismatch (state restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"combathit\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatammo") == 0) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || pGame->GetNumCharacters() == 0) {
            Fail(nStep, step.op, "requires world and party");
            return;
        }
        LONG id = pGame->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (pGame->GetObjectArray()->GetDeny(id, CGameObjectArray::THREAD_ASYNCH,
                &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(object);
        CItem* savedItems[51];
        memcpy(savedItems, source->m_equipment.m_items, sizeof(savedItems));
        BYTE savedSlot = source->m_equipment.m_selectedWeapon;
        WORD savedAbility = source->m_equipment.m_selectedWeaponAbility;
        BYTE savedSet = source->m_nWeaponSet;
        BYTE savedAttacks = source->m_baseStats.m_numberOfAttacksBase;
        CButtonData savedButtons[8];
        BYTE savedAmmo[8];
        for (int i = 0; i < 8; ++i) savedButtons[i] = source->m_quickWeapons[i];
        memcpy(savedAmmo, source->field_3D3A, sizeof(savedAmmo));
        BOOL valid = TRUE;
        int checks = 0;
        const char* ammoNames[] = { "ZRCARROW", "ZRCBOLT", "ZRCSTONE" };
        const char* launcherNames[] = { "ZRCBOW", "ZRCXBOW", "ZRCSLING" };
        for (int kind = 0; kind < 3 && valid; ++kind) {
            CItem launcher(CResRef(launcherNames[kind]), 1, 0, 0, 0, 0);
            CItem low(CResRef(ammoNames[kind]), 7, 0, 0, 0, 0);
            CItem high(CResRef(ammoNames[kind]), 19, 0, 0, 0, 0);
            CItem wrong(CResRef(ammoNames[(kind + 1) % 3]), 99, 0, 0, 0, 0);
            CItem melee(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
            BOOL demanded = launcher.Demand();
            valid = demanded && launcher.GetAbility(0) != NULL;
            if (demanded) launcher.Release();
            if (!valid) break;
            memset(source->m_equipment.m_items, 0, sizeof(savedItems));
            source->m_nWeaponSet = 0;
            source->m_equipment.m_items[43] = &launcher;
            source->m_equipment.m_items[11] = &low;
            source->m_equipment.m_items[12] = &wrong;
            source->m_equipment.m_items[13] = &high;
            source->m_equipment.m_items[10] = savedItems[10];
            // Avoid unequipping saved items while fixture pointers occupy the slots.
            source->m_equipment.m_selectedWeapon = 255;
            const WORD counts[][2] = { {7, 19}, {19, 19}, {19, 0}, {0, 0} };
            const int slots[] = {13, 11, 11, 255};
            for (int trial = 0; trial < 4; ++trial) {
                low.SetUsageCount(0, counts[trial][0]);
                high.SetUsageCount(0, counts[trial][1]);
                source->m_equipment.m_selectedWeapon = 255;
                SHORT result = source->SelectLauncherAmmo(&launcher, FALSE);
                int actualSlot = source->m_equipment.m_selectedWeapon;
                BOOL passed = result == (trial == 3 ? CGameAIBase::ACTION_ERROR : CGameAIBase::ACTION_DONE)
                    && actualSlot == slots[trial];
                if (trial != 3) {
                    passed = passed && source->m_quickWeapons[0].m_abilityId.m_itemNum == slots[trial]
                        && source->field_3D3A[0] == slots[trial];
                }
                Emit("{\"step\":%d,\"op\":\"combatammo-case\",\"launcher\":\"%s\","
                     "\"trial\":%d,\"result\":%d,\"slot\":%d,\"passed\":%d}",
                    nStep, launcherNames[kind], trial, result, actualSlot, passed);
                valid = valid && passed;
                ++checks;
            }
            source->m_equipment.m_items[45] = &melee;
            source->m_quickWeapons[2].m_abilityId.m_itemNum = 45;
            source->m_quickWeapons[2].m_abilityId.m_abilityNum = 0;
            source->m_equipment.m_selectedWeapon = 255;
            SHORT fallback = source->SelectLauncherAmmo(&launcher, TRUE);
            BOOL fallbackPassed = fallback == CGameAIBase::ACTION_DONE
                && source->m_nWeaponSet == 1 && source->m_equipment.m_selectedWeapon == 45;
            Emit("{\"step\":%d,\"op\":\"combatammo-fallback\",\"launcher\":\"%s\","
                 "\"result\":%d,\"set\":%d,\"slot\":%d,\"passed\":%d}",
                nStep, launcherNames[kind], fallback, source->m_nWeaponSet,
                source->m_equipment.m_selectedWeapon, fallbackPassed);
            valid = valid && fallbackPassed;
            ++checks;
            // Restore pointers before the stack-owned fixtures are destroyed.
            memcpy(source->m_equipment.m_items, savedItems, sizeof(savedItems));
        }
        memcpy(source->m_equipment.m_items, savedItems, sizeof(savedItems));
        source->m_nWeaponSet = savedSet;
        source->m_equipment.m_selectedWeapon = 255;
        source->SelectWeaponAbility(savedSlot, static_cast<BYTE>(savedAbility), FALSE, TRUE);
        source->m_baseStats.m_numberOfAttacksBase = savedAttacks;
        for (int i = 0; i < 8; ++i) source->m_quickWeapons[i] = savedButtons[i];
        memcpy(source->field_3D3A, savedAmmo, sizeof(savedAmmo));
        pGame->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, step.op, "ammo selection mismatch or fixture unavailable; equipment restored");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"combatammo\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "weaponpayload") == 0 || strcmp(step.op, "weapondamage") == 0) {
        BOOL baseDamage = strcmp(step.op, "weapondamage") == 0;
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || pGame->GetNumCharacters() < 2
            || strncmp(step.s[0], "ZRC", 3) != 0 || strlen(step.s[0]) > 8
            || (step.s[1][0] != '\0'
                && (strncmp(step.s[1], "ZRC", 3) != 0 || strlen(step.s[1]) > 8))) {
            Fail(nStep, "weaponpayload", "requires world, two party members and ZRC fixtures");
            return;
        }
        CGameObjectArray* objects = pGame->GetObjectArray();
        if (s_payloadTarget != -1) {
            CGameObject* object = NULL;
            BYTE rc = objects->GetShare(s_payloadTarget, CGameObjectArray::THREAD_ASYNCH,
                &object, INFINITE);
            if (rc != CGameObjectArray::SUCCESS || object == NULL) {
                Fail(nStep, "weaponpayload", "target unavailable after launch");
                return;
            }
            SHORT hp = static_cast<CGameSprite*>(object)->m_baseStats.m_hitPoints;
            objects->ReleaseShare(s_payloadTarget, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (s_payloadHP - hp < s_payloadDamage && ++s_waited < 600) {
                return;
            }
            Emit("{\"step\":%d,\"op\":\"%s\",\"item\":\"%s\",\"hpBefore\":%d,\"hpAfter\":%d,\"expectedDamage\":%d}",
                nStep, step.op, step.s[0], s_payloadHP, hp, s_payloadDamage);
            if (s_payloadHP - hp != s_payloadDamage) {
                Fail(nStep, "weaponpayload", "unexpected HP delta or projectile did not arrive");
                return;
            }
            s_payloadTarget = -1;
            s_waited = 0;
            s_pc++;
            return;
        }

        LONG sourceId = pGame->GetFixedOrderCharacterId(0);
        LONG targetId = pGame->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        BYTE rc = objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH,
            &sourceObject, INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            Fail(nStep, "weaponpayload", "source unavailable");
            return;
        }
        rc = objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH,
            &targetObject, INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, "weaponpayload", "target unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(sourceObject);
        CGameSprite* target = static_cast<CGameSprite*>(targetObject);
        CPoint targetPos = target->m_pos;
        CGameArea* area = source->m_pArea;
        SHORT hp = target->m_baseStats.m_hitPoints;
        BOOL valid = area != NULL && target->m_pArea == area
            && hp > (baseDamage ? 10 : (step.s[1][0] != '\0' ? 4 : 2));

        CItem weapon(CResRef(step.s[0]), 1, 0, 0, 0, 0);
        CItem launcher(CResRef(step.s[1]), 1, 0, 0, 0, 0);
        BOOL hasLauncher = step.s[1][0] != '\0';
        BOOL weaponDemanded = valid && weapon.Demand();
        valid = valid && weaponDemanded;
        ITEM_ABILITY* ability = valid ? weapon.GetAbility(0) : NULL;
        valid = valid && ability != NULL;
        USHORT missile = valid ? ability->missileType : 0;
        CProjectile* projectile = valid ? CProjectile::DecodeProjectile(missile, source, 0) : NULL;
        valid = valid && projectile != NULL;
        if (valid && baseDamage) {
            valid = source->m_derivedStats.m_cDamageBonusList.IsEmpty();
            if (valid) {
                CDerivedStatsTemplate savedStats = source->m_derivedStats;
                auto savedBase = source->m_baseStats;
                BYTE savedClass = source->m_typeAI.m_nClass;
                auto savedMask = source->m_typeAI.m_nClassMask;
                static_cast<CDerivedStatsTemplate&>(source->m_derivedStats) = CDerivedStatsTemplate{};
                source->m_derivedStats.m_nLuck = 19;
                source->m_derivedStats.m_nSTR = 10;
                memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
                source->m_baseStats.m_rogueLevel = 0;
                source->m_typeAI.m_nClass = 0;
                source->m_typeAI.m_nClassMask = 0;
                CGameEffect* effect = source->CreateWeaponDamage(&weapon, hasLauncher ? &launcher : NULL,
                    0, FALSE, target->GetAIType(), 0, 0, target, 0);
                valid = effect != NULL && effect->m_effectID == CGAMEEFFECT_DAMAGE
                    && effect->m_effectAmount == (hasLauncher ? 9 : 5)
                    && effect->m_dwFlags == 0 && effect->m_sourceID == sourceId;
                if (effect != NULL) {
                    projectile->m_effectList.AddTail(effect);
                }
                static_cast<CDerivedStatsTemplate&>(source->m_derivedStats) = savedStats;
                source->m_baseStats = savedBase;
                source->m_typeAI.m_nClass = savedClass;
                source->m_typeAI.m_nClassMask = savedMask;
                Emit("{\"step\":%d,\"op\":\"weapondamage-assembled\",\"item\":\"%s\",\"damage\":%ld,\"missile\":%u,\"valid\":%d}",
                    nStep, step.s[0], effect != NULL ? effect->m_effectAmount : -1, missile, valid);
            }
        }
        if (valid && !baseDamage) {
            // Transient selection for GetLauncher; restored before any engine tick.
            CItem* saved = source->m_equipment.m_items[43];
            BYTE savedSet = source->m_nWeaponSet;
            source->m_nWeaponSet = 0;
            source->m_equipment.m_items[43] = hasLauncher ? &launcher : NULL;
            source->ApplyWeaponEffects(projectile, NULL, 0, targetPos);
            source->ApplyWeaponEffects(projectile, &weapon, -1, targetPos);
            valid = projectile->m_effectList.IsEmpty();
            source->ApplyWeaponEffects(projectile, &weapon, 0, targetPos);
            source->m_equipment.m_items[43] = saved;
            source->m_nWeaponSet = savedSet;

            INT index = 0;
            POSITION pos = projectile->m_effectList.GetHeadPosition();
            while (pos != NULL) {
                CGameEffect* effect = projectile->m_effectList.GetNext(pos);
                valid = valid && effect->m_effectID == CGAMEEFFECT_DAMAGE
                    && effect->m_targetType == 2 && effect->m_sourceID == sourceId
                    && effect->m_source == source->m_pos && effect->m_target == targetPos
                    && effect->m_sourceRes == CResRef(index == 0 ? step.s[0] : step.s[1])
                    && effect->m_effectAmount == (index == 0 ? 1 : 2);
                ++index;
            }
            valid = valid && index == (hasLauncher ? 2 : 1);
            Emit("{\"step\":%d,\"op\":\"weaponpayload-assembled\",\"item\":\"%s\",\"launcher\":\"%s\",\"effects\":%d,\"missile\":%u,\"valid\":%d}",
                nStep, step.s[0], step.s[1], index, missile, valid);
        }
        if (weaponDemanded) {
            weapon.Release();
        }
        LONG height = valid ? projectile->DetermineHeight(source) : 0;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            if (projectile != NULL) {
                projectile->ClearEffects();
                delete projectile;
            }
            Fail(nStep, "weaponpayload", "fixture, party state or effect provenance mismatch");
            return;
        }
        s_payloadHP = hp;
        s_payloadDamage = baseDamage ? (hasLauncher ? 9 : 5) : (hasLauncher ? 3 : 1);
        s_payloadTarget = targetId;
        projectile->Fire(area, sourceId, targetId, targetPos, height, 0);
        return;
    }

    if (strcmp(step.op, "enddialog") == 0) {
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld) {
            Fail(nStep, "enddialog", "requires world");
            return;
        }
        g_pBaldurChitin->m_pEngineWorld->EndDialog(TRUE, TRUE);
        Emit("{\"step\":%d,\"op\":\"enddialog\",\"status\":\"ok\"}", nStep);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "resume") == 0) {
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld) {
            Fail(nStep, "resume", "requires world");
            return;
        }
        BOOL wasPaused = g_pBaldurChitin->m_pEngineWorld->m_bPaused;
        if (wasPaused) {
            g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
        }
        if (g_pBaldurChitin->m_pEngineWorld->m_bPaused) {
            Fail(nStep, "resume", "world remains paused");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"resume\",\"wasPaused\":%d,\"status\":\"ok\"}", nStep, wasPaused);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "forcepoint") == 0) {
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || step.s[0][0] == '\0' || strlen(step.s[0]) > 8) {
            Fail(nStep, "forcepoint", "requires world and a resref of 1-8 characters");
            return;
        }
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        CResRef resRef(step.s[0]);
        for (SHORT nSlot = 0; nSlot < pGame->GetNumCharacters(); nSlot++) {
            LONG id = pGame->GetFixedOrderCharacterId(nSlot);
            CGameObject* pObject = NULL;
            BYTE rc = pGame->GetObjectArray()->GetDeny(id,
                CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                continue;
            }
            BOOL queued = FALSE;
            if (pObject != NULL && pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
                BYTE nClass = 0;
                DWORD nSpecialization = 0;
                BYTE nLevel = 0;
                if (pSprite->FindAvailableSpell(resRef, nClass, nSpecialization, nLevel)) {
                    CAIAction action;
                    action.m_actionID = CAIAction::FORCESPELLPOINT;
                    action.m_dest = pSprite->GetPos();
                    action.m_specificID = 1;
                    action.SetString1(CString(step.s[0]));
                    pSprite->SetCurrAction(action);
                    queued = TRUE;
                }
            }
            pGame->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (queued) {
                Emit("{\"step\":%d,\"op\":\"forcepoint\",\"slot\":%d,\"status\":\"ok\"}", nStep, nSlot);
                s_pc++;
                return;
            }
        }
        Fail(nStep, "forcepoint", "no available party caster");
        return;
    }

    if (strcmp(step.op, "dump") == 0) {
        DumpTree(pActiveEngine, nStep);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "center") == 0) {
        CenterOnParty(nStep);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "ground") == 0) {
        DumpGround(nStep);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "bar") == 0) {
        DumpButtonArray(nStep);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "screen") == 0) {
        Emit("{\"step\":%d,\"op\":\"screen\",\"screen\":\"%s\"}",
            nStep, ScreenName(pActiveEngine));
        s_pc++;
        return;
    }

    if (strcmp(step.op, "waitgo") == 0) {
        char szGo[MAX_PATH];
        if (GetEnv("IWD2_RE_UI_GO", szGo, sizeof(szGo)) == 0) {
            Emit("{\"step\":%d,\"op\":\"waitgo\",\"status\":\"ok\",\"detail\":\"no gate set\"}",
                nStep);
            s_pc++;
            return;
        }
        FILE* fp = fopen(szGo, "rb");
        if (fp != NULL) {
            fclose(fp);
            Emit("{\"step\":%d,\"op\":\"waitgo\",\"status\":\"ok\",\"ticks\":%d}",
                nStep, s_waited);
            s_waited = 0;
            s_pc++;
            return;
        }
        if (s_waited++ > Num(step, 0, DEFAULT_WAIT_TICKS)) {
            Emit("{\"step\":%d,\"op\":\"waitgo\",\"status\":\"fail\","
                 "\"detail\":\"observer never signalled\"}", nStep);
            s_failed++;
            s_waited = 0;
            s_pc = s_count;
        }
        return;
    }

    if (strcmp(step.op, "sleep") == 0) {
        if (s_waited++ < Num(step, 0, 1)) {
            return;
        }
        s_waited = 0;
        s_pc++;
        return;
    }

    if (strcmp(step.op, "wait") == 0) {
        const char* szNow = ScreenName(pActiveEngine);
        if (strcmp(szNow, step.s[0]) == 0) {
            Emit("{\"step\":%d,\"op\":\"wait\",\"screen\":\"%s\",\"status\":\"ok\","
                 "\"ticks\":%d}", nStep, szNow, s_waited);
            s_waited = 0;
            s_pc++;
            return;
        }
        if (s_waited++ > Num(step, 0, DEFAULT_WAIT_TICKS)) {
            Emit("{\"step\":%d,\"op\":\"wait\",\"want\":\"%s\",\"got\":\"%s\","
                 "\"status\":\"fail\",\"detail\":\"timeout\"}",
                nStep, step.s[0], szNow);
            s_failed++;
            s_waited = 0;
            s_pc = s_count;
        }
        return;
    }

    if (strcmp(step.op, "goto") == 0) {
        // OnLeftPanelButtonClick is the engine's own screen switcher
        // (CBaldurEngine.cpp:768-833).
        DWORD dwButton = static_cast<DWORD>(Num(step, 0, 0));
        if (strcmp(step.s[0], "world") == 0) dwButton = 0;
        else if (strcmp(step.s[0], "spellbook") == 0) dwButton = 4;
        else if (strcmp(step.s[0], "inventory") == 0) dwButton = 5;
        else if (strcmp(step.s[0], "journal") == 0) dwButton = 6;
        else if (strcmp(step.s[0], "map") == 0) dwButton = 7;
        else if (strcmp(step.s[0], "character") == 0) dwButton = 8;
        else if (strcmp(step.s[0], "options") == 0) dwButton = 9;

        static_cast<CBaldurEngine*>(pActiveEngine)->OnLeftPanelButtonClick(dwButton);
        Emit("{\"step\":%d,\"op\":\"goto\",\"target\":\"%s\",\"button\":%lu,"
             "\"status\":\"ok\"}", nStep, step.s[0], dwButton);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "key") == 0) {
        // OnKeyDown takes the NUMBER of keys pressed this tick and reads the
        // virtual keys themselves out of the engine's own buffer, so a key has
        // to be staged there first.  Every engine exposes that buffer through
        // GetVirtualKeysFlags, so this works on any screen -- passing the key
        // code as the count instead makes the engine read a buffer nobody
        // filled, which is why ESCAPE used to do nothing outside the world.
        SHORT nKey = static_cast<SHORT>(Num(step, 0, 0));
        BYTE* pKeyFlags = pActiveEngine->GetVirtualKeysFlags();
        if (pKeyFlags != NULL) {
            pKeyFlags[0] = static_cast<BYTE>(nKey);
            pActiveEngine->OnKeyDown(1);
        }

        Emit("{\"step\":%d,\"op\":\"key\",\"vk\":%d,\"status\":\"ok\"}",
            nStep, Num(step, 0, 0));
        s_pc++;
        return;
    }

    if (strcmp(step.op, "hover") == 0) {
        CPoint pt(Num(step, 0, 0), Num(step, 1, 0));
        pActiveEngine->OnMouseMove(pt);
        Emit("{\"step\":%d,\"op\":\"hover\",\"x\":%ld,\"y\":%ld,\"status\":\"ok\"}",
            nStep, pt.x, pt.y);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "clickxy") == 0) {
        s_clickPt = CPoint(Num(step, 0, 0), Num(step, 1, 0));
        // The world does not pick from the point handed to OnLButtonDown: it
        // picks from m_ptMousePos, which the tick fills from the real cursor
        // (CGameArea.cpp:1466). A synthetic click with no move behind it hits
        // whatever the cursor last hovered, so move first, exactly as a hand
        // would. Menus are unaffected -- they hit-test the point they are given.
        s_clickRight = false;
        pActiveEngine->OnMouseMove(s_clickPt);
        pActiveEngine->OnLButtonDown(s_clickPt);
        Emit("{\"step\":%d,\"op\":\"clickxy\",\"x\":%ld,\"y\":%ld,\"status\":\"ok\"}",
            nStep, s_clickPt.x, s_clickPt.y);
        s_clickPhase = 1;
        return;
    }

    if (strcmp(step.op, "click") == 0 || strcmp(step.op, "rclick") == 0) {
        const bool bRight = step.op[0] == 'r';
        CUIPanel* pPanel = FindPanel(pActiveEngine, static_cast<DWORD>(Num(step, 0, 0)));
        if (pPanel == NULL) {
            Fail(nStep, step.op, "panel not found");
            return;
        }

        CUIControlBase* pControl = pPanel->GetControl(static_cast<DWORD>(Num(step, 1, 0)));
        if (pControl == NULL) {
            Fail(nStep, step.op, "control not found");
            return;
        }

        // Control origin is panel-relative -- CUIPanel::OnLButtonDown subtracts
        // the panel origin before hit-testing (CUIPanel.cpp:221) -- so add it
        // back to get the screen point the engine expects.
        s_clickPt = CPoint(
            pPanel->m_ptOrigin.x + pControl->m_ptOrigin.x + pControl->m_size.cx / 2,
            pPanel->m_ptOrigin.y + pControl->m_ptOrigin.y + pControl->m_size.cy / 2);

        s_clickRight = bRight;
        if (bRight) {
            pActiveEngine->OnRButtonDown(s_clickPt);
        } else {
            pActiveEngine->OnLButtonDown(s_clickPt);
        }
        Emit("{\"step\":%d,\"op\":\"%s\",\"panel\":%d,\"control\":%d,"
             "\"x\":%ld,\"y\":%ld,\"active\":%d,\"status\":\"ok\"}",
            nStep, step.op, Num(step, 0, 0), Num(step, 1, 0),
            s_clickPt.x, s_clickPt.y, pControl->m_bActive != 0 ? 1 : 0);
        s_clickPhase = 1;
        return;
    }

    if (strcmp(step.op, "expect") == 0) {
        if (strcmp(step.s[0], "picked") == 0) {
            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
            CGameArea* pArea = pGame != NULL ? pGame->GetVisibleArea() : NULL;
            int slot = Num(step, 0, -1);
            if (pArea == NULL || slot < 0 || slot >= pGame->GetNumCharacters()) {
                Fail(nStep, "expect", "picked requires a valid party slot and visible area");
                return;
            }
            LONG id = pGame->GetFixedOrderCharacterId(static_cast<SHORT>(slot));
            Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"picked\",\"slot\":%d,"
                 "\"expected\":%ld,\"actual\":%ld}", nStep, slot, id, pArea->m_iPicked);
            if (id == CGameObjectArray::INVALID_INDEX || pArea->m_iPicked != id) {
                Fail(nStep, "expect", "ground picker did not identify the requested party member");
                return;
            }
            s_pc++;
            return;
        }
        if (strcmp(step.s[0], "targeting") == 0) {
            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
            if (pGame == NULL) {
                Fail(nStep, "expect", "targeting requires game");
                return;
            }
            int state = pGame->GetState();
            int icon = pGame->GetIconIndex();
            int button = pGame->GetButtonArray()->m_nSelectedButton;
            Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"targeting\","
                 "\"state\":%d,\"icon\":%d,\"button\":%d}", nStep, state, icon, button);
            if (state != Num(step, 0, -1) || icon != Num(step, 1, -1)
                || button != Num(step, 2, -1)) {
                Fail(nStep, "expect", "targeting state/icon/button mismatch");
                return;
            }
            s_pc++;
            return;
        }
        if (strcmp(step.s[0], "partyvisible") == 0) {
            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
            CGameArea* pArea = pGame != NULL ? pGame->GetVisibleArea() : NULL;
            int slot = Num(step, 0, -1);
            if (pArea == NULL || slot < 0 || slot >= pGame->GetNumCharacters()) {
                Fail(nStep, "expect", "partyvisible requires a valid party slot and visible area");
                return;
            }
            LONG id = pGame->GetFixedOrderCharacterId(static_cast<SHORT>(slot));
            CGameObject* pMember = NULL;
            if (pGame->GetObjectArray()->GetShare(id, CGameObjectArray::THREAD_ASYNCH,
                    &pMember, INFINITE) != CGameObjectArray::SUCCESS) {
                Fail(nStep, "expect", "partyvisible member unavailable");
                return;
            }
            CInfinity* pInfinity = pArea->GetInfinity();
            CPoint pt(pMember->m_pos.x - pInfinity->nNewX + pInfinity->rViewPort.left,
                pMember->m_pos.y - pInfinity->nNewY + pInfinity->rViewPort.top);
            BOOL visible = pMember->m_pArea == pArea
                && pInfinity->rViewPort.PtInRect(pt);
            pGame->GetObjectArray()->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"partyvisible\",\"slot\":%d,"
                 "\"id\":%ld,\"x\":%ld,\"y\":%ld,\"visible\":%d}",
                nStep, slot, id, pt.x, pt.y, visible);
            if (!visible) {
                Fail(nStep, "expect", "party member is outside the visible viewport; center before targeting");
                return;
            }
            s_pc++;
            return;
        }

        if (strcmp(step.s[0], "screen") == 0) {
            const char* szNow = ScreenName(pActiveEngine);
            if (strcmp(szNow, step.s[1]) != 0) {
                Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"screen\",\"want\":\"%s\","
                     "\"got\":\"%s\",\"status\":\"fail\"}", nStep, step.s[1], szNow);
                s_failed++;
                s_pc = s_count;
                return;
            }
            Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"screen\",\"got\":\"%s\","
                 "\"status\":\"ok\"}", nStep, szNow);
            s_pc++;
            return;
        }

        if (strcmp(step.s[0], "controls") == 0) {
            CUIPanel* pPanel = FindPanel(pActiveEngine, static_cast<DWORD>(Num(step, 0, 0)));
            int nControls = CountControls(pPanel);
            if (nControls < 0) {
                Fail(nStep, "expect", "panel not found");
                return;
            }
            const int nMin = Num(step, 1, 1);
            if (nControls < nMin) {
                Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"controls\",\"panel\":%d,"
                     "\"min\":%d,\"got\":%d,\"status\":\"fail\"}",
                    nStep, Num(step, 0, 0), nMin, nControls);
                s_failed++;
                s_pc = s_count;
                return;
            }
            Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"controls\",\"panel\":%d,"
                 "\"min\":%d,\"got\":%d,\"status\":\"ok\"}",
                nStep, Num(step, 0, 0), nMin, nControls);
            s_pc++;
            return;
        }

        if (strcmp(step.s[0], "control") == 0) {
            CUIPanel* pPanel = FindPanel(pActiveEngine, static_cast<DWORD>(Num(step, 0, 0)));
            if (pPanel == NULL) {
                Fail(nStep, "expect", "panel not found");
                return;
            }
            CUIControlBase* pControl = pPanel->GetControl(static_cast<DWORD>(Num(step, 1, 0)));
            if (pControl == NULL) {
                Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"control\",\"panel\":%d,"
                     "\"control\":%d,\"status\":\"fail\",\"detail\":\"missing\"}",
                    nStep, Num(step, 0, 0), Num(step, 1, 0));
                s_failed++;
                s_pc = s_count;
                return;
            }
            Emit("{\"step\":%d,\"op\":\"expect\",\"kind\":\"control\",\"panel\":%d,"
                 "\"control\":%d,\"active\":%d,\"status\":\"ok\"}",
                nStep, Num(step, 0, 0), Num(step, 1, 0),
                pControl->m_bActive != 0 ? 1 : 0);
            s_pc++;
            return;
        }

        Fail(nStep, "expect", "unknown assertion");
        return;
    }

    Fail(nStep, step.op, "unknown op");
}

}
