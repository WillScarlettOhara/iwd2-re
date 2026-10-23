// AutoUI-only replay: real GUIREC controls, borrowed rules and deferred effects.
#include "AllocationGolden.h"

void AllocationChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    CGameObject* object=NULL; LONG id=game->GetCharacterId(0);
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"allocationchecks","requires party actor"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"allocation-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto base=actor->m_baseStats; CDerivedStats derived,temp; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto start=actor->m_startTypeAI; auto ai=actor->m_typeAI; auto allow=actor->m_bAllowEffectListCall;
    auto dirty=actor->field_562C; auto deferred=actor->field_72A2; auto bonus=actor->m_nHPCONBonusTotalOld;
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto* timer=game->GetWorldTimer(); auto clock=timer->m_gameTime;
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables()); auto& tlk=g_pBaldurChitin->GetTlkTable();
    CString oldToken; BOOL hadToken=tlk.m_mapTokens.Lookup(CScreenCharacter::TOKEN_NUMBER,oldToken);
    const CString* extraTokens[]={&CRuleTables::TOKEN_FIGHTERTYPE,&CRuleTables::TOKEN_MAGESCHOOL,&CScreenCharacter::TOKEN_WEAPONNAME};
    CString savedTokens[3]; BOOL hadExtraTokens[3];
    for (INT i=0;i<3;++i) hadExtraTokens[i]=tlk.m_mapTokens.Lookup(*extraTokens[i],savedTokens[i]);
    auto* oldCharacter=g_pBaldurChitin->m_pEngineCharacter;
    {
        CScreenCharacter screen; screen.EngineGameInit();
        g_pBaldurChitin->m_pEngineCharacter=&screen;
        BeginLevelUpTableFixture skills(rules.m_tSkills,1,16),feats(rules.m_tFeats,1,75),costs(rules.m_tSkillCosts,11,16);
        for (const auto& row : ALLOCATION_CASES) {
            ++index; actor->m_baseStats=base;
            memset(actor->m_derivedStats.m_nClassLevels,0,11);
            actor->m_derivedStats.m_fighterLevel=4; actor->m_derivedStats.m_nLevel=4;
            actor->m_derivedStats.m_classMask=CLASSMASK_FIGHTER;
            for (INT i=0;i<16;++i) {
                const BYTE values[]={0,1,127,255};
                actor->m_baseStats.m_skills[i]=row.profile==3?values[i%4]:i%3;
                skills.cells[i].Format(_T("%d"),row.mapping?15-i:i);
                for (INT cls=0;cls<11;++cls) {
                    INT cost=cls!=4?0:row.profile==0?0:row.profile==1?1:row.profile==2?(i<10?0:2):1+i%2;
                    costs.cells[11*i+cls].Format(_T("%d"),cost);
                }
            }
            for (INT i=0;i<75;++i) feats.cells[i].Format(_T("%d"),row.mapping?74-i:i);
            for (auto& bits : actor->m_baseStats.m_feats) bits=0xffffffff;
            auto& b=actor->m_baseStats;
            BYTE* ranks[]={&b.m_featBow,&b.m_featCrossbow,&b.m_featMissile,&b.m_featAxe,&b.m_featMace,&b.m_featFlail,
                &b.m_featPolearm,&b.m_featHammer,&b.m_featQuarterStaff,&b.m_featGreatSword,&b.m_featLargeSword,
                &b.m_featSmallBlade,&b.m_featToughness,&b.m_featArmoredArcana,&b.m_featCleave,&b.m_featArmorProficiency,
                &b.m_featFocusEnchantment,&b.m_featFocusEvocation,&b.m_featFocusNecromancy,&b.m_featFocusTransmutation,
                &b.m_featSpellPenetration,&b.m_featExtraRage,&b.m_featExtraShapeshifting,&b.m_featExtraSmiting,
                &b.m_featExtraTurning,&b.m_featBastardSword};
            for (auto* rank : ranks) *rank=255;
            auto clear=[&](INT feat) { b.m_feats[feat>>5]&=~(1u<<(feat&31)); };
            if (row.profile==1) clear(6);
            if (row.profile==2) clear(34);
            if (row.profile==3) { clear(6); clear(12); clear(34); clear(69); b.m_featToughness=0; }
            actor->m_baseStats.m_hitPoints=-32768; actor->m_baseStats.m_maxHitPointsBase=65535;
            actor->m_derivedStats.m_nMaxHitPoints=32767; actor->m_nHPCONBonusTotalOld=-77;
            actor->m_bAllowEffectListCall=FALSE; actor->field_562C=FALSE; actor->field_72A2=0;
            screen.m_nClass=CAIOBJECTTYPE_C_FIGHTER;
            screen.field_2BC=screen.field_64C=row.budget;
            screen.m_nExtraSkillPoints=screen.m_nExtraFeats=row.budget-1;
            screen.m_nTopSkill=screen.m_nTopFeat=12;
            for (INT i=0;i<16;++i) screen.m_storedSkills[i]=900+i;
            for (INT i=0;i<75;++i) screen.m_storedFeats[i]=900+i;
            auto* panel=screen.m_cUIManager.GetPanel(row.kind?56:55);
            if (row.kind) screen.ResetFeatsPanel(panel,actor,row.flag);
            else screen.ResetSkillsPanel(panel,actor,row.flag);
            INT top=row.kind?screen.m_nTopFeat:screen.m_nTopSkill;
            INT remaining=row.kind?screen.m_nExtraFeats:screen.m_nExtraSkillPoints;
            INT* stored=row.kind?screen.m_storedFeats:screen.m_storedSkills;
            INT count=row.kind?75:16; DWORD enabled=0,active=0;
            for (INT i=0;i<18;++i) {
                auto* button=static_cast<CUIControlButton*>(panel->GetControl(14+i));
                if (button->m_bEnabled) enabled|=1u<<i;
                if (button->m_bActive) active|=1u<<i;
            }
            CString token; check("number-token-present",tlk.m_mapTokens.Lookup(CScreenCharacter::TOKEN_NUMBER,token));
            CString expected; expected.Format(_T("%d"),remaining);
            check("canonical-panel-state",top==row.top && remaining==row.remaining && enabled==row.enabled && active==row.active);
            check("canonical-snapshot",memcmp(stored,row.stored,count*sizeof(INT))==0);
            check("signed-budget-token",token==expected);
            check("hp-preserved",actor->m_baseStats.m_hitPoints==-32768 && actor->m_baseStats.m_maxHitPointsBase==65535
                && actor->m_derivedStats.m_nMaxHitPoints==32767 && actor->m_nHPCONBonusTotalOld==-77);
            check("effect-request-only-on-init",actor->field_72A2==(row.flag?1:0) && actor->field_562C==(row.flag?1:0));
            auto* text=static_cast<CUIControlTextDisplay*>(panel->GetControl(92));
            check("real-help-display-populated",text && !text->m_plstStrings->IsEmpty());
            CString snapshot;
            for (INT i=0;i<count;++i) { CString value; value.Format(_T("%s%d"),i?",":"",stored[i]); snapshot+=value; }
            Emit("{\"op\":\"allocation-result\",\"case\":%d,\"kind\":%d,\"top\":%d,\"budget\":%d,\"stored\":[%s],\"enabled\":%u,\"active\":%u,\"effects\":%d,\"dirty\":%d,\"hp\":[%d,%u,%d,%d],\"token\":\"%s\"}",
                index,row.kind,top,remaining,static_cast<LPCTSTR>(snapshot),enabled,active,actor->field_72A2,actor->field_562C,
                actor->m_baseStats.m_hitPoints,actor->m_baseStats.m_maxHitPointsBase,actor->m_derivedStats.m_nMaxHitPoints,
                actor->m_nHPCONBonusTotalOld,static_cast<LPCTSTR>(token));
        }
        // Exercise the popup dispatcher through the actual allocation controls.
        for (INT i=0;i<16;++i) { skills.cells[i].Format(_T("%d"),15-i); actor->m_baseStats.m_skills[i]=1; screen.m_storedSkills[i]=1;
            for (INT cls=0;cls<11;++cls) costs.cells[11*i+cls]=cls==4?(i<8?"1":"2"):"0"; }
        for (INT i=0;i<75;++i) { feats.cells[i].Format(_T("%d"),i); screen.m_storedFeats[i]=actor->GetFeatValue(i); }
        actor->m_baseStats.m_feats[0]&=~(1u<<6);
        screen.m_storedSkills[7]=0;
        actor->m_startTypeAI.m_nClass=CAIOBJECTTYPE_C_FIGHTER;
        for (INT kind : {0,1}) for (INT budget : {0,2}) {
            auto* panel=screen.m_cUIManager.GetPanel(kind?56:55); screen.m_lPopupStack.AddTail(panel);
            screen.m_nTopSkill=-1; screen.m_nTopFeat=0;
            screen.m_nExtraSkillPoints=screen.m_nExtraFeats=budget;
            screen.UpdatePopupPanel(panel->m_nID,actor);
            auto* done=static_cast<CUIControlButton*>(panel->GetControl(0));
            check("allocation-refresh-done",done->m_bEnabled==(kind?budget==0:budget<=1));
            if (kind) {
                check("feat-refresh-budget-gates",static_cast<CUIControlButton*>(panel->GetControl(26))->m_bEnabled==(budget>0));
                check("feat-refresh-no-baseline-removal",!static_cast<CUIControlButton*>(panel->GetControl(27))->m_bEnabled);
            } else {
                check("skill-refresh-unsigned-top-clamped",screen.m_nTopSkill==7);
                check("skill-refresh-baseline-removal",static_cast<CUIControlButton*>(panel->GetControl(15))->m_bEnabled);
                check("skill-refresh-cost-gates",static_cast<CUIControlButton*>(panel->GetControl(14))->m_bEnabled==(budget>=2));
                check("skill-refresh-double-mapped-label",static_cast<CUIControlLabel*>(panel->GetControl(0x10000001))->GetText().Find(_T("(1)"))>=0);
                check("skill-refresh-displayed-rank",static_cast<CUIControlLabel*>(panel->GetControl(0x10000069))->GetText()==_T("1"));
            }
            check("refresh-preserves-hp",actor->m_baseStats.m_hitPoints==-32768 && actor->m_baseStats.m_maxHitPointsBase==65535
                && actor->m_derivedStats.m_nMaxHitPoints==32767 && actor->m_nHPCONBonusTotalOld==-77);
            screen.m_lPopupStack.RemoveAll();
        }
        for (DWORD id : {4u,57u,12u,51u,52u,53u}) {
            auto* panel=screen.m_cUIManager.GetPanel(id); screen.m_lPopupStack.AddTail(panel);
            screen.m_nInfoCategory=screen.m_nInfoSelectedItem=screen.m_nInfoScrollPos=0;
            screen.UpdatePopupPanel(id,actor);
            if (id==4) { CString count; count.Format(_T("%d"),actor->m_cGameStats.m_nGameKillsNumber);
                check("statistics-panel-live-count",static_cast<CUIControlLabel*>(panel->GetControl(0x10000016))->GetText()==count); }
            else if (id==57) check("information-panel-category",static_cast<CUIControlLabel*>(panel->GetControl(0x10000004))->GetText()==screen.FetchString(17088));
            else check("description-panel-scroll-route",screen.m_pCurrentScrollBar==panel->GetControl(id==51?3:1));
            screen.m_lPopupStack.RemoveAll();
        }
        // Prerequisites are also used by HasFeat during play, not just by the selection UI.
        {
            auto savedBase=actor->m_baseStats; CDerivedStats savedDerived; savedDerived=actor->m_derivedStats;
            auto reset=[&]() { actor->m_baseStats=CCreatureFileHeader(); actor->m_typeAI=ai;
                actor->m_baseStats.m_STRBase=actor->m_baseStats.m_DEXBase=actor->m_baseStats.m_WISBase=18;
                memset(actor->m_derivedStats.m_nClassLevels,0,11); actor->m_derivedStats.m_classMask=0;
                actor->m_derivedStats.m_nSTR=actor->m_derivedStats.m_nDEX=actor->m_derivedStats.m_nWIS=18; };
            reset(); actor->m_baseStats.m_STRBase=12; actor->m_derivedStats.m_nSTR=13;
            check("power-attack-base-versus-derived-exact-selector",!actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_POWER_ATTACK,1)
                && actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_POWER_ATTACK,0) && actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_POWER_ATTACK,2));
            check("power-attack-selection-uses-base-strength",!actor->CanSelectFeat(CGAMESPRITE_FEAT_POWER_ATTACK,1)
                && actor->CanSelectFeat(CGAMESPRITE_FEAT_POWER_ATTACK,0));
            actor->SetFeatValue(CGAMESPRITE_FEAT_POWER_ATTACK,1);
            check("owned-power-attack-uses-derived-strength",actor->HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK)
                && !actor->CanSelectFeat(CGAMESPRITE_FEAT_POWER_ATTACK,0));
            actor->m_derivedStats.m_nSTR=12; actor->m_baseStats.m_STRBase=13;
            check("power-attack-strength-threshold-in-both-directions",!actor->HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK)
                && actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_POWER_ATTACK,1));
            reset(); actor->m_baseStats.m_bardLevel=1;
            check("heroic-inspiration-allows-bard",actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_HEROIC_INSPIRATION,1));
            actor->m_baseStats.m_bardLevel=0;
            check("heroic-inspiration-requires-eligible-class",!actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_HEROIC_INSPIRATION,1));
            actor->SetFeatValue(CGAMESPRITE_FEAT_MARTIAL_LARGESWORD,2); actor->m_baseStats.m_paladinLevel=7;
            check("fiendslayer-rejects-paladin-below-eight",!actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_FIENDSLAYER,1));
            actor->m_baseStats.m_paladinLevel=8;
            check("fiendslayer-allows-paladin-eight",actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_FIENDSLAYER,1));
            actor->SetFeatValue(CGAMESPRITE_FEAT_MARTIAL_LARGESWORD,1);
            check("fiendslayer-still-requires-weapon-rank",!actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_FIENDSLAYER,1));
            for (INT level:{9,10}) { actor->m_baseStats.m_rogueLevel=level;
                check("slippery-mind-rogue-threshold",actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_SLIPPERY_MIND,1)==(level==10)); }
            reset(); actor->m_derivedStats.m_sorcererLevel=8; actor->m_derivedStats.m_classMask=CLASSMASK_SORCERER;
            for (INT feat:{CGAMESPRITE_FEAT_AEGIS_OF_RIME,CGAMESPRITE_FEAT_AQUA_MORTIS,CGAMESPRITE_FEAT_SCION_OF_STORMS,CGAMESPRITE_FEAT_SPIRIT_OF_FLAME}) {
                for (INT skill:{9,10,255}) { actor->m_derivedStats.m_nSkills[13]=skill;
                    check("elemental-feats-allow-sorcerer-eight-with-signed-skill",actor->CheckFeatPrerequisites(feat,1)==(skill==10)); }
            }
            actor->m_derivedStats.m_nSkills[13]=10; actor->m_derivedStats.m_sorcererLevel=0;
            actor->m_derivedStats.m_rangerLevel=8; actor->m_derivedStats.m_classMask=CLASSMASK_RANGER;
            check("ranger-eight-does-not-substitute-for-sorcerer",!actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_AEGIS_OF_RIME,1));
            actor->m_derivedStats.m_rangerLevel=15;
            check("elemental-feats-allow-ranger-fifteen",actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_AEGIS_OF_RIME,1));
            struct RaceCase { BYTE race,subrace; BOOL mercantile,poison,snake; };
            for (const RaceCase& row:{RaceCase{1,0,TRUE,FALSE,TRUE},RaceCase{4,0,FALSE,FALSE,FALSE},
                RaceCase{4,2,TRUE,TRUE,FALSE},RaceCase{6,0,FALSE,FALSE,FALSE},RaceCase{6,1,TRUE,FALSE,FALSE},RaceCase{7,0,FALSE,TRUE,FALSE}}) {
                reset(); actor->m_typeAI.m_nRace=row.race; actor->m_typeAI.m_nSubRace=row.subrace;
                actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER; actor->m_baseStats.m_fighterLevel=1;
                check("mercantile-background-race-and-subrace",actor->CanSelectFeat(CGAMESPRITE_FEAT_MERCANTILE_BACKGROUND,1)==row.mercantile);
                check("resist-poison-race-and-masked-first-level",actor->CanSelectFeat(CGAMESPRITE_FEAT_RESIST_POISON,1)==row.poison);
                check("snake-blood-race-and-masked-first-level",actor->CanSelectFeat(CGAMESPRITE_FEAT_SNAKE_BLOOD,1)==row.snake);
            }
            for (INT feat:{CGAMESPRITE_FEAT_RESIST_POISON,CGAMESPRITE_FEAT_SNAKE_BLOOD}) {
                reset(); actor->m_typeAI.m_nRace=feat==CGAMESPRITE_FEAT_RESIST_POISON?7:1;
                actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER; actor->m_baseStats.m_fighterLevel=1; actor->m_baseStats.m_wizardLevel=9;
                check("background-feat-ignores-unmasked-levels",actor->CheckFeatPrerequisites(feat,1));
                actor->m_typeAI.m_nClassMask|=CLASSMASK_WIZARD;
                check("background-feat-counts-all-masked-levels",!actor->CheckFeatPrerequisites(feat,1));
                actor->m_typeAI.m_nClassMask=0;
                check("background-feat-rejects-empty-mask",!actor->CheckFeatPrerequisites(feat,1));
                actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER|0x1000;
                check("background-feat-ignores-high-mask-bits",actor->CheckFeatPrerequisites(feat,1));
                actor->m_typeAI.m_nClassMask=0x800;
                check("background-feat-has-only-eleven-level-bytes",!actor->CheckFeatPrerequisites(feat,1));
                actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER; actor->m_baseStats.m_fighterLevel=2;
                check("background-feat-rejects-second-level",!actor->CheckFeatPrerequisites(feat,1));
            }
            reset(); actor->m_baseStats.m_rogueLevel=1; actor->m_baseStats.m_fighterLevel=4;
            actor->m_derivedStats.m_classMask=CLASSMASK_CLERIC;
            actor->SetFeatValue(CGAMESPRITE_FEAT_POWER_ATTACK,1); actor->SetFeatValue(CGAMESPRITE_FEAT_CLEAVE,1);
            actor->SetFeatValue(CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE,1);
            struct AttackCase { INT feat,minimum; };
            for (const AttackCase& row:{AttackCase{CGAMESPRITE_FEAT_ARTERIAL_STRIKE,4},AttackCase{CGAMESPRITE_FEAT_CLEAVE,4},
                AttackCase{CGAMESPRITE_FEAT_DIRTY_FIGHTING,2},AttackCase{CGAMESPRITE_FEAT_HERETICS_BANE,4},
                AttackCase{CGAMESPRITE_FEAT_IMPROVED_CRITICAL,8},AttackCase{CGAMESPRITE_FEAT_STUNNING_FIST,8},AttackCase{CGAMESPRITE_FEAT_WEAPON_FINESSE,1}}) {
                for (INT attack:{row.minimum-1,row.minimum,255}) { actor->m_baseStats.m_attackBase=attack;
                    check("feat-prerequisite-signed-base-attack-threshold",actor->CheckFeatPrerequisites(row.feat,1)==(attack==row.minimum)); }
            }
            actor->SetFeatValue(CGAMESPRITE_FEAT_MARTIAL_AXE,3); actor->SetFeatValue(CGAMESPRITE_FEAT_SIMPLE_CROSSBOW,3);
            for (INT skill:{3,4,255}) { actor->m_baseStats.m_skills[CGAMESPRITE_SKILL_CONCENTRATION]=skill;
                check("maximized-attacks-signed-concentration",actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_MAXIMIZED_ATTACKS,1)==(skill==4)); }
            for (INT skill:{7,8,255}) { actor->m_baseStats.m_skills[CGAMESPRITE_SKILL_ALCHEMY]=skill;
                check("envenom-weapon-signed-alchemy",actor->CheckFeatPrerequisites(CGAMESPRITE_FEAT_ENVENOM_WEAPON,1)==(skill==8)); }
            actor->m_baseStats=savedBase; actor->m_derivedStats=savedDerived; actor->m_typeAI=ai;
        }
        // The chargen controls share these rules, but have their own row mappings and budgets.
        {
            BeginLevelUpTableFixture cgSkills(rules.m_tSkills,3,16),cgFeats(rules.m_tFeats,3,75);
            for (INT i=0;i<16;++i) { cgSkills.cells[3*i].Format(_T("%d"),15-i);
                cgSkills.cells[3*i+1]="7192"; cgSkills.cells[3*i+2]=i==0?"11982":"7192";
                for (INT cls=0;cls<11;++cls) costs.cells[11*i+cls]=cls==4?(i%2?"2":"1"):"0"; }
            for (INT i=0;i<75;++i) { cgFeats.cells[3*i].Format(_T("%d"),i); cgFeats.cells[3*i+1]=cgFeats.cells[3*i+2]="7192"; }
            actor->m_baseStats=CCreatureFileHeader(); actor->m_baseStats.m_fighterLevel=5; actor->m_baseStats.m_attackBase=4;
            actor->m_baseStats.m_STRBase=actor->m_baseStats.m_DEXBase=actor->m_baseStats.m_INTBase=actor->m_baseStats.m_WISBase=18;
            memset(actor->m_derivedStats.m_nClassLevels,0,11); actor->m_derivedStats.m_fighterLevel=5;
            actor->m_derivedStats.m_nLevel=5; actor->m_derivedStats.m_classMask=CLASSMASK_FIGHTER;
            actor->m_startTypeAI.m_nClass=CAIOBJECTTYPE_C_FIGHTER; actor->m_bAllowEffectListCall=FALSE;
            auto* oldCreate=g_pBaldurChitin->m_pEngineCreateChar;
            CScreenCreateChar create; g_pBaldurChitin->m_pEngineCreateChar=&create;
            create.m_cUIManager.fInit(&create,CResRef("GUICG"),g_pBaldurChitin->m_bUseNewGui); create.m_nGameSprite=id;
            auto* skillPanel=create.m_cUIManager.GetPanel(6); auto* featPanel=create.m_cUIManager.GetPanel(55);
            create.m_lPopupStack.AddTail(skillPanel);
            for (auto& value:create.m_storedSkills) value=0;
            for (auto& value:create.m_storedFeats) value=0;
            auto denies=objects->m_pArray[(id>>16)&0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH];
            auto press=[&](CUIControlButtonPlusMinus* button) { button->SetActive(TRUE); button->field_662=0;
                BOOL result=button->OnLButtonDown(CPoint(0,0)); create.m_cUIManager.KillCapture(); return result; };
            auto normalized=[](CString value) { value.Replace(_T("\r"),_T("")); value.Replace(_T("\n"),_T(" "));
                while (value.Find(_T("  "))!=-1) value.Replace(_T("  "),_T(" ")); value.Trim(); return value; };
            auto help=[&](CUIPanel* panel) { CString text; auto* display=static_cast<CUIControlTextDisplay*>(panel->GetControl(92));
                POSITION pos=display->m_plstStrings->GetHeadPosition(); while (pos) text+=" "+display->m_plstStrings->GetNext(pos)->m_sString;
                return normalized(text); };
            {
                auto savedBase=actor->m_baseStats; CDerivedStats savedDerived; savedDerived=actor->m_derivedStats;
                auto* levelPanel=screen.m_cUIManager.GetPanel(56); screen.field_1840=id;
                for (UINT feat:{38u,39u,40u,41u,42u,43u,44u,53u,54u,55u,56u,57u,18u}) {
                    actor->SetFeatValue(feat,2); actor->m_baseStats.m_fighterLevel=3; actor->m_baseStats.m_attackBase=20;
                    actor->m_derivedStats.m_fighterLevel=20;
                    check("weapon-specialization-requires-four-base-fighter-levels",!actor->CanSelectFeat(feat,1) && !actor->CanSelectFeat(feat,0));
                    actor->m_baseStats.m_fighterLevel=4; actor->m_baseStats.m_attackBase=0; actor->m_derivedStats.m_fighterLevel=0;
                    check("weapon-specialization-does-not-require-base-attack-four",actor->CanSelectFeat(feat,1));
                    actor->SetFeatValue(feat,3);
                    check("weapon-specialization-stops-at-rank-three",!actor->CanSelectFeat(feat,1));
                    actor->SetFeatValue(feat,1); actor->m_baseStats.m_fighterLevel=0;
                    check("earlier-weapon-ranks-do-not-require-fighter",actor->CanSelectFeat(feat,1));
                    actor->SetFeatValue(feat,0);
                }
                for (auto& value:screen.m_storedFeats) value=0;
                for (INT kind:{0,1}) {
                    auto* panel=kind?featPanel:levelPanel;
                    auto& manager=kind?create.m_cUIManager:screen.m_cUIManager;
                    screen.m_lPopupStack.RemoveAll(); create.m_lPopupStack.RemoveAll();
                    if (kind) create.m_lPopupStack.AddTail(panel); else screen.m_lPopupStack.AddTail(panel);
                    auto refresh=[&]() { if (kind) create.UpdatePopupPanel(55,actor); else screen.UpdatePopupPanel(56,actor); };
                    auto click=[&](DWORD control) { auto* button=static_cast<CUIControlButtonPlusMinus*>(panel->GetControl(control));
                        button->SetActive(TRUE); button->field_662=0; BOOL result=button->OnLButtonDown(CPoint(0,0)); manager.KillCapture(); return result; };
                    INT& budget=kind?create.m_nExtraFeats:screen.m_nExtraFeats;
                    for (UINT feat:{38u,39u,53u,55u}) {
                        cgFeats.cells[3*8].Format(_T("%u"),feat); cgFeats.cells[3*feat]="8";
                        actor->SetFeatValue(feat,2); actor->m_baseStats.m_fighterLevel=3; actor->m_baseStats.m_attackBase=20;
                        screen.m_nTopFeat=create.m_nTopFeat=0; budget=1; refresh();
                        check("weapon-specialization-disabled-in-both-panels",!static_cast<CUIControlButton*>(panel->GetControl(30))->m_bEnabled);
                        BOOL down=click(30);
                        check("weapon-specialization-click-cannot-bypass-fighter-gate",down && actor->GetFeatValue(feat)==2 && budget==1);
                        actor->m_baseStats.m_fighterLevel=4; actor->m_baseStats.m_attackBase=0; refresh();
                        check("weapon-specialization-enabled-at-fighter-four",static_cast<CUIControlButton*>(panel->GetControl(30))->m_bEnabled);
                        down=click(30); check("weapon-specialization-purchase-through-real-controls",down && actor->GetFeatValue(feat)==3 && budget==0);
                        down=click(31); check("last-visible-feat-minus-refunds-specialization",down && actor->GetFeatValue(feat)==2 && budget==1);
                        actor->SetFeatValue(feat,0); cgFeats.cells[3*8]="8"; cgFeats.cells[3*feat].Format(_T("%u"),feat);
                    }
                    if (!kind) {
                        cgFeats.cells[0]="69"; cgFeats.cells[3*69]="0"; cgFeats.cells[2]="11982";
                        screen.m_nTopFeat=-1; budget=1; BOOL down=click(16);
                        check("levelup-feat-click-zero-extends-scroll-word-before-help-and-purchase",down && screen.m_nTopFeat==66
                            && actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==0 && budget==1 && help(panel)==normalized(create.FetchString(7192)));
                        cgFeats.cells[0]="0"; cgFeats.cells[3*69]="69"; cgFeats.cells[2]="7192";
                        auto* scroll=static_cast<CUIControlScrollBar*>(panel->GetControl(104));
                        screen.m_nTopFeat=0; scroll->OnScrollUp(); check("levelup-feat-scroll-stops-at-start",screen.m_nTopFeat==0);
                        scroll->OnScrollDown(); check("levelup-feat-scroll-advances-one-row",screen.m_nTopFeat==1);
                        scroll->OnPageDown(999); check("levelup-feat-page-down-uses-eight-rows",screen.m_nTopFeat==9);
                        scroll->OnPageUp(3); check("levelup-feat-page-up-uses-requested-count",screen.m_nTopFeat==6);
                        scroll->OnPageDown(3); check("levelup-feat-page-down-uses-requested-count",screen.m_nTopFeat==9);
                        scroll->OnPageUp(999); check("levelup-feat-page-up-limits-step-to-eight",screen.m_nTopFeat==1);
                        screen.m_nTopFeat=-1; scroll->OnScrollUp(); check("levelup-feat-scroll-up-zero-extends-word",screen.m_nTopFeat==66);
                        screen.m_nTopFeat=-1; scroll->OnPageUp(1); check("levelup-feat-page-up-zero-extends-word",screen.m_nTopFeat==66);
                        screen.m_nTopFeat=-1; scroll->OnScrollDown(); check("levelup-feat-scroll-down-preserves-upper-limit",screen.m_nTopFeat==66);
                        for (SHORT position:{0,50,100}) { scroll->field_142=100; scroll->field_144=position; scroll->OnScroll();
                            check("levelup-feat-thumb-has-no-visible-row-offset",screen.m_nTopFeat==min(75*position/100,66)); }
                    }
                }
                screen.m_cUIManager.KillCapture(); screen.m_lPopupStack.RemoveAll();
                create.m_lPopupStack.RemoveAll(); create.m_lPopupStack.AddTail(skillPanel);
                actor->m_baseStats=savedBase; actor->m_derivedStats=savedDerived;
            }
            UI_CONTROL_BUTTON hotInfo={}; hotInfo.base.nWidth=80; hotInfo.base.nHeight=40; memcpy(hotInfo.refBam,"GUIBTNA",7);
            for (INT row=0;row<11;++row) {
                hotInfo.base.nID=93+row;
                CUIControlButtonCharGenSkillsHotArea skillHelp(skillPanel,&hotInfo);
                CUIControlButtonCharGenFeatsHotArea featHelp(featPanel,&hotInfo);
                create.m_nTopSkill=3; create.m_nTopFeat=4;
                cgSkills.cells[3*(3+row)+2]="11982"; cgFeats.cells[3*(4+row)+2]="11982";
                skillHelp.OnHotAreaClick(CPoint(0,0)); featHelp.OnHotAreaClick(CPoint(0,0));
                check("chargen-help-hotareas-map-all-eleven-scrolled-rows",help(skillPanel)==normalized(create.FetchString(11982))
                    && help(featPanel)==normalized(create.FetchString(11982)));
                cgSkills.cells[3*(3+row)+2]="7192"; cgFeats.cells[3*(4+row)+2]="7192";
            }
            hotInfo.base.nID=94;
            CUIControlButtonCharGenSkillsHotArea wrappedSkillHelp(skillPanel,&hotInfo);
            CUIControlButtonCharGenFeatsHotArea wrappedFeatHelp(featPanel,&hotInfo);
            cgSkills.cells[2]=cgFeats.cells[2]="11982";
            create.m_nTopSkill=create.m_nTopFeat=-1;
            wrappedSkillHelp.OnHotAreaClick(CPoint(0,0)); wrappedFeatHelp.OnHotAreaClick(CPoint(0,0));
            check("chargen-help-zero-extends-scroll-word-before-row-offset",help(skillPanel)==normalized(create.FetchString(7192))
                && help(featPanel)==normalized(create.FetchString(11982)));
            cgSkills.cells[2]="11982"; cgFeats.cells[2]="7192";
            for (INT row=0;row<10;++row) {
                auto* plus=static_cast<CUIControlButtonCharGenSkillsPlusMinus*>(skillPanel->GetControl(14+2*row));
                auto* minus=static_cast<CUIControlButtonCharGenSkillsPlusMinus*>(skillPanel->GetControl(15+2*row));
                create.m_nTopSkill=0; create.m_nExtraSkillPoints=4; BOOL down=press(plus); INT selected=15-row;
                check("chargen-skill-purchase-maps-all-ten-rows-and-charges-native-remapped-cost",down
                    && actor->GetSkillValue(selected)==1 && create.m_nExtraSkillPoints==4-(1+row%2));
                minus->AdjustValue(); check("chargen-skill-refund-maps-all-ten-rows",actor->GetSkillValue(selected)==0 && create.m_nExtraSkillPoints==4);
            }
            auto* skillPlus=static_cast<CUIControlButtonCharGenSkillsPlusMinus*>(skillPanel->GetControl(14));
            auto* skillMinus=static_cast<CUIControlButtonCharGenSkillsPlusMinus*>(skillPanel->GetControl(15));
            create.m_nTopSkill=-1; create.m_nExtraSkillPoints=4;
            BOOL down=press(static_cast<CUIControlButtonCharGenSkillsPlusMinus*>(skillPanel->GetControl(16)));
            check("chargen-skill-mouse-down-zero-extends-top-before-help-and-purchase",down && create.m_nTopSkill==6
                && actor->GetSkillValue(0)==1 && actor->GetSkillValue(15)==0 && create.m_nExtraSkillPoints==2
                && help(skillPanel)==normalized(create.FetchString(7192)));
            actor->SetSkillValue(0,0); costs.cells[4]="2"; costs.cells[11*15+4]="1";
            create.m_nTopSkill=0; create.m_nExtraSkillPoints=1; create.UpdatePopupPanel(6,actor);
            check("chargen-skill-label-and-availability-use-different-native-cost-lookups",skillPlus->m_bEnabled
                && static_cast<CUIControlLabel*>(skillPanel->GetControl(0x10000001))->GetText().Find(_T("(2)"))>=0);
            skillPlus->AdjustValue(); check("chargen-skill-adjust-rejects-insufficient-remapped-cost",actor->GetSkillValue(15)==0 && create.m_nExtraSkillPoints==1);
            for (INT i=0;i<16;++i) { cgSkills.cells[3*i].Format(_T("%d"),i); costs.cells[11*i+4]="2"; }
            actor->SetSkillValue(0,4); create.m_nExtraSkillPoints=0; skillMinus->AdjustValue();
            check("chargen-skill-at-cap-refunds-cost-recomputed-after-decrement",actor->GetSkillValue(0)==3 && create.m_nExtraSkillPoints==2);
            skillPlus->AdjustValue(); skillPlus->AdjustValue();
            check("chargen-skill-cap-blocks-further-purchase",actor->GetSkillValue(0)==4 && create.m_nExtraSkillPoints==0);
            create.m_storedSkills[0]=4; skillMinus->AdjustValue();
            check("chargen-skill-cannot-refund-original-ranks",actor->GetSkillValue(0)==4 && create.m_nExtraSkillPoints==0);
            create.m_nTopSkill=-32768; create.UpdatePopupPanel(6,actor);
            check("chargen-skill-refresh-clamps-unsigned-scroll-position",create.m_nTopSkill==6);
            create.m_lPopupStack.RemoveAll(); create.m_lPopupStack.AddTail(featPanel);
            cgFeats.cells[0]="69"; cgFeats.cells[3*69]="0"; cgFeats.cells[2]="11982";
            create.m_nTopFeat=-1; create.m_nExtraFeats=2;
            down=press(static_cast<CUIControlButtonCharGenFeatsPlusMinus*>(featPanel->GetControl(16)));
            check("chargen-feat-mouse-down-zero-extends-top-before-help-and-purchase",down && create.m_nTopFeat==65
                && actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==0 && create.m_nExtraFeats==2
                && help(featPanel)==normalized(create.FetchString(7192)));
            create.m_nTopFeat=-32768; create.UpdatePopupPanel(55,actor);
            check("chargen-feat-refresh-clamps-unsigned-scroll-position",create.m_nTopFeat==65);
            cgFeats.cells[0]="0"; cgFeats.cells[3*9]="69"; cgFeats.cells[3*69]="9"; cgFeats.cells[3*9+2]="11982";
            create.m_nTopFeat=0; create.m_nExtraFeats=0; actor->SetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS,1);
            auto* featPlus=static_cast<CUIControlButtonCharGenFeatsPlusMinus*>(featPanel->GetControl(32));
            auto* featMinus=static_cast<CUIControlButtonCharGenFeatsPlusMinus*>(featPanel->GetControl(33));
            down=press(featMinus);
            check("chargen-tenth-feat-minus-button-refunds-mapped-rank-without-asserting",down
                && actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==0 && create.m_nExtraFeats==1
                && help(featPanel)==normalized(create.FetchString(11982)));
            down=press(featPlus); check("chargen-feat-purchase-spends-one-point",down
                && actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==1 && create.m_nExtraFeats==0);
            create.m_storedFeats[CGAMESPRITE_FEAT_TOUGHNESS]=1; featMinus->AdjustValue();
            check("chargen-feat-cannot-refund-original-ranks",actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==1 && create.m_nExtraFeats==0);
            featPlus->AdjustValue(); check("chargen-feat-no-points-blocks-purchase",actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==1);
            actor->SetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS,5); create.m_nExtraFeats=2; featPlus->AdjustValue();
            check("chargen-feat-max-rank-blocks-purchase",actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==5 && create.m_nExtraFeats==2);
            cgFeats.cells[0]="0"; cgFeats.cells[2]="7192";
            static_cast<CUIControlButtonCharGenFeatsPlusMinus*>(featPanel->GetControl(14))->AdjustValue();
            check("chargen-feat-prerequisites-block-purchase",actor->GetFeatValue(0)==0 && create.m_nExtraFeats==2);
            for (INT i=0;i<75;++i) { cgFeats.cells[3*i].Format(_T("%d"),i); actor->SetFeatValue(i,0); }
            cgFeats.cells[3*9]="47"; cgFeats.cells[3*47]="9";
            actor->SetFeatValue(CGAMESPRITE_FEAT_POWER_ATTACK,1); actor->SetFeatValue(CGAMESPRITE_FEAT_CLEAVE,2);
            actor->SetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS,1); create.m_storedFeats[CGAMESPRITE_FEAT_CLEAVE]=2; create.m_nExtraFeats=0;
            featMinus->AdjustValue();
            check("chargen-feat-removal-refunds-all-invalid-dependent-ranks-even-in-snapshot",actor->GetFeatValue(CGAMESPRITE_FEAT_POWER_ATTACK)==0
                && actor->GetFeatValue(CGAMESPRITE_FEAT_CLEAVE)==0 && actor->GetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS)==1 && create.m_nExtraFeats==3);
            actor->m_derivedStats.m_nSTR=18; create.m_storedFeats[CGAMESPRITE_FEAT_CLEAVE]=0;
            featPlus->AdjustValue();
            check("chargen-can-purchase-recovered-power-attack",actor->GetFeatValue(CGAMESPRITE_FEAT_POWER_ATTACK)==1
                && actor->HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK) && create.m_nExtraFeats==2);
            cgFeats.cells[3*9]="8"; cgFeats.cells[3*8]="47";
            featPlus->AdjustValue(); featPlus->AdjustValue();
            check("chargen-power-attack-unlocks-both-cleave-ranks",actor->GetFeatValue(CGAMESPRITE_FEAT_CLEAVE)==2
                && actor->HasFeat(CGAMESPRITE_FEAT_CLEAVE) && create.m_nExtraFeats==0);
            cgFeats.cells[3*9]="47"; cgFeats.cells[3*8]="8";
            featMinus->AdjustValue();
            check("chargen-removing-purchased-power-attack-refunds-cleave-chain",actor->GetFeatValue(CGAMESPRITE_FEAT_POWER_ATTACK)==0
                && actor->GetFeatValue(CGAMESPRITE_FEAT_CLEAVE)==0 && create.m_nExtraFeats==3);
            for (INT kind:{0,1}) {
                auto* panel=kind?featPanel:skillPanel; create.m_lPopupStack.RemoveAll(); create.m_lPopupStack.AddTail(panel);
                auto* scroll=static_cast<CUIControlScrollBar*>(panel->GetControl(104));
                SHORT& top=kind?create.m_nTopFeat:create.m_nTopSkill; INT limit=kind?65:6,total=kind?75:16;
                top=0; scroll->OnScrollUp(); check("chargen-scroll-up-stops-at-start",top==0);
                scroll->OnScrollDown(); check("chargen-scroll-down-advances-one-row",top==1);
                scroll->OnPageDown(999); check("chargen-page-down-is-limited-to-nine-rows",top==min(10,limit));
                top=5; scroll->OnPageUp(3); check("chargen-page-up-uses-requested-row-count",top==2);
                scroll->OnPageDown(3); check("chargen-page-down-uses-requested-row-count",top==5);
                scroll->OnPageUp(999); check("chargen-page-up-clamps-at-start",top==0);
                top=-1; scroll->OnScrollUp(); check("chargen-scroll-up-zero-extends-stored-word",top==limit);
                top=-1; scroll->OnPageUp(1); check("chargen-page-up-zero-extends-stored-word",top==limit);
                top=-1; scroll->OnScrollDown(); check("chargen-scroll-down-preserves-native-wrap-and-upper-limit",top==(kind?65:0));
                top=limit; scroll->OnScrollDown(); check("chargen-scroll-down-stops-at-last-page",top==limit);
                for (INT position:{0,25,50,100}) {
                    scroll->field_142=100; scroll->field_144=position; scroll->OnScroll();
                    check("chargen-thumb-position-uses-full-list-without-subtracting-visible-rows",top==min(total*position/100,limit));
                }
            }
            // Render under THREAD_1 shares, as the real renderer does; the outer fixture owns an ASYNCH deny.
            auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode()); CRect patch(32,32,128,128);
            auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write) {
                if (!video->BKLock(patch)) return FALSE;
                DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8; INT row=patch.Width()*bytes;
                BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
                if (ok && read) read->resize(row*patch.Height());
                if (ok) for (INT y=0;y<patch.Height();++y) { BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
                    if (read) memcpy(read->data()+row*y,p,row); if (write) memcpy(p,write->data()+row*y,row); }
                video->BKUnlock(); return ok;
            };
            std::vector<BYTE> savedPixels; BOOL backed=pixels(&savedPixels,NULL); check("chargen-rank-pixels-backed-up",backed);
            if (backed) {
                std::vector<BYTE> blank(savedPixels.size(),0),expectedPixels,actualPixels;
                featPanel->m_ptOrigin=CPoint(48,48); create.m_nTopFeat=0; actor->SetFeatValue(CGAMESPRITE_FEAT_TOUGHNESS,2);
                auto effects=actor->field_72A2; auto dirtyBefore=actor->field_562C; INT budgetBefore=create.m_nExtraFeats;
                CVidCell reference; reference.SetResRef(CResRef("GUIPFC"),FALSE,TRUE,TRUE);
                auto* resource=reference.pRes; INT demands=resource?resource->GetDemands():0,requests=resource?resource->m_nLockCount:0;
                auto shares=objects->m_pArray[(id>>16)&0x7FFF].m_shareCounts[CGameObjectArray::THREAD_1];
                objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                struct RankRenderCase { DWORD control; UINT feat; BOOL doubled,clipped; SHORT count; BOOL force,drawn; SHORT frame; };
                const RankRenderCase renderCases[]={
                    {40,69,FALSE,FALSE,1,FALSE,TRUE,0}, {36,69,TRUE,TRUE,1,FALSE,TRUE,1},
                    {36,6,FALSE,FALSE,1,FALSE,FALSE,0}, {40,0,FALSE,FALSE,1,FALSE,FALSE,0},
                    {40,69,FALSE,FALSE,0,FALSE,FALSE,0}, {40,69,FALSE,FALSE,-1,FALSE,TRUE,0}
                };
                for (const auto& row:renderCases) {
                    cgFeats.cells[0]="47"; cgFeats.cells[3*47].Format(_T("%u"),row.feat);
                    create.m_cUIManager.m_bDoubleSize=row.doubled; reference.SetResRef(CResRef("GUIPFC"),row.doubled,TRUE,TRUE);
                    auto* circle=static_cast<CUIControlButtonCharGenFeatsCircle*>(featPanel->GetControl(row.control));
                    circle->m_ptOrigin=CPoint(8,8); circle->m_size=CSize(24,24); circle->m_bActive=TRUE; circle->m_bInactiveRender=FALSE;
                    CRect rect(featPanel->m_ptOrigin+circle->m_ptOrigin,circle->m_size); circle->m_rDirty=rect;
                    if (row.clipped) circle->m_rDirty.right=rect.left+4;
                    BOOL ok=pixels(NULL,&blank);
                    if (row.drawn) { reference.FrameSet(row.frame); ok=ok && reference.Render(0,rect.left,rect.top,circle->m_rDirty,NULL,0,0,-1); }
                    ok=ok && pixels(&expectedPixels,NULL) && pixels(NULL,&blank); circle->m_nRenderCount=row.count;
                    BOOL result=circle->Render(row.force); ok=ok && pixels(&actualPixels,NULL);
                    check("chargen-rank-render-matches-native-frame-scale-clip-and-return",ok && result==row.drawn && actualPixels==expectedPixels
                        && circle->m_nRenderCount==(row.count>0?row.count-1:row.count));
                    if (row.drawn && !row.clipped) check("chargen-rank-render-produces-visible-pixels",actualPixels!=blank);
                    check("chargen-rank-render-does-not-refresh-or-process-effects",actor->field_72A2==effects
                        && actor->field_562C==dirtyBefore && create.m_nExtraFeats==budgetBefore && create.m_nTopFeat==0);
                }
                auto* circle=static_cast<CUIControlButtonCharGenFeatsCircle*>(featPanel->GetControl(40));
                circle->m_bActive=FALSE; circle->m_nRenderCount=1;
                check("chargen-inactive-rank-render-rejected-without-consuming-dirty-count",!circle->Render(TRUE) && circle->m_nRenderCount==1);
                create.m_nTopFeat=-1; cgFeats.cells[0]="0"; cgFeats.cells[3]="47"; cgFeats.cells[3*47]="69";
                circle=static_cast<CUIControlButtonCharGenFeatsCircle*>(featPanel->GetControl(50)); circle->m_bActive=TRUE; circle->m_nRenderCount=1;
                circle->m_ptOrigin=CPoint(8,8); circle->m_size=CSize(24,24);
                circle->m_rDirty=CRect(featPanel->m_ptOrigin+circle->m_ptOrigin,circle->m_size);
                check("chargen-rank-render-zero-extends-scroll-word-without-mutating-panel",!circle->Render(TRUE)
                    && create.m_nTopFeat==-1 && actor->field_72A2==effects);
                create.m_nGameSprite=CGameObjectArray::INVALID_INDEX; circle->m_nRenderCount=1;
                check("chargen-rank-render-missing-actor-rejected",!circle->Render(TRUE)); create.m_nGameSprite=id;
                auto* levelPanel=screen.m_cUIManager.GetPanel(56); levelPanel->m_ptOrigin=CPoint(48,48);
                screen.m_nTopFeat=0; screen.field_1840=id; create.m_nGameSprite=CGameObjectArray::INVALID_INDEX;
                INT levelBudget=screen.m_nExtraFeats;
                for (const auto& row:renderCases) {
                    cgFeats.cells[0]="47"; cgFeats.cells[3*47].Format(_T("%u"),row.feat);
                    screen.m_cUIManager.m_bDoubleSize=row.doubled; reference.SetResRef(CResRef("GUIPFC"),row.doubled,TRUE,TRUE);
                    auto* rank=static_cast<CUIControlButtonCharacterFeatsCircle*>(levelPanel->GetControl(row.control));
                    rank->m_ptOrigin=CPoint(8,8); rank->m_size=CSize(24,24); rank->m_bActive=TRUE; rank->m_bInactiveRender=FALSE;
                    CRect rect(levelPanel->m_ptOrigin+rank->m_ptOrigin,rank->m_size); rank->m_rDirty=rect;
                    if (row.clipped) rank->m_rDirty.right=rect.left+4;
                    BOOL ok=pixels(NULL,&blank);
                    if (row.drawn) { reference.FrameSet(row.frame); ok=ok && reference.Render(0,rect.left,rect.top,rank->m_rDirty,NULL,0,0,-1); }
                    ok=ok && pixels(&expectedPixels,NULL) && pixels(NULL,&blank); rank->m_nRenderCount=row.count;
                    BOOL result=rank->Render(row.force); ok=ok && pixels(&actualPixels,NULL);
                    check("levelup-rank-render-uses-character-actor-and-native-pixels",ok && result==row.drawn && actualPixels==expectedPixels
                        && rank->m_nRenderCount==(row.count>0?row.count-1:row.count));
                    check("levelup-rank-render-does-not-refresh-or-process-effects",actor->field_72A2==effects && actor->field_562C==dirtyBefore
                        && screen.m_nExtraFeats==levelBudget && screen.m_nTopFeat==0 && create.m_nGameSprite==CGameObjectArray::INVALID_INDEX);
                }
                auto* rank=static_cast<CUIControlButtonCharacterFeatsCircle*>(levelPanel->GetControl(40));
                rank->m_bActive=FALSE; rank->m_nRenderCount=1;
                check("levelup-inactive-rank-render-preserves-dirty-count",!rank->Render(TRUE) && rank->m_nRenderCount==1);
                screen.m_nTopFeat=-1; cgFeats.cells[0]="0"; cgFeats.cells[3]="47"; cgFeats.cells[3*47]="69";
                rank=static_cast<CUIControlButtonCharacterFeatsCircle*>(levelPanel->GetControl(50)); rank->m_bActive=TRUE; rank->m_nRenderCount=1;
                rank->m_ptOrigin=CPoint(8,8); rank->m_size=CSize(24,24); rank->m_rDirty=CRect(levelPanel->m_ptOrigin+rank->m_ptOrigin,rank->m_size);
                check("levelup-rank-render-zero-extends-scroll-word",!rank->Render(TRUE) && screen.m_nTopFeat==-1 && actor->field_72A2==effects);
                screen.field_1840=CGameObjectArray::INVALID_INDEX; rank->m_nRenderCount=1;
                check("levelup-rank-render-missing-character-rejected",!rank->Render(TRUE)); screen.field_1840=id; create.m_nGameSprite=id;
                check("chargen-rank-render-balances-shares-and-resource-requests",objects->m_pArray[(id>>16)&0x7FFF].m_shareCounts[CGameObjectArray::THREAD_1]==shares
                    && resource && resource->GetDemands()==demands && resource->m_nLockCount==requests);
                check("chargen-rank-pixels-restored",pixels(NULL,&savedPixels));
                CGameObject* reacquired=NULL; BYTE rc;
                do { rc=objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&reacquired,INFINITE); }
                while (rc==CGameObjectArray::SHARED || rc==CGameObjectArray::DENIED);
                check("chargen-rank-render-fixture-reacquires-original-actor",rc==CGameObjectArray::SUCCESS && reacquired==actor);
            }
            check("chargen-allocation-controls-balance-object-locks",objects->m_pArray[(id>>16)&0x7FFF].m_denyCounts[CGameObjectArray::THREAD_ASYNCH]==denies);
            create.m_cUIManager.KillCapture(); create.m_lPopupStack.RemoveAll(); g_pBaldurChitin->m_pEngineCreateChar=oldCreate;
        }
        BeginLevelUpTableFixture raceReq(rules.m_tAbilityRaceReq,2,1),raceAdj(rules.m_tAbilityRaceAdj,1,1),classReq(rules.m_tAbilityClassReq,1,1);
        actor->m_startTypeAI.m_nRace=CAIOBJECTTYPE_R_HUMAN; actor->m_startTypeAI.m_nSubRace=0;
        actor->m_baseStats.m_specialization=0;
        raceReq.x[0]="MIN"; raceReq.x[1]="MAX"; raceAdj.x[0]="MOD"; classReq.x[0]="MIN";
        raceReq.y[0]=raceAdj.y[0]=rules.GetRaceString(CAIOBJECTTYPE_R_HUMAN,0);
        classReq.y[0]=rules.GetClassString(CAIOBJECTTYPE_C_FIGHTER,0);
        for (const auto& row : ABILITY_CONSTRAINT_CASES) {
            ++index; raceReq.cells[0]=row.raceMin; raceReq.cells[1]=row.raceMax;
            raceAdj.cells[0]=row.adj; classReq.cells[0]=row.classMin;
            BYTE ability=165,minimum=165,maximum=165; INT modifier=0;
            BOOL result=screen.ResetAbility(actor,"MIN","MAX","MOD",ability,minimum,maximum,modifier);
            check("canonical-signed-ability-constraint",result==row.result && ability==row.ability
                && minimum==row.minimum && maximum==row.maximum && modifier==row.modifier);
            Emit("{\"op\":\"allocation-result\",\"case\":%d,\"kind\":2,\"result\":%d,\"ability\":%u,\"minimum\":%u,\"maximum\":%u,\"modifier\":%d}",index,result,ability,minimum,maximum,modifier);
        }
        g_pBaldurChitin->m_pEngineCharacter=oldCharacter;
    }
    if (hadToken) tlk.SetToken(CScreenCharacter::TOKEN_NUMBER,oldToken); else tlk.m_mapTokens.RemoveKey(CScreenCharacter::TOKEN_NUMBER);
    for (INT i=0;i<3;++i) { if (hadExtraTokens[i]) tlk.SetToken(*extraTokens[i],savedTokens[i]); else tlk.m_mapTokens.RemoveKey(*extraTokens[i]); }
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp; actor->m_startTypeAI=start; actor->m_typeAI=ai;
    actor->m_bAllowEffectListCall=allow; actor->field_562C=dirty; actor->field_72A2=deferred; actor->m_nHPCONBonusTotalOld=bonus;
    check("same-tick-clock-and-queue-preserved",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"allocationchecks\",\"nativeCases\":424,\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"allocationchecks","native panel/ability mismatch");
}
