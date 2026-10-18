// AutoUI-only virtual dispatch fixture; the loaded engine and keymap are restored.
struct CharacterKeyScreen : CScreenCharacter {
    INT action=-1,calls=0;
    void SelectEngine(CWarp*) override { action=102; ++calls; }
    void OnPortraitLClick(DWORD portrait) override { action=200+portrait; ++calls; }
    void OnRestButtonClick() override { action=105; ++calls; }
};

void CharacterKeyChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); CharacterKeyScreen screen;
    BYTE map[CINFGAME_KEYMAP_SIZE],flags[CINFGAME_KEYMAP_SIZE];
    memcpy(map,game->m_pKeymap,sizeof(map)); memcpy(flags,game->m_pKeymapFlags,sizeof(flags));
    screen.m_cUIManager.m_bHidden=FALSE; screen.m_cUIManager.m_pFocusedControl=NULL;
    INT checks=0,index=0; BOOL valid=TRUE;
    for (const auto& row : CHARACTER_KEY_CASES) {
        memset(game->m_pKeymap,0,sizeof(map)); memset(game->m_pKeymapFlags,0,sizeof(flags));
        if (row.binding>=0) { game->m_pKeymap[row.binding]=static_cast<BYTE>(row.key); game->m_pKeymapFlags[row.binding]=1; }
        screen.m_pVirtualKeysFlags[0]=static_cast<BYTE>(row.key); screen.m_bCtrlKeyDown=static_cast<BYTE>(row.ctrl);
        screen.action=-1; screen.calls=0; screen.m_cUIManager.field_76=0;
        // Invoke through the engine interface, so the restored vtable slot runs.
        CWarp* engine=&screen; engine->OnKeyDown(static_cast<SHORT>(row.count));
        INT actual=screen.m_cUIManager.field_76?103:screen.action;
        BOOL ok=actual==row.action && screen.calls==(row.action==-1 || row.action==103?0:1);
        ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"character-key-case\",\"row\":%d,\"key\":%d,\"expected\":%d,\"actual\":%d,\"passed\":false}",nStep,index,row.key,row.action,actual);
        ++index;
    }
    screen.m_pVirtualKeysFlags[0]='1'; screen.m_pVirtualKeysFlags[1]='6'; screen.action=-1; screen.calls=0;
    static_cast<CWarp*>(&screen)->OnKeyDown(2);
    BOOL two=screen.action==205 && screen.calls==2; ++checks; valid=valid && two;
    memcpy(game->m_pKeymap,map,sizeof(map)); memcpy(game->m_pKeymapFlags,flags,sizeof(flags));
    Emit("{\"step\":%d,\"op\":\"characterkeychecks\",\"nativeCases\":1599,\"replayed\":%u,\"checks\":%d,\"passed\":%s}",nStep,static_cast<UINT>(_countof(CHARACTER_KEY_CASES)),checks,valid?"true":"false");
    if (!valid) Fail(nStep,"characterkeychecks","native keyboard dispatch mismatch");
}
