#include "CInfCursor.h"

#include "CBaldurChitin.h"
#include "CInfGame.h"
#include "CUIControlBase.h"
#include "CUtil.h"
#include "CVidInf.h"

// 0x596C10
CInfCursor::CInfCursor()
{
    m_bVisible = FALSE;
    nfield_9F2 = 0;
    nfield_9EE = 0;
    pfield_A02 = NULL;
    m_nDirection = 0;
    m_nState = 0;
    bAnimatingCustom = 0;
    m_nCurrentCursor = -1;
}

// 0x596CD0
CInfCursor::~CInfCursor()
{
}

// 0x596DB0
void CInfCursor::CursorUpdate(CVidMode* pVidMode)
{
    if (pVidMode == NULL) {
        return;
    }

    if (g_pBaldurChitin->nm_m_field_1932) {
        return;
    }

    if (m_bVisible) {
        switch (m_nCurrentCursor) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 8:
        case 10:
        case 12:
        case 14:
        case 16:
        case 18:
        case 20:
        case 22:
        case 24:
        case 26:
        case 28:
        case 30:
        case 32:
        case 34:
        case 36:
        case 38:
        case 40:
        case 42:
        case 44:
        case 46:
            if (m_nState) {
                m_vcCursors.SequenceSet(m_nCurrentCursor + 1);
            } else {
                m_vcCursors.SequenceSet(m_nCurrentCursor);
            }

            m_vcArrow.FrameAdvance();

            g_pBaldurChitin->m_bPointerUpdated = FALSE;
            break;
        case 100:
            // NOTE: Uninline.
            SetScrollArrowDirection();
            m_vcArrow.FrameAdvance();

            g_pBaldurChitin->m_bPointerUpdated = FALSE;
            break;
        case 101:
            m_vcToolTip.FrameAdvance();

            g_pBaldurChitin->m_bPointerUpdated = FALSE;
            break;
        case 102:
            if (m_vcCustom.pRes == NULL) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
                // __LINE__:
                UTIL_ASSERT(FALSE);
            }

            if (bAnimatingCustom) {
                m_vcCustom.FrameAdvance();
                g_pBaldurChitin->m_bPointerUpdated = FALSE;
            }

            break;
        default:
            pVidMode->SetPointer(NULL, FALSE, -1);
            break;
        }
    } else {
        pVidMode->SetPointer(NULL, FALSE, -1);
        pVidMode->m_bPointerEnabled = FALSE;
    }
}

// 0x596FF0
CVidCell* CInfCursor::GetCurrentCursor()
{
    switch (m_nCurrentCursor) {
    case 100:
        return &m_vcArrow;
    case 101:
        return &m_vcToolTip;
    case 102:
        return &m_vcCustom;
    }

    return &m_vcCursors;
}

// 0x597020
void CInfCursor::Initialize()
{
    // NOTE: Uninline.
    m_vcArrow.SetResRef(CResRef("CURSARW"), FALSE, TRUE, TRUE);

    // NOTE: Uninline.
    m_vcCursors.SetResRef(CResRef("CURSORS"), FALSE, TRUE, TRUE);

    // NOTE: Uninline.
    m_vcToolTip.SetResRef(CResRef("TOOLTIP"), FALSE, TRUE, TRUE);

    m_vcToolTip.Initialize();

    m_bVisible = TRUE;
}

