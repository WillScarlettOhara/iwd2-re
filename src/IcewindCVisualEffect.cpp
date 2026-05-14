#include "IcewindCVisualEffect.h"

// 0x586A40
IcewindCVisualEffect::IcewindCVisualEffect()
{
    m_bTintEnabled = FALSE;
    m_bCopyFromBack = TRUE;
    field_2 = FALSE;
    m_bTransparent = FALSE;
    m_nTransValue = 0;
    m_dwFlags = 0x20000;
}

// 0x586A60
void IcewindCVisualEffect::SetTintFromFlags(BOOLEAN a1)
{
    if (a1 == TRUE) {
        m_bTintEnabled = TRUE;
        m_dwFlags |= 0x10000;
    } else {
        m_bTintEnabled = FALSE;
        m_dwFlags &= ~0x10000;
    }
}

// 0x586A90
void IcewindCVisualEffect::SetCopyFromBack(BOOLEAN a1)
{
    if (a1 == TRUE) {
        m_bCopyFromBack = TRUE;
        m_dwFlags |= 0x200;
    } else {
        m_bCopyFromBack = FALSE;
        m_dwFlags &= ~0x200;
    }
}

// 0x586AC0
void IcewindCVisualEffect::SetTransparency(BOOLEAN a1, INT nTransValue)
{
    if (a1 == TRUE) {
        m_bTransparent = TRUE;
        m_nTransValue = nTransValue;
        m_dwFlags |= 0x2;
    } else {
        m_bTransparent = FALSE;
        m_nTransValue = 0;
        m_dwFlags &= ~0x2;
    }
}
