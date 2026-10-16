// AutoUI-only typed replay of native special-ability mutations and call ABI.
#include "SpecialAbilityGolden.h"

void SpecialAbilityChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<1) { Fail(nStep,"specialabilitychecks","requires loaded party"); return; }
    LONG id=game->GetFixedOrderCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"specialabilitychecks","party actor unavailable"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    CSpell guard(CResRef("ZRCHIT")); BOOL demanded=guard.Demand();
    if (!demanded || !guard.pRes || !guard.pRes->m_bParsed) {
        if (demanded) guard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"specialabilitychecks","requires pinned fixture SPL"); return;
    }
    INT checks=0,index=-1,replayed=0,resourceFailures=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"special-ability-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    auto* resource=guard.pRes; auto* oldHeader=resource->m_pHeader; SPELL_HEADER header=*oldHeader; header.genericName=10453;
    resource->m_pHeader=&header;
    INT demands=resource->GetDemands(),requests=resource->m_nLockCount,refs=resource->m_pDimmKeyTableEntry->field_10;
    auto savedInnate=actor->m_innateSpells,savedSongs=actor->m_songs,savedShapes=actor->m_shapeshifts;
    CGameSpriteSpellList* owned[]={&actor->m_innateSpells,&actor->m_songs,&actor->m_shapeshifts};
    CSpellResRefList* catalogues[]={&game->m_innateSpells,&game->m_songs,&game->m_shapeshifts};
    CResRef* oldData[3]; UINT oldCounts[3]; CResRef data[3][4]; UINT ids[]={3,1,2};
    for (INT i=0;i<3;++i) { oldData[i]=catalogues[i]->m_pList; oldCounts[i]=catalogues[i]->m_nCount; }
    struct Bank { CButtonData* p; INT count; }; Bank banks[]={ {actor->m_quickWeapons,8},{actor->m_quickSpells,9},
        {actor->m_quickItems,3},{actor->m_quickInnates,9},{actor->m_quickSongs,9} };
    std::vector<CButtonData*> buttons; for (const auto& bank : banks) for (INT j=0;j<bank.count;++j) buttons.push_back(bank.p+j);
    auto snapshot=[&]() { std::vector<CButtonData> result; for (const auto* p : buttons) result.push_back(*p); return result; };
    auto restore=[&](const std::vector<CButtonData>& values) { for (size_t j=0;j<buttons.size();++j) *buttons[j]=values[j]; };
    auto same=[&](const std::vector<CButtonData>& values) { BOOL ok=TRUE;
        for (size_t j=0;j<buttons.size();++j) if (memcmp(buttons[j],&values[j],sizeof(CButtonData))!=0) ok=FALSE; return ok; };
    auto savedButtons=snapshot(); BYTE savedCache[8]; memcpy(savedCache,actor->field_3D3A,8);
    auto oldGlobal=actor->m_bGlobal; LONG oldId=actor->m_id; actor->m_id=-1; actor->m_bGlobal=TRUE;
    auto* oldEngine=g_pBaldurChitin->pActiveEngine;
    auto selected=g_pBaldurChitin->m_pEngineCharacter->m_nSelectedCharacter;
    // Feedback still writes its real TLK token and world text. The character
    // engine suppresses GUI audio; the engine pointer is restored in this tick.
    g_pBaldurChitin->pActiveEngine=g_pBaldurChitin->m_pEngineCharacter;
    auto& tlk=g_pBaldurChitin->GetTlkTable(); CString oldToken;
    BOOL hadToken=tlk.m_mapTokens.Lookup(CGameSprite::TOKEN_SPECIALABILITY,oldToken);
    STR_RES name; tlk.Fetch(10453,name);
    INT messages=g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount(); ULONG clock=game->GetWorldTimer()->m_gameTime;
    check("missing-key-fixture-absent",g_pChitin->cDimm.m_cKeyTable.FindKey(CResRef("ZRNOABIL"),1006,FALSE)==NULL);
    check("native-shapeshift-catalogue-identity",game->GetShapeshifts()==&game->m_shapeshifts && game->GetShapeshifts()!=game->GetInnateSpells());
    {
        alignas(4) BYTE bytes[8]; memset(bytes,0xCC,sizeof(bytes));
        auto* timer=new (bytes) CTimerWorld();
        check("native-six-byte-timer-and-constructor",sizeof(CTimerWorld)==6 && memcmp(bytes,SPECIAL_TIMER_BYTES,sizeof(bytes))==0);
        timer->~CTimerWorld();
        check("native-embedded-save-span",sizeof(CGameSave)==SPECIAL_SAVE_SPAN);
    }
    for (const auto& row : SPECIAL_ABILITIES) {
        ++index;
        if (!row.resource) { ++resourceFailures; continue; }
        ++replayed; CResRef query(row.key?"ZRCHIT":"ZRNOABIL");
        for (INT i=0;i<3;++i) {
            for (INT j=0;j<4;++j) { char other[9]; sprintf_s(other,"ZZCAT%d%d",i,j); data[i][j]=other; }
            if (row.mask&(1<<i)) data[i][ids[i]]=query;
            catalogues[i]->m_pList=data[i]; catalogues[i]->m_nCount=4;
            auto* list=owned[i]; list->m_List.clear();
            if (row.seed) {
                CGameSpriteSpellListEntry first={ids[i],4,2,9}; list->m_List.push_back(first);
                CGameSpriteSpellListEntry second={row.seed==1?99u:ids[i],row.seed==1?7u:6u,row.seed==1?3u:1u,row.seed==1?8u:7u}; list->m_List.push_back(second);
            }
            list->m_nSharedCurrent=row.seed==0?0:row.seed==1?5:3; list->m_nSharedMax=17; list->m_nSharedTotal=19;
        }
        tlk.SetToken(CGameSprite::TOKEN_SPECIALABILITY,"special-ability-sentinel");
        if (row.op==0) {
            auto before=snapshot(); BYTE cache[8]; memcpy(cache,actor->field_3D3A,8);
            actor->AddSpecialAbility(query,row.feedback);
            check("native-add-does-not-update-shortcuts",same(before) && memcmp(cache,actor->field_3D3A,8)==0);
            CString token; tlk.m_mapTokens.Lookup(CGameSprite::TOKEN_SPECIALABILITY,token);
            check("native-exact-TRUE-feedback-gate",token==(row.feedbackId?name.szText:CString(_T("special-ability-sentinel"))));
            if (row.feedbackId) check("native-feedback-selector",row.feedbackId==((row.mask&2)?CGameSprite::FEEDBACK_GAINEDBARDSONG:CGameSprite::FEEDBACK_GAINEDSPECIALABILITY));
        } else {
            CAbilityId receipt; receipt.m_itemType=1; receipt.m_res=query;
            check("native-complete-quick-ability-ABI",sizeof(receipt)==32 && memcmp(&receipt,row.quick,32)==0);
            SHORT counts[]={0,1,2,3,32767,-32768};
            for (size_t j=0;j<buttons.size();++j) {
                auto& b=*buttons[j]; b=CButtonData(); b.m_abilityId=receipt; b.m_count=counts[j%6]; b.m_icon="ZZICON"; b.m_name=10453;
                switch (j%10) {
                case 1: ++b.m_abilityId.m_itemType; break;
                case 2: ++b.m_abilityId.m_itemNum; break;
                case 3: ++b.m_abilityId.m_abilityNum; break;
                case 4: b.m_abilityId.m_res="ZZOTHER"; break;
                case 5: ++b.m_abilityId.m_nClass; break;
                case 6: ++b.m_abilityId.m_bCanUse; break;
                case 7: ++b.m_abilityId.m_nTooltip; break;
                case 8: b.m_abilityId.m_targetType=12; b.m_abilityId.m_strTooltipName=123; break;
                }
            }
            memset(actor->field_3D3A,0xA5,8); auto before=snapshot();
            actor->UpdateQuickButtons(receipt,-1,FALSE,FALSE); auto expected=snapshot(); BYTE expectedCache[8]; memcpy(expectedCache,actor->field_3D3A,8);
            restore(before); memset(actor->field_3D3A,0xA5,8);
            actor->RemoveSpecialAbility(query);
            check("native-removal-quick-call-integration",same(expected) && memcmp(expectedCache,actor->field_3D3A,8)==0);
        }
        for (INT i=0;i<3;++i) {
            const auto& expected=row.lists[i]; const auto* actual=owned[i];
            check("native-list-size",actual->m_List.size()==expected.count);
            for (UINT j=0;j<expected.count && j<actual->m_List.size();++j) {
                const auto& entry=actual->m_List[j]; UINT values[]={entry.m_nID,entry.m_nMax,entry.m_nCurrent,entry.m_nShared};
                check("native-list-entry-and-duplicate-order",memcmp(values,expected.entries[j],sizeof(values))==0);
            }
            UINT totals[]={actual->m_nSharedCurrent,actual->m_nSharedMax,actual->m_nSharedTotal};
            check("native-list-aggregate-counters",memcmp(totals,expected.totals,sizeof(totals))==0);
        }
        check("spell-resource-ownership-balanced",resource->GetDemands()==demands && resource->m_nLockCount==requests && resource->m_pDimmKeyTableEntry->field_10==refs);
    }
    for (INT i=0;i<3;++i) { catalogues[i]->m_pList=oldData[i]; catalogues[i]->m_nCount=oldCounts[i]; }
    actor->m_innateSpells=savedInnate; actor->m_songs=savedSongs; actor->m_shapeshifts=savedShapes;
    restore(savedButtons); memcpy(actor->field_3D3A,savedCache,8); actor->m_id=oldId; actor->m_bGlobal=oldGlobal;
    resource->m_pHeader=oldHeader;
    if (hadToken) tlk.SetToken(CGameSprite::TOKEN_SPECIALABILITY,oldToken); else tlk.m_mapTokens.RemoveKey(CGameSprite::TOKEN_SPECIALABILITY);
    g_pBaldurChitin->pActiveEngine=oldEngine; g_pBaldurChitin->m_pEngineCharacter->m_nSelectedCharacter=selected;
    check("actor-catalogues-engine-clock-and-queue-restored",game->GetWorldTimer()->m_gameTime==clock && g_pBaldurChitin->GetMessageHandler()->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    guard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"specialabilitychecks\",\"nativeCases\":315,\"replayed\":%d,\"nativeResourceFailures\":%d,\"checks\":%d,\"restored\":true,\"valid\":%s}",nStep,replayed,resourceFailures,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"specialabilitychecks","native special-ability state or ABI replay failed");
}