// 0x5972E0
void CInfCursor::SetCursor(INT nNewCursor, BOOLEAN bForce)
{
    CVidMode* pVidMode = g_pBaldurChitin->GetCurrentVideoMode();

    // NOTE: Uninline.
    SetGreyScale(FALSE);

    if (bForce || m_nCurrentCursor != nNewCursor) {
        switch (nNewCursor) {
        case 100:
            SetScrollArrowDirection();

            if (pVidMode->GetPointer() != &m_vcArrow) {
                if (pVidMode->GetPointer() == &m_vcToolTip) {
                    if (pfield_A02 != NULL) {
                        pfield_A02->ResetToolTip();
                        pfield_A02 = NULL;
                    }
                }

                m_vcArrow.FrameSet(0);
                pVidMode->SetPointer(&m_vcArrow, TRUE, -1);
            }
            break;
        case 101:
            if (pVidMode->GetPointer() != &m_vcToolTip) {
                m_vcToolTip.wfield_5E2 = 3;
                m_vcToolTip.wfield_5DA = 0;
                m_vcToolTip.wfield_5DC = 0;

                CSize size1;
                m_vcToolTip.GetFrameSize(1, 0, size1, FALSE);

                CSize size2;
                m_vcToolTip.GetFrameSize(2, m_vcToolTip.wfield_5DC, size2, FALSE);

                // TODO: Check cast.
                m_vcToolTip.m_wField5DE = static_cast<short>((size1.cx / 2) + (size2.cx / 2));
                m_vcToolTip.wfield_5E2 = 0;

                pVidMode->SetPointer(&m_vcToolTip, TRUE, -1);
                pVidMode->m_bPointerEnabled = TRUE;
                g_pBaldurChitin->m_bPointerUpdated = FALSE;
            }
            break;
        case 102:
            if (pVidMode->GetPointer() != &m_vcCustom) {
                if (pVidMode->GetPointer() == &m_vcToolTip) {
                    if (pfield_A02 != NULL) {
                        pfield_A02->ResetToolTip();
                        pfield_A02 = NULL;
                    }
                }

                pVidMode->SetPointer(&m_vcCustom, bAnimatingCustom, -1);
                g_pBaldurChitin->m_bPointerUpdated = FALSE;
            }
            break;
        default:
            if (pVidMode->GetPointer() != &m_vcCursors) {
                if (pVidMode->GetPointer() == &m_vcToolTip) {
                    if (pfield_A02 != NULL) {
                        pfield_A02->ResetToolTip();
                        pfield_A02 = NULL;
                    }
                }

                m_vcCursors.FrameSet(0);

                pVidMode->SetPointer(&m_vcCursors, 0, -1);
                pVidMode->m_bPointerEnabled = TRUE;
                g_pBaldurChitin->m_bPointerUpdated = FALSE;
            }
            break;
        }

        m_nCurrentCursor = nNewCursor;
    }
}

// 0x597610
void CInfCursor::SetGreyScale(BOOLEAN bGreyScale)
{
    if (bGreyScale) {
        // NOTE: Uninline.
        CVidCell* vc = GetCurrentCursor();
        vc->SetTintColor(RGB(180, 180, 180));

        g_pBaldurChitin->GetCurrentVideoMode()->m_dwCursorRenderFlags |= 0xA0000;
    } else {
        // NOTE: Uninline.
        CVidCell* vc = GetCurrentCursor();
        vc->SetTintColor(RGB(255, 255, 255));

        g_pBaldurChitin->GetCurrentVideoMode()->m_dwCursorRenderFlags &= ~0xA0000;
    }
}

// FIXME: `toolTipRef` should not be reference.
//
// 0x5976E0
void CInfCursor::SetToolTip(const STRREF& toolTipRef, CUIControlBase* pControl, const CString& sExtraText)
{
    if (toolTipRef != -1 || sExtraText != "") {
        m_vcToolTip.SetTextRef(toolTipRef, sExtraText);
        pfield_A02 = pControl;
        g_pBaldurChitin->GetObjectGame()->SetTempCursor(101);
    }
}

