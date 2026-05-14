#include "CUnknown2.h"

// 0x7C24C0
CUnknown2::CUnknown2()
{
    field_0 = 0;
    field_4 = 0;
    field_8 = 0;
    field_28 = 0;
    field_2C = 0;
    field_30 = 0;
}

// 0x7C24F0
CUnknown2::~CUnknown2()
{
    // NOTE: Uninline.
    CleanUp();
}

// 0x799C90
BOOL CUnknown2::IsReady()
{
    return TRUE;
}

// 0x7C25B0
void CUnknown2::CleanUp()
{
    // NOTE: Based on decompilation at 0x7C25B0.
    // Iterates field_C list, cleans up each data object, and then
    // destroys field_0, field_4, field_8 objects.
    POSITION pos = field_C.GetHeadPosition();
    while (pos != NULL) {
        void* pData = field_C.GetNext(pos);
        delete static_cast<CObject*>(pData);
    }
    field_C.RemoveAll();

    delete reinterpret_cast<CObject*>(field_8);
    field_8 = 0;

    delete reinterpret_cast<CObject*>(field_4);
    field_4 = 0;

    delete reinterpret_cast<CObject*>(field_0);
    field_0 = 0;
}
