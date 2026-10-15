#ifndef ICEWINDCGAMEANIMATIONTYPEEFFECT_H_
#define ICEWINDCGAMEANIMATIONTYPEEFFECT_H_

#include "CGameAnimationTypeEffect.h"

struct IcewindCSpellHitEmission;
struct IcewindCSpellHitEmissionRanged;

// IcewindCGameAnimationTypeEffect.cpp in IWD2.exe (vtable 0x84eeb8). The spell-hit
// detonation ember animation, a distinct binary class from the SPFLAMES/SPCHUNKS
// CGameAnimationTypeEffect (vtable 0x85ac48, ObjAnimation.cpp): both derive from
// CGameAnimationType with an identical 0x606-byte layout. Conflating the two -- our
// build gave the spell-hit anim CGameAnimationTypeEffect::Render (0x6A36A0) -- is
// what tinted Fireball's embers green. This Render (0x55d950) drives the
// clear-fill/copy-from-back gate, tinting and per-cell transparency from the
// descriptor's IcewindCVisualEffect (m_bCopyFromBack etc.) and blits the cells with
// m_visualEffect.m_dwFlags, so a Fireball detonation (CopyFromBack=1) blends its
// embers over the backdrop (orange) instead of clear-filling to the BAM's green key.
//
// Modelled as a CGameAnimationTypeEffect subclass to reuse that layout and vtable
// (the spell-hit vtable is CGameAnimationTypeEffect's with the dtor, CalculateFxRect,
// ChangeDirection, IsEndOfSequence, IncrementFrame, Render and GetCurrentFrame slots
// overridden); the binary instead derives both straight from CGameAnimationType
// (siblings). The two destructor slots (0x55d2c0 / 0x55d240) are the
// compiler-generated deleting-destructor thunks; C++ member destruction supplies
// their shared cleanup.
#pragma pack(push, 2)
class IcewindCGameAnimationTypeEffect : public CGameAnimationTypeEffect {
public:
    /* 0000 */ ~IcewindCGameAnimationTypeEffect() override; // 0x55D2C0
    IcewindCGameAnimationTypeEffect(const IcewindCSpellHitEmission& descriptor, WORD facing);        // 0x55CD70
    IcewindCGameAnimationTypeEffect(const IcewindCSpellHitEmissionRanged& descriptor, WORD facing);  // 0x55D3A0
    /* 0004 */ void CalculateFxRect(CRect& rFx, CPoint& ptReference, LONG posZ) override;             // 0x55D690
    /* 000C */ void ChangeDirection(SHORT nDirection) override;                                       // 0x55D800
    /* 0084 */ BOOL IsEndOfSequence() override;                                                       // 0x55D260
    /* 0088 */ void IncrementFrame() override;                                                        // 0x55D810
    /* 0090 */ void Render(CInfinity* pInfinity, CVidMode* pVidMode, INT nSurface, const CRect& rectFX, const CPoint& ptNewPos, const CPoint& ptReference, DWORD dwRenderFlags, COLORREF rgbTintColor, const CRect& rGCBounds, BOOL bDithered, BOOL bFadeOut, LONG posZ, BYTE transparency) override;  // 0x55d950
    /* 00C8 */ SHORT GetCurrentFrame() override;                                                      // 0x55D210
};
#pragma pack(pop)

#endif /* ICEWINDCGAMEANIMATIONTYPEEFFECT_H_ */
