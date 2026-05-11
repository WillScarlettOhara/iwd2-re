#ifndef CUIMANAGER_H_
#define CUIMANAGER_H_

#include "mfc.h"

#include "BalDataTypes.h"
#include "CResRef.h"

class CUIControlBase;
class CUIPanel;
class CWarp;

class CUIManager {
public:
    static const BYTE MOUSELBUTTON;
    static const BYTE MOUSERBUTTON;
    static const BYTE KEYBOARD;

    CUIManager();
    ~CUIManager();
    void fInit(CWarp* pWarp, CResRef cResRef, BOOL bDoubleSize);
    void fUninit();
    void AddPanel(UI_PANELHEADER* panelInfo);
    void ReorderPanelAfter(DWORD nID1, DWORD nID2);
    void TimerAsynchronousUpdate();
    CUIPanel* GetPanel(DWORD nID);
    void SetCapture(CUIControlBase* pControl, BYTE nType);
    void ClearTooltip();
    void OnMouseMove(CPoint pt);
    void OnLButtonDown(CPoint pt);
    void OnLButtonUp(CPoint pt);
    void OnLButtonDblClk(CPoint pt);
    void OnRButtonDown(CPoint pt);
    void OnRButtonUp(CPoint pt);
    BOOL OnKeyDown(SHORT nKey);
    void Render();
    void InvalidateRect(const CRect* rect);
    void InvalidateCursorRect(const CRect& rect);
    void ForceToolTip();
    void ShiftPanels(const CPoint& pt);
    void KillCapture();

    static BYTE RENDER_COUNT;

    /* 0000 */ BOOL m_bHidden;
    /* 0004 */ BOOL m_bInitialized;
    /* 0008 */ CResRef m_cResRef;
    /* 0010 */ CWarp* m_pWarp; // #guess
    /* 0014 */ CUIControlBase* m_pFocusedControl; // #guess
    /* 0018 */ int m_nStartTime;
    /* 001C */ int m_nDeadline;
    /* 0020 */ int m_nField20;
    /* 0024 */ int m_nTickInterval;
    /* 0028 */ int nm_field_28;
    /* 002C */ unsigned char m_nCaptureType;
    /* 002E */ unsigned char bm_field_2E;
    /* 002D */ unsigned char bm_field_2D;
    /* 0032 */ int nm_field_32;
    /* 0036 */ CCriticalSection pm_field_36;
    /* 0056 */ CCriticalSection m_pField56;
    /* 0076 */ int m_nField76;
    /* 007A */ CRect pfield_7A;
    /* 008A */ CRect pm_field_8A;
    /* 009A */ CRect pfield_9A;
    /* 00AA */ BOOL m_bDoubleSize;
    /* 00AE */ CTypedPtrList<CPtrList, CUIPanel*> m_lPanels; // #guess
};

#endif /* CUIMANAGER_H_ */
