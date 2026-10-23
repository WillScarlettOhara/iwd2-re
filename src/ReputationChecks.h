// AutoUI-only party reputation replay. Reputation, the happiness table and queued messages are restored this tick.

void ReputationChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); INT party=game->GetNumCharacters();
    if (party<1) { Fail(nStep,"reputationchecks","requires a party"); return; }
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"reputation-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    SHORT reputation=game->m_nReputation; auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto* baldurMessage=g_pBaldurChitin->GetBaldurMessage(); BOOLEAN inChange=baldurMessage->m_bInReputationChange;
    auto discard=[&]() { while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail(); };

    struct SetCase { int value; BOOL clamp; SHORT expected; };
    const SetCase sets[]={{15,TRUE,150},{25,TRUE,200},{0,TRUE,10},{25,FALSE,250},{-1,FALSE,-10}};
    for (const auto& row : sets) {
        ++index; game->SetReputation(row.value,row.clamp);
        check("set-reputation-scales-and-clamps",game->m_nReputation==row.expected);
    }

    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    {
        BeginLevelUpTableFixture happiness(rules.m_tHappiness,3,20);
        for (INT y=0;y<20;++y) for (INT x=0;x<3;++x) happiness.cells[y*3+x].Format(_T("%d"),x*100+y);
        LONG protagonist=game->GetProtagonist();

        struct AdjustCase { SHORT start,amount; BOOL set; BOOLEAN inChange; SHORT expected,delta; };
        const AdjustCase adjusts[]={{100,20,FALSE,FALSE,120,20},{120,90,TRUE,FALSE,90,-30},{100,15,FALSE,TRUE,115,15},{190,40,FALSE,FALSE,200,40}};
        for (const auto& row : adjusts) {
            ++index; game->m_nReputation=row.start; baldurMessage->m_bInReputationChange=row.inChange;
            game->ReputationAdjustment(row.amount,row.set);
            INT changes=0,changeDelta=0,happy=0,effects=0; BOOL happyRight=TRUE,effectRight=TRUE;
            for (POSITION pos=handler->m_messageList.FindIndex(messages);pos!=NULL;) {
                CMessage* message=handler->m_messageList.GetNext(pos);
                if (auto* change=dynamic_cast<CMessageReputationChange*>(message)) {
                    ++changes; changeDelta=change->m_deltaRep; happyRight=happyRight && change->m_targetId==protagonist; }
                if (auto* mood=dynamic_cast<CMessageSetHappiness*>(message)) {
                    ++happy; CGameObject* object=NULL;
                    if (game->GetObjectArray()->GetShare(mood->m_targetId,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                        BYTE alignment=static_cast<CGameSprite*>(object)->GetAIType().m_nAlignment&0xF;
                        INT column=alignment==2?1:alignment==3?2:0;
                        happyRight=happyRight && mood->m_happiness==column*100+(row.expected/10-1);
                        game->GetObjectArray()->ReleaseShare(mood->m_targetId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
                    }
                }
                if (auto* add=dynamic_cast<CMessageAddEffect*>(message)) {
                    ++effects; effectRight=effectRight && add->m_effect->m_effectID==CGAMEEFFECT_REPUTATION
                        && add->m_effect->m_effectAmount==row.expected; }
            }
            check("adjustment-clamps-reputation",game->m_nReputation==row.expected);
            // The announcement is BROADCAST_FORCED_OTHERS, so it never lands in the local queue.
            check("adjustment-announcement-not-queued-locally",changes==0 && changeDelta==0);
            check("adjustment-sets-happiness-and-effect-per-member",happy==party && effects==party && happyRight && effectRight);
            discard();
        }
    }

    baldurMessage->m_bInReputationChange=inChange; game->m_nReputation=reputation;
    Emit("{\"step\":%d,\"op\":\"reputationchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"reputationchecks","party reputation mismatch");
}