// FIXME: `nStrRef`, `nSecondaryStrRef`, and `nTertiaryStrRef` should not be reference.
//
// 0x597740
void CInfCursor::SetToolTip(const STRREF& nStrRef, CUIControlBase* pControl, SHORT nHotKeyIndex1, SHORT nHotKeyIndex2, CString sKey, const STRREF& nSecondaryStrRef, const STRREF& nTertiaryStrRef)
{
    if (nStrRef == -1) {
        return;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bHotkeysOnToolTips) {
        char ch = '\0';
        STR_RES strRes;

        if (nHotKeyIndex1 != -1) {
            // NOTE: Uninline.
            char nKey = g_pBaldurChitin->GetObjectGame()->GetKeymap(nHotKeyIndex1);

            ch = g_pBaldurChitin->GetObjectGame()->VirtualKeyToChar(nKey);

            if (ch != '\0' && ch != ' ') {
                sKey = CString(ch);
                if (g_pBaldurChitin->GetObjectGame()->m_pKeymapFlags[nHotKeyIndex1]) {
                    if (g_pBaldurChitin->GetTlkTable().Fetch(24638, strRes)) {
                        sKey = strRes.szText + sKey;
                    } else {
                        sKey = CString("CTRL-") + sKey;
                    }
                }
            }
        }

        if (nHotKeyIndex2 != -1 && (ch == '\0' || ch == ' ')) {
            // NOTE: Uninline.
            char nKey = g_pBaldurChitin->GetObjectGame()->GetKeymap(nHotKeyIndex2);

            if (nKey != '\0' && nKey != ' ') {
                sKey = CString(nKey);
                if (g_pBaldurChitin->GetObjectGame()->m_pKeymapFlags[nHotKeyIndex2]) {
                    if (g_pBaldurChitin->GetTlkTable().Fetch(24638, strRes)) {
                        sKey = strRes.szText + sKey;
                    } else {
                        sKey = CString("CTRL-") + sKey;
                    }
                }
            }
        }

        if (sKey != "") {
            sKey = " (" + sKey + ")";
        }
    } else {
        sKey = "";
    }

    m_vcToolTip.SetTextRef(sKey, nStrRef, nSecondaryStrRef, nTertiaryStrRef);
    pfield_A02 = pControl;
    g_pBaldurChitin->GetObjectGame()->SetTempCursor(101);
}

// 0x597C50
INT CInfCursor::SetCustomCursor(CResRef customResRef, BOOLEAN bAnimating, INT nPointerNumber)
{
    INT nOldCursor = m_nCurrentCursor;

    GetCurrentCursor()->SetTintColor(RGB(255, 255, 255));
    g_pBaldurChitin->GetCurrentVideoMode()->m_dwCursorRenderFlags &= ~0xA0000;
    m_nCurrentCursor = 102;
    bAnimatingCustom = bAnimating;
    m_vcCustom.SequenceSet(0);

    if (g_pBaldurChitin->GetCurrentVideoMode()->GetPointer() != &m_vcCustom
        || m_vcCustom.GetResRef() != customResRef
        || g_pBaldurChitin->GetCurrentVideoMode()->GetPointerNumber() != nPointerNumber) {
        if (g_pBaldurChitin->GetCurrentVideoMode()->GetPointer() == &m_vcToolTip) {
            if (pfield_A02 != NULL) {
                pfield_A02->ResetToolTip();
                pfield_A02 = NULL;
            }
        }

        if (g_pBaldurChitin->GetCurrentVideoMode()->SetPointer(&m_vcCustom, customResRef, bAnimatingCustom, nPointerNumber)) {
            g_pBaldurChitin->m_bPointerUpdated = FALSE;
        } else {
            g_pBaldurChitin->GetCurrentVideoMode()->SetPointer(&m_vcCustom,
                CResRef("NOICON"),
                bAnimatingCustom,
                nPointerNumber);
            SetCursor(0, FALSE);
        }
    }

    return nOldCursor;
}

// NOTE: Inlined.
void CInfCursor::SetScrollArrowDirection()
{
    switch (m_nDirection) {
    case 0:
        m_vcArrow.SequenceSet(2);
        break;
    case 1:
        m_vcArrow.SequenceSet(1);
        break;
    case 2:
        m_vcArrow.SequenceSet(0);
        break;
    case 3:
        m_vcArrow.SequenceSet(7);
        break;
    case 4:
        m_vcArrow.SequenceSet(6);
        break;
    case 5:
        m_vcArrow.SequenceSet(5);
        break;
    case 6:
        m_vcArrow.SequenceSet(4);
        break;
    default:
        m_vcArrow.SequenceSet(3);
        break;
    }
}

// 0x597E20
CInfToolTip::CInfToolTip()
{
    wfield_5DA = 0;
    wfield_5DC = 0;
    m_wField5DE = 0;
    wfield_5E0 = 0;
    wfield_5E2 = 0;
    wfield_5E4 = 0;
    wfield_5E6 = 0;
    wfield_5E8 = 0;
    wfield_5EA = 0;
}

