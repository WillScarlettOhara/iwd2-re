// AutoUI-only item creation replay: random treasure rows, grade upgrades and charge rolls.

void ItemCreationChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame();
    auto& rules=const_cast<CRuleTables&>(game->GetRuleTables());
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"item-creation-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    auto* options=game->GetOptions(); INT nightmare=options->m_nNightmareMode;
    {
        BeginLevelUpTableFixture norm(rules.m_tRtNorm,4,7),fury(rules.m_tRtFury,4,1);
        const char* labels[]={"ZRONE","ZRTWO","ZRTHREE","ZRNEST","ZRAX_M","ZRAX_V","ZREMPTY"};
        const char* rows[][4]={
            {"ZRCMELEE","","",""},
            {"ZRCMELEE","ZRCARROW","",""},
            {"ZRCMELEE","ZRCARROW","ZRCBOW","*"},
            {"ZRONE","*","",""},
            {"up2","","",""},
            {"UP","","",""},
            {"","","",""},
        };
        for (INT y=0;y<7;++y) { norm.y[y]=labels[y]; for (INT x=0;x<4;++x) norm.cells[y*4+x]=rows[y][x]; }
        fury.y[0]="ZRONE"; for (INT x=0;x<4;++x) fury.cells[x]=x?"":"ZRCBOW";
        options->m_nNightmareMode=0;

        // Picks consume rand() only when the last usable column index is non-zero.
        struct RandomCase { const char* in; BOOL result; const char* out; INT next; };
        const RandomCase randomCases[]={
            {"ZRONE",TRUE,"ZRCMELEE",41},
            {"ZRTWO",TRUE,"ZRCMELEE",18467},
            {"ZRTHREE",TRUE,"ZRCARROW",18467},
            {"zrnest",TRUE,"ZRCMELEE",41},
            {"ZRAX_M",TRUE,"no_drop",41},
            {"ZRAX_V",TRUE,"no_drop",41},
            {"ZREMPTY",FALSE,"",41},
            {"ZRMISSING",FALSE,"ZRMISSING",41},
        };
        for (const auto& row : randomCases) {
            ++index;
            CString sResRef(row.in); srand(1);
            BOOL result=rules.GetRandomItem(sResRef); INT next=rand();
            check("random-item-row-pick-and-rand",result==row.result && sResRef==row.out && next==row.next);
        }
        ++index;
        options->m_nNightmareMode=1; CString sFury(_T("ZRONE")); rules.GetRandomItem(sFury); options->m_nNightmareMode=0;
        check("random-item-nightmare-uses-fury-table",sFury=="ZRCBOW");

        struct UpgradeCase { const char* item; const char* cell; const char* out; };
        const UpgradeCase upgradeCases[]={
            {"ZRAX_M","up2","ZRAX_v"},{"ZRAX_l","up","ZRAX_m"},{"ZRAX_l","uP1","ZRAX_m"},
            {"ZRAX_l","up3","ZRAX_v"},{"ZRAX_v","up3","ZRAX_v"},{"ZRAX_l","upx","ZRAX_l"},
            {"ZRAX_l","upp","ZRAX_l"},{"ZRAXl","up","ZRAXl"},{"ZRAX_q","up","ZRAX_q"},{"ZRAX_l","down","ZRAX_l"},
        };
        for (const auto& row : upgradeCases) {
            ++index;
            check("upgrade-grade-step",rules.GetUpgradedItem(CString(row.item),CString(row.cell))==row.out);
        }

        ++index; srand(1);
        CItem created(CResRef("ZRTWO"),1,0,0,0,0); INT next=rand();
        check("new-item-rerolled-through-random-table",created.GetResRef()=="ZRCMELEE" && next==18467);
        ++index;
        CItem source; source.cResRef=CResRef("ZRTWO"); srand(1);
        CItem copied(source); next=rand(); source.cResRef=CResRef("");
        check("copied-item-rerolled-through-random-table",copied.GetResRef()=="ZRCMELEE" && next==18467);
        ++index;
        CItem dropped(CResRef("ZRAX_V"),1,0,0,0,0);
        check("no-drop-item-becomes-empty",dropped.GetResRef()=="" && dropped.pRes==NULL);
    }
    options->m_nNightmareMode=nightmare;

    // Only seven-character 00RIITM (unreachable digit test) and 00RTAVE1 are randomized.
    struct ChargeCase { const char* name; WORD count1,count2; WORD out1,out2; INT next; };
    const ChargeCase chargeCases[]={
        {"00RTAVE1",5,3,7,0,18467},{"00RTAVE1",0,3,1,0,41},{"00RTAVE1",5,0,5,0,41},
        {"00RIITM",5,3,5,3,41},{"00RIITM1",5,3,5,3,41},{"00rtave1",5,3,5,3,41},
    };
    for (const auto& row : chargeCases) {
        ++index;
        CItem item; item.m_useCount1=row.count1; item.m_useCount2=row.count2;
        CString sName(row.name); srand(1);
        item.RandomizeUseCount(sName); INT next=rand();
        check("charge-randomizer",item.m_useCount1==row.out1 && item.m_useCount2==row.out2 && next==row.next);
    }
    srand(1);

    Emit("{\"step\":%d,\"op\":\"itemcreationchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"itemcreationchecks","random item / charge creation mismatch");
}
