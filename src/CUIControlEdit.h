#ifndef CUICONTROLEDIT_H_
#define CUICONTROLEDIT_H_

#include "CUIControlBase.h"
#include "CVidCell.h"
#include "CVidFont.h"
#include "CVidMosaic.h"

class CUIControlEdit : public CUIControlBase {
public:
    CUIControlEdit(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo, int a3);
    /* 0000 */ ~CUIControlEdit() override;
    /* 0004 */ void SetActive(BOOLEAN bActive) override;
    /* 000C */ void KillFocus() override;
    /* 0010 */ void SetFocus() override;
    /* 0018 */ BOOL OnLButtonDown(CPoint pt) override;
    /* 0024 */ BOOL OnRButtonDown(CPoint pt) override;
    /* 002C */ void OnKeyDown(SHORT nKey) override;
    /* 0050 */ void InvalidateRect() override;
    /* 0058 */ BOOL Render(BOOL bForce) override;
    /* 0064 */ BOOL NeedRender() override;
    /* 0068 */ virtual void OnEditReturn(CString sText);
    /* 006C */ virtual CString GetText();
    /* 0070 */ virtual void SetText(CString sText);

    void AdjustVisibleIndex();

    /* 0052 */ CVidMosaic m_mosBackground;
    /* 0106 */ CVidMosaic m_mosTextFocusedBackground;
    /* 01BA */ CVidMosaic m_mosTextUnFocusedBackground;
    /* 026E */ CVidCell m_vcCursor;
    /* 0348 */ CPoint m_ptText;
    /* 0350 */ int m_nTextOffsetX;
    /* 0354 */ int m_nTextOffsetY;
    /* 0358 */ CVidFont m_cVidFont;
    /* 0858 */ short m_nMaxLength;
    /* 085A */ short m_nTextCapitalization;
    /* 085C */ short m_nEditMode;
    /* 085E */ SHORT m_nRenderCount;
    /* 0860 */ BOOL m_bFocused;
    /* 0864 */ CString m_sText;
    /* 0868 */ CString m_sTextOriginal;
    /* 086C */ SHORT m_nCursorIndex;
    /* 086E */ int m_nVisibleIndex;
    /* 0872 */ unsigned char m_nCommandHistoryIndex;
    /* 0873 */ unsigned char m_nCommandHistorySize;
    /* 0874 */ CString sm_field_874[10];
    /* 089C */ int m_bConfirmOnExit;
    /* 08A0 */ unsigned char m_bDefocusOnReturn;
    /* 08A1 */ unsigned char m_bFontNeedsLoading;
};

#endif /* CUICONTROLEDIT_H_ */
