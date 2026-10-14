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

// 0x530510
void CProjectileSPBOOM::AddVisualToArea(CGameArea* area, CPoint point)
{
    m_pos=point;
    m_pArea=area;
    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id,this,INFINITE)!=CGameObjectArray::SUCCESS) {
        delete this;
        return;
    }
    CGameObject::AddToArea(area,m_pos,0,LIST_FRONT);
    m_sound.Stop();
    if (m_fireSoundRef!=CResRef("")) {
        m_sound.SetResRef(m_fireSoundRef,TRUE,TRUE);
        if (m_loopFireSound) m_sound.SetLoopingFlag(TRUE);
        m_sound.SetChannel(15,reinterpret_cast<DWORD>(m_pArea));
        m_sound.Play(m_pos.x,m_pos.y,0,FALSE);
    }
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
