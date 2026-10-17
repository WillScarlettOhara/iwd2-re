// AutoUI-only native class/feat replay, with temporary typed party state.
#include "ClassAbilityGolden.h"

void ClassAbilityChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<1) { Fail(nStep,"classabilitychecks","requires loaded party"); return; }
    LONG id=game->GetFixedOrderCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"classabilitychecks","party actor unavailable"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    CSpell guard(CResRef("ZRCHIT")); BOOL demanded=guard.Demand();
    if (!demanded || !guard.pRes || !guard.pRes->m_bParsed) {
        if (demanded) guard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"classabilitychecks","requires pinned fixture SPL"); return;
    }
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"class-ability-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    auto base=actor->m_baseStats; CDerivedStats derived,temp,empty; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto type=actor->m_typeAI; auto allow=actor->m_bAllowEffectListCall; auto global=actor->m_bGlobal; LONG oldId=actor->m_id;
    auto innates=actor->m_innateSpells,songs=actor->m_songs,shapes=actor->m_shapeshifts;
    auto* oldEngine=g_pBaldurChitin->pActiveEngine; auto selected=g_pBaldurChitin->m_pEngineCharacter->m_nSelectedCharacter;
    g_pBaldurChitin->pActiveEngine=g_pBaldurChitin->m_pEngineCharacter;
    auto& tlk=g_pBaldurChitin->GetTlkTable(); CString oldToken; BOOL hadToken=tlk.m_mapTokens.Lookup(CGameSprite::TOKEN_SPECIALABILITY,oldToken);
    STR_RES name; tlk.Fetch(10453,name);
    auto dirty=actor->field_562C; BYTE saves[]={actor->field_70F6,actor->field_70F7,actor->field_70F8,actor->field_70F9,actor->field_70FA};
    auto* timer=game->GetWorldTimer(); auto active=timer->m_active; ULONG clock=timer->m_gameTime; timer->m_active=TRUE;
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT timed=actor->m_timedEffectList.GetCount(),equipped=actor->m_equipedEffectList.GetCount();
    struct Bank { CButtonData* p; INT n; }; Bank banks[]={{actor->m_quickWeapons,8},{actor->m_quickSpells,9},
        {actor->m_quickItems,3},{actor->m_quickInnates,9},{actor->m_quickSongs,9}};
    std::vector<CButtonData> buttons; for (const auto& b : banks) for (INT j=0;j<b.n;++j) buttons.push_back(b.p[j]);
    BYTE cache[8]; memcpy(cache,actor->field_3D3A,8);
    CSpellResRefList* catalogues[]={&game->m_innateSpells,&game->m_songs,&game->m_shapeshifts};
    CResRef* oldData[3]; UINT oldCounts[3]; CResRef refs[16];
    for (INT i=0;i<16;++i) {
        refs[i]=CLASS_ABILITY_REFS[i];
        check("grant-resource-present-in-KEY",g_pChitin->cDimm.m_cKeyTable.FindKey(refs[i],1006,FALSE)!=NULL);
    }
    for (INT i=0;i<3;++i) { oldData[i]=catalogues[i]->m_pList; oldCounts[i]=catalogues[i]->m_nCount;
        catalogues[i]->m_pList=refs; catalogues[i]->m_nCount=i==0?16:0; }
    auto* resource=guard.pRes; auto* oldHeader=resource->m_pHeader; auto* oldAbilities=resource->m_pAbilities; auto* oldEffects=resource->m_pEffects;
    SPELL_HEADER header=*oldHeader; SPELL_ABILITY ability={}; ITEM_EFFECT effects[2]={};
    header.genericName=10453; header.abilityCount=1; header.school=0; header.itemFlags=0; header.spellLevel=1;
    ability.minCasterLevel=1; ability.effectCount=2; ability.startingEffect=0;
    for (INT i=0;i<2;++i) { effects[i].effectID=CGAMEEFFECT_STR; effects[i].effectAmount=i+2;
        effects[i].durationType=3; effects[i].duration=7+i; effects[i].probabilityUpper=100; }
    resource->m_pHeader=&header; resource->m_pAbilities=&ability; resource->m_pEffects=effects;
    INT demands=resource->GetDemands(),requests=resource->m_nLockCount,keyRefs=resource->m_pDimmKeyTableEntry->field_10;
    auto& rules=game->GetRuleTables(); std::vector<ClassAbilityTableFixture*> tables;
    std::set<C2DArray*> unique;
    const DWORD specializations[]={0,SPECMASK_CLERIC_MASK};
    for (BYTE cls=1;cls<=11;++cls) for (DWORD spec : specializations) {
        auto* table=const_cast<C2DArray*>(rules.GetClassAbilityTable(cls,spec));
        if (unique.insert(table).second) tables.push_back(new ClassAbilityTableFixture(*table));
    }
    for (const auto& row : CLASS_ABILITIES) {
        ++index;
        actor->m_baseStats=CCreatureFileHeader(); auto& b=actor->m_baseStats;
        b.m_hitPoints=static_cast<SHORT>(row.hp); b.m_maxHitPointsBase=static_cast<WORD>(row.hp); b.m_attackBase=8;
        b.m_numberOfAttacksBase=b.m_saveVSFortitudeBase=b.m_saveVSReflexBase=b.m_saveVSWillBase=static_cast<BYTE>(row.seed);
        b.m_resistFireBase=b.m_resistColdBase=b.m_resistElectricityBase=b.m_resistAcidBase=b.m_resistMagicBase=static_cast<signed char>(row.seed);
        b.m_resistMagicFireBase=b.m_resistMagicColdBase=b.m_resistSlashingBase=b.m_resistCrushingBase=b.m_resistPiercingBase=b.m_resistMissileBase=b.m_resistMagicDamageBase=static_cast<signed char>(row.seed);
        memcpy(b.m_feats,row.feats,sizeof(b.m_feats)); b.m_specialization=row.spec;
        b.m_featToughness=row.ranks[0]; b.m_featExtraRage=row.ranks[1]; b.m_featExtraSmiting=row.ranks[2]; b.m_featExtraTurning=row.ranks[3];
        CCreatureFileHeader previous; memcpy(previous.m_feats,row.oldFeats,sizeof(previous.m_feats));
        previous.m_featToughness=row.oldRanks[0]; previous.m_featExtraRage=row.oldRanks[1]; previous.m_featExtraSmiting=row.oldRanks[2]; previous.m_featExtraTurning=row.oldRanks[3];
        actor->m_typeAI=type; actor->m_typeAI.m_nClassMask=0; actor->m_typeAI.m_nRace=static_cast<BYTE>(row.race); actor->m_typeAI.m_nSubRace=static_cast<BYTE>(row.subrace);
        actor->m_bAllowEffectListCall=row.active; actor->m_bGlobal=TRUE; actor->m_id=-1;
        actor->field_70F6=actor->field_70F7=actor->field_70F8=actor->field_70F9=actor->field_70FA=0;
        actor->m_derivedStats=empty; actor->m_tempStats=empty; CDerivedStats previousStats;
        CDerivedStats* stats[]={&actor->m_derivedStats,&actor->m_tempStats,&previousStats}; const BYTE* levels[]={row.levels,row.tempLevels,row.oldLevels};
        for (INT i=0;i<3;++i) { stats[i]->m_classMask=1; stats[i]->m_nLevel=static_cast<BYTE>(i==0?row.total:i==2?row.oldTotal:0);
            memcpy(stats[i]->m_nClassLevels,levels[i],11); stats[i]->m_nDEX=stats[i]->m_nWIS=13; }
        actor->m_innateSpells=CGameSpriteSpellList(); actor->m_songs=CGameSpriteSpellList(); actor->m_shapeshifts=CGameSpriteSpellList();
        if (row.removes) { CGameSpriteSpellListEntry seed={15,3,3,0}; actor->m_innateSpells.m_List.push_back(seed); }
        for (auto* table : tables) { for (auto& cell : table->cells) cell="*";
            const char* actions[]={"*","GA_ZRCHIT","RA_ZRCHIT","AP_ZRCHIT","FA_10453","*"}; table->cells[0]=actions[row.action]; }
        tlk.SetToken(CGameSprite::TOKEN_SPECIALABILITY,"class-ability-sentinel");
        CCreatureFileHeader expected=b;
        if (row.mode==0) actor->ApplyNewFeatBonuses(previous,previousStats);
        else if (row.mode==1) actor->AddClassAbilities(static_cast<BYTE>(row.cls),static_cast<SHORT>(row.delta),2);
        else actor->AddNewSpecialAbilities(previousStats,2);

        UINT values[]={static_cast<WORD>(b.m_hitPoints),b.m_maxHitPointsBase,static_cast<WORD>(b.field_3E),b.m_numberOfAttacksBase,
            b.m_saveVSFortitudeBase,b.m_saveVSReflexBase,b.m_saveVSWillBase,static_cast<BYTE>(b.m_resistFireBase),static_cast<BYTE>(b.m_resistColdBase),
            static_cast<BYTE>(b.m_resistElectricityBase),static_cast<BYTE>(b.m_resistAcidBase),static_cast<BYTE>(b.m_resistMagicBase),
            static_cast<BYTE>(b.m_resistSlashingBase),static_cast<BYTE>(b.m_resistCrushingBase),static_cast<BYTE>(b.m_resistPiercingBase),static_cast<BYTE>(b.m_resistMissileBase)};
        check("native-stat-widths-thresholds-and-deltas",memcmp(values,row.stats,sizeof(values))==0);
        expected.m_hitPoints=static_cast<SHORT>(row.stats[0]); expected.m_maxHitPointsBase=static_cast<WORD>(row.stats[1]); expected.field_3E=static_cast<SHORT>(row.stats[2]);
        expected.m_numberOfAttacksBase=static_cast<BYTE>(row.stats[3]); expected.m_saveVSFortitudeBase=static_cast<BYTE>(row.stats[4]);
        expected.m_saveVSReflexBase=static_cast<BYTE>(row.stats[5]); expected.m_saveVSWillBase=static_cast<BYTE>(row.stats[6]);
        expected.m_resistFireBase=static_cast<signed char>(row.stats[7]); expected.m_resistColdBase=static_cast<signed char>(row.stats[8]);
        expected.m_resistElectricityBase=static_cast<signed char>(row.stats[9]); expected.m_resistAcidBase=static_cast<signed char>(row.stats[10]); expected.m_resistMagicBase=static_cast<signed char>(row.stats[11]);
        expected.m_resistSlashingBase=static_cast<signed char>(row.stats[12]); expected.m_resistCrushingBase=static_cast<signed char>(row.stats[13]);
        expected.m_resistPiercingBase=static_cast<signed char>(row.stats[14]); expected.m_resistMissileBase=static_cast<signed char>(row.stats[15]);
        memcpy(expected.m_feats,row.afterFeats,sizeof(expected.m_feats));
        check("native-complete-base-stats-mutation",memcmp(&b,&expected,sizeof(b))==0);
        UINT entries=0;
        for (UINT j=0;j<16;++j) {
            UINT at=0; auto* entry=actor->m_innateSpells.Find(j,at)?actor->m_innateSpells.Get(at):NULL;
            UINT count=row.grants[j]; if (row.removes && j==15) count=0;
            if (count) ++entries;
            check("native-ordered-grant-count",count?entry && entry->m_nMax==count && entry->m_nCurrent==count:entry==NULL);
        }
        check("native-grant-list-size",actor->m_innateSpells.m_List.size()==entries && actor->m_songs.m_List.empty() && actor->m_shapeshifts.m_List.empty());
        CString token; tlk.m_mapTokens.Lookup(CGameSprite::TOKEN_SPECIALABILITY,token);
        check("native-feedback-despite-non-TRUE-argument",token==(row.feedback?name.szText:CString(_T("class-ability-sentinel"))));
        check("native-AP-timed-delivery-count",actor->m_timedEffectList.GetCount()==timed+2*static_cast<INT>(row.ap) && actor->m_equipedEffectList.GetCount()==equipped);
        if (row.ap && actor->m_timedEffectList.GetCount()==timed+2) {
            POSITION pos=actor->m_timedEffectList.GetTailPosition();
            for (INT j=1;j>=0;--j) { auto* effect=actor->m_timedEffectList.GetPrev(pos);
                check("native-AP-provenance-and-immediate-resolution",effect->m_sourceRes=="ZRCHIT" && effect->m_durationType==6
                    && effect->m_duration==clock+(7+j)*15 && effect->m_effectAmount==j+2 && effect->m_sourceID==-1); }
        }
        while (actor->m_timedEffectList.GetCount()>timed) delete actor->m_timedEffectList.RemoveTail();
        while (actor->m_equipedEffectList.GetCount()>equipped) delete actor->m_equipedEffectList.RemoveTail();
        check("fixture-resource-ownership-balanced",resource->GetDemands()==demands && resource->m_nLockCount==requests && resource->m_pDimmKeyTableEntry->field_10==keyRefs);
    }
    for (auto* table : tables) delete table;
    for (INT i=0;i<3;++i) { catalogues[i]->m_pList=oldData[i]; catalogues[i]->m_nCount=oldCounts[i]; }
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp; actor->m_typeAI=type;
    actor->m_bAllowEffectListCall=allow; actor->m_bGlobal=global; actor->m_id=oldId;
    actor->m_innateSpells=innates; actor->m_songs=songs; actor->m_shapeshifts=shapes;
    size_t button=0; for (const auto& bank : banks) for (INT j=0;j<bank.n;++j) bank.p[j]=buttons[button++]; memcpy(actor->field_3D3A,cache,8);
    actor->field_562C=dirty; actor->field_70F6=saves[0]; actor->field_70F7=saves[1]; actor->field_70F8=saves[2]; actor->field_70F9=saves[3]; actor->field_70FA=saves[4];
    resource->m_pHeader=oldHeader; resource->m_pAbilities=oldAbilities; resource->m_pEffects=oldEffects;
    timer->m_active=active; if (hadToken) tlk.SetToken(CGameSprite::TOKEN_SPECIALABILITY,oldToken); else tlk.m_mapTokens.RemoveKey(CGameSprite::TOKEN_SPECIALABILITY);
    g_pBaldurChitin->pActiveEngine=oldEngine; g_pBaldurChitin->m_pEngineCharacter->m_nSelectedCharacter=selected;
    check("outer-tick-clock-and-message-queue-preserved",timer->m_gameTime==clock && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    guard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"classabilitychecks\",\"cases\":%u,\"checks\":%d,\"passed\":%s}",nStep,static_cast<UINT>(_countof(CLASS_ABILITIES)),checks,valid?"true":"false");
    if (!valid) Fail(nStep,"classabilitychecks","native class/feat mismatch");
}
