#ifndef CIMM_H_
#define CIMM_H_

#include "mfc.h"

class CVidFont;
class CVidMosaic;

// #guess
class CImm {
public:
    // NOTE: Constructor is inlined in `CChitin` constructor. This is the only
    // way to explain how assignments appear before other static object
    // constructor calls.
    CImm()
    {
        m_dwState = 0;
        field_12C = FALSE;
        field_128 = 0;
        m_hKeyboardLayout = NULL;
        field_13C = 0;
    }

    ~CImm();

    void OnStartComposition();
    void OnEndComposition();
    void OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void ReadCompositionString(HWND hWnd, LPARAM lParam);
    void ReadResultString(HWND hWnd, BOOL a3);
    BOOL OnNotify(HWND hWnd, WPARAM wPara, LPARAM lParam);
    void HandleCandidateList(HWND hWnd, LPARAM lParam);
    CString ParseCandidateList(LPCANDIDATELIST lpCandidateList);
    void ActivateNativeIME(HWND hWnd);
    void DeactivateNativeIME(HWND hWnd);
    CString GetIMEStatusText(HWND hWnd);
    CRect GetCompositionWindowRect(const CPoint& ptPanel, const CSize& panelSize, const CPoint& ptEdit, const CSize& editSize, SHORT nFontHeight);
    void DrawCompositionWindow(const CRect& a1, const CRect& a2, CVidFont* pVidFont, BOOL bDemanded);
    CString GetCandidateList();
    CString GetCompositionString();
    void CleanUp();
    CString ConsumeResultString();

    static const CSize PADDING;

    /* 0000 */ DWORD m_dwState;
    /* 0104 */ HKL m_hKeyboardLayout;
    /* 0108 */ CCriticalSection m_cCriticalSection;
    /* 0128 */ int field_128;
    /* 012C */ BOOL field_12C;
    /* 0130 */ CString field_130;
    /* 0134 */ CString field_134;
    /* 0138 */ CString m_sCandidateList;
    /* 013C */ CVidMosaic* field_13C;
};

#endif /* CIMM_H_ */
