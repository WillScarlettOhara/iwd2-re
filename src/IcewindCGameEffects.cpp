#include "IcewindCGameEffects.h"

#include "CCreatureFile.h"
#include "CBaldurChitin.h"
#include "CGameArea.h"
#include "CGameObjectArray.h"
#include "CGameSprite.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CProjectile.h"
#include "CSpell.h"
#include "CInfGame.h"
#include "CTimerWorld.h"
#include "CUtil.h"
#include "Icewind586B70.h"
#include "IcewindMisc.h"

// NOTE: I'm not sure where these constants belong.

// 0x8AEBCC
const char RESREF_EYE_OF_THE_MIND[] = "SpIn126";

// 0x8AEBD0
const char RESREF_EYE_OF_THE_SWORD[] = "SpIn127";

// 0x8AEBD4
const char RESREF_EYE_OF_THE_MAGE[] = "SpIn128";

// 0x8AEBD8
const char RESREF_EYE_OF_VENOM[] = "SpIn129";

// 0x8AEBDC
const char RESREF_EYE_OF_THE_SPIRIT[] = "SpIn130";

// 0x8AEBE0
const char RESREF_EYE_OF_FORTITUDE[] = "SpIn131";

// 0x8AEBE4
const char RESREF_EYE_OF_STONE[] = "SpIn132";

static const DWORD CREATURE_FLAG_SUMMONED = 0x10000000;
static const DWORD SUMMON_VISIBLE_TIME = 0x7FFFFFFF;
static const WORD SUMMON_CREATURE_TYPE_GOOD = 2;
static const WORD SUMMON_CREATURE_TYPE_EVIL = 0;
static const char SUMMON_GROUP_VFX_SOUND[] = "EFF_M13";

// TODO INCOMPLETE: this recovers only the summon-group projectile cases from
// the 0x51EAF0 projectile factory. Recover the broader factory on demand.
static CProjectileSummonVFX* CreateSummonGroupProjectile(int vfxId)
{
    CResRef visualResRef;
    BOOL arrivalSound = FALSE;

    switch (vfxId) {
    case 0x120:
        visualResRef = "MSumm1X";
        arrivalSound = TRUE;
        break;
    case 0x121:
        visualResRef = "ASumm1X";
        arrivalSound = TRUE;
        break;
    case 0x122:
        visualResRef = "CEElemX";
        arrivalSound = TRUE;
        break;
    case 0x123:
        visualResRef = "CFElemX";
        arrivalSound = TRUE;
        break;
    case 0x124:
        visualResRef = "CWElemX";
        arrivalSound = TRUE;
        break;
    case 0x168:
        visualResRef = "GateX";
        break;
    default:
        return NULL;
    }

    CProjectileSummonVFX* pProjectile = new CProjectileSummonVFX(visualResRef, IcewindCVisualEffect());
    pProjectile->m_projectileType = static_cast<WORD>(vfxId);
    if (arrivalSound) {
        pProjectile->SetArrivalSound(CResRef(SUMMON_GROUP_VFX_SOUND));
        pProjectile->SetOffsetAboveTarget(TRUE);
    }
    return pProjectile;
}

// 0x49DB70
IcewindCGameEffectBeltynsBurningBlood::IcewindCGameEffectBeltynsBurningBlood(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49DBB0
CGameEffect* IcewindCGameEffectBeltynsBurningBlood::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectBeltynsBurningBlood* copy = new IcewindCGameEffectBeltynsBurningBlood(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49DC60
IcewindCGameEffectCastingGlow::IcewindCGameEffectCastingGlow(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49DCA0
CGameEffect* IcewindCGameEffectCastingGlow::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectCastingGlow* copy = new IcewindCGameEffectCastingGlow(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55E130
// INCOMPLETE: only the CastingGlow BAM projectile path recovered; the global
// projectile switch (0x57B0C0) dispatching all projectile types is not yet
// implemented. Only effects 1-8 (casting glow schools) fire CProjectileBAM.
BOOL IcewindCGameEffectCastingGlow::ApplyEffect(CGameSprite* pSprite)
{
    WORD projectileType;
    CResRef visualResRef;

    switch (m_dwFlags) {
    case 1:
        projectileType = 0x71;
        visualResRef = "AbjurCG";
        break;
    case 2:
        projectileType = 0x73;
        visualResRef = "ConjuCG";
        break;
    case 3:
        projectileType = 0x75;
        visualResRef = "DivinCG";
        break;
    case 4:
        projectileType = 0x70;
        visualResRef = "EnchaCG";
        break;
    case 5:
        projectileType = 0x72;
        visualResRef = "IllusCG";
        break;
    case 6:
        projectileType = 0x74;
        visualResRef = "InvocCG";
        break;
    case 7:
        projectileType = 0x6E;
        visualResRef = "NecroCG";
        break;
    case 8:
        projectileType = 0x6F;
        visualResRef = "AlterCG";
        break;
    default:
        m_done = TRUE;
        return TRUE;
    }

    IcewindCVisualEffect visualEffect;
    visualEffect.SetCopyFromBack(TRUE);

    BYTE sequenceDelay = static_cast<BYTE>(m_duration - g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime);
    CProjectileBAM* pProjectile = new CProjectileBAM(visualResRef,
        CResRef(""),
        sequenceDelay,
        0,
        visualEffect);
    pProjectile->m_projectileType = projectileType;

    CPoint target = pSprite->GetPos();
    pProjectile->Fire(pSprite->GetArea(),
        pSprite->GetId(),
        pSprite->GetId(),
        target,
        pProjectile->DetermineHeight(pSprite),
        0);

    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49DD50
IcewindCGameEffectPrayer::IcewindCGameEffectPrayer(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49DD90
CGameEffect* IcewindCGameEffectPrayer::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectPrayer* copy = new IcewindCGameEffectPrayer(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55E850
BOOL IcewindCGameEffectPrayer::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_BENEFICIAL_PRAYER)) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_BENEFICIAL_PRAYER, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 62);

            AddColorEffect(pSprite, 215, 182, 0, 30);

            pSprite->GetDerivedStats()->m_nDamageBonus++;
            pSprite->GetDerivedStats()->m_nTHAC0++;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude++;
            pSprite->GetDerivedStats()->m_nSaveVSReflex++;
            pSprite->GetDerivedStats()->m_nSaveVSWill++;
        }
        break;
    case 1:
        if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DETRIMENTAL_PRAYER]) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DETRIMENTAL_PRAYER, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 35);

            AddColorEffect(pSprite, 128, 0, 92, 30);

            pSprite->GetDerivedStats()->m_nDamageBonus--;
            pSprite->GetDerivedStats()->m_nTHAC0--;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude--;
            pSprite->GetDerivedStats()->m_nSaveVSReflex--;
            pSprite->GetDerivedStats()->m_nSaveVSWill--;
        }
        break;
    }
    return TRUE;
}

// 0x55ECF0
void IcewindCGameEffectPrayer::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x49DF30
IcewindCGameEffectRighteousWrathOfFaithful::IcewindCGameEffectRighteousWrathOfFaithful(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49DF70
CGameEffect* IcewindCGameEffectRighteousWrathOfFaithful::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRighteousWrathOfFaithful* copy = new IcewindCGameEffectRighteousWrathOfFaithful(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55E9C0
BOOL IcewindCGameEffectRighteousWrathOfFaithful::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_ALLIED_RIGHTEOUS_WRATH_OF_THE_FAITHFUL)) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_ALLIED_RIGHTEOUS_WRATH_OF_THE_FAITHFUL, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 67);

            AddColorEffect(pSprite, 215, 182, 0, 30);

            pSprite->GetDerivedStats()->m_nDamageBonus++;
            pSprite->GetDerivedStats()->m_nTHAC0++;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude++;
            pSprite->GetDerivedStats()->m_nSaveVSReflex++;
            pSprite->GetDerivedStats()->m_nSaveVSWill++;
            pSprite->GetDerivedStats()->m_nMaxHitPoints += 8;

            if (m_secondaryType) {
                pSprite->GetBaseStats()->m_hitPoints += 8;
            }
        }
        break;
    case 1:
        if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_ALLIED_AND_SAME_ALIGNMENT_RIGHTEOUS_WRATH_OF_THE_FAITHFUL)) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_ALLIED_AND_SAME_ALIGNMENT_RIGHTEOUS_WRATH_OF_THE_FAITHFUL, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 67);

            AddColorEffect(pSprite, 215, 182, 0, 30);

            pSprite->GetDerivedStats()->m_nDamageBonus += 2;
            pSprite->GetDerivedStats()->m_nTHAC0 += 2;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude += 2;
            pSprite->GetDerivedStats()->m_nSaveVSReflex += 2;
            pSprite->GetDerivedStats()->m_nSaveVSWill += 2;

            if (m_secondaryType) {
                pSprite->GetBaseStats()->m_hitPoints += 8;
            }

            if (pSprite->GetDerivedStats()->m_nNumberOfAttacks < 5) {
                pSprite->GetDerivedStats()->m_nNumberOfAttacks++;
            }
        }
        break;
    }
    return TRUE;
}

// 0x55EB80
void IcewindCGameEffectRighteousWrathOfFaithful::OnRemove(CGameSprite* pSprite)
{
    pSprite->m_bonusStats.m_nFatigue += 8;
    PlaySound(CResRef("EFF_E01"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x49DE40
IcewindCGameEffectRecitation::IcewindCGameEffectRecitation(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49DE80
CGameEffect* IcewindCGameEffectRecitation::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRecitation* copy = new IcewindCGameEffectRecitation(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55EBB0
BOOL IcewindCGameEffectRecitation::ApplyEffect(CGameSprite* pSprite)
{
    SHORT nModifier;

    switch (m_dwFlags) {
    case 0:
        if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_BENEFICIAL_RECITATION)) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_BENEFICIAL_RECITATION, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 64);

            AddColorEffect(pSprite, 160, 128, 96, 30);

            nModifier = 2;
            pSprite->GetDerivedStats()->m_nDamageBonus += nModifier;
            pSprite->GetDerivedStats()->m_nTHAC0 += nModifier;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude += nModifier;
            pSprite->GetDerivedStats()->m_nSaveVSReflex += nModifier;
            pSprite->GetDerivedStats()->m_nSaveVSWill += nModifier;
        }
        break;
    case 1:
        if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DETRIMENTAL_RECITATION]) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DETRIMENTAL_RECITATION, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 35);

            AddColorEffect(pSprite, 128, 0, 192, 30);

            nModifier = -2;
            pSprite->GetDerivedStats()->m_nDamageBonus += nModifier;
            pSprite->GetDerivedStats()->m_nTHAC0 += nModifier;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude += nModifier;
            pSprite->GetDerivedStats()->m_nSaveVSReflex += nModifier;
            pSprite->GetDerivedStats()->m_nSaveVSWill += nModifier;
        }
        break;
    }
    return TRUE;
}

