#include "CColorEffects.h"

#include "CGameSprite.h"
#include "CUtil.h"

// NOTE: Inlined.
CColorRange::CColorRange()
{
    m_range = 0;
    m_color = 0;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CColorEffect::CColorEffect()
{
    m_effectType = -1;
    m_range = 0;
    m_tintColor = 0;
    m_periodLength = 1;
}

// -----------------------------------------------------------------------------

// 0x448430
CColorRanges::CColorRanges()
{
}

// 0x443730
CColorRanges::~CColorRanges()
{
    ClearAll();
}

// 0x4E71B0
void CColorRanges::ClearAll()
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CColorRange* pColorRange = GetNext(pos);
        delete pColorRange;
    }
    RemoveAll();
}

// 0x442E50
ULONG CColorRanges::Marshal(BYTE** ppData)
{
    *ppData = NULL;

    LONG nSize = sizeof(CColorRange) * GetCount();

    // NOTE: Signed compare.
    if (nSize > 0) {
        BYTE* pData = new BYTE[nSize];
        int offset = 0;

        POSITION pos = GetHeadPosition();
        while (pos != NULL) {
            CColorRange* pColorEffect = GetNext(pos);

            pData[offset] = pColorEffect->m_range;
            pData[offset + 1] = pColorEffect->m_color;

            offset += sizeof(CColorRange);
        }

        *ppData = pData;
    }

    return nSize;
}

// 0x442EA0
void CColorRanges::Unmarshal(BYTE* pData, ULONG nSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CColorEffects.cpp
    // __LINE__: 170
    UTIL_ASSERT(nSize % sizeof(CColorRange) == 0);

    int count = nSize / sizeof(CColorRange);
    int offset = 0;

    for (int index = 0; index < count; index++) {
        CColorRange* pColorRange = new CColorRange;
        pColorRange->m_range = pData[offset];
        pColorRange->m_color = pData[offset + 1];
        AddTail(pColorRange);

        offset += sizeof(CColorRange);
    }
}

// 0x442F10
CColorRanges& CColorRanges::operator=(const CColorRanges& other)
{
    // NOTE: Uninline.
    ClearAll();

    POSITION pos = other.GetHeadPosition();
    while (pos != NULL) {
        CColorRange* pColorRange = other.GetNext(pos);

        CColorRange* pCopy = new CColorRange;
        pCopy->m_range = pColorRange->m_range;
        pCopy->m_color = pColorRange->m_color;

        AddTail(pCopy);
    }

    return *this;
}

// 0x442F90
void CColorRanges::Apply(CGameSprite* pSprite)
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CColorRange* pColorRange = GetNext(pos);

        if (pColorRange->m_range != -1) {
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorRange(pColorRange->m_range,
                pColorRange->m_color);
        } else {
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorRangeAll(pColorRange->m_color);
        }
    }
}

// -----------------------------------------------------------------------------

// 0x448450
CColorEffects::CColorEffects()
{
}

// 0x4437A0
CColorEffects::~CColorEffects()
{
    ClearAll();
}

// 0x4E71B0
void CColorEffects::ClearAll()
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CColorEffect* pColorEffect = GetNext(pos);
        delete pColorEffect;
    }
    RemoveAll();
}

// 0x443050
ULONG CColorEffects::Marshal(BYTE** ppData)
{
    *ppData = NULL;

    LONG nSize = sizeof(CColorEffect) * GetCount();

    // NOTE: Signed compare.
    if (nSize > 0) {
        BYTE* pData = new BYTE[nSize];
        int offset = 0;

        POSITION pos = GetHeadPosition();
        while (pos != NULL) {
            CColorEffect* pColorEffect = GetNext(pos);

            pData[offset] = pColorEffect->m_effectType;
            pData[offset + 1] = pColorEffect->m_range;
            *reinterpret_cast<COLORREF*>(&(pData[offset + 2])) = pColorEffect->m_tintColor;
            pData[offset + 6] = pColorEffect->m_periodLength;

            offset += sizeof(CColorEffect);
        }

        *ppData = pData;
    }

    return nSize;
}

// 0x4430A0
void CColorEffects::Unmarshal(BYTE* pData, ULONG nSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CColorEffects.cpp
    // __LINE__: 365
    UTIL_ASSERT(nSize % sizeof(CColorEffect) == 0);

    int count = nSize / sizeof(CColorEffect);
    int offset = 0;

    for (int index = 0; index < count; index++) {
        CColorEffect* pColorEffect = new CColorEffect;
        pColorEffect->m_effectType = pData[offset];
        pColorEffect->m_range = pData[offset + 1];
        pColorEffect->m_tintColor = *reinterpret_cast<COLORREF*>(&(pData[offset + 2]));
        pColorEffect->m_periodLength = pData[offset + 6];
        AddTail(pColorEffect);

        offset += sizeof(CColorEffect);
    }
}

// 0x443130
CColorEffects& CColorEffects::operator=(const CColorEffects& other)
{
    // NOTE: Uninline.
    ClearAll();

    POSITION pos = other.GetHeadPosition();
    while (pos != NULL) {
        CColorEffect* pColorEffect = other.GetNext(pos);

        CColorEffect* pCopy = new CColorEffect;
        pCopy->m_effectType = pColorEffect->m_effectType;
        pCopy->m_range = pColorEffect->m_range;
        pCopy->m_tintColor = pColorEffect->m_tintColor;
        pCopy->m_periodLength = pColorEffect->m_periodLength;

        AddTail(pCopy);
    }

    return *this;
}

