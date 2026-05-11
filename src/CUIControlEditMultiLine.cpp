#include "CUIControlEditMultiLine.h"

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CUIControlScrollBar.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidInf.h"

// 0x4D9410
CUIControlEditMultiLine::CUIControlEditMultiLine(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo, int a3)
    : CUIControlBase(panel, &(controlInfo->base), 0)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ChUIControls.cpp
    // __LINE__: 2458
    UTIL_ASSERT(panel != NULL && controlInfo != NULL);

    if (m_pPanel->m_pManager->m_bDoubleSize) {
        m_ptText.x = 2 * controlInfo->x;
        m_ptText.y = 2 * controlInfo->y;
    } else {
        m_ptText.x = controlInfo->x;
        m_ptText.y = controlInfo->y;
    }

    nm_field_350 = 0;
    nm_field_354 = 0;
    m_nTextCapitalization = controlInfo->nTextCapitalization;
    wm_field_85A = controlInfo->nfield_68;
    m_nRenderCount = 0;
    m_bFocused = FALSE;
    m_sText = controlInfo->initialText;
    nm_field_86A = -1;
    nm_field_86E = 0;
    bm_field_877 = 1;

    CResRef cursorResRef(controlInfo->refCursor);
    m_vcCursor.SetResRef(cursorResRef, m_pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);

    m_vcCursor.m_bDoubleSize = m_pPanel->m_pManager->m_bDoubleSize;

    m_vcCursor.SequenceSet(controlInfo->nSequence);
    m_vcCursor.FrameSet(controlInfo->nFrame);

    m_cVidFont.SetResRef(CResRef(controlInfo->refFont), m_pPanel->m_pManager->m_bDoubleSize, FALSE);
    m_cVidFont.SetColor(RGB(200, 200, 200), RGB(60, 60, 60), 0);

    nm_field_872 = a3;
    nm_field_88A = -1;
    nm_field_88E = -1;
    wm_field_87A = 0;
    wm_field_878 = 0;
    bm_field_888 = 0;
    m_rgbForegroundColor = RGB(200, 200, 200);
    m_rgbBackgroundColor = RGB(60, 60, 60);
    pm_field_36 = 1;
    nm_field_876 = 1;

    // NOTE: Uninline.
    SetNeedMouseMove();

    m_size.cx -= m_ptText.x;
    m_size.cy -= m_ptText.y;
}

// 0x4D96C0
void CUIControlEditMultiLine::SetColor(COLORREF rgbForegroundColor, COLORREF rgbBackgroundColor)
{
    m_rgbForegroundColor = rgbForegroundColor;
    m_rgbBackgroundColor = rgbBackgroundColor;
}

// 0x4D96E0
void CUIControlEditMultiLine::OnEditReturn(CString sText)
{
}

// 0x4D9740
CUIControlEditMultiLine::~CUIControlEditMultiLine()
{
}

// 0x4D9800
void CUIControlEditMultiLine::Remove()
{
    m_sText.Empty();
    nm_field_86A = 0;
    InvalidateRect();
}

// 0x4D9830
void CUIControlEditMultiLine::ReConstitute()
{
    // TODO: Incomplete.
}

// 0x4D9D00
void CUIControlEditMultiLine::CalculateNewLines(BYTE a1)
{
    // TODO: Incomplete.
}

// 0x4DAA10
CString CUIControlEditMultiLine::GetText()
{
    CString sText;
    sText.Empty();

    if (m_sText.GetLength() > 0) {
        int start = 0;
        int end;
        do {
            end = CUtil::Find(m_sText, '\r', start);
            if (end == -1) {
                end = m_sText.GetLength();
            }

            sText += m_sText.Mid(start, end);
            start = end + 1;
        } while (end < m_sText.GetLength());
    }

    return sText;
}

// 0x4DAAF0
void CUIControlEditMultiLine::KillFocus()
{
    // FIXME: Unused.
    CWnd* pWnd = g_pChitin->GetWnd();
    if (g_pChitin->cImm.nm_field_128) {
        g_pChitin->cImm.DeactivateNativeIME(g_pChitin->GetWnd()->GetSafeHwnd());
        m_pPanel->InvalidateRect(&(m_pPanel->m_rImeSuggestionsFrame));
        m_pPanel->m_rImeSuggestionsFrame.SetRectEmpty();
        m_pPanel->m_pImeSuggestionsFont = NULL;
    }

    static_cast<CBaldurEngine*>(m_pPanel->m_pManager->m_pWarp)->DisableEditKeys();

    m_bFocused = FALSE;
    InvalidateRect();
}

