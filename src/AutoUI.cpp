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
//   sleep <ticks>
//   resume                      unpause the world if paused
//   enddialog                   end the current dialogue for isolated engine tests
//   forcepoint <resref>          queue ForceSpellPoint at the first party
//                                caster's own position (requires available spell).
//                                Engine-action test, not a casting UI click.
//   weaponpayload <item> [launcher]  ZRC fixtures only; verify weapon/launcher
//                                effect provenance, fire at portrait 1, check HP.
//                                Bypasses Swing and attack/base-damage rolls.
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

    if (strcmp(step.op, "weaponpayload") == 0) {
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
            Emit("{\"step\":%d,\"op\":\"weaponpayload\",\"item\":\"%s\",\"hpBefore\":%d,\"hpAfter\":%d,\"expectedDamage\":%d}",
                nStep, step.s[0], s_payloadHP, hp, s_payloadDamage);
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
            && hp > (step.s[1][0] != '\0' ? 4 : 2);
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

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
        if (valid) {
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
        s_payloadDamage = hasLauncher ? 3 : 1;
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