// 0x55ECF0
void IcewindCGameEffectRecitation::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x49E020
IcewindCGameEffectSavingThrowChange::IcewindCGameEffectSavingThrowChange(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E060
CGameEffect* IcewindCGameEffectSavingThrowChange::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSavingThrowChange* copy = new IcewindCGameEffectSavingThrowChange(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55ED20
BOOL IcewindCGameEffectSavingThrowChange::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        ApplyBonusToAllSaves(pSprite);
        break;
    case 1:
        SetAllSaves(pSprite);
        break;
    case 2:
        ApplyPercentToAllSaves(pSprite);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameEffects.cpp
        // __LINE__: 809
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// 0x55ED90
void IcewindCGameEffectSavingThrowChange::ApplyBonusToAllSaves(CGameSprite* pSprite)
{
    if (m_durationType == 1) {
        CCreatureFileHeader* pBStats = pSprite->GetBaseStats();
        pBStats->m_saveVSFortitudeBase += static_cast<BYTE>(m_effectAmount);
        if (pBStats->m_saveVSFortitudeBase > 40) {
            pBStats->m_saveVSFortitudeBase = 40;
        }
        if (pBStats->m_saveVSFortitudeBase < -40) {
            pBStats->m_saveVSFortitudeBase = -40;
        }

        pBStats->m_saveVSReflexBase += static_cast<BYTE>(m_effectAmount);
        if (pBStats->m_saveVSReflexBase > 40) {
            pBStats->m_saveVSReflexBase = 40;
        }
        if (pBStats->m_saveVSReflexBase < -40) {
            pBStats->m_saveVSReflexBase = -40;
        }

        pBStats->m_saveVSWillBase += static_cast<BYTE>(m_effectAmount);
        if (pBStats->m_saveVSWillBase > 40) {
            pBStats->m_saveVSWillBase = 40;
        }
        if (pBStats->m_saveVSWillBase < -40) {
            pBStats->m_saveVSWillBase = -40;
        }
        m_forceRepass = TRUE;
        m_done = TRUE;
    } else {
        pSprite->m_bonusStats.m_nSaveVSFortitude += static_cast<SHORT>(m_effectAmount);
        pSprite->m_bonusStats.m_nSaveVSReflex += static_cast<SHORT>(m_effectAmount);
        pSprite->m_bonusStats.m_nSaveVSWill += static_cast<SHORT>(m_effectAmount);
        m_done = FALSE;
    }
}

// 0x55EE70
void IcewindCGameEffectSavingThrowChange::SetAllSaves(CGameSprite* pSprite)
{
    if (m_durationType == 1) {
        CCreatureFileHeader* pBStats = pSprite->GetBaseStats();
        pBStats->m_saveVSFortitudeBase = static_cast<BYTE>(m_effectAmount);
        if (pBStats->m_saveVSFortitudeBase > 40) {
            pBStats->m_saveVSFortitudeBase = 40;
        }
        if (pBStats->m_saveVSFortitudeBase < -40) {
            pBStats->m_saveVSFortitudeBase = -40;
        }

        pBStats->m_saveVSReflexBase = static_cast<BYTE>(m_effectAmount);
        if (pBStats->m_saveVSReflexBase > 40) {
            pBStats->m_saveVSReflexBase = 40;
        }
        if (pBStats->m_saveVSReflexBase < -40) {
            pBStats->m_saveVSReflexBase = -40;
        }

        pBStats->m_saveVSWillBase = static_cast<BYTE>(m_effectAmount);
        if (pBStats->m_saveVSWillBase > 40) {
            pBStats->m_saveVSWillBase = 40;
        }
        if (pBStats->m_saveVSWillBase < -40) {
            pBStats->m_saveVSWillBase = -40;
        }
        m_forceRepass = TRUE;
        m_done = TRUE;
    } else {
        CDerivedStats* pDStats = pSprite->GetDerivedStats();
        pDStats->m_nSaveVSFortitude = static_cast<BYTE>(m_effectAmount);
        if (pDStats->m_nSaveVSFortitude > 40) {
            pDStats->m_nSaveVSFortitude = 40;
        }
        if (pDStats->m_nSaveVSFortitude < -40) {
            pDStats->m_nSaveVSFortitude = -40;
        }

        pDStats->m_nSaveVSReflex = static_cast<BYTE>(m_effectAmount);
        if (pDStats->m_nSaveVSReflex > 40) {
            pDStats->m_nSaveVSReflex = 40;
        }
        if (pDStats->m_nSaveVSReflex < -40) {
            pDStats->m_nSaveVSReflex = -40;
        }

        pDStats->m_nSaveVSWill = static_cast<BYTE>(m_effectAmount);
        if (pDStats->m_nSaveVSWill > 40) {
            pDStats->m_nSaveVSWill = 40;
        }
        if (pDStats->m_nSaveVSWill < -40) {
            pDStats->m_nSaveVSWill = -40;
        }
        m_done = FALSE;
    }
}

// 0x55EFB0
void IcewindCGameEffectSavingThrowChange::ApplyPercentToAllSaves(CGameSprite* pSprite)
{
    if (m_durationType == 1) {
        CCreatureFileHeader* pBStats = pSprite->GetBaseStats();
        pBStats->m_saveVSFortitudeBase = static_cast<SHORT>(m_effectAmount) * pBStats->m_saveVSFortitudeBase / 100;
        if (pBStats->m_saveVSFortitudeBase > 40) {
            pBStats->m_saveVSFortitudeBase = 40;
        }
        if (pBStats->m_saveVSFortitudeBase < -40) {
            pBStats->m_saveVSFortitudeBase = -40;
        }

        pBStats->m_saveVSReflexBase = static_cast<SHORT>(m_effectAmount) * pBStats->m_saveVSReflexBase / 100;
        if (pBStats->m_saveVSReflexBase > 40) {
            pBStats->m_saveVSReflexBase = 40;
        }
        if (pBStats->m_saveVSReflexBase < -40) {
            pBStats->m_saveVSReflexBase = -40;
        }

        pBStats->m_saveVSWillBase = static_cast<SHORT>(m_effectAmount) * pBStats->m_saveVSWillBase / 100;
        if (pBStats->m_saveVSWillBase > 40) {
            pBStats->m_saveVSWillBase = 40;
        }
        if (pBStats->m_saveVSWillBase < -40) {
            pBStats->m_saveVSWillBase = -40;
        }
        m_forceRepass = TRUE;
        m_done = TRUE;
    } else {
        CCreatureFileHeader* pBStats = pSprite->GetBaseStats();
        CDerivedStats* pDStats = pSprite->GetDerivedStats();
        pDStats->m_nSaveVSFortitude = static_cast<SHORT>(m_effectAmount) * pBStats->m_saveVSFortitudeBase / 100;
        if (pDStats->m_nSaveVSFortitude > 40) {
            pDStats->m_nSaveVSFortitude = 40;
        }
        if (pDStats->m_nSaveVSFortitude < -40) {
            pDStats->m_nSaveVSFortitude = -40;
        }

        pDStats->m_nSaveVSReflex = static_cast<SHORT>(m_effectAmount) * pBStats->m_saveVSReflexBase / 100;
        if (pDStats->m_nSaveVSReflex > 40) {
            pDStats->m_nSaveVSReflex = 40;
        }
        if (pDStats->m_nSaveVSReflex < -40) {
            pDStats->m_nSaveVSReflex = -40;
        }

        pDStats->m_nSaveVSWill = static_cast<SHORT>(m_effectAmount) * pBStats->m_saveVSWillBase / 100;
        if (pDStats->m_nSaveVSWill > 40) {
            pDStats->m_nSaveVSWill = 40;
        }
        if (pDStats->m_nSaveVSWill < -40) {
            pDStats->m_nSaveVSWill = -40;
        }
        m_done = FALSE;
    }
}

// -----------------------------------------------------------------------------

// 0x49E110
IcewindCGameEffectSlowPoison::IcewindCGameEffectSlowPoison(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E150
CGameEffect* IcewindCGameEffectSlowPoison::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSlowPoison* copy = new IcewindCGameEffectSlowPoison(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55F180
BOOL IcewindCGameEffectSlowPoison::ApplyEffect(CGameSprite* pSprite)
{
    POSITION pos = pSprite->GetTimedEffectList()->GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* pEffect = pSprite->GetTimedEffectList()->GetNext(pos);
        if (pEffect->m_effectID == CGAMEEFFECT_POISON) {
            switch (pEffect->m_dwFlags) {
            case 2:
                pEffect->m_dwFlags = 4;
                pEffect->m_duration = 8 * pEffect->m_duration
                    - 7 * g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
                break;
            case 3:
                pEffect->m_effectAmount *= 7;
                pEffect->m_duration = 8 * pEffect->m_duration
                    - 7 * g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
                break;
            case 4:
                pEffect->m_dwFlags = 5;
                pEffect->m_duration = 8 * pEffect->m_duration
                    - 7 * g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
                break;
            }
        }
    }
    m_forceRepass = TRUE;
    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49E200
IcewindCGameEffectVampiricTouch::IcewindCGameEffectVampiricTouch(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49E240
CGameEffect* IcewindCGameEffectVampiricTouch::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectVampiricTouch* copy = new IcewindCGameEffectVampiricTouch(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49E2F0
IcewindCGameEffectColorGlowDissipate::IcewindCGameEffectColorGlowDissipate(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E330
CGameEffect* IcewindCGameEffectColorGlowDissipate::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectColorGlowDissipate* copy = new IcewindCGameEffectColorGlowDissipate(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x55F500
BOOL IcewindCGameEffectColorGlowDissipate::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->m_hasColorEffects = TRUE;

    CPersistantEffectColorEffect* pEffect = new CPersistantEffectColorEffect();
    pEffect->m_effectFinalRed = GetRValue(m_effectAmount >> 8);
    pEffect->m_effectFinalGreen = GetGValue(m_effectAmount >> 8);
    pEffect->m_effectFinalBlue = GetBValue(m_effectAmount >> 8);
    pEffect->m_effectDegreeRed = pEffect->m_effectFinalRed;
    pEffect->m_effectDegreeGreen = pEffect->m_effectFinalGreen;
    pEffect->m_effectDegreeBlue = pEffect->m_effectFinalBlue;
    pEffect->m_undo = TRUE;
    pEffect->m_colorEffect = 3;
    pEffect->m_period = (m_dwFlags >> 16) & 0xFFFF;
    pSprite->m_persistantEffects.AddTail(pEffect);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x55F5E0
IcewindCGameEffectSummon::IcewindCGameEffectSummon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
    m_pSprite = NULL;
    field_190 = 0;
}

// 0x55F620
CGameEffect* IcewindCGameEffectSummon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSummon* copy = new IcewindCGameEffectSummon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x49DB60
void IcewindCGameEffectSummon::SetSummonDelay(int a1)
{
    field_190 = a1;
}

// 0x55F710
// SUMMONALLY (opcode 410) / SUMMONENEMY (opcode 411). Caller cached on first
// apply (m_pSprite). Roll m_numDice d m_diceSize and add to m_effectAmount to
// get total count. For each: bail with summon-limit feedback if the engine
// rejects the new sprite, otherwise call SpawnFromResRef and (optionally)
// attach an Unsummon effect bounded by m_duration.
BOOL IcewindCGameEffectSummon::ApplyEffect(CGameSprite* pSprite)
{
    if (m_pSprite == NULL) {
        m_pSprite = pSprite;
    }

    int total = static_cast<int>(m_effectAmount);
    for (DWORD i = 0; i < m_numDice; ++i) {
        int roll = (m_diceSize == 0) ? 0 : (rand() % static_cast<int>(m_diceSize));
        total += roll + 1;
    }

    CGameSprite* pCaster = m_pSprite;
    for (int i = 0; i < total; ++i) {
        if (!Icewind586B70::Instance()->CanJoinParty(pCaster)) {
            pCaster->FeedBack(50, 0, 0, 0, -1, 0, 0);
            break;
        }

        CString resRef = m_res.GetResRefStr();
        CGameSprite* pSpawned = SpawnFromResRef(resRef, &m_target);

        if ((m_durationType == 0 || m_durationType == 0x1000 || m_durationType == 3)
            && pSpawned != NULL && m_durationType != 1) {
            DWORD now = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
            DWORD secs = (m_duration - now) / 15;
            CGameEffect* pUnsummon = IcewindMisc::CreateEffectSkillUnsummon(pSpawned, secs, 0);
            pUnsummon->m_firstCall = m_firstCall;
            pSpawned->AddEffect(pUnsummon, /*list*/ 1, /*noSave*/ TRUE, /*immediateApply*/ TRUE);
        }

        AddSummonVisualHit(pSpawned, (field_190 != 0) ? 0x15 : 0);
    }

    int vfxId;
    switch (field_190) {
    case 1: vfxId = 0x120; break;
    case 2: vfxId = 0x121; break;
    case 3: vfxId = 0x122; break;
    case 4: vfxId = 0x123; break;
    case 5: vfxId = 0x124; break;
    case 6: vfxId = 0x168; break;
    case 0:
    case 7: vfxId = 0; break;
    default:
        UTIL_ASSERT(FALSE);
        vfxId = 0;
    }
    PlayGroupVFX(vfxId);

    m_done = TRUE;
    return TRUE;
}

// 0x55FAD0
CGameSprite* IcewindCGameEffectSummon::SpawnFromResRef(const CString& resRef, const CPoint* pTarget)
{
    CGameArea* pArea;
    CCreatureFile creature;
    CGameSprite* pSpawned;
    BYTE* pCreatureData;
    DWORD creatureSize;
    POINT spawnGrid;
    CPoint spawnPos;
    BYTE rc;

    if (resRef.IsEmpty() || m_pSprite == NULL || pTarget == NULL) {
        return NULL;
    }

    pArea = m_pSprite->GetArea();
    if (pArea == NULL) {
        return NULL;
    }

    spawnGrid.x = pTarget->x / CPathSearch::GRID_SQUARE_SIZEX;
    spawnGrid.y = pTarget->y / CPathSearch::GRID_SQUARE_SIZEY;

    pArea->m_search.FindNearbyPassablePoint(&spawnGrid,
        spawnGrid.x,
        spawnGrid.y,
        m_pSprite->GetTerrainTable(),
        m_pSprite->GetAnimation()->GetPersonalSpace(),
        -1);

    spawnPos.x = spawnGrid.x * CPathSearch::GRID_SQUARE_SIZEX;
    spawnPos.y = spawnGrid.y * CPathSearch::GRID_SQUARE_SIZEY;

    creature.SetResRef(CResRef(resRef), TRUE, TRUE);
    creatureSize = creature.GetDataSize();
    pCreatureData = creature.GetData();
    if (pCreatureData == NULL || creatureSize == 0) {
        creature.ReleaseData();
        return NULL;
    }

    WORD creatureType = IcewindMisc::IsGoodByEA(m_pSprite) ? SUMMON_CREATURE_TYPE_GOOD : SUMMON_CREATURE_TYPE_EVIL;
    pSpawned = new CGameSprite(pCreatureData,
        creatureSize,
        creatureType,
        static_cast<WORD>(-1),
        0,
        0,
        0,
        SUMMON_VISIBLE_TIME,
        spawnPos,
        static_cast<WORD>(-1));

    pSpawned->SetResRef(CResRef(resRef));
    creature.ReleaseData();

    pSpawned->m_baseStats.m_flags |= CREATURE_FLAG_SUMMONED;

    if (field_190 != 0) {
        CAIObjectType type(pSpawned->GetAIType());
        BOOL ally = FALSE;

        if (field_190 == 1) {
            ally = IcewindMisc::IsGoodByEA(m_pSprite);
        } else if (field_190 == 2) {
            ally = !IcewindMisc::IsGoodByEA(m_pSprite);
        }

        if (ally) {
            g_pBaldurChitin->GetObjectGame()->AddCharacterToAllies(pSpawned->m_id);
            type.SetEnemyAlly(CAIOBJECTTYPE_EA_ALLY);
        } else {
            type.SetEnemyAlly(CAIOBJECTTYPE_EA_ENEMY);
        }
        pSpawned->SetAIType(type, TRUE, TRUE);
    }

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pSpawned->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSpawned),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return NULL;
    }

    pSpawned->AddToArea(pArea,
        spawnPos,
        0,
        pSpawned->GetAnimation()->GetListType());
    // 0x55FE40: dir = caster->GetDirection(caster->GetPos()).  m_pos == target
    // takes GetDirection's early-out, so this yields the CASTER's current
    // facing -- the summoned creature spawns looking the same way as its
    // summoner, not toward it.
    pSpawned->SetFacing(m_pSprite->GetDirection(m_pSprite->GetPos()));
    pSpawned->m_posStart = pSpawned->m_pos;

    Icewind586B70::Instance()->AddSummoned(m_pSprite, pSpawned);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pSpawned->m_id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);


    return pSpawned;
}

// 0x55FA10
void IcewindCGameEffectSummon::AddSummonVisualHit(CGameSprite* pSpawned, int mode)
{
    CGameObject* pSource;
    BYTE rc;

    if (mode == 0 || pSpawned == NULL) {
        return;
    }

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pSource,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameEffect* pEffect = IcewindMisc::CreateEffectVisualSpellHit(pSource, mode, 0, 0);
        pEffect->m_flags = m_flags;
        pSpawned->AddEffect(pEffect, 1, TRUE, TRUE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x55F930
void IcewindCGameEffectSummon::PlayGroupVFX(int vfxId)
{
    if (vfxId == 0 || m_pSprite == NULL || m_pSprite->GetArea() == NULL) {
        return;
    }

    CProjectileSummonVFX* pProjectile = CreateSummonGroupProjectile(vfxId);
    if (pProjectile == NULL) {
        return;
    }

    CMessageFireProjectile* pMessage = new CMessageFireProjectile(pProjectile->m_projectileType,
        CGameObjectArray::INVALID_INDEX,
        m_target,
        0,
        m_pSprite->m_id,
        m_pSprite->m_id,
        0);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    pProjectile->Fire(m_pSprite->GetArea(),
        m_pSprite->m_id,
        m_pSprite->m_id,
        m_target,
        0,
        0);
}

// -----------------------------------------------------------------------------

// 0x49E3E0
CGameEffectVisualSpellHitIWD::CGameEffectVisualSpellHitIWD(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E420
CGameEffect* CGameEffectVisualSpellHitIWD::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectVisualSpellHitIWD* copy = new CGameEffectVisualSpellHitIWD(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// Opcode 233: local school impact plus owner-gated multiplayer replication.
// 0x5601E0
BOOL CGameEffectVisualSpellHitIWD::ApplyEffect(CGameSprite* pSprite)
{
    if (pSprite->m_pArea == NULL) {
        m_done = TRUE;
        return FALSE;
    }
    const BYTE* res = m_sourceRes.GetResRef();
    BOOL positive = res[2] == 'P' || res[2] == 'p';
    CProjectile* projectile = CProjectileSummonVFX::DecodeSpellHitProjectile(m_dwFlags, pSprite, positive);
    CMessage* message = new CMessageFireProjectile(static_cast<WORD>(m_dwFlags + 0x1000),
        pSprite->m_id, pSprite->GetPos(), 0, pSprite->m_id, pSprite->m_id, 0);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    projectile->Fire(pSprite->m_pArea, pSprite->m_id, pSprite->m_id, pSprite->GetPos(), 0, 0);
    m_done = TRUE;
    return FALSE;
}

// -----------------------------------------------------------------------------

// 0x49E4D0
IcewindCGameEffectRemoveEffectOfResource::IcewindCGameEffectRemoveEffectOfResource(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E510
CGameEffect* IcewindCGameEffectRemoveEffectOfResource::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRemoveEffectOfResource* copy = new IcewindCGameEffectRemoveEffectOfResource(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49E5C0
IcewindCGameEffectSalamanderAura::IcewindCGameEffectSalamanderAura(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49E600
CGameEffect* IcewindCGameEffectSalamanderAura::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSalamanderAura* copy = new IcewindCGameEffectSalamanderAura(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49E6B0
IcewindCGameEffectUmberHulkGaze::IcewindCGameEffectUmberHulkGaze(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49E6F0
CGameEffect* IcewindCGameEffectUmberHulkGaze::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectUmberHulkGaze* copy = new IcewindCGameEffectUmberHulkGaze(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49E7A0
IcewindCGameEffectImmunityToResource::IcewindCGameEffectImmunityToResource(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E7E0
CGameEffect* IcewindCGameEffectImmunityToResource::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectImmunityToResource* copy = new IcewindCGameEffectImmunityToResource(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49E890
IcewindCGameEffectAnimationRemove::IcewindCGameEffectAnimationRemove(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E8D0
CGameEffect* IcewindCGameEffectAnimationRemove::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectAnimationRemove* copy = new IcewindCGameEffectAnimationRemove(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49E980
IcewindCGameEffectImmunityToEffectAndResource::IcewindCGameEffectImmunityToEffectAndResource(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49E9C0
CGameEffect* IcewindCGameEffectImmunityToEffectAndResource::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectImmunityToEffectAndResource* copy = new IcewindCGameEffectImmunityToEffectAndResource(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49EA70
IcewindCGameEffectDominate::IcewindCGameEffectDominate(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49EAB0
CGameEffect* IcewindCGameEffectDominate::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectDominate* copy = new IcewindCGameEffectDominate(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49EB60
IcewindCGameEffectStaticCharge::IcewindCGameEffectStaticCharge(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49EBA0
CGameEffect* IcewindCGameEffectStaticCharge::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectStaticCharge* copy = new IcewindCGameEffectStaticCharge(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49EC50
IcewindCGameEffectCallLightning::IcewindCGameEffectCallLightning(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49EC90
CGameEffect* IcewindCGameEffectCallLightning::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectCallLightning* copy = new IcewindCGameEffectCallLightning(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x586220. Load a sub-spell for a strike, on-hit spell or effects-list payload.
// Select the last ability whose minimum level is reached, comparing against
// the full signed level argument. Stamp each effect with the caster's position
// and id before delivery. The selected ability's level is not forwarded as the
// BuildAbilityEffect level argument; that call computes its own caster level.
std::list<CGameEffect*> BuildSubSpellEffects(CGameSprite* pCaster,
    const CResRef& spellRes, INT level)
{
    std::list<CGameEffect*> effects;

    CSpell cSpell(spellRes);
    cSpell.Demand();

    SHORT abilityNum = 0;
    int count = cSpell.GetAbilityCount();
    if (count > 0) {
        int i = 0;
        while (i < count) {
            SPELL_ABILITY* pAbility = cSpell.GetAbility(i);
            if (level < pAbility->minCasterLevel) {
                break;
            }
            cSpell.GetAbility(i);
            ++abilityNum;
            i = abilityNum;
            count = cSpell.GetAbilityCount();
        }
    }

    SHORT abilityIdx = static_cast<SHORT>(abilityNum - 1);
    SPELL_ABILITY* pAbility = cSpell.GetAbility(abilityIdx);
    if (pAbility != NULL && pAbility->effectCount != 0) {
        for (LONG e = 0; e < pAbility->effectCount; ++e) {
            CGameEffect* pEffect = cSpell.BuildAbilityEffect(abilityIdx, e, pCaster, 0, 0, 0);
            pEffect->m_source = pCaster->GetPos();
            pEffect->m_sourceID = pCaster->m_id;
            effects.push_back(pEffect);
        }
    }

    cSpell.Release();
    return effects;
}

// 0x564F80.  The Static Charge engine (opcode 449, res EffCL): one call fires one
// sky-strike, and the first call (m_effectAmount = the charge count) spreads the
// remaining strikes over the duration as staggered delayed self-copies.  Each
// bolt is a CProjectileCallLightningStrike armed to hit the caster's enemies,
// carrying the EffCL sub-spell's effects, fired at the caster's position (where
// it gathers and strikes nearby victims within its area).
BOOL IcewindCGameEffectCallLightning::ApplyEffect(CGameSprite* pSprite)
{
    // 1. Spread the remaining strikes as staggered delayed self-copies (one
    //    strike per copy; copies carry count 0 so they do not re-spread).
    if (m_effectAmount > 1) {
        LONG delay = 0x46;
        for (LONG i = 1; i < m_effectAmount; ++i) {
            CGameEffect* pCopy = Copy();
            pCopy->m_duration = delay;
            pCopy->m_effectAmount = 0;
            pCopy->m_durationType = 4;   // delayed timing (DAT_008491b0)
            pCopy->m_casterLevel = m_casterLevel;
            pSprite->AddEffect(pCopy, 1, TRUE, TRUE);
            delay += 0x46;
        }
    }

    // 2. Fire one strike now.  Lock the source for the operation (the effect is
    //    self-applied, so the source is the caster itself).
    CGameObject* pSource;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        m_done = TRUE;
        return TRUE;
    }
    (void)pSource;

    CProjectileCallLightningStrike* pBolt = new CProjectileCallLightningStrike();
    pBolt->SetStrikeTargetFilter(pSprite);

    std::list<CGameEffect*> effects =
        BuildSubSpellEffects(pSprite, m_res, static_cast<BYTE>(m_casterLevel));
    for (std::list<CGameEffect*>::iterator it = effects.begin(); it != effects.end(); ++it) {
        (*it)->m_sourceRes = m_sourceRes;
        pBolt->AddEffect(*it);
    }

    CPoint pos = pSprite->GetPos();
    pBolt->Fire(pSprite->m_pArea, pSprite->m_id, pSprite->m_id, pos, 0, 0);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49ED40
IcewindCGameEffectCloakOfFear::IcewindCGameEffectCloakOfFear(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49ED80
CGameEffect* IcewindCGameEffectCloakOfFear::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectCloakOfFear* copy = new IcewindCGameEffectCloakOfFear(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49EE30
IcewindCGameEffectForceMovementRate::IcewindCGameEffectForceMovementRate(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49EE70
CGameEffect* IcewindCGameEffectForceMovementRate::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectForceMovementRate* copy = new IcewindCGameEffectForceMovementRate(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5653D0
BOOL IcewindCGameEffectForceMovementRate::ApplyEffect(CGameSprite* pSprite)
{
    // NOTE: Uninline.
    if (pSprite->GetAnimation()->GetMoveScaleDefault() != 0) {
        switch (m_dwFlags) {
        case 0:
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() + static_cast<BYTE>(m_effectAmount));
            break;
        case 1:
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetMoveScale(static_cast<BYTE>(m_effectAmount));
            break;
        case 2:
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() * static_cast<BYTE>(m_effectAmount) / 100);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameEffects.cpp
            // __LINE__: 4099
            UTIL_ASSERT(FALSE);
        }
    }

    if (m_secondaryType) {
        pSprite->DropPath();
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49EF20
IcewindCGameEffectRemoveConfusion::IcewindCGameEffectRemoveConfusion(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49EF60
CGameEffect* IcewindCGameEffectRemoveConfusion::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRemoveConfusion* copy = new IcewindCGameEffectRemoveConfusion(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565590
BOOL IcewindCGameEffectRemoveConfusion::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_CONFUSED;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_CONFUSED;

    // NOTE: Uninline.
    RemoveAllOfType(pSprite, CGAMEEFFECT_CONFUSION, -1);

    // NOTE: Uninline.
    RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON, 3);

    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F010
IcewindCGameEffectEyeOfMind::IcewindCGameEffectEyeOfMind(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F050
CGameEffect* IcewindCGameEffectEyeOfMind::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfMind* copy = new IcewindCGameEffectEyeOfMind(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565640
BOOL IcewindCGameEffectEyeOfMind::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_MIND)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_THE_MIND] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F100
IcewindCGameEffectEyeOfSword::IcewindCGameEffectEyeOfSword(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F140
CGameEffect* IcewindCGameEffectEyeOfSword::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfSword* copy = new IcewindCGameEffectEyeOfSword(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5656D0
BOOL IcewindCGameEffectEyeOfSword::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_SWORD)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_THE_SWORD] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F1F0
IcewindCGameEffectEyeOfMage::IcewindCGameEffectEyeOfMage(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F230
CGameEffect* IcewindCGameEffectEyeOfMage::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfMage* copy = new IcewindCGameEffectEyeOfMage(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565760
BOOL IcewindCGameEffectEyeOfMage::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_MAGE)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_THE_MAGE] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F2E0
IcewindCGameEffectEyeOfVenom::IcewindCGameEffectEyeOfVenom(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F320
CGameEffect* IcewindCGameEffectEyeOfVenom::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfVenom* copy = new IcewindCGameEffectEyeOfVenom(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5657F0
BOOL IcewindCGameEffectEyeOfVenom::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_VENOM)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_VENOM] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F3D0
IcewindCGameEffectEyeOfSpirit::IcewindCGameEffectEyeOfSpirit(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F410
CGameEffect* IcewindCGameEffectEyeOfSpirit::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfSpirit* copy = new IcewindCGameEffectEyeOfSpirit(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565880
BOOL IcewindCGameEffectEyeOfSpirit::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_SPIRIT)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_THE_SPIRIT] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F4C0
IcewindCGameEffectEyeOfFortitude::IcewindCGameEffectEyeOfFortitude(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F500
CGameEffect* IcewindCGameEffectEyeOfFortitude::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfFortitude* copy = new IcewindCGameEffectEyeOfFortitude(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565910
BOOL IcewindCGameEffectEyeOfFortitude::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_FORTITUDE)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_FORTITUDE] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F5B0
IcewindCGameEffectEyeOfStone::IcewindCGameEffectEyeOfStone(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F5F0
CGameEffect* IcewindCGameEffectEyeOfStone::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEyeOfStone* copy = new IcewindCGameEffectEyeOfStone(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5659A0
BOOL IcewindCGameEffectEyeOfStone::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        pSprite->AddSpecialAbility(CResRef(CString(RESREF_EYE_OF_STONE)), FALSE);
    }
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EYE_OF_STONE] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F6A0
IcewindCGameEffectRemoveSevenEyes::IcewindCGameEffectRemoveSevenEyes(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F6E0
CGameEffect* IcewindCGameEffectRemoveSevenEyes::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRemoveSevenEyes* copy = new IcewindCGameEffectRemoveSevenEyes(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565A30
BOOL IcewindCGameEffectRemoveSevenEyes::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_MIND)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_SWORD)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_MAGE)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_VENOM)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_SPIRIT)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_FORTITUDE)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_STONE)));

    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F790
IcewindCGameEffectRemoveAllOfType::IcewindCGameEffectRemoveAllOfType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F7D0
CGameEffect* IcewindCGameEffectRemoveAllOfType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRemoveAllOfType* copy = new IcewindCGameEffectRemoveAllOfType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x565C20
BOOL IcewindCGameEffectRemoveAllOfType::ApplyEffect(CGameSprite* pSprite)
{
    // NOTE: Uninline.
    RemoveAllOfType(pSprite, static_cast<WORD>(m_dwFlags), -1);

    if (m_dwFlags == CGAMEEFFECT_POISON) {
        // NOTE: Uninline.
        RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON, 6);
    }

    m_forceRepass = TRUE;
    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49F880
IcewindCGameEffectShroudOfFlame::IcewindCGameEffectShroudOfFlame(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F8C0
CGameEffect* IcewindCGameEffectShroudOfFlame::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectShroudOfFlame* copy = new IcewindCGameEffectShroudOfFlame(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49F970
IcewindCGameEffectAnimalRage::IcewindCGameEffectAnimalRage(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49F9B0
CGameEffect* IcewindCGameEffectAnimalRage::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectAnimalRage* copy = new IcewindCGameEffectAnimalRage(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49FA60
IcewindCGameEffectTurnUndead::IcewindCGameEffectTurnUndead(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49FAA0
CGameEffect* IcewindCGameEffectTurnUndead::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectTurnUndead* copy = new IcewindCGameEffectTurnUndead(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x566070
void IcewindCGameEffectTurnUndead::OnRemove(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_TURNED] = false;
}

// -----------------------------------------------------------------------------

// 0x49FB50
IcewindCGameEffectVitriolicSphere::IcewindCGameEffectVitriolicSphere(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49FB90
CGameEffect* IcewindCGameEffectVitriolicSphere::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectVitriolicSphere* copy = new IcewindCGameEffectVitriolicSphere(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49FC50
IcewindCGameEffectSuppressHPDisplay::IcewindCGameEffectSuppressHPDisplay(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49FC90
CGameEffect* IcewindCGameEffectSuppressHPDisplay::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSuppressHPDisplay* copy = new IcewindCGameEffectSuppressHPDisplay(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x566500
BOOL IcewindCGameEffectSuppressHPDisplay::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_SUPPRESS_HP_INFO] = true;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49FD40
IcewindCGameEffectFloatText::IcewindCGameEffectFloatText(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49FD80
CGameEffect* IcewindCGameEffectFloatText::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFloatText* copy = new IcewindCGameEffectFloatText(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49FE30
IcewindCGameEffectMaceOfDisruption::IcewindCGameEffectMaceOfDisruption(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49FE70
CGameEffect* IcewindCGameEffectMaceOfDisruption::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectMaceOfDisruption* copy = new IcewindCGameEffectMaceOfDisruption(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49FF20
IcewindCGameEffectRangerTracking::IcewindCGameEffectRangerTracking(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49FF60
CGameEffect* IcewindCGameEffectRangerTracking::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRangerTracking* copy = new IcewindCGameEffectRangerTracking(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A0010
IcewindCGameEffectImmunityToBackstab::IcewindCGameEffectImmunityToBackstab(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0050
CGameEffect* IcewindCGameEffectImmunityToBackstab::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectImmunityToBackstab* copy = new IcewindCGameEffectImmunityToBackstab(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5668F0
BOOL IcewindCGameEffectImmunityToBackstab::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_SNEAK_ATTACK_IMMUNITY] = true;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A0100
IcewindCGameEffectSetSpellState::IcewindCGameEffectSetSpellState(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0140
CGameEffect* IcewindCGameEffectSetSpellState::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSetSpellState* copy = new IcewindCGameEffectSetSpellState(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x566910
BOOL IcewindCGameEffectSetSpellState::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates.set(m_dwFlags, true);

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A01F0
IcewindCGameEffectRodOfSmiting::IcewindCGameEffectRodOfSmiting(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0230
CGameEffect* IcewindCGameEffectRodOfSmiting::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRodOfSmiting* copy = new IcewindCGameEffectRodOfSmiting(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A02E0
IcewindCGameEffectMagicalRest::IcewindCGameEffectMagicalRest(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0320
CGameEffect* IcewindCGameEffectMagicalRest::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectMagicalRest* copy = new IcewindCGameEffectMagicalRest(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A03D0
IcewindCGameEffectBeholderDispelMagic::IcewindCGameEffectBeholderDispelMagic(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x4A0410
CGameEffect* IcewindCGameEffectBeholderDispelMagic::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectBeholderDispelMagic* copy = new IcewindCGameEffectBeholderDispelMagic(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A04C0
IcewindCGameEffectHarpyWail::IcewindCGameEffectHarpyWail(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x4A0500
CGameEffect* IcewindCGameEffectHarpyWail::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectHarpyWail* copy = new IcewindCGameEffectHarpyWail(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A05B0
IcewindCGameEffectJackalwereGaze::IcewindCGameEffectJackalwereGaze(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x4A05F0
CGameEffect* IcewindCGameEffectJackalwereGaze::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectJackalwereGaze* copy = new IcewindCGameEffectJackalwereGaze(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A06A0
IcewindCGameEffectSetGlobal::IcewindCGameEffectSetGlobal(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A06E0
CGameEffect* IcewindCGameEffectSetGlobal::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSetGlobal* copy = new IcewindCGameEffectSetGlobal(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A0790
IcewindCGameEffectHopelessness::IcewindCGameEffectHopelessness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A07D0
CGameEffect* IcewindCGameEffectHopelessness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectHopelessness* copy = new IcewindCGameEffectHopelessness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x567450
BOOL IcewindCGameEffectHopelessness::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 1280);
        m_secondaryType = 0;
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_HOPELESSNESS]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_HOPELESSNESS] = true;
        pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;
        pSprite->field_9D15 = 1;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 47);

        m_done = FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A0880
IcewindCGameEffectProtectionFromEvil::IcewindCGameEffectProtectionFromEvil(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A08C0
CGameEffect* IcewindCGameEffectProtectionFromEvil::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectProtectionFromEvil* copy = new IcewindCGameEffectProtectionFromEvil(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x567500
BOOL IcewindCGameEffectProtectionFromEvil::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 14769);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_PROTECTION_FROM_EVIL]) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_PROTECTION_FROM_EVIL, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 9);

        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(ICEWIND_CGAMEEFFECT_CHARM);
        SetMinNaturalAC(pSprite, 2);
        pSprite->GetDerivedStats()->m_nSaveVSFortitude += 2;
        pSprite->GetDerivedStats()->m_nSaveVSReflex += 2;
        pSprite->GetDerivedStats()->m_nSaveVSWill += 2;

        m_done = FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A0970
IcewindCGameEffectApplyEffectsList::IcewindCGameEffectApplyEffectsList(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A09B0
CGameEffect* IcewindCGameEffectApplyEffectsList::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectApplyEffectsList* copy = new IcewindCGameEffectApplyEffectsList(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5675E0.  Opcode 402 "Add Effects List": gate the target against a spell-protection
// Type (param2, stored in m_dwFlags -- the splprot.2da row), and on a match load the
// sub-spell named by m_res and apply its level-appropriate effects.  Self-targeting
// sub-effects land on the caster, the rest on the target.  The Type selector, the m_res
// resref and the caster level were Frida-confirmed on the original (Sunscorch SPPR113:
// Types 73/75/31/2 -> EffS3/EffS3/EffS1/EffS2, caster level read from m_casterLevel).
BOOL IcewindCGameEffectApplyEffectsList::ApplyEffect(CGameSprite* pSprite)
{
    m_done = TRUE;

    switch (m_dwFlags) {
    case 0:     // no restriction
        break;
    case 1:     if (!IcewindMisc::IsUndead(pSprite)) return TRUE; break;
    case 2:     if (IcewindMisc::IsUndead(pSprite)) return TRUE; break;
    case 3:     if (!IcewindMisc::IsImmuneToFire(pSprite)) return TRUE; break;
    case 4:     if (IcewindMisc::IsImmuneToFire(pSprite)) return TRUE; break;
    case 5:     if (!IcewindMisc::IsHumanoid(pSprite)) return TRUE; break;
    case 6:     if (IcewindMisc::IsHumanoid(pSprite)) return TRUE; break;
    case 9:     if (!IcewindMisc::IsElemental(pSprite)) return TRUE; break;
    case 0xa:   if (IcewindMisc::IsElemental(pSprite)) return TRUE; break;
    case 0xd:   if (!IcewindMisc::IsLarge(pSprite)) return TRUE; break;
    case 0xe:   if (IcewindMisc::IsLarge(pSprite)) return TRUE; break;
    case 0xf:   if (!IcewindMisc::IsElf(pSprite)) return TRUE; break;
    case 0x10:  if (IcewindMisc::IsElf(pSprite)) return TRUE; break;
    case 0x11:  if (!IcewindMisc::IsUmberhulk(pSprite)) return TRUE; break;
    case 0x12:  if (IcewindMisc::IsUmberhulk(pSprite)) return TRUE; break;
    case 0x13:  if (!IcewindMisc::IsHalfElf(pSprite)) return TRUE; break;
    case 0x14:  if (IcewindMisc::IsHalfElf(pSprite)) return TRUE; break;
    case 0x15:  // humanoid or animal
        if (IcewindMisc::IsHumanoid(pSprite)) break;
        // fall through
    case 7:
        if (!IcewindMisc::IsAnimal(pSprite)) return TRUE;
        break;
    case 0x16:  // not (humanoid or animal)
        if (IcewindMisc::IsHumanoid(pSprite)) return TRUE;
        // fall through
    case 8:
        if (IcewindMisc::IsAnimal(pSprite)) return TRUE;
        break;
    case 0x17:  if (!IcewindMisc::IsBlind(pSprite)) return TRUE; break;
    case 0x18:  if (IcewindMisc::IsBlind(pSprite)) return TRUE; break;
    case 0x1b:  if (!IcewindMisc::IsGolem(pSprite)) return TRUE; break;
    case 0x1c:  if (IcewindMisc::IsGolem(pSprite)) return TRUE; break;
    case 0x1d:  if (!IcewindMisc::IsMinotaur(pSprite)) return TRUE; break;
    case 0x1e:  if (IcewindMisc::IsMinotaur(pSprite)) return TRUE; break;
    case 0x1f:  // undead or fungus
        if (IcewindMisc::IsUndead(pSprite)) break;
        // fall through
    case 0xb:
        if (!IcewindMisc::IsFungus(pSprite)) return TRUE;
        break;
    case 0x20:  // not (undead or fungus)
        if (IcewindMisc::IsUndead(pSprite)) return TRUE;
        // fall through
    case 0xc:
        if (IcewindMisc::IsFungus(pSprite)) return TRUE;
        break;
    case 0x21:  if (!IcewindMisc::IsGood(pSprite)) return TRUE; break;
    case 0x22:  if (IcewindMisc::IsGood(pSprite)) return TRUE; break;
    case 0x23:  if (!IcewindMisc::IsNeutral(pSprite)) return TRUE; break;
    case 0x24:  if (IcewindMisc::IsNeutral(pSprite)) return TRUE; break;
    case 0x25:  if (!IcewindMisc::IsEvil(pSprite)) return TRUE; break;
    case 0x26:  if (IcewindMisc::IsEvil(pSprite)) return TRUE; break;
    case 0x27:  if (!IcewindMisc::IsPaladin(pSprite)) return TRUE; break;
    case 0x28:  if (IcewindMisc::IsPaladin(pSprite)) return TRUE; break;
    case 0x2b:  // must be the source object
        if (pSprite->m_id != m_sourceID) return TRUE;
        break;
    case 0x2c:  // must not be the source object
        if (pSprite->m_id == m_sourceID) return TRUE;
        break;
    case 0x2d:  if (!IcewindMisc::IsAcquatic(pSprite)) return TRUE; break;
    case 0x2e:  if (IcewindMisc::IsAcquatic(pSprite)) return TRUE; break;
    case 0x2f:  if (!IcewindMisc::IsLiving(pSprite)) return TRUE; break;
    case 0x30:  if (IcewindMisc::IsLiving(pSprite)) return TRUE; break;
    case 0x35:  // fire-immune or cold-immune
        if (IcewindMisc::IsImmuneToFire(pSprite)) break;
        // fall through
    case 0x19:
        if (!IcewindMisc::IsImmuneToCold(pSprite)) return TRUE;
        break;
    case 0x36:  // not (fire-immune or cold-immune)
        if (IcewindMisc::IsImmuneToFire(pSprite)) return TRUE;
        // fall through
    case 0x1a:
        if (IcewindMisc::IsImmuneToCold(pSprite)) return TRUE;
        break;
    case 0x37:  if (!IcewindMisc::sud_585070(pSprite)) return TRUE; break;
    case 0x38:  if (IcewindMisc::sud_585070(pSprite)) return TRUE; break;
    case 0x39:  if (!IcewindMisc::IsMale(pSprite)) return TRUE; break;
    case 0x3a:  if (IcewindMisc::IsMale(pSprite)) return TRUE; break;
    case 0x3b:  if (!IcewindMisc::IsLawful(pSprite)) return TRUE; break;
    case 0x3c:  if (IcewindMisc::IsLawful(pSprite)) return TRUE; break;
    case 0x3d:  if (!IcewindMisc::IsChaotic(pSprite)) return TRUE; break;
    case 0x3e:  if (IcewindMisc::IsChaotic(pSprite)) return TRUE; break;
    case 0x3f:  if (pSprite->GetAIType().m_nRace != 0xa0) return TRUE; break;
    case 0x40:  if (pSprite->GetAIType().m_nRace == 0xa0) return TRUE; break;
    case 0x41:  // target is deafened
        if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_DEAFENED)) return TRUE;
        break;
    case 0x42:
        if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_DEAFENED)) return TRUE;
        break;
    case 0x47:  // caster effectAmount vs the target's current hit points
        if (static_cast<int>(pSprite->GetBaseStats()->m_hitPoints) <= m_effectAmount) return TRUE;
        break;
    case 0x48:
        if (m_effectAmount <= static_cast<int>(pSprite->GetBaseStats()->m_hitPoints)) return TRUE;
        break;
    case 0x49:  // race 2, subrace 1
        if (pSprite->GetAIType().m_nRace != 2 || pSprite->GetAIType().m_nSubRace != 1) return TRUE;
        break;
    case 0x4a:
        if (pSprite->GetAIType().m_nRace == 2 && pSprite->GetAIType().m_nSubRace == 1) return TRUE;
        break;
    case 0x4b:  // race 4, subrace 2
        if (pSprite->GetAIType().m_nRace != 4 || pSprite->GetAIType().m_nSubRace != 2) return TRUE;
        break;
    case 0x4c:
        if (pSprite->GetAIType().m_nRace == 4 && pSprite->GetAIType().m_nSubRace == 2) return TRUE;
        break;
    case 0x4d:  // target is a summoned creature
        if ((pSprite->GetBaseStats()->m_generalState & STATE_SUMMONED_CREATURE) == 0) return TRUE;
        break;
    case 0x4e:
        if ((pSprite->GetBaseStats()->m_generalState & STATE_SUMMONED_CREATURE) != 0) return TRUE;
        break;
    case 0x4f:  if (pSprite->GetAIType().m_nRace != 0x9b) return TRUE; break;
    case 0x50:  if (pSprite->GetAIType().m_nRace == 0x9b) return TRUE; break;
    case 0x51:  if (pSprite->GetAIType().m_nRace != 0x99) return TRUE; break;
    case 0x52:  if (pSprite->GetAIType().m_nRace == 0x99) return TRUE; break;
    case 0x53:  if (pSprite->GetAIType().m_nRace != 0xa5) return TRUE; break;
    case 0x54:  if (pSprite->GetAIType().m_nRace == 0xa5) return TRUE; break;
    case 0x55:  if (pSprite->GetAIType().m_nRace != 0xbe) return TRUE; break;
    case 0x56:  if (pSprite->GetAIType().m_nRace == 0xbe) return TRUE; break;
    case 0x57:  if (pSprite->GetAIType().m_nRace != 1) return TRUE; break;
    case 0x58:  if (pSprite->GetAIType().m_nRace == 1) return TRUE; break;
    case 0x59:  if (pSprite->GetAIType().m_nRace != 0xa8) return TRUE; break;
    case 0x5a:  if (pSprite->GetAIType().m_nRace == 0xa8) return TRUE; break;
    case 0x5b:  if (!IcewindMisc::IsOutsider(pSprite)) return TRUE; break;
    case 0x5c:  if (IcewindMisc::IsOutsider(pSprite)) return TRUE; break;
    default: {
        // Relational Types (0x29/0x2a alignment-vs-source, 0x31-0x46 ally/enemy/alignment):
        // these need the caster as the second party, so lock the source first.
        CGameObject* pRel;
        BYTE rcRel = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH, &pRel, INFINITE);
        if (rcRel != CGameObjectArray::SUCCESS) {
            return FALSE;
        }
        CGameSprite* pSource = static_cast<CGameSprite*>(pRel);
        BOOL reject = FALSE;
        switch (m_dwFlags) {
        case 0x29:  if (!IcewindMisc::IsGoodEvilSame(pSprite, pSource)) reject = TRUE; break;
        case 0x2a:  if (IcewindMisc::IsGoodEvilSame(pSprite, pSource)) reject = TRUE; break;
        case 0x31:  if (!IcewindMisc::AreAllies(pSprite, pSource)) reject = TRUE; break;
        case 0x32:  if (IcewindMisc::AreAllies(pSprite, pSource)) reject = TRUE; break;
        case 0x33:  if (!IcewindMisc::AreEnemies(pSprite, pSource)) reject = TRUE; break;
        case 0x34:  if (IcewindMisc::AreEnemies(pSprite, pSource)) reject = TRUE; break;
        case 0x43:  if (!IcewindMisc::IsAlignmentSame(pSprite, pSource)) reject = TRUE; break;
        case 0x44:  if (IcewindMisc::IsAlignmentSame(pSprite, pSource)) reject = TRUE; break;
        case 0x45:  if (!IcewindMisc::IsAlignmentSame(pSprite, pSource) || !IcewindMisc::AreAllies(pSprite, pSource)) reject = TRUE; break;
        case 0x46:  if (IcewindMisc::IsAlignmentSame(pSprite, pSource) && IcewindMisc::AreAllies(pSprite, pSource)) reject = TRUE; break;
        default:
            CUtil::UtilAssert(5802, "C:\\Projects\\Icewind2\\src\\Baldur\\IcewindCGameEffects.cpp",
                "FALSE", "Out-of-range value detected in IcewindCGameEffectApplyEffectsList (-rjf)");
            break;
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
        if (reject) return TRUE;
        break;
    }
    }

    // The target passed the filter -- apply the sub-spell's effects.  The source is the
    // caster (this is a self-applied effect rider), so lock it and build its effects.
    CGameObject* pSource;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
        CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        m_done = TRUE;
        return TRUE;
    }

    CGameSprite* pCaster = static_cast<CGameSprite*>(pSource);
    std::list<CGameEffect*> effects =
        BuildSubSpellEffects(pCaster, m_res, static_cast<BYTE>(m_casterLevel));
    for (std::list<CGameEffect*>::iterator it = effects.begin(); it != effects.end(); ++it) {
        CGameEffect* pEffect = *it;
        pEffect->m_sourceRes = m_sourceRes;
        pEffect->m_target = m_target;
        pEffect->m_casterLevel = m_casterLevel;
        if (pEffect->m_targetType == 1) {
            // self-targeting sub-effect -> apply to the caster (only if it is a sprite)
            if (pCaster->GetObjectType() == CGameObject::TYPE_SPRITE) {
                pCaster->AddEffect(pEffect, 1, FALSE, TRUE);
            }
        } else {
            pSprite->AddEffect(pEffect, 1, FALSE, TRUE);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A0A60
IcewindCGameEffectArmorOfFaith::IcewindCGameEffectArmorOfFaith(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0AA0
CGameEffect* IcewindCGameEffectArmorOfFaith::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectArmorOfFaith* copy = new IcewindCGameEffectArmorOfFaith(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x568930
BOOL IcewindCGameEffectArmorOfFaith::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 509);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ARMOR_OF_FAITH]) {
        // NOTE: Using `set` (with bounds check).
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_ARMOR_OF_FAITH, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 84);

        pSprite->GetDerivedStats()->m_nResistCrushing++;
        pSprite->GetDerivedStats()->m_nResistPiercing++;
        pSprite->GetDerivedStats()->m_nResistSlashing++;
        pSprite->GetDerivedStats()->m_nResistFire++;
        pSprite->GetDerivedStats()->m_nResistCold++;
        pSprite->GetDerivedStats()->m_nResistAcid++;
        pSprite->GetDerivedStats()->m_nResistElectricity++;
        pSprite->GetDerivedStats()->m_nResistMagic++;

        m_done = FALSE;
    }
    return TRUE;
}

// 0x55ECF0
void IcewindCGameEffectArmorOfFaith::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A0B50
IcewindCGameEffectNausea::IcewindCGameEffectNausea(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0B90
CGameEffect* IcewindCGameEffectNausea::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectNausea* copy = new IcewindCGameEffectNausea(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x568A00
BOOL IcewindCGameEffectNausea::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS | STATE_SLEEPING;

        // NOTE: Using `set` (with bounds check).
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DOESNT_AWAKEN_ON_DAMAGE, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 43);

        if (m_secondaryType) {
            DisplayStringRef(pSprite, 4390); // "Nauseous"

            ITEM_EFFECT* effect = new ITEM_EFFECT;
            ClearItemEffect(effect, CGAMEEFFECT_SETSEQUENCE);
            effect->dwFlags = CGameSprite::SEQ_AWAKE;
            effect->durationType = 4;
            effect->duration = (m_duration - g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime) / 15;
            CGameEffect* pEffect = CGameEffect::DecodeEffect(effect,
                CPoint(-1, -1),
                -1,
                CPoint(-1, -1));
            delete effect;
            pSprite->AddEffect(pEffect, CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
        }

        if (pSprite->m_nSequence != CGameSprite::SEQ_DIE
            && pSprite->m_nSequence != CGameSprite::SEQ_SLEEP
            && pSprite->m_nSequence != CGameSprite::SEQ_TWITCH) {
            pSprite->SetSequence(CGameSprite::SEQ_SLEEP);
        }

        pSprite->field_9D15 = 1;
        break;
    case 1:
        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 43);

        if (m_secondaryType) {
            DisplayStringRef(pSprite, 4390); // "Nauseous"
        }

        pSprite->GetDerivedStats()->m_nDamageBonus -= 2;
        pSprite->GetDerivedStats()->m_nTHAC0 -= 2;
        pSprite->GetDerivedStats()->m_nSaveVSFortitude -= 2;
        pSprite->GetDerivedStats()->m_nSaveVSReflex -= 2;
        pSprite->GetDerivedStats()->m_nSaveVSWill -= 2;
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A0C40
IcewindCGameEffectEnfeeblement::IcewindCGameEffectEnfeeblement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0C80
CGameEffect* IcewindCGameEffectEnfeeblement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEnfeeblement* copy = new IcewindCGameEffectEnfeeblement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x568C10
BOOL IcewindCGameEffectEnfeeblement::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 7924); // "Enfeebled"

        SHORT v1 = rand() % 6 + 1;
        SHORT v2 = static_cast<BYTE>(m_casterLevel) - 1 > 10
            ? 5
            : static_cast<BYTE>(m_casterLevel) / 2;
        m_effectAmount = v1 + v2;
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ENFEEBLED]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ENFEEBLED] = true;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 90);

        pSprite->GetDerivedStats()->m_nSTR -= 15;
        if (pSprite->GetDerivedStats()->m_nSTR < 1) {
            pSprite->GetDerivedStats()->m_nSTR = 1;
        }

        m_done = FALSE;
    }
    return TRUE;
}

// 0x4BAF90
void IcewindCGameEffectEnfeeblement::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E04"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A0D30
IcewindCGameEffectFireshield::IcewindCGameEffectFireshield(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0D70
CGameEffect* IcewindCGameEffectFireshield::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFireshield* copy = new IcewindCGameEffectFireshield(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A0E20
IcewindCGameEffectDeathWard::IcewindCGameEffectDeathWard(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0E60
CGameEffect* IcewindCGameEffectDeathWard::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectDeathWard* copy = new IcewindCGameEffectDeathWard(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BAF90
void IcewindCGameEffectDeathWard::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E04"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A0F10
IcewindCGameEffectHolyPower::IcewindCGameEffectHolyPower(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A0F50
CGameEffect* IcewindCGameEffectHolyPower::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectHolyPower* copy = new IcewindCGameEffectHolyPower(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x569210
BOOL IcewindCGameEffectHolyPower::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 1315); // "Holy Power"
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_HOLY_POWER]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_HOLY_POWER] = true;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 86);

        AddColorEffect(pSprite, 128, 128, 128, 20);

        pSprite->m_bonusStats.m_nDamageBonus += 4;
        m_done = FALSE;
    }
    return TRUE;
}

// 0x4BAF90
void IcewindCGameEffectHolyPower::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E04"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A1000
IcewindCGameEffectVisualEffect::IcewindCGameEffectVisualEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1040
CGameEffect* IcewindCGameEffectVisualEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectVisualEffect* copy = new IcewindCGameEffectVisualEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x569320
BOOL IcewindCGameEffectVisualEffect::ApplyEffect(CGameSprite* pSprite)
{
    // NOTE: Using `set` (with bounds check).
    pSprite->GetDerivedStats()->m_visualEffects.set(m_dwFlags, true);

    if (m_dwFlags == IWD_VFX_SHIELD_OF_LATHANDER) {
        pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_SHIELD_OF_LATHANDER_2] = true;
    }

    if (m_dwFlags == IWD_VFX_GREATER_SHIELD_OF_LATHANDER) {
        pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_GREATER_SHIELD_OF_LATHANDER_2] = true;
    }

    if (m_dwFlags == IWD_VFX_SEVEN_EYES) {
        pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_SEVEN_EYES_2] = true;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A10F0
IcewindCGameEffectOtilukesResilientSphere::IcewindCGameEffectOtilukesResilientSphere(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1130
CGameEffect* IcewindCGameEffectOtilukesResilientSphere::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectOtilukesResilientSphere* copy = new IcewindCGameEffectOtilukesResilientSphere(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x569320
BOOL IcewindCGameEffectOtilukesResilientSphere::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 4732); // "Encased in Otiluke's Resilient Sphere"
    }

    pSprite->field_9D15 = 1;

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 100);

    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_HELD] = true;
    pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;
    pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_OTILUKES_RESILIENT_SPHERE] = true;
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_OTILUKES_RESILIENT_SPHERE] = true;

    return TRUE;
}

// 0x4BAF90
void IcewindCGameEffectOtilukesResilientSphere::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E04"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A11E0
IcewindCGameEffectBarkskin::IcewindCGameEffectBarkskin(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1220
CGameEffect* IcewindCGameEffectBarkskin::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectBarkskin* copy = new IcewindCGameEffectBarkskin(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5693B0
BOOL IcewindCGameEffectBarkskin::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 14785); // "Barkskin"
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_BARKSKIN]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_BARKSKIN] = true;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 20);

        if (m_casterLevel < 6) {
            pSprite->GetDerivedStats()->m_nACDodgeBonus += 3;
        } else if (m_casterLevel < 12) {
            pSprite->GetDerivedStats()->m_nACDodgeBonus += 4;
        } else {
            pSprite->GetDerivedStats()->m_nACDodgeBonus += 5;
        }

        pSprite->SetColorRange(2);
    }
    return TRUE;
}

// 0x55ECF0
void IcewindCGameEffectBarkskin::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A12D0
IcewindCGameEffectBleedingWounds::IcewindCGameEffectBleedingWounds(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1310
CGameEffect* IcewindCGameEffectBleedingWounds::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectBleedingWounds* copy = new IcewindCGameEffectBleedingWounds(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A13C0
IcewindCGameEffectAreaOfEffect::IcewindCGameEffectAreaOfEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1400
CGameEffect* IcewindCGameEffectAreaOfEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectAreaOfEffect* copy = new IcewindCGameEffectAreaOfEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A14B0
IcewindCGameEffectFreeAction::IcewindCGameEffectFreeAction(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A14F0
CGameEffect* IcewindCGameEffectFreeAction::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFreeAction* copy = new IcewindCGameEffectFreeAction(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x5698E0
BOOL IcewindCGameEffectFreeAction::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 14794);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FREE_ACTION)) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FREE_ACTION, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 19);

        AddColorEffect(pSprite, 128, 96, 96, 30);

        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(CGAMEEFFECT_ENTANGLE);
        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(CGAMEEFFECT_GREASEPOOL);
        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(CGAMEEFFECT_WEBHOLD);
        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(CGAMEEFFECT_HOLDCREATURE);
        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(CGAMEEFFECT_SLOW);
        pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(CGAMEEFFECT_MOVEMENTRATE_176);

        m_done = FALSE;
    }

    return TRUE;
}

// 0x55ECF0
void IcewindCGameEffectFreeAction::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A15A0
IcewindCGameEffectUnconsciousness::IcewindCGameEffectUnconsciousness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A15E0
CGameEffect* IcewindCGameEffectUnconsciousness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectUnconsciousness* copy = new IcewindCGameEffectUnconsciousness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x569BD0
BOOL IcewindCGameEffectUnconsciousness::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS | STATE_SLEEPING;

    if (m_dwFlags == 1) {
        // NOTE: Using `set` (with bounds check).
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DOESNT_AWAKEN_ON_DAMAGE, true);
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 88);

    if (m_secondaryType) {
        DisplayStringRef(pSprite, 20438); // "Knocked Unconscious"

        ITEM_EFFECT* effect = new ITEM_EFFECT;
        ClearItemEffect(effect, CGAMEEFFECT_SETSEQUENCE);
        effect->dwFlags = CGameSprite::SEQ_AWAKE;
        effect->durationType = 4;
        effect->duration = (m_duration - g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime) / 15;
        CGameEffect* pEffect = CGameEffect::DecodeEffect(effect,
            CPoint(-1, -1),
            -1,
            CPoint(-1, -1));
        delete effect;
        pSprite->AddEffect(pEffect, CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);
    }

    if (pSprite->m_nSequence != CGameSprite::SEQ_DIE
        && pSprite->m_nSequence != CGameSprite::SEQ_SLEEP
        && pSprite->m_nSequence != CGameSprite::SEQ_TWITCH) {
        pSprite->SetSequence(CGameSprite::SEQ_SLEEP);
    }

    pSprite->field_9D15 = 1;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1690
IcewindCGameEffectEntropyShield::IcewindCGameEffectEntropyShield(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A16D0
CGameEffect* IcewindCGameEffectEntropyShield::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEntropyShield* copy = new IcewindCGameEffectEntropyShield(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x569D30
BOOL IcewindCGameEffectEntropyShield::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 8104);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_ENTROPY_SHIELD)) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_ENTROPY_SHIELD, true);

        AddPortraitIcon(pSprite, 103);

        pSprite->GetDerivedStats()->m_visualEffects.set(IWD_VFX_ENTROPY_SHIELD, true);
        AddColorEffect(pSprite, 64, 192, 64, 30);
        SetMinNaturalAC(pSprite, 6);

        pSprite->m_bonusStats.m_nResistFire += 10;
        pSprite->m_bonusStats.m_nResistCold += 10;
        pSprite->m_bonusStats.m_nResistElectricity += 10;
        pSprite->m_bonusStats.m_nResistAcid += 10;
        pSprite->m_bonusStats.m_nSaveVSFortitude += 10;
        pSprite->m_bonusStats.m_nSaveVSReflex += 10;
        pSprite->m_bonusStats.m_nSaveVSWill += 10;

        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(1));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(3));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(4));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(5));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(6));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(8));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(9));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(10));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(11));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(13));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(14));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(15));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(16));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(18));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(19));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(20));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(23));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(26));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(28));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(29));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(30));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(31));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(33));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(34));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(35));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(36));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(55));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(57));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(58));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(59));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(79));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(101));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(102));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(105));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(107));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(187));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(191));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(192));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(193));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(217));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(226));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(227));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(228));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(229));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(230));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(231));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(232));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(233));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(250));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(267));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(268));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(270));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(284));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(297));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(302));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(312));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(313));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(315));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(345));
        pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(350));
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1780
IcewindCGameEffectStormShell::IcewindCGameEffectStormShell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A17C0
CGameEffect* IcewindCGameEffectStormShell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectStormShell* copy = new IcewindCGameEffectStormShell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56A090
BOOL IcewindCGameEffectStormShell::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 8830); // "Protected by Storm Shell"
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_STORM_SHELL]) {
        // NOTE: Using `set` (with bounds check).
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_STORM_SHELL, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 105);

        pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_STORM_SHELL] = true;
        pSprite->m_bonusStats.m_nResistFire += 15;
        pSprite->m_bonusStats.m_nResistCold += 15;
        pSprite->m_bonusStats.m_nResistElectricity += 15;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1870
