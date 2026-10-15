#include "IcewindCGameAnimationTypeEffect.h"

#include "CBaldurChitin.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CInfinity.h"
#include "CProjectile.h"
#include "CUtil.h"

// Native cleanup at 55D2E0 releases the TString, palette, two cells and base;
// the deleting wrapper remains compiler generated.
// 0x55D2C0
IcewindCGameAnimationTypeEffect::~IcewindCGameAnimationTypeEffect() = default;

// Spell-hit detonation overload for the plainer IcewindCSpellHitEmission
// (the 0x55D3A0 twin handles the IcewindCSpellHitEmissionRanged descriptor). It is
// identical except this emission carries no m_animFlag36 (it stays 0) and its
// random-sequence opt-in (m_animMode) comes from the plain descriptor's 0x2C slot
// rather than the ranged descriptor's 0x35 slot. The shared base/member construction
// (CGameAnimationType + the CVidCell/CVidPalette/IcewindCVisualEffect subobjects)
// runs through CGameAnimationTypeEffect's protected default ctor.
// 0x55CD70
IcewindCGameAnimationTypeEffect::IcewindCGameAnimationTypeEffect(const IcewindCSpellHitEmission& descriptor,
    WORD facing)
    : CGameAnimationTypeEffect()
{
    // facing is stashed in the m_animResName TString's unused +0 header byte (0x55D3A0);
    // the member's default ctor has already zeroed the string fields themselves.
    m_animResName.m_buf[0] = static_cast<char>(facing);
    field_5DE = 0;
    m_translucent = 0;
    field_5E1 = 0;
    m_currentBamSequence = 0;
    m_currentBamDirection = 0;
    m_extendDirectionTest = 0;
    m_animationID = 0;
    m_visualEffect = descriptor.m_visualEffect;
    m_animFlag36 = 0;
    m_animMode = descriptor.m_animMode;
    m_colorChunks = -1;
    m_bRender = TRUE;
    m_pSndDeath = "";

    if (descriptor.m_resref0Len == 0) {
        m_g1VidCell.SetResRef(CResRef("GreaseA"), FALSE, TRUE, TRUE);
    } else {
        m_g1VidCell.SetResRef(CResRef(CString(descriptor.m_resref0)), FALSE, TRUE, TRUE);

        if (m_animMode == 1) {
            BYTE cnt = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (cnt != 0) {
                m_currentBamSequence = static_cast<SHORT>(rand() % cnt);
            } else {
                m_currentBamSequence = 0;
            }
        } else {
            m_currentBamSequence = 0;
        }
        m_g1VidCell.SequenceSet(m_currentBamSequence);
        m_g1VidCell.FrameSet(0);

        m_animResName = descriptor.m_resref0 ? descriptor.m_resref0 : "";
    }

    m_currentVidCell = &m_g1VidCell;
    m_currentVidCellShadow = NULL;
    m_currentBamDirection = facing;
    m_extendDirectionTest = CGameSprite::DIR_N;
}

// Spell-hit detonation overload: builds one detonation-fan particle's
// animation from an IcewindCSpellHitVisual emission descriptor and the launch
// direction. Loads the detonation BAM into m_g1VidCell (the descriptor's
// m_resref0, or "GreaseA" when the descriptor carries no geometry), copies the
// descriptor's visual-effect parameters and, when the descriptor opts in
// (m_animMode), seeds a random start sequence.
// 0x55D3A0
IcewindCGameAnimationTypeEffect::IcewindCGameAnimationTypeEffect(const IcewindCSpellHitEmissionRanged& descriptor,
    WORD facing)
    : CGameAnimationTypeEffect()
{
    // facing is stashed in the m_animResName TString's unused +0 header byte (0x55D3A0);
    // the member's default ctor has already zeroed the string fields themselves.
    m_animResName.m_buf[0] = static_cast<char>(facing);
    field_5DE = 0;
    m_translucent = 0;
    field_5E1 = 0;
    m_currentBamSequence = 0;
    m_currentBamDirection = 0;
    m_extendDirectionTest = 0;
    m_animationID = 0;
    m_visualEffect = descriptor.m_visualEffect;
    m_animFlag36 = descriptor.m_animFlag36;
    m_animMode = descriptor.m_animMode;
    m_colorChunks = -1;
    m_bRender = TRUE;
    m_pSndDeath = "";

    if (descriptor.m_resref0Len == 0) {
        m_g1VidCell.SetResRef(CResRef("GreaseA"), FALSE, TRUE, TRUE);
    } else {
        m_g1VidCell.SetResRef(CResRef(CString(descriptor.m_resref0)), FALSE, TRUE, TRUE);

        if (m_animMode == 1) {
            BYTE cnt = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (cnt != 0) {
                m_currentBamSequence = static_cast<SHORT>(rand() % cnt);
            } else {
                m_currentBamSequence = 0;
            }
        } else {
            m_currentBamSequence = 0;
        }
        m_g1VidCell.SequenceSet(m_currentBamSequence);
        m_g1VidCell.FrameSet(0);

        m_animResName = descriptor.m_resref0 ? descriptor.m_resref0 : "";
    }

    m_currentVidCell = &m_g1VidCell;
    m_currentVidCellShadow = NULL;
    m_currentBamDirection = facing;
    m_extendDirectionTest = CGameSprite::DIR_N;
}

