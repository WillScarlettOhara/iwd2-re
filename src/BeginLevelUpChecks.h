// AutoUI-only canonical selector replay and real level-up button dispatch.
#include "BeginLevelUpGolden.h"

void BeginLevelUpChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<1) { Fail(nStep,"beginlevelupchecks","requires party"); return; }
    LONG id=game->GetCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"beginlevelupchecks","actor unavailable"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    SimulatedTransportPause transportPause(g_pChitin->cNetwork);
    CSpell guards[]={CSpell(CResRef("ZRCHIT")),CSpell(CResRef("SPIN260"))};
    BOOL loaded[2]={guards[0].Demand(),guards[1].Demand()};
    if (!loaded[0] || !loaded[1]) {
        for (INT j=0;j<2;++j) if (loaded[j]) guards[j].Release();
        objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"beginlevelupchecks","spell resources unavailable"); return;
    }
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1,clicks=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"begin-levelup-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto base=actor->m_baseStats; CDerivedStats derived,temp,empty; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto type=actor->m_typeAI,start=actor->m_startTypeAI,live=actor->m_liveTypeAI;
    auto spells=actor->m_spells; auto domains=actor->m_domainSpells; auto allow=actor->m_bAllowEffectListCall;
    auto dirty=actor->field_562C; auto deferred=actor->field_72A2; auto remote=actor->m_remotePlayerID;
    auto& network=g_pChitin->cNetwork; auto service=network.m_nServiceProvider;
    auto player=network.m_idLocalPlayer; auto host=network.m_bIsHost;
    auto* oldRefs=game->m_spells.m_pList; UINT oldCount=game->m_spells.m_nCount;
    CResRef refs[]={CResRef("ZRCHIT"),CResRef("SPIN260")}; CSpellListEntry entries[35]={};
    for (UINT i=0;i<35;++i) entries[i].m_nID=i%2;
    CSpellListEntry* data[7][9]; UINT counts[7][9];
    for (INT c=0;c<7;++c) for (INT l=0;l<9;++l) {
        auto& list=game->m_spellsByClass[c].m_lists[l]; data[c][l]=list.m_pList; counts[c][l]=list.m_nCount;
        list.m_pList=entries; list.m_nCount=0;
    }
    game->m_spells.m_pList=refs; game->m_spells.m_nCount=2;
    SPELL_HEADER headers[2]; SPELL_HEADER* oldHeaders[2]; INT demands[2],requests[2];
    for (INT j=0;j<2;++j) { auto* res=guards[j].pRes; oldHeaders[j]=res->m_pHeader;
        headers[j]=*oldHeaders[j]; headers[j].notUsableBy=0; res->m_pHeader=&headers[j];
        demands[j]=res->GetDemands(); requests[j]=res->m_nLockCount; }
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    auto* timer=game->GetWorldTimer(); auto active=timer->m_active; ULONG clock=timer->m_gameTime;
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    {
        BeginLevelUpTableFixture feats(rules.m_tFeats,23,75),xp(rules.m_tExperienceLevels,31,1);
        LevelUpTableFixture bard(rules.m_tKnownSpellsBard),sorc(rules.m_tKnownSpellsSorcerer);
        for (INT r=0;r<75;++r) for (INT c=0;c<23;++c) feats.cells[r*23+c]=(r+c*3)%11==0?"1":"0";
        for (INT i=0;i<31;++i) xp.cells[i].Format(_T("%d"),i==30?-1:i*1000);
        CScreenCharacter screen; CDerivedStats old,before;
        screen.m_pTempDerivedStats=&old; screen.m_pBeforeLevelUpDerivedStats=&before;
        for (const auto& row : BEGIN_LEVELUP_CASES) {
            ++index;
            actor->m_baseStats=CCreatureFileHeader(); actor->m_baseStats.m_specialization=row.spec;
            actor->m_derivedStats=empty; old=empty; before=empty;
            actor->m_derivedStats.m_nLevel=static_cast<BYTE>(row.newLevel);
            actor->m_derivedStats.m_nClassLevels[row.cls-1]=static_cast<BYTE>(row.newLevel);
            old.m_nLevel=static_cast<BYTE>(row.oldLevel); old.m_nClassLevels[row.cls-1]=1;
            before.m_nClassLevels[row.cls-1]=static_cast<BYTE>(row.previousClass);
            for (INT i=0;i<8;++i) actor->m_baseStats.m_favoredEnemies[i]=i<row.slots?1:255;
            actor->m_typeAI=type; actor->m_typeAI.m_nClass=static_cast<BYTE>(row.cls);
            actor->m_typeAI.m_nClassMask=row.cls==10?CLASSMASK_SORCERER:CLASSMASK_BARD;
            for (auto& list : actor->m_spells.m_spellsByClass) list.Clear();
            if (row.known) { CGameSpriteSpellListEntry entry={0,1,1,1};
                for (UINT l=0;l<9;++l) actor->m_spells.GetSpellsAtLevel(row.cls==2?0:5,l)->m_List.push_back(entry); }
            screen.m_nClass=static_cast<BYTE>(row.cls); screen.field_2BC=screen.field_64C=row.points;
            screen.field_1776=static_cast<BYTE>(row.spellLevel); screen.m_nExtraMageSpells=0;
            bard.Set(row.before,row.after); sorc.Set(row.before,row.after);
            for (INT c=0;c<7;++c) for (INT l=0;l<9;++l) game->m_spellsByClass[c].m_lists[l].m_nCount=row.count;
            BOOL result=screen.IsLevelUpPanelNeeded(row.panel,actor); DWORD hash=2166136261u;
            for (UINT f=0;f<75;++f) hash=(hash^actor->GetFeatValue(f))*16777619u;
            check("native-panel-predicate-and-feat-grants",result==row.result && hash==row.featHash);
            check("native-spell-scan-level-and-available-count",screen.field_1776==row.level && screen.m_nExtraMageSpells==row.extra);
        }
        for (const auto& row : LEVELUP_ELIGIBILITY_CASES) {
            ++index; actor->m_derivedStats=empty; actor->m_derivedStats.m_fighterLevel=1;
            actor->m_derivedStats.m_nLevel=1; actor->m_derivedStats.m_nXP=row.next==2?1000:0;
            actor->m_derivedStats.m_generalState=row.state; actor->m_startTypeAI.m_nClass=5;
            actor->m_typeAI.m_nRace=CAIOBJECTTYPE_R_HUMAN; actor->m_typeAI.m_nSubRace=0;
            actor->m_remotePlayerID=row.owner; screen.m_bMultiPlayerViewable=row.view;
            network.m_nServiceProvider=row.service; network.m_idLocalPlayer=42;
            check("native-view-owner-xp-and-death-gates",screen.IsLevelUpButtonClickable(actor)==row.result);
        }
        UINT spellCase=0;
        const DWORD masks[]={CLASSMASK_BARD,CLASSMASK_CLERIC,CLASSMASK_DRUID,CLASSMASK_PALADIN,
            CLASSMASK_RANGER,CLASSMASK_SORCERER,CLASSMASK_WIZARD,CLASSMASK_CLERIC};
        for (INT bank=0;bank<8;++bank) for (UINT stored : {0u,4u,8u})
            for (UINT level : {0u,4u,8u,9u,0xffffffffu}) for (INT kind=0;kind<4;++kind)
            for (UINT current : {0u,1u,0xffffffffu}) for (BYTE required : {BYTE(0),BYTE(1),BYTE(2)})
            for (INT present=0;present<2;++present) {
                ++index;
                for (auto& list : actor->m_spells.m_spellsByClass) list.Clear();
                actor->m_domainSpells.Clear();
                auto* list=bank==7?actor->m_domainSpells.GetSpellsAtLevel(stored):actor->m_spells.GetSpellsAtLevel(bank,stored);
                CGameSpriteSpellListEntry entry={0,1,current,0}; list->m_List.push_back(entry);
                DWORD mask=kind==0?0:kind==1?masks[bank]:kind==2?CLASSMASK_CLERIC:0x7FF;
                BOOLEAN found=actor->HasSpell(present?refs[0]:CResRef("ZRABSENT"),mask,level,required);
                BYTE expected=(HAS_SPELL_RESULTS[spellCase/8]>>(spellCase%8))&1;
                check("native-known-spell-exact-level-class-domain-and-count",found==expected);
                ++spellCase;
            }
        check("all-native-known-spell-cases-replayed",spellCase==8640);
        screen.m_pTempDerivedStats=NULL; screen.m_pBeforeLevelUpDerivedStats=NULL;
        // A private GUIREC uses the actual control, popup stack and snapshot path.
        screen.EngineGameInit(); screen.m_nSelectedCharacter=0;
        auto* oldCharacter=g_pBaldurChitin->m_pEngineCharacter;
        // Controls route through m_pEngineCharacter. Never expose this stack fixture
        // to the independent synchronous renderer through pActiveEngine.
        g_pBaldurChitin->m_pEngineCharacter=&screen;
        auto* animation=actor->m_animation.m_animation; auto* area=actor->m_pArea;
        auto* path=actor->m_pPath; auto* search=actor->m_currentSearchRequest;
        auto target=actor->m_targetPoint; auto action=actor->m_curAction;
        auto destination=actor->m_curDest; auto targetId=actor->m_targetId;
        auto* currentSpell=actor->m_curSpell; auto* currentItem=actor->m_curItem;
        auto currentAction=actor->m_currentActionId; auto tempWeapon=actor->m_nTempSelectedWeapon;
        auto tempAbility=actor->m_nTempSelectedWeaponAbility;
        auto f7118=actor->field_7118; auto f711C=actor->field_711C; auto f5616=actor->field_5616;
        auto startedCasting=actor->m_bStartedCasting; auto inCasting=actor->m_bInCasting;
        auto f5630=actor->field_5630; auto f5632=actor->field_5632; auto f5636=actor->field_5636;
        auto moveCount=actor->m_moveCount; auto stopped=actor->m_bPlayedEncumberedStopped;
        auto slowed=actor->m_bPlayedEncumberedSlowed;
        auto response=actor->m_curResponseNum,responseSet=actor->m_curResponseSetNum,script=actor->m_curScriptNum;
        auto actionCount=actor->m_actionCount; auto interrupt=actor->m_interrupt;
        auto sequence=actor->m_nSequence,direction=actor->m_nDirection,newDirection=actor->m_nNewDirection;
        CItem* items[51]; memcpy(items,actor->m_equipment.m_items,sizeof(items));
        auto weapon=actor->m_equipment.m_selectedWeapon; auto ability=actor->m_equipment.m_selectedWeaponAbility;
        std::vector<CAIAction*> queued;
        while (!actor->m_queuedActions.IsEmpty()) queued.push_back(actor->m_queuedActions.RemoveHead());
        actor->m_pArea=NULL; actor->m_pPath=NULL; actor->m_currentSearchRequest=NULL;
        actor->m_animation.m_animation=CGameAnimationType::SetAnimationType(0x6100,base.m_colors,0);
        actor->m_nSequence=actor->m_animation.SetSequence(CGAMESPRITE_SEQ_HEAD_TURN);
        actor->m_curAction=CAIAction::NULL_ACTION; actor->m_targetId=CGameObjectArray::INVALID_INDEX;
        actor->m_curSpell=NULL; actor->m_curItem=NULL; actor->m_currentActionId=0;
        for (auto& item : actor->m_equipment.m_items) item=NULL;
        actor->m_equipment.m_selectedWeapon=10; actor->m_equipment.m_selectedWeaponAbility=0;
        actor->m_baseStats=base; actor->m_baseStats.m_specialization=0x87654321;
        actor->m_derivedStats=empty; actor->m_derivedStats.m_nLevel=1; actor->m_derivedStats.m_fighterLevel=1;
        actor->m_derivedStats.m_nXP=1000; actor->m_tempStats=actor->m_derivedStats;
        actor->m_typeAI=type; actor->m_typeAI.m_nClass=5; actor->m_typeAI.m_nRace=CAIOBJECTTYPE_R_HUMAN;
        actor->m_typeAI.m_nSubRace=0; actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER;
        actor->m_startTypeAI=actor->m_typeAI; actor->m_liveTypeAI=actor->m_typeAI;
        actor->m_bAllowEffectListCall=FALSE;
        network.m_nServiceProvider=0; network.m_bIsHost=FALSE;
        auto* button=static_cast<CUIControlButtonCharacterLevelUp*>(screen.m_cUIManager.GetPanel(2)->GetControl(37));
        BOOL idle=actor->m_nSequence==CGAMESPRITE_SEQ_HEAD_TURN && !actor->m_animation.IsEndOfSequence();
        check("private-idle-animation-keeps-loaded-sounds-untouched",idle);
        for (INT permitted=0;permitted<2 && idle;++permitted) {
            screen.m_bMultiPlayerViewable=permitted;
            SHORT calls=actor->field_72A2;
            button->OnLButtonClick(CPoint(0,0)); ++clicks;
            check("actual-button-enters-only-when-eligible",screen.m_lPopupStack.GetCount()==permitted);
            if (permitted) {
                check("class-panel-opened-and-class-reset",screen.GetTopPopup()->m_nID==54 && screen.m_nClass==0);
                check("full-specialization-and-native-hp-snapshot",screen.field_77C==0x87654321u
                    && screen.m_nSpecialization==0x87654321u && screen.field_866==actor->m_nHPCONBonusTotalOld);
                check("three-independent-stats-snapshots",screen.m_pTempBaseStats && screen.m_pTempDerivedStats
                    && screen.m_pBeforeLevelUpDerivedStats && screen.m_pTempDerivedStats!=screen.m_pBeforeLevelUpDerivedStats
                    && memcmp(screen.m_pTempBaseStats,&actor->m_baseStats,sizeof(actor->m_baseStats))==0
                    && screen.m_pTempDerivedStats->m_nXP==1000 && screen.m_pBeforeLevelUpDerivedStats->m_fighterLevel==1);
                check("stop-actions-and-deferred-effects",actor->m_targetPoint==CPoint(-1,-1)
                    && actor->m_curAction.m_actionID==CAIAction::NO_ACTION && actor->field_72A2==static_cast<SHORT>(calls+1));
                CMessageStopActions descriptor(id,id,NULL);
                check("stop-message-is-other-station-only",handler->m_messageList.GetCount()==messages
                    && descriptor.GetCommType()==CMessage::BROADCAST_OTHERS
                    && descriptor.GetMsgSubType()==CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_ACTIONS);
                screen.m_bMultiPlayerViewable=FALSE;
                check("real-entry-can-be-cancelled",screen.OnCancelButtonClick()
                    && screen.m_lPopupStack.IsEmpty() && !screen.m_pTempBaseStats
                    && !screen.m_pTempDerivedStats && !screen.m_pBeforeLevelUpDerivedStats);
                screen.m_lPopupStack.RemoveAll();
                delete screen.m_pTempBaseStats; screen.m_pTempBaseStats=NULL;
                delete screen.m_pTempDerivedStats; screen.m_pTempDerivedStats=NULL;
                delete screen.m_pBeforeLevelUpDerivedStats; screen.m_pBeforeLevelUpDerivedStats=NULL;
            }
            while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail();
        }
        delete actor->m_animation.m_animation; actor->m_animation.m_animation=animation;
        actor->m_pArea=area; actor->m_pPath=path; actor->m_currentSearchRequest=search;
        actor->m_curAction=action; actor->m_actionCount=actionCount; actor->m_interrupt=interrupt;
        actor->m_curDest=destination; actor->m_targetId=targetId;
        actor->m_curSpell=currentSpell; actor->m_curItem=currentItem; actor->m_currentActionId=currentAction;
        actor->m_nTempSelectedWeapon=tempWeapon; actor->m_nTempSelectedWeaponAbility=tempAbility;
        actor->field_7118=f7118; actor->field_711C=f711C; actor->field_5616=f5616;
        actor->m_bStartedCasting=startedCasting; actor->m_bInCasting=inCasting;
        actor->field_5630=f5630; actor->field_5632=f5632; actor->field_5636=f5636;
        actor->m_moveCount=moveCount; actor->m_bPlayedEncumberedStopped=stopped; actor->m_bPlayedEncumberedSlowed=slowed;
        actor->m_curResponseNum=response; actor->m_curResponseSetNum=responseSet; actor->m_curScriptNum=script;
        actor->m_targetPoint=target; actor->m_nSequence=sequence; actor->m_nDirection=direction; actor->m_nNewDirection=newDirection;
        for (auto* entry : queued) actor->m_queuedActions.AddTail(entry);
        memcpy(actor->m_equipment.m_items,items,sizeof(items));
        actor->m_equipment.m_selectedWeapon=weapon; actor->m_equipment.m_selectedWeaponAbility=ability;
        g_pBaldurChitin->m_pEngineCharacter=oldCharacter;
    }
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
    actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live; actor->m_spells=spells; actor->m_domainSpells=domains;
    actor->m_bAllowEffectListCall=allow; actor->field_562C=dirty; actor->field_72A2=deferred; actor->m_remotePlayerID=remote;
    network.m_nServiceProvider=service; network.m_idLocalPlayer=player; network.m_bIsHost=host;
    game->m_spells.m_pList=oldRefs; game->m_spells.m_nCount=oldCount;
    for (INT c=0;c<7;++c) for (INT l=0;l<9;++l) {
        game->m_spellsByClass[c].m_lists[l].m_pList=data[c][l]; game->m_spellsByClass[c].m_lists[l].m_nCount=counts[c][l]; }
    for (INT j=0;j<2;++j) { auto* res=guards[j].pRes;
        check("spell-resource-balanced",res->GetDemands()==demands[j] && res->m_nLockCount==requests[j]); res->m_pHeader=oldHeaders[j]; }
    timer->m_active=active;
    check("clock-and-message-queue-restored",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    for (auto& guard : guards) guard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"beginlevelupchecks\",\"nativeCases\":%u,\"hasSpellCases\":8640,\"eligibilityCases\":%u,\"clicks\":%d,\"checks\":%d,\"passed\":%s}",
        nStep,static_cast<UINT>(_countof(BEGIN_LEVELUP_CASES)),static_cast<UINT>(_countof(LEVELUP_ELIGIBILITY_CASES)),clicks,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"beginlevelupchecks","native entry/selector mismatch");
}
