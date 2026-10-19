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
        // Done uses the same loaded actor, locks and GUIREC panels as Cancel.
        actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
        screen.m_nSelectedCharacter=game->GetNumCharacters();
        screen.OnDoneButtonClick();
        check("done-failed-deny-keeps-popup",screen.m_lPopupStack.GetCount()==1);
        screen.m_nSelectedCharacter=0; screen.m_lPopupStack.RemoveAll();
        for (DWORD panel : {4u,12u,18u,52u,53u,57u,9u,10u,50u}) {
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(panel));
            screen.m_nErrorState=0; screen.m_nNumErrorButtons=1;
            screen.OnDoneButtonClick();
            check("done-simple-and-error-return-to-parent",screen.m_lPopupStack.GetCount()==1 && screen.GetTopPopup()->m_nID==12);
        }
        for (DWORD panel : {6u,7u,8u,16u,55u,56u}) {
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(panel));
            screen.m_nClass=CAIOBJECTTYPE_C_WIZARD; screen.m_nSpecialization=0;
            screen.m_nExtraAbilityPoints=screen.m_nExtraMageSpells=screen.m_nExtraFeats=1;
            screen.m_nExtraSkillPoints=2; screen.m_nHatedRaceIndex=0;
            actor->m_baseStats.m_favoredEnemies[0]=CAIOBJECTTYPE_R_NO_RACE;
            screen.OnDoneButtonClick();
            check("done-unspent-budget-gates-dispatch",screen.m_lPopupStack.GetCount()==1 && screen.GetTopPopup()->m_nID==panel);
        }
        screen.m_lPopupStack.RemoveAll();
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(17));
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(18));
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(19));
        screen.m_pPortraits=new CStringList; screen.m_pPortraits->AddTail("SMALL"); screen.m_pPortraits->AddTail("LARGE");
        screen.m_nPortraitSmallIndex=0; screen.m_nPortraitLargeIndex=1;
        screen.OnDoneButtonClick();
        check("done-portraits-close-two-panels-and-retire-list",!screen.m_pPortraits && screen.m_lPopupStack.GetCount()==2
            && screen.GetTopPopup()->m_nID==17 && screen.m_cResPortraitSmallTemp==CResRef("SMALL")
            && screen.m_cResPortraitLargeTemp==CResRef("LARGE"));
        screen.OnDoneButtonClick();
        check("done-customize-commits-portraits",screen.m_lPopupStack.GetCount()==1
            && memcmp(actor->m_baseStats.m_portraitSmall,"SMALL\0\0",8)==0
            && memcmp(actor->m_baseStats.m_portraitLarge,"LARGE\0\0",8)==0);
        screen.m_lPopupStack.RemoveAll();
        {
            auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
            BeginLevelUpTableFixture levelFeats(rules.m_tFeatLevel,1,6),classFeats(rules.m_tFeatClass,11,6);
            ChargenTableFixture skillPoints(rules.m_tSkillPoints); skillPoints.Set("3");
            levelFeats.cells[2]="2"; levelFeats.cells[3]="7";
            classFeats.cells[2*11+4]="3"; classFeats.cells[3*11+4]="11";
            screen.m_pTempBaseStats=new CCreatureFileHeader(base);
            screen.m_pTempDerivedStats=new CDerivedStats; screen.m_pBeforeLevelUpDerivedStats=new CDerivedStats;
            *screen.m_pBeforeLevelUpDerivedStats=derived;
            *screen.m_pTempDerivedStats=derived;
            for (INT previousLevel : {2,4,5}) {
                actor->m_baseStats=base; actor->m_baseStats.m_INTBase=18; actor->m_baseStats.field_2FF=5;
                actor->m_typeAI=type; actor->m_typeAI.m_nRace=CAIOBJECTTYPE_R_ELF;
                actor->m_derivedStats=derived; memset(actor->m_derivedStats.m_nClassLevels,0,11);
                actor->m_derivedStats.m_fighterLevel=4; actor->m_derivedStats.m_nLevel=4;
                actor->m_derivedStats.m_classMask=CLASSMASK_FIGHTER; actor->m_derivedStats.m_nSTR=29;
                *screen.m_pTempDerivedStats=actor->m_derivedStats;
                screen.m_pTempDerivedStats->m_fighterLevel=screen.m_pTempDerivedStats->m_nLevel=previousLevel;
                screen.m_nClass=CAIOBJECTTYPE_C_FIGHTER; screen.m_nExtraAbilityPoints=0;
                screen.field_2BC=screen.field_64C=-99;
                screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(7));
                screen.OnDoneButtonClick();
                check("done-abilities-recompute-budgets",screen.field_2BC==(previousLevel==2?19:5)
                    && screen.field_64C==(previousLevel==2?23:0) && screen.GetTopPopup()->m_nID==55);
                check("done-abilities-restore-derived-snapshot",actor->m_derivedStats.m_nLevel==4
                    && actor->m_derivedStats.m_fighterLevel==4 && actor->m_derivedStats.m_nSTR==29
                    && actor->m_derivedStats.m_classMask==CLASSMASK_FIGHTER);
            }
            screen.m_nClass=CAIOBJECTTYPE_C_WIZARD; screen.m_nSpecialization=SPECMASK_WIZARD_UNIVERSAL;
            screen.field_77C=0x10000000; screen.m_lPopupStack.RemoveAll();
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(6));
            screen.m_pTempDerivedStats->m_nLevel=3;
            screen.OnDoneButtonClick();
            check("done-specialization-merges-and-advances",actor->m_baseStats.m_specialization==(0x10000000|SPECMASK_WIZARD_UNIVERSAL)
                && screen.GetTopPopup()->m_nID==7);
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(55));
            screen.m_nExtraSkillPoints=1; screen.field_64C=1; screen.field_1776=7;
            screen.OnDoneButtonClick();
            check("done-skills-retain-spell-tier-for-wizard",screen.GetTopPopup()->m_nID==56 && screen.field_1776==7);
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(56));
            screen.m_nClass=CAIOBJECTTYPE_C_RANGER; screen.m_nExtraFeats=0; screen.field_1776=9;
            actor->m_derivedStats.m_rangerLevel=5;
            memset(actor->m_baseStats.m_favoredEnemies,CAIOBJECTTYPE_R_NO_RACE,MAX_HATED_RACES);
            screen.OnDoneButtonClick();
            check("done-feats-skip-spells-to-favored-enemy",screen.GetTopPopup()->m_nID==16 && screen.field_1776==9);
            actor->m_baseStats.m_favoredEnemies[0]=CAIOBJECTTYPE_R_HUMAN;
            screen.OnDoneButtonClick();
            check("done-ranger-requests-second-favored-enemy",screen.GetTopPopup()->m_nID==16
                && screen.m_nHatedRaceIndex==1 && actor->m_baseStats.m_favoredEnemies[0]==CAIOBJECTTYPE_R_HUMAN);
            screen.m_lPopupStack.RemoveAll();
            delete screen.m_pTempBaseStats; screen.m_pTempBaseStats=NULL;
            delete screen.m_pTempDerivedStats; screen.m_pTempDerivedStats=NULL;
            delete screen.m_pBeforeLevelUpDerivedStats; screen.m_pBeforeLevelUpDerivedStats=NULL;
        }
        // Remaining Done paths: real script ownership/wire format and export prompts.
        actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
        actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
        screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(11));
        screen.m_pScripts=new CStringList; screen.m_pScripts->AddTail(""); screen.m_nScriptIndex=0;
        screen.OnDoneButtonClick();
        check("done-script-installs-and-retires-list",!screen.m_pScripts && screen.GetTopPopup()->m_nID==12
            && actor->m_special2Script && !actor->m_special2Script->m_cResRef.IsValid());
        {
            auto* remotes=game->GetRemoteObjectArray(); remotes->m_critSect.Lock();
            CGameRemoteObjectListEntry mapping; mapping.remotePlayerID=0x89ABCDEF; mapping.remoteObjectID=0x1234;
            mapping.localObjectID=id; mapping.pNext=remotes->m_pArray[0]; remotes->m_pArray[0]=&mapping;
            auto remotePlayer=actor->m_remotePlayerID; auto remoteId=actor->m_remoteObjectID;
            actor->m_remotePlayerID=mapping.remotePlayerID; actor->m_remoteObjectID=mapping.remoteObjectID;
            BYTE packet[64]={}; BYTE* payload=packet+CNetwork::SPEC_MSG_HEADER_LENGTH;
            CResRef mixed; memcpy(&mixed,"aBcDeFgH",8);
            CMessage97 sent(id,id,0x12340003,mixed); CMessage* message=&sent; DWORD size=0;
            message->MarshalMessage(&payload,&size);
            check("script-message-native-packet",size==20 && sent.GetCommType()==CMessage::BROADCAST_OTHERS
                && sent.GetMsgType()==CBaldurMessage::MSG_TYPE_CMESSAGE && sent.GetMsgSubType()==97
                && *reinterpret_cast<DWORD*>(payload)==0x89ABCDEF && *reinterpret_cast<LONG*>(payload+4)==0x1234
                && *reinterpret_cast<INT*>(payload+8)==0x12340003 && memcmp(payload+12,"aBcDeFgH",8)==0 && payload[20]==0);
            for (DWORD declared : {0u,1u,20u,64u}) {
                CMessage97 received(-77,-1,0,CResRef());
                check("script-message-ignores-declared-size",received.UnmarshalMessage(packet,declared)
                    && received.GetTarget()==id && received.GetSource()==-77 && received.m_nScript==0x12340003
                    && memcmp(received.m_resScript.GetResRef(),"aBcDeFgH",8)==0);
            }
            CMessage97 replace(id,id,0x12340003,CResRef()); replace.Run();
            check("script-message-runs-with-low-word-slot",actor->m_special2Script && !actor->m_special2Script->m_cResRef.IsValid());
            remotes->m_pArray[0]=mapping.pNext;
            CMessage97 missing(-77,-1,0,CResRef());
            check("script-message-unmapped-preserves-state",!missing.UnmarshalMessage(packet,0)
                && missing.GetSource()==-77 && missing.GetTarget()==-1);
            CMessage* absent=&missing; size=99; absent->MarshalMessage(&payload,&size); absent->Run();
            check("script-message-missing-target-empty",size==0);
            actor->m_remotePlayerID=remotePlayer; actor->m_remoteObjectID=remoteId; remotes->m_critSect.Unlock();
        }
        const char* names[]={"  A.B?:<>|*/\\\"  "," AuX ","CON","prn","COM1"};
        const char* exported[]={"A_B_________","AuX_","CON_","prn_","COM1"};
        for (INT i=0;i<5;++i) {
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(13));
            screen.m_pCharacters=new CStringList; screen.m_pCharacters->AddTail(exported[i]);
            static_cast<CUIControlEdit*>(screen.GetTopPopup()->GetControl(6))->SetText(names[i]);
            screen.OnDoneButtonClick();
            check("done-export-sanitizes-before-overwrite-prompt",screen.GetTopPopup()->m_nID==10
                && screen.m_nErrorState==7 && screen.field_176E==exported[i]
                && screen.m_dwErrorTextId==0x685D && screen.m_strErrorButtonText[0]==0x3BCA && screen.m_strErrorButtonText[1]==0x359F);
            screen.OnErrorButtonClick(1); screen.OnCancelButtonClick();
            check("done-export-prompt-cancel-retires-list",!screen.m_pCharacters && screen.m_lPopupStack.IsEmpty());
        }
        for (CString name : {CString(_T("_IWD2_RE_MISSING_VOICE_")),CString('X',40)}) {
            auto& directories=g_pBaldurChitin->cDimm.m_lDirectories; INT count=directories.GetCount();
            check("sound-fixture-directory-absent",GetFileAttributes(game->GetDirSounds()+name)==INVALID_FILE_ATTRIBUTES);
            screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(20));
            screen.m_pSounds=new CStringList; screen.m_pSounds->AddTail(name); screen.m_nCustomSoundSetIndex=0;
            memset(actor->field_725A,0xa5,32); actor->m_secondarySounds=CResRef("OLD");
            BYTE expected[32]; memset(expected,0xa5,32); DWORD length=name.GetAllocLength();
            memcpy(expected,name.GetString(),min(length,32UL)); if (length<32) expected[length]=0;
            screen.OnDoneButtonClick();
            check("done-sound-name-cap-and-missing-wave",!screen.m_pSounds && screen.m_lPopupStack.IsEmpty()
                && memcmp(actor->field_725A,expected,32)==0 && actor->m_secondarySounds==CResRef("OLD"));
            while (directories.GetCount()>count) directories.RemoveTail();
        }
        {
            auto& overrideFile=g_pBaldurChitin->GetTlkTable().m_override;
            BYTE slot=static_cast<BYTE>(game->GetFixedOrderCharacterPortraitNum(id));
            STRREF oldId=actor->m_baseStats.m_biography,newId=0xF240+slot;
            STR_RES oldText,newText,result; BOOL hadOld=overrideFile.Fetch(oldId,oldText),hadNew=overrideFile.Fetch(newId,newText);
            auto connected=network.m_bConnectionEstablished; network.m_bConnectionEstablished=FALSE;
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(12));
            screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(51));
            auto* edit=static_cast<CUIControlEditMultiLine*>(screen.GetTopPopup()->GetControl(4));
            edit->SetText(_T("Recovered biography\r\nSecond line"));
            check("biography-editor-normalizes-carriage-returns",edit->GetText()==_T("Recovered biography\nSecond line"));
            screen.OnDoneButtonClick();
            check("done-biography-commits-fixed-slot",screen.GetTopPopup()->m_nID==12 && actor->m_baseStats.m_biography==newId);
            check("done-biography-stores-override",overrideFile.Fetch(newId,result));
            check("done-biography-commits-editor-text",result.szText==_T("Recovered biography\nSecond line"));
            overrideFile.Remove(newId); if (hadNew) overrideFile.AddUserEntry(newId,newText);
            if (hadOld && oldId!=newId) overrideFile.AddUserEntry(oldId,oldText);
            actor->m_baseStats.m_biography=oldId; network.m_bConnectionEstablished=connected;
            screen.m_lPopupStack.RemoveAll();
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
