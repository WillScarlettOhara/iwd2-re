#include "CVidBitmap.h"

#include "CChitin.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "CVideo3d.h"

// 0x7B2A40
CVidBitmap::CVidBitmap()
{
    m_nBitCount = 8;
    m_bDoubleSize = FALSE;
}

// 0x7B2DB0
CVidBitmap::~CVidBitmap()
{
}

// FIXME: Use reference.
//
// 0x7B2AD0
CVidBitmap::CVidBitmap(CResRef cNewResRef, BOOL bDoubleSize)
{
    // NOTE: Uninline.
    SetResRef(cNewResRef, TRUE, TRUE);

    m_nBitCount = 8;
    m_bDoubleSize = bDoubleSize;
}

// FIXME: Use reference.
//
// 0x7B2C40
CVidBitmap::CVidBitmap(CResRef cNewResRef, BOOL bDoubleSize, USHORT nBitCount)
{
    // NOTE: Uninline.
    SetResRef(cNewResRef, TRUE, TRUE);

    m_nBitCount = nBitCount;
    m_bDoubleSize = bDoubleSize;
}

// 0x7B2E90
SHORT CVidBitmap::GetBitCount(BOOL bDemanded)
{
    if (!bDemanded) {
        pRes->Demand();
    }

    SHORT nBitCount = pRes->GetBitDepth();

    if (!bDemanded) {
        pRes->Release();
    }

    return nBitCount;
}