IcewindCGameEffectProtectionFromElements::IcewindCGameEffectProtectionFromElements(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A18B0
CGameEffect* IcewindCGameEffectProtectionFromElements::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectProtectionFromElements* copy = new IcewindCGameEffectProtectionFromElements(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56A140
BOOL IcewindCGameEffectProtectionFromElements::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 8842); // "Protected by Protection from the Elements"
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_PROTECTION_FROM_THE_ELEMENTS]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_PROTECTION_FROM_THE_ELEMENTS] = true;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 93);

        AddColorEffect(pSprite, 192, 0, 0, 61);
        AddColorEffect(pSprite, 0, 192, 0, 43);
        AddColorEffect(pSprite, 0, 0, 192, 79);

        pSprite->m_bonusStats.m_nResistFire += 15;
        pSprite->m_bonusStats.m_nResistCold += 15;
        pSprite->m_bonusStats.m_nResistElectricity += 15;
        pSprite->m_bonusStats.m_nResistAcid += 15;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1960
IcewindCGameEffectAegis::IcewindCGameEffectAegis(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A19A0
CGameEffect* IcewindCGameEffectAegis::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectAegis* copy = new IcewindCGameEffectAegis(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B6E0
void IcewindCGameEffectAegis::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E01"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A1A50
IcewindCGameEffectExecutionersEyes::IcewindCGameEffectExecutionersEyes(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1A90
CGameEffect* IcewindCGameEffectExecutionersEyes::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectExecutionersEyes* copy = new IcewindCGameEffectExecutionersEyes(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56A500
BOOL IcewindCGameEffectExecutionersEyes::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EXECUTIONERS_EYES]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_EXECUTIONERS_EYES] = true;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 120);

        pSprite->SetColorRange(8);
        pSprite->GetDerivedStats()->m_nTHAC0 += 4;
        pSprite->GetDerivedStats()->m_nCriticalHitBonus += 4;
    }
    return TRUE;
}