// 0x55D690. Same FX-rect computation as the CGameAnimationTypeEffect twin (0x6A3280):
// the cell's centre point plus its frame size, widened to enclose a shadow cell. The
// detonation embers never carry a shadow (the ctors force m_currentVidCellShadow to
// NULL), so only the else branch runs at runtime; the shadow arm is reproduced from
// the twin for fidelity.
void IcewindCGameAnimationTypeEffect::CalculateFxRect(CRect& rFx, CPoint& ptReference, LONG posZ)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameAnimationTypeEffect.cpp
    // __LINE__: 157
    UTIL_ASSERT(m_currentVidCell != NULL);

    CPoint ptChar;
    m_currentVidCell->GetCurrentCenterPoint(ptChar, FALSE);

    if (m_currentVidCellShadow != NULL) {
        ptChar.y += posZ;
    }

    ptReference = ptChar;

    CSize frameSize;
    m_currentVidCell->GetCurrentFrameSize(frameSize, FALSE);

    if (m_currentVidCellShadow != NULL) {
        CPoint ptShadow;
        m_currentVidCellShadow->GetCurrentCenterPoint(ptShadow, FALSE);

        ptReference.x = max(ptShadow.x, ptReference.x);
        ptReference.y = max(ptShadow.y, ptReference.y);

        frameSize.cx += ptReference.x - ptChar.x;
        frameSize.cy += ptReference.y - ptChar.y;
        rFx.SetRect(0, 0, frameSize.cx, frameSize.cy);

        m_currentVidCellShadow->GetCurrentFrameSize(frameSize, FALSE);

        rFx.right = max(frameSize.cx + ptReference.x - ptShadow.x, rFx.right);
        rFx.bottom = max(frameSize.cy + ptReference.y - ptShadow.y, rFx.bottom);

        ptReference.y = max(ptChar.y - posZ, ptShadow.y + posZ);
    } else {
        rFx.SetRect(0, 0, frameSize.cx, frameSize.cy);
    }
}

// 0x55D800. The detonation embers have no directional sequences, so unlike the
// CGameAnimationTypeEffect twin (0x6A3450) this only records the facing -- no assert,
// no sequence reselection.
void IcewindCGameAnimationTypeEffect::ChangeDirection(SHORT nDirection)
{
    m_currentBamDirection = nDirection;
}

// 0x55D260. Same as the CGameAnimationTypeEffect twin (0x6A3140): delegates to the
// current cell.
BOOL IcewindCGameAnimationTypeEffect::IsEndOfSequence()
{
    // __FILE__: .\Include\IcewindCGameAnimationTypeEffect.h
    // __LINE__: 38
    UTIL_ASSERT(m_currentVidCell != NULL);

    return m_currentVidCell->IsEndOfSequence(FALSE);
}

// Advances the ember animation. Unlike the CGameAnimationTypeEffect twin
// (0x6A3550, which keys on m_animationID), at end-of-sequence this either stops
// rendering (m_animFlag36 == 1, a one-shot burst), reseeds a random sequence
// (m_animMode == 1) or steps to the next sequence clamped to the last; mid-sequence it
// just advances the frame. The embers never carry a shadow cell (the ctors force
// m_currentVidCellShadow to NULL), so the shadow arms are inert here.
// 0x55D810
void IcewindCGameAnimationTypeEffect::IncrementFrame()
{
    if (m_currentVidCell->IsEndOfSequence(FALSE)) {
        if (m_animFlag36 == 1) {
            m_bRender = 0;
            return;
        }

        if (m_animMode == 1) {
            BYTE cnt = static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE));
            if (cnt != 0) {
                m_currentBamSequence = static_cast<SHORT>(rand() % cnt);
            } else {
                m_currentBamSequence = 0;
            }
        } else {
            m_currentBamSequence = static_cast<SHORT>(static_cast<USHORT>(m_currentBamSequence) + 1);
            if (m_currentBamSequence >= static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE))) {
                m_currentBamSequence = static_cast<SHORT>(static_cast<BYTE>(m_g1VidCell.GetNumberSequences(FALSE)) - 1);
            }
        }

        m_g1VidCell.SequenceSet(m_currentBamSequence);
        m_g1VidCell.FrameSet(0);

        if (m_currentVidCellShadow != NULL) {
            m_g1VidCellShadow.SequenceSet(m_currentBamSequence);
            m_g1VidCellShadow.FrameSet(0);
        }
    } else {
        m_currentVidCell->FrameAdvance();

        if (m_currentVidCellShadow != NULL) {
            m_currentVidCellShadow->FrameAdvance();
        }
    }
}