// 0x4431C0
void CColorEffects::Apply(CGameSprite* pSprite)
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CColorEffect* pColorEffect = GetNext(pos);

        if (pColorEffect->m_range != -1) {
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorEffect(pColorEffect->m_effectType,
                pColorEffect->m_range,
                pColorEffect->m_tintColor,
                pColorEffect->m_periodLength);
        } else {
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorEffectAll(pColorEffect->m_effectType,
                pColorEffect->m_tintColor,
                pColorEffect->m_periodLength);
        }
    }
}

// Phase 1-2: Scaffold functions
// 0x443710
void FUN_00443710() {
    // TODO: Incomplete.
}

// 0x443780
void FUN_00443780() {
    // TODO: Incomplete.
}

// 0x4437F0
void FUN_004437f0() {
    // TODO: Incomplete.
}

// 0x443810
void FUN_00443810() {
    // TODO: Incomplete.
}

// 0x443860
void FUN_00443860() {
    // TODO: Incomplete.
}

// 0x443880
void FUN_00443880() {
    // TODO: Incomplete.
}

// 0x448130
void FUN_00448130() {
    // TODO: Incomplete.
}

// 0x4481A0
void FUN_004481a0() {
    // TODO: Incomplete.
}

// 0x448250
void FUN_00448250() {
    // TODO: Incomplete.
}

// 0x4482F0
void FUN_004482f0() {
    // TODO: Incomplete.
}

// 0x448310
void FUN_00448310() {
    // TODO: Incomplete.
}

// 0x448330
void FUN_00448330() {
    // TODO: Incomplete.
}

// 0x448350
void FUN_00448350() {
    // TODO: Incomplete.
}

// 0x448370
void FUN_00448370() {
    // TODO: Incomplete.
}

// 0x448390
void FUN_00448390() {
    // TODO: Incomplete.
}

// 0x4483B0
void FUN_004483b0() {
    // TODO: Incomplete.
}

// 0x4483D0
void FUN_004483d0() {
    // TODO: Incomplete.
}

// 0x4483F0
void FUN_004483f0() {
    // TODO: Incomplete.
}

// 0x448410
void FUN_00448410() {
    // TODO: Incomplete.
}

// 0x448470
void FUN_00448470() {
    // TODO: Incomplete.
}

// 0x448550
void FUN_00448550() {
    // TODO: Incomplete.
}

// 0x448580
void FUN_00448580() {
    // TODO: Incomplete.
}

// 0x4485D0
void FUN_004485d0() {
    // TODO: Incomplete.
}

// 0x448630
void FUN_00448630() {
    // TODO: Incomplete.
}

// 0x448680
void FUN_00448680() {
    // TODO: Incomplete.
}

// 0x4486D0
void FUN_004486d0() {
    // TODO: Incomplete.
}

// 0x4486F0
void FUN_004486f0() {
    // TODO: Incomplete.
}

// 0x448700
void FUN_00448700() {
    // TODO: Incomplete.
}

// 0x448730
void FUN_00448730() {
    // TODO: Incomplete.
}

// 0x448740
void FUN_00448740() {
    // TODO: Incomplete.
}

// 0x448980
void FUN_00448980() {
    // TODO: Incomplete.
}

// 0x448A50
void FUN_00448a50() {
    // TODO: Incomplete.
}

// 0x448AC0
void FUN_00448ac0() {
    // TODO: Incomplete.
}

// 0x448AD0
void FUN_00448ad0() {
    // TODO: Incomplete.
}

// 0x448B00
void FUN_00448b00() {
    // TODO: Incomplete.
}

// 0x448BD0
void FUN_00448bd0() {
    // TODO: Incomplete.
}

// 0x448C00
void FUN_00448c00() {
    // TODO: Incomplete.
}

// 0x448C40
void FUN_00448c40() {
    // TODO: Incomplete.
}

// 0x448C70
void FUN_00448c70() {
    // TODO: Incomplete.
}

// 0x448D50
void FUN_00448d50() {
    // TODO: Incomplete.
}

// 0x448DA0
void FUN_00448da0() {
    // TODO: Incomplete.
}

// 0x448DD0
void FUN_00448dd0() {
    // TODO: Incomplete.
}

// 0x449010
void FUN_00449010() {
    // TODO: Incomplete.
}

// 0x4490E0
void FUN_004490e0() {
    // TODO: Incomplete.
}

// 0x449110
void FUN_00449110() {
    // TODO: Incomplete.
}

// 0x449400
void FUN_00449400() {
    // TODO: Incomplete.
}

// 0x449800
void FUN_00449800() {
    // TODO: Incomplete.
}

// 0x449840
void FUN_00449840() {
    // TODO: Incomplete.
}

// 0x449880
void FUN_00449880() {
    // TODO: Incomplete.
}

// 0x449910
void FUN_00449910() {
    // TODO: Incomplete.
}

// 0x449B80
void FUN_00449b80() {
    // TODO: Incomplete.
}

// 0x449ED0
void FUN_00449ed0() {
    // TODO: Incomplete.
}

// 0x44A300
void FUN_0044a300() {
    // TODO: Incomplete.
}

// 0x44A3F0
void FUN_0044a3f0() {
    // TODO: Incomplete.
}

// 0x44A460
void FUN_0044a460() {
    // TODO: Incomplete.
}

