// AutoUI-only completion replay. All party state is restored before the next tick.
#include "ChargenCompletionGolden.h"

void ChargenCompletionChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<1) { Fail(nStep,"chargencompletionchecks","requires loaded party"); return; }
    LONG id=game->GetFixedOrderCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"chargencompletionchecks","party actor unavailable"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    CResHelper<CResText,1012> textGuard; textGuard.SetResRef(CResRef("RACESPAB"),FALSE,TRUE);
    CItem staffGuard(CResRef("00STAF01"),0,0,0,0,0),fistGuard(CResRef("ZRCMELEE"),1,0,0,0,0);
    BOOL staffLoaded=staffGuard.Demand(),fistLoaded=fistGuard.Demand();
    BOOL textLoaded=textGuard.pRes && textGuard.pRes->CRes::Demand()!=NULL;
    if (!staffLoaded || !fistLoaded || !textLoaded) {
        if (staffLoaded) staffGuard.Release(); if (fistLoaded) fistGuard.Release();
        if (textLoaded) textGuard.pRes->CRes::Release();
        objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"chargencompletionchecks","completion resources unavailable"); return;
    }
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1,cases=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"chargen-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto base=actor->m_baseStats; CDerivedStats derived,temp,empty; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto type=actor->m_typeAI,start=actor->m_startTypeAI; auto allow=actor->m_bAllowEffectListCall;
    auto global=actor->m_bGlobal; LONG oldId=actor->m_id;
    auto spells=actor->m_spells; auto domains=actor->m_domainSpells;
    auto innates=actor->m_innateSpells,songs=actor->m_songs,shapes=actor->m_shapeshifts;
    CItem* oldItems[51]; memcpy(oldItems,actor->m_equipment.m_items,sizeof(oldItems));
    auto selectedWeapon=actor->m_equipment.m_selectedWeapon;
    auto selectedAbility=actor->m_equipment.m_selectedWeaponAbility;
    auto weaponSet=actor->m_nWeaponSet;
    auto tempWeapon=actor->m_nTempSelectedWeapon,tempAbility=actor->m_nTempSelectedWeaponAbility;
    auto dirty=actor->field_562C; auto deferred=actor->field_72A2;
    auto inEquip=actor->field_7544,pendingEquip=actor->field_7540;
    auto* movement=actor->m_movementScript; actor->m_movementScript=NULL;
    CButtonData buttons[8]; for (INT i=0;i<8;++i) buttons[i]=actor->m_quickWeapons[i];
    BYTE cache[8]; memcpy(cache,actor->field_3D3A,8);
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount();
    INT timed=actor->m_timedEffectList.GetCount(),equipped=actor->m_equipedEffectList.GetCount();
    ULONG clock=game->GetWorldTimer()->m_gameTime;
    CSpellResRefList* catalogues[]={&game->m_innateSpells,&game->m_songs,&game->m_shapeshifts};
    CResRef* oldData[3]; UINT oldCounts[3]; CResRef refs[]={CResRef("ZRCHIT"),CResRef("SPIN231")};
    for (INT i=0;i<3;++i) { oldData[i]=catalogues[i]->m_pList; oldCounts[i]=catalogues[i]->m_nCount;
        catalogues[i]->m_pList=refs; catalogues[i]->m_nCount=i==0?2:0; }
    for (auto& ref : refs) check("grant-KEY-present",g_pChitin->cDimm.m_cKeyTable.FindKey(ref,1006,FALSE)!=NULL);
    auto* text=textGuard.pRes; CString oldText=text->m_string; BOOL oldParsed=text->m_bParsed;
    auto* staff=staffGuard.pRes; auto* fist=fistGuard.pRes;
    auto* oldStaffHeader=staff->m_pHeader; auto* oldStaffAbilities=staff->m_pAbilities;
    auto* oldFistHeader=fist->m_pHeader; auto* oldFistAbilities=fist->m_pAbilities;
    ITEM_HEADER itemHeader=*oldStaffHeader; ITEM_ABILITY itemAbility={};
    itemHeader.itemType=0; itemHeader.itemFlags=0; itemHeader.equipedEffectCount=0; itemHeader.abilityCount=1;
    itemHeader.genericName=-1; itemHeader.maxStackable=1;
    itemAbility.type=1; itemAbility.quickSlotType=1; itemAbility.actionType=1;
    staff->m_pHeader=fist->m_pHeader=&itemHeader; staff->m_pAbilities=fist->m_pAbilities=&itemAbility;
    INT textDemands=text->GetDemands(),staffDemands=staff->GetDemands(),fistDemands=fist->GetDemands();
    INT textRequests=text->m_nLockCount,staffRequests=staff->m_nLockCount,fistRequests=fist->m_nLockCount;
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    {
        ITEM_HEADER header=itemHeader; header.abilityCount=2;
        ITEM_ABILITY abilities[2]={}; abilities[1].type=2;
        CResRef originalRef=staffGuard.cResRef; BOOL parsed=staff->m_bParsed;
        staff->m_pHeader=&header; staff->m_pAbilities=abilities;
        for (const auto& row : ITEM_ABILITY_CASES) {
            abilities[0].type=static_cast<WORD>(row.type);
            for (INT i=1;i<row.demands;++i) staff->CRes::Demand();
            INT before=staff->GetDemands(); staff->m_bParsed=row.parsed;
            if (row.mode==1) staffGuard.cResRef="";
            if (row.mode==2) staffGuard.pRes=NULL;
            ITEM_ABILITY* actual=staffGuard.GetAbility(row.index);
            check("native-item-ability-pointer-and-low-byte-type",actual==(row.result<0?NULL:&abilities[row.result]));
            check("native-item-ability-does-not-demand",staff->GetDemands()==before);
            staffGuard.cResRef=originalRef; staffGuard.pRes=staff; staff->m_bParsed=parsed;
            for (INT i=1;i<row.demands;++i) staff->CRes::Release();
        }
        staff->m_pHeader=&itemHeader; staff->m_pAbilities=&itemAbility;
    }
    {
        ChargenTableFixture raceTable(rules.m_tRaceSpAb),goldTable(rules.m_tStartingGold),
            bardTable(rules.m_tMaxSpellsBard),sorcTable(rules.m_tMaxSpellsSorcerer),
            bonusTable(rules.m_tMaxSpellsBonus),reputationTable(rules.m_tReputationStart);
        goldTable.Gold(rules.GetClassString(CAIOBJECTTYPE_C_BARBARIAN,0));
        reputationTable.Set(CString(_T("12")));
        auto seedList=[](CGameSpriteSpellList& list,INT n,UINT seed) {
            list.Clear(); list.m_nSharedCurrent=seed; list.m_nSharedMax=9+seed; list.m_nSharedTotal=19+seed;
            for (INT j=0;j<n;++j) { CGameSpriteSpellListEntry entry={static_cast<UINT>(j),j==0?0:3+seed,j+seed,29+seed+j}; list.m_List.push_back(entry); }
        };
        auto hash=[&]() {
            DWORD value=2166136261u; auto word=[&](UINT n) { value=(value^n)*16777619u; };
            auto list=[&](const CGameSpriteSpellList& s) { word(s.m_nSharedCurrent); word(s.m_nSharedMax); word(s.m_nSharedTotal); word(static_cast<UINT>(s.m_List.size()));
                for (const auto& e : s.m_List) { word(e.m_nID); word(e.m_nMax); word(e.m_nCurrent); word(e.m_nShared); } };
            for (auto& c : actor->m_spells.m_spellsByClass) for (auto& l : c.m_lists) list(l);
            for (auto& l : actor->m_domainSpells.m_lists) list(l);
            list(actor->m_innateSpells); list(actor->m_songs); list(actor->m_shapeshifts); return value;
        };
        for (const auto& row : CHARGEN_CASES) {
            ++index;
            // Arbitrary receipt buttons exercise original ownership/cache branches.
            // The real fixed item request exposes at most one slot-43 button.
            if (row.mode==2 && (row.slot!=43 || row.buttons>1)) continue;
            ++cases;
            actor->m_baseStats=CCreatureFileHeader(); actor->m_baseStats.m_generalState=0x40000000;
            actor->m_baseStats.field_3E=-7; actor->m_baseStats.m_gold=row.gold;
            actor->m_derivedStats=empty; actor->m_tempStats=empty;
            actor->m_derivedStats.m_nLevel=2; actor->m_derivedStats.m_barbarianLevel=2;
            actor->m_derivedStats.m_bardLevel=actor->m_derivedStats.m_sorcererLevel=1;
            actor->m_derivedStats.m_classMask=CLASSMASK_BARBARIAN|CLASSMASK_BARD|CLASSMASK_SORCERER;
            actor->m_derivedStats.m_nCHR=13;
            actor->m_typeAI=type; actor->m_typeAI.m_nClass=static_cast<BYTE>(row.cls);
            actor->m_typeAI.m_nClassMask=0; actor->m_bAllowEffectListCall=FALSE;
            actor->m_bGlobal=TRUE; actor->m_id=-1;
            actor->m_startTypeAI=start; actor->m_startTypeAI.m_nRace=static_cast<BYTE>(row.race);
            actor->m_startTypeAI.m_nSubRace=static_cast<BYTE>(row.sub); actor->m_startTypeAI.m_nAlignment=0x12;
            actor->m_startTypeAI.m_nClassMask=CLASSMASK_BARD|CLASSMASK_SORCERER;
            for (UINT c=0;c<7;++c) for (UINT l=0;l<9;++l) seedList(*actor->m_spells.GetSpellsAtLevel(c,l),2,row.seed+l);
            for (UINT l=0;l<9;++l) seedList(*actor->m_domainSpells.GetSpellsAtLevel(l),2,row.seed+l);
            seedList(actor->m_innateSpells,row.seed?1:0,row.seed);
            if (row.seed) { actor->m_innateSpells.m_List[0].m_nMax=7; actor->m_innateSpells.m_List[0].m_nCurrent=2; }
            seedList(actor->m_songs,2,row.seed); seedList(actor->m_shapeshifts,2,row.seed);
            raceTable.Set(CString(row.table==0?_T("*"):_T("RACESPAB")));
            CString maximum; maximum.Format(_T("%d"),row.maximum); bardTable.Set(maximum); sorcTable.Set(maximum);
            bonusTable.Set(CString(_T("2")));
            text->m_bParsed=TRUE;
            text->m_string=row.table==1
                ? "2DA V1.0\n*\n SPELL\n0 ZRCHIT\n1 ZRCHIT\n2 SPIN231\n3 *\n4 MISSING\n5 ZRCHIT\n6 SPIN231\n"
                : "2DA V1.0\n*\n SPELL COUNT\n0 ZRCHIT 1\n1 ZRCHIT 1\n2 SPIN231 2\n3 * 3\n4 MISSING 1\n5 ZRCHIT 0\n6 SPIN231 1\n";
            if (row.mode==0) {
                actor->AddRacialAbilities();
                for (UINT j=0;j<2;++j) { UINT at=0; auto* entry=actor->m_innateSpells.Find(j,at)?actor->m_innateSpells.Get(at):NULL;
                    UINT expected=(j==0 && row.seed?7:0)+row.grants[j];
                    check("native-racial-dedup-and-repeat-count",expected?entry && entry->m_nMax==expected:entry==NULL); }
            } else if (row.mode==1) {
                g_pBaldurChitin->m_pEngineCreateChar->CompleteCharacterWrapup(actor);
                check("native-spell-refill-and-untouched-fields",hash()==row.hash);
                check("native-movement-script-and-reputation",actor->m_movementScript && actor->m_movementScript->m_cResRef=="DPLAYER3" && actor->m_baseStats.m_reputation==120);
                delete actor->m_movementScript; actor->m_movementScript=NULL;
            } else {
                for (auto& p : actor->m_equipment.m_items) p=NULL;
                auto* testFist=new CItem(CResRef("ZRCMELEE"),1,0,0,0,0);
                actor->m_equipment.m_items[10]=testFist;
                actor->m_equipment.m_items[0]=new CItem(CResRef("ZRCMELEE"),1,0,0,0,0);
                actor->m_equipment.m_items[50]=new CItem(CResRef("ZRCMELEE"),1,0,0,0,0);
                actor->m_equipment.m_selectedWeapon=10; actor->m_equipment.m_selectedWeaponAbility=0;
                actor->field_7544=actor->field_7540=0;
                actor->m_nWeaponSet=0; actor->field_3D3A[0]=99;
                actor->m_quickWeapons[0]=buttons[0]; CButtonData before=actor->m_quickWeapons[0];
                itemAbility.quickSlotType=row.buttons?1:0;
                SHORT calls=actor->field_72A2;
                g_pBaldurChitin->m_pEngineCreateChar->CompleteCharacterSkills(actor);
                check("native-initial-item-and-preserved-fist",actor->m_equipment.m_items[10]==testFist && (actor->m_equipment.m_items[43]!=NULL)==(row.cls!=6));
                check("native-quick-ammo-cache",actor->field_3D3A[0]==row.cache);
                if (row.cls!=6 && row.buttons) check("initial-weapon-button",actor->m_quickWeapons[0].m_abilityId.m_itemNum==43 && actor->m_quickWeapons[0].m_abilityId.m_abilityNum==0);
                else check("empty-list-or-monk-keeps-button",memcmp(&actor->m_quickWeapons[0],&before,sizeof(before))==0);
                check("effect-pass-requested",actor->field_562C==1 && actor->field_72A2==static_cast<SHORT>(calls+1));
                for (INT slot=0;slot<51;++slot) { if (slot!=10 && slot!=43) check("old-items-removed",actor->m_equipment.m_items[slot]==NULL); }
                delete actor->m_equipment.m_items[43]; actor->m_equipment.m_items[43]=NULL;
                delete testFist; actor->m_equipment.m_items[10]=NULL;
            }
            check("native-racial-state-and-WORD-value",actor->m_baseStats.m_generalState==row.state && actor->m_baseStats.field_3E==row.field3e);
            check("native-starting-or-preserved-gold",actor->m_baseStats.m_gold==row.afterGold);
            check("no-effects-created",actor->m_timedEffectList.GetCount()==timed && actor->m_equipedEffectList.GetCount()==equipped);
            while (g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()>messages) delete g_pBaldurChitin->GetMessageHandler()->m_messageList.RemoveTail();
            check("resource-demands-balanced",text->GetDemands()==textDemands && staff->GetDemands()==staffDemands && fist->GetDemands()==fistDemands);
            // Native C2DArray::Load requests twice; its helper destructor cancels once.
            // Retire that extra request here so the disposable replay has no residue.
            if (row.mode==0 && row.table!=0) {
                check("native-table-request-residue",text->m_nLockCount==textRequests+1);
                text->CancelRequest();
            }
        }
    }
    for (INT i=0;i<3;++i) { catalogues[i]->m_pList=oldData[i]; catalogues[i]->m_nCount=oldCounts[i]; }
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp;
    actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_bAllowEffectListCall=allow;
    actor->m_bGlobal=global; actor->m_id=oldId; actor->m_spells=spells; actor->m_domainSpells=domains;
    actor->m_innateSpells=innates; actor->m_songs=songs; actor->m_shapeshifts=shapes;
    memcpy(actor->m_equipment.m_items,oldItems,sizeof(oldItems));
    actor->m_equipment.m_selectedWeapon=selectedWeapon; actor->m_equipment.m_selectedWeaponAbility=selectedAbility;
    actor->m_nWeaponSet=weaponSet;
    actor->m_nTempSelectedWeapon=tempWeapon; actor->m_nTempSelectedWeaponAbility=tempAbility;
    actor->m_movementScript=movement; actor->field_562C=dirty; actor->field_72A2=deferred;
    actor->field_7540=pendingEquip; actor->field_7544=inEquip;
    for (INT i=0;i<8;++i) actor->m_quickWeapons[i]=buttons[i]; memcpy(actor->field_3D3A,cache,8);
    text->m_string=oldText; text->m_bParsed=oldParsed;
    staff->m_pHeader=oldStaffHeader; staff->m_pAbilities=oldStaffAbilities;
    fist->m_pHeader=oldFistHeader; fist->m_pAbilities=oldFistAbilities;
    check("resource-requests-balanced",text->m_nLockCount==textRequests && staff->m_nLockCount==staffRequests && fist->m_nLockCount==fistRequests);
    check("same-tick-restoration",game->GetWorldTimer()->m_gameTime==clock && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    text->CRes::Release(); staffGuard.Release(); fistGuard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"chargencompletionchecks\",\"nativeCases\":%u,\"replayed\":%d,\"itemAbilityCases\":%u,\"checks\":%d,\"passed\":%s}",nStep,static_cast<UINT>(_countof(CHARGEN_CASES)),cases,static_cast<UINT>(_countof(ITEM_ABILITY_CASES)),checks,valid?"true":"false");
    if (!valid) Fail(nStep,"chargencompletionchecks","native completion mismatch");
}
