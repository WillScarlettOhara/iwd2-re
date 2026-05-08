#include "CCreatureFile.h"

// 0x712370
CCreatureFile::CCreatureFile()
{
}

// 0x4821C0
CCreatureFile::~CCreatureFile()
{
}

// 0x7123B0
BYTE* CCreatureFile::GetData()
{
    if (pRes != NULL) {
        return reinterpret_cast<BYTE*>(pRes->Demand());
    } else {
        return NULL;
    }
}

// 0x7123C0
DWORD CCreatureFile::GetDataSize()
{
    if (pRes == NULL) {
        return 0;
    }

    if (pRes->Demand() == NULL) {
        return 0;
    }

    DWORD nSize = pRes->m_nResSizeActual;

    pRes->Release();

    return nSize;
}

// 0x5C7B00
void CCreatureFile::ReleaseData()
{
    if (pRes != NULL) {
        pRes->Release();
    }
}

// Phase 1-2: Scaffold functions
// 0x481D30
void FUN_00481d30() {
    // TODO: Incomplete.
}

// 0x481EB0
void FUN_00481eb0() {
    // TODO: Incomplete.
}

// 0x482040
void FUN_00482040() {
    // TODO: Incomplete.
}

// 0x482210
void FUN_00482210() {
    // TODO: Incomplete.
}

// 0x5C7B10
void FUN_005c7b10() {
    // TODO: Incomplete.
}

// 0x711C90
void FUN_00711c90() {
    // TODO: Incomplete.
}

// 0x711CB0
void FUN_00711cb0() {
    // TODO: Incomplete.
}

// 0x712310
void FUN_00712310() {
    // TODO: Incomplete.
}

