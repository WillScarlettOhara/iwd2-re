#include "CGameAnimation.h"

#include "CGameAnimationType.h"
#include "CUtil.h"

// NOTE: Inlined.
CGameAnimation::CGameAnimation()
{
    m_animation = NULL;
    m_nSequence = 0;
}

// NOTE: Inlined.
CGameAnimation::~CGameAnimation()
{
    if (m_animation != NULL) {
        delete m_animation;
    }
}

// NOTE: Inlined.
void CGameAnimation::CalculateFxRect(CRect& rFx, CPoint& ptReference, LONG posZ)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2071
    UTIL_ASSERT(m_animation != NULL);

    m_animation->CalculateFxRect(rFx, ptReference, posZ);
}

// 0x56E210
void CGameAnimation::CalculateGCBoundsRect(CRect& rGCBounds, const CPoint& pos, const CPoint& ptReference, LONG posZ, LONG nWidth, LONG nHeight)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2073
    UTIL_ASSERT(m_animation != NULL);

    m_animation->CalculateGCBoundsRect(rGCBounds, pos, ptReference, posZ, nWidth, nHeight);
}

// NOTE: Inlined.
void CGameAnimation::ChangeDirection(SHORT nDirection)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2074
    UTIL_ASSERT(m_animation != NULL);

    m_animation->ChangeDirection(nDirection);
}

// NOTE: Inlined.
void CGameAnimation::EquipArmor(CHAR armorLevel, BYTE* colorRangeValues)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2075
    UTIL_ASSERT(m_animation != NULL);

    m_animation->EquipArmor(armorLevel, colorRangeValues);
}

// 0x45B690
USHORT CGameAnimation::GetAnimationId()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2115
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->m_animationID;
}

// NOTE: Inlined.
void CGameAnimation::GetAnimationResRef(CString& resRef, BYTE range)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2080
    UTIL_ASSERT(m_animation != NULL);

    m_animation->GetAnimationResRef(resRef, range);
}

// NOTE: Inlined.
BOOLEAN CGameAnimation::CanLieDown()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2081
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->CanLieDown();
}

// NOTE: Inlined.
BOOLEAN CGameAnimation::DetectedByInfravision()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2082
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->DetectedByInfravision();
}

// NOTE: Inlined.
BYTE CGameAnimation::GetColorBlood()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2084
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetColorBlood();
}

// NOTE: Inlined.
void CGameAnimation::SetAnimationType(USHORT animationID, BYTE* colorRangeValues, WORD facing)
{
    if (m_animation != NULL) {
        delete m_animation;
    }

    m_animation = CGameAnimationType::SetAnimationType(animationID, colorRangeValues, facing);
}

// NOTE: Inlined.
BYTE CGameAnimation::GetListType()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2086
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetListType();
}

// NOTE: Inlined.
BYTE CGameAnimation::GetMoveScale()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2087
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetMoveScale();
}

// NOTE: Inlined.
BYTE CGameAnimation::GetMoveScaleDefault()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2088
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetMoveScaleDefault();
}

// NOTE: Inlined.
void CGameAnimation::SetMoveScale(BYTE scale)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2089
    UTIL_ASSERT(m_animation != NULL);

    m_animation->SetMoveScale(scale);
}

// NOTE: Inlined.
void CGameAnimation::ResetMoveScale()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2090
    UTIL_ASSERT(m_animation != NULL);

    m_animation->ResetMoveScale();
}

// NOTE: Inlined.
const CRect& CGameAnimation::GetEllipseRect()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2091
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetEllipseRect();
}

// NOTE: Inlined.
BOOL CGameAnimation::GetPathSmooth()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2092
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetPathSmooth();
}

// NOTE: Inlined.
BYTE CGameAnimation::GetPersonalSpace()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2093
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetPersonalSpace();
}

// NOTE: Inlined.
char* CGameAnimation::GetSndArmor()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2094
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetSndArmor();
}

// NOTE: Inlined.
char* CGameAnimation::GetSndReady()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2096
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetSndReady();
}

// NOTE: Inlined.
char* CGameAnimation::GetSndWalk(SHORT tableIndex)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2097
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetSndWalk(tableIndex);
}

