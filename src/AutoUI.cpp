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
//   depletionreference          compare exhausted ammunition transitions, including
//                                resource changes and the non-PC melee fallback.
//   combatparty                 verify independent party-effect copies and arena
//                                recipients; discard effects and restore state.
//   weapondamage <item> [launcher]  calculate base damage and deliver by projectile;
//                                ZRC fixtures only, no saving, check actual HP loss.
//   swingdamage <item> [launcher] run Swing through one real combat bitmap round;
//                                check ammunition and actual HP loss, no saving.
//   combatmove                  check seen/follow movement guards; restore state.
//   combatgroup [mode]          1: dispatch, 2: also UpdateTarget; restore state.
//   groupdamage                 run GroupAttack on normal ticks until HP loss;
//                                restore temporary enemy classification, stop order.
//   rangedready <0|1|2> [mode]  temporarily equip bow/crossbow/sling test fixtures;
//                                mode 0 = 40 ammo, 1 = last round, 2 = last + spare.
//                                modes 3/4 = last round + melee, transient non-PC/PC.
//   rangedrestore               verify ammo spent and restore the saved equipment.
//   depletewait                 verify first pile deleted and fallback selected.
//   attackplace <worldx> <worldy> prepare party member 0 using JumpToPoint.
//   attackmark [requireMove]    remember target HP and the attacker's position.
//   attackwait                  require a normal attack order and HP loss, plus
//                                movement if marked; stop the attack on completion.
//   projectilechecks            verify opcode 430 payloads, source failures and
//                                concrete missile types; restore resource/queue.
//   projectiledamage <flags>     decode/apply opcode 430 with ZRCHIT on portrait 1.
//   projectilewait              require three target damage and no source damage
//                                from that projectile on normal engine ticks.
//   targetgatechecks            check recovered spell target/invisibility gates;
//                                restore both actors in the same tick.
//   spellusechecks              check spell usefulness, visibility and scripted
//                                selection/trigger dispatch; restore all state.
//
// Results are one JSON object per line to IWD2_RE_UI_RESULT, last line the
// verdict. A failed assertion stops the script.

#include "AutoUI.h"

#include <cmath>

#include "mfc.h"

