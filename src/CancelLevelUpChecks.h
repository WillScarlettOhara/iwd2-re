// AutoUI-only canonical done gates and loaded-world cancellation fixtures.
#include "CancelLevelUpGolden.h"

void CancelLevelUpChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<1) { Fail(nStep,"cancellevelupchecks","requires party"); return; }
    LONG id=game->GetCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"cancellevelupchecks","actor unavailable"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    SimulatedTransportPause transportPause(g_pChitin->cNetwork);
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1,cancels=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"cancel-levelup-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto base=actor->m_baseStats; CDerivedStats derived,temp; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto type=actor->m_typeAI,start=actor->m_startTypeAI,live=actor->m_liveTypeAI;
    auto spells=actor->m_spells; auto domains=actor->m_domainSpells; auto secondary=actor->m_secondarySounds;
    BYTE sound[32]; memcpy(sound,actor->field_725A,sizeof(sound));
    auto* oldScript=actor->m_special2Script; actor->m_special2Script=NULL;
    auto allow=actor->m_bAllowEffectListCall; auto dirty=actor->field_562C;
    auto deferred=actor->field_72A2; auto hpBonus=actor->m_nHPCONBonusTotalOld;
    auto& network=g_pChitin->cNetwork; auto service=network.m_nServiceProvider; auto host=network.m_bIsHost;
    auto* timer=game->GetWorldTimer(); auto active=timer->m_active; auto clock=timer->m_gameTime;
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto* oldCharacter=g_pBaldurChitin->m_pEngineCharacter;
    {
        CScreenCharacter screen; screen.EngineGameInit(); screen.m_nSelectedCharacter=0;
        // Only controls need this routing; the render thread must retain the live engine.
        g_pBaldurChitin->m_pEngineCharacter=&screen;
        screen.m_bMultiPlayerViewable=FALSE; screen.m_bMultiPlayerModifyable=TRUE;
        for (const auto& row : DONE_GATE_CASES) {
            ++index; screen.m_lPopupStack.RemoveAll();
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(row.panel));
            screen.m_nClass=row.cls; screen.m_nSpecialization=row.spec;
            screen.m_nExtraAbilityPoints=screen.m_nExtraMageSpells=screen.m_nScriptIndex=row.points;
            screen.m_nCustomSoundSetIndex=screen.m_nExtraSkillPoints=screen.m_nExtraFeats=row.points;
            screen.m_nPortraitSmallIndex=row.smallPortrait; screen.m_nPortraitLargeIndex=row.largePortrait;
            screen.m_nHatedRaceIndex=row.index; actor->m_baseStats.m_favoredEnemies[row.index]=row.race;
            BOOL result=screen.IsDoneButtonClickable(actor);
            check("canonical-done-panel-gates",result==row.result);
            Emit("{\"op\":\"native-done-gate\",\"case\":%d,\"input\":[%lu,%u,%lu,%d,%d,%d,%d,%u],\"result\":%d}",
                index,row.panel,static_cast<UINT>(row.cls),row.spec,row.points,row.smallPortrait,row.largePortrait,row.index,static_cast<UINT>(row.race),result);
        }
        screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(13));
        auto* edit=static_cast<CUIControlEdit*>(screen.GetTopPopup()->GetControl(6));
        for (UINT i=0;i<_countof(DONE_GATE_TEXTS);++i) {
            ++index; edit->SetText(DONE_GATE_TEXTS[i]);
            BOOL result=screen.IsDoneButtonClickable(actor);
            check("canonical-export-text-trimming",result==DONE_GATE_TEXT_RESULTS[i]);
            Emit("{\"op\":\"native-done-text\",\"case\":%u,\"result\":%d}",i,result);
        }
        screen.m_lPopupStack.RemoveAll();
        for (const char* text : {"","ABCDEFGH","aBcDeFgH","ZRCHIT"}) {
            ++index; CAIScript script; CResRef ref; BYTE raw[8]={};
            memcpy(raw,text,min(strlen(text),sizeof(raw))); memcpy(&ref,raw,sizeof(raw));
            auto listCount=script.m_caList.GetCount(); script.field_8=0x12345678;
            script.SetResRef(ref);
            check("canonical-eight-byte-script-reference",memcmp(script.m_cResRef.GetResRef(),raw,8)==0
                && script.field_8==0x12345678 && script.m_caList.GetCount()==listCount);
        }
        actor->m_baseStats=base; actor->m_bAllowEffectListCall=FALSE;
        network.m_nServiceProvider=CNetwork::SERV_PROV_NULL; network.m_bIsHost=FALSE;

        // Failed object lookup leaves the popup and actor alone.
        screen.m_nSelectedCharacter=game->GetNumCharacters();
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
        check("failed-deny-returns-false-without-dismissal",!screen.OnCancelButtonClick()
            && screen.m_lPopupStack.GetCount()==1 && memcmp(&actor->m_baseStats,&base,sizeof(base))==0);
        screen.m_nSelectedCharacter=0; screen.m_lPopupStack.RemoveAll();

        const DWORD simplePanels[]={4,9,10,11,12,13,17,18,19,20,50,51,52,53,57};
        for (DWORD panel : simplePanels) for (INT longName=0;longName<3;++longName) {
            ++index; actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
            actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
            memset(actor->field_725A,0xa5,32); actor->m_secondarySounds=CResRef("OLD");
            screen.m_cResPortraitSmall=CResRef("SMALL"); screen.m_cResPortraitLarge=CResRef("LARGE");
            screen.m_cResSoundSet=CResRef("SOUNDS"); screen.field_856=longName==1?CString('X',40):CString(_T("voice"));
            if (longName==2) screen.field_856.Preallocate(64);
            INT capacity=screen.field_856.GetAllocLength(),length=screen.field_856.GetLength();
            LPTSTR nameBuffer=screen.field_856.GetBuffer(capacity);
            for (INT j=length+1;j<capacity;++j) nameBuffer[j]='Z';
            screen.field_856.ReleaseBuffer(length);
            BYTE expectedSound[32]; memset(expectedSound,0xa5,sizeof(expectedSound));
            memcpy(expectedSound,screen.field_856.GetString(),min(capacity,32));
            if (capacity<32) expectedSound[capacity]=0;
            // Empty script takes the real failed-resource path without loading another actor's script.
            screen.field_832="";
            if (panel==11) screen.m_pScripts=new CStringList;
            if (panel==13) screen.m_pCharacters=new CStringList;
            if (panel==19) screen.m_pPortraits=new CStringList;
            if (panel==20) screen.m_pSounds=new CStringList;
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(panel));
            check("simple-popup-cancel-returns-to-parent",screen.OnCancelButtonClick()
                && screen.m_lPopupStack.GetCount()==1 && screen.GetTopPopup()->m_nID==12); ++cancels;
            if (panel==11) check("script-list-deleted",screen.m_pScripts==NULL);
            if (panel==13) check("export-list-deleted",screen.m_pCharacters==NULL);
            if (panel==19) check("portrait-list-deleted",screen.m_pPortraits==NULL);
            if (panel==20) check("sound-list-deleted",screen.m_pSounds==NULL);
            if (panel==17 || panel==18) {
                check("portraits-restored-before-parent",memcmp(actor->m_baseStats.m_portraitSmall,"SMALL\0\0",8)==0
                    && memcmp(actor->m_baseStats.m_portraitLarge,"LARGE\0\0",8)==0
                    && screen.m_cResPortraitSmallTemp==screen.m_cResPortraitSmall
                    && screen.m_cResPortraitLargeTemp==screen.m_cResPortraitLarge);
            }
            if (panel==17 || panel==20) {
                BOOL same=memcmp(actor->field_725A,expectedSound,sizeof(expectedSound))==0;
                check("sound-name-exact-cap-and-terminator",same && actor->m_secondarySounds==CResRef("SOUNDS"));
            }
            if (panel==17) {
                check("actual-script-installation",actor->m_special2Script!=NULL && !actor->m_special2Script->m_cResRef.IsValid());
                delete actor->m_special2Script; actor->m_special2Script=NULL;
            }
        }

        const DWORD levelPanels[]={6,7,8,16,54,55,56};
        for (DWORD panel : levelPanels) for (INT dead=0;dead<2;++dead) {
            ++index; actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
            actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
            screen.m_lPopupStack.RemoveAll(); screen.m_nSelectedCharacter=0;
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(panel));
            screen.m_nClass=CAIOBJECTTYPE_C_FIGHTER; screen.m_typeAI=start;
            screen.field_2BC=panel==56?1:0; screen.field_64C=panel==8 || panel==16?1:0;
            screen.field_77C=0x87654321; screen.field_866=-77;
            screen.m_pTempBaseStats=new CCreatureFileHeader(base);
            screen.m_pTempDerivedStats=new CDerivedStats;
            screen.m_pBeforeLevelUpDerivedStats=new CDerivedStats;
            *screen.m_pTempDerivedStats=derived; *screen.m_pBeforeLevelUpDerivedStats=derived;
            screen.m_pTempDerivedStats->m_nLevel=1; screen.m_pTempDerivedStats->m_fighterLevel=1;
            screen.m_pBeforeLevelUpDerivedStats->m_fighterLevel=1;
            actor->m_derivedStats.m_nLevel=panel==55?4:2; actor->m_derivedStats.m_fighterLevel=actor->m_derivedStats.m_nLevel;
            actor->m_derivedStats.m_generalState=dead?STATE_DEAD:0;
            screen.m_pTempBaseStats->m_hitPoints=-32768; screen.m_pTempBaseStats->m_maxHitPointsBase=65535;
            screen.m_pTempBaseStats->m_generalState=0x4000;
            screen.m_pTempDerivedStats->m_nMaxHitPoints=32767;
            screen.m_pTempDerivedStats->m_generalState=0x1000;
            for (INT j=0;j<6;++j) reinterpret_cast<BYTE*>(&screen.m_pTempBaseStats->m_STRBase)[j]=static_cast<BYTE>(20+j);
            for (INT j=0;j<8;++j) screen.m_pTempBaseStats->m_favoredEnemies[j]=static_cast<BYTE>(10+j);
            for (INT j=0;j<CGAMESPRITE_SKILL_NUMSKILLS;++j) screen.m_storedSkills[j]=j+3;
            for (INT j=0;j<CGAMESPRITE_FEAT_NUMFEATS;++j) screen.m_storedFeats[j]=0;
            CGameSpriteSpellListEntry entry={321,7,5,3};
            for (UINT cl=0;cl<7;++cl) for (UINT level=0;level<9;++level) {
                auto* list=screen.m_pTempDerivedStats->m_spells.GetSpellsAtLevel(cl,level);
                list->m_List.clear(); list->m_List.push_back(entry);
            }
            for (UINT level=0;level<9;++level) {
                auto& list=screen.m_spells.m_lists[level]; list.m_List.clear();
                if (!(level%2)) list.m_List.push_back(entry);
                list.m_nSharedCurrent=100+level; list.m_nSharedMax=200+level; list.m_nSharedTotal=300+level;
                auto& domain=screen.m_pTempDerivedStats->field_CAC.m_lists[level];
                domain.m_List.clear(); domain.m_List.push_back(entry);
            }
            screen.m_spells.m_nHighestLevel=8;
            auto effects=actor->field_72A2;
            check("level-popup-cancellation",screen.OnCancelButtonClick()); ++cancels;
            DWORD expectedPanel=panel==54?0:panel==8 || panel==16?56:panel==56?55:panel==55?7:54;
            check("real-reverse-panel-selection",expectedPanel?screen.GetTopPopup()!=NULL
                && screen.GetTopPopup()->m_nID==expectedPanel:screen.m_lPopupStack.IsEmpty());
            if (panel==6 || panel==7 || panel==54) {
                check("hp-and-death-survive-rollback",actor->m_baseStats.m_hitPoints==-32768
                    && actor->m_baseStats.m_maxHitPointsBase==65535 && actor->m_derivedStats.m_nMaxHitPoints==32767
                    && actor->m_nHPCONBonusTotalOld==-77 && actor->field_72A2==static_cast<SHORT>(effects+1)
                    && actor->m_derivedStats.m_generalState==(0x1000u|(dead?STATE_DEAD:0))
                    && actor->m_baseStats.m_generalState==(0x4000u|(dead?STATE_DEAD:0)));
            }
            if (panel==8) {
                for (UINT level=0;level<9;++level) {
                    auto& list=screen.m_spells.m_lists[level];
                    check("native-empty-list-aggregate-quirk",list.m_List.empty() && list.m_nSharedCurrent==100+level
                        && list.m_nSharedMax==(level%2?200+level:0) && list.m_nSharedTotal==(level%2?300+level:0));
                    auto& domain=actor->m_domainSpells.m_lists[level];
                    check("domain-bank-deep-restoration",domain.m_List.size()==1 && domain.m_List[0].m_nID==321
                        && &domain.m_List[0]!=&screen.m_pTempDerivedStats->field_CAC.m_lists[level].m_List[0]);
                    for (UINT cl=0;cl<7;++cl) {
                        auto* list=actor->m_spells.GetSpellsAtLevel(cl,level);
                        check("class-bank-deep-restoration",list->m_List.size()==1 && list->m_List[0].m_nCurrent==5
                            && &list->m_List[0]!=&screen.m_pTempDerivedStats->m_spells.GetSpellsAtLevel(cl,level)->m_List[0]);
                    }
                }
                check("temporary-spell-highest-reset",screen.m_spells.m_nHighestLevel==0);
            }
            if (panel==16) check("favored-enemies-restored",memcmp(actor->m_baseStats.m_favoredEnemies,"\12\13\14\15\16\17\20\21",8)==0);
            if (panel==54) check("snapshots-retired-on-abort",screen.m_pTempBaseStats==NULL
                && screen.m_pTempDerivedStats==NULL && screen.m_pBeforeLevelUpDerivedStats==NULL
                && screen.field_866==0 && screen.m_nSpecialization==0x87654321u);
            if (panel==55) {
                check("skill-scroll-reset",screen.m_nTopSkill==CGAMESPRITE_SKILL_NUMSKILLS);
                for (UINT skill=0;skill<CGAMESPRITE_SKILL_NUMSKILLS;++skill) check("stored-skills-restored",actor->m_baseStats.m_skills[skill]==skill+3);
            }
            if (panel==56) for (UINT feat=0;feat<CGAMESPRITE_FEAT_NUMFEATS;++feat) check("stored-feats-restored",actor->GetFeatValue(feat)==0);
            screen.m_lPopupStack.RemoveAll();
            delete screen.m_pTempBaseStats; screen.m_pTempBaseStats=NULL;
            delete screen.m_pTempDerivedStats; screen.m_pTempDerivedStats=NULL;
            delete screen.m_pBeforeLevelUpDerivedStats; screen.m_pBeforeLevelUpDerivedStats=NULL;
        }
        // Exercise the actual GUIREC Cancel control with independent snapshots.
        actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
        actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(54));
        screen.m_pTempBaseStats=new CCreatureFileHeader(base); screen.m_pTempDerivedStats=new CDerivedStats;
        screen.m_pBeforeLevelUpDerivedStats=new CDerivedStats;
        *screen.m_pTempDerivedStats=derived; *screen.m_pBeforeLevelUpDerivedStats=derived;
        screen.m_typeAI=start; screen.field_866=hpBonus; screen.field_77C=base.m_specialization;
        CUIControlButtonCharacterPopupCancel* button=NULL;
        POSITION pos=screen.GetTopPopup()->m_lControls.GetHeadPosition();
        while (pos && !button) button=dynamic_cast<CUIControlButtonCharacterPopupCancel*>(screen.GetTopPopup()->m_lControls.GetNext(pos));
        check("actual-cancel-control-present",button!=NULL);
        if (button) { button->OnLButtonClick(CPoint(0,0)); ++cancels; }
        check("actual-cancel-control-clears-snapshots",screen.m_lPopupStack.IsEmpty()
            && !screen.m_pTempBaseStats && !screen.m_pTempDerivedStats && !screen.m_pBeforeLevelUpDerivedStats);
        screen.m_lPopupStack.RemoveAll();
        delete screen.m_pTempBaseStats; screen.m_pTempBaseStats=NULL;
        delete screen.m_pTempDerivedStats; screen.m_pTempDerivedStats=NULL;
        delete screen.m_pBeforeLevelUpDerivedStats; screen.m_pBeforeLevelUpDerivedStats=NULL;
        g_pBaldurChitin->m_pEngineCharacter=oldCharacter;
    }
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
    actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
    actor->m_spells=spells; actor->m_domainSpells=domains; actor->m_secondarySounds=secondary;
    memcpy(actor->field_725A,sound,sizeof(sound)); delete actor->m_special2Script; actor->m_special2Script=oldScript;
    actor->m_bAllowEffectListCall=allow; actor->field_562C=dirty; actor->field_72A2=deferred; actor->m_nHPCONBonusTotalOld=hpBonus;
    network.m_nServiceProvider=service; network.m_bIsHost=host; timer->m_active=active;
    check("clock-and-message-queue-unchanged",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"cancellevelupchecks\",\"nativeCases\":1279,\"gateReplays\":321,\"cancels\":%d,\"checks\":%d,\"passed\":%s}",nStep,cancels,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"cancellevelupchecks","cancel/reverse/gate mismatch");
}