// 0x597EE0
void CInfToolTip::Initialize()
{
    SequenceSet(0);
    wfield_5E2 = 2;

    m_font.SetResRef(CResRef("TOOLFONT"), FALSE, TRUE);
    m_font.SetColor(RGB(255, 255, 255), RGB(14, 14, 14), TRUE);
    wfield_5E4 = 256;
    wfield_5DA = 0;
    wfield_5DC = 0;

    CSize size1;
    GetFrameSize(1, 0, size1, FALSE);

    CSize size2;
    GetFrameSize(2, 0, size2, FALSE);

    // TODO: Check cast.
    m_wField5DE = static_cast<short>((size2.cx / 2) + (size1.cx / 2));
    wfield_5E0 = m_wField5DE;

    wfield_5E8 = GetSequenceLength(1, FALSE);
    wfield_5E6 = 0;
    wfield_5EA = GetSequenceLength(2, FALSE);
}

// 0x597FD0
BOOL CInfToolTip::FrameAdvance()
{
    CSize frameSize1;
    CSize frameSize2;

    switch (wfield_5E2) {
    case 0:
        wfield_5DA++;
        if (wfield_5DA >= wfield_5E8) {
            wfield_5DA = 0;
        }

        wfield_5DC++;
        if (wfield_5DC >= wfield_5EA) {
            wfield_5DC = 0;
        }

        m_wField5DE += 8;

        GetFrameSize(1, wfield_5DA, frameSize1, FALSE);
        GetFrameSize(2, wfield_5DC, frameSize2, FALSE);

        m_wField5DE = wfield_5E6 + static_cast<SHORT>(frameSize1.cx / 2 + frameSize2.cx / 2);
        wfield_5E2 = 2;
        break;
    case 1:
        wfield_5DA--;
        if (wfield_5DA < 0) {
            wfield_5DA = 0;
        }

        wfield_5DC--;
        if (wfield_5DC < 0) {
            wfield_5DC = 0;
        }

        m_wField5DE--;

        GetFrameSize(1, wfield_5DA, frameSize1, FALSE);
        GetFrameSize(2, wfield_5DC, frameSize2, FALSE);

        if (m_wField5DE <= frameSize1.cx / 2 + frameSize2.cx / 2) {
            wfield_5E2 = 3;
        }
        break;
    case 2:
        GetFrameSize(1, wfield_5DA, frameSize1, FALSE);
        GetFrameSize(2, wfield_5DC, frameSize2, FALSE);

        m_wField5DE = wfield_5E6 + static_cast<SHORT>(frameSize1.cx / 2 + frameSize2.cx / 2);
        break;
    case 3:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
        // __LINE__: 884
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// 0x5981D0
BOOL CInfToolTip::GetFrame()
{
    // NOTE: Original code is slightly different. There is some inlining, not
    // sure which.
    BAMHEADER* pHeader = pRes->m_bCacheHeader
        ? pRes->m_pBamHeaderCopy
        : pRes->m_pBamHeader;

    SHORT nSequences = pHeader->nSequences;
    if (nSequences == 0) {
        nSequences = 256;
    }

    if (m_nCurrentSequence >= nSequences) {
        m_nCurrentSequence = 0;
    }

    SHORT nFrames = pRes->m_pSequences[m_nCurrentSequence].nFrames;
    if (m_nCurrentFrame >= nFrames) {
        if (m_bCyclic != 0) {
            m_nCurrentFrame = 0;
        } else {
            m_nCurrentFrame--;
        }
    }

    if (m_nCurrentFrame < 0) {
        if (m_bCyclic != 0) {
            m_nCurrentFrame = nFrames - 1;
        } else {
            m_nCurrentFrame = 0;
        }
    }

    m_pFrame = pRes->GetFrame(m_nCurrentSequence, m_nCurrentFrame, m_bDoubleSize);

    return m_pFrame != NULL;
}

// 0x5982C0
BOOL CInfToolTip::Render(INT nSurface, int x, int y, const CRect& rClip, CVidPoly* pClipPoly, int nPolys, DWORD dwFlags, int nTransVal)
{
    CVidInf* pVidInf = static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode());

    CSize size2;
    GetFrameSize(2, wfield_5DC, size2, TRUE);

    CSize size1;
    GetFrameSize(1, wfield_5DA, size1, TRUE);

    LONG nWidth1 = size1.cx / 2;
    LONG nWidth2 = size2.cx / 2;

    SequenceSet(0);
    FrameSet(0);

    if (pRes->Demand() == NULL) {
        return FALSE;
    }

    RealizePalette(dwFlags);

    CSize frameSize;
    GetCurrentFrameSize(frameSize, TRUE);

    CRect rFXRect(0, 0, nWidth1 + nWidth2 + wfield_5E0, size2.cy);
    CRect rFXClip(0, 0, 0, 0);
    DWORD dwRenderFlags = dwFlags | 0x100 | 0x1;

    pVidInf->FXPrep(rFXRect, dwRenderFlags, CPoint(0, 0), CPoint(0, 0), rFXClip);

    if (!pVidInf->FXLock(rFXRect, dwRenderFlags)) {
        pRes->Release();
        return FALSE;
    }

    CPoint ptCenter;
    GetCurrentCenterPoint(ptCenter, TRUE);

    rFXClip.left = nWidth1;
    rFXClip.top = 0;
    rFXClip.right = rFXRect.Width() - nWidth2;
    rFXClip.bottom = rFXRect.Height();
    if (!pVidInf->FXRender(this, ptCenter.x - (rFXRect.left + frameSize.cx - rFXRect.right) / 2, ptCenter.y, rFXClip, TRUE, dwRenderFlags)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
        // __LINE__: 1068
        UTIL_ASSERT(FALSE);
    }

    rFXClip.left = size1.cx;
    rFXClip.top = 0;
    rFXClip.right = rFXRect.Width() - size2.cx;
    rFXClip.bottom = rFXRect.Height();
    RenderText(pVidInf, size1.cx, 0, rFXClip, dwRenderFlags, static_cast<SHORT>(wfield_5E4 - size2.cx - size1.cx));

    RealizePalette(dwRenderFlags);
    GetCurrentCenterPoint(ptCenter, TRUE);

    INT nDestX = max(min(max(x - wfield_5E0 / 2, rClip.left), rClip.right - wfield_5E0 - nWidth2 - nWidth1), rClip.left);
    INT nDestY = max(min(max(y - ptCenter.y, rClip.top), rClip.bottom - frameSize.cy - ptCenter.y), rClip.top);

    SequenceSet(2);
    FrameSet(wfield_5DC);
    GetCurrentCenterPoint(ptCenter, TRUE);

    rFXClip.left = 0;
    rFXClip.top = 0;
    rFXClip.right = rFXRect.Width();
    rFXClip.bottom = rFXRect.Height();
    if (!pVidInf->FXRender(this, ptCenter.x + nWidth1 + wfield_5E0 - nWidth2, ptCenter.y, rFXClip, TRUE, dwRenderFlags)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
        // __LINE__: 1111
        UTIL_ASSERT(FALSE);
    }

    SequenceSet(1);
    FrameSet(wfield_5DA);
    GetCurrentCenterPoint(ptCenter, TRUE);

    rFXClip.left = 0;
    rFXClip.top = 0;
    rFXClip.right = rFXRect.Width();
    rFXClip.bottom = rFXRect.Height();
    if (!pVidInf->FXRender(this, ptCenter.x, ptCenter.y, rFXClip, TRUE, dwRenderFlags)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
        // __LINE__: 1132
        UTIL_ASSERT(FALSE);
    }

    pVidInf->FXUnlock(dwRenderFlags, NULL, CPoint(0, 0));
    pVidInf->FXBltToBack(rFXRect, nDestX, nDestY, 0, 0, rClip, dwRenderFlags);

    pRes->Release();
    return TRUE;
}

// 0x598770
void CInfToolTip::RenderText(CVidInf* pVidInf, INT x, int a4, const CRect& rFXClip, DWORD dwFlags, SHORT nMaxWidth)
{
    int index;
    CString sLine;
    LONG nWidth = rFXClip.Width();

    if (m_font.GetRes()->Demand() == NULL) {
        return;
    }

    for (index = 0; index < 2; index++) {
        if (sfield_5EC[index] == "") {
            break;
        }
    }

    INT y = m_font.GetBaseLineHeight(TRUE) + (rFXClip.Height() - index * m_font.GetFontHeight(TRUE)) / 2;
    for (index = 0; index < 2; index++) {
        if (sfield_5EC[index] == "") {
            break;
        }

        sLine = sfield_5EC[index];

        // FIXME: Calls `GetStringLength` two times.
        pVidInf->FXTextOut(&m_font,
            sLine,
            x + nWidth / 2 - min(m_font.GetStringLength(sLine, TRUE), nMaxWidth) / 2,
            y,
            rFXClip,
            dwFlags,
            TRUE);

        y += m_font.GetFontHeight(TRUE);
    }

    m_font.GetRes()->Release();
}

// 0x598940
void CInfToolTip::SetTextRef(const STRREF& textRef, const CString& sExtra)
{
    STR_RES strRes;
    BYTE nLine;
    BYTE nLines;

    wfield_5E6 = 0;

    for (int index = 0; index < 2; index++) {
        sfield_5EC[index] = "";
    }

    CSize size1;
    GetFrameSize(1, wfield_5DA, size1, TRUE);

    CSize size2;
    GetFrameSize(2, wfield_5DC, size2, TRUE);

    WORD nMaxWidth = static_cast<WORD>(wfield_5E4 - size2.cx - size1.cx);

    m_font.GetRes()->Demand();

    if (sExtra != "") {
        if (textRef != -1 && g_pBaldurChitin->GetTlkTable().Fetch(textRef, strRes)) {
            sfield_5EC[0] = strRes.szText;
            CUtil::TrimRight(sfield_5EC[0]);
            nLines = CUtil::SplitString(&m_font,
                sExtra,
                nMaxWidth - 8,
                1,
                &(sfield_5EC[1]),
                FALSE,
                TRUE,
                TRUE,
                -1);
        } else {
            int pos = CUtil::Find(sExtra, '\n', 0);
            if (pos != -1) {
                sfield_5EC[0] = sExtra.Left(pos);
                CUtil::TrimRight(sfield_5EC[0]);
                nLines = CUtil::SplitString(&m_font,
                    sExtra.Mid(pos + 1),
                    nMaxWidth - 8,
                    1,
                    &(sfield_5EC[1]),
                    FALSE,
                    TRUE,
                    TRUE,
                    -1);
            } else {
                nLines = CUtil::SplitString(&m_font,
                    sExtra.Mid(pos + 1),
                    nMaxWidth - 8,
                    2,
                    sfield_5EC,
                    FALSE,
                    TRUE,
                    TRUE,
                    -1);
            }
        }

        if (nLines > 0) {
            CUtil::TrimRight(sfield_5EC[nLines - 1]);
            for (nLine = 0; nLine < nLines; nLine++) {
                LONG nLineWidth = m_font.GetStringLength(sfield_5EC[nLine], TRUE);
                if (nLineWidth > wfield_5E6) {
                    wfield_5E6 = static_cast<SHORT>(nLineWidth);
                }
            }
        }

        m_font.GetRes()->Release();

        wfield_5E6 = min(wfield_5E6 + 8, nMaxWidth);
    } else {
        if (textRef != -1 && g_pBaldurChitin->GetTlkTable().Fetch(textRef, strRes)) {
            nLines = CUtil::SplitString(&m_font,
                strRes.szText,
                nMaxWidth - 8,
                2,
                sfield_5EC,
                FALSE,
                TRUE,
                TRUE,
                -1);
            if (nLines > 0) {
                CUtil::TrimRight(sfield_5EC[nLines - 1]);
                for (nLine = 0; nLine < nLines; nLine++) {
                    LONG nLineWidth = m_font.GetStringLength(sfield_5EC[nLine], TRUE);
                    if (nLineWidth > wfield_5E6) {
                        wfield_5E6 = static_cast<SHORT>(nLineWidth);
                    }
                }
            }

            m_font.GetRes()->Release();

            wfield_5E6 = min(wfield_5E6 + 8, nMaxWidth);
        } else {
            m_font.GetRes()->Release();
        }
    }
}

// 0x598D30
void CInfToolTip::SetTextRef(CString sExtra, const STRREF& textRef1, const STRREF& textRef2, const STRREF& textRef3)
{
    STR_RES strRes1;
    STR_RES strRes2;

    if (textRef1 != -1) {
        wfield_5E6 = 0;

        for (int index = 0; index < 2; index++) {
            sfield_5EC[index] = "";
        }

        CSize size1;
        GetFrameSize(1, wfield_5DA, size1, TRUE);

        CSize size2;
        GetFrameSize(2, wfield_5DC, size2, TRUE);

        WORD nMaxWidth = static_cast<WORD>(wfield_5E4 - size2.cx - size1.cx);

        m_font.GetRes()->Demand();

        if (g_pBaldurChitin->GetTlkTable().Fetch(textRef1, strRes1)) {
            if (textRef2 != -1) {
                if (g_pBaldurChitin->GetTlkTable().Fetch(textRef2, strRes2)) {
                    strRes1.szText = strRes2.szText + strRes1.szText;
                }
            }

            if (textRef3 != -1) {
                if (g_pBaldurChitin->GetTlkTable().Fetch(textRef3, strRes2)) {
                    strRes1.szText = strRes1.szText + strRes2.szText;
                }
            }

            strRes1.szText = strRes1.szText + sExtra;

            BYTE nLines = CUtil::SplitString(&m_font,
                strRes1.szText,
                nMaxWidth - 8,
                2,
                sfield_5EC,
                FALSE,
                TRUE,
                TRUE,
                -1);
            if (nLines > 0) {
                CUtil::TrimRight(sfield_5EC[nLines - 1]);
                for (BYTE nLine = 0; nLine < nLines; nLine++) {
                    wfield_5E6 = max(static_cast<SHORT>(m_font.GetStringLength(sfield_5EC[nLine], TRUE)), wfield_5E6);
                }
            }

            m_font.GetRes()->Release();
            wfield_5E6 = min(wfield_5E6 + 8, nMaxWidth);
        }
    }
}

// 0x5990F0
void CInfToolTip::StoreBackground(INT nFrom, INT nTo, INT x, INT y, const CRect& rClip, CRect& rStorage, BOOLEAN bNumbered)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
    // __LINE__: 1471
    UTIL_ASSERT(pRes != NULL);

    m_nCurrentSequence = 0;
    m_nCurrentFrame = 0;

    if (pRes->Demand() == NULL) {
        // NOTE: rStorage is not nullified like in the overload below.
        return;
    }

    // NOTE: Static dispatch.
    if (!GetFrame()) {
        pRes->Release();
        return;
    }

    CSize size2;
    GetFrameSize(2, wfield_5DC, size2, TRUE);

    CSize size1;
    GetFrameSize(1, wfield_5DA, size1, TRUE);

    wfield_5E0 = m_wField5DE;

    rStorage.left = max(min(max(x - m_wField5DE / 2 - size1.cx / 2, rClip.left),
                            rClip.right - m_wField5DE - size1.cx - size2.cx / 2),
        rClip.left);
    rStorage.top = max(min(max(y - m_pFrame->nCenterY, rClip.top),
                           rClip.bottom - m_pFrame->nHeight),
        rClip.top);
    rStorage.right = rStorage.left + size1.cx + size2.cx / 2 + wfield_5E0;
    rStorage.bottom = rStorage.top + m_pFrame->nHeight;

    if (rStorage.left < rClip.left) {
        rStorage.left = rClip.left;
    }

    if (rStorage.top < rClip.top) {
        rStorage.top = rClip.top;
    }

    if (rStorage.right >= rClip.right) {
        rStorage.right = rClip.right;
    }

    if (rStorage.bottom >= rClip.bottom) {
        rStorage.bottom = rClip.bottom;
    }

    do {
        HRESULT hr = g_pChitin->cVideo.cVidBlitter.BltFast(g_pChitin->GetCurrentVideoMode()->pSurfaces[nTo],
            0,
            0,
            g_pChitin->GetCurrentVideoMode()->pSurfaces[nFrom],
            &rStorage,
            DDBLTFAST_WAIT);
        g_pChitin->GetCurrentVideoMode()->CheckResults(hr);
        if (hr != DDERR_SURFACELOST && hr != DDERR_WASSTILLDRAWING) {
            break;
        }
    } while (!g_pChitin->nm_m_field_1932);

    pRes->Release();
    m_pFrame = NULL;
}

