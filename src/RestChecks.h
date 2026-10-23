// AutoUI-only party rest replay. Positions, flags, HP, time and queued messages are restored this tick.

void RestChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    INT party=game->GetNumCharacters();
    if (party<2) { Fail(nStep,"restchecks","requires two party members"); return; }
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"rest-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    std::vector<CGameSprite*> actors; std::vector<CPoint> positions; std::vector<SHORT> hitPoints;
    for (INT i=0;i<party;++i) {
        CGameObject* object=NULL; LONG id=game->GetCharacterId(static_cast<SHORT>(i));
        if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
            Fail(nStep,"restchecks","party member unavailable"); return; }
        auto* sprite=static_cast<CGameSprite*>(object); actors.push_back(sprite);
        positions.push_back(sprite->m_pos); hitPoints.push_back(sprite->m_baseStats.m_hitPoints);
        objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    }
    CGameArea* area=actors[0]->GetArea(); ULONG clock0=game->m_worldTime.m_gameTime;
    if (area==NULL) { Fail(nStep,"restchecks","requires a loaded area"); return; }
    DWORD areaFlags=area->m_header.m_flags; BOOLEAN leader=game->m_singlePlayerPermissions.m_bPermissions[CGamePermission::LEADER];
    auto gather=[&]() { for (auto* sprite : actors) sprite->m_pos=positions[0]; };
    STRREF saveError=-1; BOOLEAN canSave=game->CanSaveGame(saveError,TRUE,FALSE);
    STRREF error=-1;

    // Baseline cases run outside the save's own no-rest area flag.
    area->m_header.m_flags=areaFlags&~0x2;
    ++index; gather();
    check("quick-check-gathered-party",game->CanRestParty(error,FALSE,TRUE,FALSE)==TRUE);
    ++index;
    BOOL full=game->CanRestParty(error,FALSE,FALSE,FALSE);
    check("full-check-gathered-party",canSave?(full==TRUE && error==15358):(full==FALSE && (error==16500 || error==10309)));
    ++index; actors[0]->m_derivedStats.m_spellStates.set(SPLSTATE_MOLD_TOUCH);
    // The quick check returns before the spell-state scan, so this uses the full check.
    check("mold-touch-blocks-rest",game->CanRestParty(error,FALSE,FALSE,FALSE)==FALSE && error==10309);
    actors[0]->m_derivedStats.m_spellStates.reset(SPLSTATE_MOLD_TOUCH);
    ++index; area->m_header.m_flags=areaFlags|0x2;
    check("dangerous-area-blocks-rest",game->CanRestParty(error,FALSE,TRUE,FALSE)==FALSE && error==10309);
    ++index;
    check("dangerous-area-override",game->CanRestParty(error,TRUE,TRUE,FALSE)==TRUE);
    area->m_header.m_flags=areaFlags&~0x2;
    ++index; actors[1]->m_pos.x=positions[0].x+0x300;
    BOOL apart=game->CanRestParty(error,FALSE,TRUE,FALSE);
    check("scattered-party-blocks-rest",apart==FALSE && error==(canSave?16457:error) && error!=15358);
    gather();
    if (canSave) {
        ++index; game->m_singlePlayerPermissions.m_bPermissions[CGamePermission::LEADER]=FALSE;
        check("leader-permission-required",game->CanRestParty(error,FALSE,FALSE,FALSE)==FALSE && error==17198);
        game->m_singlePlayerPermissions.m_bPermissions[CGamePermission::LEADER]=leader;
    }
    for (INT i=0;i<party;++i) actors[i]->m_pos=positions[i];
    area->m_header.m_flags=areaFlags;

    // Encounter gates only: these cases never reach creature spawning.
    {
        CAreaFileRestEncounter saved=area->m_headerRestEncounter; auto& encounter=area->m_headerRestEncounter;
        struct EncounterCase { WORD active,probability,creatures,types; INT next; };
        const EncounterCase cases[]={{0,100,1,1,41},{1,0,1,1,18467},{1,100,0,1,18467},{1,100,1,0,18467}};
        for (const auto& row : cases) {
            ++index; encounter.m_activated=row.active; encounter.m_probDay=encounter.m_probNight=row.probability;
            encounter.m_randomCreatureNum=row.creatures; encounter.m_maxTypeNum=row.types;
            srand(1); BOOLEAN result=area->CheckRestEncounter(); INT next=rand();
            check("rest-encounter-gates-and-roll",result==FALSE && next==row.next && game->m_worldTime.m_gameTime==clock0);
        }
        area->m_headerRestEncounter=saved; srand(1);
    }

    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    ULONG clock=game->m_worldTime.m_gameTime; BOOL heal=CInfGame::m_bHealPartyOnRest;
    auto queued=[&](LONG id,INT& amount,SHORT& action,SHORT& trigger) {
        INT found=0; POSITION pos=handler->m_messageList.FindIndex(messages);
        while (pos!=NULL) {
            CMessage* message=handler->m_messageList.GetNext(pos);
            if (message->m_targetId!=id) continue;
            if (auto* add=dynamic_cast<CMessageAddEffect*>(message)) { amount=add->m_effect->m_effectAmount; found|=1; }
            if (auto* act=dynamic_cast<CMessageAddAction*>(message)) { action=act->m_action.m_actionID; found|=2; }
            if (auto* trig=dynamic_cast<CMessageSetTrigger*>(message)) { trigger=trig->m_trigger.m_triggerID; found|=4; }
        }
        return found;
    };
    auto discard=[&]() { while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail(); };
    LONG firstId=game->GetCharacterId(0);

    ++index; CInfGame::m_bHealPartyOnRest=FALSE;
    BOOLEAN rented=game->RestPartyPrivate(7,TRUE);
    INT amount=-1; SHORT action=-1,trigger=-1; INT found=queued(firstId,amount,action,trigger);
    check("renting-rest-returns-true-and-advances-one-period",rented==TRUE && game->m_worldTime.m_gameTime==clock+CRuleTables::TIME_RESTING);
    check("rest-queues-heal-action-and-trigger",found==7 && amount==7 && action==96 && trigger==0x93);
    discard(); game->m_worldTime.m_gameTime=clock;

    // Single player runs as a hosted null-provider session, so RestParty's
    // notices are followed by StartRestMultiplayerHost's before the rest.
    ++index;
    BOOLEAN hosted=g_pChitin->cNetwork.m_bConnectionEstablished && g_pChitin->cNetwork.m_bIsHost;
    BOOLEAN viaParty=game->RestParty(7,TRUE); INT notices=0;
    for (POSITION pos=handler->m_messageList.FindIndex(messages);pos!=NULL;)
        if (dynamic_cast<CMessage101*>(handler->m_messageList.GetNext(pos))!=NULL) ++notices;
    check("rest-party-renting-notices-and-rest",viaParty==TRUE && notices==(hosted?2*party:party)
        && game->m_worldTime.m_gameTime==clock+CRuleTables::TIME_RESTING);
    discard(); game->m_worldTime.m_gameTime=clock;

    ++index; CInfGame::m_bHealPartyOnRest=TRUE;
    for (INT i=0;i<party;++i) actors[i]->m_baseStats.m_hitPoints=static_cast<SHORT>(actors[i]->m_baseStats.m_maxHitPointsBase);
    actors[0]->m_baseStats.m_hitPoints=static_cast<SHORT>(actors[0]->m_baseStats.m_maxHitPointsBase-10);
    game->RestPartyPrivate(1000,TRUE); amount=-1; found=queued(firstId,amount,action,trigger);
    check("heal-on-rest-wounded-party-doubles-time",game->m_worldTime.m_gameTime==clock+2*CRuleTables::TIME_RESTING && (found&1) && amount==1000);
    discard(); game->m_worldTime.m_gameTime=clock;
    ++index; actors[0]->m_baseStats.m_hitPoints=static_cast<SHORT>(actors[0]->m_baseStats.m_maxHitPointsBase);
    game->RestPartyPrivate(1000,TRUE);
    check("heal-on-rest-healthy-party-one-period",game->m_worldTime.m_gameTime==clock+CRuleTables::TIME_RESTING);
    discard(); game->m_worldTime.m_gameTime=clock;

    CInfGame::m_bHealPartyOnRest=heal;
    for (INT i=0;i<party;++i) actors[i]->m_baseStats.m_hitPoints=hitPoints[i];
    Emit("{\"step\":%d,\"op\":\"restchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"restchecks","party rest mismatch");
}
