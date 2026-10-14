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
//   combatmessagechecks         verify effect/projectile packets and network
//                                receipt; remove launches and restore fixtures.
//   effectpacketdamage          receive a damage effect on portrait 1; require
//                                three HP lost and no damage to portrait 0.
//   groupedpacketdamage         receive two damage effects immediately and
//                                deferred; require five HP per ordered batch.
//   receivedonhitdamage         receive message 117, trigger its installed spell
//                                and use onhitwait to require real HP loss.
//   weaponsoundchecks          verify pending attack sounds and animation frames;
//                                restore creature lists, speech and item headers.
//   damagearrowchecks          verify offscreen party locators on the back surface;
//                                restore camera, actor, locator and sampled pixels.
//   effectcontrollerchecks     verify VEF updates and complete HandleEffects passes;
//                                restore owning stats, lists and registered fixtures.
//   vefloadchecks              verify raw VEF resources and both projectile factories;
//                                restore the temporary key, resources, objects and queues.
//   vefmessagechecks           verify trailing VEF packets and immediate/deferred MG
//                                receipt; restore remote identifiers and network state.
//   reboundchecks              exercise bolt launch, expiry and wall/corner rebounds;
//                                restore search data, actors, objects and queues.
//   sparklechecks              exercise palettes, particle lifecycle and projectile
//                                trail creation; restore objects, search and queues.
//   sparklefamilychecks        exercise explosive/trap/spell-hit trails and timers;
//                                restore objects, queues, cursor and Time Stop.
//   projectilelaunchchecks     verify both travelling launches and sparkle expiry;
//   projectileaimchecks        replay canonical native motion and RNG states;
//   projectiletickchecks       verify both travelling AI ticks and homing;
//                                restore actors, objects, queues and Time Stop.
//   explodingrenderchecks     compare flight/detonation BAM pixels and render gates;
//   vfxrenderchecks           compare VFX masks, translucency, shadow bounds and sequences;
//   beamrenderchecks          verify segmented beam pixels, rejection and height stepping;
//   beamchecks                verify beam launch, ticks, polygon strikes and delivery;
//   chainchecks               verify chain history, child launch and ordered payloads;
//   nearestchecks             replay area selection and verify three chain impacts;
//                                restore camera, search, visibility and back pixels.
//   sparklewait                require emitted clusters to leave all area lists.
//   rebounddamage              service a registered bolt beside portrait 1, then
//                                use projectilewait to require actual payload damage.
//   projectiledamage <flags> [trail] decode/apply opcode 430 with ZRCHIT on portrait 1;
//                                optional test-only activation of the native trail flag.
//   projectilewait              require three target damage and no source damage
//                                from that projectile on normal engine ticks.
//   targetgatechecks            check recovered spell target/invisibility gates;
//                                restore both actors in the same tick.
//   spellusechecks              check spell usefulness, visibility and scripted
//                                selection/trigger dispatch; restore all state.
//   targetresolvechecks         check target fixation, markers and formation
//                                requests through the real action dispatcher.
//   targettriggerchecks         check stored targets, selector predicates and HP
//
// Results are one JSON object per line to IWD2_RE_UI_RESULT, last line the
// verdict. A failed assertion stops the script.

#include "AutoUI.h"

#include "CVisualEffect.h"
#include <new>

#include <cmath>
#include <algorithm>
#include <type_traits>

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
#include "CGameAnimationType.h"
#include "CGameObject.h"
#include "CGameObjectArray.h"
#include "CGameRemoteObjectArray.h"
#include "CGameOptions.h"
#include "CGameSprite.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameEffect.h"
#include "CItem.h"
#include "CMessage.h"
#include "CMessageDoorOpen.h"
#include "CProjectileSPBOOM.h"
#include "CNetwork.h"
#include "CPathSearch.h"
#include "CVariableHash.h"
#include "CProjectile.h"
#include "CGameTemporal.h"
#include "CSpell.h"
#include "IcewindCGameEffects.h"
#include "CDimmKeyTable.h"
#include "CInfGame.h"
#include "CInfButtonArray.h"
#include "CVidInf.h"
#include "CSparkleCluster.h"
#include "CParticle.h"
#include "CInfinity.h"
#include "CScreenWorld.h"
#include "CUIControlBase.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
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

// Fake provider/connection fields must never reach the real transport loop.
// Local player zero guards its initial wait only: an already-running inner
// loop keeps servicing until ThreadLoop returns negative. Take both transport
// locks before pausing our message thread, and resume only after restoration.
struct SimulatedTransportPause {
    CNetwork& network;
    HANDLE thread;
    DWORD previous;
    explicit SimulatedTransportPause(CNetwork& value) : network(value), thread(g_pChitin->m_hMessageThread) {
        EnterCriticalSection(&network.m_critSectQueue);
        EnterCriticalSection(&network.m_critSect);
        previous = SuspendThread(thread);
        UTIL_ASSERT_MSG(previous != static_cast<DWORD>(-1), "AutoUI transport fixture could not pause its message thread");
    }
    ~SimulatedTransportPause() {
        DWORD count = ResumeThread(thread);
        LeaveCriticalSection(&network.m_critSect);
        LeaveCriticalSection(&network.m_critSectQueue);
        UTIL_ASSERT_MSG(count == previous + 1, "AutoUI transport fixture did not restore its thread suspend count");
    }
};
const int MAX_NUMS = 3;
// Ticks to let the engine settle after a click before the next step runs.
const int CLICK_SETTLE_TICKS = 2;
const int DEFAULT_WAIT_TICKS = 3000;

// These animation fixtures exercise the recovered prefix only. They supply
// explicit state and object-array retirement while VEF resource loading and the
// full CVEFVidCell constructor/destructor remain unrecovered.
class VefCellFixture : public CVidCell {
public:
    INT advances = 0;
    BOOL FrameAdvance() override { ++advances; return TRUE; }
};

class VefAnimationFixture : public CVEFVidCell {
public:
    INT* deleted;
    explicit VefAnimationFixture(INT* count) : deleted(count)
    {
        m_bltFlagsExtra = 1;
        m_offset = CPoint(0, 0);
        m_offsetFlags = 0;
        m_animationSpeed = 15;
        m_direction = 0;
        m_directionFlags = 0;
        m_nDuration = -1;
        m_cVidCell = NULL;
        m_cShadowVidCell = NULL;
        m_done = FALSE;
        m_newDirection = 0;
    }
    ~VefAnimationFixture() override
    {
        if (deleted != NULL) ++*deleted;
        if (m_id != CGameObjectArray::INVALID_INDEX) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(
                m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
        }
    }
};

class EffectPassFixture : public CGameEffect {
public:
    INT calls = 0;
    INT retryPasses = 0;
    BOOL result = TRUE;
    BOOL clearEquippedRetry = FALSE;
    SHORT bonus = 0;
    CVEFVidCell* animation = NULL;
    BOOL sawDeleteOnEveryPass = TRUE;
    BOOL ResolveEffect(CGameSprite* actor) override
    {
        ++calls;
        m_forceRepass = calls <= retryPasses;
        if (clearEquippedRetry) actor->m_equipedEffectList.m_retry = FALSE;
        actor->m_bonusStats.m_nDamageBonus += bonus;
        if (animation != NULL) {
            sawDeleteOnEveryPass = sawDeleteOnEveryPass && (animation->m_bltFlagsExtra & 0x20) != 0;
            animation->m_bltFlagsExtra &= ~0x20u;
        }
        return result;
    }
};

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

void EffectControllerChecks(INT nStep)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* objects = game->GetObjectArray();
    LONG actorId = game->GetFixedOrderCharacterId(0);
    CGameObject* raw = NULL;
    BYTE rc = objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE);
    if (rc != CGameObjectArray::SUCCESS || raw == NULL || raw->GetObjectType() != CGameObject::TYPE_SPRITE) {
        if (rc == CGameObjectArray::SUCCESS) objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Fail(nStep, "effectcontrollerchecks", "requires party sprite"); return;
    }
    CGameSprite* actor = static_cast<CGameSprite*>(raw);
    BOOL valid = TRUE;
    INT checks = 0;
    auto check = [&](const char* name, BOOL passed) {
        valid = valid && passed; ++checks;
        Emit("{\"step\":%d,\"op\":\"effectcontroller-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
    };
    auto balanced = [&](LONG id) {
        auto& entry = objects->m_pArray[(id >> 16) & 0x7FFF];
        return entry.m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == 0;
    };
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT deleted = 0;
    CVisualEffect controller;
    check("constructor", controller.m_activeAnimations.IsEmpty() && controller.m_timedEvents.IsEmpty()
        && controller.m_triggeredEvents.IsEmpty() && controller.m_nTargetId == -1
        && controller.m_nSpeed == -1 && controller.m_nDuration == -1 && controller.m_flags == 0
        && controller.m_ptStartDeltaV == CPoint(0, 0) && controller.m_nDeltaVDec == 0
        && controller.m_posExact == CPoint(0, 0) && controller.m_posOld == CPoint(0, 0)
        && controller.m_posDelta == CPoint(0, 0) && controller.m_ptTargetPos == CPoint(0, 0)
        && controller.m_nExistanceCounter == 0 && controller.m_nDirection == 0 && controller.m_nNewDirection == 0
        && memcmp(controller.m_terrainTable, CGameObject::DEFAULT_TERRAIN_TABLE, 16) == 0);
    check("base-layout", sizeof(CVisualEffectBase) == 0x1C0 && sizeof(CVEFVidCellBase) == 0x1E4 && sizeof(CVEFEvent) == 0xE0
        && reinterpret_cast<BYTE*>(static_cast<CVisualEffectBase*>(&controller)) - reinterpret_cast<BYTE*>(&controller) == sizeof(CGameObject));
    VefCellFixture cell, shadow;
    VefAnimationFixture animation(NULL);
    animation.m_cVidCell = &cell; animation.m_cShadowVidCell = &shadow;
    check("animation-prefix-layout", reinterpret_cast<BYTE*>(static_cast<CVEFVidCellBase*>(&animation)) - reinterpret_cast<BYTE*>(&animation) == sizeof(CGameObject)
        && reinterpret_cast<BYTE*>(&animation.m_cVidCell) - reinterpret_cast<BYTE*>(static_cast<CVEFVidCellBase*>(&animation)) == 0x1E4);
    const INT expectedAdvances[29] = {2, 4, 6, 9, 10, 12, 15, 20, 20, 20, 30, 30, 30, 30, 30, 30, 30, 30, 30, 40, 40, 40, 45, 48, 50, 51, 54, 56, 58};
    for (INT speed = 1; speed < 30; ++speed) {
        animation.m_animationSpeed = speed; cell.advances = shadow.advances = 0;
        for (INT tick = 0; tick < 60; ++tick) animation.FrameAdvance(tick);
        check("frame-positive-cadence", cell.advances == expectedAdvances[speed - 1] && shadow.advances == cell.advances && animation.m_done == FALSE);
        cell.advances = shadow.advances = 0;
        for (INT tick = -59; tick <= 0; ++tick) animation.FrameAdvance(tick);
        check("frame-negative-cadence", cell.advances == expectedAdvances[speed - 1] && shadow.advances == cell.advances);
    }
    animation.m_animationSpeed = 1; animation.m_cShadowVidCell = NULL;
    cell.advances = 0; animation.FrameAdvance(1);
    check("skipped-no-shadow", cell.advances == 0);
    animation.FrameAdvance(0);
    check("advanced-no-shadow", cell.advances == 1);
    animation.m_nDuration = 4; animation.m_done = FALSE; animation.FrameAdvance(4);
    check("duration-strict-boundary", animation.m_done == FALSE);
    animation.FrameAdvance(5);
    check("duration-on-skipped-frame", animation.m_done == TRUE && cell.advances == 1);
    animation.m_nDuration = -1; animation.m_done = 7; animation.FrameAdvance(0x7FFFFFFF);
    check("negative-duration-preserves-done", animation.m_done == 7);
    animation.m_nDuration = 0; animation.m_done = FALSE; animation.FrameAdvance(0x7FFFFFFF);
    check("zero-duration-never-expires", animation.m_done == FALSE);
    CVidCell realCell(game->m_vcLocator.cResRef, FALSE);
    BYTE frameCount = realCell.GetSequenceLength(0, FALSE);
    check("real-bam-available", frameCount != 0);
    if (frameCount != 0) {
        realCell.SequenceSet(0); realCell.FrameSet(frameCount - 1);
        animation.m_cVidCell = &realCell; animation.m_bltFlagsExtra = 0; animation.FrameAdvance(0);
        check("nonloop-end-completion", animation.m_done == TRUE);
        animation.m_done = FALSE; animation.m_bltFlagsExtra = 1;
        realCell.FrameSet(frameCount - 1); animation.FrameAdvance(0);
        check("loop-end-retained", animation.m_done == FALSE);
    }
    animation.m_cVidCell = &cell; animation.m_nDuration = -1; animation.m_done = FALSE;
    const CPoint rotated[16] = {CPoint(1000,1000), CPoint(541,1307), CPoint(0,1414), CPoint(-541,1307),
        CPoint(-1000,1000), CPoint(-1307,541), CPoint(-1414,0), CPoint(-1307,-541),
        CPoint(-1000,-1000), CPoint(-541,-1307), CPoint(0,-1414), CPoint(541,-1307),
        CPoint(1000,-1000), CPoint(1307,-541), CPoint(1414,0), CPoint(1307,541)};
    animation.m_offset = CPoint(1000, 1000); animation.m_offsetFlags = 1;
    for (INT direction = 0; direction < 16; ++direction) {
        animation.m_newDirection = direction; animation.CalculatePos(CPoint(0, 0));
        check("position-all-directions", animation.m_pos == rotated[direction]);
    }
    animation.m_offset = CPoint(3, -2); animation.m_newDirection = 1;
    animation.CalculatePos(CPoint(100, 200));
    check("position-signed-truncation", animation.m_pos == CPoint(103, 200));
    animation.m_offsetFlags = 2; animation.CalculatePos(CPoint(100, 200));
    check("position-bit-one-only", animation.m_pos == CPoint(103, 198));
    animation.m_offset = CPoint(1, -1); animation.CalculatePos(CPoint(0x7FFFFFFF, static_cast<LONG>(0x80000000u)));
    check("position-long-wrap", animation.m_pos == CPoint(static_cast<LONG>(0x80000000u), 0x7FFFFFFF));
    animation.m_direction = 0x1234FFFF; animation.m_directionFlags = 3; animation.m_pos = CPoint(100, 200);
    animation.SetDirection(CPoint(0, 0), CPoint(10, 0), CPoint(100, 200), 6);
    check("direction-target-precedence-signed-word", animation.m_newDirection == -1);
    animation.SetDirection(CPoint(0, 0), CPoint(10, 0), CPoint(200, 200), 6);
    check("direction-target-east", animation.m_newDirection == 12);
    animation.m_directionFlags = 2; animation.SetDirection(CPoint(0, 0), CPoint(10, 0), CPoint(200, 200), -1234);
    check("direction-explicit-word", animation.m_newDirection == -1234);
    animation.m_directionFlags = 0; animation.m_direction = 0x12348000;
    animation.SetDirection(CPoint(10, 0), CPoint(10, 0), CPoint(200, 200), 6);
    check("direction-stationary-word", animation.m_newDirection == -32768);
    animation.SetDirection(CPoint(10, 0), CPoint(0, 0), CPoint(200, 200), 6);
    check("direction-movement-east", animation.m_newDirection == 12);
    check("direction-static-stationary", CVEFVidCell::GetDirection(CPoint(-5, 2), CPoint(-5, 2), -1234) == -1234);
    check("direction-static-south", CVEFVidCell::GetDirection(CPoint(0, 0), CPoint(0, 10), 6) == 0);
    controller.m_pos = controller.m_posExact = CPoint(0, 0); controller.m_nSpeed = 4;
    controller.Move(CPoint(5, 0));
    check("move-strict-stop-radius", controller.m_pos == CPoint(0, 0) && controller.m_posDelta == CPoint(0, 0));
    controller.Move(CPoint(6, 0));
    check("move-axis-fixed-point", controller.m_pos == CPoint(4, 0) && controller.m_posExact == CPoint(4096, 0) && controller.m_nNewDirection == 12);
    controller.m_pos = controller.m_posExact = CPoint(0, 0); controller.m_nSpeed = 1;
    controller.Move(CPoint(2, 2));
    check("move-rounded-distance", controller.m_posDelta == CPoint(682, 682) && controller.m_pos == CPoint(0, 0));
    controller.m_pos = CPoint(0, 0); controller.m_posExact = CPoint(-1, -1); controller.m_nSpeed = 0;
    controller.Move(CPoint(100, 0));
    check("move-negative-exact-y-truncation", controller.m_pos == CPoint(-1, 0));
    controller.m_pos = controller.m_posExact = CPoint(0, 0); controller.m_nSpeed = 46340;
    controller.Move(CPoint(65536, 0));
    check("move-wrapped-zero-length", controller.m_posDelta == CPoint(1, 1));
    const CPoint velocity[4] = {CPoint(-1, 1), CPoint(-4, 4), CPoint(-5, 5), CPoint(0, 0)};
    const CPoint decayed[4] = {CPoint(0, 0), CPoint(0, 0), CPoint(-1, 1), CPoint(0, 0)};
    for (INT i = 0; i < 4; ++i) {
        controller.m_pos = controller.m_posExact = CPoint(0, 0); controller.m_nSpeed = 0;
        controller.m_ptStartDeltaV = velocity[i]; controller.m_nDeltaVDec = 4;
        controller.Move(CPoint(0, 100));
        check("move-velocity-decay", controller.m_ptStartDeltaV == decayed[i]);
    }
    rc = objects->Add(&controller.m_id, &controller, INFINITE);
    check("controller-registration", rc == CGameObjectArray::SUCCESS);
    auto addAnimation = [&]() -> VefAnimationFixture* {
        auto* child = new VefAnimationFixture(&deleted);
        child->m_cVidCell = &cell;
        BYTE added = objects->Add(&child->m_id, child, INFINITE);
        check("animation-registration", added == CGameObjectArray::SUCCESS);
        if (added != CGameObjectArray::SUCCESS) { delete child; return NULL; }
        controller.m_activeAnimations.AddTail(child->m_id);
        return child;
    };
    auto* child = addAnimation();
    if (rc == CGameObjectArray::SUCCESS && child != NULL) {
        LONG childId = child->m_id;
        controller.m_activeAnimations.AddHead(-1);
        child->m_bltFlagsExtra = 0x80000004u;
        controller.SetAllDelete();
        check("delete-flag-preserves-dword-and-list", child->m_bltFlagsExtra == 0x80000024u && controller.m_activeAnimations.GetCount() == 2 && deleted == 0 && balanced(childId));
        controller.SetAllDelete();
        check("delete-flag-idempotent", child->m_bltFlagsExtra == 0x80000024u && balanced(childId));
        controller.m_pos = controller.m_ptTargetPos = CPoint(0, 0); controller.m_nSpeed = 0;
        controller.m_nExistanceCounter = 0x7FFFFFFF; controller.AIUpdate();
        check("flagged-child-retirement", deleted == 1 && controller.m_activeAnimations.GetCount() == 1 && controller.m_activeAnimations.GetHead() == -1);
        check("update-counter-wrap", controller.m_nExistanceCounter == static_cast<LONG>(0x80000000u) && controller.m_posOld == controller.m_pos);
        controller.m_activeAnimations.RemoveAll();
        child = addAnimation();
        if (child != NULL) {
            childId = child->m_id;
            controller.m_pos = controller.m_ptTargetPos = CPoint(20, 30); controller.m_posOld = CPoint(10, 30);
            controller.m_nExistanceCounter = 0; controller.m_flags = 1; controller.m_posZ = 123;
            child->m_bltFlagsExtra = 5; child->m_offset = CPoint(3, -2); cell.advances = 0;
            controller.AIUpdate();
            check("live-child-update", child->m_pos == CPoint(23, 28) && child->m_posZ == 123 && child->m_newDirection == 12
                && cell.advances == 1 && controller.m_nExistanceCounter == 1 && controller.m_posOld == CPoint(20, 30) && balanced(childId));
            controller.m_flags = 0; controller.m_posZ = 456; child->m_nDuration = 1;
            controller.AIUpdate();
            check("height-gated-and-duration-boundary", child->m_posZ == 123 && deleted == 1 && balanced(childId));
            controller.AIUpdate();
            check("completed-child-retirement", deleted == 2 && controller.m_activeAnimations.IsEmpty());
        }
        child = addAnimation();
        if (child != NULL) {
            // Park the two owning stats objects at their original addresses.
            // The recovered assignment is partial and MFC list copies are not
            // safe snapshots. Fresh stats own only fixture allocations; those
            // are destroyed before restoring the exact parked bytes/resources.
            std::vector<BYTE> derived(sizeof(CDerivedStats)), bonus(sizeof(CDerivedStats));
            memcpy(derived.data(), &actor->m_derivedStats, derived.size());
            memcpy(bonus.data(), &actor->m_bonusStats, bonus.size());
            new (&actor->m_derivedStats) CDerivedStats;
            new (&actor->m_bonusStats) CDerivedStats;
            std::vector<CGameEffect*> equipped, timed;
            while (!actor->m_equipedEffectList.IsEmpty()) equipped.push_back(actor->m_equipedEffectList.RemoveHead());
            while (!actor->m_timedEffectList.IsEmpty()) timed.push_back(actor->m_timedEffectList.RemoveHead());
            auto base = actor->m_baseStats;
            auto ai = actor->m_typeAI; auto liveAi = actor->m_liveTypeAI;
            BOOL imprisoned = actor->m_activeImprisonment;
            LONG visualId = actor->field_72E2;
            BOOL eqRetry = actor->m_equipedEffectList.m_retry, timeRetry = actor->m_timedEffectList.m_retry;
            auto* engine = static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine);
            INT picked = engine->m_nPickedCharacter;
            auto& bar = game->m_cButtonArray;
            INT barState = bar.m_nState; INT barTypes[12]; memcpy(barTypes, bar.m_buttonTypes, sizeof(barTypes));
            EffectPassFixture eq, time;
            eq.retryPasses = 1; eq.bonus = 3; eq.animation = child;
            time.clearEquippedRetry = TRUE; time.bonus = 5;
            actor->m_equipedEffectList.AddTail(&eq); actor->m_timedEffectList.AddTail(&time);
            actor->field_72E2 = controller.m_id; actor->m_activeImprisonment = FALSE;
            engine->m_nPickedCharacter = -1; bar.m_nState = 0x6F; bar.m_buttonTypes[0] = -1234;
            BOOL handled = actor->HandleEffects();
            check("effects-retry-captured-before-timed", handled == TRUE && eq.calls == 2 && time.calls == 2 && eq.sawDeleteOnEveryPass);
            check("effects-bonus-reset-each-pass", actor->m_derivedStats.m_nDamageBonus == 8 && actor->m_activeImprisonment == TRUE);
            check("unpicked-bar-unchanged", bar.m_buttonTypes[0] == -1234);
            eq.retryPasses = 0; eq.animation = NULL; eq.result = 2;
            check("effects-even-bool-mask", actor->HandleEffects() == FALSE);
            eq.result = 3; time.result = 2;
            check("effects-timed-even-bool-mask", actor->HandleEffects() == FALSE);
            time.result = 3;
            check("effects-odd-bool-mask", actor->HandleEffects() == TRUE);
            eq.result = time.result = TRUE;
            actor->m_baseStats.m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] = 255;
            actor->m_baseStats.m_INTBase = 10;
            actor->HandleEffects();
            check("effects-signed-byte-skill-floor", actor->m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 0);
            actor->m_baseStats.m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] = 126;
            actor->m_baseStats.m_INTBase = 14;
            actor->HandleEffects();
            check("effects-skill-positive-overflow-floor", actor->m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 0);
            actor->m_baseStats.m_skills[CGAMESPRITE_SKILL_DISABLE_DEVICE] = 4;
            actor->HandleEffects();
            check("effects-normal-skill-fold", actor->m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_DISABLE_DEVICE] == 6);
            engine->m_nPickedCharacter = game->GetCharacterPortraitNum(actorId); bar.m_buttonTypes[0] = -1234;
            actor->HandleEffects(); check("picked-bar-refreshed", bar.m_buttonTypes[0] == 100);
            actor->m_id = -1; engine->m_nPickedCharacter = -1; bar.m_buttonTypes[0] = -1234;
            actor->HandleEffects(); check("minus-one-portrait-refresh", bar.m_buttonTypes[0] == 100);
            actor->m_id = actorId; actor->field_72E2 = -1;
            check("missing-controller-effects-still-run", actor->HandleEffects() == TRUE);
            actor->m_equipedEffectList.RemoveAll(); actor->m_timedEffectList.RemoveAll();
            for (auto* effect : equipped) actor->m_equipedEffectList.AddTail(effect);
            for (auto* effect : timed) actor->m_timedEffectList.AddTail(effect);
            actor->m_equipedEffectList.m_retry = eqRetry; actor->m_timedEffectList.m_retry = timeRetry;
            actor->m_baseStats = base; actor->m_typeAI = ai; actor->m_liveTypeAI = liveAi;
            actor->m_activeImprisonment = imprisoned; actor->field_72E2 = visualId;
            actor->m_derivedStats.~CDerivedStats(); actor->m_bonusStats.~CDerivedStats();
            memcpy(&actor->m_derivedStats, derived.data(), derived.size());
            memcpy(&actor->m_bonusStats, bonus.data(), bonus.size());
            engine->m_nPickedCharacter = picked; bar.SetState(barState, 0); memcpy(bar.m_buttonTypes, barTypes, sizeof(barTypes));
            check("effects-controller-deny-balanced", balanced(controller.m_id) && balanced(child->m_id));
        }
    }
    controller.Render(raw->m_pArea, g_pChitin->GetCurrentVideoMode(), 0);
    controller.SetAllDelete(); controller.AIUpdate();
    check("remaining-controller-child-retired", controller.m_activeAnimations.IsEmpty());
    if (controller.m_id != -1) { objects->Delete(controller.m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE); controller.m_id = -1; }
    // Destructor owns the remaining child, both event lists and the area link.
    {
        CVisualEffect* owned = new CVisualEffect;
        owned->m_activeAnimations.AddTail(-1);
        auto* remaining = new VefAnimationFixture(&deleted);
        if (objects->Add(&remaining->m_id, remaining, INFINITE) == CGameObjectArray::SUCCESS) {
            owned->m_activeAnimations.AddTail(remaining->m_id);
            owned->m_timedEvents.AddTail(new CVEFEvent); owned->m_timedEvents.AddTail(new CVEFEvent);
            owned->m_triggeredEvents.AddTail(new CVEFEvent);
            owned->m_pArea = raw->m_pArea;
            INT before = deleted; delete owned;
            check("destructor-owned-child-and-events", deleted == before + 1);
        } else { delete remaining; delete owned; check("destructor-fixture-registration", FALSE); }
    }
    check("same-tick-no-clock-or-messages", game->GetWorldTimer()->m_gameTime == clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == messages);
    objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    check("actor-deny-balanced", balanced(actorId));
    Emit("{\"step\":%d,\"op\":\"effectcontrollerchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
    if (!valid) Fail(nStep, "effectcontrollerchecks", "effect controller mismatch");
}

void SparkleChecks(INT nStep)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* objects = game->GetObjectArray();
    LONG actorId = game->GetFixedOrderCharacterId(0);
    CGameObject* actor = NULL;
    if (objects->GetShare(actorId, CGameObjectArray::THREAD_ASYNCH, &actor, INFINITE)
        != CGameObjectArray::SUCCESS) {
        Fail(nStep, "sparklechecks", "requires loaded actor and area"); return;
    }
    if (actor->m_pArea == NULL) {
        objects->ReleaseShare(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Fail(nStep, "sparklechecks", "requires loaded area"); return;
    }
    CGameArea* area = actor->m_pArea;
    CPoint origin = actor->m_pos;
    objects->ReleaseShare(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"sparkle-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
    };
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    auto unqueue = [&](LONG id) {
        POSITION p = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortAdd.RemoveAt(p);
        p = area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortBackAdd.RemoveAt(p);
        p = area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortFlightAdd.RemoveAt(p);
    };
    auto retire = [&](CGameObject* object) {
        unqueue(object->m_id); object->RemoveFromArea();
        objects->Delete(object->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE); delete object;
    };
    struct Cluster : CSparkleCluster {
        BOOL* dead;
        Cluster(SHORT color, USHORT type, BOOL* d) : CSparkleCluster(color, type, 8), dead(d) {}
        ~Cluster() override { *dead = TRUE; }
    };
    const SHORT rows[12] = {5,0,6,1,8,2,7,9,3,4,10,11};
    for (SHORT color = 1; color <= 12; ++color) {
        BOOL dead = FALSE; Cluster* c = new Cluster(color, 1, &dead);
        check("ctor-state", c->m_particleList.IsEmpty() && c->m_nTimer == 0 && c->m_nGravity == 50
            && c->m_wType == 1 && c->m_nConjureDuration == 8 && c->m_rBounding.IsRectEmpty()
            && c->m_pVidMode == g_pBaldurChitin->GetCurrentVideoMode());
        COLORREF palette[] = {c->m_colorStart,c->m_colorFade1,c->m_colorFade2,c->m_colorFade3,c->m_colorFade4};
        for (LONG i = 0; i < 5; ++i) {
            UINT red, green, blue;
            sscanf(game->GetRuleTables().m_tSparkleColors.GetAt(CPoint(i,rows[color-1])), "RGB(%u,%u,%u)", &red,&green,&blue);
            check("palette-row", palette[i] == RGB(red,green,blue));
        }
        delete c; check("empty-dtor", dead);
    }
    for (LONG height : {25L,100L,500L,-7L}) {
        BOOL dead = FALSE; Cluster* c = new Cluster(2, 1, &dead);
        c->AddToArea(area, origin, height, CGameObject::LIST_FRONT);
        check("registered-front", area->m_lVertSortAdd.Find(reinterpret_cast<int*>(c->m_id)) != NULL
            && c->m_pArea == area && c->m_pos == origin && c->m_posZ == height);
        unqueue(c->m_id);
        check("travel-count-and-fade-heights", c->m_particleList.GetCount() == 14
            && c->m_fade1Height == static_cast<BYTE>(height*99/100)
            && c->m_fade2Height == static_cast<BYTE>(height*85/100)
            && c->m_fade3Height == static_cast<BYTE>(height*75/100)
            && c->m_fade4Height == static_cast<BYTE>(height*55/100));
        check("travel-bounds", c->m_rBounding == CRect(max(origin.x-5,0),max(origin.y-5-height,0),origin.x+4,origin.y+4));
        POSITION p = c->m_particleList.GetHeadPosition(); INT i = 0;
        while (p != NULL) {
            CParticle* particle = c->m_particleList.GetNext(p);
            check("initial-particle-contract", particle->m_pos.z == static_cast<LONG>(static_cast<DWORD>(height)<<10)
                && particle->m_vel.x == ((i&1) == 0 ? 4 : -4) && particle->m_vel.y == 0 && particle->m_vel.z == 0
                && particle->m_nGravity == 50+10*i && particle->m_nTailLength == 0
                && particle->m_wType == CParticle::CONSTANT_VELOCITY);
            ++i;
        }
        retire(c); check("populated-dtor", dead);
    }
    // Exact threshold equality selects the preceding band; color saturation,
    // height integration and unrelated particle clocks have independent sentinels.
    for (LONG height : {54L,55L,74L,75L,84L,85L,98L,99L,100L}) {
        BOOL dead = FALSE; Cluster* c = new Cluster(2, 4, &dead);
        objects->Add(&c->m_id, c, INFINITE); c->m_pArea = area; c->m_pos = origin;
        c->m_fade1Height=99; c->m_fade2Height=85; c->m_fade3Height=75; c->m_fade4Height=55;
        c->m_colorFade1=RGB(70,2,3); c->m_colorFade2=RGB(2,80,4);
        c->m_colorFade3=RGB(3,4,90); c->m_colorFade4=RGB(100,5,6);
        CParticle* p = new CParticle(1000,2000,height<<10,7,-9,30,RGB(11,12,13),17,5,1);
        p->m_nTimeStamp=23; p->m_nLifeSpan=31; c->m_particleList.AddTail(p);
        c->AIUpdate();
        COLORREF base = height<55 ? RGB(100,5,6) : height<75 ? RGB(3,4,90) : height<85 ? RGB(2,80,4) : RGB(70,2,3);
        INT scale=(height&3)+1;
        COLORREF expected = height>=99 ? RGB(11,12,13)
            : RGB(min(GetRValue(base)*scale,255),min(GetGValue(base)*scale,255),min(GetBValue(base)*scale,255));
        check("fade-boundaries-and-motion", !dead && p->m_rgbColor==expected && p->m_pos.x==1007
            && p->m_pos.y==1991 && p->m_pos.z==(height<<10)+25 && p->m_vel.z==25 && p->m_vel.x==7 && p->m_vel.y==-9);
        check("particle-clock-not-advanced", p->m_nTimeStamp==23 && p->m_nLifeSpan==31 && p->m_nTailLength==17);
        retire(c);
    }
    {
        BOOL dead=FALSE; Cluster* c=new Cluster(2,2,&dead);
        c->AddToArea(area,origin,100,CGameObject::LIST_FRONT); unqueue(c->m_id);
        check("conjure-bounds", c->m_rBounding==CRect(max(origin.x-13,0),max(origin.y-113,0),origin.x+13,origin.y+13));
        POSITION pos=c->m_particleList.GetHeadPosition();
        while(pos) c->m_particleList.GetNext(pos)->m_nGravity=0;
        static const CPoint minimum[] = {CPoint(3,-5),CPoint(5,5),CPoint(-5,3),CPoint(-15,5),
            CPoint(-13,-5),CPoint(-15,-15),CPoint(-5,-13),CPoint(5,-15)};
        for(INT tick=0;tick<9;++tick) {
            c->AIUpdate();
            check("conjure-duration", !dead && c->m_nTimer==min(tick+1,8)
                && c->m_particleList.GetCount()==14+14*min(tick+1,8));
            if(tick<8) {
                POSITION p=c->m_particleList.GetTailPosition();
                for(INT i=0;i<14;++i) {
                    CParticle* particle=c->m_particleList.GetPrev(p);
                    LONG x=particle->m_pos.x>>10; LONG y=particle->m_pos.y>>10;
                    LONG ymin=(origin.y+minimum[tick].y)*4/3;
                    check("conjure-eight-origins", x>=origin.x+minimum[tick].x && x<origin.x+minimum[tick].x+10
                        && y>=ymin && y<=(origin.y+minimum[tick].y+9)*4/3 && particle->m_pos.z==102400);
                }
            }
        }
        retire(c);
    }
    // Eight headings, three velocity streams and fourteen distinct gravities.
    const CPoint directions[] = {CPoint(0,100),CPoint(-100,100),CPoint(-100,0),CPoint(-100,-100),
        CPoint(0,-100),CPoint(100,-100),CPoint(100,0),CPoint(100,100)};
    const SHORT velocities[][6]={{700,700,0,1000,-700,700},{0,1000,-700,700,-1000,0},
        {-700,700,-1000,0,-700,-700},{-1000,0,-700,-700,0,-1000},
        {-700,-700,0,-1000,700,-700},{0,-1000,700,-700,1000,0},
        {700,-700,1000,0,700,700},{1000,0,700,700,0,1000}};
    for(INT heading=0;heading<8;++heading) {
        BOOL dead=FALSE; Cluster* c=new Cluster(7,4,&dead);
        c->m_explosionDir=origin+directions[heading];
        c->AddToArea(area,origin,25,CGameObject::LIST_FRONT); unqueue(c->m_id);
        check("explosion-bounds-count", c->m_particleList.GetCount()==42
            && c->m_rBounding==CRect(max(origin.x-50,0),max(origin.y-75,0),origin.x+50,origin.y+25));
        POSITION p=c->m_particleList.GetHeadPosition(); INT i=0;
        while(p) {
            CParticle* particle=c->m_particleList.GetNext(p);
            check("explosion-stream-contract", particle->m_vel.x==velocities[heading][(i/14)*2]
                && particle->m_vel.y==velocities[heading][(i/14)*2+1] && particle->m_vel.z==1000
                && particle->m_nGravity==50+10*(i%14)); ++i;
        }
        // Native ignores the passed area/video; render restores all particle colors.
        COLORREF color=c->m_particleList.GetHead()->m_rgbColor;
        c->Render(NULL,NULL,CVIDINF_SURFACE_BACK);
        check("render-color-restored", c->m_particleList.GetHead()->m_rgbColor==color);
        retire(c);
    }
    for(BOOL hold : {FALSE,TRUE}) {
        BOOL dead=FALSE; Cluster* c=new Cluster(2,2,&dead);
        c->AddToArea(area,origin,25,CGameObject::LIST_FRONT); unqueue(c->m_id);
        LONG id=c->m_id; CGameObject* shared=NULL;
        if(hold) check("delete-blocked-share", objects->GetShare(id,CGameObjectArray::THREAD_1,&shared,INFINITE)==CGameObjectArray::SUCCESS);
        POSITION pos=c->m_particleList.GetHeadPosition();
        while(pos) { CParticle* p=c->m_particleList.GetNext(pos); p->m_pos.z=1; p->m_nGravity=1; p->m_vel.z=0; }
        c->AIUpdate();
        check("last-particle-immediate-retirement", hold ? !dead && c->m_particleList.IsEmpty()
            && c->m_nTimer==0 && c->m_pArea==NULL : dead);
        if(hold) { objects->ReleaseShare(id,CGameObjectArray::THREAD_1,INFINITE); retire(c); }
    }
    // Exercise both production spawn branches through the projectile virtual.
    // Consume only the trail each invocation added, before ordinary area ticks.
    auto consumeTrail = [&](CPoint point) {
        check("one-trail-queued", area->m_lVertSortAdd.GetCount()==front+1);
        if(area->m_lVertSortAdd.GetCount()!=front+1) return;
        LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetTail());
        CGameObject* object=NULL;
        BYTE rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
        CSparkleCluster* c=rc==CGameObjectArray::SUCCESS ? dynamic_cast<CSparkleCluster*>(object) : NULL;
        check("projectile-trail-contract", c && c->m_pos==point && c->m_posZ==25
            && c->m_wType==1 && c->m_nConjureDuration==25 && c->m_particleList.GetCount()==14);
        if(rc==CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); retire(object);
        }
    };
    auto savedStop=game->m_nTimeStop; auto savedCaster=game->m_nTimeStopCaster;
    game->m_nTimeStop=0;
    struct Travel : CProjectileTravelling {
        INT aims=0, arrivals=0;
        Travel() : CProjectileTravelling(CResRef("SPMAGMIS")) {}
        void AimAtPoint(int, int) override { ++aims; m_pos.x+=16; }
        void OnArrival() override { ++arrivals; }
        void Seed(CGameArea* area) {
            m_pArea=area; CGameObject::m_pArea=area; m_pos=CPoint(40,36); m_posZ=25;
            m_targetX=400; m_targetY=36; m_velocity=1; m_flightDistSq=1;
            m_targetId=-1; m_lifetime=100; m_sparkleColor=2;
        }
    };
    {
        Travel* t=new Travel; check("travel-default-no-trail", !t->m_bSparkleTrail);
        objects->Add(&t->m_id,t,INFINITE); t->Seed(area);
        CProjectile* dispatch=t; dispatch->AIUpdate();
        check("disabled-trail-no-spawn", t->aims==1 && area->m_lVertSortAdd.GetCount()==front);
        t->m_bSparkleTrail=TRUE; dispatch->AIUpdate(); consumeTrail(CPoint(72,36));
        game->m_nTimeStop=1; game->m_nTimeStopCaster=-1;
        dispatch->AIUpdate();
        check("time-stop-no-trail", t->aims==2 && area->m_lVertSortAdd.GetCount()==front);
        game->m_nTimeStop=0; t->Seed(area); t->m_pos=CPoint(400,36); dispatch->AIUpdate();
        check("arrival-no-trail", t->arrivals==1 && t->aims==2 && area->m_lVertSortAdd.GetCount()==front);
        retire(t);
    }
    struct BoltTrail : CProjectileReboundingBolt {
        void Seed(CGameArea* area, LONG step) {
            m_pArea=area; CGameObject::m_pArea=area; m_pos=CPoint(40,36); m_posZ=25;
            m_posAccumX=40*1024; m_posAccumY=36*4096/3;
            m_stepX=step; m_stepY=0; m_nDuration=60; m_nBounces=-1;
            m_targetMap.m_serviceCountdown=5; memset(m_terrainTable,1,sizeof(m_terrainTable));
            m_bSparkleTrail=TRUE; m_sparkleColor=2;
        }
    };
    {
        BYTE dynamic[64]; memset(dynamic,0x80,sizeof(dynamic));
        CSingleLock lock(&area->m_search.m_critSect,TRUE);
        BYTE* saved=area->m_search.m_pDynamicCost; CSize grid=area->m_search.m_GridSquareDimensions;
        area->m_search.m_pDynamicCost=dynamic; area->m_search.m_GridSquareDimensions=CSize(8,8);
        BoltTrail* b=new BoltTrail; objects->Add(&b->m_id,b,INFINITE);
        b->Seed(area,1024); CProjectile* dispatch=b; dispatch->AIUpdate();
        check("same-cell-no-trail", b->m_pos==CPoint(41,36) && area->m_lVertSortAdd.GetCount()==front);
        b->Seed(area,16*1024); dispatch->AIUpdate(); consumeTrail(CPoint(56,36));
        b->Seed(area,16*1024); dynamic[3+3*8]=1; dispatch->AIUpdate(); consumeTrail(CPoint(24,36));
        retire(b);
        area->m_search.m_pDynamicCost=saved; area->m_search.m_GridSquareDimensions=grid;
    }
    game->m_nTimeStop=savedStop; game->m_nTimeStopCaster=savedCaster;
    check("same-tick-restoration", game->GetWorldTimer()->m_gameTime==clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages
        && area->m_lVertSortAdd.GetCount()==front && area->m_lVertSortBackAdd.GetCount()==back
        && area->m_lVertSortFlightAdd.GetCount()==flight);
    srand(1);
    Emit("{\"step\":%d,\"op\":\"sparklechecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep,checks,valid?"true":"false");
    if(!valid) Fail(nStep,"sparklechecks","particle lifecycle mismatch");
}

// Virtual interception keeps arrivals/strikes local while exercising each
// production AIUpdate, its real allocations and object/area registration.
template <class T>
struct SparkleExplodingFixture : T {
    using T::T;
    INT aims = 0, arrivals = 0, removals = 0, explosions = 0;
    CPoint aimed;
    void AimAtPoint(int x, int y) override { ++aims; aimed = CPoint(x,y); this->m_pos.x += 16; }
    void OnArrival() override { ++arrivals; }
    void RemoveSelf() override { ++removals; }
    void Explode() override { ++explosions; }
    void Seed(CGameArea* area) {
        this->m_pArea = area; this->CGameObject::m_pArea = area;
        this->m_pos = CPoint(40,36); this->m_posZ = 25;
        this->m_targetX = 400; this->m_targetY = 36; this->m_velocity = 1;
        this->m_stepX = 0; this->m_stepY = 0; this->m_targetId = -1;
        this->m_sparkleColor = 2; this->m_bSparkleTrail = TRUE;
        this->m_nState = 0; this->m_bExplodeCell1Active = 0; this->m_bExplodeCell2Active = 0;
        this->m_bBurstPending = 0; this->m_lingerPeriod = 7; this->m_lingerCountdown = 0;
        this->m_strikesLeft = 2; this->m_bPreScan = 1;
        this->m_preCheckRange = 0; this->m_strikeRange = 0;
        this->m_targetType.SetInstance(0x7FFFFFFF);
        this->m_posVertList = NULL;
    }
    void Position(CPoint point, SHORT velocity = 1) { this->m_pos = point; this->m_velocity = velocity; }
    void Lingering(SHORT countdown) { this->m_nState = 1; this->m_lingerCountdown = countdown; }
    SHORT Countdown() const { return this->m_lingerCountdown; }
    SHORT Charges() const { return this->m_strikesLeft; }
    void ScanFrom(POSITION pos) { this->m_posVertList = pos; }
};

void SparkleFamilyChecks(INT nStep)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* objects = game->GetObjectArray();
    LONG actorId = game->GetFixedOrderCharacterId(0);
    CGameObject* actor = NULL;
    if (objects->GetShare(actorId,CGameObjectArray::THREAD_ASYNCH,&actor,INFINITE)
        != CGameObjectArray::SUCCESS) { Fail(nStep,"sparklefamilychecks","requires loaded actor"); return; }
    CGameArea* area = actor->m_pArea;
    CPoint actorPoint = actor->GetPos();
    objects->ReleaseShare(actorId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    if (!area || area->m_lVertSort.IsEmpty()) { Fail(nStep,"sparklefamilychecks","requires area front list"); return; }
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount();
    INT flight = area->m_lVertSortFlightAdd.GetCount(), sorted = area->m_lVertSort.GetCount();
    auto stop = game->m_nTimeStop; auto caster = game->m_nTimeStopCaster;
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"sparkle-family-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    auto unqueue = [&](LONG id) {
        POSITION p = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if(p) area->m_lVertSortAdd.RemoveAt(p);
        p = area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id)); if(p) area->m_lVertSortBackAdd.RemoveAt(p);
        p = area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id)); if(p) area->m_lVertSortFlightAdd.RemoveAt(p);
    };
    auto retire = [&](CGameObject* object) {
        unqueue(object->m_id); object->RemoveFromArea();
        objects->Delete(object->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete object;
    };
    auto consume = [&](INT sparkles, INT puffs, CPoint point) {
        check("queued-spawn-count",area->m_lVertSortAdd.GetCount()==front+sparkles+puffs);
        INT clusters = 0, temporals = 0;
        while(area->m_lVertSortAdd.GetCount()>front) {
            LONG id = reinterpret_cast<LONG>(area->m_lVertSortAdd.GetTail());
            CGameObject* object = NULL;
            BYTE rc = objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
            check("spawn-is-registered",rc==CGameObjectArray::SUCCESS);
            if(rc!=CGameObjectArray::SUCCESS) { unqueue(id); continue; }
            CSparkleCluster* c = dynamic_cast<CSparkleCluster*>(object);
            CGameTemporal* t = dynamic_cast<CGameTemporal*>(object);
            if(c) {
                ++clusters;
                check("sparkle-after-motion",c->m_pos==point && c->m_posZ==25 && c->m_pArea==area
                    && c->m_wType==CSparkleCluster::TYPE_TRAVEL && c->m_nConjureDuration==25
                    && c->m_particleList.GetCount()==14);
            } else if(t) {
                ++temporals;
                check("flame-puff-jitter-height",t->m_pos.x>=point.x-2 && t->m_pos.x<=point.x+2
                    && t->m_pos.y>=point.y-2 && t->m_pos.y<=point.y+2 && t->m_posZ==-25 && t->m_pArea==area);
            } else check("expected-spawn-class",FALSE);
            objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
            retire(object);
        }
        check("sparkles-and-puffs-coexist",clusters==sparkles && temporals==puffs);
    };
    game->m_nTimeStop = 0;
    auto exercise = [&](auto* p, INT puffs, INT lingerReload) {
        objects->Add(&p->m_id,p,INFINITE); p->Seed(area);
        CProjectile* dispatch = p;
        dispatch->AIUpdate(); check("flight-aim",p->aims==1 && p->aimed==CPoint(400,36)); consume(1,puffs,CPoint(56,36));
        p->m_bSparkleTrail=FALSE; dispatch->AIUpdate(); consume(0,puffs,CPoint(72,36));
        p->m_bSparkleTrail=2;
        game->m_nTimeStop=1; game->m_nTimeStopCaster=-1;
        dispatch->AIUpdate(); check("frozen-no-motion-or-spawn",p->aims==2 && area->m_lVertSortAdd.GetCount()==front);
        game->m_nTimeStopCaster=p->m_id;
        dispatch->AIUpdate(); check("time-stop-owner-flies",p->aims==3); consume(1,puffs,CPoint(88,36));
        game->m_nTimeStop=0;
        p->Position(CPoint(398,36)); dispatch->AIUpdate();
        check("exact-radius-arrival",p->arrivals==1 && p->aims==3 && area->m_lVertSortAdd.GetCount()==front);
        p->Position(CPoint(395,36),-4); dispatch->AIUpdate(); consume(1,puffs,CPoint(411,36));
        check("signed-velocity-radius",p->arrivals==1 && p->aims==4);
        // Squaring 50000 wraps negative before the signed comparison.
        p->Position(CPoint(50400,36)); dispatch->AIUpdate();
        check("wrapped-x-distance-arrives",p->arrivals==2 && p->aims==4 && area->m_lVertSortAdd.GetCount()==front);
        p->Position(CPoint(400,15036)); dispatch->AIUpdate();
        check("wrapped-y-distance-arrives",p->arrivals==3 && p->aims==4 && area->m_lVertSortAdd.GetCount()==front);
        p->Position(CPoint(40,36)); p->Lingering(-32768); dispatch->AIUpdate();
        check("linger-short-wrap",p->Countdown()==32767 && p->Charges()==2 && p->aims==4);
        p->Lingering(1); dispatch->AIUpdate(); check("linger-countdown-zero",p->Countdown()==0);
        // GetCloseObjects requires a real list cursor, even for an empty scan.
        p->ScanFrom(area->m_lVertSort.GetHeadPosition()); dispatch->AIUpdate();
        check("no-target-rearm-difference",p->Countdown()==lingerReload && p->Charges()==2 && p->explosions==0);
        p->m_posVertList=NULL; retire(p);
    };
    exercise(new SparkleExplodingFixture<CProjectileExploding>(CResRef("SPMAGMIS")),0,7);
    exercise(new SparkleExplodingFixture<CProjectileExplodingWeapon>(),3,6);
    exercise(new SparkleExplodingFixture<CProjectileSkullTrap>(CResRef("SPMAGMIS"),CResRef(""),0),0,0);
    {
        auto* p = new SparkleExplodingFixture<CProjectileExplodingFlame>();
        exercise(p,3,6);
        p = new SparkleExplodingFixture<CProjectileExplodingFlame>();
        objects->Add(&p->m_id,p,INFINITE); p->Seed(area); p->m_targetId=actorId;
        CProjectile* dispatch=p; dispatch->AIUpdate();
        check("flame-live-target",p->aimed==actorPoint && p->removals==0); consume(1,3,CPoint(56,36));
        p->m_targetId=0x7FFFFFFF; dispatch->AIUpdate();
        check("flame-missing-target-removes",p->removals==1 && p->aims==1 && area->m_lVertSortAdd.GetCount()==front);
        p->m_targetId=actorId; p->CProjectile::m_pArea=NULL; dispatch->AIUpdate();
        check("flame-other-area-removes",p->removals==2 && p->aims==1 && area->m_lVertSortAdd.GetCount()==front);
        p->CProjectile::m_pArea=area; retire(p);
    }
    struct Spell : IcewindCProjectileSpellHit {
        INT aims=0, arrivals=0, removals=0, gathers=0, strikes=0;
        Spell() : IcewindCProjectileSpellHit(0) {}
        void AimAtPoint(int,int) override { ++aims; m_pos.x+=16; }
        void OnArrival() override { ++arrivals; }
        void RemoveSelf() override { ++removals; }
        std::list<LONG> GatherTargets() override { ++gathers; return {17,23}; }
        void Strike(std::list<LONG>& ids) override { if(ids==std::list<LONG>{17,23}) ++strikes; }
        void Seed(CGameArea* area) {
            m_pArea=area; CGameObject::m_pArea=area; m_pos=CPoint(40,36); m_posZ=25;
            m_targetX=400; m_targetY=36; m_velocity=1; m_sparkleColor=2; m_bSparkleTrail=TRUE;
            m_bHasTravelCell=1; m_bDetonated=0; m_bAnimateCell1=0; m_bAnimateCell2=0;
            m_lifetime=5; m_strikeCountdown=5; m_strikePeriod=7;
        }
        void Position(CPoint pos) { m_pos=pos; }
        void Snap() { m_bHasTravelCell=0; }
        void Detonate(LONG lifetime,LONG countdown) { m_bDetonated=1; m_lifetime=lifetime; m_strikeCountdown=countdown; }
        LONG Lifetime() const { return m_lifetime; }
        LONG Countdown() const { return m_strikeCountdown; }
    };
    {
        Spell* p=new Spell; objects->Add(&p->m_id,p,INFINITE); p->Seed(area); CProjectile* dispatch=p;
        dispatch->AIUpdate(); consume(1,0,CPoint(56,36));
        check("spell-flight-keeps-counters",p->Lifetime()==5 && p->Countdown()==5);
        p->m_bSparkleTrail=FALSE; dispatch->AIUpdate(); consume(0,0,CPoint(72,36));
        p->m_bSparkleTrail=TRUE; game->m_nTimeStop=1; game->m_nTimeStopCaster=-1;
        dispatch->AIUpdate(); check("spell-time-stop",p->aims==2 && p->Lifetime()==5 && area->m_lVertSortAdd.GetCount()==front);
        game->m_nTimeStopCaster=p->m_id; dispatch->AIUpdate(); consume(1,0,CPoint(88,36));
        game->m_nTimeStop=0; p->Position(CPoint(398,36)); dispatch->AIUpdate();
        check("spell-radius-arrival",p->arrivals==1 && p->aims==3 && area->m_lVertSortAdd.GetCount()==front);
        p->Position(CPoint(50400,36)); dispatch->AIUpdate();
        check("spell-wrapped-distance",p->arrivals==2 && p->aims==3);
        p->Snap(); p->Position(CPoint(40,36)); dispatch->AIUpdate();
        check("spell-no-cell-snap",p->m_pos==CPoint(400,36) && p->arrivals==3 && p->aims==3);
        p->Detonate(5,1); dispatch->AIUpdate();
        check("spell-gather-strike-reload",p->gathers==1 && p->strikes==1 && p->Countdown()==7 && p->Lifetime()==4);
        p->Detonate(static_cast<LONG>(0x80000000UL),static_cast<LONG>(0x80000000UL)); dispatch->AIUpdate();
        check("spell-long-wrap",p->Lifetime()==0x7FFFFFFF && p->Countdown()==0x7FFFFFFF && p->gathers==1 && p->removals==0);
        p->Detonate(1,2); dispatch->AIUpdate();
        check("spell-final-tick",p->Lifetime()==0 && p->Countdown()==1 && p->removals==1);
        retire(p);
    }
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster;
    check("same-tick-restoration",game->GetWorldTimer()->m_gameTime==clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages
        && area->m_lVertSortAdd.GetCount()==front && area->m_lVertSortBackAdd.GetCount()==back
        && area->m_lVertSortFlightAdd.GetCount()==flight && area->m_lVertSort.GetCount()==sorted);
    srand(1);
    Emit("{\"step\":%d,\"op\":\"sparklefamilychecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if(!valid) Fail(nStep,"sparklefamilychecks","projectile family mismatch");
}

template <class T>
struct ProjectileLaunchFixture : T {
    BOOL* dead;
    INT areaCalls = 0, registrations = 0;
    BOOL registeredBeforeTarget = FALSE;
    ProjectileLaunchFixture(BOOL* result) : T(CResRef("ARARROW")), dead(result) {
        this->m_fireSoundRef = "";
        this->m_bSparkleTrail = FALSE;
        this->m_targetX = -1111; this->m_targetY = -2222;
    }
    ~ProjectileLaunchFixture() override { *dead = TRUE; }
    void AddToArea(CGameArea*, const CPoint&, LONG, BYTE) override { ++areaCalls; }
    void SetAIType(const CAIObjectType& type, int flags) override {
        ++registrations;
        registeredBeforeTarget = this->m_targetX == -1111 && this->m_targetY == -2222;
        CGameObject::SetAIType(type, flags);
    }
    void Configure(SHORT speed, BYTE armed, SHORT lifetime) {
        this->m_velocity = speed; this->m_distLifetime = armed; this->m_lifetime = lifetime;
    }
    SHORT Lifetime() const { return this->m_lifetime; }
    SHORT Facing() const { return this->m_facing; }
    LONG Metric() const { return this->m_flightDistSq; }
};

void ProjectileLaunchChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame();
    auto* objects = game->GetObjectArray();
    LONG actorId = game->GetFixedOrderCharacterId(0);
    CGameObject* actor = NULL;
    if (objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &actor, INFINITE) != CGameObjectArray::SUCCESS) {
        Fail(nStep, "projectilelaunchchecks", "requires loaded actor"); return;
    }
    CGameArea* area = actor->m_pArea;
    if (!area || actor->GetObjectType() != CGameObject::TYPE_SPRITE) {
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Fail(nStep, "projectilelaunchchecks", "requires party sprite area"); return;
    }
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"projectilelaunch-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
    };
    CPoint actorPos = actor->m_pos;
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    auto stop = game->m_nTimeStop; auto caster = game->m_nTimeStopCaster;
    game->m_nTimeStop = 0;
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    auto unqueue = [&](LONG id) {
        POSITION p = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortAdd.RemoveAt(p);
        p = area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortBackAdd.RemoveAt(p);
        p = area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortFlightAdd.RemoveAt(p);
    };
    auto retire = [&](CGameObject* object) {
        if (object->m_id != -1) {
            unqueue(object->m_id); object->RemoveFromArea();
            objects->Delete(object->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
        }
        delete object;
    };
    // A registered non-sprite supplies exact origins without facing offsets.
    CGameObject* source = new CGameObject;
    source->m_pos = CPoint(0,0);
    check("exact-origin-source-registered", objects->Add(&source->m_id, source, INFINITE) == CGameObjectArray::SUCCESS);
    CRect height; CPoint ref;
    auto* sprite = static_cast<CGameSprite*>(actor);
    sprite->GetAnimation()->CalculateFxRect(height, ref, sprite->m_posZ);
    SHORT expectedZ = static_cast<SHORT>(32 - ((height.bottom - height.top) >> 1));
    struct LaunchCase { LONG x, y; SHORT speed; BYTE armed; LONG metric; SHORT lifetime; };
    const LaunchCase cases[] = {
        {29,5,10,1,885,4}, {9,3,10,1,97,2}, {32767,100,10,1,1073694066,3277},
        {32768,0,10,1,1073741824,-3275}, {32769,0,-10,1,1073807361,3277},
        {32768,0,-32768,1,1073741824,2}, {65536,3,3,1,16,2},
        {1,11586,10,0,-238577883,-123}, {50000,0,10,0,-1794967296,-123},
        {-3,-2,3,1,16,2}, {0,0,1,1,0,1}
    };
    auto exercise = [&](auto* type) {
        using Missile = typename std::remove_pointer<decltype(type)>::type;
        for (const auto& c : cases) {
            BOOL dead = FALSE;
            auto* missile = new Missile(&dead);
            missile->Configure(c.speed, c.armed, -123);
            missile->m_bHasHeight = TRUE;
            actor->m_pos = CPoint(c.x,c.y);
            CProjectile* dispatch = missile;
            dispatch->Fire(area, source->m_id, actorId, CPoint(333,444), -99999, 1234);
            check("live-target-and-signed-distance", !dead && missile->Metric() == c.metric
                && CPoint(missile->m_targetX,missile->m_targetY) == CPoint(c.x,c.y));
            check("rounded-narrowed-signed-lifetime", missile->Lifetime() == c.lifetime);
            check("animation-half-height", missile->m_posZ == 32 && missile->m_nDeltaZ == expectedZ && missile->m_nDeltaZLast == expectedZ);
            check("qualified-base-registration-order", missile->areaCalls == 0 && missile->registrations == 1 && missile->registeredBeforeTarget
                && missile->CGameObject::m_pArea == area && missile->m_pArea == area && missile->m_listType == CGameObject::LIST_FRONT
                && area->m_lVertSortAdd.Find(reinterpret_cast<int*>(missile->m_id)) != NULL);
            check("fixed-origin-and-facing", missile->m_pos == CPoint(0,0) && missile->m_posAccumX == 0 && missile->m_posAccumY == 0
                && missile->Facing() == missile->GetDirection(CPoint(c.x,c.y)));
            retire(missile); check("owned-retirement", dead);
        }
        actor->m_pos = CPoint(128,96);
        for (BOOL hasHeight : {FALSE, TRUE}) {
            CPoint start; CProjectile::GetStart(actorId, start, TRUE);
            for (BOOL nearby : {FALSE, TRUE}) {
                BOOL dead = FALSE; auto* missile = new Missile(&dead);
                missile->Configure(10,0,-123); missile->m_bHasHeight = static_cast<BOOLEAN>(hasHeight);
                CPoint target = nearby ? start : CPoint(500,300);
                CProjectile* dispatch = missile;
                dispatch->Fire(area, actorId, -1, target, -99999, 1234);
                check("sprite-origin-close-snap", missile->m_pos == (nearby ? actor->m_pos : start));
                check("sprite-cast-height-ignores-argument", missile->m_posZ == (hasHeight ? sprite->GetAnimation()->GetCastHeight() : 0)
                    && missile->m_nDeltaZ == 0 && missile->m_nDeltaZLast == 0 && missile->Metric() == 1 && missile->Lifetime() == -123);
                retire(missile);
            }
        }
        for (CPoint origin : {CPoint(-7,-2), CPoint(0x7FFFFFFF,0x7FFFFFFF), CPoint(0,0)}) {
            BOOL dead = FALSE; auto* missile = new Missile(&dead);
            source->m_pos = origin; missile->Configure(10,0,7); missile->m_bHasHeight = FALSE;
            CProjectile* dispatch = missile;
            dispatch->Fire(area, source->m_id, -1, CPoint(500,300), 8888, -1);
            CPoint exact = origin == CPoint(-7,-2) ? CPoint(-7168,-2730)
                : origin == CPoint(0x7FFFFFFF,0x7FFFFFFF) ? CPoint(-1024,-1365) : CPoint(0,0);
            check("wrapped-fixedpoint-negative-origin", missile->m_pos == origin && missile->m_posAccumX == exact.x
                && missile->m_posAccumY == exact.y && missile->m_posZ == 0 && missile->Lifetime() == 7);
            retire(missile);
        }
        source->m_pos = CPoint(0,0);
        for (BOOL missingTarget : {FALSE, TRUE}) {
            BOOL dead = FALSE; auto* missile = new Missile(&dead);
            missile->Configure(10,1,55);
            CProjectile* dispatch = missile;
            dispatch->Fire(area, missingTarget ? source->m_id : -2, missingTarget ? -2 : -1, CPoint(200,300), 0,0);
            check("missing-source-or-target-leaves-unregistered", !dead && missile->m_id == -1 && missile->registrations == 0
                && missile->areaCalls == 0 && missile->Lifetime() == 55 && missile->m_targetX == -1111);
            retire(missile);
        }
        // Force Add's existing NO_MEMORY branch, then restore before any tick.
        BOOL dead = FALSE; auto* missile = new Missile(&dead);
        missile->Configure(10,0,55);
        SHORT nextId = objects->m_nNextObjectId;
        objects->m_nNextObjectId = 0x7FFF;
        CProjectile* dispatch = missile;
        dispatch->Fire(area, source->m_id, -1, CPoint(200,300), 0,0);
        objects->m_nNextObjectId = nextId;
        check("registration-failure-deletes-before-area", dead && area->m_lVertSortAdd.GetCount() == front);
        // A nonempty resource arms the native looping branch even if not found.
        dead = FALSE; missile = new Missile(&dead); missile->Configure(10,0,55);
        missile->m_fireSoundRef = "ZRCNONE"; missile->m_loopFireSound = TRUE;
        dispatch = missile; dispatch->Fire(area, source->m_id, -1, CPoint(200,300), 0,0);
        check("launch-sound-loop-and-channel", missile->m_sound.GetLooping() && missile->m_sound.m_nChannel == 15
            && missile->m_sound.m_nArea == reinterpret_cast<DWORD>(area));
        retire(missile);
    };
    exercise(static_cast<ProjectileLaunchFixture<CProjectileTravelling>*>(NULL));
    exercise(static_cast<ProjectileLaunchFixture<IcewindCProjectileTravellingVFX>*>(NULL));
    // Original factory-2F evidence: arrival guards precede the zero timer.
    struct Sparkle : CProjectileSparkle {
        INT arrivals = 0, removals = 0;
        Sparkle() : CProjectileSparkle(0) { m_fireSoundRef = ""; }
        void OnArrival() override { ++arrivals; }
        void RemoveSelf() override { ++removals; }
        SHORT Lifetime() const { return m_lifetime; }
        BYTE Armed() const { return m_distLifetime; }
    };
    for (BOOL nearby : {FALSE, TRUE}) {
        auto* missile = new Sparkle;
        check("sparkle-native-zero-default", missile->Lifetime() == 0 && missile->Armed() == 0);
        CProjectile* dispatch = missile;
        dispatch->Fire(area, source->m_id, -1, nearby ? CPoint(0,0) : CPoint(300,0), 0,0);
        check("sparkle-fire-retains-zero", missile->Lifetime() == 0 && missile->Armed() == 0);
        dispatch->AIUpdate();
        check("sparkle-arrival-before-expiry", nearby ? missile->arrivals == 1 && missile->removals == 0 && missile->Lifetime() == 0
            : missile->arrivals == 0 && missile->removals == 1 && missile->Lifetime() == -1);
        retire(missile);
    }
    // Exercise full-width direction scaling separately from area insertion.
    CProjectile direction;
    direction.m_pos = CPoint(0,0);
    check("direction-positive-wrap", direction.GetDirection(CPoint(0,0x7FFFFFFF)) == direction.GetDirection(CPoint(0,-1)));
    direction.m_pos = CPoint(0,0x7FFFFFFF);
    check("direction-source-wrap", direction.GetDirection(CPoint(0,0)) == 0);
    retire(source);
    actor->m_pos = actorPos; game->m_nTimeStop = stop; game->m_nTimeStopCaster = caster;
    objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    check("same-tick-restoration", actor->m_pos == actorPos && game->GetWorldTimer()->m_gameTime == clock
        && game->m_nTimeStop == stop && game->m_nTimeStopCaster == caster
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == messages
        && area->m_lVertSortAdd.GetCount() == front && area->m_lVertSortBackAdd.GetCount() == back
        && area->m_lVertSortFlightAdd.GetCount() == flight
        && objects->m_pArray[(actorId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == 0);
    Emit("{\"step\":%d,\"op\":\"projectilelaunchchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"projectilelaunchchecks","travelling launch mismatch");
}

// Use a symmetric anchor for synthetic signed-height/mirror combinations.
// Keep the real BAM pinned, restore its complete frame record and demand
// count before releasing either resource-service lock or ticking the game.
struct ProjectileRenderFrameAnchor {
    CResCell* resource;
    CResCellHeader* header;
    FRAMEENTRY* frame = NULL;
    FRAMEENTRY* headerFrame = NULL;
    FRAMEENTRY saved = {};
    FRAMEENTRY savedHeader = {};
    INT demands;
    INT headerDemands = 0;
    BOOL active = TRUE;
    BOOL pinned;
    explicit ProjectileRenderFrameAnchor(CVidCell* cell) : resource(cell->pRes), header(cell->m_header.pRes) {
        EnterCriticalSection(&g_pChitin->m_critSectDimm);
        EnterCriticalSection(&g_pChitin->m_critSectService);
        demands = resource->GetDemands();
        pinned = resource->Demand() != NULL;
        if (pinned) frame = resource->GetFrame(0, 0, FALSE);
        if (header != NULL) {
            headerDemands = header->GetDemands();
            if (header->Demand() != NULL) headerFrame = header->GetFrame(0, 0, FALSE);
        }
        if (frame != NULL) {
            saved = *frame;
            frame->nCenterY = static_cast<SHORT>(frame->nHeight / 2);
        }
        if (headerFrame != NULL) {
            savedHeader = *headerFrame;
            headerFrame->nCenterY = static_cast<SHORT>(headerFrame->nHeight / 2);
        }
    }
    BOOL Restore() {
        if (!active) return TRUE;
        if (frame != NULL) *frame = saved;
        BOOL ok = frame != NULL && memcmp(frame, &saved, sizeof(saved)) == 0;
        if (header != NULL) {
            if (headerFrame != NULL) *headerFrame = savedHeader;
            ok = ok && headerFrame != NULL && memcmp(headerFrame, &savedHeader, sizeof(savedHeader)) == 0;
            header->Release();
            ok = ok && header->GetDemands() == headerDemands;
        }
        resource->Release();
        ok = ok && resource->GetDemands() == demands;
        LeaveCriticalSection(&g_pChitin->m_critSectService);
        LeaveCriticalSection(&g_pChitin->m_critSectDimm);
        active = FALSE;
        return ok;
    }
    ~ProjectileRenderFrameAnchor() { Restore(); }
};

#include "BeamTickGolden.h"
#include "AreaNearestGolden.h"
#include "SparkleExplosionGolden.h"
#include "AcidBlobGolden.h"
#include "ThrownWeaponGolden.h"
#include "SprayGolden.h"

BOOL SprayPixels(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* area=game->GetVisibleArea();
    auto* handler=g_pBaldurChitin->GetMessageHandler();
    auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) return FALSE;
    INT checks=0,draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"spray-pixel-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    struct Access : CProjectileSPBRNHND { using CProjectileTravelling::m_palette; };
    auto* master=game->GetMasterBitmap(); INT demands=master->pRes->GetDemands();
    auto& infinity=area->m_cInfinity; CRect view=infinity.rViewPort;
    INT viewX=infinity.nCurrentX,viewY=infinity.nCurrentY; auto* wed=infinity.pResWED;
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount(),pending=area->m_lVertSortAdd.GetCount();
    std::vector<BYTE> visibility(area->m_visibility.m_pMap,area->m_visibility.m_pMap+area->m_visibility.m_nMapSize);
    CPoint origin(232,1002); INT cost=origin.y/12*area->m_search.m_GridSquareDimensions.cx+origin.x/16;
    BYTE dynamic=area->m_search.m_pDynamicCost[cost]; CRect patch(64,64,416,336); std::vector<BYTE> saved;
    auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear)->BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8;
        INT stride=patch.Width()*bytes; BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
        if (ok && read) read->resize(stride*patch.Height());
        if (ok && write) ok=write->size()==static_cast<size_t>(stride*patch.Height());
        if (ok) for (INT y=0;y<patch.Height();++y) {
            BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data()+y*stride,p,stride); if (write) memcpy(p,write->data()+y*stride,stride);
            if (clear) memset(p,0,stride);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp=pixels(&saved,NULL,FALSE); check("pixels-backed-up",backedUp);
    if (backedUp) {
        infinity.rViewPort=CRect(80,80,400,320); infinity.nCurrentX=72; infinity.nCurrentY=882; infinity.pResWED=NULL;
        memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size()); area->m_search.m_pDynamicCost[cost]=0;
        for (const auto& row : SPRAY_FACTORY) {
            auto* parent=dynamic_cast<CProjectileSPBRNHND*>(CProjectile::DecodeProjectile(0x1A,NULL,row.delay));
            check("pixel-factory-parent",parent!=NULL); if (!parent) continue;
            auto& parentPalette=parent->*(&Access::m_palette);
            auto* child=new CProjectileSprayCell(CResRef("SPBRNHND"),row.life,0,parentPalette);
            for (INT kind=0;kind<2;++kind) {
                CProjectileTravelling* object=kind?static_cast<CProjectileTravelling*>(child):parent;
                object->m_pArea=area; object->CGameObject::m_pArea=area;
                object->m_pos=origin; object->m_posZ=17; object->m_listType=CGameObject::LIST_FRONT;
                object->AimAtPoint(origin.x,origin.y+12); object->m_pos=origin;
                CVidPalette palette(CVidPalette::TYPE_RANGE); palette.SetRange(0,row.color,*master);
                CVidCell cell(CResRef("SPBRNHND"),FALSE); cell.SequenceSet(0); cell.FrameSet(0); cell.SetPalette(palette);
                cell.m_cPalette.m_rangeColors[0]=kind?row.color:row.cached;
                ProjectileRenderFrameAnchor anchor(&cell); check("spray-bam-frame",anchor.frame && (!anchor.header || anchor.headerFrame));
                CPoint center; CSize size; cell.GetCurrentCenterPoint(center,FALSE); cell.GetCurrentFrameSize(size,FALSE);
                CRect bounds(0,0,size.cx,size.cy); CPoint point(232,985);
                DWORD flags=(kind?0x20000:row.flags)|CInfinity::FXPREP_COPYFROMBACK;
                for (INT mode=0;mode<3;++mode) {
                    SHORT tile=area->m_visibility.PointToTile(origin);
                    if (mode==1) area->m_visibility.m_pMap[tile]=0; if (mode==2) infinity.nCurrentX+=10000;
                    std::vector<BYTE> expected,actual; BOOL ok=pixels(NULL,NULL,TRUE);
                    if (mode==0) {
                        infinity.FXPrep(bounds,flags,CVIDINF_SURFACE_BACK,point,center);
                        if (infinity.FXLock(bounds,flags)) {
                            infinity.FXRender(&cell,center.x,center.y,flags,0x80); infinity.FXUnlock(flags,NULL,CPoint(0,0));
                            ok=infinity.FXBltFrom(CVIDINF_SURFACE_BACK,bounds,point.x,point.y,center.x,center.y,flags) && ok;
                        } else ok=FALSE;
                    }
                    ok=pixels(&expected,NULL,TRUE) && ok;
                    if (mode==0) { BOOL nonempty=FALSE; for (BYTE pixel : expected) nonempty=nonempty || pixel!=0;
                        check("normal-spray-has-visible-pixels",nonempty); }
                    object->Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws;
                    ok=pixels(&actual,NULL,FALSE) && ok;
                    check("spray-parent-child-complete-pixels",ok && expected==actual);
                    check("render-surface-unlocked",video->m_SurfaceDesc.lpSurface==NULL);
                    area->m_visibility.m_pMap[tile]=CVisibilityMap::VISIBILITY_MASK;
                    if (mode==2) infinity.nCurrentX-=10000;
                }
                check("spray-frame-restored",anchor.Restore()); object->CGameObject::m_pArea=NULL;
            }
            delete child; delete parent;
        }
    }
    memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size()); area->m_search.m_pDynamicCost[cost]=dynamic;
    infinity.rViewPort=view; infinity.nCurrentX=viewX; infinity.nCurrentY=viewY; infinity.pResWED=wed;
    check("pixels-restored",backedUp && pixels(NULL,&saved,FALSE));
    check("pixel-fixture-state-restored",game->GetWorldTimer()->m_gameTime==clock && master->pRes->GetDemands()==demands
        && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==pending);
    Emit("{\"step\":%d,\"op\":\"spraypixels\",\"checks\":%d,\"renderCalls\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,draws,valid?"true":"false");
    return valid;
}

void SprayChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    auto* handler=g_pBaldurChitin->GetMessageHandler();
    INT checks=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"spray-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    struct Access : CProjectileSPBRNHND {
        using CProjectileSprayLifetime::m_animationLifetime;
        using CProjectileTravelling::m_pVidCell; using CProjectileTravelling::m_palette;
        using CProjectileTravelling::m_facing;
    };
    ULONG clock=game->GetWorldTimer()->m_gameTime; auto stop=game->m_nTimeStop,caster=game->m_nTimeStopCaster;
    INT messages=handler->m_messageList.GetCount(); auto areaCounter=CGameArea::dword_8D212C; auto areaFlag=CGameArea::byte_8D2138;
    auto* area=new CGameArea(255); area->m_pGame=game;
    auto* master=game->GetMasterBitmap(); INT masterDemands=master->pRes->GetDemands();
    struct Origin : CGameObject { BYTE GetObjectType() override { return TYPE_NONE; } };
    auto* origin=new Origin; origin->m_pArea=area; origin->m_pos=CPoint(128,96);
    check("source-registered",objects->Add(&origin->m_id,origin,INFINITE)==CGameObjectArray::SUCCESS);
    auto retire=[&](CGameObject* p) {
        POSITION node=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id)); if (node) area->m_lVertSortAdd.RemoveAt(node);
        p->RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p;
    };
    auto pending=[&]()->std::vector<CProjectile*> {
        std::vector<CProjectile*> result; POSITION pos=area->m_lVertSortAdd.GetHeadPosition();
        while (pos) { LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(pos)); CGameObject* p=NULL;
            if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&p,INFINITE)==CGameObjectArray::SUCCESS) {
                auto* projectile=dynamic_cast<CProjectile*>(p); check("pending-object-is-projectile",projectile!=NULL);
                if (projectile) result.push_back(projectile); objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
            } else check("pending-object-registered",FALSE);
        }
        return result;
    };
    for (const auto& row : SPRAY_FACTORY) {
        CProjectile* object=CProjectile::DecodeProjectile(0x1A,NULL,row.delay);
        auto* spray=dynamic_cast<CProjectileSPBRNHND*>(object);
        check("native-factory-and-ctor",spray && object->m_projectileType==0x19 && object->m_bHasHeight
            && object->m_fireSoundRef==CResRef("EFF_M20") && object->m_arrivalSoundRef==CResRef("")
            && object->field_174==23 && object->field_178==50);
        if (spray) {
            auto* cell=spray->*(&Access::m_pVidCell); auto& palette=spray->*(&Access::m_palette);
            check("native-palette-order-and-lifetime",cell->GetResRef()==CResRef("SPBRNHND") && cell->GetCurrentSequenceId()==0
                && palette.m_rangeColors[0]==row.color && cell->m_cPalette.m_rangeColors[0]==row.cached
                && palette.m_pPalette==cell->m_cPalette.m_pPalette && spray->GetRenderFlags()==row.flags
                && spray->*(&Access::m_animationLifetime)==row.life);
        }
        delete object;
    }
    game->m_nTimeStop=0;
    for (const auto& row : SPRAY_SPAWN) {
        Access parent; parent.m_pArea=area; parent.m_posZ=31; parent.m_bHasHeight=TRUE;
        parent.SpawnSpray(CPoint(128,96),row.count,row.facing,row.oneSide);
        auto cells=pending(); check("native-recursive-child-count",cells.size()==static_cast<size_t>(row.size));
        for (INT i=0;i<static_cast<INT>(cells.size());++i) {
            auto* cell=dynamic_cast<CProjectileSprayCell*>(cells[i]);
            check("distinct-child-class-and-placement",cell && i<row.size && cell->m_pos==CPoint(row.points[i][0],row.points[i][1])
                && cell->m_posZ==31 && cell->m_listType==CGameObject::LIST_FRONT && cell->m_bHasHeight
                && cell->m_pArea==area && cell->CGameObject::m_pArea==area && cell->GetRenderFlags()==0x20000);
        }
        for (auto* cell : cells) retire(cell);
        check("spawn-fixtures-retired",area->m_lVertSortAdd.IsEmpty() && handler->m_messageList.GetCount()==messages);
    }
    for (const auto& row : SPRAY_FIRE) {
        origin->m_pos=CPoint(row.origin[0],row.origin[1]);
        auto* parent=new Access; parent->Fire(area,origin->m_id,-1,CPoint(row.target[0],row.target[1]),row.height,13);
        check("native-fire-state",parent->m_pos==CPoint(row.launch[0],row.launch[1]) && parent->m_posZ==row.height
            && parent->m_posAccumX==row.accum[0] && parent->m_posAccumY==row.accum[1] && parent->m_facing==row.facing
            && parent->m_targetX==row.target[0] && parent->m_targetY==row.target[1] && parent->m_sourceId==origin->m_id);
        auto cells=pending(); check("native-fire-registers-parent-and-cells",cells.size()==static_cast<size_t>(row.size+1)
            && !cells.empty() && cells[0]==parent);
        for (INT i=1;i<static_cast<INT>(cells.size());++i) check("native-fire-recursive-placement",i<=row.size
            && dynamic_cast<CProjectileSprayCell*>(cells[i])!=NULL && cells[i]->m_pos==CPoint(row.points[i-1][0],row.points[i-1][1])
            && cells[i]->m_posZ==row.height && cells[i]->m_listType==CGameObject::LIST_FRONT);
        for (INT i=static_cast<INT>(cells.size())-1;i>=0;--i) retire(cells[i]);
        check("empty-fire-no-damage-messages",handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.IsEmpty());
    }
    for (const auto& row : SPRAY_TICK) {
        BOOL removed=FALSE,destroyed=FALSE,order=TRUE; INT frames=0;
        struct Sample : Access {
            CVidCell* original=NULL; BOOL* removed; BOOL* destroyed; BOOL fail;
            ~Sample() override { *destroyed=TRUE; }
            void RemoveFromArea() override { *removed=TRUE; m_pVidCell=original; if (fail) m_id=-1; }
        };
        struct Frame : CVidCell {
            SHORT* remaining; SHORT expected; INT* calls; BOOL* order;
            BOOL FrameAdvance() override { ++*calls; *order=*order && *remaining==expected; return TRUE; }
        } frame;
        auto* sample=new Sample; sample->removed=&removed; sample->destroyed=&destroyed; sample->fail=row.deleteFail;
        sample->original=sample->m_pVidCell; sample->m_animationLifetime=row.before;
        frame.remaining=&sample->m_animationLifetime; frame.expected=row.after; frame.calls=&frames; frame.order=&order;
        sample->m_pVidCell=&frame; LONG id=-1; objects->Add(&sample->m_id,sample,INFINITE); id=sample->m_id;
        game->m_nTimeStop=row.stop; game->m_nTimeStopCaster=row.stop==1?id+1:id;
        sample->AIUpdate();
        check("native-countdown-before-frame-and-retirement",order && frames==row.frames && removed==row.expired && destroyed==row.destroyed);
        if (!destroyed) {
            check("native-signed-word-countdown",sample->m_animationLifetime==row.after);
            sample->m_pVidCell=sample->original; objects->Delete(id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete sample;
        }
    }
    {
        Access absent; absent.Fire(area,-1,-1,CPoint(256,96),31,0);
        check("missing-source-keeps-unregistered-object",absent.m_id==-1 && area->m_lVertSortAdd.IsEmpty());
    }
    retire(origin); delete area; CGameArea::dword_8D212C=areaCounter; CGameArea::byte_8D2138=areaFlag;
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster;
    check("same-tick-state-restored",game->GetWorldTimer()->m_gameTime==clock && master->pRes->GetDemands()==masterDemands
        && handler->m_messageList.GetCount()==messages);
    check("spray-pixel-comparisons",SprayPixels(nStep));
    Emit("{\"step\":%d,\"op\":\"spraychecks\",\"checks\":%d,\"nativeCases\":261,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"spraychecks","spray constructor/placement/lifetime mismatch");
}

void ThrownWeaponChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* area=game->GetVisibleArea();
    auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) {
        Fail(nStep,"thrownweaponchecks","requires loaded software area"); return;
    }
    INT checks=0,draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) {
        ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"thrown-weapon-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    for (const auto& row : PROJECTILE_IMMUNITY) {
        CImmunitiesProjectile list;
        for (INT i=0;i<row.count;++i) list.AddTail(reinterpret_cast<DWORD*>(row.values[i]));
        check("native-full-dword-immunity-lookup",list.OnList(row.query)==row.found && list.GetCount()==row.count);
        POSITION pos=list.GetHeadPosition();
        for (INT i=0;i<row.count;++i) check("immunity-lookup-keeps-list-order",reinterpret_cast<DWORD>(list.GetNext(pos))==row.values[i]);
    }
    struct Access : CProjectileTravelling {
        using CProjectileTravelling::m_pVidCell; using CProjectileTravelling::m_palette;
        using CProjectileTravelling::m_velocity; using CProjectileTravelling::m_dirCount;
        using CProjectileTravelling::m_tinted; using CProjectileTravelling::m_useHeightOffset;
        using CProjectileTravelling::m_mirror; using CProjectileTravelling::m_hasShadowCell;
    };
    auto* master=game->GetMasterBitmap(); INT demands=master->pRes->GetDemands();
    auto& infinity=area->m_cInfinity; CRect view=infinity.rViewPort;
    INT viewX=infinity.nCurrentX,viewY=infinity.nCurrentY; auto* wed=infinity.pResWED;
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount(),pending=area->m_lVertSortAdd.GetCount();
    std::vector<BYTE> visibility(area->m_visibility.m_pMap,area->m_visibility.m_pMap+area->m_visibility.m_nMapSize);
    CPoint origin(232,1002); INT cost=origin.y/12*area->m_search.m_GridSquareDimensions.cx+origin.x/16;
    BYTE dynamic=area->m_search.m_pDynamicCost[cost];
    CRect patch(64,64,416,336); std::vector<BYTE> savedPixels;
    auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear)->BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8;
        INT stride=patch.Width()*bytes; BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
        if (ok && read) read->resize(stride*patch.Height());
        if (ok && write) ok=write->size()==static_cast<size_t>(stride*patch.Height());
        if (ok) for (INT y=0;y<patch.Height();++y) {
            BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data()+y*stride,p,stride);
            if (write) memcpy(p,write->data()+y*stride,stride);
            if (clear) memset(p,0,stride);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp=pixels(&savedPixels,NULL,FALSE); check("pixels-backed-up",backedUp);
    if (backedUp) {
        infinity.rViewPort=CRect(80,80,400,320); infinity.nCurrentX=72; infinity.nCurrentY=882; infinity.pResWED=NULL;
        memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size()); area->m_search.m_pDynamicCost[cost]=0;
        for (const auto& row : THROWN_WEAPON_FACTORY) {
            CProjectile* object=CProjectile::DecodeProjectile(row.type,NULL,row.delay);
            auto* travel=dynamic_cast<CProjectileTravelling*>(object);
            BOOL kind=row.type==0x177?dynamic_cast<CProjectileBoulder*>(object)!=NULL:
                row.type<=0xB?dynamic_cast<CProjectileAxe*>(object)!=NULL:
                row.type<=0x1F?dynamic_cast<CProjectileDagger*>(object)!=NULL:dynamic_cast<CProjectileSpear*>(object)!=NULL;
            check("native-factory-class-and-stamp",kind && object->m_projectileType==row.type-1 && object->m_bHasHeight
                && object->m_fireSoundRef==CResRef("") && object->m_arrivalSoundRef==CResRef(""));
            if (!travel) { delete object; continue; }
            // Public using-declarations permit typed pointers to inherited
            // members; no cast to a test subclass or binary offsets.
            auto* animation=travel->*(&Access::m_pVidCell); auto& palette=travel->*(&Access::m_palette);
            check("native-leaf-defaults",animation->GetResRef()==CResRef(row.cell) && animation->GetCurrentSequenceId()==0
                && travel->*(&Access::m_velocity)==row.speed && travel->*(&Access::m_dirCount)==16
                && travel->*(&Access::m_tinted)==TRUE && travel->*(&Access::m_useHeightOffset)==FALSE
                && travel->*(&Access::m_mirror)==FALSE && travel->*(&Access::m_hasShadowCell)==FALSE
                && object->m_callBackProjectile==-1 && object->m_nTargetId==-1 && object->field_17E.IsEmpty());
            check("native-dagger-palette-binding",animation->m_bPaletteChanged==row.colored
                && (!row.colored || (palette.m_rangeColors[5]==27 && palette.m_rangeColors[4]==30
                    && animation->m_cPalette.m_rangeColors[5]==27 && animation->m_cPalette.m_rangeColors[4]==30
                    && palette.m_pPalette==animation->m_cPalette.m_pPalette)));
            object->m_pArea=area; object->CGameObject::m_pArea=area;
            object->m_pos=origin; object->m_posZ=17; object->m_listType=CGameObject::LIST_FLIGHT;
            travel->AimAtPoint(origin.x,origin.y+12); object->m_pos=origin;
            CVidPalette reference(CVidPalette::TYPE_RANGE);
            CVidCell cell(CResRef(row.cell),FALSE); cell.SequenceSet(0); cell.FrameSet(0);
            if (row.colored) { reference.SetRange(5,27,*master); reference.SetRange(4,30,*master); cell.SetPalette(reference); }
            ProjectileRenderFrameAnchor anchor(&cell);
            check("weapon-bam-frame-available",anchor.frame && (!anchor.header || anchor.headerFrame));
            CPoint center; CSize size; cell.GetCurrentCenterPoint(center,FALSE); cell.GetCurrentFrameSize(size,FALSE);
            CRect bounds(0,0,size.cx,size.cy); CPoint point(232,985); DWORD flags=0x30000|CInfinity::FXPREP_COPYFROMBACK;
            cell.SetTintColor(area->GetTintColor(point,CGameObject::LIST_FLIGHT));
            for (INT mode=0;mode<3;++mode) {
                SHORT tile=area->m_visibility.PointToTile(origin);
                if (mode==1) area->m_visibility.m_pMap[tile]=0;
                if (mode==2) infinity.nCurrentX+=10000;
                std::vector<BYTE> expected,actual; BOOL ok=pixels(NULL,NULL,TRUE);
                if (mode==0) {
                    infinity.FXPrep(bounds,flags,CVIDINF_SURFACE_BACK,point,center);
                    if (infinity.FXLock(bounds,flags)) {
                        infinity.FXRender(&cell,center.x,center.y,flags,0x80);
                        infinity.FXUnlock(flags,NULL,CPoint(0,0));
                        ok=infinity.FXBltFrom(CVIDINF_SURFACE_BACK,bounds,point.x,point.y,center.x,center.y,flags) && ok;
                    } else ok=FALSE;
                }
                ok=pixels(&expected,NULL,TRUE) && ok;
                if (mode==0) { BOOL visiblePixels=FALSE; for (BYTE pixel : expected) visiblePixels=visiblePixels || pixel!=0;
                    check("normal-reference-draws-visible-pixels",visiblePixels); }
                object->Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws;
                ok=pixels(&actual,NULL,FALSE) && ok;
                check("native-weapon-full-software-pixels",ok && expected==actual);
                check("renderer-surface-unlocked",video->m_SurfaceDesc.lpSurface==NULL);
                area->m_visibility.m_pMap[tile]=CVisibilityMap::VISIBILITY_MASK;
                if (mode==2) infinity.nCurrentX-=10000;
            }
            check("frame-and-demand-restoration",anchor.Restore());
            object->CGameObject::m_pArea=NULL; delete object;
        }
    }
    memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size()); area->m_search.m_pDynamicCost[cost]=dynamic;
    infinity.rViewPort=view; infinity.nCurrentX=viewX; infinity.nCurrentY=viewY; infinity.pResWED=wed;
    check("pixels-restored",backedUp && pixels(NULL,&savedPixels,FALSE));
    check("same-tick-state-restored",master->pRes->GetDemands()==demands && game->GetWorldTimer()->m_gameTime==clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==pending);
    Emit("{\"step\":%d,\"op\":\"thrownweaponchecks\",\"checks\":%d,\"renderCalls\":%d,\"nativeCases\":36,\"restored\":true,\"valid\":%s}",nStep,checks,draws,valid?"true":"false");
    if (!valid) Fail(nStep,"thrownweaponchecks","weapon leaf/immunity/render mismatch");
}

void AcidBlobChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* area=game->GetVisibleArea();
    auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) {
        Fail(nStep,"acidblobchecks","requires loaded software area"); return;
    }
    INT checks=0,draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) {
        ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"acid-blob-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    auto* master=game->GetMasterBitmap(); INT masterDemands=master->pRes->GetDemands();
    struct Blob : CProjectileAcidBlob {
        using CProjectileTravelling::m_palette;
        using CProjectileTravelling::m_pVidCell;
        BOOL Defaults() { return m_pVidCell->GetResRef()==CResRef("ACIDBLOB") && m_pVidCell->GetCurrentSequenceId()==0
            && m_palette.m_rangeColors[0]==CVIDPALETTE_ACID && m_pVidCell->m_cPalette.m_rangeColors[0]==CVIDPALETTE_ACID
            && m_palette.m_pPalette==m_pVidCell->m_cPalette.m_pPalette && m_tinted==TRUE && m_useHeightOffset==FALSE
            && m_mirror==FALSE && m_hasShadowCell==FALSE && m_dirCount==16 && m_velocity==20
            && m_fireSoundRef==CResRef("") && m_callBackProjectile==-1 && m_nTargetId==-1 && field_17E.IsEmpty(); }
    };
    Blob original; check("native-constructor-defaults",original.Defaults());
    for (const auto& row : ACID_BLOB_RANGE) {
        Blob sample; USHORT range=static_cast<USHORT>(row.range);
        sample.SetRange(range,static_cast<USHORT>(row.value));
        check("native-palette-bind-before-range",sample.m_palette.m_rangeColors[range]==row.color
            && sample.m_pVidCell->m_cPalette.m_rangeColors[range]==row.cached
            && sample.m_palette.m_pPalette==sample.m_pVidCell->m_cPalette.m_pPalette
            && sample.m_pVidCell->m_bPaletteChanged==TRUE);
        master->pRes->Demand();
        for (INT x=0;x<12;++x) {
            RGBQUAD expected={}; BOOL loaded=master->GetPixelColor(expected,x,row.row,TRUE);
            check("range-rgb-from-full-word-row",loaded && memcmp(&expected,&sample.m_palette.m_pPalette[4+12*range+x],sizeof(expected))==0);
        }
        master->pRes->Release();
    }
    auto& infinity=area->m_cInfinity; CRect savedView=infinity.rViewPort;
    INT savedX=infinity.nCurrentX,savedY=infinity.nCurrentY; auto* wed=infinity.pResWED;
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT pending=area->m_lVertSortAdd.GetCount();
    std::vector<BYTE> visibility(area->m_visibility.m_pMap,area->m_visibility.m_pMap+area->m_visibility.m_nMapSize);
    CPoint origin(232,1002); INT cost=origin.y/12*area->m_search.m_GridSquareDimensions.cx+origin.x/16;
    BYTE dynamic=area->m_search.m_pDynamicCost[cost];
    CRect patch(64,64,416,336); std::vector<BYTE> savedPixels;
    auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear)->BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8;
        INT stride=patch.Width()*bytes; BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
        if (ok && read) read->resize(stride*patch.Height());
        if (ok && write) ok=write->size()==static_cast<size_t>(stride*patch.Height());
        if (ok) for (INT y=0;y<patch.Height();++y) {
            BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data()+y*stride,p,stride);
            if (write) memcpy(p,write->data()+y*stride,stride);
            if (clear) memset(p,0,stride);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp=pixels(&savedPixels,NULL,FALSE); check("pixels-backed-up",backedUp);
    if (backedUp) {
        infinity.rViewPort=CRect(80,80,400,320); infinity.nCurrentX=72; infinity.nCurrentY=882; infinity.pResWED=NULL;
        memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size()); area->m_search.m_pDynamicCost[cost]=0;
        for (const auto& row : ACID_BLOB_FACTORY) {
            CProjectile* object=CProjectile::DecodeProjectile(row.type,NULL,row.delay);
            check("native-factory-type-and-sounds",dynamic_cast<CProjectileAcidBlob*>(object)!=NULL
                && object->m_projectileType==row.type-1 && object->m_bHasHeight==TRUE
                && object->m_fireSoundRef==CResRef("") && object->m_arrivalSoundRef==CResRef(""));
            object->m_pArea=area; object->CGameObject::m_pArea=area;
            object->m_pos=origin; object->m_posZ=17; object->m_listType=CGameObject::LIST_FLIGHT;
            // The native base ctor leaves facing unwritten; Fire normally
            // initializes it. Aim south for sequence zero, then restore the
            // fixture position advanced by AimAtPoint.
            static_cast<CProjectileTravelling*>(object)->AimAtPoint(origin.x,origin.y+12);
            object->m_pos=origin;
            CVidPalette palette(CVidPalette::TYPE_RANGE); palette.SetRange(0,row.color,*master);
            CVidCell cell(CResRef("ACIDBLOB"),FALSE); cell.SequenceSet(0); cell.FrameSet(0); cell.SetPalette(palette);
            // Captured native metadata is the old row, while the RGB pointer
            // already references the new row. No source SetRange call here.
            cell.m_cPalette.m_rangeColors[0]=row.cached;
            // Construct and demand palette resources before holding the
            // resource-service locks used by the frame anchor.
            ProjectileRenderFrameAnchor anchor(&cell);
            check("acid-bam-frame-available",anchor.frame && (!anchor.header || anchor.headerFrame));
            CPoint center; CSize size; cell.GetCurrentCenterPoint(center,FALSE); cell.GetCurrentFrameSize(size,FALSE);
            CRect bounds(0,0,size.cx,size.cy); CPoint point(232,985); DWORD flags=0x30000|CInfinity::FXPREP_COPYFROMBACK;
            cell.SetTintColor(area->GetTintColor(point,CGameObject::LIST_FLIGHT));
            for (INT mode=0;mode<3;++mode) {
                SHORT tile=area->m_visibility.PointToTile(origin);
                if (mode==1) area->m_visibility.m_pMap[tile]=0;
                if (mode==2) infinity.nCurrentX+=10000;
                std::vector<BYTE> expected,actual;
                BOOL ok=pixels(NULL,NULL,TRUE);
                if (mode==0) {
                    infinity.FXPrep(bounds,flags,CVIDINF_SURFACE_BACK,point,center);
                    if (infinity.FXLock(bounds,flags)) {
                        infinity.FXRender(&cell,center.x,center.y,flags,0x80);
                        infinity.FXUnlock(flags,NULL,CPoint(0,0));
                        ok=infinity.FXBltFrom(CVIDINF_SURFACE_BACK,bounds,point.x,point.y,center.x,center.y,flags) && ok;
                    } else ok=FALSE;
                }
                ok=pixels(&expected,NULL,TRUE) && ok;
                if (mode==0) {
                    BOOL visiblePixels=FALSE; for (BYTE pixel : expected) visiblePixels=visiblePixels || pixel!=0;
                    check("normal-reference-draws-visible-pixels",visiblePixels);
                }
                object->Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws;
                ok=pixels(&actual,NULL,FALSE) && ok;
                check("native-color-variant-full-pixels",ok && expected==actual);
                check("renderer-surface-unlocked",video->m_SurfaceDesc.lpSurface==NULL);
                area->m_visibility.m_pMap[tile]=CVisibilityMap::VISIBILITY_MASK;
                if (mode==2) infinity.nCurrentX-=10000;
            }
            check("frame-and-demand-restoration",anchor.Restore());
            object->CGameObject::m_pArea=NULL; delete object;
        }
    }
    memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size()); area->m_search.m_pDynamicCost[cost]=dynamic;
    infinity.rViewPort=savedView; infinity.nCurrentX=savedX; infinity.nCurrentY=savedY; infinity.pResWED=wed;
    check("pixels-restored",backedUp && pixels(NULL,&savedPixels,FALSE));
    check("same-tick-state-restored",master->pRes->GetDemands()==masterDemands && game->GetWorldTimer()->m_gameTime==clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==pending);
    Emit("{\"step\":%d,\"op\":\"acidblobchecks\",\"checks\":%d,\"renderCalls\":%d,\"nativeCases\":13,\"restored\":true,\"valid\":%s}",nStep,checks,draws,valid?"true":"false");
    if (!valid) Fail(nStep,"acidblobchecks","acid leaf/palette/render mismatch");
}

void SparkleExplosionChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* area=game->GetVisibleArea();
    auto* objects=game->GetObjectArray(); auto* handler=g_pBaldurChitin->GetMessageHandler();
    auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) {
        Fail(nStep,"sparkleexplosionchecks","requires loaded software area"); return;
    }
    INT checks=0, draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) {
        ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"sparkle-explosion-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    INT messages=handler->m_messageList.GetCount(), front=area->m_lVertSortAdd.GetCount();
    INT sorted=area->m_lVertSort.GetCount(), back=area->m_lVertSortBackAdd.GetCount(), flight=area->m_lVertSortFlightAdd.GetCount();
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    auto stop=game->m_nTimeStop, caster=game->m_nTimeStopCaster;
    auto& infinity=area->m_cInfinity;
    CRect view=infinity.rViewPort; INT viewX=infinity.nCurrentX, viewY=infinity.nCurrentY;
    auto* wed=infinity.pResWED;
    std::vector<BYTE> visibility(area->m_visibility.m_pMap,area->m_visibility.m_pMap+area->m_visibility.m_nMapSize);
    CRect patch(64,64,416,336); std::vector<BYTE> savedPixels;
    auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear)->BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8;
        INT row=patch.Width()*bytes; BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
        if (ok && read) read->resize(row*patch.Height());
        if (ok && write) ok=write->size()==static_cast<size_t>(row*patch.Height());
        if (ok) for (INT y=0;y<patch.Height();++y) {
            BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data()+y*row,p,row);
            if (write) memcpy(p,write->data()+y*row,row);
            if (clear) memset(p,0,row);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp=pixels(&savedPixels,NULL,FALSE); check("back-surface-backed-up",backedUp);
    struct Origin : CGameObject { BYTE GetObjectType() override { return TYPE_NONE; } };
    auto* origin=new Origin; origin->m_pArea=area; origin->m_pos=CPoint(128,96);
    check("source-registered",objects->Add(&origin->m_id,origin,INFINITE)==CGameObjectArray::SUCCESS);
    LONG targetId=game->GetFixedOrderCharacterId(1); CGameObject* target=NULL; CPoint targetPos;
    BYTE targetRC=objects->GetShare(targetId,CGameObjectArray::THREAD_ASYNCH,&target,INFINITE);
    check("party-target-available",targetRC==CGameObjectArray::SUCCESS);
    if (targetRC==CGameObjectArray::SUCCESS) { targetPos=target->GetPos(); objects->ReleaseShare(targetId,CGameObjectArray::THREAD_ASYNCH,INFINITE); }
    auto retire=[&](CGameObject* object) {
        POSITION node=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(object->m_id));
        if (node) area->m_lVertSortAdd.RemoveAt(node);
        object->RemoveFromArea();
        check("fixture-object-retired",objects->Delete(object->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE)==CGameObjectArray::SUCCESS);
        delete object;
    };
    struct Impact : CProjectileImpactBAM {
        using CProjectileImpactBAM::m_vidCell;
        using CProjectileImpactBAM::m_bitmap;
        using CProjectileImpactBAM::m_renderFlags;
        using CProjectileImpactBAM::m_hasPalette;
        using CProjectileImpactBAM::m_translucency;
        INT arrivals=0;
        Impact(const char* cell,const char* palette,LONG flags=8,BYTE translucency=0)
            : CProjectileImpactBAM(CString(cell),CString(_T("")),CString(palette),flags,translucency) {}
        void OnArrival() override { ++arrivals; }
    };
    struct Sparkle : CProjectileSparkleExplosion {
        Sparkle() : CProjectileSparkleExplosion(CResRef("TRAVEL")) {}
        BOOL Defaults() { return m_childProjectileType==0x4E && m_pVidCell->GetResRef()==CResRef("TRAVEL")
            && m_renderFlags==0x20000 && m_sparkleColor==0; }
    };
    Sparkle defaults; check("leaf-only-overrides-explode",defaults.Defaults());
    game->m_nTimeStop=0;
    infinity.pResWED=NULL; infinity.rViewPort=CRect(80,80,400,320);
    infinity.nCurrentX=72; infinity.nCurrentY=882;
    memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size());
    for (const auto& golden : SPARKLE_EXPLOSION_FACTORY) {
        CProjectile* object=CProjectile::DecodeProjectile(golden.type,NULL,255);
        BOOL kind=golden.sparkle?dynamic_cast<CProjectileSparkleExplosion*>(object)!=NULL:
            dynamic_cast<CProjectileImpactBAM*>(object)!=NULL;
        check("native-factory-class-and-metadata",kind && object->m_projectileType==golden.type-1
            && object->m_fireSoundRef==CResRef(golden.fire) && object->m_arrivalSoundRef==CResRef(golden.arrival));
        if (golden.sparkle && kind && targetRC==CGameObjectArray::SUCCESS) {
            auto* parent=static_cast<CProjectileSparkleExplosion*>(object);
            check("native-sparkle-factory-flags",parent->m_sparkleColor==golden.color && parent->m_bSparkleTrail==TRUE
                && parent->m_bHasHeight==TRUE && parent->GetRenderFlags()==0x20008);
            parent->m_pArea=area; parent->m_sourceId=origin->m_id; parent->m_targetId=targetId;
            parent->m_targetX=-123; parent->m_targetY=456;
            parent->Explode();
            check("native-colored-impact-launched",area->m_lVertSortAdd.GetCount()==front+1);
            if (area->m_lVertSortAdd.GetCount()>front) {
                LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetTail()); CGameObject* child=NULL;
                BYTE rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&child,INFINITE);
                check("impact-registered",rc==CGameObjectArray::SUCCESS && dynamic_cast<CProjectileImpactBAM*>(child)!=NULL);
                if (rc==CGameObjectArray::SUCCESS) {
                    auto* effect=static_cast<CProjectile*>(child); USHORT expected=0;
                    for (const auto& row : SPARKLE_EXPLOSION_DISPATCH) if (row.color==golden.color) expected=row.type;
                    check("native-impact-provenance-and-placement",effect->m_projectileType==expected-1
                        && effect->m_sourceId==origin->m_id && effect->m_targetId==targetId
                        && effect->m_pos==CPoint(targetPos.x,targetPos.y+1) && effect->m_posZ==30
                        && effect->CGameObject::m_pArea==area && effect->m_effectList.IsEmpty());
                    objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); retire(child);
                }
            }
            while (handler->m_messageList.GetCount()>messages) {
                CMessage* message=handler->m_messageList.RemoveTail();
                auto* group=dynamic_cast<CMessage105*>(message);
                check("empty-impact-group-ownership",group && group->m_nCount==0
                    && group->m_sourceId==origin->m_id && group->m_targetId==targetId);
                if (group) group->Run();
                delete message;
            }
        } else if (!golden.sparkle && kind && backedUp) {
            Impact reference(golden.cell,golden.palette);
            check("native-impact-constructor-state",reference.m_vidCell.GetResRef()==CResRef(golden.cell)
                && reference.m_hasPalette==!CString(golden.palette).IsEmpty() && reference.m_renderFlags==8
                && reference.m_translucency==0 && reference.m_bHasHeight==TRUE
                && reference.m_vidCell.GetCurrentSequenceId()==0 && reference.m_vidCell.m_nCurrentFrame==0);
            if (reference.m_vidCell.pRes) {
                ProjectileRenderFrameAnchor anchor(&reference.m_vidCell);
                if (anchor.frame && (!anchor.header || anchor.headerFrame)) {
                    object->m_pArea=area; object->CGameObject::m_pArea=area;
                    object->m_pos=CPoint(232,1002); object->m_posZ=17; object->m_listType=CGameObject::LIST_FLIGHT;
                    INT bitmapDemands=reference.m_bitmap.pRes?reference.m_bitmap.pRes->GetDemands():0;
                    auto paint=[&](BOOL visible)->BOOL {
                        if (!visible) return TRUE;
                        CPoint center; CSize size;
                        reference.m_vidCell.GetCurrentCenterPoint(center,FALSE); reference.m_vidCell.GetCurrentFrameSize(size,FALSE);
                        CRect bounds(0,0,size.cx,size.cy); CPoint point(232,985);
                        DWORD flags=8|CInfinity::FXPREP_COPYFROMBACK;
                        if (reference.m_hasPalette) {
                            reference.m_bitmap.pRes->Demand();
                            reference.m_vidCell.SetPalette(reference.m_bitmap.pRes->GetColorTable(),reference.m_bitmap.pRes->GetColorCount(),CVidPalette::TYPE_RESOURCE);
                            reference.m_bitmap.pRes->Release();
                        }
                        infinity.FXPrep(bounds,flags,CVIDINF_SURFACE_BACK,point,center);
                        if (!infinity.FXLock(bounds,flags)) return FALSE;
                        infinity.FXRender(&reference.m_vidCell,center.x,center.y,flags,0);
                        infinity.FXUnlock(flags,NULL,CPoint(0,0));
                        return infinity.FXBltFrom(CVIDINF_SURFACE_BACK,bounds,point.x,point.y,center.x,center.y,flags);
                    };
                    for (INT mode=0;mode<3;++mode) {
                        SHORT tile=area->m_visibility.PointToTile(object->m_pos);
                        if (mode==1) area->m_visibility.m_pMap[tile]=0;
                        if (mode==2) infinity.nCurrentX+=10000;
                        std::vector<BYTE> expected,actual;
                        BOOL ok=pixels(NULL,NULL,TRUE) && paint(mode==0) && pixels(&expected,NULL,TRUE);
                        object->Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws;
                        ok=pixels(&actual,NULL,FALSE) && ok;
                        check("impact-full-software-pixels",ok && actual==expected);
                        check("impact-render-resource-and-surface-balance",video->m_SurfaceDesc.lpSurface==NULL
                            && (!reference.m_bitmap.pRes || reference.m_bitmap.pRes->GetDemands()==bitmapDemands));
                        area->m_visibility.m_pMap[tile]=CVisibilityMap::VISIBILITY_MASK;
                        if (mode==2) infinity.nCurrentX-=10000;
                    }
                } else check("impact-bam-frame-available",FALSE);
                check("impact-frame-and-demands-restored",anchor.Restore());
            } else check("impact-bam-resource-available",FALSE);
            object->CGameObject::m_pArea=NULL;
        }
        delete object;
    }
    for (const auto& row : SPARKLE_EXPLOSION_DISPATCH) if (row.type==0) {
        defaults.m_sparkleColor=row.color; defaults.m_pArea=area;
        defaults.Explode(); check("unsupported-signed-color-does-not-launch",area->m_lVertSortAdd.GetCount()==front);
    }
    for (LONG flags : {LONG(0),LONG(8),LONG(0x80000000u)}) for (BYTE translucency : {BYTE(0),BYTE(2),BYTE(255)}) {
        Impact state("ARARROW","",flags,translucency);
        state.m_pArea=area; state.CGameObject::m_pArea=area; state.m_pos=CPoint(232,1002);
        state.m_posZ=17; state.m_listType=CGameObject::LIST_FLIGHT;
        ProjectileRenderFrameAnchor anchor(&state.m_vidCell);
        SHORT frame=state.m_vidCell.m_nCurrentFrame;
        state.Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws;
        LONG expected=flags==0?0:flags|CInfinity::FXPREP_COPYFROMBACK;
        check("render-zero-flags-guard-and-dword-preservation",state.m_renderFlags==expected
            && state.m_translucency==translucency && state.m_vidCell.m_nCurrentFrame==frame);
        check("render-state-surface-and-demands-balanced",video->m_SurfaceDesc.lpSurface==NULL && anchor.Restore());
        state.CGameObject::m_pArea=NULL;
    }
    {
        Impact animation("SHAREA",""); animation.m_pArea=area; animation.m_id=-123;
        animation.m_pos=CPoint(232,1002); animation.m_posZ=17;
        SHORT before=animation.m_vidCell.m_nCurrentFrame;
        game->m_nTimeStop=1; game->m_nTimeStopCaster=-1; animation.AIUpdate();
        check("impact-time-stop-keeps-frame",animation.m_vidCell.m_nCurrentFrame==before && animation.arrivals==0);
        game->m_nTimeStopCaster=-123; animation.AIUpdate();
        check("impact-time-stop-owner-advances",animation.m_vidCell.m_nCurrentFrame==before+1 && animation.arrivals==0);
        game->m_nTimeStop=0;
        animation.m_vidCell.FrameSet(animation.m_vidCell.GetSequenceLength(0,FALSE)-1);
        before=animation.m_vidCell.m_nCurrentFrame; animation.AIUpdate();
        check("impact-end-arrives-before-advance",animation.arrivals==1 && animation.m_vidCell.m_nCurrentFrame==before);
    }
    retire(origin);
    memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size());
    infinity.pResWED=wed; infinity.rViewPort=view; infinity.nCurrentX=viewX; infinity.nCurrentY=viewY;
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster;
    check("back-surface-restored",backedUp && pixels(NULL,&savedPixels,FALSE));
    check("same-tick-state-restored",game->GetWorldTimer()->m_gameTime==clock && handler->m_messageList.GetCount()==messages
        && area->m_lVertSortAdd.GetCount()==front && area->m_lVertSortBackAdd.GetCount()==back
        && area->m_lVertSortFlightAdd.GetCount()==flight && area->m_lVertSort.GetCount()==sorted);
    Emit("{\"step\":%d,\"op\":\"sparkleexplosionchecks\",\"checks\":%d,\"renderCalls\":%d,\"nativeCases\":37,\"restored\":true,\"valid\":%s}",nStep,checks,draws,valid?"true":"false");
    if (!valid) Fail(nStep,"sparkleexplosionchecks","colored impact factory/lifecycle/render mismatch");
}

void AreaNearestChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame(); auto* objects = game->GetObjectArray();
    auto* handler = g_pBaldurChitin->GetMessageHandler();
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"nearest-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    struct Saved {
        CGameSprite* p = NULL; CCreatureFileHeader base; CDerivedStats derived,temp;
        CPoint pos; CGameArea* area; POSITION node; BYTE list,ea,race,saves[5];
        BOOL active,ai,imprisoned,color,dirty,blood; INT sequence,bloodAmount,damageTaken,timed,equipped; ULONG locator;
        void Capture(CGameSprite* sprite) {
            p = sprite; base = p->m_baseStats; derived = p->m_derivedStats; temp = p->m_tempStats;
            pos = p->m_pos; area = p->m_pArea; node = p->m_posVertList; list = p->m_listType;
            ea = p->m_typeAI.m_nEnemyAlly; race = p->m_typeAI.m_nRace;
            active = p->m_active; ai = p->m_activeAI; imprisoned = p->m_activeImprisonment;
            color = p->m_hasColorEffects; dirty = p->field_562C; blood = p->m_bBloodFlashOn;
            sequence = p->m_nSequence; bloodAmount = p->m_nBloodFlashAmount;
            damageTaken = p->m_nDamageTaken; locator = p->m_nDamageLocatorTime;
            timed = p->m_timedEffectList.GetCount(); equipped = p->m_equipedEffectList.GetCount();
            saves[0]=p->field_70F6; saves[1]=p->field_70F7; saves[2]=p->field_70F8; saves[3]=p->field_70F9; saves[4]=p->field_70FA;
        }
        void Restore() {
            p->m_baseStats=base; p->m_derivedStats=derived; p->m_tempStats=temp;
            p->m_pos=pos; p->m_pArea=area; p->m_posVertList=node; p->m_listType=list;
            p->m_typeAI.m_nEnemyAlly=ea; p->m_typeAI.m_nRace=race;
            p->m_active=active; p->m_activeAI=ai; p->m_activeImprisonment=imprisoned;
            p->m_hasColorEffects=color; p->field_562C=dirty; p->m_bBloodFlashOn=blood;
            p->m_nSequence=sequence; p->m_nBloodFlashAmount=bloodAmount;
            p->m_nDamageTaken=damageTaken; p->m_nDamageLocatorTime=locator;
            p->field_70F6=saves[0]; p->field_70F7=saves[1]; p->field_70F8=saves[2]; p->field_70F9=saves[3]; p->field_70FA=saves[4];
        }
    } saved[4];
    LONG ids[4]; BYTE locked[4]; CGameObject* raw[4] = {};
    BOOL available = TRUE;
    for (INT i=0;i<4;++i) {
        ids[i]=game->GetFixedOrderCharacterId(i);
        locked[i]=objects->GetDeny(ids[i],CGameObjectArray::THREAD_ASYNCH,&raw[i],INFINITE);
        if (locked[i]==CGameObjectArray::SUCCESS) saved[i].Capture(static_cast<CGameSprite*>(raw[i])); else available=FALSE;
    }
    check("four-party-fixtures-available",available);
    ULONG clock=game->GetWorldTimer()->m_gameTime; INT messages=handler->m_messageList.GetCount();
    auto timeStop=game->m_nTimeStop, timeStopCaster=game->m_nTimeStopCaster;
    if (available) {
        auto areaCounter=CGameArea::dword_8D212C; auto areaFlag=CGameArea::byte_8D2138;
        auto* area=new CGameArea(255); area->m_pGame=game;
        std::vector<BYTE> visibility(128*128,CVisibilityMap::VISIBILITY_MASK);
        area->m_visibility.m_pMap=visibility.data(); area->m_visibility.m_nWidth=128;
        area->m_visibility.m_nHeight=128; area->m_visibility.m_nMapSize=visibility.size();
        area->m_visibility.m_nSearchRangeH=16; area->m_search.m_GridSquareDimensions=CSize(16,16);
        CAIObjectType type(CAIObjectType::ANYONE); type.m_nEnemyAlly=222;
        for (INT test=0;test<_countof(AREA_NEAREST_CASES);++test) {
            const auto& c=AREA_NEAREST_CASES[test]; area->m_lVertSort.RemoveAll();
            for (INT j=0;j<4;++j) {
                auto* p=saved[j].p; p->m_pos=CPoint(c.actors[j][0],c.actors[j][1]);
                p->m_derivedStats.m_generalState=c.actors[j][2]; p->m_baseStats.m_bStealthMode=BYTE(c.actors[j][3]);
                p->m_typeAI.m_nRace=BYTE(c.actors[j][4]); p->m_typeAI.m_nEnemyAlly=222;
                p->m_active=c.actors[j][5]; p->m_activeAI=p->m_activeImprisonment=TRUE;
                p->m_posVertList=NULL; p->m_listType=CGameObject::LIST_FRONT;
            }
            for (INT j=0;j<c.count;++j) {
                INT slot=c.order[j]; LONG id=slot==99?-123:ids[slot];
                POSITION node=area->m_lVertSort.AddTail(reinterpret_cast<int*>(id));
                if (slot!=99 && c.actors[slot][6]) saved[slot].p->m_posVertList=node;
            }
            LONG result=c.point?area->FindObjectNear(c.center[0],c.center[1],type,c.range,area->m_visibleTerrainTable,
                FALSE,TRUE,c.mode,c.include):area->GetNearest(ids[0],type,c.range,area->m_visibleTerrainTable,
                FALSE,TRUE,c.sleep,BYTE(c.mode),c.include);
            LONG expected=c.result==-1?-1:ids[c.result];
            if (result!=expected) Emit("{\"step\":%d,\"op\":\"nearest-native-mismatch\",\"index\":%d,\"actual\":%ld,\"expected\":%ld}",nStep,test,result,expected);
            check("captured-native-selection",result==expected);
        }
        // Point filters receive the perspective-scaled candidate point.
        area->m_lVertSort.RemoveAll();
        auto* target=saved[1].p; target->m_pos=CPoint(140,96); target->m_typeAI.m_nRace=0;
        target->m_derivedStats.m_generalState=0; target->m_active=TRUE; target->m_baseStats.m_bStealthMode=0;
        target->m_posVertList=area->m_lVertSort.AddTail(reinterpret_cast<int*>(ids[1]));
        CAIObjectType shape(type); shape.SetCenter(CPoint(140,128)); shape.SetRadius(0);
        check("location-filter-sees-scaled-y",area->FindObjectNear(128,96,shape,50,area->m_visibleTerrainTable,FALSE,TRUE,FALSE,FALSE)==ids[1]);
        shape.SetCenter(CPoint(140,96));
        check("location-filter-does-not-use-world-y",area->FindObjectNear(128,96,shape,50,area->m_visibleTerrainTable,FALSE,TRUE,FALSE,FALSE)==-1);
        target->m_pos=CPoint(128,96);
        check("point-los-equal-position",area->FindObjectNear(128,96,type,50,area->m_visibleTerrainTable,TRUE,TRUE,FALSE,FALSE)==ids[1]);
        saved[0].p->m_pos=CPoint(128,96); saved[0].p->m_listType=CGameObject::LIST_BACK;
        check("nonfront-search-delegates-to-point",area->GetNearest(ids[0],type,50,area->m_visibleTerrainTable,TRUE,TRUE,TRUE,0,FALSE)==ids[1]);
        // Launch the factory carrier far enough away to exercise actual
        // movement, then let decreasing spacings select two unseen targets.
        area->m_lVertSort.RemoveAll();
        for (INT i=0;i<4;++i) {
            auto* p=saved[i].p; p->m_typeAI.m_nRace=0; p->m_typeAI.m_nEnemyAlly=saved[i].ea;
            p->m_baseStats.m_generalState=p->m_derivedStats.m_generalState=0;
            p->m_active=p->m_activeAI=p->m_activeImprisonment=TRUE; p->m_baseStats.m_bStealthMode=0;
            p->m_baseStats.m_hitPoints=p->m_baseStats.m_maxHitPointsBase=512;
            p->m_derivedStats.m_nMaxHitPoints=512; p->m_derivedStats.m_spellStates.reset(SPLSTATE_TORTOISE_SHELL);
            p->m_derivedStats.m_cImmunitiesAIType.ClearAll();
            p->m_nSequence=CGameSprite::SEQ_SLEEP;
            p->m_pos=CPoint(i==1?137:i==2?141:i==3?143:128,96);
            p->m_pArea=area;
            p->m_listType=CGameObject::LIST_FRONT; p->m_posVertList=NULL;
            if (i) p->m_posVertList=area->m_lVertSort.AddTail(reinterpret_cast<int*>(ids[i]));
        }
        struct Origin : CGameObject { BYTE GetObjectType() override { return TYPE_NONE; } };
        auto* origin=new Origin; origin->m_pos=CPoint(48,96); origin->m_pArea=area; origin->m_listType=CGameObject::LIST_BACK;
        objects->Add(&origin->m_id,origin,INFINITE);
        auto retire=[&](CGameObject* object) {
            POSITION node=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(object->m_id));
            if (node) area->m_lVertSortAdd.RemoveAt(node);
            object->RemoveFromArea(); objects->Delete(object->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete object;
        };
        for (BYTE delay : {BYTE(0),BYTE(255)}) {
            CProjectile* decoded=CProjectile::DecodeProjectile(0xD2,NULL,delay);
            check("chain-factory-class-and-type",dynamic_cast<IcewindCProjectileChainLightning*>(decoded)
                && decoded->m_projectileType==0xD1 && decoded->m_fireSoundRef==CResRef("TRA_09")
                && decoded->m_effectList.IsEmpty() && decoded->m_sourceId==0);
            delete decoded;
        }
        CProjectile* chain=CProjectile::DecodeProjectile(0xD2,saved[0].p,17);
        check("real-chain-factory-carrier",dynamic_cast<IcewindCProjectileChainLightning*>(chain)!=NULL);
        CSpell damage(CResRef("ZRCHIT")); BOOL loaded=damage.Demand()!=NULL && damage.pRes->m_bParsed;
        check("real-chain-damage-resource",loaded);
        if (loaded) {
            ITEM_EFFECT effect=damage.pRes->m_pEffects[0]; effect.effectAmount=6; effect.dwFlags=0x40000000;
            effect.special=DWORD(-1); effect.numDice=0; effect.diceSize=0;
            CGameEffect* payload=CGameEffect::DecodeEffect(&effect,origin->m_pos,ids[0],CPoint(137,96)); payload->m_casterLevel=4;
            chain->AddEffect(payload);
            chain->Fire(area,origin->m_id,ids[1],saved[1].p->m_pos,17,0);
            CGameObject* registered=NULL;
            BYTE registration=objects->GetShare(chain->m_id,CGameObjectArray::THREAD_ASYNCH,&registered,INFINITE);
            check("initial-chain-fire-registers-target",chain->m_id!=CGameObjectArray::INVALID_INDEX
                && registration==CGameObjectArray::SUCCESS && registered==chain
                && chain->m_sourceId==origin->m_id && chain->m_targetId==ids[1]
                && chain->m_pos==origin->m_pos && chain->m_targetX==137 && chain->m_targetY==96);
            if (registration==CGameObjectArray::SUCCESS) objects->ReleaseShare(chain->m_id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
            LONG initialId=chain->m_id; CPoint initialPos=chain->m_pos;
            game->m_nTimeStop=1; game->m_nTimeStopCaster=origin->m_id;
            chain->AIUpdate();
            check("factory-chain-obeys-time-stop",chain->m_pos==initialPos && !chain->m_effectList.IsEmpty()
                && handler->m_messageList.GetCount()==messages);
            game->m_nTimeStop=0;
            INT totalTicks=0;
            for (INT hop=0;hop<3 && chain;++hop) {
                LONG parentId=chain->m_id, targetId=chain->m_targetId;
                BOOL arrived=FALSE, moved=FALSE; INT ticks=0;
                // Pending nodes are consumed by area sorting in a normal
                // tick. Remove ours before self-deleting OnArrival here.
                POSITION pending=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(parentId));
                if (pending) area->m_lVertSortAdd.RemoveAt(pending);
                for (;ticks<64 && !arrived;++ticks) {
                    CPoint previous=chain->m_pos;
                    chain->AIUpdate(); ++totalTicks;
                    CGameObject* live=NULL;
                    BYTE rc=objects->GetShare(parentId,CGameObjectArray::THREAD_ASYNCH,&live,INFINITE);
                    arrived=rc!=CGameObjectArray::SUCCESS;
                    if (arrived) check("arrival-deletes-object-array-entry",rc==CGameObjectArray::DELETED);
                    if (!arrived) {
                        chain=static_cast<CProjectile*>(live); moved=moved || chain->m_pos!=previous;
                        objects->ReleaseShare(parentId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                        check("travelling-chain-retains-payload",!chain->m_effectList.IsEmpty()
                            && handler->m_messageList.GetCount()==messages);
                    }
                }
                check("chain-arrival-retires-parent",arrived);
                if (hop==0) check("initial-chain-really-travels",moved && ticks>1 && parentId==initialId);
                IcewindCProjectileChainLightning* child=NULL;
                POSITION node=area->m_lVertSortAdd.GetHeadPosition();
                while (node) {
                    LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(node)); CGameObject* object=NULL;
                    if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                        if (id!=parentId) child=dynamic_cast<IcewindCProjectileChainLightning*>(object);
                        objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                    }
                }
                check("three-live-chain-hops",hop<2?child && child->m_sourceId==parentId
                    && child->m_targetId==ids[hop+2] && child->m_projectileType==0:child==NULL);
                Emit("{\"step\":%d,\"op\":\"chain-live-hop\",\"hop\":%d,\"target\":%ld,\"child\":%ld,\"ticks\":%d,\"arrived\":%s}",nStep,hop,targetId,child?child->m_targetId:-1,ticks,arrived?"true":"false");
                while (handler->m_messageList.GetCount()>messages) {
                    CMessage* message=handler->m_messageList.RemoveTail();
                    if (dynamic_cast<CMessage105*>(message) || dynamic_cast<CMessageAddEffect*>(message)) message->Run();
                    delete message;
                }
                if (!arrived) retire(chain);
                chain=child;
            }
            check("factory-flight-chain-all-carriers-retired",chain==NULL && area->m_lVertSortAdd.IsEmpty() && totalTicks>3);
            check("factory-chain-does-not-damage-caster",saved[0].p->m_baseStats.m_hitPoints==512);
            for (INT i=1;i<4;++i) {
                LONG loss=512-saved[i].p->m_baseStats.m_hitPoints;
                check("chain-actual-hp-loss",loss==6);
                check("instant-chain-effect-does-not-persist",saved[i].p->m_timedEffectList.GetCount()==saved[i].timed
                    && saved[i].p->m_equipedEffectList.GetCount()==saved[i].equipped);
                Emit("{\"step\":%d,\"op\":\"chain-live-damage\",\"target\":%d,\"loss\":%ld}",nStep,i,loss);
            }
            // The new BAM carrier delivers ordinary effects during Fire,
            // before registration/animation, and must not repeat them on expiry.
            CProjectile* impact=CProjectile::DecodeProjectile(0xC5,saved[0].p,0);
            impact->AddEffect(CGameEffect::DecodeEffect(&effect,origin->m_pos,ids[0],saved[1].p->m_pos));
            LONG hpBefore=saved[1].p->m_baseStats.m_hitPoints;
            impact->Fire(area,origin->m_id,ids[1],CPoint(-123,456),30,2);
            LONG impactId=impact->m_id;
            check("bam-fire-delivers-before-animation",impact->m_effectList.IsEmpty()
                && handler->m_messageList.GetCount()==messages+1
                && impact->m_pos==CPoint(137,97) && impact->m_posZ==30);
            while (handler->m_messageList.GetCount()>messages) {
                CMessage* message=handler->m_messageList.RemoveTail();
                if (dynamic_cast<CMessage105*>(message) || dynamic_cast<CMessageAddEffect*>(message)) message->Run();
                delete message;
            }
            check("bam-fire-actual-hp-loss",hpBefore-saved[1].p->m_baseStats.m_hitPoints==6
                && saved[0].p->m_baseStats.m_hitPoints==512);
            POSITION pending=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(impactId));
            if (pending) area->m_lVertSortAdd.RemoveAt(pending);
            BOOL retired=FALSE; INT ticks=0;
            for (;ticks<512 && !retired;++ticks) {
                impact->AIUpdate(); CGameObject* live=NULL;
                BYTE rc=objects->GetShare(impactId,CGameObjectArray::THREAD_ASYNCH,&live,INFINITE);
                retired=rc!=CGameObjectArray::SUCCESS;
                if (retired) check("bam-expiry-deletes-carrier",rc==CGameObjectArray::DELETED);
                else { impact=static_cast<CProjectile*>(live); objects->ReleaseShare(impactId,CGameObjectArray::THREAD_ASYNCH,INFINITE); }
            }
            if (!retired) retire(impact);
            while (handler->m_messageList.GetCount()>messages) {
                CMessage* message=handler->m_messageList.RemoveTail();
                auto* group=dynamic_cast<CMessage105*>(message);
                check("bam-expiry-queues-empty-owned-group",group && group->m_nCount==0
                    && group->m_sourceId==origin->m_id && group->m_targetId==ids[1]);
                if (group) group->Run();
                delete message;
            }
            check("bam-animation-expires-without-repeat-damage",retired
                && hpBefore-saved[1].p->m_baseStats.m_hitPoints==6 && handler->m_messageList.GetCount()==messages
                && saved[1].p->m_timedEffectList.GetCount()==saved[1].timed && saved[1].p->m_equipedEffectList.GetCount()==saved[1].equipped);
            Emit("{\"step\":%d,\"op\":\"impact-bam-damage\",\"loss\":%ld,\"ticks\":%d,\"retired\":%s}",nStep,
                hpBefore-saved[1].p->m_baseStats.m_hitPoints,ticks,retired?"true":"false");
            for (USHORT type : {USHORT(0x6C),USHORT(0xC0),USHORT(0xC1),USHORT(0xC2),
                USHORT(7),USHORT(10),USHORT(11),USHORT(27),USHORT(30),USHORT(31),USHORT(56),USHORT(59),USHORT(60),USHORT(375)}) {
                CProjectile* blob=CProjectile::DecodeProjectile(type,saved[0].p,255);
                BOOL acid=type==0x6C || (type>=0xC0 && type<=0xC2);
                check("travelling-live-factory-class",dynamic_cast<CProjectileTravelling*>(blob)!=NULL && blob->m_projectileType==type-1);
                auto& immunity=saved[1].p->m_derivedStats.m_cImmunitiesProjectile;
                auto& levels=saved[1].p->m_derivedStats.m_cImmunitiesSpellLevel;
                immunity.RemoveAll(); memset(levels.m_levels,0,sizeof(levels.m_levels));
                immunity.AddTail(reinterpret_cast<DWORD*>(DWORD(type-1)|0x10000));
                blob->m_nSpellLevel=0;
                check("projectile-immunity-is-full-dword",!blob->IsTargetImmune(saved[1].p));
                immunity.RemoveAll(); immunity.AddTail(reinterpret_cast<DWORD*>(DWORD(type-1)));
                blob->m_sourceId=origin->m_id; blob->m_targetId=ids[1]; blob->m_pArea=area; blob->m_casterResRef=CResRef("");
                LONG immuneHP=saved[1].p->m_baseStats.m_hitPoints;
                blob->AddEffect(CGameEffect::DecodeEffect(&effect,origin->m_pos,ids[0],saved[1].p->m_pos));
                blob->DeliverEffects();
                check("projectile-immunity-discards-owned-payload",blob->m_effectList.IsEmpty()
                    && handler->m_messageList.GetCount()==messages && saved[1].p->m_baseStats.m_hitPoints==immuneHP);
                immunity.RemoveAll(); levels.m_levels[0]=levels.m_levels[8]=levels.m_levels[9]=TRUE;
                for (DWORD level : {0u,8u,9u,0x10008u,0x10009u,0xFFFFFFFFu}) {
                    blob->m_nSpellLevel=level;
                    check("spell-immunity-byte-gate-zero-through-eight",blob->IsTargetImmune(saved[1].p)==((level&0xFF)==0 || (level&0xFF)==8));
                }
                memset(levels.m_levels,0,sizeof(levels.m_levels)); blob->m_nSpellLevel=0;
                blob->AddEffect(CGameEffect::DecodeEffect(&effect,origin->m_pos,ids[0],saved[1].p->m_pos));
                LONG before=saved[1].p->m_baseStats.m_hitPoints;
                blob->Fire(area,origin->m_id,ids[1],CPoint(-123,456),99,0);
                LONG id=blob->m_id; CPoint launch=blob->m_pos;
                check("acid-launch-retains-damage-payload",blob->m_effectList.GetCount()==1 && handler->m_messageList.GetCount()==messages
                    && blob->m_pos==origin->m_pos && blob->m_targetX==137 && blob->m_targetY==96 && blob->m_posZ==32);
                game->m_nTimeStop=1; game->m_nTimeStopCaster=origin->m_id; blob->AIUpdate();
                check("acid-time-stop-freezes-flight",blob->m_pos==launch && blob->m_effectList.GetCount()==1);
                game->m_nTimeStop=0;
                POSITION pending=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id));
                if (pending) area->m_lVertSortAdd.RemoveAt(pending);
                BOOL arrived=FALSE,moved=FALSE; INT ticks=0;
                for (;ticks<64 && !arrived;++ticks) {
                    CPoint previous=blob->m_pos;
                    blob->AIUpdate(); CGameObject* live=NULL;
                    BYTE rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&live,INFINITE);
                    arrived=rc!=CGameObjectArray::SUCCESS;
                    if (arrived) check("acid-arrival-deletes-carrier",rc==CGameObjectArray::DELETED);
                    else {
                        blob=static_cast<CProjectile*>(live); moved=moved || blob->m_pos!=previous;
                        objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                        check("acid-does-not-hit-during-flight",handler->m_messageList.GetCount()==messages
                            && saved[1].p->m_baseStats.m_hitPoints==before && blob->m_effectList.GetCount()==1);
                    }
                }
                if (!arrived) retire(blob);
                check("acid-actual-flight-before-arrival",arrived && moved && ticks>1);
                check("acid-arrival-queues-one-owned-group",handler->m_messageList.GetCount()==messages+1);
                INT groups=0,sounds=0;
                while (handler->m_messageList.GetCount()>messages) {
                    CMessage* message=handler->m_messageList.RemoveTail();
                    auto* group=dynamic_cast<CMessage105*>(message);
                    auto* sound=dynamic_cast<CMessagePlaySound*>(message);
                    check("acid-arrival-message-ownership",(group && group->m_sourceId==origin->m_id
                        && group->m_targetId==ids[1] && group->m_nCount==1)
                        || (sound && sound->m_sourceId==ids[1] && sound->m_targetId==ids[1]
                            && sound->m_soundId==CGameSprite::SOUND_DAMAGE && sound->m_showText && sound->m_showCircle));
                    if (group) { ++groups; group->Run(); }
                    // Applying the real damage queues the defender's bark.
                    // Dispose our sound fixture without changing speech state.
                    if (sound) ++sounds;
                    delete message;
                }
                check("acid-one-damage-group-and-owned-bark",groups==1 && sounds==1);
                LONG loss=before-saved[1].p->m_baseStats.m_hitPoints;
                check("acid-arrival-real-six-hp-loss",loss==6 && saved[0].p->m_baseStats.m_hitPoints==512
                    && saved[1].p->m_timedEffectList.GetCount()==saved[1].timed && saved[1].p->m_equipedEffectList.GetCount()==saved[1].equipped);
                Emit("{\"step\":%d,\"op\":\"%s\",\"type\":%u,\"loss\":%ld,\"ticks\":%d,\"retired\":%s}",nStep,
                    acid?"acid-live-damage":"thrown-live-damage",type,loss,ticks,arrived?"true":"false");
            }
            // SPBRNHND copies the payload to selected polygon targets, then
            // delivers the original to its explicit target during Fire.
            origin->m_pos=CPoint(128,96);
            saved[1].p->m_pos=CPoint(160,96); saved[2].p->m_pos=CPoint(180,96); saved[3].p->m_pos=CPoint(40,300);
            LONG beforeSpray[3]={saved[1].p->m_baseStats.m_hitPoints,saved[2].p->m_baseStats.m_hitPoints,saved[3].p->m_baseStats.m_hitPoints};
            auto* spray=dynamic_cast<CProjectileSPBRNHND*>(CProjectile::DecodeProjectile(0x1A,saved[0].p,0));
            check("spray-factory-carrier",spray!=NULL);
            if (spray) {
                spray->AddEffect(CGameEffect::DecodeEffect(&effect,origin->m_pos,ids[0],CPoint(256,96)));
                spray->Fire(area,origin->m_id,ids[3],CPoint(256,96),31,0);
                LONG parentId=spray->m_id;
                check("spray-fire-delivers-immediately",spray->m_effectList.IsEmpty() && handler->m_messageList.GetCount()==messages+3
                    && spray->m_pos==CPoint(138,96) && spray->m_targetX==256 && spray->m_targetY==96 && spray->m_posZ==31);
                INT copies=0,groups=0,barks=0;
                while (handler->m_messageList.GetCount()>messages) {
                    CMessage* message=handler->m_messageList.RemoveTail();
                    auto* copy=dynamic_cast<CMessageAddEffect*>(message); auto* group=dynamic_cast<CMessage105*>(message);
                    auto* sound=dynamic_cast<CMessagePlaySound*>(message);
                    check("spray-owned-message-routing",(copy && copy->m_sourceId==origin->m_id && (copy->m_targetId==ids[1] || copy->m_targetId==ids[2]))
                        || (group && group->m_sourceId==origin->m_id && group->m_targetId==ids[3] && group->m_nCount==1)
                        || (sound && sound->m_sourceId==sound->m_targetId && (sound->m_targetId==ids[1] || sound->m_targetId==ids[2] || sound->m_targetId==ids[3])
                            && sound->m_soundId==CGameSprite::SOUND_DAMAGE));
                    if (copy) { ++copies; copy->Run(); } if (group) { ++groups; group->Run(); } if (sound) ++barks;
                    delete message;
                }
                check("spray-two-copies-and-original-target",copies==2 && groups==1 && barks==3 && saved[0].p->m_baseStats.m_hitPoints==512);
                for (INT i=0;i<3;++i) {
                    LONG loss=beforeSpray[i]-saved[i+1].p->m_baseStats.m_hitPoints;
                    check("spray-real-six-hp-loss",loss==6);
                    Emit("{\"step\":%d,\"op\":\"spray-live-damage\",\"target\":%d,\"loss\":%ld}",nStep,i+1,loss);
                }
                std::vector<LONG> carriers; POSITION node=area->m_lVertSortAdd.GetHeadPosition(); INT child=0;
                while (node) {
                    LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(node)); CGameObject* object=NULL;
                    if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                        if (id!=parentId) {
                            auto* cell=dynamic_cast<CProjectileSprayCell*>(object);
                            check("spray-native-registered-cell-order",cell && child<_countof(SPRAY_DAMAGE_POINTS)
                                && object->m_pos==CPoint(SPRAY_DAMAGE_POINTS[child][0],SPRAY_DAMAGE_POINTS[child][1]) && object->m_posZ==31);
                            ++child; carriers.push_back(id);
                        }
                        objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                    }
                }
                check("spray-native-nine-cell-fan",child==_countof(SPRAY_DAMAGE_POINTS)); carriers.push_back(parentId);
                for (LONG id : carriers) { POSITION queued=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if (queued) area->m_lVertSortAdd.RemoveAt(queued); }
                game->m_nTimeStop=1; game->m_nTimeStopCaster=origin->m_id;
                for (LONG id : carriers) { CGameObject* object=NULL;
                    if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                        objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); object->AIUpdate();
                    }
                }
                game->m_nTimeStop=0;
                for (INT tick=0;tick<55;++tick) for (LONG id : carriers) {
                    CGameObject* object=NULL; BYTE rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
                    check("spray-carrier-alive-before-expiry",rc==CGameObjectArray::SUCCESS);
                    if (rc==CGameObjectArray::SUCCESS) { objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); object->AIUpdate(); }
                    rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
                    check("spray-exact-fifty-five-tick-expiry",tick==54?rc==CGameObjectArray::DELETED:rc==CGameObjectArray::SUCCESS);
                    if (rc==CGameObjectArray::SUCCESS) objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                }
                for (LONG id : carriers) { CGameObject* object=NULL;
                    if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                        objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); retire(object);
                    }
                }
                check("spray-expiry-no-repeat-damage",handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.IsEmpty());
                for (INT i=0;i<3;++i) check("spray-hit-stays-instant",beforeSpray[i]-saved[i+1].p->m_baseStats.m_hitPoints==6
                    && saved[i+1].p->m_timedEffectList.GetCount()==saved[i+1].timed && saved[i+1].p->m_equipedEffectList.GetCount()==saved[i+1].equipped);
            }
        }
        if (chain) {
            if (chain->CGameObject::m_pArea) retire(chain); else delete chain;
        }
        damage.Release(); retire(origin);
        for (INT i=0;i<4;++i) saved[i].Restore();
        area->m_lVertSort.RemoveAll(); area->m_visibility.m_pMap=NULL; delete area;
        CGameArea::dword_8D212C=areaCounter; CGameArea::byte_8D2138=areaFlag;
    }
    for (INT i=0;i<4;++i) if (locked[i]==CGameObjectArray::SUCCESS) objects->ReleaseDeny(ids[i],CGameObjectArray::THREAD_ASYNCH,INFINITE);
    game->m_nTimeStop=timeStop; game->m_nTimeStopCaster=timeStopCaster;
    check("same-tick-clock-messages-restored",game->GetWorldTimer()->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    Emit("{\"step\":%d,\"op\":\"nearestchecks\",\"checks\":%d,\"nativeCases\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,INT(_countof(AREA_NEAREST_CASES)),valid?"true":"false");
    if (!valid) Fail(nStep,"nearestchecks","area selection or chained damage mismatch");
}

void ChainLightningChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame(); auto* objects = game->GetObjectArray();
    auto* area = game->GetVisibleArea(); auto* handler = g_pBaldurChitin->GetMessageHandler();
    if (!area || !area->m_visibility.m_pMap) { Fail(nStep,"chainchecks","requires loaded area"); return; }
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"chain-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    struct Chain : IcewindCProjectileChainLightning {
        Chain(const std::set<LONG>& ids) : IcewindCProjectileChainLightning(ids) {}
        const std::set<LONG>& Seen() const { return m_struckTargets; }
        BOOL Configured() const { return m_dirCount==16 && m_pVidCell->GetCurrentSequenceId()==0
            && m_velocity==40 && m_visualEffect.m_bCopyFromBack==TRUE && !m_bHasHeight && !m_useHeightOffset; }
    };
    std::set<LONG> ids = {LONG(0x80000000u),-1,0,0x7FFFFFFF};
    Chain copied(ids); ids.clear();
    check("constructor-copies-signed-ids",copied.Seen().size()==4 && *copied.Seen().begin()==LONG(0x80000000u));
    check("constructor-configures-independent-carrier",copied.Configured() && copied.m_fireSoundRef==CResRef("TRA_09")
        && copied.m_effectList.IsEmpty() && copied.m_sourceId==0);
    copied.m_targetId = -2; copied.AddEffect(new CGameEffect()); copied.DeliverEffects();
    check("missing-target-insert-before-failed-delivery",copied.Seen().count(-2)==1 && copied.m_effectList.GetCount()==1);
    CGameObject* startObject = NULL; CGameObject* nextObject = NULL;
    LONG startId = game->GetFixedOrderCharacterId(0), nextId = game->GetFixedOrderCharacterId(1);
    BYTE startRC = objects->GetDeny(startId,CGameObjectArray::THREAD_ASYNCH,&startObject,INFINITE);
    BYTE nextRC = objects->GetDeny(nextId,CGameObjectArray::THREAD_ASYNCH,&nextObject,INFINITE);
    check("party-targets-available",startRC==CGameObjectArray::SUCCESS && nextRC==CGameObjectArray::SUCCESS);
    ULONG clock = game->GetWorldTimer()->m_gameTime; INT messageCount = handler->m_messageList.GetCount();
    INT front = area->m_lVertSort.GetCount(), pending = area->m_lVertSortAdd.GetCount();
    if (startRC==CGameObjectArray::SUCCESS && nextRC==CGameObjectArray::SUCCESS) {
        auto* start = static_cast<CGameSprite*>(startObject); auto* next = static_cast<CGameSprite*>(nextObject);
        CPoint startPos = start->m_pos, nextPos = next->m_pos;
        POSITION startNode = start->m_posVertList, nextNode = next->m_posVertList;
        auto startList = start->m_listType, nextList = next->m_listType;
        auto active = next->m_active, activeAI = next->m_activeAI, imprisoned = next->m_activeImprisonment;
        auto stealth = next->m_baseStats.m_bStealthMode;
        auto state = next->m_derivedStats.m_generalState;
        std::vector<BYTE> visibility(area->m_visibility.m_pMap,area->m_visibility.m_pMap+area->m_visibility.m_nMapSize);
        memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size());
        start->m_pos = next->m_pos = CPoint(128,96);
        start->m_listType = next->m_listType = CGameObject::LIST_FRONT;
        POSITION a = area->m_lVertSort.AddTail(reinterpret_cast<int*>(startId));
        POSITION b = area->m_lVertSort.AddTail(reinterpret_cast<int*>(nextId));
        start->m_posVertList = a; next->m_posVertList = b;
        next->m_active = next->m_activeAI = next->m_activeImprisonment = TRUE;
        next->m_baseStats.m_bStealthMode = 0; next->m_derivedStats.m_generalState &= ~(STATE_DEAD|STATE_SLEEPING);
        check("real-nearest-selects-unseen-party-member",area->GetNearest(startId,CAIObjectType::ANYONE,0x7FFF,
            area->m_visibleTerrainTable,TRUE,TRUE,TRUE,0,FALSE)==nextId);
        struct Payload : CGameEffect {
            std::vector<CGameEffect*>* copies;
            CGameEffect* Copy() override {
                auto* effect = new CGameEffect; effect->CopyFromBase(this); copies->push_back(effect); return effect;
            }
        };
        auto retire = [&](CProjectile* p) {
            POSITION node = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id));
            if (node) area->m_lVertSortAdd.RemoveAt(node);
            p->RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p;
        };
        // First-hop halving applies to DWORD dice count, not flat damage.
        // Caster level is compared using its low BYTE against set cardinality.
        for (INT test = 0; test < 10; ++test) {
            std::set<LONG> history;
            if (test==2 || test==8) history.insert(-123);
            if (test==9) history.insert(nextId);
            auto* chain = new Chain(history);
            chain->m_sourceId = startId; chain->m_targetId = startId;
            chain->m_casterResRef = CResRef("ZRCHIT"); chain->m_projectileType = 0x6C;
            chain->m_pArea = area; chain->CGameObject::m_pArea = area;
            chain->m_pos = CPoint(128,96); chain->m_posZ = 17;
            objects->Add(&chain->m_id,chain,INFINITE); LONG chainId = chain->m_id;
            std::vector<CGameEffect*> copies;
            auto* effect = new Payload; effect->copies = &copies;
            effect->m_effectID = test==7?CGAMEEFFECT_AC:CGAMEEFFECT_DAMAGE;
            effect->m_effectAmount = -17; effect->m_numDice = test==6?0x80000001u:9;
            effect->m_casterLevel = test==3?1:test==4?256:test==5?258:test==8?2:3;
            if (test==1 || test==3) {
                auto* prefix = new Payload; prefix->copies = &copies;
                prefix->m_effectID = CGAMEEFFECT_AC; prefix->m_effectAmount = 999;
                chain->AddEffect(prefix);
            }
            chain->AddEffect(effect);
            chain->DeliverEffects();
            BOOL shouldLaunch = test!=3 && test!=4 && test!=8 && test!=9;
            CProjectile* child = NULL;
            POSITION p = area->m_lVertSortAdd.GetHeadPosition();
            while (p) {
                LONG id = reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(p)); CGameObject* object = NULL;
                if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                    if (dynamic_cast<IcewindCProjectileChainLightning*>(object)) child = static_cast<CProjectile*>(object);
                    objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                }
            }
            check("child-registration-and-cardinality-limit",(child!=NULL)==shouldLaunch);
            check("parent-payload-delivered-on-every-path",chain->m_effectList.IsEmpty()
                && chain->Seen().count(startId)==1 && handler->m_messageList.GetCount()==messageCount+1);
            if (child) {
                check("child-launch-source-is-parent-projectile",child->m_sourceId==chainId && child->m_targetId==nextId
                    && child->m_targetX==128 && child->m_targetY==96 && child->m_effectList.GetCount()==(test==1?2:1));
                if (test==1) check("ordered-mixed-payload-copy",child->m_effectList.GetHead()->m_effectID==CGAMEEFFECT_AC
                    && child->m_effectList.GetHead()->m_effectAmount==999);
                CGameEffect* copy = child->m_effectList.GetTail();
                DWORD expectedDice = test==7 || test==2?9:test==6?0x40000000u:4;
                check("clone-dice-and-flat-damage",copy!=effect && copy->m_numDice==expectedDice
                    && copy->m_effectAmount==-17 && effect->m_numDice==(test==6?0x80000001u:9));
                check("child-constructor-does-not-copy-caster-metadata",child->m_casterResRef==CResRef("")
                    && child->m_projectileType==0);
                retire(child);
            } else if (!copies.empty()) {
                // Native threshold abort leaks its unattached copy; the test
                // owns that pointer explicitly so no fixture allocation leaks.
                check("aborted-copy-was-not-attached",copies.size()==(test==3?2:1));
                delete copies.back();
            }
            while (handler->m_messageList.GetCount()>messageCount) {
                auto* message = handler->m_messageList.RemoveTail();
                auto* group = dynamic_cast<CMessage105*>(message);
                check("original-effect-message-provenance",group && group->m_sourceId==startId
                    && group->m_targetId==startId && group->m_nCount==((test==1 || test==3)?2:1));
                delete message;
            }
            retire(chain);
            Emit("{\"step\":%d,\"op\":\"chain-hop\",\"case\":%d,\"launched\":%s}",nStep,test,child?"true":"false");
        }
        area->m_lVertSort.RemoveAt(a); area->m_lVertSort.RemoveAt(b);
        start->m_pos = startPos; next->m_pos = nextPos;
        start->m_posVertList = startNode; next->m_posVertList = nextNode;
        start->m_listType = startList; next->m_listType = nextList;
        next->m_active = active; next->m_activeAI = activeAI; next->m_activeImprisonment = imprisoned;
        next->m_baseStats.m_bStealthMode = stealth; next->m_derivedStats.m_generalState = state;
        memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size());
    }
    if (nextRC==CGameObjectArray::SUCCESS) objects->ReleaseDeny(nextId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    if (startRC==CGameObjectArray::SUCCESS) objects->ReleaseDeny(startId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    check("same-tick-state-restored",game->GetWorldTimer()->m_gameTime==clock && handler->m_messageList.GetCount()==messageCount
        && area->m_lVertSort.GetCount()==front && area->m_lVertSortAdd.GetCount()==pending);
    Emit("{\"step\":%d,\"op\":\"chainchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"chainchecks","chain propagation mismatch");
}

void BeamChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame(); auto* objects = game->GetObjectArray();
    auto* area = game->GetVisibleArea();
    if (!area) { Fail(nStep,"beamchecks","requires loaded area"); return; }
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"beam-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    auto stop = game->m_nTimeStop; auto caster = game->m_nTimeStopCaster;
    auto* handler = g_pBaldurChitin->GetMessageHandler(); INT messages = handler->m_messageList.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    INT sorted = area->m_lVertSort.GetCount();
    auto unqueue = [&](LONG id) {
        POSITION p = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortAdd.RemoveAt(p);
        p = area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortBackAdd.RemoveAt(p);
        p = area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortFlightAdd.RemoveAt(p);
    };
    auto retire = [&](CGameObject* p) {
        unqueue(p->m_id); p->RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p;
    };
    struct Origin : CGameObject {
        BYTE GetObjectType() override { return TYPE_NONE; }
    };
    auto* source = new Origin; source->m_pos = CPoint(128,96); source->m_pArea = area;
    check("source-registered",objects->Add(&source->m_id,source,INFINITE)==CGameObjectArray::SUCCESS);
    struct Target : CGameObject {
        CPoint live = CPoint(256,96); INT reads = 0;
        CPoint& GetPos() override { ++reads; return live; }
    };
    auto* target = new Target; target->m_pos = target->live; target->m_pArea = area; target->m_posZ = 31;
    check("target-registered",objects->Add(&target->m_id,target,INFINITE)==CGameObjectArray::SUCCESS);
    struct Beam : IcewindCProjectileScorcher {
        struct Cell : CVidCell {
            INT advances = 0;
            Cell() : CVidCell(CResRef("ARARROW"),FALSE) {}
            BOOL FrameAdvance() override { ++advances; return TRUE; }
        };
        Cell* cell; BOOL* dead = NULL; BOOL isolatedRemoval = FALSE;
        INT removals = 0, areaRemovals = 0, gathers = 0, deliveries = 0;
        std::vector<std::pair<LONG,BYTE>> strikes;
        Beam() { delete m_pVidCell; cell = new Cell; m_pVidCell = cell; }
        ~Beam() { if (dead) *dead = TRUE; }
        void RemoveSelf() override { ++removals; }
        void RemoveFromArea() override { ++areaRemovals; if (!isolatedRemoval) CGameObject::RemoveFromArea(); }
        void GatherTargets() override { ++gathers; }
        void Strike(LONG target,BYTE initial) override { strikes.emplace_back(target,initial); }
        void DeliverEffects() override { ++deliveries; IcewindCProjectileScorcher::DeliverEffects(); }
        void Seed(CGameArea* area,LONG source) {
            m_pArea = area; CGameObject::m_pArea = area; m_listType = CGameObject::LIST_FLIGHT;
            m_pos = CPoint(10,20); m_posZ = 17; m_sourceId = source; m_targetId = -1;
            m_targetX = 256; m_targetY = 96; m_velocity = 10; m_tickCount = 0; m_lifeSpan = 110;
            m_strikePeriod = 3; m_clearPeriod = 60; m_strikeTarget = 0x80000001u;
            m_deltaZ = 99; m_nSegments = 123; m_posAccumX = -111; m_posAccumY = -222; m_stepX = -333; m_stepY = -444;
            m_facing = 0; m_direction = 0; m_pVidCell->SequenceSet(0); m_pVidCell->FrameSet(0);
            m_struckTargets.clear(); strikes.clear(); gathers = removals = areaRemovals = deliveries = cell->advances = 0;
            m_sound.SetCoordinates(111,222,333);
        }
        LONG Tick() const { return m_tickCount; }
        void Tick(LONG value) { m_tickCount = value; }
        BYTE Life() const { return m_lifeSpan; }
        void Life(BYTE value) { m_lifeSpan = value; }
        BYTE Segments() const { return m_nSegments; }
        LONG HeightSlope() const { return m_deltaZ; }
        CPoint Delta() const { return CPoint(m_stepX,m_stepY); }
        CPoint Exact() const { return CPoint(m_posAccumX,m_posAccumY); }
        void Speed(SHORT value) { m_velocity = value; }
        void TargetPoint(CPoint point) { m_targetX = point.x; m_targetY = point.y; }
        void Periods(LONG strike,LONG clear) { m_strikePeriod = strike; m_clearPeriod = clear; }
        BOOL Seen(LONG id) const { return m_struckTargets.count(static_cast<DWORD>(id)) != 0; }
        void Remember(LONG id) { m_struckTargets.insert(static_cast<DWORD>(id)); }
        void Scan() { m_pos = CPoint(128,96); m_facing = 4; IcewindCProjectileScorcher::GatherTargets(); }
        LONG InitialTarget() const { return m_strikeTarget; }
    };
    game->m_nTimeStop = 0;
    auto* beam = new Beam; beam->Seed(area,source->m_id); CProjectile* dispatch = beam;
    dispatch->AIUpdate();
    check("tick-before-frame-and-source-anchor",beam->Tick()==1 && beam->Life()==109 && beam->cell->advances==1
        && beam->m_pos==CPoint(128,96) && beam->Exact()==CPoint(131072,131072));
    check("rounded-distance-ceil-and-steps",beam->Segments()==13 && beam->Delta()==CPoint(10082,0) && beam->HeightSlope()==0);
    check("initial-strike-before-gather",beam->strikes.size()==1 && beam->strikes[0]==std::make_pair(LONG(0x80000001u),BYTE(1))
        && beam->gathers==1 && beam->Seen(0x80000001u));
    check("sound-at-caster",beam->m_sound.m_nXCoordinate==128 && beam->m_sound.m_nYCoordinate==96 && beam->m_sound.m_nZCoordinate==17);
    dispatch->AIUpdate(); dispatch->AIUpdate(); dispatch->AIUpdate();
    check("strike-period-and-dedup",beam->Tick()==4 && beam->strikes.size()==1 && beam->gathers==2);
    beam->Tick(60); dispatch->AIUpdate();
    check("clear-before-restrike",beam->strikes.size()==2 && beam->gathers==3 && beam->Seen(0x80000001u));
    beam->Seed(area,source->m_id); game->m_nTimeStop = 1; game->m_nTimeStopCaster = source->m_id; dispatch->AIUpdate();
    check("time-stop-advances-only-clock",beam->Tick()==1 && beam->Life()==110 && beam->cell->advances==0 && beam->Exact()==CPoint(-111,-222));
    game->m_nTimeStopCaster = beam->m_id; dispatch->AIUpdate();
    check("time-stop-object-exemption",beam->Tick()==2 && beam->Life()==109 && beam->cell->advances==1 && beam->gathers==0);
    game->m_nTimeStop = 0;
    beam->Seed(area,source->m_id); beam->Tick(0x7FFFFFFF); dispatch->AIUpdate();
    check("signed-tick-wrap",beam->Tick()==LONG(0x80000000u) && beam->gathers==0);
    for (BYTE life : {BYTE(1),BYTE(255)}) {
        beam->Seed(area,source->m_id); beam->Life(life); dispatch->AIUpdate();
        check("byte-life-and-final-live-tick",beam->Life()==BYTE(life-1) && beam->cell->advances==1 && !beam->areaRemovals);
    }
    beam->Seed(area,source->m_id); beam->Life(0); dispatch->AIUpdate();
    check("expiry-bad-id-does-not-delete",beam->Tick()==1 && beam->areaRemovals==1 && beam->cell->advances==0);
    beam->Seed(area,source->m_id); beam->m_targetId = -2; dispatch->AIUpdate();
    check("missing-target-retires-after-frame",beam->removals==1 && beam->cell->advances==1 && beam->gathers==0);
    beam->Seed(area,source->m_id); beam->m_targetId = target->m_id; target->m_pArea = NULL; dispatch->AIUpdate();
    check("other-area-target-retires",beam->removals==1 && beam->cell->advances==1);
    target->m_pArea = area;
    beam->Seed(area,source->m_id); beam->m_targetId = target->m_id; target->reads = 0; dispatch->AIUpdate();
    check("virtual-live-target-and-height",target->reads==1 && beam->HeightSlope()==14 && beam->Segments()==13);
    beam->Seed(area,-1); dispatch->AIUpdate();
    check("missing-source-retains-tick-and-frame",beam->Life()==109 && beam->cell->advances==1 && beam->gathers==0 && beam->Exact()==CPoint(-111,-222));
    beam->Seed(area,source->m_id); beam->TargetPoint(source->m_pos); dispatch->AIUpdate();
    check("zero-distance-after-strike-arms-expiry",beam->Life()==0 && beam->Segments()==0 && beam->strikes.size()==1
        && beam->Delta()==CPoint(-333,-444));
    for (SHORT speed : {SHORT(1),SHORT(-1),SHORT(-10),SHORT(32767)}) {
        beam->Seed(area,source->m_id); beam->Speed(speed); dispatch->AIUpdate();
        BYTE expected = speed==1?128:speed==-1?130:speed==-10?245:1;
        check("signed-velocity-byte-segments",beam->Segments()==expected && beam->Delta()==CPoint(131072/expected,0));
    }
    beam->Seed(area,source->m_id); beam->TargetPoint(CPoint(128,99)); dispatch->AIUpdate();
    check("isometric-distance-rounded",beam->Segments()==1 && beam->Delta()==CPoint(0,4096));
    // The canonical numeric capture replaces RemoveFromArea with a counter;
    // match that collaborator here. Actual removal/deletion is tested below.
    beam->isolatedRemoval = TRUE;
    for (INT i = 0; i < _countof(BEAM_TICK_INPUTS); ++i) {
        const auto& c = BEAM_TICK_INPUTS[i]; const auto& expected = BEAM_TICK_OUTPUTS[i];
        source->m_pos = CPoint(c.origin[0],c.origin[1]);
        beam->Seed(area,c.missing?-1:source->m_id); beam->TargetPoint(CPoint(c.target[0],c.target[1]));
        beam->Speed(c.speed); beam->Tick(c.tick); beam->Life(c.life);
        game->m_nTimeStop = c.stop; game->m_nTimeStopCaster = c.exempt?beam->m_id:source->m_id;
        dispatch->AIUpdate();
        BOOL matches = beam->m_pos==CPoint(expected.state[0],expected.state[1])
            && beam->Exact()==CPoint(expected.state[2],expected.state[3]) && beam->Delta()==CPoint(expected.state[4],expected.state[5])
            && beam->HeightSlope()==expected.state[6] && beam->Tick()==expected.state[7]
            && beam->Life()==expected.life && beam->Segments()==expected.segments
            && beam->cell->advances==expected.frames && beam->areaRemovals==expected.removed;
        if (!matches) Emit("{\"step\":%d,\"op\":\"beam-native-mismatch\",\"index\":%d,\"state\":[%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld],\"life\":%u,\"segments\":%u,\"frames\":%d,\"removed\":%d}",
            nStep,i,beam->m_pos.x,beam->m_pos.y,beam->Exact().x,beam->Exact().y,beam->Delta().x,beam->Delta().y,
            beam->HeightSlope(),beam->Tick(),beam->Life(),beam->Segments(),beam->cell->advances,beam->areaRemovals);
        check("captured-native-tick-state",matches);
    }
    beam->isolatedRemoval = FALSE;
    source->m_pos = CPoint(128,96); game->m_nTimeStop = 0;
    // Real scan: temporarily add private objects to the front list, then
    // remove them before deleting; no ordinary world tick sees these nodes.
    auto* inside = new Origin; inside->m_pos = CPoint(160,96); inside->m_pArea = area;
    objects->Add(&inside->m_id,inside,INFINITE);
    auto* outside = new Origin; outside->m_pos = CPoint(300,96); outside->m_pArea = area;
    objects->Add(&outside->m_id,outside,INFINITE);
    POSITION nodes[3] = {area->m_lVertSort.AddTail(reinterpret_cast<int*>(inside->m_id)),
        area->m_lVertSort.AddTail(reinterpret_cast<int*>(outside->m_id)), area->m_lVertSort.AddTail(reinterpret_cast<int*>(source->m_id))};
    beam->Seed(area,source->m_id); beam->Scan();
    check("polygon-strikes-inside-excludes-source",beam->strikes.size()==1
        && beam->strikes[0]==std::make_pair(inside->m_id,BYTE(0)) && beam->Seen(inside->m_id) && !beam->Seen(source->m_id));
    beam->Scan(); check("polygon-dedup",beam->strikes.size()==1);
    for (POSITION p : nodes) area->m_lVertSort.RemoveAt(p);
    inside->m_pArea = outside->m_pArea = NULL; retire(inside); retire(outside);
    beam->Seed(area,source->m_id); beam->AddEffect(new CGameEffect());
    dispatch->Fire(area,source->m_id,-1,CPoint(256,96),-999,123);
    check("beam-fire-registers-and-delivery-is-empty",beam->m_id!=-1 && beam->InitialTarget()==-1 && beam->deliveries==1
        && beam->CGameObject::m_pArea==area && handler->m_messageList.GetCount()==messages && beam->m_effectList.GetCount()==1);
    retire(beam);
    {
        BOOL dead = FALSE; auto* expiring = new Beam; expiring->dead = &dead;
        expiring->Seed(area,source->m_id); expiring->Life(0); objects->Add(&expiring->m_id,expiring,INFINITE);
        LONG id = expiring->m_id; expiring->AIUpdate(); CGameObject* stale = NULL;
        check("successful-expiry-deletes-object",dead && objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&stale,INFINITE)!=CGameObjectArray::SUCCESS);
    }
    CProjectile* decoded = CProjectile::DecodeProjectile(0x6D,NULL,0);
    check("native-factory-6D",dynamic_cast<IcewindCProjectileScorcher*>(decoded)!=NULL
        && decoded->m_projectileType==0x6C && decoded->m_fireSoundRef==CResRef("TRA_17")); delete decoded;
    // Exercise actual delivery to the loaded defender. Its point equals the
    // launch origin, making native CheckLOS's equal-point branch deterministic.
    CGameObject* actorObject = NULL; CGameObject* victimObject = NULL;
    LONG actorId = game->GetFixedOrderCharacterId(0), victimId = game->GetFixedOrderCharacterId(1);
    BYTE actorRC = objects->GetDeny(actorId,CGameObjectArray::THREAD_ASYNCH,&actorObject,INFINITE);
    BYTE victimRC = objects->GetDeny(victimId,CGameObjectArray::THREAD_ASYNCH,&victimObject,INFINITE);
    check("strike-actors-available",actorRC==CGameObjectArray::SUCCESS && victimRC==CGameObjectArray::SUCCESS);
    if (actorRC==CGameObjectArray::SUCCESS && victimRC==CGameObjectArray::SUCCESS) {
        auto* actor = static_cast<CGameSprite*>(actorObject); auto* victim = static_cast<CGameSprite*>(victimObject);
        CPoint victimPos = victim->m_pos; CPoint origin;
        CProjectile::GetStart(actorId,origin,TRUE); victim->m_pos = origin;
        auto newDirection = actor->m_nNewDirection; auto deltaDirection = actor->m_deltaDirection;
        auto skipDirection = actor->m_skipDeltaDirection; auto backwards = actor->m_walkBackwards;
        actor->m_walkBackwards = FALSE;
        SHORT expectedFacing = actor->GetDirection(victim->m_pos);
        auto* spriteBeam = new Beam; spriteBeam->Seed(area,actorId); spriteBeam->m_targetId = victimId;
        spriteBeam->AIUpdate();
        check("sprite-source-turn-and-anchor",actor->m_nNewDirection==expectedFacing && spriteBeam->m_pos==origin
            && spriteBeam->HeightSlope()==victim->m_posZ-17);
        delete spriteBeam;
        actor->m_nNewDirection = newDirection; actor->m_deltaDirection = deltaDirection;
        actor->m_skipDeltaDirection = skipDirection; actor->m_walkBackwards = backwards;
        IcewindCProjectileScorcher delivery; delivery.m_sourceId = actorId; delivery.m_pArea = area;
        for (BYTE initial : {BYTE(0),BYTE(1),BYTE(2),BYTE(255)}) {
            INT before = handler->m_messageList.GetCount();
            delivery.Strike(victimId,initial);
            INT added = handler->m_messageList.GetCount()-before;
            check("strike-subspell-queued",added>0);
            while (handler->m_messageList.GetCount()>before) {
                CMessage* message = handler->m_messageList.RemoveTail();
                auto* payload = dynamic_cast<CMessageAddEffect*>(message);
                check("strike-owned-payload-provenance",payload && payload->m_effect && message->m_sourceId==actorId && message->m_targetId==victimId
                    && payload->m_effect->m_sourceRes==CResRef("SpWi217") && payload->m_effect->m_sourceID==actorId);
                delete message;
            }
        }
        // Pin the real sub-spell resource and substitute only its typed tables.
        // The Strike -> BuildSubSpellEffects -> AddEffect message path remains
        // production code; an instant ZRCHIT record makes HP deterministic.
        CSpell damage(CResRef("ZRCHIT"));
        BOOL damageAvailable = damage.Demand() != NULL && damage.pRes->m_bParsed;
        check("damage-fixture-available",damageAvailable);
        if (damageAvailable) {
            for (BYTE initial : {BYTE(0),BYTE(1),BYTE(2),BYTE(255)}) {
                CSpell subSpell(CResRef(initial==1?"EffAS1":"EffAS2"));
                BOOL available = subSpell.Demand() != NULL && subSpell.pRes->m_bParsed;
                check("subspell-damage-fixture-available",available);
                if (available) {
                    EnterCriticalSection(&g_pChitin->m_critSectDimm);
                    EnterCriticalSection(&g_pChitin->m_critSectService);
                    auto* res = subSpell.pRes;
                    auto* oldHeader = res->m_pHeader; auto* oldAbilities = res->m_pAbilities; auto* oldEffects = res->m_pEffects;
                    SPELL_HEADER header = *damage.pRes->m_pHeader;
                    SPELL_ABILITY ability = damage.pRes->m_pAbilities[0];
                    ITEM_EFFECT effect = damage.pRes->m_pEffects[0];
                    header.abilityCount = 1; ability.minCasterLevel = 1; ability.effectCount = 1; ability.startingEffect = 0;
                    effect.effectAmount = initial==1?3:2;
                    effect.dwFlags = 0x40000000; effect.special = DWORD(-1);
                    res->m_pHeader = &header; res->m_pAbilities = &ability; res->m_pEffects = &effect;
                    auto base = victim->m_baseStats;
                    CDerivedStats derived; derived = victim->m_derivedStats;
                    CDerivedStats temp; temp = victim->m_tempStats;
                    auto colorEffects = victim->m_hasColorEffects; auto dirty = victim->field_562C;
                    auto bloodFlash = victim->m_bBloodFlashOn; auto bloodAmount = victim->m_nBloodFlashAmount;
                    auto locatorTime = victim->m_nDamageLocatorTime; auto damageTaken = victim->m_nDamageTaken;
                    auto sequence = victim->m_nSequence;
                    BYTE saves[5] = {victim->field_70F6,victim->field_70F7,victim->field_70F8,victim->field_70F9,victim->field_70FA};
                    INT timed = victim->m_timedEffectList.GetCount(), equipped = victim->m_equipedEffectList.GetCount();
                    // Isolate HP from blood/animation/wake/low-health effects.
                    victim->m_baseStats.m_hitPoints = victim->m_baseStats.m_maxHitPointsBase = 512;
                    victim->m_baseStats.m_generalState = victim->m_derivedStats.m_generalState = 0;
                    victim->m_derivedStats.m_nMaxHitPoints = 512;
                    victim->m_derivedStats.m_spellStates.reset(SPLSTATE_TORTOISE_SHELL);
                    victim->m_nSequence = CGameSprite::SEQ_SLEEP;
                    LONG hp = victim->m_baseStats.m_hitPoints; INT before = handler->m_messageList.GetCount();
                    delivery.Strike(victimId,initial);
                    INT count = handler->m_messageList.GetCount()-before;
                    check("one-deterministic-strike-effect",count==1);
                    while (handler->m_messageList.GetCount()>before) {
                        CMessage* message = handler->m_messageList.RemoveTail();
                        if (dynamic_cast<CMessageAddEffect*>(message)) message->Run();
                        delete message;
                    }
                    LONG loss = hp-victim->m_baseStats.m_hitPoints;
                    check("actual-beam-strike-hp-loss",loss==(initial==1?3:2));
                    check("instant-strike-does-not-retain-effect",victim->m_timedEffectList.GetCount()==timed
                        && victim->m_equipedEffectList.GetCount()==equipped);
                    Emit("{\"step\":%d,\"op\":\"beam-strike-damage\",\"initial\":%u,\"loss\":%ld}",nStep,initial,loss);
                    victim->m_baseStats = base; victim->m_derivedStats = derived; victim->m_tempStats = temp;
                    victim->m_hasColorEffects = colorEffects; victim->field_562C = dirty;
                    victim->m_bBloodFlashOn = bloodFlash; victim->m_nBloodFlashAmount = bloodAmount;
                    victim->m_nDamageLocatorTime = locatorTime; victim->m_nDamageTaken = damageTaken; victim->m_nSequence = sequence;
                    victim->field_70F6 = saves[0]; victim->field_70F7 = saves[1]; victim->field_70F8 = saves[2];
                    victim->field_70F9 = saves[3]; victim->field_70FA = saves[4];
                    res->m_pHeader = oldHeader; res->m_pAbilities = oldAbilities; res->m_pEffects = oldEffects;
                    LeaveCriticalSection(&g_pChitin->m_critSectService);
                    LeaveCriticalSection(&g_pChitin->m_critSectDimm);
                }
                subSpell.Release();
            }
        }
        damage.Release();
        delivery.Strike(-1,TRUE); delivery.Strike(source->m_id,TRUE);
        check("strike-invalid-and-non-ai-targets",handler->m_messageList.GetCount()==messages);
        delivery.m_sourceId = -1; delivery.Strike(victimId,TRUE);
        check("strike-missing-source",handler->m_messageList.GetCount()==messages);
        victim->m_pos = victimPos;
    }
    if (victimRC==CGameObjectArray::SUCCESS) objects->ReleaseDeny(victimId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    if (actorRC==CGameObjectArray::SUCCESS) objects->ReleaseDeny(actorId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    target->m_pArea = source->m_pArea = NULL; retire(target); retire(source);
    game->m_nTimeStop = stop; game->m_nTimeStopCaster = caster;
    check("same-tick-state-restored",game->GetWorldTimer()->m_gameTime==clock && handler->m_messageList.GetCount()==messages
        && area->m_lVertSortAdd.GetCount()==front && area->m_lVertSortBackAdd.GetCount()==back && area->m_lVertSortFlightAdd.GetCount()==flight
        && area->m_lVertSort.GetCount()==sorted);
    Emit("{\"step\":%d,\"op\":\"beamchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"beamchecks","beam lifecycle or strike mismatch");
}

#include "BeamRenderGolden.h"

void BeamRenderChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame();
    auto* area = game->GetVisibleArea();
    auto* video = static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || g_pChitin->cVideo.m_bIs3dAccelerated || !area->m_visibility.m_pMap) {
        Fail(nStep, "beamrenderchecks", "requires loaded software area"); return;
    }
    INT checks = 0, test = 0, draws = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"beam-render-case\",\"case\":%d,\"name\":\"%s\",\"passed\":false}", nStep, test, name);
    };
    auto& infinity = area->m_cInfinity;
    CRect savedView = infinity.rViewPort;
    INT savedX = infinity.nCurrentX, savedY = infinity.nCurrentY;
    auto* savedWED = infinity.pResWED;
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    std::vector<BYTE> visibility(area->m_visibility.m_pMap, area->m_visibility.m_pMap + area->m_visibility.m_nMapSize);
    CRect patch(64, 64, 416, 336);
    std::vector<BYTE> savedPixels;
    auto pixels = [&](std::vector<BYTE>* read, const std::vector<BYTE>* write, BOOL clear) -> BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes = video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8;
        INT row = patch.Width() * bytes;
        BOOL ok = bytes >= 2 && bytes <= 4 && video->m_SurfaceDesc.lpSurface != NULL;
        if (ok && read) read->resize(row * patch.Height());
        if (ok && write) ok = write->size() == static_cast<size_t>(row * patch.Height());
        if (ok) for (INT y = 0; y < patch.Height(); ++y) {
            BYTE* p = static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface) + y * video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data() + y * row, p, row);
            if (write) memcpy(p, write->data() + y * row, row);
            if (clear) memset(p, 0, row);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp = pixels(&savedPixels, NULL, FALSE);
    check("pixels-backed-up", backedUp);
    struct Beam : IcewindCProjectileScorcher {
        using IcewindCProjectileScorcher::m_nSegments;
        using IcewindCProjectileScorcher::m_deltaZ;
        using IcewindCProjectileScorcher::m_offsets;
        using IcewindCProjectileScorcher::m_offsetsInitialized;
        using IcewindCProjectileScorcher::InitializeOffsets;
        using CProjectileTravelling::m_pVidCell;
        using CProjectileTravelling::m_pShadowCell;
        using CProjectileTravelling::m_direction;
        using CProjectileTravelling::m_stepX;
        using CProjectileTravelling::m_stepY;
        INT flagsCalls = 0;
        RGBQUAD palette[256] = {};
        BOOL Defaults() {
            CSize size; m_pVidCell->GetFrameSize(0, 0, size, FALSE);
            return m_lifeSpan == 110 && m_nSegments == 0 && m_deltaZ == 0 && m_bGlow == 0
                && m_struckTargets.empty() && m_tickCount == 0 && m_strikePeriod == 3 && m_clearPeriod == 60
                && m_strikeTarget == 0 && m_useHeightOffset == 1 && m_hasShadowCell == 0 && m_dirCount == 16
                && m_velocity == size.cy * 3 / 4 && m_fireSoundRef == CResRef("TRA_M04")
                && m_callBackProjectile == -1 && m_nTargetId == -1 && field_17E.IsEmpty() && m_bHasHeight == TRUE;
        }
        void Reset() {
            m_visible = 0; m_hasShadowCell = 1; m_useHeightOffset = 0; m_dirCount = 16;
            m_bMirrorNorth = 255; m_bMirrorEast = 255; m_tinted = 1; m_mirror = 0; m_paletteSwap = 1;
            m_visualEffect.m_dwFlags = 0xFFFFFFFFu; m_visualEffect.m_bTintEnabled = TRUE;
            m_visualEffect.m_bCopyFromBack = TRUE; m_visualEffect.m_nTransValue = 254;
            flagsCalls = 0;
        }
        DWORD GetRenderFlags() override { ++flagsCalls; return 0xFFFFFFFFu; }
    };
    std::vector<CPoint> savedOffsets = Beam::m_offsets;
    BYTE savedOffsetsInitialized = Beam::m_offsetsInitialized;
    Beam beam;
    check("native-constructor-defaults", beam.Defaults());
    const CPoint offsets[16] = {CPoint(-20,0), CPoint(25,8), CPoint(19,14), CPoint(10,18),
        CPoint(0,20), CPoint(-10,18), CPoint(-19,14), CPoint(-25,8), CPoint(-27,0),
        CPoint(-25,-8), CPoint(-19,-14), CPoint(-10,-18), CPoint(0,-20), CPoint(-10,18), CPoint(-19,14), CPoint(-24,8)};
    auto offsetTable = [&]() { return beam.m_offsets.size() == 16 && std::equal(beam.m_offsets.begin(), beam.m_offsets.end(), offsets); };
    check("native-sixteen-offsets", offsetTable());
    beam.m_offsets.push_back(CPoint(123,456)); beam.m_offsetsInitialized = 2;
    beam.InitializeOffsets(); check("table-truncation-and-exact-true-guard", offsetTable());
    beam.m_offsets.clear(); beam.m_offsetsInitialized = 0;
    beam.InitializeOffsets(); check("table-growth", offsetTable());
    beam.m_offsets[0] = CPoint(123,456); beam.InitializeOffsets();
    check("initialized-table-preserved", beam.m_offsets[0] == CPoint(123,456));
    beam.m_offsetsInitialized = 0; beam.InitializeOffsets(); check("table-restored", offsetTable());
    beam.SetVidCell(CResRef("ARARROW"));
    beam.m_pShadowCell = new CVidCell(CResRef("ARARROW"), FALSE);
    for (INT i = 0; i < 256; ++i) { beam.palette[i].rgbRed = static_cast<BYTE>(64 + i % 192); beam.palette[i].rgbGreen = 80; }
    beam.m_pVidCell->SetPalette(beam.palette, 256, CVidPalette::TYPE_RESOURCE);
    beam.m_pArea = area; beam.CGameObject::m_pArea = area; beam.m_listType = CGameObject::LIST_FLIGHT;
    ProjectileRenderFrameAnchor anchor(beam.m_pVidCell);
    check("bam-fixture-valid", backedUp && anchor.frame != NULL && (anchor.header == NULL || anchor.headerFrame != NULL));
    if (backedUp && anchor.frame != NULL && (anchor.header == NULL || anchor.headerFrame != NULL)) {
        infinity.pResWED = NULL;
        CPoint origin(232,1002);
        auto reset = [&]() {
            ++test; beam.Reset(); beam.m_pos = origin; beam.m_posZ = 17; beam.m_deltaZ = 0;
            beam.m_nSegments = 5; beam.m_direction = 0;
            beam.m_posAccumX = 232 * 1024; beam.m_posAccumY = 1336 * 1024;
            beam.m_stepX = 8 * 1024; beam.m_stepY = 0;
            beam.m_pVidCell->SequenceSet(0); beam.m_pVidCell->FrameSet(0);
            beam.m_pVidCell->SetTintColor(RGB(180,80,240));
            infinity.rViewPort = CRect(80,80,400,320); infinity.nCurrentX = origin.x - 160; infinity.nCurrentY = origin.y - 120;
            memset(area->m_visibility.m_pMap, CVisibilityMap::VISIBILITY_MASK, visibility.size());
        };
        auto compare = [&](const std::vector<CPoint>& points, DWORD mask) {
            std::vector<BYTE> expected, actual;
            BOOL ok = pixels(NULL, NULL, TRUE);
            CPoint reference; CSize size;
            beam.m_pVidCell->GetCurrentCenterPoint(reference, FALSE); beam.m_pVidCell->GetCurrentFrameSize(size, FALSE);
            CRect bounds(0,0,size.cx,size.cy);
            for (const CPoint& point : points) {
                infinity.FXPrep(bounds, mask, CVIDINF_SURFACE_BACK, point, reference);
                if (infinity.FXLock(bounds, mask)) {
                    infinity.FXRender(beam.m_pVidCell, reference.x, reference.y, mask, 0);
                    infinity.FXUnlock(mask, NULL, CPoint(0,0));
                    ok = infinity.FXBltFrom(CVIDINF_SURFACE_BACK, bounds, point.x, point.y, reference.x, reference.y, mask) && ok;
                } else ok = FALSE;
            }
            ok = pixels(&expected, NULL, TRUE) && ok;
            CPoint savedPos = beam.m_pos; INT ax = beam.m_posAccumX, ay = beam.m_posAccumY;
            CProjectile* object = &beam; object->Render(NULL, video, CVIDINF_SURFACE_BACK); ++draws;
            ok = pixels(&actual, NULL, FALSE) && ok;
            check("complete-segment-pixels", ok && expected == actual);
            check("nonempty-reference", std::any_of(expected.begin(), expected.end(), [](BYTE p) { return p != 0; }) == !points.empty());
            check("ignored-family-flags-and-unchanged-state", beam.flagsCalls == 0 && beam.m_pos == savedPos
                && beam.m_posAccumX == ax && beam.m_posAccumY == ay && beam.m_pVidCell->GetTintColor() == RGB(180,80,240)
                && beam.m_pVidCell->m_nCurrentFrame == 0 && beam.m_pVidCell->m_nCurrentSequence == 0 && video->m_SurfaceDesc.lpSurface == NULL);
        };
        const DWORD masks[16] = {0x280,0x280,0x280,0x280,0x280,0x2A0,0x2A0,0x2A0,0x2A0,0x2B0,0x2B0,0x2B0,0x290,0x290,0x290,0x290};
        for (SHORT direction = 0; direction < 16; ++direction) {
            reset(); beam.m_direction = direction;
            compare({CPoint(232,985),CPoint(240,985),CPoint(248,985),CPoint(256,985),CPoint(264,985)}, masks[direction]);
        }
        for (LONG delta : {LONG(-100),LONG(-2),LONG(0),LONG(2),LONG(100)}) {
            reset(); beam.m_deltaZ = delta;
            // Independent sampled heights from native min(abs(delta), index).
            std::vector<CPoint> points = {CPoint(232,985),CPoint(240,985),CPoint(248,985),CPoint(256,985),CPoint(264,985)};
            const INT low[5] = {0,-1,-2,-3,-4}, high[5] = {0,1,2,3,4}, low2[5] = {0,-1,-2,-2,-2}, high2[5] = {0,1,2,2,2};
            for (INT i = 0; i < 5; ++i) points[i].y += delta == -100 ? high[i] : delta == -2 ? high2[i] : delta == 2 ? low2[i] : delta == 100 ? low[i] : 0;
            compare(points, 0x280);
        }
        for (BYTE count : {BYTE(0),BYTE(1),BYTE(2),BYTE(127),BYTE(128),BYTE(255)}) {
            reset(); beam.m_nSegments = count; beam.m_stepX = 0;
            compare(std::vector<CPoint>(count, CPoint(232,985)), 0x280);
        }
        reset(); beam.m_stepY = 4 * 1024;
        compare({CPoint(232,985),CPoint(240,988),CPoint(248,991),CPoint(256,994),CPoint(264,997)}, 0x280);
        reset(); beam.m_stepY = -4 * 1024;
        compare({CPoint(232,985),CPoint(240,982),CPoint(248,979),CPoint(256,976),CPoint(264,973)}, 0x280);
        reset(); beam.m_posZ = -17;
        compare({CPoint(232,1019),CPoint(240,1019),CPoint(248,1019),CPoint(256,1019),CPoint(264,1019)}, 0x280);
        reset(); beam.m_nSegments = 2; beam.m_posAccumY += 1; beam.m_stepY = -2;
        compare({CPoint(232,985),CPoint(240,984)}, 0x280);
        auto hide = [&](CPoint point) {
            SHORT tile = static_cast<SHORT>(point.y / 32 * area->m_visibility.m_nWidth + point.x / 32);
            if (tile >= 0 && tile < area->m_visibility.m_nMapSize) area->m_visibility.m_pMap[tile] = 0;
            else check("hidden-tile-valid", FALSE);
        };
        reset(); hide(CPoint(232,985)); compare({}, 0x280);
        reset(); hide(CPoint(256,985)); compare({CPoint(232,985),CPoint(240,985),CPoint(248,985)}, 0x280);
        reset(); hide(origin); compare({CPoint(232,985),CPoint(240,985),CPoint(248,985),CPoint(256,985),CPoint(264,985)}, 0x280);
        reset(); infinity.nCurrentX += 10000; compare({}, 0x280);
        reset(); infinity.nCurrentX = 235; compare({}, 0x280);
        reset(); infinity.nCurrentY = 995; compare({}, 0x280);
        reset(); beam.m_stepX = 200 * 1024; compare({CPoint(232,985)}, 0x280);
        for (LONG wrap : {LONG(32 * 65536),LONG(-32 * 65536)}) {
            reset(); beam.m_nSegments = 1; beam.m_pos.y += wrap; infinity.nCurrentY += wrap;
            compare({CPoint(232,985 + wrap)}, 0x280);
        }
        // Native captures provide the anchors/masks; they contain no C++
        // reconstruction of the renderer's arithmetic. Draw those samples
        // with the real BAM, then compare the complete virtual-render pixels.
        for (INT i = 0; i < _countof(BEAM_RENDER_INPUTS); ++i) {
            reset(); const auto& c = BEAM_RENDER_INPUTS[i];
            beam.m_pos = CPoint(c.state[0],c.state[1]); beam.m_posZ = c.state[2];
            beam.m_posAccumX = c.state[3]; beam.m_posAccumY = c.state[4];
            beam.m_stepX = c.state[5]; beam.m_stepY = c.state[6]; beam.m_deltaZ = c.state[7];
            beam.m_direction = c.direction; beam.m_nSegments = c.count;
            infinity.nCurrentX = c.view[0]; infinity.nCurrentY = c.view[1];
            std::vector<CPoint> points; DWORD mask = masks[c.direction];
            for (const auto& sample : BEAM_RENDER_SAMPLES) if (sample.input == i) {
                points.push_back(CPoint(sample.x,sample.y)); mask = sample.flags;
            }
            compare(points, mask);
        }
    }
    memcpy(area->m_visibility.m_pMap, visibility.data(), visibility.size());
    infinity.pResWED = savedWED; infinity.rViewPort = savedView; infinity.nCurrentX = savedX; infinity.nCurrentY = savedY;
    check("frame-and-demands-restored", anchor.Restore());
    check("pixels-restored", backedUp && pixels(NULL, &savedPixels, FALSE));
    Beam::m_offsets = savedOffsets; Beam::m_offsetsInitialized = savedOffsetsInitialized;
    check("static-offset-state-restored", Beam::m_offsets == savedOffsets && Beam::m_offsetsInitialized == savedOffsetsInitialized);
    check("same-tick-state-restored", game->GetWorldTimer()->m_gameTime == clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == messages
        && area->m_lVertSortAdd.GetCount() == front && area->m_lVertSortBackAdd.GetCount() == back && area->m_lVertSortFlightAdd.GetCount() == flight);
    Emit("{\"step\":%d,\"op\":\"beamrenderchecks\",\"checks\":%d,\"renderCalls\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, draws, valid ? "true" : "false");
    if (!valid) Fail(nStep, "beamrenderchecks", "beam renderer or construction mismatch (state restored)");
}

void ExplodingRenderChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame();
    auto* area = game->GetVisibleArea();
    auto* video = static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || g_pChitin->cVideo.m_bIs3dAccelerated || !area->m_visibility.m_pMap
        || !area->m_search.m_pDynamicCost) {
        Fail(nStep, "explodingrenderchecks", "requires loaded software area"); return;
    }
    INT checks = 0, test = -1; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"exploding-render-case\",\"case\":%d,\"name\":\"%s\",\"passed\":false}",
            nStep, test, name);
    };
    auto& infinity = area->m_cInfinity;
    CRect savedView = infinity.rViewPort;
    INT savedX = infinity.nCurrentX, savedY = infinity.nCurrentY;
    auto* savedWED = infinity.pResWED;
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount();
    INT flight = area->m_lVertSortFlightAdd.GetCount();
    CRect patch(64, 64, 416, 336);
    std::vector<BYTE> savedPixels;
    auto pixels = [&](std::vector<BYTE>* read, const std::vector<BYTE>* write, BOOL clear) -> BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes = video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8;
        INT row = patch.Width() * bytes;
        BOOL ok = bytes >= 2 && bytes <= 4 && video->m_SurfaceDesc.lpSurface != NULL;
        if (ok && read) read->resize(row * patch.Height());
        if (ok && write) ok = write->size() == static_cast<size_t>(row * patch.Height());
        if (ok) for (INT y = 0; y < patch.Height(); ++y) {
            BYTE* p = static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface) + y * video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data() + y * row, p, row);
            if (write) memcpy(p, write->data() + y * row, row);
            if (clear) memset(p, 0, row);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp = pixels(&savedPixels, NULL, FALSE);
    check("back-pixels-backed-up", backedUp);
    struct Missile : CProjectileExploding {
        using CProjectileTravelling::GetCellBounds;
        using CProjectileTravelling::m_pVidCell;
        using CProjectileTravelling::m_pShadowCell;
        using CProjectileTravelling::m_visible;
        using CProjectileTravelling::m_tinted;
        using CProjectileTravelling::m_mirror;
        using CProjectileTravelling::m_hasShadowCell;
        using CProjectileTravelling::m_direction;
        using CProjectileTravelling::m_useHeightOffset;
        using CProjectileTravelling::m_paletteSwap;
        using CProjectileExploding::m_nState;
        using CProjectileExploding::m_bExplodeCell1Active;
        using CProjectileExploding::m_bExplodeCell2Active;
        using CProjectileExploding::m_explodeCell1;
        using CProjectileExploding::m_explodeCell2;
        INT flagsCalls = 0;
        RGBQUAD palettes[3][256] = {};
        Missile() : CProjectileExploding(CResRef("ARARROW")) {
            m_explodeCell1.SetResRef(CResRef("ARARROW"), FALSE, TRUE, FALSE);
            m_explodeCell2.SetResRef(CResRef("ARARROW"), FALSE, TRUE, FALSE);
            // The un-clipped FX blitter requires matching cell extents. Native
            // Skull Trap also supplies a travel shadow when it carries Z.
            m_pShadowCell = new CVidCell(CResRef("ARARROW"), FALSE);
            m_pShadowCell->SequenceSet(0); m_pShadowCell->FrameSet(0);
            m_pVidCell->SequenceSet(0); m_pVidCell->FrameSet(0);
            m_explodeCell1.SequenceSet(0); m_explodeCell1.FrameSet(0);
            m_explodeCell2.SequenceSet(0); m_explodeCell2.FrameSet(0);
            for (INT i = 0; i < 256; ++i) {
                BYTE intensity = static_cast<BYTE>(64 + i % 192);
                palettes[0][i].rgbRed = intensity;
                palettes[1][i].rgbBlue = intensity;
                palettes[2][i].rgbGreen = intensity;
            }
            // Distinct owned palettes make overdraw order observable even
            // when the native pass has no tint flag.
            m_pVidCell->SetPalette(palettes[0], 256, CVidPalette::TYPE_RESOURCE);
            m_explodeCell1.SetPalette(palettes[1], 256, CVidPalette::TYPE_RESOURCE);
            m_explodeCell2.SetPalette(palettes[2], 256, CVidPalette::TYPE_RESOURCE);
            m_dirCount = 1; m_renderFlags = 0;
            m_hasShadowCell = 0; m_useHeightOffset = 0; m_paletteSwap = 0;
            CSize size;
            m_pVidCell->GetCurrentFrameSize(size, FALSE);
            // Symmetric mirror bounds enclose both the lifted cell and its
            // unlifted secondary for either sign of the test height.
            m_mirrorMinX = size.cx;
            m_mirrorMinY = size.cy + 17;
        }
        DWORD GetRenderFlags() override { ++flagsCalls; return CProjectileTravelling::GetRenderFlags(); }
    } missile;
    ProjectileRenderFrameAnchor anchor(missile.m_pVidCell);
    if (anchor.frame == NULL || (anchor.header != NULL && anchor.headerFrame == NULL)) {
        Fail(nStep, "explodingrenderchecks", "BAM fixture frame unavailable"); return;
    }
    missile.m_pArea = area; missile.CGameObject::m_pArea = area;
    missile.m_listType = CGameObject::LIST_FLIGHT;
    CPoint origin(232, 1002);
    SHORT tile = static_cast<SHORT>(origin.y / 32 * area->m_visibility.m_nWidth + origin.x / 32);
    INT cost = (origin.y / 12) * area->m_search.m_GridSquareDimensions.cx + origin.x / 16;
    if (tile < 0 || tile >= area->m_visibility.m_nMapSize || cost < 0
        || cost >= area->m_search.m_GridSquareDimensions.cx * area->m_search.m_GridSquareDimensions.cy) {
        check("fixture-coordinates-in-map", FALSE);
    } else if (backedUp) {
        BYTE visible = area->m_visibility.m_pMap[tile], dynamic = area->m_search.m_pDynamicCost[cost];
        // Isolate BAM geometry from unrelated loaded-area wall polygons. The
        // production clipping call still executes and returns on NULL WED.
        infinity.pResWED = NULL;
        auto reset = [&]() {
            missile.m_pos = origin; missile.m_posZ = 17;
            missile.m_nState = 1; missile.m_bExplodeCell1Active = 1; missile.m_bExplodeCell2Active = 0;
            missile.m_visible = 0; missile.m_tinted = 0; missile.m_mirror = 0;
            missile.m_hasShadowCell = 0;
            missile.m_direction = 0; missile.flagsCalls = 0; missile.m_paletteSwap = 0;
            missile.m_pVidCell->SetTintColor(RGB(240, 180, 120));
            missile.m_explodeCell1.SetTintColor(RGB(80, 180, 240));
            missile.m_explodeCell2.SetTintColor(RGB(230, 60, 100));
            infinity.rViewPort = CRect(80, 80, 400, 320);
            infinity.nCurrentX = origin.x - 160; infinity.nCurrentY = origin.y - 120;
            area->m_visibility.m_pMap[tile] = CVisibilityMap::VISIBILITY_MASK;
            area->m_search.m_pDynamicCost[cost] = 0;
        };
        // This reference paints the specified cells at explicit anchors using
        // the existing video pipeline. It contains no visibility/state/direction
        // decision tree from the recovered renderer.
        auto paint = [&](BOOL travel, BOOL explosion, BOOL second, DWORD flags, LONG y, LONG z, BOOL flipRef) {
            if (!travel && !explosion) return TRUE;
            CRect bounds; CPoint reference;
            missile.GetCellBounds(bounds, reference, NULL);
            if (flipRef) reference.y = bounds.Height() - reference.y;
            CPoint cellCenter;
            CSize cellSize;
            missile.m_explodeCell2.GetCurrentCenterPoint(cellCenter, FALSE);
            missile.m_explodeCell2.GetCurrentFrameSize(cellSize, FALSE);
            Emit("{\"step\":%d,\"op\":\"exploding-render-reference\",\"case\":%d,\"flags\":%lu,\"bounds\":[%ld,%ld],\"ref\":[%ld,%ld],\"center\":[%ld,%ld],\"size\":[%ld,%ld],\"z\":%ld}",
                nStep, test, flags, bounds.Width(), bounds.Height(), reference.x, reference.y,
                cellCenter.x, cellCenter.y, cellSize.cx, cellSize.cy, z);
            // FXRender writes a complete BAM frame without clipping. Reject
            // invalid synthetic anchors before touching a video surface.
            if (second && (reference.x < cellCenter.x || reference.y < cellCenter.y
                || reference.x - cellCenter.x + cellSize.cx > bounds.Width()
                || reference.y - cellCenter.y + cellSize.cy > bounds.Height())) return FALSE;
            CVidCell* primary = travel ? missile.m_pVidCell : &missile.m_explodeCell1;
            primary->GetCurrentCenterPoint(cellCenter, FALSE);
            primary->GetCurrentFrameSize(cellSize, FALSE);
            if (reference.x < cellCenter.x || reference.y - z < cellCenter.y
                || reference.x - cellCenter.x + cellSize.cx > bounds.Width()
                || reference.y - z - cellCenter.y + cellSize.cy > bounds.Height()) return FALSE;
            CPoint point(missile.m_pos.x, y);
            infinity.FXPrep(bounds, CInfinity::FXPREP_COPYFROMBACK | flags, CVIDINF_SURFACE_BACK, point, reference);
            if (!infinity.FXLock(bounds, flags)) return FALSE;
            if (second) infinity.FXRender(&missile.m_explodeCell2, reference.x, reference.y, flags, 0);
            infinity.FXRender(travel ? missile.m_pVidCell : &missile.m_explodeCell1,
                reference.x, reference.y - z, flags, 0x80);
            infinity.FXUnlock(flags, NULL, CPoint(0, 0));
            return infinity.FXBltFrom(CVIDINF_SURFACE_BACK, bounds, point.x, point.y, reference.x, reference.y, flags);
        };
        auto compare = [&](BOOL travel, BOOL explosion, BOOL second, DWORD flags, LONG y, LONG z,
                           BOOL flipRef, BOOL requireDraw, INT expectedCalls) {
            std::vector<BYTE> expected, actual;
            BOOL ok = pixels(NULL, NULL, TRUE);
            if (travel) ok = paint(TRUE, FALSE, FALSE, CInfinity::FXPREP_COPYFROMBACK | flags, y, z, flipRef) && ok;
            if (explosion) ok = paint(FALSE, TRUE, second, flags, y, z, flipRef) && ok;
            if (!ok) { check("reference-geometry-valid", FALSE); return; }
            ok = pixels(&expected, NULL, TRUE) && ok;
            if (missile.m_tinted != 0) missile.m_explodeCell1.SetTintColor(RGB(1, 2, 3));
            CProjectile* object = &missile;
            object->Render(NULL, video, CVIDINF_SURFACE_BACK);
            ok = pixels(&actual, NULL, FALSE) && ok;
            check("complete-clipped-pixels", ok && actual == expected);
            check("expected-visible-draw", std::any_of(expected.begin(), expected.end(), [](BYTE x) { return x != 0; }) == (requireDraw != FALSE));
            check("virtual-flags-call-count", missile.flagsCalls == expectedCalls);
            check("frames-and-surfaces-balanced", missile.m_pVidCell->m_nCurrentFrame == 0
                && missile.m_explodeCell1.m_nCurrentFrame == 0 && missile.m_explodeCell2.m_nCurrentFrame == 0
                && video->m_SurfaceDesc.lpSurface == NULL);
        };
        for (test = 0; test < 6; ++test) {
            reset();
            if (test == 0 || test == 1) missile.m_nState = 0;
            if (test == 1 || test == 3 || test == 5) missile.m_visible = 1;
            if (test == 2 || test == 3) missile.m_bExplodeCell1Active = 0;
            if (test == 2) missile.m_bExplodeCell2Active = 1; // Cell 2 alone never draws.
            compare(missile.m_visible != 0, test >= 4, FALSE, 0, origin.y - 17, 0, FALSE,
                missile.m_visible != 0 || test >= 4, test >= 4 ? 2 : 1);
        }
        for (test = 6; test < 22; ++test) {
            reset(); missile.m_direction = static_cast<SHORT>(test - 6);
            // Native directional thresholds: N=8, W=4, E=12. The plain-cell
            // path retains its reference even when its vertical blit is flipped.
            DWORD flags = test > 14 ? CInfinity::MIRROR_FX : 0;
            if (test > 10 && test < 18) flags |= CInfinity::MIRROR_FX_UPDOWN;
            compare(FALSE, TRUE, FALSE, flags, origin.y - 17, 0, FALSE, TRUE, 2);
        }
        for (test = 22; test < 30; ++test) {
            reset(); missile.m_bExplodeCell2Active = 1; missile.m_hasShadowCell = 1;
            missile.m_mirror = (test & 1) ? 1 : 0;
            missile.m_direction = test & 2 ? 8 : 0;
            missile.m_posZ = test & 4 ? -17 : 17;
            BOOL flipped = (test & 2) != 0;
            DWORD flags = 4 | (missile.m_mirror ? 0x200 : 0) | (flipped ? CInfinity::MIRROR_FX_UPDOWN : 0);
            compare(FALSE, TRUE, TRUE, flags, origin.y, flipped ? -missile.m_posZ : missile.m_posZ, flipped, TRUE, 2);
            check("secondary-tint-untouched", missile.m_explodeCell2.GetTintColor() == RGB(230, 60, 100));
        }
        for (test = 30; test < 34; ++test) {
            reset();
            if (test == 30) area->m_visibility.m_pMap[tile] = 0;
            if (test == 31) area->m_search.m_pDynamicCost[cost] = 0x80;
            if (test == 32) infinity.nCurrentX += 10000;
            if (test == 33) infinity.nCurrentY += 10000;
            compare(FALSE, FALSE, FALSE, 0, origin.y - 17, 0, FALSE, FALSE, 2);
        }
        for (test = 34; test < 38; ++test) {
            reset(); missile.m_tinted = 1; missile.m_bExplodeCell2Active = test & 1;
            missile.m_hasShadowCell = test & 1;
            missile.m_nState = (test & 2) ? -1 : 1; // Any nonzero linger state.
            CPoint tintPos(origin.x, origin.y - (test & 1 ? 0 : 17));
            COLORREF tint = area->GetTintColor(tintPos, CGameObject::LIST_FLIGHT);
            missile.m_explodeCell1.SetTintColor(tint);
            compare(FALSE, TRUE, (test & 1) != 0, 0x10000 | (test & 1 ? 4 : 0), tintPos.y,
                test & 1 ? 17 : 0, FALSE, TRUE, 2);
            check("first-cell-receives-area-tint", missile.m_explodeCell1.GetTintColor() == tint);
            check("travel-and-secondary-tints-untouched", missile.m_pVidCell->GetTintColor() == RGB(240, 180, 120)
                && missile.m_explodeCell2.GetTintColor() == RGB(230, 60, 100));
        }
        for (test = 38; test < 40; ++test) {
            reset(); missile.m_pos.y += test == 38 ? 32 * 65536 : -32 * 65536;
            infinity.nCurrentY += test == 38 ? 32 * 65536 : -32 * 65536;
            compare(FALSE, TRUE, FALSE, 0, missile.m_pos.y - 17, 0, FALSE, TRUE, 2);
        }
        for (test = 40; test < 44; ++test) {
            reset(); missile.m_mirror = 1; missile.m_direction = test & 1 ? 8 : 0;
            missile.m_posZ = test & 2 ? -17 : 17;
            BOOL flipped = (test & 1) != 0;
            compare(FALSE, TRUE, FALSE, 0x200 | (flipped ? CInfinity::MIRROR_FX_UPDOWN : 0),
                origin.y, flipped ? -missile.m_posZ : missile.m_posZ, flipped, TRUE, 2);
        }
        test = 44; reset(); missile.m_posZ = 0; missile.m_bExplodeCell2Active = 1;
        // With zero height, the identical frame extents also safely exercise
        // the independent explosion-shadow flag without a travel shadow.
        compare(FALSE, TRUE, TRUE, 4, origin.y, 0, FALSE, TRUE, 2);
        for (test = 45; test < 47; ++test) {
            reset(); missile.m_nState = 0; missile.m_visible = 1;
            missile.m_pos.y += test == 45 ? 32 * 65536 : -32 * 65536;
            infinity.nCurrentY += test == 45 ? 32 * 65536 : -32 * 65536;
            // The travelling base must draw at wrapped tile indices too;
            // an invisible flight cell would not expose a widened-index bug.
            compare(TRUE, FALSE, FALSE, 0, missile.m_pos.y - 17, 0, FALSE, TRUE, 1);
        }
        area->m_visibility.m_pMap[tile] = visible; area->m_search.m_pDynamicCost[cost] = dynamic;
    }
    infinity.pResWED = savedWED; infinity.rViewPort = savedView;
    infinity.nCurrentX = savedX; infinity.nCurrentY = savedY;
    check("bam-frame-and-demand-restored", anchor.Restore());
    check("sampled-pixels-restored", backedUp && pixels(NULL, &savedPixels, FALSE));
    check("same-tick-state-restored", infinity.pResWED == savedWED && infinity.rViewPort == savedView
        && infinity.nCurrentX == savedX && infinity.nCurrentY == savedY
        && game->GetWorldTimer()->m_gameTime == clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == messages
        && area->m_lVertSortAdd.GetCount() == front && area->m_lVertSortBackAdd.GetCount() == back
        && area->m_lVertSortFlightAdd.GetCount() == flight);
    Emit("{\"step\":%d,\"op\":\"explodingrenderchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}",
        nStep, checks, valid ? "true" : "false");
    if (!valid) Fail(nStep, "explodingrenderchecks", "explosive render mismatch (state restored)");
}

void VfxRenderChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame();
    auto* area = game->GetVisibleArea();
    auto* video = static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || g_pChitin->cVideo.m_bIs3dAccelerated || !area->m_visibility.m_pMap
        || !area->m_search.m_pDynamicCost) {
        Fail(nStep, "vfxrenderchecks", "requires loaded software area"); return;
    }
    INT checks = 0, test = 0, draws = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"vfx-render-case\",\"case\":%d,\"name\":\"%s\",\"passed\":false}", nStep, test, name);
    };
    auto& infinity = area->m_cInfinity;
    CRect savedView = infinity.rViewPort;
    INT savedX = infinity.nCurrentX, savedY = infinity.nCurrentY;
    auto* savedWED = infinity.pResWED;
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    INT messages = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    CRect patch(64, 64, 416, 336);
    std::vector<BYTE> savedPixels;
    auto pixels = [&](std::vector<BYTE>* read, const std::vector<BYTE>* write, BOOL clear) -> BOOL {
        if (!video->BKLock(patch)) return FALSE;
        DWORD bytes = video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8;
        INT row = patch.Width() * bytes;
        BOOL ok = bytes >= 2 && bytes <= 4 && video->m_SurfaceDesc.lpSurface != NULL;
        if (ok && read) read->resize(row * patch.Height());
        if (ok && write) ok = write->size() == static_cast<size_t>(row * patch.Height());
        if (ok) for (INT y = 0; y < patch.Height(); ++y) {
            BYTE* p = static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface) + y * video->m_SurfaceDesc.lPitch;
            if (read) memcpy(read->data() + y * row, p, row);
            if (write) memcpy(p, write->data() + y * row, row);
            if (clear) memset(p, 0, row);
        }
        video->BKUnlock(); return ok;
    };
    BOOL backedUp = pixels(&savedPixels, NULL, FALSE);
    check("back-pixels-backed-up", backedUp);
    struct Missile : IcewindCProjectileTravellingVFX {
        using IcewindCProjectileTravellingVFX::GetCellBounds;
        using IcewindCProjectileTravellingVFX::UpdateDirectionSequence;
        using IcewindCProjectileTravellingVFX::m_visualEffect;
        using IcewindCProjectileTravellingVFX::m_bMirrorNorth;
        using IcewindCProjectileTravellingVFX::m_bMirrorEast;
        using CProjectileTravelling::m_pVidCell;
        using CProjectileTravelling::m_pShadowCell;
        using CProjectileTravelling::m_hasShadowCell;
        using CProjectileTravelling::m_direction;
        using CProjectileTravelling::m_facing;
        using CProjectileTravelling::m_dirCount;
        using CProjectileTravelling::m_visible;
        using CProjectileTravelling::m_useHeightOffset;
        using CProjectileTravelling::m_paletteSwap;
        using CProjectileTravelling::m_pTravelPaletteRes;
        INT flagsCalls = 0;
        RGBQUAD palettes[2][256] = {};
        Missile() : IcewindCProjectileTravellingVFX(CResRef("ARARROW")) {
            m_pShadowCell = new CVidCell(CResRef("ARARROW"), FALSE);
            for (INT i = 0; i < 256; ++i) {
                BYTE intensity = static_cast<BYTE>(64 + i % 192);
                palettes[0][i].rgbRed = intensity; palettes[1][i].rgbGreen = intensity;
            }
            m_pVidCell->SetPalette(palettes[0], 256, CVidPalette::TYPE_RESOURCE);
            m_pShadowCell->SetPalette(palettes[1], 256, CVidPalette::TYPE_RESOURCE);
            // These old-family fields must not supply VFX flags or bounds.
            m_renderFlags = 8; m_tinted = 1; m_mirror = 1;
            m_mirrorMinX = 80; m_mirrorMinY = 80;
            m_pTravelPaletteRes = NULL; m_travelPaletteRef = CResRef(""); m_travelPaletteRequested = FALSE;
            SetTravelPalette("STTRAVL1");
        }
        DWORD GetRenderFlags() override { ++flagsCalls; return 0xFFFFFFFFu; }
    } missile;
    missile.m_pArea = area; missile.CGameObject::m_pArea = area; missile.m_listType = CGameObject::LIST_FLIGHT;
    CResBitmap* palette = missile.m_pTravelPaletteRes;
    if (palette == NULL) { Fail(nStep, "vfxrenderchecks", "travel palette unavailable"); return; }
    INT paletteDemands = palette->GetDemands();
    if (palette->Demand() == NULL) {
        palette->Release(); Fail(nStep, "vfxrenderchecks", "travel palette demand failed"); return;
    }
    ProjectileRenderFrameAnchor anchor(missile.m_pVidCell);
    CPoint origin(232, 1002);
    SHORT tile = static_cast<SHORT>(origin.y / 32 * area->m_visibility.m_nWidth + origin.x / 32);
    INT cost = origin.y / 12 * area->m_search.m_GridSquareDimensions.cx + origin.x / 16;
    BOOL available = backedUp && anchor.frame != NULL && (anchor.header == NULL || anchor.headerFrame != NULL)
        && tile >= 0 && tile < area->m_visibility.m_nMapSize && cost >= 0
        && cost < area->m_search.m_GridSquareDimensions.cx * area->m_search.m_GridSquareDimensions.cy;
    check("fixture-inputs-valid", available);
    if (available) {
        BYTE visible = area->m_visibility.m_pMap[tile], dynamic = area->m_search.m_pDynamicCost[cost];
        infinity.pResWED = NULL;
        auto reset = [&]() {
            ++test;
            missile.m_pos = origin; missile.m_posZ = 17; missile.m_visible = 1;
            missile.m_dirCount = 1; missile.m_direction = 0; missile.m_facing = 0;
            missile.m_hasShadowCell = 0; missile.m_useHeightOffset = 0; missile.m_bHasHeight = FALSE;
            missile.m_bMirrorNorth = 0; missile.m_bMirrorEast = 0; missile.flagsCalls = 0;
            missile.m_visualEffect = IcewindCVisualEffect(); missile.m_visualEffect.SetCopyFromBack(FALSE);
            missile.m_paletteSwap = 0;
            missile.m_pVidCell->SequenceSet(0); missile.m_pVidCell->FrameSet(0);
            missile.m_pShadowCell->SequenceSet(0); missile.m_pShadowCell->FrameSet(0);
            missile.m_pVidCell->SetPalette(missile.palettes[0], 256, CVidPalette::TYPE_RESOURCE);
            missile.m_pShadowCell->SetPalette(missile.palettes[1], 256, CVidPalette::TYPE_RESOURCE);
            missile.m_pVidCell->SetTintColor(RGB(180, 80, 240)); missile.m_pShadowCell->SetTintColor(RGB(230, 60, 100));
            infinity.rViewPort = CRect(80, 80, 400, 320);
            infinity.nCurrentX = origin.x - 160; infinity.nCurrentY = origin.y - 120;
            area->m_visibility.m_pMap[tile] = CVisibilityMap::VISIBILITY_MASK; area->m_search.m_pDynamicCost[cost] = 0;
        };
        auto compare = [&](BOOL draw, DWORD flags, LONG y, LONG z, BOOL flipRef, SHORT sequence, SHORT facing) {
            std::vector<BYTE> expected, actual;
            BOOL ok = pixels(NULL, NULL, TRUE);
            SHORT oldSequence = missile.m_pVidCell->m_nCurrentSequence;
            COLORREF oldTint = missile.m_pVidCell->GetTintColor();
            if (draw) {
                missile.m_pVidCell->SequenceSet(sequence);
                CRect bounds; CPoint reference;
                missile.GetCellBounds(bounds, reference, NULL);
                if (flipRef) reference.y = bounds.Height() - reference.y;
                auto fits = [&](CVidCell* cell, LONG height) {
                    CPoint center; CSize size;
                    cell->GetCurrentCenterPoint(center, FALSE); cell->GetCurrentFrameSize(size, FALSE);
                    return reference.x >= center.x && reference.y - height >= center.y
                        && reference.x - center.x + size.cx <= bounds.Width()
                        && reference.y - height - center.y + size.cy <= bounds.Height();
                };
                BOOL geometry = fits(missile.m_pVidCell, z)
                    && (!missile.m_hasShadowCell || fits(missile.m_pShadowCell, 0));
                check("reference-anchors-in-bounds", geometry);
                if (!geometry) { missile.m_pVidCell->SequenceSet(oldSequence); return; }
                CPoint point(missile.m_pos.x, y);
                if (missile.m_visualEffect.m_bTintEnabled == TRUE)
                    missile.m_pVidCell->SetTintColor(area->GetTintColor(point, missile.m_listType));
                if (missile.m_paletteSwap) missile.m_pVidCell->SetPalette(palette->GetColorTable(), palette->GetColorCount(), CVidPalette::TYPE_RESOURCE);
                infinity.FXPrep(bounds, CInfinity::FXPREP_COPYFROMBACK | flags, CVIDINF_SURFACE_BACK, point, reference);
                if (infinity.FXLock(bounds, flags)) {
                    if (missile.m_hasShadowCell) infinity.FXRender(missile.m_pShadowCell, reference.x, reference.y, flags, 0);
                    infinity.FXRender(missile.m_pVidCell, reference.x, reference.y - z, flags, missile.m_visualEffect.m_nTransValue);
                    infinity.FXUnlock(flags, NULL, CPoint(0, 0));
                    ok = infinity.FXBltFrom(CVIDINF_SURFACE_BACK, bounds, point.x, point.y, reference.x, reference.y, flags) && ok;
                } else ok = FALSE;
            }
            COLORREF expectedTint = missile.m_pVidCell->GetTintColor();
            // Real BAM drawing normalizes unavailable sequences to zero.
            // The separate helper checks below assert the raw selection and
            // frame preservation before this resource-dependent draw step.
            SHORT expectedSequence = missile.m_pVidCell->m_nCurrentSequence;
            ok = pixels(&expected, NULL, TRUE) && ok;
            missile.m_pVidCell->SequenceSet(oldSequence); missile.m_pVidCell->SetTintColor(oldTint);
            if (missile.m_paletteSwap) missile.m_pVidCell->SetPalette(missile.palettes[0], 256, CVidPalette::TYPE_RESOURCE);
            CProjectile* object = &missile; object->Render(NULL, video, CVIDINF_SURFACE_BACK); ++draws;
            ok = pixels(&actual, NULL, FALSE) && ok;
            check("complete-pixels", ok && expected == actual);
            check("expected-visible-draw", std::any_of(expected.begin(), expected.end(), [](BYTE x) { return x != 0; }) == (draw != FALSE));
            check("independent-flags-and-tints", missile.flagsCalls == 0 && missile.m_pVidCell->GetTintColor() == expectedTint
                && missile.m_pShadowCell->GetTintColor() == RGB(230, 60, 100));
            check("sequence-facing-frame-and-lock", missile.m_pVidCell->m_nCurrentSequence == expectedSequence && missile.m_direction == facing
                && missile.m_pVidCell->m_nCurrentFrame == 0 && missile.m_pShadowCell->m_nCurrentFrame == 0
                && video->m_SurfaceDesc.lpSurface == NULL);
        };
        // Explicit outcomes: [initial horizontal bit][east/north == 1][facing].
        static const BYTE masks[2][4][16] = {
            {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
             {0,0,0,0,0,0,0,0,0,16,16,16,16,16,16,16},
             {0,0,0,0,0,16,16,16,16,16,16,16,0,0,0,0},
             {0,0,0,0,0,16,16,16,16,0,0,0,16,16,16,16}},
            {{16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16},
             {16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16},
             {16,16,16,16,16,0,0,0,0,0,0,0,16,16,16,16},
             {16,16,16,16,16,0,0,0,0,0,0,0,16,16,16,16}}
        };
        for (INT seed = 0; seed < 2; ++seed) for (BYTE north : {BYTE(0), BYTE(1), BYTE(2), BYTE(255)})
            for (BYTE east : {BYTE(0), BYTE(1), BYTE(2), BYTE(255)}) for (SHORT direction = 0; direction < 16; ++direction) {
                reset(); missile.m_bMirrorNorth = north; missile.m_bMirrorEast = east; missile.m_direction = direction;
                missile.m_visualEffect.m_dwFlags |= seed ? CInfinity::MIRROR_FX : 0;
                INT row = (east == 1 ? 1 : 0) + (north == 1 ? 2 : 0);
                compare(TRUE, 0x20000 | masks[seed][row][direction], origin.y - 17, 0, FALSE, 0, direction);
            }
        for (SHORT direction : {SHORT(0), SHORT(8), SHORT(12), SHORT(15)}) for (LONG height : {LONG(-17), LONG(0), LONG(17)}) {
            reset(); missile.m_direction = direction; missile.m_bMirrorNorth = 1; missile.m_bMirrorEast = 1;
            missile.m_posZ = height; missile.m_hasShadowCell = 1;
            BOOL flipped = direction == 8;
            compare(TRUE, 0x20004 | masks[0][3][direction], origin.y, flipped ? -height : height, flipped, 0, direction);
        }
        for (BYTE tint : {BYTE(0), BYTE(1), BYTE(2), BYTE(255)}) for (BYTE copy : {BYTE(0), BYTE(1), BYTE(2), BYTE(255)}) {
            reset(); missile.m_visualEffect.m_bTintEnabled = tint; missile.m_visualEffect.m_bCopyFromBack = copy;
            // The mask and the Boolean are independent native inputs.
            missile.m_visualEffect.m_dwFlags |= 0x10000;
            compare(TRUE, 0x30000, origin.y - 17, 0, FALSE, 0, 0);
        }
        for (INT transparency : {0, 64, 128, 254}) {
            reset(); missile.m_visualEffect.SetTransparency(TRUE, transparency);
            compare(TRUE, 0x20002, origin.y - 17, 0, FALSE, 0, 0);
            reset(); missile.m_visualEffect.SetCopyFromBack(TRUE);
            compare(TRUE, 0x20200, origin.y - 17, 0, FALSE, 0, 0);
        }
        for (INT mode = 0; mode < 7; ++mode) {
            reset();
            if (mode == 0) missile.m_visible = 0;
            if (mode == 1) area->m_visibility.m_pMap[tile] = 0;
            if (mode == 2) area->m_search.m_pDynamicCost[cost] = 0x80;
            if (mode == 3 || mode == 4) { infinity.nCurrentX += mode == 3 ? 10000 : 0; infinity.nCurrentY += mode == 4 ? 10000 : 0; }
            if (mode >= 5) { missile.m_pos.y += mode == 5 ? 32 * 65536 : -32 * 65536; infinity.nCurrentY += mode == 5 ? 32 * 65536 : -32 * 65536; }
            compare(mode >= 5, 0x20000, missile.m_pos.y - 17, 0, FALSE, 0, 0);
        }
        reset(); missile.m_paletteSwap = 1;
        compare(TRUE, 0x20000, origin.y - 17, 0, FALSE, 0, 0);
        static const SHORT sequences[4][16] = {
            {0,1,2,3,4,5,6,7,8,7,6,5,4,3,2,1}, {0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1},
            {0,0,2,2,4,4,2,2,0,0,2,2,4,4,2,2}, {0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1}
        };
        for (BYTE north : {BYTE(0), BYTE(2)}) for (SHORT facing = 0; facing < 16; ++facing) {
            reset(); missile.m_bMirrorNorth = north; missile.m_dirCount = 16; missile.m_direction = -1; missile.m_facing = facing;
            compare(TRUE, 0x20000, origin.y - 17, 0, FALSE, sequences[north != 0][facing], facing);
        }
        for (SHORT count : {SHORT(2), SHORT(8), SHORT(16), SHORT(3), SHORT(1)}) for (BOOL other : {FALSE, TRUE})
            for (BYTE north : {BYTE(0), BYTE(2)}) for (SHORT facing = 0; facing < 16; ++facing) {
                reset(); missile.m_bMirrorNorth = north; missile.m_dirCount = count; missile.m_direction = -1; missile.m_facing = facing;
                CVidCell* selected = other ? missile.m_pShadowCell : missile.m_pVidCell;
                selected->SequenceSet(6); selected->FrameSet(3);
                missile.UpdateDirectionSequence(other ? selected : NULL);
                INT row = count == 2 ? 3 : count == 8 ? 2 : north != 0 ? 1 : 0;
                SHORT expected = count == 1 || count == 3 ? 6 : sequences[row][facing];
                check("direction-sequence-table", selected->m_nCurrentSequence == expected && selected->m_nCurrentFrame == 3
                    && missile.m_direction == (other || count == 1 ? -1 : facing));
                missile.m_direction = facing; selected->SequenceSet(6);
                missile.UpdateDirectionSequence(selected);
                check("unchanged-facing-skips-alternate", selected->m_nCurrentSequence == 6 && selected->m_nCurrentFrame == 3);
            }
        for (LONG height : {LONG(-17), LONG(0), LONG(17)}) for (BOOL shadow : {FALSE, TRUE}) for (BOOL other : {FALSE, TRUE}) {
            reset(); missile.m_posZ = height; missile.m_hasShadowCell = shadow;
            missile.m_pShadowCell->SequenceSet(4);
            CVidCell* selected = other ? missile.m_pShadowCell : missile.m_pVidCell;
            CPoint center; CSize size;
            selected->GetCurrentCenterPoint(center, FALSE); selected->GetCurrentFrameSize(size, FALSE);
            CRect expected(0, 0, size.cx, size.cy); CPoint ref = center;
            if (shadow) {
                CRect mainRect(-center.x, -center.y - height, size.cx - center.x, size.cy - center.y - height);
                missile.m_pShadowCell->GetCurrentCenterPoint(center, FALSE); missile.m_pShadowCell->GetCurrentFrameSize(size, FALSE);
                CRect shadowRect(-center.x, -center.y, size.cx - center.x, size.cy - center.y);
                UnionRect(&expected, &mainRect, &shadowRect);
                ref = CPoint(-expected.left, -expected.top); expected.OffsetRect(ref);
            }
            CRect actual; CPoint actualRef;
            missile.GetCellBounds(actual, actualRef, other ? selected : NULL);
            check("bounds-equal-independent-rectangle-union", actual == expected && actualRef == ref);
        }
        area->m_visibility.m_pMap[tile] = visible; area->m_search.m_pDynamicCost[cost] = dynamic;
    }
    infinity.pResWED = savedWED; infinity.rViewPort = savedView; infinity.nCurrentX = savedX; infinity.nCurrentY = savedY;
    check("bam-frame-and-demand-restored", anchor.Restore());
    palette->Release(); check("palette-demand-balanced", palette->GetDemands() == paletteDemands);
    check("sampled-pixels-restored", backedUp && pixels(NULL, &savedPixels, FALSE));
    check("same-tick-state-restored", game->GetWorldTimer()->m_gameTime == clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == messages
        && area->m_lVertSortAdd.GetCount() == front && area->m_lVertSortBackAdd.GetCount() == back && area->m_lVertSortFlightAdd.GetCount() == flight);
    Emit("{\"step\":%d,\"op\":\"vfxrenderchecks\",\"checks\":%d,\"renderCalls\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, draws, valid ? "true" : "false");
    if (!valid) Fail(nStep, "vfxrenderchecks", "VFX render, bounds or sequence mismatch (state restored)");
}

// Expected states captured from both canonical native integrators.
#include "ProjectileAimGolden.h"

template <class T>
struct ProjectileAimFixture : T {
    ProjectileAimFixture() : T(CResRef("ARARROW")) {}
    void Seed(const ProjectileAimInput& c) {
        this->m_pos = CPoint(c.state[0],c.state[1]);
        this->m_posAccumX=c.state[2]; this->m_posAccumY=c.state[3];
        this->m_driftX=c.state[4]; this->m_driftY=c.state[5];
        this->m_jitterMinX=c.state[6]; this->m_jitterMinY=c.state[7];
        this->m_jitterMaxX=c.state[8]; this->m_jitterMaxY=c.state[9];
        this->m_targetX=c.state[10]; this->m_targetY=c.state[11];
        this->m_velocity=c.speed; this->m_driftDecay=c.decay;
        this->m_hasDrift=c.flag; this->m_nTargetId=-1;
        this->m_posZ=-98765; this->m_lifetime=123;
    }
    CPoint Step() const { return CPoint(this->m_stepX,this->m_stepY); }
    CPoint Drift() const { return CPoint(this->m_driftX,this->m_driftY); }
    SHORT Facing() const { return this->m_facing; }
    BOOL Unchanged(const ProjectileAimInput& c) const {
        return this->m_velocity==c.speed && this->m_driftDecay==c.decay
            && this->m_hasDrift==c.flag && this->m_lifetime==123 && this->m_posZ==-98765
            && this->m_jitterMinX==c.state[6] && this->m_jitterMinY==c.state[7]
            && this->m_jitterMaxX==c.state[8] && this->m_jitterMaxY==c.state[9];
    }
};

void ProjectileAimChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    INT checks=0; BOOL valid=TRUE;
    INT input=-1, tick=-1, family=0;
    auto check=[&](const char* name,BOOL ok) {
        ++checks; valid=valid && ok;
        if(!ok) Emit("{\"step\":%d,\"op\":\"projectileaim-case\",\"case\":\"%s\",\"input\":%d,\"tick\":%d,\"family\":%d,\"passed\":false}",nStep,name,input,tick,family);
    };
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    auto exercise=[&](auto* type) {
        using Missile=typename std::remove_pointer<decltype(type)>::type;
        ++family;
        auto* p=new Missile; CProjectileTravelling* dispatch=p;
        for(const auto& expected:PROJECTILE_AIM_OUTPUTS) {
            input=expected.input; tick=expected.tick;
            const auto& c=PROJECTILE_AIM_INPUTS[expected.input];
            if(expected.tick==0) { p->Seed(c); srand(c.seed); }
            dispatch->AimAtPoint(c.x,c.y);
            const LONG* s=expected.state;
            check("native-cell-position",p->m_pos==CPoint(s[0],s[1]));
            check("native-fixed-accumulators",p->m_posAccumX==s[2] && p->m_posAccumY==s[3]);
            check("native-signed-velocity-step",p->Step()==CPoint(s[4],s[5]));
            check("native-drift-decay",p->Drift()==CPoint(s[6],s[7]));
            check("native-target-retention",p->m_targetX==s[8] && p->m_targetY==s[9]);
            check("native-facing",p->Facing()==expected.facing);
            check("unmodified-motion-settings",p->Unchanged(c));
            if(expected.nextRandom!=-1) check("native-random-consumption",rand()==expected.nextRandom);
        }
        // Cover the attached-object branch with the loaded engine's real array.
        auto* attached=new CVisualEffect;
        attached->m_pos=CPoint(77,88); attached->m_posZ=12345;
        check("attached-controller-registered",objects->Add(&attached->m_id,attached,INFINITE)==CGameObjectArray::SUCCESS);
        LONG attachedId=attached->m_id;
        auto c=PROJECTILE_AIM_INPUTS[0]; c.x=100; c.y=0; c.speed=10;
        p->Seed(c); p->m_nTargetId=attachedId; dispatch->AimAtPoint(c.x,c.y);
        check("attached-height-only",attached->m_posZ==-98765 && attached->m_pos==CPoint(77,88));
        check("attached-deny-balanced",objects->m_pArray[(attachedId>>16)&0x7FFF]
            .m_denyCounts[CGameObjectArray::THREAD_ASYNCH]==0);
        check("attached-controller-deleted",objects->Delete(attachedId,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE)==CGameObjectArray::SUCCESS);
        delete attached;
        for(LONG id:{LONG(-1),LONG(-2),attachedId}) {
            p->Seed(c); p->m_nTargetId=id; dispatch->AimAtPoint(c.x,c.y);
            check("missing-attachment-keeps-flight",p->m_pos==CPoint(10,0) && p->Step()==CPoint(10240,0)
                && p->m_targetX==100 && p->m_posZ==-98765 && p->m_nTargetId==id);
        }
        delete p;
    };
    exercise(static_cast<ProjectileAimFixture<CProjectileTravelling>*>(NULL));
    exercise(static_cast<ProjectileAimFixture<IcewindCProjectileTravellingVFX>*>(NULL));
    srand(1);
    check("same-tick-restoration",game->GetWorldTimer()->m_gameTime==clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages);
    Emit("{\"step\":%d,\"op\":\"projectileaimchecks\",\"checks\":%d,\"nativeStatesPerFamily\":%d,\"restored\":true,\"valid\":%s}",
        nStep,checks,static_cast<INT>(sizeof(PROJECTILE_AIM_OUTPUTS)/sizeof(PROJECTILE_AIM_OUTPUTS[0])),valid?"true":"false");
    if(!valid) Fail(nStep,"projectileaimchecks","travelling numeric/native state mismatch");
}

template <class T>
struct ProjectileTickFixture : T {
    CVidCell* savedCell;
    VefCellFixture cell;
    INT aims=0, arrivals=0, removals=0;
    CPoint aimed;
    LONG aimHeight=0;
    ProjectileTickFixture() : T(CResRef("ARARROW")), savedCell(this->m_pVidCell) {
        this->m_pVidCell=&cell;
    }
    ~ProjectileTickFixture() override { this->m_pVidCell=savedCell; }
    void OnArrival() override { ++arrivals; }
    void RemoveSelf() override { ++removals; }
    void AimAtPoint(int x,int y) override {
        ++aims; aimed=CPoint(x,y); aimHeight=this->m_posZ;
        T::AimAtPoint(x,y);
    }
    void Seed(CGameArea* area) {
        aims=arrivals=removals=cell.advances=0;
        this->m_pArea=area; this->CGameObject::m_pArea=area;
        this->m_pos=CPoint(40,36); this->m_posZ=25;
        this->m_posAccumX=40960; this->m_posAccumY=49152;
        this->m_targetX=400; this->m_targetY=36;
        this->m_velocity=1; this->m_flightDistSq=129600;
        this->m_targetId=this->m_nTargetId=-1; this->m_lifetime=100;
        this->m_hasDrift=this->m_driftX=this->m_driftY=0;
        this->m_jitterMinX=this->m_jitterMinY=this->m_jitterMaxX=this->m_jitterMaxY=0;
        this->m_nDeltaZ=this->m_nDeltaZLast=-20;
        this->m_bSparkleTrail=FALSE; this->m_sparkleColor=2;
        this->m_sound.m_nXCoordinate=111; this->m_sound.m_nYCoordinate=222;
        this->m_sound.m_nZCoordinate=333;
    }
    void Position(CPoint pos,CPoint target,SHORT velocity=1) {
        this->m_pos=pos; this->m_targetX=target.x; this->m_targetY=target.y; this->m_velocity=velocity;
    }
    void Lifetime(SHORT life) { this->m_lifetime=life; }
    SHORT Lifetime() const { return this->m_lifetime; }
    void Metric(LONG metric) { this->m_flightDistSq=metric; }
    void Drift(INT enabled) { this->m_hasDrift=enabled; }
};

void ProjectileTickChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    LONG actorId=game->GetFixedOrderCharacterId(0);
    CGameObject* actor=NULL;
    if(objects->GetDeny(actorId,CGameObjectArray::THREAD_ASYNCH,&actor,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"projectiletickchecks","requires loaded party actor"); return;
    }
    CGameArea* area=actor->m_pArea;
    if(!area || actor->GetObjectType()!=CGameObject::TYPE_SPRITE) {
        objects->ReleaseDeny(actorId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"projectiletickchecks","requires party sprite area"); return;
    }
    INT checks=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) {
        ++checks; valid=valid && ok;
        if(!ok) Emit("{\"step\":%d,\"op\":\"projectiletick-case\",\"case\":\"%s\",\"passed\":false}",nStep,name);
    };
    CPoint actorPos=actor->m_pos;
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    auto stop=game->m_nTimeStop; auto caster=game->m_nTimeStopCaster;
    game->m_nTimeStop=0;
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT front=area->m_lVertSortAdd.GetCount(), back=area->m_lVertSortBackAdd.GetCount();
    INT flight=area->m_lVertSortFlightAdd.GetCount(), sorted=area->m_lVertSort.GetCount();
    auto retire=[&](CGameObject* p) {
        LONG id=p->m_id;
        POSITION at=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if(at) area->m_lVertSortAdd.RemoveAt(at);
        at=area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id)); if(at) area->m_lVertSortBackAdd.RemoveAt(at);
        at=area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id)); if(at) area->m_lVertSortFlightAdd.RemoveAt(at);
        p->RemoveFromArea(); objects->Delete(id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p;
    };
    struct Target : CGameObject {
        CPoint live=CPoint(180,60);
        INT reads=0;
        CPoint& GetPos() override { ++reads; return live; }
    };
    auto* target=new Target;
    target->m_pos=CPoint(200,80); target->m_pArea=area;
    check("live-target-registered",objects->Add(&target->m_id,target,INFINITE)==CGameObjectArray::SUCCESS);
    auto exercise=[&](auto* type) {
        using Missile=typename std::remove_pointer<decltype(type)>::type;
        auto* p=new Missile;
        check("flight-fixture-registered",objects->Add(&p->m_id,p,INFINITE)==CGameObjectArray::SUCCESS);
        CProjectile* dispatch=p;
        p->Seed(area); dispatch->AIUpdate();
        check("point-frame-motion-countdown",p->cell.advances==1 && p->aims==1 && !p->arrivals && !p->removals
            && p->Lifetime()==99 && p->m_pos==CPoint(41,36) && p->aimed==CPoint(400,36));
        check("sound-follows-final-position",p->m_sound.m_nXCoordinate==41 && p->m_sound.m_nYCoordinate==36 && p->m_sound.m_nZCoordinate==25);
        p->Seed(area); game->m_nTimeStop=1; game->m_nTimeStopCaster=-1; dispatch->AIUpdate();
        check("time-stop-freezes-all",p->cell.advances==0 && p->aims==0 && p->Lifetime()==100 && p->m_sound.m_nXCoordinate==111);
        game->m_nTimeStopCaster=p->m_id; dispatch->AIUpdate();
        check("time-stop-own-object-exemption",p->cell.advances==1 && p->aims==1 && p->Lifetime()==99);
        game->m_nTimeStop=0;
        struct Arrival { CPoint pos,target; SHORT speed; LONG metric; };
        const Arrival arrivals[]={
            {CPoint(40,36),CPoint(47,40),1,129600},
            {CPoint(-1,-1),CPoint(-15,-11),1,129600},
            {CPoint(47,36),CPoint(49,36),1,129600},
            {CPoint(40,36),CPoint(400,36),1,0},
            {CPoint(40,36),CPoint(65576,36),1,129600},
            {CPoint(40,36),CPoint(41,11622),1,129600}
        };
        for(const auto& c:arrivals) {
            p->Seed(area); p->Position(c.pos,c.target,c.speed); p->Metric(c.metric); p->Lifetime(0);
            dispatch->AIUpdate();
            check("arrival-before-zero-timer",p->cell.advances==1 && p->arrivals==1 && p->removals==0
                && p->aims==0 && p->Lifetime()==0 && p->m_sound.m_nXCoordinate==111);
        }
        p->Seed(area); p->Position(CPoint(47,36),CPoint(50,36)); dispatch->AIUpdate();
        check("outside-inclusive-radius-flies",p->arrivals==0 && p->aims==1 && p->Lifetime()==99);
        for(SHORT lifetime:{SHORT(0),SHORT(1),SHORT(-1),SHORT(-32768)}) {
            p->Seed(area); p->Lifetime(lifetime); dispatch->AIUpdate();
            SHORT expected=lifetime==0?-1:lifetime==1?0:lifetime==-1?-2:32767;
            check("native-short-timer-wrap",p->Lifetime()==expected && p->removals==(lifetime==0)
                && p->aims==(lifetime!=0) && p->cell.advances==1);
            check("expiry-before-sound",lifetime!=0 || p->m_sound.m_nXCoordinate==111);
        }
        p->Seed(area); p->m_targetId=-2; dispatch->AIUpdate();
        check("missing-live-target-retires",p->removals==1 && p->aims==0 && p->Lifetime()==99 && p->m_sound.m_nXCoordinate==111);
        p->Seed(area); p->m_targetId=target->m_id; target->m_pArea=NULL; dispatch->AIUpdate();
        check("other-area-live-target-retires",p->removals==1 && p->aims==0 && p->Lifetime()==99);
        target->m_pArea=area; target->reads=0;
        p->Seed(area); p->m_targetId=target->m_id; p->Metric(10000); dispatch->AIUpdate();
        check("live-virtual-pos-and-stored-distance-height",p->aimed==CPoint(180,60) && target->reads==1
            && p->aimHeight==-214 && p->m_nDeltaZLast==-259 && p->aims==1 && p->removals==0);
        p->Seed(area); p->m_targetId=target->m_id; p->m_nDeltaZ=32767; dispatch->AIUpdate();
        check("height-product-wrap-before-division",p->aimHeight==-328 && p->m_nDeltaZLast==-373);
        p->Seed(area); p->m_targetId=target->m_id; p->m_nDeltaZ=0; p->m_nDeltaZLast=-32768;
        p->m_posZ=0x7FFFFFFF; dispatch->AIUpdate();
        check("height-addition-wrap",p->aimHeight==-2147450881 && p->m_nDeltaZLast==0);
        p->Seed(area); p->m_targetId=target->m_id; p->Drift(1); dispatch->AIUpdate();
        check("non-sprite-drift-preserves-height",p->aimHeight==25 && p->m_nDeltaZLast==-20 && p->aims==1);
        actor->m_pos=CPoint(400,36);
        for(SHORT delta:{SHORT(-1),SHORT(1)}) {
            p->Seed(area); p->m_targetId=actorId; p->Drift(1); p->m_nDeltaZ=delta;
            dispatch->AIUpdate();
            check("sprite-drift-native-restores-old-height",p->aimHeight==25 && p->m_posZ==25
                && p->aimed==CPoint(400,36) && p->m_nDeltaZLast==-20);
        }
        p->Seed(area); p->m_bSparkleTrail=TRUE; dispatch->AIUpdate();
        check("trail-spawn-after-real-motion",area->m_lVertSortAdd.GetCount()==front+1 && p->m_pos==CPoint(41,36));
        POSITION at=area->m_lVertSortAdd.FindIndex(front);
        if(at) {
            LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetAt(at));
            CGameObject* object=NULL;
            BYTE rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
            auto* trail=rc==CGameObjectArray::SUCCESS?dynamic_cast<CSparkleCluster*>(object):NULL;
            check("trail-real-cluster-at-final-height",trail && trail->m_pos==p->m_pos && trail->m_posZ==p->m_posZ
                && trail->m_particleList.GetCount()==14);
            if(rc==CGameObjectArray::SUCCESS) {
                objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); retire(object);
            }
        } else check("trail-real-cluster-at-final-height",FALSE);
        check("live-target-locks-balanced",objects->m_pArray[(target->m_id>>16)&0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH]==0);
        retire(p);
    };
    exercise(static_cast<ProjectileTickFixture<CProjectileTravelling>*>(NULL));
    exercise(static_cast<ProjectileTickFixture<IcewindCProjectileTravellingVFX>*>(NULL));
    target->m_pArea=NULL; retire(target);
    actor->m_pos=actorPos; game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster;
    objects->ReleaseDeny(actorId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    check("same-tick-restoration",actor->m_pos==actorPos && game->GetWorldTimer()->m_gameTime==clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages
        && area->m_lVertSortAdd.GetCount()==front && area->m_lVertSortBackAdd.GetCount()==back
        && area->m_lVertSortFlightAdd.GetCount()==flight && area->m_lVertSort.GetCount()==sorted
        && objects->m_pArray[(actorId>>16)&0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH]==0);
    srand(1);
    Emit("{\"step\":%d,\"op\":\"projectiletickchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if(!valid) Fail(nStep,"projectiletickchecks","travelling flight tick mismatch");
}

void ReboundChecks(INT nStep)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* objects = game->GetObjectArray();
    LONG actorId = game->GetFixedOrderCharacterId(0);
    CGameObject* actor = NULL;
    if (objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &actor, INFINITE)
        != CGameObjectArray::SUCCESS) {
        Fail(nStep, "reboundchecks", "requires loaded actor"); return;
    }
    CGameArea* area = actor->m_pArea;
    if (area == NULL) {
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Fail(nStep, "reboundchecks", "requires loaded area"); return;
    }
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"rebound-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
    };
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    auto stop = game->m_nTimeStop; auto stopCaster = game->m_nTimeStopCaster;
    auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
    INT messageCount = messages.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    struct Bolt : CProjectileReboundingBolt {
        BOOL* dead; INT* removed;
        Bolt(BOOL* d, INT* r) : dead(d), removed(r) { m_fireSoundRef = ""; }
        ~Bolt() override { *dead = TRUE; }
        void RemoveFromArea() override { ++*removed; CGameObject::RemoveFromArea(); }
        void Seed(CGameArea* area, LONG x, LONG y, LONG sx, LONG sy) {
            m_pArea = area; CGameObject::m_pArea = area;
            m_pos = CPoint(x, y); m_posZ = -7;
            m_posAccumX = static_cast<LONG>(static_cast<DWORD>(x) << 10);
            m_posAccumY = static_cast<LONG>(static_cast<DWORD>(y) << 12) / 3;
            m_stepX = sx; m_stepY = sy;
            m_targetX = 111; m_targetY = 112; m_facing = 13;
            m_pVidCell->m_nCurrentFrame = 10;
            m_targetMap.m_serviceCountdown = 5;
            memset(m_terrainTable, 1, sizeof(m_terrainTable));
        }
        CPoint Delta() const { return CPoint(m_stepX, m_stepY); }
        CPoint Exact() const { return CPoint(m_posAccumX, m_posAccumY); }
        CPoint Target() const { return CPoint(m_targetX, m_targetY); }
        SHORT Facing() const { return m_facing; }
        SHORT Frame() const { return m_pVidCell->m_nCurrentFrame; }
        SHORT Speed() const { return m_velocity; }
        void Speed(SHORT speed) { m_velocity = speed; }
        BOOL Defaults() const { return m_dirCount == 16 && m_tinted == 0 && m_useHeightOffset == 0
            && m_hasShadowCell == 0 && (m_renderFlags & 8) != 0 && m_bSparkleTrail == FALSE; }
    };
    auto unqueue = [&](LONG id) {
        POSITION p = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortAdd.RemoveAt(p);
        p = area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortBackAdd.RemoveAt(p);
        p = area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id)); if (p) area->m_lVertSortFlightAdd.RemoveAt(p);
    };
    auto retire = [&](Bolt* bolt) {
        unqueue(bolt->m_id); bolt->RemoveFromArea();
        objects->Delete(bolt->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
        delete bolt;
    };
    game->m_nTimeStop = 0;
    {
        BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed);
        check("constructor-defaults", b->Defaults() && b->m_nDuration == 60 && b->m_nBounces == -1
            && b->m_targetMap.m_pOwner == b && b->m_targetMap.m_nRange == 50
            && b->m_targetMap.m_servicePeriod == 2 && b->m_targetMap.m_strikeInterval == 33);
        delete b;
        CProjectile* decoded = CProjectile::DecodeProjectile(0xCF, NULL, 0);
        auto* concrete = dynamic_cast<CProjectileReboundingBolt*>(decoded);
        check("factory-CF-first-wall", concrete && concrete->m_nBounces == 1 && concrete->m_projectileType == 0xCE);
        delete decoded;
    }
    // Fire is tested through the public virtual, including its unusual +0x72
    // argument and the registration that precedes the second share failure.
    CPoint oldActorPos = actor->m_pos;
    actor->m_pos = CPoint(128, 96);
    for (LONG third : {actorId, -1L}) {
        BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed);
        CProjectile* virtualBolt = b;
        virtualBolt->Fire(area, actorId, third, CPoint(256, 192), -123, 7);
        check("launch-third-argument-and-invalid-target", !dead && b->m_sourceId == third && b->m_targetId == -1);
        check("launch-registers-before-second-share", b->m_id != -1);
        if (third != -1) {
            CPoint start = b->m_pos;
            LONG dx = 256 - start.x, dy = 256 - (start.y * 4) / 3;
            LONG length = static_cast<LONG>(sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);
            check("launch-area-height-fixedpoint", b->CGameObject::m_pArea == area && b->m_posZ == -123
                && b->Exact() == CPoint(start.x * 1024, (start.y * 4096) / 3) && b->field_17C == 1);
            check("launch-normalized-velocity", b->Delta() == CPoint(dx * 1024 * b->Speed() / length,
                dy * 1024 * b->Speed() / length));
        } else check("launch-missing-third-does-not-add-area", b->CGameObject::m_pArea == NULL);
        retire(b);
    }
    {
        BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed);
        b->Fire(area, -1, actorId, CPoint(256, 192), 0, 0);
        check("launch-missing-second-does-not-register", !dead && b->m_id == -1);
        delete b;
    }
    actor->m_pos = oldActorPos;

    // Plain object launch origins avoid sprite-facing offsets. Fixed golden
    // lengths cover the zero vector, fallback origin and sqrt + 0.5 rounding.
    CGameObject* origin = new CGameObject;
    CGameObject* fallback = new CGameObject;
    origin->m_pos = CPoint(128, 96); fallback->m_pos = CPoint(124, 93);
    objects->Add(&origin->m_id, origin, INFINITE); objects->Add(&fallback->m_id, fallback, INFINITE);
    for (SHORT speed : {SHORT(4), SHORT(-1), SHORT(-32768)}) {
        struct LaunchCase { CPoint point; LONG third; LONG dx; LONG dy; LONG length; };
        LaunchCase cases[] = {
            {CPoint(128, 96), origin->m_id, 0, 0, 0},
            {CPoint(128, 96), fallback->m_id, 4, 4, 6},
            {CPoint(131, 99), fallback->m_id, 3, 4, 5},
            {CPoint(127, 95), fallback->m_id, -1, -2, 2},
        };
        for (const auto& test : cases) {
            BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed); b->Speed(speed);
            b->Fire(area, origin->m_id, test.third, test.point, 17, -1);
            check("golden-launch-rounding-and-fallback", !dead && b->m_pos == CPoint(128, 96)
                && b->Exact() == CPoint(131072, 131072));
            if (test.length == 0) check("zero-vector-arms-expiry", b->m_nDuration == 0);
            else {
                auto scaled = [&](LONG value) { return static_cast<LONG>(static_cast<DWORD>(value * 1024)
                    * static_cast<LONG>(speed)) / test.length; };
                check("golden-signed-launch-speed", b->m_nDuration == 60 && b->Delta() == CPoint(scaled(test.dx), scaled(test.dy)));
            }
            retire(b);
        }
    }
    objects->Delete(origin->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE); delete origin;
    objects->Delete(fallback->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE); delete fallback;

    BYTE dynamic[64]; memset(dynamic, 0x80, sizeof(dynamic));
    BYTE* savedDynamic = area->m_search.m_pDynamicCost;
    CSize savedGrid = area->m_search.m_GridSquareDimensions;
    CSingleLock searchLock(&area->m_search.m_critSect, TRUE);
    area->m_search.m_pDynamicCost = dynamic; area->m_search.m_GridSquareDimensions = CSize(8, 8);
    auto gridOf = [](LONG x, LONG y) { return CPoint(x / 16, y / 12); };
    auto wall = [&](CPoint cell) { dynamic[cell.x + cell.y * 8] = 1; };
    for (CPoint delta : {CPoint(16, 8), CPoint(8, 16), CPoint(16, 16)}) {
        for (LONG signX : {-1L, 1L}) for (LONG signY : {-1L, 1L}) {
            LONG sx = delta.x * signX, sy = delta.y * signY;
            for (INT mask = 0; mask < 8; ++mask) for (LONG budget : {-1L, 0L, 1L, 2L}) {
                memset(dynamic, 0x80, sizeof(dynamic));
                if (mask & 1) wall(gridOf(56 + sx, 42 + sy * 3 / 4));
                if (mask & 2) wall(gridOf(56 - sx, 42 + sy * 3 / 4));
                if (mask & 4) wall(gridOf(56 + sx, 42 - sy * 3 / 4));
                BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed);
                objects->Add(&b->m_id, b, INFINITE); LONG id = b->m_id;
                b->Seed(area, 56, 42, sx * 1024, sy * 1024); b->m_nBounces = budget;
                CProjectile* virtualBolt = b; virtualBolt->AIUpdate();
                BOOL stopAtWall = (mask & 1) && budget == 1;
                check("wall-contact-retirement", dead == stopAtWall && removed == (stopAtWall ? 1 : 0));
                if (dead) {
                    CGameObject* stale = NULL;
                    check("retired-id-unavailable", objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &stale, INFINITE) != CGameObjectArray::SUCCESS);
                    continue;
                }
                // Golden wall choices: X-major tests X then Y; Y-major's
                // second test ignores index 8 when its cost is passable.
                INT flips = 0;
                if (mask & 1) {
                    static const INT xChoice[8] = {0, 1, 0, 2, 0, 1, 0, 3};
                    static const INT yChoice[8] = {0, 2, 0, 2, 0, 1, 0, 1};
                    flips = delta.x >= delta.y ? xChoice[mask] : yChoice[mask];
                }
                LONG outX = (flips & 1) ? -sx : sx, outY = (flips & 2) ? -sy : sy;
                check("golden-signed-wall-corner-velocity", b->Delta() == CPoint(outX * 1024, outY * 1024));
                check("golden-retracted-position", b->m_pos == CPoint(56 + outX, 42 + outY * 3 / 4));
                check("duration-budget-frame-and-service", b->m_nDuration == 59 && b->Frame() == 11
                    && b->m_nBounces == (((mask & 1) && budget > 0) ? budget - 1 : budget)
                    && b->m_targetMap.m_serviceCountdown == 4);
                check("sound-follows-head", b->m_sound.m_nXCoordinate == b->m_pos.x
                    && b->m_sound.m_nYCoordinate == b->m_pos.y && b->m_sound.m_nZCoordinate == -7);
                if (mask & 1) check("bounce-next-point-and-facing", b->Target() == CPoint(56 + 2 * outX, 42 + 2 * outY * 3 / 4)
                    && b->Facing() == b->GetDirection(b->Target()));
                else check("clear-crossing-keeps-aim", b->Target() == CPoint(111, 112) && b->Facing() == 13);
                retire(b);
            }
        }
    }
    // Time Stop, same-cell bypass, strict boundaries, wrapping and final tick.
    for (INT mode = 0; mode < 11; ++mode) {
        memset(dynamic, 0x80, sizeof(dynamic));
        BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed);
        objects->Add(&b->m_id, b, INFINITE); b->Seed(area, 56, 42, 1024, 0);
        if (mode == 0 || mode == 1) { game->m_nTimeStop = 1; game->m_nTimeStopCaster = mode == 0 ? -1 : b->m_id; }
        if (mode == 2) { wall(CPoint(3, 3)); b->m_nBounces = 1; }
        if (mode == 3) b->m_nDuration = 0;
        if (mode == 4) b->m_nDuration = 1;
        if (mode == 5) b->m_nDuration = LONG_MIN;
        if (mode == 6) b->Seed(area, 0, 42, -1024, 0);
        if (mode == 7) b->Seed(area, 56, 0, 0, -2048);
        if (mode == 8) b->Seed(area, area->GetInfinity()->nAreaX - 1, 42, 1024, 0);
        if (mode == 9) b->Seed(area, 56, area->GetInfinity()->nAreaY - 1, 0, 2048);
        if (mode == 10) { b->Seed(area, 56, 42, 16384, 8192); memset(b->m_terrainTable, CPathSearch::COST_IMPASSABLE, 16); }
        b->AIUpdate();
        if (mode == 3 || (mode >= 6 && mode <= 9)) check("expiry-and-strict-bounds", dead && removed == 1);
        else {
            check("live-guard-case", !dead);
            if (!dead) {
                if (mode == 0) check("time-stop-no-mutation", b->m_nDuration == 60 && b->Frame() == 10 && b->m_pos == CPoint(56, 42) && b->m_targetMap.m_serviceCountdown == 5);
                if (mode == 1 || mode == 2) check("owner-time-stop-and-same-cell", b->m_pos == CPoint(57, 42) && b->m_targetMap.m_serviceCountdown == 4 && b->m_nBounces == (mode == 2 ? 1 : -1));
                if (mode == 4) { check("duration-one-moves-before-expiry", b->m_nDuration == 0 && b->Frame() == 11); b->AIUpdate(); check("following-tick-expires", dead); }
                if (mode == 5) check("duration-signed-wrap", b->m_nDuration == LONG_MAX);
                if (mode == 10) check("cost-only-corner-reverses-both", b->Delta() == CPoint(-16384, -8192) && b->m_pos == CPoint(40, 36));
                if (!dead) retire(b);
            }
        }
        game->m_nTimeStop = 0;
    }
    {
        BOOL dead = FALSE; INT removed = 0; Bolt* b = new Bolt(&dead, &removed);
        objects->Add(&b->m_id, b, INFINITE); b->Seed(area, 56, 42, 1024, 0); b->m_nDuration = 0;
        CGameObject* shared = NULL;
        BYTE rc = objects->GetShare(b->m_id, CGameObjectArray::THREAD_1, &shared, INFINITE);
        check("delete-failure-share-fixture", rc == CGameObjectArray::SUCCESS);
        b->AIUpdate();
        check("delete-failure-removes-but-keeps-object", !dead && removed == 1 && b->CGameObject::m_pArea == NULL && b->Frame() == 10);
        if (rc == CGameObjectArray::SUCCESS) objects->ReleaseShare(b->m_id, CGameObjectArray::THREAD_1, INFINITE);
        if (!dead) retire(b);
    }
    area->m_search.m_pDynamicCost = savedDynamic; area->m_search.m_GridSquareDimensions = savedGrid;
    searchLock.Unlock();
    game->m_nTimeStop = stop; game->m_nTimeStopCaster = stopCaster;
    check("same-tick-restoration", actor->m_pos == oldActorPos && game->GetWorldTimer()->m_gameTime == clock
        && messages.GetCount() == messageCount && area->m_lVertSortAdd.GetCount() == front
        && area->m_lVertSortBackAdd.GetCount() == back && area->m_lVertSortFlightAdd.GetCount() == flight);
    objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    check("actor-deny-balanced", objects->m_pArray[(actorId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == 0);
    Emit("{\"step\":%d,\"op\":\"reboundchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
    if (!valid) Fail(nStep, "reboundchecks", "bolt launch or bounce mismatch");
}

void VefLoadChecks(INT nStep)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* objects = game->GetObjectArray();
    LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
    CGameObject* actor = NULL;
    BYTE rc = objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &actor, INFINITE);
    if (rc != CGameObjectArray::SUCCESS || actor == NULL || actor->m_pArea == NULL) {
        if (rc == CGameObjectArray::SUCCESS) objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Fail(nStep, "vefloadchecks", "requires loaded party area"); return;
    }
    CGameArea* area = actor->m_pArea;
    BOOL valid = TRUE;
    INT checks = 0;
    auto check = [&](const char* name, BOOL passed) {
        valid = valid && passed; ++checks;
        Emit("{\"step\":%d,\"op\":\"vefload-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
    };
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
    INT messageCount = messages.GetCount();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    auto balanced = [&](LONG id) { return objects->m_pArray[(id >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == 0; };
    auto retire = [&](LONG id) {
        CGameObject* object = NULL;
        if (objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS) {
            // AddToArea has queued these objects, but the area has not ticked.
            POSITION pos = area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id));
            if (pos != NULL) area->m_lVertSortAdd.RemoveAt(pos);
            pos = area->m_lVertSortBackAdd.Find(reinterpret_cast<int*>(id));
            if (pos != NULL) area->m_lVertSortBackAdd.RemoveAt(pos);
            pos = area->m_lVertSortFlightAdd.Find(reinterpret_cast<int*>(id));
            if (pos != NULL) area->m_lVertSortFlightAdd.RemoveAt(pos);
            object->RemoveFromArea();
            objects->Delete(id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
            delete object;
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    };
    for (USHORT type : {USHORT(1019), USHORT(1020)}) {
        CRes* resource = g_pBaldurChitin->AllocResObject(type);
        auto* binary = dynamic_cast<CResBinary*>(resource);
        check("binary-resource-factory", binary != NULL && binary->m_bParsed == FALSE);
        delete resource;
    }
    check("binary-header-helper-layout", sizeof(CResBinary) == sizeof(CRes) + 8 && sizeof(CVEFHeader) == 0x24
        && sizeof(CVisualEffectFile) == 16 && sizeof(CMessageProjectileTrailingVFX) == sizeof(CMessage) + 0x1E);
    CVEFEvent zeroEvent;
    BYTE zeroBytes[0xE0] = {};
    check("event-constructor-complete-zero", memcmp(&zeroEvent, zeroBytes, 0xE0) == 0);
    CVisualEffectFile empty;
    check("absent-helper-resource", empty.GetVEF() == NULL);
    CVisualEffectFile blank(CResRef("")), missing(CResRef("ZRCNONE"));
    check("empty-and-missing-helper-constructors", blank.pRes == NULL && !blank.bLoading
        && blank.cResRef == "" && missing.pRes == NULL && !missing.bLoading && missing.cResRef == "");
    check("missing-resource", CVisualEffect::Load("ZRCNONE", area, actor->m_pos, targetId, CPoint(-1, -1), 123) == -1);

    // A unique temporary KEY slot owns a pinned, already-loaded resource.
    // Keep one lock, one demand and one reference so no loader queue owns it.
    CDimm& dimm = g_pChitin->cDimm;
    auto& keys = dimm.m_cKeyTable;
    CResRef ref("ZRCVEF");
    EnterCriticalSection(&g_pChitin->m_critSectDimm);
    EnterCriticalSection(&g_pChitin->m_critSectService);
    UINT hash = keys.Hash(ref, 1020), slot = hash;
    while (keys.m_pEntries[slot].resRef != "") {
        slot = (slot + 1) % keys.m_nEntries;
        if (slot == hash) break;
    }
    BOOL available = keys.m_pEntries[slot].resRef == "" && keys.FindKey(ref, 1020, FALSE) == NULL;
    check("temporary-key-available", available);
    if (available) {
        CDimmKeyTableEntry savedKey = keys.m_pEntries[slot];
        short demands = dimm.field_8;
        int demandFlag = dimm.field_4, demandedBytes = dimm.field_29C;
        std::vector<BYTE> data(0x410, 0xCD);
        auto* header = reinterpret_cast<CVEFHeader*>(data.data());
        memcpy(header->signature, "VEF ", 4); memcpy(header->version, "V1.0", 4);
        header->timedOffset = 0x50; header->timedCount = 2;
        header->triggeredOffset = 0x240; header->triggeredCount = 2;
        header->targetType = 0xFEDCBA98u; header->speed = -32768; header->duration = 0x7FFFFFFF;
        CVEFEvent golden[4];
        for (INT i = 0; i < 4; ++i) {
            CVEFEvent record;
            record.m_nTimeFrame = i == 0 ? static_cast<LONG>(0x80000000u) : i * 7;
            record.m_dwFlags = 0xF0000000u + i; record.m_nRepeatRate = -1234 - i;
            record.m_nResType = 1018 + i; record.m_specialFlags = 0x89ABCDEFu + i;
            memcpy(&record.m_res, "aBcDeF12", 8);
            for (INT j = 0; j < 49; ++j) record.m_pad[j] = 0xDEAD0000u + i * 100 + j;
            memcpy(data.data() + (i < 2 ? 0x50 + i * 0xE0 : 0x240 + (i - 2) * 0xE0), &record, 0xE0);
            golden[i] = record; golden[i].m_res.MakeUpper();
        }
        auto originalData = data;
        CResBinary resource;
        resource.m_nID = static_cast<RESID>(-1);
        resource.dwFlags = 0x14; resource.nSize = static_cast<DWORD>(data.size());
        resource.m_nResSizeActual = resource.nSize; resource.m_pData = data.data();
        resource.m_nLockCount = 1; resource.m_nServiceCount = 1;
        auto& key = keys.m_pEntries[slot];
        key.resRef = ref; key.pRes = &resource; key.nID = static_cast<RESID>(-1);
        key.field_10 = 1; key.nResType = 1020; key.bUpdated = TRUE;
        resource.m_pDimmKeyTableEntry = &key;
        check("temporary-key-resolves", keys.FindKey(ref, 1020, FALSE) == &key);

        // First demand parses; nested demand preserves the previous pointer.
        resource.m_nServiceCount = 0;
        check("first-demand-version", resource.Demand() == data.data() && resource.m_bParsed == TRUE && resource.m_resVersion == reinterpret_cast<char*>(data.data()) + 4);
        resource.m_resVersion = reinterpret_cast<char*>(0x12345678);
        check("nested-demand-retains-version", resource.Demand() == data.data() && resource.m_resVersion == reinterpret_cast<char*>(0x12345678) && resource.GetDemands() == 2);
        resource.Release(); check("nested-release-retains-parsed", resource.m_bParsed == TRUE && resource.GetDemands() == 1);
        resource.Release(); check("last-release-clears-parsed", resource.m_bParsed == FALSE && resource.GetDemands() == 0 && resource.m_resVersion == reinterpret_cast<char*>(0x12345678));
        resource.m_nServiceCount = 1;
        {
            CVisualEffectFile file(ref);
            check("helper-request-reference", file.pRes == &resource && file.bLoading == TRUE && resource.m_nLockCount == 2 && key.field_10 == 2);
            for (INT mode = 0; mode < 5; ++mode) {
                header->timedCount = mode == 1 || mode == 3 ? 0 : 2;
                header->triggeredCount = mode == 2 || mode == 3 ? 0 : 2;
                // The native loader ignores both signature and version.
                if (mode == 4) { memcpy(header->signature, "????", 4); memcpy(header->version, "!!!!", 4); }
                DWORD timedOffset = header->timedOffset, triggeredOffset = header->triggeredOffset;
                if (header->timedCount == 0) header->timedOffset = 0xFFFFFFFFu;
                if (header->triggeredCount == 0) header->triggeredOffset = 0xFFFFFFFFu;
                CVisualEffect* effect = file.GetVEF();
                BOOL passed = effect != NULL;
                if (effect != NULL) {
                    CVisualEffectBase expectedBase;
                    memset(&expectedBase, 0, sizeof(expectedBase));
                    expectedBase.m_nTargetType = header->targetType;
                    expectedBase.m_nSpeed = header->speed; expectedBase.m_nDuration = header->duration;
                    passed = passed && effect->m_timedEvents.GetCount() == header->timedCount + header->triggeredCount
                        && effect->m_triggeredEvents.IsEmpty() && effect->m_activeAnimations.IsEmpty()
                        && effect->m_nTargetType == header->targetType && effect->m_nSpeed == -32768 && effect->m_nDuration == 0x7FFFFFFF
                        && effect->m_nTimedEventOffset == 0 && effect->m_nTimedEventCount == 0
                        && effect->m_nTriggeredEventOffset == 0 && effect->m_nTriggeredEventCount == 0
                        && effect->m_flags == 0 && effect->m_nHeight == 0 && effect->m_ptStartDeltaV == CPoint(0, 0)
                        && effect->m_nTargetId == -1 && effect->m_posExact == CPoint(0, 0)
                        && memcmp(static_cast<CVisualEffectBase*>(effect), &expectedBase, sizeof(expectedBase)) == 0;
                    POSITION pos = effect->m_timedEvents.GetHeadPosition();
                    for (INT i = 0; i < 4; ++i) {
                        if ((i < 2 && header->timedCount == 0) || (i >= 2 && header->triggeredCount == 0)) continue;
                        if (pos == NULL) { passed = FALSE; break; }
                        CVEFEvent* event = effect->m_timedEvents.GetNext(pos);
                        passed = passed && memcmp(event, &golden[i], 0xE0) == 0;
                        event->m_pad[48] ^= 0xFFFFFFFFu;
                    }
                    delete effect;
                }
                check("event-arrays-order-copy-and-header", passed && resource.m_nServiceCount == 1);
                header->timedOffset = timedOffset; header->triggeredOffset = triggeredOffset;
            }
        }
        check("helper-cancel-reference-balanced", resource.m_nLockCount == 1 && key.field_10 == 1);
        // Restore the header; copied event mutation must not touch source bytes.
        memcpy(header, originalData.data(), sizeof(CVEFHeader));
        check("source-resource-unchanged", data == originalData);
        {
            CVisualEffectFile file(ref);
            CVisualEffect* first = file.GetVEF(); CVisualEffect* second = file.GetVEF();
            BOOL independent = first != NULL && second != NULL && first != second;
            if (independent) {
                CVEFEvent* event = first->m_timedEvents.GetHead();
                CVEFEvent* other = second->m_timedEvents.GetHead();
                independent = event != other;
                event->m_nRepeatRate = 87654;
                independent = independent && other->m_nRepeatRate == golden[0].m_nRepeatRate;
            }
            delete first; delete second;
            check("repeated-load-independent-ownership", independent && resource.m_nServiceCount == 1);
        }
        resource.m_pData = NULL; resource.m_bParsed = FALSE;
        {
            CVisualEffectFile file(ref);
            check("null-demand-released", file.GetVEF() == NULL && resource.m_nServiceCount == 1);
        }
        check("null-load", CVisualEffect::Load("zrcvef", area, actor->m_pos, targetId, CPoint(-1, -1), 99) == -1
            && resource.m_nLockCount == 1 && key.field_10 == 1);
        resource.m_bParsed = TRUE; resource.m_resVersion = reinterpret_cast<char*>(0x12345678);
        check("null-nested-demand-keeps-version", resource.Demand() == NULL && resource.m_bParsed == TRUE
            && resource.m_resVersion == reinterpret_cast<char*>(0x12345678));
        resource.Release(); resource.m_bParsed = FALSE;
        resource.m_pData = data.data();
        const CPoint origins[6] = {actor->m_pos, CPoint(-7, -2), CPoint(7, 2), CPoint(123, 456),
            CPoint(0x7FFFFFFF, 0x7FFFFFFF), CPoint(static_cast<LONG>(0x80000000u), static_cast<LONG>(0x80000000u))};
        const CPoint expectedExact[6] = {CPoint(static_cast<LONG>(static_cast<DWORD>(actor->m_pos.x) << 10), (actor->m_pos.y * 4 / 3) * 1024),
            CPoint(-7168, -2048), CPoint(7168, 2048), CPoint(125952, 622592), CPoint(-1024, -1024), CPoint(0, 0)};
        for (INT i = 0; i < 6; ++i) {
            CPoint target = i == 0 ? CPoint(-1, -1) : i == 1 ? CPoint(-1, 123) : i == 2 ? CPoint(123, -1) : CPoint(333, 444);
            CString name(_T("zrcvef"));
            LONG id = CVisualEffect::Load(name, area, origins[i], targetId, target, -12345);
            CGameObject* object = NULL;
            BYTE got = objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
            auto* effect = got == CGameObjectArray::SUCCESS ? dynamic_cast<CVisualEffect*>(object) : NULL;
            check("load-target-origin-height-exact", effect != NULL && effect->m_nTargetId == targetId
                && effect->m_ptTargetPos == (i == 0 ? origins[i] : target) && effect->m_pos == origins[i]
                && effect->m_posZ == -12345 && effect->m_listType == CGameObject::LIST_FRONT
                && effect->m_posExact == expectedExact[i] && effect->m_posOld == CPoint(0, 0)
                && effect->m_nSpeed == -32768 && effect->m_timedEvents.GetCount() == 4 && name == "zrcvef"
                && area->m_lVertSortAdd.Find(reinterpret_cast<int*>(id)) != NULL);
            if (got == CGameObjectArray::SUCCESS) objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (id != -1) { check("load-deny-balanced", balanced(id)); retire(id); }
        }
        struct BaseMissile : CProjectileTravelling {
            BaseMissile(SHORT speed) : CProjectileTravelling(CResRef("ARARROW")) { m_velocity = speed; }
        };
        struct IceMissile : IcewindCProjectileTravellingVFX {
            IceMissile(SHORT speed) : IcewindCProjectileTravellingVFX(CResRef("ARARROW")) { m_velocity = speed; }
            BOOL PackedPrefix() const {
                const BYTE* north = reinterpret_cast<const BYTE*>(&m_bMirrorNorth);
                return reinterpret_cast<const BYTE*>(&m_bMirrorEast) - north == 1
                    && reinterpret_cast<const BYTE*>(&m_visualEffect) - north == 2;
            }
        };
        for (INT family = 0; family < 2; ++family) {
            for (INT mode = 0; mode < 5; ++mode) {
                SHORT speed = mode == 0 ? 4 : mode == 1 ? -1 : -32768;
                CProjectile* missile = family == 0 ? static_cast<CProjectile*>(new BaseMissile(speed)) : static_cast<CProjectile*>(new IceMissile(speed));
                if (family == 1 && mode == 0) check("vfx-prefix-packed-two", static_cast<IceMissile*>(missile)->PackedPrefix());
                missile->field_17E = mode == 3 ? "" : mode == 4 ? "ZRCNONE" : "zrcvef";
                missile->m_nTargetId = 0x12345678; // A previous ID does not gate the native branch.
                LONG caller = missile->m_id;
                missile->Fire(area, sourceId, targetId, CPoint(333, 444), 123456, 2345);
                LONG id = missile->m_nTargetId;
                BOOL loaded = mode < 3;
                CGameObject* object = NULL;
                BYTE got = loaded ? objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) : CGameObjectArray::BAD_INDEX;
                auto* effect = got == CGameObjectArray::SUCCESS ? dynamic_cast<CVisualEffect*>(object) : NULL;
                check("projectile-controller-speed-and-flag", loaded ? effect != NULL && effect->m_nSpeed == speed && effect->m_flags == 1
                    && effect->m_ptTargetPos == CPoint(333, 444) && effect->m_nTargetId == targetId
                    && effect->m_pos == missile->m_pos && effect->m_posZ == missile->m_posZ
                    : id == (mode == 3 ? 0x12345678 : -1));
                CString areaName; area->m_resRef.CopyToString(areaName);
                CMessageProjectileTrailingVFX* msg = messages.GetCount() > messageCount ? dynamic_cast<CMessageProjectileTrailingVFX*>(messages.GetTail()) : NULL;
                check("projectile-message-metadata", mode == 3 ? messages.GetCount() == messageCount : msg != NULL
                    && msg->m_sourceId == caller && msg->m_targetId == targetId && msg->m_resRef == CResRef(missile->field_17E)
                    && msg->m_resRef2 == areaName && CPoint(msg->m_launchX, msg->m_launchY) == missile->m_pos
                    && CPoint(msg->m_targetX, msg->m_targetY) == CPoint(333, 444) && msg->m_launchHeight == missile->m_posZ);
                if (got == CGameObjectArray::SUCCESS) objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                while (messages.GetCount() > messageCount) delete messages.RemoveTail();
                if (loaded && id != -1) { check("projectile-controller-deny-balanced", balanced(id)); retire(id); }
                LONG missileId = missile->m_id; retire(missileId);
                check("projectile-resource-balanced", resource.m_nLockCount == 1 && resource.m_nServiceCount == 1 && key.field_10 == 1);
                check("projectile-target-deny-balanced", balanced(targetId));
            }
        }
        check("all-source-events-unchanged", data == originalData);
        keys.m_pEntries[slot] = savedKey;
        resource.m_pData = NULL; resource.m_pDimmKeyTableEntry = NULL; resource.dwFlags = 0;
        resource.m_nServiceCount = resource.m_nLockCount = 0; resource.nSize = resource.m_nResSizeActual = 0;
        dimm.field_8 = demands; dimm.field_4 = demandFlag; dimm.field_29C = demandedBytes;
        check("temporary-key-restored", keys.FindKey(ref, 1020, FALSE) == NULL);
    }
    LeaveCriticalSection(&g_pChitin->m_critSectService);
    LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    check("same-tick-queue-clock-restored", game->GetWorldTimer()->m_gameTime == clock && messages.GetCount() == messageCount
        && area->m_lVertSortAdd.GetCount() == front && area->m_lVertSortBackAdd.GetCount() == back && area->m_lVertSortFlightAdd.GetCount() == flight);
    objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    check("actor-deny-balanced", balanced(sourceId));
    Emit("{\"step\":%d,\"op\":\"vefloadchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
    if (!valid) Fail(nStep, "vefloadchecks", "VEF load or projectile connection mismatch");
}

void VefMessageChecks(INT nStep)
{
    auto* game = g_pBaldurChitin->GetObjectGame();
    auto* objects = game->GetObjectArray();
    auto* remotes = game->GetRemoteObjectArray();
    LONG actorId = game->GetFixedOrderCharacterId(0);
    CGameObject* actor = NULL;
    if (objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &actor, INFINITE) != CGameObjectArray::SUCCESS || actor == NULL) {
        Fail(nStep, "vefmessagechecks", "requires party actor"); return;
    }
    INT checks = 0; BOOL valid = TRUE;
    auto check = [&](const char* name, BOOL ok) {
        ++checks; valid = valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"vef-message-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
    };
    PLAYER_ID oldPlayer = actor->m_remotePlayerID;
    LONG oldRemote = actor->m_remoteObjectID;
    remotes->m_critSect.Lock();
    CGameRemoteObjectListEntry mapping;
    mapping.remotePlayerID = 0x89ABCDEF; mapping.remoteObjectID = 0x1234;
    mapping.localObjectID = actorId; mapping.pNext = remotes->m_pArray[0];
    remotes->m_pArray[0] = &mapping;
    actor->m_remotePlayerID = mapping.remotePlayerID; actor->m_remoteObjectID = mapping.remoteObjectID;
    ULONG clock = game->GetWorldTimer()->m_gameTime;
    auto* area = actor->GetArea();
    INT front = area->m_lVertSortAdd.GetCount(), back = area->m_lVertSortBackAdd.GetCount(), flight = area->m_lVertSortFlightAdd.GetCount();
    INT queued = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    // Golden payload uses explicit little-endian bytes and is independent of
    // the serializer. Packet storage includes the network's specific header.
    auto golden = [&](const CString& resource, const CString& areaName, CPoint origin, CPoint target, SHORT height, PLAYER_ID player, LONG remote) {
        std::vector<BYTE> bytes(CNetwork::SPEC_MSG_HEADER_LENGTH, 0);
        bytes[CNetwork::SPEC_MSG_FLAG] = CNetwork::SPEC_MSG_FLAG_ENABLED;
        bytes[CNetwork::SPEC_MSG_TYPE] = CBaldurMessage::MSG_TYPE_CMESSAGE;
        bytes[CNetwork::SPEC_MSG_SUBTYPE] = 86;
        auto word = [&](DWORD value, INT count) { for (INT i = 0; i < count; ++i) bytes.push_back(static_cast<BYTE>(value >> (8 * i))); };
        word(player, 4); word(remote, 4);
        bytes.push_back(static_cast<BYTE>(resource.GetLength()));
        for (INT i = 0; i < resource.GetLength(); ++i) bytes.push_back(resource[i]);
        bytes.push_back(static_cast<BYTE>(areaName.GetLength()));
        for (INT i = 0; i < areaName.GetLength(); ++i) bytes.push_back(areaName[i]);
        word(origin.x, 4); word(origin.y, 4); word(target.x, 4); word(target.y, 4); word(static_cast<WORD>(height), 2);
        return bytes;
    };
    auto fields = [&](CMessageProjectileTrailingVFX* msg, const CResRef& resource, const CString& areaName, CPoint origin, CPoint target, SHORT height, LONG source, LONG id) {
        return msg && msg->m_resRef == resource && msg->m_resRef2 == areaName
            && msg->m_launchX == origin.x && msg->m_launchY == origin.y
            && msg->m_targetX == target.x && msg->m_targetY == target.y
            && msg->m_launchHeight == height && msg->GetSource() == source && msg->GetTarget() == id;
    };
    BYTE storage[640];
    const char* names = "AbCdEfGh";
    for (INT resLength = 0; resLength <= 8; ++resLength) {
        for (INT areaLength = 0; areaLength <= 8; ++areaLength) {
            CResRef resource(CString(names, resLength));
            CString resourceName; resource.CopyToString(resourceName);
            CString areaName(names, areaLength);
            CPoint origin(INT_MIN, 0x12345678), target(-1, INT_MAX);
            CMessageProjectileTrailingVFX message(resource, areaName, origin, target, SHRT_MIN, -12345, actorId);
            check("constructor-complete-fields-and-identity", fields(&message, resource, areaName, origin, target, SHRT_MIN, -12345, actorId));
            CMessage* virtualMessage = &message;
            memset(storage, 0xA5, sizeof(storage)); BYTE* data = storage + CNetwork::SPEC_MSG_HEADER_LENGTH;
            BYTE* originalData = data; DWORD size = 0;
            virtualMessage->MarshalMessage(&data, &size);
            auto expected = golden(resourceName, areaName, origin, target, SHRT_MIN, mapping.remotePlayerID, mapping.remoteObjectID);
            check("virtual-marshal-golden-and-exact-write", data == originalData && size == expected.size() - CNetwork::SPEC_MSG_HEADER_LENGTH
                && memcmp(data, expected.data() + CNetwork::SPEC_MSG_HEADER_LENGTH, size) == 0
                && storage[CNetwork::SPEC_MSG_HEADER_LENGTH - 1] == 0xA5 && data[size] == 0xA5);
            CMessageProjectileTrailingVFX decoded(CResRef("OLD"), CString(_T("OLD")), CPoint(1, 2), CPoint(3, 4), 5, 0x12345678, -1);
            CMessage* decoder = &decoded;
            check("virtual-decode-full-width-fields", decoder->UnmarshalMessage(expected.data(), static_cast<DWORD>(expected.size()))
                && fields(&decoded, resource, areaName, origin, target, SHRT_MIN, 0x12345678, actorId));
        }
    }
    auto packet = golden(CString(_T("aBcDeFgH")), CString(_T("aR10mIx")), CPoint(-87654321, INT_MAX), CPoint(INT_MIN, -1), SHRT_MAX, mapping.remotePlayerID, mapping.remoteObjectID);
    for (DWORD declaredSize : {0UL, 1UL, static_cast<DWORD>(packet.size() - 1), static_cast<DWORD>(packet.size() + 100)}) {
        CMessageProjectileTrailingVFX decoded(CResRef(), CString(), CPoint(0, 0), CPoint(0, 0), 0, -77, -1);
        check("native-decoder-ignores-declared-size-on-backed-buffer", decoded.UnmarshalMessage(packet.data(), declaredSize)
            && fields(&decoded, CResRef("ABCDEFGH"), CString(_T("aR10mIx")), CPoint(-87654321, INT_MAX), CPoint(INT_MIN, -1), SHRT_MAX, -77, actorId));
    }
    for (LONG id : {CGameObjectArray::INVALID_INDEX, static_cast<LONG>(0x7FFF1234)}) {
        CMessageProjectileTrailingVFX missing(CResRef("X"), CString(_T("AR")), CPoint(-1, 2), CPoint(3, -4), -1, 99, id);
        BYTE* data = storage; DWORD size = 0; missing.MarshalMessage(&data, &size);
        auto expected = golden(CString(_T("X")), CString(_T("AR")), CPoint(-1, 2), CPoint(3, -4), -1, 0, -1);
        check("missing-target-still-marshals-entire-payload", size == expected.size() - CNetwork::SPEC_MSG_HEADER_LENGTH
            && memcmp(data, expected.data() + CNetwork::SPEC_MSG_HEADER_LENGTH, size) == 0);
    }
    CString longArea('q', 256);
    CMessageProjectileTrailingVFX longName(CResRef("X"), longArea, CPoint(1, 2), CPoint(3, 4), 5, 0, actorId);
    BYTE* data = storage; DWORD size = 0; longName.MarshalMessage(&data, &size);
    auto longExpected = golden(CString(_T("X")), longArea, CPoint(1, 2), CPoint(3, 4), 5, mapping.remotePlayerID, mapping.remoteObjectID);
    check("marshal-uses-full-string-length-but-byte-prefix-wraps", size == 285 && data[10] == 0
        && memcmp(data, longExpected.data() + CNetwork::SPEC_MSG_HEADER_LENGTH, size) == 0);
    // The original decoder's two nine-byte buffers trust their lengths. Only
    // valid resource-name lengths are decoded here; long names test Marshal.
    auto missingPacket = golden(CString(_T("x")), CString(_T("MiXeD")), CPoint(-1, -2), CPoint(3, 4), -1, 0x76543210, 0x1234);
    CMessageProjectileTrailingVFX missingDecode(CResRef(), CString(), CPoint(0, 0), CPoint(0, 0), 0, 88, actorId);
    check("unmapped-target-decodes-successfully-with-invalid-id", missingDecode.UnmarshalMessage(missingPacket.data(), static_cast<DWORD>(missingPacket.size()))
        && fields(&missingDecode, CResRef("X"), CString(_T("MiXeD")), CPoint(-1, -2), CPoint(3, 4), -1, 88, -1));

    CNetwork& network = g_pChitin->cNetwork;
    auto* settings = game->GetMultiplayerSettings();
    auto* baldur = g_pBaldurChitin->GetBaldurMessage();
    INT provider = network.m_nServiceProvider; PLAYER_ID local = network.m_idLocalPlayer;
    BOOLEAN connected = network.m_bConnectionEstablished, host = network.m_bIsHost, initialized = network.m_SystemWindow.m_bInitialized;
    BOOLEAN reply = baldur->m_bDialogReplyReturned; BYTE blocked = game->field_4BD4;
    PLAYER_ID ready[CNETWORK_MAX_PLAYERS]; memcpy(ready, settings->m_pnPlayerReady, sizeof(ready));
    BOOLEAN windows[CNETWORK_MAX_PLAYERS];
    SimulatedTransportPause transportPause(network);
    EnterCriticalSection(&network.m_critSectQueue);
    network.m_idLocalPlayer = 0; network.m_bIsHost = FALSE; network.m_bConnectionEstablished = TRUE;
    network.m_SystemWindow.m_bInitialized = TRUE; baldur->m_bDialogReplyReturned = FALSE;
    for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) {
        windows[i] = network.m_pSlidingWindow[i].m_bInitialized; network.m_pSlidingWindow[i].m_bInitialized = FALSE;
        settings->m_pnPlayerReady[i] = 0xFFFFFFFF;
    }
    settings->m_pnPlayerReady[0] = 0;
    auto& incoming = network.m_SystemWindow.m_lQueueIncomingMessages;
    std::list<CNETWORKWINDOW_QUEUEENTRY*> savedIncoming;
    while (!incoming.IsEmpty()) savedIncoming.push_back(incoming.RemoveHead());
    auto receive = [&](const std::vector<BYTE>& bytes) {
        auto* node = new CNETWORKWINDOW_QUEUEENTRY;
        node->idFrom = 0xABCDEF12; node->idTo = 0; node->dwSize = CNetwork::dword_85E694 + static_cast<DWORD>(bytes.size());
        node->pData = new BYTE[node->dwSize]; memset(node->pData, 0, node->dwSize);
        memcpy(node->pData + CNetwork::dword_85E698, (LPCTSTR)CNetwork::MG, CNetwork::dword_85E69C);
        memcpy(node->pData + CNetwork::dword_85E694, bytes.data(), bytes.size()); incoming.AddTail(node);
    };
    {
        CMessageHandler handler;
        for (INT mode = 0; mode < 3; ++mode) {
            network.m_nServiceProvider = mode == 0 ? CNetwork::SERV_PROV_NULL : 1;
            game->field_4BD4 = mode == 1 ? 0 : 255;
            receive(packet); handler.AsynchronousUpdate();
            check("mg-frame-consumed", incoming.IsEmpty());
            if (mode < 2) check("immediate-reception-runs-and-retires", handler.m_messageList.IsEmpty());
            else {
                auto* decoded = handler.m_messageList.IsEmpty() ? NULL : dynamic_cast<CMessageProjectileTrailingVFX*>(handler.m_messageList.GetHead());
                check("deferred-reception-retains-all-decoded-fields", handler.m_messageList.GetCount() == 1
                    && fields(decoded, CResRef("ABCDEFGH"), CString(_T("aR10mIx")), CPoint(-87654321, INT_MAX), CPoint(INT_MIN, -1), SHRT_MAX, 0, actorId));
                handler.AsynchronousUpdate(); check("blocked-queue-retained", handler.m_messageList.GetCount() == 1);
                game->field_4BD4 = 0; handler.AsynchronousUpdate(); check("unblocked-queue-retired", handler.m_messageList.IsEmpty());
            }
        }
        network.m_nServiceProvider = 1; game->field_4BD4 = 1;
        receive(packet); receive(missingPacket); receive(packet); handler.AsynchronousUpdate();
        POSITION pos = handler.m_messageList.GetHeadPosition();
        const LONG ids[] = {actorId, -1, actorId};
        BOOL fifo = handler.m_messageList.GetCount() == 3;
        for (INT i = 0; i < 3 && pos != NULL; ++i) {
            auto* msg = dynamic_cast<CMessageProjectileTrailingVFX*>(handler.m_messageList.GetNext(pos));
            fifo = fifo && msg && msg->GetTarget() == ids[i];
        }
        check("deferred-fifo-includes-unmapped-message", fifo);
        game->field_4BD4 = 0; handler.AsynchronousUpdate(); check("fifo-drain-retired-all", handler.m_messageList.IsEmpty());
        settings->m_pnPlayerReady[0] = 0xFFFFFFFF;
        receive(packet); handler.AsynchronousUpdate(); check("not-ready-drops-vef-frame", incoming.IsEmpty() && handler.m_messageList.IsEmpty());
        settings->m_pnPlayerReady[0] = 0;
        for (BYTE state : {static_cast<BYTE>(0), static_cast<BYTE>(255)}) {
            network.m_bConnectionEstablished = state; receive(packet); handler.AsynchronousUpdate();
            check("connection-requires-exact-true", incoming.GetCount() == 1 && handler.m_messageList.IsEmpty());
            network.m_bConnectionEstablished = TRUE; handler.AsynchronousUpdate(); check("resumed-connection-consumes-frame", incoming.IsEmpty());
        }
        struct ObservedVEF : CMessageProjectileTrailingVFX {
            INT* count;
            ObservedVEF(INT* value) : CMessageProjectileTrailingVFX(CResRef(), CString(), CPoint(0, 0), CPoint(0, 0), 0, 0, 0), count(value) {}
            void Run() override { ++*count; CMessageProjectileTrailingVFX::Run(); }
        };
        INT ran = 0; handler.m_messageList.AddTail(new ObservedVEF(&ran)); handler.AsynchronousUpdate();
        check("subtype-86-actually-reaches-local-run", ran == 1 && handler.m_messageList.IsEmpty());
    }
    while (!incoming.IsEmpty()) { auto* node = incoming.RemoveHead(); delete node->pData; delete node; }
    for (auto* node : savedIncoming) incoming.AddTail(node);
    for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) network.m_pSlidingWindow[i].m_bInitialized = windows[i];
    network.m_nServiceProvider = provider; network.m_idLocalPlayer = local; network.m_bConnectionEstablished = connected;
    network.m_bIsHost = host; network.m_SystemWindow.m_bInitialized = initialized; baldur->m_bDialogReplyReturned = reply;
    game->field_4BD4 = blocked; memcpy(settings->m_pnPlayerReady, ready, sizeof(ready)); LeaveCriticalSection(&network.m_critSectQueue);
    remotes->m_pArray[0] = mapping.pNext; actor->m_remotePlayerID = oldPlayer; actor->m_remoteObjectID = oldRemote;
    remotes->m_critSect.Unlock(); objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    check("same-tick-clock-queues-and-area-restored", game->GetWorldTimer()->m_gameTime == clock
        && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == queued
        && area->m_lVertSortAdd.GetCount() == front && area->m_lVertSortBackAdd.GetCount() == back && area->m_lVertSortFlightAdd.GetCount() == flight);
    Emit("{\"step\":%d,\"op\":\"vefmessagechecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
    if (!valid) Fail(nStep, "vefmessagechecks", "VEF packet or receiver mismatch");
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

    if (strcmp(step.op, "effectcontrollerchecks") == 0) {
        EffectControllerChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "vefloadchecks") == 0) {
        VefLoadChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "vefmessagechecks") == 0) {
        VefMessageChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "reboundchecks") == 0) {
        ReboundChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "sparklechecks") == 0) {
        SparkleChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "sparklefamilychecks") == 0) {
        SparkleFamilyChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "projectilelaunchchecks") == 0) {
        ProjectileLaunchChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "projectiletickchecks") == 0) {
        ProjectileTickChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "projectileaimchecks") == 0) {
        ProjectileAimChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "explodingrenderchecks") == 0) {
        ExplodingRenderChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "vfxrenderchecks") == 0) {
        VfxRenderChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "beamrenderchecks") == 0) {
        BeamRenderChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "beamchecks") == 0) {
        BeamChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "chainchecks") == 0) {
        ChainLightningChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "nearestchecks") == 0) {
        AreaNearestChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "sparkleexplosionchecks") == 0) {
        SparkleExplosionChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "acidblobchecks") == 0) {
        AcidBlobChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "thrownweaponchecks") == 0) {
        ThrownWeaponChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "spraychecks") == 0) {
        SprayChecks(nStep); ++s_pc; return;
    }
    if (strcmp(step.op, "sparklewait") == 0) {
        CInfGame* game=g_pBaldurChitin->GetObjectGame(); CGameArea* area=game->GetVisibleArea();
        if(area==NULL) { Fail(nStep,step.op,"requires visible area"); return; }
        INT active=0;
        auto count=[&](CTypedPtrList<CPtrList,int*>& list) {
            POSITION p=list.GetHeadPosition();
            while(p) {
                LONG id=reinterpret_cast<LONG>(list.GetNext(p)); CGameObject* object=NULL;
                if(game->GetObjectArray()->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                    if(dynamic_cast<CSparkleCluster*>(object)!=NULL) ++active;
                    game->GetObjectArray()->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                }
            }
        };
        count(area->m_lVertSort); count(area->m_lVertSortBack); count(area->m_lVertSortFlight);
        count(area->m_lVertSortAdd); count(area->m_lVertSortBackAdd); count(area->m_lVertSortFlightAdd);
        if(active!=0) { if(++s_waited>DEFAULT_WAIT_TICKS) Fail(nStep,step.op,"trail clusters did not retire"); return; }
        Emit("{\"step\":%d,\"op\":\"sparklewait\",\"active\":0,\"valid\":true}",nStep);
        s_waited=0; ++s_pc; return;
    }

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

    if (strcmp(step.op, "targettriggerchecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* rawSource = NULL;
        CGameObject* rawTarget = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &rawSource, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and two party members"); return;
        }
        if (objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &rawTarget, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable"); return;
        }
        auto* source = static_cast<CGameSprite*>(rawSource);
        auto* target = static_cast<CGameSprite*>(rawTarget);
        auto sourceType = source->m_typeAI, targetType = target->m_typeAI;
        auto marked = source->field_342, spellTarget = source->field_3BA;
        SHORT hp = target->m_baseStats.m_hitPoints, maxHp = target->m_derivedStats.m_nMaxHitPoints;
        DWORD flags = target->m_baseStats.m_flags, state = target->m_derivedStats.m_generalState;
        auto sourceMarker = source->m_marker, targetMarker = target->m_marker;
        LONG targetIdBefore = source->m_targetId;
        BYTE sourceDenies = objects->m_pArray[(sourceId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH];
        BYTE targetDenies = objects->m_pArray[(targetId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH];
        INT queued = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            valid = valid && passed; ++checks;
            Emit("{\"step\":%d,\"op\":\"targettrigger-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
        };
        auto evaluate = [&](SHORT id, const CAIObjectType& type, LONG specific, LONG stat = 0) {
            CAITrigger cause(id, type, specific);
            cause.SetInt1(stat);
            CAITrigger before(cause);
            BOOL result = source->EvaluateStatusTrigger(cause);
            check("input-trigger-remains-unchanged", cause.m_triggerID == before.m_triggerID
                && cause.m_specificID == before.m_specificID && cause.m_specific2 == before.m_specific2
                && cause.m_triggerCause.Equal(before.m_triggerCause));
            return result;
        };
        CAIObjectType explicitTarget(CAIObjectType::ANYONE);
        explicitTarget.m_nInstance = targetId;
        explicitTarget.SetRadius(9876);
        explicitTarget.SetRect(CRect(1, 2, 3, 4));
        CAIObjectType stale(explicitTarget);
        stale.m_nInstance = 0x7FFF7FFE;
        for (SHORT id : {static_cast<SHORT>(CAITRIGGER_SETLASTMARKEDOBJECT), static_cast<SHORT>(CAITRIGGER_SETSPELLTARGET)}) {
            target->m_typeAI.Set(targetType);
            if (id == CAITRIGGER_SETSPELLTARGET) {
                target->m_typeAI.SetRect(CRect(22, 33, 44, 55));
            } else {
                target->m_typeAI.SetCenter(CPoint(17, 31)); target->m_typeAI.SetRadius(444);
            }
            source->field_342.Set(sourceType); source->field_3BA.Set(sourceType);
            check("set-explicit-target-always-true", evaluate(id, explicitTarget, 123) == TRUE);
            auto& changed = id == CAITRIGGER_SETSPELLTARGET ? source->field_3BA : source->field_342;
            auto& untouched = id == CAITRIGGER_SETSPELLTARGET ? source->field_342 : source->field_3BA;
            check("set-copies-object-ai-type", changed.Equal(target->m_typeAI)
                && changed.m_nLocationType == target->m_typeAI.m_nLocationType
                && (id == CAITRIGGER_SETSPELLTARGET ? changed.m_rect == CRect(22, 33, 44, 55)
                    : changed.m_nRadius == 444 && changed.m_ptCenter == CPoint(17, 31)));
            check("set-keeps-other-marker", untouched.Equal(sourceType));
            BYTE special[5] = {static_cast<BYTE>(id == CAITRIGGER_SETSPELLTARGET ? 55 : 51), 0, 0, 0, 0};
            CAIObjectType stored(CAIObjectType::ANYONE);
            stored.SetSpecialCase(special); stored.Decode(source);
            check("stored-selector-resolves-target", stored.GetInstance() == targetId);
            check("set-stale-target-always-true", evaluate(id, stale, 0) == TRUE);
            check("set-stale-target-clears-to-noone", changed.Equal(CAIObjectType::NOONE));
            stored.Set(CAIObjectType::ANYONE); stored.SetSpecialCase(special); stored.Decode(source);
            check("cleared-selector-stays-noone", stored.Equal(CAIObjectType::NOONE));
            check("set-noone-always-true", evaluate(id, CAIObjectType::NOONE, 0) == TRUE);
            check("set-myself-always-true", evaluate(id, CAIObjectType::OBJECT_MYSELF, 0) == TRUE);
            check("set-myself-copies-source", changed.Equal(sourceType));
            CAIObjectType immune(sourceType);
            auto& immunities = target->m_derivedStats.m_cImmunitiesAIType;
            POSITION immunePos = immunities.AddTail(&immune);
            check("set-target-does-not-reject-ai-immunity", evaluate(id, explicitTarget, 0) == TRUE
                && changed.Equal(target->m_typeAI));
            immunities.RemoveAt(immunePos);
            BYTE objectType = target->m_objectType;
            target->m_objectType = CGameObject::TYPE_NONE;
            check("set-target-does-not-require-sprite-type", evaluate(id, explicitTarget, 0) == TRUE
                && changed.Equal(target->m_typeAI));
            target->m_objectType = objectType;
        }
        target->m_typeAI.Set(targetType);
        source->m_typeAI.Set(CAIObjectType::ANYONE);
        source->m_typeAI.m_nInstance = sourceId;
        source->m_typeAI.m_nEnemyAlly = 255; source->m_typeAI.m_nGeneral = 4;
        source->m_typeAI.m_nRace = 7; source->m_typeAI.m_nSpecific = 253;
        source->m_typeAI.m_nGender = 2; source->m_typeAI.m_nAlignment = 0x23;
        source->m_typeAI.m_nClassMask = CLASSMASK_BARBARIAN | CLASSMASK_WIZARD;
        struct AttributeCase { SHORT id; LONG value; BOOL expected; };
        const AttributeCase attributes[] = {
            {CAITRIGGER_ALLEGIANCE, 255, TRUE}, {CAITRIGGER_ALLEGIANCE, 511, FALSE},
            {CAITRIGGER_GENERAL, 4, TRUE}, {CAITRIGGER_GENERAL, 260, FALSE},
            {CAITRIGGER_RACE, 7, TRUE}, {CAITRIGGER_RACE, 263, FALSE},
            {CAITRIGGER_SPECIFICS, 253, TRUE}, {CAITRIGGER_SPECIFICS, 509, FALSE},
            {CAITRIGGER_GENDER, 2, TRUE}, {CAITRIGGER_GENDER, 258, FALSE},
            {CAITRIGGER_CLASS, 1, TRUE}, {CAITRIGGER_CLASS, 11, TRUE}, {CAITRIGGER_CLASS, 2, FALSE},
            {CAITRIGGER_CLASS, 257, TRUE}, {CAITRIGGER_CLASS, 267, TRUE},
            {CAITRIGGER_ALIGNMENT, 1, TRUE}, {CAITRIGGER_ALIGNMENT, 2, TRUE}, {CAITRIGGER_ALIGNMENT, 3, TRUE},
            {CAITRIGGER_ALIGNMENT, 0x10, FALSE}, {CAITRIGGER_ALIGNMENT, 0x20, TRUE}, {CAITRIGGER_ALIGNMENT, 0x30, TRUE},
            {CAITRIGGER_ALIGNMENT, 0x23, TRUE}, {CAITRIGGER_ALIGNMENT, 0x21, FALSE},
            {CAITRIGGER_ALIGNMENT, 0x123, FALSE}, {CAITRIGGER_ALIGNMENT, 0, FALSE},
        };
        for (const auto& item : attributes)
            check("decoded-attribute-width-or-mask", evaluate(item.id, CAIObjectType::OBJECT_MYSELF, item.value) == item.expected);
        source->m_typeAI.Set(sourceType);
        struct HealthCase { SHORT current; SHORT maximum; LONG percent; LONG lost; };
        const HealthCase health[] = {
            {7, 20, 35, 13}, {1, 3, 33, 2}, {2, 3, 66, 1}, {0, 20, 0, 20},
            {-1, 20, -5, 21}, {-1, 3, -33, 4}, {21, 20, 105, -1},
            {32767, 32767, 100, 0}, {-32768, 32767, -100, 65535},
            {32767, -32768, -99, -65535}, {-32768, -32768, 100, 0},
            {1, -3, -33, -4}, {-2, -3, 66, -1},
        };
        const SHORT healthIds[3][3] = {
            {CAITRIGGER_HP, CAITRIGGER_HPGT, CAITRIGGER_HPLT},
            {CAITRIGGER_HPPERCENT, CAITRIGGER_HPPERCENTGT, CAITRIGGER_HPPERCENTLT},
            {CAITRIGGER_HPLOST, CAITRIGGER_HPLOSTGT, CAITRIGGER_HPLOSTLT},
        };
        for (const auto& item : health) {
            target->m_baseStats.m_hitPoints = item.current; target->m_derivedStats.m_nMaxHitPoints = item.maximum;
            LONG values[] = {item.current, item.percent, item.lost};
            for (INT group = 0; group < 3; ++group) {
                for (INT offset = -1; offset <= 1; ++offset) {
                    for (INT relation = 0; relation < 3; ++relation) {
                        BOOL expected = relation == 0 ? offset == 0 : relation == 1 ? offset < 0 : offset > 0;
                        check("health-signed-boundary", evaluate(healthIds[group][relation], explicitTarget, values[group] + offset) == expected);
                    }
                }
            }
        }
        for (const auto& ids : healthIds) for (SHORT id : ids)
            check("missing-health-target-false", evaluate(id, stale, 0) == FALSE);
        target->m_baseStats.m_hitPoints = 7; target->m_derivedStats.m_nMaxHitPoints = 20;
        target->m_baseStats.m_flags = 0x80000000; target->m_derivedStats.m_generalState = STATE_SLEEPING;
        const AttributeCase states[] = {
            {CAITRIGGER_STATECHECK, STATE_SLEEPING, TRUE}, {CAITRIGGER_STATECHECK, STATE_DEAD, FALSE},
            {CAITRIGGER_STATECHECK, STATE_SLEEPING | STATE_DEAD, TRUE}, {CAITRIGGER_STATECHECK, 0, FALSE},
            {CAITRIGGER_NOTSTATECHECK, STATE_SLEEPING, FALSE}, {CAITRIGGER_NOTSTATECHECK, STATE_DEAD, TRUE},
            {CAITRIGGER_NOTSTATECHECK, 0, TRUE},
            {CAITRIGGER_ISCREATUREAREAFLAG, static_cast<LONG>(0x80000000), TRUE},
            {CAITRIGGER_ISCREATUREAREAFLAG, STATE_SLEEPING, FALSE}, {CAITRIGGER_ISCREATUREAREAFLAG, 0, FALSE},
        };
        for (const auto& item : states)
            check("derived-state-versus-base-flags", evaluate(item.id, explicitTarget, item.value) == item.expected);
        for (SHORT id : {static_cast<SHORT>(CAITRIGGER_STATECHECK), static_cast<SHORT>(CAITRIGGER_NOTSTATECHECK),
                 static_cast<SHORT>(CAITRIGGER_ISCREATUREAREAFLAG), static_cast<SHORT>(CAITRIGGER_CHECKSTAT),
                 static_cast<SHORT>(CAITRIGGER_CHECKSTATGT), static_cast<SHORT>(CAITRIGGER_CHECKSTATLT)})
            check("missing-stat-or-state-target-false", evaluate(id, stale, 0, STAT_MAXHITPOINTS) == FALSE);
        for (INT offset = -1; offset <= 1; ++offset) {
            check("checkstat-equal", evaluate(CAITRIGGER_CHECKSTAT, explicitTarget, 20 + offset, STAT_MAXHITPOINTS) == (offset == 0));
            check("checkstat-greater", evaluate(CAITRIGGER_CHECKSTATGT, explicitTarget, 20 + offset, STAT_MAXHITPOINTS) == (offset < 0));
            check("checkstat-less", evaluate(CAITRIGGER_CHECKSTATLT, explicitTarget, 20 + offset, STAT_MAXHITPOINTS) == (offset > 0));
        }
        check("checkstat-narrows-stat-id-to-short", evaluate(CAITRIGGER_CHECKSTAT, explicitTarget, 20, 65537) == TRUE);
        target->m_typeAI.Set(CAIObjectType::NOT_SPRITE);
        check("quickdecode-rejects-not-sprite-sentinel", evaluate(CAITRIGGER_HP, explicitTarget, 7) == FALSE);
        source->m_typeAI.Set(sourceType); target->m_typeAI.Set(targetType);
        source->field_342.Set(marked); source->field_3BA.Set(spellTarget);
        target->m_baseStats.m_hitPoints = hp; target->m_derivedStats.m_nMaxHitPoints = maxHp;
        target->m_baseStats.m_flags = flags; target->m_derivedStats.m_generalState = state;
        check("no-combat-marker-side-effects", source->m_targetId == targetIdBefore
            && source->m_marker.m_type == sourceMarker.m_type && target->m_marker.m_type == targetMarker.m_type
            && source->m_marker.m_nRecticleCounter == sourceMarker.m_nRecticleCounter
            && target->m_marker.m_nRecticleCounter == targetMarker.m_nRecticleCounter);
        check("no-new-messages", g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount() == queued);
        check("deny-counts-balanced", objects->m_pArray[(sourceId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == sourceDenies
            && objects->m_pArray[(targetId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == targetDenies);
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"targettriggerchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "target/health trigger mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "combatmessagechecks") == 0 || strcmp(step.op, "effectpacketdamage") == 0
        || strcmp(step.op, "groupedpacketdamage") == 0 || strcmp(step.op, "receivedonhitdamage") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        auto* remotes = game->GetRemoteObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* sourceRaw = NULL; CGameObject* targetRaw = NULL;
        BYTE sourceRC = objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &sourceRaw, INFINITE);
        BYTE targetRC = objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &targetRaw, INFINITE);
        if (sourceRC != CGameObjectArray::SUCCESS || targetRC != CGameObjectArray::SUCCESS
            || !sourceRaw || !targetRaw || sourceRaw->GetObjectType() != CGameObject::TYPE_SPRITE
            || targetRaw->GetObjectType() != CGameObject::TYPE_SPRITE) {
            if (targetRC == CGameObjectArray::SUCCESS) objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (sourceRC == CGameObjectArray::SUCCESS) objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires two party sprites"); return;
        }
        auto* source = static_cast<CGameSprite*>(sourceRaw); auto* target = static_cast<CGameSprite*>(targetRaw);
        INT checks = 0; BOOL valid = TRUE;
        auto check = [&](const char* name, BOOL ok) {
            ++checks; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"combat-message-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        BOOL damage = strcmp(step.op, "combatmessagechecks") != 0;
        BOOL groupedDamage = strcmp(step.op, "groupedpacketdamage") == 0;
        BOOL onHitDamage = strcmp(step.op, "receivedonhitdamage") == 0;
        auto sourceOnHit = source->m_derivedStats.m_onHitSpells;
        auto targetOnHit = target->m_derivedStats.m_onHitSpells;
        SHORT sourceHP = source->m_baseStats.m_hitPoints, targetHP = target->m_baseStats.m_hitPoints;
        ULONG time = game->GetWorldTimer()->m_gameTime;
        PLAYER_ID sourcePlayer = source->m_remotePlayerID, targetPlayer = target->m_remotePlayerID;
        LONG sourceRemote = source->m_remoteObjectID, targetRemote = target->m_remoteObjectID;
        remotes->m_critSect.Lock();
        CGameRemoteObjectListEntry sourceMap, targetMap;
        sourceMap.remotePlayerID = 0x76543210; sourceMap.remoteObjectID = 0x5678; sourceMap.localObjectID = sourceId;
        targetMap.remotePlayerID = 0x89ABCDEF; targetMap.remoteObjectID = 0x1234; targetMap.localObjectID = targetId;
        sourceMap.pNext = &targetMap; targetMap.pNext = remotes->m_pArray[0]; remotes->m_pArray[0] = &sourceMap;
        source->m_remotePlayerID = sourceMap.remotePlayerID; source->m_remoteObjectID = sourceMap.remoteObjectID;
        target->m_remotePlayerID = targetMap.remotePlayerID; target->m_remoteObjectID = targetMap.remoteObjectID;
        auto& outgoing = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT queued = outgoing.GetCount();
        const DWORD header = CNetwork::SPEC_MSG_HEADER_LENGTH;
        BYTE effectBytes[128] = {}, projectileBytes[64] = {};
        BYTE* payload = effectBytes + header; DWORD size = 0;
        ITEM_EFFECT item = {}; item.effectID = CGAMEEFFECT_DAMAGE; item.durationType = 1;
        item.probabilityUpper = 100; item.effectAmount = 3; item.dwFlags = 0x800000;
        CMessageAddEffect sending(CGameEffect::DecodeEffect(&item, source->GetPos(), sourceId, target->GetPos()), 99, targetId);
        sending.m_effect->m_sourceRes = "ZRCHIT";
        if (!damage) {
            auto* effect = sending.m_effect;
            check("damage-virtual-uses-dice", effect->UsesDice() == TRUE);
            effect->m_effectID = 0x1234000C; effect->m_targetType = 0x11223344; effect->m_spellLevel = 0x22334455;
            effect->m_effectAmount = 0x12345678; effect->m_dwFlags = 0xDEADBEEF;
            effect->m_durationType = 0x12348000; effect->m_duration = 0x87654321;
            effect->m_probabilityUpper = 0x1280; effect->m_probabilityLower = 0x23FF;
            effect->m_res = "ABC"; effect->m_numDice = 0x01020304; effect->m_diceSize = 0x05060708;
            effect->m_savingThrow = 0xAABBCCDD; effect->m_saveMod = -2; effect->m_special = 0x11223344;
            effect->m_source = CPoint(0x01020304, -1); effect->m_target = CPoint(LONG_MIN, -1); effect->field_188 = 0x1234AA;
            const BYTE golden[] = {
                0xEF,0xCD,0xAB,0x89,0x34,0x12,0,0,0x0C,0,0x44,0x55,0x78,0x56,0x34,0x12,
                0xEF,0xBE,0xAD,0xDE,0,0x80,0x21,0x43,0x65,0x87,0x80,0xFF,
                'A','B','C',0,0,0,0,0,'Z','R','C','H','I','T',0,0,
                4,3,2,1,8,7,6,5,0xDD,0xCC,0xBB,0xAA,0xFE,0xFF,0xFF,0xFF,0x44,0x33,0x22,0x11,
                4,3,2,1,0xFF,0xFF,0xFF,0xFF,0x10,0x32,0x54,0x76,0x78,0x56,0,0,
                0,0,0,0x80,0xFF,0xFF,0xFF,0xFF,0xAA};
            memset(effectBytes, 0xA5, sizeof(effectBytes));
            static_cast<CMessage*>(&sending)->MarshalMessage(&payload, &size);
            check("effect-golden-widths-resrefs-dice-and-guards", size == sizeof(golden) && size == 89
                && payload == effectBytes + header && memcmp(payload, golden, sizeof(golden)) == 0
                && effectBytes[0] == 0xA5 && payload[size] == 0xA5);
            ITEM_EFFECT ac = {}; ac.effectID = CGAMEEFFECT_AC;
            CMessageAddEffect nonDice(CGameEffect::DecodeEffect(&ac, CPoint(1, 2), sourceId, CPoint(3, 4)), 77, targetId);
            nonDice.m_effect->m_res = ""; nonDice.m_effect->m_sourceRes = "";
            nonDice.m_effect->m_minLevel = 0x01020304; nonDice.m_effect->m_maxLevel = 0x11223344;
            nonDice.m_effect->m_numDice = 9; nonDice.m_effect->m_diceSize = 10;
            nonDice.m_effect->m_flags = 3; nonDice.m_effect->m_durationType = 1;
            static_cast<CMessage*>(&nonDice)->MarshalMessage(&payload, &size);
            DWORD maxLevel, minLevel; WORD duration;
            memcpy(&maxLevel, payload + 44, 4); memcpy(&minLevel, payload + 48, 4); memcpy(&duration, payload + 20, 2);
            const BYTE zeros[16] = {};
            check("non-dice-level-bounds-empty-resrefs-and-no-flag-merging", size == 89 && maxLevel == 0x11223344
                && minLevel == 0x01020304 && duration == 1 && memcmp(payload + 28, zeros, sizeof(zeros)) == 0);
            for (LONG missing : {-1L, 0x7FFF0001L}) {
                sending.m_targetId = missing; memset(effectBytes, 0xA5, sizeof(effectBytes)); size = 999;
                static_cast<CMessage*>(&sending)->MarshalMessage(&payload, &size);
                check("effect-missing-target-zero-size-buffer-untouched", size == 0 && payload[0] == 0xA5);
            }
            sending.m_targetId = targetId;
            // Return to a valid, deterministic damage effect before decoding.
            delete sending.m_effect;
            sending.m_effect = CGameEffect::DecodeEffect(&item, source->GetPos(), sourceId, target->GetPos());
            sending.m_effect->m_sourceRes = "ZRCHIT"; sending.m_effect->m_special = 0x12345678; sending.m_effect->field_188 = 255;
        }
        static_cast<CMessage*>(&sending)->MarshalMessage(&payload, &size);
        DWORD effectSize = size + header;
        if (!damage) {
            CSpell spell(CResRef("ZRCHIT"));
            LONG casterLevel = source->GetCasterLevel(&spell, 0, 0);
            DWORD firstCall = static_cast<DWORD>(source->GetFeatValue(0x40)) << 1;
            CMessageAddEffect received(NULL, 88, -1);
            check("effect-virtual-decode", static_cast<CMessage*>(&received)->UnmarshalMessage(effectBytes, effectSize));
            auto* effect = received.m_effect;
            check("effect-decode-damage-special-origin-and-caster", effect && dynamic_cast<CGameEffectDamage*>(effect)
                && effect->m_effectAmount == 3 && effect->m_special == 0x12345678 && effect->m_sourceID == sourceId
                && effect->m_source == source->GetPos() && effect->m_target == target->GetPos()
                && effect->m_sourceRes == "ZRCHIT" && effect->m_casterLevel == casterLevel && effect->m_firstCall == firstCall
                && effect->field_188 == 255 && received.GetSource() == 88 && received.GetTarget() == targetId);
            PLAYER_ID missingPlayer = 0x24681357;
            memcpy(payload, &missingPlayer, 4);
            CMessageAddEffect rejected(NULL, 88, sourceId);
            check("effect-unmapped-header-only-keeps-state", !rejected.UnmarshalMessage(effectBytes, header + 8)
                && rejected.GetTarget() == sourceId && rejected.m_effect == NULL && rejected.GetSource() == 88);
            memcpy(payload, &targetMap.remotePlayerID, 4); memcpy(payload + 72, &missingPlayer, 4);
            check("effect-unmapped-source-retargets-without-replacing-effect", !rejected.UnmarshalMessage(effectBytes, header + 80)
                && rejected.GetTarget() == targetId && rejected.m_effect == NULL);
            PLAYER_ID noPlayer = 0; LONG noObject = -1;
            memcpy(payload + 72, &noPlayer, 4); memcpy(payload + 76, &noObject, 4);
            check("effect-sentinel-source-decodes", rejected.UnmarshalMessage(effectBytes, effectSize));
            check("effect-sentinel-source-default-level", rejected.m_effect && rejected.m_effect->m_sourceID == -1
                && rejected.m_effect->m_casterLevel == 1 && rejected.m_effect->m_sourceRes == "ZRCHIT");
            static_cast<CMessage*>(&sending)->MarshalMessage(&payload, &size);
        }
        CMessageFireProjectile projectile(1, targetId, CPoint(LONG_MIN, LONG_MAX), -2, 99, sourceId, 255);
        projectile.field_20 = 0x12345678;
        BYTE* projectilePayload = projectileBytes + header; DWORD projectileSize = 0;
        static_cast<CMessage*>(&projectile)->MarshalMessage(&projectilePayload, &projectileSize);
        if (!damage) {
            const BYTE golden[] = {0x10,0x32,0x54,0x76,0x78,0x56,0,0,2,0,
                0xEF,0xCD,0xAB,0x89,0x34,0x12,0,0,0,0,0,0x80,0xFF,0xFF,0xFF,0x7F,
                0xFE,0xFF,0xFF,0xFF,0x78,0x56,0x34,0x12,0xFF};
            check("projectile-golden-pack2-fields", projectileSize == sizeof(golden)
                && memcmp(projectilePayload, golden, sizeof(golden)) == 0 && projectilePayload[35] == 0);
            CMessageFireProjectile decoded(7, sourceId, CPoint(9, 10), 11, 88, targetId, 12);
            check("projectile-virtual-decode", static_cast<CMessage*>(&decoded)->UnmarshalMessage(projectileBytes, header + projectileSize));
            check("projectile-decode-raw-type-and-full-tail", decoded.GetTarget() == sourceId && decoded.GetSource() == 88
                && decoded.m_projectileType == 2 && decoded.m_projectileTargetId == targetId
                && decoded.m_projectileTarget == CPoint(LONG_MIN, LONG_MAX) && decoded.m_height == -2
                && decoded.field_20 == 0x12345678 && decoded.field_1E == 255);
            for (WORD type : {static_cast<WORD>(0), static_cast<WORD>(1), static_cast<WORD>(11), static_cast<WORD>(16), static_cast<WORD>(65535)}) {
                projectile.m_projectileType = type; projectile.MarshalMessage(&projectilePayload, &projectileSize);
                check("projectile-decode-does-not-add-one", decoded.UnmarshalMessage(projectileBytes, header + projectileSize) && decoded.m_projectileType == type);
            }
            PLAYER_ID missing = 0x24681357;
            memcpy(projectilePayload, &missing, 4);
            check("projectile-unmapped-caster-still-accepts", decoded.UnmarshalMessage(projectileBytes, header + projectileSize) && decoded.GetTarget() == -1);
            memcpy(projectilePayload + 10, &missing, 4);
            decoded.m_projectileTargetId = sourceId; decoded.m_projectileTarget = CPoint(9, 10);
            check("projectile-unmapped-target-fails-after-caster-and-type", !decoded.UnmarshalMessage(projectileBytes, header + 18)
                && decoded.GetTarget() == -1 && decoded.m_projectileType == 65535 && decoded.m_projectileTargetId == sourceId
                && decoded.m_projectileTarget == CPoint(9, 10));
            PLAYER_ID noPlayer = 0; LONG noObject = -1;
            memcpy(projectilePayload + 10, &noPlayer, 4); memcpy(projectilePayload + 14, &noObject, 4);
            check("projectile-sentinel-target-accepts", decoded.UnmarshalMessage(projectileBytes, header + projectileSize) && decoded.m_projectileTargetId == -1);
            projectile.m_targetId = -1; memset(projectileBytes, 0xA5, sizeof(projectileBytes)); projectileSize = 999;
            projectile.MarshalMessage(&projectilePayload, &projectileSize);
            check("projectile-missing-caster-zero-size-buffer-untouched", projectileSize == 0 && projectilePayload[0] == 0xA5);
            projectile.m_targetId = sourceId; projectile.m_projectileType = 2;
            projectile.m_projectileTargetId = -1; projectile.MarshalMessage(&projectilePayload, &projectileSize);
            memcpy(&noPlayer, projectilePayload + 10, 4); memcpy(&noObject, projectilePayload + 14, 4);
            check("projectile-missing-target-serializes-sentinel", noPlayer == 0 && noObject == -1 && projectileSize == 35);
            projectile.m_projectileTargetId = targetId; projectile.m_projectileTarget = target->GetPos();
            projectile.m_height = 0; projectile.field_1E = 0;
            projectile.MarshalMessage(&projectilePayload, &projectileSize);
        }
        projectileSize += header;
        CMessage105::Node emptyHead = { &emptyHead, &emptyHead, NULL };
        CMessage105::Node groupHead, groupNodes[2];
        groupHead.m_pNext = &groupNodes[0]; groupHead.m_pPrev = &groupNodes[1]; groupHead.m_pMessage = NULL;
        for (INT i = 0; i < 2; ++i) {
            groupNodes[i].m_pNext = i == 0 ? &groupNodes[1] : &groupHead;
            groupNodes[i].m_pPrev = i == 0 ? &groupHead : &groupNodes[0];
            ITEM_EFFECT part = item; part.effectAmount = i + 2;
            auto* message = new CMessageAddEffect(CGameEffect::DecodeEffect(&part, source->GetPos(), sourceId, target->GetPos()), 123, targetId);
            message->m_effect->m_sourceRes = "ZRCHIT";
            groupNodes[i].m_pMessage = message;
        }
        CMessage105 grouped(&groupHead, 0x1234, targetId);
        BYTE groupBytes[512] = {}; BYTE* groupPayload = groupBytes + header; DWORD groupSize = 0;
        static_cast<CMessage*>(&grouped)->MarshalMessage(&groupPayload, &groupSize);
        if (!damage) {
            const BYTE goldenHeader[] = {0xEF,0xCD,0xAB,0x89,0x34,0x12,0,0,2,89,0,0,0};
            check("group-constructor-copies-nodes-and-owns-original-messages", grouped.field_C == 0x34
                && grouped.GetSource() == 0x1234 && grouped.GetTarget() == targetId && grouped.m_nCount == 2
                && grouped.m_pNodeHead != &groupHead && grouped.m_pNodeHead->m_pNext->m_pMessage == groupNodes[0].m_pMessage
                && grouped.m_pNodeHead->m_pPrev->m_pMessage == groupNodes[1].m_pMessage
                && groupHead.m_pNext == &groupNodes[0] && groupHead.m_pPrev == &groupNodes[1]);
            check("group-golden-header-size-and-buffer-guards", groupSize == 191 && groupPayload == groupBytes + header
                && memcmp(groupPayload, goldenHeader, sizeof(goldenHeader)) == 0 && groupPayload[groupSize] == 0);
            for (INT i = 0; i < 2; ++i) {
                BYTE child[128] = {}; BYTE* childPayload = child; DWORD childSize = 0;
                groupNodes[i].m_pMessage->MarshalMessage(&childPayload, &childSize);
                check("group-preserves-child-packets-in-order", childSize == 89 && memcmp(groupPayload + 13 + i * 89, child, 89) == 0);
            }
            CMessage105 decoded(&emptyHead, 0x5678, sourceId);
            check("group-virtual-decode-and-message-origin", static_cast<CMessage*>(&decoded)->UnmarshalMessage(groupBytes, header + groupSize)
                && decoded.GetTarget() == targetId && decoded.GetSource() == 0x5678 && decoded.field_C == 0x78 && decoded.m_nCount == 2);
            auto* node = decoded.m_pNodeHead->m_pNext;
            for (INT i = 0; i < 2 && node != decoded.m_pNodeHead; ++i, node = node->m_pNext) {
                auto* child = dynamic_cast<CMessageAddEffect*>(node->m_pMessage);
                check("group-decodes-concrete-child-effects-and-metadata", child && child->m_effect && dynamic_cast<CGameEffectDamage*>(child->m_effect)
                    && child->GetSource() == 0 && child->GetTarget() == targetId && child->m_effect->m_sourceID == sourceId
                    && child->m_effect->m_effectAmount == i + 2 && child->m_effect->m_sourceRes == "ZRCHIT");
            }
            auto* first = decoded.m_pNodeHead->m_pNext;
            check("group-decode-appends-and-ignores-outer-size", decoded.UnmarshalMessage(groupBytes, 0)
                && decoded.m_nCount == 4 && decoded.m_pNodeHead->m_pNext == first
                && decoded.m_pNodeHead->m_pPrev->m_pNext == decoded.m_pNodeHead);
            PLAYER_ID missing = 0x24681357; memcpy(groupPayload, &missing, 4);
            check("group-unmapped-outer-header-leaves-existing-list", !decoded.UnmarshalMessage(groupBytes, header + 8)
                && decoded.m_nCount == 4 && decoded.m_pNodeHead->m_pNext == first && decoded.GetTarget() == targetId);
            memcpy(groupPayload, &targetMap.remotePlayerID, 4);
            groupPayload[8] = 0; DWORD oddStride = 7; memcpy(groupPayload + 9, &oddStride, 4);
            check("group-empty-decode-retains-list-and-accepts-unused-stride", decoded.UnmarshalMessage(groupBytes, header + 13)
                && decoded.m_nCount == 4 && decoded.m_pNodeHead->m_pNext == first);
            CMessage105 empty(&emptyHead, 99, targetId);
            BYTE emptyBytes[32] = {}; BYTE* emptyPayload = emptyBytes; DWORD emptySize = 0;
            static_cast<CMessage*>(&empty)->MarshalMessage(&emptyPayload, &emptySize);
            check("group-empty-marshal-is-thirteen-bytes", emptySize == 13 && emptyPayload[8] == 0
                && memcmp(emptyPayload, goldenHeader, 8) == 0 && emptyPayload[9] == 89 && emptyPayload[13] == 0);
            empty.m_targetId = -1; memset(emptyBytes, 0xA5, sizeof(emptyBytes)); emptySize = 999;
            empty.MarshalMessage(&emptyPayload, &emptySize);
            check("group-missing-marshal-target-zero-size-untouched", emptySize == 0 && emptyPayload[0] == 0xA5);
            INT runOrder = 0, destroyed = 0;
            struct GroupProbe : CMessage {
                INT digit; INT* order; INT* destroyed;
                GroupProbe(INT digit, INT* order, INT* destroyed) : CMessage(0, 0), digit(digit), order(order), destroyed(destroyed) {}
                void MarshalMessage(BYTE** data, DWORD* size) override {
                    ++*data; memset(*data, digit, 89); *size = 89;
                }
                void Run() override { *order = *order * 10 + digit; }
                ~GroupProbe() override { ++*destroyed; }
            };
            {
                CMessage105::Node probeHead, probeNodes[2];
                probeHead = { &probeNodes[0], &probeNodes[1], NULL };
                probeNodes[0] = { &probeNodes[1], &probeHead, new GroupProbe(1, &runOrder, &destroyed) };
                probeNodes[1] = { &probeHead, &probeNodes[0], new GroupProbe(2, &runOrder, &destroyed) };
                CMessage105 probe(&probeHead, 0, targetId);
                BYTE bytes[256] = {}; BYTE* cursor = bytes; DWORD length = 0;
                static_cast<CMessage*>(&probe)->MarshalMessage(&cursor, &length);
                check("group-preserves-virtual-child-buffer-pointer-updates", cursor == bytes && length == 191
                    && bytes[13] == 0 && bytes[14] == 1 && bytes[102] == 1 && bytes[103] == 0
                    && bytes[104] == 2 && bytes[192] == 2 && bytes[193] == 0);
                static_cast<CMessage*>(&probe)->Run();
                check("group-run-keeps-order-and-retains-children", runOrder == 12 && destroyed == 0 && probe.m_nCount == 2);
                probe.Run();
                check("group-repeat-run-preserves-list", runOrder == 1212 && destroyed == 0 && probe.m_nCount == 2);
            }
            check("group-destructor-deletes-each-owned-child-once", destroyed == 2);
            // The original deliberately keeps a child even if its decoder fails.
            groupPayload[8] = 2; DWORD headerOnly = 8; memcpy(groupPayload + 9, &headerOnly, 4);
            for (INT i = 0; i < 2; ++i) { memcpy(groupPayload + 13 + i * 8, &missing, 4); memcpy(groupPayload + 17 + i * 8, &targetRemote, 4); }
            CMessage105 failedChildren(&emptyHead, 77, sourceId);
            check("group-ignores-child-decode-failure", failedChildren.UnmarshalMessage(groupBytes, header + 29)
                && failedChildren.GetTarget() == targetId && failedChildren.m_nCount == 2);
            for (auto* part = failedChildren.m_pNodeHead->m_pNext; part != failedChildren.m_pNodeHead; part = part->m_pNext) {
                auto* child = dynamic_cast<CMessageAddEffect*>(part->m_pMessage);
                check("group-retains-failed-child-with-null-effect", child && child->GetTarget() == 0 && child->m_effect == NULL);
            }
            for (INT i = 0; i < 2; ++i) static_cast<CMessageAddEffect*>(groupNodes[i].m_pMessage)->m_effect->m_effectID = CGAMEEFFECT_DETECTTRAPS;
            grouped.MarshalMessage(&groupPayload, &groupSize);
        }
        CMessage117 onHit(CResRef("ZRCORIG1"), CResRef("ZRCHIT"), 1, target->GetPos(), 99, sourceId);
        BYTE onHitBytes[64] = {}; BYTE* onHitPayload = onHitBytes + header; DWORD onHitSize = 0;
        static_cast<CMessage*>(&onHit)->MarshalMessage(&onHitPayload, &onHitSize);
        if (!damage) {
            onHit.m_sourceRes = "SRC12345"; onHit.m_spellRes = "DST67890";
            onHit.m_level = 0x89ABCDEF; onHit.m_target = CPoint(LONG_MIN, LONG_MAX);
            static_cast<CMessage*>(&onHit)->MarshalMessage(&onHitPayload, &onHitSize);
            const BYTE golden[] = {0x10,0x32,0x54,0x76,0x78,0x56,0,0,
                'S','R','C','1','2','3','4','5','D','S','T','6','7','8','9','0',
                0xEF,0xCD,0xAB,0x89,0,0,0,0x80,0xFF,0xFF,0xFF,0x7F};
            check("onhit-golden-packet-full-resrefs-level-points-and-guards", onHitSize == sizeof(golden)
                && onHitPayload == onHitBytes + header && memcmp(onHitPayload, golden, sizeof(golden)) == 0 && onHitPayload[36] == 0);
            CMessage117 decoded(CResRef("OLD"), CResRef("OLD2"), 7, CPoint(8, 9), 88, targetId);
            check("onhit-virtual-decode-keeps-caller-and-all-widths", static_cast<CMessage*>(&decoded)->UnmarshalMessage(onHitBytes, header + onHitSize)
                && decoded.GetSource() == 88 && decoded.GetTarget() == sourceId && decoded.m_sourceRes == "SRC12345"
                && decoded.m_spellRes == "DST67890" && decoded.m_level == 0x89ABCDEF && decoded.m_target == CPoint(LONG_MIN, LONG_MAX));
            PLAYER_ID missing = 0x24681357; memcpy(onHitPayload, &missing, 4);
            check("onhit-unmapped-header-only-leaves-all-state", !decoded.UnmarshalMessage(onHitBytes, header + 8)
                && decoded.GetSource() == 88 && decoded.GetTarget() == sourceId && decoded.m_sourceRes == "SRC12345"
                && decoded.m_spellRes == "DST67890" && decoded.m_level == 0x89ABCDEF && decoded.m_target == CPoint(LONG_MIN, LONG_MAX));
            onHit.m_targetId = -1; memset(onHitBytes, 0xA5, sizeof(onHitBytes)); onHitSize = 999;
            onHit.MarshalMessage(&onHitPayload, &onHitSize);
            check("onhit-missing-marshal-target-zero-size-untouched", onHitSize == 0 && onHitPayload[0] == 0xA5);
            onHit.m_targetId = sourceId; onHit.m_sourceRes = ""; onHit.m_spellRes = ""; onHit.m_level = 0; onHit.m_target = CPoint(0, 0);
            onHit.MarshalMessage(&onHitPayload, &onHitSize);
            const BYTE zeros[28] = {};
            check("onhit-empty-resrefs-zero-level-and-point", onHitSize == 36 && memcmp(onHitPayload + 8, zeros, sizeof(zeros)) == 0);
            onHit.m_sourceRes = "ZRCORIG1"; onHit.m_spellRes = "ZRCHIT";
            onHit.m_level = 0x1000101; onHit.m_target = CPoint(-123, 456);
            onHit.MarshalMessage(&onHitPayload, &onHitSize);
        }
        CNetwork& network = g_pChitin->cNetwork;
        auto* settings = game->GetMultiplayerSettings();
        INT provider = network.m_nServiceProvider;
        PLAYER_ID localPlayer = network.m_idLocalPlayer;
        BOOLEAN connected = network.m_bConnectionEstablished, hosting = network.m_bIsHost;
        BOOLEAN initialized = network.m_SystemWindow.m_bInitialized;
        BYTE blocked = game->field_4BD4;
        BOOLEAN returned = g_pBaldurChitin->GetBaldurMessage()->m_bDialogReplyReturned;
        PLAYER_ID ready[CNETWORK_MAX_PLAYERS]; memcpy(ready, settings->m_pnPlayerReady, sizeof(ready));
        BOOLEAN windows[CNETWORK_MAX_PLAYERS];
        SimulatedTransportPause transportPause(network);
        EnterCriticalSection(&network.m_critSectQueue);
        // Also preserve the native initial-wait guard during fake sessions.
        network.m_idLocalPlayer = 0;
        for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) {
            windows[i] = network.m_pSlidingWindow[i].m_bInitialized; network.m_pSlidingWindow[i].m_bInitialized = FALSE;
        }
        auto& incoming = network.m_SystemWindow.m_lQueueIncomingMessages;
        std::list<CNETWORKWINDOW_QUEUEENTRY*> savedIncoming;
        while (!incoming.IsEmpty()) savedIncoming.push_back(incoming.RemoveHead());
        network.m_bIsHost = FALSE; network.m_SystemWindow.m_bInitialized = TRUE; network.m_bConnectionEstablished = TRUE;
        settings->m_pnPlayerReady[0] = network.m_idLocalPlayer;
        g_pBaldurChitin->GetBaldurMessage()->m_bDialogReplyReturned = FALSE;
        auto receive = [&](BYTE* packet, DWORD length, BYTE subtype) {
            packet[CNetwork::SPEC_MSG_FLAG] = CNetwork::SPEC_MSG_FLAG_ENABLED;
            packet[CNetwork::SPEC_MSG_TYPE] = CBaldurMessage::MSG_TYPE_CMESSAGE;
            packet[CNetwork::SPEC_MSG_SUBTYPE] = subtype;
            auto* node = new CNETWORKWINDOW_QUEUEENTRY;
            node->idFrom = 0xABCDEF12; node->idTo = network.m_idLocalPlayer;
            node->dwSize = CNetwork::dword_85E694 + length; node->pData = new BYTE[node->dwSize]; memset(node->pData, 0, node->dwSize);
            memcpy(node->pData + CNetwork::dword_85E698, (LPCTSTR)CNetwork::MG, CNetwork::dword_85E69C);
            memcpy(node->pData + CNetwork::dword_85E694, packet, length); incoming.AddTail(node);
        };
        {
            CMessageHandler handler;
            if (onHitDamage) {
                s_onHitSourceHP = sourceHP; s_onHitTargetHP = targetHP;
                network.m_nServiceProvider = CNetwork::SERV_PROV_TCP_IP; game->field_4BD4 = 0;
                receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                auto& spells = source->m_derivedStats.m_onHitSpells;
                check("received-onhit-installs-actual-spell-before-trigger", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                    && spells.size() == 1 && spells.front().m_sourceRes == "ZRCORIG1" && spells.front().m_spellRes == "ZRCHIT"
                    && spells.front().m_level == 1 && spells.front().m_target == target->GetPos());
            } else if (groupedDamage) {
                network.m_nServiceProvider = CNetwork::SERV_PROV_NULL; game->field_4BD4 = 255;
                receive(groupBytes, header + groupSize, 105); handler.AsynchronousUpdate();
                check("received-group-immediate-damage-with-null-provider", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                    && source->m_baseStats.m_hitPoints == sourceHP && targetHP - target->m_baseStats.m_hitPoints == 5);
                network.m_nServiceProvider = CNetwork::SERV_PROV_TCP_IP;
                receive(groupBytes, header + groupSize, 105); handler.AsynchronousUpdate();
                auto* pending = handler.m_messageList.IsEmpty() ? NULL : dynamic_cast<CMessage105*>(handler.m_messageList.GetHead());
                check("received-group-deferred-damage-waits", incoming.IsEmpty() && handler.m_messageList.GetCount() == 1
                    && pending && pending->m_nCount == 2 && targetHP - target->m_baseStats.m_hitPoints == 5);
                game->field_4BD4 = 0; handler.AsynchronousUpdate();
                check("received-group-unblocked-delivers-both-effects", handler.m_messageList.IsEmpty()
                    && source->m_baseStats.m_hitPoints == sourceHP && targetHP - target->m_baseStats.m_hitPoints == 10);
            } else if (damage) {
                network.m_nServiceProvider = CNetwork::SERV_PROV_NULL; game->field_4BD4 = 0;
                receive(effectBytes, effectSize, 1); handler.AsynchronousUpdate();
                check("received-damage-reaches-defender-only", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                    && source->m_baseStats.m_hitPoints == sourceHP && targetHP - target->m_baseStats.m_hitPoints == 3);
            } else {
                WORD trap = CGAMEEFFECT_DETECTTRAPS; memcpy(payload + 8, &trap, 2);
                for (BYTE block : {static_cast<BYTE>(0), static_cast<BYTE>(255)}) {
                    network.m_nServiceProvider = CNetwork::SERV_PROV_TCP_IP; game->field_4BD4 = block;
                    receive(effectBytes, effectSize, 1); handler.AsynchronousUpdate();
                    check("received-effect-immediate-or-owned-queue", incoming.IsEmpty() && (block == 0 ? handler.m_messageList.IsEmpty()
                        : handler.m_messageList.GetCount() == 1 && dynamic_cast<CMessageAddEffect*>(handler.m_messageList.GetHead())));
                    if (block) {
                        auto* effectMessage = dynamic_cast<CMessageAddEffect*>(handler.m_messageList.GetHead());
                        check("received-effect-retains-owned-payload", effectMessage && effectMessage->m_effect
                            && effectMessage->m_effect->m_sourceID == sourceId && effectMessage->GetTarget() == targetId);
                        game->field_4BD4 = 0; handler.AsynchronousUpdate();
                        check("received-effect-unblocked-run", handler.m_messageList.IsEmpty());
                    }
                }
                auto& spells = source->m_derivedStats.m_onHitSpells;
                auto seedSpells = [&]() {
                    spells.clear();
                    COnHitSpell entry;
                    entry.m_sourceRes = "OLD"; entry.m_spellRes = "OLD2";
                    entry.m_level = 7; entry.m_target = CPoint(8, 9);
                    spells.push_back(entry); spells.push_back(entry);
                };
                auto installed = [&]() {
                    return spells.size() == 1 && spells.front().m_sourceRes == onHit.m_sourceRes
                        && spells.front().m_spellRes == onHit.m_spellRes && spells.front().m_level == onHit.m_level
                        && spells.front().m_target == onHit.m_target;
                };
                for (INT mode = 0; mode < 4; ++mode) {
                    seedSpells();
                    network.m_nServiceProvider = mode == 3 ? CNetwork::SERV_PROV_NULL : CNetwork::SERV_PROV_TCP_IP;
                    game->field_4BD4 = mode == 0 ? 0 : mode == 1 ? 1 : 255;
                    receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                    BOOL immediate = mode == 0 || mode == 3;
                    check("received-onhit-replaces-list-or-drops-when-blocked", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                        && (immediate ? installed() : spells.size() == 2 && spells.front().m_sourceRes == "OLD"));
                    game->field_4BD4 = 0; handler.AsynchronousUpdate();
                    check("received-onhit-blocked-packet-never-runs-later", handler.m_messageList.IsEmpty()
                        && (immediate ? installed() : spells.size() == 2 && spells.front().m_level == 7));
                }
                network.m_nServiceProvider = CNetwork::SERV_PROV_TCP_IP; game->field_4BD4 = 0;
                receive(onHitBytes, header + onHitSize, 117);
                onHit.m_sourceRes = "SECOND"; onHit.m_level = 0xFFFFFFFF; onHit.m_target = CPoint(LONG_MAX, LONG_MIN);
                onHit.MarshalMessage(&onHitPayload, &onHitSize);
                receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                check("received-onhit-fifo-last-replaces-first-full-width", incoming.IsEmpty() && handler.m_messageList.IsEmpty() && installed());
                onHit.m_sourceRes = ""; onHit.m_spellRes = ""; onHit.m_level = 0; onHit.m_target = CPoint(0, 0);
                onHit.MarshalMessage(&onHitPayload, &onHitSize);
                receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                check("received-onhit-empty-metadata-still-installs-one-record", installed() && handler.m_messageList.IsEmpty());
                seedSpells(); game->field_4BD4 = 255;
                receive(effectBytes, effectSize, 1); receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                check("received-onhit-drop-does-not-disturb-deferred-effect", incoming.IsEmpty() && handler.m_messageList.GetCount() == 1
                    && dynamic_cast<CMessageAddEffect*>(handler.m_messageList.GetHead()) && spells.size() == 2);
                game->field_4BD4 = 0; handler.AsynchronousUpdate();
                check("received-onhit-stays-dropped-after-mixed-unblock", handler.m_messageList.IsEmpty() && spells.size() == 2);
                PLAYER_ID missingOnHit = 0x24681357; memcpy(onHitPayload, &missingOnHit, 4);
                receive(onHitBytes, header + 8, 117); handler.AsynchronousUpdate();
                check("received-onhit-unmapped-immediate-and-heap-retry-discard", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                    && spells.size() == 2 && spells.front().m_sourceRes == "OLD");
                onHit.MarshalMessage(&onHitPayload, &onHitSize);
                network.m_bConnectionEstablished = FALSE;
                receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                check("disconnected-onhit-remains-in-network-queue", incoming.GetCount() == 1 && handler.m_messageList.IsEmpty() && spells.size() == 2);
                network.m_bConnectionEstablished = TRUE; handler.AsynchronousUpdate();
                check("reconnected-onhit-consumes-and-replaces-list", incoming.IsEmpty() && handler.m_messageList.IsEmpty() && installed());
                for (INT mode = 0; mode < 3; ++mode) {
                    network.m_nServiceProvider = mode == 2 ? CNetwork::SERV_PROV_NULL : CNetwork::SERV_PROV_TCP_IP;
                    game->field_4BD4 = mode == 0 ? 0 : 255;
                    receive(groupBytes, header + groupSize, 105); handler.AsynchronousUpdate();
                    check("received-group-immediate-or-owned-queue", incoming.IsEmpty() && (mode == 1
                        ? handler.m_messageList.GetCount() == 1 && dynamic_cast<CMessage105*>(handler.m_messageList.GetHead())
                        : handler.m_messageList.IsEmpty()));
                    if (mode == 1) {
                        auto* message = dynamic_cast<CMessage105*>(handler.m_messageList.GetHead());
                        check("received-group-retains-two-ordered-owned-effects", message && message->m_nCount == 2
                            && message->GetSource() == 0 && message->GetTarget() == targetId
                            && dynamic_cast<CMessageAddEffect*>(message->m_pNodeHead->m_pNext->m_pMessage)
                            && dynamic_cast<CMessageAddEffect*>(message->m_pNodeHead->m_pPrev->m_pMessage)
                            && static_cast<CMessageAddEffect*>(message->m_pNodeHead->m_pNext->m_pMessage)->m_effect->m_effectAmount == 2
                            && static_cast<CMessageAddEffect*>(message->m_pNodeHead->m_pPrev->m_pMessage)->m_effect->m_effectAmount == 3);
                        game->field_4BD4 = 0; handler.AsynchronousUpdate();
                        check("received-group-unblocked-run-and-release", handler.m_messageList.IsEmpty());
                    }
                }
                network.m_nServiceProvider = CNetwork::SERV_PROV_TCP_IP;
                game->field_4BD4 = 255;
                receive(effectBytes, effectSize, 1); receive(groupBytes, header + groupSize, 105); handler.AsynchronousUpdate();
                check("mixed-single-and-grouped-message-fifo", handler.m_messageList.GetCount() == 2
                    && dynamic_cast<CMessageAddEffect*>(handler.m_messageList.GetHead())
                    && dynamic_cast<CMessage105*>(handler.m_messageList.GetTail()));
                game->field_4BD4 = 0; handler.AsynchronousUpdate();
                check("mixed-single-and-grouped-message-unblock", handler.m_messageList.IsEmpty());
                network.m_bConnectionEstablished = FALSE;
                receive(groupBytes, header + groupSize, 105); handler.AsynchronousUpdate();
                check("disconnected-group-remains-in-network-queue", incoming.GetCount() == 1 && handler.m_messageList.IsEmpty());
                network.m_bConnectionEstablished = TRUE; handler.AsynchronousUpdate();
                check("reconnected-group-consumes-and-runs", incoming.IsEmpty() && handler.m_messageList.IsEmpty());
                // Inspect real launches before their first tick, then remove them.
                for (WORD type : {static_cast<WORD>(2), static_cast<WORD>(12), static_cast<WORD>(17), static_cast<WORD>(0x131)}) {
                    for (BYTE block : {static_cast<BYTE>(0), static_cast<BYTE>(255)}) {
                        std::set<CGameObject*> before;
                        for (INT i = 0; i < objects->m_nArraySize; ++i) if (objects->m_pArray[i].m_objectId != -1) before.insert(objects->m_pArray[i].m_objectPtr);
                        projectile.m_projectileType = type; projectile.MarshalMessage(&projectilePayload, &size);
                        game->field_4BD4 = block; receive(projectileBytes, header + size, 26); handler.AsynchronousUpdate();
                        if (block) {
                            auto* message = handler.m_messageList.IsEmpty() ? NULL : dynamic_cast<CMessageFireProjectile*>(handler.m_messageList.GetHead());
                            check("deferred-projectile-retains-raw-payload", handler.m_messageList.GetCount() == 1 && message
                                && message->m_projectileType == type && message->GetTarget() == sourceId && message->m_projectileTargetId == targetId);
                            game->field_4BD4 = 0; handler.AsynchronousUpdate();
                        }
                        CProjectile* launched = NULL; INT created = 0;
                        for (INT i = 0; i < objects->m_nArraySize; ++i) {
                            auto& entry = objects->m_pArray[i];
                            if (entry.m_objectId == -1 || before.count(entry.m_objectPtr)) continue;
                            auto* candidate = dynamic_cast<CProjectile*>(entry.m_objectPtr);
                            if (candidate) { launched = candidate; ++created; }
                        }
                        check("received-projectile-launches-once-with-source-target", created == 1 && incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                            && launched && launched->m_projectileType == type - 1 && launched->m_sourceId == sourceId
                            && launched->m_targetId == (type == 0x131 ? -1 : targetId) && launched->m_pArea == source->m_pArea);
                        if (launched) {
                            auto* whirlwind = dynamic_cast<CProjectileWhirlwind*>(launched);
                            check("received-projectile-concrete-type-and-whirlwind-seed", type == 2 ? dynamic_cast<CProjectileArrow*>(launched) != NULL
                                : type == 12 ? dynamic_cast<CProjectileBolt*>(launched) != NULL : type == 17 ? dynamic_cast<CProjectileSlingStone*>(launched) != NULL
                                : whirlwind && whirlwind->m_wanderSeed == 0x12345678);
                            launched->RemoveSelf();
                        }
                    }
                }
                projectile.m_projectileType = 2; projectile.MarshalMessage(&projectilePayload, &size);
                for (INT gate = 0; gate < 3; ++gate) {
                    SHORT nextId = objects->m_nNextObjectId;
                    auto* area = source->m_pArea;
                    auto owner = source->m_remotePlayerID;
                    network.m_nServiceProvider = gate == 0 ? CNetwork::SERV_PROV_NULL : CNetwork::SERV_PROV_TCP_IP;
                    if (gate == 1) source->m_remotePlayerID = network.m_idLocalPlayer;
                    if (gate == 2) source->m_pArea = NULL;
                    game->field_4BD4 = 0; receive(projectileBytes, header + size, 26); handler.AsynchronousUpdate();
                    check("received-projectile-owner-and-area-gates", objects->m_nNextObjectId == nextId
                        && handler.m_messageList.IsEmpty() && incoming.IsEmpty());
                    source->m_pArea = area; source->m_remotePlayerID = owner;
                }
                network.m_nServiceProvider = CNetwork::SERV_PROV_TCP_IP;
                PLAYER_ID missing = 0x24681357;
                memcpy(payload + 72, &missing, 4); memcpy(projectilePayload + 10, &missing, 4);
                for (BYTE block : {static_cast<BYTE>(0), static_cast<BYTE>(255)}) {
                    game->field_4BD4 = block;
                    memcpy(groupPayload, &missing, 4);
                    receive(effectBytes, header + 80, 1); receive(projectileBytes, header + 18, 26);
                    receive(groupBytes, header + 8, 105); handler.AsynchronousUpdate();
                    check("failed-incoming-decode-discards-owned-messages", incoming.IsEmpty() && handler.m_messageList.IsEmpty());
                }
                memcpy(payload + 72, &sourceMap.remotePlayerID, 4); memcpy(projectilePayload + 10, &targetMap.remotePlayerID, 4);
                memcpy(groupPayload, &targetMap.remotePlayerID, 4);
                for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) settings->m_pnPlayerReady[i] = network.m_idLocalPlayer ^ 0xFFFFFFFF;
                game->field_4BD4 = 0; receive(effectBytes, effectSize, 1); receive(projectileBytes, header + size, 26);
                receive(groupBytes, header + groupSize, 105); seedSpells();
                receive(onHitBytes, header + onHitSize, 117); handler.AsynchronousUpdate();
                check("not-ready-discards-combat-packets", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                    && spells.size() == 2 && spells.front().m_sourceRes == "OLD");
                check("packet-and-launch-checks-do-not-damage-party", source->m_baseStats.m_hitPoints == sourceHP && target->m_baseStats.m_hitPoints == targetHP);
            }
        }
        while (!incoming.IsEmpty()) { auto* node = incoming.RemoveHead(); delete node->pData; delete node; }
        for (auto* node : savedIncoming) incoming.AddTail(node);
        for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) network.m_pSlidingWindow[i].m_bInitialized = windows[i];
        network.m_nServiceProvider = provider; network.m_bConnectionEstablished = connected;
        network.m_idLocalPlayer = localPlayer;
        network.m_bIsHost = hosting; network.m_SystemWindow.m_bInitialized = initialized;
        memcpy(settings->m_pnPlayerReady, ready, sizeof(ready)); game->field_4BD4 = blocked;
        g_pBaldurChitin->GetBaldurMessage()->m_bDialogReplyReturned = returned;
        LeaveCriticalSection(&network.m_critSectQueue);
        if (!damage) while (outgoing.GetCount() > queued) delete outgoing.RemoveTail();
        remotes->m_pArray[0] = targetMap.pNext;
        source->m_remotePlayerID = sourcePlayer; source->m_remoteObjectID = sourceRemote;
        target->m_remotePlayerID = targetPlayer; target->m_remoteObjectID = targetRemote;
        remotes->m_critSect.Unlock();
        if (onHitDamage) {
            if (valid) source->ApplyOnHitSpells(target);
            check("received-onhit-trigger-queues-two-owned-effect-messages", outgoing.GetCount() == queued + 2);
        }
        source->m_derivedStats.m_onHitSpells = sourceOnHit;
        target->m_derivedStats.m_onHitSpells = targetOnHit;
        check("network-fixtures-and-clock-restored", game->GetWorldTimer()->m_gameTime == time && (damage || outgoing.GetCount() == queued));
        INT sourceLoss = sourceHP - source->m_baseStats.m_hitPoints, targetLoss = targetHP - target->m_baseStats.m_hitPoints;
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"%s\",\"checks\":%d,\"sourceLoss\":%d,\"targetLoss\":%d,\"restored\":true,\"valid\":%s}",
            nStep, step.op, checks, sourceLoss, targetLoss, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "combat message mismatch (fixtures restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "damagearrowchecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        auto* video = static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
        LONG id = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        BYTE rc = objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE);
        if (rc != CGameObjectArray::SUCCESS || !raw || raw->GetObjectType() != CGameObject::TYPE_SPRITE
            || !raw->m_pArea || g_pChitin->cVideo.m_bIs3dAccelerated) {
            if (rc == CGameObjectArray::SUCCESS) objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires party actor and software back surface"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        auto* area = actor->m_pArea;
        auto& infinity = area->m_cInfinity;
        auto& cell = game->m_vcLocator;
        CPoint savedPos = actor->m_pos; LONG savedZ = actor->m_posZ;
        BYTE savedList = actor->m_listType;
        BOOL active = actor->m_active, ai = actor->m_activeAI, imprisoned = actor->m_activeImprisonment;
        BOOLEAN stealth = actor->m_baseStats.m_bStealthMode;
        DWORD state = actor->m_derivedStats.m_generalState;
        BOOL blood = actor->m_bBloodFlashOn; SHORT time = actor->m_nDamageLocatorTime;
        COLORREF damageColor = actor->m_nDamageLocatorColor, markerColor = actor->m_marker.m_rgbColor;
        LONG picked = area->m_iPicked, counter = g_pBaldurChitin->nAUCounter;
        DWORD level = game->GetOptions()->m_nLocatorFeedbackLevel;
        CRect savedView = infinity.rViewPort;
        INT savedX = infinity.nCurrentX, savedY = infinity.nCurrentY;
        SHORT sequence = cell.m_nCurrentSequence, frame = cell.m_nCurrentFrame;
        COLORREF tint = cell.GetTintColor();
        ULONG clock = game->GetWorldTimer()->m_gameTime;
        INT queued = g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
        CRect patch(64, 64, 416, 336);
        std::vector<BYTE> savedPixels;
        // Read/write rows without copying pitch padding. Every lock is balanced.
        auto pixels = [&](std::vector<BYTE>* read, const std::vector<BYTE>* write, BOOL clear) -> BOOL {
            if (!video->BKLock(patch)) return FALSE;
            DWORD bytes = video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8;
            INT row = patch.Width() * bytes;
            BOOL ok = bytes >= 2 && bytes <= 4 && video->m_SurfaceDesc.lpSurface != NULL;
            if (ok && read) read->resize(row * patch.Height());
            if (ok && write) ok = write->size() == static_cast<size_t>(row * patch.Height());
            if (ok) for (INT y = 0; y < patch.Height(); ++y) {
                BYTE* pRow = static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface) + y * video->m_SurfaceDesc.lPitch;
                if (read) memcpy(read->data() + y * row, pRow, row);
                if (write) memcpy(pRow, write->data() + y * row, row);
                if (clear) memset(pRow, 0, row);
            }
            video->BKUnlock(); return ok;
        };
        BOOL valid = TRUE; INT checks = 0;
        auto check = [&](const char* name, BOOL ok) {
            ++checks; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"damage-arrow-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        BOOL backedUp = pixels(&savedPixels, NULL, FALSE);
        check("back-surface-pixels-backed-up", backedUp);
        auto reset = [&]() {
            actor->m_id = id; actor->m_pArea = area;
            actor->m_active = actor->m_activeAI = actor->m_activeImprisonment = 1;
            actor->m_baseStats.m_bStealthMode = 0; actor->m_derivedStats.m_generalState = 0;
            actor->m_bBloodFlashOn = FALSE; actor->m_nDamageLocatorTime = 0;
            actor->m_marker.m_rgbColor = RGB(80, 230, 100); actor->m_nDamageLocatorColor = RGB(250, 40, 70);
            actor->m_pos = CPoint(1500, 1120); actor->m_posZ = 0; actor->m_listType = CGameObject::LIST_FLIGHT;
            infinity.rViewPort = CRect(80, 80, 400, 320); infinity.nCurrentX = infinity.nCurrentY = 1000;
            game->GetOptions()->m_nLocatorFeedbackLevel = 4; area->m_iPicked = -1;
        };
        // Expected screen anchors/sequences are fixed by the PE's eight branches.
        // Compare the complete patch to a direct BAM render, including its clip.
        auto render = [&](const char* name, SHORT expectedSequence, INT x, INT y, COLORREF color) {
            if (!backedUp) return;
            std::vector<BYTE> expected, actual;
            BOOL ok = pixels(NULL, NULL, TRUE);
            COLORREF faded = video->ApplyFadeAmount(color);
            if (expectedSequence != -1) {
                cell.SequenceSet(expectedSequence); cell.FrameSet(0); cell.SetTintColor(faded);
                ok = cell.Render(CVIDINF_SURFACE_BACK, x, y, infinity.rViewPort, NULL, 0, 0x20000, -1) && ok;
            }
            ok = pixels(&expected, NULL, TRUE) && ok;
            BOOL drawn = std::any_of(expected.begin(), expected.end(), [](BYTE value) { return value != 0; });
            cell.SequenceSet(-12); cell.FrameSet(-13); cell.SetTintColor(RGB(11, 22, 33));
            actor->RenderDamageArrow(area, NULL, CVIDINF_SURFACE_BACK);
            ok = pixels(&actual, NULL, FALSE) && ok;
            check(name, ok && actual == expected && drawn == (expectedSequence != -1));
            check("locator-sequence-frame-tint-and-unlocked-surface", expectedSequence == -1
                ? cell.m_nCurrentSequence == -12 && cell.m_nCurrentFrame == -13 && cell.GetTintColor() == RGB(11, 22, 33)
                : cell.m_nCurrentSequence == expectedSequence && cell.m_nCurrentFrame == 0 && cell.GetTintColor() == faded);
            check("back-surface-unlocked", video->m_SurfaceDesc.lpSurface == NULL);
        };
        struct Direction { LONG x, y, z; SHORT sequence; INT screenX, screenY; };
        const Direction directions[] = {
            {1500,1120,0,0,400,200}, {1500,800,0,1,400,80}, {1160,800,0,2,240,80},
            {800,800,0,3,80,80}, {800,1120,0,4,80,200}, {800,1500,0,5,80,320},
            {1160,1500,0,6,240,320}, {1500,1500,0,7,400,320},
            {800,1120,24,4,80,200}, {800,1120,-500,3,80,80}, {800,1120,500,5,80,320},
            {1000,800,0,2,80,80}, {1320,800,0,2,400,80},
            {800,1000,0,4,80,80}, {800,1240,0,4,80,320}};
        for (const auto& direction : directions) {
            reset(); actor->m_pos = CPoint(direction.x, direction.y); actor->m_posZ = direction.z;
            render("direction-anchor-and-full-clipped-pixels", direction.sequence, direction.screenX, direction.screenY, actor->m_marker.m_rgbColor);
        }
        struct Gate { DWORD level; BOOL blood; SHORT time; LONG counter; BOOL picked; BOOL draw; BOOL damageTint; };
        const Gate gates[] = {
            {0,1,1,10,TRUE,FALSE,FALSE}, {1,1,1,10,TRUE,FALSE,FALSE}, {6,1,1,10,TRUE,FALSE,FALSE},
            {0xFFFFFFFF,1,1,10,TRUE,FALSE,FALSE}, {2,0,1,10,TRUE,FALSE,FALSE}, {2,1,0,10,TRUE,FALSE,FALSE},
            {2,1,1,9,TRUE,FALSE,FALSE}, {2,1,1,10,FALSE,TRUE,TRUE}, {2,2,-1,19,FALSE,TRUE,FALSE},
            {2,1,1,20,TRUE,FALSE,FALSE}, {2,1,1,-1,TRUE,FALSE,FALSE},
            {3,0,0,0,FALSE,FALSE,FALSE}, {3,0,0,0,TRUE,TRUE,FALSE}, {3,1,1,9,TRUE,TRUE,TRUE},
            {3,1,1,10,FALSE,TRUE,TRUE}, {3,1,-1,10,FALSE,TRUE,FALSE}, {4,0,0,0,FALSE,TRUE,FALSE},
            {5,2,1,0,FALSE,TRUE,TRUE}};
        for (const auto& gate : gates) {
            reset(); game->GetOptions()->m_nLocatorFeedbackLevel = gate.level;
            actor->m_bBloodFlashOn = gate.blood; actor->m_nDamageLocatorTime = gate.time;
            g_pBaldurChitin->nAUCounter = gate.counter; area->m_iPicked = gate.picked ? id : -1;
            render("feedback-blink-pick-and-signed-time", gate.draw ? 0 : -1, 400, 200,
                gate.damageTint ? actor->m_nDamageLocatorColor : actor->m_marker.m_rgbColor);
        }
        for (INT guard = 0; guard < 8; ++guard) {
            reset();
            switch (guard) {
            case 0: actor->m_id = -1; break;
            case 1: actor->m_active = 0; break;
            case 2: actor->m_activeAI = 0; break;
            case 3: actor->m_activeImprisonment = 0; break;
            case 4: actor->m_baseStats.m_bStealthMode = 1; break;
            case 5: actor->m_derivedStats.m_generalState = STATE_DEAD; break;
            case 6: actor->m_pArea = NULL; break;
            case 7: actor->m_pos = CPoint(1160,1120); break;
            }
            render("nonparty-inactive-stealth-dead-otherarea-visible", -1, 0, 0, 0);
        }
        reset(); actor->m_baseStats.m_bStealthMode = 2; actor->m_active = -1;
        render("stealth-byte-is-exact-one-and-active-is-truthy", 0, 400, 200, actor->m_marker.m_rgbColor);
        reset(); actor->m_pos = CPoint(999,1120);
        render("partially-visible-body-suppresses-offscreen-anchor", -1, 0, 0, 0);
        reset(); actor->m_pos = savedPos; actor->m_posZ = 24; actor->m_listType = savedList;
        infinity.nCurrentX = savedPos.x + 500; infinity.nCurrentY = savedPos.y - 120;
        INT height = area->GetHeightOffset(savedPos, savedList);
        render("real-terrain-height-and-horizontal-anchor-excludes-z", 4, 80, 200 + height, actor->m_marker.m_rgbColor);
        actor->m_id = id; actor->m_pArea = area; actor->m_pos = savedPos; actor->m_posZ = savedZ; actor->m_listType = savedList;
        actor->m_active = active; actor->m_activeAI = ai; actor->m_activeImprisonment = imprisoned;
        actor->m_baseStats.m_bStealthMode = stealth; actor->m_derivedStats.m_generalState = state;
        actor->m_bBloodFlashOn = blood; actor->m_nDamageLocatorTime = time;
        actor->m_nDamageLocatorColor = damageColor; actor->m_marker.m_rgbColor = markerColor;
        infinity.rViewPort = savedView; infinity.nCurrentX = savedX; infinity.nCurrentY = savedY;
        area->m_iPicked = picked; g_pBaldurChitin->nAUCounter = counter;
        game->GetOptions()->m_nLocatorFeedbackLevel = level;
        cell.SequenceSet(sequence); cell.FrameSet(frame); cell.SetTintColor(tint);
        check("sampled-pixels-restored", backedUp && pixels(NULL, &savedPixels, FALSE));
        check("locator-does-not-post-messages-or-advance-clock", queued == g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()
            && clock == game->GetWorldTimer()->m_gameTime);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"damagearrowchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "damage locator mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "weaponsoundchecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG actorId = game->GetFixedOrderCharacterId(0);
        CItem weapon(CResRef("ZRCMELEE"), 1, 1, 1, 0, 1);
        CItem launcher(CResRef("ZRCBOW"), 1, 1, 1, 0, 1);
        BOOL weaponDemanded = weapon.Demand(), launcherDemanded = launcher.Demand();
        if (!weaponDemanded || !launcherDemanded || !weapon.pRes->m_pHeader || !launcher.pRes->m_pHeader) {
            if (weaponDemanded) weapon.Release();
            if (launcherDemanded) launcher.Release();
            Fail(nStep, step.op, "requires combat item fixtures"); return;
        }
        CGameObject* raw = NULL;
        BYTE rc = objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE);
        if (rc != CGameObjectArray::SUCCESS || !raw || raw->GetObjectType() != CGameObject::TYPE_SPRITE) {
            if (rc == CGameObjectArray::SUCCESS) objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            weapon.Release(); launcher.Release();
            Fail(nStep, step.op, "requires party sprite"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        BOOL valid = TRUE; INT checks = 0;
        auto check = [&](const char* name, BOOL ok) {
            ++checks; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"weapon-sound-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        CString savedSound = actor->field_7408; INT savedFrame = actor->field_740C;
        DWORD savedState = actor->m_derivedStats.m_generalState;
        BOOLEAN custom = actor->field_70FB;
        SHORT animationSequence = actor->m_animation.m_nSequence, actorSequence = actor->m_nSequence;
        WORD weaponType = weapon.pRes->m_pHeader->itemType, launcherType = launcher.pRes->m_pHeader->itemType;
        STRREF speech[3];
        for (INT i = 0; i < 3; ++i) { speech[i] = actor->m_baseStats.m_speech[40 + i]; actor->m_baseStats.m_speech[40 + i] = -1; }
        CList<CGameSpriteSoundEntry, CGameSpriteSoundEntry>* lists[] = {
            &actor->field_72F0, &actor->field_730C, &actor->field_7328, &actor->field_7344 };
        std::list<CGameSpriteSoundEntry> savedLists[4];
        for (INT i = 0; i < 4; ++i) {
            for (POSITION pos = lists[i]->GetHeadPosition(); pos != NULL;) savedLists[i].push_back(lists[i]->GetNext(pos));
            lists[i]->RemoveAll();
        }
        ULONG time = game->GetWorldTimer()->m_gameTime;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList; INT queued = messages.GetCount();
        actor->field_7408 = "CANARY"; actor->field_740C = -123;
        actor->m_derivedStats.m_generalState = STATE_SILENCED;
        actor->DecodeSwingSound(NULL, NULL);
        check("silenced-preserves-selection-before-item-access", actor->field_7408 == "CANARY" && actor->field_740C == -123);
        actor->m_derivedStats.m_generalState = 0; actor->field_70FB = FALSE;
        actor->m_animation.m_nSequence = CGAMESPRITE_SEQ_AWAKE;
        struct Family { WORD type; const char* prefix; INT first; INT last; INT width; };
        const Family families[] = {
            {0,"Sw_Swd",1,5,2}, {15,"Sw_Ms",1,5,2}, {16,"Sw_Sm",1,7,2}, {17,"Sw_Sm",1,7,2},
            {18,"Sw_Slg",1,5,2}, {19,"Sw_Swd",1,5,2}, {20,"Sw_Swd",1,5,2}, {21,"Sw_Lg",1,6,2},
            {22,"Sw_Sm",1,7,2}, {23,"Sw_Flal",0,4,1}, {24,"Sw_Fist",0,2,1}, {25,"Sw_Lg",1,6,2},
            {26,"Sw_Lg",1,6,2}, {27,"Sw_Xbow",0,4,1}, {28,"Sw_Fist",0,2,1}, {29,"Sw_Sm",1,7,2},
            {30,"Sw_Lg",1,6,2}, {44,"Sw_Lg",1,6,2}, {57,"Sw_Swd",1,5,2}, {69,"Sw_Swd",1,5,2}
        };
        for (const auto& family : families) {
            weapon.pRes->m_pHeader->itemType = family.type;
            actor->field_7408 = "CANARY";
            for (INT repeat = 0; repeat < 16; ++repeat) {
                CString previous = actor->field_7408; actor->field_740C = -123;
                actor->DecodeSwingSound(&weapon, NULL);
                CString suffix = actor->field_7408.Mid(static_cast<INT>(strlen(family.prefix)));
                INT number = atoi(suffix);
                check("weapon-family-range-format-and-no-repeat", actor->field_7408.Left(static_cast<INT>(strlen(family.prefix))) == family.prefix
                    && suffix.GetLength() == family.width && number >= family.first && number <= family.last
                    && actor->field_7408 != previous && actor->field_740C == 0);
            }
        }
        weapon.pRes->m_pHeader->itemType = 16; launcher.pRes->m_pHeader->itemType = 27;
        actor->DecodeSwingSound(&weapon, &launcher);
        check("launcher-type-overrides-ammunition-type", actor->field_7408.Left(7) == "Sw_Xbow" && actor->field_740C == 0);
        const char* creatureItems[] = {"WYVERN1", "WYVERN2", "WOLFWI1", "WOLFWI2"};
        const char* creatureSounds[] = {"WYVER05", "WYVER06", "WWOLF07", "WWOLF06"};
        for (INT i = 0; i < 4; ++i) {
            weapon.cResRef = creatureItems[i]; actor->DecodeSwingSound(&weapon, &launcher);
            check("creature-item-exemption-precedes-launcher", actor->field_7408 == creatureSounds[i] && actor->field_740C == 0);
        }
        weapon.cResRef = "ZRCMELEE";
        for (WORD type : {static_cast<WORD>(31), static_cast<WORD>(0xFFFF)}) {
            weapon.pRes->m_pHeader->itemType = type;
            actor->field_7408 = "CANARY"; actor->field_740C = -123;
            actor->DecodeSwingSound(&weapon, NULL);
            check("unknown-type-without-speech-preserves-selection", actor->field_7408 == "CANARY" && actor->field_740C == -123);
        }
        actor->field_70FB = 2; actor->m_nSequence = CGAMESPRITE_SEQ_DIE;
        const SHORT sequences[] = {CGAMESPRITE_SEQ_ATTACK_SLASH, CGAMESPRITE_SEQ_ATTACK_BACKSLASH,
            CGAMESPRITE_SEQ_ATTACK_JAB, CGAMESPRITE_SEQ_SHOOT};
        for (INT i = 0; i < 4; ++i) {
            CGameSpriteSoundEntry first, second;
            first.field_0 = "FIRST"; first.field_4 = LONG_MIN;
            second.field_0 = "SECOND"; second.field_4 = LONG_MAX;
            lists[i]->AddTail(first); lists[i]->AddTail(second);
            actor->m_animation.m_nSequence = sequences[i]; actor->field_7408 = "FIRST";
            actor->DecodeSwingSound(NULL, NULL);
            check("custom-list-uses-animation-sequence-and-avoids-repeat", actor->field_7408 == "SECOND" && actor->field_740C == LONG_MAX);
            actor->DecodeSwingSound(NULL, NULL);
            check("custom-list-wraps-to-first-with-signed-frame", actor->field_7408 == "FIRST" && actor->field_740C == LONG_MIN);
            lists[i]->RemoveAll(); first.field_0 = ""; first.field_4 = -7; lists[i]->AddTail(first);
            actor->DecodeSwingSound(NULL, NULL);
            check("custom-empty-record-still-overwrites-and-keeps-frame", actor->field_7408 == "" && actor->field_740C == -7);
            lists[i]->RemoveAll();
        }
        actor->field_70FB = FALSE;
        for (INT i = 0; i < 3; ++i) {
            actor->m_animation.m_nSequence = sequences[i];
            actor->m_baseStats.m_speech[40 + i] = 0xFF000000;
            actor->field_7408 = "CANARY"; actor->field_740C = -123;
            actor->DecodeSwingSound(&weapon, NULL);
            check("empty-tlk-fallback-preserves-old-selection", actor->field_7408 == "CANARY" && actor->field_740C == -123);
            // Reuse the character's actual voice entry as the expected TLK sound.
            STR_RES entry;
            g_pBaldurChitin->GetTlkTable().Fetch(actor->m_baseStats.m_speech[9], entry);
            CString expected; entry.cSound.cResRef.CopyToString(expected);
            actor->m_baseStats.m_speech[40 + i] = actor->m_baseStats.m_speech[9];
            actor->DecodeSwingSound(&weapon, NULL);
            check("attack-sequence-selects-matching-tlk-slot", expected.IsEmpty()
                ? actor->field_7408 == "CANARY" && actor->field_740C == -123 : actor->field_7408 == expected && actor->field_740C == 0);
        }
        for (INT i = 0; i < 4; ++i) { lists[i]->RemoveAll(); for (const auto& entry : savedLists[i]) lists[i]->AddTail(entry); }
        for (INT i = 0; i < 3; ++i) actor->m_baseStats.m_speech[40 + i] = speech[i];
        actor->m_animation.m_nSequence = animationSequence; actor->m_nSequence = actorSequence;
        actor->m_derivedStats.m_generalState = savedState; actor->field_70FB = custom;
        actor->field_7408 = savedSound; actor->field_740C = savedFrame;
        weapon.pRes->m_pHeader->itemType = weaponType; launcher.pRes->m_pHeader->itemType = launcherType;
        weapon.Release(); launcher.Release();
        check("sound-checks-do-not-post-messages-or-advance-clock", messages.GetCount() == queued && game->GetWorldTimer()->m_gameTime == time);
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"weaponsoundchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "weapon sound mismatch (fixtures restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "itemreceivechecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        auto* remotes = game->GetRemoteObjectArray();
        LONG actorId = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS
            || raw == NULL || raw->GetObjectType() != CGameObject::TYPE_SPRITE) {
            if (raw != NULL) objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires party sprite"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        INT checks = 0; BOOL valid = TRUE;
        auto check = [&](const char* name, BOOL ok) {
            ++checks; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"item-receive-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        CNetwork& network = g_pChitin->cNetwork;
        auto* baldur = g_pBaldurChitin->GetBaldurMessage();
        auto* settings = game->GetMultiplayerSettings();
        INT provider = network.m_nServiceProvider;
        PLAYER_ID localPlayer = network.m_idLocalPlayer;
        BOOLEAN connected = network.m_bConnectionEstablished, hosting = network.m_bIsHost;
        BOOLEAN initialized = network.m_SystemWindow.m_bInitialized;
        BYTE blocked = game->field_4BD4;
        BOOLEAN returned = baldur->m_bDialogReplyReturned, pending = baldur->m_bDialogRequestPending;
        INT updates = baldur->m_nDialogReplyUpdates;
        PLAYER_ID ready[CNETWORK_MAX_PLAYERS];
        memcpy(ready, settings->m_pnPlayerReady, sizeof(ready));
        BOOLEAN windows[CNETWORK_MAX_PLAYERS];
        SimulatedTransportPause transportPause(network);
        EnterCriticalSection(&network.m_critSectQueue);
        network.m_idLocalPlayer = 0;
        for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) {
            windows[i] = network.m_pSlidingWindow[i].m_bInitialized;
            network.m_pSlidingWindow[i].m_bInitialized = FALSE;
        }
        auto& incoming = network.m_SystemWindow.m_lQueueIncomingMessages;
        std::list<CNETWORKWINDOW_QUEUEENTRY*> savedIncoming;
        while (!incoming.IsEmpty()) savedIncoming.push_back(incoming.RemoveHead());
        network.m_bIsHost = FALSE;
        network.m_SystemWindow.m_bInitialized = TRUE; network.m_bConnectionEstablished = TRUE;
        settings->m_pnPlayerReady[0] = network.m_idLocalPlayer;
        baldur->m_bDialogReplyReturned = FALSE;
        ULONG time = game->GetWorldTimer()->m_gameTime;
        auto& outgoing = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT queued = outgoing.GetCount();
        CItem* original = actor->m_equipment.m_items[18];
        actor->m_equipment.m_items[18] = NULL;
        game->m_disposableItemsCritSect.Lock();
        INT disposable = game->m_lDisposableItems.GetCount();
        PLAYER_ID player = actor->m_remotePlayerID;
        LONG remote = actor->m_remoteObjectID;
        remotes->m_critSect.Lock();
        CGameRemoteObjectListEntry mapping;
        mapping.remotePlayerID = network.m_idLocalPlayer ^ 0x89ABCDEF; mapping.remoteObjectID = 0x1234;
        mapping.localObjectID = actorId; mapping.pNext = remotes->m_pArray[0];
        remotes->m_pArray[0] = &mapping;
        actor->m_remotePlayerID = mapping.remotePlayerID; actor->m_remoteObjectID = mapping.remoteObjectID;
        CItem fixture(CResRef("ZRCARROW"), 1, 1, 1, 0, 1);
        fixture.m_nAbilities = 77; fixture.m_numSounds = 2;
        fixture.m_useCount1 = 0; fixture.m_useCount2 = 0x8000; fixture.m_useCount3 = 0xFFFF;
        fixture.m_wear = 0xBEEF; fixture.m_flags = 0xDEADBEEF;
        {
            CMessageAddItem copy(fixture, 0x12345678, actorId);
            check("constructor-resource-raw-words-and-flags", copy.m_item.cResRef == "ZRCARROW"
                && copy.m_item.m_useCount1 == 0 && copy.m_item.m_useCount2 == 0x8000
                && copy.m_item.m_useCount3 == 0xFFFF && copy.m_item.m_wear == 0xBEEF
                && copy.m_item.m_flags == 0xDEADBEEF && copy.GetSource() == 0x12345678 && copy.GetTarget() == actorId);
            check("constructor-does-not-copy-ability-or-sound-cache", copy.m_item.m_nAbilities == 0 && copy.m_item.m_numSounds == 0);
        }
        BYTE add[64] = {}, remove[64] = {};
        DWORD addSize = 0, removeSize = 0;
        BYTE* data = add + CNetwork::SPEC_MSG_HEADER_LENGTH;
        CMessageAddItem addMessage(fixture, 99, actorId);
        addMessage.MarshalMessage(&data, &addSize);
        data = remove + CNetwork::SPEC_MSG_HEADER_LENGTH;
        CMessageRemoveItem removeMessage(18, 99, actorId);
        removeMessage.MarshalMessage(&data, &removeSize);
        addSize += CNetwork::SPEC_MSG_HEADER_LENGTH; removeSize += CNetwork::SPEC_MSG_HEADER_LENGTH;
        for (BYTE* packet : {add, remove}) {
            packet[CNetwork::SPEC_MSG_FLAG] = CNetwork::SPEC_MSG_FLAG_ENABLED;
            packet[CNetwork::SPEC_MSG_TYPE] = CBaldurMessage::MSG_TYPE_CMESSAGE;
        }
        add[CNetwork::SPEC_MSG_SUBTYPE] = 2; remove[CNetwork::SPEC_MSG_SUBTYPE] = 34;
        auto receive = [&](BYTE* packet, DWORD size) {
            auto* node = new CNETWORKWINDOW_QUEUEENTRY;
            node->idFrom = 0xABCDEF12; node->idTo = network.m_idLocalPlayer;
            node->dwSize = CNetwork::dword_85E694 + size;
            node->pData = new BYTE[node->dwSize]; memset(node->pData, 0, node->dwSize);
            memcpy(node->pData + CNetwork::dword_85E698, (LPCTSTR)CNetwork::MG, CNetwork::dword_85E69C);
            memcpy(node->pData + CNetwork::dword_85E694, packet, size);
            incoming.AddTail(node);
        };
        auto cleanup = [&]() {
            delete actor->m_equipment.m_items[18]; actor->m_equipment.m_items[18] = NULL;
            while (game->m_lDisposableItems.GetCount() > disposable) delete game->m_lDisposableItems.RemoveTail();
            while (outgoing.GetCount() > queued) delete outgoing.RemoveTail();
        };
        {
            CMessageHandler handler;
            // Each condition independently selects immediate delivery. A
            // nonzero synchronization byte blocks both receipt and local Run.
            const INT providers[] = {CNetwork::SERV_PROV_NULL, 1, 1};
            const BYTE blocks[] = {255, 0, 255};
            for (INT mode = 0; mode < 3; ++mode) {
                network.m_nServiceProvider = providers[mode]; game->field_4BD4 = blocks[mode];
                receive(add, addSize); handler.AsynchronousUpdate();
                check("network-frame-consumed", incoming.IsEmpty());
                if (mode < 2) {
                    check("immediate-receive-delivers-item", handler.m_messageList.IsEmpty()
                        && actor->m_equipment.m_items[18] && actor->m_equipment.m_items[18]->cResRef == "ZRCARROW");
                } else {
                    auto* decoded = handler.m_messageList.IsEmpty() ? NULL
                        : dynamic_cast<CMessageAddItem*>(handler.m_messageList.GetHead());
                    check("deferred-receive-retains-decoded-message", handler.m_messageList.GetCount() == 1
                        && decoded && decoded->GetSource() == 0 && decoded->GetTarget() == actorId
                        && decoded->m_item.m_nAbilities == 0 && decoded->m_item.m_numSounds == 0
                        && actor->m_equipment.m_items[18] == NULL);
                    handler.AsynchronousUpdate();
                    check("blocked-local-queue-kept", handler.m_messageList.GetCount() == 1 && actor->m_equipment.m_items[18] == NULL);
                    game->field_4BD4 = 0; handler.AsynchronousUpdate();
                    check("unblocked-local-queue-delivers", handler.m_messageList.IsEmpty() && actor->m_equipment.m_items[18] != NULL);
                }
                receive(remove, removeSize); handler.AsynchronousUpdate();
                check("received-remove-retires-item", incoming.IsEmpty() && handler.m_messageList.IsEmpty()
                    && actor->m_equipment.m_items[18] == NULL && game->m_lDisposableItems.GetCount() == disposable + 1);
                cleanup();
            }
            network.m_nServiceProvider = 1; game->field_4BD4 = 1;
            receive(add, addSize); receive(remove, removeSize); handler.AsynchronousUpdate();
            check("deferred-pair-preserves-fifo-types", handler.m_messageList.GetCount() == 2
                && dynamic_cast<CMessageAddItem*>(handler.m_messageList.GetHead())
                && dynamic_cast<CMessageRemoveItem*>(handler.m_messageList.GetTail()) && actor->m_equipment.m_items[18] == NULL);
            game->field_4BD4 = 0; handler.AsynchronousUpdate();
            check("fifo-pair-adds-then-retires", handler.m_messageList.IsEmpty() && actor->m_equipment.m_items[18] == NULL
                && game->m_lDisposableItems.GetCount() == disposable + 1); cleanup();
            for (BYTE connection : {static_cast<BYTE>(0), static_cast<BYTE>(255)}) {
                network.m_bConnectionEstablished = connection; receive(add, addSize); handler.AsynchronousUpdate();
                check("connection-requires-exact-true", incoming.GetCount() == 1 && actor->m_equipment.m_items[18] == NULL);
                network.m_bConnectionEstablished = TRUE; handler.AsynchronousUpdate();
                check("connection-resumed-delivers", incoming.IsEmpty() && actor->m_equipment.m_items[18] != NULL); cleanup();
            }
            for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) settings->m_pnPlayerReady[i] = network.m_idLocalPlayer ^ 0xFFFFFFFF;
            receive(add, addSize); handler.AsynchronousUpdate();
            check("not-ready-discards-inventory-packet", incoming.IsEmpty() && actor->m_equipment.m_items[18] == NULL && handler.m_messageList.IsEmpty());
            settings->m_pnPlayerReady[0] = network.m_idLocalPlayer;
            PLAYER_ID missing = 0x76543210;
            for (BYTE* packet : {add, remove}) {
                memcpy(packet + CNetwork::SPEC_MSG_HEADER_LENGTH, &missing, sizeof(missing));
                for (BYTE block : {static_cast<BYTE>(0), static_cast<BYTE>(1)}) {
                    game->field_4BD4 = block;
                    receive(packet, CNetwork::SPEC_MSG_HEADER_LENGTH + 8); handler.AsynchronousUpdate();
                    check("unmapped-header-only-fails-without-queue-or-run", incoming.IsEmpty()
                        && handler.m_messageList.IsEmpty() && actor->m_equipment.m_items[18] == NULL);
                }
            }
            network.m_bConnectionEstablished = FALSE;
            for (BYTE flag : {static_cast<BYTE>(0), static_cast<BYTE>(1), static_cast<BYTE>(255)}) {
                baldur->m_bDialogReplyReturned = flag;
                check("reply-getter-retains-raw-byte", baldur->GetDialogReplyReturned() == flag);
                baldur->m_bDialogRequestPending = TRUE; baldur->m_nDialogReplyUpdates = 7;
                handler.AsynchronousUpdate();
                check("reply-countdown-requires-exact-true", baldur->m_nDialogReplyUpdates == (flag == TRUE ? 6 : 7));
            }
            baldur->m_bDialogRequestPending = 255; baldur->m_bDialogReplyReturned = TRUE; baldur->m_nDialogReplyUpdates = 7;
            handler.AsynchronousUpdate(); check("reply-pending-requires-exact-true", baldur->m_nDialogReplyUpdates == 7);
            baldur->m_bDialogRequestPending = TRUE; baldur->m_nDialogReplyUpdates = INT_MIN;
            handler.AsynchronousUpdate(); check("reply-countdown-wraps-full-width", baldur->m_nDialogReplyUpdates == INT_MAX);
            baldur->m_nDialogReplyUpdates = INT_MAX;
            handler.AsynchronousUpdate(); check("reply-countdown-keeps-high-word", baldur->m_nDialogReplyUpdates == INT_MAX - 1);
        }
        struct OwnedMessage : CMessage {
            CMessageHandler* handler; INT* destroyed; INT* ran; BOOL* order;
            OwnedMessage(CMessageHandler* h, INT* d, INT* r, BOOL* o) : CMessage(0, 0), handler(h), destroyed(d), ran(r), order(o) {}
            ~OwnedMessage() override { *order = *order && handler->m_messageList.GetCount() == 3; ++*destroyed; }
            void Run() override { ++*ran; }
        };
        INT destroyed = 0, ran = 0; BOOL order = TRUE;
        {
            CMessageHandler handler;
            handler.m_messageList.AddTail(new OwnedMessage(&handler, &destroyed, &ran, &order));
            handler.m_messageList.AddTail(static_cast<CMessage*>(NULL));
            handler.m_messageList.AddTail(new OwnedMessage(&handler, &destroyed, &ran, &order));
        }
        check("handler-destructor-owns-messages-without-running-or-unlinking-early", destroyed == 2 && ran == 0 && order);
        cleanup(); actor->m_equipment.m_items[18] = original;
        game->m_disposableItemsCritSect.Unlock();
        remotes->m_pArray[0] = mapping.pNext; actor->m_remotePlayerID = player; actor->m_remoteObjectID = remote;
        remotes->m_critSect.Unlock();
        while (!incoming.IsEmpty()) { auto* node = incoming.RemoveHead(); delete node->pData; delete node; }
        for (auto* node : savedIncoming) incoming.AddTail(node);
        for (INT i = 0; i < CNETWORK_MAX_PLAYERS; ++i) network.m_pSlidingWindow[i].m_bInitialized = windows[i];
        network.m_nServiceProvider = provider; network.m_bConnectionEstablished = connected;
        network.m_idLocalPlayer = localPlayer;
        network.m_bIsHost = hosting; network.m_SystemWindow.m_bInitialized = initialized;
        memcpy(settings->m_pnPlayerReady, ready, sizeof(ready)); game->field_4BD4 = blocked;
        baldur->m_bDialogReplyReturned = returned; baldur->m_bDialogRequestPending = pending; baldur->m_nDialogReplyUpdates = updates;
        LeaveCriticalSection(&network.m_critSectQueue);
        check("queue-disposal-and-clock-restored", outgoing.GetCount() == queued && game->m_lDisposableItems.GetCount() == disposable
            && game->GetWorldTimer()->m_gameTime == time);
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"itemreceivechecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "item receive mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "itempacketchecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        auto* remotes = game->GetRemoteObjectArray();
        LONG actorId = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS
            || raw == NULL || raw->GetObjectType() != CGameObject::TYPE_SPRITE) {
            if (raw != NULL) objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires party sprite"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        INT checks = 0; BOOL valid = TRUE;
        auto check = [&](const char* name, BOOL ok) {
            checks++; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"item-packet-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        PLAYER_ID oldPlayer = actor->m_remotePlayerID;
        LONG oldObject = actor->m_remoteObjectID;
        actor->m_remotePlayerID = 0x89ABCDEF; actor->m_remoteObjectID = 0x1234;
        remotes->m_critSect.Lock();
        CGameRemoteObjectListEntry mapping;
        mapping.remotePlayerID = actor->m_remotePlayerID; mapping.remoteObjectID = actor->m_remoteObjectID;
        mapping.localObjectID = actorId; mapping.pNext = remotes->m_pArray[0];
        remotes->m_pArray[0] = &mapping;
        const DWORD header = CNetwork::SPEC_MSG_HEADER_LENGTH;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT queued = messages.GetCount();
        ULONG time = game->GetWorldTimer()->m_gameTime;
        CItem fixture(CResRef("ZRCARROW"), 1, 1, 1, 0, 1);
        fixture.Demand();
        ITEM_HEADER* itemHeader = fixture.pRes->m_pHeader;
        WORD abilities = itemHeader->abilityCount, stack = itemHeader->maxStackable, lore = itemHeader->loreValue;
        itemHeader->abilityCount = 3; itemHeader->maxStackable = 2;
        CMessageAddItem sending(fixture, 99, actorId);
        sending.m_item.m_useCount1 = 0x1234; sending.m_item.m_useCount2 = 0x8000; sending.m_item.m_useCount3 = 0xFFFF;
        sending.m_item.m_wear = 0xFFFF; sending.m_item.m_flags = 0xDEADBEEF;
        const BYTE golden[] = {0xEF,0xCD,0xAB,0x89,0x34,0x12,0,0,8,'Z','R','C','A','R','R','O','W',
            0x34,0x12,0,0x80,0xFF,0xFF,0xFF,0xFF,0xEF,0xBE,0xAD,0xDE};
        BYTE bytes[64]; memset(bytes, 0xA5, sizeof(bytes));
        BYTE* payload = bytes + header; DWORD length = 0;
        CMessage* virtualMessage = &sending;
        virtualMessage->MarshalMessage(&payload, &length);
        check("add-golden-packet-and-guards", length == sizeof(golden) && payload == bytes + header
            && memcmp(payload, golden, sizeof(golden)) == 0 && bytes[0] == 0xA5 && payload[length] == 0xA5);
        for (const char* name : {"", "A", "ZRCBOLT", "ZRCARROW"}) {
            sending.m_item.cResRef = name;
            memset(bytes, 0xA5, sizeof(bytes)); length = 0;
            virtualMessage->MarshalMessage(&payload, &length);
            DWORD nameLength = static_cast<DWORD>(strlen(name));
            WORD uses[3]; memcpy(uses, payload + 9 + nameLength, sizeof(uses));
            check("add-resource-length-and-no-terminator", length == 21 + nameLength && payload[8] == nameLength
                && memcmp(payload + 9, name, nameLength) == 0 && payload[length] == 0xA5);
            check("add-usage-count-api", uses[0] == (nameLength ? 0x1234 : 0)
                && uses[1] == (nameLength ? 0x8000 : 0) && uses[2] == (nameLength ? 0xFFFF : 0));
        }
        sending.m_item.cResRef = "ZRCARROW";
        for (LONG target : {-1L, 0x7FFF0001L}) {
            sending.m_targetId = target; memset(bytes, 0xA5, sizeof(bytes)); length = 999;
            virtualMessage->MarshalMessage(&payload, &length);
            check("add-missing-target-zero-size-untouched-buffer", length == 0 && payload == bytes + header && payload[0] == 0xA5);
            CMessageRemoveItem removal(18, 99, target); length = 999;
            static_cast<CMessage*>(&removal)->MarshalMessage(&payload, &length);
            check("remove-missing-target-zero-size-untouched-buffer", length == 0 && payload[0] == 0xA5);
        }
        sending.m_targetId = actorId;
        for (SHORT slot : {static_cast<SHORT>(0), static_cast<SHORT>(18), static_cast<SHORT>(50),
                 static_cast<SHORT>(-1), static_cast<SHORT>(SHORT_MIN), static_cast<SHORT>(SHORT_MAX)}) {
            memset(bytes, 0xA5, sizeof(bytes));
            CMessageRemoveItem removal(slot, 99, actorId);
            static_cast<CMessage*>(&removal)->MarshalMessage(&payload, &length);
            SHORT wireSlot; memcpy(&wireSlot, payload + 8, sizeof(wireSlot));
            check("remove-signed-short-packet", length == 10 && memcmp(payload, golden, 8) == 0
                && wireSlot == slot && payload[10] == 0xA5 && bytes[0] == 0xA5);
            CMessageRemoveItem received(7, 88, -1);
            check("remove-decode-mapped", static_cast<CMessage*>(&received)->UnmarshalMessage(bytes, header + length)
                && received.m_targetId == actorId && received.m_sourceId == 88 && received.m_slot == slot);
        }
        for (const char* name : {"ZRCARROW", "", "NO_DROP"}) {
            for (WORD use : {static_cast<WORD>(0), static_cast<WORD>(1), static_cast<WORD>(7),
                     static_cast<WORD>(32768), static_cast<WORD>(65535)}) {
                for (DWORD flags : {0UL, 1UL, 0x80000000UL, 0xFFFFFFFFUL}) {
                    for (WORD loreValue : {static_cast<WORD>(0), static_cast<WORD>(100)}) {
                        itemHeader->loreValue = loreValue;
                        memcpy(payload, golden, 8); DWORD cnt = 8;
                        BYTE nameLength = static_cast<BYTE>(strlen(name)); payload[cnt++] = nameLength;
                        memcpy(payload + cnt, name, nameLength); cnt += nameLength;
                        WORD uses[] = {use, static_cast<WORD>(65535 - use), static_cast<WORD>(use ^ 0x8000)};
                        memcpy(payload + cnt, uses, sizeof(uses)); cnt += sizeof(uses);
                        WORD wear = use; memcpy(payload + cnt, &wear, sizeof(wear)); cnt += sizeof(wear);
                        memcpy(payload + cnt, &flags, sizeof(flags)); cnt += sizeof(flags);
                        CMessageAddItem received(fixture, 88, -1);
                        received.m_item.m_nAbilities = 77;
                        check("add-decode-mapped", static_cast<CMessage*>(&received)->UnmarshalMessage(bytes, header + cnt));
                        BOOL bound = nameLength && strcmp(name, "NO_DROP") != 0;
                        DWORD normalized = flags;
                        if (!bound || loreValue == 0) normalized |= 1;
                        if (!(normalized & 1)) normalized |= 8;
                        check("add-constructor-normalizes-resource-charges-and-flags", received.m_targetId == actorId
                            && received.m_sourceId == 88 && received.m_item.cResRef == (bound ? "ZRCARROW" : "")
                            && received.m_item.m_useCount1 == (bound ? max(uses[0], 1) : uses[0])
                            && received.m_item.m_useCount2 == (bound ? max(uses[1], 1) : uses[1])
                            && received.m_item.m_useCount3 == (bound ? max(uses[2], 1) : uses[2])
                            && received.m_item.m_wear == wear && received.m_item.m_flags == normalized
                            && received.m_item.m_nAbilities == 77);
                    }
                }
            }
        }
        CMessageAddItem rejected(fixture, 88, actorId);
        rejected.m_item.m_useCount1 = 9; rejected.m_item.m_useCount2 = 8; rejected.m_item.m_useCount3 = 7;
        rejected.m_item.m_wear = 6; rejected.m_item.m_flags = 5;
        CMessageRemoveItem rejectedRemove(18, 88, actorId);
        PLAYER_ID missing = 0x76543210; memcpy(payload, &missing, sizeof(missing));
        check("unmapped-header-only-add-rejected", !static_cast<CMessage*>(&rejected)->UnmarshalMessage(bytes, header + 8));
        check("unmapped-add-leaves-all-state", rejected.m_targetId == actorId && rejected.m_sourceId == 88
            && rejected.m_item.cResRef == "ZRCARROW" && rejected.m_item.m_useCount1 == 9 && rejected.m_item.m_useCount2 == 8
            && rejected.m_item.m_useCount3 == 7 && rejected.m_item.m_wear == 6 && rejected.m_item.m_flags == 5);
        check("unmapped-header-only-remove-rejected", !static_cast<CMessage*>(&rejectedRemove)->UnmarshalMessage(bytes, header + 8));
        check("unmapped-remove-leaves-all-state", rejectedRemove.m_targetId == actorId && rejectedRemove.m_sourceId == 88 && rejectedRemove.m_slot == 18);
        itemHeader->abilityCount = abilities; itemHeader->maxStackable = stack; itemHeader->loreValue = lore;
        fixture.Release();
        CItem* originalItem = actor->m_equipment.m_items[18];
        actor->m_equipment.m_items[18] = NULL;
        game->m_disposableItemsCritSect.Lock();
        INT disposableCount = game->m_lDisposableItems.GetCount();
        memcpy(payload, golden, sizeof(golden));
        CMessageAddItem delivered(fixture, 88, -1);
        BOOL decoded = static_cast<CMessage*>(&delivered)->UnmarshalMessage(bytes, header + sizeof(golden));
        if (decoded) static_cast<CMessage*>(&delivered)->Run();
        check("decoded-add-delivers-to-real-inventory", decoded && actor->m_equipment.m_items[18] != NULL
            && actor->m_equipment.m_items[18]->cResRef == "ZRCARROW"
            && actor->m_equipment.m_items[18]->m_useCount1 == 0x1234 && actor->m_equipment.m_items[18]->m_wear == 0xFFFF);
        CMessageRemoveItem outgoingRemove(18, actorId, actorId);
        static_cast<CMessage*>(&outgoingRemove)->MarshalMessage(&payload, &length);
        CMessageRemoveItem deliveredRemove(7, 88, -1);
        decoded = static_cast<CMessage*>(&deliveredRemove)->UnmarshalMessage(bytes, header + length);
        if (decoded) static_cast<CMessage*>(&deliveredRemove)->Run();
        check("decoded-remove-retires-delivered-item", decoded && actor->m_equipment.m_items[18] == NULL
            && game->m_lDisposableItems.GetCount() == disposableCount + 1);
        delete actor->m_equipment.m_items[18];
        actor->m_equipment.m_items[18] = originalItem;
        while (game->m_lDisposableItems.GetCount() > disposableCount) delete game->m_lDisposableItems.RemoveTail();
        game->m_disposableItemsCritSect.Unlock();
        while (messages.GetCount() > queued) delete messages.RemoveTail();
        check("no-message-or-clock-side-effects", messages.GetCount() == queued && game->GetWorldTimer()->m_gameTime == time);
        remotes->m_pArray[0] = mapping.pNext;
        actor->m_remotePlayerID = oldPlayer; actor->m_remoteObjectID = oldObject;
        remotes->m_critSect.Unlock();
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"itempacketchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "item packet mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "doortogglechecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG actorId = game->GetFixedOrderCharacterId(0);
        CGameObject* raw = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires loaded party"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        CGameDoor* door = NULL;
        for (INT i = 0; i < objects->m_nArraySize && door == NULL; i++) {
            auto& entry = objects->m_pArray[i];
            if (entry.m_objectId == -1 || entry.m_objectPtr == NULL || entry.m_objectPtr->GetObjectType() != CGameObject::TYPE_DOOR
                || entry.m_objectPtr->m_pArea != actor->m_pArea) continue;
            CGameObject* object = NULL;
            if (objects->GetDeny(entry.m_objectPtr->m_id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS) door = static_cast<CGameDoor*>(object);
        }
        if (door == NULL) {
            objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires local door"); return;
        }
        auto* area = door->m_pArea;
        auto flags = door->m_dwFlags;
        auto pos = door->m_pos, openDest = door->m_ptOpenDest, closedDest = door->m_ptClosedDest;
        auto tile = door->m_tiledObject.m_wAIState;
        auto key = door->m_keyType, openSound = door->m_openSound, closeSound = door->m_closeSound;
        auto str = door->m_strNotPickable;
        auto activated = door->m_trapActivated;
        auto* openSearch = door->m_pOpenSearch;
        auto* closedSearch = door->m_pClosedSearch;
        auto openCount = door->m_nOpenSearch, closedCount = door->m_nClosedSearch;
        auto sound = door->m_sndDoor.GetResRef();
        auto channel = door->m_sndDoor.m_nChannel, priority = door->m_sndDoor.m_nPriority;
        auto soundArea = door->m_sndDoor.m_nArea;
        auto soundX = door->m_sndDoor.m_nXCoordinate, soundY = door->m_sndDoor.m_nYCoordinate, soundZ = door->m_sndDoor.m_nZCoordinate;
        auto action = door->m_curAction;
        auto action3 = door->m_curAction.m_specificID3;
        auto gridSize = area->m_search.m_GridSquareDimensions;
        ULONG time = game->GetWorldTimer()->m_gameTime;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT messageCount = messages.GetCount(), triggerCount = door->m_pendingTriggers.GetCount();
        INT checks = 0;
        BOOL valid = TRUE;
        auto check = [&](const char* name, BOOL ok) {
            checks++; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"door-toggle-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        auto discard = [&]() {
            while (messages.GetCount() > messageCount) delete messages.RemoveTail();
            while (door->m_pendingTriggers.GetCount() > triggerCount) delete door->m_pendingTriggers.RemoveTail();
        };
        auto reset = [&](DWORD bits) {
            discard(); door->m_sndDoor.Stop(); door->m_dwFlags = bits;
            door->m_trapActivated = 0; door->m_keyType = ""; door->m_strNotPickable = -1;
            door->m_pOpenSearch = door->m_pClosedSearch = NULL;
            door->m_nOpenSearch = door->m_nClosedSearch = 0;
            door->m_ptOpenDest = CPoint(111, 112); door->m_ptClosedDest = CPoint(211, 212);
            door->m_openSound = door->m_closeSound = "";
        };
        area->m_search.m_GridSquareDimensions = CSize(20, 20);
        for (BYTE space : {static_cast<BYTE>(0), static_cast<BYTE>(1), static_cast<BYTE>(2), static_cast<BYTE>(3), static_cast<BYTE>(5)}) {
            int radius = (static_cast<int>(space) - 1) / 2;
            for (LONG dx : {-3L, -2L, -1L, 0L, 1L, 2L, 3L}) {
                CPoint point(10 + dx, 10);
                check("door-overlap-radius", area->m_search.IsOverDoor(CPoint(10, 10), space, &point, 1) == (dx >= -radius && dx <= radius));
            }
        }
        CPoint point(0, 0);
        for (CPoint origin : {CPoint(-1, 0), CPoint(0, -1), CPoint(320, 0), CPoint(0, 320)}) check("door-overlap-hard-limit", !area->m_search.IsOverDoor(origin, 255, &point, 1));
        check("door-overlap-empty", !area->m_search.IsOverDoor(CPoint(0, 0), 1, NULL, 0));
        area->m_search.m_GridSquareDimensions = CSize(2, 2);
        point = CPoint(3, 0);
        check("door-overlap-asymmetric-negative-clipping", area->m_search.IsOverDoor(CPoint(0, 0), 7, &point, 1));
        point = CPoint(3, 1);
        check("door-overlap-positive-clipping", !area->m_search.IsOverDoor(CPoint(1, 1), 3, &point, 1));
        area->m_search.m_GridSquareDimensions = gridSize;
        check("occupant-missing-door", area->FindDoorOccupant(CGameObjectArray::INVALID_INDEX, NULL, 0) == NULL);
        check("occupant-empty-footprint", area->FindDoorOccupant(door->m_id, NULL, 0) == NULL);
        for (DWORD bits : {0UL, 1UL, 0x80UL, 0x81UL, 0x400UL, 0x401UL}) {
            for (WORD trap : {static_cast<WORD>(0), static_cast<WORD>(1), static_cast<WORD>(65535)}) {
                for (DWORD repeat : {0UL, 4UL}) {
                    reset(bits | repeat); door->m_trapActivated = trap;
                    door->ToggleDoor(actor->GetAIType(), FALSE);
                    BOOL opening = !(bits & 1);
                    check("toggle-state-and-destination", (door->m_dwFlags & 1) == (opening ? 1 : 0)
                        && door->m_pos == (opening ? CPoint(211, 212) : CPoint(111, 112))
                        && door->m_tiledObject.m_wAIState == (opening ? CTiledObject::STATE_PRIMARY_TILE : CTiledObject::STATE_SECONDARY_TILE));
                    check("toggle-trap-word-and-repeat", door->m_trapActivated == ((repeat || !trap) ? trap : 0)
                        && door->m_pendingTriggers.GetCount() == triggerCount + (trap ? 1 : 0));
                    if (trap) {
                        auto* trigger = door->m_pendingTriggers.GetTail();
                        check("toggle-trigger-cause", trigger->m_triggerID == (opening ? CAITRIGGER_OPENED : CAITRIGGER_CLOSED)
                            && trigger->m_triggerCause.m_nInstance == actorId);
                    }
                    check("toggle-sound-channel", door->m_sndDoor.m_nChannel == 2 && door->m_sndDoor.m_nArea == reinterpret_cast<DWORD>(area));
                }
            }
        }
        for (DWORD bits : {2UL, 3UL, 0x202UL}) {
            reset(bits); door->ToggleDoor(actor->GetAIType(), FALSE);
            check("locked-without-key", door->m_dwFlags == bits && messages.GetCount() == messageCount + 1
                && messages.GetTail()->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXTREF_SEND);
            reset(bits); door->ToggleDoor(actor->GetAIType(), TRUE);
            check("force-unlocks-and-toggles", !(door->m_dwFlags & 2) && (door->m_dwFlags & 1) == ((bits & 1) ? 0 : 1));
        }
        reset(0x800); door->ToggleDoor(actor->GetAIType(), TRUE);
        check("special-first-open-refusal", door->m_dwFlags == 0x1800 && messages.GetCount() == messageCount);
        door->ToggleDoor(actor->GetAIType(), TRUE);
        check("special-second-open", door->m_dwFlags == 0x1801);
        for (BYTE requested : {static_cast<BYTE>(0), static_cast<BYTE>(1), static_cast<BYTE>(255)}) {
            for (DWORD opened : {0UL, 1UL}) {
                reset(opened);
                CMessageDoorOpen message(requested, actorId, door->m_id);
                message.Run();
                check("open-message-preserves-raw-byte", (door->m_dwFlags & 1) == (opened == requested ? opened : opened ^ 1));
            }
        }
        reset(0);
        CMessageDoorOpen missingSource(TRUE, CGameObjectArray::INVALID_INDEX, door->m_id); missingSource.Run();
        check("open-message-missing-source", door->m_dwFlags == 0);
        CMessageDoorOpen missingTarget(TRUE, actorId, CGameObjectArray::INVALID_INDEX); missingTarget.Run();
        check("open-message-missing-target", door->m_dwFlags == 0);
        reset(2);
        auto lockDifficulty = door->m_lockDifficulty;
        door->m_lockDifficulty = 25;
        auto* knock = new CGameEffect(); knock->m_effectID = CGAMEEFFECT_KNOCK; knock->m_sourceID = actorId;
        door->AddEffect(knock, 0, TRUE, TRUE);
        if (messages.GetCount() == messageCount + 1 && dynamic_cast<CMessageDoorOpen*>(messages.GetTail()) != NULL) {
            CMessage* request = messages.RemoveTail(); request->Run(); delete request;
            check("knock-dooropen-toggle-chain", door->m_dwFlags == 1);
        } else check("knock-dooropen-toggle-chain", FALSE);
        door->m_lockDifficulty = lockDifficulty;
        CPoint masks[2] = {CPoint(0, 0), CPoint(1, 0)};
        BYTE oldMask[2] = {area->m_search.m_pDynamicCost[0], area->m_search.m_pDynamicCost[1]};
        for (DWORD opaque : {0UL, 0x400UL}) {
            reset(opaque);
            door->m_pClosedSearch = &masks[0]; door->m_nClosedSearch = 1;
            door->m_pOpenSearch = &masks[1]; door->m_nOpenSearch = 1;
            area->m_search.m_pDynamicCost[0] = 0x80; area->m_search.m_pDynamicCost[1] = 0;
            door->ToggleDoor(actor->GetAIType(), FALSE);
            check("opening-moves-search-mask", area->m_search.m_pDynamicCost[0] == 0 && area->m_search.m_pDynamicCost[1] == (opaque ? 0x81 : 0x80));
            door->ToggleDoor(actor->GetAIType(), FALSE);
            check("closing-moves-search-mask", area->m_search.m_pDynamicCost[0] == (opaque ? 0x81 : 0x80) && area->m_search.m_pDynamicCost[1] == 0);
        }
        area->m_search.m_pDynamicCost[0] = oldMask[0]; area->m_search.m_pDynamicCost[1] = oldMask[1];
        CPoint occupied(actor->m_pos.x / 16, actor->m_pos.y / 12);
        LONG cell = occupied.y * gridSize.cx + occupied.x;
        BYTE cost = area->m_search.m_pDynamicCost[cell];
        area->m_search.m_pDynamicCost[cell] |= 2;
        reset(1); door->m_pClosedSearch = &occupied; door->m_nClosedSearch = 1;
        door->ToggleDoor(actor->GetAIType(), FALSE);
        check("occupied-close-remains-open", door->m_dwFlags == 1);
        CGameObject* occupant = area->FindDoorOccupant(door->m_id, &occupied, 1);
        check("occupied-footprint-finds-sprite", occupant != NULL && occupant->GetObjectType() == CGameObject::TYPE_SPRITE);
        if (occupant != NULL) objects->ReleaseShare(occupant->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        area->m_search.m_pDynamicCost[cell] = cost;
        reset(0);
        CItem* equipment[51];
        memcpy(equipment, actor->m_equipment.m_items, sizeof(equipment));
        memset(actor->m_equipment.m_items, 0, sizeof(equipment));
        CButtonData quick[8]; BYTE quickAmmo[8];
        for (int i = 0; i < 8; i++) quick[i] = actor->m_quickWeapons[i];
        memcpy(quickAmmo, actor->field_3D3A, sizeof(quickAmmo));
        game->m_disposableItemsCritSect.Lock();
        int disposables = game->m_lDisposableItems.GetCount();
        auto inventoryCleanup = [&]() {
            discard();
            for (int i = 0; i < 51; i++) { delete actor->m_equipment.m_items[i]; actor->m_equipment.m_items[i] = NULL; }
            while (game->m_lDisposableItems.GetCount() > disposables) delete game->m_lDisposableItems.RemoveTail();
        };
        auto runInventory = [&]() {
            while (messages.GetCount() > messageCount) {
                POSITION at = messages.FindIndex(messageCount);
                auto* message = messages.GetAt(at); messages.RemoveAt(at);
                if (dynamic_cast<CMessageRemoveItem*>(message) != NULL || dynamic_cast<CMessageAddItem*>(message) != NULL) message->Run();
                delete message;
            }
        };
        CItem packedItem;
        check("item-packed-tail", sizeof(CItem) == reinterpret_cast<const BYTE*>(&packedItem.m_numSounds) - reinterpret_cast<const BYTE*>(&packedItem) + sizeof(packedItem.m_numSounds));
        check("place-null", actor->PlaceItem(NULL, TRUE, TRUE, 5, TRUE));
        CItem* item = new CItem(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
        check("place-party-inventory", actor->PlaceItem(item, TRUE, FALSE, 1, FALSE) && actor->m_equipment.m_items[18] == item && (item->m_flags & 2));
        check("find-item-case-insensitive", actor->FindItemPersonal("zrcmelee", 0, FALSE) == 18);
        check("find-item-missing-sentinel", actor->FindItemPersonal("ZRNOITEM", 0, FALSE) == -1 && actor->FindItemPersonal("ZRCMELEE", 0, TRUE) == -1);
        CMessageRemoveItem remove(18, door->m_id, actorId); remove.Run();
        check("remove-item-runs", actor->m_equipment.m_items[18] == NULL && game->m_lDisposableItems.GetCount() == disposables + 1);
        discard();
        CItem fixture(CResRef("ZRCMELEE"), 1, 0, 0, 0, 0);
        CMessageAddItem add(fixture, door->m_id, actorId); add.Run();
        check("add-item-runs", actor->m_equipment.m_items[18] != NULL && actor->m_equipment.m_items[18]->GetResRef() == CResRef("ZRCMELEE"));
        discard();
        door->m_keyType = "ZRCMELEE"; door->m_dwFlags = 0x4002;
        door->ToggleDoor(actor->GetAIType(), FALSE);
        check("key-unlocks-opens-and-enqueues-removal", door->m_dwFlags == 0x4001 && door->m_curAction.m_actionID == 116 && door->m_curAction.GetString1() == "ZRCMELEE");
        runInventory();
        check("key-consumed-from-party", actor->m_equipment.m_items[18] == NULL);
        inventoryCleanup();
        check("place-party-recursive-count", actor->PlaceItem(new CItem(fixture), TRUE, FALSE, 3, FALSE)
            && actor->m_equipment.m_items[18] != NULL && actor->m_equipment.m_items[19] != NULL && actor->m_equipment.m_items[20] != NULL);
        check("find-item-ordinal", actor->FindItemPersonal("ZRCMELEE", 1, FALSE) == 19 && actor->FindItemPersonal("ZRCMELEE", 3, FALSE) == -1);
        door->m_curAction.m_actionID = 188; door->m_curAction.SetString1("ZRCMELEE");
        check("take-all-return", door->TakePartyItem() == CGameAIBase::ACTION_DONE);
        runInventory();
        check("take-all-removes-each-match", actor->m_equipment.m_items[18] == NULL && actor->m_equipment.m_items[19] == NULL && actor->m_equipment.m_items[20] == NULL);
        inventoryCleanup();
        actor->m_equipment.m_items[18] = new CItem(CResRef("ZRCARROW"), 7, 0, 0, 0, 0);
        door->m_curAction.m_actionID = 204; door->m_curAction.m_specificID = 3; door->m_curAction.SetString1("ZRCARROW");
        check("take-number-partial-return", door->TakePartyItem() == CGameAIBase::ACTION_DONE);
        runInventory();
        check("take-number-retains-stack-remainder", actor->m_equipment.m_items[18] != NULL && actor->m_equipment.m_items[18]->GetUsageCount(0) == 4);
        inventoryCleanup();
        SHORT portrait = game->GetCharacterPortraitNum(actorId);
        if (portrait != -1) {
            LONG portraitId = game->m_characterPortraits[portrait];
            game->m_characterPortraits[portrait] = CGameObjectArray::INVALID_INDEX;
            CItem* direct = new CItem(fixture);
            check("nonparty-direct-slot", actor->PlaceItem(direct, TRUE, FALSE, 1, FALSE) && actor->m_equipment.m_items[42] == direct);
            inventoryCleanup();
            CItem* acquired = new CItem(fixture);
            check("nonparty-acquired-slot-quirk", actor->PlaceItem(acquired, FALSE, FALSE, 1, FALSE) && actor->m_equipment.m_items[45] == acquired);
            inventoryCleanup();
            game->m_characterPortraits[portrait] = portraitId;
        }
        actor->UpdateQuickWeaponData();
        check("quick-weapon-empty-refresh", actor->m_quickWeapons[1].m_abilityId.m_itemNum == -1 && actor->field_3D3A[1] == 0);
        inventoryCleanup();
        memcpy(actor->m_equipment.m_items, equipment, sizeof(equipment));
        for (int i = 0; i < 8; i++) actor->m_quickWeapons[i] = quick[i];
        memcpy(actor->field_3D3A, quickAmmo, sizeof(quickAmmo));
        game->m_disposableItemsCritSect.Unlock();
        reset(0); door->m_dwFlags = flags; door->m_pos = pos;
        door->m_ptOpenDest = openDest; door->m_ptClosedDest = closedDest;
        door->m_tiledObject.m_wAIState = tile; door->m_keyType = key; door->m_strNotPickable = str;
        door->m_trapActivated = activated;
        door->m_pOpenSearch = openSearch; door->m_pClosedSearch = closedSearch;
        door->m_nOpenSearch = openCount; door->m_nClosedSearch = closedCount;
        door->m_openSound = openSound; door->m_closeSound = closeSound;
        door->m_sndDoor.SetResRef(sound, TRUE, TRUE); door->m_sndDoor.m_nChannel = channel;
        door->m_sndDoor.m_nPriority = priority; door->m_sndDoor.m_nArea = soundArea;
        door->m_sndDoor.m_nXCoordinate = soundX; door->m_sndDoor.m_nYCoordinate = soundY; door->m_sndDoor.m_nZCoordinate = soundZ;
        door->m_curAction = action; door->m_curAction.m_specificID3 = action3;
        check("world-time-unchanged", game->GetWorldTimer()->m_gameTime == time);
        objects->ReleaseDeny(door->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"doortogglechecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "door toggle/inventory mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "dooreffectchecks") == 0) {
        auto* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        auto* remote = game->GetRemoteObjectArray();
        CGameObject* raw = NULL;
        LONG actorId = game->GetFixedOrderCharacterId(0);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(actorId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires loaded party"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        CGameDoor* door = NULL;
        for (INT i = 0; i < objects->m_nArraySize && door == NULL; ++i) {
            auto& entry = objects->m_pArray[i];
            if (entry.m_objectId == -1 || entry.m_objectPtr == NULL
                || entry.m_objectPtr->GetObjectType() != CGameObject::TYPE_DOOR
                || entry.m_objectPtr->m_pArea != actor->m_pArea) continue;
            CGameObject* object = NULL;
            if (objects->GetDeny(entry.m_objectPtr->GetId(), CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS)
                door = static_cast<CGameDoor*>(object);
        }
        if (door == NULL) {
            objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires local door"); return;
        }
        auto flags = door->m_dwFlags;
        auto difficulty = door->m_lockDifficulty;
        auto detectionDifficulty = door->m_trapDetectionDifficulty;
        auto activated = door->m_trapActivated, detected = door->m_trapDetected;
        auto draw = door->m_drawPoly;
        auto drawMessage = g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly;
        auto actorPlayer = actor->m_remotePlayerID, doorPlayer = door->m_remotePlayerID;
        auto actorRemote = actor->m_remoteObjectID, doorRemote = door->m_remoteObjectID;
        ULONG time = game->GetWorldTimer()->m_gameTime;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT messageCount = messages.GetCount();
        INT checks = 0;
        BOOL valid = TRUE;
        auto check = [&](const char* name, BOOL ok) {
            ++checks; valid = valid && ok;
            if (!ok) Emit("{\"step\":%d,\"op\":\"door-effect-case\",\"case\":\"%s\",\"passed\":false}", nStep, name);
        };
        auto discard = [&]() { while (messages.GetCount() > messageCount) delete messages.RemoveTail(); };
        auto effect = [&](DWORD opcode) {
            auto* result = new CGameEffect();
            result->m_effectID = opcode;
            result->m_sourceID = actorId;
            return result;
        };
        CGameAIBase* target = door;
        for (DWORD lock : {0UL, 99UL, 100UL, 101UL, 0xFFFFFFFFUL}) {
            for (DWORD bits : {0UL, 1UL, 2UL, 3UL, 0xFFFFFFFFUL}) {
                discard(); door->m_dwFlags = bits; door->m_lockDifficulty = lock;
                target->AddEffect(effect(CGAMEEFFECT_KNOCK), 255, FALSE, FALSE);
                check("knock-lock-sentinel", door->m_dwFlags == (lock == 100 ? bits : bits & ~2U));
                if (lock != 100) {
                    check("knock-opens-with-distinct-message", messages.GetCount() == messageCount + 1
                        && messages.GetTail()->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_OPEN
                        && static_cast<CMessageDoorOpen*>(messages.GetTail())->m_bOpenDoor == TRUE
                        && messages.GetTail()->m_sourceId == door->m_id && messages.GetTail()->m_targetId == door->m_id);
                }
            }
        }
        for (WORD active : {static_cast<WORD>(0), static_cast<WORD>(1), static_cast<WORD>(65535)}) {
            for (WORD wasDetected : {static_cast<WORD>(0), static_cast<WORD>(1), static_cast<WORD>(65535)}) {
                for (WORD rank : {static_cast<WORD>(0), static_cast<WORD>(99), static_cast<WORD>(100), static_cast<WORD>(65535)}) {
                    for (DWORD bits : {0UL, 8UL}) {
                        for (SHORT flash : {static_cast<SHORT>(0), static_cast<SHORT>(400), static_cast<SHORT>(-1)}) {
                            for (BOOL inMessage : {FALSE, TRUE}) {
                                discard(); door->m_dwFlags = bits;
                                door->m_trapActivated = active; door->m_trapDetected = wasDetected;
                                door->m_trapDetectionDifficulty = rank; door->m_drawPoly = flash;
                                g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly = inMessage;
                                target->AddEffect(effect(CGAMEEFFECT_DETECTTRAPS), 0, TRUE, TRUE);
                                BOOL newly = active != 0 && wasDetected == 0 && rank < 100 && bits == 8;
                                BOOL highlighted = active != 0 && (wasDetected != 0 || newly);
                                BOOL sendFlash = highlighted && flash != 400 && !inMessage;
                                check("detect-unsigned-rank-and-active-gates", door->m_trapDetected == (newly ? 1 : wasDetected)
                                    && door->m_trapActivated == active && door->m_dwFlags == bits
                                    && door->m_drawPoly == (highlighted ? 400 : flash));
                                check("detect-message-count", messages.GetCount() == messageCount + (newly ? 3 : 0) + (sendFlash ? 1 : 0));
                                POSITION pos = messages.GetHeadPosition();
                                for (INT i = 0; i < messageCount; ++i) messages.GetNext(pos);
                                if (newly && pos != NULL) {
                                    auto* trigger = messages.GetNext(pos);
                                    check("detect-trigger-first", trigger->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_TRIGGER
                                        && static_cast<CMessageSetTrigger*>(trigger)->m_trigger.m_triggerID == CAITRIGGER_DETECTED);
                                    if (pos != NULL) {
                                        auto* status = messages.GetNext(pos);
                                        check("detect-uses-trigger-status", status->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TRIGGER_STATUS
                                            && static_cast<CMessageTriggerStatus*>(status)->m_trapDetected == 1
                                            && static_cast<CMessageTriggerStatus*>(status)->m_trapActivated == active);
                                    }
                                }
                                if (newly) check("detect-door-status-last", messages.GetTail()->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_STATUS);
                            }
                        }
                    }
                }
            }
        }
        for (LONG amount : {0L, -1L, 255L, LONG_MIN, LONG_MAX}) {
            discard(); auto* cast = effect(CGAMEEFFECT_CASTSPELL);
            cast->m_res = CResRef("ZRCHIT"); cast->m_effectAmount = amount;
            target->AddEffect(cast, 2, FALSE, FALSE);
            auto* message = messages.GetCount() == messageCount + 1 ? messages.GetTail() : NULL;
            check("forced-cast-message", message != NULL && message->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_ACTION);
            if (message != NULL && message->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_ACTION) {
                auto* action = static_cast<CMessageAddAction*>(message);
                check("forced-cast-payload-and-routing", action->m_action.m_actionID == CAIAction::FORCESPELL
                    && action->m_action.m_specificID == amount && action->m_action.m_string1 == CString(_T("ZRCHIT"))
                    && action->m_action.m_acteeID.m_nInstance == door->GetAIType().m_nInstance
                    && message->m_targetId == actorId && message->m_sourceId == door->m_id);
            }
        }
        for (DWORD opcode : {0UL, static_cast<DWORD>(CGAMEEFFECT_DAMAGE), 430UL, 0xFFFFFFFFUL}) {
            discard(); door->m_dwFlags = flags; door->m_drawPoly = draw;
            target->AddEffect(effect(opcode), 1, TRUE, TRUE);
            check("unsupported-effect-discarded", messages.GetCount() == messageCount && door->m_dwFlags == flags && door->m_drawPoly == draw);
        }
        discard();
        const PLAYER_ID testPlayer = 0x0F170001;
        const LONG testActor = 0x7EFD0001, testDoor = 0x7EFD0002;
        LONG found;
        BOOL isolated = !remote->Find(testPlayer, 1, found) && !remote->Find(testPlayer, 2, found);
        check("isolated-remote-keys", isolated);
        if (isolated) {
            remote->Add(testPlayer, 1, testActor, FALSE); remote->Add(testPlayer, 2, testDoor, FALSE);
            actor->m_remotePlayerID = door->m_remotePlayerID = testPlayer;
            actor->m_remoteObjectID = 1; door->m_remoteObjectID = 2;
            for (BOOLEAN open : {static_cast<BOOLEAN>(0), static_cast<BOOLEAN>(1), static_cast<BOOLEAN>(255)}) {
                for (LONG source : {actorId, -1L}) {
                    for (LONG recipient : {door->m_id, -1L}) {
                        BYTE bytes[32]; memset(bytes, 0xA5, sizeof(bytes));
                        BYTE* payload = bytes + CNetwork::SPEC_MSG_HEADER_LENGTH;
                        DWORD length = 0;
                        CMessageDoorOpen original(open, source, recipient);
                        check("door-open-message-tags", original.GetCommType() == CMessage::SEND
                            && original.GetMsgType() == CBaldurMessage::MSG_TYPE_CMESSAGE && original.GetMsgSubType() == 18);
                        original.MarshalMessage(&payload, &length);
                        DWORD words[4]; memcpy(words, payload, sizeof(words));
                        check("door-open-wire-format", length == 17 && payload[16] == open && payload[17] == 0xA5
                            && words[0] == (recipient == -1 ? 0 : testPlayer) && static_cast<LONG>(words[1]) == (recipient == -1 ? -1 : 2)
                            && words[2] == (source == -1 ? 0 : testPlayer) && static_cast<LONG>(words[3]) == (source == -1 ? -1 : 1));
                        CMessageDoorOpen decoded(99, 77, 88);
                        BOOL result = decoded.UnmarshalMessage(bytes, CNetwork::SPEC_MSG_HEADER_LENGTH + length);
                        check("door-open-decode-routing", result == (recipient != -1)
                            && decoded.m_targetId == (recipient == -1 ? -1 : testDoor)
                            && decoded.m_sourceId == (recipient == -1 ? 77 : source == -1 ? -1 : testActor)
                            && decoded.m_bOpenDoor == (recipient == -1 ? 99 : open));
                    }
                }
            }
            remote->Delete(testActor); remote->Delete(testDoor);
            check("remote-mappings-removed", !remote->Find(testPlayer, 1, found) && !remote->Find(testPlayer, 2, found));
        }
        actor->m_remotePlayerID = actorPlayer; actor->m_remoteObjectID = actorRemote;
        door->m_remotePlayerID = doorPlayer; door->m_remoteObjectID = doorRemote;
        // Spawn from an existing CRE, inspect the real object and removal effect,
        // then remove every newly registered projectile and sprite before ticks.
        std::vector<LONG> existing;
        for (INT i = 0; i < objects->m_nArraySize; ++i)
            if (objects->m_pArray[i].m_objectId != -1 && objects->m_pArray[i].m_objectPtr != NULL)
                existing.push_back(objects->m_pArray[i].m_objectPtr->GetId());
        auto* summon = effect(CGAMEEFFECT_SUMMON);
        summon->m_res = CResRef("00SZMBL2");
        summon->m_target = actor->m_pos; summon->m_durationType = 0;
        summon->m_duration = time + 15000; summon->m_casterLevel = 0x12345678;
        Emit("{\"step\":%d,\"op\":\"door-effect-summon\",\"phase\":\"begin\"}", nStep);
        target->AddEffect(summon, 255, FALSE, FALSE);
        Emit("{\"step\":%d,\"op\":\"door-effect-summon\",\"phase\":\"returned\"}", nStep);
        INT sprites = 0, projectiles = 0;
        std::vector<LONG> created;
        LONG summonedId = -1;
        for (INT i = 0; i < objects->m_nArraySize; ++i) {
            auto& entry = objects->m_pArray[i];
            if (entry.m_objectId == -1 || entry.m_objectPtr == NULL) continue;
            LONG id = entry.m_objectPtr->GetId();
            if (std::find(existing.begin(), existing.end(), id) != existing.end()) continue;
            auto* object = entry.m_objectPtr; created.push_back(id);
            if (object->GetObjectType() == CGameObject::TYPE_SPRITE) {
                ++sprites; summonedId = id; auto* spawned = static_cast<CGameSprite*>(object);
                check("summon-resource-and-area", spawned->m_resRef == CResRef("00SZMBL2") && spawned->m_pArea == door->m_pArea);
                check("summon-grid-position-and-facing", spawned->m_pos.x % CPathSearch::GRID_SQUARE_SIZEX == 0
                    && spawned->m_pos.y % CPathSearch::GRID_SQUARE_SIZEY == 0 && spawned->m_nDirection >= 0 && spawned->m_nDirection < 16);
                CGameEffect* removal = NULL;
                POSITION pos = spawned->m_timedEffectList.GetHeadPosition();
                while (pos != NULL) {
                    auto* entry = spawned->m_timedEffectList.GetNext(pos);
                    if (entry->m_effectID == 68) removal = entry;
                }
                check("summon-removal-effect", removal != NULL && removal->m_effectAmount == 1
                    && removal->m_casterLevel == 0x12345678 && removal->m_sourceID == -1);
            } else if (object->IsProjectile()) {
                ++projectiles; auto* projectile = static_cast<CProjectile*>(object);
                check("summon-visual-projectile", projectile->m_projectileType == 143 && projectile->m_sourceId == projectile->m_targetId);
            }
        }
        check("summon-created-sprite-and-projectile", sprites == 1 && projectiles >= 1);
        INT fireMessages = 0;
        POSITION queuedPos = messages.GetHeadPosition();
        for (INT index = 0; queuedPos != NULL; ++index) {
            auto* message = messages.GetNext(queuedPos);
            if (index < messageCount || message->GetMsgSubType() != CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FIRE_PROJECTILE) continue;
            auto* fire = static_cast<CMessageFireProjectile*>(message);
            ++fireMessages;
            check("summon-fire-message", fire->m_projectileType == 144 && fire->m_projectileTargetId == summonedId
                && fire->m_projectileTarget == CPoint(0, 0) && fire->m_height == 30
                && fire->m_sourceId == summonedId && fire->m_targetId == summonedId && fire->field_1E == 0);
        }
        check("summon-fire-message-count", fireMessages == 1);
        auto stop = game->m_nTimeStop, stopCaster = game->m_nTimeStopCaster;
        // Inspect the leaf animation through a test-only derived object, then
        // run the actual summoned visual through its full removal lifecycle.
        struct SummonVisualProbe : CProjectileSPBOOM {
            SHORT Frame() const { return m_pVidCell->m_nCurrentFrame; }
            void ResetFrame() { m_pVidCell->FrameSet(0); }
            BOOL Defaults() const { return m_dirCount == 1 && m_tinted == 0 && m_mirror == 0
                && m_useHeightOffset == 0 && m_hasShadowCell == 0 && m_fireSoundRef == CResRef(""); }
        } probe;
        check("summon-visual-constructor", probe.Defaults());
        probe.m_id = -1; probe.ResetFrame();
        game->m_nTimeStop = 1; game->m_nTimeStopCaster = actorId;
        SHORT frame = probe.Frame(); probe.AIUpdate();
        check("summon-visual-time-stop", probe.Frame() == frame);
        game->m_nTimeStopCaster = -1; probe.AIUpdate();
        check("summon-visual-time-stop-owner", probe.Frame() == frame + 1);
        game->m_nTimeStop = 0;
        for (LONG id : created) {
            CGameObject* object = NULL;
            if (objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) continue;
            if (!object->IsProjectile()) continue;
            CPoint pos = object->m_pos;
            INT ticks = 0;
            do {
                check("summon-visual-stationary", object->m_pos == pos);
                object->AIUpdate();
                ++ticks;
            } while (ticks < 512 && objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS);
            check("summon-visual-animation-removal", objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS);
        }
        game->m_nTimeStop = stop; game->m_nTimeStopCaster = stopCaster;
        Emit("{\"step\":%d,\"op\":\"door-effect-summon\",\"sprites\":%d,\"projectiles\":%d,\"fireMessages\":%d}", nStep, sprites, projectiles, fireMessages);
        for (LONG id : created) {
            CGameObject* object = NULL;
            if (objects->GetShare(id, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) != CGameObjectArray::SUCCESS) continue;
            if (object->IsProjectile()) {
                static_cast<CProjectile*>(object)->RemoveSelf();
            } else {
                if (object->GetObjectType() == CGameObject::TYPE_SPRITE) static_cast<CGameSprite*>(object)->m_bDeleteOnRemove = FALSE;
                object->RemoveFromArea();
                if (objects->Delete(object->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE) == CGameObjectArray::SUCCESS) delete object;
            }
        }
        discard();
        check("world-clock-unchanged", game->GetWorldTimer()->m_gameTime == time);
        door->m_dwFlags = flags; door->m_lockDifficulty = difficulty;
        door->m_trapDetectionDifficulty = detectionDifficulty; door->m_trapActivated = activated;
        door->m_trapDetected = detected; door->m_drawPoly = draw;
        g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly = drawMessage;
        objects->ReleaseDeny(door->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(actorId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"dooreffectchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "door effect mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "doorchecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        CGameObject* raw = NULL;
        LONG id = game->GetFixedOrderCharacterId(0);
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld
            || objects->GetDeny(id, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires loaded party"); return;
        }
        auto* actor = static_cast<CGameSprite*>(raw);
        CGameDoor* door = NULL;
        for (INT i = 0; i < objects->m_nArraySize && door == NULL; ++i) {
            auto& entry = objects->m_pArray[i];
            if (entry.m_objectId == -1 || entry.m_objectPtr == NULL
                || entry.m_objectPtr->GetObjectType() != CGameObject::TYPE_DOOR
                || entry.m_objectPtr->m_pArea != actor->m_pArea) continue;
            CGameObject* object = NULL;
            if (objects->GetDeny(entry.m_objectPtr->GetId(), CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS)
                door = static_cast<CGameDoor*>(object);
        }
        if (door == NULL || g_pChitin->cVideo.Is3dAccelerated()) {
            if (door != NULL) objects->ReleaseDeny(door->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires local door and software video"); return;
        }
        auto* area = door->m_pArea;
        auto areaGroupMove = area->m_groupMove;
        auto* group = game->GetGroup();
        auto* buttons = game->GetButtonArray();
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT messageCount = messages.GetCount();
        std::vector<LONG> members;
        POSITION memberPos = group->m_memberList.GetHeadPosition();
        while (memberPos != NULL) members.push_back(reinterpret_cast<LONG>(group->m_memberList.GetNext(memberPos)));
        group->m_memberList.RemoveAll();
        auto groupChanged = group->m_groupChanged;
        auto state = game->m_nState;
        auto button = buttons->m_nSelectedButton;
        auto icon = game->m_iconIndex;
        auto lastClick = game->m_lastClick;
        auto lastTarget = game->m_lastTarget;
        auto selectedPortrait = g_pBaldurChitin->m_pEngineWorld->m_nSelectedCharacter;
        auto commandSounds = game->GetOptions()->m_nCommandSoundsFrequency;
        game->GetOptions()->m_nCommandSoundsFrequency = 0;
        CTypedPtrList<CPtrList, CAIAction*> queue;
        while (!actor->m_queuedActions.IsEmpty()) queue.AddTail(actor->m_queuedActions.RemoveHead());
        auto actorPos = actor->m_pos;
        auto base = actor->m_baseStats;
        auto interrupt = actor->m_interrupt, formation = actor->m_inFormation;
        auto pause = actor->m_userCommandPause;
        auto trigger = actor->m_triggerId;
        auto response = actor->m_curResponseNum, responseSet = actor->m_curResponseSetNum, script = actor->m_curScriptNum;
        auto useButton = actor->m_currentUseButton;
        auto encumberance = actor->m_derivedStats.m_nEncumberance;
        actor->m_derivedStats.m_nEncumberance = 0;
        auto flags = door->m_dwFlags;
        auto drawPoly = door->m_drawPoly;
        auto doorPos = door->m_pos, first = door->m_ptDest1, second = door->m_ptDest2, openDest = door->m_ptOpenDest;
        auto tile = door->m_tiledObject.m_wAIState;
        auto activated = door->m_trapActivated, detected = door->m_trapDetected;
        auto* openSearch = door->m_pOpenSearch;
        auto* closedSearch = door->m_pClosedSearch;
        auto openCount = door->m_nOpenSearch, closedCount = door->m_nClosedSearch;
        auto host = g_pChitin->cNetwork.m_bIsHost;
        auto picked = area->m_iPicked;
        ULONG time = game->GetWorldTimer()->m_gameTime;
        BYTE actorDenies = objects->m_pArray[(id >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH];
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            ++checks; valid = valid && passed;
            Emit("{\"step\":%d,\"op\":\"door-case\",\"case\":\"%s\",\"passed\":%s}", nStep, name, passed ? "true" : "false");
        };
        auto discard = [&]() {
            while (messages.GetCount() > messageCount) delete messages.RemoveTail();
            while (!actor->m_queuedActions.IsEmpty()) delete actor->m_queuedActions.RemoveHead();
        };
        auto click = [&](SHORT clickState, BYTE clickIcon, DWORD doorFlags) {
            discard(); game->m_nState = clickState; game->m_iconIndex = clickIcon;
            game->m_lastClick = CPoint(99, 100); game->m_lastTarget = id;
            buttons->m_nSelectedButton = 77;
            door->m_dwFlags = doorFlags;
            door->OnActionButton(CPoint(123, 124));
        };
        click(0, 0, 0);
        check("empty-selection-clears-target", game->m_lastClick == CPoint(-1, -1)
            && game->m_lastTarget == CGameObjectArray::INVALID_INDEX && messages.GetCount() == messageCount);
        click(0, 0, 0x80);
        check("undiscovered-secret-falls-back-to-ground", game->m_lastClick == CPoint(123, 124)
            && messages.GetCount() == messageCount);
        area->m_groupMove = TRUE;
        click(3, 0, 0);
        check("formation-click-falls-back-and-resets", game->m_nState == 0
            && buttons->m_nSelectedButton == 100 && area->m_groupMove == FALSE);
        group->m_memberList.AddTail(reinterpret_cast<LONG*>(id));
        for (BYTE fallback : {static_cast<BYTE>(18), static_cast<BYTE>(40), static_cast<BYTE>(255)}) {
            click(2, fallback, 0);
            check("target-mode-ground-fallback-retains-state", game->m_nState == 2
                && buttons->m_nSelectedButton == 77 && messages.GetCount() == messageCount);
        }
        for (DWORD doorFlags : {0UL, 1UL, 3UL}) {
            click(2, 12, doorFlags);
            check("bash-only-closed-and-locked", game->m_nState == 2 && messages.GetCount() == messageCount);
        }
        auto actionMessage = [&]() -> CMessageAddAction* {
            if (messages.GetCount() != messageCount + 1) return NULL;
            CMessage* message = messages.GetTail();
            return message->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_ACTION
                ? static_cast<CMessageAddAction*>(message) : NULL;
        };
        click(2, 12, 2);
        auto* action = actionMessage();
        check("bash-leader-payload", action != NULL && action->m_action.m_actionID == CAIAction::BASHDOOR
            && action->m_action.m_acteeID.m_nInstance == door->m_typeAI.m_nInstance
            && action->m_targetId == id && actor->m_interrupt && actor->m_inFormation);
        check("bash-target-mode-reset", game->m_nState == 0 && buttons->m_nSelectedButton == 100);
        for (INT disarm = 0; disarm < 2; ++disarm) {
            door->m_trapActivated = door->m_trapDetected = static_cast<WORD>(disarm);
            for (BYTE skill : {static_cast<BYTE>(0), static_cast<BYTE>(1), static_cast<BYTE>(255)}) {
                actor->m_baseStats.m_skills[disarm ? 5 : 10] = skill;
                click(2, 36, 2);
                action = actionMessage();
                check("skill-rank-gate", skill == 0 ? game->m_nState == 2 && messages.GetCount() == messageCount
                    : game->m_nState == 0 && buttons->m_nSelectedButton == 100 && action != NULL
                        && action->m_action.m_actionID == (disarm ? CAIAction::REMOVETRAPS : CAIAction::PICKLOCK)
                        && action->m_action.m_acteeID.m_nInstance == door->m_typeAI.m_nInstance);
            }
        }
        door->m_trapActivated = 1; door->m_trapDetected = 0;
        click(2, 36, 0);
        check("undetected-trap-unlocked-ground-fallback", game->m_nState == 2 && messages.GetCount() == messageCount);
        actor->m_currentUseButton.m_abilityId.m_itemType = 2;
        actor->m_currentUseButton.m_abilityId.m_itemNum = 3;
        actor->m_currentUseButton.m_abilityId.m_abilityNum = 4;
        click(2, 20, 0);
        check("magic-item-queues-door-target", actor->m_queuedActions.GetCount() == 1
            && actor->m_queuedActions.GetHead()->m_actionID == CAIAction::USEITEM
            && actor->m_queuedActions.GetHead()->m_acteeID.m_nInstance == door->m_id
            && game->m_nState == 0 && buttons->m_nSelectedButton == 100);
        RESREF areaName;
        memcpy(areaName, area->m_header.m_areaName, sizeof(areaName));
        struct RangeCase { const char* areaName; LONG dx; BOOL close; };
        const RangeCase ranges[] = {{"ar0000", 64, TRUE}, {"ar0000", 65, FALSE},
            {"ar5004", 20, TRUE}, {"AR5004", 21, FALSE}, {"ar5202", 28, TRUE}, {"AR5202", 29, FALSE},
            {"ar5004x", 21, TRUE}};
        actor->m_pos = CPoint(160, 120);
        for (const auto& entry : ranges) {
            memset(area->m_header.m_areaName, 0, sizeof(RESREF));
            memcpy(area->m_header.m_areaName, entry.areaName, strlen(entry.areaName));
            door->m_ptDest1 = CPoint(160 + entry.dx, 120);
            door->m_ptDest2 = CPoint(160 + entry.dx + 100, 120);
            click(0, 0, 0x100);
            check("door-order-and-range-message-count", messages.GetCount() == messageCount + (entry.close ? 1 : 2));
            POSITION pos = messages.GetTailPosition();
            if (!entry.close && messages.GetCount() == messageCount + 2) {
                auto* move = static_cast<CMessageAddAction*>(messages.GetPrev(pos));
                check("outside-range-followup-move", move->m_action.m_actionID == CAIAction::MOVETOPOINT
                    && move->m_action.m_dest == door->m_ptDest1 && move->m_action.m_specificID == 0
                    && move->m_action.m_specificID2 == -1 && move->m_action.m_specificID3 == -1);
            }
            if (messages.GetCount() >= messageCount + 1) {
                auto* use = static_cast<CMessageAddAction*>(messages.GetPrev(pos));
                check("ordinary-click-target-is-instance-selector", use->m_action.m_actionID == CAIAction::USEDOOR
                    && use->m_action.m_acteeID.m_nInstance == door->m_id
                    && use->m_action.m_acteeID.m_nEnemyAlly == 0 && use->m_targetId == id);
            }
        }
        memcpy(area->m_header.m_areaName, areaName, sizeof(areaName));
        discard();
        door->m_pOpenSearch = door->m_pClosedSearch = NULL;
        door->m_nOpenSearch = door->m_nClosedSearch = 0;
        g_pChitin->cNetwork.m_bIsHost = FALSE;
        const DWORD deltas[] = {0, 1, 10, 32767, 32768, 4500, 4501, 0xFFFFFFFF};
        for (SHORT flash : {static_cast<SHORT>(0), static_cast<SHORT>(10), static_cast<SHORT>(32767),
                 static_cast<SHORT>(-1), static_cast<SHORT>(-32768)}) {
            for (DWORD delta : deltas) {
                door->m_drawPoly = flash; door->m_dwFlags = 0x1021;
                SHORT expected = static_cast<DWORD>(static_cast<LONG>(flash)) < delta
                    ? 0 : static_cast<SHORT>(static_cast<DWORD>(flash) - delta);
                check("compress-time-flash-unsigned-width", door->CompressTime(delta) == TRUE && door->m_drawPoly == expected);
                check("compress-time-strict-hour-flag", door->m_dwFlags == (delta > 4500 ? 0x21 : 0x1021));
            }
        }
        unsigned seedClose = 0, seedStay = 0;
        for (unsigned seed = 1; seed < 100; ++seed) {
            srand(seed);
            if (rand() % 5 == 0) seedStay = seed; else seedClose = seed;
            if (seedStay != 0 && seedClose != 0) break;
        }
        check("both-autoclose-random-outcomes-available", seedClose != 0 && seedStay != 0);
        for (BOOL shouldClose : {FALSE, TRUE}) {
            door->m_dwFlags = 0x1001; door->m_pos = doorPos; door->m_ptOpenDest = CPoint(321, 654);
            door->m_tiledObject.m_wAIState = CTiledObject::STATE_PRIMARY_TILE;
            srand(shouldClose ? seedClose : seedStay);
            door->CompressTime(4501);
            check("compress-time-random-close", door->m_dwFlags == (shouldClose ? 0 : 1)
                && door->m_pos == (shouldClose ? CPoint(321, 654) : doorPos)
                && door->m_tiledObject.m_wAIState == (shouldClose ? CTiledObject::STATE_SECONDARY_TILE : CTiledObject::STATE_PRIMARY_TILE));
        }
        CPoint blockedPoint(0, 0);
        BYTE savedCost = area->m_search.m_pDynamicCost[0];
        area->m_search.m_pDynamicCost[0] = savedCost | 2;
        door->m_pClosedSearch = &blockedPoint; door->m_nClosedSearch = 1;
        door->m_dwFlags = 0x1001; srand(seedClose); door->CompressTime(4501);
        check("occupied-search-cell-prevents-close-clears-flag", door->m_dwFlags == 1);
        area->m_search.m_pDynamicCost[0] = savedCost;
        door->m_pClosedSearch = NULL; door->m_nClosedSearch = 0;
        door->m_dwFlags = 0x1001; g_pChitin->cNetwork.m_bIsHost = TRUE;
        srand(seedClose); door->CompressTime(4501);
        check("host-enqueues-door-status", messages.GetCount() == messageCount + 1
            && messages.GetTail()->GetMsgSubType() == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_STATUS
            && messages.GetTail()->m_sourceId == door->m_id && messages.GetTail()->m_targetId == door->m_id);
        discard(); g_pChitin->cNetwork.m_bIsHost = host;
        check("compress-time-does-not-advance-world-clock", game->GetWorldTimer()->m_gameTime == time);
        // Render real, clipped geometry on the current software back surface.
        auto* infinity = area->GetInfinity();
        auto* video = static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
        CRect rect(infinity->nCurrentX + 20, infinity->nCurrentY + 20,
            infinity->nCurrentX + 44, infinity->nCurrentY + 44);
        CAreaPoint polygon[4] = {{static_cast<WORD>(rect.left), static_cast<WORD>(rect.top)},
            {static_cast<WORD>(rect.right), static_cast<WORD>(rect.top)},
            {static_cast<WORD>(rect.right), static_cast<WORD>(rect.bottom)},
            {static_cast<WORD>(rect.left), static_cast<WORD>(rect.bottom)}};
        std::vector<CRect*> closedGrid, openGrid;
        for (INT i = 0; i < door->m_closedBoundingGrid.GetSize(); ++i) closedGrid.push_back(door->m_closedBoundingGrid[i]);
        for (INT i = 0; i < door->m_openBoundingGrid.GetSize(); ++i) openGrid.push_back(door->m_openBoundingGrid[i]);
        auto* closedPoints = door->m_pClosedPolygonPoints;
        auto* openPoints = door->m_pOpenPolygonPoints;
        auto nClosed = door->m_nClosedPolygon, nOpen = door->m_nOpenPolygon;
        door->m_pClosedPolygonPoints = door->m_pOpenPolygonPoints = polygon;
        door->m_nClosedPolygon = door->m_nOpenPolygon = 4;
        door->m_closedBoundingGrid.SetSize(2); door->m_openBoundingGrid.SetSize(2);
        door->m_closedBoundingGrid[0] = door->m_openBoundingGrid[0] = NULL;
        door->m_closedBoundingGrid[1] = door->m_openBoundingGrid[1] = &rect;
        auto pixel = [&](DWORD& result) -> BOOL {
            CRect sample(infinity->rViewPort.left + 30, infinity->rViewPort.top + 30,
                infinity->rViewPort.left + 31, infinity->rViewPort.top + 31);
            if (!video->BKLock(sample)) return FALSE;
            DWORD bytes = video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8;
            result = 0;
            BOOL ok = bytes > 0 && bytes <= sizeof(result) && video->m_SurfaceDesc.lpSurface != NULL;
            if (ok) memcpy(&result, video->m_SurfaceDesc.lpSurface, bytes);
            video->BKUnlock(); return ok;
        };
        DWORD before = 0, after = 0;
        BOOL readBefore = pixel(before);
        door->RenderClosedPoly(area, video, CVIDINF_SURFACE_BACK, video->GetColor(RGB(0xF0, 0x20, 0xE0)));
        check("closed-polygon-draw-changes-back-pixel", readBefore && pixel(after) && before != after);
        before = after;
        door->RenderOpenPoly(area, video, CVIDINF_SURFACE_BACK, video->GetColor(RGB(0x20, 0xE0, 0x30)));
        check("open-polygon-draw-changes-back-pixel", pixel(after) && before != after);
        door->m_closedBoundingGrid.SetSize(static_cast<INT>(closedGrid.size()));
        door->m_openBoundingGrid.SetSize(static_cast<INT>(openGrid.size()));
        for (INT i = 0; i < static_cast<INT>(closedGrid.size()); ++i) door->m_closedBoundingGrid[i] = closedGrid[i];
        for (INT i = 0; i < static_cast<INT>(openGrid.size()); ++i) door->m_openBoundingGrid[i] = openGrid[i];
        door->m_pClosedPolygonPoints = closedPoints; door->m_pOpenPolygonPoints = openPoints;
        door->m_nClosedPolygon = nClosed; door->m_nOpenPolygon = nOpen;
        door->m_pos = actorPos; area->m_iPicked = door->m_id;
        for (DWORD renderFlags : {0UL, 1UL, 2UL, 3UL, 0x80UL, 0x180UL, 0x2000UL}) {
            for (SHORT flash : {static_cast<SHORT>(0), static_cast<SHORT>(1), static_cast<SHORT>(-1)}) {
                door->m_dwFlags = renderFlags; door->m_drawPoly = flash;
                game->m_nState = 2; game->m_iconIndex = 36;
                door->m_trapActivated = door->m_trapDetected = 1;
                door->Render(area, video, CVIDINF_SURFACE_BACK);
                check("render-does-not-change-door-state", door->m_dwFlags == renderFlags && door->m_drawPoly == flash
                    && door->m_pos == actorPos && video->m_SurfaceDesc.lpSurface == NULL);
            }
        }
        discard();
        actor->m_pos = actorPos; actor->m_baseStats = base;
        actor->m_interrupt = interrupt; actor->m_inFormation = formation;
        actor->m_userCommandPause = pause; actor->m_triggerId = trigger;
        actor->m_curResponseNum = response; actor->m_curResponseSetNum = responseSet; actor->m_curScriptNum = script;
        actor->m_currentUseButton = useButton;
        actor->m_derivedStats.m_nEncumberance = encumberance;
        while (!queue.IsEmpty()) actor->m_queuedActions.AddTail(queue.RemoveHead());
        group->m_memberList.RemoveAll();
        actor->m_groupPosition = NULL;
        for (LONG member : members) {
            POSITION pos = group->m_memberList.AddTail(reinterpret_cast<LONG*>(member));
            CGameObject* object = NULL;
            if (objects->GetShare(member, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS) {
                static_cast<CGameSprite*>(object)->m_groupPosition = pos;
                objects->ReleaseShare(member, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            } else check("restore-group-member-available", FALSE);
        }
        group->m_groupChanged = groupChanged;
        game->GetOptions()->m_nCommandSoundsFrequency = commandSounds;
        buttons->m_nSelectedButton = button; buttons->UpdateState();
        game->m_nState = state; game->m_iconIndex = icon;
        game->m_lastClick = lastClick; game->m_lastTarget = lastTarget;
        g_pBaldurChitin->m_pEngineWorld->m_nSelectedCharacter = selectedPortrait;
        door->m_dwFlags = flags; door->m_drawPoly = drawPoly;
        door->m_pos = doorPos; door->m_ptDest1 = first; door->m_ptDest2 = second; door->m_ptOpenDest = openDest;
        door->m_tiledObject.m_wAIState = tile; door->m_trapActivated = activated; door->m_trapDetected = detected;
        door->m_pOpenSearch = openSearch; door->m_pClosedSearch = closedSearch;
        door->m_nOpenSearch = openCount; door->m_nClosedSearch = closedCount; area->m_iPicked = picked;
        area->m_groupMove = areaGroupMove;
        check("messages-and-denies-balanced", messages.GetCount() == messageCount
            && objects->m_pArray[(id >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == actorDenies);
        objects->ReleaseDeny(door->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"doorchecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "door mismatch (state restored)"); return; }
        s_pc++; return;
    }

    if (strcmp(step.op, "targetresolvechecks") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        auto* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* rawSource = NULL;
        CGameObject* rawTarget = NULL;
        if (pActiveEngine != g_pBaldurChitin->m_pEngineWorld || game->GetNumCharacters() < 2
            || objects->GetDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, &rawSource, INFINITE) != CGameObjectArray::SUCCESS) {
            Fail(nStep, step.op, "requires world and two party members"); return;
        }
        if (objects->GetDeny(targetId, CGameObjectArray::THREAD_ASYNCH, &rawTarget, INFINITE) != CGameObjectArray::SUCCESS) {
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "target unavailable"); return;
        }
        auto* source = static_cast<CGameSprite*>(rawSource);
        auto* target = static_cast<CGameSprite*>(rawTarget);
        CSingleLock searchLock(&game->field_1B58, TRUE);
        if (source->m_pArea == NULL || source->m_pPath != NULL || target->m_pPath != NULL || source->m_currentSearchRequest != NULL
            || source->m_animation.m_animation == NULL || target->m_animation.m_animation == NULL) {
            objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires idle actors with animations"); return;
        }
        HANDLE savedSearchSemaphore = game->m_hSearchThread;
        HANDLE testSearchSemaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
        if (testSearchSemaphore == NULL) {
            objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "test semaphore unavailable"); return;
        }
        game->m_hSearchThread = testSearchSemaphore;
        CAIAction savedAction;
        savedAction = source->m_curAction;
        auto savedSourceType = source->m_typeAI, savedTargetType = target->m_typeAI;
        auto savedSourceMarker = source->m_marker, savedTargetMarker = target->m_marker;
        auto savedState = source->m_derivedStats.m_generalState;
        auto savedSpells = source->m_derivedStats.m_spellStates;
        auto savedUnselectable = source->m_nUnselectableCounter;
        auto savedMorale = source->m_moraleFailure;
        auto savedSelected = source->m_bSelected;
        auto savedTargetId = source->m_targetId;
        auto savedReturn = source->m_nLastActionReturn;
        auto savedPause = source->m_userCommandPause;
        auto savedCount = source->m_actionCount;
        auto savedPhase = source->field_5618;
        auto savedThrottle = source->field_5616;
        auto savedDest = source->m_curDest, savedTargetDest = target->m_curDest;
        auto savedTargetPos = target->m_pos;
        auto savedTargetObjectType = target->m_objectType;
        auto* savedTargetArea = target->m_pArea;
        auto savedEncumberance = source->m_derivedStats.m_nEncumberance;
        auto savedBaseFlags = source->m_baseStats.m_flags;
        auto savedLastAction = source->m_lastActionID;
        auto savedFollowRange = source->m_followRange;
        auto savedSourcePos = source->m_pos;
        auto savedSourceObjectType = source->m_objectType;
        auto savedSeen = source->m_lSeen;
        auto savedSeenCount = source->m_canBeSeen;
        auto savedSequence = source->m_nSequence;
        auto savedInvalidDest = source->m_pathSearchInvalidDest;
        auto savedStop = game->m_nTimeStop;
        auto savedEmpty = game->m_searchRequestListEmpty;
        auto* sourceAnimation = source->m_animation.m_animation;
        auto* targetAnimation = target->m_animation.m_animation;
        auto sourceSpace = sourceAnimation->m_personalSpace, targetSpace = targetAnimation->m_personalSpace;
        std::vector<CAIObjectType*> savedImmunities;
        auto& immunities = target->m_derivedStats.m_cImmunitiesAIType;
        POSITION immunityPos = immunities.GetHeadPosition();
        while (immunityPos != NULL) savedImmunities.push_back(immunities.GetNext(immunityPos));
        immunities.RemoveAll();
        CAIObjectType immune(savedSourceType);
        LONG stale = 0x7FFF7FFE;
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT messageCount = messages.GetCount(), requestCount = game->m_searchRequests.GetCount();
        INT backRequestCount = game->m_searchRequestsBack.GetCount();
        BYTE sourceDenies = objects->m_pArray[(sourceId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH];
        BYTE targetDenies = objects->m_pArray[(targetId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH];
        BOOL valid = TRUE;
        INT checks = 0;
        auto check = [&](const char* name, BOOL passed) {
            valid = valid && passed; ++checks;
            Emit("{\"step\":%d,\"op\":\"targetresolve-case\",\"case\":\"%s\",\"passed\":%s}",
                nStep, name, passed ? "true" : "false");
        };
        auto discardRequest = [&]() {
            if (source->m_currentSearchRequest != NULL) {
                POSITION pos = game->m_searchRequests.Find(source->m_currentSearchRequest);
                POSITION backPos = game->m_searchRequestsBack.Find(source->m_currentSearchRequest);
                check("request-still-waiting-under-lock", (pos != NULL || backPos != NULL)
                    && source->m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_WAITING);
                if (pos != NULL) game->m_searchRequests.RemoveAt(pos);
                if (backPos != NULL) game->m_searchRequestsBack.RemoveAt(backPos);
                check("request-signals-private-semaphore", WaitForSingleObject(testSearchSemaphore, 0) == WAIT_OBJECT_0);
                delete source->m_currentSearchRequest;
                source->m_currentSearchRequest = NULL;
            }
            game->m_searchRequestListEmpty = savedEmpty;
            while (messages.GetCount() > messageCount) delete messages.RemoveTail();
        };
        auto reset = [&]() {
            discardRequest();
            source->m_curAction = CAIAction::NULL_ACTION;
            source->m_curAction.m_actionID = 14;
            source->m_curAction.m_acteeID.Set(savedTargetType);
            source->m_curAction.m_acteeID.m_nInstance = targetId;
            source->m_typeAI.Set(savedSourceType); target->m_typeAI.Set(savedTargetType);
            source->m_typeAI.m_nEnemyAlly = CAIObjectType::EA_PC;
            source->m_derivedStats.m_generalState = 0; source->m_derivedStats.m_spellStates.reset();
            source->m_derivedStats.m_nEncumberance = 0;
            source->m_baseStats.m_flags = savedBaseFlags & ~0x100000;
            source->m_lastActionID = 123; source->m_followRange = 123;
            source->m_pos = savedSourcePos; source->m_objectType = savedSourceObjectType;
            source->m_nUnselectableCounter = 0; source->m_moraleFailure = FALSE;
            source->m_bSelected = TRUE; source->m_targetId = CGameObjectArray::INVALID_INDEX;
            source->m_marker.m_type = target->m_marker.m_type = CMarker::ELLIPSE;
            source->m_marker.m_nRecticleCounter = target->m_marker.m_nRecticleCounter = 0;
            source->m_marker.m_nRecticleForceRenderTarget = target->m_marker.m_nRecticleForceRenderTarget = 7;
            source->m_nLastActionReturn = 123; source->m_userCommandPause = 100; source->m_actionCount = 0;
            source->field_5616 = 1; source->field_5618 = 0;
            source->m_curDest = CPoint(-1, -1); target->m_curDest = savedTargetDest;
            target->m_pos = savedTargetPos; target->m_pArea = savedTargetArea;
            target->m_objectType = savedTargetObjectType;
            sourceAnimation->m_personalSpace = targetAnimation->m_personalSpace = 3;
            immunities.RemoveAll(); game->m_nTimeStop = 0;
        };
        auto resolve = [&]() {
            CGameObject* result = source->ResolveCurrentActionTarget();
            if (result != NULL) objects->ReleaseShare(result->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return result;
        };
        reset();
        check("selected-explicit-target", resolve() == target && source->m_targetId == targetId
            && target->m_marker.m_nRecticleCounter == 1 && target->m_marker.m_type == CMarker::RECTICLE);
        check("same-target-does-not-add-reticle", resolve() == target && target->m_marker.m_nRecticleCounter == 1);
        check("no-force-render-extension", target->m_marker.m_nRecticleForceRenderTarget == 7);
        reset(); source->m_targetId = sourceId; source->m_marker.SetType(CMarker::RECTICLE);
        check("switch-removes-old-reticle", resolve() == target && source->m_marker.m_nRecticleCounter == 0
            && source->m_marker.m_type == CMarker::ELLIPSE && target->m_marker.m_nRecticleCounter == 1);
        reset(); source->m_targetId = sourceId;
        source->m_marker.SetType(CMarker::RECTICLE); source->m_marker.SetType(CMarker::RECTICLE);
        check("switch-releases-one-reticle-owner", resolve() == target && source->m_marker.m_nRecticleCounter == 1
            && source->m_marker.m_type == CMarker::RECTICLE);
        for (INT mode = 0; mode < 4; ++mode) {
            reset(); source->m_targetId = sourceId; source->m_marker.SetType(CMarker::RECTICLE);
            if (mode == 0) source->m_bSelected = FALSE;
            if (mode == 1) source->m_derivedStats.m_generalState = STATE_DEAD;
            if (mode == 2) source->m_nUnselectableCounter = 1;
            if (mode == 3) source->m_moraleFailure = TRUE;
            check("unorderable-switch-publishes-without-markers", resolve() == target && source->m_targetId == targetId
                && source->m_marker.m_nRecticleCounter == 1 && target->m_marker.m_nRecticleCounter == 0);
        }
        reset(); immunities.AddTail(&immune);
        check("immunity-after-marker-update", resolve() == NULL && source->m_targetId == targetId
            && target->m_marker.m_nRecticleCounter == 1);
        check("same-immune-target-keeps-marker", resolve() == NULL && target->m_marker.m_nRecticleCounter == 1);
        reset(); source->m_targetId = stale;
        check("stale-old-target-retains-id", resolve() == target && source->m_targetId == stale
            && target->m_marker.m_nRecticleCounter == 0);
        immunities.AddTail(&immune);
        check("stale-old-target-still-checks-immunity", resolve() == NULL && source->m_targetId == stale);
        for (INT selected = 0; selected < 2; ++selected) {
            reset(); source->m_bSelected = selected;
            source->m_targetId = targetId; target->m_marker.SetType(CMarker::RECTICLE);
            source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
            check("null-clears-id", resolve() == NULL && source->m_targetId == CGameObjectArray::INVALID_INDEX);
            check("null-clears-marker-only-when-selected", target->m_marker.m_nRecticleCounter == (selected ? 0 : 1));
            reset(); source->m_bSelected = selected; source->m_targetId = stale;
            source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
            check("null-stale-selected-retains-id", resolve() == NULL
                && source->m_targetId == (selected ? stale : CGameObjectArray::INVALID_INDEX));
        }
        reset(); source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
        check("null-without-old-target", resolve() == NULL && source->m_targetId == CGameObjectArray::INVALID_INDEX);
        reset(); target->m_objectType = CGameObject::TYPE_NONE; immunities.AddTail(&immune);
        check("untyped-nonsprite-no-marker-or-immunity", resolve() == target && source->m_targetId == targetId
            && target->m_marker.m_nRecticleCounter == 0);
        reset(); target->m_typeAI.m_nSpecific = 253;
        source->m_curAction.m_acteeID.Set(CAIObjectType::ANYONE);
        source->m_curAction.m_acteeID.m_nSpecific = 253;
        source->m_curAction.m_acteeID.m_nRadius = 9876;
        source->m_curAction.m_acteeID.m_rect = CRect(1, 2, 3, 4);
        check("nearest-selector-fixes-instance", resolve() == target
            && source->m_curAction.m_acteeID.m_nInstance == targetId);
        check("fixation-preserves-selector", source->m_curAction.m_acteeID.m_nSpecific == 253
            && source->m_curAction.m_acteeID.m_nRadius == 9876 && source->m_curAction.m_acteeID.m_rect == CRect(1, 2, 3, 4)
            && source->m_curAction.m_acteeID.m_nRace == 0);
        reset();
        check("action14-resolves-and-stores-done", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->m_nLastActionReturn == CGameAIBase::ACTION_DONE && source->m_targetId == targetId);
        reset(); source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
        check("action14-null-is-done", source->ExecuteAction() == CGameAIBase::ACTION_DONE);
        reset(); source->m_curAction.m_actionID = 47; source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
        check("formation-null-is-error", source->ExecuteAction() == CGameAIBase::ACTION_ERROR
            && source->m_nLastActionReturn == CGameAIBase::ACTION_ERROR);
        reset(); source->m_curAction.m_actionID = 0x13D; source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
        check("jump-object-null-is-done", source->ExecuteAction() == CGameAIBase::ACTION_DONE);
        CGameArea* sourceArea = source->m_pArea;
        source->m_pArea = NULL;
        reset(); source->m_curAction.m_actionID = 0x13D;
        check("jump-object-resolves-before-area-less-exit", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->m_targetId == targetId && source->m_nLastActionReturn == CGameAIBase::ACTION_DONE);
        reset(); source->m_curAction.m_actionID = CAIAction::JUMPTOPOINT;
        source->m_curAction.m_dest = CPoint(1234, 5678);
        check("jump-point-dispatch-stores-return", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->m_targetId == CGameObjectArray::INVALID_INDEX && source->m_nLastActionReturn == CGameAIBase::ACTION_DONE);
        source->m_pArea = sourceArea;
        for (SHORT action : {static_cast<SHORT>(180), static_cast<SHORT>(208), static_cast<SHORT>(263)}) {
            reset(); source->m_curAction.m_actionID = action; source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
            check("move-variant-null-is-error", source->ExecuteAction() == CGameAIBase::ACTION_ERROR
                && source->m_nLastActionReturn == CGameAIBase::ACTION_ERROR);
            reset(); source->m_curAction.m_actionID = action; target->m_pArea = NULL;
            check("move-variant-area-less-is-done", source->ExecuteAction() == CGameAIBase::ACTION_DONE);
            reset(); source->m_curAction.m_actionID = action; target->m_pos = source->m_pos;
            source->m_bSelected = FALSE;
            check("move-variant-close-return", source->ExecuteAction()
                == (action == 180 ? CGameAIBase::ACTION_INTERRUPTABLE : CGameAIBase::ACTION_DONE));
        }
        // All requests stay queued under the same critical section until inspected and discarded.
        reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT;
        source->m_curAction.m_acteeID.Set(CAIObjectType::NOONE);
        check("move22-missing-is-done", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->m_nLastActionReturn == CGameAIBase::ACTION_DONE && source->m_lastActionID == 123);
        reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT;
        source->m_curAction.m_acteeID.m_nInstance = stale;
        check("move22-stale-is-done", source->ExecuteAction() == CGameAIBase::ACTION_DONE);
        reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT; source->m_objectType = CGameObject::TYPE_NONE;
        check("move22-requires-sprite-source", source->ExecuteAction() == CGameAIBase::ACTION_ERROR
            && source->m_nLastActionReturn == CGameAIBase::ACTION_ERROR && source->m_targetId == CGameObjectArray::INVALID_INDEX);
        for (BYTE allegiance : {static_cast<BYTE>(30), static_cast<BYTE>(31), static_cast<BYTE>(255)}) {
            reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT;
            target->m_typeAI.m_nEnemyAlly = allegiance; target->m_pArea = NULL;
            check("move22-flags-good-target-and-stores-action", source->ExecuteAction() == CGameAIBase::ACTION_DONE
                && ((source->m_baseStats.m_flags & 0x100000) != 0) == (allegiance == 30)
                && source->m_lastActionID == CAIAction::MOVETOOBJECT && source->m_targetId == targetId);
        }
        reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT;
        source->m_baseStats.m_flags |= 0x100000; target->m_typeAI.m_nEnemyAlly = 255; target->m_pArea = NULL;
        check("move22-keeps-existing-base-flag", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && (source->m_baseStats.m_flags & 0x100000) != 0);
        reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT; immunities.AddTail(&immune);
        check("move22-immunity-after-target-publication", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->m_targetId == targetId && source->m_lastActionID == 123);
        reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT;
        target->m_objectType = CGameObject::TYPE_NONE; target->m_pos = source->m_pos;
        check("move22-nonsprite-uses-position", source->ExecuteAction() == CGameAIBase::ACTION_DONE
            && source->m_lastActionID == 123 && source->m_targetId == targetId
            && target->m_marker.m_nRecticleCounter == 0 && (source->m_baseStats.m_flags & 0x100000) == 0);
        const SHORT movementActions[] = {CAIAction::MOVETOOBJECT, 180, 208};
        for (SHORT action : movementActions) for (SHORT frame : {static_cast<SHORT>(-8), static_cast<SHORT>(-1),
                static_cast<SHORT>(0), static_cast<SHORT>(1), static_cast<SHORT>(8), static_cast<SHORT>(32767)}) {
            reset(); source->m_curAction.m_actionID = action; source->m_bSelected = FALSE;
            source->m_pos = CPoint(160, 120); target->m_pos = CPoint(480, 120);
            source->field_5616 = frame;
            check("move-object-dispatch-return", source->ExecuteAction()
                == (action == 208 ? CGameAIBase::ACTION_NORMAL : CGameAIBase::ACTION_INTERRUPTABLE));
            check("move-object-short-throttle-wrap", source->field_5616 == static_cast<SHORT>(frame + 1));
            BOOL scheduled = frame == -8 || frame == 0 || frame == 8;
            auto* request = source->m_currentSearchRequest;
            check("move-object-throttle-and-phase", (request != NULL) == scheduled && source->field_5618 == (scheduled ? 1 : 0)
                && source->m_followRange == 0);
            if (request != NULL) {
                check("move-object-searches-target-id", request->m_nTargetIds == 1 && request->m_targetIds != NULL
                    && request->m_targetIds[0] == targetId && request->m_nTargetPoints == 0
                    && source->m_curDest == target->m_pos);
                check("move-object-search-provenance", request->m_sourceId == sourceId
                    && request->m_sourceSide == source->m_typeAI.GetEnemyAlly() && request->m_searchBitmap == &source->m_pArea->m_search
                    && memcmp(request->m_terrainTable, source->m_terrainTable, 16) == 0
                    && request->m_frontList == CSearchRequest::LIST_FRONT);
            }
        }
        // MoveToPointRange preserves the original boolean Y-distance quirk.
        struct PointRangeCase { LONG x, y, range; SHORT expected; };
        const PointRangeCase pointRanges[] = {
            {160, 120, 0, CGameAIBase::ACTION_DONE}, {175, 131, 0, CGameAIBase::ACTION_DONE},
            {176, 120, 0, CGameAIBase::ACTION_INTERRUPTABLE}, {160, 132, 0, CGameAIBase::ACTION_INTERRUPTABLE},
            {176, 120, 1, CGameAIBase::ACTION_INTERRUPTABLE}, {176, 132, 1, CGameAIBase::ACTION_DONE},
            {176, 360, 1, CGameAIBase::ACTION_DONE}, {192, 120, 2, CGameAIBase::ACTION_INTERRUPTABLE},
            {192, 132, 2, CGameAIBase::ACTION_DONE}, {176, 120, -1, CGameAIBase::ACTION_INTERRUPTABLE},
            {176, 132, -1, CGameAIBase::ACTION_DONE}, {-1, 120, 0, CGameAIBase::ACTION_ERROR},
            {160, -1, 0, CGameAIBase::ACTION_ERROR}, {160, 120, 65536, CGameAIBase::ACTION_INTERRUPTABLE},
            {160, 132, 65536, CGameAIBase::ACTION_DONE},
        };
        for (const auto& entry : pointRanges) {
            reset(); source->m_pos = CPoint(160, 120);
            CPoint destination(entry.x, entry.y);
            SHORT result = source->MoveToPointRange(destination, entry.range);
            check("point-range-literal-threshold", result == entry.expected);
            auto* request = source->m_currentSearchRequest;
            check("point-range-request-only-when-moving", (request != NULL) == (entry.expected == CGameAIBase::ACTION_INTERRUPTABLE));
            if (request != NULL) {
                check("point-range-one-exclusive-goal", request->m_nTargetPoints == 1 && request->m_exclusiveTargetPoints == TRUE
                    && request->m_targetPoints[0].x == entry.x && request->m_targetPoints[0].y == entry.y
                    && request->m_nTargetIds == 0 && source->m_curDest == destination);
                check("point-range-search-provenance", request->m_sourceId == sourceId
                    && request->m_sourceSide == source->m_typeAI.GetEnemyAlly() && request->m_removeSelf == TRUE
                    && request->m_searchBitmap == &source->m_pArea->m_search && request->m_frontList == CSearchRequest::LIST_FRONT);
            }
        }
        for (INT count : {0, 1, 32767}) for (INT background : {0, 1}) for (INT same : {0, 1}) {
            reset(); source->m_pos = CPoint(160, 120); source->m_actionCount = static_cast<SHORT>(count);
            source->m_curAction.m_specificID = background ? CAIAction::BACKGROUND : 0;
            CPoint destination(480, 360);
            if (same) source->m_curDest = destination;
            BOOL allowed = !background || count == 0;
            SHORT expected = allowed ? CGameAIBase::ACTION_INTERRUPTABLE : CGameAIBase::ACTION_ERROR;
            check("point-range-background-action-gate", source->MoveToPointRange(destination, 0) == expected);
            check("point-range-same-destination-or-rejected-has-no-request", (source->m_currentSearchRequest != NULL) == (allowed && !same));
            if (source->m_currentSearchRequest != NULL && background) {
                auto* request = source->m_currentSearchRequest;
                check("point-range-background-request", request->m_frontList == CSearchRequest::LIST_BACK
                    && request->m_minNodesBack == 110 && request->m_maxNodesBack == CSearchRequest::MINNODESBACK
                    && request->m_sourcePt.x == 10 && request->m_sourcePt.y == 10);
            }
        }
        reset(); source->m_pos = CPoint(160, 120);
        CPoint edgeX(source->m_pArea->GetInfinity()->nAreaX, 120), edgeY(160, source->m_pArea->GetInfinity()->nAreaY);
        check("point-range-area-right-boundary", source->MoveToPointRange(edgeX, 0) == CGameAIBase::ACTION_ERROR);
        check("point-range-area-bottom-boundary", source->MoveToPointRange(edgeY, 0) == CGameAIBase::ACTION_ERROR);
        reset();
        CGameDoor* door = NULL;
        for (INT slot = 0; slot < objects->m_nArraySize && door == NULL; ++slot) {
            auto& entry = objects->m_pArray[slot];
            if (entry.m_objectId == -1 || entry.m_objectPtr == NULL
                || entry.m_objectPtr->GetObjectType() != CGameObject::TYPE_DOOR) continue;
            CGameObject* object = NULL;
            if (objects->GetDeny(entry.m_objectPtr->GetId(), CGameObjectArray::THREAD_ASYNCH, &object, INFINITE) == CGameObjectArray::SUCCESS)
                door = static_cast<CGameDoor*>(object);
        }
        check("loaded-door-available", door != NULL);
        if (door != NULL) {
            CPoint savedFirst = door->m_ptDest1, savedSecond = door->m_ptDest2, savedDoorPos = door->m_pos;
            struct DoorCase { LONG x1, y1, x2, y2, x, y; BOOL first; };
            const DoorCase doors[] = {
                {0, 0, 10, 0, 0, 0, TRUE}, {0, 0, 10, 0, 10, 0, FALSE},
                {0, 0, 10, 0, 5, 0, FALSE}, {0, 0, 0, 0, 77, -99, FALSE},
                {0, 3, 4, 0, 0, 0, FALSE}, {0, 3, 4, 1, 0, 0, TRUE},
                {0, 6, 7, 0, 0, 0, FALSE}, {0, -6, -7, 0, 0, 0, FALSE},
                {-10, -10, 10, 10, -9, -9, TRUE}, {-10, -10, 10, 10, 0, 0, FALSE},
                {65536, 0, 1, 0, 0, 0, TRUE}, {0, 16384, 1, 0, 0, 0, TRUE},
                {46341, 0, 0, 0, 0, 0, TRUE}, {0, 12000, 0, 0, 0, 0, TRUE},
            };
            for (const auto& entry : doors) {
                door->m_ptDest1 = CPoint(entry.x1, entry.y1); door->m_ptDest2 = CPoint(entry.x2, entry.y2);
                CPoint point(entry.x, entry.y), before(point);
                const CPoint& destination = door->GetMoveDest(point);
                check("door-weighted-distance-tie-and-long-wrap", &destination == (entry.first ? &door->m_ptDest1 : &door->m_ptDest2));
                check("door-helper-keeps-input-and-destinations", point == before
                    && door->m_ptDest1 == CPoint(entry.x1, entry.y1) && door->m_ptDest2 == CPoint(entry.x2, entry.y2));
            }
            // Identical approach points make the original uninitialized caller input irrelevant.
            reset(); source->m_curAction.m_actionID = CAIAction::MOVETOOBJECT;
            source->m_curAction.m_acteeID.Set(CAIObjectType::ANYONE); source->m_curAction.m_acteeID.SetInstance(door->GetId());
            door->m_ptDest1 = door->m_ptDest2 = source->m_pos; door->m_pos = CPoint(-999, -999);
            check("move22-door-uses-approach-point", source->ExecuteAction() == CGameAIBase::ACTION_DONE
                && source->m_lastActionID == 123 && source->m_targetId == door->GetId()
                && source->m_currentSearchRequest == NULL);
            CAIObjectType doorSelector(CAIObjectType::ANYONE); doorSelector.SetInstance(door->GetId());
            auto savedMarked = source->field_342;
            for (INT mode : {1, 2, 3, 0}) {
                CAITrigger cause(CAITRIGGER_RANGE, doorSelector, 0); cause.SetInt1(mode);
                check("range-door-uses-approach-point", source->EvaluateStatusTrigger(cause) == (mode == 2));
            }
            check("range-door-publishes-marked-type", source->field_342.Equal(door->GetAIType()));
            source->field_342.Set(savedMarked);
            door->m_ptDest1 = savedFirst; door->m_ptDest2 = savedSecond; door->m_pos = savedDoorPos;
            objects->ReleaseDeny(door->GetId(), CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        discardRequest();
        for (INT cells : {15, 16, 17}) {
            for (SHORT frame : {static_cast<SHORT>(-8), static_cast<SHORT>(-1), static_cast<SHORT>(0),
                     static_cast<SHORT>(1), static_cast<SHORT>(7), static_cast<SHORT>(8), static_cast<SHORT>(32767)}) {
                reset(); source->m_curAction.m_actionID = 47; source->m_bSelected = FALSE;
                target->m_pos = CPoint(source->m_pos.x + cells * 16, source->m_pos.y);
                target->m_curDest = CPoint(target->m_pos.x + 160, target->m_pos.y + 120);
                source->m_curAction.m_dest = CPoint(3, -2); source->field_5616 = frame;
                check("formation-dispatch-interruptable", source->ExecuteAction() == CGameAIBase::ACTION_INTERRUPTABLE
                    && source->m_nLastActionReturn == CGameAIBase::ACTION_INTERRUPTABLE);
                check("formation-offset-is-in-grid-cells", source->m_curDest == CPoint(target->m_curDest.x + 48, target->m_curDest.y - 24));
                check("formation-short-counter", source->field_5616 == static_cast<SHORT>(frame + 1));
                BOOL scheduled = frame == -8 || frame == 0 || frame == 8;
                auto* request = source->m_currentSearchRequest;
                check("formation-throttle", (request != NULL) == scheduled && source->field_5618 == (scheduled ? 1 : 0));
                if (request != NULL) {
                    check("formation-two-exclusive-goals", request->m_nTargetPoints == 2 && request->m_exclusiveTargetPoints == TRUE
                        && request->m_targetPoints[0].x == source->m_curDest.x && request->m_targetPoints[0].y == source->m_curDest.y
                        && request->m_targetPoints[1].x == target->m_curDest.x && request->m_targetPoints[1].y == target->m_curDest.y);
                    check("formation-search-provenance", request->m_sourceId == sourceId && request->m_sourceSide == source->m_typeAI.m_nEnemyAlly
                        && request->m_searchBitmap == &source->m_pArea->m_search
                        && memcmp(request->m_terrainTable, source->m_terrainTable, 16) == 0
                        && request->m_pathSmooth == source->m_animation.GetPathSmooth());
                    BOOL sameParty = request->m_nPartyIds == game->m_group.GetCount();
                    INT member = 0;
                    POSITION partyPos = game->m_group.m_memberList.GetHeadPosition();
                    while (partyPos != NULL) {
                        LONG id = reinterpret_cast<LONG>(game->m_group.m_memberList.GetNext(partyPos));
                        sameParty = sameParty && request->m_partyIds != NULL && request->m_partyIds[member++] == id;
                    }
                    check("formation-party-snapshot", sameParty);
                    check("formation-front-request", request->m_frontList == CSearchRequest::LIST_FRONT
                        && request->m_collisionSearch == FALSE && request->m_collisionDelay == 0);
                }
            }
        }
        reset(); target->m_curDest = CPoint(-1, 999); target->m_pos = CPoint(source->m_pos.x + 320, source->m_pos.y + 240);
        source->m_curAction.m_dest = CPoint(-2, 3);
        check("formation-partial-sentinel-uses-position", source->Formation(target) == CGameAIBase::ACTION_INTERRUPTABLE
            && source->m_curDest == CPoint(target->m_pos.x - 32, target->m_pos.y + 36));
        reset(); target->m_curDest = CPoint(999, -1); target->m_pos = CPoint(source->m_pos.x + 320, source->m_pos.y + 240);
        source->m_curAction.m_dest = CPoint(65537, -65536);
        check("formation-full-width-offset", source->Formation(target) == CGameAIBase::ACTION_INTERRUPTABLE
            && source->m_curDest == CPoint(target->m_pos.x + 1048592, target->m_pos.y - 786432));
        reset(); target->m_curDest = CPoint(-1, -1); target->m_pos = source->m_pos;
        source->m_curAction.m_dest = CPoint(65536, -65536);
        check("formation-distance-square-wraps", source->Formation(target) == CGameAIBase::ACTION_INTERRUPTABLE
            && source->m_curDest == CPoint(-1, -1) && source->field_5616 == 0);
        reset(); source->m_curDest = target->m_curDest = CPoint(2000, 2000); source->field_5618 = 1; source->field_5616 = 7;
        check("formation-same-destination-resets-throttle", source->Formation(target) == CGameAIBase::ACTION_INTERRUPTABLE
            && source->field_5616 == 0 && source->field_5618 == 1 && source->m_currentSearchRequest == NULL);
        struct SpaceCase { BYTE source, target; INT cells; BOOL close; };
        const SpaceCase spaces[] = {{3, 3, 2, TRUE}, {3, 3, 3, FALSE}, {0, 0, 254, TRUE}, {0, 0, 255, FALSE},
            {1, 1, 256, TRUE}, {1, 1, 257, FALSE}, {2, 2, 256, TRUE}, {255, 255, 254, TRUE}, {255, 255, 255, FALSE}};
        for (const auto& entry : spaces) {
            reset(); sourceAnimation->m_personalSpace = entry.source; targetAnimation->m_personalSpace = entry.target;
            target->m_curDest = CPoint(source->m_pos.x + entry.cells * 16, source->m_pos.y);
            check("formation-byte-space-boundary", source->Formation(target) == CGameAIBase::ACTION_INTERRUPTABLE
                && source->field_5616 == (entry.close ? 0 : 2));
        }
        discardRequest();
        check("request-queue-restored", game->m_searchRequests.GetCount() == requestCount
            && game->m_searchRequestsBack.GetCount() == backRequestCount && messages.GetCount() == messageCount);
        check("deny-counts-balanced", objects->m_pArray[(sourceId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == sourceDenies
            && objects->m_pArray[(targetId >> 16) & 0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH] == targetDenies);
        immunities.RemoveAll();
        for (auto* entry : savedImmunities) immunities.AddTail(entry);
        source->m_curAction = savedAction; source->m_typeAI.Set(savedSourceType); target->m_typeAI.Set(savedTargetType);
        source->m_marker = savedSourceMarker; target->m_marker = savedTargetMarker;
        source->m_derivedStats.m_generalState = savedState; source->m_derivedStats.m_spellStates = savedSpells;
        source->m_nUnselectableCounter = savedUnselectable; source->m_moraleFailure = savedMorale; source->m_bSelected = savedSelected;
        source->m_targetId = savedTargetId; source->m_nLastActionReturn = savedReturn;
        source->m_userCommandPause = savedPause; source->m_actionCount = savedCount;
        source->field_5618 = savedPhase; source->field_5616 = savedThrottle;
        source->m_curDest = savedDest; target->m_curDest = savedTargetDest; target->m_pos = savedTargetPos;
        target->m_objectType = savedTargetObjectType; target->m_pArea = savedTargetArea;
        source->m_derivedStats.m_nEncumberance = savedEncumberance; source->m_lSeen.Set(savedSeen); source->m_canBeSeen = savedSeenCount;
        source->m_baseStats.m_flags = savedBaseFlags; source->m_lastActionID = savedLastAction; source->m_followRange = savedFollowRange;
        source->m_pos = savedSourcePos; source->m_objectType = savedSourceObjectType;
        sourceAnimation->m_personalSpace = sourceSpace; targetAnimation->m_personalSpace = targetSpace;
        source->SetSequence(static_cast<BYTE>(savedSequence)); source->m_pathSearchInvalidDest = savedInvalidDest;
        game->m_nTimeStop = savedStop; game->m_searchRequestListEmpty = savedEmpty;
        game->m_hSearchThread = savedSearchSemaphore;
        CloseHandle(testSearchSemaphore);
        objects->ReleaseDeny(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseDeny(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        Emit("{\"step\":%d,\"op\":\"targetresolvechecks\",\"checks\":%d,\"restored\":true,\"valid\":%s}", nStep, checks, valid ? "true" : "false");
        if (!valid) { Fail(nStep, step.op, "target/formation mismatch (state restored)"); return; }
        s_pc++; return;
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

    if (strcmp(step.op, "rebounddamage") == 0) {
        CInfGame* game = g_pBaldurChitin->GetObjectGame();
        CGameObjectArray* objects = game->GetObjectArray();
        LONG sourceId = game->GetFixedOrderCharacterId(0), targetId = game->GetFixedOrderCharacterId(1);
        CGameObject* source = NULL; CGameObject* raw = NULL;
        BYTE sourceRC = objects->GetShare(sourceId, CGameObjectArray::THREAD_ASYNCH, &source, INFINITE);
        BYTE targetRC = objects->GetShare(targetId, CGameObjectArray::THREAD_ASYNCH, &raw, INFINITE);
        if (sourceRC != CGameObjectArray::SUCCESS || targetRC != CGameObjectArray::SUCCESS
            || raw->m_pArea == NULL || raw->m_posVertList == NULL
            || raw->m_listType != CGameObject::LIST_FRONT) {
            if (sourceRC == CGameObjectArray::SUCCESS) objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (targetRC == CGameObjectArray::SUCCESS) objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            Fail(nStep, step.op, "requires two loaded area actors"); return;
        }
        auto* target = static_cast<CGameSprite*>(raw);
        CGameArea* area = target->m_pArea;
        s_projectileSourceHP = static_cast<CGameSprite*>(source)->m_baseStats.m_hitPoints;
        s_projectileTargetHP = target->m_baseStats.m_hitPoints; s_projectileFlags = 0xCF;
        struct StationaryBolt : CProjectileReboundingBolt {
            void Place(CGameArea* area, CPoint point, LONG source) {
                m_pArea = area; CGameObject::m_pArea = area; m_pos = point;
                m_posAccumX = point.x * 1024;
                // Positive ceiling pins the exact head to the creature's pixel.
                m_posAccumY = static_cast<LONG>((static_cast<LONGLONG>(point.y) * 4096 + 2) / 3);
                m_stepX = m_stepY = 0; m_sourceId = source;
                m_targetMap.m_nRange = 0;
            }
        };
        auto* b = new StationaryBolt;
        b->Place(area, target->m_pos, sourceId);
        ITEM_EFFECT item = {}; item.effectID = CGAMEEFFECT_DAMAGE; item.durationType = 1;
        item.probabilityUpper = 100; item.effectAmount = 3; item.dwFlags = 0x800000;
        b->AddEffect(CGameEffect::DecodeEffect(&item, source->m_pos, sourceId, target->m_pos));
        auto& messages = g_pBaldurChitin->GetMessageHandler()->m_messageList;
        INT before = messages.GetCount(); BOOL valid = FALSE;
        if (objects->Add(&b->m_id, b, INFINITE) == CGameObjectArray::SUCCESS) {
            EnterCriticalSection(&g_pBaldurChitin->m_pEngineWorld->field_106);
            b->m_posVertList = area->m_lVertSort.InsertBefore(target->m_posVertList, reinterpret_cast<int*>(b->m_id));
            CProjectile* virtualBolt = b; virtualBolt->AIUpdate();
            valid = b->m_targetMap.m_targets.count(targetId) == 1
                && b->m_targetMap.m_targets[targetId].m_nStrikes == 1 && messages.GetCount() == before + 1;
            area->m_lVertSort.RemoveAt(b->m_posVertList); b->m_posVertList = NULL;
            LeaveCriticalSection(&g_pBaldurChitin->m_pEngineWorld->field_106);
            b->RemoveFromArea(); objects->Delete(b->m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
        }
        delete b;
        objects->ReleaseShare(targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        objects->ReleaseShare(sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (!valid) {
            while (messages.GetCount() > before) delete messages.RemoveTail();
            Fail(nStep, step.op, "proximity service did not deliver exactly one strike"); return;
        }
        Emit("{\"step\":%d,\"op\":\"rebounddamage\",\"queuedDamage\":3,\"recipient\":%ld,\"projectileRemoved\":true}", nStep, targetId);
        ++s_pc; return;
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
            INT queuedBefore=target->m_pArea->m_lVertSortAdd.GetCount();
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
            if(applied && strcmp(step.s[0],"trail")==0) {
                auto& queue=target->m_pArea->m_lVertSortAdd;
                applied=queue.GetCount()==queuedBefore+1;
                if(applied) {
                    LONG id=reinterpret_cast<LONG>(queue.GetTail()); CGameObject* object=NULL;
                    BYTE rc=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
                    auto* projectile=rc==CGameObjectArray::SUCCESS ? dynamic_cast<CProjectileTravelling*>(object) : NULL;
                    applied=projectile!=NULL;
                    if(projectile) { projectile->m_bSparkleTrail=TRUE; projectile->m_sparkleColor=2; }
                    if(rc==CGameObjectArray::SUCCESS) objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                }
                Emit("{\"step\":%d,\"op\":\"projectile-trail-enabled\",\"valid\":%s}",nStep,applied?"true":"false");
            }
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
            auto* defender = static_cast<CGameSprite*>(object);
            SHORT hp = defender->m_baseStats.m_hitPoints;
            objects->ReleaseShare(s_payloadTarget, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            if (s_payloadHP - hp < s_payloadDamage && ++s_waited < 600) {
                return;
            }
            if (s_payloadHP - hp != s_payloadDamage) {
                Fail(nStep, "weaponpayload", "unexpected HP delta or projectile did not arrive");
                return;
            }
            if (swingDamage) {
                CAIObjectType targetType(CAIObjectType::ANYONE);
                targetType.SetInstance(s_payloadTarget);
                SHORT maximum = defender->m_derivedStats.m_nMaxHitPoints;
                BOOL healthValid = defender->EvaluateStatusTrigger(CAITrigger(CAITRIGGER_HP, targetType, hp)) == TRUE
                    && defender->EvaluateStatusTrigger(CAITrigger(CAITRIGGER_HPLT, targetType, s_payloadHP)) == TRUE
                    && defender->EvaluateStatusTrigger(CAITrigger(CAITRIGGER_HPLOST, targetType, maximum - hp)) == TRUE;
                if (maximum != 0) {
                    healthValid = healthValid
                        && defender->EvaluateStatusTrigger(CAITrigger(CAITRIGGER_HPPERCENT, targetType, hp * 100 / maximum)) == TRUE;
                }
                Emit("{\"step\":%d,\"op\":\"damage-health-triggers\",\"hp\":%d,\"maximum\":%d,\"checks\":%d,\"valid\":%s}",
                    nStep, hp, maximum, maximum != 0 ? 4 : 3, healthValid ? "true" : "false");
                if (!healthValid) { Fail(nStep, step.op, "health triggers disagree after actual Swing damage"); return; }
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