// NOTE: Inlined.
DWORD CGameAnimation::GetSndWalkFreq()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2098
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetSndWalkFreq();
}

// NOTE: Inlined.
BOOL CGameAnimation::IsFalseColor()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2099
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->IsFalseColor();
}

// NOTE: Inlined.
BOOL CGameAnimation::IsBeginningOfSequence()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2102
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->IsBeginningOfSequence();
}

// NOTE: Inlined.
BOOL CGameAnimation::IsEndOfSequence()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2103
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->IsEndOfSequence();
}

// NOTE: Inlined.
void CGameAnimation::IncrementFrame()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2104
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->IncrementFrame();
}

// NOTE: Inlined.
void CGameAnimation::DecrementFrame()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2105
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->DecrementFrame();
}

// NOTE: Inlined.
void CGameAnimation::Render(CInfinity* pInfinity, CVidMode* pVidMode, INT nSurface, const CRect& rectFX, const CPoint& ptNewPos, const CPoint& ptReference, DWORD dwRenderFlags, COLORREF rgbTintColor, const CRect& rGCBounds, BOOL bDithered, BOOL bFadeOut, LONG posZ, BYTE transparency)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2106
    UTIL_ASSERT(m_animation != NULL);

    m_animation->Render(pInfinity,
        pVidMode,
        nSurface,
        rectFX,
        ptNewPos,
        ptReference,
        dwRenderFlags,
        rgbTintColor,
        rGCBounds,
        bDithered,
        bFadeOut,
        posZ,
        transparency);
}

// NOTE: Uninline.
void CGameAnimation::ClearColorEffectsAll()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2108
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->ClearColorEffectsAll();
}

// NOTE: Inlined.
void CGameAnimation::SetColorEffect(BYTE effectType, BYTE colorRange, COLORREF tintColor, BYTE periodLength)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2109
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->SetColorEffect(effectType, colorRange, tintColor, periodLength);
}

// NOTE: Inlined.
void CGameAnimation::SetColorEffectAll(BYTE effectType, COLORREF tintColor, BYTE periodLength)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2110
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->SetColorEffectAll(effectType, tintColor, periodLength);
}

// NOTE: Inlined.
void CGameAnimation::SetColorRange(BYTE colorRange, BYTE rangeValue)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2111
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->SetColorRange(colorRange, rangeValue);
}

// NOTE: Inlined.
void CGameAnimation::SetColorRangeAll(BYTE rangeValue)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2112
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->SetColorRangeAll(rangeValue);
}

// NOTE: Inlined.
SHORT CGameAnimation::SetSequence(SHORT nSequence)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2113
    UTIL_ASSERT(m_animation != NULL);

    m_nSequence = nSequence;

    return m_animation->SetSequence(nSequence);
}

// NOTE: Inlined.
BYTE CGameAnimation::GetAttackFrameType(BYTE numAttacks, BYTE speedFactor, BYTE combatFrame)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2114
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetAttackFrameType(numAttacks, speedFactor, combatFrame);
}

// NOTE: Inlined.
BOOL CGameAnimation::GetAboveGround()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2116
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetAboveGround();
}

// NOTE: Inlined.
SHORT CGameAnimation::GetCastHeight()
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2117
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetCastHeight();
}

// NOTE: Inlined.
SHORT CGameAnimation::GetCurrentFrame()
{
    return m_animation->GetCurrentFrame();
}

// NOTE: Inlined.
BOOLEAN CGameAnimation::GetCurrentResRef(CString& resBody, CString& resWeapon, CString& resShield, CString& resHelmet)
{
    // __FILE__: .\Include\ObjAnimation.h
    // __LINE__: 2120
    UTIL_ASSERT(m_animation != NULL);

    return m_animation->GetCurrentResRef(resBody, resWeapon, resShield, resHelmet);
}

// Phase 1-2: Scaffold functions
// 0x56D9B0
void FUN_0056d9b0() {
    // TODO: Incomplete.
}

// 0x56DA10
void FUN_0056da10() {
    // TODO: Incomplete.
}

