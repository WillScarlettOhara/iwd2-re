#include "IcewindCVisualEffect.h"

// 0x586A40
IcewindCVisualEffect::IcewindCVisualEffect()
{
    m_field_0 = FALSE;
    m_field_1 = TRUE;
    m_field_2 = FALSE;
    m_field_3 = FALSE;
    m_nTransValue = 0;
    m_dwFlags = 0x20000;
}

// 0x586A60
void IcewindCVisualEffect::SetActive(BOOLEAN a1)
{
    if (a1 == TRUE) {
        m_field_0 = TRUE;
        m_dwFlags |= 0x10000;
    } else {
        m_field_0 = FALSE;
        m_dwFlags &= ~0x10000;
    }
}

// 0x586A90
void IcewindCVisualEffect::SetGlowing(BOOLEAN a1)
{
    if (a1 == TRUE) {
        m_field_1 = TRUE;
        m_dwFlags |= 0x200;
    } else {
        m_field_1 = FALSE;
        m_dwFlags &= ~0x200;
    }
}

// 0x586AC0
void IcewindCVisualEffect::SetTransparent(BOOLEAN a1, INT nTransValue)
{
    if (a1 == TRUE) {
        m_field_3 = TRUE;
        m_nTransValue = nTransValue;
        m_dwFlags |= 0x2;
    } else {
        m_field_3 = FALSE;
        m_nTransValue = 0;
        m_dwFlags &= ~0x2;
    }
}

// Phase 1-2: Scaffold functions
// 0x5867D0
void FUN_005867d0() {
    // TODO: Incomplete.
}

// 0x5868C0
void FUN_005868c0() {
    // TODO: Incomplete.
}

// 0x5868E0
void FUN_005868e0() {
    // TODO: Incomplete.
}

