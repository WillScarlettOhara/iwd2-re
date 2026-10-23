// AutoUI-only area travel replay on the second party member. It returns to its start point,
// facing and flags this tick; queued messages, limbo entries and summon links are discarded.

void AreaTravelChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    if (game->GetNumCharacters()<2) { Fail(nStep,"areatravelchecks","requires two party members"); return; }
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"area-travel-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    LONG id=game->GetCharacterId(1),masterId=game->GetCharacterId(0); CGameObject* object=NULL;
    if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"areatravelchecks","party member unavailable"); return; }
    objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    auto* sprite=static_cast<CGameSprite*>(object); CGameArea* area=sprite->GetArea();
    if (area==NULL) { Fail(nStep,"areatravelchecks","requires a loaded area"); return; }
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto discard=[&]() { while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail(); };
    auto queued=[&](BYTE subType) { INT found=0;
        for (POSITION pos=handler->m_messageList.FindIndex(messages);pos!=NULL;) {
            CMessage* message=handler->m_messageList.GetNext(pos);
            if (message->GetMsgType()==CBaldurMessage::MSG_TYPE_CMESSAGE && message->GetMsgSubType()==subType) ++found; }
        return found; };
    auto within=[](const CPoint& a,const CPoint& b,LONG range) { return abs(a.x-b.x)<=range && abs(a.y-b.y)<=range; };
    CString areaName; area->m_resRef.CopyToString(areaName);
    CPoint start=sprite->m_pos,startPos=sprite->m_posStart; SHORT facing=sprite->m_nDirection;
    BOOLEAN global=sprite->m_bGlobal; BOOL clearAI=sprite->m_clearAIOnRemoveFromArea;
    INT moves=game->m_cMoveList.GetCount(),limbo=game->m_cLimboList.GetCount();
    CPoint target(start.x+3*CPathSearch::GRID_SQUARE_SIZEX,start.y);

    // A creature outside the global list never travels through MoveGlobal.
    ++index; sprite->m_bGlobal=FALSE; sprite->MoveGlobal(CString(_T("AR9999")),target);
    check("move-global-ignores-local-creature",sprite->GetArea()==area && sprite->m_pos==start);
    // A global creature already in the destination only jumps.
    ++index; sprite->m_bGlobal=TRUE; sprite->MoveGlobal(areaName,target);
    check("move-global-same-area-jumps",sprite->GetArea()==area && within(sprite->m_pos,target,2*CPathSearch::GRID_SQUARE_SIZEX)
        && handler->m_messageList.GetCount()>=messages);
    sprite->m_bGlobal=global; sprite->JumpToPoint(start,FALSE); discard();

    // A delayed jump parks the creature in limbo with an upper-case destination.
    ++index; CString lower=areaName; lower.MakeLower();
    SHORT rc=sprite->JumpToArea(lower,target,5,7);
    CMoveListEntry* entry=game->m_cLimboList.GetCount()==limbo+1?game->m_cLimboList.GetTail():NULL;
    CString entryArea; if (entry!=NULL) entry->m_areaResRef.CopyToString(entryArea);
    check("jump-area-delay-queues-limbo",rc==CGameAIBase::ACTION_DONE && entry!=NULL && entry->m_nSpriteId==id
        && entryArea==areaName && entry->m_ptDestination==target && entry->m_nFacing==5 && entry->m_nDelay==7
        && game->m_cMoveList.GetCount()==moves);
    check("jump-area-delay-leaves-area",sprite->GetArea()==NULL && sprite->m_bDeleteOnRemove==TRUE
        && sprite->m_clearAIOnRemoveFromArea==clearAI);
    if (entry!=NULL) delete game->m_cLimboList.RemoveTail();
    ++index; rc=sprite->JumpToArea(areaName,target,5,0);
    check("jump-area-without-area-fails",rc==CGameAIBase::ACTION_ERROR && game->m_cMoveList.GetCount()==moves
        && game->m_cLimboList.GetCount()==limbo);

    // MoveOntoArea snaps to the grid centre, faces, announces and registers the creature.
    ++index; sprite->MoveOntoArea(area,start,facing);
    const SCRIPTNAME& script=sprite->GetScriptName(); CVariable* named=area->m_namedCreatures.FindKey(CString(script),id);
    check("move-onto-area-places-creature",sprite->GetArea()==area && within(sprite->m_pos,start,2*CPathSearch::GRID_SQUARE_SIZEX)
        && sprite->m_pos.x%CPathSearch::GRID_SQUARE_SIZEX==CPathSearch::GRID_SQUARE_SIZEX/2
        && sprite->m_pos.y%CPathSearch::GRID_SQUARE_SIZEY==CPathSearch::GRID_SQUARE_SIZEY/2
        && sprite->m_nDirection==facing && sprite->m_posStart==sprite->m_pos);
    // The sprite update is BROADCAST_OTHERS, so a single-player queue never holds it.
    check("move-onto-area-names-creature",named!=NULL && named->m_intValue==id);
    discard();

    // An immediate jump into a loaded area moves now and keeps the AI.
    ++index; rc=sprite->JumpToArea(areaName,target,3,0);
    check("jump-area-loaded-moves-now",rc==CGameAIBase::ACTION_DONE && sprite->GetArea()==area
        && within(sprite->m_pos,target,4*CPathSearch::GRID_SQUARE_SIZEX)
        && sprite->m_pos.x%CPathSearch::GRID_SQUARE_SIZEX==CPathSearch::GRID_SQUARE_SIZEX/2 && sprite->m_nDirection==3
        && sprite->m_clearAIOnRemoveFromArea==FALSE && game->m_cMoveList.GetCount()==moves
        && game->m_cLimboList.GetCount()==limbo);
    sprite->m_clearAIOnRemoveFromArea=clearAI; discard();
    sprite->JumpToPoint(start,FALSE); sprite->SetFacing(facing); sprite->m_posStart=startPos; discard();
    check("travel-restored",sprite->GetArea()==area && sprite->m_pos==start);

    // Leaving for an unloaded area sends a dismissed summon's links home without the poof.
    ++index; auto* summons=Icewind586B70::Instance(); INT slot=-1;
    for (INT i=0;i<static_cast<INT>(summons->mEntries.size());++i) if (summons->mEntries[i].field_0==0) { slot=i; break; }
    if (slot>=0) {
        auto& link=summons->mEntries[slot]; link.field_0=id; link.field_4=masterId; link.field_8=true;
        BOOL remove=sprite->m_removeFromArea;
        summons->Unsummon(sprite);
        check("unsummon-skips-dismissed",link.field_0==id && link.field_8 && handler->m_messageList.GetCount()==messages
            && sprite->m_removeFromArea==remove);
        CGameObject* master=NULL;
        if (objects->GetShare(masterId,CGameObjectArray::THREAD_ASYNCH,&master,INFINITE)==CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(masterId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
            summons->UnsummonAllOf(static_cast<CGameSprite*>(master));
        }
        check("unsummon-all-clears-links",link.field_0==0 && link.field_4==0 && !link.field_8
            && queued(CBaldurMessage::MSG_SUBTYPE_CMESSAGE_UNSUMMON)==1
            && queued(CBaldurMessage::MSG_SUBTYPE_CMESSAGE_REMOVE_SUMMON_LINK)==1
            && handler->m_messageList.GetCount()==messages+2 && sprite->m_removeFromArea==remove);
        link.Clear(); discard();
    } else {
        check("summon-slot-available",FALSE);
    }

    Emit("{\"step\":%d,\"op\":\"areatravelchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"areatravelchecks","area travel mismatch");
}
