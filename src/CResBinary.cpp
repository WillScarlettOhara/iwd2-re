#include "CResBinary.h"

// 0x4028F0
CResBinary::CResBinary()
{
    // Native 0x4028FE only clears +0x54; +0x50 is written by Demand.
    m_bParsed = FALSE;
}

// 0x402930
CResBinary::~CResBinary()
{
}

// 0x402940
void* CResBinary::Demand()
{
    void* data = CRes::Demand();
    if (m_bParsed && CRes::GetDemands() > 1) {
        return data;
    }
    if (data != NULL) {
        m_resVersion = static_cast<char*>(data) + 4;
        m_bParsed = TRUE;
    }
    return m_bParsed ? data : NULL;
}

// 0x402980
int CResBinary::Release()
{
    int result = CRes::Release();
    if (CRes::GetDemands() <= 0) {
        m_bParsed = FALSE;
    }
    return result;
}