// 0x56B6E0
void IcewindCGameEffectExecutionersEyes::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E01"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A1B40
IcewindCGameEffectWhenStruck::IcewindCGameEffectWhenStruck(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1B80
CGameEffect* IcewindCGameEffectWhenStruck::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectWhenStruck* copy = new IcewindCGameEffectWhenStruck(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56A5A0
BOOL IcewindCGameEffectWhenStruck::ApplyEffect(CGameSprite* pSprite)
{
    COnHitSpell spell;
    spell.m_sourceRes = m_sourceRes;
    spell.m_spellRes = m_res;
    spell.m_level = static_cast<BYTE>(m_casterLevel);
    spell.m_target = m_target;
    pSprite->m_derivedStats.m_onHitSpells.push_back(spell);

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessage117(m_sourceRes,
            m_res, static_cast<BYTE>(m_casterLevel), m_target, pSprite->m_id, pSprite->m_id), FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1C30
IcewindCGameEffectProjectile::IcewindCGameEffectProjectile(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1C70
CGameEffect* IcewindCGameEffectProjectile::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectProjectile* copy = new IcewindCGameEffectProjectile(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56A730
BOOL IcewindCGameEffectProjectile::ApplyEffect(CGameSprite* pSprite)
{
    CGameObjectArray* objects = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
    CGameObject* pSource;
    if (objects->GetShare(m_sourceID, CGameObjectArray::THREAD_ASYNCH,
            &pSource, INFINITE) == CGameObjectArray::SUCCESS) {
        CProjectile* projectile = CProjectile::DecodeProjectile(m_dwFlags + 1, pSprite, 0);
        std::list<CGameEffect*> effects = BuildSubSpellEffects(
            static_cast<CGameSprite*>(pSource), m_res, 1);
        for (CGameEffect* effect : effects) {
            effect->m_sourceRes = m_res;
            effect->m_target = m_target;
            effect->m_casterLevel = m_casterLevel;
            projectile->AddEffect(effect);
        }
        projectile->Fire(pSprite->m_pArea, pSource->m_id, pSprite->m_id,
            pSprite->GetPos(), 0, 0);
        objects->ReleaseShare(m_sourceID, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }
    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1D20
IcewindCGameEffectEnergyDrain::IcewindCGameEffectEnergyDrain(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1D60
CGameEffect* IcewindCGameEffectEnergyDrain::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectEnergyDrain* copy = new IcewindCGameEffectEnergyDrain(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56A910
BOOL IcewindCGameEffectEnergyDrain::ApplyEffect(CGameSprite* pSprite)
{
    if (pSprite != NULL && pSprite->m_bInUnmarshal) {
        return FALSE;
    }

    if (m_secondaryType) {
        DisplayStringRef(pSprite, 16286);
    }

    // NOTE: Using `set` (with bounds check).
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_ENERGY_DRAINED, true);

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 123);

    pSprite->GetDerivedStats()->m_nTHAC0 -= static_cast<SHORT>(m_effectAmount);
    pSprite->GetDerivedStats()->m_nSaveVSFortitude -= static_cast<SHORT>(m_effectAmount);
    pSprite->GetDerivedStats()->m_nSaveVSReflex -= static_cast<SHORT>(m_effectAmount);
    pSprite->GetDerivedStats()->m_nSaveVSWill -= static_cast<SHORT>(m_effectAmount);
    pSprite->GetDerivedStats()->m_nMaxHitPoints += -5 * static_cast<SHORT>(m_effectAmount);

    if (m_secondaryType) {
        pSprite->GetBaseStats()->m_hitPoints += -5 * static_cast<SHORT>(m_effectAmount);
    }

    if (pSprite->GetBaseStats()->m_hitPoints <= 0 || pSprite->GetDerivedStats()->m_nMaxHitPoints <= 0) {
        CGameEffectDeath* pDeath = new CGameEffectDeath();
        pDeath->m_source = m_source;
        pDeath->m_sourceID = m_sourceID;
        pDeath->m_dwFlags = 0x4;
        pSprite->AddEffect(pDeath, CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);

        pSprite->GetDerivedStats()->m_nMaxHitPoints = 1;
        pSprite->GetBaseStats()->m_hitPoints = 0;
        m_done = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1E10
IcewindCGameEffectTortoiseShell::IcewindCGameEffectTortoiseShell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1E50
CGameEffect* IcewindCGameEffectTortoiseShell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectTortoiseShell* copy = new IcewindCGameEffectTortoiseShell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56AAE0
BOOL IcewindCGameEffectTortoiseShell::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_TORTOISE_SHELL]) {
        if (m_secondaryType) {
            DisplayStringRef(pSprite, 17063); // "Encased in Tortoise Shell"
        }

        // NOTE: Using `set` (with bounds check).
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_TORTOISE_SHELL, true);
        pSprite->GetDerivedStats()->m_visualEffects.set(IWD_VFX_TORTOISE_SHELL, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 125);

        pSprite->field_9D15 = 1;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_HELD] = true;
        pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;
        pSprite->GetDerivedStats()->m_nTortoiseShellPoints = m_effectAmount;
    } else {
        m_done = TRUE;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1F00
IcewindCGameEffectBlink::IcewindCGameEffectBlink(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A1F40
CGameEffect* IcewindCGameEffectBlink::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectBlink* copy = new IcewindCGameEffectBlink(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56ABB0
BOOL IcewindCGameEffectBlink::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_BLINK] = true;
    pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_INVISIBILITY] = true;

    switch (m_dwFlags) {
    case 0:
        AddPortraitIcon(pSprite, 130);
        break;
    case 1:
        AddPortraitIcon(pSprite, 145);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameEffects.cpp
        // __LINE__: 6894
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A1FF0
IcewindCGameEffectPersistantEffect::IcewindCGameEffectPersistantEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2030
CGameEffect* IcewindCGameEffectPersistantEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectPersistantEffect* copy = new IcewindCGameEffectPersistantEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A20E0
IcewindCGameEffectDamageReduction::IcewindCGameEffectDamageReduction(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2120
CGameEffect* IcewindCGameEffectDamageReduction::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectDamageReduction* copy = new IcewindCGameEffectDamageReduction(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56ADC0
BOOL IcewindCGameEffectDamageReduction::ApplyEffect(CGameSprite* pSprite)
{
    // The binary leaves the amount uninitialized on the m_effectAmount != 0
    // path, so it holds the incoming pSprite argument slot; only the
    // m_effectAmount == 0 path -- which this opcode's data always takes, the DR
    // value being the m_dwFlags "level" x5 -- assigns it.
    LONG amount = reinterpret_cast<LONG>(pSprite);
    if (m_effectAmount == 0) {
        if (m_dwFlags == 6) {
            m_dwFlags = 3;
        }
        amount = m_dwFlags * 5;
    }

    DR_ENTRY entry(amount, m_dwFlags);
    entry.AddDamageType(0);
    entry.AddDamageType(0x100000);
    entry.AddDamageType(0x1000000);
    entry.AddDamageType(0x2000000);
    entry.AddDamageType(0x4000000);
    entry.AddDamageType(0x8000000);
    pSprite->GetDerivedStats()->m_drReductions.push_back(entry);
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A21D0
IcewindCGameEffectProtectionFromArrows::IcewindCGameEffectProtectionFromArrows(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2210
CGameEffect* IcewindCGameEffectProtectionFromArrows::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectProtectionFromArrows* copy = new IcewindCGameEffectProtectionFromArrows(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56AF80
BOOL IcewindCGameEffectProtectionFromArrows::ApplyEffect(CGameSprite* pSprite)
{
    DR_ENTRY entry(10, m_dwFlags);
    entry.AddDamageType(0x4000000);
    entry.AddDamageType(0x2000000);
    pSprite->GetDerivedStats()->m_drReductions.push_back(entry);
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A22C0
IcewindCGameEffectTensersTransformation::IcewindCGameEffectTensersTransformation(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2300
CGameEffect* IcewindCGameEffectTensersTransformation::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectTensersTransformation* copy = new IcewindCGameEffectTensersTransformation(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B0F0
BOOL IcewindCGameEffectTensersTransformation::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_TENSERS_TRANSFORMATION]) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_TENSERS_TRANSFORMATION, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 55);

        pSprite->SetColorRange(62);

        if (m_secondaryType) {
            m_effectAmount2 = IcewindMisc::Roll(m_casterLevel, 6);
            m_effectAmount3 = IcewindMisc::Roll(2, 4);
            m_effectAmount4 = IcewindMisc::Roll(2, 4);
        }

        pSprite->GetDerivedStats()->m_nMaxHitPoints += static_cast<SHORT>(m_effectAmount2);
        if (m_secondaryType) {
            pSprite->GetBaseStats()->m_hitPoints += static_cast<SHORT>(m_effectAmount2);
        }
        pSprite->GetDerivedStats()->m_nSTR += static_cast<SHORT>(m_effectAmount3);
        pSprite->GetDerivedStats()->m_nDEX += static_cast<SHORT>(m_effectAmount4);
        pSprite->GetDerivedStats()->m_nTHAC0 += static_cast<BYTE>(m_casterLevel) / 2;
        pSprite->GetDerivedStats()->m_nSaveVSFortitude += 5;
        pSprite->GetDerivedStats()->m_nACDodgeBonus += 4;
        pSprite->GetDerivedStats()->m_disabledSpellTypes[0] = TRUE;
        pSprite->GetDerivedStats()->m_disabledSpellTypes[1] = TRUE;
    }
    return TRUE;
}

// 0x56B6E0
void IcewindCGameEffectTensersTransformation::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E01"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A23B0
IcewindCGameEffectSmiteEvil::IcewindCGameEffectSmiteEvil(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A23F0
CGameEffect* IcewindCGameEffectSmiteEvil::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectSmiteEvil* copy = new IcewindCGameEffectSmiteEvil(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B220
BOOL IcewindCGameEffectSmiteEvil::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_SMITE_EVIL] = true;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A24A0
IcewindCGameEffectRestoration::IcewindCGameEffectRestoration(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A24E0
CGameEffect* IcewindCGameEffectRestoration::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectRestoration* copy = new IcewindCGameEffectRestoration(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4A2590
IcewindCGameEffectAlicornLanceGlow::IcewindCGameEffectAlicornLanceGlow(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A25D0
CGameEffect* IcewindCGameEffectAlicornLanceGlow::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectAlicornLanceGlow* copy = new IcewindCGameEffectAlicornLanceGlow(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B4B0
BOOL IcewindCGameEffectAlicornLanceGlow::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 21646);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ALICORN_LANCE]) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ALICORN_LANCE] = true;
        pSprite->GetDerivedStats()->m_nACDodgeBonus -= 2;
        AddColorEffect(pSprite, 185, 185, 185);
        m_done = FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A2680
IcewindCGameEffectGlobeOfInvulnerability::IcewindCGameEffectGlobeOfInvulnerability(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A26C0
CGameEffect* IcewindCGameEffectGlobeOfInvulnerability::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectGlobeOfInvulnerability* copy = new IcewindCGameEffectGlobeOfInvulnerability(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B520
BOOL IcewindCGameEffectGlobeOfInvulnerability::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_secondaryType) {
            DisplayStringRef(pSprite, 14771);
        }

        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_MINOR_GLOBE_OF_INVULNERABILITY, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 96);

        pSprite->GetDerivedStats()->m_visualEffects.set(IWD_VFX_MINOR_GLOBE_OF_INVULNERABILITY);
        // Immune to spells of level 1-3 -- the table CProjectile::IsTargetImmune
        // indexes with the projectile's spell level.
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[1] = TRUE;
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[2] = TRUE;
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[3] = TRUE;
        break;
    case 1:
        if (m_secondaryType) {
            DisplayStringRef(pSprite, 14771);
        }

        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_GLOBE_OF_INVULNERABILITY, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 97);

        pSprite->GetDerivedStats()->m_visualEffects.set(IWD_VFX_GLOBE_OF_INVULNERABILITY);
        // Immune to spells of level 1-4.
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[1] = TRUE;
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[2] = TRUE;
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[3] = TRUE;
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[4] = TRUE;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\IcewindCGameEffects.cpp
        // __LINE__: 7204
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// 0x56B6E0
void IcewindCGameEffectGlobeOfInvulnerability::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E01"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A2770
IcewindCGameEffectLowerResistance::IcewindCGameEffectLowerResistance(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A27B0
CGameEffect* IcewindCGameEffectLowerResistance::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectLowerResistance* copy = new IcewindCGameEffectLowerResistance(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B680
BOOL IcewindCGameEffectLowerResistance::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 16967); // "Spell Resistance Lowered"
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[CGAMEEFFECT_CASTINGGLOW]) {
        pSprite->GetDerivedStats()->m_spellStates[CGAMEEFFECT_CASTINGGLOW] = true;
        AdjustResistMagic(pSprite, -static_cast<BYTE>(m_casterLevel) * 2);
    }

    return TRUE;
}

// 0x56B6E0
void IcewindCGameEffectLowerResistance::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E01"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A2860
IcewindCGameEffectBane::IcewindCGameEffectBane(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A28A0
CGameEffect* IcewindCGameEffectBane::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectBane* copy = new IcewindCGameEffectBane(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B710
BOOL IcewindCGameEffectBane::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        DisplayStringRef(pSprite, 14406); // "Afflicted by Bane"
        RemoveAllOfType(pSprite, CGAMEEFFECT_NON_CUMULATIVEAID, -1);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[CGAMEEFFECT_DURATIONCASTING]) {
        AddPortraitIcon(pSprite, 35);
        AddColorEffect(pSprite, 0, 0, 128, 30);
        SetSpellState(pSprite, CGAMEEFFECT_DURATIONCASTING);
        AdjustTHAC0(pSprite, -1);
        pSprite->GetBaseStats()->m_morale--;
    }

    return TRUE;
}

// 0x4BAF90
void IcewindCGameEffectBane::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E04"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x4A2950
IcewindCGameEffectFeatPowerAttack::IcewindCGameEffectFeatPowerAttack(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2990
CGameEffect* IcewindCGameEffectFeatPowerAttack::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFeatPowerAttack* copy = new IcewindCGameEffectFeatPowerAttack(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B790
BOOL IcewindCGameEffectFeatPowerAttack::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK)) {
        return FALSE;
    }

    INT nRank = pSprite->GetFeatRank(CGAMESPRITE_FEAT_POWER_ATTACK);

    // NOTE: Using `set` (with bounds check).
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_POWER_ATTACK_1, nRank == 1);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_POWER_ATTACK_2, nRank == 2);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_POWER_ATTACK_3, nRank == 3);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_POWER_ATTACK_4, nRank == 4);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_POWER_ATTACK_5, nRank == 5);

    if (nRank != 0
        && (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_1)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_2)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_3)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_4)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_5))) {
        if (m_secondaryType) {
            // NOTE: Odd mixing of `test` (with bounds check) and `operator[]`.
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_1)
                || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_2)
                || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_3]
                || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_4]
                || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_5]) {
                pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                    1,
                    0,
                    0,
                    35785, // // "Expertise"
                    0,
                    0);
            }
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                0,
                0,
                0,
                35794, // "Power Attack"
                0,
                0);
        }

        // NOTE: Odd mixing of `set` (with bounds check) and `operator[]`.
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_EXPERTISE_1, false);
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_2] = false;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_3] = false;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_4] = false;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_EXPERTISE_5] = false;
    } else {
        m_done = TRUE;
        if (m_secondaryType) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                1,
                nRank,
                0,
                35794, // "Power Attack"
                0,
                0);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A2A40
