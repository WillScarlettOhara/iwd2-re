// AutoUI-only poison pipeline replay. Borrow the actor for one tick and park
// owning stats/lists so no original effect is evaluated or deleted by the test.
void PersistentEffectChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    LONG id=game->GetCharacterId(0); CGameObject* raw=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&raw,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"persistenteffectchecks","requires loaded party actor"); return;
    }
    auto* actor=static_cast<CGameSprite*>(raw); INT checks=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"persistent-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    auto base=actor->m_baseStats; auto allow=actor->m_bAllowEffectListCall; auto dirty=actor->field_562C; auto deferred=actor->field_72A2;
    auto sequence=actor->m_nSequence; auto color=actor->m_hasColorEffects; auto sound=actor->m_poisonSoundTimer;
    auto damage=actor->m_nDamageTaken; auto flash=actor->m_bBloodFlashOn; auto flashAmount=actor->m_nBloodFlashAmount;
    auto locator=actor->m_nDamageLocatorTime; auto held=actor->field_9D15;
    BYTE saves[]={actor->field_70F6,actor->field_70F7,actor->field_70F8,actor->field_70F9,actor->field_70FA};
    auto* timer=game->GetWorldTimer(); ULONG clock=timer->m_gameTime; auto active=timer->m_active;
    auto cutscene=game->m_gameSave.m_cutScene; auto difficulty=game->m_cOptions.m_nDifficultyMultiplier;
    auto mpDifficulty=game->m_cOptions.m_nMPDifficultyMultiplier; auto text=game->m_cOptions.m_nEffectTextLevel;
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    std::vector<int*> icons;
    auto next=actor->m_timedEffectList.m_posNext,current=actor->m_timedEffectList.m_posCurrent;
    auto eqNext=actor->m_equipedEffectList.m_posNext,eqCurrent=actor->m_equipedEffectList.m_posCurrent;
    BOOL newTimed=actor->m_timedEffectList.m_newEffect,retry=actor->m_timedEffectList.m_retry;
    BOOL newEquipped=actor->m_equipedEffectList.m_newEffect,eqRetry=actor->m_equipedEffectList.m_retry;
    // Detach whole lists by retaining their nodes: callers can keep POSITIONs.
    std::vector<BYTE> timedBytes(sizeof(CGameEffectList)),equippedBytes(sizeof(CGameEffectList));
    memcpy(timedBytes.data(),&actor->m_timedEffectList,timedBytes.size());
    memcpy(equippedBytes.data(),&actor->m_equipedEffectList,equippedBytes.size());
    new (&actor->m_timedEffectList) CGameEffectList; new (&actor->m_equipedEffectList) CGameEffectList;
    while (!actor->m_portraitIcons.IsEmpty()) icons.push_back(actor->m_portraitIcons.RemoveHead());
    CDerivedStats* stats[]={&actor->m_derivedStats,&actor->m_tempStats,&actor->m_bonusStats};
    std::vector<BYTE> parked[3];
    for (INT i=0;i<3;++i) { parked[i].resize(sizeof(CDerivedStats)); memcpy(parked[i].data(),stats[i],sizeof(CDerivedStats)); new (stats[i]) CDerivedStats; }
    auto discard=[&]() {
        while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail();
        while (!actor->m_timedEffectList.IsEmpty()) delete actor->m_timedEffectList.RemoveTail();
        while (!actor->m_equipedEffectList.IsEmpty()) delete actor->m_equipedEffectList.RemoveTail();
        for (auto* stat:stats) while (!stat->m_cRegeneratedPersistantEffectList.IsEmpty()) delete stat->m_cRegeneratedPersistantEffectList.RemoveTail();
    };
    auto reset=[&]() {
        discard(); actor->m_baseStats=base; actor->m_baseStats.m_hitPoints=actor->m_baseStats.m_maxHitPointsBase=120;
        actor->m_baseStats.m_generalState=0; memset(actor->m_baseStats.m_feats,0,sizeof(actor->m_baseStats.m_feats));
        actor->m_portraitIcons.RemoveAll();
        for (auto* stat:stats) { stat->~CDerivedStats(); new (stat) CDerivedStats; stat->m_nMaxHitPoints=120; }
        actor->m_nSequence=CGameSprite::SEQ_DAMAGE; actor->m_bAllowEffectListCall=FALSE; actor->field_9D15=0;
        timer->m_gameTime=100; timer->m_active=TRUE; game->m_gameSave.m_cutScene=FALSE;
        game->m_cOptions.m_nDifficultyMultiplier=game->m_cOptions.m_nMPDifficultyMultiplier=0;
        game->m_cOptions.m_nEffectTextLevel=text&~8; actor->m_poisonSoundTimer=1;
    };
    auto poison=[&](DWORD mode,LONG amount,DWORD secondary=1) {
        ITEM_EFFECT item={}; item.effectID=CGAMEEFFECT_POISON; item.dwFlags=mode; item.effectAmount=amount;
        item.durationType=1; item.probabilityUpper=100;
        auto* result=CGameEffect::DecodeEffect(&item,actor->GetPos(),id,actor->GetPos());
        result->m_secondaryType=secondary; result->m_durationType=0x1000; result->m_duration=200;
        result->m_special=90; result->m_effectAmount2=21; result->m_sourceRes="ZRP00001"; return result;
    };
    reset();
    struct Schedule { INT start,next; };
    for (const Schedule& row:{Schedule{100,100},Schedule{99,99},Schedule{98,113},Schedule{102,117},Schedule{120,120}}) {
        CPersistantEffect84C420 effect(0x200000,3,15,row.start,200);
        check("persistent-scheduler-uses-signed-time-difference",effect.field_20==row.next && effect.field_18==row.start);
    }
    struct Mode { DWORD mode; INT interval,amount; };
    for (const Mode& row:{Mode{2,15,3},Mode{3,45,1},Mode{4,105,3},Mode{5,735,3},Mode{6,105,3},Mode{7,21,3}}) {
        reset(); auto* effect=poison(row.mode,3);
        check("poison-real-decoder-and-virtual-apply",dynamic_cast<CGameEffectPoison*>(effect)!=NULL && effect->ApplyEffect(actor));
        auto& list=actor->m_derivedStats.m_cRegeneratedPersistantEffectList;
        auto* periodic=list.IsEmpty()?NULL:dynamic_cast<CPersistantEffect84C420*>(list.GetTail());
        check("poison-enqueues-one-periodic-damage-controller",list.GetCount()==1 && periodic);
        if (periodic) {
            check("poison-native-mode-cadence-and-amount",periodic->field_24==row.interval && periodic->field_38==row.amount
                && periodic->field_34==0x200000 && periodic->field_18==100 && periodic->field_1C==200 && periodic->field_20==100);
            check("poison-controller-source-and-eight-byte-resref",periodic->m_sourceID==id && memcmp(periodic->field_29,"ZRP00001",8)==0 && periodic->field_31==0);
            auto* copy=static_cast<CPersistantEffect84C420*>(periodic->Copy());
            check("poison-controller-copy-keeps-schedule-and-provenance",copy!=periodic && copy->field_24==row.interval
                && copy->field_38==row.amount && copy->m_sourceID==id && memcmp(copy->field_29,periodic->field_29,8)==0); delete copy;
        }
        check("poison-state-and-icon",(actor->m_derivedStats.m_generalState&STATE_POISONED)!=0
            && actor->m_portraitIcons.Find(reinterpret_cast<int*>(6),0)!=NULL && effect->m_special==100);
        check("only-paralytic-poison-adds-held-state",actor->field_9D15==(row.mode==6)
            && actor->m_derivedStats.m_spellStates.test(7)==(row.mode==6)
            && ((actor->m_derivedStats.m_generalState&STATE_HELPLESS)!=0)==(row.mode==6));
        delete effect;
    }
    reset(); auto* effect=poison(2,3,0); effect->m_durationType=1; effect->ApplyEffect(actor);
    auto* periodic=static_cast<CPersistantEffect84C420*>(actor->m_derivedStats.m_cRegeneratedPersistantEffectList.GetTail());
    check("permanent-poison-has-no-expiry-and-preserves-start-on-repass",periodic->field_1C==0 && periodic->field_18==90 && effect->m_special==90); delete effect;
    reset(); effect=poison(2,3); effect->ApplyEffect(actor);
    auto& live=actor->m_derivedStats.m_cRegeneratedPersistantEffectList;
    live.AIUpdate(actor,1); check("poison-strict-first-tick-boundary",actor->m_baseStats.m_hitPoints==120);
    timer->m_gameTime=101; live.AIUpdate(actor,1);
    check("poison-tick-delivers-real-three-hp-damage",actor->m_baseStats.m_hitPoints==117 && actor->m_nDamageTaken==3);
    timer->m_gameTime=115; live.AIUpdate(actor,14); check("poison-waits-through-exact-next-time",actor->m_baseStats.m_hitPoints==117);
    timer->m_gameTime=116; live.AIUpdate(actor,31);
    check("poison-catch-up-delivers-native-delta-count",actor->m_baseStats.m_hitPoints==108 && actor->m_nDamageTaken==9);
    timer->m_gameTime=201; live.AIUpdate(actor,85); check("poison-controller-does-not-damage-after-expiry",actor->m_baseStats.m_hitPoints==108); delete effect;
    reset(); std::vector<CGameEffect*> slowed;
    for (DWORD mode:{2u,3u,4u,7u}) { auto* item=poison(mode,3,0); actor->m_timedEffectList.AddTail(item); slowed.push_back(item); }
    ITEM_EFFECT slowItem={}; slowItem.effectID=ICEWIND_CGAMEEFFECT_SLOWPOISON; slowItem.probabilityUpper=100;
    IcewindCGameEffectSlowPoison slow(&slowItem,actor->GetPos(),id,actor->GetPos()); slow.m_dwFlags=99; slow.ApplyEffect(actor);
    check("slow-poison-uses-each-target-mode",slowed[0]->m_dwFlags==4 && slowed[1]->m_dwFlags==3 && slowed[2]->m_dwFlags==5 && slowed[3]->m_dwFlags==7);
    check("slow-poison-extends-only-supported-modes",slowed[0]->m_duration==900 && slowed[1]->m_duration==900
        && slowed[2]->m_duration==900 && slowed[3]->m_duration==200 && slowed[1]->m_effectAmount==21);
    check("slow-poison-does-not-rewrite-its-own-selector",slow.m_dwFlags==99 && slow.m_done && slow.m_forceRepass);
    reset(); actor->m_timedEffectList.AddTail(poison(2,3,0)); actor->m_equipedEffectList.AddTail(poison(4,3,0));
    actor->m_baseStats.m_generalState|=STATE_POISONED; actor->m_derivedStats.m_generalState|=STATE_POISONED;
    ITEM_EFFECT cureItem={}; cureItem.effectID=CGAMEEFFECT_CUREPOISON; cureItem.probabilityUpper=100;
    CGameEffectCurePoison cure(&cureItem,actor->GetPos(),id,actor->GetPos()); cure.ApplyEffect(actor);
    check("cure-poison-removes-both-owning-effect-lists",actor->m_timedEffectList.IsEmpty() && actor->m_equipedEffectList.IsEmpty());
    check("cure-poison-clears-base-and-derived-state",!(actor->m_baseStats.m_generalState&STATE_POISONED)
        && !(actor->m_derivedStats.m_generalState&STATE_POISONED) && cure.m_done);
    reset(); effect=poison(8,-2); effect->ApplyEffect(actor);
    check("constitution-poison-does-not-create-periodic-damage",actor->m_derivedStats.m_cRegeneratedPersistantEffectList.IsEmpty());
    auto* delivery=handler->m_messageList.GetCount()>messages?dynamic_cast<CMessageAddEffect*>(handler->m_messageList.GetTail()):NULL;
    check("constitution-poison-queues-native-effect",delivery && delivery->m_effect && delivery->m_effect->m_effectID==CGAMEEFFECT_CON
        && delivery->m_effect->m_effectAmount==-2 && delivery->m_effect->m_duration==70 && delivery->m_effect->m_sourceID==id);
    effect->m_secondaryType=0; INT queued=handler->m_messageList.GetCount(); effect->ApplyEffect(actor);
    check("constitution-poison-is-one-shot-on-secondary-type",handler->m_messageList.GetCount()==queued && effect->m_special==90);
    effect->m_special=1; effect->OnRemove(actor);
    delivery=dynamic_cast<CMessageAddEffect*>(handler->m_messageList.GetTail());
    check("constitution-poison-removal-carries-amount-plus-special",delivery && delivery->m_effect
        && delivery->m_effect->m_effectAmount==-1 && delivery->m_effect->m_savingThrow==4); delete effect;
    discard();
    for (INT i=0;i<3;++i) { stats[i]->~CDerivedStats(); memcpy(stats[i],parked[i].data(),sizeof(CDerivedStats)); }
    actor->m_timedEffectList.~CGameEffectList(); actor->m_equipedEffectList.~CGameEffectList();
    memcpy(&actor->m_timedEffectList,timedBytes.data(),timedBytes.size()); memcpy(&actor->m_equipedEffectList,equippedBytes.data(),equippedBytes.size());
    actor->m_portraitIcons.RemoveAll(); for (auto* icon:icons) actor->m_portraitIcons.AddTail(icon);
    actor->m_baseStats=base; actor->m_bAllowEffectListCall=allow; actor->field_562C=dirty; actor->field_72A2=deferred;
    actor->m_nSequence=sequence; actor->m_hasColorEffects=color; actor->m_poisonSoundTimer=sound; actor->m_nDamageTaken=damage;
    actor->m_bBloodFlashOn=flash; actor->m_nBloodFlashAmount=flashAmount; actor->m_nDamageLocatorTime=locator; actor->field_9D15=held;
    actor->field_70F6=saves[0]; actor->field_70F7=saves[1]; actor->field_70F8=saves[2]; actor->field_70F9=saves[3]; actor->field_70FA=saves[4];
    timer->m_gameTime=clock; timer->m_active=active; game->m_gameSave.m_cutScene=cutscene;
    game->m_cOptions.m_nDifficultyMultiplier=difficulty; game->m_cOptions.m_nMPDifficultyMultiplier=mpDifficulty; game->m_cOptions.m_nEffectTextLevel=text;
    check("original-effect-list-positions-restored",actor->m_timedEffectList.m_posNext==next && actor->m_timedEffectList.m_posCurrent==current
        && actor->m_equipedEffectList.m_posNext==eqNext && actor->m_equipedEffectList.m_posCurrent==eqCurrent
        && actor->m_timedEffectList.m_newEffect==newTimed && actor->m_timedEffectList.m_retry==retry
        && actor->m_equipedEffectList.m_newEffect==newEquipped && actor->m_equipedEffectList.m_retry==eqRetry);
    check("same-tick-clock-and-queue-restored",timer->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"persistenteffectchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"persistenteffectchecks","poison scheduler or delivery mismatch");
}