// 0x56DB30
void FUN_0056db30() {
    // TODO: Incomplete.
}

// 0x56DF00
void FUN_0056df00() {
    // TODO: Incomplete.
}

// 0x56E1F0
void FUN_0056e1f0() {
    // TODO: Incomplete.
}

// 0x56E260
void FUN_0056e260() {
    // TODO: Incomplete.
}

// 0x56E280
void FUN_0056e280() {
    // TODO: Incomplete.
}

// 0x56E580
void FUN_0056e580() {
    // TODO: Incomplete.
}

// 0x56ECF0
void FUN_0056ecf0() {
    // TODO: Incomplete.
}

// 0x56EDD0
void FUN_0056edd0() {
    // TODO: Incomplete.
}

// 0x56F070
void FUN_0056f070() {
    // TODO: Incomplete.
}

// 0x56F090
void FUN_0056f090() {
    // TODO: Incomplete.
}

// 0x56F180
void FUN_0056f180() {
    // TODO: Incomplete.
}

// 0x56F1F0
void FUN_0056f1f0() {
    // TODO: Incomplete.
}

// 0x56F410
void FUN_0056f410() {
    // TODO: Incomplete.
}

// 0x56F820
void FUN_0056f820() {
    // TODO: Incomplete.
}

// 0x56FAF0
void FUN_0056faf0() {
    // TODO: Incomplete.
}

// 0x56FDC0
void FUN_0056fdc0() {
    // TODO: Incomplete.
}

// 0x56FE30
void FUN_0056fe30() {
    // TODO: Incomplete.
}

// 0x56FEC0
void FUN_0056fec0() {
    // TODO: Incomplete.
}

// 0x56FED0
void FUN_0056fed0() {
    // TODO: Incomplete.
}

// 0x5701B0
void FUN_005701b0() {
    // TODO: Incomplete.
}

// 0x5701E0
void FUN_005701e0() {
    // TODO: Incomplete.
}

// 0x5703F0
void FUN_005703f0() {
    // TODO: Incomplete.
}

// 0x5704E0
void FUN_005704e0() {
    // TODO: Incomplete.
}

// 0x5705D0
void FUN_005705d0() {
    // TODO: Incomplete.
}

// 0x570620
void FUN_00570620() {
    // TODO: Incomplete.
}

// 0x570690
void FUN_00570690() {
    // TODO: Incomplete.
}

// 0x570780
void FUN_00570780() {
    // TODO: Incomplete.
}

// 0x570B80
void FUN_00570b80() {
    // TODO: Incomplete.
}

// 0x570C50
void FUN_00570c50() {
    // TODO: Incomplete.
}

// 0x570CC0
void FUN_00570cc0() {
    // TODO: Incomplete.
}

// 0x570D10
void FUN_00570d10() {
    // TODO: Incomplete.
}

// 0x570D50
void FUN_00570d50() {
    // TODO: Incomplete.
}

// 0x570DE0
void FUN_00570de0() {
    // TODO: Incomplete.
}

// 0x570E00
void FUN_00570e00() {
    // TODO: Incomplete.
}

// 0x571070
void FUN_00571070() {
    // TODO: Incomplete.
}

// 0x5710D0
void FUN_005710d0() {
    // TODO: Incomplete.
}

// 0x571170
void FUN_00571170() {
    // TODO: Incomplete.
}

// 0x571310
void FUN_00571310() {
    // TODO: Incomplete.
}

// 0x5714E0
void FUN_005714e0() {
    // TODO: Incomplete.
}

// 0x571930
void FUN_00571930() {
    // TODO: Incomplete.
}

// 0x571A60
void FUN_00571a60() {
    // TODO: Incomplete.
}

// 0x571B30
void FUN_00571b30() {
    // TODO: Incomplete.
}

// 0x571CE0
void FUN_00571ce0() {
    // TODO: Incomplete.
}

// 0x571E80
void FUN_00571e80() {
    // TODO: Incomplete.
}

// 0x572080
void FUN_00572080() {
    // TODO: Incomplete.
}

// 0x572110
void FUN_00572110() {
    // TODO: Incomplete.
}