// 0x7B2ED0
BOOL CVidBitmap::GetImageDimensions(CSize& size, BOOLEAN bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7277
    UTIL_ASSERT(pRes != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (!bDemanded) {
        if (pRes->Demand() == NULL) {
            return FALSE;
        }
    }

    pRes->GetImageDimensions(size, m_bDoubleSize);

    if (!bDemanded) {
        pRes->Release();
    }

    return TRUE;
}

// 0x7B2F60
BOOL CVidBitmap::GetPixelColor(RGBQUAD& color, INT nX, INT nY, BOOL bLoaded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7337
    UTIL_ASSERT(pRes != NULL);

    if (pRes == NULL) {
        return FALSE;
    }

    if (bLoaded) {
        pRes->GetPixelColor(nX, nY, color, m_bDoubleSize);
        return TRUE;
    }

    if (pRes->Demand()) {
        pRes->GetPixelColor(nX, nY, color, m_bDoubleSize);
        pRes->Release();
        return TRUE;
    }

    return FALSE;
}

// 0x7B3000
BOOL CVidBitmap::GetPixelValue(BYTE& value, INT x, INT y, BOOLEAN bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7400
    UTIL_ASSERT((m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT));

    if (pRes == NULL) {
        return FALSE;
    }

    if (m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT) {
        if (bDemanded) {
            value = pRes->GetPixelValue(x, y, m_bDoubleSize);
            return TRUE;
        }

        if (pRes->Demand() != NULL) {
            value = pRes->GetPixelValue(x, y, m_bDoubleSize);
            pRes->Release();
            return TRUE;
        }
    }

    return FALSE;
}

// 0x7B30C0
BYTE CVidBitmap::GetPixelValue(INT x, INT y, BOOLEAN bDemanded)
{
    // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage.cpp
    // __LINE__: 7440
    UTIL_ASSERT((m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT));

    if (pRes == NULL) {
        return 0;
    }

    if (m_nBitCount == CVIDBITMAP_8BIT || m_nBitCount == CVIDBITMAP_4BIT) {
        if (bDemanded) {
            return pRes->GetPixelValue(x, y, m_bDoubleSize);
        }

        if (pRes->Demand() != NULL) {
            BYTE value = pRes->GetPixelValue(x, y, m_bDoubleSize);
            pRes->Release();
            return value;
        }
    }

    return 0;
}

// 0x7B3180
void CVidBitmap::SetRes(CResBitmap* pResBmp)
{
    pRes = pResBmp;
}

// 0x7B3190
BOOL CVidBitmap::RenderDirect(INT nSurface, INT x, INT y, const CRect& rClip, DWORD dwFlags, BOOLEAN bDemanded)
{
    if (g_pChitin->cVideo.Is3dAccelerated()) {
        return RenderDirect3d(x, y, rClip, dwFlags, bDemanded);
    }

    if (pRes == NULL) {
        return FALSE;
    }

    if (rClip.IsRectEmpty()) {
        return FALSE;
    }

    if (!bDemanded) {
        pRes->Demand();
    }

    BYTE* pData = pRes->GetImageData(m_bDoubleSize);
    m_nBitCount = pRes->GetBitDepth();

    CSize bmpSize;
    pRes->GetImageDimensions(bmpSize, m_bDoubleSize);

    INT nBytesPerPixel;
    switch (m_nBitCount) {
    case 8:
        nBytesPerPixel = 1;
        break;
    case 24:
        nBytesPerPixel = 3;
        break;
    default:
        if (!bDemanded) {
            pRes->Release();
        }
        return FALSE;
    }

    LONG nDataJump = (-((bmpSize.cx * nBytesPerPixel) & 3)) & 3;

    CRect rSurface(CPoint(x, y), bmpSize);

    if (bmpSize.cy > rClip.Height()) {
        INT nSkipBottom = y + bmpSize.cy - rClip.bottom;
        if (nSkipBottom > 0) {
            pData += nSkipBottom * (nDataJump + bmpSize.cx * nBytesPerPixel);
            bmpSize.cy -= nSkipBottom;
            rSurface.bottom -= nSkipBottom;
        }

        INT nSkipTop = rClip.top - y;
        if (nSkipTop > 0) {
            bmpSize.cy -= nSkipTop;
            rSurface.top += nSkipTop;
        }
    }

    if (bmpSize.cx > rClip.Width()) {
        INT nSkipRight = x + bmpSize.cx - rClip.right;
        if (nSkipRight > 0) {
            nDataJump += nBytesPerPixel * nSkipRight;
            bmpSize.cx -= nSkipRight;
            rSurface.right -= nSkipRight;
        }

        INT nSkipLeft = rClip.left - x;
        if (nSkipLeft > 0) {
            nDataJump += nBytesPerPixel * nSkipLeft;
            pData += nBytesPerPixel * nSkipLeft;
            bmpSize.cx -= nSkipLeft;
            rSurface.left += nSkipLeft;
        }
    }

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);

    BOOL bResult = FALSE;

    if (g_pChitin->GetCurrentVideoMode()->LockSurface(nSurface, &ddsd, rSurface)) {
        BYTE* pSurface = reinterpret_cast<BYTE*>(ddsd.lpSurface) + 4 * (bmpSize.cy - 1) * (ddsd.lPitch / 4);
        switch (m_nBitCount) {
        case 8:
            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                // NOTE: Uninline.
                bResult = BltBmp8To16(reinterpret_cast<WORD*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
                break;
            case 24:
                bResult = BltBmp8To24(reinterpret_cast<BYTE*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
                break;
            case 32:
                bResult = BltBmp8To32(reinterpret_cast<DWORD*>(pSurface),
                    ddsd.lPitch,
                    pData,
                    bmpSize,
                    nDataJump,
                    dwFlags);
                break;
            }
            break;
        case 24:
            switch (g_pChitin->cVideo.m_nBpp) {
            case 16:
                if (g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) != RGB(128, 128, 128)) {
                    bResult = BltBmp24To16Tint(reinterpret_cast<WORD*>(pSurface),
                        ddsd.lPitch,
                        pData,
                        bmpSize,
                        nDataJump,
                        dwFlags);
                } else {
                    bResult = BltBmp24To16(reinterpret_cast<WORD*>(pSurface),
                        ddsd.lPitch,
                        pData,
                        bmpSize,
                        nDataJump,
                        dwFlags);
                }
                break;
            case 24:
                if (g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) != RGB(128, 128, 128)) {
                    bResult = BltBmp24To24Tint(reinterpret_cast<BYTE*>(pSurface),
                        ddsd.lPitch,
                        pData,
                        bmpSize,
                        nDataJump,
                        dwFlags);
                } else {
                    bResult = BltBmp24To24(reinterpret_cast<BYTE*>(pSurface),
                        ddsd.lPitch,
                        pData,
                        bmpSize,
                        nDataJump,
                        dwFlags);
                }
                break;
            case 32:
                if ((dwFlags & 0xFFFF0000) == 0
                    && g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) == RGB(128, 128, 128)) {
                    bResult = BltBmp24To32(reinterpret_cast<DWORD*>(pSurface),
                        ddsd.lPitch,
                        pData,
                        bmpSize,
                        nDataJump,
                        dwFlags);
                } else {
                    bResult = BltBmp24To32Tint(reinterpret_cast<DWORD*>(pSurface),
                        ddsd.lPitch,
                        pData,
                        bmpSize,
                        nDataJump,
                        dwFlags);
                }
                break;
            }
            break;
        }

        g_pChitin->GetCurrentVideoMode()->UnLockSurface(nSurface, ddsd.lpSurface);
    }

    if (!bDemanded) {
        pRes->Release();
    }

    return bResult;
}

