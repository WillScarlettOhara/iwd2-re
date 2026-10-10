#include "CProjectileSPBOOM.h"
#include "CBaldurChitin.h"
#include "CInfGame.h"
#include "CGameObjectArray.h"

// 0x530300
CProjectileSPBOOM::CProjectileSPBOOM()
    : CProjectileTravelling(CResRef("SPBOOM"))
{
    m_pVidCell->SequenceSet(0);
    m_palette.SetRange(0, 0x43, *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
    m_pVidCell->SetPalette(m_palette);
    m_tinted = 0;
    m_useHeightOffset = 0;
    m_mirror = 0;
    m_hasShadowCell = 0;
    m_fireSoundRef = CResRef("");
    m_dirCount = 1;
}

// 0x5306F0
void CProjectileSPBOOM::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_nTimeStop != 0 && pGame->m_nTimeStopCaster != m_id) return;
    m_pVidCell->FrameAdvance();
    if (m_pVidCell->IsEndOfSequence(FALSE)) {
        RemoveFromArea();
        BYTE rc = pGame->GetObjectArray()->Delete(m_id, CGameObjectArray::THREAD_ASYNCH, NULL, INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            delete this;
            return;
        }
    }
    m_sound.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
}
