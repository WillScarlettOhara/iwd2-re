#include "CGameSpy.h"

#include <winsock2.h>

// #binary-identical
// 0x4D0930
CGameSpy::CGameSpy()
{
    nm_field_2 = 0;
    nm_field_12 = 0;
    nm_field_16 = 0;
    nm_field_22 = 0;
    m_bConnected = 0;
    nm_field_1E = 0;
    nm_field_1A = 0;
    nm_field_28 = 0;
    m_bWinsockInitialized = 0;
    bm_field_26 = 0;
}

// #binary-identical
// 0x4D0960
CGameSpy::~CGameSpy()
{
    if (m_bWinsockInitialized == 1) {
        WSACleanup();
    }
    m_bWinsockInitialized = 0;
    m_bConnected = 0;
}

// 0x4D1EC0
BOOLEAN CGameSpy::ConnectToGameSpy(BOOL a1)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4D2060
void CGameSpy::sub_4D2060()
{
    // TODO: Incomplete.
}

// 0x4D2210
BOOLEAN CGameSpy::sub_4D2210()
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4D2310
BOOLEAN CGameSpy::sub_4D2310()
{
    // TODO: Incomplete.

    return FALSE;
}

// Phase 1-2: Scaffold functions
// 0x4D0680
void FUN_004d0680() {
    // TODO: Incomplete.
}

// 0x4D06D0
void FUN_004d06d0() {
    // TODO: Incomplete.
}

// 0x4D0980
void FUN_004d0980() {
    // TODO: Incomplete.
}

// 0x4D0B20
void FUN_004d0b20() {
    // TODO: Incomplete.
}

// 0x4D0B80
void FUN_004d0b80() {
    // TODO: Incomplete.
}

// 0x4D0C70
void FUN_004d0c70() {
    // TODO: Incomplete.
}

// 0x4D0DA0
void FUN_004d0da0() {
    // TODO: Incomplete.
}

// 0x4D0F40
void FUN_004d0f40() {
    // TODO: Incomplete.
}

// 0x4D1220
void FUN_004d1220() {
    // TODO: Incomplete.
}

// 0x4D15D0
void FUN_004d15d0() {
    // TODO: Incomplete.
}

