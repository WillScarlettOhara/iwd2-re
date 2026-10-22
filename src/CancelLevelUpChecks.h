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
        {
            // Park the owning objects intact; only fixture effects may be retired.
            std::vector<BYTE> eqBytes(sizeof(CGameEffectList)),timeBytes(sizeof(CGameEffectList)),statsBytes(sizeof(CDerivedStats));
            memcpy(eqBytes.data(),&actor->m_equipedEffectList,eqBytes.size());
            memcpy(timeBytes.data(),&actor->m_timedEffectList,timeBytes.size());
            memcpy(statsBytes.data(),&actor->m_derivedStats,statsBytes.size());
            new (&actor->m_equipedEffectList) CGameEffectList;
            new (&actor->m_timedEffectList) CGameEffectList;
            new (&actor->m_derivedStats) CDerivedStats;
            auto* oldVariables=actor->m_pLocalVariables; auto removed=actor->m_removeFromArea;
            BYTE saves[5]; memcpy(saves,&actor->field_70F6,sizeof(saves));
            auto* oldMisc=actor->m_equipment.m_items[CGameSpriteEquipment::SLOT_MISC+27];
            CItem* borrowed=NULL;
            for (INT i=0;i<CGameSpriteEquipment::NUM_SLOT && !borrowed;++i) {
                if (i!=CGameSpriteEquipment::SLOT_FIST) borrowed=actor->m_equipment.m_items[i];
            }
            check("cre-fixture-has-item-for-last-misc-slot",borrowed!=NULL);
            actor->m_equipment.m_items[CGameSpriteEquipment::SLOT_MISC+27]=borrowed;
            const LONG bonuses[]={0x12340007,static_cast<LONG>(0xFFFF8000),0x00010001};
            const SHORT hitPoints[]={-32760,32760,-1},maxPoints[]={32760,-32760,0};
            const WORD baseMax[]={4,65535,0};
            for (INT pass=0;pass<3;++pass) {
                actor->m_baseStats=base; actor->m_startTypeAI=start; actor->m_startTypeAI.SetSubRace(0xD7);
                actor->m_startTypeAI.m_nGeneral=0x45; actor->m_liveTypeAI=live; actor->m_liveTypeAI.m_nGeneral=0xA5;
                actor->m_removeFromArea=0x12340002; actor->m_nHPCONBonusTotalOld=bonuses[pass];
                actor->m_baseStats.m_hitPoints=hitPoints[pass]; actor->m_baseStats.m_maxHitPointsBase=baseMax[pass];
                actor->m_derivedStats.m_nMaxHitPoints=maxPoints[pass]; actor->m_typeAI=type;
                CVariableHash locals(8); actor->m_pLocalVariables=&locals;
                locals.m_hashEntries[0].m_intValue=123; // Empty names are skipped without clearing their data.
                const char* value="AbCdEfGhIjKlMnOpQrStUvWxYz12345";
                for (INT i=0;i<4;++i) {
                    auto& variable=locals.m_hashEntries[2*i+1]; CString name; name.Format(_T("LOCAL%d"),i);
                    variable.SetName(name); variable.m_type=static_cast<WORD>(0xFFF0+i);
                    variable.m_resRefType=static_cast<WORD>(0xFED0+i); variable.m_dwValue=0x89ABCDEF+i;
                    variable.m_intValue=-12345-i; variable.m_floatValue=i==3?4294967297.75:-7.75;
                    memcpy(variable.m_stringValue,value,i==0?8:(i==1?9:(i==2?17:31)));
                }
                std::vector<BYTE> variables(8*sizeof(CVariable)); memcpy(variables.data(),locals.m_hashEntries,variables.size());
                auto add=[&](CGameEffectList& list,WORD opcode,DWORD duration)->POSITION {
                    auto* effect=new CGameEffect; effect->m_effectID=opcode; effect->m_durationType=duration;
                    return list.AddTail(effect);
                };
                add(actor->m_equipedEffectList,CGAMEEFFECT_JUMPTOAREA,9);
                auto eqCurrent=add(actor->m_equipedEffectList,CGAMEEFFECT_JUMPTOAREA,9);
                actor->m_equipedEffectList.m_posCurrent=eqCurrent;
                actor->m_equipedEffectList.m_posNext=actor->m_equipedEffectList.GetHeadPosition();
                add(actor->m_timedEffectList,CGAMEEFFECT_JUMPTOAREA,9);
                auto timeCurrent=add(actor->m_timedEffectList,CGAMEEFFECT_JUMPTOAREA,9);
                add(actor->m_timedEffectList,CGAMEEFFECT_DAMAGE,9); add(actor->m_timedEffectList,CGAMEEFFECT_DAMAGE,0);
                actor->m_timedEffectList.m_posCurrent=timeCurrent;
                actor->m_timedEffectList.m_posNext=actor->m_timedEffectList.GetHeadPosition();
                CCreatureFileEquipment equipment; CCreatureFileItem* items=NULL; LONG itemCount=0;
                actor->m_equipment.Marshal(&equipment,&items,&itemCount,TRUE); delete[] items;
                if (!pass) equipment.m_miscItem[27]=0xFFFF;
                BYTE* creature=NULL; LONG size=0; WORD facing=0;
                actor->Marshal(&creature,&size,&facing,static_cast<BYTE>(pass),TRUE);
                auto* header=reinterpret_cast<CCreatureFileHeader*>(creature+8);
                auto* offsets=reinterpret_cast<CCreatureFileOffsets*>(creature+0x384);
                SHORT bonus=static_cast<SHORT>(bonuses[pass]);
                check("cre-hp-bonus-short-subtraction-and-wrap",header->m_hitPoints==static_cast<SHORT>(hitPoints[pass]-bonus)
                    && header->m_maxHitPointsBase==static_cast<WORD>(baseMax[pass]-bonus));
                check("cre-live-hp-restored",actor->m_baseStats.m_hitPoints==hitPoints[pass]
                    && actor->m_baseStats.m_maxHitPointsBase==baseMax[pass] && actor->m_derivedStats.m_nMaxHitPoints==maxPoints[pass]);
                check("cre-header-mutations-and-facing",header->m_effectVersion==1 && header->m_subrace==0xD7
                    && header->m_bRemoveFromArea==2 && actor->m_baseStats.m_subrace==0xD7
                    && actor->m_baseStats.m_bRemoveFromArea==2 && facing==actor->m_nDirection);
                check("cre-general-from-live-ai",offsets->m_general==0xA5
                    && offsets->m_enemyAlly==start.m_nEnemyAlly && offsets->m_race==start.m_nRace);
                check("cre-equipment-export-last-slot-quirk",memcmp(creature+offsets->m_equipmentListOffset,&equipment,sizeof(equipment))==0);
                check("cre-effects-filtered-before-cleanup",offsets->m_effectListCount==(pass?8u:3u)
                    && offsets->m_effectListOffset+offsets->m_effectListCount*sizeof(CGameEffectBase)==static_cast<DWORD>(size));
                check("cre-jump-effects-retired-except-current",actor->m_equipedEffectList.GetCount()==1
                    && actor->m_timedEffectList.GetCount()==(pass?7:3)
                    && actor->m_equipedEffectList.m_posCurrent==eqCurrent && actor->m_equipedEffectList.m_posNext==eqCurrent
                    && actor->m_timedEffectList.m_posCurrent==timeCurrent && actor->m_timedEffectList.m_posNext==timeCurrent);
                if (!pass) {
                    check("cre-export-preserves-local-table",memcmp(variables.data(),locals.m_hashEntries,variables.size())==0);
                } else {
                    CVariable empty;
                    for (INT i=0;i<4;++i) {
                        const auto* effect=reinterpret_cast<const CGameEffectBase*>(creature+offsets->m_effectListOffset)+(4+i);
                        char name[32]={}; sprintf(name,"LOCAL%d",i);
                        BYTE resources[24]={}; memcpy(resources,value,i==0?8:(i==1?9:(i==2?17:24)));
                        for (auto& ch : resources) if (ch>='a' && ch<='z') ch-=32;
                        check("cre-local-effect-full-width-values",effect->m_effectID==CGAMEEFFECT_SETLOCALVARIABLE
                            && effect->m_durationType==1 && effect->m_flags==static_cast<WORD>(0xFFF0+i)
                            && effect->m_effectAmount2==static_cast<WORD>(0xFED0+i) && effect->m_dwFlags==0x89ABCDEF+i
                            && effect->m_effectAmount==-12345-i && effect->m_effectAmount3==(i==3?1:-7)
                            && memcmp(effect->m_scriptName,name,32)==0);
                        check("cre-local-effect-three-uppercase-resource-chunks",memcmp(effect->m_res.GetResRef(),resources,8)==0
                            && memcmp(effect->m_res2.GetResRef(),resources+8,8)==0 && memcmp(effect->m_res3.GetResRef(),resources+16,8)==0);
                        check("cre-local-table-slot-consumed",memcmp(&locals.m_hashEntries[2*i+1],&empty,sizeof(empty))==0);
                    }
                    check("cre-empty-local-name-data-retained",locals.m_hashEntries[0].m_intValue==123);
                    INT count=actor->m_timedEffectList.GetCount(); locals.MarshalToCharacter(actor);
                    check("cre-consumed-locals-not-duplicated",actor->m_timedEffectList.GetCount()==count);
                }
                delete[] creature;
                while (!actor->m_equipedEffectList.IsEmpty()) delete actor->m_equipedEffectList.RemoveHead();
                while (!actor->m_timedEffectList.IsEmpty()) delete actor->m_timedEffectList.RemoveHead();
                actor->m_equipedEffectList.m_posCurrent=actor->m_equipedEffectList.m_posNext=NULL;
                actor->m_timedEffectList.m_posCurrent=actor->m_timedEffectList.m_posNext=NULL;
                actor->m_pLocalVariables=oldVariables;
            }
            actor->m_equipment.m_items[CGameSpriteEquipment::SLOT_MISC+27]=oldMisc;
            actor->m_removeFromArea=removed; actor->m_nHPCONBonusTotalOld=hpBonus; memcpy(&actor->field_70F6,saves,sizeof(saves));
            actor->m_equipedEffectList.~CGameEffectList(); actor->m_timedEffectList.~CGameEffectList(); actor->m_derivedStats.~CDerivedStats();
            memcpy(&actor->m_equipedEffectList,eqBytes.data(),eqBytes.size());
            memcpy(&actor->m_timedEffectList,timeBytes.data(),timeBytes.size()); memcpy(&actor->m_derivedStats,statsBytes.data(),statsBytes.size());
            actor->m_baseStats=base; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
        }
        {
            // The existing profile owns temp, including cleanup after a crash.
            auto& dimm=g_pChitin->cDimm; CString directory=game->m_sTempDir+"_chr_export\\";
            CResRef ref("ZRCHREX"); CString previousDirectory=game->m_sCharactersDir;
            BOOL isolated=GetFileAttributes(directory)==INVALID_FILE_ATTRIBUTES
                && !dimm.m_cKeyTable.FindKey(ref,1018,1) && !dimm.m_cKeyTable.FindKey(ref,1022,1)
                && !dimm.m_cKeyTable.FindKey(CResRef("ZRCHRNOB"),1018,1) && !dimm.m_cKeyTable.FindKey(CResRef("ZRCHRNOB"),1022,1)
                && !dimm.m_cKeyTable.FindKey(CResRef("ZRBADCHR"),1018,1) && !dimm.m_cKeyTable.FindKey(CResRef("ZRMISS"),1018,1);
            check("character-export-fixture-isolated",isolated);
            if (isolated) {
                game->m_sCharactersDir=directory;
                auto& overrides=g_pBaldurChitin->GetTlkTable().m_override;
                const STRREF biographyId=0xF2AA; STR_RES previousText,text;
                BOOL hadText=overrides.Fetch(biographyId,previousText);
                auto oldName=actor->m_sName;
                CButtonData weapons[8],spells[9],items[3],innates[9],songs[9];
                memcpy(weapons,actor->m_quickWeapons,sizeof(weapons)); memcpy(spells,actor->m_quickSpells,sizeof(spells));
                memcpy(items,actor->m_quickItems,sizeof(items)); memcpy(innates,actor->m_quickInnates,sizeof(innates));
                memcpy(songs,actor->m_quickSongs,sizeof(songs));
                INT customButtons[9]; BYTE weaponButtons[8];
                memcpy(customButtons,actor->field_3D14,sizeof(customButtons)); memcpy(weaponButtons,actor->field_3D3A,sizeof(weaponButtons));
                auto lastClass=actor->m_nLastSpellbookClassIndex,lastLevel=actor->m_nLastSpellbookSpellLevel,lastSong=actor->m_nLastSong;
                for (INT pass=0;pass<2;++pass) {
                    actor->m_baseStats=base; actor->m_baseStats.m_name=-1; actor->m_baseStats.m_biography=biographyId;
                    actor->m_sName=pass?CString('N',40):CString(_T("Export test"));
                    text.szText=pass?CString():CString(_T("Export biography\r\nSecond line"));
                    overrides.AddUserEntry(biographyId,text);
                    actor->m_secondarySounds=CResRef("VOICE"); memset(actor->field_725A,0,32);
                    memset(actor->field_725A,'V',pass?31:5);
                    BYTE expected[0x224]={}; memcpy(expected,"CHR V2.2",8);
                    memcpy(expected+8,actor->m_sName.GetString(),min(actor->m_sName.GetLength(),32));
                    for (INT i=0;i<9;++i) {
                        char resource[9]; sprintf(resource,"SpElL%03d",i);
                        auto& ability=actor->m_quickSpells[i].m_abilityId; memcpy(&ability.m_res,resource,8);
                        ability.m_nClass=static_cast<BYTE>(0xE0+i); ability.m_bCanUse=static_cast<BYTE>(0x80+i);
                        ability.m_nTooltip=static_cast<SHORT>(0xABF0+i);
                        memcpy(&actor->m_quickInnates[i].m_abilityId.m_res,"InNaTe12",8);
                        memcpy(&actor->m_quickSongs[i].m_abilityId.m_res,"SoNg1234",8);
                        actor->field_3D14[i]=0x12340000+i;
                        memcpy(expected+0x50+8*i,resource,8); expected[0x98+i]=static_cast<BYTE>(0xE0+i);
                        expected[0x164+i]=static_cast<BYTE>(0x80+i); expected[0x16D+i]=static_cast<BYTE>(0xF0+i);
                        memcpy(expected+0xAE+8*i,"InNaTe12",8); memcpy(expected+0xF6+8*i,"SoNg1234",8);
                        *reinterpret_cast<INT*>(expected+0x13E+4*i)=0x12340000+i;
                        if (i<8) {
                            actor->m_quickWeapons[i].m_abilityId.m_itemNum=static_cast<SHORT>(-300+i);
                            actor->m_quickWeapons[i].m_abilityId.m_abilityNum=static_cast<SHORT>(0x7FF0+i);
                            actor->field_3D3A[i]=static_cast<BYTE>(0xA0+i);
                            *reinterpret_cast<SHORT*>(expected+0x30+2*i)=static_cast<SHORT>(-300+i);
                            *reinterpret_cast<SHORT*>(expected+0x40+2*i)=static_cast<SHORT>(0x7FF0+i);
                            expected[0x1A4+i]=static_cast<BYTE>(0xA0+i);
                        }
                        if (i<3) {
                            actor->m_quickItems[i].m_abilityId.m_itemNum=static_cast<SHORT>(-20+i);
                            actor->m_quickItems[i].m_abilityId.m_abilityNum=static_cast<SHORT>(0xFFF0+i);
                            *reinterpret_cast<SHORT*>(expected+0xA2+2*i)=static_cast<SHORT>(-20+i);
                            *reinterpret_cast<SHORT*>(expected+0xA8+2*i)=static_cast<SHORT>(0xFFF0+i);
                        }
                    }
                    actor->m_nLastSpellbookClassIndex=0xFA; actor->m_nLastSpellbookSpellLevel=0xFB; actor->m_nLastSong=0xFC;
                    expected[0x162]=0xFA; expected[0x163]=0xFB; expected[0x178]=0xFC;
                    memcpy(expected+0x17C,"VOICE",5); memcpy(expected+0x184,actor->field_725A,32);
                    BYTE* creature=NULL; LONG size=0; WORD facing=0;
                    actor->Marshal(&creature,&size,&facing,FALSE,FALSE);
                    *reinterpret_cast<DWORD*>(expected+0x28)=0x224; *reinterpret_cast<DWORD*>(expected+0x2C)=size;
                    if (!pass) {
                        screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(13));
                        screen.m_pCharacters=new CStringList;
                        static_cast<CUIControlEdit*>(screen.GetTopPopup()->GetControl(6))->SetText("zrChReX");
                        screen.OnDoneButtonClick();
                        if (screen.m_nErrorState==6 && screen.GetTopPopup() && screen.GetTopPopup()->m_nID==10) screen.OnErrorButtonClick(0);
                        check("done-export-writes-and-retires-list",!screen.m_pCharacters && screen.m_lPopupStack.IsEmpty());
                    } else {
                        check("character-export-existing-resource-keys",dimm.m_cKeyTable.FindKey(ref,1018,1)!=NULL
                            && dimm.m_cKeyTable.FindKey(ref,1022,1)!=NULL);
                        game->CharacterExport(id,"zrChReX");
                    }
                    auto read=[&](const char* extension,std::vector<BYTE>& bytes)->BOOL {
                        CFile file; if (!file.Open(directory+"ZRCHREX."+extension,CFile::modeRead|CFile::typeBinary,NULL)) return FALSE;
                        bytes.resize(static_cast<size_t>(file.GetLength()));
                        UINT count=bytes.empty()?0:file.Read(bytes.data(),static_cast<UINT>(bytes.size())); file.Close();
                        return count==bytes.size();
                    };
                    std::vector<BYTE> chr,bio;
                    BOOL readChr=read("CHR",chr),readBio=read("RES",bio);
                    check("character-export-complete-golden-header",readChr && chr.size()==0x224+size
                        && memcmp(chr.data(),expected,sizeof(expected))==0);
                    check("character-export-complete-creature-payload",readChr && chr.size()==0x224+size
                        && memcmp(chr.data()+0x224,creature,size)==0 && memcmp(chr.data()+0x224,"CRE V2.2",8)==0);
                    check("character-export-biography-terminator-and-empty-truncation",readBio
                        && bio.size()==(pass?0:text.szText.GetLength()+1)
                        && (pass || memcmp(bio.data(),text.szText.GetString(),bio.size())==0));
                    check("character-export-temporary-key-balanced",!dimm.m_bTemporaryKeyAdd);
                    delete[] creature;
                    if (!pass) check("character-export-rescan",dimm.AddToDirectoryList(directory,TRUE));
                }
                game->CharacterExport(CGameObjectArray::INVALID_INDEX,"ZRMISS");
                check("character-export-missing-actor-no-file",GetFileAttributes(directory+"ZRMISS.CHR")==INVALID_FILE_ATTRIBUTES
                    && GetFileAttributes(directory+"ZRMISS.RES")==INVALID_FILE_ATTRIBUTES);
                {
                    CFile file; std::vector<BYTE> data;
                    BOOL loaded=file.Open(directory+"ZRCHREX.CHR",CFile::modeRead|CFile::typeBinary,NULL);
                    if (loaded) {
                        data.resize(static_cast<size_t>(file.GetLength()));
                        loaded=file.Read(data.data(),static_cast<UINT>(data.size()))==data.size(); file.Close();
                    }
                    check("character-import-fixture-export-loaded",loaded && data.size()>0x224);
                    if (loaded && data.size()>0x224) {
                        auto write=[&](const CString& name,const void* bytes,UINT size)->BOOL {
                            CFile output;
                            if (!output.Open(directory+name,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary,NULL)) return FALSE;
                            if (size) output.Write(bytes,size); output.Close(); return TRUE;
                        };
                        auto* header=reinterpret_cast<CCharacterFileHeader*>(data.data());
                        memset(header->m_quickSpellsSpellId,0,sizeof(header->m_quickSpellsSpellId));
                        memset(header->m_quickInnatesSpellId,0,sizeof(header->m_quickInnatesSpellId));
                        memset(header->m_quickSongsSpellId,0,sizeof(header->m_quickSongsSpellId));
                        memset(header->m_quickWeaponsItemNum,0xFF,sizeof(header->m_quickWeaponsItemNum));
                        memset(header->m_quickWeaponsAbilityNum,0xFF,sizeof(header->m_quickWeaponsAbilityNum));
                        memset(header->m_quickItemsItemNum,0xFF,sizeof(header->m_quickItemsItemNum));
                        memset(header->m_quickItemsAbilityNum,0xFF,sizeof(header->m_quickItemsAbilityNum));
                        memset(header->m_name,' ',32); memcpy(header->m_name,"  Import name",13);
                        memcpy(header->m_secondarySounds,"vOiCe123",8);
                        SHORT weapon=-1;
                        for (SHORT slot=CGameSpriteEquipment::SLOT_WEAPON;slot<CGameSpriteEquipment::NUM_SLOT && weapon==-1;++slot) {
                            CItem* item=actor->m_equipment.m_items[slot];
                            if (!item || item->GetItemType()==0x3A) continue;
                            item->Demand(); ITEM_ABILITY* ability=item->GetAbility(0);
                            if (ability && ability->type!=4 && ability->type!=2) weapon=slot;
                            item->Release();
                        }
                        check("character-import-fixture-has-melee-shortcut",weapon!=-1);
                        header->m_quickWeaponsItemNum[0]=weapon; header->m_quickWeaponsAbilityNum[0]=0;
                        auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
                        BeginLevelUpTableFixture gold(rules.m_tStartingGold,5,11);
                        gold.x[0]=CRuleTables::SIDES; gold.x[1]=CRuleTables::ROLLS; gold.x[2]=CRuleTables::MODIFIER;
                        gold.x[3]=CRuleTables::MULTIPLIER; gold.x[4]=CRuleTables::BONUS_PER_LEVEL;
                        for (INT cl=0;cl<11;++cl) {
                            gold.y[cl]=rules.GetClassString(cl+1,0);
                            gold.cells[5*cl]="1"; gold.cells[5*cl+1]="1"; gold.cells[5*cl+2]="6";
                            gold.cells[5*cl+3]="10"; gold.cells[5*cl+4]="0";
                        }
                        auto oldGold=game->m_gameSave.m_nPartyGold; auto connected=network.m_bConnectionEstablished;
                        network.m_bConnectionEstablished=FALSE;
                        const STRREF strref=0xF242; STR_RES oldBiography,importedBiography;
                        BOOL hadBiography=overrides.Fetch(strref,oldBiography);
                        auto& directories=dimm.m_lDirectories; INT beforeDirectories=directories.GetCount();
                        const char biography[]="Imported biography\r\nSecond line\0ignored tail";
                        check("character-import-fixture-biography-written",write("ZRCHREX.RES",biography,sizeof(biography)));
                        {
                            CResHelper<CResBIO,1022> resource; resource.SetResRef(ref,TRUE,TRUE);
                            BOOL correct=resource.pRes!=NULL && dynamic_cast<CResBIO*>(static_cast<CRes*>(resource.pRes))==resource.pRes;
                            check("biography-resource-native-factory",correct);
                            if (correct) {
                                auto* raw=resource.pRes->Demand();
                                check("biography-resource-first-demand",raw && resource.pRes->m_bParsed && resource.pRes->GetDemands()==1);
                                check("biography-resource-nested-demand",resource.pRes->Demand()==raw && resource.pRes->GetDemands()==2);
                                resource.pRes->Release(); check("biography-resource-retains-parse-while-demanded",resource.pRes->m_bParsed);
                                resource.pRes->Release(); check("biography-resource-last-release",!resource.pRes->m_bParsed && resource.pRes->GetDemands()==0);
                            }
                        }
                        for (INT pass=0;pass<2;++pass) {
                            CString name=pass?"ZRCHRNOB":"ZRCHREX";
                            memset(header->m_soundSet,0,32);
                            memcpy(header->m_soundSet,pass?"_IMPORT_LONG_VOICE_MISSING_12345":"_IMPORT_MISSING",pass?31:15);
                            CString sounds(reinterpret_cast<const char*>(header->m_soundSet));
                            DWORD capacity=sounds.GetAllocLength(); BYTE expectedSound[32]={};
                            memcpy(expectedSound,sounds.GetBuffer(capacity),min(capacity,32UL)); if (capacity<32) expectedSound[capacity]=0;
                            game->m_gameSave.m_nPartyGold=1000;
                            BOOL written=write(name+".CHR",data.data(),static_cast<UINT>(data.size()));
                            check("character-import-fixture-chr-written",written);
                            if (!written) continue;
                            INT beforeMessages=handler->m_messageList.GetCount();
                            LONG imported=game->ImportCharacter(name,0x102); CGameObject* object=NULL;
                            BOOL found=imported!=0 && objects->GetDeny(imported,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS;
                            check("character-import-registers-disposable-actor",found);
                            if (found) {
                                auto* sprite=static_cast<CGameSprite*>(object);
                                check("character-import-name-keeps-all-32-bytes",sprite->m_sName==CString(header->m_name,32));
                                check("character-import-custom-shortcuts-and-bookmark",memcmp(sprite->field_3D14,header->field_13E,sizeof(sprite->field_3D14))==0
                                    && sprite->m_nLastSpellbookClassIndex==header->m_nLastSpellbookClassIndex
                                    && sprite->m_nLastSpellbookSpellLevel==header->m_nLastSpellbookSpellLevel);
                                check("character-import-rebinds-weapon-shortcut",weapon==-1 || (sprite->m_quickWeapons[0].m_abilityId.m_itemNum==weapon
                                    && sprite->m_quickWeapons[0].m_abilityId.m_abilityNum==0 && sprite->m_quickWeapons[0].m_abilityId.m_itemType==2));
                                check("character-import-sound-identity-and-capacity-copy",sprite->m_secondarySounds==CResRef(header->m_secondarySounds)
                                    && memcmp(sprite->field_725A,expectedSound,32)==0);
                                check("character-import-global-unplaced-and-starting-gold",sprite->m_bGlobal && sprite->m_pArea==NULL
                                    && sprite->m_resRef==CResRef(name) && game->m_gameSave.m_nPartyGold==1070);
                                check("character-import-biography-slot-byte-and-missing",sprite->m_baseStats.m_biography==(pass?-1:strref));
                                if (!pass) check("character-import-biography-nul-terminated",overrides.Fetch(strref,importedBiography)
                                    && importedBiography.szText==CString(biography));
                                objects->ReleaseDeny(imported,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                                check("character-import-disposable-actor-retired",objects->Delete(imported,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE)==CGameObjectArray::SUCCESS);
                                delete sprite;
                            }
                            while (handler->m_messageList.GetCount()>beforeMessages) delete handler->m_messageList.RemoveTail();
                            check("character-import-temporary-key-balanced",!dimm.m_bTemporaryKeyAdd);
                        }
                        check("character-import-missing-file-returns-zero",game->ImportCharacter("ZRMISS",0)==0 && !dimm.m_bTemporaryKeyAdd);
                        const char bad[]="NOPE0000";
                        if (write("ZRBADCHR.CHR",bad,8)) check("character-import-bad-signature-returns-zero",game->ImportCharacter("ZRBADCHR",0)==0 && !dimm.m_bTemporaryKeyAdd);
                        {
                            CScreenCreateChar preview;
                            preview.m_cUIManager.fInit(&preview,CResRef("GUICG"),g_pBaldurChitin->m_bUseNewGui);
                            for (INT side=0;side<4;++side) preview.m_cUIManager.AddPanel(&g_pBaldurChitin->m_aBorderPanels[side]);
                            auto* display=static_cast<CUIControlTextDisplay*>(preview.m_cUIManager.GetPanel(20)->GetControl(4));
                            auto* portrait=static_cast<CUIControlButtonCharGenPortrait*>(preview.m_cUIManager.GetPanel(0)->GetControl(12));
                            auto& tlk=g_pBaldurChitin->GetTlkTable(); CString oldClass,oldLevel;
                            BOOL hadClass=tlk.m_mapTokens.Lookup(CRuleTables::TOKEN_CLASS,oldClass);
                            BOOL hadLevel=tlk.m_mapTokens.Lookup(CRuleTables::TOKEN_LEVEL,oldLevel);
                            auto rows=[&]() {
                                std::vector<CString> result; POSITION pos=display->m_plstStrings->GetHeadPosition();
                                while (pos) {
                                    auto* line=display->m_plstStrings->GetNext(pos);
                                    if (line->m_nLine==0) result.push_back(line->m_sString);
                                    else if (!result.empty()) result.back()+=" "+line->m_sString;
                                }
                                for (auto& row:result) row.Trim();
                                return result;
                            };
                            auto* creature=reinterpret_cast<CCreatureFileHeader*>(data.data()+header->m_creatureOffset+8);
                            creature->m_STRBase=3; creature->m_DEXBase=12; creature->m_CONBase=19;
                            creature->m_INTBase=8; creature->m_WISBase=17; creature->m_CHRBase=20;
                            memset(header->m_name,0,32); memcpy(header->m_name,"Preview character",17);
                            for (INT pass=0;pass<3;++pass) {
                                CString name=pass==1?"ZRCHRNOB":"ZRCHREX";
                                creature->m_name=pass==1?1145:-1;
                                check("character-preview-fixture-written",write(name+".CHR",data.data(),static_cast<UINT>(data.size())));
                                if (pass<2) portrait->m_portraitResRef=CResRef("OLD");
                                SHORT first=objects->m_nNextObjectId; INT before=handler->m_messageList.GetCount();
                                preview.UpdateCharacterSummary(name,display);
                                LONG previewId=objects->FindObjectId(first); CGameObject* object=NULL;
                                BOOL found=objects->GetDeny(previewId,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS;
                                check("character-preview-native-registration",found && objects->m_nNextObjectId==first+1);
                                if (found) {
                                    auto* sprite=static_cast<CGameSprite*>(object); std::vector<CString> expected;
                                    auto fetch=[&](STRREF strref) { return CBaldurEngine::FetchString(strref); };
                                    expected.push_back(fetch(1047)); expected.push_back(pass==1?fetch(1145):CString(_T("Preview character")));
                                    expected.push_back(""); expected.push_back(fetch(1048)); CString value;
                                    rules.GetRaceStringMixed(sprite->m_startTypeAI.m_nRace,value,sprite->m_startTypeAI.m_nSubRace);
                                    expected.push_back(value); expected.push_back(""); expected.push_back(fetch(12136));
                                    CString actualClass,actualLevel;
                                    tlk.m_mapTokens.Lookup(CRuleTables::TOKEN_CLASS,actualClass); tlk.m_mapTokens.Lookup(CRuleTables::TOKEN_LEVEL,actualLevel);
                                    INT classes=0;
                                    for (BYTE cl=1;cl<=CAIOBJECT_CLASS_MAX;++cl) {
                                        INT level=sprite->GetClassLevel(cl); if (level<=0) continue;
                                        ++classes; rules.GetClassStringMixed(cl,sprite->GetSpecialization(),sprite->m_baseStats.m_flags,
                                            value,sprite->m_startTypeAI.m_nGender!=CAIOBJECTTYPE_SEX_FEMALE);
                                        tlk.SetToken(CRuleTables::TOKEN_CLASS,value); CString number; number.Format(_T("%d"),level);
                                        tlk.SetToken(CRuleTables::TOKEN_LEVEL,number); expected.push_back(fetch(16480));
                                    }
                                    CString expectedClass,expectedLevel;
                                    tlk.m_mapTokens.Lookup(CRuleTables::TOKEN_CLASS,expectedClass); tlk.m_mapTokens.Lookup(CRuleTables::TOKEN_LEVEL,expectedLevel);
                                    check("character-preview-class-tokens",classes>0 && actualClass==expectedClass && actualLevel==expectedLevel);
                                    expected.push_back(""); expected.push_back(fetch(1049));
                                    rules.GetAlignmentStringMixed(sprite->m_startTypeAI.m_nAlignment,value); expected.push_back(value);
                                    expected.push_back(""); expected.push_back(fetch(17088));
                                    const STRREF labels[]={1145,1151,1178,1179,1180,1181};
                                    const INT scores[]={3,12,19,8,17,20},modifiers[]={-4,1,4,-1,3,5};
                                    for (INT i=0;i<6;++i) {
                                        value.Format(_T("%s: %d  (%+d)"),fetch(labels[i]).GetString(),scores[i],modifiers[i]); expected.push_back(value);
                                    }
                                    expected.push_back(""); for (auto& row:expected) row.Trim();
                                    check("character-preview-complete-summary-order-and-values",rows()==expected);
                                    INT row=0,headings=0; BOOL colors=TRUE; POSITION pos=display->m_plstStrings->GetHeadPosition();
                                    while (pos) {
                                        auto* line=display->m_plstStrings->GetNext(pos); if (line->m_nLine) continue;
                                        BOOL heading=row==0 || row==3 || row==6 || row==8+classes || row==11+classes;
                                        if (heading) ++headings;
                                        colors=colors && line->m_rgbTextColor==(heading?RGB(200,200,0):display->m_rgbTextColor); ++row;
                                    }
                                    check("character-preview-five-colored-headings",colors && headings==5);
                                    check("character-preview-large-portrait",portrait->m_portraitResRef==CResRef(sprite->m_baseStats.m_portraitLarge));
                                    check("character-preview-no-import-gold-or-placement",game->m_gameSave.m_nPartyGold==1070 && !sprite->m_bGlobal && !sprite->m_pArea);
                                    objects->ReleaseDeny(previewId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                                    check("character-preview-disposable-actor-retired",objects->Delete(previewId,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE)==CGameObjectArray::SUCCESS);
                                    delete sprite;
                                }
                                while (handler->m_messageList.GetCount()>before) delete handler->m_messageList.RemoveTail();
                                check("character-preview-temporary-key-balanced",!dimm.m_bTemporaryKeyAdd);
                            }
                            CResRef previousPortrait=portrait->m_portraitResRef;
                            for (CString name:{CString(_T("ZRMISS")),CString(_T("ZRBADCHR"))}) {
                                SHORT next=objects->m_nNextObjectId; preview.UpdateCharacterSummary(name,display);
                                auto result=rows(); CString error=CBaldurEngine::FetchString(39695); error.Trim();
                                check("character-preview-failure-replaces-old-summary",result.size()==1 && result[0]==error);
                                check("character-preview-failure-preserves-portrait-and-register",portrait->m_portraitResRef==previousPortrait
                                    && objects->m_nNextObjectId==next && !dimm.m_bTemporaryKeyAdd);
                            }
                            {
                                INT before=handler->m_messageList.GetCount();
                                LONG initial=game->ImportCharacter("ZRCHRNOB",0x102);
                                check("chargen-import-done-disposable-source-created",initial!=0);
                                if (initial!=0) {
                                    auto provider=network.m_nServiceProvider; network.m_nServiceProvider=CNetwork::SERV_PROV_NULL;
                                    auto* oldCreate=g_pBaldurChitin->m_pEngineCreateChar; g_pBaldurChitin->m_pEngineCreateChar=&preview;
                                    preview.m_nGameSprite=initial; preview.m_nCharacterSlot=2; preview.m_nCurrentStep=6;
                                    preview.m_pCharacters=new CStringList; preview.m_pCharacters->AddTail("ZRCHRNOB"); preview.m_nCharacterIndex=0;
                                    preview.m_lPopupStack.AddTail(preview.m_cUIManager.GetPanel(20)); preview.OnDoneButtonClick();
                                    LONG imported=preview.m_nGameSprite; CGameObject* object=NULL;
                                    check("chargen-import-done-replaces-source-and-retires-list",imported!=0 && imported!=initial
                                        && !preview.m_pCharacters && preview.m_lPopupStack.IsEmpty());
                                    if (imported!=0) {
                                        auto& entry=objects->m_pArray[(imported>>16)&0x7FFF];
                                        check("chargen-import-done-releases-replacement-deny",entry.m_objectId==static_cast<SHORT>(imported)
                                            && entry.m_denyCounts[CGameObjectArray::THREAD_ASYNCH]==0);
                                        BOOL found=objects->GetDeny(imported,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS;
                                        check("chargen-import-done-replacement-registered",found);
                                        if (found) {
                                            objects->ReleaseDeny(imported,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                                            check("chargen-import-done-replacement-retired",objects->Delete(imported,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE)==CGameObjectArray::SUCCESS);
                                            delete static_cast<CGameSprite*>(object);
                                        }
                                    }
                                    preview.m_nGameSprite=CGameObjectArray::INVALID_INDEX;
                                    g_pBaldurChitin->m_pEngineCreateChar=oldCreate; network.m_nServiceProvider=provider;
                                }
                                while (handler->m_messageList.GetCount()>before) delete handler->m_messageList.RemoveTail();
                                check("chargen-import-done-temporary-key-balanced",!dimm.m_bTemporaryKeyAdd);
                            }
                            {
                                auto* oldCreate=g_pBaldurChitin->m_pEngineCreateChar; g_pBaldurChitin->m_pEngineCreateChar=&preview;
                                preview.m_nGameSprite=id; STRREF biography=actor->m_baseStats.m_biography; actor->m_baseStats.m_biography=1145;
                                CString names[]={CString(_T("_IWD2_RE_MISSING_VOICE_")),CString('X',40),CString('X',31)};
                                char* shortName=names[2].GetBuffer(31); memcpy(shortName,"ZRVOICE",7); names[2].ReleaseBuffer(7);
                                check("chargen-sound-fixture-retains-capacity",names[2].GetAllocLength()>names[2].GetLength() && names[2].GetAllocLength()<32);
                                for (const auto& name:names) {
                                    check("chargen-sound-fixture-directory-absent",GetFileAttributes(game->GetDirSounds()+name)==INVALID_FILE_ATTRIBUTES);
                                    INT count=directories.GetCount(); DWORD capacity=name.GetAllocLength();
                                    memset(actor->field_725A,0xA5,32); actor->m_secondarySounds=CResRef("OLD");
                                    preview.m_pSounds=new CStringList; preview.m_pSounds->AddTail(name); preview.m_nCustomSoundSetIndex=0;
                                    preview.m_nCurrentStep=7; preview.m_lPopupStack.AddTail(preview.m_cUIManager.GetPanel(19));
                                    preview.OnDoneButtonClick();
                                    BOOL copied=memcmp(actor->field_725A,name.GetString(),min(name.GetLength(),32))==0;
                                    if (capacity<32) {
                                        copied=copied && actor->field_725A[capacity]==0;
                                        for (UINT i=capacity+1;i<32;++i) copied=copied && static_cast<BYTE>(actor->field_725A[i])==0xA5;
                                    }
                                    check("chargen-sound-done-native-capacity-copy-and-list-retirement",copied && !preview.m_pSounds
                                        && preview.m_lPopupStack.IsEmpty() && preview.m_nCurrentStep==8 && actor->m_secondarySounds==CResRef("OLD"));
                                    while (directories.GetCount()>count) directories.RemoveTail();
                                }
                                actor->m_baseStats.m_biography=biography; preview.m_nGameSprite=CGameObjectArray::INVALID_INDEX;
                                g_pBaldurChitin->m_pEngineCreateChar=oldCreate;
                            }
                            if (hadClass) tlk.SetToken(CRuleTables::TOKEN_CLASS,oldClass); else tlk.m_mapTokens.RemoveKey(CRuleTables::TOKEN_CLASS);
                            if (hadLevel) tlk.SetToken(CRuleTables::TOKEN_LEVEL,oldLevel); else tlk.m_mapTokens.RemoveKey(CRuleTables::TOKEN_LEVEL);
                        }
                        game->m_gameSave.m_nPartyGold=oldGold; network.m_bConnectionEstablished=connected;
                        overrides.Remove(strref); if (hadBiography) overrides.AddUserEntry(strref,oldBiography);
                        while (directories.GetCount()>beforeDirectories) directories.RemoveTail();
                    }
                }
                dimm.RemoveFromDirectoryList(directory,TRUE);
                game->m_sCharactersDir=previousDirectory; actor->m_sName=oldName;
                memcpy(actor->m_quickWeapons,weapons,sizeof(weapons)); memcpy(actor->m_quickSpells,spells,sizeof(spells));
                memcpy(actor->m_quickItems,items,sizeof(items)); memcpy(actor->m_quickInnates,innates,sizeof(innates));
                memcpy(actor->m_quickSongs,songs,sizeof(songs)); memcpy(actor->field_3D14,customButtons,sizeof(customButtons));
                memcpy(actor->field_3D3A,weaponButtons,sizeof(weaponButtons));
                actor->m_nLastSpellbookClassIndex=lastClass; actor->m_nLastSpellbookSpellLevel=lastLevel; actor->m_nLastSong=lastSong;
                actor->m_baseStats=base; actor->m_secondarySounds=secondary; memcpy(actor->field_725A,sound,32);
                overrides.Remove(biographyId); if (hadText) overrides.AddUserEntry(biographyId,previousText);
            }
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