#include "BalDataTypes.h"
#include "CAIAction.h"
#include "CAICondition.h"
#include "CAIResponse.h"
#include "CAIScript.h"
#include "CAITrigger.h"
#include "CAIIdList.h"
#include "CRuleTables.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CGameArea.h"
#include "CGameObject.h"
#include "CGameObjectArray.h"
#include "CGameRemoteObjectArray.h"
#include "CGameOptions.h"
#include "CGameSprite.h"
#include "CGameContainer.h"
#include "CGameEffect.h"
#include "CItem.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CVariableHash.h"
#include "CProjectile.h"
#include "CSpell.h"
#include "IcewindCGameEffects.h"
#include "CDimmKeyTable.h"
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
LONG s_attackTarget = -1;
SHORT s_attackHP = 0;
BOOL s_attackOrderSeen = FALSE;
CPoint s_attackStart;
BOOL s_attackRequireMove = FALSE;
BOOL s_attackMoved = FALSE;
SHORT s_onHitSourceHP = 0;
SHORT s_onHitTargetHP = 0;
SHORT s_projectileSourceHP = 0;
SHORT s_projectileTargetHP = 0;
INT s_projectileFlags = 0;

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
            {
                auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
                INT before = queue.GetCount(), payloads = 0;
                source->m_baseStats.m_feats[0] |= 1 << CGAMESPRITE_FEAT_DIRTY_FIGHTING;
                source->m_derivedStats.m_nDEX = 13;
                check("dirty-fighting-prerequisites", source->HasFeat(CGAMESPRITE_FEAT_DIRTY_FIGHTING), TRUE);
                attack("dirty-fighting-critical", 9063, 1, 1);
                source->m_derivedStats.m_nDEX = 10;
                source->m_baseStats.m_feats[0] = 0;
                while (queue.GetCount() > before) {
                    CMessage* raw = queue.RemoveTail();
                    CMessageAddEffect* message = dynamic_cast<CMessageAddEffect*>(raw);
                    if (message != NULL) {
                        check("dirty-fighting-recipient", message->GetTarget(), targetId);
                        check("dirty-fighting-caller", message->GetSource(), sourceId);
                        ++payloads;
                    }
                    delete raw;
                }
                check("dirty-fighting-payload-count", payloads, 1);
            }
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
            {
                auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
                INT before = queue.GetCount(), payloads = 0;
                std::vector<int*> icons;
                POSITION iconPos = source->m_portraitIcons.GetHeadPosition();
                while (iconPos != NULL) icons.push_back(source->m_portraitIcons.GetNext(iconPos));
                source->m_derivedStats.m_spellStates[SPLSTATE_COAT_WEAPON_WITH_POISON] = true;
                damage("poison-weapon-base-damage", 8);
                while (queue.GetCount() > before) {
                    CMessage* raw = queue.RemoveTail();
                    CMessageAddEffect* message = dynamic_cast<CMessageAddEffect*>(raw);
                    if (message != NULL && message->m_effect->m_effectID == CGAMEEFFECT_POISON) {
                        check("poison-recipient", message->GetTarget(), targetId);
                        check("poison-caller", message->GetSource(), sourceId);
                        ++payloads;
                    }
                    delete raw;
                }
                check("poison-payload-count", payloads, 1);
                source->m_portraitIcons.RemoveAll();
                for (int* icon : icons) source->m_portraitIcons.AddTail(icon);
            }
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

    if (strcmp(step.op, "combatparty") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto& network = g_pChitin->cNetwork;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || game->GetNumCharacters() < 2
            || (network.GetServiceProvider() != CNetwork::SERV_PROV_NULL && network.GetSessionOpen())) {
            Fail(nStep, step.op, "requires offline world and two party members");
            return;
        }
        auto* objects = game->GetObjectArray();
        LONG sourceId = game->GetCharacterId(0);
        CGameObject* sourceObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH,
                &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(sourceObject);
        auto arena = game->m_gameSave.m_bArenaMode;
        auto provider = network.m_nServiceProvider;
        auto player = network.m_idLocalPlayer;
        auto connected = network.m_bConnectionEstablished;
        SHORT partyCount = game->m_nCharacters;
        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"combatparty-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        ITEM_EFFECT itemEffect = {};
        CGameEffect::ClearItemEffect(&itemEffect, CGAMEEFFECT_DAMAGE);
        itemEffect.effectAmount = 7;
        CGameEffect* effect = CGameEffect::DecodeEffect(&itemEffect,
            CPoint(123, 456), sourceId, CPoint(789, 321));
        effect->m_sourceRes = "ZRCORIG1";
        // A local save can mark its null-provider session open. Keep the
        // synchronous ownership checks from invoking the network transport.
        network.m_bConnectionEstablished = FALSE;
        for (INT test = 0; test < 5; ++test) {
            game->m_gameSave.m_bArenaMode = test != 0;
            BOOL filter = test == 2 || test == 4;
            network.m_nServiceProvider = filter ? 1 : CNetwork::SERV_PROV_NULL;
            network.m_idLocalPlayer = source->m_remotePlayerID ^ (test == 4 ? 1 : 0);
            game->m_nCharacters = test == 3 ? 0 : partyCount;
            std::vector<LONG> recipients;
            for (SHORT portrait = 0; portrait < game->GetNumCharacters(); ++portrait) {
                LONG id = game->GetCharacterId(portrait);
                CGameObject* object = NULL;
                if (!filter) {
                    recipients.push_back(id);
                } else if (objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH,
                               &object, INFINITE) == CGameObjectArray::SUCCESS) {
                    if (object->m_remotePlayerID == network.m_idLocalPlayer) {
                        recipients.push_back(id);
                    }
                    objects->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }
            }
            if (test == 4) {
                check("foreign-owner-excluded", recipients.size() < static_cast<size_t>(partyCount));
            }
            INT before = queue.GetCount();
            source->ApplyEffectToParty(effect);
            check("recipient-count", queue.GetCount() == before + static_cast<INT>(recipients.size()));
            std::vector<CGameEffect*> copies;
            INT index = static_cast<INT>(recipients.size()) - 1;
            while (queue.GetCount() > before) {
                CMessage* raw = queue.RemoveTail();
                auto* message = dynamic_cast<CMessageAddEffect*>(raw);
                check("message-type", message != NULL);
                if (message != NULL) {
                    CGameEffect* copy = message->m_effect;
                    check("recipient-order", index >= 0 && message->GetTarget() == recipients[index]);
                    check("caller", message->GetSource() == sourceId);
                    BOOL unique = copy != NULL && copy != effect;
                    for (auto* previous : copies) {
                        unique = unique && copy != previous;
                    }
                    check("independent-copy", unique);
                    if (copy != NULL) {
                        check("payload", copy->m_effectAmount == 7 && copy->m_effectID == CGAMEEFFECT_DAMAGE);
                        check("provenance", copy->m_source == CPoint(123, 456)
                            && copy->m_target == CPoint(789, 321) && copy->m_sourceID == sourceId
                            && copy->m_sourceRes == CResRef("ZRCORIG1"));
                        if (unique) {
                            copy->m_effectAmount = 99;
                            copies.push_back(copy);
                        }
                    }
                    // Retain all copies until the identity checks finish; even a
                    // regression sharing pointers must fail without double deletion.
                    message->m_effect = NULL;
                }
                delete raw;
                --index;
            }
            for (auto* copy : copies) {
                delete copy;
            }
            check("input-retained", effect->m_effectAmount == 7);
        }
        delete effect;
        game->m_nCharacters = partyCount;
        game->m_gameSave.m_bArenaMode = arena;
        network.m_nServiceProvider = provider;
        network.m_idLocalPlayer = player;
        network.m_bConnectionEstablished = connected;
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, step.op, "party effect ownership/recipient mismatch (state restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"combatparty\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatteams") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        CGameObject* object = NULL;
        LONG id = game->GetFixedOrderCharacterId(0);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || game->GetObjectArray()->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, "combatteams", "party source unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(object);
        CGameArea* area = source->m_pArea;
        CVariableHash globals, areaVars, locals;
        auto swapHash = [](CVariableHash& left, CVariableHash& right) {
            std::swap(left.m_hashEntries, right.m_hashEntries);
            std::swap(left.m_nTableEntries, right.m_nTableEntries);
        };
        swapHash(globals, *game->GetVariables());
        swapHash(areaVars, *area->GetVariables());
        CVariableHash* savedLocals = source->m_pLocalVariables;
        source->m_pLocalVariables = &locals;
        INT savedTeams = source->field_58C;
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"combatteams-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        auto value = [](CVariableHash* vars, const char* name) {
            CVariable* var = vars->FindKey(CString(name));
            return var != NULL ? var->m_intValue : LONG_MIN;
        };
        auto run = [&](const char* scope, LONG amount, BYTE increment, LONG target) {
            CString areaName(scope), name;
            name = "ZRC_TEST";
            CMessageSetVariable message(areaName, name, amount, increment, id, target);
            message.Run();
            return message.m_sAreaName;
        };
        run("GLOBAL", 7, 0, id);
        check("global-create", value(game->GetVariables(), "ZRC_TEST") == 7);
        run("GLOBAL", -2, 1, id);
        check("global-increment", value(game->GetVariables(), "ZRC_TEST") == 5);
        run("GLOBAL", 9, 2, id);
        check("increment-exactly-one", value(game->GetVariables(), "ZRC_TEST") == 9);
        run("GLOBAL", 11, 0, -1);
        check("global-without-target", value(game->GetVariables(), "ZRC_TEST") == 11);
        run("LOCALS", 3, 1, id);
        check("locals-create", value(&locals, "ZRC_TEST") == 3);
        run("LOCALS", 4, 1, id);
        check("locals-increment", value(&locals, "ZRC_TEST") == 7);
        CString actualArea(reinterpret_cast<const char*>(area->m_header.m_areaName));
        check("myarea-resolves", run("MYAREA", 13, 0, id) == actualArea);
        check("area-create", value(area->GetVariables(), "ZRC_TEST") == 13);
        run(actualArea, -5, 1, id);
        check("area-increment", value(area->GetVariables(), "ZRC_TEST") == 8);
        run("global", 99, 0, id);
        check("scope-case-sensitive", value(game->GetVariables(), "ZRC_TEST") == 11);

        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = queue.GetCount();
        for (DWORD mask : { 0u, 1u, 0x200000u, 0x80200000u }) {
            source->field_58C = mask;
            source->SetTeamEnemies();
            check("inactive-or-no-team", queue.GetCount() == before);
        }
        CVariable existing;
        CString teamName;
        teamName = "TEAM_0_ENEMY";
        existing.SetName(teamName);
        existing.m_intValue = 9;
        game->GetVariables()->AddKey(existing);
        source->field_58C = 0x80300001;
        source->SetTeamEnemies();
        check("two-team-messages", queue.GetCount() == before + 2);
        check("team-existing", value(game->GetVariables(), "TEAM_0_ENEMY") == 1);
        check("team-created", value(game->GetVariables(), "TEAM_20_ENEMY") == 1);
        check("team-high-bit-ignored", value(game->GetVariables(), "TEAM_31_ENEMY") == LONG_MIN);
        check("team-mask-preserved", static_cast<DWORD>(source->field_58C) == 0x80300001);
        INT index = 0;
        while (queue.GetCount() > before) {
            CMessage* raw = queue.RemoveTail();
            CMessageSetVariable* message = dynamic_cast<CMessageSetVariable*>(raw);
            check("team-message-type", message != NULL);
            if (message != NULL) {
                check("team-message-fields", message->m_sAreaName == actualArea
                    && message->m_sGlobalName == (index == 0 ? "TEAM_20_ENEMY" : "TEAM_0_ENEMY")
                    && message->m_nValue == 1 && message->m_bIncrement == 0 && message->m_flags == 0
                    && message->GetSource() == id && message->GetTarget() == id);
                message->Run();
            }
            delete raw;
            ++index;
        }
        check("team-area-zero", value(area->GetVariables(), "TEAM_0_ENEMY") == 1);
        check("team-area-twenty", value(area->GetVariables(), "TEAM_20_ENEMY") == 1);

        CString globalScope, testName;
        globalScope = "GLOBAL";
        testName = "ZRC_WIRE";
        CMessageSetVariable sent(globalScope, testName, -17, 1, id, id);
        sent.m_flags = 0x80000004;
        BYTE bytes[256] = {};
        BYTE* payload = bytes + CNetwork::SPEC_MSG_HEADER_LENGTH;
        DWORD size = 0;
        sent.MarshalMessage(&payload, &size);
        check("wire-size", size == 21 + 8 + 6);
        // Force an unmapped target to exercise the invalid-id fallback.
        *reinterpret_cast<DWORD*>(payload) = 0xDEADBEEF;
        *reinterpret_cast<DWORD*>(payload + 4) = 0x7FFFFFFF;
        CMessageSetVariable received(CString(), CString(), 0, 0, id, id);
        check("wire-decode", received.UnmarshalMessage(bytes, size + CNetwork::SPEC_MSG_HEADER_LENGTH));
        check("wire-fields", received.m_sAreaName == globalScope && received.m_sGlobalName == testName
            && received.m_nValue == -17 && received.m_bIncrement == 1 && received.m_flags == 0x80000004);
        check("wire-unmapped-target", received.GetTarget() == CGameObjectArray::INVALID_INDEX);
        received.MarshalMessage(&payload, &size);
        check("wire-invalid-target-empty", size == 0);

        source->field_58C = savedTeams;
        source->m_pLocalVariables = savedLocals;
        swapHash(areaVars, *area->GetVariables());
        swapHash(globals, *game->GetVariables());
        game->GetObjectArray()->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, "combatteams", "variable/message mismatch (original hashes restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"combatteams\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "spellusechecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        auto* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable"); return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable"); return;
        }
        auto* source = static_cast<CGameSprite*>(sourceObject);
        auto* target = static_cast<CGameSprite*>(targetObject);
        CSpell guard(CResRef("SPWI112"));
        if (!guard.Demand() || !guard.pRes->m_bParsed || source->m_pArea == NULL) {
            objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "spell or source area unavailable"); return;
        }
        auto* resource = guard.pRes;
        auto* oldHeader = resource->m_pHeader;
        auto* oldAbilities = resource->m_pAbilities;
        auto* oldEffects = resource->m_pEffects;
        SPELL_HEADER header = *oldHeader;
        SPELL_ABILITY abilities[2] = {};
        ITEM_EFFECT effects[4] = {};
        header.itemType = 1; header.spellLevel = 2; header.abilityCount = 1;
        header.itemFlags = 0; header.school = 0;
        abilities[0].minCasterLevel = 0; abilities[0].effectCount = 1;
        abilities[1].minCasterLevel = 2; abilities[1].effectCount = 2; abilities[1].startingEffect = 2;
        for (auto& effect : effects) {
            effect.effectID = CGAMEEFFECT_DAMAGE; effect.probabilityUpper = 100;
        }
        resource->m_pHeader = &header; resource->m_pAbilities = abilities; resource->m_pEffects = effects;
        auto sourceBase = source->m_baseStats, targetBase = target->m_baseStats;
        auto sourceState = source->m_derivedStats.m_generalState, targetState = target->m_derivedStats.m_generalState;
        auto sourceStates = source->m_derivedStats.m_spellStates, targetStates = target->m_derivedStats.m_spellStates;
        auto sourceMask = source->m_derivedStats.m_classMask;
        BYTE sourceLevels[CAIOBJECT_CLASS_MAX];
        memcpy(sourceLevels, source->m_derivedStats.m_nClassLevels, sizeof(sourceLevels));
        SHORT arcaneFailure = source->m_derivedStats.m_nSpellFailureArcane;
        SHORT divineFailure = source->m_derivedStats.m_nSpellFailureDivine;
        SHORT mentalSpeed = source->m_derivedStats.m_nMentalSpeed;
        SHORT oldActionCount = source->m_actionCount;
        BOOL disabled[3], immuneLevels[10];
        memcpy(disabled, source->m_derivedStats.m_disabledSpellTypes, sizeof(disabled));
        memcpy(immuneLevels, target->m_derivedStats.m_cImmunitiesSpellLevel.m_levels, sizeof(immuneLevels));
        auto sourceImmunities = source->m_derivedStats.m_cImmunitiesSpell;
        auto targetImmunities = target->m_derivedStats.m_cImmunitiesSpell;
        auto targetOpcodes = target->m_derivedStats.m_activeEffectOpcodes;
        auto sourceSpells = source->m_spells;
        auto sourceDomain = source->m_domainSpells;
        CAIAction oldAction;
        oldAction = source->m_curAction;
        auto oldSelection = source->field_3BA;
        INT oldSpell = source->field_588;
        SHORT oldReturn = source->m_nLastActionReturn;
        LONG oldTargetId = source->m_targetId;
        BOOL oldSelected = source->m_bSelected;
        source->m_targetId = CGameObjectArray::INVALID_INDEX;
        source->m_bSelected = FALSE;
        CPoint oldPos = target->m_pos;
        CGameArea* oldArea = target->m_pArea;
        WORD oldAreaType = source->m_pArea->m_header.m_areaType;
        CSpellResRefList* lists[] = {&game->m_innateSpells, &game->m_songs, &game->m_shapeshifts};
        CResRef* oldListData[3];
        UINT oldListCounts[3];
        for (INT index = 0; index < 3; ++index) {
            oldListData[index] = lists[index]->m_pList; oldListCounts[index] = lists[index]->m_nCount;
        }
        INT demands = resource->m_nServiceCount, requests = resource->m_nLockCount;
        INT refs = resource->m_pDimmKeyTableEntry->field_10;
        INT queued = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"spelluse-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
            valid = valid && passed; ++checks;
        };
        auto reset = [&]() {
            source->m_derivedStats.m_generalState = target->m_derivedStats.m_generalState = STATE_NORMAL;
            source->m_derivedStats.m_spellStates.reset(); target->m_derivedStats.m_spellStates.reset();
            source->m_derivedStats.m_nSpellFailureArcane = source->m_derivedStats.m_nSpellFailureDivine = 0;
            memset(source->m_derivedStats.m_disabledSpellTypes, 0, sizeof(disabled));
            memset(target->m_derivedStats.m_cImmunitiesSpellLevel.m_levels, 0, sizeof(immuneLevels));
            source->m_derivedStats.m_cImmunitiesSpell.clear(); target->m_derivedStats.m_cImmunitiesSpell.clear();
            memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
            memset(source->m_derivedStats.m_nClassLevels, 3, sizeof(sourceLevels));
            source->m_derivedStats.m_classMask = CLASSMASK_BARD | CLASSMASK_WIZARD;
            source->m_baseStats.m_flags = target->m_baseStats.m_flags = 0;
            target->m_baseStats.m_visible = 0; target->m_baseStats.m_bStealthMode = FALSE;
            header.itemType = 1; header.spellLevel = 2; header.abilityCount = 1; header.itemFlags = 0;
            abilities[0].minCasterLevel = 0; abilities[0].effectCount = 1;
            effects[0].effectID = CGAMEEFFECT_DAMAGE; effects[0].dwFlags = 0;
            effects[1].effectID = CGAMEEFFECT_DAMAGE; effects[1].dwFlags = 0;
            target->m_pArea = oldArea; target->m_pos = source->m_pos;
            source->m_pArea->m_header.m_areaType = oldAreaType;
            for (INT index = 0; index < 3; ++index) lists[index]->m_nCount = 0;
        };
        auto useful = [&](DWORD flags = 0) { return source->CheckSpellUseful(target, 2112, flags); };
        reset();
        check("ordinary-damage-useful", useful());
        check("null-target-diagnostic", !source->CheckSpellUseful(NULL, 2112, 0xFFFFFFFF));
        check("zero-spell-diagnostic", !source->CheckSpellUseful(target, 0, 0x80000000));
        for (DWORD flags : {1U, 2U, 0xFFFFFFFFU}) check("query-flags-do-not-bypass", useful(flags));
        source->m_derivedStats.m_generalState = STATE_SILENCED;
        check("silenced-source", !useful());
        source->m_derivedStats.m_spellStates.set(SPLSTATE_VOCALIZE);
        check("vocalize-state-bypasses-silence", useful());
        source->m_derivedStats.m_spellStates.reset();
        source->m_baseStats.m_feats[2] = 1U << (CGAMESPRITE_FEAT_SUBVOCAL_CASTING - 64);
        check("subvocal-feat-bypasses-silence", useful());
        reset();
        {
            CSpell vocalize(CResRef("SPWI219"));
            BOOL ready = vocalize.Demand() && vocalize.pRes->m_bParsed;
            check("silence-exemption-resource-available", ready);
            if (ready) {
                auto* vocalizeHeader = vocalize.pRes->m_pHeader;
                auto* vocalizeAbilities = vocalize.pRes->m_pAbilities;
                auto* vocalizeEffects = vocalize.pRes->m_pEffects;
                vocalize.pRes->m_pHeader = &header;
                vocalize.pRes->m_pAbilities = abilities;
                vocalize.pRes->m_pEffects = effects;
                source->m_derivedStats.m_generalState = STATE_SILENCED;
                check("numeric-2219-bypasses-silence", source->CheckSpellUseful(target, 2219, 0));
                vocalize.pRes->m_pHeader = vocalizeHeader;
                vocalize.pRes->m_pAbilities = vocalizeAbilities;
                vocalize.pRes->m_pEffects = vocalizeEffects;
                vocalize.Release();
            }
        }
        reset();
        target->m_derivedStats.m_generalState = STATE_SILENCED;
        check("silenced-target-not-caster", useful());
        reset();
        header.itemFlags = 0x2000; source->m_pArea->m_header.m_areaType &= ~1;
        check("outdoor-spell-rejected-indoors", !useful());
        source->m_pArea->m_header.m_areaType |= 1;
        check("outdoor-spell-allowed-outdoors", useful());
        reset();
        for (SHORT caster : {1, 2}) {
            header.itemType = caster;
            source->m_derivedStats.m_disabledSpellTypes[caster] = 1;
            check("disabled-spell-type-exact-true", !useful());
            source->m_derivedStats.m_disabledSpellTypes[caster] = 2;
            check("disabled-spell-type-two-not-true", useful());
            source->m_derivedStats.m_disabledSpellTypes[caster] = 0;
            SHORT& failure = caster == 1 ? source->m_derivedStats.m_nSpellFailureArcane : source->m_derivedStats.m_nSpellFailureDivine;
            for (SHORT value : {-1, 99, 100, 101}) {
                failure = value;
                check("failure-threshold", useful() == (value < 100));
            }
            failure = 0;
        }
        reset();
        TString key; key = "SPWI112";
        source->m_derivedStats.m_cImmunitiesSpell.insert(key);
        check("caster-resource-immunity-ignored", useful());
        target->m_derivedStats.m_cImmunitiesSpell.insert(key);
        check("target-resource-immunity", !useful());
        reset();
        for (DWORD level : {0U, 1U, 2U, 8U, 9U, 258U}) {
            header.spellLevel = level;
            memset(target->m_derivedStats.m_cImmunitiesSpellLevel.m_levels, 1, sizeof(immuneLevels));
            check("byte-level-immunity-boundary", useful() == (static_cast<BYTE>(level) >= 9));
        }
        header.spellLevel = 1;
        CResRef exception("SPWI112");
        for (INT list = 0; list < 3; ++list) {
            lists[list]->m_pList = &exception; lists[list]->m_nCount = 1;
            check("level-one-catalog-exemption", useful());
            lists[list]->m_nCount = 0;
        }
        reset();
        header.abilityCount = 0;
        check("no-abilities", !useful());
        header.abilityCount = 1; abilities[0].minCasterLevel = 4;
        check("caster-level-too-low", !useful());
        abilities[0].minCasterLevel = 3;
        check("caster-level-exact-threshold", useful());
        abilities[0].minCasterLevel = 0; abilities[0].effectCount = 0;
        check("eligible-empty-ability", useful());
        abilities[0].effectCount = 1;
        header.abilityCount = 2;
        effects[0].effectID = CGAMEEFFECT_BERSERK;
        effects[2].effectID = CGAMEEFFECT_DAMAGE;
        target->m_derivedStats.m_generalState = STATE_BERSERK;
        check("selected-count-uses-ability-zero-payload", !useful());
        target->m_derivedStats.m_generalState = 0;
        check("missing-second-ability-zero-effect-skipped", useful());
        reset();
        struct StateCase { WORD opcode; DWORD state; };
        const StateCase generalCases[] = {
            {CGAMEEFFECT_BERSERK, STATE_BERSERK},
            {CGAMEEFFECT_CHARM, STATE_CHARMED},
            {CGAMEEFFECT_HASTE, STATE_HASTED},
            {CGAMEEFFECT_PANIC, STATE_PANIC},
            {CGAMEEFFECT_POISON, STATE_POISONED},
            {CGAMEEFFECT_SILENCE, STATE_SILENCED},
            {CGAMEEFFECT_SLEEP, STATE_SLEEPING},
            {CGAMEEFFECT_SLOW, STATE_SLOWED},
            {CGAMEEFFECT_STUN, STATE_STUNNED},
            {CGAMEEFFECT_INFRAVISION, STATE_INFRAVISION},
            {CGAMEEFFECT_BLUR, STATE_BLUR},
            {CGAMEEFFECT_NONDETECTION, STATE_NONDETECTION},
            {CGAMEEFFECT_BLINDNESS, STATE_BLIND},
            {CGAMEEFFECT_DISEASE, STATE_DISEASED},
            {CGAMEEFFECT_MIRRORIMAGE, STATE_MIRRORIMAGE},
            {CGAMEEFFECT_CONFUSION, STATE_CONFUSED},
            {CGAMEEFFECT_NON_CUMULATIVEAID, STATE_AID},
            {CGAMEEFFECT_NON_CUMULATIVEBLESS, STATE_BLESS},
            {CGAMEEFFECT_NON_CUMULATIVECHANT, STATE_CHANT},
            {ICEWIND_CGAMEEFFECT_UNCONSCIOUSNESS, STATE_SLEEPING},
        };
        for (const auto& entry : generalCases) {
            reset(); effects[0].effectID = entry.opcode;
            check("general-effect-absent", useful());
            source->m_derivedStats.m_generalState = entry.state;
            // Silence is a caster restriction even when testing its target effect.
            if (entry.state == STATE_SILENCED) source->m_derivedStats.m_spellStates.set(SPLSTATE_VOCALIZE);
            check("general-effect-caster-state-ignored", useful());
            target->m_derivedStats.m_generalState = entry.state;
            check("general-effect-already-active", !useful());
        }
        struct SpellCase { WORD opcode; UINT state; };
        const SpellCase spellCases[] = {
            {CGAMEEFFECT_DEAFNESS, SPLSTATE_DEAFENED},
            {CGAMEEFFECT_NON_CUMULATIVEDRAWUPONHOLYMIGHT, SPLSTATE_DRAW_UPON_HOLY_MIGHT},
            {CGAMEEFFECT_SANCTUARY, SPLSTATE_SANCTUARY},
            {CGAMEEFFECT_ENTANGLE, SPLSTATE_ENTANGLE},
            {CGAMEEFFECT_WEBHOLD, SPLSTATE_WEB},
            {CGAMEEFFECT_GREASEPOOL, SPLSTATE_GREASE},
            {CGAMEEFFECT_STONESKINS, SPLSTATE_STONESKIN},
            {ICEWIND_CGAMEEFFECT_PRAYER, SPLSTATE_BENEFICIAL_PRAYER},
            {ICEWIND_CGAMEEFFECT_RECITATION, SPLSTATE_BENEFICIAL_RECITATION},
            {ICEWIND_CGAMEEFFECT_CLOAKOFFEAR, SPLSTATE_CLOAK_OF_FEAR},
            {ICEWIND_CGAMEEFFECT_SHROUDOFFLAME, SPLSTATE_SHROUD_OF_FLAME},
            {ICEWIND_CGAMEEFFECT_HOPELESSNESS, SPLSTATE_HOPELESSNESS},
            {ICEWIND_CGAMEEFFECT_PROTECTIONFROMEVIL, SPLSTATE_PROTECTION_FROM_EVIL},
            {ICEWIND_CGAMEEFFECT_ARMOROFFAITH, SPLSTATE_ARMOR_OF_FAITH},
            {ICEWIND_CGAMEEFFECT_NAUSEA, SPLSTATE_NAUSEA},
            {ICEWIND_CGAMEEFFECT_DEATHWARD, SPLSTATE_DEATH_WARD},
            {ICEWIND_CGAMEEFFECT_HOLYPOWER, SPLSTATE_HOLY_POWER},
            {ICEWIND_CGAMEEFFECT_CHARM, SPLSTATE_CONTROLLED},
            {ICEWIND_CGAMEEFFECT_OTILUKESRESILIENTSPHERE, SPLSTATE_OTILUKES_RESILIENT_SPHERE},
            {ICEWIND_CGAMEEFFECT_BARKSKIN, SPLSTATE_BARKSKIN},
            {ICEWIND_CGAMEEFFECT_FREEACTION, SPLSTATE_FREE_ACTION},
            {ICEWIND_CGAMEEFFECT_ENTROPYSHIELD, SPLSTATE_ENTROPY_SHIELD},
            {ICEWIND_CGAMEEFFECT_STORMSHELL, SPLSTATE_STORM_SHELL},
            {ICEWIND_CGAMEEFFECT_PROTECTIONFROMELEMENTS, SPLSTATE_PROTECTION_FROM_THE_ELEMENTS},
            {CGAMEEFFECT_HOLDCREATURE, SPLSTATE_HELD},
            {ICEWIND_CGAMEEFFECT_AEGIS, SPLSTATE_AEGIS},
            {ICEWIND_CGAMEEFFECT_EXECUTIONERSEYES, SPLSTATE_EXECUTIONERS_EYES},
            {ICEWIND_CGAMEEFFECT_TORTOISESHELL, SPLSTATE_TORTOISE_SHELL},
            {ICEWIND_CGAMEEFFECT_BLINK, SPLSTATE_BLINK},
            {ICEWIND_CGAMEEFFECT_PROTECTIONFORMARROWS, SPLSTATE_PROTECTION_FROM_NORMAL_MISSILES},
            {ICEWIND_CGAMEEFFECT_TENSERSTRANSFORMATION, SPLSTATE_TENSERS_TRANSFORMATION},
            {ICEWIND_CGAMEEFFECT_GLOBEOFINVULNERABILITY, SPLSTATE_GLOBE_OF_INVULNERABILITY},
            {ICEWIND_CGAMEEFFECT_LOWERRESISTANCE, SPLSTATE_LOWER_RESISTANCE},
            {ICEWIND_CGAMEEFFECT_BANE, SPLSTATE_BANE},
            {ICEWIND_CGAMEEFFECT_CONTROLUNDEAD, SPLSTATE_CONTROLLED},
            {ICEWIND_CGAMEEFFECT_HOLDUNDEAD, SPLSTATE_HELD},
        };
        for (const auto& entry : spellCases) {
            reset(); effects[0].effectID = entry.opcode;
            check("spell-effect-absent", useful());
            source->m_derivedStats.m_spellStates.set(entry.state);
            check("spell-effect-caster-state-ignored", useful());
            target->m_derivedStats.m_spellStates.set(entry.state);
            check("spell-effect-already-active", !useful());
        }
        reset(); effects[0].effectID = CGAMEEFFECT_INVISIBLE;
        target->m_derivedStats.m_generalState = STATE_INVISIBLE;
        check("invisibility-effect-already-active", !useful());
        target->m_baseStats.m_visible = 1;
        check("force-visible-invisibility-effect-useful", useful());
        for (WORD opcode : {CGAMEEFFECT_FEEBLEMINDEDNESS, ICEWIND_CGAMEEFFECT_ENFEEBLEMENT}) {
            reset(); effects[0].effectID = opcode;
            check("enfeeblement-absent", useful());
            target->m_derivedStats.m_generalState = STATE_FEEBLEMINDED;
            check("enfeeblement-general-state", !useful());
            target->m_derivedStats.m_generalState = 0;
            target->m_derivedStats.m_spellStates.set(SPLSTATE_ENFEEBLED);
            check("enfeeblement-spell-state", !useful());
        }
        for (DWORD flags : {0U, 1U, 0xFFFFFFFFU}) {
            reset(); effects[0].effectID = ICEWIND_CGAMEEFFECT_FIRESHIELD; effects[0].dwFlags = flags;
            UINT selected = flags == 0 ? SPLSTATE_RED_FIRESHIELD : SPLSTATE_BLUE_FIRESHIELD;
            target->m_derivedStats.m_spellStates.set(selected == 5 ? 6 : 5);
            check("opposite-fireshield-useful", useful());
            target->m_derivedStats.m_spellStates.set(selected);
            check("selected-fireshield-active", !useful());
        }
        for (UINT state : {16U, 17U}) {
            reset(); effects[0].effectID = ICEWIND_CGAMEEFFECT_RIGHTEOUSWRATHOFFAITHFUL;
            check("righteous-wrath-absent", useful());
            target->m_derivedStats.m_spellStates.set(state);
            check("either-righteous-wrath-state-blocks", !useful());
        }
        for (DWORD state : {0U, 31U, 32U, 157U, 255U}) {
            reset(); effects[0].effectID = ICEWIND_CGAMEEFFECT_SETSPELLSTATE; effects[0].dwFlags = state;
            check("arbitrary-spell-state-absent", useful());
            target->m_derivedStats.m_spellStates.set(state);
            check("arbitrary-spell-state-active", !useful());
        }
        for (WORD opcode : {CGAMEEFFECT_IMMUNITYSPELL, ICEWIND_CGAMEEFFECT_IMMUNITYSPELL}) {
            reset(); effects[0].effectID = opcode;
            check("zero-immunity-selector-ignored", useful());
            effects[0].dwFlags = 63;
            check("immunity-selector-true", useful());
            effects[0].dwFlags = 66; target->m_derivedStats.m_spellStates.set(SPLSTATE_DEAFENED);
            check("immunity-selector-false", !useful());
        }
        reset(); abilities[0].effectCount = 2;
        effects[0].effectID = CGAMEEFFECT_DAMAGE; effects[1].effectID = CGAMEEFFECT_BERSERK;
        target->m_derivedStats.m_generalState = STATE_BERSERK;
        check("later-blocked-effect-rejects-whole-spell", !useful());
        effects[0].effectID = CGAMEEFFECT_BERSERK; effects[1].effectID = CGAMEEFFECT_DAMAGE;
        check("earlier-blocked-effect-rejects-whole-spell", !useful());
        reset();
        check("visibility-null-target", !source->CheckTargetVisibility(NULL, FALSE));
        check("visibility-near-target", source->CheckTargetVisibility(target, FALSE));
        target->m_pArea = NULL;
        check("visibility-different-area", !source->CheckTargetVisibility(target, TRUE));
        target->m_pArea = oldArea;
        target->m_derivedStats.m_generalState = STATE_DEAD;
        check("visibility-dead-target", !source->CheckTargetVisibility(target, FALSE));
        check("visibility-allow-dead", source->CheckTargetVisibility(target, TRUE));
        target->m_derivedStats.m_generalState = 0; target->m_baseStats.m_bStealthMode = TRUE;
        check("visibility-stealth-mode-blocks", !source->CheckTargetVisibility(target, TRUE));
        target->m_baseStats.m_bStealthMode = FALSE;
        target->m_pos.x += (source->GetVisualRange() + 1) * CPathSearch::GRID_SQUARE_SIZEX;
        check("visibility-search-grid-range", !source->CheckTargetVisibility(target, FALSE));
        reset();
        CAIObjectType selector(CAIObjectType::ANYONE); selector.SetInstance(targetId);
        CAITrigger trigger(CAITRIGGER_ISSPELLTARGETVALID, selector, 2112);
        trigger.m_specific2 = 0xFFFFFFFF;
        check("script-trigger-combines-gates", source->EvaluateStatusTrigger(trigger));
        target->m_derivedStats.m_spellStates.set(SPLSTATE_SANCTUARY);
        check("script-trigger-sanctuary", !source->EvaluateStatusTrigger(trigger));
        target->m_derivedStats.m_spellStates.reset();
        effects[0].effectID = CGAMEEFFECT_BERSERK;
        target->m_derivedStats.m_generalState = STATE_BERSERK;
        check("script-trigger-already-affected", !source->EvaluateStatusTrigger(trigger));
        reset();
        CAIAction action;
        action.m_actionID = 0x141; action.m_acteeID = selector;
        auto select = [&](const char* text, DWORD flags) {
            action.m_string1 = text; action.m_specificID = flags;
            source->m_curAction = action; source->field_588 = 0;
            return source->ExecuteAction();
        };
        check("script-action-selects-useful", select("2112", 9) == CGameAIBase::ACTION_DONE
            && source->field_588 == 2112 && source->field_3BA.Equal(target->GetAIType()));
        effects[0].effectID = CGAMEEFFECT_BERSERK; target->m_derivedStats.m_generalState = STATE_BERSERK;
        check("script-action-skips-ineffective", select("2112", 9) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        target->m_derivedStats.m_spellStates.set(SPLSTATE_SANCTUARY);
        check("script-action-bit-two-bypasses-both-gates", select("2112", 11) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        reset();
        target->m_baseStats.m_bStealthMode = TRUE;
        check("script-action-enforces-visibility", select("2112", 8) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        check("script-action-bit-one-bypasses-visibility", select("2112", 9) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        reset();
        check("script-action-malformed-list", select("211", 9) == CGameAIBase::ACTION_ERROR);
        check("script-action-empty-list", select("", 9) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        check("script-action-zero-entry-skipped", select("00002112", 9) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        check("script-action-random-start-wraps", select("00002112", 13) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        source->m_nLastActionReturn = 123;
        check("script-action-unresolved-entry-preserves-last-return", select("0000", 1) == CGameAIBase::ACTION_ERROR
            && source->field_588 == 0 && source->m_nLastActionReturn == 123);
        source->field_588 = 2112; source->m_curAction.m_string1 = "bad";
        check("script-action-existing-selection-preserved", source->ExecuteAction() == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        action.m_acteeID = CAIObjectType::NOONE;
        check("script-action-no-target-default", select("2112", 9) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        check("script-action-null-target-gate-error", select("2112", 0x29) == CGameAIBase::ACTION_ERROR && source->field_588 == 0);
        check("script-action-no-target-explicit-bypass", select("2112", 0x2B) == CGameAIBase::ACTION_DONE
            && source->field_588 == 2112 && source->field_3BA.Equal(CAIObjectType::NOONE));
        trigger.m_triggerCause = CAIObjectType::NOONE;
        check("script-trigger-no-target", !source->EvaluateStatusTrigger(trigger));
        action.m_acteeID = selector;
        UINT spellId = 0;
        BOOL catalogued = game->m_spells.Find(CResRef("SPWI112"), spellId);
        check("script-action-spell-catalogued", catalogued);
        for (UINT index = 0; index < CSPELLLIST_NUM_CLASSES; ++index) source->m_spells.Get(index)->Clear();
        source->m_domainSpells.Clear();
        check("script-action-unmemorized", select("2112", 1) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        if (catalogued) {
            source->m_spells.Get(game->GetSpellcasterIndex(CAIOBJECTTYPE_C_WIZARD))->Add(spellId, 0, 1, 1, 0);
            check("script-action-memorized", select("2112", 1) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        }
        // Marking is script state: these original triggers succeed even when
        // they cannot select an available spell. None consumes a spell charge.
        auto markedTrigger = [&](SHORT id, LONG spell) {
            CAITrigger query(id, CAIObjectType::NOONE, spell);
            query.m_specific2 = 0xFFFFFFFF;
            return source->EvaluateStatusTrigger(query);
        };
        auto markedAction = [&](SHORT id, LONG spell) {
            CAIAction query;
            query.m_actionID = id; query.m_specificID = spell;
            source->m_curAction = query; source->m_nLastActionReturn = 123;
            return source->ExecuteAction();
        };
        source->field_3BA = selector;
        source->field_588 = 0;
        check("marked-zero-equals-zero", markedTrigger(CAITRIGGER_ISMARKEDSPELL, 0));
        check("marked-zero-has-no-spell", !markedTrigger(CAITRIGGER_HAVESPELL, 0));
        for (LONG number : {0L, 2112L, -123L, 0x12345678L}) {
            check("force-marked-trigger-succeeds", markedTrigger(CAITRIGGER_FORCEMARKEDSPELL, number) && source->field_588 == number);
            check("marked-trigger-full-width-match", markedTrigger(CAITRIGGER_ISMARKEDSPELL, number));
            check("marked-trigger-mismatch", !markedTrigger(CAITRIGGER_ISMARKEDSPELL, number + 1));
            check("force-marked-action-succeeds", markedAction(0x13F, number) == CGameAIBase::ACTION_DONE
                && source->field_588 == number && source->m_nLastActionReturn == CGameAIBase::ACTION_DONE);
            check("numeric-mark-keeps-target", source->field_3BA.Equal(selector));
        }
        for (UINT index = 0; index < CSPELLLIST_NUM_CLASSES; ++index) source->m_spells.Get(index)->Clear();
        source->m_domainSpells.Clear();
        source->field_588 = 0;
        check("have-spell-explicit-unavailable", !markedTrigger(CAITRIGGER_HAVESPELL, 2112));
        check("set-marked-trigger-unavailable-still-true", markedTrigger(CAITRIGGER_SETMARKEDSPELL, 2112) && source->field_588 == 0);
        check("set-marked-action-unavailable-done", markedAction(0x135, 2112) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        source->field_588 = 2112;
        check("have-spell-marked-unavailable", !markedTrigger(CAITRIGGER_HAVESPELL, 0));
        check("set-marked-trigger-retains-selection", markedTrigger(CAITRIGGER_SETMARKEDSPELL, 2220) && source->field_588 == 2112);
        check("set-marked-action-retains-selection", markedAction(0x135, 2220) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
        check("set-marked-trigger-clears", markedTrigger(CAITRIGGER_SETMARKEDSPELL, 0) && source->field_588 == 0);
        source->field_588 = 2112;
        check("set-marked-action-clears", markedAction(0x135, 0) == CGameAIBase::ACTION_DONE && source->field_588 == 0);
        if (catalogued) {
            auto* wizard = source->m_spells.Get(game->GetSpellcasterIndex(CAIOBJECTTYPE_C_WIZARD));
            wizard->Add(spellId, 0, 1, 0, 0);
            check("have-spell-known-but-exhausted", !markedTrigger(CAITRIGGER_HAVESPELL, 2112));
            source->field_588 = 0;
            check("set-marked-exhausted-still-true", markedTrigger(CAITRIGGER_SETMARKEDSPELL, 2112) && source->field_588 == 0);
            for (UINT level = 0; level < CSPELLLIST_MAX_LEVELS; ++level) {
                wizard->Clear(); wizard->Add(spellId, level, 1, 1, 0);
                source->field_588 = 0;
                check("have-spell-all-nine-levels", markedTrigger(CAITRIGGER_HAVESPELL, 2112));
                check("set-marked-all-nine-levels", markedTrigger(CAITRIGGER_SETMARKEDSPELL, 2112) && source->field_588 == 2112);
            }
            for (BYTE nClass : {CAIOBJECTTYPE_C_BARD, CAIOBJECTTYPE_C_CLERIC, CAIOBJECTTYPE_C_DRUID,
                     CAIOBJECTTYPE_C_PALADIN, CAIOBJECTTYPE_C_RANGER, CAIOBJECTTYPE_C_SORCERER, CAIOBJECTTYPE_C_WIZARD}) {
                for (UINT index = 0; index < CSPELLLIST_NUM_CLASSES; ++index) source->m_spells.Get(index)->Clear();
                source->m_spells.Get(game->GetSpellcasterIndex(nClass))->Add(spellId, 0, 1, 1, 0);
                source->field_588 = 0;
                check("have-spell-any-casting-class", markedTrigger(CAITRIGGER_HAVESPELL, 2112));
                check("set-marked-trigger-selects-available", markedTrigger(CAITRIGGER_SETMARKEDSPELL, 2112) && source->field_588 == 2112);
                check("have-spell-zero-uses-marker", markedTrigger(CAITRIGGER_HAVESPELL, 0));
                source->field_588 = 0;
                check("set-marked-action-selects-available", markedAction(0x135, 2112) == CGameAIBase::ACTION_DONE && source->field_588 == 2112);
                // No-usefulness gate on marking/HaveSpell: silence and existing
                // target effects do not turn a memorized spell into unavailable.
                source->m_derivedStats.m_generalState = STATE_SILENCED;
                target->m_derivedStats.m_generalState = STATE_BERSERK;
                check("have-spell-availability-only", markedTrigger(CAITRIGGER_HAVESPELL, 0));
                source->m_derivedStats.m_generalState = target->m_derivedStats.m_generalState = STATE_NORMAL;
                check("marking-does-not-consume-memorized-spell", markedTrigger(CAITRIGGER_HAVESPELL, 2112));
            }
        }
        reset();
        source->field_588 = 2112;
        CAIObjectType spellTarget(CAIObjectType::ANYONE);
        spellTarget.m_SpecialCase[0] = 55;
        spellTarget.Decode(source);
        check("marked-spell-target-selector-decodes", spellTarget.Equal(target->GetAIType()));
        CString markedResRef;
        source->SpellIdToResRef(0, markedResRef);
        check("marked-spell-zero-resolves-resource", markedResRef == "SPWI112");
        auto preparation = [&](LONG numeric, WORD speed, SHORT mental, SHORT count) {
            abilities[0].speedFactor = speed;
            source->m_derivedStats.m_nMentalSpeed = mental;
            source->m_actionCount = count;
            return markedAction(0x137, numeric);
        };
        struct DelayCase { WORD speed; SHORT mental; BOOL waits; LONG ticks; };
        const DelayCase delayCases[] = {
            {0, 0, FALSE, 0}, {1, 0, FALSE, 0}, {2, 0, TRUE, 14},
            {6, 0, TRUE, 74}, {6, 1, TRUE, 44}, {6, 2, TRUE, 14},
            {6, 3, FALSE, 0}, {1, 2, FALSE, 0}, {0, -1, TRUE, 14},
            {6, -2, TRUE, 134}, {0, 32767, FALSE, 0},
            {65535, 0, FALSE, 0}, {32768, 0, TRUE, 491504},
            {32769, 0, FALSE, 0}, {0, -32768, FALSE, 0},
            {65535, 32767, FALSE, 0}, {65535, -32768, FALSE, 0},
        };
        for (const auto& entry : delayCases) {
            SHORT result = preparation(0, entry.speed, entry.mental, 0);
            check("preparation-short-boundary-return", result == (entry.waits ? CGameAIBase::ACTION_INTERRUPTABLE : CGameAIBase::ACTION_DONE));
            check("preparation-short-boundary-action", source->m_curAction.m_actionID == (entry.waits ? CAIAction::WAIT : 0x137));
            check("preparation-short-boundary-count", source->m_curAction.m_specificID == (entry.waits ? entry.ticks : 2112));
            check("preparation-stores-last-return", source->m_nLastActionReturn == result);
        }
        source->field_588 = 2219;
        check("preparation-explicit-id-ignores-marker", preparation(2112, 6, 0, 1) == CGameAIBase::ACTION_INTERRUPTABLE
            && source->m_curAction.m_specificID == 4 && source->field_588 == 2219);
        source->field_588 = 0;
        check("preparation-explicit-id-without-marker", preparation(2112, 2, 0, 1) == CGameAIBase::ACTION_DONE
            && source->m_curAction.m_actionID == CAIAction::WAIT && source->m_curAction.m_specificID == 0);
        check("preparation-no-spell-direct-error", preparation(0, 6, 0, 0) == CGameAIBase::ACTION_ERROR && source->m_nLastActionReturn == 123);
        source->field_588 = 2112;
        header.abilityCount = 0;
        check("preparation-no-abilities", preparation(0, 6, 0, 0) == CGameAIBase::ACTION_DONE && source->m_curAction.m_actionID == 0x137);
        header.abilityCount = 1; abilities[0].minCasterLevel = 4;
        check("preparation-too-low-caster", preparation(0, 6, 0, 0) == CGameAIBase::ACTION_DONE && source->m_curAction.m_actionID == 0x137);
        header.abilityCount = 2; abilities[0].minCasterLevel = 0; abilities[1].minCasterLevel = 2; abilities[1].speedFactor = 6;
        check("preparation-last-eligible-ability", preparation(0, 2, 0, 0) == CGameAIBase::ACTION_INTERRUPTABLE && source->m_curAction.m_specificID == 74);
        abilities[1].minCasterLevel = 4;
        check("preparation-stops-at-first-ineligible", preparation(0, 2, 0, 0) == CGameAIBase::ACTION_INTERRUPTABLE && source->m_curAction.m_specificID == 14);
        abilities[0].minCasterLevel = 4; abilities[1].minCasterLevel = 0;
        check("preparation-does-not-skip-first-ineligible", preparation(0, 6, 0, 0) == CGameAIBase::ACTION_DONE && source->m_curAction.m_actionID == 0x137);
        header.abilityCount = 1; abilities[0].minCasterLevel = 0;
        check("preparation-enters-real-wait-dispatch", preparation(0, 2, 0, 0) == CGameAIBase::ACTION_INTERRUPTABLE
            && source->m_curAction.m_specificID == 14);
        source->m_actionCount = 1;
        for (INT tick = 0; tick < 14; ++tick) {
            SHORT result = source->ExecuteAction();
            check("prepared-wait-countdown", result == (tick == 13 ? CGameAIBase::ACTION_DONE : CGameAIBase::ACTION_INTERRUPTABLE)
                && source->m_curAction.m_specificID == 13 - tick);
        }
        check("query-no-hp-change", source->m_baseStats.m_hitPoints == sourceBase.m_hitPoints && target->m_baseStats.m_hitPoints == targetBase.m_hitPoints);
        check("query-no-messages", queued == g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount());
        check("query-no-active-opcode-change", target->m_derivedStats.m_activeEffectOpcodes == targetOpcodes);
        check("balanced-resource-ownership", demands == resource->m_nServiceCount && requests == resource->m_nLockCount
            && refs == resource->m_pDimmKeyTableEntry->field_10);
        source->m_baseStats = sourceBase; target->m_baseStats = targetBase;
        source->m_derivedStats.m_generalState = sourceState; target->m_derivedStats.m_generalState = targetState;
        source->m_derivedStats.m_spellStates = sourceStates; target->m_derivedStats.m_spellStates = targetStates;
        source->m_derivedStats.m_classMask = sourceMask;
        memcpy(source->m_derivedStats.m_nClassLevels, sourceLevels, sizeof(sourceLevels));
        source->m_derivedStats.m_nSpellFailureArcane = arcaneFailure; source->m_derivedStats.m_nSpellFailureDivine = divineFailure;
        source->m_derivedStats.m_nMentalSpeed = mentalSpeed;
        source->m_actionCount = oldActionCount;
        memcpy(source->m_derivedStats.m_disabledSpellTypes, disabled, sizeof(disabled));
        memcpy(target->m_derivedStats.m_cImmunitiesSpellLevel.m_levels, immuneLevels, sizeof(immuneLevels));
        source->m_derivedStats.m_cImmunitiesSpell = sourceImmunities; target->m_derivedStats.m_cImmunitiesSpell = targetImmunities;
        source->m_spells = sourceSpells; source->m_domainSpells = sourceDomain;
        source->m_curAction = oldAction; source->field_588 = oldSpell; source->field_3BA = oldSelection; source->m_nLastActionReturn = oldReturn;
        source->m_targetId = oldTargetId; source->m_bSelected = oldSelected;
        target->m_pArea = oldArea; target->m_pos = oldPos; source->m_pArea->m_header.m_areaType = oldAreaType;
        for (INT index = 0; index < 3; ++index) { lists[index]->m_pList = oldListData[index]; lists[index]->m_nCount = oldListCounts[index]; }
        resource->m_pHeader = oldHeader; resource->m_pAbilities = oldAbilities; resource->m_pEffects = oldEffects;
        guard.Release();
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) { Fail(nStep, step.op, "spell/visibility/script mismatch (all state restored)"); return; }
        Emit("{\"step\":%d,\"op\":\"spellusechecks\",\"checks\":%d,\"restored\":true}", nStep, checks);
        ++s_pc; return;
    }

    if (strcmp(step.op, "targetgatechecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        auto* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(sourceObject);
        auto* target = static_cast<CGameSprite*>(targetObject);
        DWORD sourceState = source->m_derivedStats.m_generalState;
        DWORD targetState = target->m_derivedStats.m_generalState;
        DWORD sourceFlags = source->m_baseStats.m_flags, targetFlags = target->m_baseStats.m_flags;
        DWORD sourceBaseState = source->m_baseStats.m_generalState, targetBaseState = target->m_baseStats.m_generalState;
        auto sourceSpells = source->m_derivedStats.m_spellStates, targetSpells = target->m_derivedStats.m_spellStates;
        auto targetVisible = target->m_baseStats.m_visible;
        BOOL sourceSee = source->m_derivedStats.m_bSeeInvisible;
        SHORT sourceAction = source->m_curAction.m_actionID;
        INT queueCount = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"targetgate-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        auto reset = [&]() {
            source->m_derivedStats.m_generalState = target->m_derivedStats.m_generalState = STATE_NORMAL;
            source->m_baseStats.m_generalState = target->m_baseStats.m_generalState = STATE_NORMAL;
            source->m_baseStats.m_flags = target->m_baseStats.m_flags = 0;
            source->m_derivedStats.m_spellStates.reset();
            target->m_derivedStats.m_spellStates.reset();
            source->m_derivedStats.m_bSeeInvisible = FALSE;
            source->m_curAction.m_actionID = 0;
            target->m_baseStats.m_visible = 0;
        };
        reset();
        check("normal-target", source->CheckSpellTarget(target, 0));
        check("null-target", !source->CheckSpellTarget(NULL, 0));
        check("self-target-is-permitted", source->CheckSpellTarget(source, 0));
        const DWORD ignored[] = {1, 2, 0x80000000, 0xFFFFFFFF};
        for (DWORD argument : ignored) check("second-stack-argument-ignored", source->CheckSpellTarget(target, argument));
        source->m_derivedStats.m_generalState = STATE_DEAD;
        check("dead-source", !source->CheckSpellTarget(target, 0));
        reset();
        target->m_derivedStats.m_generalState = STATE_DEAD;
        check("dead-target", !source->CheckSpellTarget(target, 0));
        reset();
        target->m_baseStats.m_flags = 0x20000;
        check("target-base-flag-blocks", !source->CheckSpellTarget(target, 0));
        reset();
        source->m_baseStats.m_flags = 0x20000;
        check("same-source-base-flag-does-not-block", source->CheckSpellTarget(target, 0));
        reset();
        source->m_derivedStats.m_spellStates.set(SPLSTATE_HELD);
        check("held-source", !source->CheckSpellTarget(target, 0));
        reset();
        target->m_derivedStats.m_spellStates.set(SPLSTATE_HELD);
        check("held-target-remains-permitted", source->CheckSpellTarget(target, 0));
        reset();
        source->m_derivedStats.m_generalState = STATE_HELPLESS;
        check("helpless-source", !source->CheckSpellTarget(target, 0));
        reset();
        target->m_derivedStats.m_generalState = STATE_HELPLESS;
        check("helpless-target-remains-permitted", source->CheckSpellTarget(target, 0));
        reset();
        source->m_baseStats.m_generalState = target->m_baseStats.m_generalState = STATE_DEAD;
        check("derived-state-used-instead-of-base", source->CheckSpellTarget(target, 0));
        reset();
        source->m_derivedStats.m_generalState = STATE_STUNNED | STATE_SILENCED | STATE_PANIC;
        check("unlisted-source-states-remain-permitted", source->CheckSpellTarget(target, 0));
        reset();
        source->m_derivedStats.m_spellStates.set(SPLSTATE_SANCTUARY);
        check("source-sanctuary-remains-permitted", source->CheckSpellTarget(target, 0));
        reset();
        target->m_derivedStats.m_spellStates.set(SPLSTATE_SANCTUARY);
        check("target-sanctuary-blocks", !source->CheckSpellTarget(target, 0));
        reset();
        target->m_derivedStats.m_generalState = STATE_IMPROVEDINVISIBILITY;
        check("improved-state-alone-is-not-regular-invisibility", source->CheckSpellTarget(target, 0));
        target->m_derivedStats.m_generalState = STATE_INVISIBLE;
        check("invisible-target-blocks", !source->CheckSpellTarget(target, 0));
        const BYTE visible[] = {1, 2, 3, 0x80};
        for (BYTE flag : visible) {
            target->m_baseStats.m_visible = flag;
            check("only-force-visible-bit-exposes-target", source->CheckSpellTarget(target, 0) == ((flag & 1) != 0));
        }
        target->m_baseStats.m_visible = 0;
        source->m_derivedStats.m_bSeeInvisible = TRUE;
        check("derived-see-invisible", source->CheckSpellTarget(target, 0));
        source->m_derivedStats.m_bSeeInvisible = FALSE;
        source->m_baseStats.m_flags = 0x10000;
        check("base-see-invisible", source->CheckSpellTarget(target, 0));
        source->m_baseStats.m_flags = 0;
        const SHORT dialogues[] = {CAIAction::DIALOGUE, CAIAction::STARTDIALOG, CAIAction::STARTDIALOGUENOSET};
        for (SHORT action : dialogues) {
            source->m_curAction.m_actionID = action;
            check("dialogue-can-see-invisible", source->GetCanSeeInvisible() && source->CheckSpellTarget(target, 0));
            target->m_derivedStats.m_spellStates.set(SPLSTATE_SANCTUARY);
            check("dialogue-does-not-bypass-sanctuary", !source->CheckSpellTarget(target, 0));
            target->m_derivedStats.m_spellStates.reset(SPLSTATE_SANCTUARY);
        }
        source->m_curAction.m_actionID = CAIAction::ATTACK;
        check("attack-action-has-no-invisibility-exemption", !source->GetCanSeeInvisible() && !source->CheckSpellTarget(target, 0));
        check("direct-visibility-nonzero-bool", target->CheckInvisibility(0x100));
        check("pure-query-no-messages", queueCount == g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount());
        source->m_derivedStats.m_generalState = sourceState;
        target->m_derivedStats.m_generalState = targetState;
        source->m_baseStats.m_generalState = sourceBaseState;
        target->m_baseStats.m_generalState = targetBaseState;
        source->m_baseStats.m_flags = sourceFlags;
        target->m_baseStats.m_flags = targetFlags;
        source->m_derivedStats.m_spellStates = sourceSpells;
        target->m_derivedStats.m_spellStates = targetSpells;
        source->m_derivedStats.m_bSeeInvisible = sourceSee;
        source->m_curAction.m_actionID = sourceAction;
        target->m_baseStats.m_visible = targetVisible;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) { Fail(nStep, step.op, "target gate mismatch (actors restored)"); return; }
        Emit("{\"step\":%d,\"op\":\"targetgatechecks\",\"checks\":%d,\"restored\":true}", nStep, checks);
        ++s_pc;
        return;
    }

    if (strcmp(step.op, "whenstruckchecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            Fail(nStep, step.op, "requires world, party and local service provider");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(1);
        CGameObject* object = NULL;
        if (objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "defender unavailable");
            return;
        }
        CGameSprite* sprite = static_cast<CGameSprite*>(object);
        auto saved = sprite->m_derivedStats.m_onHitSpells;
        auto savedPlayer = sprite->m_remotePlayerID;
        auto savedRemote = sprite->m_remoteObjectID;
        INT provider = g_pChitin->cNetwork.m_nServiceProvider;
        BOOLEAN connected = g_pChitin->cNetwork.m_bConnectionEstablished;
        // The loaded local save can mark its dummy session established. Keep
        // the provider test closed without opening/closing an actual session.
        g_pChitin->cNetwork.m_bConnectionEstablished = FALSE;
        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT queued = queue.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"whenstruck-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        ITEM_EFFECT item = {};
        item.effectID = ICEWIND_CGAMEEFFECT_WHENSTRUCK;
        CGameEffect* effect = CGameEffect::DecodeEffect(&item, CPoint(123, 456), id, CPoint(-678, 901));
        check("decode-virtual-installer", dynamic_cast<IcewindCGameEffectWhenStruck*>(effect) != NULL);
        if (effect != NULL) {
            effect->m_sourceRes = "ZRCORIG2";
            effect->m_res = "ZRCHIT";
            g_pChitin->cNetwork.m_nServiceProvider = CNetwork::SERV_PROV_NULL;
            sprite->m_derivedStats.m_onHitSpells.clear();
            const DWORD levels[] = {0, 1, 255, 256, 257, 0x112FF};
            for (INT index = 0; index < 6; ++index) {
                effect->m_casterLevel = levels[index];
                BOOL done = effect->m_done;
                check("installer-success", effect->ApplyEffect(sprite));
                const COnHitSpell& tail = sprite->m_derivedStats.m_onHitSpells.back();
                check("append-retains-earlier-records", sprite->m_derivedStats.m_onHitSpells.size() == index + 1
                    && sprite->m_derivedStats.m_onHitSpells.front().m_level == 0);
                check("installer-byte-level-and-provenance", tail.m_level == static_cast<BYTE>(levels[index])
                    && tail.m_sourceRes == CResRef("ZRCORIG2") && tail.m_spellRes == CResRef("ZRCHIT")
                    && tail.m_target == CPoint(-678, 901) && effect->m_done == done);
                check("local-no-message", queue.GetCount() == queued);
            }
            CGameEffect* copy = effect->Copy();
            check("copy-concrete-type", dynamic_cast<IcewindCGameEffectWhenStruck*>(copy) != NULL);
            check("copy-payload", copy && copy->m_casterLevel == 0x112FF && copy->m_sourceID == id
                && copy->m_source == CPoint(123, 456) && copy->m_target == CPoint(-678, 901)
                && copy->m_sourceRes == CResRef("ZRCORIG2") && copy->m_res == CResRef("ZRCHIT"));
            if (copy) check("copy-virtual-installer", copy->ApplyEffect(sprite));
            delete copy;
            g_pChitin->cNetwork.m_nServiceProvider = 1;
            check("provider-with-closed-session", effect->ApplyEffect(sprite)
                && sprite->m_derivedStats.m_onHitSpells.size() == 8 && queue.GetCount() == queued);
            // BROADCAST_FORCED_OTHERS deletes the outgoing message when the
            // session is closed. It must not run locally and replace the list.
            g_pChitin->cNetwork.m_nServiceProvider = provider;
            delete effect;
        }
        CMessage117 message(CResRef("ZRCORIG2"), CResRef("ZRCHIT"), 0x1000101,
            CPoint(-678, 901), 77, id);
        check("message-routing", message.GetCommType() == CMessage::BROADCAST_FORCED_OTHERS
            && message.GetMsgType() == CBaldurMessage::MSG_TYPE_CMESSAGE && message.GetMsgSubType() == 117
            && message.GetSource() == 77 && message.GetTarget() == id);
        BYTE buffer[64];
        memset(buffer, 0xA5, sizeof(buffer));
        BYTE* data = buffer;
        DWORD size = 999;
        message.MarshalMessage(&data, &size);
        check("marshal-preallocated-buffer", data == buffer && size == 36 && buffer[36] == 0xA5);
        check("marshal-object-identity", *reinterpret_cast<PLAYER_ID*>(buffer) == savedPlayer
            && *reinterpret_cast<LONG*>(buffer + 4) == savedRemote);
        check("marshal-exact-payload", memcmp(buffer + 8, "ZRCORIG2", 8) == 0
            && memcmp(buffer + 16, "ZRCHIT\0\0", 8) == 0
            && *reinterpret_cast<DWORD*>(buffer + 24) == 0x1000101
            && *reinterpret_cast<LONG*>(buffer + 28) == -678
            && *reinterpret_cast<LONG*>(buffer + 32) == 901);
        message.m_targetId = CGameObjectArray::INVALID_INDEX;
        BYTE unchanged[64];
        memcpy(unchanged, buffer, sizeof(buffer));
        size = 999;
        message.MarshalMessage(&data, &size);
        check("marshal-invalid-target", size == 0 && data == buffer && memcmp(buffer, unchanged, sizeof(buffer)) == 0);
        message.Run();
        check("run-invalid-target", sprite->m_derivedStats.m_onHitSpells.size() == 8);
        message.m_targetId = id;

        // Install one typed remote mapping in bucket zero for the duration of
        // this tick. Restore the exact head instead of deleting actor mappings.
        CGameRemoteObjectArray* remotes = game->GetRemoteObjectArray();
        CGameRemoteObjectListEntry mapping;
        mapping.remotePlayerID = 0xF1234567;
        mapping.remoteObjectID = 0x1234;
        mapping.localObjectID = id;
        mapping.pNext = remotes->m_pArray[0];
        remotes->m_pArray[0] = &mapping;
        BYTE wire[64] = {};
        DWORD header = CNetwork::SPEC_MSG_HEADER_LENGTH;
        memcpy(wire + header, buffer, 36);
        *reinterpret_cast<PLAYER_ID*>(wire + header) = mapping.remotePlayerID;
        *reinterpret_cast<LONG*>(wire + header + 4) = mapping.remoteObjectID;
        CMessage117 received(CResRef("OLDRES"), CResRef("OLDSPELL"), 3, CPoint(4, 5), 88, -1);
        check("unmarshal-mapped", received.UnmarshalMessage(wire, header + 36));
        check("unmarshal-full-width-and-provenance", received.GetTarget() == id && received.GetSource() == 88
            && received.m_sourceRes == CResRef("ZRCORIG2") && received.m_spellRes == CResRef("ZRCHIT")
            && received.m_level == 0x1000101 && received.m_target == CPoint(-678, 901));
        *reinterpret_cast<PLAYER_ID*>(wire + header) = 0xF7654321;
        check("unmarshal-unmapped", !received.UnmarshalMessage(wire, header + 36));
        check("unmapped-leaves-message-unchanged", received.GetTarget() == id && received.GetSource() == 88
            && received.m_sourceRes == CResRef("ZRCORIG2") && received.m_spellRes == CResRef("ZRCHIT")
            && received.m_level == 0x1000101 && received.m_target == CPoint(-678, 901));
        remotes->m_pArray[0] = mapping.pNext;
        received.Run();
        check("run-replaces-all-records", sprite->m_derivedStats.m_onHitSpells.size() == 1);
        const COnHitSpell& installed = sprite->m_derivedStats.m_onHitSpells.front();
        check("run-full-width-and-provenance", installed.m_level == 0x1000101
            && installed.m_sourceRes == CResRef("ZRCORIG2") && installed.m_spellRes == CResRef("ZRCHIT")
            && installed.m_target == CPoint(-678, 901));
        sprite->m_derivedStats.m_onHitSpells.clear();
        received.m_level = 0;
        received.Run();
        check("run-empty-list-level-zero", sprite->m_derivedStats.m_onHitSpells.size() == 1
            && sprite->m_derivedStats.m_onHitSpells.front().m_level == 0);

        std::list<CMessage*> pending;
        while (!queue.IsEmpty()) pending.push_back(queue.RemoveHead());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessage117(CResRef("ZRCORIG1"),
            CResRef("ZRCHIT"), 257, CPoint(12, -34), 99, id), TRUE);
        g_pBaldurChitin->GetMessageHandler()->AsynchronousUpdate();
        check("message-drain-installs-riposte", queue.IsEmpty() && sprite->m_derivedStats.m_onHitSpells.size() == 1
            && sprite->m_derivedStats.m_onHitSpells.front().m_sourceRes == CResRef("ZRCORIG1")
            && sprite->m_derivedStats.m_onHitSpells.front().m_level == 257
            && sprite->m_derivedStats.m_onHitSpells.front().m_target == CPoint(12, -34));
        for (CMessage* entry : pending) queue.AddTail(entry);

        CGameObject nonSprite;
        LONG other = -1;
        BOOL added = objects->Add(&other, &nonSprite, INFINITE) == CGameObjectArray::SUCCESS;
        check("non-sprite-registration", added);
        if (added) {
            received.m_targetId = other;
            received.Run();
            check("run-non-sprite-keeps-defender", sprite->m_derivedStats.m_onHitSpells.size() == 1
                && sprite->m_derivedStats.m_onHitSpells.front().m_level == 257);
            CGameObject* removed = NULL;
            check("non-sprite-released-and-removed", objects->Delete(other, CGameObjectArray::THREAD_ASYNCH,
                &removed, INFINITE) == CGameObjectArray::SUCCESS && removed == &nonSprite);
        }
        sprite->m_derivedStats.m_onHitSpells = saved;
        sprite->m_remotePlayerID = savedPlayer;
        sprite->m_remoteObjectID = savedRemote;
        g_pChitin->cNetwork.m_nServiceProvider = provider;
        g_pChitin->cNetwork.m_bConnectionEstablished = connected;
        objects->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, step.op, "installer/message mismatch (actor, mapping and queue restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"whenstruckchecks\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "projectilechecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(sourceObject);
        CGameSprite* target = static_cast<CGameSprite*>(targetObject);
        CSpell spell(CResRef("ZRCHIT"));
        if (!spell.Demand() || !spell.pRes->m_bParsed) {
            objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "ZRCHIT fixture unavailable");
            return;
        }
        CResSpell* resource = spell.pRes;
        SPELL_HEADER* originalHeader = resource->m_pHeader;
        SPELL_ABILITY* originalAbilities = resource->m_pAbilities;
        ITEM_EFFECT* originalEffects = resource->m_pEffects;
        SPELL_HEADER header = *originalHeader;
        SPELL_ABILITY abilities[2] = {originalAbilities[0], originalAbilities[0]};
        ITEM_EFFECT effects[4] = {originalEffects[0], originalEffects[1], originalEffects[0], originalEffects[1]};
        header.abilityCount = 2;
        abilities[1].minCasterLevel = 2;
        abilities[1].startingEffect = 2;
        effects[2].effectAmount = 11;
        effects[3].effectAmount = 12;
        resource->m_pHeader = &header;
        resource->m_pAbilities = abilities;
        resource->m_pEffects = effects;
        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        std::list<CMessage*> pending;
        while (!queue.IsEmpty()) pending.push_back(queue.RemoveHead());
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"projectile-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        auto discardMessages = [&]() {
            while (!queue.IsEmpty()) delete queue.RemoveHead();
        };
        ITEM_EFFECT item = {};
        item.effectID = ICEWIND_CGAMEEFFECT_PROJECTILE;
        CGameEffect* effect = CGameEffect::DecodeEffect(&item, CPoint(123, 456), sourceId, CPoint(-678, 901));
        check("decode-concrete-projectile", dynamic_cast<IcewindCGameEffectProjectile*>(effect) != NULL);
        if (effect) {
            effect->m_res = "ZRCHIT";
            effect->m_sourceRes = "ZRCORIG2";
            effect->m_casterLevel = 0x112FF;
            CGameEffect* copy = effect->Copy();
            check("copy-concrete-type", dynamic_cast<IcewindCGameEffectProjectile*>(copy) != NULL);
            check("copy-payload", copy && copy->m_res == CResRef("ZRCHIT")
                && copy->m_sourceRes == CResRef("ZRCORIG2") && copy->m_casterLevel == 0x112FF
                && copy->m_sourceID == sourceId && copy->m_source == CPoint(123, 456)
                && copy->m_target == CPoint(-678, 901));
            const DWORD levels[] = {0, 1, 255, 256, 257, 0x112FF};
            for (INT index = 0; index < 6; ++index) {
                CGameEffect* current = index == 5 && copy ? copy : effect;
                current->m_casterLevel = levels[index];
                current->m_done = FALSE;
                check("instant-virtual-apply-done", current->ApplyEffect(target) && current->m_done);
                check("instant-one-grouped-message", queue.GetCount() == 1);
                CMessage105* message = queue.IsEmpty() ? NULL : dynamic_cast<CMessage105*>(queue.GetHead());
                check("grouped-target-routing", message && message->GetSource() == sourceId
                    && message->GetTarget() == targetId && message->m_nCount == 2);
                INT count = 0;
                if (message) {
                    for (auto* node = message->m_pNodeHead->m_pNext; node != message->m_pNodeHead; node = node->m_pNext) {
                        auto* child = dynamic_cast<CMessageAddEffect*>(node->m_pMessage);
                        CGameEffect* payload = child ? child->m_effect : NULL;
                        check("forced-level-one-ability-order", payload && payload->m_effectAmount == count + 1);
                        check("payload-full-level-and-origin", payload && payload->m_casterLevel == levels[index]
                            && payload->m_sourceID == sourceId && payload->m_source == source->GetPos()
                            && payload->m_sourceRes == CResRef("ZRCHIT") && payload->m_target == CPoint(-678, 901)
                            && payload->m_projectileType == 0);
                        check("both-target-types-delivered-to-defender", child && child->GetSource() == sourceId
                            && child->GetTarget() == targetId && payload && payload->m_targetType == count + 1);
                        ++count;
                    }
                }
                check("instant-payload-count", count == 2);
                discardMessages();
            }
            delete copy;
            // Compare the live travelling objects before any engine tick, then
            // remove them through their recovered ownership path.
            effect->m_casterLevel = 0x112FF;
            const DWORD flags[] = {1, 11, 16, 0x10001};
            for (INT index = 0; index < 4; ++index) {
                std::set<CGameObject*> before;
                for (INT slot = 0; slot < objects->m_nArraySize; ++slot) {
                    if (objects->m_pArray[slot].m_objectId != -1) before.insert(objects->m_pArray[slot].m_objectPtr);
                }
                effect->m_dwFlags = flags[index];
                effect->m_done = FALSE;
                check("travelling-virtual-apply-done", effect->ApplyEffect(target) && effect->m_done);
                CProjectile* projectile = NULL;
                INT created = 0;
                for (INT slot = 0; slot < objects->m_nArraySize; ++slot) {
                    auto& entry = objects->m_pArray[slot];
                    if (entry.m_objectId == -1 || before.count(entry.m_objectPtr)) continue;
                    CProjectile* candidate = dynamic_cast<CProjectile*>(entry.m_objectPtr);
                    if (candidate) { projectile = candidate; ++created; }
                }
                check("one-travelling-object-no-early-message", created == 1 && queue.IsEmpty());
                check("word-factory-selects-concrete-type", projectile && (index == 1
                    ? dynamic_cast<CProjectileBolt*>(projectile) != NULL : index == 2
                    ? dynamic_cast<CProjectileSlingStone*>(projectile) != NULL
                    : dynamic_cast<CProjectileArrow*>(projectile) != NULL));
                check("fire-source-target-area", projectile && projectile->m_sourceId == sourceId
                    && projectile->m_targetId == targetId && projectile->m_pArea == target->m_pArea
                    && projectile->CGameObject::m_pArea == target->m_pArea
                    && projectile->m_targetX == target->GetPos().x && projectile->m_targetY == target->GetPos().y);
                check("travelling-retains-two-effects", projectile && projectile->m_effectList.GetCount() == 2);
                if (projectile) {
                    POSITION pos = projectile->m_effectList.GetHeadPosition();
                    INT count = 0;
                    while (pos) {
                        CGameEffect* payload = projectile->m_effectList.GetNext(pos);
                        check("travelling-payload-metadata", payload && payload->m_effectAmount == ++count
                            && payload->m_casterLevel == 0x112FF && payload->m_sourceRes == CResRef("ZRCHIT")
                            && payload->m_sourceID == sourceId && payload->m_source == source->GetPos()
                            && payload->m_target == CPoint(-678, 901)
                            && payload->m_projectileType == static_cast<WORD>(flags[index]));
                    }
                    projectile->RemoveSelf();
                }
            }
            effect->m_dwFlags = 0;
            abilities[0].effectCount = 0;
            check("empty-payload-still-finishes", effect->ApplyEffect(target) && effect->m_done);
            auto* empty = queue.IsEmpty() ? NULL : dynamic_cast<CMessage105*>(queue.GetHead());
            check("empty-grouped-message", queue.GetCount() == 1 && empty && empty->m_nCount == 0);
            discardMessages();
            abilities[0].effectCount = originalAbilities[0].effectCount;
            const LONG invalidIds[] = {CGameObjectArray::INVALID_INDEX, sourceId ^ 0x10000};
            for (LONG invalidId : invalidIds) {
                effect->m_sourceID = invalidId;
                effect->m_done = FALSE;
                SHORT nextId = objects->m_nNextObjectId;
                check("missing-source-finishes-without-firing", effect->ApplyEffect(target) && effect->m_done
                    && queue.IsEmpty() && objects->m_nNextObjectId == nextId);
            }
            delete effect;
        }
        discardMessages();
        for (CMessage* message : pending) queue.AddTail(message);
        resource->m_pHeader = originalHeader;
        resource->m_pAbilities = originalAbilities;
        resource->m_pEffects = originalEffects;
        spell.Release();
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, step.op, "projectile payload mismatch (resource and queue restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"projectilechecks\",\"checks\":%d,\"restored\":true}", nStep, checks);
        ++s_pc;
        return;
    }

    if (strcmp(step.op, "projectiledamage") == 0 || strcmp(step.op, "projectilewait") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(sourceObject);
        auto* target = static_cast<CGameSprite*>(targetObject);
        BOOL fire = strcmp(step.op, "projectiledamage") == 0;
        BOOL applied = TRUE;
        INT sourceLoss = 0, targetLoss = 0;
        if (fire) {
            s_projectileFlags = Num(step, 0, 0);
            s_projectileSourceHP = source->m_baseStats.m_hitPoints;
            s_projectileTargetHP = target->m_baseStats.m_hitPoints;
            ITEM_EFFECT item = {};
            item.effectID = ICEWIND_CGAMEEFFECT_PROJECTILE;
            item.dwFlags = s_projectileFlags;
            CGameEffect* effect = CGameEffect::DecodeEffect(&item, source->GetPos(), sourceId, target->GetPos());
            if (effect) {
                effect->m_res = "ZRCHIT";
                effect->m_casterLevel = 257;
                applied = effect->ApplyEffect(target) && effect->m_done;
                delete effect;
            } else applied = FALSE;
        } else {
            sourceLoss = s_projectileSourceHP - source->m_baseStats.m_hitPoints;
            targetLoss = s_projectileTargetHP - target->m_baseStats.m_hitPoints;
        }
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!applied) {
            Fail(nStep, step.op, "decoded projectile did not finish");
            return;
        }
        if (!fire && (sourceLoss != 0 || targetLoss != 3)) {
            if (sourceLoss != 0 || targetLoss > 3 || ++s_waited > DEFAULT_WAIT_TICKS)
                Fail(nStep, step.op, "expected three target damage and no source damage");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"%s\",\"flags\":%d,\"sourceLoss\":%d,\"targetLoss\":%d,\"checks\":%d}",
            nStep, step.op, s_projectileFlags, sourceLoss, targetLoss, fire ? 1 : 2);
        s_waited = 0;
        ++s_pc;
        return;
    }

    if (strcmp(step.op, "hitspellchecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(object);
        CSpell spell(CResRef("ZRCHIT"));
        if (!spell.Demand() || !spell.pRes->m_bParsed) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "ZRCHIT fixture unavailable");
            return;
        }
        // Keep the resource demanded while substituting typed local tables.
        // Nested Demand calls then retain these tables. Restore before Release.
        CResSpell* resource = spell.pRes;
        SPELL_HEADER* originalHeader = resource->m_pHeader;
        SPELL_ABILITY* originalAbilities = resource->m_pAbilities;
        ITEM_EFFECT* originalEffects = resource->m_pEffects;
        SPELL_HEADER header = *originalHeader;
        SPELL_ABILITY abilities[3] = {};
        ITEM_EFFECT effects[6] = {};
        for (INT index = 0; index < 3; ++index) {
            abilities[index] = originalAbilities[0];
            abilities[index].minCasterLevel = index == 0 ? 1 : index == 1 ? 3 : 257;
            abilities[index].startingEffect = index * 2;
            for (INT e = 0; e < 2; ++e) {
                effects[index * 2 + e] = originalEffects[e];
                effects[index * 2 + e].effectAmount = index * 10 + e + 1;
                effects[index * 2 + e].saveMod = -3;
            }
        }
        header.abilityCount = 3;
        header.school = 0;
        header.secondaryType = 37;
        header.itemFlags = 0x1234;
        resource->m_pHeader = &header;
        resource->m_pAbilities = abilities;
        resource->m_pEffects = effects;
        auto savedBase = source->m_baseStats;
        auto savedSpells = source->m_spells;
        auto savedDomain = source->m_domainSpells;
        SHORT savedInt = source->m_derivedStats.m_nINT;
        SHORT savedWis = source->m_derivedStats.m_nWIS;
        SHORT savedChr = source->m_derivedStats.m_nCHR;
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"hitspell-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
        memset(source->m_baseStats.m_classLevels, 0, sizeof(source->m_baseStats.m_classLevels));
        source->m_baseStats.m_specialization = 0;
        CGameEffect* effect = spell.BuildAbilityEffect(0, 0, NULL, 3, 0x40000, 17);
        check("null-caster-metadata", effect && effect->m_school == 0 && effect->field_48 == 37
            && effect->m_sourceType == 1 && effect->m_sourceFlags == 0x1234
            && effect->m_sourceRes == CResRef("ZRCHIT"));
        check("null-caster-unpacked", effect && effect->m_casterLevel == 0 && effect->m_firstCall == 0);
        check("decode-coordinates", effect && effect->m_source == CPoint(-1, -1)
            && effect->m_target == CPoint(-1, -1) && effect->m_sourceID == -1 && effect->m_saveMod == -3);
        delete effect;
        CGameObject nonSprite;
        effect = spell.BuildAbilityEffect(0, 0, &nonSprite, 11, 0, 17);
        check("non-sprite-level-one", effect && effect->m_casterLevel == 0xB01 && effect->m_firstCall == 0);
        delete effect;
        check("invalid-ability", spell.BuildAbilityEffect(3, 0, source, 0, 0, 17) == NULL);
        check("invalid-effect", spell.BuildAbilityEffect(0, 2, source, 0, 0, 17) == NULL);
        header.itemFlags = 0x4000;
        header.field_72 = 2;
        header.field_73 = 3;
        effect = spell.BuildAbilityEffect(0, 0, NULL, 3, 0x40000, 0);
        check("null-caster-default-duration-level", effect && effect->m_duration == 161 && effect->m_casterLevel == 0);
        delete effect;
        effect = spell.BuildAbilityEffect(0, 0, &nonSprite, 11, 0, 0);
        check("non-sprite-default-duration-level", effect && effect->m_duration == 161 && effect->m_casterLevel == 0xB01);
        delete effect;
        effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 255);
        check("explicit-byte-level-duration", effect && effect->m_duration == 3591 && effect->m_casterLevel == 0xBFF);
        delete effect;
        header.itemFlags = 0x1234;
        source->m_baseStats.m_feats[2] = 1;
        source->m_baseStats.m_featSpellPenetration = 2;
        effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
        check("sprite-level-and-penetration", effect && effect->m_casterLevel == 0xB11 && effect->m_firstCall == 4);
        delete effect;
        effect = spell.BuildAbilityEffect(0, 0, source, 3, 0x40000, 17);
        BYTE domainIndex = game->GetRuleTables().GetSpecializationIndex(3, 0x40000);
        check("cleric-domain-packed", effect && effect->m_casterLevel == (17 | 3 << 8 | DWORD(domainIndex) << 16));
        delete effect;
        const BYTE schools[] = {4, 6, 7, 8};
        const DWORD specializations[] = {0x200, 0x800, 0x1000, 0x2000};
        source->m_baseStats.m_featFocusEnchantment = 2;
        source->m_baseStats.m_featFocusEvocation = 2;
        source->m_baseStats.m_featFocusNecromancy = 2;
        source->m_baseStats.m_featFocusTransmutation = 2;
        for (INT index = 0; index < 4; ++index) {
            header.school = schools[index];
            source->m_baseStats.m_specialization = specializations[index];
            source->m_baseStats.m_feats[1] = 1U << (28 + index);
            effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
            check("focus-and-matching-school", effect && effect->m_school == schools[index] && effect->m_saveMod == 3);
            delete effect;
            source->m_baseStats.m_specialization = 0;
            effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
            check("focus-without-specialization", effect && effect->m_saveMod == 1);
            delete effect;
            source->m_baseStats.m_feats[1] = 0;
            source->m_baseStats.m_specialization = specializations[index];
            effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
            check("specialization-without-focus", effect && effect->m_saveMod == -1);
            delete effect;
        }
        header.school = 0;
        source->m_baseStats.m_specialization = 0;
        const INT levels[] = {-1, 0, 1, 2, 3, 255, 256, 257, 65535, 0x10001};
        const INT amounts[] = {0, 0, 1, 1, 11, 11, 11, 21, 21, 21};
        for (INT index = 0; index < 10; ++index) {
            std::list<CGameEffect*> built = BuildSubSpellEffects(source, CResRef("ZRCHIT"), levels[index]);
            check("signed-level-ability-selection", amounts[index] == 0 ? built.empty()
                : built.size() == 2 && built.front()->m_effectAmount == amounts[index]
                    && built.back()->m_effectAmount == amounts[index] + 1);
            if (!built.empty()) {
                check("subspell-source-provenance", built.front()->m_source == source->GetPos()
                    && built.back()->m_sourceID == sourceId && built.front()->m_sourceRes == CResRef("ZRCHIT"));
            }
            for (CGameEffect* payload : built) delete payload;
        }
        abilities[1].effectCount = 0;
        auto empty = BuildSubSpellEffects(source, CResRef("ZRCHIT"), 3);
        check("empty-selected-ability", empty.empty());
        for (CGameEffect* payload : empty) delete payload;
        abilities[1].effectCount = 2;
        CGameObject* targetObject = NULL;
        LONG targetId = game->GetFixedOrderCharacterId(1);
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) == CGameObjectArray::SUCCESS) {
            auto savedOnHit = source->m_derivedStats.m_onHitSpells;
            auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
            INT before = queue.GetCount();
            const INT onHitLevels[] = {-1, 256, 257};
            const INT onHitAmounts[] = {0, 11, 21};
            for (INT index = 0; index < 3; ++index) {
                source->m_derivedStats.m_onHitSpells.clear();
                COnHitSpell entry;
                entry.m_sourceRes = "ZRCLEVEL";
                entry.m_spellRes = "ZRCHIT";
                entry.m_level = onHitLevels[index];
                entry.m_target = CPoint(678, 901);
                source->m_derivedStats.m_onHitSpells.push_back(entry);
                source->ApplyOnHitSpells(static_cast<CGameSprite*>(targetObject));
                check("onhit-full-width-level", queue.GetCount() == before + (index == 0 ? 0 : 2));
                for (INT e = queue.GetCount() - before - 1; e >= 0; --e) {
                    CMessage* raw = queue.RemoveTail();
                    CMessageAddEffect* message = dynamic_cast<CMessageAddEffect*>(raw);
                    check("onhit-level-payload-and-recipient", message && message->m_effect->m_effectAmount == onHitAmounts[index] + e
                        && message->GetTarget() == (e == 0 ? sourceId : targetId)
                        && message->m_effect->m_sourceRes == CResRef("ZRCLEVEL")
                        && message->m_effect->m_target == CPoint(678, 901));
                    delete raw;
                }
            }
            source->m_derivedStats.m_onHitSpells = savedOnHit;
            objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        } else {
            check("onhit-target-available", FALSE);
        }
        header.abilityCount = 0;
        empty = BuildSubSpellEffects(source, CResRef("ZRCHIT"), 3);
        check("zero-abilities", empty.empty());
        for (CGameEffect* payload : empty) delete payload;
        header.abilityCount = 3;

        UINT requiredId = 0;
        BOOL found = game->m_spells.Find(CResRef("SPWI710"), requiredId);
        check("required-spell-catalogued", found);
        source->m_derivedStats.m_nINT = 18;
        source->m_derivedStats.m_nCHR = 16;
        source->m_derivedStats.m_nWIS = 20;
        for (UINT cls = 0; cls < CSPELLLIST_NUM_CLASSES; ++cls) {
            source->m_spells.m_spellsByClass[cls].Clear();
            DWORD mask = game->GetSpellcasterClassMask(cls);
            if (found && (mask & (CLASSMASK_WIZARD | CLASSMASK_SORCERER)) != 0) {
                CGameSpriteSpellListEntry entry = {requiredId, 1, 1, 0};
                source->m_spells.m_spellsByClass[cls].m_lists[0].m_List.push_back(entry);
            }
        }
        source->m_domainSpells.Clear();
        if (found) {
            CGameSpriteSpellListEntry entry = {requiredId, 1, 1, 0};
            source->m_domainSpells.m_lists[0].m_List.push_back(entry);
        }
        const char* specialSpells[] = {"SPIN126", "SPIN127", "SPIN128", "SPIN129", "SPIN130", "SPIN131", "SPIN132"};
        // Alias only the helper's resref, retaining the pinned fixture resource.
        // This tests SPIN metadata branches without overriding installed assets.
        for (INT index = 0; index < 7; ++index) {
            spell.cResRef = specialSpells[index];
            source->m_baseStats.m_wizardLevel = 1;
            source->m_baseStats.m_sorcererLevel = 0;
            source->m_baseStats.m_clericLevel = 0;
            effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
            check("special-wizard-save-modifier", effect && effect->m_saveMod == 1);
            delete effect;
        }
        source->m_baseStats.m_wizardLevel = 0;
        source->m_baseStats.m_sorcererLevel = 1;
        effect = spell.BuildAbilityEffect(0, 0, source, 10, 0, 17);
        check("special-sorcerer-save-modifier", effect && effect->m_saveMod == 0);
        delete effect;
        source->m_baseStats.m_sorcererLevel = 0;
        source->m_baseStats.m_clericLevel = 1;
        source->m_baseStats.m_specialization = 0x40000;
        effect = spell.BuildAbilityEffect(0, 0, source, 3, 0x40000, 17);
        check("special-domain-save-modifier", effect && effect->m_saveMod == 2);
        delete effect;
        source->m_baseStats.m_specialization = 0x80000;
        effect = spell.BuildAbilityEffect(0, 0, source, 3, 0x80000, 17);
        check("wrong-domain-no-bonus", effect && effect->m_saveMod == -3);
        delete effect;
        source->m_baseStats.m_specialization = 0x40000;
        source->m_baseStats.m_wizardLevel = 1;
        source->m_baseStats.m_sorcererLevel = 1;
        effect = spell.BuildAbilityEffect(0, 0, source, 3, 0x40000, 17);
        check("three-class-average", effect && effect->m_saveMod == 1);
        delete effect;
        source->m_derivedStats.m_nINT = 8;
        source->m_derivedStats.m_nCHR = 8;
        source->m_baseStats.m_clericLevel = 0;
        effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
        check("unsigned-negative-average", effect && effect->m_saveMod == 0x7FFFFFFC);
        delete effect;
        source->m_baseStats.m_wizardLevel = 0;
        source->m_baseStats.m_sorcererLevel = 0;
        effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
        check("no-class-no-special-bonus", effect && effect->m_saveMod == -3);
        delete effect;
        source->m_baseStats.m_wizardLevel = 1;
        for (UINT cls = 0; cls < CSPELLLIST_NUM_CLASSES; ++cls) {
            source->m_spells.m_spellsByClass[cls].Clear();
        }
        effect = spell.BuildAbilityEffect(0, 0, source, 11, 0, 17);
        check("missing-required-spell-no-bonus", effect && effect->m_saveMod == -3);
        delete effect;
        source->m_baseStats = savedBase;
        source->m_spells = savedSpells;
        source->m_domainSpells = savedDomain;
        source->m_derivedStats.m_nINT = savedInt;
        source->m_derivedStats.m_nWIS = savedWis;
        source->m_derivedStats.m_nCHR = savedChr;
        spell.cResRef = "ZRCHIT";
        resource->m_pHeader = originalHeader;
        resource->m_pAbilities = originalAbilities;
        resource->m_pEffects = originalEffects;
        spell.Release();
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, step.op, "spell payload mismatch (resource and actor restored)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"hitspellchecks\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "onhitdamage") == 0 || strcmp(step.op, "onhitwait") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(sourceObject);
        CGameSprite* target = static_cast<CGameSprite*>(targetObject);
        BOOL fire = strcmp(step.op, "onhitdamage") == 0;
        INT sourceLoss = 0, targetLoss = 0;
        if (fire) {
            s_onHitSourceHP = source->m_baseStats.m_hitPoints;
            s_onHitTargetHP = target->m_baseStats.m_hitPoints;
            auto saved = source->m_derivedStats.m_onHitSpells;
            source->m_derivedStats.m_onHitSpells.clear();
            COnHitSpell entry;
            entry.m_sourceRes = "ZRCORIG1";
            entry.m_spellRes = "ZRCHIT";
            entry.m_level = 1;
            entry.m_target = target->GetPos();
            source->m_derivedStats.m_onHitSpells.push_back(entry);
            source->ApplyOnHitSpells(target);
            source->m_derivedStats.m_onHitSpells = saved;
        } else {
            sourceLoss = s_onHitSourceHP - source->m_baseStats.m_hitPoints;
            targetLoss = s_onHitTargetHP - target->m_baseStats.m_hitPoints;
        }
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!fire && (sourceLoss != 1 || targetLoss != 2)) {
            if (sourceLoss > 1 || targetLoss > 2 || ++s_waited > DEFAULT_WAIT_TICKS) {
                Fail(nStep, step.op, "expected one self damage and two target damage");
            }
            return;
        }
        Emit("{\"step\":%d,\"op\":\"%s\",\"sourceLoss\":%d,\"targetLoss\":%d,\"checks\":%d}",
            nStep, step.op, sourceLoss, targetLoss, fire ? 0 : 2);
        s_waited = 0;
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatonhit") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2) {
            Fail(nStep, "combatonhit", "requires world and two party members");
            return;
        }
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceObject = NULL;
        CGameObject* targetObject = NULL;
        if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, "combatonhit", "source unavailable");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetObject, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, "combatonhit", "target unavailable");
            return;
        }
        CGameSprite* source = static_cast<CGameSprite*>(sourceObject);
        CGameSprite* target = static_cast<CGameSprite*>(targetObject);
        auto saved = source->m_derivedStats.m_onHitSpells;
        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = queue.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"combatonhit-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        source->m_derivedStats.m_onHitSpells.clear();
        source->ApplyOnHitSpells(target);
        check("empty-list", queue.GetCount() == before);
        for (INT index = 0; index < 2; ++index) {
            COnHitSpell entry;
            entry.m_sourceRes = index == 0 ? "ZRCORIG1" : "ZRCORIG2";
            entry.m_spellRes = "ZRCHIT";
            entry.m_level = 1;
            entry.m_target = CPoint(123 + index, 456 + index);
            source->m_derivedStats.m_onHitSpells.push_back(entry);
        }
        source->ApplyOnHitSpells(target);
        check("four-effects", queue.GetCount() == before + 4);
        {
            CDerivedStats copied;
            copied = source->m_derivedStats;
            check("copy-list", copied.m_onHitSpells.size() == 2
                && copied.m_onHitSpells.front().m_sourceRes == CResRef("ZRCORIG1")
                && copied.m_onHitSpells.back().m_target == CPoint(124, 457));
            copied.m_activeEffectOpcodes.insert(CGAMEEFFECT_SLEEP);
            copied.Reload(NULL, &source->m_baseStats, NULL, NULL);
            check("reload-clears-list", copied.m_onHitSpells.empty());
            check("reload-clears-opcodes", copied.m_activeEffectOpcodes.empty());
        }
        INT count = queue.GetCount() - before;
        for (INT index = count - 1; index >= 0; --index) {
            CMessage* rawMessage = queue.RemoveTail();
            CMessageAddEffect* message = dynamic_cast<CMessageAddEffect*>(rawMessage);
            check("message-type", message != NULL);
            if (message != NULL) {
                CGameEffect* effect = message->m_effect;
                INT entry = index / 2, effectIndex = index % 2;
                check("recipient", message->GetTarget() == (effectIndex == 0 ? sourceId : targetId));
                check("caller", message->GetSource() == sourceId);
                check("source-resource", effect->m_sourceRes == CResRef(entry == 0 ? "ZRCORIG1" : "ZRCORIG2"));
                check("target-point", effect->m_target == CPoint(123 + entry, 456 + entry));
                check("source-point", effect->m_source == source->GetPos() && effect->m_sourceID == sourceId);
                check("payload", effect->m_effectAmount == effectIndex + 1 && effect->m_targetType == effectIndex + 1);
            }
            delete rawMessage;
        }
        source->m_derivedStats.m_onHitSpells = saved;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            Fail(nStep, "combatonhit", "message mismatch (state restored, effects discarded)");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"combatonhit\",\"checks\":%d,\"restored\":true}", nStep, checks);
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
            // The depletion helper owns and deletes the exhausted heap item.
            CItem* exhausted = new CItem(CResRef(ammoNames[kind]), 0, 0, 0, 0, 0);
            exhausted->SetUsageCount(0, 0);
            source->m_equipment.m_items[14] = exhausted;
            high.SetUsageCount(0, 7);
            source->m_nWeaponSet = 0;
            source->m_equipment.m_selectedWeapon = 14;
            source->m_equipment.m_selectedWeaponAbility = 0;
            BOOL depleted = source->CheckItemDepleted(14, 0);
            BOOL replaced = !depleted && source->m_equipment.m_items[14] == NULL
                && source->m_equipment.m_selectedWeapon == 13 && high.GetUsageCount(0) == 7;
            Emit("{\"step\":%d,\"op\":\"combatammo-depleted\",\"launcher\":\"%s\","
                 "\"result\":%d,\"slot\":%d,\"passed\":%d}",
                nStep, launcherNames[kind], depleted, source->m_equipment.m_selectedWeapon, replaced);
            valid = valid && replaced;
            ++checks;
            if (source->m_equipment.m_items[14] != NULL) {
                delete source->m_equipment.m_items[14];
                source->m_equipment.m_items[14] = NULL;
            }
            auto checkDepletion = [&](const char* name, SHORT slot, SHORT ability, BOOL expected) {
                BOOL actual = source->CheckItemDepleted(slot, ability);
                if (strcmp(name, "missing ability") == 0) {
                    auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
                    auto* message = messages.IsEmpty() ? NULL : dynamic_cast<CMessageSetTrigger*>(messages.GetTail());
                    valid = valid && message != NULL && message->m_trigger.m_string1 == ammoNames[kind]
                        && message->m_trigger.m_string2.IsEmpty();
                }
                Emit("{\"step\":%d,\"op\":\"combatammo-depletion-case\",\"case\":\"%s\","
                     "\"actual\":%d,\"expected\":%d}", nStep, name, actual, expected);
                valid = valid && actual == expected;
                ++checks;
            };
            checkDepletion("remaining charges", 13, 0, FALSE);
            checkDepletion("invalid slot", -1, 0, TRUE);
            checkDepletion("empty slot", 14, 0, TRUE);
            checkDepletion("missing ability", 13, 99, TRUE);
            checkDepletion("unlimited melee ability", 45, 0, FALSE);
            source->m_equipment.m_selectedWeapon = 10;
            source->m_equipment.m_selectedWeaponAbility = 0;
            SHORT ranged = source->EquipRangedWeapon();
            BOOL rangedPassed = ranged == CGameAIBase::ACTION_DONE
                && source->m_nWeaponSet == 0 && source->m_equipment.m_selectedWeapon == 13;
            Emit("{\"step\":%d,\"op\":\"combatammo-ranged\",\"launcher\":\"%s\","
                 "\"result\":%d,\"slot\":%d,\"passed\":%d}",
                nStep, launcherNames[kind], ranged, source->m_equipment.m_selectedWeapon, rangedPassed);
            valid = valid && rangedPassed;
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

    if (strcmp(step.op, "depletionreference") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() == 0) {
            Fail(nStep, step.op, "requires world and party");
            return;
        }
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (game->GetObjectArray()->GetDeny(id, CGameObjectArray::THREAD_ASYNCH,
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
        BYTE savedEA = source->m_typeAI.m_nEnemyAlly;
        CButtonData savedButtons[8];
        BYTE savedAmmo[8];
        for (int i = 0; i < 8; ++i) savedButtons[i] = source->m_quickWeapons[i];
        memcpy(savedAmmo, source->field_3D3A, sizeof(savedAmmo));
        BOOL valid = source->m_curAction.m_actionID == CAIAction::NO_ACTION
            && source->m_queuedActions.IsEmpty() && savedEA == CAIObjectType::EA_PC;
        const char* ammoNames[] = { "ZRCARROW", "ZRCBOLT", "ZRCSTONE" };
        const char* alternateNames[] = { "ZRCARR2", "ZRCBLT2", "ZRCSTN2" };
        const char* launcherNames[] = { "ZRCBOW", "ZRCXBOW", "ZRCSLING" };
        const char* cases[] = { "pc-empty", "pc-same", "pc-different", "ally-empty", "pc-wrapped" };
        int checks = 0;
        for (int kind = 0; kind < 3 && valid; ++kind) {
            for (int trial = 0; trial < 5 && valid; ++trial) {
                CItem launcher(CResRef(launcherNames[kind]), 1, 0, 0, 0, 0);
                CItem melee(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
                CItem weaker(CResRef("ZRCDAGGR"), 1, 0, 0, 0, 0);
                CItem reserve(CResRef(trial == 2 ? alternateNames[kind] : ammoNames[kind]), 7, 0, 0, 0, 0);
                CItem* exhausted = new CItem(CResRef(ammoNames[kind]), 1, 0, 0, 0, 0);
                exhausted->SetUsageCount(0, trial == 4 ? 0xFFFF : 0);
                BOOL hasReserve = trial == 1 || trial == 2 || trial == 4;
                memset(source->m_equipment.m_items, 0, sizeof(savedItems));
                for (int i = 0; i < 8; ++i) {
                    source->m_quickWeapons[i] = CButtonData();
                    source->m_quickWeapons[i].m_abilityId.m_itemNum = static_cast<SHORT>(43 + i);
                    source->m_quickWeapons[i].m_abilityId.m_abilityNum = 0;
                }
                memset(source->field_3D3A, 0, sizeof(savedAmmo));
                source->m_equipment.m_items[10] = savedItems[10];
                source->m_equipment.m_items[11] = exhausted;
                source->m_equipment.m_items[12] = hasReserve ? &reserve : NULL;
                source->m_equipment.m_items[43] = &launcher;
                source->m_equipment.m_items[45] = &melee;
                source->m_equipment.m_items[47] = &weaker;
                source->m_nWeaponSet = 0;
                source->m_equipment.m_selectedWeapon = 11;
                source->m_equipment.m_selectedWeaponAbility = 0;
                source->m_typeAI.m_nEnemyAlly = trial == 3 ? CAIObjectType::EA_ALLY : CAIObjectType::EA_PC;
                auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
                INT oldCount = messages.GetCount();
                BOOL result = source->CheckItemDepleted(11, 0);
                INT triggerCount = 0;
                SHORT triggerId = 0;
                CString string1, string2;
                BOOL triggerActors = TRUE;
                POSITION position = messages.GetTailPosition();
                for (INT count = messages.GetCount() - oldCount; count > 0 && position != NULL; --count) {
                    auto* message = dynamic_cast<CMessageSetTrigger*>(messages.GetPrev(position));
                    if (message != NULL) {
                        ++triggerCount;
                        triggerId = message->m_trigger.m_triggerID;
                        string1 = message->m_trigger.m_string1;
                        string2 = message->m_trigger.m_string2;
                        triggerActors = triggerActors && message->GetSource() == id && message->GetTarget() == id;
                    }
                }
                INT slot = source->m_equipment.m_selectedWeapon;
                BOOL deleted = source->m_equipment.m_items[11] == NULL;
                BOOL sameResource = trial == 1 || trial == 4;
                valid = deleted && result == !sameResource
                    && slot == (hasReserve ? 12 : (trial == 3 ? 45 : 10))
                    && source->m_nWeaponSet == (trial == 3 ? 1 : 0)
                    && source->m_equipment.m_selectedWeaponAbility == 0
                    && (!hasReserve || reserve.GetUsageCount(0) == 7)
                    && triggerCount == (sameResource ? 0 : 1) && triggerActors
                    && triggerId == (sameResource ? 0 : CAITrigger::UNUSABLE)
                    && string1 == (sameResource ? "" : ammoNames[kind]) && string2.IsEmpty();
                Emit("{\"step\":%d,\"op\":\"depletion-reference\",\"item\":\"%s\",\"case\":\"%s\","
                     "\"returned\":%d,\"deleted\":%d,\"slot\":%d,\"set\":%d,\"ability\":%d,\"reserve\":%d,"
                     "\"triggers\":%d,\"triggerId\":%d,\"string1\":\"%s\",\"string2\":\"%s\",\"triggerActors\":%d}",
                    nStep, ammoNames[kind], cases[trial], result, deleted, slot, source->m_nWeaponSet,
                    source->m_equipment.m_selectedWeaponAbility, hasReserve ? reserve.GetUsageCount(0) : -1,
                    triggerCount, triggerId, (LPCTSTR)string1, (LPCTSTR)string2, triggerActors);
                ++checks;
                if (!deleted) delete source->m_equipment.m_items[11];
                // Restore real pointers before fixture destructors and before another engine tick.
                memcpy(source->m_equipment.m_items, savedItems, sizeof(savedItems));
                source->m_typeAI.m_nEnemyAlly = savedEA;
            }
        }
        memcpy(source->m_equipment.m_items, savedItems, sizeof(savedItems));
        source->m_typeAI.m_nEnemyAlly = savedEA;
        source->m_nWeaponSet = savedSet;
        source->m_equipment.m_selectedWeapon = 255;
        source->SelectWeaponAbility(savedSlot, static_cast<BYTE>(savedAbility), FALSE, TRUE);
        source->m_baseStats.m_numberOfAttacksBase = savedAttacks;
        for (int i = 0; i < 8; ++i) source->m_quickWeapons[i] = savedButtons[i];
        memcpy(source->field_3D3A, savedAmmo, sizeof(savedAmmo));
        game->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid || checks != 15) {
            Fail(nStep, step.op, "depletion transition mismatch; equipment and classification restored");
            return;
        }
        Emit("{\"step\":%d,\"op\":\"depletionreference\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "rangedready") == 0 || strcmp(step.op, "rangedrestore") == 0
        || strcmp(step.op, "depletewait") == 0) {
        static LONG owner = -1;
        static CItem* savedItems[51];
        static CItem* ammo = NULL;
        static CItem* launcher = NULL;
        static CItem* spare = NULL;
        static CItem* melee = NULL;
        static CItem* weaker = NULL;
        static BYTE savedEnemyAlly;
        static BOOL npcBranchArmed = FALSE;
        static INT mode = 0;
        static BYTE selected, weaponSet, attacks, savedAmmo[8];
        static WORD ability;
        static CButtonData buttons[8];
        BOOL restore = strcmp(step.op, "rangedrestore") == 0;
        BOOL depletedWait = strcmp(step.op, "depletewait") == 0;
        const char* ammoNames[] = { "ZRCARROW", "ZRCBOLT", "ZRCSTONE" };
        const char* launcherNames[] = { "ZRCBOW", "ZRCXBOW", "ZRCSLING" };
        INT kind = step.nCount >= 1 ? step.n[0] : -1;
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || (restore || depletedWait ? owner != id : (owner != -1 || kind < 0 || kind > 2))
            || game->GetObjectArray()->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "invalid fixture state or actor unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(object);
        BOOL valid = TRUE;
        if (depletedWait) {
            BOOL empty = source->m_equipment.m_items[11] == NULL;
            // Keep the UI order a real party click, then exercise IsPC's non-PC
            // branch during the pending shot. This is not a full NPC AI test.
            if (mode == 3 && !npcBranchArmed && !empty) {
                savedEnemyAlly = source->m_typeAI.m_nEnemyAlly;
                source->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_ALLY;
                npcBranchArmed = TRUE;
            }
            if (!empty && ++s_waited < 600) {
                game->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                return;
            }
            valid = empty && ((mode == 1 || mode == 4) ? source->m_equipment.m_selectedWeapon == 10
                : mode == 3 ? npcBranchArmed && source->m_equipment.m_selectedWeapon == 45
                    && source->m_nWeaponSet == 1 && source->m_equipment.m_items[45] == melee
                : mode == 2 && source->m_equipment.m_selectedWeapon == 12
                    && source->m_equipment.m_items[12] == spare && spare->GetUsageCount(0) == 1);
            Emit("{\"step\":%d,\"op\":\"depletewait\",\"mode\":%d,\"empty\":%d,\"selected\":%u,\"action\":%d,\"valid\":%d}",
                nStep, mode, empty, source->m_equipment.m_selectedWeapon, source->m_curAction.m_actionID, valid);
            if (npcBranchArmed) {
                source->m_typeAI.m_nEnemyAlly = savedEnemyAlly;
                npcBranchArmed = FALSE;
                Emit("{\"step\":%d,\"op\":\"npc-classification-restored\",\"enemyAlly\":%u}", nStep, source->m_typeAI.m_nEnemyAlly);
            }
            source->ClearActions(FALSE);
            source->SetCurrAction(CAIAction::NULL_ACTION);
            source->m_attackFrame = -2;
            game->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (!valid) { Fail(nStep, step.op, "last-ammo transition failed"); return; }
            s_waited = 0;
            s_pc++;
            return;
        }
        if (!restore) {
            mode = step.nCount >= 2 ? step.n[1] : 0;
            valid = source->m_curProjectile == NULL && source->m_curAction.m_actionID == 0;
            if (valid) {
                ammo = new CItem(CResRef(ammoNames[kind]), mode == 0 ? 40 : 1, 0, 0, 0, 0);
                launcher = new CItem(CResRef(launcherNames[kind]), 1, 0, 0, 0, 0);
                BOOL ammoLoaded = ammo->Demand();
                BOOL launcherLoaded = launcher->Demand();
                valid = ammoLoaded && launcherLoaded && ammo->GetAbility(0) != NULL && launcher->GetAbility(0) != NULL;
                if (ammoLoaded) ammo->Release();
                if (launcherLoaded) launcher->Release();
                if (valid) {
                    owner = id;
                    memcpy(savedItems, source->m_equipment.m_items, sizeof(savedItems));
                    selected = source->m_equipment.m_selectedWeapon;
                    ability = source->m_equipment.m_selectedWeaponAbility;
                    weaponSet = source->m_nWeaponSet;
                    attacks = source->m_baseStats.m_numberOfAttacksBase;
                    memcpy(savedAmmo, source->field_3D3A, sizeof(savedAmmo));
                    for (INT i = 0; i < 8; ++i) buttons[i] = source->m_quickWeapons[i];
                    // Remove the original weapon's equipped effects through the engine.
                    // The fist remains in place while the fixture slots are swapped.
                    source->SelectWeaponAbility(10, 0, FALSE, TRUE);
                    // Retain the real items; only fixture pointers occupy weapon/ammo slots.
                    for (INT i = 11; i < 15; ++i) source->m_equipment.m_items[i] = NULL;
                    for (INT i = 43; i < 51; ++i) source->m_equipment.m_items[i] = NULL;
                    source->m_equipment.m_items[11] = ammo;
                    if (mode == 2) {
                        spare = new CItem(CResRef(ammoNames[kind]), 1, 0, 0, 0, 0);
                        source->m_equipment.m_items[12] = spare;
                    }
                    if (mode == 3 || mode == 4) {
                        melee = new CItem(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
                        weaker = new CItem(CResRef("ZRCDAGGR"), 1, 0, 0, 0, 0);
                        source->m_equipment.m_items[45] = melee;
                        source->m_equipment.m_items[47] = weaker;
                        source->m_quickWeapons[2].m_abilityId.m_itemNum = 45;
                        source->m_quickWeapons[2].m_abilityId.m_abilityNum = 0;
                        source->m_quickWeapons[4].m_abilityId.m_itemNum = 47;
                        source->m_quickWeapons[4].m_abilityId.m_abilityNum = 0;
                    }
                    source->m_equipment.m_items[43] = launcher;
                    source->m_nWeaponSet = 0;
                    valid = source->SelectLauncherAmmo(launcher, FALSE) == CGameAIBase::ACTION_DONE
                        && source->m_equipment.m_selectedWeapon == 11 && ammo->GetUsageCount(0) == (mode == 0 ? 40 : 1);
                    source->m_attackFrame = -2;
                    Emit("{\"step\":%d,\"op\":\"rangedready\",\"ammo\":\"%s\",\"launcher\":\"%s\",\"count\":%u,\"slot\":%u}",
                        nStep, ammoNames[kind], launcherNames[kind], ammo->GetUsageCount(0), source->m_equipment.m_selectedWeapon);
                } else {
                    delete ammo; ammo = NULL;
                    delete launcher; launcher = NULL;
                }
            }
        } else {
            // attackwait has already stopped the order. Verify the tested ammo was spent.
            valid = source->m_curAction.m_actionID == 0;
            BOOL ammoPresent = source->m_equipment.m_items[11] == ammo;
            BOOL sparePresent = spare != NULL && source->m_equipment.m_items[12] == spare;
            WORD remaining = ammoPresent ? ammo->GetUsageCount(0) : 0;
            valid = valid && (mode == 0 ? remaining > 0 && remaining < 40 : source->m_equipment.m_items[11] == NULL);
            source->ClearActions(FALSE);
            source->SetCurrAction(CAIAction::NULL_ACTION);
            delete source->m_curProjectile;
            source->m_curProjectile = NULL;
            source->SelectWeaponAbility(10, 0, FALSE, TRUE);
            memcpy(source->m_equipment.m_items, savedItems, sizeof(savedItems));
            source->m_nWeaponSet = weaponSet;
            source->SelectWeaponAbility(selected, static_cast<BYTE>(ability), FALSE, TRUE);
            source->m_baseStats.m_numberOfAttacksBase = attacks;
            memcpy(source->field_3D3A, savedAmmo, sizeof(savedAmmo));
            for (INT i = 0; i < 8; ++i) source->m_quickWeapons[i] = buttons[i];
            // A depletion regression may have deleted ammo; never delete it twice.
            if (ammoPresent) delete ammo;
            if (sparePresent) delete spare;
            spare = NULL;
            delete melee;
            delete weaker;
            melee = NULL;
            weaker = NULL;
            delete launcher;
            ammo = NULL;
            launcher = NULL;
            owner = -1;
            valid = valid && source->m_equipment.m_selectedWeapon == selected
                && source->m_equipment.m_selectedWeaponAbility == ability
                && memcmp(source->m_equipment.m_items, savedItems, sizeof(savedItems)) == 0;
            Emit("{\"step\":%d,\"op\":\"rangedrestore\",\"remaining\":%u,\"restored\":true}", nStep, remaining);
        }
        game->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) { Fail(nStep, step.op, "ranged fixture selection or consumption failed"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "attackplace") == 0 && step.nCount == 2) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        CGameObject* object = NULL;
        LONG id = game->GetFixedOrderCharacterId(0);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || game->GetObjectArray()->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(object);
        source->ClearActions(FALSE);
        source->SetCurrAction(CAIAction::NULL_ACTION);
        source->DropPath();
        source->DropSearchRequest();
        source->JumpToPoint(CPoint(step.n[0], step.n[1]), TRUE);
        Emit("{\"step\":%d,\"op\":\"attackplace\",\"x\":%ld,\"y\":%ld,\"encumbrance\":%d,\"space\":%u}",
            nStep, source->GetPos().x, source->GetPos().y, source->m_derivedStats.m_nEncumberance,
            source->m_animation.GetPersonalSpace());
        game->GetObjectArray()->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "groupdamage") == 0) {
        static BOOL running = FALSE;
        static LONG sourceId, targetId;
        static BYTE enemyAlly;
        static SHORT hpBefore;
        static BOOL attackSeen, repeatSeen;
        static CAIObjectType expected;
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        CGameObject* raw = NULL;
        CGameObject* other = NULL;
        if (!running) {
            sourceId = game->GetFixedOrderCharacterId(0);
            targetId = game->GetFixedOrderCharacterId(1);
        }
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "source unavailable");
            return;
        }
        if (objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &other, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        auto* target = static_cast<CGameSprite*>(other);
        if (!running) {
            BOOL ready = source->m_curAction.m_actionID == CAIAction::NO_ACTION
                && source->m_queuedActions.IsEmpty() && source->m_curProjectile == NULL
                && target->m_curAction.m_actionID == CAIAction::NO_ACTION
                && target->m_queuedActions.IsEmpty() && target->m_baseStats.m_hitPoints > 60
                && source->m_pArea != NULL && source->m_pArea == target->m_pArea
                && source->m_typeAI.m_nEnemyAlly == CAIObjectType::EA_PC;
            if (!ready) {
                objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                Fail(nStep, step.op, "requires idle party actors in one area and sufficient HP");
                return;
            }
            enemyAlly = target->m_typeAI.m_nEnemyAlly;
            hpBefore = target->m_baseStats.m_hitPoints;
            target->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
            expected.Set(target->GetAIType());
            expected.SetName(CString(_T("")));
            expected.SetInstance(-1);
            attackSeen = repeatSeen = FALSE;
            running = TRUE;
            s_waited = 0;
            source->SetCurrAction(CAIAction(CAIAction::GROUPATTACK, CAIObjectType::ANYONE, 0, 0, 0));
            Emit("{\"step\":%d,\"op\":\"groupdamage-start\",\"source\":%ld,\"target\":%ld,\"hp\":%d,\"slot\":%u}",
                nStep, sourceId, targetId, hpBefore, source->m_equipment.m_selectedWeapon);
        } else {
            attackSeen = attackSeen || (source->m_curAction.m_actionID == CAIAction::ATTACKNOSOUND
                && source->m_curAction.m_acteeID.Equal(expected));
            POSITION pos = source->m_queuedActions.GetHeadPosition();
            while (pos != NULL) {
                CAIAction* queued = source->m_queuedActions.GetNext(pos);
                repeatSeen = repeatSeen || (queued->m_actionID == CAIAction::GROUPATTACK
                    && queued->m_acteeID.Equal(expected));
            }
            // Reject competing party attacks rather than crediting their damage
            // to the actor whose GroupAttack order is under test.
            BOOL competingAttack = FALSE;
            for (INT i = 1; i < game->GetNumCharacters(); ++i) {
                CGameObject* member = NULL;
                LONG memberId = game->GetFixedOrderCharacterId(i);
                if (objects->GetShare(memberId, CGameObjectArray::THREAD_ASYNCH, &member, INFINITE) == CGameObjectArray::SUCCESS) {
                    SHORT actionId = static_cast<CGameSprite*>(member)->m_curAction.m_actionID;
                    competingAttack = competingAttack || actionId == CAIAction::ATTACK
                        || actionId == CAIAction::ATTACKNOSOUND || actionId == CAIAction::ATTACKONEROUND;
                    objects->ReleaseShare(memberId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }
            }
            SHORT hp = target->m_baseStats.m_hitPoints;
            BOOL success = hp < hpBefore && attackSeen && repeatSeen && !competingAttack;
            BOOL finished = success || competingAttack || ++s_waited >= 600;
            if (s_waited % 30 == 0 || finished) {
                Emit("{\"step\":%d,\"op\":\"groupdamage-wait\",\"ticks\":%d,\"action\":%d,\"frame\":%d,\"hpBefore\":%d,\"hpAfter\":%d,\"attackSeen\":%d,\"repeatSeen\":%d,\"competingAttack\":%d}",
                    nStep, s_waited, source->m_curAction.m_actionID, source->m_attackFrame,
                    hpBefore, hp, attackSeen, repeatSeen, competingAttack);
            }
            if (finished) {
                source->ClearActions(FALSE);
                source->SetCurrAction(CAIAction::NULL_ACTION);
                source->DropPath();
                source->DropSearchRequest();
                source->m_attackFrame = -2;
                delete source->m_curProjectile;
                source->m_curProjectile = NULL;
                target->m_typeAI.m_nEnemyAlly = enemyAlly;
                running = FALSE;
                s_waited = 0;
                Emit("{\"step\":%d,\"op\":\"groupdamage-restored\",\"enemyAlly\":%u,\"success\":%d}", nStep, enemyAlly, success);
                objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                if (!success) { Fail(nStep, step.op, "group order did not produce isolated damage"); return; }
                ++s_pc;
                return;
            }
        }
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        return;
    }

    if (strcmp(step.op, "combatgroup") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        CGameObject* raw = NULL;
        CGameObject* other = NULL;
        LONG id = game->GetFixedOrderCharacterId(0);
        LONG targetId = game->GetFixedOrderCharacterId(1);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        if (objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &other, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        auto* target = static_cast<CGameSprite*>(other);
        CAIAction action;
        action = source->m_curAction;
        auto interrupt = source->m_interrupt;
        auto lastReturn = source->m_nLastActionReturn;
        auto savedTargetId = source->m_targetId;
        auto sourceMarker = source->m_marker;
        auto targetMarker = target->m_marker;
        auto selected = source->m_bSelected;
        auto actionCount = source->m_actionCount;
        auto userPause = source->m_userCommandPause;
        auto unselectable = source->m_nUnselectableCounter;
        auto sourceEA = source->m_typeAI.m_nEnemyAlly;
        auto targetType = target->m_typeAI;
        auto active = target->m_active;
        auto activeAI = target->m_activeAI;
        auto activeImprisonment = target->m_activeImprisonment;
        auto& queue = source->m_queuedActions;
        INT before = queue.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"combatgroup-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        // Isolate marker changes from any target already present in the save.
        source->m_targetId = CGameObjectArray::INVALID_INDEX;
        if (step.nCount > 0 && step.n[0] == 2) {
            check("target-orderable-precondition", source->Orderable(FALSE));
            source->m_bSelected = TRUE;
            source->m_actionCount = 0;
            source->m_userCommandPause = CTimerWorld::TIMESCALE_MSEC_PER_SEC * 5 - 2;
            target->m_marker.m_type = CMarker::ELLIPSE;
            target->m_marker.m_nRecticleCounter = 0;
            target->m_marker.m_nRecticleForceRenderTarget = 0;
            source->UpdateTarget(NULL);
            check("target-empty-clear", source->m_targetId == CGameObjectArray::INVALID_INDEX);
            source->UpdateTarget(target);
            check("target-select-threshold", source->m_targetId == targetId
                && target->m_marker.m_type == CMarker::RECTICLE
                && target->m_marker.m_nRecticleCounter == 1
                && target->m_marker.m_nRecticleForceRenderTarget == 0);
            ++source->m_userCommandPause;
            source->UpdateTarget(target);
            check("target-duplicate-no-op", target->m_marker.m_nRecticleCounter == 1
                && target->m_marker.m_nRecticleForceRenderTarget == 0);
            source->UpdateTarget(NULL);
            check("target-clear", source->m_targetId == CGameObjectArray::INVALID_INDEX
                && target->m_marker.m_type == CMarker::ELLIPSE && target->m_marker.m_nRecticleCounter == 0);
            source->UpdateTarget(target);
            check("target-force-render", target->m_marker.m_nRecticleForceRenderTarget == CTimerWorld::TIMESCALE_MSEC_PER_SEC);
            source->m_marker.m_type = CMarker::ELLIPSE;
            source->m_marker.m_nRecticleCounter = 0;
            source->UpdateTarget(source);
            check("target-switch", source->m_targetId == id && source->m_marker.m_nRecticleCounter == 1
                && target->m_marker.m_nRecticleCounter == 0 && target->m_marker.m_type == CMarker::ELLIPSE);
            source->UpdateTarget(NULL);
            target->m_marker.m_nRecticleForceRenderTarget = 0;
            source->m_actionCount = 1;
            source->UpdateTarget(target);
            check("target-action-count", target->m_marker.m_nRecticleCounter == 1
                && target->m_marker.m_nRecticleForceRenderTarget == 0);
            target->m_marker.SetType(CMarker::RECTICLE);
            source->UpdateTarget(NULL);
            check("target-shared-marker", target->m_marker.m_nRecticleCounter == 1
                && target->m_marker.m_type == CMarker::RECTICLE);
            source->m_bSelected = FALSE;
            source->UpdateTarget(target);
            check("target-unselected", source->m_targetId == targetId && target->m_marker.m_nRecticleCounter == 1);
            source->UpdateTarget(NULL);
            check("target-unselected-clear", source->m_targetId == CGameObjectArray::INVALID_INDEX
                && target->m_marker.m_nRecticleCounter == 1);
            source->m_bSelected = TRUE;
            source->m_nUnselectableCounter = 1;
            source->UpdateTarget(target);
            check("target-unorderable", source->m_targetId == targetId && target->m_marker.m_nRecticleCounter == 1);
            source->UpdateTarget(NULL);
            check("target-unorderable-clear", source->m_targetId == CGameObjectArray::INVALID_INDEX
                && target->m_marker.m_nRecticleCounter == 1);
            source->m_nUnselectableCounter = unselectable;
            // Low-word bit 15 is rejected by GetDeny, without referring to a live object.
            source->m_targetId = 0x8000;
            source->UpdateTarget(target);
            check("target-missing-old-preserved", source->m_targetId == 0x8000
                && target->m_marker.m_nRecticleCounter == 1);
            source->UpdateTarget(NULL);
            check("target-missing-old-clear-preserved", source->m_targetId == 0x8000);
            source->m_targetId = CGameObjectArray::INVALID_INDEX;
            source->m_marker = sourceMarker;
            target->m_marker = targetMarker;
            source->m_bSelected = selected;
            source->m_actionCount = actionCount;
            source->m_userCommandPause = userPause;
        }
        // Deliberately distinguish the target from the attacker even if both
        // party members have the same classification in the loaded save.
        target->m_typeAI.m_nSpecific = source->GetAIType().m_nSpecific ^ 1;
        target->m_typeAI.SetName(CString(_T("ZRCTARGET")));
        target->m_typeAI.SetInstance(targetId);
        CAIObjectType expected(target->GetAIType());
        expected.SetName(CString(_T("")));
        expected.SetInstance(-1);
        source->m_curAction = CAIAction(CAIAction::GROUPATTACK, source->GetAIType(), 7, 8, 9);
        auto originalTarget = source->m_curAction.m_acteeID;
        target->m_active = target->m_activeAI = target->m_activeImprisonment = TRUE;
        source->m_interrupt = FALSE;
        check("null-target", source->GroupAttack(NULL) == CGameAIBase::ACTION_DONE);
        target->m_active = FALSE;
        check("inactive-target", source->GroupAttack(target) == CGameAIBase::ACTION_DONE);
        target->m_active = TRUE;
        target->m_activeAI = FALSE;
        check("inactive-ai", source->GroupAttack(target) == CGameAIBase::ACTION_DONE);
        target->m_activeAI = TRUE;
        target->m_activeImprisonment = FALSE;
        check("imprisoned-target", source->GroupAttack(target) == CGameAIBase::ACTION_DONE);
        target->m_activeImprisonment = TRUE;
        source->m_interrupt = TRUE;
        check("interrupted", source->GroupAttack(target) == CGameAIBase::ACTION_INTERRUPTABLE);
        check("null-before-interrupt", source->GroupAttack(NULL) == CGameAIBase::ACTION_DONE);
        check("guards-preserve-order", queue.GetCount() == before
            && source->m_curAction.m_acteeID.Equal(originalTarget));
        source->m_interrupt = FALSE;
        check("active-target", source->GroupAttack(target) == CGameAIBase::ACTION_DONE);
        check("current-target", source->m_curAction.m_acteeID.Equal(expected)
            && source->m_curAction.m_acteeID.GetName().IsEmpty());
        check("two-actions", queue.GetCount() == before + 2);
        if (queue.GetCount() == before + 2) {
            CAIAction* attack = queue.RemoveHead();
            CAIAction* repeat = queue.RemoveHead();
            check("attack-target", attack->m_actionID == CAIAction::ATTACKNOSOUND
                && attack->m_acteeID.Equal(expected) && attack->m_acteeID.GetName().IsEmpty());
            check("attack-specifics", attack->m_specificID == 0
                && attack->m_specificID2 == 0 && attack->m_specificID3 == 0);
            check("repeat-target", repeat->m_actionID == CAIAction::GROUPATTACK
                && repeat->m_acteeID.Equal(expected) && repeat->m_acteeID.GetName().IsEmpty());
            check("repeat-specifics", repeat->m_specificID == 7
                && repeat->m_specificID2 == 8 && repeat->m_specificID3 == 9);
            delete attack;
            delete repeat;
        }
        while (queue.GetCount() > before) delete queue.RemoveHead();
        if (step.nCount > 0 && step.n[0] != 0) {
            source->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_PC;
            target->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
            expected.Set(target->GetAIType());
            expected.SetName(CString(_T("")));
            expected.SetInstance(-1);
            // A self selector deliberately contradicts the enemy search.
            source->m_curAction = CAIAction(CAIAction::GROUPATTACK, source->GetAIType(), 7, 8, 9);
            auto* area = source->m_pArea;
            source->m_pArea = NULL;
            source->m_nLastActionReturn = 123;
            SHORT result = source->ExecuteAction();
            source->m_pArea = area;
            check("dispatch-no-target", result == CGameAIBase::ACTION_DONE
                && source->m_nLastActionReturn == result && queue.GetCount() == before);
            source->m_interrupt = TRUE;
            source->m_nLastActionReturn = 123;
            result = source->ExecuteAction();
            check("dispatch-interrupted", result == CGameAIBase::ACTION_INTERRUPTABLE
                && source->m_nLastActionReturn == result && queue.GetCount() == before);
            source->m_interrupt = FALSE;
            source->m_nLastActionReturn = 123;
            result = source->ExecuteAction();
            check("dispatch-active", result == CGameAIBase::ACTION_DONE
                && source->m_nLastActionReturn == result);
            check("dispatch-enemy-selector", source->m_curAction.m_acteeID.Equal(expected)
                && source->m_curAction.m_acteeID.GetName().IsEmpty());
            check("dispatch-two-actions", queue.GetCount() == before + 2);
            if (queue.GetCount() == before + 2) {
                CAIAction* attack = queue.RemoveHead();
                CAIAction* repeat = queue.RemoveHead();
                check("dispatch-silent-attack", attack->m_actionID == CAIAction::ATTACKNOSOUND
                    && attack->m_acteeID.Equal(expected) && attack->m_acteeID.GetName().IsEmpty());
                check("dispatch-repeat", repeat->m_actionID == CAIAction::GROUPATTACK
                    && repeat->m_acteeID.Equal(expected) && repeat->m_specificID == 7
                    && repeat->m_specificID2 == 8 && repeat->m_specificID3 == 9);
                delete attack;
                delete repeat;
            }
            while (queue.GetCount() > before) delete queue.RemoveHead();
        }
        source->m_curAction = action;
        source->m_interrupt = interrupt;
        source->m_nLastActionReturn = lastReturn;
        source->m_targetId = savedTargetId;
        source->m_marker = sourceMarker;
        target->m_marker = targetMarker;
        source->m_typeAI.m_nEnemyAlly = sourceEA;
        target->m_typeAI = targetType;
        target->m_active = active;
        target->m_activeAI = activeAI;
        target->m_activeImprisonment = activeImprisonment;
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) { Fail(nStep, step.op, "GroupAttack mismatch (state restored)"); return; }
        Emit("{\"step\":%d,\"op\":\"combatgroup\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "runawaymove") == 0) {
        static BOOL running = FALSE;
        static CAIAction savedAction;
        static CTypedPtrList<CPtrList, CAIAction*> savedQueue;
        static CPoint start;
        static CPoint threat;
        static CPoint savedDest;
        static SHORT savedCount;
        static SHORT savedActionCount;
        static SHORT savedReturn;
        static SHORT savedSequence;
        static BOOL savedInterrupt;
        static INT elapsed;
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        LONG targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* raw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and party actor");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        if (!running) {
            CGameObject* target = NULL;
            if (source->m_pPath != NULL || source->m_currentSearchRequest != NULL
                || objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &target, INFINITE) != CGameObjectArray::SUCCESS) {
                objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                Fail(nStep, step.op, "requires idle actor and available target");
                return;
            }
            BOOL sameArea = source->m_pArea != NULL && source->m_pArea == target->m_pArea;
            threat = target->GetPos();
            objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (!sameArea) {
                objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                Fail(nStep, step.op, "requires same area");
                return;
            }
            savedAction = source->m_curAction;
            savedCount = source->field_5614;
            savedActionCount = source->m_actionCount;
            savedReturn = source->m_nLastActionReturn;
            savedSequence = source->m_nSequence;
            savedInterrupt = source->m_interrupt;
            savedDest = source->m_curDest;
            while (!source->m_queuedActions.IsEmpty()) savedQueue.AddTail(source->m_queuedActions.RemoveHead());
            start = source->m_pos;
            CAIObjectType selector;
            selector.m_nInstance = targetId;
            source->m_curAction = CAIAction(CAIAction::RUNAWAYFROMNOINTERRUPT, selector, 1000, 0, 0);
            source->field_5614 = 14;
            source->m_actionCount = 0;
            source->m_interrupt = FALSE;
            SHORT result = source->ExecuteAction();
            Emit("{\"step\":%d,\"op\":\"runawaymove-start\",\"returned\":%d,\"x\":%ld,\"y\":%ld,\"destX\":%ld,\"destY\":%ld,\"request\":%s}",
                nStep, result, start.x, start.y, source->m_curDest.x, source->m_curDest.y,
                source->m_currentSearchRequest != NULL ? "true" : "false");
            running = TRUE;
            elapsed = 0;
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return;
        }
        ++elapsed;
        CPoint finish = source->m_pos;
        LONG away = (finish.x - start.x) * (start.x - threat.x)
            + (finish.y - start.y) * (start.y - threat.y);
        BOOL moved = finish != start && away > 0;
        if (!moved && elapsed < 150) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return;
        }
        source->DropPath();
        source->DropSearchRequest();
        while (!source->m_queuedActions.IsEmpty()) delete source->m_queuedActions.RemoveHead();
        while (!savedQueue.IsEmpty()) source->m_queuedActions.AddTail(savedQueue.RemoveHead());
        source->m_curAction = savedAction;
        source->field_5614 = savedCount;
        source->m_actionCount = savedActionCount;
        source->m_nLastActionReturn = savedReturn;
        source->m_curDest = savedDest;
        source->m_interrupt = savedInterrupt;
        source->SetSequence(savedSequence);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        running = FALSE;
        Emit("{\"step\":%d,\"op\":\"runawaymove\",\"ticks\":%d,\"startX\":%ld,\"startY\":%ld,\"endX\":%ld,\"endY\":%ld,\"away\":%ld,\"valid\":%s}",
            nStep, elapsed, start.x, start.y, finish.x, finish.y, away, moved ? "true" : "false");
        if (!moved) { Fail(nStep, step.op, "actor did not flee (test action/path removed)"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "tickcounters") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "actor unavailable"); return;
        }
        auto* source = static_cast<CGameSprite*>(object);
        auto savedPoison = source->m_poisonSoundTimer;
        auto savedSequence = source->m_nSequence;
        auto savedPause = source->m_nCommandPause;
        auto savedUserPause = source->m_userCommandPause;
        auto savedActive = game->GetWorldTimer()->m_active;
        auto savedSpriteActive = source->m_active;
        auto savedAIActive = source->m_activeAI;
        auto savedImprisonment = source->m_activeImprisonment;
        auto savedSelected = source->m_bSelected;
        auto savedState = source->m_derivedStats.m_generalState;
        auto savedBaseState = source->m_baseStats.m_generalState;
        auto savedSpells = source->m_derivedStats.m_spellStates;
        auto savedImmunity = source->m_baseStats.field_2F0[3];
        auto savedFlags = source->field_3428;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        BOOL valid = TRUE;
        for (INT test = 0; test < 7; ++test) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = CAIAction::WAIT;
            source->m_curAction.m_specificID = 100;
            source->m_actionCount = 1;
            source->m_interrupt = FALSE;
            source->m_inCutScene = source->m_moraleFailure = FALSE;
            source->field_727E = FALSE;
            source->m_dialogWait = 0;
            source->m_bSelected = FALSE;
            source->m_baseStats.m_generalState = source->m_derivedStats.m_generalState = 0;
            source->m_derivedStats.m_spellStates.reset();
            source->m_baseStats.field_2F0[3] |= 1;
            source->field_3428 = 7;
            source->m_active = test != 6;
            source->m_activeAI = source->m_activeImprisonment = TRUE;
            game->GetWorldTimer()->m_active = test % 2 == 0;
            SHORT poison = test < 2 || test == 6 ? 5 : test < 4 ? 0 : -1;
            source->m_poisonSoundTimer = poison;
            source->m_userCommandPause = 0;
            source->m_nCommandPause = 5;
            source->ProcessAI();
            BOOL passed = source->m_poisonSoundTimer == (poison > 0 ? poison - 1 : poison)
                && source->m_nCommandPause == (test % 2 == 0 && test != 6 ? 4 : 5);
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"tick-counter-case\",\"case\":%d,\"poison\":%d,\"pause\":%d,\"passed\":%s}",
                nStep, test, source->m_poisonSoundTimer, source->m_nCommandPause, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        CGameArea* savedArea = source->m_pArea;
        source->m_pArea = NULL;
        source->m_baseStats.m_generalState = source->m_derivedStats.m_generalState = STATE_DEAD;
        source->m_curAction.m_actionID = CAIAction::WAIT;
        source->m_curAction.m_specificID = 100;
        source->AddAction(CAIAction(CAIAction::WAIT, 200));
        source->m_interrupt = TRUE;
        source->ProcessAI();
        BOOL detachedPassed = source->m_curAction.GetActionID() == CAIAction::WAIT
            && source->m_curAction.m_specificID == 200 && !source->m_interrupt;
        valid = valid && detachedPassed;
        Emit("{\"step\":%d,\"op\":\"detached-dead\",\"passed\":%s}", nStep, detachedPassed ? "true" : "false");
        source->m_pArea = savedArea;
        auto savedMode = game->m_gameSave.m_mode;
        auto savedScale = source->m_animation.GetMoveScale();
        auto savedOverride = source->field_72AA;
        auto savedOriginalScale = source->field_72A8;
        auto savedMoveDirty = source->field_562C;
        auto savedDest = source->m_posDest;
        auto savedDelta = source->m_posDelta;
        source->m_baseStats.m_generalState = source->m_derivedStats.m_generalState = 0;
        source->SetSequence(static_cast<BYTE>(savedSequence));
        game->GetWorldTimer()->m_active = FALSE;
        for (INT test = 0; test < 9; ++test) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = CAIAction::WAIT;
            source->m_curAction.m_specificID = 100;
            source->m_interrupt = FALSE;
            game->m_gameSave.m_mode = test < 5 || test == 7 || test == 8 ? 0x142 : 0;
            source->field_72AA = test == 2 || test == 3 || test == 5 || test == 6;
            source->field_72A8 = test == 6 ? 6 : 4;
            source->field_562C = 0;
            source->m_animation.SetMoveScale(test == 1 || test == 2 ? 6 : 4);
            source->m_posDest = source->m_pos + (test == 4 ? CPoint(0, 0)
                : test == 7 ? CPoint(-3, -3) : test == 8 ? CPoint(1, 1) : CPoint(3, 3));
            source->m_posDelta = CPoint(123, 456);
            source->ProcessAI();
            CPoint expected = test == 1 || test == 2 || test == 6 ? CPoint(123, 456)
                : test == 4 ? CPoint(0, 0) : test == 5 ? CPoint(2457, 3276)
                : test == 7 ? CPoint(-3686, -4915) : test == 8 ? CPoint(6144, 6144)
                : CPoint(3686, 4915);
            BOOL passed = source->m_posDelta == expected
                && source->m_animation.GetMoveScale() == (test == 5 ? 4 : 6)
                && source->field_72AA == (test == 5 || test == 6 ? FALSE : TRUE)
                && source->field_72A8 == (test == 1 || test == 6 ? 6 : 4)
                && source->field_562C == (test == 5 || test == 6 ? 1 : 0);
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"move-scale-case\",\"case\":%d,\"x\":%ld,\"y\":%ld,\"passed\":%s}",
                nStep, test, source->m_posDelta.x, source->m_posDelta.y, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        game->m_gameSave.m_mode = savedMode;
        source->m_animation.SetMoveScale(savedScale);
        source->field_72AA = savedOverride;
        source->field_72A8 = savedOriginalScale;
        source->field_562C = savedMoveDirty;
        source->m_posDest = savedDest;
        source->m_posDelta = savedDelta;
        auto& network = g_pChitin->cNetwork;
        auto savedProvider = network.m_nServiceProvider;
        auto savedLocalPlayer = network.m_idLocalPlayer;
        auto savedOwner = source->m_remotePlayerID;
        auto savedEscaping = source->m_bEscapingArea;
        auto savedDialogWait = source->m_dialogWait;
        for (INT test = 0; test < 4; ++test) {
            network.m_nServiceProvider = test < 2 ? CNetwork::SERV_PROV_NULL : CNetwork::SERV_PROV_TCP_IP;
            network.m_idLocalPlayer = 123;
            source->m_remotePlayerID = test % 2 == 0 ? 456 : 123;
            source->m_bEscapingArea = TRUE;
            source->m_dialogWait = 100;
            source->m_poisonSoundTimer = 5;
            source->ProcessAI();
            BOOL passed = source->m_dialogWait == (test == 2 ? 100 : 0)
                && source->m_poisonSoundTimer == 4;
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"network-owner-case\",\"case\":%d,\"dialogWait\":%d,\"passed\":%s}",
                nStep, test, source->m_dialogWait, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        network.m_nServiceProvider = savedProvider;
        network.m_idLocalPlayer = savedLocalPlayer;
        source->m_remotePlayerID = savedOwner;
        source->m_bEscapingArea = savedEscaping;
        source->m_dialogWait = savedDialogWait;
        auto savedArena = game->m_gameSave.m_bArenaMode;
        auto savedEA = source->m_typeAI.m_nEnemyAlly;
        auto savedLiveEA = source->m_liveTypeAI.m_nEnemyAlly;
        auto savedStartEA = source->m_startTypeAI.m_nEnemyAlly;
        auto& visibility = savedArea->m_visibility;
        LONG savedCharacterIds[6];
        memcpy(savedCharacterIds, visibility.m_aCharacterIds, sizeof(savedCharacterIds));
        BYTE* savedVisibility = new BYTE[visibility.m_nMapSize];
        memcpy(savedVisibility, visibility.m_pMap, visibility.m_nMapSize);
        for (INT test = 0; test < 8; ++test) {
            memcpy(visibility.m_aCharacterIds, savedCharacterIds, sizeof(savedCharacterIds));
            memcpy(visibility.m_pMap, savedVisibility, visibility.m_nMapSize);
            BOOL registered = visibility.IsCharacterIdOnMap(id);
            if (!registered) registered = visibility.AddCharacter(source->m_pos, id, source->m_visibleTerrainTable);
            if (test == 7 && registered) visibility.RemoveCharacter(source->m_pos, id, source->m_visibleTerrainTable);
            game->m_gameSave.m_bArenaMode = test != 0;
            network.m_nServiceProvider = test == 1 ? CNetwork::SERV_PROV_NULL : CNetwork::SERV_PROV_TCP_IP;
            network.m_idLocalPlayer = 123;
            source->m_remotePlayerID = test == 2 ? 123 : 456;
            BYTE ea = test == 4 ? 31 : test == 5 ? 0 : CAIObjectType::EA_GOODCUTOFF;
            source->m_typeAI.m_nEnemyAlly = ea;
            source->m_liveTypeAI.m_nEnemyAlly = 11;
            source->m_startTypeAI.m_nEnemyAlly = 12;
            source->m_pArea = test == 6 ? NULL : savedArea;
            source->ProcessAI();
            BOOL opponent = test >= 3;
            BOOL changed = opponent && test != 4;
            BOOL passed = registered
                && visibility.IsCharacterIdOnMap(id) == (test < 3 || test == 6)
                && source->m_typeAI.m_nEnemyAlly == (changed ? CAIObjectType::EA_ARENAPC : ea)
                && source->m_liveTypeAI.m_nEnemyAlly == (changed ? CAIObjectType::EA_ARENAPC : 11)
                && source->m_startTypeAI.m_nEnemyAlly == (changed ? CAIObjectType::EA_ARENAPC : 12);
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"arena-owner-case\",\"case\":%d,\"visible\":%s,\"ea\":%d,\"passed\":%s}",
                nStep, test, visibility.IsCharacterIdOnMap(id) ? "true" : "false",
                source->m_typeAI.m_nEnemyAlly, passed ? "true" : "false");
            source->m_pArea = savedArea;
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        memcpy(visibility.m_aCharacterIds, savedCharacterIds, sizeof(savedCharacterIds));
        memcpy(visibility.m_pMap, savedVisibility, visibility.m_nMapSize);
        delete[] savedVisibility;
        game->m_gameSave.m_bArenaMode = savedArena;
        network.m_nServiceProvider = savedProvider;
        network.m_idLocalPlayer = savedLocalPlayer;
        source->m_remotePlayerID = savedOwner;
        source->m_typeAI.m_nEnemyAlly = savedEA;
        source->m_liveTypeAI.m_nEnemyAlly = savedLiveEA;
        source->m_startTypeAI.m_nEnemyAlly = savedStartEA;
        auto savedListType = source->m_listType;
        auto savedPath = source->m_pPath;
        LONG* testPath = new LONG[1];
        testPath[0] = 0;
        for (INT test = 0; test < 4; ++test) {
            source->m_listType = test == 1 ? CGameObject::LIST_FLIGHT : CGameObject::LIST_FRONT;
            source->m_pPath = test == 2 ? testPath : NULL;
            source->SetSequence(test == 3 ? CGameSprite::SEQ_READY : CGameSprite::SEQ_WALK);
            SHORT expected = test == 0 ? source->GetIdleSequence() : test == 3 ? CGameSprite::SEQ_READY : CGameSprite::SEQ_WALK;
            source->ProcessAI();
            BOOL passed = source->m_nSequence == expected;
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"walk-idle-case\",\"case\":%d,\"sequence\":%d,\"passed\":%s}",
                nStep, test, source->m_nSequence, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        source->m_listType = savedListType;
        source->m_pPath = savedPath;
        delete[] testPath;
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        while (messages.GetCount() > before) delete messages.RemoveTail();
        source->m_poisonSoundTimer = savedPoison;
        source->m_nCommandPause = savedPause;
        source->m_userCommandPause = savedUserPause;
        source->m_active = savedSpriteActive;
        source->m_activeAI = savedAIActive;
        source->m_activeImprisonment = savedImprisonment;
        source->m_bSelected = savedSelected;
        source->m_derivedStats.m_generalState = savedState;
        source->m_baseStats.m_generalState = savedBaseState;
        source->m_derivedStats.m_spellStates = savedSpells;
        source->SetSequence(static_cast<BYTE>(savedSequence));
        source->m_baseStats.field_2F0[3] = savedImmunity;
        source->field_3428 = savedFlags;
        game->GetWorldTimer()->m_active = savedActive;
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"tickcounters\",\"checks\":33,\"valid\":%s}", nStep, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "tick counters mismatch"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "protectchecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "actor unavailable"); return;
        }
        auto* source = static_cast<CGameSprite*>(object);
        auto savedEA = source->m_typeAI.m_nEnemyAlly;
        auto savedSelected = source->m_bSelected;
        auto savedActive = game->GetWorldTimer()->m_active;
        auto savedTargetPoint = source->m_targetPoint;
        auto savedTarget = source->m_targetId;
        auto savedGroup = source->m_groupMove;
        source->m_bSelected = FALSE;
        source->m_groupMove = FALSE;
        game->GetWorldTimer()->m_active = FALSE;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        BOOL valid = TRUE;
        for (INT test = 0; test < 4; ++test) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = test == 3 ? CAIAction::WAIT : CAIAction::PROTECTPOINT;
            source->m_curAction.m_dest = source->m_pos;
            source->m_typeAI.m_nEnemyAlly = test == 0 ? 199 : test == 1 ? 200 : 255;
            source->m_interrupt = FALSE;
            source->m_targetId = CGameObjectArray::INVALID_INDEX;
            source->AddAction(CAIAction(CAIAction::WAIT, 100));
            source->ProcessAI();
            BOOL replaced = test == 1 || test == 2;
            INT drops = 0, actions = 0;
            BOOL passed = source->m_queuedActions.GetCount() == (replaced ? 0 : 1)
                && source->m_curAction.GetActionID() == (test == 3 ? CAIAction::WAIT : CAIAction::PROTECTPOINT);
            while (messages.GetCount() > before) {
                CMessage* raw = messages.RemoveTail();
                if (dynamic_cast<CMessageDropPath*>(raw) != NULL) ++drops;
                auto* action = dynamic_cast<CMessageAddAction*>(raw);
                if (action != NULL) {
                    ++actions;
                    passed = passed && action->m_action.GetActionID() == CAIAction::NO_ACTION
                        && action->m_action.m_specificID == 0 && action->m_action.m_specificID2 == -1
                        && action->m_action.m_specificID3 == 0 && action->m_action.m_dest == CPoint(-1, -1)
                        && action->m_action.m_internalFlags == 0;
                }
                delete raw;
            }
            passed = passed && drops == (replaced ? 1 : 0) && actions == (replaced ? 1 : 0);
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"protect-case\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
        }
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        source->m_typeAI.m_nEnemyAlly = savedEA;
        source->m_bSelected = savedSelected;
        source->m_targetPoint = savedTargetPoint;
        source->m_targetId = savedTarget;
        source->m_groupMove = savedGroup;
        game->GetWorldTimer()->m_active = savedActive;
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"protectchecks\",\"checks\":4,\"valid\":%s}", nStep, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "protect point replacement mismatch"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "fatiguechecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* object = NULL;
        if (objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "actor unavailable"); return;
        }
        auto* source = static_cast<CGameSprite*>(object);
        BYTE savedFatigue = source->m_baseStats.m_fatigue;
        ULONG savedEpoch = source->field_722A;
        SHORT savedLuck = source->m_derivedStats.m_nLuck;
        ULONG savedTime = game->GetWorldTimer()->m_gameTime;
        BOOL hadIcon = source->m_portraitIcons.Find(reinterpret_cast<int*>(39)) != NULL;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        ULONG period = 18000;
        auto run = [&](const char* name, BYTE base, ULONG elapsed, BOOL effectExpected, BOOL soundExpected) {
            game->GetWorldTimer()->m_gameTime = 1000000;
            source->m_baseStats.m_fatigue = base;
            source->field_722A = 1000000 - elapsed;
            source->m_derivedStats.m_nLuck = 10;
            source->CheckFatigue();
            INT effects = 0, sounds = 0;
            BOOL passed = TRUE;
            LONG amount = elapsed / period;
            while (messages.GetCount() > before) {
                CMessage* raw = messages.RemoveTail();
                auto* effectMessage = dynamic_cast<CMessageAddEffect*>(raw);
                auto* soundMessage = dynamic_cast<CMessagePlaySound*>(raw);
                if (effectMessage != NULL) {
                    ++effects;
                    auto* effect = effectMessage->m_effect;
                    passed = passed && effect->m_effectID == CGAMEEFFECT_FATIGUE
                        && effect->m_effectAmount == amount && effect->m_dwFlags == 1 && effect->m_durationType == 1;
                    effect->ApplyEffect(source);
                    passed = passed && source->m_baseStats.m_fatigue == min(amount, 100);
                } else if (soundMessage != NULL) {
                    ++sounds;
                    passed = passed && soundMessage->m_soundId == 3
                        && soundMessage->m_showText && soundMessage->m_showCircle;
                } else passed = FALSE;
                delete raw;
            }
            signed char modifier = static_cast<signed char>(atoi(game->GetRuleTables().m_tFatigueMod.GetAt(CPoint(0, amount))));
            passed = passed && effects == (effectExpected ? 1 : 0) && sounds == (soundExpected ? 1 : 0)
                && source->m_derivedStats.m_nLuck == 10 + (effectExpected ? modifier : 0)
                && source->field_722A == (amount < base ? 1000000 - period * base : 1000000 - elapsed);
            if (effectExpected && modifier != 0) passed = passed && source->m_portraitIcons.Find(reinterpret_cast<int*>(39)) != NULL;
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"fatigue-case\",\"case\":\"%s\",\"effects\":%d,\"sounds\":%d,\"passed\":%s}",
                nStep, name, effects, sounds, passed ? "true" : "false");
        };
        run("equal", 2, 2 * period, FALSE, FALSE);
        run("below-boundary", 0, period - 1, FALSE, FALSE);
        run("increase", 0, 2 * period, TRUE, FALSE);
        run("sound-threshold-six", 0, 6 * period, TRUE, FALSE);
        run("sound-threshold-seven", 0, 7 * period, TRUE, TRUE);
        run("clock-correction", 5, 2 * period, FALSE, FALSE);
        run("clock-correction-sound", 7, 2 * period, FALSE, TRUE);
        LONG savedFrame = source->field_44A;
        BOOLEAN savedActive = game->GetWorldTimer()->m_active;
        game->GetWorldTimer()->m_active = FALSE;
        game->GetWorldTimer()->m_gameTime = savedTime;
        for (INT phase = 0; phase < 2; ++phase) {
            source->m_baseStats.m_fatigue = 5;
            source->field_722A = savedTime - 2 * period;
            source->field_44A = static_cast<DWORD>(id) % 2051 + phase;
            source->ProcessAI();
            BOOL passed = source->field_722A == savedTime - (phase == 0 ? 5 : 2) * period;
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"fatigue-phase\",\"case\":%d,\"passed\":%s}",
                nStep, phase, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        source->field_44A = savedFrame;
        game->GetWorldTimer()->m_active = savedActive;
        source->m_baseStats.m_fatigue = savedFatigue;
        source->field_722A = savedEpoch;
        source->m_derivedStats.m_nLuck = savedLuck;
        game->GetWorldTimer()->m_gameTime = savedTime;
        if (!hadIcon) {
            POSITION pos = source->m_portraitIcons.Find(reinterpret_cast<int*>(39));
            if (pos != NULL) source->m_portraitIcons.RemoveAt(pos);
        }
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"fatiguechecks\",\"checks\":%d,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "fatigue mismatch"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "actiongates") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and party actor");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        if (source->m_pArea == NULL || source->m_pPath != NULL || source->m_currentSearchRequest != NULL) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires idle actor in area");
            return;
        }
        auto savedState = source->m_derivedStats.m_generalState;
        auto savedBaseState = source->m_baseStats.m_generalState;
        auto savedSpells = source->m_derivedStats.m_spellStates;
        auto savedHold = source->m_derivedStats.m_bCasterHold;
        auto savedFlags = source->m_baseStats.m_flags;
        auto savedImmunity = source->m_baseStats.field_2F0[3];
        auto savedStart = source->m_posStart;
        auto savedTargetPoint = source->m_targetPoint;
        auto savedGroupMove = source->m_groupMove;
        auto savedSpriteActive = source->m_active;
        auto savedAIActive = source->m_activeAI;
        auto savedImprisonmentActive = source->m_activeImprisonment;
        auto savedUnselectable = source->m_nUnselectableCounter;
        auto savedSequence = source->m_nSequence;
        auto savedSelected = source->m_bSelected;
        auto savedEA = source->m_typeAI.m_nEnemyAlly;
        auto savedAllow = source->m_bAllowDialogInterrupt;
        auto savedIgnore = source->m_derivedStats.m_bIgnoreDialogPause;
        auto savedModal = source->m_nModalState;
        auto savedHiding = source->m_bHiding;
        auto savedTempWeapon = source->m_nTempSelectedWeapon;
        auto savedTempAbility = source->m_nTempSelectedWeaponAbility;
        auto savedTrackedAction = source->m_currentActionId;
        auto savedActionTarget = source->m_targetId;
        auto* savedCurrentSpell = source->m_curSpell;
        auto* savedCurrentItem = source->m_curItem;
        auto* savedCurrentProjectile = source->m_curProjectile;
        auto savedCastPhase = source->field_710C;
        auto savedActionPhase = source->field_5618;
        auto savedMovePhase = source->field_5616;
        auto savedNoActionCount = source->m_noActionCount;
        auto savedActionLuck = source->m_derivedStats.m_nLuck;
        auto savedActionUserPause = source->m_userCommandPause;
        auto savedForceRender = source->m_marker.m_nRecticleForceRender;
        auto savedStop = game->m_nTimeStop;
        auto savedCaster = game->m_nTimeStopCaster;
        auto savedActive = game->GetWorldTimer()->m_active;
        auto savedCutscene = game->GetGameSave()->m_cutScene;
        auto& instants = const_cast<CAIIdList&>(game->GetRuleTables().m_lInstantActions);
        auto savedFaster = instants.m_faster;
        instants.m_faster = FALSE;
        CAIId* fixture = NULL;
        if (instants.Find(269) == NULL) {
            fixture = new CAIId();
            fixture->m_id = 269;
            fixture->m_line = "ZRC_STARTPOS";
            instants.m_idList.AddTail(fixture);
        }
        BOOL valid = instants.Find(CAIAction::WAIT) == NULL;
        INT checks = 0;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        source->m_bSelected = FALSE;
        source->m_nModalState = 0;
        source->m_currentActionId = 0;
        source->m_targetId = CGameObjectArray::INVALID_INDEX;
        source->m_curSpell = NULL;
        source->m_curItem = NULL;
        source->m_curProjectile = NULL;
        game->GetWorldTimer()->m_active = TRUE;
        game->GetGameSave()->m_cutScene = FALSE;
        CPoint marker(savedStart.x + 3, savedStart.y + 5);
        auto run = [&](const char* name, DWORD state, INT spell, BOOL casterHold,
                       LONG stop, BOOL ownsStop, BOOL instant, BOOL shouldExecute, BOOL paused = FALSE, BOOL cutscene = FALSE, INT inactive = -1) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = instant ? 269 : CAIAction::WAIT;
            source->m_curAction.m_specificID = 100;
            source->m_curAction.m_dest = marker;
            source->m_actionCount = 1;
            source->m_interrupt = FALSE;
            source->m_nLastActionReturn = CGameAIBase::ACTION_DONE;
            source->m_baseStats.m_flags = savedFlags & 0x7FFFFFFF;
            source->m_baseStats.field_2F0[3] |= 1;
            source->m_baseStats.m_generalState = state;
            source->m_derivedStats.m_generalState = state;
            source->m_derivedStats.m_spellStates.reset();
            if (spell >= 0) source->m_derivedStats.m_spellStates.set(spell);
            source->m_derivedStats.m_bCasterHold = casterHold;
            source->m_inCutScene = cutscene;
            source->m_active = inactive != 0;
            source->m_activeAI = inactive != 1;
            source->m_activeImprisonment = inactive != 2;
            source->m_moraleFailure = FALSE;
            source->field_727E = FALSE;
            source->field_3428 = 7;
            source->field_558E = FALSE;
            source->m_userCommandPause = source->m_nCommandPause = 0;
            source->m_dialogWait = 0;
            source->m_posStart = savedStart;
            source->m_nSequence = CGameSprite::SEQ_READY;
            game->m_nTimeStop = stop;
            game->m_nTimeStopCaster = ownsStop ? id : -1;
            game->GetWorldTimer()->m_active = !paused;
            source->m_nUnselectableCounter = 5;
            if (instant || (state & STATE_DEAD) || inactive >= 0) source->AddAction(CAIAction(CAIAction::WAIT, 200));
            LONG beforeFrame = source->field_44A;
            source->ProcessAI();
            BOOL executed = instant ? source->m_posStart == marker
                : source->m_curAction.m_actionID == CAIAction::WAIT && source->m_curAction.m_specificID == 99;
            BOOL passed = executed == shouldExecute;
            if ((state & STATE_DEAD) || inactive >= 0) {
                passed = passed && source->m_curAction.m_actionID == CAIAction::WAIT
                    && source->m_curAction.m_specificID == 200 && !source->m_interrupt;
            } else if (!instant && !shouldExecute) {
                passed = passed && source->m_curAction.m_specificID == 100;
            }
            if (inactive >= 0) passed = passed && source->field_44A == beforeFrame;
            passed = passed && source->m_nUnselectableCounter == (paused ? 5 : 4);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"actiongates-case\",\"case\":\"%s\",\"instant\":%s,\"executed\":%s,\"action\":%d,\"specific\":%ld,\"passed\":%s}",
                nStep, name, instant ? "true" : "false", executed ? "true" : "false",
                source->m_curAction.m_actionID, source->m_curAction.m_specificID, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        };
        for (INT instant = 0; instant < 2; ++instant) {
            run("able", 0, -1, FALSE, 0, FALSE, instant, TRUE);
            run("caster-hold", 0, -1, TRUE, 0, FALSE, instant, instant);
            run("held", 0, SPLSTATE_HELD, FALSE, 0, FALSE, instant, instant);
            run("hopelessness", 0, SPLSTATE_HOPELESSNESS, FALSE, 0, FALSE, instant, instant);
            run("resilient-sphere", 0, SPLSTATE_OTILUKES_RESILIENT_SPHERE, FALSE, 0, FALSE, instant, instant);
            run("petrified", STATE_STONE_DEATH, -1, FALSE, 0, FALSE, instant, instant);
            run("frozen", STATE_FROZEN_DEATH, -1, FALSE, 0, FALSE, instant, instant);
            run("time-stop-other", 0, -1, FALSE, 30, FALSE, instant, instant);
            run("time-stop-caster", 0, -1, FALSE, 30, TRUE, instant, TRUE);
            run("negative-time-stop", 0, -1, FALSE, -1, FALSE, instant, instant);
            run("dead", STATE_DEAD, -1, FALSE, 0, FALSE, instant, instant);
        }
        run("paused-ordinary", 0, -1, FALSE, 0, FALSE, FALSE, FALSE, TRUE);
        run("paused-instant", 0, -1, FALSE, 0, FALSE, TRUE, TRUE, TRUE);
        run("paused-held-instant", 0, SPLSTATE_HELD, FALSE, 0, FALSE, TRUE, TRUE, TRUE);
        run("paused-caster-hold-instant", 0, -1, TRUE, 0, FALSE, TRUE, TRUE, TRUE);
        run("paused-time-stop-instant", 0, -1, FALSE, 30, FALSE, TRUE, TRUE, TRUE);
        for (INT instant = 0; instant < 2; ++instant) {
            run("cutscene-able", 0, -1, FALSE, 0, FALSE, instant, TRUE, FALSE, TRUE);
            run("cutscene-held", 0, SPLSTATE_HELD, FALSE, 0, FALSE, instant, instant, FALSE, TRUE);
            run("cutscene-caster-hold", 0, -1, TRUE, 0, FALSE, instant, instant, FALSE, TRUE);
            run("cutscene-time-stop", 0, -1, FALSE, 30, FALSE, instant, instant, FALSE, TRUE);
        }
        for (INT inactive = 0; inactive < 3; ++inactive) {
            run("inactive-ordinary", 0, -1, FALSE, 0, FALSE, FALSE, FALSE, FALSE, FALSE, inactive);
            run("inactive-instant", 0, -1, FALSE, 0, FALSE, TRUE, TRUE, FALSE, FALSE, inactive);
        }
        source->m_active = source->m_activeAI = source->m_activeImprisonment = TRUE;
        CTypedPtrList<CPtrList, CAITrigger*> savedTriggers;
        while (!source->m_pendingTriggers.IsEmpty()) savedTriggers.AddTail(source->m_pendingTriggers.RemoveHead());
        game->GetWorldTimer()->m_active = FALSE;
        for (INT triggerCase = 0; triggerCase < 4; ++triggerCase) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = CAIAction::WAIT;
            source->m_dialogWait = 10;
            source->m_inCutScene = FALSE;
            source->m_pendingTriggers.AddTail(new CAITrigger(triggerCase == 0 ? CAITRIGGER_ATTACKEDBY
                : triggerCase == 1 ? CAITRIGGER_HITBY : CAITRIGGER_NO_TRIGGER, 0));
            if (triggerCase == 3) source->m_pendingTriggers.AddTail(new CAITrigger(CAITRIGGER_HITBY, 0));
            source->ProcessAI();
            BOOL passed = source->m_dialogWait == (triggerCase < 2 ? 0 : 10);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"dialog-combat-trigger\",\"case\":%d,\"passed\":%s}",
                nStep, triggerCase, passed ? "true" : "false");
            while (!source->m_pendingTriggers.IsEmpty()) delete source->m_pendingTriggers.RemoveHead();
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        while (!savedTriggers.IsEmpty()) source->m_pendingTriggers.AddTail(savedTriggers.RemoveHead());
        source->m_dialogWait = 0;
        CPoint savedExact = source->m_posExact;
        LONG savedFrame = source->field_44A;
        LONG* savedSyncPath = source->m_pPath;
        LONG* syncPath = new LONG[1];
        syncPath[0] = 0;
        for (INT syncCase = 0; syncCase < 3; ++syncCase) {
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = CAIAction::WAIT;
            source->m_curAction.m_specificID = 100;
            source->field_44A = static_cast<DWORD>(id) % 16 + (syncCase == 1 ? 1 : 0);
            source->SetSequence(syncCase == 2 ? CGameSprite::SEQ_WALK : CGameSprite::SEQ_READY);
            source->m_pPath = syncCase == 2 ? syncPath : savedSyncPath;
            CPoint exact((source->m_pos.x << CGameSprite::EXACT_SCALE),
                (source->m_pos.y << (CGameSprite::EXACT_SCALE + 2)) / 3);
            CPoint stale(exact.x + 8, exact.y + 8);
            source->m_posExact = stale;
            source->ProcessAI();
            BOOL passed = source->m_posExact == (syncCase == 0 ? exact : stale);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"periodic-sync\",\"case\":%d,\"passed\":%s}",
                nStep, syncCase, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        source->m_pPath = savedSyncPath;
        delete[] syncPath;
        source->m_posExact = savedExact;
        source->field_44A = savedFrame;
        source->SetSequence(CGameSprite::SEQ_READY);
        game->GetWorldTimer()->m_active = TRUE;
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        source->m_inCutScene = TRUE;
        source->m_nCommandPause = 10;
        source->ProcessAI();
        BOOL idleCleared = !source->m_inCutScene && source->m_nCommandPause <= 0;
        valid = valid && idleCleared;
        ++checks;
        Emit("{\"step\":%d,\"op\":\"cutscene-idle\",\"passed\":%s}", nStep, idleCleared ? "true" : "false");
        while (messages.GetCount() > before) delete messages.RemoveTail();
        game->GetWorldTimer()->m_active = FALSE;
        source->m_groupMove = FALSE;
        for (INT preview = 0; preview < 3; ++preview) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = 23;
            source->m_curAction.m_dest = savedStart;
            CAIAction queued;
            queued.m_actionID = 27;
            queued.m_dest = marker;
            source->AddAction(queued);
            source->m_interrupt = TRUE;
            source->m_userCommandPause = 0;
            source->m_nCommandPause = preview == 2 ? 0 : 10;
            source->m_baseStats.m_flags = savedFlags & 0x7FFFFFFF;
            if (preview == 1) source->m_baseStats.m_flags |= 0x80000000;
            source->m_targetPoint = CPoint(-1, -1);
            source->ProcessAI();
            BOOL passed = source->m_targetPoint == (preview == 1 ? savedStart : marker)
                && source->m_curAction.GetActionID() == (preview == 1 ? 23 : 27)
                && source->m_interrupt == (preview == 1);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"paused-preview\",\"case\":%d,\"passed\":%s}",
                nStep, preview, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        for (INT markerCase = 0; markerCase < 4; ++markerCase) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = CAIAction::WAIT;
            source->m_curAction.m_specificID = 100;
            source->m_actionCount = 1;
            source->m_interrupt = FALSE;
            source->m_userCommandPause = source->m_nCommandPause = 0;
            source->m_baseStats.m_flags = savedFlags & 0x7FFFFFFF;
            source->m_dialogWait = 0;
            source->m_inCutScene = source->m_moraleFailure = FALSE;
            source->field_3428 = 7;
            source->m_groupMove = markerCase % 2 != 0;
            game->GetWorldTimer()->m_active = markerCase < 2;
            CPoint sentinel(123, 456);
            source->m_targetPoint = sentinel;
            source->ProcessAI();
            BOOL passed = source->m_targetPoint == (markerCase == 2 ? CPoint(-1, -1) : sentinel)
                && source->m_curAction.GetActionID() == CAIAction::WAIT;
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"marker-tail-case\",\"case\":%d,\"x\":%ld,\"y\":%ld,\"passed\":%s}",
                nStep, markerCase, source->m_targetPoint.x, source->m_targetPoint.y, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        CAIAction savedResolveScratch;
        savedResolveScratch = CGameSprite::m_aiDoAction;
        for (INT scratchCase = 0; scratchCase < 4; ++scratchCase) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = scratchCase == 0 ? CAIAction::NO_ACTION : CAIAction::WAIT;
            source->m_curAction.m_specificID = scratchCase == 2 ? 1 : 100;
            source->m_actionCount = 1;
            source->m_interrupt = scratchCase == 1 || scratchCase == 3;
            source->m_userCommandPause = 0;
            source->m_nCommandPause = scratchCase == 3 ? 10 : 0;
            source->m_baseStats.m_flags = savedFlags & 0x7FFFFFFF;
            source->m_dialogWait = 0;
            source->m_inCutScene = source->m_moraleFailure = FALSE;
            source->field_3428 = 7;
            source->m_groupMove = FALSE;
            game->GetWorldTimer()->m_active = scratchCase == 1 || scratchCase == 2;
            CGameSprite::m_aiDoAction = CAIAction::NULL_ACTION;
            CGameSprite::m_aiDoAction.m_actionID = CAIAction::WAIT;
            CGameSprite::m_aiDoAction.m_specificID = 901;
            CGameSprite::m_aiDoAction.m_specificID2 = 902;
            CGameSprite::m_aiDoAction.m_specificID3 = 903;
            source->AddAction(CAIAction(CAIAction::WAIT, 200));
            source->ProcessAI();
            BOOL resolved = scratchCase == 1 || scratchCase == 2;
            BOOL passed = source->m_curAction.GetActionID() == CAIAction::WAIT
                && source->m_curAction.m_specificID == 200 && !source->m_interrupt
                && CGameSprite::m_aiDoAction.m_specificID == (resolved ? 200 : 901)
                && (resolved || (CGameSprite::m_aiDoAction.m_specificID2 == 902
                    && CGameSprite::m_aiDoAction.m_specificID3 == 903));
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-scratch-case\",\"case\":%d,\"specific\":%ld,\"scratch\":%ld,\"passed\":%s}",
                nStep, scratchCase, source->m_curAction.m_specificID,
                CGameSprite::m_aiDoAction.m_specificID, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        CGameSprite::m_aiDoAction = savedResolveScratch;
        POSITION savedVertList = source->m_posVertList;
        for (INT listCase = 0; listCase < 4; ++listCase) {
            BOOL registered = listCase % 2 == 0;
            BOOL instant = listCase >= 2;
            source->m_posVertList = registered ? savedVertList : NULL;
            run(registered ? "render-list-present" : "render-list-absent",
                0, -1, FALSE, 0, FALSE, instant, registered);
            source->m_posVertList = savedVertList;
        }
        for (INT transition = 0; transition < 6; ++transition) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            if (transition == 1 || transition == 3) source->m_curAction.m_actionID = CAIAction::WAIT;
            source->m_groupMove = transition == 5;
            CPoint sentinel(123, 456);
            source->m_targetPoint = sentinel;
            CAIAction incoming;
            incoming.m_actionID = transition == 2 || transition >= 4 ? CAIAction::MOVETOPOINT
                : transition == 3 ? CAIAction::WAIT : CAIAction::NO_ACTION;
            incoming.m_dest = marker;
            if (transition >= 4) {
                CAIAction queued;
                queued.m_actionID = CAIAction::PROTECTPOINT;
                queued.m_dest = savedStart;
                source->AddAction(queued);
            }
            source->SetCurrAction(incoming);
            CPoint expected = transition == 0 || transition == 5 ? sentinel
                : transition == 1 || transition == 3 ? CPoint(-1, -1) : marker;
            BOOL passed = source->m_targetPoint == expected
                && source->m_curAction.GetActionID() == incoming.GetActionID()
                && source->m_queuedActions.GetCount() == (transition >= 4 ? 1 : 0);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-marker-transition\",\"case\":%d,\"x\":%ld,\"y\":%ld,\"passed\":%s}",
                nStep, transition, source->m_targetPoint.x, source->m_targetPoint.y, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        // Exercise StopContainer with a real registered, empty ground container.
        auto* world = g_pBaldurChitin->m_pEngineWorld;
        LONG savedContainer = game->m_iContainer;
        LONG savedContainerSprite = game->m_iContainerSprite;
        INT savedPopup = world->m_nPopupState;
        BYTE savedAutoHide = world->m_nAutoHideInterface;
        CRect savedView = world->m_newViewSize;
        BYTE savedResize = world->m_waitingOnResize;
        BOOL savedForceResize = world->m_bForceViewSize;
        auto* containerPanel = world->GetManager()->GetPanel(8);
        auto* actionPanel = world->GetManager()->GetPanel(1);
        auto* commandPanel = world->GetManager()->GetPanel(world->GetPanel_22_0());
        BOOL savedContainerActive = containerPanel->m_bActive;
        BOOL savedActionActive = actionPanel->m_bActive;
        BOOL savedCommandActive = commandPanel->m_bActive;
        auto* container = new CGameContainer(source->m_pArea,
            CRect(source->m_pos.x, source->m_pos.y, source->m_pos.x + 16, source->m_pos.y + 12));
        game->m_iContainer = container->m_id;
        world->m_nAutoHideInterface = 0;
        for (INT containerCase = 0; containerCase < 4; ++containerCase) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            world->m_nPopupState = containerCase == 2 ? -1 : 8;
            game->m_iContainerSprite = containerCase == 1 ? CGameObjectArray::INVALID_INDEX : id;
            containerPanel->SetActive(TRUE);
            CAIAction incoming;
            incoming.m_actionID = containerCase == 0 ? CAIAction::NO_ACTION : CAIAction::WAIT;
            source->SetCurrAction(incoming);
            BOOL passed = world->m_nPopupState == (containerCase >= 2 ? -1 : 8)
                && containerPanel->m_bActive == (containerCase != 3)
                && source->m_curAction.GetActionID() == incoming.GetActionID()
                && container->m_lstItems.IsEmpty();
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-container-transition\",\"case\":%d,\"popup\":%d,\"panelActive\":%s,\"passed\":%s}",
                nStep, containerCase, world->m_nPopupState,
                containerPanel->m_bActive ? "true" : "false", passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        game->m_iContainer = savedContainer;
        game->m_iContainerSprite = savedContainerSprite;
        world->m_nPopupState = savedPopup;
        world->m_nAutoHideInterface = savedAutoHide;
        world->m_newViewSize = savedView;
        world->m_waitingOnResize = savedResize;
        world->m_bForceViewSize = savedForceResize;
        containerPanel->SetActive(savedContainerActive);
        actionPanel->SetActive(savedActionActive);
        commandPanel->SetActive(savedCommandActive);
        auto& buttons = game->m_cButtonArray;
        INT savedBarState = buttons.m_nState;
        INT savedBarSelection = buttons.m_nSelectedButton;
        auto savedBarStack = buttons.m_stateStack;
        INT savedPortrait = world->m_nSelectedCharacter;
        SHORT portrait = game->GetCharacterPortraitNum(id);
        CTypedPtrList<CPtrList, LONG*> savedGroupMembers;
        savedGroupMembers.AddTail(&game->m_group.m_memberList);
        LONG otherId = game->GetCharacterId((portrait + 1) % game->GetNumCharacters());
        auto modalCase = [&](BYTE mode, SHORT actionId, INT barState, BOOL selected,
                             BOOL preserve, BOOL reset) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_nModalState = mode;
            source->m_bHiding = mode == 3;
            // World selection is derived from the group's actual leader.
            game->m_group.m_memberList.RemoveAll();
            game->m_group.m_memberList.AddTail(reinterpret_cast<LONG*>(selected ? id : otherId));
            buttons.m_nState = barState;
            buttons.m_nSelectedButton = 7;
            buttons.m_stateStack.clear();
            buttons.m_stateStack.push_front(0x72);
            CAIAction incoming;
            incoming.m_actionID = actionId;
            incoming.m_dest = marker;
            source->SetCurrAction(incoming);
            BOOL special = mode == 2 || mode == 4;
            BOOL passed = source->m_nModalState == (preserve ? mode : 0)
                && source->m_curAction.GetActionID() == actionId
                && buttons.m_stateStack.size() == (reset ? 0 : 1)
                && buttons.m_nState == (reset ? 0x72 : barState)
                && buttons.m_nSelectedButton == (!preserve && special ? 100 : 7)
                && source->m_bHiding == (mode == 3 && preserve);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-modal-transition\",\"mode\":%u,\"action\":%d,\"bar\":%d,\"selected\":%s,\"reset\":%s,\"passed\":%s}",
                nStep, mode, actionId, barState, selected ? "true" : "false",
                reset ? "true" : "false", passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        };
        const SHORT modalActions[] = { 0, 23, 22, 84, 18, 83, 99, 89, 90, 215,
            13, 33, 234, 273, 274, 293, 183, 233, 30, 63, 10, 257, 236, 249, 252, 36, 85 };
        for (BYTE mode = 2; mode <= 4; ++mode) {
            for (SHORT actionId : modalActions) modalCase(mode, actionId, 0x6E, TRUE, TRUE, FALSE);
        }
        const BYTE exitingModes[] = { 0, 2, 3, 4, 5 };
        for (BYTE mode : exitingModes) {
            modalCase(mode, CAIAction::ATTACK, 0x6E, TRUE, mode == 0, mode == 2 || mode == 4);
        }
        modalCase(2, CAIAction::ATTACK, 0x6E, FALSE, FALSE, FALSE);
        modalCase(4, CAIAction::ATTACK, 0x6E, FALSE, FALSE, FALSE);
        const INT protectedBars[] = { 0x66, 0x65, 0x68, 0x67, 0x69, 0x70, 0x73,
            0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x6A };
        for (INT bar : protectedBars) modalCase(2, CAIAction::ATTACK, bar, TRUE, FALSE, FALSE);
        modalCase(4, CAIAction::ATTACK, 0x6B, TRUE, FALSE, TRUE);
        source->m_nModalState = savedModal;
        source->m_bHiding = savedHiding;
        game->m_group.m_memberList.RemoveAll();
        game->m_group.m_memberList.AddTail(&savedGroupMembers);
        world->m_nSelectedCharacter = savedPortrait;
        buttons.SetState(savedBarState, 0);
        buttons.m_stateStack = savedBarStack;
        buttons.m_nSelectedButton = savedBarSelection;
        BYTE savedWeapon = source->m_equipment.m_selectedWeapon;
        WORD savedAbility = source->m_equipment.m_selectedWeaponAbility;
        INT savedTrapFlag = source->field_580;
        source->m_nModalState = 0;
        const SHORT weaponActions[] = { 0, 63, 3, 34, 92, 93, 28 };
        for (INT weaponCase = 0; weaponCase < 7; ++weaponCase) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_equipment.m_selectedWeapon = savedWeapon;
            source->m_equipment.m_selectedWeaponAbility = 0x1234;
            source->m_nTempSelectedWeapon = 0xFE;
            source->m_nTempSelectedWeaponAbility = 0xAB;
            CAIAction incoming;
            incoming.m_actionID = weaponActions[weaponCase];
            source->SetCurrAction(incoming);
            BOOL preserve = incoming.GetActionID() == 92;
            BOOL passed = source->m_nTempSelectedWeapon == (preserve ? 0xFE : savedWeapon)
                && source->m_nTempSelectedWeaponAbility == (preserve ? 0xAB : 0x34)
                && source->m_equipment.m_selectedWeapon == savedWeapon
                && source->m_equipment.m_selectedWeaponAbility == 0x1234;
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-weapon-snapshot\",\"action\":%d,\"ability\":%u,\"passed\":%s}",
                nStep, incoming.GetActionID(), source->m_nTempSelectedWeaponAbility, passed ? "true" : "false");
            source->m_equipment.m_selectedWeaponAbility = savedAbility;
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        const SHORT oldTrapActions[] = { 0, 28, 28, 28, 63 };
        const SHORT newTrapActions[] = { 28, 28, 63, 0, 63 };
        for (INT trapCase = 0; trapCase < 5; ++trapCase) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = oldTrapActions[trapCase];
            source->field_580 = 99;
            CAIAction incoming;
            incoming.m_actionID = newTrapActions[trapCase];
            source->SetCurrAction(incoming);
            BOOL passed = source->field_580 == 99
                && source->m_curAction.GetActionID() == incoming.GetActionID()
                && messages.GetCount() - before == (trapCase < 4 ? 1 : 0);
            if (trapCase < 4 && messages.GetCount() > before) {
                CMessage* message = messages.RemoveTail();
                BOOL isTrapMessage = message->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_107;
                passed = passed && isTrapMessage && message->GetSource() == id && message->GetTarget() == id;
                if (isTrapMessage) {
                    BYTE expected = trapCase < 2 ? TRUE : FALSE;
                    passed = passed && static_cast<CMessage107*>(message)->field_C == expected;
                    message->Run();
                    passed = passed && source->field_580 == expected;
                }
                delete message;
            }
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-trap-message\",\"case\":%d,\"flag\":%d,\"passed\":%s}",
                nStep, trapCase, source->field_580, passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        for (INT messageCase = 0; messageCase < 2; ++messageCase) {
            source->field_580 = 99;
            CMessage107 message(0xFF, id, messageCase == 0 ? id : CGameObjectArray::INVALID_INDEX);
            message.Run();
            BOOL passed = source->field_580 == (messageCase == 0 ? 255 : 99);
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"trap-message-target\",\"case\":%d,\"flag\":%d,\"passed\":%s}",
                nStep, messageCase, source->field_580, passed ? "true" : "false");
        }
        auto notificationCase = [&](SHORT tracked, SHORT incomingId, BOOL special, BOOL leaveTraps) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = leaveTraps ? CAIAction::REMOVETRAPS : CAIAction::WAIT;
            source->m_currentActionId = tracked;
            source->m_bPlayedEncumberedStopped = TRUE;
            source->m_bPlayedEncumberedSlowed = TRUE;
            CAIAction incoming;
            incoming.m_actionID = incomingId;
            incoming.m_specificID = 101;
            incoming.m_specificID2 = 202;
            incoming.m_specificID3 = 303;
            incoming.m_internalFlags = 0x10203040;
            BOOL notify = tracked != incomingId && (special || tracked != 0);
            INT expectedCount = (notify ? 1 : 0) + (leaveTraps ? 1 : 0);
            source->SetCurrAction(incoming);
            BOOL passed = messages.GetCount() - before == expectedCount
                && source->m_currentActionId == tracked
                && source->m_curAction.GetActionID() == incomingId
                && source->m_curAction.m_specificID == 101
                && source->m_curAction.m_specificID2 == 202
                && source->m_curAction.m_specificID3 == 303
                && source->m_curAction.m_internalFlags == 0x10203040
                && !source->m_bPlayedEncumberedStopped && !source->m_bPlayedEncumberedSlowed;
            POSITION pos = messages.FindIndex(before);
            if (leaveTraps && pos != NULL) {
                CMessage* trap = messages.GetNext(pos);
                passed = passed && trap->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_107;
            }
            if (notify && pos != NULL) {
                CMessage* message = messages.GetNext(pos);
                BOOL isNotification = message->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_CURRENT_ACTION_ID;
                SHORT expected = special ? incomingId : 0;
                passed = passed && isNotification && message->GetSource() == id && message->GetTarget() == id;
                if (isNotification) {
                    passed = passed && static_cast<CMessageSetCurrentActionId*>(message)->m_actionId == expected;
                    message->Run();
                    passed = passed && source->m_currentActionId == expected;
                }
            }
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"action-id-notification\",\"tracked\":%d,\"incoming\":%d,\"trapFirst\":%s,\"notify\":%s,\"passed\":%s}",
                nStep, tracked, incomingId, leaveTraps ? "true" : "false", notify ? "true" : "false", passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        };
        const SHORT notifiedActions[] = { 108, 176, 8, 137, 198 };
        for (SHORT incoming : notifiedActions) {
            notificationCase(0, incoming, TRUE, FALSE);
            notificationCase(incoming, incoming, TRUE, FALSE);
            notificationCase(777, incoming, TRUE, FALSE);
        }
        const SHORT ordinaryActions[] = { 0, 63, 3, 139 };
        for (SHORT incoming : ordinaryActions) {
            notificationCase(0, incoming, FALSE, FALSE);
            notificationCase(incoming == 0 ? -1 : incoming, incoming, FALSE, FALSE);
            notificationCase(777, incoming, FALSE, FALSE);
        }
        notificationCase(0, CAIAction::DIALOGUE, TRUE, TRUE);
        LONG markerTargetId = game->GetFixedOrderCharacterId(1);
        CGameObject* markerRaw = NULL;
        BYTE markerRc = objects->GetDeny(markerTargetId, CGameObjectArray::THREAD_ASYNCH, &markerRaw, INFINITE);
        if (markerRc == CGameObjectArray::SUCCESS && markerRaw->GetObjectType() == CGameObject::TYPE_SPRITE) {
            auto* target = static_cast<CGameSprite*>(markerRaw);
            CMarker savedTargetMarker = target->m_marker;
            CMarker savedSourceMarker = source->m_marker;
            source->m_currentActionId = 0;
            source->m_nModalState = 0;
            source->m_derivedStats.m_spellStates.reset();
            source->m_nUnselectableCounter = 0;
            source->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_PC;
            source->m_moraleFailure = FALSE;
            game->m_nTimeStop = 0;
            for (INT markerCase = 0; markerCase < 9; ++markerCase) {
                source->m_curAction = CAIAction::NULL_ACTION;
                source->m_bSelected = markerCase != 3;
                source->m_derivedStats.m_generalState = markerCase == 4 ? STATE_HELPLESS : 0;
                source->m_targetId = markerCase == 5 ? CGameObjectArray::INVALID_INDEX
                    : markerCase == 6 ? 0x7FFFFFFF : markerCase == 7 ? id
                    : markerCase == 8 ? container->m_id : markerTargetId;
                target->m_marker.m_type = CMarker::RECTICLE;
                SHORT initialCount = markerCase == 1 ? 2 : markerCase == 2 ? 0 : 1;
                target->m_marker.m_nRecticleCounter = initialCount;
                source->m_marker.m_type = CMarker::RECTICLE;
                source->m_marker.m_nRecticleCounter = markerCase == 7 ? 1 : 3;
                CAIAction incoming;
                incoming.m_actionID = markerCase == 0 ? CAIAction::NO_ACTION : CAIAction::WAIT;
                BOOL orderable = source->Orderable(FALSE);
                source->SetCurrAction(incoming);
                SHORT expectedCount = markerCase == 0 ? 0 : markerCase == 1 ? 1 : initialCount;
                BOOL passed = source->m_targetId == CGameObjectArray::INVALID_INDEX
                    && source->m_curAction.GetActionID() == incoming.GetActionID()
                    && orderable == (markerCase != 4)
                    && target->m_marker.m_nRecticleCounter == expectedCount
                    && target->m_marker.m_type == (markerCase == 0 ? CMarker::ELLIPSE : CMarker::RECTICLE)
                    && source->m_marker.m_nRecticleCounter == (markerCase == 7 ? 0 : 3)
                    && source->m_marker.m_type == (markerCase == 7 ? CMarker::ELLIPSE : CMarker::RECTICLE);
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"action-target-release\",\"case\":%d,\"count\":%d,\"passed\":%s}",
                    nStep, markerCase, target->m_marker.m_nRecticleCounter, passed ? "true" : "false");
                while (messages.GetCount() > before) delete messages.RemoveTail();
            }
            target->m_marker = savedTargetMarker;
            source->m_marker = savedSourceMarker;
        } else {
            valid = FALSE;
        }
        if (markerRc == CGameObjectArray::SUCCESS) objects->ReleaseDeny(markerTargetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        container->RemoveFromArea();
        {
            struct LifetimeProjectile : CProjectile {
                BOOL* destroyed;
                explicit LifetimeProjectile(BOOL* flag) : destroyed(flag) {}
                ~LifetimeProjectile() override { *destroyed = TRUE; }
            };
            CSpell guard(CResRef("SPWI112"));
            auto* resource = guard.pRes;
            BOOL resourceValid = resource != NULL && resource->m_pDimmKeyTableEntry != NULL;
            valid = valid && resourceValid;
            if (resourceValid) {
                auto* key = resource->m_pDimmKeyTableEntry;
                INT refs = key->field_10;
                INT requests = resource->m_nLockCount;
                source->m_nModalState = 0;
                source->m_currentActionId = 0;
                for (INT cleanupCase = 0; cleanupCase < 8; ++cleanupCase) {
                    INT kind = cleanupCase % 4;
                    BOOL idle = cleanupCase < 4;
                    source->m_curAction = CAIAction::NULL_ACTION;
                    source->m_curSpell = kind == 0 ? NULL : new CSpell();
                    if (kind >= 2) source->m_curSpell->SetResRef(CResRef("SPWI112"), kind == 3, TRUE);
                    BOOL destroyed = FALSE;
                    auto* projectile = kind == 0 ? NULL : new LifetimeProjectile(&destroyed);
                    source->m_curProjectile = projectile;
                    auto* item = new CItem();
                    source->m_curItem = item;
                    source->field_710C = 7;
                    source->field_5618 = 123;
                    CAIAction incoming;
                    incoming.m_actionID = idle ? CAIAction::NO_ACTION : CAIAction::WAIT;
                    source->SetCurrAction(incoming);
                    BOOL passed = source->m_curSpell == NULL && source->m_curItem == NULL
                        && item->cResRef == ""
                        && source->m_curProjectile == (idle ? projectile : NULL)
                        && destroyed == (!idle && projectile != NULL)
                        && source->field_710C == (idle ? 7 : 0)
                        && source->field_5618 == (idle ? 123 : 0)
                        && key->field_10 == refs && resource->m_nLockCount == requests;
                    valid = valid && passed;
                    ++checks;
                    Emit("{\"step\":%d,\"op\":\"action-resource-cleanup\",\"case\":%d,\"idle\":%s,\"destroyed\":%s,\"passed\":%s}",
                        nStep, cleanupCase, idle ? "true" : "false", destroyed ? "true" : "false", passed ? "true" : "false");
                    if (idle) delete projectile;
                    source->m_curProjectile = NULL;
                    delete item;
                    while (messages.GetCount() > before) delete messages.RemoveTail();
                }
            }
        }
        {
            const SHORT sequences[] = { CGameSprite::SEQ_DIE, CGameSprite::SEQ_SLEEP,
                CGameSprite::SEQ_TWITCH, CGameSprite::SEQ_WALK, CGameSprite::SEQ_ATTACK,
                CGameSprite::SEQ_CAST, CGameSprite::SEQ_READY, CGameSprite::SEQ_HEAD_TURN,
                CGameSprite::SEQ_AWAKE };
            for (INT test = 0; test < 36; ++test) {
                BOOL idle = test >= 27;
                SHORT sequence = sequences[test % 9];
                SHORT luck = test < 9 ? 8 : test < 18 ? -8 : 0;
                source->m_derivedStats.m_nLuck = luck;
                source->m_nSequence = sequence;
                source->m_noActionCount = 123;
                source->field_5616 = 99;
                CAIAction incoming;
                incoming.m_actionID = idle ? CAIAction::NO_ACTION : CAIAction::WAIT;
                source->SetCurrAction(incoming);
                BOOL preserve = idle || test % 9 < 4;
                SHORT expected = preserve ? sequence : source->GetIdleSequence();
                SHORT phase = source->field_5616;
                BOOL passed = source->m_nSequence == expected
                    && source->m_noActionCount == (idle ? 123 : 0)
                    && (idle ? phase == 0 : luck == 8 ? phase == 0
                        : luck == -8 ? phase == 7 : phase >= 0 && phase < 8);
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"action-idle-phase\",\"case\":%d,\"sequence\":%d,\"phase\":%d,\"passed\":%s}",
                    nStep, test, source->m_nSequence, phase, passed ? "true" : "false");
                while (messages.GetCount() > before) delete messages.RemoveTail();
            }
        }
        source->field_5616 = savedMovePhase;
        {
            const SHORT pauses[] = { -32768, -1, 0, 72, 73, 74, 75, 32767 };
            auto markerType = source->m_marker.m_type;
            auto markerCount = source->m_marker.m_nRecticleCounter;
            auto targetForce = source->m_marker.m_nRecticleForceRenderTarget;
            auto destForce = source->m_destMarker.m_nRecticleForceRender;
            for (INT test = 0; test < 16; ++test) {
                BOOL idle = test >= 8;
                source->m_userCommandPause = pauses[test % 8];
                source->m_marker.m_nRecticleForceRender = 99;
                CAIAction incoming;
                incoming.m_actionID = idle ? CAIAction::NO_ACTION : CAIAction::WAIT;
                source->SetCurrAction(incoming);
                BOOL passed = source->m_marker.m_nRecticleForceRender
                        == (!idle && pauses[test % 8] > 73 ? 15 : 99)
                    && source->m_userCommandPause == pauses[test % 8]
                    && source->m_marker.m_type == markerType
                    && source->m_marker.m_nRecticleCounter == markerCount
                    && source->m_marker.m_nRecticleForceRenderTarget == targetForce
                    && source->m_destMarker.m_nRecticleForceRender == destForce;
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"action-pause-marker\",\"case\":%d,\"force\":%d,\"passed\":%s}",
                    nStep, test, source->m_marker.m_nRecticleForceRender, passed ? "true" : "false");
                while (messages.GetCount() > before) delete messages.RemoveTail();
            }
        }
        source->field_5616 = savedMovePhase;
        {
            CSpell guard(CResRef("SPWI112"));
            auto* resource = guard.pRes;
            BOOL ready = resource != NULL && resource->m_pDimmKeyTableEntry != NULL;
            valid = valid && ready;
            if (ready) {
                auto* key = resource->m_pDimmKeyTableEntry;
                INT refs = key->field_10;
                INT requests = resource->m_nLockCount;
                BYTE expectedClass = 0;
                DWORD expectedSpec = 0;
                BYTE expectedLevel = 0;
                BOOL available = source->FindAvailableSpell(CResRef("SPWI112"), expectedClass, expectedSpec, expectedLevel);
                if (!available && guard.Demand()) {
                    expectedClass = guard.GetCasterType() == 1 ? CAIOBJECTTYPE_C_WIZARD : CAIOBJECTTYPE_C_CLERIC;
                    expectedLevel = static_cast<BYTE>(guard.GetLevel());
                    guard.Release();
                }
                LONG expectedPacked = expectedClass
                    | (game->GetRuleTables().GetSpecializationIndex(expectedClass, expectedSpec) << 8);
                const SHORT spells[] = { CAIAction::SPELL, CAIAction::SPELLPOINT,
                    CAIAction::FORCESPELL, CAIAction::FORCESPELLPOINT, CAIAction::REALLYFORCESPELL,
                    CAIAction::SPELLNODEC, CAIAction::SPELLPOINTNODEC };
                for (INT test = 0; test < 29; ++test) {
                    INT kind = test % 4;
                    CAIAction incoming;
                    incoming.m_actionID = test == 28 ? CAIAction::WAIT : spells[test / 4];
                    incoming.m_specificID = 2112;
                    incoming.m_specificID2 = kind == 2 ? 0x1234000B : 0;
                    incoming.m_specificID3 = kind == 2 ? 0x12340001 : 0;
                    incoming.m_string1 = kind == 1 ? "" : kind == 3 ? "ZRCNONE" : "SPWI112";
                    LONG packed = kind == 2 ? incoming.m_specificID2 : kind == 3 || test == 28 ? 0 : expectedPacked;
                    LONG level = kind == 2 ? incoming.m_specificID3 : kind == 3 || test == 28 ? 0 : expectedLevel;
                    source->SetCurrAction(incoming);
                    BOOL passed = source->m_curAction.m_actionID == incoming.m_actionID
                        && source->m_curAction.m_specificID == 2112
                        && source->m_curAction.m_specificID2 == packed
                        && source->m_curAction.m_specificID3 == level
                        && source->m_curAction.m_string1 == incoming.m_string1
                        && incoming.m_specificID2 == (kind == 2 ? 0x1234000B : 0)
                        && source->m_curSpell == NULL && key->field_10 == refs
                        && resource->m_nLockCount == requests;
                    valid = valid && passed;
                    ++checks;
                    Emit("{\"step\":%d,\"op\":\"action-spell-metadata\",\"case\":%d,\"available\":%s,\"class\":%ld,\"level\":%ld,\"passed\":%s}",
                        nStep, test, available ? "true" : "false", source->m_curAction.m_specificID2,
                        source->m_curAction.m_specificID3, passed ? "true" : "false");
                    while (messages.GetCount() > before) delete messages.RemoveTail();
                }
            }
        }
        source->field_5616 = savedMovePhase;
        source->m_noActionCount = savedNoActionCount;
        {
            auto savedSpellLists = source->m_spells;
            auto savedDomainList = source->m_domainSpells;
            auto savedSpecialization = source->m_baseStats.m_specialization;
            CSpell mageGuard(CResRef("SPWI112"));
            CSpell priestGuard(CResRef("SPPR101"));
            UINT mageId = 0, priestId = 0;
            BOOL ready = game->m_spells.Find(CResRef("SPWI112"), mageId)
                && game->m_spells.Find(CResRef("SPPR101"), priestId)
                && mageGuard.pRes != NULL && priestGuard.pRes != NULL
                && mageGuard.pRes->m_pDimmKeyTableEntry != NULL
                && priestGuard.pRes->m_pDimmKeyTableEntry != NULL;
            valid = valid && ready;
            if (ready) {
                auto* mageKey = mageGuard.pRes->m_pDimmKeyTableEntry;
                auto* priestKey = priestGuard.pRes->m_pDimmKeyTableEntry;
                INT mageRefs = mageKey->field_10, priestRefs = priestKey->field_10;
                INT mageRequests = mageGuard.pRes->m_nLockCount;
                INT priestRequests = priestGuard.pRes->m_nLockCount;
                const SHORT actions[] = { CAIAction::SPELL, CAIAction::SPELLPOINT,
                    CAIAction::FORCESPELL, CAIAction::FORCESPELLPOINT, CAIAction::REALLYFORCESPELL,
                    CAIAction::SPELLNODEC, CAIAction::SPELLPOINTNODEC };
                for (INT test = 0; test < 28; ++test) {
                    INT kind = test % 4;
                    for (UINT c = 0; c < CSPELLLIST_NUM_CLASSES; ++c) source->m_spells.Get(c)->Clear();
                    source->m_domainSpells.Clear();
                    source->m_baseStats.m_specialization = game->GetRuleTables().GetSpecializationMask(CAIOBJECTTYPE_C_CLERIC, 1);
                    if (kind == 1 || kind == 2) {
                        BYTE caster = kind == 1 ? CAIOBJECTTYPE_C_WIZARD : CAIOBJECTTYPE_C_CLERIC;
                        source->m_spells.Get(game->GetSpellcasterIndex(caster))->Add(kind == 1 ? mageId : priestId, 0, 1, 1, 0);
                    } else if (kind == 3) {
                        source->m_domainSpells.Add(priestId, 0, 1, 1, 0);
                    }
                    CAIAction incoming;
                    incoming.m_actionID = actions[test / 4];
                    incoming.m_string1 = kind == 1 ? "SPWI112" : "SPPR101";
                    // Domain selection bypasses ordinary lists when specialization is nonzero.
                    incoming.m_specificID2 = kind == 3 ? 0x103 : 0;
                    incoming.m_specificID3 = 0;
                    source->SetCurrAction(incoming);
                    LONG expectedClass = kind == 1 ? CAIOBJECTTYPE_C_WIZARD : kind == 3 ? 0x103 : CAIOBJECTTYPE_C_CLERIC;
                    UINT index = 0;
                    BOOL countPreserved = TRUE;
                    if (kind == 1 || kind == 2) {
                        auto* list = source->m_spells.Get(game->GetSpellcasterIndex(static_cast<BYTE>(expectedClass)))->GetSpellsAtLevel(0);
                        countPreserved = list->Find(kind == 1 ? mageId : priestId, index) && list->Get(index)->m_nCurrent == 1;
                    } else if (kind == 3) {
                        auto* list = source->m_domainSpells.GetSpellsAtLevel(0);
                        countPreserved = list->Find(priestId, index) && list->Get(index)->m_nCurrent == 1;
                    }
                    BOOL passed = source->m_curAction.m_specificID2 == expectedClass
                        && source->m_curAction.m_specificID3 == 1 && source->m_curSpell == NULL
                        && countPreserved && mageKey->field_10 == mageRefs && priestKey->field_10 == priestRefs
                        && mageGuard.pRes->m_nLockCount == mageRequests && priestGuard.pRes->m_nLockCount == priestRequests;
                    valid = valid && passed;
                    ++checks;
                    Emit("{\"step\":%d,\"op\":\"action-spell-lists\",\"case\":%d,\"class\":%ld,\"level\":%ld,\"passed\":%s}",
                        nStep, test, source->m_curAction.m_specificID2, source->m_curAction.m_specificID3, passed ? "true" : "false");
                    while (messages.GetCount() > before) delete messages.RemoveTail();
                }
            }
            source->m_spells = savedSpellLists;
            source->m_domainSpells = savedDomainList;
            source->m_baseStats.m_specialization = savedSpecialization;
        }
        source->field_5616 = savedMovePhase;
        source->m_noActionCount = savedNoActionCount;
        {
            auto* area = source->m_pArea;
            auto loaded = area->m_bAreaLoaded;
            auto saveAllowed = area->field_B16;
            auto loading = game->m_bInLoadGame;
            auto selected = source->m_equipment.m_selectedWeapon;
            auto ability = source->m_equipment.m_selectedWeaponAbility;
            auto* slot = source->m_equipment.m_items[0];
            auto* fist = source->m_equipment.m_items[CGameSpriteEquipment::SLOT_FIST];
            CItem empty;
            BOOL ready = fist != NULL;
            if (ready) {
                fist->Demand();
                ready = fist->GetAbility(0) != NULL;
                fist->Release();
            }
            valid = valid && ready;
            if (ready) {
                const BYTE sequences[] = { CGameSprite::SEQ_ATTACK, CGameSprite::SEQ_SHOOT,
                    CGameSprite::SEQ_CAST, 255 };
                game->m_bInLoadGame = FALSE;
                source->m_equipment.m_selectedWeapon = 0;
                for (INT test = 0; test < 64; ++test) {
                    INT areaKind = test / 16;
                    INT weaponKind = test / 4 % 4;
                    BYTE sequence = sequences[test % 4];
                    BYTE initial = sequence;
                    source->m_pArea = areaKind == 0 ? NULL : area;
                    area->m_bAreaLoaded = areaKind >= 2;
                    area->field_B16 = areaKind == 3;
                    source->m_equipment.m_items[0] = weaponKind == 0 ? NULL : weaponKind == 1 ? &empty : fist;
                    source->m_equipment.m_selectedWeaponAbility = weaponKind == 3 ? 0xFFFF : 0;
                    source->CheckSequence(sequence);
                    BYTE expected = test % 4 >= 2 || weaponKind == 2 ? initial
                        : areaKind == 3 ? CGameSprite::SEQ_READY : CGameSprite::SEQ_HEAD_TURN;
                    BOOL passed = sequence == expected;
                    valid = valid && passed;
                    ++checks;
                    Emit("{\"step\":%d,\"op\":\"check-attack-sequence\",\"case\":%d,\"sequence\":%d,\"passed\":%s}",
                        nStep, test, sequence, passed ? "true" : "false");
                }
            }
            source->m_pArea = area;
            area->m_bAreaLoaded = loaded;
            area->field_B16 = saveAllowed;
            game->m_bInLoadGame = loading;
            source->m_equipment.m_selectedWeapon = selected;
            source->m_equipment.m_selectedWeaponAbility = ability;
            source->m_equipment.m_items[0] = slot;
        }
        {
            struct ArrivalState { INT callback = 0, delivered = 0, removed = 0, destroyed = 0, virtualRemove = 0; };
            struct ArrivalProjectile : CProjectile {
                ArrivalState* state;
                INT* clock;
                ArrivalProjectile(ArrivalState* s, INT* c) : state(s), clock(c) {}
                ~ArrivalProjectile() override { state->destroyed = ++*clock; }
                void CallBack() override { state->callback = ++*clock; }
                void DeliverEffects() override { state->delivered = ++*clock; }
                void RemoveFromArea() override { state->removed = ++*clock; }
                void RemoveSelf() override { ++state->virtualRemove; }
            };
            for (INT test = 0; test < 9; ++test) {
                INT callbackKind = test / 3;
                INT attachedKind = test % 3;
                INT clock = 0;
                ArrivalState mainState, callbackState, attachedState;
                auto* missile = new ArrivalProjectile(&mainState, &clock);
                auto* callback = callbackKind == 1 ? new ArrivalProjectile(&callbackState, &clock) : NULL;
                auto* attached = attachedKind == 1 ? new ArrivalProjectile(&attachedState, &clock) : NULL;
                BOOL mainRegistered = objects->Add(&missile->m_id, missile, INFINITE) == CGameObjectArray::SUCCESS;
                BOOL callbackRegistered = callback != NULL && objects->Add(&callback->m_id, callback, INFINITE) == CGameObjectArray::SUCCESS;
                BOOL attachedRegistered = attached != NULL && objects->Add(&attached->m_id, attached, INFINITE) == CGameObjectArray::SUCCESS;
                BOOL ready = mainRegistered && (callback == NULL || callbackRegistered) && (attached == NULL || attachedRegistered);
                BOOL passed = ready;
                if (ready) {
                    missile->m_callBackProjectile = callbackKind == 0 ? CGameObjectArray::INVALID_INDEX
                        : callbackKind == 1 ? callback->m_id : 0x7FFFFFFF;
                    missile->m_nTargetId = attachedKind == 0 ? CGameObjectArray::INVALID_INDEX
                        : attachedKind == 1 ? attached->m_id : 0x7FFFFFFF;
                    missile->OnArrival();
                    BOOL blocked = callbackKind == 2;
                    passed = blocked ? mainState.delivered == 0 && mainState.removed == 0 && mainState.destroyed == 0
                        : mainState.delivered > 0 && mainState.removed > mainState.delivered && mainState.destroyed > mainState.removed;
                    passed = passed && callbackState.callback == (callbackKind == 1 ? 1 : 0)
                        && attachedState.virtualRemove == 0
                        && (attachedKind != 1 || (blocked ? attachedState.destroyed == 0
                            : attachedState.removed > 0 && attachedState.destroyed > attachedState.removed
                                && mainState.delivered > attachedState.destroyed));
                }
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"projectile-arrival\",\"case\":%d,\"callback\":%d,\"delivered\":%d,\"destroyed\":%d,\"passed\":%s}",
                    nStep, test, callbackState.callback, mainState.delivered, mainState.destroyed, passed ? "true" : "false");
                if (!mainState.destroyed) { if (mainRegistered) missile->CProjectile::RemoveSelf(); else delete missile; }
                if (callback != NULL) { if (callbackRegistered) callback->CProjectile::RemoveSelf(); else delete callback; }
                if (attached != NULL && !attachedState.destroyed) { if (attachedRegistered) attached->CProjectile::RemoveSelf(); else delete attached; }
            }
        }
        {
            struct FlightProjectile : CProjectileTravelling {
                INT arrived = 0, removed = 0, aimed = 0;
                CPoint aim;
                FlightProjectile() : CProjectileTravelling(CResRef("ARARROW")) {}
                void OnArrival() override { ++arrived; }
                void RemoveSelf() override { ++removed; }
                void AimAtPoint(int x, int y) override { ++aimed; aim = CPoint(x, y); }
                void Prepare(CGameArea* area, LONG target, INT test) {
                    m_pArea = test == 5 ? NULL : area;
                    m_id = 123;
                    m_pos = CPoint(160, 120);
                    m_posZ = 30;
                    m_targetX = test == 8 ? 160 : 320;
                    m_targetY = test == 8 ? 120 : 240;
                    m_targetId = test == 4 ? 0x7FFFFFFF : test == 6 ? CGameObjectArray::INVALID_INDEX : target;
                    m_velocity = 1;
                    m_flightDistSq = test == 11 ? 0 : 102400;
                    m_lifetime = test == 7 ? 0 : 5;
                    m_hasDrift = test == 2 || test == 3;
                    m_nDeltaZ = test % 2 ? -10 : 10;
                    m_nDeltaZLast = 2;
                    m_bSparkleTrail = FALSE;
                }
                SHORT Life() const { return m_lifetime; }
            };
            auto stop = game->m_nTimeStop;
            auto caster = game->m_nTimeStopCaster;
            for (INT test = 0; test < 12; ++test) {
                FlightProjectile missile;
                missile.Prepare(source->GetArea(), id, test);
                game->m_nTimeStop = test == 9 || test == 10 ? 1 : 0;
                game->m_nTimeStopCaster = test == 10 ? 123 : -1;
                missile.AIUpdate();
                BOOL arrived = test == 8 || test == 11;
                BOOL removed = test == 4 || test == 5 || test == 7;
                BOOL frozen = test == 9;
                BOOL aimed = !arrived && !removed && !frozen;
                LONG height = test == 0 || test == 6 || test == 10 ? 33 : test == 1 ? 23 : 30;
                // Untargeted travel does not interpolate height.
                if (test == 6) height = 30;
                BOOL passed = missile.arrived == arrived && missile.removed == removed
                    && missile.aimed == aimed && missile.m_posZ == height
                    && missile.Life() == (frozen || arrived ? 5 : test == 7 ? -1 : 4)
                    && (!aimed || missile.aim == (test == 6 ? CPoint(320, 240) : source->GetPos()));
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"projectile-flight\",\"case\":%d,\"height\":%ld,\"arrived\":%d,\"removed\":%d,\"passed\":%s}",
                    nStep, test, missile.m_posZ, missile.arrived, missile.removed, passed ? "true" : "false");
            }
            game->m_nTimeStop = stop;
            game->m_nTimeStopCaster = caster;
        }
        {
            struct LaunchProjectile : CProjectileTravelling {
                LaunchProjectile(BOOL sound, BOOL loop) : CProjectileTravelling(CResRef("ARARROW")) {
                    m_fireSoundRef = CResRef(sound ? "TRA_02" : "");
                    m_loopFireSound = loop;
                    m_bHasHeight = FALSE;
                    m_bSparkleTrail = FALSE;
                }
                BOOL SoundMatches(BOOL sound, BOOL loop, CGameArea* area) {
                    return m_sound.GetLooping() == (sound && loop)
                        && !m_sound.m_bFireForget
                        && (!sound || (m_sound.m_nChannel == 15
                            && m_sound.m_nArea == reinterpret_cast<DWORD>(area)
                            && m_sound.m_nXCoordinate == m_pos.x
                            && m_sound.m_nYCoordinate == m_pos.y));
                }
            };
            for (INT test = 0; test < 4; ++test) {
                BOOL sound = (test & 1) != 0;
                BOOL loop = (test & 2) != 0;
                LaunchProjectile* missile = new LaunchProjectile(sound, loop);
                missile->Fire(source->GetArea(), id, CGameObjectArray::INVALID_INDEX,
                    source->GetPos(), 0, 0);
                CGameObject* registered = NULL;
                BOOL found = objects->GetShare(missile->m_id, CGameObjectArray::THREAD_ASYNCH,
                    &registered, INFINITE) == CGameObjectArray::SUCCESS;
                BOOL passed = found && registered == missile && missile->GetArea() == source->GetArea()
                    && missile->m_posZ == 0 && missile->SoundMatches(sound, loop, source->GetArea());
                if (found) objects->ReleaseShare(missile->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"projectile-launch\",\"case\":%d,\"passed\":%s}",
                    nStep, test, passed ? "true" : "false");
                missile->CProjectile::RemoveSelf();
            }
        }
        {
            struct HeightProjectile : CProjectileTravelling {
                HeightProjectile(BOOL height) : CProjectileTravelling(CResRef("ARARROW")) {
                    m_fireSoundRef = CResRef("");
                    m_bHasHeight = height;
                    m_bSparkleTrail = FALSE;
                    m_distLifetime = 1;
                    m_velocity = 20;
                    m_nDeltaZ = 37;
                    m_nDeltaZLast = -44;
                    m_flightDistSq = 999;
                }
                BOOL Matches(SHORT delta, INT distance, SHORT lifetime, CPoint target) const {
                    return m_nDeltaZ == delta && m_nDeltaZLast == delta
                        && m_flightDistSq == distance && m_lifetime == lifetime
                        && m_targetX == target.x && m_targetY == target.y;
                }
            };
            LONG savedHeight = source->m_posZ;
            for (INT test = 0; test < 10; ++test) {
                BOOL height = (test & 1) != 0;
                BOOL creature = test < 6;
                BOOL missing = test >= 8;
                source->m_posZ = test < 2 ? -13 : test < 4 ? 0 : 19;
                CRect bounds;
                CPoint reference;
                source->GetAnimation()->CalculateFxRect(bounds, reference, source->m_posZ);
                LONG launchHeight = height ? source->GetAnimation()->GetCastHeight() : 0;
                CPoint target = source->GetPos();
                if (test == 7) target += CPoint(160, 120);
                HeightProjectile* missile = new HeightProjectile(height);
                missile->Fire(source->GetArea(), test == 9 ? 0x7FFFFFFF : id,
                    test == 8 ? 0x7FFFFFFF : creature ? id : CGameObjectArray::INVALID_INDEX,
                    target, 123, 7);
                BOOL registered = missile->m_id != CGameObjectArray::INVALID_INDEX;
                BOOL passed;
                if (missing) {
                    passed = !registered && missile->GetArea() == NULL && missile->m_pos == CPoint(-1, -1);
                } else {
                    INT dx = target.x - missile->m_pos.x;
                    INT dy = target.y - missile->m_pos.y;
                    INT distance = dx * dx + dy * dy * 16 / 9;
                    SHORT lifetime = static_cast<SHORT>(static_cast<INT>(std::sqrt(static_cast<double>(distance))) / 20 + 1);
                    SHORT delta = creature ? static_cast<SHORT>(launchHeight - bounds.Height() / 2) : 0;
                    passed = registered && missile->GetArea() == source->GetArea()
                        && missile->m_posZ == launchHeight
                        && missile->Matches(delta, creature ? distance : 1, lifetime, target);
                }
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"projectile-launch-height\",\"case\":%d,\"passed\":%s}",
                    nStep, test, passed ? "true" : "false");
                if (registered) missile->CProjectile::RemoveSelf(); else delete missile;
            }
            source->m_posZ = savedHeight;
        }
        {
            SHORT direction = source->m_nDirection;
            const CRect& ellipse = source->GetAnimation()->GetEllipseRect();
            INT x2 = static_cast<SHORT>(static_cast<SHORT>(ellipse.right) * static_cast<SHORT>(ellipse.right));
            INT y2 = static_cast<SHORT>(static_cast<SHORT>(ellipse.bottom) * static_cast<SHORT>(ellipse.bottom));
            INT diagonal = static_cast<SHORT>(std::sqrt(static_cast<double>(x2 * y2 / (x2 + y2))));
            CPoint offsets[8] = {
                CPoint(0, ellipse.bottom >> 1), CPoint(-2 * diagonal, 2 * diagonal),
                CPoint(-2 * ellipse.right, 1), CPoint(-2 * diagonal, -2 * diagonal),
                CPoint(0, -2 * ellipse.bottom), CPoint(2 * diagonal, -2 * diagonal),
                CPoint(2 * ellipse.right, 1), CPoint(2 * diagonal, 2 * diagonal)
            };
            for (INT test = 0; test < 49; ++test) {
                source->m_nDirection = test % 16;
                BYTE offset = test < 16 ? 0 : test < 32 ? 1 : 255;
                CPoint point(123, -456);
                CProjectile::GetStart(test == 48 ? 0x7FFFFFFF : id, point, offset);
                CPoint expected = test == 48 ? CPoint(123, -456) : source->GetPos();
                if (test >= 16 && test < 48) expected += offsets[(test % 16) / 2];
                BOOL passed = point == expected;
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"projectile-start\",\"case\":%d,\"passed\":%s}",
                    nStep, test, passed ? "true" : "false");
            }
            CRect savedEllipse = ellipse;
            CRect& mutableEllipse = const_cast<CRect&>(ellipse);
            mutableEllipse = CRect(-256, -12, 256, 12);
            // 256 squared truncates to zero in the PE's signed SHORT product.
            CPoint wideOffsets[8] = {
                CPoint(0, 6), CPoint(0, 0), CPoint(-512, 1), CPoint(0, 0),
                CPoint(0, -24), CPoint(0, 0), CPoint(512, 1), CPoint(0, 0)
            };
            for (INT facing = 0; facing < 16; ++facing) {
                source->m_nDirection = static_cast<SHORT>(facing);
                CPoint point;
                CProjectile::GetStart(id, point, 1);
                BOOL passed = point == source->GetPos() + wideOffsets[facing / 2];
                valid = valid && passed;
                ++checks;
                Emit("{\"step\":%d,\"op\":\"projectile-start-wide\",\"case\":%d,\"passed\":%s}",
                    nStep, facing, passed ? "true" : "false");
            }
            mutableEllipse = savedEllipse;
            source->m_nDirection = direction;
        }
        {
            struct BatchCanary : CMessage {
                INT value, *order, *destroyed;
                BatchCanary(INT n, INT* run, INT* death) : CMessage(0, 0), value(n), order(run), destroyed(death) {}
                ~BatchCanary() override { ++*destroyed; }
                void Run() override { *order = *order * 10 + value; }
            };
            for (INT test = 0; test < 6; ++test) {
                INT count = test / 2 == 0 ? 0 : test / 2 == 1 ? 1 : 3;
                INT order = 0, destroyed = 0;
                CMessage105::Node head = { &head, &head, NULL };
                CMessage105::Node nodes[3];
                for (INT i = 0; i < count; ++i) {
                    nodes[i] = { &head, head.m_pPrev, new BatchCanary(i + 1, &order, &destroyed) };
                    head.m_pPrev->m_pNext = &nodes[i];
                    head.m_pPrev = &nodes[i];
                }
                CMessage105* batch = new CMessage105(&head, id, id);
                for (INT i = 0; i < count; ++i) nodes[i].m_pMessage = NULL;
                if (test & 1) batch->Run();
                BOOL passed = batch->m_nCount == count && destroyed == 0
                    && order == ((test & 1) ? (count == 3 ? 123 : count) : 0);
                delete batch;
                passed = passed && destroyed == count;
                valid = valid && passed; ++checks;
                Emit("{\"step\":%d,\"op\":\"impact-batch-ownership\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
            }
            CProjectile* ordinary = new CProjectile();
            BOOL ordinaryRegistered = objects->Add(&ordinary->m_id, ordinary, INFINITE) == CGameObjectArray::SUCCESS;
            auto savedImmunity = source->m_derivedStats.m_cImmunitiesSpellLevel.m_levels[5];
            for (INT test = 0; test < 12; ++test) {
                INT mode = test / 3, count = test % 3 == 2 ? 3 : test % 3;
                CProjectile missile;
                missile.m_sourceId = id;
                missile.m_targetId = mode == 1 ? 0x7FFFFFFF : mode == 2 ? ordinary->m_id : id;
                missile.m_nSpellLevel = 5;
                missile.m_casterResRef = CResRef("");
                source->m_derivedStats.m_cImmunitiesSpellLevel.m_levels[5] = mode == 3 ? 1 : 0;
                CGameEffect* effects[3] = {};
                for (INT i = 0; i < count; ++i) {
                    effects[i] = new CGameEffect();
                    missile.m_effectList.AddTail(effects[i]);
                }
                INT queued = messages.GetCount();
                missile.DeliverEffects();
                BOOL passed = ordinaryRegistered && missile.m_effectList.GetCount() == (mode == 1 ? count : 0)
                    && messages.GetCount() == queued + (mode == 0 ? 1 : 0);
                if (mode == 0 && messages.GetCount() > queued) {
                    CMessage105* batch = dynamic_cast<CMessage105*>(messages.GetTail());
                    passed = passed && batch != NULL && batch->m_nCount == count;
                    if (batch != NULL) {
                        CMessage105::Node* node = batch->m_pNodeHead->m_pNext;
                        for (INT i = 0; i < count && node != batch->m_pNodeHead; ++i) {
                            CMessageAddEffect* child = dynamic_cast<CMessageAddEffect*>(node->m_pMessage);
                            passed = passed && child != NULL && child->m_effect == effects[i]
                                && child->GetSource() == id && child->GetTarget() == id;
                            node = node->m_pNext;
                        }
                        passed = passed && node == batch->m_pNodeHead;
                    }
                }
                valid = valid && passed; ++checks;
                Emit("{\"step\":%d,\"op\":\"impact-delivery\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
                while (messages.GetCount() > queued) delete messages.RemoveTail();
                missile.ClearEffects();
            }
            source->m_derivedStats.m_cImmunitiesSpellLevel.m_levels[5] = savedImmunity;
            if (ordinaryRegistered) ordinary->CProjectile::RemoveSelf(); else delete ordinary;
        }
        {
            struct BAMProbe : CProjectileBAM {
                using CProjectileBAM::m_vidCell;
                using CProjectileBAM::m_sequenceDelay;
                using CProjectileBAM::m_initialDelay;
                INT delivered = 0, arrived = 0;
                BAMProbe() : CProjectileBAM(CResRef("ARARROW"), CResRef(""), 0, 0, IcewindCVisualEffect()) {}
                BAMProbe(BYTE delay, BYTE initial, const IcewindCVisualEffect& visual)
                    : CProjectileBAM(CResRef("ARARROW"), CResRef("TRA_02"), delay, initial, visual) {}
                void DeliverEffects() override { ++delivered; }
                void OnArrival() override { ++arrived; }
                BOOL ConstructorState(BYTE initial, const IcewindCVisualEffect& visual) {
                    return m_initialDelay == initial && m_vidCell.m_nCurrentFrame == 0
                        && m_sourceId == 0 && m_targetId == 0 && m_bHasHeight && !m_bSparkleTrail
                        && m_fireSoundRef == CResRef("TRA_02") && m_arrivalSoundRef == CResRef("")
                        && m_sound.GetResRef() == CResRef("TRA_02") && m_sound.m_nChannel == 4 && m_sound.m_nArea == 0
                        && m_bitmap.m_nBitCount == 8 && !m_bitmap.m_bDoubleSize
                        && !field_326 && !field_32A && !field_32B
                        && memcmp(&m_visualEffect, &visual, sizeof(visual)) == 0;
                }
                void PrepareRender(CGameArea* area, CPoint pos, BYTE initial) {
                    m_pArea = area; m_pos = pos; m_posZ = 0; m_listType = CGameObject::LIST_FRONT;
                    m_initialDelay = initial; m_vidCell.SequenceSet(2); m_vidCell.FrameSet(0);
                }
                void Prepare(INT test) {
                    m_id = 123;
                    m_pos = CPoint(160, 120); m_posZ = 30;
                    m_initialDelay = test <= 2 ? 3 : test == 11 ? 255 : 0;
                    m_sequenceDelay = test == 6 ? 1 : test == 7 ? 2 : test == 10 ? 255 : 0;
                    m_vidCell.SequenceSet(test == 4 || test == 5 ? 2 : 0);
                    m_vidCell.FrameSet(test == 5 || test == 6 ? 32767 : test == 4 || test == 7 || test == 10 ? -1 : 77);
                    m_sound.SetCoordinates(-1, -2, -3);
                }
                BOOL Matches(INT initial, INT delay, INT sequence, INT frame, BOOL sound) {
                    return m_initialDelay == initial && m_sequenceDelay == delay
                        && m_vidCell.m_nCurrentSequence == sequence && m_vidCell.m_nCurrentFrame == frame
                        && m_sound.m_nXCoordinate == (sound ? 160 : -1)
                        && m_sound.m_nYCoordinate == (sound ? 120 : -2)
                        && m_sound.m_nZCoordinate == (sound ? 30 : -3);
                }
            };
            IcewindCVisualEffect visual;
            visual.m_bTintEnabled = 1; visual.m_bCopyFromBack = 1; visual.field_2 = 17;
            visual.m_bTransparent = 1; visual.m_nTransValue = 93; visual.m_dwFlags = 0x42;
            BAMProbe measure;
            BYTE length = measure.m_vidCell.GetSequenceLength(2, FALSE);
            for (INT test = 0; test < 3; ++test) {
                BYTE delay = test == 0 ? 0 : test == 1 ? length : 255;
                BYTE initial = test == 0 ? 0 : test == 1 ? 1 : 255;
                BAMProbe missile(delay, initial, visual);
                BOOL passed = missile.ConstructorState(initial, visual)
                    && missile.m_sequenceDelay == (delay > length ? delay - length : 0)
                    && missile.m_vidCell.m_nCurrentSequence == (delay > length ? 0 : 2)
                    && missile.m_sound.m_nXCoordinate == 0 && missile.m_sound.m_nYCoordinate == 0;
                valid = valid && passed; ++checks;
                Emit("{\"step\":%d,\"op\":\"bam-constructor\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
            }
            CGameArea* renderArea = source->GetArea();
            CInfinity* infinity = renderArea->GetInfinity();
            auto savedX = infinity->nCurrentX; auto savedY = infinity->nCurrentY;
            for (INT test = 0; test < 4; ++test) {
                BAMProbe missile;
                missile.PrepareRender(renderArea, test == 1 ? CPoint(-32, -32) : source->m_pos, test == 0 ? 1 : 0);
                if (test == 2) { infinity->nCurrentX = 1000000; infinity->nCurrentY = 1000000; }
                // NULL deliberately verifies that Render uses its own area.
                missile.Render(NULL, g_pBaldurChitin->GetCurrentVideoMode(), 0);
                infinity->nCurrentX = savedX; infinity->nCurrentY = savedY;
                BOOL passed = missile.m_initialDelay == (test == 0 ? 1 : 0)
                    && missile.m_vidCell.m_nCurrentFrame == 0 && missile.delivered == 0 && missile.arrived == 0;
                valid = valid && passed; ++checks;
                Emit("{\"step\":%d,\"op\":\"bam-render\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
            }
            for (INT test = 0; test < 4; ++test) {
                BAMProbe* missile = new BAMProbe();
                missile->Fire(source->GetArea(), id, test == 3 ? 0x7FFFFFFF : id, CPoint(-700, -800), test - 1, 0);
                BOOL registered = missile->m_id != CGameObjectArray::INVALID_INDEX;
                BOOL passed = test == 3 ? !registered && missile->delivered == 0 : registered
                    && missile->GetArea() == source->GetArea() && missile->m_posZ == test - 1
                    && missile->m_targetX == source->m_pos.x && missile->m_targetY == source->m_pos.y
                    && missile->m_posAccumX == missile->m_pos.x * 1024
                    && missile->m_posAccumY == missile->m_pos.y * 4096 / 3 && missile->delivered == 1;
                valid = valid && passed; ++checks;
                Emit("{\"step\":%d,\"op\":\"bam-launch\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
                if (registered) missile->CProjectile::RemoveSelf(); else delete missile;
            }
            auto stop = game->m_nTimeStop; auto caster = game->m_nTimeStopCaster;
            for (INT test = 0; test < 12; ++test) {
                BAMProbe missile; missile.Prepare(test);
                game->m_nTimeStop = test == 0 || test == 1 || test == 8 || test == 9 ? 1 : 0;
                game->m_nTimeStopCaster = test == 1 || test == 9 ? 123 : -1;
                missile.AIUpdate();
                BOOL passed = test == 0 ? missile.Matches(3, 0, 0, 77, FALSE)
                    : test == 1 || test == 2 ? missile.Matches(2, 0, 0, 77, FALSE)
                    : test == 4 ? missile.Matches(0, 0, 2, 0, TRUE)
                    : test == 5 ? missile.Matches(0, 0, 2, 32767, FALSE)
                    : test == 6 ? missile.Matches(0, 0, 1, 0, TRUE)
                    : test == 7 ? missile.Matches(0, 1, 0, 0, TRUE)
                    : test == 8 ? missile.Matches(0, 0, 0, 77, FALSE)
                    : test == 10 ? missile.Matches(0, 254, 0, 0, TRUE)
                    : test == 11 ? missile.Matches(254, 0, 0, 77, FALSE)
                    : missile.Matches(0, 0, 2, 0, TRUE);
                passed = passed && missile.arrived == (test == 5);
                valid = valid && passed; ++checks;
                Emit("{\"step\":%d,\"op\":\"bam-flight\",\"case\":%d,\"passed\":%s}", nStep, test, passed ? "true" : "false");
            }
            game->m_nTimeStop = stop; game->m_nTimeStopCaster = caster;
        }
        source->m_userCommandPause = savedActionUserPause;
        source->m_marker.m_nRecticleForceRender = savedForceRender;
        source->m_noActionCount = savedNoActionCount;
        source->m_derivedStats.m_nLuck = savedActionLuck;
        source->m_curSpell = savedCurrentSpell;
        source->m_curItem = savedCurrentItem;
        source->m_curProjectile = savedCurrentProjectile;
        source->field_710C = savedCastPhase;
        source->field_5618 = savedActionPhase;
        source->m_targetId = savedActionTarget;
        source->m_currentActionId = savedTrackedAction;
        source->field_580 = savedTrapFlag;
        source->m_nModalState = savedModal;
        source->m_equipment.m_selectedWeapon = savedWeapon;
        source->m_equipment.m_selectedWeaponAbility = savedAbility;
        source->m_nTempSelectedWeapon = savedTempWeapon;
        source->m_nTempSelectedWeaponAbility = savedTempAbility;
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        source->m_derivedStats.m_generalState = savedState;
        source->m_baseStats.m_generalState = savedBaseState;
        source->m_derivedStats.m_spellStates = savedSpells;
        source->m_derivedStats.m_bCasterHold = savedHold;
        source->m_baseStats.m_flags = savedFlags;
        source->m_baseStats.field_2F0[3] = savedImmunity;
        source->m_posStart = savedStart;
        source->m_targetPoint = savedTargetPoint;
        source->m_groupMove = savedGroupMove;
        source->m_active = savedSpriteActive;
        source->m_activeAI = savedAIActive;
        source->m_activeImprisonment = savedImprisonmentActive;
        source->m_nUnselectableCounter = savedUnselectable;
        source->SetSequence(static_cast<BYTE>(savedSequence));
        source->m_bSelected = savedSelected;
        source->m_typeAI.m_nEnemyAlly = savedEA;
        source->m_bAllowDialogInterrupt = savedAllow;
        source->m_derivedStats.m_bIgnoreDialogPause = savedIgnore;
        source->m_inCutScene = source->m_moraleFailure = FALSE;
        source->field_3428 = 0;
        source->field_558E = TRUE;
        game->m_nTimeStop = savedStop;
        game->m_nTimeStopCaster = savedCaster;
        game->GetWorldTimer()->m_active = savedActive;
        game->GetGameSave()->m_cutScene = savedCutscene;
        if (fixture != NULL) {
            instants.m_idList.RemoveAt(instants.m_idList.Find(fixture));
            delete fixture;
        }
        instants.m_faster = savedFaster;
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"actiongates\",\"checks\":%d,\"rulesRestored\":true,\"disposable\":true,\"valid\":%s}",
            nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "common action resolution mismatch"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "scriptschedule") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and party actor");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        if (source->m_pArea == NULL || source->m_pPath != NULL || source->m_currentSearchRequest != NULL) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires idle actor in an area");
            return;
        }
        CAIScript script;
        CAICondition condition;
        CAIResponse response;
        response.m_weight = 100;
        response.m_responseNum = 1;
        response.Add(CAIAction(CAIAction::WAIT, 9876));
        CAIResponseSet responses;
        responses.Add(response);
        script.Add(condition, responses);
        CAIScript* savedScripts[7] = { source->m_overrideScript, source->m_special1Script,
            source->m_teamScript, source->m_special2Script, source->m_combatScript,
            source->m_special3Script, source->m_movementScript };
        source->m_overrideScript = &script;
        source->m_special1Script = source->m_teamScript = source->m_special2Script = NULL;
        source->m_combatScript = source->m_special3Script = source->m_movementScript = NULL;
        auto savedState = source->m_derivedStats.m_generalState;
        auto savedBaseState = source->m_baseStats.m_generalState;
        auto savedFlags = source->m_baseStats.m_flags;
        auto savedImmunity = source->m_baseStats.field_2F0[3];
        auto savedPos = source->m_pos;
        auto savedSequence = source->m_nSequence;
        auto savedSelected = source->m_bSelected;
        auto savedCutscene = game->GetGameSave()->m_cutScene;
        auto savedActive = game->GetWorldTimer()->m_active;
        source->m_bSelected = FALSE;
        source->m_baseStats.field_2F0[3] |= 1;
        game->GetWorldTimer()->m_active = TRUE;
        CRect viewport(source->m_pArea->m_cInfinity.rViewPort);
        INT viewX, viewY;
        source->m_pArea->m_cInfinity.GetViewPosition(viewX, viewY);
        viewport.OffsetRect(viewX, viewY);
        CPoint visible(viewport.left + 1, viewport.top + 1);
        CPoint hidden(viewport.right + 1, viewport.bottom + 1);
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto reset = [&]() {
            source->ClearActions(FALSE);
            while (!source->m_pendingTriggers.IsEmpty()) delete source->m_pendingTriggers.RemoveHead();
            source->m_curAction = CAIAction(CAIAction::WAIT, LONG_MAX);
            source->m_nLastActionReturn = CGameAIBase::ACTION_DONE;
            source->m_actionCount = 1;
            source->m_interrupt = FALSE;
            source->m_baseStats.m_flags = savedFlags & 0x7FFFFFFF;
            source->m_baseStats.m_generalState = 0;
            source->m_derivedStats.m_generalState = 0;
            source->m_inCutScene = FALSE;
            source->m_moraleFailure = FALSE;
            source->field_727E = FALSE;
            source->field_3428 = 0;
            source->field_558E = TRUE;
            source->field_54C = 0;
            source->field_550 = 0;
            source->field_552 = 0;
            source->m_userCommandPause = 0;
            source->m_nCommandPause = 0;
            source->m_dialogWait = 0;
            source->m_nSequence = CGameSprite::SEQ_READY;
            source->m_pos = visible;
            game->GetGameSave()->m_cutScene = FALSE;
            // Active ProcessAI advances the frame before checking phase.
            source->field_44A = static_cast<INT>(static_cast<DWORD>(id) % 16) - 1;
        };
        auto marker = [&]() {
            BOOL found = source->m_curAction.m_actionID == CAIAction::WAIT
                && source->m_curAction.m_specificID >= 9874 && source->m_curAction.m_specificID <= 9876;
            POSITION pos = source->m_queuedActions.GetHeadPosition();
            while (pos != NULL) {
                CAIAction* action = source->m_queuedActions.GetNext(pos);
                found = found || (action->m_actionID == CAIAction::WAIT && action->m_specificID == 9876);
            }
            return found;
        };
        auto check = [&](const char* name, BOOL passed) {
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"scriptschedule-case\",\"case\":\"%s\",\"scheduled\":%d,\"deferred\":%d,\"grace\":%d,\"marker\":%s,\"passed\":%s}",
                nStep, name, source->field_54C, source->field_550, source->field_552,
                marker() ? "true" : "false", passed ? "true" : "false");
            while (messages.GetCount() > before) delete messages.RemoveTail();
        };
        reset(); source->ProcessPendingTriggers(); check("virtual-selects-response", marker());
        reset(); source->m_derivedStats.m_generalState = STATE_PANIC;
        source->ProcessPendingTriggers(); check("panic-gate", !marker());
        reset(); source->m_inCutScene = TRUE;
        source->ProcessPendingTriggers(); check("sprite-cutscene-gate", !marker());
        reset(); game->GetGameSave()->m_cutScene = TRUE;
        source->ProcessPendingTriggers(); check("game-cutscene-gate", !marker());
        reset(); source->m_curAction.m_internalFlags = 1;
        source->ProcessPendingTriggers(); check("can-act-gate", !marker());
        reset(); source->m_baseStats.m_flags |= 0x80000000;
        source->ProcessPendingTriggers(); check("base-flag-gate", !marker());
        reset(); source->ProcessAI(); check("visible-phase-selects", marker() && source->field_54C == 1 && source->field_550 == 0);
        reset(); source->field_44A++;
        source->ProcessAI(); check("off-phase", !marker() && source->field_54C == 0);
        for (INT bit = 1; bit <= 4; bit *= 2) {
            reset(); source->field_3428 = bit;
            source->ProcessAI(); check("low-flags-block-scheduling", !marker() && source->field_54C == 0);
        }
        reset(); source->field_3428 = 8;
        source->ProcessAI(); check("other-flag-allows", marker() && source->field_54C == 1);
        reset(); source->m_pos = hidden;
        source->ProcessAI(); check("offscreen-deferred", !marker() && source->field_54C == 1 && source->field_550 == 1);
        reset(); source->m_pos = hidden; source->field_550 = 120;
        source->ProcessAI(); check("deferral-limit-selects", marker() && source->field_550 == 0);
        reset(); source->m_pos = hidden; source->field_552 = 2;
        source->ProcessAI(); check("grace-selects-and-decrements", marker() && source->field_552 == 1);
        reset(); source->m_pos = hidden; source->m_pendingTriggers.AddTail(new CAITrigger(CAITRIGGER_NO_TRIGGER, 0));
        source->ProcessAI(); check("trigger-selects-and-refreshes-grace", marker() && source->field_552 == 4);
        reset(); source->m_userCommandPause = 3; source->m_pendingTriggers.AddTail(new CAITrigger(CAITRIGGER_NO_TRIGGER, 0));
        source->ProcessAI(); check("user-pause-clears-triggers", !marker() && source->field_54C == 0 && source->m_pendingTriggers.IsEmpty());
        reset(); source->m_nCommandPause = 4; source->m_pendingTriggers.AddTail(new CAITrigger(CAITRIGGER_NO_TRIGGER, 0));
        source->ProcessAI(); check("command-pause-clears-triggers", !marker() && source->field_54C == 0 && source->m_pendingTriggers.IsEmpty());
        reset(); source->m_derivedStats.m_generalState = STATE_DEAD; source->field_558E = FALSE;
        source->ProcessAI(); check("dead-latch-blocks", !marker() && source->field_54C == 0);
        reset(); source->m_derivedStats.m_generalState = STATE_DEAD;
        source->ProcessAI(); check("dead-final-script", marker() && source->field_54C == 1 && source->field_558E == FALSE);
        source->ClearActions(FALSE);
        while (!source->m_pendingTriggers.IsEmpty()) delete source->m_pendingTriggers.RemoveHead();
        source->m_curAction = CAIAction::NULL_ACTION;
        source->m_overrideScript = savedScripts[0]; source->m_special1Script = savedScripts[1];
        source->m_teamScript = savedScripts[2]; source->m_special2Script = savedScripts[3];
        source->m_combatScript = savedScripts[4]; source->m_special3Script = savedScripts[5];
        source->m_movementScript = savedScripts[6];
        source->m_derivedStats.m_generalState = savedState;
        source->m_baseStats.m_generalState = savedBaseState;
        source->m_baseStats.m_flags = savedFlags;
        source->m_baseStats.field_2F0[3] = savedImmunity;
        source->m_pos = savedPos;
        source->SetSequence(static_cast<BYTE>(savedSequence));
        source->m_bSelected = savedSelected;
        source->m_inCutScene = FALSE;
        source->m_moraleFailure = FALSE;
        source->field_3428 = 0;
        source->field_558E = TRUE;
        source->m_userCommandPause = source->m_nCommandPause = 0;
        game->GetGameSave()->m_cutScene = savedCutscene;
        game->GetWorldTimer()->m_active = savedActive;
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"scriptschedule\",\"checks\":%d,\"disposable\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "script scheduling mismatch"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "dialogwait") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        LONG targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* raw = NULL;
        CGameObject* target = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &target, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        if (source->m_pPath != NULL || source->m_currentSearchRequest != NULL) {
            objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires idle actor");
            return;
        }
        auto savedWait = source->m_dialogWait;
        auto savedTarget = source->m_dialogWaitTarget;
        auto savedInterrupt = source->m_bAllowDialogInterrupt;
        auto savedIgnore = source->m_derivedStats.m_bIgnoreDialogPause;
        auto savedEA = source->m_typeAI.m_nEnemyAlly;
        auto savedSequence = source->m_nSequence;
        auto savedDirection = source->m_nDirection;
        auto savedSelected = source->m_bSelected;
        auto savedBase = source->m_baseStats.m_generalState;
        auto savedState = source->m_derivedStats.m_generalState;
        auto savedImmunity = source->m_baseStats.field_2F0[3];
        auto savedFailure = source->m_moraleFailure;
        auto savedCutscene = game->GetGameSave()->m_cutScene;
        auto savedActive = game->GetWorldTimer()->m_active;
        source->m_bSelected = FALSE;
        source->m_inCutScene = FALSE;
        source->m_baseStats.field_2F0[3] |= 1;
        game->GetGameSave()->m_cutScene = FALSE;
        game->GetWorldTimer()->m_active = TRUE;
        SHORT direction = source->GetDirection(target->GetPos());
        SHORT idle = source->GetIdleSequence();
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        LONG dummyPath[2] = { 0, 0 };
        auto run = [&](const char* name, SHORT sequence, BOOL aligned, BOOL missingTarget,
                       BOOL path, BOOL failure, BOOL allow, BOOL ignore, BYTE ea, DWORD state,
                       BOOL paused, INT expectedSequence, INT expectedDirection, INT expectedDrop, LONG initialWait = 3) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction(CAIAction::WAIT, LONG_MAX);
            source->m_dialogWait = initialWait;
            source->m_dialogWaitTarget = missingTarget ? -1 : targetId;
            source->m_bAllowDialogInterrupt = allow;
            source->m_derivedStats.m_bIgnoreDialogPause = ignore;
            source->m_typeAI.m_nEnemyAlly = ea;
            source->m_nSequence = sequence;
            source->m_nDirection = aligned ? direction : static_cast<SHORT>((direction + 1) % 16);
            source->m_baseStats.m_generalState = state | (failure ? STATE_PANIC : 0);
            source->m_derivedStats.m_generalState = state | (failure ? STATE_PANIC : 0);
            source->m_moraleFailure = failure;
            source->m_pPath = path ? dummyPath : NULL;
            source->ProcessAI();
            source->m_pPath = NULL;
            INT sequences = 0, directions = 0, drops = 0, updates = 0;
            BOOL payload = TRUE;
            while (messages.GetCount() > before) {
                CMessage* message = messages.RemoveTail();
                if (auto* seq = dynamic_cast<CMessageSetSequence*>(message)) {
                    ++sequences;
                    payload = payload && seq->m_sequence == idle && seq->GetTarget() == id;
                }
                if (auto* face = dynamic_cast<CMessageSetDirection*>(message)) {
                    ++directions;
                    payload = payload && face->m_face == target->GetPos() && face->GetTarget() == id;
                }
                if (dynamic_cast<CMessageDropPath*>(message) != NULL) ++drops;
                if (dynamic_cast<CMessageSpriteUpdate*>(message) != NULL) ++updates;
                delete message;
            }
            BOOL passed = source->m_dialogWait == (paused ? initialWait - 1 : initialWait);
            if (paused) {
                passed = passed && sequences == expectedSequence && directions == expectedDirection
                    && drops == expectedDrop && payload;
            }
            if (failure) {
                // SpriteUpdate broadcasts to others; it never enters the local queue.
                passed = passed && !source->m_moraleFailure && updates == 0
                    && (source->m_baseStats.m_generalState & STATE_PANIC) == 0
                    && (source->m_derivedStats.m_generalState & STATE_PANIC) == 0;
            }
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"dialogwait-case\",\"case\":\"%s\",\"wait\":%ld,\"sequence\":%d,\"direction\":%d,\"drop\":%d,\"localUpdates\":%d,\"passed\":%s}",
                nStep, name, source->m_dialogWait, sequences, directions, drops, updates, passed ? "true" : "false");
        };
        run("idle-and-face", CGameSprite::SEQ_ATTACK, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 1, 1, 0);
        run("already-idle-aligned", idle, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 0, 0);
        run("countdown-completes", idle, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 0, 0, 1);
        run("die-keeps-sequence", CGameSprite::SEQ_DIE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 1, 0);
        run("twitch-keeps-sequence", CGameSprite::SEQ_TWITCH, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 1, 0);
        run("sleep-keeps-sequence-and-direction", CGameSprite::SEQ_SLEEP, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 0, 0);
        run("missing-target", CGameSprite::SEQ_ATTACK, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 1, 0, 0);
        run("path-drop", idle, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 0, 1);
        run("morale-exit-and-path-drop", idle, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, CAIObjectType::EA_PC, 0, TRUE, 0, 0, 2);
        run("interrupt-disabled", idle, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, CAIObjectType::EA_PC, 0, FALSE, 0, 0, 0);
        run("ignore-dialog-pause", idle, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, CAIObjectType::EA_PC, 0, FALSE, 0, 0, 0);
        run("enemy-cutoff", idle, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_EVILCUTOFF, 0, FALSE, 0, 0, 0);
        run("sleeping-state", CGameSprite::SEQ_SLEEP, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, CAIObjectType::EA_PC, STATE_SLEEPING, FALSE, 0, 0, 0);
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        source->m_dialogWait = savedWait;
        source->m_dialogWaitTarget = savedTarget;
        source->m_bAllowDialogInterrupt = savedInterrupt;
        source->m_derivedStats.m_bIgnoreDialogPause = savedIgnore;
        source->m_typeAI.m_nEnemyAlly = savedEA;
        source->m_nSequence = savedSequence;
        source->m_nDirection = savedDirection;
        source->m_bSelected = savedSelected;
        source->m_baseStats.m_generalState = savedBase;
        source->m_derivedStats.m_generalState = savedState;
        source->m_baseStats.field_2F0[3] = savedImmunity;
        source->m_moraleFailure = savedFailure;
        game->GetGameSave()->m_cutScene = savedCutscene;
        game->GetWorldTimer()->m_active = savedActive;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"dialogwait\",\"checks\":%d,\"disposable\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "dialogue pause mismatch"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "moralfailure") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and party actor");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        if (source->m_pPath != NULL || source->m_currentSearchRequest != NULL) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires idle actor");
            return;
        }
        auto savedMorale = source->m_baseStats.field_25C;
        auto savedImmunity = source->m_baseStats.field_2F0[3];
        auto savedBreak = source->m_derivedStats.m_nMoraleBreak;
        auto savedRecovery = source->m_derivedStats.m_nMoraleRecoveryTime;
        auto savedBaseState = source->m_baseStats.m_generalState;
        auto savedState = source->m_derivedStats.m_generalState;
        auto savedSelected = source->m_bSelected;
        auto savedBerserk = source->m_berserkActive;
        auto savedFailure = source->m_moraleFailure;
        auto savedCutscene = game->GetGameSave()->m_cutScene;
        auto savedActive = game->GetWorldTimer()->m_active;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount();
        BOOL valid = TRUE;
        INT seen[3] = { 0, 0, 0 };
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            ++checks;
            valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"moralfailure-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
        };
        source->m_bSelected = FALSE;
        source->m_inCutScene = FALSE;
        game->GetGameSave()->m_cutScene = FALSE;
        game->GetWorldTimer()->m_active = TRUE;
        source->m_baseStats.field_2F0[3] &= ~1;
        source->m_derivedStats.m_nMoraleRecoveryTime = 1000;
        source->m_derivedStats.m_nMoraleBreak = 20;
        // Keep action execution blocked while observing ProcessAI selection.
        // This disposable profile never saves the actor's test state.
        for (INT attempt = 0; attempt < 96 && (seen[0] == 0 || seen[1] == 0 || seen[2] == 0); ++attempt) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction(CAIAction::WAIT, LONG_MAX);
            source->m_baseStats.field_25C = 0;
            source->m_baseStats.m_generalState = STATE_STONE_DEATH;
            source->m_derivedStats.m_generalState = STATE_STONE_DEATH;
            source->m_moraleFailure = FALSE;
            source->m_berserkActive = FALSE;
            source->m_dialogWait = 0;
            source->ProcessAI();
            check("entry-latch-and-single-action", source->m_moraleFailure
                && source->m_queuedActions.GetCount() == 1 && source->m_interrupt);
            if (source->m_queuedActions.GetCount() != 1) break;
            CAIAction* action = source->m_queuedActions.GetHead();
            SHORT actionId = action->m_actionID;
            INT variant = actionId == CAIAction::PANIC ? 0
                : actionId == CAIAction::RUNAWAYFROMNOINTERRUPT ? 1
                : actionId == CAIAction::BERSERK ? 2 : -1;
            check("selected-action-and-state", variant >= 0
                && ((source->m_baseStats.m_generalState & STATE_PANIC) != 0) == (variant < 2)
                && source->m_berserkActive == (variant == 2)
                && action->m_specificID == (variant == 0 ? 0 : LONG_MAX)
                && action->m_specificID2 == (variant == 0 ? -1 : 0)
                && action->m_dest == CPoint(-1, -1) && action->m_internalFlags == 0);
            if (variant >= 0) ++seen[variant];
            source->ProcessAI();
            check("failure-retains-action", source->m_moraleFailure
                && source->m_queuedActions.GetCount() == 1
                && source->m_queuedActions.GetHead() == action);
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        check("all-three-selections", seen[0] > 0 && seen[1] > 0 && seen[2] > 0);
        source->m_baseStats.field_25C = 20;
        source->m_derivedStats.m_nMoraleBreak = 0;
        source->m_baseStats.m_generalState = STATE_STONE_DEATH | STATE_PANIC;
        source->m_derivedStats.m_generalState = STATE_STONE_DEATH | STATE_PANIC;
        source->m_moraleFailure = TRUE;
        source->ProcessAI();
        BOOL dropSeen = FALSE;
        while (messages.GetCount() > before) {
            CMessage* message = messages.RemoveTail();
            if (dynamic_cast<CMessageDropPath*>(message) != NULL && message->GetTarget() == id) dropSeen = TRUE;
            delete message;
        }
        check("recovery-clears-panic-and-drops-path", !source->m_moraleFailure
            && (source->m_baseStats.m_generalState & STATE_PANIC) == 0
            && (source->m_derivedStats.m_generalState & STATE_PANIC) == 0 && dropSeen);
        for (INT count = 0; count < 2; ++count) {
            source->ClearActions(FALSE);
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = CAIAction::PANIC;
            source->m_actionCount = count;
            SHORT result = source->ExecuteAction();
            check("panic-dispatch-queues-randomwalk", result == CGameAIBase::ACTION_DONE
                && source->m_nLastActionReturn == result
                && source->m_queuedActions.GetCount() == 1
                && source->m_queuedActions.GetHead()->m_actionID == CAIAction::RANDOMWALK
                && messages.GetCount() == before + (count == 0 ? 2 : 0));
            while (messages.GetCount() > before) delete messages.RemoveTail();
        }
        source->ClearActions(FALSE);
        source->m_curAction = CAIAction::NULL_ACTION;
        source->m_baseStats.field_25C = savedMorale;
        source->m_baseStats.field_2F0[3] = savedImmunity;
        source->m_derivedStats.m_nMoraleBreak = savedBreak;
        source->m_derivedStats.m_nMoraleRecoveryTime = savedRecovery;
        source->m_baseStats.m_generalState = savedBaseState;
        source->m_derivedStats.m_generalState = savedState;
        source->m_bSelected = savedSelected;
        source->m_berserkActive = savedBerserk;
        source->m_moraleFailure = savedFailure;
        game->GetGameSave()->m_cutScene = savedCutscene;
        game->GetWorldTimer()->m_active = savedActive;
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"moralfailure\",\"checks\":%d,\"panic\":%d,\"run\":%d,\"berserk\":%d,\"disposable\":true,\"valid\":%s}",
            nStep, checks, seen[0], seen[1], seen[2], valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "ProcessAI morale failure mismatch"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatmorale") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and party actor");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        auto savedMorale = source->m_baseStats.field_25C;
        auto savedImmunity = source->m_baseStats.field_2F0[3];
        auto savedBreak = source->m_derivedStats.m_nMoraleBreak;
        auto savedRecovery = source->m_derivedStats.m_nMoraleRecoveryTime;
        auto savedFrame = source->field_44A;
        auto savedFlags = source->m_curAction.m_internalFlags;
        auto savedCutscene = game->GetGameSave()->m_cutScene;
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BYTE morale, DWORD recovery, DWORD threshold,
                         LONG frame, BYTE immunity, BOOL cutscene, BYTE flags,
                         BYTE expectedMorale, BOOL expectedResult) {
            source->m_baseStats.field_25C = morale;
            source->m_baseStats.field_2F0[3] = immunity;
            source->m_derivedStats.m_nMoraleBreak = threshold;
            source->m_derivedStats.m_nMoraleRecoveryTime = recovery;
            source->field_44A = frame;
            source->m_curAction.m_internalFlags = flags;
            game->GetGameSave()->m_cutScene = cutscene;
            BOOL result = source->CheckMorale();
            BOOL passed = result == expectedResult && source->m_baseStats.field_25C == expectedMorale;
            valid = valid && passed;
            ++checks;
            Emit("{\"step\":%d,\"op\":\"combatmorale-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
        };
        check("immunity-bypasses-update", 255, 0, 255, 0, 1, FALSE, 0, 255, TRUE);
        check("clamp", 255, 2, 19, 1, 0, FALSE, 0, 20, TRUE);
        check("zero-recovery-resets", 0, 0, 9, 1, 0, FALSE, 0, 10, TRUE);
        check("threshold-equality-fails", 10, 2, 10, 1, 0, FALSE, 0, 10, FALSE);
        check("below-threshold-fails", 9, 2, 10, 1, 0, FALSE, 0, 9, FALSE);
        check("recovery-up", 9, 2, 9, 30, 0, FALSE, 0, 10, TRUE);
        check("recovery-down", 11, 2, 10, 30, 0, FALSE, 0, 10, FALSE);
        check("recovery-equilibrium", 10, 2, 9, 30, 0, FALSE, 0, 10, TRUE);
        check("off-phase", 9, 2, 9, 29, 0, FALSE, 0, 9, FALSE);
        check("unsigned-frame", 9, 1, 9, -1, 0, FALSE, 0, 10, TRUE);
        check("unsigned-threshold", 10, 2, 0xFFFFFFFF, 1, 0, FALSE, 0, 10, FALSE);
        check("cutscene-after-recovery", 9, 2, 255, 30, 0, TRUE, 0, 10, TRUE);
        check("action-exemption-after-recovery", 11, 2, 255, 30, 0, FALSE, 1, 10, TRUE);
        check("other-action-flag", 9, 2, 9, 1, 0, FALSE, 2, 9, FALSE);
        source->m_baseStats.field_25C = savedMorale;
        source->m_baseStats.field_2F0[3] = savedImmunity;
        source->m_derivedStats.m_nMoraleBreak = savedBreak;
        source->m_derivedStats.m_nMoraleRecoveryTime = savedRecovery;
        source->field_44A = savedFrame;
        source->m_curAction.m_internalFlags = savedFlags;
        game->GetGameSave()->m_cutScene = savedCutscene;
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"combatmorale\",\"checks\":%d,\"restored\":true,\"valid\":%s}",
            nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "morale mismatch (state restored)"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatrunaway") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG id = game->GetFixedOrderCharacterId(0);
        LONG targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* raw = NULL;
        CGameObject* targetRaw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &targetRaw, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        auto* target = static_cast<CGameSprite*>(targetRaw);
        CAIAction savedAction;
        savedAction = source->m_curAction;
        auto savedCount = source->field_5614;
        auto savedReturn = source->m_nLastActionReturn;
        auto savedPos = source->m_pos;
        auto savedTarget = source->m_targetId;
        auto savedSequence = source->m_nSequence;
        auto savedWeapon = source->m_equipment.m_items[43];
        auto savedSelected = source->m_equipment.m_selectedWeapon;
        auto* savedArea = target->m_pArea;
        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = queue.GetCount();
        BOOL valid = source->m_pArea != NULL && source->m_pArea == target->m_pArea;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"combatrunaway-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        source->m_pos = target->m_pos;
        source->m_targetId = targetId;
        source->m_equipment.m_selectedWeapon = 43;
        source->m_equipment.m_items[43] = NULL;
        source->field_5614 = 9;
        check("null-target", source->RunAwayFrom(NULL) == CGameAIBase::ACTION_ERROR && source->field_5614 == 9);
        target->m_pArea = NULL;
        check("different-area", source->RunAwayFrom(target) == CGameAIBase::ACTION_ERROR && source->field_5614 == 9);
        target->m_pArea = savedArea;
        CAIObjectType selector;
        selector.m_nInstance = targetId;
        for (INT variant = 0; variant < 2; ++variant) {
            SHORT action = variant == 0 ? CAIAction::RUNAWAYFROMNOINTERRUPT : CAIAction::RUNAWAYFROM;
            source->m_curAction = CAIAction(action, selector, 1000, 0, 0);
            for (SHORT count = 0; count < 14; ++count) {
                source->field_5614 = count;
                SHORT result = source->ExecuteAction();
                check(variant == 0 ? "noninterruptible-duration" : "interruptible-duration",
                    result == (variant == 0 ? CGameAIBase::ACTION_NORMAL : CGameAIBase::ACTION_INTERRUPTABLE)
                    && source->m_nLastActionReturn == result && source->field_5614 == count + 1
                    && queue.GetCount() == before);
            }
        }
        source->m_curAction = CAIAction(CAIAction::RUNAWAYFROMNOINTERRUPT, selector, LONG_MAX, 0, 0);
        source->field_5614 = SHORT_MAX;
        check("signed-counter-wrap", source->ExecuteAction() == CGameAIBase::ACTION_NORMAL
            && source->field_5614 == static_cast<SHORT>(-32768));
        source->m_curAction.m_specificID = 0;
        source->field_5614 = 1;
        source->m_nSequence = CGameSprite::SEQ_READY;
        check("expired-ready", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->field_5614 == 0 && queue.GetCount() == before);
        source->field_5614 = 1;
        source->m_nSequence = CGameSprite::SEQ_ATTACK;
        check("expired-attack", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->field_5614 == 0 && queue.GetCount() == before + 2);
        if (queue.GetCount() == before + 2) {
            CMessage* sequence = queue.RemoveTail();
            CMessage* drop = queue.RemoveTail();
            check("expired-messages", dynamic_cast<CMessageSetSequence*>(sequence) != NULL
                && dynamic_cast<CMessageDropPath*>(drop) != NULL
                && sequence->GetTarget() == id && drop->GetTarget() == id);
            delete sequence;
            delete drop;
        }
        while (queue.GetCount() > before) delete queue.RemoveTail();
        source->m_curAction = savedAction;
        source->field_5614 = savedCount;
        source->m_nLastActionReturn = savedReturn;
        source->m_pos = savedPos;
        source->m_targetId = savedTarget;
        source->m_nSequence = savedSequence;
        source->m_equipment.m_items[43] = savedWeapon;
        source->m_equipment.m_selectedWeapon = savedSelected;
        target->m_pArea = savedArea;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"combatrunaway\",\"checks\":%d,\"restored\":true,\"valid\":%s}",
            nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "runaway duration/guard mismatch (state restored)"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatberserk") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        CGameObject* raw = NULL;
        LONG id = game->GetFixedOrderCharacterId(0);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 1
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and party actor");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        if (source->m_pArea == NULL) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires actor area");
            return;
        }
        CAIAction savedAction;
        savedAction = source->m_curAction;
        BOOL savedBerserk = source->m_berserkActive;
        SHORT savedReturn = source->m_nLastActionReturn;
        CTypedPtrList<CPtrList, CAIAction*> savedQueue;
        while (!source->m_queuedActions.IsEmpty()) {
            savedQueue.AddTail(source->m_queuedActions.RemoveHead());
        }
        CAIAction waitAction(CAIAction::WAIT, 6);
        BOOL valid = waitAction.m_actionID == CAIAction::WAIT
            && waitAction.m_specificID == 6 && waitAction.m_specificID2 == 0
            && waitAction.m_specificID3 == 0 && waitAction.m_internalFlags == 0;
        INT activeCases = 0;
        for (INT i = 0; i < 14; ++i) {
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = 124;
            source->m_curAction.m_specificID = i == 1 ? 1 : 12;
            source->m_berserkActive = i != 0;
            SHORT result = source->ExecuteAction();
            INT count = source->m_queuedActions.GetCount();
            valid = valid && result == CGameAIBase::ACTION_DONE
                && source->m_nLastActionReturn == result
                && source->m_curAction.m_specificID == (i == 1 ? 0 : 11);
            if (i < 2) {
                valid = valid && count == 0;
            } else {
                valid = valid && (count == 1 || count == 2);
                if (count > 0) {
                    SHORT actionId = source->m_queuedActions.GetHead()->m_actionID;
                    valid = valid && (actionId == CAIAction::NO_ACTION
                        || actionId == CAIAction::ATTACK || actionId == CAIAction::WAIT);
                }
                ++activeCases;
            }
            while (!source->m_queuedActions.IsEmpty()) {
                delete source->m_queuedActions.RemoveHead();
            }
        }
        source->m_curAction = savedAction;
        source->m_berserkActive = savedBerserk;
        source->m_nLastActionReturn = savedReturn;
        while (!savedQueue.IsEmpty()) {
            source->m_queuedActions.AddTail(savedQueue.RemoveHead());
        }
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"combatberserk\",\"active_cases\":%d,\"valid\":%s}",
            nStep, activeCases, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "berserk queue mismatch (state restored)"); return; }
        s_pc++;
        return;
    }

    if (strcmp(step.op, "combatmove") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        CGameObject* raw = NULL;
        CGameObject* target = NULL;
        LONG id = game->GetFixedOrderCharacterId(0);
        LONG targetId = game->GetFixedOrderCharacterId(1);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and two party members");
            return;
        }
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &target, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(raw);
        auto action = source->m_curAction;
        auto seen = source->m_lSeen;
        auto visibility = source->m_canBeSeen;
        auto encumbrance = source->m_derivedStats.m_nEncumberance;
        auto pos = source->m_pos;
        auto path = source->m_pPath;
        auto& queue = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = queue.GetCount();
        BOOL valid = source->m_pArea == target->m_pArea;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            Emit("{\"step\":%d,\"op\":\"combatmove-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
            valid = valid && passed;
            ++checks;
        };
        source->m_derivedStats.m_nEncumberance = 0;
        check("null-target", source->MoveToObject(NULL) == CGameAIBase::ACTION_ERROR);
        // Synchronous position change: no tick/render/search-map update occurs.
        source->m_pos = target->GetPos();
        source->m_curAction.m_actionID = 0x107;
        source->m_lSeen.Set(CAIObjectType::NOONE);
        check("visible-target-done", source->MoveToObject(target) == CGameAIBase::ACTION_DONE);
        check("seen-and-visibility", source->m_lSeen.Equal(target->GetAIType())
            && source->m_canBeSeen == CGameObject::VISIBLE_DELAY * 4 + 4);
        check("seen-message-count", queue.GetCount() == before + 1);
        if (queue.GetCount() > before) {
            CMessage* message = queue.RemoveTail();
            auto* last = dynamic_cast<CMessageSetLastObject*>(message);
            check("seen-message-payload", last != NULL && last->m_type == CAITRIGGER_SEE
                && last->GetTarget() == id && last->GetSource() == id
                && last->m_lAttacker.Equal(target->GetAIType()));
            delete message;
        }
        check("already-seen-done", source->MoveToObject(target) == CGameAIBase::ACTION_DONE);
        check("already-seen-no-message", queue.GetCount() == before);
        LONG dummyPath = 0;
        source->m_pPath = &dummyPath;
        source->m_curAction.m_actionID = CAIAction::MOVETOOBJECTFOLLOW;
        check("follow-close-interruptible", source->MoveToObject(target) == CGameAIBase::ACTION_INTERRUPTABLE);
        check("drop-path-message-count", queue.GetCount() == before + 1);
        if (queue.GetCount() > before) {
            CMessage* message = queue.RemoveTail();
            check("drop-path-message", dynamic_cast<CMessageDropPath*>(message) != NULL
                && message->GetTarget() == id && message->GetSource() == id);
            delete message;
        }
        source->m_pPath = NULL;
        check("follow-idle-interruptible", source->MoveToObject(target) == CGameAIBase::ACTION_INTERRUPTABLE);
        check("follow-idle-no-message", queue.GetCount() == before);
        while (queue.GetCount() > before) delete queue.RemoveTail();
        source->m_curAction = action;
        source->m_lSeen = seen;
        source->m_canBeSeen = visibility;
        source->m_derivedStats.m_nEncumberance = encumbrance;
        source->m_pos = pos;
        source->m_pPath = path;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) { Fail(nStep, step.op, "movement guard mismatch (state restored)"); return; }
        Emit("{\"step\":%d,\"op\":\"combatmove\",\"checks\":%d,\"restored\":true}", nStep, checks);
        s_pc++;
        return;
    }

    if (strcmp(step.op, "attackmark") == 0 || strcmp(step.op, "attackwait") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        BOOL mark = strcmp(step.op, "attackmark") == 0;
        LONG targetId = mark ? game->GetFixedOrderCharacterId(1) : s_attackTarget;
        CGameObject* target = NULL;
        CGameObject* attacker = NULL;
        LONG sourceId = game->GetFixedOrderCharacterId(0);
        if (objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &target, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "target unavailable");
            return;
        }
        SHORT hp = static_cast<CGameSprite*>(target)->m_baseStats.m_hitPoints;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (mark) {
            s_attackTarget = targetId;
            s_attackHP = hp;
            s_attackOrderSeen = FALSE;
            s_attackRequireMove = step.nCount > 0 && step.n[0] != 0;
            s_attackMoved = FALSE;
            if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &attacker, INFINITE) != CGameObjectArray::SUCCESS) {
                Fail(nStep, step.op, "attacker unavailable");
                return;
            }
            s_attackStart = attacker->GetPos();
            objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            s_pc++;
            Emit("{\"step\":%d,\"op\":\"attackmark\",\"hp\":%d,\"x\":%ld,\"y\":%ld,\"requireMove\":%d}",
                nStep, hp, s_attackStart.x, s_attackStart.y, s_attackRequireMove);
            return;
        }
        if (objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &attacker, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "attacker unavailable");
            return;
        }
        auto* source = static_cast<CGameSprite*>(attacker);
        s_attackOrderSeen = s_attackOrderSeen || source->m_curAction.m_actionID == CAIAction::ATTACK;
        s_attackMoved = s_attackMoved || (source->m_curAction.m_actionID == CAIAction::ATTACK
            && source->GetPos() != s_attackStart);
        BOOL success = hp < s_attackHP && s_attackOrderSeen && (!s_attackRequireMove || s_attackMoved);
        if (success || s_waited >= 600) {
            source->ClearActions(FALSE);
            source->SetCurrAction(CAIAction::NULL_ACTION);
            source->DropPath();
            source->DropSearchRequest();
            source->m_attackFrame = -2;
        }
        if (s_waited % 30 == 0 || success) {
            Emit("{\"step\":%d,\"op\":\"attackwait\",\"ticks\":%d,\"action\":%d,\"frame\":%d,\"hpBefore\":%d,\"hpAfter\":%d,\"orderSeen\":%d,\"moved\":%d,\"x\":%ld,\"y\":%ld}",
                nStep, s_waited, source->m_curAction.m_actionID, source->m_attackFrame,
                s_attackHP, hp, s_attackOrderSeen, s_attackMoved, source->GetPos().x, source->GetPos().y);
        }
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (success) {
            s_waited = 0;
            s_pc++;
        } else if (++s_waited > 600) {
            Fail(nStep, step.op, "UI attack order did not damage target");
        }
        return;
    }

    if (strcmp(step.op, "weaponpayload") == 0 || strcmp(step.op, "weapondamage") == 0
        || strcmp(step.op, "swingdamage") == 0 || strcmp(step.op, "attackdamage") == 0
        || strcmp(step.op, "swingonhit") == 0
        || strcmp(step.op, "finalshotdamage") == 0) {
        BOOL swingOnHit = strcmp(step.op, "swingonhit") == 0;
        BOOL baseDamage = strcmp(step.op, "weapondamage") == 0;
        BOOL finalShot = strcmp(step.op, "finalshotdamage") == 0;
        INT finalMode = finalShot && step.nCount > 1 ? step.n[1] : 0;
        BOOL attackDamage = strcmp(step.op, "attackdamage") == 0 || finalShot;
        BOOL swingDamage = strcmp(step.op, "swingdamage") == 0 || swingOnHit || attackDamage;
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
            if (s_payloadHP - hp != s_payloadDamage) {
                Fail(nStep, "weaponpayload", "unexpected HP delta or projectile did not arrive");
                return;
            }
            if (swingOnHit) {
                CGameObject* sourceObject = NULL;
                LONG sourceId = pGame->GetFixedOrderCharacterId(0);
                if (objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceObject, INFINITE) != CGameObjectArray::SUCCESS) {
                    Fail(nStep, step.op, "retaliation target unavailable");
                    return;
                }
                INT loss = s_onHitSourceHP - static_cast<CGameSprite*>(sourceObject)->m_baseStats.m_hitPoints;
                objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                if (loss < 2 && ++s_waited < 600) return;
                Emit("{\"step\":%d,\"op\":\"swingonhit-retaliation\",\"sourceLoss\":%d,\"targetLoss\":%d,\"checks\":2}", nStep, loss, s_payloadHP - hp);
                if (loss != 2) {
                    Fail(nStep, step.op, "melee hit did not trigger exactly two retaliation damage");
                    return;
                }
            }
            Emit("{\"step\":%d,\"op\":\"%s\",\"item\":\"%s\",\"hpBefore\":%d,\"hpAfter\":%d,\"expectedDamage\":%d}",
                nStep, step.op, step.s[0], s_payloadHP, hp, s_payloadDamage);
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
            && hp > (baseDamage || swingDamage ? 14 : (step.s[1][0] != '\0' ? 4 : 2));

        CItem weapon(CResRef(step.s[0]), swingDamage ? 3 : 1, 0, 0, 0, 0);
        CItem launcher(CResRef(step.s[1]), 1, 0, 0, 0, 0);
        BOOL hasLauncher = step.s[1][0] != '\0';
        BOOL weaponDemanded = valid && weapon.Demand();
        valid = valid && weaponDemanded;
        ITEM_ABILITY* ability = valid ? weapon.GetAbility(0) : NULL;
        valid = valid && ability != NULL;
        USHORT missile = valid ? ability->missileType : 0;
        CProjectile* projectile = valid && !swingDamage ? CProjectile::DecodeProjectile(missile, source, 0) : NULL;
        valid = valid && (swingDamage || projectile != NULL);
        if (valid && swingDamage) {
            valid = source->m_derivedStats.m_cDamageBonusList.IsEmpty()
                && target->m_derivedStats.m_cImmunitiesWeapon.IsEmpty();
            if (valid) {
                const char* reserveName = finalMode == 2 ? (strcmp(step.s[0], "ZRCARROW") == 0 ? "ZRCARR2" : (strcmp(step.s[0], "ZRCBOLT") == 0 ? "ZRCBLT2" : "ZRCSTN2")) : step.s[0];
                CItem reserve(CResRef(reserveName), 7, 0, 0, 0, 0);
                // CheckItemDepleted owns deletion of the last unit. The stack
                // weapon remains a resource keeper for the ability pointer.
                CItem* lastUnit = finalShot ? new CItem(CResRef(step.s[0]), 1, 0, 0, 0, 0) : NULL;
                CDerivedStatsTemplate savedStats = source->m_derivedStats;
                auto savedBase = source->m_baseStats;
                auto savedType = source->m_typeAI;
                CAIAction savedAction;
                savedAction = source->m_curAction;
                auto actionCount = source->m_actionCount;
                auto lastReturn = source->m_nLastActionReturn;
                auto runawayCount = source->field_5614;
                auto sourcePos = source->m_pos;
                auto lastAction = source->m_lastActionID;
                auto feedbackTarget = source->field_70EE;
                auto retaliation = target->m_derivedStats.m_onHitSpells;
                auto team = target->field_58C;
                auto targetType = target->m_typeAI;
                auto selected = source->m_equipment.m_selectedWeapon;
                auto selectedAbility = source->m_equipment.m_selectedWeaponAbility;
                auto weaponSet = source->m_nWeaponSet;
                auto allow = source->m_bAllowEffectListCall;
                auto frame = source->m_attackFrame;
                auto speed = source->m_speedFactor;
                auto active = source->field_5630;
                auto offhand = source->field_5632;
                auto count = source->field_5636;
                auto attackState = source->field_7118;
                auto pending = source->m_curProjectile;
                CItem* oldAmmo = source->m_equipment.m_items[11];
                CItem* oldMain = source->m_equipment.m_items[43];
                CItem* oldOff = source->m_equipment.m_items[44];
                CItem* oldReserves[3];
                BYTE cachedAmmo[8];
                memcpy(oldReserves, &source->m_equipment.m_items[12], sizeof(oldReserves));
                memcpy(cachedAmmo, source->field_3D3A, sizeof(cachedAmmo));
                CButtonData buttons[8];
                for (INT i = 0; i < 8; ++i) buttons[i] = source->m_quickWeapons[i];
                CResRef weaponRefs[CGAMESAVECHARACTER_NUM_STATS_WEAPONS];
                SHORT weaponUses[CGAMESAVECHARACTER_NUM_STATS_WEAPONS];
                for (INT i = 0; i < CGAMESAVECHARACTER_NUM_STATS_WEAPONS; ++i) {
                    weaponRefs[i] = source->m_cGameStats.m_pWeaponStats[i].m_cResRef;
                    weaponUses[i] = source->m_cGameStats.m_pWeaponStats[i].m_nTimesUsed;
                }
                SHORT hitBonus = ability->thac0Bonus;
                ability->thac0Bonus = SHORT_MAX;
                static_cast<CDerivedStatsTemplate&>(source->m_derivedStats) = CDerivedStatsTemplate{};
                source->m_derivedStats.m_nLuck = 19;
                source->m_derivedStats.m_nSTR = 10;
                source->m_derivedStats.m_nNumberOfAttacks = 1;
                memset(source->m_baseStats.m_feats, 0, sizeof(source->m_baseStats.m_feats));
                source->m_baseStats.m_rogueLevel = 0;
                source->m_typeAI.m_nClass = 0;
                source->m_typeAI.m_nClassMask = 0;
                target->m_derivedStats.m_onHitSpells.clear();
                if (swingOnHit) {
                    s_onHitSourceHP = source->m_baseStats.m_hitPoints;
                    Emit("{\"step\":%d,\"op\":\"swingonhit-context\",\"hp\":%d,\"crushing\":%d,\"tempCrushing\":%d,\"resistMagic\":%d,\"sphere\":%d,\"invulnerable\":%d,\"generalState\":%lu,\"baseState\":%lu}",
                        nStep, s_onHitSourceHP, savedStats.m_nResistCrushing, source->m_tempStats.m_nResistCrushing,
                        savedStats.m_nResistMagic, source->m_derivedStats.m_spellStates[SPLSTATE_OTILUKES_RESILIENT_SPHERE] ? 1 : 0,
                        source->m_animation.IsInvulnerable(), savedStats.m_generalState, source->m_baseStats.m_generalState);
                    ITEM_EFFECT struck = {};
                    struck.effectID = ICEWIND_CGAMEEFFECT_WHENSTRUCK;
                    CGameEffect* effect = CGameEffect::DecodeEffect(&struck, target->GetPos(), targetId, source->GetPos());
                    effect->m_sourceRes = "ZRCORIG2";
                    effect->m_res = "ZRCHIT";
                    effect->m_casterLevel = 0xB01;
                    valid = valid && effect->ApplyEffect(target)
                        && target->m_derivedStats.m_onHitSpells.size() == 1
                        && target->m_derivedStats.m_onHitSpells.front().m_level == 1;
                    delete effect;
                }
                target->field_58C = 0x200000;
                source->m_equipment.m_items[11] = finalShot ? lastUnit : (hasLauncher ? &weapon : oldAmmo);
                source->m_equipment.m_items[43] = hasLauncher ? &launcher : &weapon;
                source->m_equipment.m_items[44] = NULL;
                source->m_equipment.m_selectedWeapon = hasLauncher ? 11 : 43;
                source->m_equipment.m_selectedWeaponAbility = 0;
                source->m_nWeaponSet = 0;
                if (finalShot) {
                    source->m_equipment.m_items[12] = finalMode == 1 ? NULL : &reserve;
                    source->m_equipment.m_items[13] = NULL;
                    source->m_equipment.m_items[14] = NULL;
                    memset(source->field_3D3A, 0, sizeof(cachedAmmo));
                    for (INT i = 0; i < 8; ++i) {
                        source->m_quickWeapons[i] = CButtonData();
                        source->m_quickWeapons[i].m_abilityId.m_itemNum = static_cast<SHORT>(43 + i);
                        source->m_quickWeapons[i].m_abilityId.m_abilityNum = 0;
                    }
                }
                source->m_bAllowEffectListCall = TRUE;
                source->m_curProjectile = NULL;
                source->m_attackFrame = -1;
                source->field_5630 = 0;
                source->field_5632 = 0;
                source->field_5636 = 0;
                SHORT actionId = step.nCount > 0 ? static_cast<SHORT>(step.n[0]) : CAIAction::ATTACK;
                BOOL runaway = attackDamage && (actionId == CAIAction::RUNAWAYFROM
                    || actionId == CAIAction::RUNAWAYFROMNOINTERRUPT);
                if (runaway) {
                    // Temporarily make the other party member the nearest enemy.
                    target->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_ENEMY;
                    source->m_pos = target->m_pos;
                    source->field_5614 = 0;
                    BYTE sourceSpace = source->m_animation.GetPersonalSpace();
                    BYTE targetSpace = target->m_animation.GetPersonalSpace();
                    BYTE space = static_cast<BYTE>(((targetSpace - 1) >> 1) + ((sourceSpace - 1) >> 1) - 1);
                    SHORT reach = static_cast<SHORT>((space + static_cast<SHORT>(ability->range + 1)) * 16);
                    LONG nearest = area->GetNearest(sourceId, source->m_typeAI.GetEnemyOf(), reach,
                        source->m_terrainTable, TRUE, source->GetCanSeeInvisible(), FALSE, 0, FALSE);
                    Emit("{\"step\":%d,\"op\":\"runaway-retaliation-target\",\"target\":%ld,\"nearest\":%ld,\"range\":%d,\"sanctuary\":%d}",
                        nStep, targetId, nearest, reach, target->m_derivedStats.m_spellStates[SPLSTATE_SANCTUARY] ? 1 : 0);
                    valid = valid && nearest == targetId;
                }
                if (attackDamage) {
                    CAIObjectType targetType;
                    targetType.m_nInstance = targetId;
                    source->m_curAction = CAIAction(actionId, targetType,
                        actionId == CAIAction::ATTACKREEVALUATE || runaway ? 1000 : 0, 0, 0);
                    source->m_actionCount = 0;
                }
                SHORT result = attackDamage ? source->ExecuteAction() : source->Swing(target);
                if (attackDamage && actionId == CAIAction::ATTACKREEVALUATE) {
                    Emit("{\"step\":%d,\"op\":\"reevaluate-start\",\"target\":%ld,\"resolved\":%ld,\"returned\":%d,\"lastAction\":%d,\"frame\":%d,\"budget\":%ld}",
                        nStep, targetId, source->m_curAction.m_acteeID2.m_nInstance,
                        result, source->m_lastActionID, source->m_attackFrame, source->m_curAction.m_specificID);
                }
                INT ticks = 1;
                for (SHORT combatFrame = 1; combatFrame <= 101 && source->m_attackFrame != -2 && !(finalShot && finalMode != 0 && finalMode != 3 && result == CGameAIBase::ACTION_ERROR); ++combatFrame) {
                    source->m_attackFrame = combatFrame;
                    if (attackDamage) source->m_actionCount = combatFrame;
                    // Isolate retaliation from the fifteenth-count movement request.
                    if (runaway) source->field_5614 = 0;
                    result = attackDamage ? source->ExecuteAction() : source->Swing(target);
                    valid = valid && (result != CGameAIBase::ACTION_ERROR || (finalShot && finalMode != 0 && finalMode != 3));
                    ++ticks;
                }
                if (finalShot && finalMode == 3) {
                    valid = valid && result == CGameAIBase::ACTION_INTERRUPTABLE
                        && source->m_attackFrame == -2 && reserve.GetUsageCount(0) == 7
                        && source->m_equipment.m_items[11] == NULL
                        && source->m_equipment.m_selectedWeapon == 12;
                    Emit("{\"step\":%d,\"op\":\"resume-first\",\"item\":\"%s\",\"calls\":%d,\"remaining\":%u,\"returned\":%d,\"frame\":%d,\"valid\":%d}",
                        nStep, step.s[0], ticks, reserve.GetUsageCount(0), result, source->m_attackFrame, valid);
                    // A new controlled Attack starts from the engine-selected slot.
                    // Do not reinstall or reselect the reserve.
                    source->m_attackFrame = -1;
                    source->m_actionCount = 0;
                    result = source->ExecuteAction();
                    ticks = 1;
                    for (SHORT combatFrame = 1; combatFrame <= 101 && source->m_attackFrame != -2; ++combatFrame) {
                        source->m_attackFrame = combatFrame;
                        source->m_actionCount = combatFrame;
                        result = source->ExecuteAction();
                        valid = valid && result != CGameAIBase::ACTION_ERROR;
                        ++ticks;
                    }
                }
                WORD remaining = finalShot ? (finalMode == 1 ? 0 : reserve.GetUsageCount(0)) : weapon.GetUsageCount(0);
                valid = valid && result == (finalShot && finalMode != 0 && finalMode != 3 ? CGameAIBase::ACTION_ERROR : attackDamage && actionId == CAIAction::ATTACKONEROUND
                        ? CGameAIBase::ACTION_DONE : runaway && actionId == CAIAction::RUNAWAYFROMNOINTERRUPT
                        ? CGameAIBase::ACTION_NORMAL : CGameAIBase::ACTION_INTERRUPTABLE)
                    && (finalShot && finalMode != 0 && finalMode != 3 ? source->m_attackFrame >= 0 : source->m_attackFrame == -2)
                    && (finalShot && finalMode != 0 && finalMode != 3 || source->m_curProjectile == NULL)
                    && remaining == (finalShot ? (finalMode == 1 ? 0 : (finalMode == 3 ? 6 : 7)) : (ability->maxUsageCount != 0 ? 2 : 3));
                BOOL deleted = finalShot && source->m_equipment.m_items[11] == NULL;
                valid = valid && (!finalShot || (hasLauncher && deleted
                    && source->m_equipment.m_selectedWeapon == (finalMode == 1 ? 10 : 12)
                    && source->m_equipment.m_selectedWeaponAbility == 0 && source->m_nWeaponSet == 0));
                Emit("{\"step\":%d,\"op\":\"%s-round\",\"item\":\"%s\",\"calls\":%d,\"remaining\":%u,\"valid\":%d,\"action\":%d,\"returned\":%d,\"frame\":%d,\"deleted\":%d,\"slot\":%d,\"ability\":%d,\"set\":%d}",
                    nStep, step.op, step.s[0], ticks, remaining, valid,
                    attackDamage ? actionId : 0, result, source->m_attackFrame, deleted,
                    source->m_equipment.m_selectedWeapon, source->m_equipment.m_selectedWeaponAbility, source->m_nWeaponSet);
                if (finalShot && source->m_equipment.m_items[11] == lastUnit) delete lastUnit;
                delete source->m_curProjectile;
                source->m_curProjectile = pending;
                source->m_attackFrame = frame;
                source->m_speedFactor = speed;
                source->field_5630 = active;
                source->field_5632 = offhand;
                source->field_5636 = count;
                source->field_7118 = attackState;
                source->m_bAllowEffectListCall = allow;
                source->m_equipment.m_items[11] = oldAmmo;
                source->m_equipment.m_items[43] = oldMain;
                source->m_equipment.m_items[44] = oldOff;
                memcpy(&source->m_equipment.m_items[12], oldReserves, sizeof(oldReserves));
                source->m_equipment.m_selectedWeapon = selected;
                source->m_equipment.m_selectedWeaponAbility = selectedAbility;
                source->m_nWeaponSet = weaponSet;
                if (finalShot) {
                    source->m_equipment.m_selectedWeapon = 255;
                    source->SelectWeaponAbility(selected, static_cast<BYTE>(selectedAbility), FALSE, TRUE);
                }
                memcpy(source->field_3D3A, cachedAmmo, sizeof(cachedAmmo));
                for (INT i = 0; i < 8; ++i) source->m_quickWeapons[i] = buttons[i];
                for (INT i = 0; i < CGAMESAVECHARACTER_NUM_STATS_WEAPONS; ++i) {
                    source->m_cGameStats.m_pWeaponStats[i].m_cResRef = weaponRefs[i];
                    source->m_cGameStats.m_pWeaponStats[i].m_nTimesUsed = weaponUses[i];
                }
                ability->thac0Bonus = hitBonus;
                static_cast<CDerivedStatsTemplate&>(source->m_derivedStats) = savedStats;
                source->m_baseStats = savedBase;
                source->m_typeAI = savedType;
                source->m_curAction = savedAction;
                source->m_actionCount = actionCount;
                source->m_nLastActionReturn = lastReturn;
                source->field_5614 = runawayCount;
                source->m_pos = sourcePos;
                source->m_lastActionID = lastAction;
                source->field_70EE = feedbackTarget;
                target->m_derivedStats.m_onHitSpells = retaliation;
                target->field_58C = team;
                target->m_typeAI = targetType;
            }
        }
        if (valid && baseDamage) {
            Emit("{\"step\":%d,\"op\":\"damage-context\",\"item\":\"%s\",\"derivedCrushing\":%d,\"tempCrushing\":%d,\"tempMaxHP\":%d,\"difficulty\":%lu,\"multiplier\":%ld,\"mpMultiplier\":%ld,\"baseCrushing\":%d,\"barbarianLevel\":%u,\"nameRef\":%lu,\"x\":%ld,\"y\":%ld}",
                nStep, step.s[0], target->m_derivedStats.m_nResistCrushing, target->m_tempStats.m_nResistCrushing,
                target->m_tempStats.m_nMaxHitPoints, pGame->m_cOptions.m_nDifficultyLevel,
                pGame->m_cOptions.m_nDifficultyMultiplier, pGame->m_cOptions.m_nMPDifficultyMultiplier,
                target->m_baseStats.m_resistCrushingBase, target->m_baseStats.m_barbarianLevel,
                target->m_baseStats.m_name, target->m_pos.x, target->m_pos.y);
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
        if (valid && !baseDamage && !swingDamage) {
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
        LONG height = valid && !swingDamage ? projectile->DetermineHeight(source) : 0;
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
        s_payloadDamage = swingDamage ? (hasLauncher ? (finalMode == 3 ? 24 : 12) : 6)
            : (baseDamage ? (hasLauncher ? 9 : 5) : (hasLauncher ? 3 : 1));
        if (swingOnHit) ++s_payloadDamage;
        s_payloadTarget = targetId;
        if (!swingDamage) projectile->Fire(area, sourceId, targetId, targetPos, height, 0);
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

    if (strcmp(step.op, "projectilecompress") == 0) {
        CGameObjectArray* objects = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
        CProjectile* unregistered = new CProjectileInstant();
        BOOLEAN result = unregistered->CompressTime(901);
        BOOL retained = unregistered->m_id == CGameObjectArray::INVALID_INDEX;
        Emit("{\"op\":\"projectile-compress\",\"case\":\"unregistered\",\"returned\":%d,\"retained\":%d}", result, retained);
        delete unregistered;
        if (result || !retained) {
            Fail(nStep, step.op, "unregistered projectile changed");
            return;
        }
        CProjectile* registered = new CProjectileInstant();
        BYTE rc = objects->Add(&registered->m_id, registered, INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            delete registered;
            Fail(nStep, step.op, "registration failed");
            return;
        }
        LONG id = registered->m_id;
        result = registered->CompressTime(901);
        CGameObject* after = NULL;
        rc = objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &after, INFINITE);
        BOOL removed = rc == CGameObjectArray::DELETED;
        if (rc == CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            registered->RemoveSelf();
        }
        Emit("{\"op\":\"projectile-compress\",\"case\":\"registered\",\"returned\":%d,\"removed\":%d}", result, removed);
        if (result || !removed) {
            Fail(nStep, step.op, "registered projectile not removed");
            return;
        }
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