// 0x4DAB80
void CUIControlEditMultiLine::SetFocus()
{
    m_bFocused = TRUE;
    sm_field_866 = m_sText;

    InvalidateRect();

    static_cast<CBaldurEngine*>(m_pPanel->m_pManager->m_pWarp)->EnableEditKeys(wm_field_85A);
}

// 0x4DABD0
BOOL CUIControlEditMultiLine::OnLButtonDown(CPoint pt)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4DB070
void CUIControlEditMultiLine::OnLButtonUp(CPoint pt)
{
    if (m_bActive) {
        bm_field_888 = 0;
        m_pPanel->m_pManager->SetCapture(this, 2);
        if (nm_field_88E == nm_field_88A) {
            nm_field_86A = nm_field_88E;
            nm_field_88E = -1;
        }

        InvalidateRect();
    }
}

// 0x4DB0C0
BOOL CUIControlEditMultiLine::OnRButtonDown(CPoint pt)
{
    if (!m_bActive) {
        return FALSE;
    }

    int x = pt.x - (m_ptText.x + m_ptOrigin.x);
    int y = pt.y - (m_ptText.y + m_ptOrigin.y);

    if (x < 0 || x >= m_size.cx || y < 0 || y >= m_size.cy) {
        if (m_bFocused == TRUE) {
            if (nm_field_872) {
                m_sText = sm_field_866;
                ReConstitute();
            }

            // NOTE: Uninline.
            m_pPanel->m_pManager->KillCapture();
        }
    }

    return TRUE;
}

// 0x4DB160
void CUIControlEditMultiLine::OnKeyDown(SHORT nKey)
{
    // TODO: Incomplete.
}

// 0x4E05B0
void CUIControlEditMultiLine::OnMouseMove(CPoint pt)
{
    // TODO: Incomplete.
}

// 0x4E09F0
BOOL CUIControlEditMultiLine::NeedRender()
{
    return (m_bActive || m_bInactiveRender) && m_nRenderCount > 0;
}