// 0x599360
void CInfToolTip::StoreBackground(INT x, INT y, const CRect& rClip, CRect& rStorage, BOOLEAN bNumbered)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfCursor.cpp
    // __LINE__: 1560
    UTIL_ASSERT(pRes != NULL);

    m_nCurrentSequence = 0;
    m_nCurrentFrame = 0;

    if (pRes->Demand() == NULL) {
        rStorage.SetRect(0, 0, 0, 0);
        return;
    }

    // NOTE: Static dispatch.
    if (!GetFrame()) {
        pRes->Release();
        return;
    }

    CSize size2;
    GetFrameSize(2, wfield_5DC, size2, TRUE);

    CSize size1;
    GetFrameSize(1, wfield_5DA, size1, TRUE);

    wfield_5E0 = m_wField5DE;

    rStorage.left = max(min(max(x - m_wField5DE / 2 - size1.cx / 2, rClip.left),
                            rClip.right - m_wField5DE - size1.cx - size2.cx / 2),
        rClip.left);
    rStorage.top = max(min(max(y - m_pFrame->nCenterY, rClip.top),
                           rClip.bottom - m_pFrame->nHeight),
        rClip.top);
    rStorage.right = rStorage.left + size1.cx + size2.cx / 2 + wfield_5E0;
    rStorage.bottom = rStorage.top + m_pFrame->nHeight;

    if (rStorage.left < rClip.left) {
        rStorage.left = rClip.left;
    }

    if (rStorage.top < rClip.top) {
        rStorage.top = rClip.top;
    }

    if (rStorage.right >= rClip.right) {
        rStorage.right = rClip.right;
    }

    if (rStorage.bottom >= rClip.bottom) {
        rStorage.bottom = rClip.bottom;
    }

    pRes->Release();
    m_pFrame = NULL;
}