// The detonation-ember render. Unlike the CGameAnimationTypeEffect twin
// (0x6A36A0, which keys clear-fill on m_translucent/transparency and blits with the
// caller's render flags), every blend decision here comes from the descriptor's
// IcewindCVisualEffect: clear-fill only for an opaque, non-copy-from-back, shadowless
// cell; tint only when m_bTintEnabled; per-cell transparency from m_nTransValue; and
// the cells themselves are drawn with m_visualEffect.m_dwFlags. A Fireball detonation
// sets CopyFromBack=1, so its embers copy-from-back (blend over the backdrop, orange)
// instead of clear-filling to the BAM's green key colour.
// 0x55D950
void IcewindCGameAnimationTypeEffect::Render(CInfinity* pInfinity, CVidMode* pVidMode, INT nSurface, const CRect& rectFX, const CPoint& ptNewPos, const CPoint& ptReference, DWORD dwRenderFlags, COLORREF rgbTintColor, const CRect& rGCBounds, BOOL bDithered, BOOL bFadeOut, LONG posZ, BYTE transparency)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameAnimationTypeEffect.cpp
    // __LINE__: 262
    UTIL_ASSERT(pInfinity != NULL && pVidMode != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameAnimationTypeEffect.cpp
    // __LINE__: 263
    UTIL_ASSERT(m_currentVidCell != NULL);

    if (!m_bRender) {
        return;
    }

    CPoint ptPos(ptNewPos.x, static_cast<LONG>(static_cast<DWORD>(ptNewPos.y) + static_cast<DWORD>(posZ)));
    CRect rFXRect(rectFX);

    // The blend mode comes entirely from the visual-effect descriptor: an opaque,
    // non-copy-from-back, shadowless ember clear-fills the FX surface to its key
    // colour; anything else (transparent, field_2, copy-from-back, or shadowed)
    // copies the backdrop in first so the ember blends over it.
    if (m_currentVidCellShadow == NULL
        && !m_visualEffect.m_bTransparent
        && !m_visualEffect.field_2
        && !m_visualEffect.m_bCopyFromBack) {
        dwRenderFlags |= CInfinity::FXPREP_CLEARFILL;
    } else {
        dwRenderFlags |= CInfinity::FXPREP_COPYFROMBACK;
    }

    pInfinity->FXPrep(rFXRect, dwRenderFlags, nSurface, ptPos, ptReference);

    if (pInfinity->FXLock(rFXRect, dwRenderFlags)) {
        if (m_visualEffect.m_bTintEnabled == 1) {
            m_currentVidCell->SetTintColor(rgbTintColor);
        }

        if (m_currentVidCellShadow != NULL) {
            pInfinity->FXRender(m_currentVidCellShadow,
                ptReference.x,
                static_cast<LONG>(static_cast<DWORD>(ptReference.y) - static_cast<DWORD>(posZ)),
                m_visualEffect.m_dwFlags | 0x4,
                0);
        }

        if (m_visualEffect.m_bTransparent == 1) {
            pInfinity->FXRender(m_currentVidCell,
                ptReference.x,
                ptReference.y,
                m_visualEffect.m_dwFlags,
                m_visualEffect.m_nTransValue);
        } else {
            pInfinity->FXRender(m_currentVidCell,
                ptReference.x,
                ptReference.y,
                m_visualEffect.m_dwFlags,
                0);
        }

        // Both shadow/no-shadow paths in the binary push identical clipping-poly
        // arguments (ptPos.y unadjusted, posZ folded to 0, the raw GC bounds), so the
        // compiler's duplicated branch collapses to one call here.
        pInfinity->FXRenderClippingPolys(ptPos.x,
            ptPos.y,
            0,
            ptReference,
            rGCBounds,
            bDithered,
            m_visualEffect.m_dwFlags);

        if (bFadeOut) {
            CPoint point(static_cast<LONG>(static_cast<DWORD>(ptPos.x) + static_cast<DWORD>(ptReference.x)),
                static_cast<LONG>(static_cast<DWORD>(ptPos.y) + static_cast<DWORD>(ptReference.y)));
            pInfinity->FXUnlock(dwRenderFlags, &rFXRect, point);
        } else {
            pInfinity->FXUnlock(dwRenderFlags, NULL, CPoint(0, 0));
        }

        pInfinity->FXBltFrom(nSurface,
            rFXRect,
            ptPos.x,
            ptPos.y,
            ptReference.x,
            ptReference.y,
            m_visualEffect.m_dwFlags | 0x1);
    }
}

// 0x55D210. The detonation embers report no meaningful frame index -- unlike the
// CGameAnimationTypeEffect twin (0x6A3180) this returns -1 unconditionally.
SHORT IcewindCGameAnimationTypeEffect::GetCurrentFrame()
{
    return -1;
}