// 0x4E0A20
BOOL CUIControlEditMultiLine::Render(BOOL bForce)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4E1580
void CUIControlEditMultiLine::InvalidateRect()
{
    if (m_bActive || m_bInactiveRender) {
        CSingleLock lock(&(m_pPanel->m_pManager->pfield_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount = CUIManager::RENDER_COUNT;
        lock.Unlock();
    }
}

// 0x4E1610
void CUIControlEditMultiLine::AdjustVisibleIndex()
{
    if (wm_field_87A > wm_field_878) {
        if (nm_field_86A == 0) {
            nm_field_86E = 0;
        }

        if (nm_field_86A != m_sText.GetLength()) {
            int numberOfLines = 0;
            if (m_sText.GetLength() > 0) {
                int start = 0;
                int end;
                do {
                    end = CUtil::FindOneOf(m_sText.Mid(start), CString("\n\r"), 0);
                    if (end == -1) {
                        end = m_sText.GetLength();
                    }

                    if (start <= nm_field_86A && nm_field_86A <= end + start) {
                        break;
                    }

                    start += end + 1;
                    numberOfLines++;

                    if (start >= m_sText.GetLength()) {
                        start = m_sText.GetLength();
                    }
                } while (end < m_sText.GetLength());
            }

            if (nm_field_86E >= numberOfLines || numberOfLines >= nm_field_86E + wm_field_878) {
                if (nm_field_86E < numberOfLines) {
                    nm_field_86E = max(numberOfLines - wm_field_878 + 1, 0);
                } else {
                    nm_field_86E = numberOfLines;
                }
            } else {
                if (wm_field_87A - nm_field_86E < wm_field_878) {
                    nm_field_86E = wm_field_87A - wm_field_878;
                }

                if (nm_field_86E < numberOfLines) {
                    nm_field_86E = max(numberOfLines - wm_field_878 + 1, 0);
                } else {
                    nm_field_86E = numberOfLines;
                }
            }
        } else {
            nm_field_86E = wm_field_87A - wm_field_878;
        }
    } else {
        nm_field_86E = 0;
    }
}

// 0x4E17B0
void CUIControlEditMultiLine::SetText(CString sText)
{
    sm_field_866 = sText;
    if (sText != m_sText) {
        m_sText = sText;
        nm_field_86A = 0;
        if (sText.GetLength() != 0) {
            CalculateNewLines(3);
            InvalidateRect();
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4E1860
CUIControlEditMultiLineScroller::CUIControlEditMultiLineScroller(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo)
    : CUIControlEditMultiLine(panel, controlInfo, 0)
{
    bm_field_896 = 0;
    bm_field_897 = 0;
    bm_field_898 = 0;
    bm_field_899 = 0;
    nm_field_892 = pm_field_36;
}

// 0x4E18D0
CUIControlEditMultiLineScroller::~CUIControlEditMultiLineScroller()
{
}

// 0x4E1990
void CUIControlEditMultiLineScroller::AdjustVisibleIndex()
{
    CUIControlEditMultiLine::AdjustVisibleIndex();

    if (nm_field_892 != -1) {
        CUIControlScrollBar* pScrollBar = static_cast<CUIControlScrollBar*>(m_pPanel->GetControl(nm_field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(wm_field_878, wm_field_878, wm_field_87A);
    }
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnScroll(SHORT a1, SHORT a2)
{
    if (wm_field_87A < wm_field_878) {
        return;
    }

    // TODO: Check casts.
    int v1 = static_cast<int>(static_cast<float>(a1) / static_cast<float>(a2) * (wm_field_87A - wm_field_878));

    if (v1 == nm_field_86E || v1 < 0 || v1 > wm_field_87A - wm_field_878) {
        return;
    }

    nm_field_86E = v1;

    if (nm_field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(nm_field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(nm_field_86E, wm_field_87A, wm_field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnScrollUp()
{
    nm_field_86E--;
    if (nm_field_86E < 0) {
        nm_field_86E = 0;
    }

    if (nm_field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(nm_field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(nm_field_86E, wm_field_87A, wm_field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnScrollDown()
{
    nm_field_86E++;
    if (nm_field_86E > wm_field_87A - wm_field_878) {
        nm_field_86E = wm_field_87A - wm_field_878;
    }

    if (nm_field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(nm_field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(nm_field_86E, wm_field_87A, wm_field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnPageUp(DWORD nLines)
{
    SHORT v1 = static_cast<SHORT>(nLines);
    if (v1 > wm_field_878) {
        v1 = wm_field_878;
    }

    nm_field_86E -= v1;
    if (nm_field_86E < 0) {
        nm_field_86E = 0;
    }

    if (nm_field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(nm_field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(nm_field_86E, wm_field_87A, wm_field_878);
    }

    InvalidateRect();
}

// NOTE: Uninline.
void CUIControlEditMultiLineScroller::OnPageDown(DWORD nLines)
{
    if (wm_field_87A < wm_field_878) {
        nm_field_86E = 0;
    } else {
        SHORT v1 = static_cast<SHORT>(nLines);
        if (v1 > wm_field_878) {
            v1 = wm_field_878;
        }

        nm_field_86E += v1;
        if (nm_field_86E > wm_field_87A - wm_field_878) {
            nm_field_86E = wm_field_87A - wm_field_878;
        }
    }

    if (nm_field_892 != -1) {
        CUIControlEditScrollBar* pScrollBar = static_cast<CUIControlEditScrollBar*>(m_pPanel->GetControl(nm_field_892));

        // NOTE: Uninline.
        pScrollBar->AdjustScrollBar(nm_field_86E, wm_field_87A, wm_field_878);
    }

    InvalidateRect();
}

// Phase 1-2: Scaffold functions
// 0x4D9720
void FUN_004d9720() {
    // TODO: Incomplete.
}

// 0x4E0A4C
void FUN_004e0a4c() {
    // TODO: Incomplete.
}

// 0x4E18B0
void FUN_004e18b0() {
    // TODO: Incomplete.
}