// Phase 1-2: Scaffold functions
// 0x5959A0
void FUN_005959a0() {
    // TODO: Incomplete.
}

// 0x595C80
void FUN_00595c80() {
    // TODO: Incomplete.
}

// 0x595D60
void FUN_00595d60() {
    // TODO: Incomplete.
}

// 0x595E70
void FUN_00595e70() {
    // TODO: Incomplete.
}

// 0x595EB0
void FUN_00595eb0() {
    // TODO: Incomplete.
}

// 0x595F70
void FUN_00595f70() {
    // TODO: Incomplete.
}

// 0x595FB0
void FUN_00595fb0() {
    // TODO: Incomplete.
}

// 0x596040
void FUN_00596040() {
    // TODO: Incomplete.
}

// 0x5960E0
void FUN_005960e0() {
    // TODO: Incomplete.
}

// 0x5961A0
void FUN_005961a0() {
    // TODO: Incomplete.
}

// 0x596250
void FUN_00596250() {
    // TODO: Incomplete.
}

// 0x5962E0
void FUN_005962e0() {
    // TODO: Incomplete.
}

// 0x596580
void FUN_00596580() {
    // TODO: Incomplete.
}

// 0x596600
void FUN_00596600() {
    // TODO: Incomplete.
}

// 0x596670
void FUN_00596670() {
    // TODO: Incomplete.
}

// 0x5967A0
void FUN_005967a0() {
    // TODO: Incomplete.
}

// 0x596810
void FUN_00596810() {
    // TODO: Incomplete.
}

// 0x596870
void FUN_00596870() {
    // TODO: Incomplete.
}

// 0x5968C0
void FUN_005968c0() {
    // TODO: Incomplete.
}

// 0x596950
void FUN_00596950() {
    // TODO: Incomplete.
}

// 0x596980
void FUN_00596980() {
    // TODO: Incomplete.
}

// 0x5969F0
void FUN_005969f0() {
    // TODO: Incomplete.
}

// 0x596A80
void FUN_00596a80() {
    // TODO: Incomplete.
}

// 0x596B10
void FUN_00596b10() {
    // TODO: Incomplete.
}

// 0x596B50
void FUN_00596b50() {
    // TODO: Incomplete.
}

// 0x5972F4
void FUN_005972f4() {
    // TODO: Incomplete.
}

