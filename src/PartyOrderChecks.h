// AutoUI-only party order and toolbar replay. Portrait order and queued messages are restored this tick.

void PartyOrderChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); INT party=game->GetNumCharacters();
    if (party<3) { Fail(nStep,"partyorderchecks","requires three party members"); return; }
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"party-order-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto discard=[&]() { while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail(); };
    auto leaderSounds=[&](LONG target) {
        INT found=0;
        for (POSITION pos=handler->m_messageList.FindIndex(messages);pos!=NULL;) {
            if (auto* sound=dynamic_cast<CMessagePlaySound*>(handler->m_messageList.GetNext(pos)))
                if (sound->m_soundId==2 && sound->m_showText==TRUE && sound->m_showCircle==FALSE && sound->m_targetId==target) ++found;
        }
        return found;
    };

    LONG first=game->GetCharacterId(0),second=game->GetCharacterId(1),third=game->GetCharacterId(2);
    ++index; game->SwapCharacters(0,1);
    check("swap-with-leader-swaps-and-announces",game->GetCharacterId(0)==second && game->GetCharacterId(1)==first
        && leaderSounds(second)==1 && handler->m_messageList.GetCount()==messages+1);
    discard(); game->SwapCharacters(0,1); discard();
    check("swap-back-restores-order",game->GetCharacterId(0)==first && game->GetCharacterId(1)==second);
    ++index; game->SwapCharacters(1,2);
    check("swap-without-leader-is-silent",game->GetCharacterId(1)==third && game->GetCharacterId(2)==second
        && handler->m_messageList.GetCount()==messages);
    game->SwapCharacters(1,2); discard();

    // The toolbar follows the selection: none, several, or one PC/non-PC sprite.
    ++index;
    if (g_pBaldurChitin->m_pEngineWorld->GetPopupState()!=8) {
        BYTE count=game->m_group.GetCount(); BYTE expected=CInfButtonArray::STATE_NONE;
        if (count>1) expected=0x6E;
        if (count==1) {
            LONG* list=game->m_group.GetGroupList(); CGameObject* object=NULL;
            if (list!=NULL && game->GetObjectArray()->GetShare(list[0],CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                expected=IcewindMisc::IsPC(static_cast<CGameSprite*>(object))?0x72:0x6E;
                game->GetObjectArray()->ReleaseShare(list[0],CGameObjectArray::THREAD_ASYNCH,INFINITE);
            }
            delete[] list;
        }
        INT state=game->m_cButtonArray.m_nState; INT selected=game->m_cButtonArray.m_nSelectedButton;
        game->SelectToolbar();
        check("toolbar-state-follows-selection",game->m_cButtonArray.m_nState==expected && game->m_nState==0);
        game->m_cButtonArray.m_nSelectedButton=selected; game->m_cButtonArray.SetState(state,0);
    }

    Emit("{\"step\":%d,\"op\":\"partyorderchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"partyorderchecks","party order or toolbar mismatch");
}