IcewindCGameEffectFeatExpertise::IcewindCGameEffectFeatExpertise(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2A80
CGameEffect* IcewindCGameEffectFeatExpertise::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFeatExpertise* copy = new IcewindCGameEffectFeatExpertise(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56B960
BOOL IcewindCGameEffectFeatExpertise::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->HasFeat(CGAMESPRITE_FEAT_EXPERTISE)) {
        return FALSE;
    }

    INT nRank = pSprite->GetFeatRank(CGAMESPRITE_FEAT_EXPERTISE);

    // NOTE: Using `set` (with bounds check).
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_EXPERTISE_1, nRank == 1);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_EXPERTISE_2, nRank == 2);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_EXPERTISE_3, nRank == 3);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_EXPERTISE_4, nRank == 4);
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_EXPERTISE_5, nRank == 5);

    if (nRank != 0
        && (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_1)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_2)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_3)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_4)
            || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_EXPERTISE_5))) {
        if (m_secondaryType) {
            // NOTE: Odd mixing of `test` (with bounds check) and `operator[]`.
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_1)
                || pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_FEAT_POWER_ATTACK_2)
                || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_3]
                || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_4]
                || pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_5]) {
                pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                    1,
                    0,
                    0,
                    35794, // "Power Attack"
                    0,
                    0);
            }
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                0,
                0,
                0,
                35785, // "Expertise"
                0,
                0);
        }

        // NOTE: Odd mixing of `set` (with bounds check) and `operator[]`.
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_FEAT_POWER_ATTACK_1, false);
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_2] = false;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_3] = false;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_4] = false;
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_POWER_ATTACK_5] = false;
    } else {
        m_done = TRUE;
        if (m_secondaryType) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                1,
                nRank,
                0,
                35785, // "Expertise"
                0,
                0);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A2B30
