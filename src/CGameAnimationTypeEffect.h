#ifndef CGAMEANIMATIONTYPEEFFECT_H_
#define CGAMEANIMATIONTYPEEFFECT_H_

#include "CGameAnimationType.h"
#include "CVidCell.h"
#include "IcewindCVisualEffect.h"
#include "TString.h"

// The spell-hit detonation-ember sibling (vtable 0x84eeb8, IcewindCGameAnimationTypeEffect.cpp)
// is recovered as a subclass in IcewindCGameAnimationTypeEffect.h; its descriptor
// ctors (0x55CD70 / 0x55D3A0) used to live here when the two binary classes were
// conflated.
#pragma pack(push, 2)
class CGameAnimationTypeEffect : public CGameAnimationType {
public:
    CGameAnimationTypeEffect(USHORT animationID, BYTE* colorRangeValues, WORD facing);
    /* 0000 */ ~CGameAnimationTypeEffect() override;
    /* 0004 */ void CalculateFxRect(CRect& rFx, CPoint& ptReference, LONG posZ) override;
    /* 0008 */ void CalculateGCBoundsRect(CRect& rGCBounds, const CPoint& pos, const CPoint& ptReference, LONG posZ, LONG nWidth, LONG nHeight) override;
    /* 002C */ BOOLEAN DetectedByInfravision() override;
    /* 000C */ void ChangeDirection(SHORT nDirection) override;
    /* 007C */ BOOL IsMirroring() override;
    /* 0084 */ BOOL IsEndOfSequence() override;
    /* 0088 */ void IncrementFrame() override;
    /* 0090 */ void Render(CInfinity* pInfinity, CVidMode* pVidMode, INT nSurface, const CRect& rectFX, const CPoint& ptNewPos, const CPoint& ptReference, DWORD dwRenderFlags, COLORREF rgbTintColor, const CRect& rGCBounds, BOOL bDithered, BOOL bFadeOut, LONG posZ, BYTE transparency) override;
    /* 00AC */ SHORT SetSequence(SHORT nSequence) override;
    /* 00C8 */ SHORT GetCurrentFrame() override;

    // 0x55DBD0 (non-virtual -- no vtable slot references it). Swap the cell's BAM
    // to a new resref and reseed the sequence; used to flip the spell-hit cloud
    // animation between ICloudA/ICloudB. Named after BG2's OverrideAnimation
    // (which is virtual / (CResRef,int) there); IWD2's takes one resref string.
    void OverrideAnimation(const char* resref);

protected:
    // Shared base/member construction for the IcewindCGameAnimationTypeEffect
    // spell-hit sibling: builds the CGameAnimationType base and the
    // CVidCell/CVidPalette/IcewindCVisualEffect subobjects (m_palette as TYPE_RANGE)
    // before the subclass fills in its own members. Not a standalone-constructible
    // animation -- the binary's two classes both derive straight from
    // CGameAnimationType, this scaffolds the shared part of their ctors.
    CGameAnimationTypeEffect();

public:
    /* 03FE */ CVidCell* m_currentVidCell;
    /* 0402 */ CVidCell* m_currentVidCellShadow;
    /* 0406 */ CVidCell m_g1VidCell;
    /* 04E0 */ CVidCell m_g1VidCellShadow;
    /* 05BA */ CVidPalette m_palette;
    /* 05DE */ unsigned char field_5DE;
    /* 05DF */ BOOLEAN m_translucent;
    /* 05E0 */ BOOLEAN m_bRender;
    /* 05E1 */ unsigned char field_5E1;
    /* 05E2 */ SHORT m_currentBamSequence;
    /* 05E4 */ SHORT m_currentBamDirection;
    /* 05E6 */ BYTE m_extendDirectionTest;
    /* 05E7 */ unsigned char m_animMode;             // = descriptor.m_animMode; random-sequence opt-in (==1)
    // Tail revealed by the 0x55D3A0 spell-hit ctor (the class is 0x606 bytes in
    // IWD2.exe; the 0x6A1F50 ctor leaves these untouched).
    /* 05E8 */ BYTE m_animFlag36;                    // = descriptor.m_animFlag36
    /* 05EA */ IcewindCVisualEffect m_visualEffect;  // = descriptor.m_visualEffect
    // 16-byte TString (data @0x5FA, len @0x5FE, cap @0x602). The ctors stash
    // (BYTE)facing in its unused +0 header byte (m_buf[0] @0x5F6); the string ops
    // never touch +0, so the two coexist exactly as in the binary.
    /* 05F6 */ TString m_animResName;
};
#pragma pack(pop)

#endif /* CGAMEANIMATIONTYPEEFFECT_H_ */