// NOTE: Inlined.
BOOL CVidBitmap::BltBmp8To16(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x7B37A0
BOOL CVidBitmap::BltBmp24To16(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x7B3B70
BOOL CVidBitmap::BltBmp24To16Tint(WORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x7B4230
BOOL CVidBitmap::GetTintValues(INT& nRed, INT& nGreen, INT& nBlue, INT& nShift, DWORD dwFlags)
{
    INT nMax = 255;
    BOOL bTint = FALSE;
    INT nLightPercentage = 0;
    INT nAddPercentage = 0;

    COLORREF rgbGlobalTint = g_pChitin->GetCurrentVideoMode()->GetGlobalTintColor();
    LONG nAUCounter = g_pChitin->nAUCounter + reinterpret_cast<LONG>(this);

    if ((dwFlags & 0x20000) != 0) {
        if ((dwFlags & 0x10000) != 0) {
            nMax = 255 * 255;
            nRed = GetRValue(m_paletteAffects.rgbTintColor) * GetRValue(rgbGlobalTint);
            nGreen = GetGValue(m_paletteAffects.rgbTintColor) * GetGValue(rgbGlobalTint);
            nBlue = GetBValue(m_paletteAffects.rgbTintColor) * GetBValue(rgbGlobalTint);
            nShift = 16;
        } else {
            nMax = 255;
            nRed = GetRValue(m_paletteAffects.rgbTintColor);
            nGreen = GetGValue(m_paletteAffects.rgbTintColor);
            nBlue = GetBValue(m_paletteAffects.rgbTintColor);
            nShift = 8;
        }
        bTint = TRUE;
    } else if ((dwFlags & 0x10000) != 0) {
        nMax = 255;
        nRed = GetRValue(rgbGlobalTint);
        nGreen = GetGValue(rgbGlobalTint);
        nBlue = GetBValue(rgbGlobalTint);
        nShift = 8;
        bTint = TRUE;
    }

    if (m_paletteAffects.pRangeTints[0] != NULL) {
        COLORREF rgbRange = *m_paletteAffects.pRangeTints[0];
        INT nPeriod = m_paletteAffects.aRangeTintPeriods[0];
        if (nPeriod == 1) {
            nRed *= GetRValue(rgbRange);
            nGreen *= GetGValue(rgbRange);
            nBlue *= GetBValue(rgbRange);
        } else {
            INT nPercentage = nAUCounter % (2 * nPeriod);
            if (nPercentage > nPeriod) {
                nPercentage = 2 * nPeriod - nPercentage;
            }

            nRed *= CVidPalette::NO_TINT - (CVidPalette::NO_TINT - GetRValue(rgbRange)) * nPercentage / nPeriod;
            nGreen *= CVidPalette::NO_TINT - (CVidPalette::NO_TINT - GetGValue(rgbRange)) * nPercentage / nPeriod;
            nBlue *= CVidPalette::NO_TINT - (CVidPalette::NO_TINT - GetBValue(rgbRange)) * nPercentage / nPeriod;
        }

        bTint = TRUE;
        nShift += 8;
        nMax *= 255;
    }

    if (m_paletteAffects.pRangeAdds[0] != NULL) {
        INT nPeriod = m_paletteAffects.aRangeAddPeriods[0];
        if (nPeriod == 1 || nPeriod == 0) {
            nAddPercentage = nPeriod;
        } else {
            nAddPercentage = nAUCounter % (2 * nPeriod);
            if (nAddPercentage > nPeriod) {
                nAddPercentage = 2 * nPeriod - nAddPercentage;
            }
        }
    }

    if (m_paletteAffects.pRangeLights[0] != NULL) {
        INT nPeriod = m_paletteAffects.aRangeLightPeriods[0];
        if (nPeriod == 1) {
            nLightPercentage = 1;
        } else {
            nLightPercentage = nAUCounter % (2 * nPeriod);
            if (nLightPercentage > nPeriod) {
                nLightPercentage = 2 * nPeriod - nLightPercentage;
            }
        }
    }

    // Pulsing adds and lights wash the tint towards white for suppressed ranges.
    if ((CVidPalette::m_SuppressTintMasks[0] & m_paletteAffects.suppressTints) != 0
        && (nLightPercentage != 0 || nAddPercentage != 0)) {
        if (nAddPercentage < nLightPercentage) {
            INT nPeriod = m_paletteAffects.aRangeLightPeriods[0];
            nRed += (nMax - nRed) * nLightPercentage / nPeriod;
            nGreen += (nMax - nGreen) * nLightPercentage / nPeriod;
            nBlue += (nMax - nBlue) * nLightPercentage / nPeriod;
        } else {
            INT nPeriod = m_paletteAffects.aRangeAddPeriods[0];
            nRed += (nMax - nRed) * nAddPercentage / nPeriod;
            nGreen += (nMax - nGreen) * nAddPercentage / nPeriod;
            nBlue += (nMax - nBlue) * nAddPercentage / nPeriod;
        }
    }

    return bTint;
}

// 0x7B4600
BOOL CVidBitmap::GetAddValues(INT& nRed, INT& nGreen, INT& nBlue, INT& nShift, DWORD dwFlags)
{
    BYTE nBrightness = g_pChitin->GetCurrentVideoMode()->m_nBrightnessCorrection;
    BOOL bAdd = FALSE;

    if ((dwFlags & 0x40000) != 0) {
        if (nBrightness == 0) {
            nRed = static_cast<BYTE>(~GetRValue(m_paletteAffects.rgbAddColor));
            nGreen = static_cast<BYTE>(~GetGValue(m_paletteAffects.rgbAddColor));
            nBlue = static_cast<BYTE>(~GetBValue(m_paletteAffects.rgbAddColor));
            nShift = 8;
        } else {
            INT nInverse = static_cast<BYTE>(~nBrightness);
            nRed = static_cast<BYTE>(~GetRValue(m_paletteAffects.rgbAddColor)) * nInverse;
            nGreen = static_cast<BYTE>(~GetGValue(m_paletteAffects.rgbAddColor)) * nInverse;
            nBlue = static_cast<BYTE>(~GetBValue(m_paletteAffects.rgbAddColor)) * nInverse;
            nShift = 16;
        }
        bAdd = TRUE;
    } else if (nBrightness != 0) {
        nRed = static_cast<BYTE>(~nBrightness);
        nGreen = static_cast<BYTE>(~nBrightness);
        nBlue = static_cast<BYTE>(~nBrightness);
        nShift = 8;
        bAdd = TRUE;
    }

    if (m_paletteAffects.pRangeAdds[0] != NULL) {
        COLORREF rgbRange = *m_paletteAffects.pRangeAdds[0];
        INT nPeriod = m_paletteAffects.aRangeAddPeriods[0];
        if (nPeriod == 1 || nPeriod == 0) {
            nRed *= static_cast<BYTE>(~GetRValue(rgbRange));
            nGreen *= static_cast<BYTE>(~GetGValue(rgbRange));
            nBlue *= static_cast<BYTE>(~GetBValue(rgbRange));
        } else {
            INT nPercentage = (g_pChitin->nAUCounter + reinterpret_cast<LONG>(this)) % (2 * nPeriod);
            if (nPercentage > nPeriod) {
                nPercentage = 2 * nPeriod - nPercentage;
            }

            nRed *= static_cast<BYTE>(~static_cast<BYTE>(GetRValue(rgbRange) * nPercentage / nPeriod));
            nGreen *= static_cast<BYTE>(~static_cast<BYTE>(GetGValue(rgbRange) * nPercentage / nPeriod));
            nBlue *= static_cast<BYTE>(~static_cast<BYTE>(GetBValue(rgbRange) * nPercentage / nPeriod));
        }

        nShift += 8;
        return TRUE;
    }

    return bAdd;
}

// 0x7B4800
BOOL CVidBitmap::GetLightValues(INT& nRed, INT& nGreen, INT& nBlue, DWORD dwFlags)
{
    INT nGamma = g_pChitin->GetCurrentVideoMode()->m_nGammaCorrection;
    BOOL bLight = FALSE;

    if ((dwFlags & 0x200000) != 0) {
        if (nGamma == 0) {
            nRed += GetRValue(m_paletteAffects.rgbLightColor);
            nGreen += GetGValue(m_paletteAffects.rgbLightColor);
            nBlue += GetBValue(m_paletteAffects.rgbLightColor);
        } else {
            nRed += GetRValue(m_paletteAffects.rgbLightColor) + nGamma;
            nGreen += GetGValue(m_paletteAffects.rgbLightColor) + nGamma;
            nBlue += GetBValue(m_paletteAffects.rgbLightColor) + nGamma;
        }
        bLight = TRUE;
    } else if (nGamma != 0) {
        nRed += nGamma;
        nGreen += nGamma;
        nBlue += nGamma;
        bLight = TRUE;
    }

    if (m_paletteAffects.pRangeLights[0] == NULL) {
        return bLight;
    }

    COLORREF rgbRange = *m_paletteAffects.pRangeLights[0];
    INT nPeriod = m_paletteAffects.aRangeLightPeriods[0];
    if (nPeriod != 1) {
        // NOTE: A zero period divides by zero here, as in the original.
        INT nPercentage = (g_pChitin->nAUCounter + reinterpret_cast<LONG>(this)) % (2 * nPeriod);
        if (nPercentage > nPeriod) {
            nPercentage = 2 * nPeriod - nPercentage;
        }

        nRed += GetRValue(rgbRange) * nPercentage / nPeriod;
        nGreen += GetGValue(rgbRange) * nPercentage / nPeriod;
        nBlue += GetBValue(rgbRange) * nPercentage / nPeriod;
        return TRUE;
    }

    nRed += GetRValue(rgbRange);
    nGreen += GetGValue(rgbRange);
    nBlue += GetBValue(rgbRange);
    return TRUE;
}

// 0x7CCB20
BOOL CVidBitmap::BltBmp8To24(BYTE* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x7CCBF0
BOOL CVidBitmap::BltBmp24To24(BYTE* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x7CCE80
BOOL CVidBitmap::BltBmp24To24Tint(BYTE* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x7CD5E0
BOOL CVidBitmap::BltBmp8To32(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    m_cPalette.SetPalette(pRes->GetColorTable(), pRes->GetColorCount(), CVidPalette::TYPE_RESOURCE);
    m_cPalette.Realize(rgbTempPal, 32, dwFlags | 0x800000, &m_paletteAffects, 255);

    for (LONG y = 0; y < bmpSize.cy; y++) {
        for (LONG x = 0; x < bmpSize.cx; x++) {
            *pSurface++ = rgbTempPal[*pData++];
        }

        pSurface += -bmpSize.cx - lPitch / 4;
        pData += nDataJump;
    }

    return TRUE;
}

// 0x7CD690
BOOL CVidBitmap::BltBmp24To32(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    DWORD dwRBitShift = pVidMode->m_dwRBitShift;
    DWORD dwGBitShift = pVidMode->m_dwGBitShift;
    DWORD dwBBitShift = pVidMode->m_dwBBitShift;

    if ((dwFlags & 0x1) != 0) {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                BYTE r = pData[2];
                BYTE g = pData[1];
                BYTE b = pData[0];
                BYTE a;

                if (r != 0 || g != 255 || b != 0) {
                    if ((dwFlags & 0x4) == 0 || r != 0 || g != 0 || b != 0) {
                        a = 255;
                    } else {
                        a = 128;
                        r = ((*pSurface >> dwRBitShift) >> 1) & 0x7F;
                        g = ((*pSurface >> dwGBitShift) >> 1) & 0x7F;
                        b = ((*pSurface >> dwBBitShift) >> 1) & 0x7F;
                    }

                    *pSurface = (a << 24) | (r << dwRBitShift) | (g << dwGBitShift) | (b << dwBBitShift);
                }

                pSurface++;
                pData += 3;
            }

            pSurface += -bmpSize.cx - lPitch / 4;
            pData += nDataJump;
        }
    } else {
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                *pSurface++ = (pData[0] << dwBBitShift) | (pData[1] << dwGBitShift) | (pData[2] << dwRBitShift);
                pData += 3;
            }

            pSurface += -bmpSize.cx - lPitch / 4;
            pData += nDataJump;
        }
    }

    return TRUE;
}

// 0x7CD870
BOOL CVidBitmap::BltBmp24To32Tint(DWORD* pSurface, LONG lPitch, BYTE* pData, const CSize& bmpSize, LONG nDataJump, DWORD dwFlags)
{
    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();

    INT nTintRed = 255;
    INT nTintGreen = 255;
    INT nTintBlue = 255;
    INT nTintShift = 0;
    INT nAddRed = 1;
    INT nAddGreen = 1;
    INT nAddBlue = 1;
    INT nAddShift = 0;
    INT nLightRed = 1 << CVidPalette::LIGHT_SCALE;
    INT nLightGreen = 1 << CVidPalette::LIGHT_SCALE;
    INT nLightBlue = 1 << CVidPalette::LIGHT_SCALE;

    BYTE nRShift = static_cast<BYTE>(pVidMode->m_dwRBitShift);
    BYTE nGShift = static_cast<BYTE>(pVidMode->m_dwGBitShift);
    BYTE nBShift = static_cast<BYTE>(pVidMode->m_dwBBitShift);

    BOOL bTint = GetTintValues(nTintRed, nTintGreen, nTintBlue, nTintShift, dwFlags);
    BOOL bAdd = GetAddValues(nAddRed, nAddGreen, nAddBlue, nAddShift, dwFlags);
    BOOL bLight = GetLightValues(nLightRed, nLightGreen, nLightBlue, dwFlags);

    if ((dwFlags & 0x1) == 0) {
        // NOTE: The original is inline assembly on 32-bit registers. The add
        // inversions and the light clamp only touch the low byte, so any higher
        // bits survive into the packed pixel.
        for (LONG y = 0; y < bmpSize.cy; y++) {
            for (LONG x = 0; x < bmpSize.cx; x++) {
                DWORD r = pData[2];
                DWORD g = pData[1];
                DWORD b = pData[0];

                if ((dwFlags & 0x80000) != 0) {
                    r = (r + g + b) >> 2;
                    g = r;
                    b = r;
                }

                if (bTint) {
                    r = (r * nTintRed) >> nTintShift;
                    g = (g * nTintGreen) >> nTintShift;
                    b = (b * nTintBlue) >> nTintShift;
                }

                if (bAdd) {
                    r = (((r ^ 0xFF) * nAddRed) >> nAddShift) ^ 0xFF;
                    g = (((g ^ 0xFF) * nAddGreen) >> nAddShift) ^ 0xFF;
                    b = (((b ^ 0xFF) * nAddBlue) >> nAddShift) ^ 0xFF;
                }

                if (bLight) {
                    r = (r * nLightRed) >> 3;
                    g = (g * nLightGreen) >> 3;
                    b = (b * nLightBlue) >> 3;

                    if ((r & 0xFF00) != 0) {
                        r |= 0xFF;
                    }
                    if ((g & 0xFF00) != 0) {
                        g |= 0xFF;
                    }
                    if ((b & 0xFF00) != 0) {
                        b |= 0xFF;
                    }
                }

                *pSurface = (r << nRShift) | (g << nGShift) | (b << nBShift);

                pSurface++;
                pData += 3;
            }

            pSurface = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pSurface) - (lPitch + bmpSize.cx * 4));
            pData += nDataJump;
        }

        return TRUE;
    }

    COLORREF rgbBrightnessContrast = pVidMode->ApplyBrightnessContrast(RGB(0, 0, 0));

    for (LONG y = 0; y < bmpSize.cy; y++) {
        for (LONG x = 0; x < bmpSize.cx; x++) {
            INT r = pData[2];
            INT g = pData[1];
            INT b = pData[0];

            if (r != 0 || g != 255 || b != 0) {
                DWORD a;
                if ((dwFlags & 0x4) == 0 || r != 0 || g != 0 || b != 0) {
                    a = 255;

                    if ((dwFlags & 0x80000) != 0) {
                        r = static_cast<BYTE>((b + g + r) >> 2);
                        g = r;
                        b = r;
                    }

                    if (bTint) {
                        r = static_cast<BYTE>((r * nTintRed) >> nTintShift);
                        g = static_cast<BYTE>((g * nTintGreen) >> nTintShift);
                        b = static_cast<BYTE>((b * nTintBlue) >> nTintShift);
                    }

                    if (bAdd) {
                        r = static_cast<BYTE>(~static_cast<BYTE>((static_cast<BYTE>(~r) * nAddRed) >> nAddShift));
                        g = static_cast<BYTE>(~static_cast<BYTE>((static_cast<BYTE>(~g) * nAddGreen) >> nAddShift));
                        b = static_cast<BYTE>(~static_cast<BYTE>((static_cast<BYTE>(~b) * nAddBlue) >> nAddShift));
                    }

                    if (bLight) {
                        r = min((r * nLightRed) >> CVidPalette::LIGHT_SCALE, 255);
                        g = min((g * nLightGreen) >> CVidPalette::LIGHT_SCALE, 255);
                        b = min((b * nLightBlue) >> CVidPalette::LIGHT_SCALE, 255);

                        // Keep a tinted pixel from becoming the transparent key.
                        if (r == 0 && g == 255 && b == 0) {
                            g = 254;
                        }
                    }
                } else {
                    // 0x85E6F2
                    a = 128;

                    DWORD dwColor = *pSurface;
                    r = (((dwColor >> nRShift) & 0xFF) + GetRValue(rgbBrightnessContrast)) >> 1;
                    g = (((dwColor >> nGShift) & 0xFF) + GetGValue(rgbBrightnessContrast)) >> 1;
                    b = (((dwColor >> nBShift) & 0xFF) + GetBValue(rgbBrightnessContrast)) >> 1;
                }

                *pSurface = (b << nBShift) | (g << nGShift) | (r << nRShift) | (a << 24);
            }

            pSurface++;
            pData += 3;
        }

        pSurface -= (lPitch >> 2) + bmpSize.cx;
        pData += nDataJump;
    }

    return TRUE;
}