IcewindCGameEffectFeatArterialStrike::IcewindCGameEffectFeatArterialStrike(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2B70
CGameEffect* IcewindCGameEffectFeatArterialStrike::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFeatArterialStrike* copy = new IcewindCGameEffectFeatArterialStrike(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56BB30
BOOL IcewindCGameEffectFeatArterialStrike::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->HasFeat(CGAMESPRITE_FEAT_ARTERIAL_STRIKE)) {
        return FALSE;
    }

    INT nRank = pSprite->GetFeatRank(CGAMESPRITE_FEAT_ARTERIAL_STRIKE);
    if (nRank > 0) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_ARTERIAL_STRIKE] = true;
        if (m_secondaryType != 0) {
            if (pSprite->GetDerivedStats()->m_spellStates[CGAMESPRITE_FEAT_HAMSTRING]) {
                pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                    1,
                    0,
                    0,
                    35787, // "Hamstring"
                    0,
                    0);
            }
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                0,
                0,
                0,
                35774, // "Arterial Strike"
                0,
                0);
        }
        pSprite->GetDerivedStats()->m_spellStates[CGAMESPRITE_FEAT_HAMSTRING] = false;
    } else {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_ARTERIAL_STRIKE] = false;
        m_done = TRUE;
        if (m_secondaryType != 0) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                1,
                0,
                0,
                35774, // "Arterial Strike"
                0,
                0);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A2C20
