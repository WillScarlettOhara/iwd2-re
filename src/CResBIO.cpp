#include "CResBIO.h"

// 0x423790
CResBIO::~CResBIO()
{
}

// 0x4029F0
void* CResBIO::Demand()
{
    void* pData = CRes::Demand();
    if (m_bParsed && GetDemands() > 1) {
        return pData;
    }
    if (pData != NULL) {
        m_bParsed = TRUE;
    }
    return m_bParsed ? pData : NULL;
}

// Shared with CResCHR::Release (0x4022B0).
// 0x4022B0
int CResBIO::Release()
{
    int result = CRes::Release();
    if (GetDemands() <= 0) {
        m_bParsed = FALSE;
    }
    return result;
}