// 0x7C6800
void CVidBitmap::RenderTexture(INT x, INT y, const CSize& bmpSize, const CRect& rClip, DWORD dwFlags)
{
    if (x + bmpSize.cx < rClip.left
        || x > rClip.right
        || y + bmpSize.cy < rClip.top
        || y > rClip.bottom) {
        return;
    }

    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float x4;
    float y4;

    x1 = static_cast<float>(x);
    y1 = static_cast<float>(y);
    x2 = static_cast<float>(x + bmpSize.cx);
    y2 = static_cast<float>(y + bmpSize.cy);
    x3 = 0.0f;
    y3 = 0.0f;
    x4 = static_cast<float>(bmpSize.cx) / static_cast<float>(CVidInf::FX_WIDTH);
    y4 = static_cast<float>(bmpSize.cy) / static_cast<float>(CVidInf::FX_HEIGHT);

    if (x < rClip.left) {
        x1 = static_cast<float>(rClip.left);
        x3 = static_cast<float>(rClip.left - x) / static_cast<float>(CVidInf::FX_WIDTH);
    }

    if (y < rClip.top) {
        y1 = static_cast<float>(rClip.top);
        y3 = static_cast<float>(rClip.top - y) / static_cast<float>(CVidInf::FX_HEIGHT);
    }

    if (x + bmpSize.cx > rClip.right) {
        x2 = static_cast<float>(rClip.right);
        x4 = static_cast<float>(rClip.right - x) / static_cast<float>(CVidInf::FX_WIDTH);
    }

    if (y + bmpSize.cy > rClip.bottom) {
        y2 = static_cast<float>(rClip.bottom);
        y4 = static_cast<float>(rClip.bottom - y) / static_cast<float>(CVidInf::FX_HEIGHT);
    }

    CVideo3d::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x1) != 0) {
        CVideo3d::glEnable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

        CVideo3d::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }

    x1 += CVideo3d::SUB_PIXEL_SHIFT;
    y1 += CVideo3d::SUB_PIXEL_SHIFT;
    x2 += CVideo3d::SUB_PIXEL_SHIFT;
    y2 += CVideo3d::SUB_PIXEL_SHIFT;

    CVideo3d::glBegin(GL_TRIANGLE_STRIP);
    CVideo3d::glTexCoord2f(x3, y3);
    CVideo3d::glVertex3f(x1, y1, 0.0f);
    CVideo3d::glTexCoord2f(x3, y4);
    CVideo3d::glVertex3f(x1, y2, 0.0f);
    CVideo3d::glTexCoord2f(x4, y3);
    CVideo3d::glVertex3f(x2, y1, 0.0f);
    CVideo3d::glTexCoord2f(x4, y4);
    CVideo3d::glVertex3f(x2, y2, 0.0f);
    CVideo3d::glEnd();
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    if ((dwFlags & 0x1) != 0) {
        CVideo3d::glDisable(GL_BLEND);
        g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
    }
}