IcewindCGameEffectFeatHamstring::IcewindCGameEffectFeatHamstring(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2C60
CGameEffect* IcewindCGameEffectFeatHamstring::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    IcewindCGameEffectFeatHamstring* copy = new IcewindCGameEffectFeatHamstring(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56BC20
BOOL IcewindCGameEffectFeatHamstring::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->HasFeat(CGAMESPRITE_FEAT_HAMSTRING)) {
        return FALSE;
    }

    INT nRank = pSprite->GetFeatRank(CGAMESPRITE_FEAT_HAMSTRING);
    if (nRank > 0) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_HAMSTRING] = true;
        if (m_secondaryType != 0) {
            if (pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_ARTERIAL_STRIKE]) {
                pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                    1,
                    0,
                    0,
                    35774, // "Arterial Strike"
                    0,
                    0);
            }
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                0,
                0,
                0,
                35787, // "Hamstring"
                0,
                0);
        }
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_ARTERIAL_STRIKE] = false;
    } else {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_HAMSTRING] = false;
        m_done = TRUE;
        if (m_secondaryType != 0) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                1,
                0,
                0,
                35787, // "Hamstring"
                0,
                0);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4A2D10
IcewindCGameEffectFeatRapidShot::IcewindCGameEffectFeatRapidShot(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4A2D50
CGameEffect* IcewindCGameEffectFeatRapidShot::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectStoneSkins* copy = new CGameEffectStoneSkins(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x56BD10
BOOL IcewindCGameEffectFeatRapidShot::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->HasFeat(CGAMESPRITE_FEAT_RAPID_SHOT)) {
        return FALSE;
    }

    INT nRank = pSprite->GetFeatRank(CGAMESPRITE_FEAT_RAPID_SHOT);
    if (nRank > 0) {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_RAPID_SHOT] = true;
        if (m_secondaryType != 0) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                0,
                0,
                0,
                35796, // "Rapid Shot"
                0,
                0);
        }
    } else {
        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_FEAT_RAPID_SHOT] = false;
        m_done = TRUE;
        if (m_secondaryType != 0) {
            pSprite->FeedBack(CGameSprite::FEEDBACK_TOGGLEFEAT,
                1,
                0,
                0,
                35796, // "Rapid Shot"
                0,
                0);
        }
    }

    return TRUE;
}
