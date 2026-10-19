// AutoUI-only native HP/RNG replay and real character-creation caller.
#include "HitPointsGolden.h"

void HitPointsChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame();
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    auto* options=game->GetOptions(); BOOL maximum=options->m_bMaxHP;
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"hitpoint-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto* timer=game->GetWorldTimer(); ULONG clock=timer->m_gameTime;
    C2DArray* banks[]={&rules.m_tHitPointsWarrior,&rules.m_tHitPointsWizard,&rules.m_tHitPointsPriest,
        &rules.m_tHitPointsRogue,&rules.m_tHitPointsBarbarian,&rules.m_tHitPointsMonk};
    std::vector<BeginLevelUpTableFixture*> tables;
    for (auto* bank : banks) {
        auto* table=new BeginLevelUpTableFixture(*bank,3,255);
        table->x[0]=CRuleTables::SIDES; table->x[1]=CRuleTables::ROLLS; table->x[2]=CRuleTables::MODIFIER;
        for (INT level=0;level<255;++level) table->y[level].Format(_T("%d"),level+1);
        tables.push_back(table);
    }
    auto fill=[&](INT profile) {
        for (UINT bank=0;bank<6;++bank) for (UINT level=0;level<255;++level) for (UINT column=0;column<3;++column)
            tables[bank]->cells[3*level+column]=HITPOINT_TABLE_CELLS[profile][bank][level][column];
    };
    {
        CDerivedStats oldStats,newStats; CAIObjectType type;
        INT profile=-1;
        for (const auto& row : HITPOINT_CASES) {
            ++index;
            if (profile!=row.profile) { profile=row.profile; fill(profile); }
            memcpy(oldStats.m_nClassLevels,row.oldLevels,11); memcpy(newStats.m_nClassLevels,row.newLevels,11);
            options->m_bMaxHP=row.maximum; srand(row.seed);
            const INT* a=row.args; INT result;
            switch (row.kind) {
            case 0: result=rules.RollHitPoints(a[0],a[1],a[2],a[3],a[4]); break;
            case 1: result=rules.RollHitPoints(*banks[a[6]],static_cast<BYTE>(a[0]),static_cast<BYTE>(a[1]),a[2],a[3],a[4],a[5]); break;
            case 2: result=rules.RollHitPoints(a[0],oldStats,newStats,a[1],a[2]); break;
            default: result=rules.GetStartingHitPoints(type,oldStats,newStats,static_cast<BYTE>(a[0])); break;
            }
            INT next=rand();
            check("canonical-hitpoint-return",result==row.result);
            check("canonical-random-consumption",next==row.nextRandom);
            Emit("{\"op\":\"hitpoint-result\",\"case\":%d,\"kind\":%d,\"result\":%d,\"nextRandom\":%d}",index,row.kind,result,next);
        }
    }
    // Exercise the real caller with effects deferred and class grants empty.
    auto* objects=game->GetObjectArray(); LONG id=game->GetCharacterId(0); CGameObject* object=NULL;
    BYTE rc=objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE);
    check("creation-caller-actor-available",rc==CGameObjectArray::SUCCESS);
    if (rc==CGameObjectArray::SUCCESS) {
        auto* actor=static_cast<CGameSprite*>(object);
        auto base=actor->m_baseStats; CDerivedStats derived,temp,empty;
        derived=actor->m_derivedStats; temp=actor->m_tempStats;
        auto type=actor->m_typeAI,start=actor->m_startTypeAI,live=actor->m_liveTypeAI;
        auto innates=actor->m_innateSpells; auto allow=actor->m_bAllowEffectListCall;
        auto dirty=actor->field_562C; auto deferred=actor->field_72A2; auto bonus=actor->m_nHPCONBonusTotalOld;
        {
            ChargenTableFixture grants(rules.m_tClassAbilitiesFighter);
            grants.Set("*"); fill(0); options->m_bMaxHP=TRUE;
            for (INT level : {1,3}) for (INT populated : {0,1}) {
                actor->m_baseStats=base; actor->m_derivedStats=empty;
                memset(actor->m_derivedStats.m_nClassLevels,0,11);
                actor->m_derivedStats.m_fighterLevel=static_cast<BYTE>(level);
                actor->m_derivedStats.m_nLevel=static_cast<BYTE>(level);
                actor->m_derivedStats.m_classMask=CLASSMASK_FIGHTER;
                actor->m_tempStats=actor->m_derivedStats;
                actor->m_typeAI=type; actor->m_typeAI.m_nClass=CAIOBJECTTYPE_C_FIGHTER;
                actor->m_typeAI.m_nRace=CAIOBJECTTYPE_R_HUMAN; actor->m_typeAI.m_nSubRace=0;
                actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER;
                actor->m_startTypeAI=actor->m_liveTypeAI=actor->m_typeAI;
                actor->m_baseStats.m_CONBase=21; actor->m_baseStats.m_DEXBase=19; actor->m_baseStats.m_WISBase=23;
                actor->m_bAllowEffectListCall=FALSE;
                actor->m_innateSpells.m_List.clear();
                if (populated) { CGameSpriteSpellListEntry entry={0,1,1,0}; actor->m_innateSpells.m_List.push_back(entry); }
                actor->m_innateSpells.m_nSharedCurrent=91;
                actor->m_innateSpells.m_nSharedMax=92; actor->m_innateSpells.m_nSharedTotal=93;
                SHORT calls=actor->field_72A2;
                g_pBaldurChitin->m_pEngineCreateChar->CompleteCharacterAbilities(actor);
                check("creation-uses-zero-old-and-live-new-stats",actor->m_baseStats.m_hitPoints==10*level
                    && actor->m_baseStats.m_maxHitPointsBase==10*level && actor->m_nHPCONBonusTotalOld==0);
                check("creation-preserves-empty-bank-aggregates",actor->m_innateSpells.m_List.empty()
                    && actor->m_innateSpells.m_nSharedCurrent==91 && actor->m_innateSpells.m_nSharedMax==(populated?0:92)
                    && actor->m_innateSpells.m_nSharedTotal==(populated?0:93));
                check("creation-requests-two-effect-passes",actor->field_562C==TRUE && actor->field_72A2==static_cast<SHORT>(calls+2));
            }
        }
        actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
        actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
        actor->m_innateSpells=innates; actor->m_bAllowEffectListCall=allow;
        actor->field_562C=dirty; actor->field_72A2=deferred; actor->m_nHPCONBonusTotalOld=bonus;
        objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    }
    for (auto* table : tables) delete table;
    options->m_bMaxHP=maximum; srand(1);
    check("same-tick-clock-and-queue-preserved",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    Emit("{\"step\":%d,\"op\":\"hitpointchecks\",\"nativeCases\":%u,\"checks\":%d,\"passed\":%s}",nStep,static_cast<UINT>(_countof(HITPOINT_CASES)),checks,valid?"true":"false");
    if (!valid) Fail(nStep,"hitpointchecks","native HP/RNG or creation caller mismatch");
}
