// AutoUI-only inventory identify/scribe replay. Fixture items go into two empty pack slots of the
// first party member; slots, popup fields and queued messages are restored this tick.

void IdentifyChecks(INT nStep)
{
    auto* inv=g_pBaldurChitin->m_pEngineInventory; auto* game=g_pBaldurChitin->GetObjectGame();
    // Entering the inventory stalls on this save, so the screen is driven from the world; its panels exist since EngineGameInit.
    INT selected=inv->m_nSelectedCharacter; inv->m_nSelectedCharacter=0;
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"identify-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    LONG id=game->GetCharacterId(static_cast<SHORT>(inv->m_nSelectedCharacter)); CGameObject* object=NULL;
    if (game->GetObjectArray()->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        inv->m_nSelectedCharacter=selected; Fail(nStep,"identifychecks","selected character unavailable"); return; }
    game->GetObjectArray()->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    auto* sprite=static_cast<CGameSprite*>(object);
    INT buttons[2]={-1,-1};
    for (INT button=30,found=0;button<=45 && found<2;++button) {
        INT slot=inv->MapButtonIdToInventoryId(button);
        if (slot>=0 && slot<51 && sprite->m_equipment.m_items[slot]==NULL) buttons[found++]=button; }
    if (buttons[1]<0) { inv->m_nSelectedCharacter=selected; Fail(nStep,"identifychecks","requires two empty pack slots"); return; }
    INT itemSlot=inv->MapButtonIdToInventoryId(buttons[0]),scrollSlot=inv->MapButtonIdToInventoryId(buttons[1]);
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount();
    auto discard=[&]() { while (handler->m_messageList.GetCount()>messages) delete handler->m_messageList.RemoveTail(); };
    INT errorState=inv->m_nErrorState; DWORD requester=inv->m_nRequesterButtonId;
    int copied[3]={inv->field_51C,inv->field_520,inv->field_524};
    auto* item=new CItem(CResRef(_T("ZRCMELEE")),1,0,0,0,0); item->m_flags&=~0x1;
    auto* scroll=new CItem(CResRef(_T("SPWI110Z")),5,0,0,0,1);
    sprite->m_equipment.m_items[itemSlot]=item; sprite->m_equipment.m_items[scrollSlot]=scroll;

    // Plain notices answer only their first button; the rest popup has three.
    ++index; inv->m_nErrorState=0;
    check("notice-first-button-only",inv->IsErrorButtonClickable(0) && !inv->IsErrorButtonClickable(1));
    inv->m_nErrorState=1;
    check("rest-three-buttons",inv->IsErrorButtonClickable(2) && !inv->IsErrorButtonClickable(3));
    inv->m_nErrorState=5;
    check("result-first-button-only",inv->IsErrorButtonClickable(0) && !inv->IsErrorButtonClickable(2));

    // The identify popup offers spell and scroll only for an unidentified item.
    ++index; inv->m_nErrorState=3; inv->m_nRequesterButtonId=buttons[0];
    DWORD mask=0xFFF; BOOL spell=sprite->HasSpell(CGameSprite::SPWI110,mask,9,TRUE) || sprite->HasInnateSpell(CGameSprite::SPIN285,TRUE);
    BOOL usable=scroll->pRes!=NULL && game->CheckItemUsable(sprite,scroll)!=0;
    Emit("{\"step\":%d,\"op\":\"identify-detail\",\"spell\":%d,\"usable\":%d,\"charges\":%d}",nStep,spell,usable,scroll->GetUsageCount(0));
    check("identify-cancel-always",inv->IsErrorButtonClickable(2));
    check("identify-spell-needs-a-cast",inv->IsErrorButtonClickable(0)==spell);
    check("identify-scroll-needs-a-usable-scroll",inv->IsErrorButtonClickable(1)==usable);

    // Reading the scroll spends a charge and identifies the item.
    ++index; inv->IdentifyWithScroll();
    check("scroll-identifies-and-spends",(item->m_flags&0x1)!=0 && scroll->GetUsageCount(0)==4
        && sprite->m_equipment.m_items[scrollSlot]==scroll);
    check("identified-item-offers-no-identify",!inv->IsErrorButtonClickable(0) && !inv->IsErrorButtonClickable(1));
    discard();

    // The spell path identifies even when nothing is left to spend.
    ++index; item->m_flags&=~0x1;
    if (!spell) {
        inv->IdentifyWithSpell();
        check("spell-identifies-without-a-cast",(item->m_flags&0x1)!=0);
    }
    discard();

    // Scribing the scroll queues its learn-spell effect and remembers the spell for the result popup.
    ++index; inv->m_nRequesterButtonId=buttons[1];
    UINT spellId=0; BOOL known=game->m_spells.Find(CGameSprite::SPWI110,spellId);
    inv->field_51C=0; inv->field_520=0; inv->field_524=0;
    inv->CopySpell();
    INT effects=0;
    for (POSITION pos=handler->m_messageList.FindIndex(messages);pos!=NULL;)
        if (dynamic_cast<CMessageAddEffect*>(handler->m_messageList.GetNext(pos))!=NULL) ++effects;
    check("scribe-spends-a-charge",scroll->GetUsageCount(0)==3 && sprite->m_equipment.m_items[scrollSlot]==scroll);
    check("scribe-queues-learn-effect",effects==1);
    check("scribe-remembers-spell",known && inv->field_51C==static_cast<int>(spellId) && inv->field_520==1 && inv->field_524==1);
    discard();

    sprite->m_equipment.m_items[itemSlot]=NULL; sprite->m_equipment.m_items[scrollSlot]=NULL;
    delete item; delete scroll;
    inv->m_nErrorState=errorState; inv->m_nRequesterButtonId=requester; inv->m_nSelectedCharacter=selected;
    inv->field_51C=copied[0]; inv->field_520=copied[1]; inv->field_524=copied[2];
    game->m_cButtonArray.UpdateButtons();

    Emit("{\"step\":%d,\"op\":\"identifychecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"identifychecks","identify or scribe mismatch");
}
