// AutoUI-only LevelUp replay. No save is written; restoration completes this tick.
#include "LevelUpGolden.h"

void LevelUpChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<1) { Fail(nStep,"levelupchecks","requires party"); return; }
    LONG id=game->GetFixedOrderCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"levelupchecks","actor unavailable"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object); auto* screen=g_pBaldurChitin->m_pEngineCharacter;
    CSpell rageGuard(CResRef("SPIN260")),domainGuard(CResRef("ZRCHIT"));
    BOOL rageLoaded=rageGuard.Demand(),domainLoaded=domainGuard.Demand();
    if (!rageLoaded || !domainLoaded) {
        if (rageLoaded) rageGuard.Release(); if (domainLoaded) domainGuard.Release();
        objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"levelupchecks","spell fixtures unavailable"); return;
    }
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"levelup-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto base=actor->m_baseStats; CDerivedStats derived,temp,empty; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto type=actor->m_typeAI,start=actor->m_startTypeAI; auto allow=actor->m_bAllowEffectListCall;
    auto global=actor->m_bGlobal; LONG oldId=actor->m_id; auto dirty=actor->field_562C; auto deferred=actor->field_72A2;
    auto spells=actor->m_spells; auto domains=actor->m_domainSpells;
    auto innates=actor->m_innateSpells,songs=actor->m_songs,shapes=actor->m_shapeshifts;
    CItem* oldItems[51]; memcpy(oldItems,actor->m_equipment.m_items,sizeof(oldItems));
    auto selectedWeapon=actor->m_equipment.m_selectedWeapon; auto selectedAbility=actor->m_equipment.m_selectedWeaponAbility;
    auto weaponSet=actor->m_nWeaponSet; auto inEquip=actor->field_7544,pendingEquip=actor->field_7540;
    auto tempWeapon=actor->m_nTempSelectedWeapon,tempAbility=actor->m_nTempSelectedWeaponAbility;
    auto* animation=actor->m_animation.m_animation;
    auto* oldEngine=g_pBaldurChitin->pActiveEngine; auto selected=screen->m_nSelectedCharacter;
    auto cls=screen->m_nClass; auto spec=screen->m_nSpecialization; auto points=screen->m_nExtraSkillPoints;
    auto* tempBase=screen->m_pTempBaseStats; auto* tempDerived=screen->m_pTempDerivedStats;
    auto* beforeDerived=screen->m_pBeforeLevelUpDerivedStats; auto field866=screen->field_866,field86A=screen->field_86A;
    struct Bank { CButtonData* p; INT n; }; Bank banks[]={{actor->m_quickWeapons,8},{actor->m_quickSpells,9},
        {actor->m_quickItems,3},{actor->m_quickInnates,9},{actor->m_quickSongs,9}};
    std::vector<CButtonData> buttons; for (const auto& bank : banks) for (INT j=0;j<bank.n;++j) buttons.push_back(bank.p[j]);
    BYTE cache[8]; memcpy(cache,actor->field_3D3A,8);
    CSpellResRefList* catalogues[]={&game->m_spells,&game->m_innateSpells,&game->m_songs,&game->m_shapeshifts};
    CResRef* oldData[4]; UINT oldCounts[4]; CResRef refs[]={CResRef("ZRCHIT"),CResRef("ZRCHIT2")};
    CResRef innateRefs[]={CResRef("SPIN236"),CResRef("SPIN260")};
    for (INT i=0;i<4;++i) { oldData[i]=catalogues[i]->m_pList; oldCounts[i]=catalogues[i]->m_nCount;
        catalogues[i]->m_pList=i==1?innateRefs:refs; catalogues[i]->m_nCount=i<2?2:0; }
    CSpellListEntry* domainData[9][9]; UINT domainCounts[9][9]; CSpellListEntry domainEntry={0,0};
    for (INT d=0;d<9;++d) for (INT l=0;l<9;++l) {
        auto& list=game->m_spellsByDomain[d].m_lists[l]; domainData[d][l]=list.m_pList; domainCounts[d][l]=list.m_nCount;
        list.m_pList=&domainEntry; list.m_nCount=l==0?1:0;
    }
    auto* rage=rageGuard.pRes; auto* spell=domainGuard.pRes;
    auto* rageHeader=rage->m_pHeader; auto* rageAbilities=rage->m_pAbilities; auto* spellHeader=spell->m_pHeader;
    SPELL_HEADER rageFixture=*rageHeader,spellFixture=*spellHeader; SPELL_ABILITY abilities[2]={};
    rageFixture.genericName=10453; rageFixture.itemType=4; rageFixture.abilityCount=2;
    for (INT j=0;j<2;++j) { abilities[j].minCasterLevel=j?5:1; abilities[j].quickSlotType=4;
        abilities[j].actionType=j?9:7; memcpy(abilities[j].quickSlotIcon,"RAGEICON",8); }
    rage->m_pHeader=&rageFixture; rage->m_pAbilities=abilities; spell->m_pHeader=&spellFixture;
    INT rageDemands=rage->GetDemands(),rageRequests=rage->m_nLockCount;
    INT spellDemands=spell->GetDemands(),spellRequests=spell->m_nLockCount;
    auto* timer=game->GetWorldTimer(); auto active=timer->m_active; ULONG clock=timer->m_gameTime; timer->m_active=TRUE;
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    INT timed=actor->m_timedEffectList.GetCount(),equipped=actor->m_equipedEffectList.GetCount();
    g_pBaldurChitin->pActiveEngine=screen; screen->m_nSelectedCharacter=0;
    actor->m_animation.m_animation=NULL; for (auto& item : actor->m_equipment.m_items) item=NULL;
    actor->m_equipment.m_selectedWeapon=10; actor->m_equipment.m_selectedWeaponAbility=0;
    actor->m_nWeaponSet=0; actor->field_7544=actor->field_7540=0;
    auto seedList=[](CGameSpriteSpellList& list,INT n) {
        list.Clear(); list.m_nSharedCurrent=2; list.m_nSharedMax=9; list.m_nSharedTotal=19;
        for (INT j=0;j<n;++j) { CGameSpriteSpellListEntry e={static_cast<UINT>(j),static_cast<UINT>(7+j),static_cast<UINT>(2+j),static_cast<UINT>(12+j)}; list.m_List.push_back(e); }
    };
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    {
        LevelUpTableFixture bard(rules.m_tMaxSpellsBard),sorc(rules.m_tMaxSpellsSorcerer),
            bonus(rules.m_tMaxSpellsBonus),shapeTable(rules.m_tMaxDruidShapeshifts),cleric(rules.m_tMaxSpellsCleric);
        bonus.Set(0,0);
        for (const auto& row : LEVELUP_CASES) {
            ++index;
            if (index%24==0) Emit("{\"step\":%d,\"op\":\"levelup-progress\",\"row\":%d,\"class\":%d}",nStep,index,row.cls);
            actor->m_baseStats=CCreatureFileHeader(); actor->m_baseStats.m_specialization=row.spec;
            actor->m_baseStats.m_specFlags=static_cast<BYTE>(row.flags); actor->m_baseStats.m_resistElectricityBase=126;
            actor->m_baseStats.m_druidLevel=1;
            actor->m_derivedStats=empty; actor->m_derivedStats.m_nClassLevels[row.cls-1]=10;
            actor->m_derivedStats.m_monkLevel=row.cls==6?10:1; actor->m_derivedStats.m_paladinLevel=row.cls==7?10:1;
            actor->m_derivedStats.m_nLevel=static_cast<BYTE>(row.rage?row.caster:12);
            if (row.rage) actor->m_derivedStats.m_barbarianLevel=15;
            actor->m_derivedStats.m_nCHR=13; actor->m_derivedStats.m_classMask=0x7FF;
            actor->m_tempStats=actor->m_derivedStats; actor->m_tempStats.m_disabledSpellTypes[2]=0x102;
            actor->m_typeAI=type; actor->m_typeAI.m_nClass=static_cast<BYTE>(row.cls);
            actor->m_typeAI.m_nAlignment=static_cast<BYTE>(row.alignment);
            actor->m_typeAI.m_nClassMask=row.cls==2?CLASSMASK_BARD:row.cls==10?CLASSMASK_SORCERER:0;
            actor->m_startTypeAI=start; actor->m_startTypeAI.m_nClassMask=0;
            actor->m_startTypeAI.m_nAlignment=static_cast<BYTE>(row.alignment);
            actor->m_bAllowEffectListCall=FALSE; actor->m_bGlobal=TRUE; actor->m_id=-1;
            screen->m_nClass=static_cast<BYTE>(row.cls); screen->m_nSpecialization=row.spec; screen->m_nExtraSkillPoints=row.points;
            screen->m_pTempBaseStats=new CCreatureFileHeader(actor->m_baseStats);
            screen->m_pTempDerivedStats=new CDerivedStats(); *screen->m_pTempDerivedStats=actor->m_derivedStats;
            if (row.cls==2 || row.cls==10 || row.cls==4) screen->m_pTempDerivedStats->m_nClassLevels[row.cls-1]=1;
            screen->m_pBeforeLevelUpDerivedStats=new CDerivedStats(); *screen->m_pBeforeLevelUpDerivedStats=actor->m_derivedStats;
            screen->m_pBeforeLevelUpDerivedStats->m_clericLevel=static_cast<BYTE>(row.oldCleric);
            screen->field_866=77; screen->field_86A=88;
            WORD bits[]={0,0x100,0x300,0,0,0x100,0x100,0x100,0x100,0x300,0x200,0x200};
            actor->m_baseStats.m_animationType=static_cast<SHORT>(row.rage?0x6100:0x6000|bits[row.cls]);
            actor->m_baseStats.field_22=static_cast<SHORT>(row.high);
            for (auto& bank : banks) for (INT j=0;j<bank.n;++j) bank.p[j]=CButtonData();
            for (UINT c=0;c<7;++c) for (UINT l=0;l<9;++l) seedList(*actor->m_spells.GetSpellsAtLevel(c,l),row.seed);
            actor->m_domainSpells.Clear(); actor->m_innateSpells.Clear(); actor->m_songs.Clear(); seedList(actor->m_shapeshifts,2);
            if (row.rage) {
                CGameSpriteSpellListEntry old={0,65536u,static_cast<UINT>(row.rage-1),12}; actor->m_innateSpells.m_List.push_back(old);
                for (INT j=0;j<9;++j) actor->m_quickInnates[j].m_abilityId.m_res=j%2?CResRef("OTHER"):CGameSprite::SPIN236;
            }
            bard.Set(row.oldMax,row.newMax); sorc.Set(row.oldMax,row.newMax); shapeTable.Set(row.oldMax,row.newMax);
            cleric.Set(0,0,row.domainFlag>=0); spellFixture.notUsableBy=static_cast<DWORD>(row.domainFlag);
            auto* table=const_cast<C2DArray*>(rules.GetClassAbilityTable(static_cast<BYTE>(row.cls),row.spec));
            ClassAbilityTableFixture* quiet=table?new ClassAbilityTableFixture(*table):NULL;
            SHORT calls=actor->field_72A2;
            screen->LevelUp(actor);
            delete quiet;
            check("native-points-and-multiclass-flags",actor->m_baseStats.field_2FF==row.afterPoints && actor->m_baseStats.m_specFlags==row.afterFlags);
            check("native-first-cleric-byte-resistance",static_cast<BYTE>(actor->m_baseStats.m_resistElectricityBase)==row.electric);
            DWORD actualAnimation=static_cast<WORD>(actor->m_baseStats.m_animationType)|(static_cast<DWORD>(static_cast<WORD>(actor->m_baseStats.field_22))<<16);
            check("native-animation-WORD-replacement-clears-high-WORD",actualAnimation==row.animation);
            check("temporary-snapshots-retired",!screen->m_pTempBaseStats && !screen->m_pTempDerivedStats && !screen->m_pBeforeLevelUpDerivedStats && !screen->field_866 && !screen->field_86A);
            check("effect-processing-requested",actor->field_562C==1 && actor->field_72A2==static_cast<SHORT>(calls+1));
            DWORD hash=2166136261u; auto word=[&](UINT n) { hash=(hash^n)*16777619u; };
            for (UINT l=0;l<9;++l) {
                auto* list=actor->m_spells.GetSpellsAtLevel(row.cls==2?0:5,l);
                word(list->m_nSharedCurrent); word(list->m_nSharedMax); word(list->m_nSharedTotal);
                for (const auto& entry : list->m_List) { word(entry.m_nID); word(entry.m_nMax); word(entry.m_nCurrent); word(entry.m_nShared); }
            }
            check("native-all-nine-spell-levels",hash==row.spellHash);
            UINT shapeState[11]={actor->m_shapeshifts.m_nSharedCurrent,actor->m_shapeshifts.m_nSharedMax,actor->m_shapeshifts.m_nSharedTotal};
            for (INT j=0;j<2;++j) { auto* entry=actor->m_shapeshifts.Get(j); shapeState[3+4*j]=entry->m_nID; shapeState[4+4*j]=entry->m_nMax; shapeState[5+4*j]=entry->m_nCurrent; shapeState[6+4*j]=entry->m_nShared; }
            check("native-shapeshift-delta-only-if-positive",memcmp(shapeState,row.shape,sizeof(shapeState))==0);
            check("native-domain-alignment-and-class-flag",actor->m_domainSpells.m_lists[0].m_List.size()==static_cast<size_t>(row.domain));
            if (row.rage) {
                const auto& button=actor->m_quickInnates[0];
                check("native-rage-button-payload",button.m_count==row.rageButton[0] && button.m_bDisabled==row.rageButton[1]
                    && button.m_abilityId.m_targetType==row.rageButton[2] && button.m_abilityId.m_itemType==row.rageButton[3]
                    && button.m_abilityId.m_res==CGameSprite::SPIN260 && button.m_icon=="RAGEICON");
                UINT at=0; check("rage-uses-transferred-and-old-grant-retired",!actor->m_innateSpells.Find(0,at) && actor->m_innateSpells.Find(1,at)
                    && actor->m_innateSpells.Get(at)->m_nCurrent==static_cast<UINT>(row.rage-1));
                for (INT j=0;j<9;++j) check("all-nine-innate-shortcuts",actor->m_quickInnates[j].m_abilityId.m_res==(j%2?CResRef("OTHER"):CGameSprite::SPIN260));
            }
            delete actor->m_animation.m_animation; actor->m_animation.m_animation=NULL;
            check("no-effects-created",actor->m_timedEffectList.GetCount()==timed && actor->m_equipedEffectList.GetCount()==equipped);
            while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail();
            check("spell-resource-demands-and-requests-balanced",rage->GetDemands()==rageDemands && rage->m_nLockCount==rageRequests && spell->GetDemands()==spellDemands && spell->m_nLockCount==spellRequests);
        }
    }
    {
        CItem itemGuard(CResRef("ZRCMELEE"),1,0,0,0,0);
        BOOL loaded=itemGuard.Demand(); check("drop-item-resource-loaded",loaded);
        if (loaded) {
            auto* itemResource=itemGuard.pRes; auto* originalHeader=itemResource->m_pHeader;
            ITEM_HEADER header=*originalHeader; itemResource->m_pHeader=&header;
            header.equipedEffectCount=0; header.itemType=20; header.maxStackable=1;
            header.minLevelRequired=0; header.itemFlags=4;
            actor->m_id=oldId; actor->m_typeAI.m_nAlignment=0x11; actor->m_typeAI.m_nClass=5;
            actor->m_typeAI.m_nClassMask=CLASSMASK_FIGHTER; actor->m_derivedStats=empty;
            actor->m_derivedStats.m_nLevel=1; actor->m_derivedStats.m_fighterLevel=1;
            actor->m_derivedStats.m_classMask=CLASSMASK_FIGHTER; actor->m_tempStats=actor->m_derivedStats;
            actor->m_animation.m_animation=CGameAnimationType::SetAnimationType(0x6100,actor->m_baseStats.m_colors,0);
            INT demands=itemResource->GetDemands();
            for (INT slot : {0,50}) for (INT mode=0;mode<3;++mode) {
                ++index;
                for (auto& item : actor->m_equipment.m_items) item=NULL;
                auto* fist=new CItem(CResRef("ZRCMELEE"),1,0,0,0,0);
                auto* item=new CItem(CResRef("ZRCMELEE"),1,0,0,0,0);
                actor->m_equipment.m_items[10]=fist; actor->m_equipment.m_items[slot]=item;
                actor->m_equipment.m_selectedWeapon=static_cast<BYTE>(slot==50?50:10); actor->m_equipment.m_selectedWeaponAbility=0;
                actor->m_nWeaponSet=0; actor->field_7544=actor->field_7540=0;
                header.notUsableBy=mode?0x4000:0; header.itemFlags=mode==1?0x14:4;
                actor->m_quickWeapons[7].m_name=123; actor->field_3D3A[7]=99;
                STRREF error=-1;
                check("drop-real-usability-boundary",game->CanCharacterUseItem(0,item,error,TRUE)==(mode==0));
                screen->CheckDropSlot(slot);
                check("drop-keeps-usable-or-cursed-and-moves-invalid",actor->m_equipment.m_items[slot]==(mode==2?NULL:item)
                    && actor->m_equipment.m_items[18]==(mode==2?item:NULL));
                if (slot==50 && mode) check("drop-clears-weapon-shortcut-before-swap",actor->m_quickWeapons[7].m_name==-1 && actor->field_3D3A[7]==0);
                for (auto& entry : actor->m_equipment.m_items) entry=NULL;
                delete item; delete fist;
                while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail();
                check("drop-resource-demand-balanced",itemResource->GetDemands()==demands);
            }
            delete actor->m_animation.m_animation; actor->m_animation.m_animation=NULL;
            itemResource->m_pHeader=originalHeader; itemGuard.Release();
        }
    }
    {
        // Chargen stats: cleric domain skill floors; an invalid sprite id skips the appearance refresh.
        CScreenCreateChar create; create.m_nGameSprite=CGameObjectArray::INVALID_INDEX;
        BeginLevelUpTableFixture skillTable(rules.m_tSkills,24,3);
        for (auto& cell : skillTable.cells) cell="0";
        skillTable.cells[0*24+15]="9"; skillTable.cells[0*24+16]="4"; skillTable.cells[1*24+16]="3";
        struct StatsCase { BYTE cls; DWORD spec; BYTE gender,race; BYTE skills[3]; WORD animation; };
        const StatsCase statsCases[]={
            {CAIOBJECTTYPE_C_CLERIC,SPECMASK_CLERIC_LATHANDER,CAIOBJECTTYPE_SEX_MALE,CAIOBJECTTYPE_R_HUMAN,{4,6,0},0x6000},
            {CAIOBJECTTYPE_C_CLERIC,SPECMASK_CLERIC_ILMATER,CAIOBJECTTYPE_SEX_FEMALE,CAIOBJECTTYPE_R_DWARF,{9,6,0},0x6012},
            {CAIOBJECTTYPE_C_WIZARD,0,CAIOBJECTTYPE_SEX_MALE,CAIOBJECTTYPE_R_ELF,{1,6,0},0x6201},
        };
        for (const auto& row : statsCases) {
            ++index;
            actor->m_baseStats=CCreatureFileHeader(); actor->m_baseStats.m_specialization=row.spec;
            actor->m_baseStats.m_skills[0]=1; actor->m_baseStats.m_skills[1]=6; actor->m_baseStats.m_skills[2]=0;
            actor->m_derivedStats=empty; actor->m_tempStats=empty;
            actor->m_typeAI=type; actor->m_typeAI.m_nClass=row.cls; actor->m_typeAI.m_nGender=row.gender;
            actor->m_typeAI.m_nRace=row.race; actor->m_typeAI.m_nSubRace=0; actor->m_startTypeAI=actor->m_typeAI;
            actor->field_562C=0;
            create.UpdateCharacterStats(actor);
            const auto& stats=actor->m_baseStats;
            check("chargen-stats-domain-skill-floors",stats.m_skills[0]==row.skills[0] && stats.m_skills[1]==row.skills[1]
                && stats.m_skills[2]==row.skills[2]);
            check("chargen-stats-animation-id",static_cast<WORD>(stats.m_animationType)==row.animation && actor->m_animation.m_animation!=NULL);
            check("chargen-stats-base-levels-copied",stats.m_clericLevel==actor->m_derivedStats.m_clericLevel
                && stats.m_wizardLevel==actor->m_derivedStats.m_wizardLevel && stats.m_characterLevel==actor->m_derivedStats.m_nLevel);
            check("chargen-stats-effect-processing-requested",actor->field_562C==1);
            delete actor->m_animation.m_animation; actor->m_animation.m_animation=NULL;
            while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail();
        }
    }
    for (INT d=0;d<9;++d) for (INT l=0;l<9;++l) { game->m_spellsByDomain[d].m_lists[l].m_pList=domainData[d][l]; game->m_spellsByDomain[d].m_lists[l].m_nCount=domainCounts[d][l]; }
    for (INT i=0;i<4;++i) { catalogues[i]->m_pList=oldData[i]; catalogues[i]->m_nCount=oldCounts[i]; }
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp; actor->m_typeAI=type; actor->m_startTypeAI=start;
    actor->m_bAllowEffectListCall=allow; actor->m_bGlobal=global; actor->m_id=oldId; actor->field_562C=dirty; actor->field_72A2=deferred;
    actor->m_spells=spells; actor->m_domainSpells=domains; actor->m_innateSpells=innates; actor->m_songs=songs; actor->m_shapeshifts=shapes;
    actor->m_animation.m_animation=animation; memcpy(actor->m_equipment.m_items,oldItems,sizeof(oldItems));
    actor->m_equipment.m_selectedWeapon=selectedWeapon; actor->m_equipment.m_selectedWeaponAbility=selectedAbility;
    actor->m_nWeaponSet=weaponSet; actor->field_7544=inEquip; actor->field_7540=pendingEquip;
    actor->m_nTempSelectedWeapon=tempWeapon; actor->m_nTempSelectedWeaponAbility=tempAbility;
    size_t b=0; for (auto& bank : banks) for (INT j=0;j<bank.n;++j) bank.p[j]=buttons[b++]; memcpy(actor->field_3D3A,cache,8);
    screen->m_nClass=cls; screen->m_nSpecialization=spec; screen->m_nExtraSkillPoints=points;
    screen->m_pTempBaseStats=tempBase; screen->m_pTempDerivedStats=tempDerived; screen->m_pBeforeLevelUpDerivedStats=beforeDerived;
    screen->field_866=field866; screen->field_86A=field86A; screen->m_nSelectedCharacter=selected;
    g_pBaldurChitin->pActiveEngine=oldEngine; timer->m_active=active;
    rage->m_pHeader=rageHeader; rage->m_pAbilities=rageAbilities; spell->m_pHeader=spellHeader;
    check("outer-tick-clock-and-queue-restored",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    rageGuard.Release(); domainGuard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"levelupchecks\",\"nativeCases\":%u,\"checks\":%d,\"passed\":%s}",nStep,static_cast<UINT>(_countof(LEVELUP_CASES)),checks,valid?"true":"false");
    if (!valid) Fail(nStep,"levelupchecks","native LevelUp mismatch");
}
