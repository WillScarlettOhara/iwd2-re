#include "CResINI.h"

// NOTE: Inlined.
CResINI::CResINI()
{
    m_bParsed = FALSE;
}

// NOTE: Inlined.
CResINI::~CResINI()
{
}

// 0x4029B0
void* CResINI::Demand()
{
    void* pData = CRes::Demand();
    if (!m_bParsed || GetDemands() <= 1) {
        // NOTE: Uninline.
        Parse(pData);

        if (!m_bParsed) {
            return NULL;
        }
    }

    return pData;
}

// NOTE: Inlined.
BOOL CResINI::Parse(void* pData)
{
    if (pData == NULL) {
        return FALSE;
    }

    m_bParsed = TRUE;

    return m_bParsed;
}

// Phase 1-2: Scaffold functions
// 0x402980
void FUN_00402980() {
    // TODO: Incomplete.
}

// 0x4029F0
void FUN_004029f0() {
    // TODO: Incomplete.
}

// 0x402A50
void FUN_00402a50() {
    // TODO: Incomplete.
}

