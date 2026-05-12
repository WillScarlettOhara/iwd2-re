#include "CDungeonMaster.h"

#include "debuglog.h"

#include "CBaldurChitin.h"
#include "CInfCursor.h"
#include "CScreenConnection.h"
#include "CScreenWorld.h"

// #binary-identical
// 0x5995A0
CDungeonMaster::CDungeonMaster()
{
    SetVideoMode(0);
}

// 0x599610
CDungeonMaster::~CDungeonMaster()
{
}

// 0x599670
void CDungeonMaster::EngineActivated()
{
    DBG("DM::EngineActivated");
    if (CChitin::byte_8FB950
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE
        && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
    }
    DBG("DM::EngineActivated: past network check");

    g_pBaldurChitin->m_pEngineConnection->field_FB0 = 0;
    DBG("DM::EngineActivated: field_FB0 set");
    g_pBaldurChitin->m_pEngineConnection->StartConnection(FALSE);
    DBG("DM::EngineActivated: StartConnection done");
    // FIXME: Skip recursive SelectEngine (crashes)
    // SelectEngine(g_pBaldurChitin->m_pEngineConnection);
    DBG("DM::EngineActivated: done (skipping SelectEngine)");
}

// 0x599700
void CDungeonMaster::EngineDeactivated()
{
    if (CChitin::byte_8FB950
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE
        && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
    }
}

// 0x599750
void CDungeonMaster::EngineInitialized()
{
    g_pBaldurChitin->GetObjectCursor()->Initialize();
}

// 0x599760
void CDungeonMaster::TimerSynchronousUpdate()
{
    pVidMode->EraseScreen(0, 0);
    pVidMode->DoTextOut(0, CString("Master Engine"), 30, 30, RGB(255, 255, 255));
    pVidMode->Flip(TRUE);
}
