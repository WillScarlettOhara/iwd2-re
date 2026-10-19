// AutoUI-only spell-panel replay with borrowed catalogues and loaded resources.
#include "SpellSelectionGolden.h"

void SpellSelectionChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray();
    LONG id=game->GetCharacterId(0); CGameObject* object=NULL;
    if (objects->GetDeny(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) {
        Fail(nStep,"spellselectionchecks","requires party actor"); return;
    }
    auto* actor=static_cast<CGameSprite*>(object);
    CSpell guards[]={CSpell(CResRef("ZRCHIT")),CSpell(CResRef("SPIN260")),CSpell(CResRef("SPWI101"))};
    BOOL loaded[]={guards[0].Demand(),guards[1].Demand(),guards[2].Demand()};
    if (!loaded[0] || !loaded[1] || !loaded[2]) {
        for (INT i=0;i<3;++i) if (loaded[i]) guards[i].Release();
        objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
        Fail(nStep,"spellselectionchecks","requires spell resources"); return;
    }
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    INT checks=0,index=-1,draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"spell-selection-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto base=actor->m_baseStats; CDerivedStats derived,temp; derived=actor->m_derivedStats; temp=actor->m_tempStats;
    auto type=actor->m_typeAI,start=actor->m_startTypeAI,live=actor->m_liveTypeAI; auto spells=actor->m_spells;
    auto* oldRefs=game->m_spells.m_pList; UINT oldCount=game->m_spells.m_nCount;
    CResRef refs[]={CResRef("ZRCHIT"),CResRef("SPIN260"),CResRef("")}; CSpellListEntry entries[255]={};
    CSpellListEntry* oldData[7][9]; UINT oldCounts[7][9];
    for (INT c=0;c<7;++c) for (INT l=0;l<9;++l) {
        auto& list=game->m_spellsByClass[c].m_lists[l]; oldData[c][l]=list.m_pList; oldCounts[c][l]=list.m_nCount;
        list.m_pList=entries; list.m_nCount=0;
    }
    game->m_spells.m_pList=refs; game->m_spells.m_nCount=3;
    SPELL_HEADER headers[2]; SPELL_HEADER* oldHeaders[2]; INT demands[3],requests[3];
    for (INT i=0;i<3;++i) {
        demands[i]=guards[i].pRes->GetDemands(); requests[i]=guards[i].pRes->m_nLockCount;
        if (i<2) { oldHeaders[i]=guards[i].pRes->m_pHeader; headers[i]=*oldHeaders[i];
            headers[i].genericName=10453+i; headers[i].genericDescription=7192; headers[i].school=i+1;
            const char* icon=i?"GUIBTNB":"GUIBTNA"; memset(headers[i].itemIcon,0,8); memcpy(headers[i].itemIcon,icon,7);
            guards[i].pRes->m_pHeader=&headers[i]; }
    }
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    auto& tlk=g_pBaldurChitin->GetTlkTable(); CString oldToken; BOOL hadToken=tlk.m_mapTokens.Lookup(CScreenCharacter::TOKEN_NUMBER,oldToken);
    auto* handler=g_pBaldurChitin->GetMessageHandler(); INT messages=handler->m_messageList.GetCount(); ULONG clock=game->GetWorldTimer()->m_gameTime;
    auto* oldCharacter=g_pBaldurChitin->m_pEngineCharacter;
    {
        ChargenTableFixture knownBard(rules.m_tKnownSpellsBard),knownSorc(rules.m_tKnownSpellsSorcerer);
        ChargenTableFixture maxBard(rules.m_tMaxSpellsBard),maxSorc(rules.m_tMaxSpellsSorcerer),bonus(rules.m_tMaxSpellsBonus);
        knownBard.Set("5"); knownSorc.Set("5"); maxBard.Set("3"); maxSorc.Set("3"); bonus.Set("2");
        CScreenCharacter screen; screen.EngineGameInit(); screen.m_nSelectedCharacter=0;
        g_pBaldurChitin->m_pEngineCharacter=&screen;
        auto* panel=screen.m_cUIManager.GetPanel(8); screen.m_lPopupStack.AddTail(panel);
        auto* button=static_cast<CUIControlButtonCharacter5EBC60*>(panel->GetControl(0));
        INT previousKind=-1;
        for (const auto& row : SPELL_SELECTION_CASES) {
            ++index;
            if (row.kind==1) {
                if (previousKind!=1) { button->field_676=""; button->field_66E=""; button->m_nToolTipStrRef=-999; }
                button->SetSpell(CResRef(row.spell));
                CString spell=button->field_676.GetResRefStr(),icon=button->field_66E.GetResRefStr();
                check("canonical-spell-binding",spell==row.spell && icon==row.icon && button->m_nToolTipStrRef==row.tooltip);
                Emit("{\"op\":\"spell-selection-result\",\"case\":%d,\"kind\":1,\"spell\":\"%s\",\"icon\":\"%s\",\"tooltip\":%d}",index,static_cast<LPCTSTR>(spell),static_cast<LPCTSTR>(icon),button->m_nToolTipStrRef);
                previousKind=row.kind; continue;
            }
            previousKind=row.kind; actor->m_baseStats=base;
            actor->m_baseStats.m_specialization=row.kind==0?SPECMASK_WIZARD_UNIVERSAL:row.spec;
            memset(actor->m_derivedStats.m_nClassLevels,0,11); actor->m_derivedStats.m_nCHR=18;
            BYTE cls=row.kind==2?CAIOBJECTTYPE_C_WIZARD:static_cast<BYTE>(row.cls);
            actor->m_derivedStats.m_nClassLevels[cls-1]=5; actor->m_derivedStats.m_nLevel=5;
            actor->m_derivedStats.m_classMask=game->GetClassMask(cls);
            actor->m_typeAI=type; actor->m_typeAI.m_nClass=cls; actor->m_typeAI.m_nClassMask=game->GetClassMask(cls);
            actor->m_startTypeAI=actor->m_liveTypeAI=actor->m_typeAI;
            for (auto& group : actor->m_spells.m_spellsByClass) {
                group.Clear(); group.m_nHighestLevel=0;
                for (auto& list : group.m_lists) list.m_nSharedCurrent=list.m_nSharedMax=list.m_nSharedTotal=0;
            }
            for (INT j=0;j<255;++j) entries[j].m_nID=row.kind==2 && row.count==255?(j==254?0:2):j%3;
            for (INT c=0;c<7;++c) for (INT l=0;l<9;++l)
                game->m_spellsByClass[c].m_lists[l].m_nCount=row.kind==2?(c==6 && l==row.level-1?row.count:0):row.kind==3?1:row.count;
            if (row.kind==2) {
                BOOL result=g_pBaldurChitin->m_pEngineCreateChar->HasSpecialistSpell(static_cast<BYTE>(row.level),actor);
                check("canonical-specialist-scan",result==row.result);
                Emit("{\"op\":\"spell-selection-result\",\"case\":%d,\"kind\":2,\"result\":%d}",index,result); continue;
            }
            screen.m_nClass=cls; screen.field_1776=static_cast<BYTE>(row.kind==3?1:row.level); screen.m_nSpecialization=row.spec;
            headers[0].notUsableBy=0x4000; headers[1].notUsableBy=row.kind==0 && row.profile?0:0x4000;
            UINT classIndex=game->GetSpellcasterIndex(cls);
            if (row.kind==0) {
                if (row.known) { CGameSpriteSpellListEntry entry={0,1,1,0};
                    actor->m_spells.GetSpellsAtLevel(classIndex,row.level-1)->m_List.push_back(entry); }
                for (INT j=0;j<30;++j) { auto* b=static_cast<CUIControlButtonCharacter5EBC60*>(panel->GetControl(j)); b->field_676="OLD"; b->SetSelected(TRUE); }
                screen.ResetSpellsPanel(actor); DWORD enabled=0; CString names;
                for (INT j=0;j<30;++j) {
                    auto* b=static_cast<CUIControlButtonCharacter5EBC60*>(panel->GetControl(j)); CString name=b->field_676.GetResRefStr();
                    check("canonical-spell-slot",name==row.spells[j] && !b->m_bSelected);
                    if (b->m_bEnabled) enabled|=1u<<j;
                    CString value; value.Format(_T("%s\"%s\""),j?",":"",static_cast<LPCTSTR>(name)); names+=value;
                }
                CString token; tlk.m_mapTokens.Lookup(CScreenCharacter::TOKEN_NUMBER,token); CString expected; expected.Format(_T("%d"),screen.m_nExtraMageSpells);
                check("canonical-budget-and-enabled",screen.m_nExtraMageSpells==row.budget && enabled==row.enabled && token==expected);
                Emit("{\"op\":\"spell-selection-result\",\"case\":%d,\"kind\":0,\"budget\":%d,\"enabled\":%u,\"spells\":[%s],\"token\":\"%s\"}",index,screen.m_nExtraMageSpells,enabled,static_cast<LPCTSTR>(names),static_cast<LPCTSTR>(token));
            } else {
                screen.m_nExtraMageSpells=row.extra; screen.field_1776=1; button->field_676="ZRCHIT"; button->SetSelected(row.selected);
                auto* group=actor->m_spells.Get(classIndex); auto& list=group->m_lists[0];
                if (row.selected) { CGameSpriteSpellListEntry entry={0,3,0,0}; list.m_List.push_back(entry);
                    list.m_nSharedMax=list.m_nSharedTotal=3; group->m_nHighestLevel=1; }
                button->OnLButtonClick(CPoint(0,0));
                BOOL same=list.m_List.size()==row.entryCount && list.m_nSharedCurrent==row.totals[0]
                    && list.m_nSharedMax==row.totals[1] && list.m_nSharedTotal==row.totals[2] && group->m_nHighestLevel==row.highest;
                CString data;
                for (UINT j=0;j<list.m_List.size();++j) { auto& a=list.m_List[j];
                    same=same && a.m_nID==row.entries[j][0] && a.m_nMax==row.entries[j][1] && a.m_nCurrent==row.entries[j][2] && a.m_nShared==row.entries[j][3];
                    CString value; value.Format(_T("%s[%u,%u,%u,%u]"),j?",":"",a.m_nID,a.m_nMax,a.m_nCurrent,a.m_nShared); data+=value; }
                check("canonical-real-click-and-owned-list",same && screen.m_nExtraMageSpells==row.budget);
                // An exhausted click keeps the unselected button unselected.
                check("canonical-click-selection",button->m_bSelected==(row.selected?FALSE:row.extra>0));
                CString budget; budget.Format(_T("%d"),screen.m_nExtraMageSpells);
                check("popup-refreshes-budget-label",static_cast<CUIControlLabel*>(panel->GetControl(0x10000022))->GetText()==budget);
                check("popup-refreshes-done",static_cast<CUIControlButton*>(panel->GetControl(33))->m_bEnabled==(screen.m_nExtraMageSpells==0));
                Emit("{\"op\":\"spell-selection-result\",\"case\":%d,\"kind\":3,\"budget\":%d,\"selected\":%d,\"entries\":[%s],\"totals\":[%u,%u,%u],\"highest\":%u}",index,screen.m_nExtraMageSpells,button->m_bSelected,static_cast<LPCTSTR>(data),list.m_nSharedCurrent,list.m_nSharedMax,list.m_nSharedTotal,group->m_nHighestLevel);
            }
        }
        // Exercise the recovered icon renderer on the software back surface.
        auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
        CRect patch(32,32,96+button->m_size.cx+abs(button->field_63E),96+button->m_size.cy+abs(button->field_642));
        std::vector<BYTE> savedPixels;
        auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear) {
            if (!video->BKLock(patch)) return FALSE;
            DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8; INT row=patch.Width()*bytes;
            BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
            if (ok && read) read->resize(row*patch.Height());
            if (ok) for (INT y=0;y<patch.Height();++y) { BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
                if (read) memcpy(read->data()+row*y,p,row); if (write) memcpy(p,write->data()+row*y,row); if (clear) memset(p,0,row); }
            video->BKUnlock(); return ok;
        };
        BOOL backed=pixels(&savedPixels,NULL,FALSE); check("ui-pixels-backed-up",backed);
        CResRef actualIcon(guards[2].pRes->m_pHeader->itemIcon); CString icon; actualIcon.CopyToString(icon);
        check("loaded-spell-icon-nonempty",!icon.IsEmpty()); if (!icon.IsEmpty()) icon.SetAt(icon.GetLength()-1,'B');
        button->field_66E=icon; button->field_676="SPWI101"; panel->m_ptOrigin=CPoint(64,64); button->m_ptOrigin=CPoint(0,0);
        for (BOOL enabled : {FALSE,TRUE}) for (BOOL pressed : {FALSE,TRUE}) for (BOOL clipped : {FALSE,TRUE}) {
            button->SetEnabled(enabled); button->m_bActive=TRUE; button->m_bPressed=pressed;
            CRect frame(panel->m_ptOrigin+button->m_ptOrigin,button->m_size);
            if (pressed) frame.OffsetRect(button->field_63E,button->field_642);
            button->m_rDirty=frame; if (clipped) button->m_rDirty.right=button->m_rDirty.left+button->m_size.cx/2;
            std::vector<BYTE> expected,actual,basePixels; BOOL ok=backed && pixels(NULL,NULL,TRUE);
            button->m_nRenderCount=1; BOOL result=button->CUIControlButton3State::Render(TRUE);
            ok=pixels(&basePixels,NULL,FALSE) && ok;
            if (result) { CRect clip; clip.IntersectRect(frame,button->m_rDirty);
                CIcon::RenderIcon(0,frame.TopLeft(),button->m_size,clip,button->field_66E,screen.m_cUIManager.m_bDoubleSize,enabled?0:2,0,FALSE,0,FALSE,0); }
            ok=pixels(&expected,NULL,TRUE) && ok; button->m_nRenderCount=1;
            BOOL rendered=button->Render(TRUE); ++draws; ok=pixels(&actual,NULL,FALSE) && ok;
            check("ui-full-software-icon-pixels",ok && result && rendered && actual==expected);
            if (!clipped) check("icon-adds-visible-pixels",basePixels!=expected);
        }
        button->m_bActive=FALSE; button->m_bInactiveRender=FALSE; check("inactive-icon-rejected",!button->Render(TRUE));
        button->m_bActive=TRUE; button->m_nRenderCount=0; check("clean-icon-rejected",!button->Render(FALSE));
        check("ui-pixels-restored",backed && pixels(NULL,&savedPixels,FALSE));
        // Real racial 2DA loading, with a deterministic TLK description column.
        {
            BeginLevelUpTableFixture descriptions(rules.m_tSrList,4,256);
            for (auto& cell : descriptions.cells) cell="7192";
            actor->m_startTypeAI.m_nRace=CAIOBJECTTYPE_R_HUMAN;
            CString race=rules.GetRaceString(CAIOBJECTTYPE_R_HUMAN,0);
            CString resource=rules.m_tSrTable.GetAt(CString(_T("SRFILE")),race);
            C2DArray guard; guard.Load(CResRef(resource)); auto* res=guard.pRes; INT before=res?res->m_nLockCount:0;
            for (BYTE subrace : {BYTE(0),BYTE(1),BYTE(2)}) {
                actor->m_startTypeAI.m_nSubRace=subrace; screen.ResetRaceDescriptionPanel(actor);
                auto* text=static_cast<CUIControlTextDisplay*>(screen.m_cUIManager.GetPanel(53)->GetControl(0));
                CString value; if (!text->m_plstStrings->IsEmpty()) text->GetString(0,value);
                check("racial-description-loaded-and-displayed",value==screen.FetchString(7192));
                // The native 2DA loader leaves one additional request per load.
                if (res && res->m_nLockCount==before+1) res->CancelRequest();
                check("racial-table-request-balanced",res && res->m_nLockCount==before);
            }
            if (res) res->CancelRequest(); // Retire the guard loader's extra request.
        }
        // Continue a real Done sequence across skills, feats and two spell tiers.
        {
            auto allow=actor->m_bAllowEffectListCall; auto dirty=actor->field_562C; auto deferred=actor->field_72A2;
            // Feat-panel initialization must retain the synthetic class progression.
            actor->m_bAllowEffectListCall=FALSE;
            BeginLevelUpTableFixture bardProgress(rules.m_tKnownSpellsBard,2,2),sorcProgress(rules.m_tKnownSpellsSorcerer,2,2);
            for (auto* fixture : {&bardProgress,&sorcProgress}) {
                fixture->cells[0]=fixture->cells[1]="0"; fixture->cells[2]=fixture->cells[3]="1";
            }
            screen.m_pTempDerivedStats=new CDerivedStats;
            for (BYTE cls : {BYTE(CAIOBJECTTYPE_C_BARD),BYTE(CAIOBJECTTYPE_C_SORCERER)}) {
                actor->m_baseStats=base; actor->m_derivedStats=derived;
                memset(actor->m_derivedStats.m_nClassLevels,0,11);
                actor->m_derivedStats.m_nClassLevels[cls-1]=2; actor->m_derivedStats.m_nLevel=2;
                actor->m_derivedStats.m_classMask=game->GetClassMask(cls);
                actor->m_typeAI=type; actor->m_typeAI.m_nClass=cls; actor->m_typeAI.m_nClassMask=game->GetClassMask(cls);
                actor->m_startTypeAI=actor->m_liveTypeAI=actor->m_typeAI;
                *screen.m_pTempDerivedStats=actor->m_derivedStats;
                screen.m_pTempDerivedStats->m_nClassLevels[cls-1]=screen.m_pTempDerivedStats->m_nLevel=1;
                for (auto& group : actor->m_spells.m_spellsByClass) group.Clear();
                entries[0].m_nID=0; headers[0].notUsableBy=0x4000;
                UINT caster=game->GetSpellcasterIndex(cls);
                game->m_spellsByClass[caster].m_lists[0].m_nCount=game->m_spellsByClass[caster].m_lists[1].m_nCount=1;
                screen.m_nClass=cls; screen.field_1776=7; screen.field_64C=1; screen.m_nExtraSkillPoints=1;
                screen.m_lPopupStack.RemoveAll(); screen.m_lPopupStack.AddTail(screen.m_cUIManager.GetPanel(55));
                screen.OnDoneButtonClick();
                BOOL ready=screen.GetTopPopup()->m_nID==56 && screen.field_1776==0
                    && actor->m_derivedStats.GetClassLevel(cls)==2;
                check("done-caster-skills-restart-spell-tier",ready);
                if (!ready) break;
                screen.m_nExtraFeats=0; screen.OnDoneButtonClick();
                check("done-caster-feats-open-first-tier",screen.GetTopPopup()->m_nID==8 && screen.field_1776==1
                    && screen.m_nExtraMageSpells==1 && button->field_676==CResRef("ZRCHIT"));
                button->OnLButtonClick(CPoint(0,0));
                check("done-sequence-first-spell-selected",screen.m_nExtraMageSpells==0);
                auto* done=dynamic_cast<CUIControlButtonCharacterPopupDone*>(panel->GetControl(33));
                check("actual-spell-done-control-present",done!=NULL);
                if (done) done->OnLButtonClick(CPoint(0,0));
                check("done-spell-reopens-next-tier",screen.GetTopPopup()->m_nID==8 && screen.field_1776==2
                    && screen.m_nExtraMageSpells==1 && !button->m_bSelected
                    && actor->GetSpellsAtLevel(cls,0)->m_List.size()==1
                    && actor->GetSpellsAtLevel(cls,1)->m_List.empty());
            }
            screen.m_lPopupStack.RemoveAll(); delete screen.m_pTempDerivedStats; screen.m_pTempDerivedStats=NULL;
            actor->m_bAllowEffectListCall=allow; actor->field_562C=dirty; actor->field_72A2=deferred;
        }
        screen.m_lPopupStack.RemoveAll(); g_pBaldurChitin->m_pEngineCharacter=oldCharacter;
    }
    if (hadToken) tlk.SetToken(CScreenCharacter::TOKEN_NUMBER,oldToken); else tlk.m_mapTokens.RemoveKey(CScreenCharacter::TOKEN_NUMBER);
    for (INT c=0;c<7;++c) for (INT l=0;l<9;++l) { auto& list=game->m_spellsByClass[c].m_lists[l]; list.m_pList=oldData[c][l]; list.m_nCount=oldCounts[c][l]; }
    game->m_spells.m_pList=oldRefs; game->m_spells.m_nCount=oldCount;
    actor->m_baseStats=base; actor->m_derivedStats=derived; actor->m_tempStats=temp; actor->m_spells=spells;
    actor->m_typeAI=type; actor->m_startTypeAI=start; actor->m_liveTypeAI=live;
    for (INT i=0;i<3;++i) { if (i<2) guards[i].pRes->m_pHeader=oldHeaders[i];
        check("spell-resources-balanced",guards[i].pRes->GetDemands()==demands[i] && guards[i].pRes->m_nLockCount==requests[i]); }
    check("same-tick-clock-and-queue-preserved",game->GetWorldTimer()->m_gameTime==clock && handler->m_messageList.GetCount()==messages);
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    for (auto& guard : guards) guard.Release(); objects->ReleaseDeny(id,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    Emit("{\"step\":%d,\"op\":\"spellselectionchecks\",\"nativeCases\":%u,\"checks\":%d,\"draws\":%d,\"passed\":%s}",nStep,static_cast<UINT>(_countof(SPELL_SELECTION_CASES)),checks,draws,valid?"true":"false");
    if (!valid) Fail(nStep,"spellselectionchecks","spell panel/binding/click/scan/render mismatch");
}