// 0x7C6B80
BOOL CVidBitmap::Render3d(INT x, INT y, const CRect& rClip, DWORD dwFlags, BOOLEAN bDemanded)
{
    if (pRes == NULL || rClip.IsRectEmpty()) {
        return FALSE;
    }

    if (!bDemanded) {
        pRes->Demand();
    }

    BYTE* pData = pRes->GetImageData(m_bDoubleSize);
    m_nBitCount = pRes->GetBitDepth();

    CSize bmpSize;
    GetImageDimensions(bmpSize, m_bDoubleSize);

    INT nBytesPerPixel;
    switch (m_nBitCount) {
    case 8:
        nBytesPerPixel = 1;
        break;
    case 24:
        nBytesPerPixel = 3;
        break;
    default:
        if (!bDemanded) {
            pRes->Release();
        }
        return FALSE;
    }

    LONG nDataJump = (-((bmpSize.cx * nBytesPerPixel) & 3)) & 3;

    CVideo3d::glEnable(GL_TEXTURE_2D);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    g_pChitin->cVideo.field_13E = 2;
    CVideo3d::glBindTexture(GL_TEXTURE_2D, 2);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CVideo3d::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);

    CSize tileSize;
    LONG lTilePitch;
    LONG nTileDataJump = 0;

    BYTE* pTileRowData = pData;
    for (INT yOffset = 0; yOffset < bmpSize.cy; yOffset += CVidInf::FX_HEIGHT) {
        BYTE* pTileData = pTileRowData;
        for (INT xOffset = 0; xOffset < bmpSize.cx; xOffset += CVidInf::FX_WIDTH) {
            if ((dwFlags & 0x1) != 0) {
                CVidInf::FXClear(CVideo3d::texImageData, CVidInf::FX_WIDTH * CVidInf::FX_HEIGHT);
            }

            tileSize.cx = min(bmpSize.cx - xOffset, CVidInf::FX_WIDTH);
            tileSize.cy = min(bmpSize.cy - yOffset, CVidInf::FX_HEIGHT);

            if (bmpSize.cx > tileSize.cx) {
                nTileDataJump = nBytesPerPixel * (bmpSize.cx - tileSize.cx);
            }

            lTilePitch = g_pChitin->cVideo.field_13A
                ? CVidTile::BYTES_PER_TEXEL * CVIDINF_FX_WIDTH
                : CVidTile::BYTES_PER_TEXEL * tileSize.cx;

            BOOLEAN bResult = TRUE;
            switch (m_nBitCount) {
            case 8:
                bResult = BltBmp8To32(&(CVideo3d::texImageData[lTilePitch * (tileSize.cy - 1) / CVidTile::BYTES_PER_TEXEL]),
                    lTilePitch,
                    pTileData,
                    tileSize,
                    nDataJump + nTileDataJump,
                    dwFlags);
                break;
            case 24:
                if ((dwFlags & 0xFFFF0000) == 0
                    && g_pChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(RGB(128, 128, 128)) == RGB(128, 128, 128)) {
                    bResult = BltBmp24To32(&(CVideo3d::texImageData[(lTilePitch * (tileSize.cy - 1) / CVidTile::BYTES_PER_TEXEL)]),
                        lTilePitch,
                        pTileData,
                        tileSize,
                        nDataJump + nTileDataJump,
                        dwFlags);
                } else {
                    bResult = BltBmp24To32Tint(&(CVideo3d::texImageData[lTilePitch * (tileSize.cy - 1) / CVidTile::BYTES_PER_TEXEL]),
                        lTilePitch,
                        pTileData,
                        tileSize,
                        nDataJump + nTileDataJump,
                        dwFlags);
                }
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\chitin\ChVidImage3d.cpp
                // __LINE__: 2120
                UTIL_ASSERT(FALSE);
            }

            if (!bResult) {
                if (!bDemanded) {
                    pRes->Release();
                }
                return FALSE;
            }

            if (g_pChitin->cVideo.field_13A) {
                CVideo3d::glTexImage2D(GL_TEXTURE_2D,
                    0,
                    GL_RGBA,
                    CVidInf::FX_WIDTH,
                    CVidInf::FX_HEIGHT,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    CVideo3d::texImageData);
                g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
            } else {
                CVideo3d::glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    tileSize.cx,
                    tileSize.cy,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    CVideo3d::texImageData);
                g_pChitin->GetCurrentVideoMode()->CheckResults3d(0);
            }

            RenderTexture(x + xOffset,
                y + bmpSize.cy - tileSize.cy - yOffset,
                tileSize,
                rClip,
                dwFlags);

            pTileData += CVidInf::FX_WIDTH * nBytesPerPixel;
        }
        pTileRowData += CVidInf::FX_HEIGHT * (nDataJump + bmpSize.cx * nBytesPerPixel);
    }

    if (!bDemanded) {
        pRes->Release();
    }

    return TRUE;
}

// 0x7C7070
BOOL CVidBitmap::RenderDirect3d(INT x, INT y, const CRect& rClip, DWORD dwFlags, BOOLEAN bDemanded)
{
    return Render3d(x, y, rClip, dwFlags, bDemanded);
}
