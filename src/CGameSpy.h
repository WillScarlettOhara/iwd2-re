#ifndef CGAMESPY_H_
#define CGAMESPY_H_

#include "mfc.h"

// #guess
class CGameSpy {
public:
    CGameSpy();
    ~CGameSpy();
    BOOLEAN ConnectToGameSpy(BOOL a1);
    void sub_4D2060();
    BOOLEAN sub_4D2210();
    BOOLEAN sub_4D2310();

    /* 0000 */ unsigned char m_bWinsockInitialized;
    /* 0001 */ unsigned char m_bConnected;
    /* 0002 */ int nm_field_2;
    /* 0012 */ int nm_field_12;
    /* 0016 */ int nm_field_16;
    /* 001A */ int nm_field_1A;
    /* 001E */ int nm_field_1E;
    /* 0022 */ int nm_field_22;
    /* 0026 */ unsigned char bm_field_26;
    /* 0028 */ int nm_field_28;
};

#endif /* CGAMESPY_H_ */
