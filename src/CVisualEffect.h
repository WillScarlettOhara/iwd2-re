#ifndef CVISUALEFFECT_H_
#define CVISUALEFFECT_H_

#include "CGameObject.h"
#include "CResRef.h"
#include "CVidBitmap.h"
#include "CVidCell.h"

#pragma pack(push, 2)
class CVEFEvent {
public:
    /* 0x0000 */ LONG m_nTimeFrame;
    /* 0x0004 */ DWORD m_dwFlags;
    /* 0x0008 */ LONG m_nRepeatRate;
    /* 0x000C */ LONG m_nResType;
    /* 0x0010 */ CResRef m_res;
    /* 0x0018 */ DWORD m_specialFlags;
    /* 0x001C */ DWORD m_pad[49];
};

class CVisualEffectBase {
public:
    /* 0x0000 */ DWORD m_nTimedEventOffset;
    /* 0x0004 */ DWORD m_nTimedEventCount;
    /* 0x0008 */ DWORD m_nTriggeredEventOffset;
    /* 0x000C */ DWORD m_nTriggeredEventCount;
    /* 0x0010 */ DWORD m_nTargetType;
    /* 0x0014 */ LONG m_nSpeed;
    /* 0x0018 */ LONG m_nDuration;
    /* 0x001C */ CPoint m_ptStartDeltaV;
    /* 0x0024 */ LONG m_nDeltaVDec;
    /* 0x0028 */ LONG m_nHeight;
    /* 0x002C */ DWORD m_flags;
    /* 0x0030 */ DWORD m_extra[100];
};

class CVEFVidCellBase {
public:
    /* 0x0000 */ CResRef m_cVidCellRef;
    /* 0x0008 */ CResRef m_cShadowVidCellRef;
    /* 0x0010 */ DWORD m_bltFlags;
    /* 0x0014 */ DWORD m_bltInfo;
    /* 0x0018 */ DWORD m_bltFlagsExtra;
    /* 0x001C */ DWORD m_bltInfoExtra;
    /* 0x0020 */ CPoint m_offset;
    /* 0x0028 */ DWORD m_offsetFlags;
    /* 0x002C */ LONG m_animationSpeed;
    /* 0x0030 */ LONG m_numDirections;
    /* 0x0034 */ LONG m_direction;
    /* 0x0038 */ DWORD m_directionFlags;
    /* 0x003C */ CResRef m_cNewPaletteRef;
    /* 0x0044 */ LONG m_height;
    /* 0x0048 */ CPoint m_glowSize;
    /* 0x0050 */ DWORD m_glowIntensity;
    /* 0x0054 */ LONG m_nDuration;
    /* 0x0058 */ CResRef m_cVVCResRes;
    /* 0x0060 */ LONG m_nStartSequence;
    /* 0x0064 */ LONG m_nLoopSequence;
    /* 0x0068 */ LONG m_nCurrentSequence;
    /* 0x006C */ DWORD m_sequenceFlags;
    /* 0x0070 */ CResRef m_cStartSoundRef;
    /* 0x0078 */ CResRef m_cLoopSoundRef;
    /* 0x0080 */ CResRef m_cAlphaBamRef;
    /* 0x0088 */ LONG m_nEndSequence;
    /* 0x008C */ CResRef m_cEndSoundRef;
    /* 0x0094 */ DWORD m_extra[84];
};

// Recovered prefix through m_newDirection; loading, rendering, sound and the
// remaining tail of the IWD2 animation object have not been recovered yet.
class CVEFVidCell : public CGameObject, public CVEFVidCellBase {
public:
    static const SHORT OFFSET_FROMX_X[16];
    static const SHORT OFFSET_FROMX_Y[16];
    static const SHORT OFFSET_FROMY_X[16];
    static const SHORT OFFSET_FROMY_Y[16];

    void FrameAdvance(LONG nCounter);
    void CalculatePos(const CPoint& pos);
    void SetDirection(const CPoint& pos, const CPoint& oldPos, const CPoint& targetPos, SHORT nDirection);
    static SHORT GetDirection(const CPoint& pos, const CPoint& targetPos, SHORT nDirection);

    /* 0x0252 */ CVidCell* m_cVidCell;
    /* 0x0256 */ CVidCell* m_cShadowVidCell;
    /* 0x025A */ CVidPalette m_palette;
    /* 0x027E */ CVidBitmap m_newPalette;
    /* 0x0338 */ BOOL m_done;
    /* 0x033C */ LONG m_newDirection;
};

class CVisualEffect : public CGameObject, public CVisualEffectBase {
public:
    static const LONG MAX_FRAME_RATE;

    CVisualEffect();
    ~CVisualEffect() override;
    void AIUpdate() override;
    void Render(CGameArea* pArea, CVidMode* pVidMode, int a3) override;
    void SetAllDelete();
    void Move(const CPoint& targetPos);

    /* 0x022E */ CList<LONG, LONG> m_activeAnimations;
    /* 0x024A */ CTypedPtrList<CPtrList, CVEFEvent*> m_timedEvents;
    /* 0x0266 */ CTypedPtrList<CPtrList, CVEFEvent*> m_triggeredEvents;
    /* 0x0282 */ CPoint m_ptTargetPos;
    /* 0x028A */ LONG m_nTargetId;
    /* 0x028E */ CPoint m_posExact;
    /* 0x0296 */ CPoint m_posOld;
    /* 0x029E */ LONG m_nExistanceCounter;
    /* 0x02A2 */ LONG m_nDirection;
    /* 0x02A6 */ LONG m_nNewDirection;
    /* 0x02AA */ CPoint m_posDelta;
    /* 0x02B2 */ BYTE m_terrainTable[16];
};
#pragma pack(pop)

#endif /* CVISUALEFFECT_H_ */
