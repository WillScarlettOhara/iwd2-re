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
    auto start=actor->m_startTypeAI; auto allow=actor->m_bAllowEffectListCall;
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
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp; actor->m_startTypeAI=start;
    actor->m_bAllowEffectListCall=allow; actor->field_562C=dirty; actor->field_72A2=deferred; actor->m_nHPCONBonusTotalOld=bonus;
    check("same-tick-clock-and-queue-preserved",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"allocationchecks\",\"nativeCases\":424,\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"allocationchecks","native panel/ability mismatch");
}
