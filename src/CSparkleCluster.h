#ifndef CSPARKLECLUSTER_H_
#define CSPARKLECLUSTER_H_

#include "CGameObject.h"

class CParticle;

#pragma pack(push, 2)
class CSparkleCluster : public CGameObject {
public:
    static const USHORT TYPE_TRAVEL;
    static const USHORT TYPE_CONJURE;
    static const USHORT TYPE_EXPLOSION;
    static const USHORT DEFAULT_CONJURE_DURATION;
    CSparkleCluster(SHORT colorType, USHORT type, USHORT conjureDuration);
    ~CSparkleCluster() override;
    void AddToArea(CGameArea* area, const CPoint& pos, LONG height, BYTE listType) override;
    void AIUpdate() override;
    void Render(CGameArea* area, CVidMode* vidMode, int surface) override;
    void GenerateExplosion();

    /* 0x006E */ CVidMode* m_pVidMode;
    /* 0x0072 */ CTypedPtrList<CPtrList, CParticle*> m_particleList;
    /* 0x008E */ COLORREF m_colorStart;
    /* 0x0092 */ COLORREF m_colorFade1;
    /* 0x0096 */ COLORREF m_colorFade2;
    /* 0x009A */ COLORREF m_colorFade3;
    /* 0x009E */ COLORREF m_colorFade4;
    /* 0x00A2 */ BYTE m_fade1Height;
    /* 0x00A3 */ BYTE m_fade2Height;
    /* 0x00A4 */ BYTE m_fade3Height;
    /* 0x00A5 */ BYTE m_fade4Height;
    /* 0x00A6 */ CRect m_rBounding;
    /* 0x00B6 */ USHORT m_wType;
    /* 0x00B8 */ DWORD m_nTimer;
    /* 0x00BC */ USHORT m_nConjureDuration;
    /* 0x00BE */ CPoint m_explosionDir;
    /* 0x00C6 */ int m_nGravity;
};
#pragma pack(pop)

static_assert(sizeof(CSparkleCluster) - sizeof(CGameObject) == 0xCA - 0x6E,
    "CSparkleCluster native member span must remain packed to two bytes");

#endif
