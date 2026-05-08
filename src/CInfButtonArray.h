#ifndef CINFBUTTONARRAY_H_
#define CINFBUTTONARRAY_H_

#include "CVidCell.h"

class CButtonData;

class CInfButtonSettings {
public:
    CInfButtonSettings();
    ~CInfButtonSettings();

    /* 0000 */ int nm_field_0;
    /* 0004 */ int nm_field_4;
    /* 0008 */ int nm_field_8;
    /* 000C */ int nfield_C;
    /* 0010 */ int nm_field_10;
    /* 0014 */ CVidCell pm_field_14;
    /* 00EE */ CVidCell pfield_EE;
    /* 01C8 */ int nm_field_1C8;
    /* 01CC */ int nm_field_1CC;
    /* 01D0 */ int nm_field_1D0;
    /* 01D8 */ int nm_field_1D8;
    /* 01DC */ BOOL m_bGreyOut;
};

class CInfButtonArray {
public:
    static const BYTE STATE_NONE;

    CInfButtonArray();

    static void GetSelectedQuickWeaponData(CButtonData& cButtonData);
    static BYTE GetSelectedModalMode();
    BYTE GetButtonId(INT buttonType);
    BOOL ResetState();
    void UpdateState();
    BOOL SetState(INT nState, int a2);
    void UpdateButtons();
    void OnLButtonPressed(int buttonID);
    void OnRButtonPressed(int buttonID);

    void SetSelectedButton(INT nSelectedButton);

    /* 0000 */ CInfButtonSettings m_buttonArray[12];
    /* 16B0 */ INT m_buttonTypes[12];
    /* 16E8 */ CVidCell pm_field_16E8;
    /* 17C2 */ CVidCell pm_field_17C2;
    /* 189C */ CVidCell pm_field_189C;
    /* 197E */ INT m_nSelectedButton;
    /* 1982 */ int m_nState;
    /* 19B2 */ int nm_field_19B2;
};

#endif /* CINFBUTTONARRAY_H_ */
