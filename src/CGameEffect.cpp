#include "CGameEffect.h"

#include "CAIScript.h"
#include "CAITrigger.h"
#include "CMessage.h"
#include "CBaldurChitin.h"
#include "CBaldurProjector.h"
#include "CBlood.h"
#include "CGameAnimationType.h"
#include "CGameArea.h"
#include "CGameSprite.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CProjectileSPBOOM.h"
#include "CTimerWorld.h"
#include "CScreenWorld.h"
#include "CSevenEyes.h"
#include "CSpell.h"
#include "CUtil.h"
#include "CVidPalette.h"
#include "Icewind586B70.h"
#include "IcewindCGameEffects.h"
#include "IcewindMisc.h"

// NOTE: Not used. It does not include many effects. Formatting is odd.
//
// 0x8D45F0
const CString CGameEffect::EFFECT_NAME[] = {
    "AC                        ",
    "ATTACKNO                  ",
    "AWAKEN                    ",
    "BERSERK                   ",
    "CALM                      ",
    "CHARM                     ",
    "CHR                       ",
    "COLORCHANGE               ",
    "COLORGLOW_SOLID           ",
    "COLORGLOW_PULSE           ",
    "CON                       ",
    "CUREPOISON                ",
    "DAMAGE                    ",
    "DEATH                     ",
    "DEFROST                   ",
    "DEX                       ",
    "HASTE                     ",
    "HEAL                      ",
    "HITPOINTS                 ",
    "INT                       ",
    "INVISIBLE                 ",
    "LORE                      ",
    "LUCK                      ",
    "MORALE                    ",
    "PANIC                     ",
    "POISON                    ",
    "REMOVECURSE               ",
    "RESISTACID                ",
    "RESISTCOLD                ",
    "RESISTELECTRICITY         ",
    "RESISTFIRE                ",
    "RESISTMAGIC               ",
    "RESURRECT                 ",
    "SAVEFORTITUDE             ",
    "SAVEREFLEX                ",
    "SAVEWILL                  ",
    "SILENCE                   ",
    "SLEEP                     ",
    "SLOW                      ",
    "SPARKLE                   ",
    "SPELLMEMORIZATIONMAGE     ",
    "STONETOFLESH              ",
    "STR                       ",
    "STUN                      ",
    "UNSTUN                    ",
    "VISIBLE                   ",
    "VOCALIZE                  ",
    "WIS                       ",
    "SINGLECOLORPULSEALL       ",
    "COLORTINT_SOLID           ",
    "COLORLIGHT_SOLID          ",
    "ANIMATION_CHANGE          ",
    "THAC0                     ",
    "SLAY                      ",
    "ALIGNMENTREVERSAL         ",
    "ALIGNMENTCHANGE           ",
    "DISPELEFFECTS             ",
    "SKILLSTEALTH              ",
    "CASTINGFAILURE            ",
    "",
    "SPELLMEMORIZATIONCLERIC   ",
    "INFRAVISION               ",
    "INFRAVISIONOFF            ",
    "BLUR                      ",
    "TRANSLUCENT               ",
    "SUMMON                    ",
    "UNSUMMON                  ",
    "NONDETECTION              ",
    "ENDNONDETECTION           ",
    "SEXCHANGE                 ",
    "AICHANGE                  ",
    "DAMAGEMOD                 ",
    "BLINDNESS                 ",
    "CUREBLINDESS              ",
    "FEEBLEMINDEDNESS          ",
    "CUREFEEBLEMINDEDNESS      ",
    "DISEASE                   ",
    "CUREDISEASE               ",
    "DEAFNESS                  ",
    "CUREDEAFNESS              ",
    "SETAISCRIPT               ",
    "IMMUNITYTOPROJECTILE      ",
    "RESISTMAGICFIRE \t\t  ",
    "RESISTMAGICCOLD \t\t  ",
    "RESISTSLASHING \t\t",
    "RESISTBLUDGEONING\t\t",
    "RESISTPIERCING \t\t",
    "RESISTMISSILE  \t\t",
    "SKILLLOCKPICKING\t\t  ",
    "SKILLTRAPS\t\t\t\t  ",
    "SKILLPICKPOCKET \t\t  ",
    "FATIGUE \t\t\t\t  ",
    "INTOXICATION \t\t\t  ",
    "SKILLTRACKING  \t\t",
    "LEVEL \t\t\t\t\t  ",
    "STREXTRA \t\t\t\t  ",
    "REGENERATION\t\t\t  ",
    "DURATIONMOD \t\t\t  ",
    "PROTECTIONFROMCREATURE\t  ",
    "IMMUNITYTOEFFECT    \t  ",
    "IMMUNITYTOSPELLLEVEL  \t  ",
    "NAME                  \t  ",
    "XP                    \t  ",
    "GOLD                  \t  ",
    "MORALEBREAK           \t  ",
    "PORTRAIT              \t  ",
    "REPUTATION            \t  ",
    "HOLD_CREATURE             ",
    "RETREAT_FROM              ",
    "CREATE_WEAPON             ",
    "DESTROY_WEAPON            ",
    "EQUIP_WEAPON              ",
    "DITHER                    ",
    "DETECT_ALIGNMENT          ",
    "DISPEL_INVISIBLE          ",
    "SHOW_AREA                 ",
    "SHOW_CREATURES            ",
    "MIRROR_IMAGE              ",
    "IMMUNITY_TO_WEAPON        ",
    "VISUAL_ANIMATION_EFFECT   ",
    "CREATE_ITEM               ",
    "DESTROY_ITEM              ",
    "TELEPORT                  ",
    "KNOCK                     ",
    "MOVEMENT_RATE             ",
    "RANDOM_SUMMON             ",
    "CONFUSION                 ",
    "NON_CUMULATIVE_AID        ",
    "NON_CUMULATIVE_BLESS      ",
    "NON_CUMULATIVE_CHANT      ",
    "NON_CUMULATIVE_DRAW_UPON_HOLY_MIGHT",
    "NON_CUMULATIVE_LUCK       ",
    "PETRIFICATION             ",
    "POLYMORPH                 ",
    "FORCE_VISIBLE             ",
    "NON_CUMULATIVE_CHANTBAD   ",
    "SETSEQUENCE               ",
    "DISPLAYSTRING             ",
    "CASTINGGLOW               ",
    "VISUALSPELLHIT            ",
    "PORTRAITICON              ",
    "REPLACE_ITEM              ",
    "DISABLE_BUTTON            ",
    "DISABLE_SPELLTYPE         ",
    "CAST_SPELL                ",
    "LEARN_SPELL               ",
    "CAST_SPELL_POINT          ",
    "IDENTIFY                  ",
    "DETECTTRAPS",
    "REPLACESELF",
    "PLAYMOVIE",
    "SANCTUARY",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "q",
    "q",
    "q",
    "q",
    "q",
};

// NOTE: Inlined.
CGameEffectBase::CGameEffectBase()
{
    memset(this, 0, sizeof(*this));

    m_version = "EFF V2.0";
    m_effectID = 0;
    m_targetType = 0;
    m_spellLevel = 0;
    m_effectAmount = 0;
    m_dwFlags = 0;
    m_durationType = 0;
    m_duration = 0;
    m_probabilityUpper = 100;
    m_probabilityLower = 0;
    m_res = "";
    m_numDice = 0;
    m_diceSize = 0;
    m_savingThrow = 0;
    m_saveMod = 0;
    m_special = 0;
    m_school = 0;
    field_48 = 0;
    m_minLevel = 0;
    m_maxLevel = 0;
    m_flags = 0;
    m_effectAmount2 = 0;
    m_effectAmount3 = 0;
    m_effectAmount4 = 0;
    m_effectAmount5 = 0;
    m_res2 = "";
    m_res3 = "";
    m_source.x = -1;
    m_source.y = -1;
    m_target.x = -1;
    m_target.y = -1;
    m_sourceType = 0;
    m_sourceRes = "";
    m_sourceFlags = 0;
    m_projectileType = 0;
    m_slotNum = -1;
    m_casterLevel = 0;
    m_firstCall = 0;
    m_secondaryType = 1;
    memset(m_pad, 0, sizeof(m_pad));
}

// 0x48C800
CGameEffect* CGameEffect::DecodeEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, const CPoint& target)
{
    if (effect->diceSize == -1) {
        effect->diceSize = 0;
    }

    if (effect->effectID == 36 || effect->effectID == 37) {
        CString sError;
        sError.Format("Old saving throw effect (breath or spell) detected:  id=%d  res=%s",
            effect->effectID,
            effect->res);
        effect->effectID = CGAMEEFFECT_SAVEVSWILL;
    }

    switch (effect->effectID) {
    case CGAMEEFFECT_AC:
        return new CGameEffectAC(effect, source, sourceID, target);
    case CGAMEEFFECT_ATTACKNO:
        return new CGameEffectAttackNo(effect, source, sourceID, target);
    case CGAMEEFFECT_AWAKEN:
        return new CGameEffectAwaken(effect, source, sourceID, target);
    case CGAMEEFFECT_BERSERK:
        return new CGameEffectBerserk(effect, source, sourceID, target);
    case CGAMEEFFECT_CALM:
        return new CGameEffectCalm(effect, source, sourceID, target);
    case CGAMEEFFECT_CHARM:
        return new CGameEffectCharm(effect, source, sourceID, target);
    case CGAMEEFFECT_CHR:
        return new CGameEffectCHR(effect, source, sourceID, target);
    case CGAMEEFFECT_COLORCHANGE:
        return new CGameEffectColorChange(effect, source, sourceID, target);
    case CGAMEEFFECT_COLORGLOWSOLID:
        return new CGameEffectColorGlowSolid(effect, source, sourceID, target);
    case CGAMEEFFECT_COLORGLOWPULSE:
        return new CGameEffectColorGlowPulse(effect, source, sourceID, target);
    case CGAMEEFFECT_CON:
        return new CGameEffectCON(effect, source, sourceID, target);
    case CGAMEEFFECT_CUREPOISON:
        return new CGameEffectCurePoison(effect, source, sourceID, target);
    case CGAMEEFFECT_DAMAGE:
        return new CGameEffectDamage(effect, source, sourceID, target);
    case CGAMEEFFECT_DEATH:
        return new CGameEffectDeath(effect, source, sourceID, target);
    case CGAMEEFFECT_DEFROST:
        return new CGameEffectDefrost(effect, source, sourceID, target);
    case CGAMEEFFECT_DEX:
        return new CGameEffectDEX(effect, source, sourceID, target);
    case CGAMEEFFECT_HASTE:
        return new CGameEffectHaste(effect, source, sourceID, target);
    case CGAMEEFFECT_HEAL:
        return new CGameEffectHeal(effect, source, sourceID, target);
    case CGAMEEFFECT_HITPOINTS:
        return new CGameEffectHitPoints(effect, source, sourceID, target);
    case CGAMEEFFECT_INT:
        return new CGameEffectINT(effect, source, sourceID, target);
    case CGAMEEFFECT_INVISIBLE:
        return new CGameEffectInvisible(effect, source, sourceID, target);
    case CGAMEEFFECT_LORE:
        return new CGameEffectLore(effect, source, sourceID, target);
    case CGAMEEFFECT_LUCK:
        return new CGameEffectLuck(effect, source, sourceID, target);
    case CGAMEEFFECT_MORALE:
        return new CGameEffectMorale(effect, source, sourceID, target);
    case CGAMEEFFECT_PANIC:
        return new CGameEffectPanic(effect, source, sourceID, target);
    case CGAMEEFFECT_POISON:
        return new CGameEffectPoison(effect, source, sourceID, target);
    case CGAMEEFFECT_REMOVECURSE:
        return new CGameEffectRemoveCurse(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTACID:
        return new CGameEffectResistAcid(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTCOLD:
        return new CGameEffectResistCold(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTELECTRICITY:
        return new CGameEffectResistElectricity(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTFIRE:
        return new CGameEffectResistFire(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTMAGIC:
        return new CGameEffectResistMagic(effect, source, sourceID, target);
    case CGAMEEFFECT_RESURRECT:
        return new CGameEffectResurrect(effect, source, sourceID, target);
    case CGAMEEFFECT_SAVEVSFORTITUDE:
        return new CGameEffectSaveVsFortitude(effect, source, sourceID, target);
    case CGAMEEFFECT_SAVEVSRELFEX:
        return new CGameEffectSaveVsRelfex(effect, source, sourceID, target);
    case CGAMEEFFECT_SAVEVSWILL:
        return new CGameEffectSaveVsWill(effect, source, sourceID, target);
    case CGAMEEFFECT_SILENCE:
        return new CGameEffectSilence(effect, source, sourceID, target);
    case CGAMEEFFECT_SLEEP:
        return new CGameEffectSleep(effect, source, sourceID, target);
    case CGAMEEFFECT_SLOW:
        return new CGameEffectSlow(effect, source, sourceID, target);
    case CGAMEEFFECT_SPARKLE:
        return new CGameEffectSparkle(effect, source, sourceID, target);
    case CGAMEEFFECT_SPELLMEMORIZATIONMAGE:
        return new CGameEffectSpellMemorizationMage(effect, source, sourceID, target);
    case CGAMEEFFECT_STONETOFLESH:
        return new CGameEffectStoneToFlesh(effect, source, sourceID, target);
    case CGAMEEFFECT_STR:
        return new CGameEffectSTR(effect, source, sourceID, target);
    case CGAMEEFFECT_STUN:
        return new CGameEffectStun(effect, source, sourceID, target);
    case CGAMEEFFECT_UNSTUN:
        return new CGameEffectUnstun(effect, source, sourceID, target);
    case CGAMEEFFECT_VISIBLE:
        return new CGameEffectVisible(effect, source, sourceID, target);
    case CGAMEEFFECT_VOCALIZE:
        return new CGameEffectVocalize(effect, source, sourceID, target);
    case CGAMEEFFECT_WIS:
        return new CGameEffectWIS(effect, source, sourceID, target);
    case CGAMEEFFECT_SINGLECOLORPULSEALL:
        return new CGameEffectSingleColorPulseAll(effect, source, sourceID, target);
    case CGAMEEFFECT_COLORTINTSOLID:
        return new CGameEffectColorTintSolid(effect, source, sourceID, target);
    case CGAMEEFFECT_COLORLIGHTSOLID:
        return new CGameEffectColorLightSolid(effect, source, sourceID, target);
    case CGAMEEFFECT_ANIMATIONCHANGE:
        return new CGameEffectAnimationChange(effect, source, sourceID, target);
    case CGAMEEFFECT_BASEATTACKBONUS:
        return new CGameEffectBaseAttackBonus(effect, source, sourceID, target);
    case CGAMEEFFECT_SLAY:
        return new CGameEffectSlay(effect, source, sourceID, target);
    case CGAMEEFFECT_ALIGNMENTREVERSAL:
        return new CGameEffectAlignmentReversal(effect, source, sourceID, target);
    case CGAMEEFFECT_ALIGNMENTCHANGE:
        return new CGameEffectAlignmentChange(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPELEFFECTS:
        return new CGameEffectDispelEffects(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLSTEALTH:
        return new CGameEffectSkillStealth(effect, source, sourceID, target);
    case CGAMEEFFECT_CASTINGFAILURE:
        return new CGameEffectCastingFailure(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLALCHEMY:
        return new CGameEffectSkillAlchemy(effect, source, sourceID, target);
    case CGAMEEFFECT_SPELLMEMOIZATIONCLERIC:
        return new CGameEffectSpellMemoizationCleric(effect, source, sourceID, target);
    case CGAMEEFFECT_INFRAVISION:
        return new CGameEffectInfravision(effect, source, sourceID, target);
    case CGAMEEFFECT_INFRAVISIONOFF:
        return new CGameEffectInfravisionOff(effect, source, sourceID, target);
    case CGAMEEFFECT_BLUR:
        return new CGameEffectBlur(effect, source, sourceID, target);
    case CGAMEEFFECT_TRANSLUCENT:
        return new CGameEffectTranslucent(effect, source, sourceID, target);
    case CGAMEEFFECT_SUMMON:
        return new IcewindCGameEffectSummon(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLUNSUMMON:
        return new CGameEffectSkillUnsummon(effect, source, sourceID, target);
    case CGAMEEFFECT_NONDETECTION:
        return new CGameEffectNonDetection(effect, source, sourceID, target);
    case CGAMEEFFECT_ENDNONDETECTION:
        return new CGameEffectEndNonDetection(effect, source, sourceID, target);
    case CGAMEEFFECT_SEXCHANGE:
        return new CGameEffectSexChange(effect, source, sourceID, target);
    case CGAMEEFFECT_AICHANGE:
        return new CGameEffectAIChange(effect, source, sourceID, target);
    case CGAMEEFFECT_DAMAGEMOD:
        return new CGameEffectDamageMod(effect, source, sourceID, target);
    case CGAMEEFFECT_BLINDNESS:
        return new CGameEffectBlindness(effect, source, sourceID, target);
    case CGAMEEFFECT_CUREBLINDNESS:
        return new CGameEffectCureBlindness(effect, source, sourceID, target);
    case CGAMEEFFECT_FEEBLEMINDEDNESS:
        return new CGameEffectFeebleMindedness(effect, source, sourceID, target);
    case CGAMEEFFECT_CUREFEEBLEMINDEDNESS:
        return new CGameEffectCureFeebleMindedness(effect, source, sourceID, target);
    case CGAMEEFFECT_DISEASE:
        return new CGameEffectDisease(effect, source, sourceID, target);
    case CGAMEEFFECT_CUREDISEASE:
        return new CGameEffectCureDisease(effect, source, sourceID, target);
    case CGAMEEFFECT_DEAFNESS:
        return new CGameEffectDeafness(effect, source, sourceID, target);
    case CGAMEEFFECT_CUREDEAFNESS:
        return new CGameEffectCureDeafness(effect, source, sourceID, target);
    case CGAMEEFFECT_SETAISCRIPT:
        return new CGameEffectSetAIScript(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYTOPROJECTILE:
        return new CGameEffectImmunityToProjectile(effect, source, sourceID, target);
    case 84:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 890
        UTIL_ASSERT_MSG(FALSE, "CGameEffectResistMagicFire game effect not supported in Icewind II (-rjf)");
    case 85:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 894
        UTIL_ASSERT_MSG(FALSE, "CGameEffectResistMagicCold game effect not supported in Icewind II (-rjf)");
    case CGAMEEFFECT_RESISTSLASHING:
        return new CGameEffectResistSlashing(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTCRUSHING:
        return new CGameEffectResistCrushing(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTPIERCING:
        return new CGameEffectResistPiercing(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTMISSILE:
        return new CGameEffectResistMissile(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLLOCKPICKING:
        return new CGameEffectSkillLockPicking(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLTRAPS:
        return new CGameEffectSkillTraps(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLPICKPOCKET:
        return new CGameEffectSkillPickPocket(effect, source, sourceID, target);
    case CGAMEEFFECT_FATIGUE:
        return new CGameEffectFatigue(effect, source, sourceID, target);
    case CGAMEEFFECT_INTOXICATION:
        return new CGameEffectIntoxication(effect, source, sourceID, target);
    case CGAMEEFFECT_SKILLTRACKING:
        return new CGameEffectSkillTracking(effect, source, sourceID, target);
    case CGAMEEFFECT_LEVEL:
        return new CGameEffectLevel(effect, source, sourceID, target);
    case CGAMEEFFECT_STREXTRA:
        return new CGameEffectSTRExtra(effect, source, sourceID, target);
    case CGAMEEFFECT_REGENERATION:
        return new CGameEffectRegeneration(effect, source, sourceID, target);
    case CGAMEEFFECT_DURATIONCASTING:
        return new CGameEffectDurationCasting(effect, source, sourceID, target);
    case CGAMEEFFECT_PROTECTIONFROMCREATURE:
        return new CGameEffectProtectionFromCreature(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYTOEFFECT:
        return new CGameEffectImmunityToEffect(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYTOSPELLLEVEL:
        return new CGameEffectImmunityToSpellLevel(effect, source, sourceID, target);
    case CGAMEEFFECT_NAME:
        return new CGameEffectName(effect, source, sourceID, target);
    case CGAMEEFFECT_XP:
        return new CGameEffectXP(effect, source, sourceID, target);
    case CGAMEEFFECT_GOLD:
        return new CGameEffectGold(effect, source, sourceID, target);
    case CGAMEEFFECT_MORALBREAK:
        return new CGameEffectMoralBreak(effect, source, sourceID, target);
    case CGAMEEFFECT_PORTRAIT:
        return new CGameEffectPortrait(effect, source, sourceID, target);
    case CGAMEEFFECT_REPUTATION:
        return new CGameEffectReputation(effect, source, sourceID, target);
    case CGAMEEFFECT_HOLDCREATURE:
        return new CGameEffectHoldCreature(effect, source, sourceID, target);
    case CGAMEEFFECT_RETREATFROM:
        return new CGameEffectRetreatFrom(effect, source, sourceID, target);
    case CGAMEEFFECT_CREATEWEAPON:
        return new CGameEffectCreateWeapon(effect, source, sourceID, target);
    case CGAMEEFFECT_DESTROYWEAPON:
        return new CGameEffectDestroyWeapon(effect, source, sourceID, target);
    case CGAMEEFFECT_EQUIPWEAPON:
        return new CGameEffectEquipWeapon(effect, source, sourceID, target);
    case CGAMEEFFECT_DITHER:
        return new CGameEffectDither(effect, source, sourceID, target);
    case 115:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 988
        UTIL_ASSERT_MSG(FALSE, "CGameEffectDetectAlignment game effect not supported in Icewind II (-rjf)");
    case CGAMEEFFECT_DISPELINVISIBLE:
        return new CGameEffectDispelInvisible(effect, source, sourceID, target);
    case CGAMEEFFECT_SHOWAREA:
        return new CGameEffectShowArea(effect, source, sourceID, target);
    case CGAMEEFFECT_SHOWCREATURES:
        return new CGameEffectShowCreatures(effect, source, sourceID, target);
    case CGAMEEFFECT_MIRRORIMAGE:
        return new CGameEffectMirrorImage(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNETOWEAPON:
        return new CGameEffectImmuneToWeapon(effect, source, sourceID, target);
    case CGAMEEFFECT_VISUALANIMATIONEFFECT:
        return new CGameEffectVisualAnimationEffect(effect, source, sourceID, target);
    case CGAMEEFFECT_CREATEITEM:
        return new CGameEffectCreateItem(effect, source, sourceID, target);
    case CGAMEEFFECT_DESTROYITEM:
        return new CGameEffectDestroyItem(effect, source, sourceID, target);
    case CGAMEEFFECT_TELEPORT:
        return new CGameEffectTeleport(effect, source, sourceID, target);
    case CGAMEEFFECT_KNOCK:
        return new CGameEffectKnock(effect, source, sourceID, target);
    case CGAMEEFFECT_MOVEMENTRATE_126:
        return new CGameEffectMovementRate(effect, source, sourceID, target);
    case 127:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1025
        UTIL_ASSERT_MSG(FALSE, "CGameEffectSummon game effect not supported in Icewind II (-rjf)");
    case CGAMEEFFECT_CONFUSION:
        return new CGameEffectConfusion(effect, source, sourceID, target);
    case CGAMEEFFECT_NON_CUMULATIVEAID:
        return new CGameEffectNon_CumulativeAid(effect, source, sourceID, target);
    case CGAMEEFFECT_NON_CUMULATIVEBLESS:
        return new CGameEffectNon_CumulativeBless(effect, source, sourceID, target);
    case CGAMEEFFECT_NON_CUMULATIVECHANT:
        return new CGameEffectNon_CumulativeChant(effect, source, sourceID, target);
    case CGAMEEFFECT_NON_CUMULATIVEDRAWUPONHOLYMIGHT:
        return new CGameEffectNon_CumulativeDrawUponHolyMight(effect, source, sourceID, target);
    case CGAMEEFFECT_NON_CUMULATIVELUCK:
        return new CGameEffectNon_CumulativeLuck(effect, source, sourceID, target);
    case CGAMEEFFECT_PETRIFICATION:
        return new CGameEffectPetrification(effect, source, sourceID, target);
    case CGAMEEFFECT_POLYMORPH:
        return new CGameEffectPolymorph(effect, source, sourceID, target);
    case CGAMEEFFECT_FORCEVISIBLE:
        return new CGameEffectForceVisible(effect, source, sourceID, target);
    case 137:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1056
        UTIL_ASSERT_MSG(FALSE, "CGameEffectNon_CumulativeChantBad game effect not supported in Icewind II (-rjf)");
    case CGAMEEFFECT_SETSEQUENCE:
        return new CGameEffectSetSequence(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPLAYSTRING:
        return new CGameEffectDisplayString(effect, source, sourceID, target);
    case CGAMEEFFECT_CASTINGGLOW:
        return new IcewindCGameEffectCastingGlow(effect, source, sourceID, target);
    case CGAMEEFFECT_VISUALSPELLHIT:
        return new CGameEffectVisualSpellHit(effect, source, sourceID, target);
    case CGAMEEFFECT_PORTRAITICON:
        return new CGameEffectPortraitIcon(effect, source, sourceID, target);
    case CGAMEEFFECT_REPLACEITEM:
        return new CGameEffectReplaceItem(effect, source, sourceID, target);
    case CGAMEEFFECT_DISABLEBUTTON:
        return new CGameEffectDisableButton(effect, source, sourceID, target);
    case CGAMEEFFECT_DISABLESPELLTYPE:
        return new CGameEffectDisableSpellType(effect, source, sourceID, target);
    case CGAMEEFFECT_CASTSPELL:
        return new CGameEffectCastSpell(effect, source, sourceID, target);
    case CGAMEEFFECT_LEARNSPELL:
        return new CGameEffectLearnSpell(effect, source, sourceID, target);
    case CGAMEEFFECT_CASTSPELLPOINT:
        return new CGameEffectCastSpellPoint(effect, source, sourceID, target);
    case CGAMEEFFECT_IDENTIFY:
        return new CGameEffectIdentify(effect, source, sourceID, target);
    case CGAMEEFFECT_DETECTTRAPS:
        return new CGameEffectDetectTraps(effect, source, sourceID, target);
    case CGAMEEFFECT_REPLACESELF:
        return new CGameEffectReplaceSelf(effect, source, sourceID, target);
    case CGAMEEFFECT_PLAYMOVIE:
        return new CGameEffectPlayMovie(effect, source, sourceID, target);
    case CGAMEEFFECT_SANCTUARY:
        return new CGameEffectSanctuary(effect, source, sourceID, target);
    case CGAMEEFFECT_ENTANGLE:
        return new CGameEffectEntangle(effect, source, sourceID, target);
    case 155:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 818
        UTIL_ASSERT_MSG(FALSE, "CGameEffectMinorGlobe game effect not supported in Icewind II (-rjf)");
    case 156:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 822
        UTIL_ASSERT_MSG(FALSE, "CGameEffectShieldGlobe game effect not supported in Icewind II (-rjf)");
    case CGAMEEFFECT_WEBHOLD:
        return new CGameEffectWebHold(effect, source, sourceID, target);
    case CGAMEEFFECT_GREASEPOOL:
        return new CGameEffectGreasePool(effect, source, sourceID, target);
    case CGAMEEFFECT_MIRRORIMAGERUN:
        return new CGameEffectMirrorImageRun(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPELSANCTUARY:
        return new CGameEffectDispelSanctuary(effect, source, sourceID, target);
    case CGAMEEFFECT_REMOVEPANIC:
        return new CGameEffectRemovePanic(effect, source, sourceID, target);
    case CGAMEEFFECT_REMOVEHOLD:
        return new CGameEffectRemoveHold(effect, source, sourceID, target);
    case CGAMEEFFECT_RESTOREMOVEMENTRATE:
        return new CGameEffectRestoreMovementRate(effect, source, sourceID, target);
    case CGAMEEFFECT_REMOVEINTOXICATION:
        return new CGameEffectRemoveIntoxication(effect, source, sourceID, target);
    case CGAMEEFFECT_PAUSECASTER:
        return new CGameEffectPauseCaster(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTANCETOMAGIC:
        return new CGameEffectResistanceToMagic(effect, source, sourceID, target);
    case CGAMEEFFECT_MISSILETHAC0BONUS:
        return new CGameEffectMissileTHAC0Bonus(effect, source, sourceID, target);
    case CGAMEEFFECT_DESTROYSELF:
        return new CGameEffectDestroySelf(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYTOPORTRAITICON:
        return new CGameEffectImmunityToPortraitIcon(effect, source, sourceID, target);
    case CGAMEEFFECT_DAMAGEVISUALEFFECT:
        return new CGameEffectDamageVisualEffect(effect, source, sourceID, target);
    case CGAMEEFFECT_ADDINNATEABILITY:
        return new CGameEffectAddInnateAbility(effect, source, sourceID, target);
    case CGAMEEFFECT_REMOVEINNATEABILITY:
        return new CGameEffectRemoveInnateAbility(effect, source, sourceID, target);
    case CGAMEEFFECT_RESISTANCETOPOISON:
        return new CGameEffectResistanceToPoison(effect, source, sourceID, target);
    case CGAMEEFFECT_PLAYSOUND:
        return new CGameEffectPlaySound(effect, source, sourceID, target);
    case CGAMEEFFECT_HOLDCREATURESPELL_175:
        return new CGameEffectHoldCreatureSpell(effect, source, sourceID, target);
    case CGAMEEFFECT_MOVEMENTRATE_176:
        return new CGameEffectMovementRate(effect, source, sourceID, target);
    case CGAMEEFFECT_APPLYEFFECT:
        return new CGameEffectApplyEffect(effect, source, sourceID, target);
    case CGAMEEFFECT_SELECTIVETOHITBONUS:
        return new CGameEffectSelectiveToHitBonus(effect, source, sourceID, target);
    case CGAMEEFFECT_SELECTDAMAGEBONUS:
        return new CGameEffectSelectiveDamageBonus(effect, source, sourceID, target);
    case CGAMEEFFECT_RESTRICTEQUIPITEM:
        return new CGameEffectRestrictEquipItem(effect, source, sourceID, target);
    case CGAMEEFFECT_RESTRICTEQUIPITEMTYPE:
        return new CGameEffectRestrictEquipItemType(effect, source, sourceID, target);
    case CGAMEEFFECT_APPLYEFFECTEQUIPITEM:
        return new CGameEffectApplyEffectEquipItem(effect, source, sourceID, target);
    case CGAMEEFFECT_APPLYEFFECTEQUIPITEMTYPE:
        return new CGameEffectApplyEffectEquipItemType(effect, source, sourceID, target);
    case CGAMEEFFECT_SETDONOJUMP:
        return new CGameEffectSetDoNoJump(effect, source, sourceID, target);
    case CGAMEEFFECT_HOLDCREATURESPELL_185:
        return new CGameEffectHoldCreatureSpell(effect, source, sourceID, target);
    case CGAMEEFFECT_JUMPTOAREA:
        return new CGameEffectJumpToArea(effect, source, sourceID, target);
    case CGAMEEFFECT_SETLOCALVARIABLE:
        return new CGameEffectSetLocalVariable(effect, source, sourceID, target);
    case CGAMEEFFECT_AURACLEANSING:
        return new CGameEffectAuraCleansing(effect, source, sourceID, target);
    case CGAMEEFFECT_MENTALSPEED:
        return new CGameEffectMentalSpeed(effect, source, sourceID, target);
    case CGAMEEFFECT_PHYSICALSPEED:
        return new CGameEffectPhysicalSpeed(effect, source, sourceID, target);
    case CGAMEEFFECT_CASTINGLEVELBONUS:
        return new CGameEffectCastingLevelBonus(effect, source, sourceID, target);
    case CGAMEEFFECT_FINDFAMILIAR:
        return new CGameEffectFindFamiliar(effect, source, sourceID, target);
    case CGAMEEFFECT_SEEINVISIBLE:
        return new CGameEffectSeeInvisible(effect, source, sourceID, target);
    case CGAMEEFFECT_IGNOREDIALOGPAUSE:
        return new CGameEffectIgnoreDialogPause(effect, source, sourceID, target);
    case CGAMEEFFECT_HITPOINTSONDEATH:
        return new CGameEffectHitPointsOnDeath(effect, source, sourceID, target);
    case CGAMEEFFECT_FAMILIARBLOCK:
        return new CGameEffectFamiliarBlock(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCEPROJECTILE:
        return new CGameEffectBounceProjectile(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCEEFFECT:
        return new CGameEffectBounceEffect(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCELEVEL:
        return new CGameEffectBounceLevel(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCELEVELDECREMENT:
        return new CGameEffectBounceLevelDecrement(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYLEVELDECREMENT:
        return new CGameEffectImmunityLevelDecrement(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCESCHOOL:
        return new CGameEffectBounceSchool(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCESECONDARYTYPE:
        return new CGameEffectBounceSecondaryType(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYSCHOOL:
        return new CGameEffectImmunitySchool(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYSECONDARYTYPE:
        return new CGameEffectImmunitySecondaryType(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYSPELL:
        return new CGameEffectImmunitySpell(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCESPELL:
        return new CGameEffectBounceSpell(effect, source, sourceID, target);
    case CGAMEEFFECT_MINHITPOINTS:
        return new CGameEffectMinHitPoints(effect, source, sourceID, target);
    case CGAMEEFFECT_POWERWORDKILL:
        return new CGameEffectPowerWordKill(effect, source, sourceID, target);
    case CGAMEEFFECT_POWERWORDSTUN:
        return new CGameEffectPowerWordStun(effect, source, sourceID, target);
    case CGAMEEFFECT_IMPRISONMENT:
        return new CGameEffectImprisonment(effect, source, sourceID, target);
    case CGAMEEFFECT_FREEDOM:
        return new CGameEffectFreedom(effect, source, sourceID, target);
    case CGAMEEFFECT_MAZE:
        return new CGameEffectMaze(effect, source, sourceID, target);
    case CGAMEEFFECT_SECONDARYCASTLIST:
        return new CGameEffectSecondaryCastList(effect, source, sourceID, target);
    case CGAMEEFFECT_VISUALEFFECT:
        return new CGameEffectVisualEffect(effect, source, sourceID, target);
    case CGAMEEFFECT_LEVELDRAIN:
        return new CGameEffectLevelDrain(effect, source, sourceID, target);
    case CGAMEEFFECT_POWERWORDSLEEP:
        return new CGameEffectPowerWordSleep(effect, source, sourceID, target);
    case CGAMEEFFECT_STONESKINS:
        return new CGameEffectStoneSkins(effect, source, sourceID, target);
    case CGAMEEFFECT_PROTECTIONCIRCLE:
        return new CGameEffectProtectionCircle(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPELSCHOOL:
        return new CGameEffectDispelSchool(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPELSECONDARYTYPE:
        return new CGameEffectDispelSecondaryType(effect, source, sourceID, target);
    case CGAMEEFFECT_RANDOMTELEPORT:
        return new CGameEffectRandomTeleport(effect, source, sourceID, target);
    case CGAMEEFFECT_IMMUNITYSCHOOLDECREMENT:
        return new CGameEffectImmunitySchoolDecrement(effect, source, sourceID, target);
    case CGAMEEFFECT_LEVELRESTORATION:
        return new CGameEffectLevelRestoration(effect, source, sourceID, target);
    case CGAMEEFFECT_REVEALMAGIC:
        return new CGameEffectRevealMagic(effect, source, sourceID, target);
    case CGAMEEFFECT_SECONDARYTYPEDECREMENT:
        return new CGameEffectSecondaryTypeDecrement(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCESCHOOLDECREMENT:
        return new CGameEffectBounceSchoolDecrement(effect, source, sourceID, target);
    case CGAMEEFFECT_BOUNCESECONDARYTYPEDECREMENT:
        return new CGameEffectBounceSecondaryTypeDecrement(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPELSCHOOLONE:
        return new CGameEffectDispelSchoolOne(effect, source, sourceID, target);
    case CGAMEEFFECT_DISPELSECONDARYTYPEONE:
        return new CGameEffectDispelSecondaryTypeOne(effect, source, sourceID, target);
    case CGAMEEFFECT_TIMESTOP:
        return new CGameEffectTimeStop(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_COLORGLOWDISSIPATE:
        return new IcewindCGameEffectColorGlowDissipate(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_VISUALSPELLHIT:
        return new CGameEffectVisualSpellHitIWD(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SNILLOCSSNOWBALLSWARMDAMAGE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1329
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSnillocsSnowballSwarmDamage game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_CASTINGGLOW:
        return new IcewindCGameEffectCastingGlow(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PANIC:
        return new CGameEffectPanic(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_MAGICALSTONEDAMAGE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1340
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectMagicalStoneDamage game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_SAVINGTHROWCHANGE:
        return new IcewindCGameEffectSavingThrowChange(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SLOWPOISON:
        return new IcewindCGameEffectSlowPoison(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SUMMONINGSPELL:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1350
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSummoningSpell game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_VAMPIRICTOUCH:
        return new IcewindCGameEffectVampiricTouch(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_CREATUREAFFECTEDANIMATION:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1357
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectCreatureAffectedAnimation game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_ANIMATEDEAD:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1361
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectAnimateDead game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_PRAYER:
        return new IcewindCGameEffectPrayer(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PRAYERBAD:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1368
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectPrayerBad game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_SUMMONINGSPELLMIXED:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1372
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSummoningSpellMixed game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_BELTYNSBURNINGBLOOD:
        return new IcewindCGameEffectBeltynsBurningBlood(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SHADOWMONSTERS:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1379
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectShadowMonsters game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_RECITATION:
        return new IcewindCGameEffectRecitation(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_RECITATIONBAD:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1386
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectRecitationBad game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_LICHTOUCH:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1390
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectLichTouch game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_SOLSSEARINGORB:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1394
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSolsSearingOrb game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_OILOFSERPENTSSCALES:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1398
        UTIL_ASSERT_MSG(FALSE, "Oil of the Serpent's Scales game effect not supported in Icewind II (-rjf)");
    case ICEWIND_CGAMEEFFECT_REMOVEEFFECTOFRESOURCE:
        return new IcewindCGameEffectRemoveEffectOfResource(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SALAMANDERAURA:
        return new IcewindCGameEffectSalamanderAura(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_UMBERHULKGAZE:
        return new IcewindCGameEffectUmberHulkGaze(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_ZOMBIELORDAURA:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1411
        UTIL_ASSERT_MSG(FALSE, "Zombie Lord Aura game effect not supported in Icewind II (-rjf)");
        (effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_IMMUNITYTORESOURCE:
        return new IcewindCGameEffectImmunityToResource(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SUMMONCREATURE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1418
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSummonCreature game effect not supported in Icewind II (-rjf)");
        (effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_ANIMATIONREMOVE:
        return new IcewindCGameEffectAnimationRemove(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_IMMUNITYTOEFFECTRESOURCE:
        return new IcewindCGameEffectImmunityToEffectAndResource(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SUMMONPOMABS:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1428
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSummonPomabs game effect not supported in Icewind II (-rjf)");
        (effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_DOMINATE:
        return new IcewindCGameEffectDominate(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_STATICCHARGE:
        return new IcewindCGameEffectStaticCharge(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_CLOAKOFFEAR:
        return new IcewindCGameEffectCloakOfFear(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FORCEMOVEMENTRATE:
        if (1) {
            IcewindCGameEffectForceMovementRate* pEffect = new IcewindCGameEffectForceMovementRate(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_FORCEMOVEMENTRATE);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_REMOVECONFUSION:
        return new IcewindCGameEffectRemoveConfusion(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEOFMIND:
        return new IcewindCGameEffectEyeOfMind(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEYOFSWORD:
        return new IcewindCGameEffectEyeOfSword(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEOFMAGE:
        return new IcewindCGameEffectEyeOfMage(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEOFVENOM:
        return new IcewindCGameEffectEyeOfVenom(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEOFSPIRIT:
        return new IcewindCGameEffectEyeOfSpirit(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEOFFORTITUDE:
        return new IcewindCGameEffectEyeOfFortitude(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EYEOFSTONE:
        return new IcewindCGameEffectEyeOfStone(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_REMOVESEVENEYES:
        return new IcewindCGameEffectRemoveSevenEyes(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_REMOVEALLOFTYPE:
        return new IcewindCGameEffectRemoveAllOfType(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SOULEATER:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1477
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectSoulEater game effect not supported in Icewind II (-rjf)");
        (effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SHROUDOFFLAME:
        return new IcewindCGameEffectShroudOfFlame(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_ANIMALRAGE:
        return new IcewindCGameEffectAnimalRage(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_TURNUNDEAD:
        return new IcewindCGameEffectTurnUndead(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_VITRIOLICSPHERE:
        return new IcewindCGameEffectVitriolicSphere(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SUPPRESSHPDISPLAY:
        return new IcewindCGameEffectSuppressHPDisplay(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FLOATTEXT:
        return new IcewindCGameEffectFloatText(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_MACEOFDISRUPTION:
        return new IcewindCGameEffectMaceOfDisruption(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SLEEP:
        if (1) {
            CGameEffectSleep* pEffect = new CGameEffectSleep(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_SLEEP);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_RANGERTRACKING:
        return new IcewindCGameEffectRangerTracking(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_IMMUNITYTOBACKSTAB:
        return new IcewindCGameEffectImmunityToBackstab(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SETSPELLSTATE:
        return new IcewindCGameEffectSetSpellState(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_DRAGONGEMCUTSCENE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1515
        UTIL_ASSERT_MSG(FALSE, "IcewindCGameEffectDragonGemCutscene game effect not supported in Icewind II (-rjf)");
        (effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_IMMUNITYSPELL:
        if (1) {
            CGameEffectImmunitySpell* pEffect = new CGameEffectImmunitySpell(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_IMMUNITYSPELL);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_RODOFSMITING:
        return new IcewindCGameEffectRodOfSmiting(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_MAGICALREST:
        return new IcewindCGameEffectMagicalRest(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BEHOLDERDISPELMAGIC:
        return new IcewindCGameEffectBeholderDispelMagic(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_HARPYWAIL:
        return new IcewindCGameEffectHarpyWail(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_JACKALWEREGAZE:
        return new IcewindCGameEffectJackalwereGaze(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SETGLOBAL:
        return new IcewindCGameEffectSetGlobal(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_HIDEINSHADOWS:
        return new CGameEffectHideInShadows(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SKILLUSEMAGICDEVICE:
        return new CGameEffectSkillUseMagicDevice(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_HOPELESSNESS:
        return new IcewindCGameEffectHopelessness(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PROTECTIONFROMEVIL:
        return new IcewindCGameEffectProtectionFromEvil(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_APPLYEFFECTSLIST:
        if (1) {
            IcewindCGameEffectApplyEffectsList* pEffect = new IcewindCGameEffectApplyEffectsList(effect, source, sourceID, target);
            pEffect->SetPermanentFlag(TRUE);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_ARMOROFFAITH:
        return new IcewindCGameEffectArmorOfFaith(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_NAUSEA:
        return new IcewindCGameEffectNausea(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_ENFEEBLEMENT:
        return new IcewindCGameEffectEnfeeblement(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FIRESHIELD:
        return new IcewindCGameEffectFireshield(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_DEATHWARD:
        return new IcewindCGameEffectDeathWard(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_HOLYPOWER:
        return new IcewindCGameEffectHolyPower(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_RIGHTEOUSWRATHOFFAITHFUL:
        return new IcewindCGameEffectRighteousWrathOfFaithful(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SUMMONALLY:
        if (1) {
            IcewindCGameEffectSummon* pEffect = new IcewindCGameEffectSummon(effect, source, sourceID, target);
            pEffect->SetSummonDelay(1);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_SUMMONENEMY:
        if (1) {
            IcewindCGameEffectSummon* pEffect = new IcewindCGameEffectSummon(effect, source, sourceID, target);
            pEffect->SetSummonDelay(2);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_CHARM:
        if (1) {
            CGameEffectCharm* pEffect = new CGameEffectCharm(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_CHARM);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_VISUALEFFECT:
        return new IcewindCGameEffectVisualEffect(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_OTILUKESRESILIENTSPHERE:
        return new IcewindCGameEffectOtilukesResilientSphere(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BARKSKIN:
        return new IcewindCGameEffectBarkskin(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BLEEDINGWOUNDS:
        return new IcewindCGameEffectBleedingWounds(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_AREAOFEFFECT:
        return new IcewindCGameEffectAreaOfEffect(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FREEACTION:
        return new IcewindCGameEffectFreeAction(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_UNCONSCIOUSNESS:
        return new IcewindCGameEffectUnconsciousness(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_MAGIC_DEATH:
        if (1) {
            CGameEffectDeath* pEffect = new CGameEffectDeath(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_MAGIC_DEATH);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_ENTROPYSHIELD:
        return new IcewindCGameEffectEntropyShield(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_STORMSHELL:
        return new IcewindCGameEffectStormShell(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PROTECTIONFROMELEMENTS:
        return new IcewindCGameEffectProtectionFromElements(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_HOLDUNDEAD:
        if (1) {
            CGameEffectHoldCreature* pEffect = new CGameEffectHoldCreature(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_HOLDUNDEAD);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_CONTROLUNDEAD:
        if (1) {
            CGameEffectCharm* pEffect = new CGameEffectCharm(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_CONTROLUNDEAD);
            pEffect->SetDWFlags(0x1);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_AEGIS:
        return new IcewindCGameEffectAegis(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_EXECUTIONERSEYES:
        return new IcewindCGameEffectExecutionersEyes(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BANISH:
        if (1) {
            CGameEffectDeath* pEffect = new CGameEffectDeath(effect, source, sourceID, target);
            pEffect->SetEffectID(ICEWIND_CGAMEEFFECT_MAGIC_DEATH);
            pEffect->SetDWFlags(0x800);
            return pEffect;
        }
    case ICEWIND_CGAMEEFFECT_WHENSTRUCK:
        return new IcewindCGameEffectWhenStruck(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PROJECTILE:
        return new IcewindCGameEffectProjectile(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_ENERGYDRAIN:
        return new IcewindCGameEffectEnergyDrain(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_TORTOISESHELL:
        return new IcewindCGameEffectTortoiseShell(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BLINK:
        return new IcewindCGameEffectBlink(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PERSISTANTEFFECT:
        return new IcewindCGameEffectPersistantEffect(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_DAYBLINDNESS:
        return new CGameEffectDayBlindness(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_DAMAGEREDUCTION:
        return new IcewindCGameEffectDamageReduction(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_DISGUISE:
        return new CGameEffectDisguise(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_HEROICINSPIRATION:
        return new CGameEffectHeroicInspiration(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PREVENTAISLOWDOWN:
        return new CGameEffectPreventAISlowdown(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BARBARIANRAGE:
        return new CGameEffectBarbarianRage(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_MOVEMENTRATEWITHPORTRAIT:
        return new CGameEffectMovementRateWithPortrait(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FEATCLEAVE:
        return new CGameEffectFeatCleave(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_PROTECTIONFORMARROWS:
        return new IcewindCGameEffectProtectionFromArrows(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_TENSERSTRANSFORMATION:
        return new IcewindCGameEffectTensersTransformation(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SLIPPERYMIND:
        return new CGameEffectSlipperyMind(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_SMITEEVIL:
        return new IcewindCGameEffectSmiteEvil(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_RESTORATION:
        return new IcewindCGameEffectRestoration(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_ALICORNLANCEGLOW:
        return new IcewindCGameEffectAlicornLanceGlow(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_CALLLIGHTNING:
        return new IcewindCGameEffectCallLightning(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_GLOBEOFINVULNERABILITY:
        return new IcewindCGameEffectGlobeOfInvulnerability(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_LOWERRESISTANCE:
        return new IcewindCGameEffectLowerResistance(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_BANE:
        return new IcewindCGameEffectBane(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FEATPOWERATTACK:
        return new IcewindCGameEffectFeatPowerAttack(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FEATEXPERTISE:
        return new IcewindCGameEffectFeatExpertise(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FEATARTERIALSTRIKE:
        return new IcewindCGameEffectFeatArterialStrike(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FEATHAMSTRING:
        return new IcewindCGameEffectFeatHamstring(effect, source, sourceID, target);
    case ICEWIND_CGAMEEFFECT_FEATRAPIDSHOT:
        return new IcewindCGameEffectFeatRapidShot(effect, source, sourceID, target);
    }

    CString sError;
    sError.Format("Unknown effect id:  %d", effect->effectID);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 1744
    UTIL_ASSERT_MSG(FALSE, (LPCSTR)sError);
}

// 0x452C80
CGameEffect::CGameEffect()
{
    m_sourceID = -1;
    m_done = FALSE;
    m_forceRepass = FALSE;
    field_118 = 0;
    m_compareIdOnly = FALSE;
    m_compareIdAndFlagsOnly = FALSE;
    m_secondaryType = 1;
    field_188 = 0;
}

// 0x48C310
CGameEffect::CGameEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target, BOOL usesDice)
{
    m_effectID = effect->effectID;
    m_targetType = effect->targetType;
    m_spellLevel = effect->spellLevel;
    m_effectAmount = effect->effectAmount;
    m_dwFlags = effect->dwFlags;

    if ((effect->durationType & 0x100) != 0) {
        m_flags |= 0x1;
    }

    if ((effect->durationType & 0x200) != 0) {
        m_flags |= 0x2;
    }

    m_durationType = effect->durationType & ~0x300;
    m_duration = effect->duration;
    m_probabilityUpper = effect->probabilityUpper;
    m_probabilityLower = effect->probabilityLower;

    m_res = effect->res;

    if (usesDice) {
        m_numDice = effect->numDice;
        m_diceSize = effect->diceSize;
    } else {
        m_minLevel = effect->diceSize;
        m_maxLevel = effect->numDice;
    }

    m_savingThrow = effect->savingThrow;
    m_saveMod = effect->saveMod;
    m_special = effect->special;

    m_source = source;
    m_sourceID = sourceID;
    m_target = target;

    // 0x48C310 ctor tail: the CGameEffect-level state is initialized here, not
    // by the (separate) default ctor.  field_118 latches the raw duration --
    // the delayed-effect state machine (ResolveEffect case 6) reads it back as
    // the post-delay duration after m_duration is converted to an absolute
    // deadline.  Missing m_done/m_forceRepass left uninitialized heap garbage
    // (0xCDCDCDCD) that made AddEffect delete freshly decoded effects.
    m_done = FALSE;
    m_compareIdOnly = FALSE;
    m_compareIdAndFlagsOnly = FALSE;
    m_forceRepass = FALSE;
    field_188 = 0;
    field_118 = m_duration;
    m_secondaryType = 1;
}

// 0x499B10
CGameEffect::~CGameEffect()
{
}

// 0x452E20
CGameEffect* CGameEffect::Copy()
{
    // __FILE__: .\Include\CGameEffect.h
    // __LINE__: 496
    UTIL_ASSERT(FALSE);
}

// 0x452E40
BOOL CGameEffect::ApplyEffect(CGameSprite* pSprite)
{
    // TODO: Incomplete.  Original base implementation asserts; many decoded
    // effect classes in the reconstruction still have no ApplyEffect override.
    // Treat those as no-op so effect-list processing can safely handle item
    // equipped effects while missing opcodes are recovered incrementally.
    return TRUE;
}

// 0x4A3030
//
// Duration-type state machine.  Types 3/4/5 are "apply after delay": the first
// resolve converts them to the waiting types 6/7/8 with m_duration holding the
// absolute game-time deadline, WITHOUT applying.  Once the deadline passes,
// they convert to their post-delay form (6 -> 0x1000 with the real duration
// taken from field_118, 7 -> 1, 8 -> 2) and apply.  Type 0 converts to an
// absolute-expiry 0x1000 on first resolve and applies; 0x1000 re-applies every
// pass until expiry, then fires OnRemove.  Types 1/2 apply straight through.
BOOL CGameEffect::ResolveEffect(CGameSprite* pSprite)
{
    if (pSprite->GetAIType().Equal(CAIObjectType::NOT_SPRITE)) {
        return FALSE;
    }

    switch (m_durationType) {
    case 0:
        m_durationType = 0x1000;
        m_duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + m_duration * 15;
        break;
    case 0x1000:
        if (CheckExpiration()) {
            m_done = TRUE;
            OnRemove(pSprite);
            return TRUE;
        }
        break;
    case 3:
        m_durationType = 6;
        m_duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + m_duration * 15;
        return TRUE;
    case 4:
        m_durationType = 7;
        m_duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + m_duration * 15;
        return TRUE;
    case 5:
        m_durationType = 8;
        m_duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + m_duration * 15;
        return TRUE;
    case 6:
        if (!CheckExpiration()) {
            return TRUE;
        }
        m_durationType = 0x1000;
        m_duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + field_118 * 15;
        FireSpell(pSprite);
        DisplayString(pSprite);
        break;
    case 7:
        if (!CheckExpiration()) {
            return TRUE;
        }
        m_durationType = 1;
        FireSpell(pSprite);
        DisplayString(pSprite);
        break;
    case 8:
        if (!CheckExpiration()) {
            return TRUE;
        }
        m_durationType = 2;
        FireSpell(pSprite);
        DisplayString(pSprite);
        break;
    default:
        break;
    }

    BOOL bResult = ApplyEffect(pSprite);
    m_secondaryType = 0;
    return bResult;
}

// 0x799E60
void CGameEffect::OnAdd(CGameSprite* pSprite)
{
    // Base no-op; subclasses override to react to effect application.
    //
    // The binary really is empty here: vtable slot 0x0010 of CGameEffect holds
    // 0x799E60, a bare `ret 4` shared by /OPT:ICF with every other one-argument
    // no-op virtual in the image. It is NOT 0x4A3FF0 -- that is FireSpell
    // below, whose marker this function had drifted above.
}

// 0x4A3FF0
//
// Fired when a delayed effect's deadline elapses (waiting types 6/7/8 in
// ResolveEffect).  When the effect carries source flag 0x400 and a valid
// m_sourceID, the original takes a share on the source object and posts
// CMessage feedback back to it.  Name from the BG2 PDB CGameEffect method
// list (FireSpell).
void CGameEffect::FireSpell(CGameSprite* pSprite)
{
    // 0x4A3FF0.  When a delayed effect (duration type 6/7/8) reaches its
    // deadline, the binary shares the source object (gate: m_sourceFlags &
    // 0x400 and valid m_sourceID), conditionally posts two
    // CMessageAddTrigger messages (vtable 0x847BB8, size 0x62) to the
    // source for AI script evaluation, then unconditionally calls
    // OnAdd(pSprite).
    //
    // The trigger messages carry CAITrigger objects built from the source's
    // AI type and globals at 0x847DD8 / 0x847D9C.  The first message is
    // gated on m_effectID == 12 (opcode 0x0C).  Both are delivered via
    // CMessageHandler::AddMessage.
    //
    // PARTIAL: trigger-message dispatch not yet recovered.  OnAdd is called
    // so subclasses (e.g. opcode-233 visual spell hit) re-apply.

    // Source feedback + trigger dispatch (0x4A4016–0x4A42CC).
    // Binary gates on (m_sourceFlags & 0x400) — relaxed here so the
    // trigger dispatch always fires when a source ID is present.
    if (m_sourceID != CGameObjectArray::INVALID_INDEX) {
        CGameObject* pSource;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // Binary 0x4A40CB–0x4A41B9: if m_effectID == 12 (Damage), post a
            // CMessageSetTrigger with trigger ID 0x0020 from global 0x847DD8.
            if (m_effectID == 12) {
                const CAIObjectType& aiType = static_cast<CGameAIBase*>(pSource)->GetAIType();
                CAITrigger trigger(0x0020, aiType, 0);
                CMessageSetTrigger* pMsg = new CMessageSetTrigger(
                    trigger, m_sourceID, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
            }

            // Binary 0x4A41B9–0x4A4247: always post a CMessageSetTrigger
            // with trigger ID 0x0002 from global 0x847D9C.
            {
                const CAIObjectType& aiType = static_cast<CGameAIBase*>(pSource)->GetAIType();
                CAITrigger trigger(0x0002, aiType, 0);
                CMessageSetTrigger* pMsg = new CMessageSetTrigger(
                    trigger, m_sourceID, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    // Unconditional: re-invoke OnAdd (vtable slot 4, 0x4A42CC–0x4A42D1).
    OnAdd(pSprite);
}

// 0x4A3310
//
// CGameEffect::CheckAdd -- the effect-immunity gate.  CGameSprite::AddEffect
// (0x733050) calls this on the incoming effect to decide whether it penetrates
// the target's protections: returns 1 to add/apply, 0 when the target is immune
// (or shatters).  Faithful __thiscall this == the effect; the five BYTE* params
// address the target's scratch bytes field_70F6..field_70FA, of which
// field_70F6..field_70F9 are forwarded to the effect's apply virtual and
// field_70FA carries the gate level byte.
//
// PARTIAL.  Recovered: the death-state / drain-to-death rejection gate, the
// probability gate, and the spell-immunity gate (the m_sourceRes resref lookup
// in the target's m_cImmunitiesSpell set -- the read side of the list
// CGameEffectImmunitySpell::ApplyEffect at 0x4BFE20 writes -- plus the seven
// CSevenEyes ward checks) and the source-notify trigger dispatch that
// accompanies a rejection (0x4A3619: the two CMessageSetTrigger posts to the
// caster, minus the once-only FUN_006F50A0 variable-set sub-block).  Still
// stubbed: the main admission tail
// (0x4A3BD3..0x4A3BA0) -- existing-effect dedup through the target effect list
// (sprite +0xDA4 via the inlined red-black-tree lookups at 0x4C4EF0 / 0x4C5AE0),
// spell-school / spell-level gating (CRuleTables::GetSpellAbilityValue, 0x547040),
// dispel feedback, and the counter-immunity spawn -- which run under SEH.
// Recovering that tail is a dedicated arc; this method is intentionally NOT wired
// into AddEffect (which remains the reconstructed minimum), so the "applies"
// default below changes no behaviour.
int CGameEffect::CheckAdd(CGameSprite* pSprite, BYTE* pField70F6, BYTE* pField70F7, BYTE* pField70F8, BYTE* pField70F9, BYTE* pField70FA)
{
    // Shatter-death gate (0x4A3341): when the target is in the petrify/freeze
    // death state (m_baseStats.m_flags & 0x20000) and the incoming effect is a
    // damage / death / animation opcode, the original shatters the creature and
    // blocks the effect.  Opcode list verified against the cascade at 0x4A3369.
    bool bReject = false;
    if ((pSprite->m_baseStats.m_flags & 0x20000) != 0) {
        // A source-flagged effect (m_sourceFlags & 0x400) also routes here.
        bReject = (m_sourceFlags & 0x400) != 0;
        switch (m_effectID) {
        case 0x3:   case 0x5:   case 0xc:   case 0xd:   case 0x18:  case 0x19:
        case 0x26:  case 0x27:  case 0x28:  case 0x2d:  case 0x37:  case 0x3a:
        case 0x3c:  case 0x48:  case 0x4a:  case 0x4c:  case 0x4e:  case 0x50:
        case 0x52:  case 0x6d:  case 0x6e:  case 0x80:  case 0x86:  case 0x87:
        case 0x88:  case 0x89:  case 0x9a:  case 0x9d:  case 0x9e:  case 0xa0:
        case 0xa5:  case 0xa8:  case 0xac:  case 0xb1:  case 0xf1:  case 0xf5:
        case 0xf7:  case 0xfa:  case 0xff:  case 0x100: case 0x101: case 0x107:
        case 0x108: case 0x115: case 0x116: case 0x117: case 0x118: case 0x119:
        case 0x11a: case 0x11c: case 0x11d: case 0x125: case 0x126: case 0x127:
        case 0x190: case 0x194: case 0x195: case 0x19c: case 0x19e: case 0x1a0:
        case 0x1a3: case 0x1a4: case 0x1a8: case 0x1a9: case 0x1ac: case 0x1af:
        case 0x1b3:
            bReject = true;
            break;
        default:
            break;
        }
    }

    // Drain-to-death opcodes route to the same source-notify block regardless of
    // the target's death state (0x4A35EC): negative m_effectAmount with opcode
    // 0x12 / 0x17, or positive m_effectAmount with 0x5D / 0x5E.
    if (!bReject) {
        LONG nAmount = static_cast<LONG>(m_effectAmount);
        bReject = (nAmount < 0 && (m_effectID == 0x12 || m_effectID == 0x17))
            || (nAmount > 0 && (m_effectID == 0x5d || m_effectID == 0x5e));
    }

    if (bReject) {
        // Source-notify block (0x4A3619): the effect is rejected; the caster is
        // shared and sent two CMessageSetTrigger notifications (trigger IDs 0x0020
        // and 0x0002 from globals 0x847DD8 / 0x847D9C, each carrying a CAITrigger
        // built from the caster's AI type), then released.
        if (m_sourceID != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pSource;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                // UNIMPLEMENTED: the once-only first-notify sub-block (0x4A367B)
                // -- sets the pSprite +0x58C flag | 0x200000 and calls the
                // thiscall FUN_006F50A0, gated on the caster's AI-type byte at
                // +4 against the threshold at 0x847C3B.
                {
                    CAITrigger trigger(0x0020,
                        static_cast<CGameAIBase*>(pSource)->GetAIType(), 0);
                    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
                        m_sourceID, pSprite->m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                }
                {
                    CAITrigger trigger(0x0002,
                        static_cast<CGameAIBase*>(pSource)->GetAIType(), 0);
                    CMessageSetTrigger* pMessage = new CMessageSetTrigger(trigger,
                        m_sourceID, pSprite->m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }
        return 0;
    }

    // Probability gate (0x4A38A6): the roll is the target's per-AI-tick d100
    // scratch byte (field_70FA), shared by every effect added on the same tick --
    // this is what makes exclusive probability ranges (e.g. the four Summon
    // Monster creature choices) pick exactly one.  rand() is called and discarded.
    rand();
    WORD roll = *pField70FA;
    if (roll > m_probabilityUpper || roll < m_probabilityLower) {
        return 0;
    }

    // Spell-immunity gate (0x4A38CA): the target resists when the incoming
    // effect's source spell (m_sourceRes) is on its spell-immunity list -- the
    // std::set<TString> that CGameEffectImmunitySpell::ApplyEffect (0x4BFE20)
    // populates -- or when one of the seven Eye wards intercepts.  Reject either.
    TString sSourceRes;
    sSourceRes = m_sourceRes.GetResRefStr();
    CSpellImmunitySet& cImmunities = pSprite->GetDerivedStats()->m_cImmunitiesSpell;
    if (cImmunities.find(sSourceRes) != cImmunities.end()
        || CSevenEyes::CheckEyeOfStone(pSprite, this)
        || CSevenEyes::CheckEyeOfSpirit(pSprite, this)
        || CSevenEyes::CheckEyeOfMind(pSprite, this)
        || CSevenEyes::CheckEyeOfFortitude(pSprite, this)
        || CSevenEyes::CheckEyeOfVenom(pSprite, this)
        || CSevenEyes::CheckEyeOfMage(pSprite, this)
        || CSevenEyes::CheckEyeOfSword(pSprite, this)) {
        return 0;
    }

    // SPWI420 special-resref pre-branch (0x4A39F0): before the normal admission,
    // a party member of class 7 (paladin) with the general-state bit 0x800 clear
    // intercepts the incoming effect when its source spell (m_sourceRes) is
    // "SPWI420" (Emotion: Fear).  It runs only for a
    // good-aligned target (AI enemy-ally at or below EA_GOODCUTOFF) that carries a
    // live source resref; any other case drops straight through to the admission
    // tail below.
    if (pSprite->GetAIType().m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF
        && m_sourceRes.GetResRef()[0] != '\0'
        && memcmp(m_sourceRes.GetResRef(), "SPWI420", 7) == 0) {

        // Party/state/class gate (0x4A3A3F): the block bites only on a party member
        // (owns a portrait), whose general-state bit 0x800 is clear -- the binary's
        // (~m_generalState >> 0xB) & 1 -- and who has at least two levels in class 7.
        CDerivedStats* pStats = pSprite->GetDerivedStats();
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->m_id)
                != CGameObjectArray::INVALID_INDEX
            && (((~pStats->m_generalState) >> 0xB) & 1) != 0
            && pStats->GetClassLevel(7) >= 2) {

            if (m_effectID == 0x18) {
                // UNIMPLEMENTED (0x4A3A8A): the "spell blocked" feedback.  The binary
                // copies m_sourceRes into a temp CResRef, requests the spell resource
                // (0x4699F0 with the resref built via CResRef::operator= 0x78AD40),
                // and on a live resource prints CSpell::GetGenericName through
                // CGameSprite::FeedBack(0x85C11A, ...), then tears the request down
                // (CResRef::operator!= 0x78AB40 / CRes::CancelRequest /
                // CDimm::ReleaseResObject).  None of that machinery is recoverable;
                // the reject below is preserved.
            }

            // Reject (0x4A3BA0).  The binary's tail-merged epilogue at 0x4A3B9B also
            // calls CGameObjectArray::ReleaseShare(m_sourceID, THREAD_ASYNCH, INFINITE)
            // here; it balances the single caster share the binary holds open across
            // this whole region.  This recovery manages the caster share locally in
            // each reject path instead (see the source-notify and conflict blocks), so
            // no share is held at this point -- emitting the release would unbalance
            // it, so it is intentionally omitted.
            return 0;
        }
    }

    // Main admission tail (0x4A3BD3).
    CDerivedStats* pStats = pSprite->GetDerivedStats();

    // Conflict lookup (0x4A3BD3): an effect with this opcode is already active on
    // the target when m_effectID is in m_activeEffectOpcodes.  The binary inlines
    // a std::set lower_bound (0x4C5AE0, mis-symbolled "CGameEffect::CheckAdd") and
    // confirms the exact match.
    CEffectOpcodeSet::iterator itActive =
        pStats->m_activeEffectOpcodes.lower_bound(static_cast<int>(m_effectID));
    bool bAlreadyActive = itActive != pStats->m_activeEffectOpcodes.end()
        && static_cast<DWORD>(*itActive) == m_effectID;

    if (!bAlreadyActive) {
        // No conflict.  When the effect carries a saving throw and a live source,
        // share the caster and fold its spell-ability save value into m_saveMod
        // before the save roll (0x4A3CE3).
        if (m_savingThrow != 0 && m_sourceID != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pSource;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                // m_casterLevel packs the save-ability descriptor: byte 1 = class,
                // byte 2 = specialisation index, byte 0 = caster level.  Only a
                // creature source with a class contributes (0x4A3D80).
                BYTE nClass = static_cast<BYTE>(m_casterLevel >> 8);
                if (nClass != 0 && pSource->GetObjectType() == '1') {
                    BYTE nSpecIndex = static_cast<BYTE>(m_casterLevel >> 16);
                    DWORD nSpecialization = 0;
                    if (nClass == 3) {
                        nSpecialization = g_pBaldurChitin->GetObjectGame()
                            ->GetRuleTables().GetSpecializationMask(3, nSpecIndex);
                    }
                    m_saveMod += CRuleTables::GetSpellAbilityValue(
                        static_cast<CGameSprite*>(pSource), m_sourceRes, m_savingThrow,
                        nClass, nSpecialization, static_cast<BYTE>(m_casterLevel));
                }
                // Source-resref fold (0x4A3E13): the effect only takes the Animal
                // Empathy save modifier (0x547620) when it came from Charm Animal
                // itself.  0x4A3DFD/0x4A3E0B compare m_sourceRes as two dwords against
                // the global CResRef at 0x8F8E60 / 0x8F8E64 -- that global is
                // CGameSprite::SPIN108 ("SPIN108", Charm Animal), the same one
                // CInfButtonArray builds the Animal Empathy button from.
                if (m_sourceRes == CGameSprite::SPIN108) {
                    m_saveMod += CRuleTables::GetAnimalEmpathySaveMod(1,
                        static_cast<CGameSprite*>(pSource), m_saveMod, m_spellLevel);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }

        // Saving-throw roll (0x4A3E4F): the effect's own save virtual decides
        // whether it is resisted.  It applies only if the save is not made and the
        // target does not already carry an effect-list immunity to it.
        if (CheckSave(pSprite, pField70F6, pField70F7, pField70F8, pField70F9)
            && !pStats->m_cImmunitiesEffect.OnList(this)) {

            // Spell-level immunity (0x4A3E94): a set per-level slot blocks the
            // effect, unless it is a level-1 innate / bard-song / shapeshift spell.
            bool bImmunityGatePassed;
            BYTE nLevel = static_cast<BYTE>(m_spellLevel);
            if (nLevel < 9 && pStats->m_cImmunitiesSpellLevel.m_levels[nLevel] != 0) {
                UINT nID = 0;
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                bImmunityGatePassed = m_spellLevel == 1
                    && (pGame->m_innateSpells.Find(m_res, nID)
                        || pGame->m_songs.Find(m_res, nID)
                        || pGame->m_shapeshifts.Find(m_res, nID));
            } else {
                bImmunityGatePassed = true;
            }

            if (bImmunityGatePassed) {
                BOOL bApply = UsesDice();
                if (!bApply) {
                    // Class-level range gate (0x4A3F13) for non-dice effects: the
                    // caster class level must fall within [m_minLevel, m_maxLevel].
                    if (m_maxLevel == 0
                        || pStats->GetClassMaskLevel(0xFFF) <= m_maxLevel) {
                        if (m_minLevel > 0xFFFF) {
                            m_minLevel = 0;
                        }
                        bApply = pStats->GetClassMaskLevel(0xFFF) >= m_minLevel;
                    }
                }
                if (bApply) {
                    // Effect admitted (0x4A3F7E).
                    if (m_durationType == 0x1000 || m_durationType == 0
                        || m_durationType == 1 || m_durationType == 2
                        || m_durationType == 9) {
                        CGameEffect::OnAdd(pSprite);
                        DisplayString(pSprite);
                    }
                    return 1;
                }
            }
        }
        return 0;
    }

    // Conflict (0x4A3C1F): an effect with this opcode is already present.  Share
    // the caster; if the incoming effect names a source resref, emit the "immune
    // to resource" feedback and add a counter spell-immunity effect -- carrying
    // the same caster level -- to the target.  Reject regardless.
    CGameObject* pSource;
    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
        CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        if (m_sourceRes.GetResRef()[0] != '\0') {
            CGameEffect::FeedBackImmuneToResource(pSprite, m_sourceRes);
            CGameEffect* pCounter = IcewindMisc::CreateEffectImmunitySpell(pSource,
                m_sourceRes.GetResRef(), 0, 0, 0, 2);
            pCounter->m_casterLevel = m_casterLevel;
            pSprite->AddEffect(pCounter, 1, TRUE, TRUE);
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }
    return 0;
}

// 0x4A42F0
// The base saving-throw computation: a d20 save against m_savingThrow (the
// fortitude/reflex/will bitmask, bits 0x4/0x8/0x10) with DC = m_saveMod + 10 +
// m_spellLevel.  The four scratch bytes are pre-rolled d20/percentile values with a
// sticky 0x80 "consumed" flag: F6 = fortitude roll, F7 = reflex roll, F8 = will
// roll, F9 = the magic-resistance roll.  Returns non-zero when the save did NOT
// block the effect (the value CheckAdd's admission tail consumes).
int CGameEffect::CheckSave(CGameSprite* pSprite, BYTE* pField70F6, BYTE* pField70F7, BYTE* pField70F8, BYTE* pField70F9)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    BYTE nFortRoll = *pField70F6 & 0x7f;
    BYTE nReflexRoll = *pField70F7 & 0x7f;
    BYTE nWillRoll = *pField70F8 & 0x7f;
    BYTE nResistRoll = *pField70F9 & 0x7f;

    int nSaveTotal = 0;
    int nRoll = nResistRoll;
    BOOL bSaveRolled = FALSE;

    // Objects whose AI race id is 0xBE never save (0x4A4374).
    if (pSprite->GetAIType().m_nRace == (BYTE)0xBE) {
        return TRUE;
    }

    // Cutscene gate (0x4A4387): while a cutscene is running, only a self-targeted
    // effect, opcode 0x68, or a death effect (opcode 0x0D) with a non-default
    // death type may roll a save; everything else is blocked outright.
    if (pGame->m_gameSave.m_cutScene == 1) {
        if (pSprite->m_id == m_sourceID) {
            return TRUE;
        }
        if (m_effectID == 0x68) {
            return TRUE;
        }
        if (m_effectID != 0xd) {
            return FALSE;
        }
        if (static_cast<CGameEffectDeath*>(this)->m_deathType == 0) {
            return FALSE;
        }
    }

    // Petrified / frozen targets cannot be affected by opcode 0x19, or by opcode
    // 0x8E under dw-flags 6, while they carry stoneskins or mirror images (0x4A43C8).
    if (m_effectID == 0x19
        && (pSprite->m_derivedStats.m_nMirrorImages != 0
            || pSprite->m_derivedStats.m_nStoneSkins > 0)) {
        return FALSE;
    }
    if (m_effectID == 0x8e && m_dwFlags == 6
        && (pSprite->m_derivedStats.m_nMirrorImages != 0
            || pSprite->m_derivedStats.m_nStoneSkins > 0)) {
        return FALSE;
    }

    // Magic-resistance gate (0x4A4414): dispellable source effects (m_sourceFlags
    // flagged 0x400, m_flags bit 0 set and bit 1 clear) roll against the target's
    // magic resistance -- halved for opcode 0x1C3.  A resist clears the low duration
    // bits, marks the roll consumed and blocks the effect.
    if ((m_sourceFlags & 0x400) != 0
        && (m_flags & 2) == 0
        && (m_flags & 1) != 0) {
        SHORT nResist = pSprite->m_derivedStats.m_nResistMagic;
        if (m_effectID == 0x1c3) {
            nResist = (SHORT)(nResist / 2);
        }
        // UNIMPLEMENTED (0x4A445A): the magic-resistance feedback -- a CGameSprite::
        // FeedBack floating number and, on a resist, the "resisted" TLK message
        // (0x4B18) assembled through the CSound/CString helpers at 0x5864F0 /
        // 0x7FC98F / 0x7FCED7 / 0x7FA83C.  Only the display is dropped.
        if ((int)(BYTE)m_casterLevel + (int)m_firstCall + (int)nResistRoll < (int)nResist) {
            m_dwFlags &= ~3u;
            *pField70F9 |= 0x80;
            return FALSE;
        }
    }

    // No save-type bitmask set -> the effect always lands (0x4A4642).
    if (m_savingThrow == 0) {
        return TRUE;
    }

    int nSaveDC = (int)m_saveMod + 10 + (int)m_spellLevel;

    // Fortitude save (0x4A4653): bitmask bit 0x4.
    if ((m_savingThrow & 4) != 0) {
        nRoll = nFortRoll;
        nSaveTotal = pSprite->m_derivedStats.m_nSaveVSFortitude + nFortRoll;
        if (m_effectID == 0x19) {
            if (pSprite->GetAIType().m_nRace == 4) {
                nSaveTotal += 2;
            }
            if (pSprite->HasFeat(0x3b)) {
                nSaveTotal += 2;
            }
            if (pSprite->HasFeat(0x32)) {
                nSaveTotal += 4;
            }
        }
        if ((*pField70F6 & 0x80) == 0) {
            *pField70F6 |= 0x80;
            bSaveRolled = TRUE;
        }
    }

    // Reflex save (0x4A46E2): bitmask bit 0x8.  When the effect's source object is a
    // trap ('A'), the target's trap-sense bonus is folded in.
    if ((m_savingThrow & 8) != 0) {
        nRoll = nReflexRoll;
        int nTrapSense = 0;
        if (m_sourceID != CGameObjectArray::INVALID_INDEX) {
            CGameObject* pSource;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH, &pSource, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if (pSource->GetObjectType() == 'A') {
                    nTrapSense = g_pBaldurChitin->GetObjectGame()->GetRuleTables()
                        .GetTrapSenseBonus(*pSprite->GetDerivedStats());
                }
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }
        nSaveTotal = pSprite->m_derivedStats.m_nSaveVSReflex + nReflexRoll + nTrapSense;
        if ((*pField70F7 & 0x80) == 0) {
            *pField70F7 |= 0x80;
            bSaveRolled = TRUE;
        }
    }

    // Will save (0x4A47B4): bitmask bit 0x10.
    if ((m_savingThrow & 0x10) != 0) {
        nRoll = nWillRoll;
        nSaveTotal = pSprite->m_derivedStats.m_nSaveVSWill + nWillRoll;

        // UNIMPLEMENTED (0x4A47EA): the SPWI420 (Emotion: Fear) paladin aura of
        // courage.  When the source spell is SPWI420 the original scans the party
        // for a nearby paladin -- GetClassLevel(7) >= 2, general-state bit 0x800
        // clear, within squared distance 0x2711 of the target -- and, if one is
        // found, adds +4 to the will save.  (class 7 = PALADIN in CLASS.IDS; the
        // +4-vs-fear ally aura is the D&D 3e Aura of Courage.)  Same special-cased
        // resref CheckAdd's pre-branch (0x4A3B90) leaves unrecovered; dropped here
        // for parity.

        if (m_effectID == 0x18 && pSprite->GetAIType().m_nRace == 5) {
            nSaveTotal += 2;
        }
        if (pSprite->m_typeAI.m_nRace == 2 && pSprite->m_typeAI.m_nSubRace == 1) {
            nSaveTotal += 2;
        }
        if ((*pField70F8 & 0x80) == 0) {
            *pField70F8 |= 0x80;
            bSaveRolled = TRUE;
        }
    }

    // Shared modifiers applied to whichever save was rolled (0x4A498B).
    if ((pSprite->m_baseStats.m_specFlags & CRE_SPECFLAG_NIGHTMARE_BOOST) != 0) {
        nSaveTotal += 5;
    }
    if (pSprite->GetAIType().m_nEnemyAlly != CAIObjectType::EA_PC) {
        // Difficulty-based save adjustment for non-PCs (0x4A49B0).  The binary reads
        // the difficulty at pGame+0x4456, which is m_cOptions (+0x43EA) plus
        // CGameOptions::m_nDifficultyLevel (+0x6C), and adds one of two read-only
        // .data constants: 0x84EB04 = -4 on difficulty 1, 0x84EB08 = -2 on 2.
        // Nothing in the image writes either, and this is their only reader.
        DWORD nDifficulty = pGame->m_cOptions.m_nDifficultyLevel;
        if (nDifficulty == 1) {
            nSaveTotal += -4;
        } else if (nDifficulty == 2) {
            nSaveTotal += -2;
        }
    }
    BYTE nRace = pSprite->GetAIType().m_nRace;
    if ((nRace == 2 || nRace == 3) && m_school == 4) {
        nSaveTotal += 2;
    }
    if (nRace == 6 && m_school == 5) {
        nSaveTotal += 2;
    }
    if ((m_flags & 2) == 0 && nRace == 4) {
        nSaveTotal += 2;
    }
    if (pSprite->GetClassLevel(6) >= 3 && m_school == 4) {
        nSaveTotal += 2;
    }
    if (m_school != 0) {
        BYTE nSchool = g_pBaldurChitin->GetObjectGame()->GetRuleTables()
            .MapCharacterSpecializationToSchool((WORD)pSprite->m_baseStats.m_specialization);
        if ((DWORD)nSchool == (DWORD)m_school) {
            nSaveTotal += 2;
        }
    }

    // Resolve the save (0x4A4A5E): a natural 20 always saves; otherwise the total
    // must reach the DC and not be a natural 1.
    BOOL bSaveMade;
    if (nRoll == 20) {
        bSaveMade = TRUE;
    } else if (nSaveTotal >= nSaveDC && nRoll != 1) {
        bSaveMade = TRUE;
    } else {
        bSaveMade = FALSE;
    }

    if (bSaveRolled) {
        // UNIMPLEMENTED (0x4A4A86): the save-result feedback -- FUN_00765830 shows
        // the "saved / failed" floating text with the roll and margin, its name
        // string built by FUN_005864F0.  Both helpers are unrecovered.
    }

    return bSaveMade ? FALSE : TRUE;
}

// 0x799E60
void CGameEffect::OnLoad(CGameSprite* pSprite)
{
}

// 0x78E6E0
BOOL CGameEffect::UsesDice()
{
    return FALSE;
}

// 0x4A2E40
void CGameEffect::PlaySound(const CResRef& soundResRef, CGameSprite* pSprite)
{
    m_sound.Stop();

    CMessage* message = new CMessagePlaySoundRef(soundResRef,
        pSprite->GetId(),
        pSprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    m_sound.SetResRef(soundResRef, TRUE, TRUE);

    if (m_sound.m_nLooping == 0) {
        m_sound.SetFireForget(TRUE);
    }

    m_sound.SetChannel(14, reinterpret_cast<DWORD>(pSprite->GetArea()));
    m_sound.Play(pSprite->GetPos().x, pSprite->GetPos().y, 0, FALSE);
}

// 0x4A3270
BOOL CGameEffect::CheckExpiration()
{
    switch (m_durationType) {
    case 0:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 1990
        UTIL_ASSERT(FALSE);
    case 6:
    case 7:
    case 8:
    case 0x1000:
        return g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime >= m_duration;
    }
    return FALSE;
}

// 0x4A4B00
ITEM_EFFECT* CGameEffect::GetItemEffect()
{
    ITEM_EFFECT* effect = new ITEM_EFFECT;
    effect->effectID = static_cast<WORD>(m_effectID);
    effect->targetType = static_cast<BYTE>(m_targetType);
    effect->spellLevel = static_cast<BYTE>(m_spellLevel);
    effect->effectAmount = m_effectAmount;
    effect->dwFlags = m_dwFlags;
    effect->durationType = static_cast<WORD>(m_durationType);

    if ((m_flags & 0x1) != 0) {
        effect->durationType |= 0x100;
    }

    if ((m_flags & 0x2) != 0) {
        effect->durationType |= 0x200;
    }

    effect->duration = m_duration;
    effect->probabilityUpper = static_cast<BYTE>(m_probabilityUpper);
    effect->probabilityLower = static_cast<BYTE>(m_probabilityLower);
    m_res.GetResRef(effect->res);

    if (UsesDice()) {
        effect->numDice = m_numDice;
        effect->diceSize = m_diceSize;
    } else {
        effect->numDice = m_maxLevel;
        effect->diceSize = m_minLevel;
    }

    effect->savingThrow = m_savingThrow;
    effect->saveMod = m_saveMod;
    effect->special = m_special;

    return effect;
}

// 0x4A4BB0
void CGameEffect::DisplayString(CGameSprite* pSprite)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if ((pGame->GetOptions()->m_nEffectTextLevel & 0x8) != 0
        && pGame->m_bInLoadGame != TRUE
        && pGame->m_bInLoadArea != TRUE) {
        CString sEffectID;
        sEffectID.Format("%d", m_effectID);

        STRREF str = atol(pGame->GetRuleTables().m_tEffectText.GetAt(CRuleTables::STRREF_STR, sEffectID));
        if (str != -1) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(pSprite->GetNameRef(),
                str,
                CVidPalette::RANGE_COLORS[pSprite->GetBaseStats()->m_colors[2]],
                RGB(0xD7, 0xD7, 0xBE),
                -1,
                pSprite->GetId(),
                pSprite->GetId());
        }
    }
}

// 0x4A51D0
void CGameEffect::OnRemove(CGameSprite* pSprite)
{
}

// 0x4A4D10
void CGameEffect::DisplayStringRef(CGameSprite* pSprite, STRREF str)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if ((pGame->GetOptions()->m_nEffectTextLevel & 0x8) != 0
        && pGame->m_bInLoadGame != TRUE
        && pGame->m_bInLoadArea != TRUE) {
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(pSprite->GetNameRef(),
            str,
            CVidPalette::RANGE_COLORS[pSprite->GetBaseStats()->m_colors[2]],
            RGB(0xD7, 0xD7, 0xBE),
            -1,
            pSprite->GetId(),
            pSprite->GetId());
    }
}

// 0x4A4EF0
void CGameEffect::AddColorEffect(CGameSprite* pSprite, BYTE r, BYTE g, BYTE b, BYTE periodLength)
{
    pSprite->m_hasColorEffects = 1;

    COLORREF tintColor = RGB(r, g, b);

    // NOTE: Uninline.
    pSprite->GetAnimation()->SetColorEffectAll(1, tintColor, periodLength);

    CColorEffect* pColorEffect = new CColorEffect;
    pColorEffect->m_effectType = 1;
    pColorEffect->m_range = -1;
    pColorEffect->m_tintColor = tintColor;
    pColorEffect->m_periodLength = periodLength;
    pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
}

// 0x4A4FA0
void CGameEffect::AddColorEffect(CGameSprite* pSprite, BYTE r, BYTE g, BYTE b)
{
    AddColorEffect(pSprite, r, g, b, 1);
}

// 0x48C5B0
BOOL CGameEffect::Compare(const CGameEffect& other)
{
    return m_effectID == other.m_effectID
        && m_effectAmount == other.m_effectAmount
        && m_dwFlags == other.m_dwFlags
        && m_durationType == other.m_durationType
        && m_duration == other.m_duration
        && m_source == other.m_source
        && m_sourceID == other.m_sourceID
        && m_targetType == other.m_targetType
        && m_probabilityUpper == other.m_probabilityUpper
        && m_probabilityLower == other.m_probabilityLower
        && m_spellLevel == other.m_spellLevel;
}

// 0x4A4FC0
BOOL CGameEffect::Compare(const CGameEffect& other, BOOL bCompareSource)
{
    if (m_sourceRes.GetResRefStr().Compare(other.m_sourceRes.GetResRefStr()) != 0) {
        return FALSE;
    }

    if (m_effectID != other.m_effectID) {
        return FALSE;
    }

    if (m_effectAmount != other.m_effectAmount) {
        return FALSE;
    }

    if (m_dwFlags != other.m_dwFlags) {
        return FALSE;
    }

    if (m_durationType != other.m_durationType) {
        return FALSE;
    }

    if (m_duration != other.m_duration) {
        return FALSE;
    }

    if (m_source != other.m_source && bCompareSource) {
        return FALSE;
    }

    if (m_sourceID != other.m_sourceID && bCompareSource) {
        return FALSE;
    }

    if (m_targetType != other.m_targetType) {
        return FALSE;
    }

    if (m_probabilityUpper != other.m_probabilityUpper) {
        return FALSE;
    }

    if (m_probabilityLower != other.m_probabilityLower) {
        return FALSE;
    }

    return m_spellLevel == other.m_spellLevel;
}

// 0x48C670
void CGameEffect::CopyFromBase(CGameEffectBase* pBase)
{
    m_version = pBase->m_version;
    m_effectID = pBase->m_effectID;
    m_targetType = pBase->m_targetType;
    m_spellLevel = pBase->m_spellLevel;
    m_effectAmount = pBase->m_effectAmount;
    m_dwFlags = pBase->m_dwFlags;
    m_durationType = pBase->m_durationType;
    m_duration = pBase->m_duration;
    m_probabilityUpper = pBase->m_probabilityUpper;
    m_probabilityLower = pBase->m_probabilityLower;
    m_res = pBase->m_res;
    m_numDice = pBase->m_numDice;
    m_diceSize = pBase->m_diceSize;
    m_savingThrow = pBase->m_savingThrow;
    m_saveMod = pBase->m_saveMod;
    m_special = pBase->m_special;
    m_school = pBase->m_school;
    field_48 = pBase->field_48;
    m_minLevel = pBase->m_minLevel;
    m_maxLevel = pBase->m_maxLevel;
    m_flags = pBase->m_flags;
    m_effectAmount2 = pBase->m_effectAmount2;
    m_effectAmount3 = pBase->m_effectAmount3;
    m_effectAmount4 = pBase->m_effectAmount4;
    m_effectAmount5 = pBase->m_effectAmount5;
    m_res2 = pBase->m_res2;
    m_res3 = pBase->m_res3;
    m_source = pBase->m_source;
    m_target = pBase->m_target;
    m_sourceType = pBase->m_sourceType;
    m_sourceRes = pBase->m_sourceRes;
    m_sourceFlags = pBase->m_sourceFlags;
    m_projectileType = pBase->m_projectileType;
    m_slotNum = pBase->m_slotNum;
    memcpy(m_scriptName, pBase->m_scriptName, 32);
    m_casterLevel = pBase->m_casterLevel;
    m_firstCall = pBase->m_firstCall;
    m_secondaryType = pBase->m_secondaryType;
}

// 0x4933E0
void CGameEffect::SetEffectID(WORD effectID)
{
    m_effectID = effectID;
}

// 0x4933F0
void CGameEffect::SetDWFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

// 0x493400
void CGameEffect::SetPermanentFlag(BOOL a1)
{
    if (a1) {
        m_flags |= 0x2;
    } else {
        m_flags &= ~0x2;
    }
}

// 0x594020
void CGameEffect::SetEnabled(BOOL bEnabled)
{
    if (bEnabled) {
        m_flags |= 0x1;
    } else {
        m_flags &= ~0x1;
    }
}

// 0x594040
void CGameEffect::SetSource(const CPoint& pt)
{
    m_source = pt;
}

// 0x594060
void CGameEffect::SetSourceId(LONG sourceID)
{
    m_sourceID = sourceID;
}

// 0x4C3F30
void CGameEffect::SetMinNaturalAC(CGameSprite* pSprite, INT nValue)
{
    pSprite->GetDerivedStats()->m_nACNaturalBonus = max(pSprite->GetDerivedStats()->m_nACNaturalBonus, nValue);
}

// 0x4C3F60
void CGameEffect::AdjustResistMagic(CGameSprite* pSprite, SHORT nValue)
{
    pSprite->GetDerivedStats()->m_nResistMagic += nValue;
    if (pSprite->GetDerivedStats()->m_nResistMagic < 0) {
        pSprite->GetDerivedStats()->m_nResistMagic = 0;
    } else if (pSprite->GetDerivedStats()->m_nResistMagic > 50) {
        pSprite->GetDerivedStats()->m_nResistMagic = 50;
    }
}

// 0x4C3FA0
void CGameEffect::AdjustTHAC0(CGameSprite* pSprite, SHORT nValue)
{
    pSprite->GetDerivedStats()->m_nTHAC0 += nValue;
}

// NOTE: Convenience.
void CGameEffect::AdjustSTR(CGameSprite* pSprite, INT nValue)
{
    if ((nValue > 0 && nValue > pSprite->GetDerivedStats()->field_128)
        || (nValue < 0 && nValue < pSprite->GetDerivedStats()->field_128)) {
        pSprite->m_bonusStats.m_nSTR += static_cast<SHORT>(nValue) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_128);
        pSprite->GetDerivedStats()->field_128 = nValue;
    }
}

// 0x4C3FC0
void CGameEffect::AdjustDEX(CGameSprite* pSprite, INT nValue)
{
    if ((nValue > 0 && nValue > pSprite->GetDerivedStats()->field_12C)
        || (nValue < 0 && nValue < pSprite->GetDerivedStats()->field_12C)) {
        pSprite->m_bonusStats.m_nDEX += static_cast<SHORT>(nValue) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_12C);
        pSprite->GetDerivedStats()->field_12C = nValue;
    }
}

// NOTE: Convenience.
void CGameEffect::AdjustINT(CGameSprite* pSprite, INT nValue)
{
    if ((nValue > 0 && nValue > pSprite->GetDerivedStats()->field_134)
        || (nValue < 0 && nValue < pSprite->GetDerivedStats()->field_134)) {
        pSprite->m_bonusStats.m_nINT += static_cast<SHORT>(nValue) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_134);
        pSprite->GetDerivedStats()->field_134 = nValue;
    }
}

// NOTE: Convenience.
void CGameEffect::AdjustWIS(CGameSprite* pSprite, INT nValue)
{
    if ((nValue > 0 && nValue > pSprite->GetDerivedStats()->field_138)
        || (nValue < 0 && nValue < pSprite->GetDerivedStats()->field_138)) {
        pSprite->m_bonusStats.m_nWIS += static_cast<SHORT>(nValue) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_138);
        pSprite->GetDerivedStats()->field_138 = nValue;
    }
}

// NOTE: Convenience.
void CGameEffect::AdjustCON(CGameSprite* pSprite, INT nValue)
{
    if ((nValue > 0 && nValue > pSprite->GetDerivedStats()->field_130)
        || (nValue < 0 && nValue < pSprite->GetDerivedStats()->field_130)) {
        pSprite->m_bonusStats.m_nCON += static_cast<SHORT>(nValue) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_130);
        pSprite->GetDerivedStats()->field_130 = nValue;
    }
}

// 0x4C4000
void CGameEffect::Immobilize(CGameSprite* pSprite)
{
    // NOTE: Uninline.
    pSprite->GetAnimation()->SetMoveScale(0);

    if (m_secondaryType) {
        CMessageDropPath* pMessage = new CMessageDropPath(pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }
}

// 0x4C4080
void CGameEffect::AddSlowEffect(CGameSprite* pSprite)
{
    if ((pSprite->GetDerivedStats()->m_generalState & STATE_HASTED) != 0) {
        pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
            CGAMEEFFECT_HASTE,
            pSprite->GetEquipedEffectList()->GetPosCurrent(),
            -1);
        pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
            CGAMEEFFECT_HASTE,
            pSprite->GetTimedEffectList()->GetPosCurrent(),
            -1);
    }

    pSprite->GetDerivedStats()->m_generalState |= STATE_SLOWED;
    pSprite->GetDerivedStats()->m_nACDodgeBonus -= 2;
    pSprite->GetDerivedStats()->m_nTHAC0 -= 2;
    pSprite->GetDerivedStats()->m_nSaveVSReflex -= 2;
    pSprite->m_bonusStats.m_nDamageBonus -= 2;

    if (pSprite->GetDerivedStats()->m_nNumberOfAttacks > 2) {
        pSprite->GetDerivedStats()->m_nNumberOfAttacks -= 1;
    }

    if (pSprite->GetAnimation()->GetMoveScale() != 0) {
        pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() / 2);

        if (m_secondaryType != 0) {
            pSprite->DropPath();
        }
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 41);
}

// 0x4C4200
void CGameEffect::AddPanicEffect(CGameSprite* pSprite)
{
    pSprite->sub_761650();
    pSprite->GetDerivedStats()->m_generalState |= STATE_PANIC;

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 36);
}

// 0x4C4260
void CGameEffect::RemoveAllOfType(CGameSprite* pSprite, WORD effectID, LONG effectAmount)
{
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        effectID,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        effectAmount);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        effectID,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        effectAmount);
}

// 0x4C42A0
void CGameEffect::AddPortraitIcon(CGameSprite* pSprite, int icon)
{
    // NOTE: Uninline.
    pSprite->AddPortraitIcon(icon);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(pSprite->GetId()), 1);
}

// 0x4C42F0
void CGameEffect::SetSpellState(CGameSprite* pSprite, DWORD dwSpellState)
{
    pSprite->GetDerivedStats()->m_spellStates.set(dwSpellState);
}

// 0x4C4330
void CGameEffect::FeedBackImmuneToResource(CGameSprite* pSprite, const CResRef& resRef)
{
    if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(resRef, 1005, TRUE)) {
        CItem* pItem = new CItem(resRef, 1, 0, 0, 0, 0);
        pSprite->FeedBack(CGameSprite::FEEDBACK_IMMUNE_TO_RESOURCE,
            0,
            0,
            0,
            pItem->GetIdentifiedName(),
            0,
            0);
        delete pItem;
    } else if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(resRef, 1006, TRUE)) {
        CSpell* pSpell = new CSpell(resRef);
        pSprite->FeedBack(CGameSprite::FEEDBACK_IMMUNE_TO_RESOURCE,
            0,
            0,
            0,
            pSpell->GetGenericName(),
            0,
            0);
        delete pSpell;
    }
}

// 0x4A2E00
void CGameEffect::ClearItemEffect(ITEM_EFFECT* itemEffect, WORD newEffectId)
{
    itemEffect->effectID = newEffectId;
    itemEffect->targetType = 0;
    itemEffect->spellLevel = 0;
    itemEffect->effectAmount = 0;
    itemEffect->dwFlags = 0;
    itemEffect->durationType = 0;
    itemEffect->duration = 0;
    itemEffect->probabilityUpper = 100;
    itemEffect->probabilityLower = 0;
    itemEffect->numDice = 0;
    itemEffect->diceSize = 0;
    itemEffect->savingThrow = 0;
    itemEffect->saveMod = 0;
    itemEffect->special = 0;
}

// 0x4B8730
void CGameEffect::StripItemSlot(CGameSprite* pSprite, INT slotNum)
{
    CItem* pItem = pSprite->GetEquipment()->m_items[slotNum];
    if (pItem != NULL) {
        pSprite->GetEquipment()->m_items[slotNum] = NULL;
        pItem->Unequip(pSprite, slotNum, TRUE, FALSE);
        g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pItem);

        if (pSprite->InControl()
            && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            CMessage* message = new CMessageSpriteEquipment(pSprite,
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (pSprite->GetEquipment()->m_selectedWeapon == slotNum) {
            pSprite->EquipMostDamagingMelee();
        }
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAC::CGameEffectAC(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493420
CGameEffect* CGameEffectAC::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAC* copy = new CGameEffectAC(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A51E0
BOOL CGameEffectAC::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_nACDodgeBonus += static_cast<SHORT>(m_effectAmount);
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nACArmorBonus = max(pSprite->GetDerivedStats()->m_nACArmorBonus, static_cast<SHORT>(m_effectAmount));
        break;
    case 2:
        pSprite->GetDerivedStats()->m_nACNaturalBonus = max(pSprite->GetDerivedStats()->m_nACNaturalBonus, static_cast<SHORT>(m_effectAmount));
        break;
    case 3:
        pSprite->GetDerivedStats()->m_nACDeflectionBonus = max(pSprite->GetDerivedStats()->m_nACDeflectionBonus, static_cast<SHORT>(m_effectAmount));
        break;
    case 4:
        pSprite->GetDerivedStats()->m_nACCrushingMod += static_cast<SHORT>(m_effectAmount);
        break;
    case 5:
        pSprite->GetDerivedStats()->m_nACPiercingMod += static_cast<SHORT>(m_effectAmount);
        break;
    case 6:
        pSprite->GetDerivedStats()->m_nACSlashingMod += static_cast<SHORT>(m_effectAmount);
        break;
    case 7:
        pSprite->GetDerivedStats()->m_nACMissileMod += static_cast<SHORT>(m_effectAmount);
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAttackNo::CGameEffectAttackNo(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4934D0
CGameEffect* CGameEffectAttackNo::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAttackNo* copy = new CGameEffectAttackNo(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAwaken::CGameEffectAwaken(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493580
CGameEffect* CGameEffectAwaken::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAwaken* copy = new CGameEffectAwaken(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A5540
BOOL CGameEffectAwaken::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_SLEEPING;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_SLEEPING;

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_SLEEP,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_SLEEP,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        14);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        14);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBerserk::CGameEffectBerserk(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493630
CGameEffect* CGameEffectBerserk::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBerserk* copy = new CGameEffectBerserk(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4936E0
CGameEffectBarbarianRage::CGameEffectBarbarianRage(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493720
CGameEffect* CGameEffectBarbarianRage::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBarbarianRage* copy = new CGameEffectBarbarianRage(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A5BF0
BOOL CGameEffectBarbarianRage::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_disabledSpellTypes[0] = TRUE;
    pSprite->GetDerivedStats()->m_disabledSpellTypes[1] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[2] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[3] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[4] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[5] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[6] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[7] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[8] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[9] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[10] = TRUE;
    pSprite->GetDerivedStats()->m_disabledButtons[0] = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCalm::CGameEffectCalm(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4937D0
CGameEffect* CGameEffectCalm::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCalm* copy = new CGameEffectCalm(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A5E50
BOOL CGameEffectCalm::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_BERSERK;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_BERSERK;

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_BERSERK,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_BERSERK,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        4);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        4);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x493880
CGameEffectCharm::CGameEffectCharm(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4938C0
CGameEffect* CGameEffectCharm::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCharm* copy = new CGameEffectCharm(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A6200
void CGameEffectCharm::OnRemove(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_CHARMED;
}

// -----------------------------------------------------------------------------

// 0x493970
CGameEffectSlipperyMind::CGameEffectSlipperyMind(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4939B0
CGameEffect* CGameEffectSlipperyMind::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSlipperyMind* copy = new CGameEffectSlipperyMind(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCHR::CGameEffectCHR(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493A60
CGameEffect* CGameEffectCHR::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCHR* copy = new CGameEffectCHR(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectColorChange::CGameEffectColorChange(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493B10
CGameEffect* CGameEffectColorChange::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectColorChange* copy = new CGameEffectColorChange(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A6BE0
BOOL CGameEffectColorChange::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->m_hasColorRangeEffects = TRUE;

    if (m_slotNum == pSprite->GetWeaponSlot()
        && (m_dwFlags & 0xF0) == CGameAnimationType::RANGE_WEAPON) {
        m_dwFlags &= 0xF;
        m_dwFlags |= CGameAnimationType::RANGE_SHIELD;
    }

    if (m_dwFlags == 255) {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorRangeAll(static_cast<BYTE>(m_effectAmount));

        CColorRange* pColorRange = new CColorRange();
        pColorRange->m_range = -1;
        pColorRange->m_color = static_cast<BYTE>(m_effectAmount);
        pSprite->GetDerivedStats()->m_appliedColorRanges.AddTail(pColorRange);
    } else {
        CColorRange* pColorRange = new CColorRange();
        pColorRange->m_range = static_cast<BYTE>(m_dwFlags);
        pColorRange->m_color = static_cast<BYTE>(m_effectAmount);
        pSprite->GetDerivedStats()->m_appliedColorRanges.AddTail(pColorRange);

        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorRange(static_cast<BYTE>(m_dwFlags),
            static_cast<BYTE>(m_effectAmount));
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectColorGlowSolid::CGameEffectColorGlowSolid(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493BC0
CGameEffect* CGameEffectColorGlowSolid::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectColorGlowSolid* copy = new CGameEffectColorGlowSolid(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A6D30
BOOL CGameEffectColorGlowSolid::ApplyEffect(CGameSprite* pSprite)
{
    if (m_slotNum == pSprite->GetWeaponSlot()
        && (m_dwFlags & 0xF0) == CGameAnimationType::RANGE_WEAPON) {
        m_dwFlags &= 0xF;
        m_dwFlags |= CGameAnimationType::RANGE_SHIELD;
    }

    pSprite->m_hasColorEffects = TRUE;

    COLORREF tintColor = m_effectAmount >> 8;

    if (m_dwFlags == 255) {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorEffectAll(1, tintColor, 1);

        CColorEffect* pColorEffect = new CColorEffect();
        pColorEffect->m_effectType = 1;
        pColorEffect->m_range = -1;
        pColorEffect->m_tintColor = tintColor;
        pColorEffect->m_periodLength = 1;
        pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
    } else {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorEffect(1, static_cast<BYTE>(m_dwFlags), tintColor, 1);

        CColorEffect* pColorEffect = new CColorEffect();
        pColorEffect->m_effectType = 1;
        pColorEffect->m_range = static_cast<BYTE>(m_dwFlags);
        pColorEffect->m_tintColor = tintColor;
        pColorEffect->m_periodLength = 1;
        pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectColorGlowPulse::CGameEffectColorGlowPulse(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493C70
CGameEffect* CGameEffectColorGlowPulse::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectColorGlowPulse* copy = new CGameEffectColorGlowPulse(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A6E90
BOOL CGameEffectColorGlowPulse::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->m_hasColorEffects = TRUE;

    COLORREF tintColor = m_effectAmount >> 8;
    WORD range = m_dwFlags & 0xFFFF;
    WORD period = (m_dwFlags >> 16) & 0xFFFF;

    if (m_slotNum == pSprite->GetWeaponSlot()
        && (range & 0xF0) == CGameAnimationType::RANGE_WEAPON) {
        range &= 0xF;
        range |= CGameAnimationType::RANGE_SHIELD;
    }

    if (range == 255) {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorEffectAll(1, tintColor, static_cast<BYTE>(period));

        CColorEffect* pColorEffect = new CColorEffect();
        pColorEffect->m_effectType = 1;
        pColorEffect->m_range = -1;
        pColorEffect->m_tintColor = tintColor;
        pColorEffect->m_periodLength = static_cast<BYTE>(period);
        pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
    } else {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorEffect(1, static_cast<BYTE>(range), tintColor, static_cast<BYTE>(period));

        CColorEffect* pColorEffect = new CColorEffect();
        pColorEffect->m_effectType = 1;
        pColorEffect->m_range = static_cast<BYTE>(range);
        pColorEffect->m_tintColor = tintColor;
        pColorEffect->m_periodLength = static_cast<BYTE>(period);
        pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCON::CGameEffectCON(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493F30
CGameEffect* CGameEffectCON::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCON* copy = new CGameEffectCON(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A74B0
BOOL CGameEffectCON::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_CONBase = min(max(pSprite->GetBaseStats()->m_CONBase + static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Uninline.
            AdjustCON(pSprite, m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_CONBase = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nCON = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_CONBase = min(max(pSprite->GetBaseStats()->m_CONBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nCON = min(max(pSprite->GetBaseStats()->m_CONBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 4660
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCurePoison::CGameEffectCurePoison(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493FE0
CGameEffect* CGameEffectCurePoison::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCurePoison* copy = new CGameEffectCurePoison(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A76B0
BOOL CGameEffectCurePoison::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_POISON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_POISON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        6);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        6);

    pSprite->GetBaseStats()->m_generalState &= ~STATE_POISONED;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_POISONED;

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDamage::CGameEffectDamage()
{
    m_effectID = CGAMEEFFECT_DAMAGE;
    m_special = -1;
}

// NOTE: Inlined.
CGameEffectDamage::CGameEffectDamage(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
    if (m_special == 0) {
        m_special = -1;
    }
}

// 0x49FC40
BOOL CGameEffectDamage::UsesDice()
{
    return TRUE;
}

// 0x494090
CGameEffect* CGameEffectDamage::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDamage* copy = new CGameEffectDamage(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A7750
void CGameEffectDamage::WakeOnDamage(CGameSprite* pSprite)
{
    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) == 0) {
        if ((pSprite->GetBaseStats()->m_generalState & STATE_SLEEPING) != 0
            || (pSprite->GetDerivedStats()->m_generalState & STATE_SLEEPING) != 0) {
            if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_DOESNT_AWAKEN_ON_DAMAGE)) {
                pSprite->GetBaseStats()->m_generalState &= ~(STATE_SLEEPING | STATE_HELPLESS);
                pSprite->GetDerivedStats()->m_generalState &= ~(STATE_SLEEPING | STATE_HELPLESS);

                // NOTE: Uninline.
                RemoveAllOfType(pSprite, CGAMEEFFECT_SLEEP, -1);

                // NOTE: Uninline.
                RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON, 14);
            }
        }

        if ((pSprite->GetDerivedStats()->m_generalState & STATE_STUNNED) != 0) {
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_UNSTUN_ON_DAMAGE)) {
                pSprite->GetBaseStats()->m_generalState &= ~STATE_STUNNED;
                pSprite->GetDerivedStats()->m_generalState &= ~STATE_STUNNED;

                // NOTE: Uninline.
                RemoveAllOfType(pSprite, CGAMEEFFECT_STUN, -1);

                // NOTE: Uninline.
                RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON, 44);

                // NOTE: Uninline.
                RemoveAllOfType(pSprite, CGAMEEFFECT_HOLDCREATURE, -1);

                // NOTE: Uninline.
                RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON, 13);
            }
        }
    }
}

// 0x4A51D0
//
// Damage suppresses the generic per-effect text ("Damage Taken" from
// EFFECTTEXT.2DA): its own combat-log line is printed inside ApplyEffect
// (the vtable +0x28 virtual), after the corpse/immunity gates.  Without
// this override the base CGameEffect::DisplayString prints on every
// damage-over-time tick, including hits absorbed by a corpse.
void CGameEffectDamage::DisplayString(CGameSprite* pSprite)
{
}

// CGameEffectDamage::ApplyEffect -- opcode #12, the core combat-damage handler.
// Rolls the effect's dice, scales and reduces the total (difficulty, Tortoise
// Shell absorption, per-type resistances with the caster's +20% elemental
// feats), subtracts it from the target's hit points, and on a lethal result
// builds + applies the type-specific CGameEffectDeath.
//
// Remaining HACK stubs (each marked below): the saving-throw pass (0x448250),
// the armor hit-thud resolver (0x4A8DF0), the combat-log damage line (vtable
// +0x28, 0x4AA6D0), the Tortoise-Shell hit feedback (0x4AB0A0) and its
// "shell destroyed" log line, the stunning add-immunity check, and the
// chunked-death debris spawner (0x586220).
// 0x4A7900
BOOL CGameEffectDamage::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->m_hasColorEffects = TRUE;

    // m_dwFlags packs the damage descriptor: low word = how to apply it
    // (0/3 = subtract, 1 = set, 2 = percent), high word = damage-type bitmask
    // (DAMAGES.IDS << 16).
    DWORD mode = m_dwFlags & 0xFFFF;
    DWORD damageType = m_dwFlags & 0xFFFF0000;

    // Venom Immunity: druids of level 9 and up shrug off poison damage.
    if (damageType == 0x200000
        && pSprite->GetDerivedStats()->GetClassLevel(CAIOBJECTTYPE_C_DRUID) > 8) {
        m_done = TRUE;
        return TRUE;
    }

    SHORT nOldHitPoints = pSprite->GetBaseStats()->m_hitPoints;

    // Invulnerable animations (doors, statues, ...) take no damage at all.
    if (pSprite->GetAnimation()->IsInvulnerable()) {
        m_done = TRUE;
        return TRUE;
    }

    // No damage while a cutscene is running.
    if (g_pBaldurChitin->GetObjectGame()->m_gameSave.m_cutScene == TRUE) {
        m_done = TRUE;
        return TRUE;
    }

    // Blood and splatter fly away from the damage source: face the source
    // object if it still exists, the effect's source point otherwise.
    SHORT nBloodDirection;
    if (m_sourceID == CGameObjectArray::INVALID_INDEX) {
        nBloodDirection = pSprite->GetDirection(m_source);
    } else {
        CGameObject* pSource;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                &pSource,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            nBloodDirection = pSprite->GetDirection(pSource->GetPos());
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            nBloodDirection = 0;
        }
    }

    // Damaging a frozen or petrified corpse shatters it: queue the matching
    // death effect and stop -- the hit itself does nothing else.
    DWORD nGeneralState = pSprite->GetBaseStats()->m_generalState;
    if ((nGeneralState & STATE_FROZEN_DEATH) != 0
        || (nGeneralState & STATE_STONE_DEATH) != 0) {
        CGameEffectDeath* pDeath = new CGameEffectDeath();
        pDeath->m_deathType = 0;
        pDeath->m_source = m_source;
        pDeath->m_sourceID = m_sourceID;
        pDeath->m_dwFlags = (nGeneralState & STATE_FROZEN_DEATH) != 0 ? 0x40 : 0x80;
        pDeath->m_casterLevel = m_casterLevel;
        pSprite->AddEffect(pDeath, CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);

        m_done = TRUE;
        return TRUE;
    }

    // An ordinary corpse (or one already pulped below -9 HP) takes no further
    // damage -- this is what keeps area spells from re-killing the dead.
    if (pSprite->GetBaseStats()->m_hitPoints < -9
        || (nGeneralState & STATE_DEAD) != 0) {
        m_done = TRUE;
        return TRUE;
    }

    // --- roll the dice: total = base amount + sum of m_numDice d(m_diceSize) ---
    int total = 0;
    for (DWORD i = 0; i < m_numDice; i++) {
        total += CUtil::UtilRandInt(m_diceSize, -pSprite->m_tempStats.m_nLuck) + 1;
    }
    total += m_effectAmount;
    m_effectAmount = total;
    if (total < 1) {
        m_done = TRUE;
        return TRUE;
    }

    SHORT nSequenceBefore = pSprite->m_nSequence;

    // mode 3 deals half (rounded down, floored at 1).
    if (mode == 3) {
        total /= 2;
        if (total < 1) {
            total = 1;
        }
        m_effectAmount = total;
    }

    // --- scaling and reduction (subtractive modes only) ---
    if (mode == 0 || mode == 3) {
        // Party members take difficulty-scaled damage.  The extra-damage part
        // is skipped for friendly fire between two good-aligned sprites and
        // when the matching gameplay option is on; an easy-mode reduction
        // (negative multiplier) always applies.
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId()) != -1) {
            BOOL bExtraDamage = TRUE;

            CGameObject* pSource;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pSource,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if (pSource->GetObjectType() == CGameObject::TYPE_SPRITE
                    && IcewindMisc::IsGoodByEA(pSprite) == TRUE
                    && IcewindMisc::IsGoodByEA(static_cast<CGameSprite*>(pSource)) == TRUE) {
                    bExtraDamage = FALSE;
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            LONG nMultiplier;
            if (g_pChitin->cNetwork.m_nServiceProvider == 0) {
                nMultiplier = g_pBaldurChitin->GetObjectGame()->m_cOptions.m_nDifficultyMultiplier;
            } else {
                nMultiplier = g_pBaldurChitin->GetObjectGame()->m_cOptions.m_nMPDifficultyMultiplier;
            }

            if (g_pBaldurChitin->GetObjectGame()->m_cOptions.m_bSuppressExtraDifficultyDamage == TRUE) {
                bExtraDamage = FALSE;
            }

            if (nMultiplier < 1 || bExtraDamage) {
                m_effectAmount += m_effectAmount * nMultiplier / 100;
            }
        }

        // Tortoise Shell soaks the whole hit into its hit-point pool; the
        // lingering shell effects mirror the remaining pool so it survives a
        // save/load.
        if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_TORTOISE_SHELL)) {
            CDerivedStats* pDStats = pSprite->GetDerivedStats();
            pDStats->m_nTortoiseShellPoints -= m_effectAmount;

            POSITION pos = pSprite->GetTimedEffectList()->GetHeadPosition();
            while (pos != NULL) {
                CGameEffect* pEffect = pSprite->GetTimedEffectList()->GetNext(pos);
                if (pEffect->m_effectID == CGAMEEFFECT_TORTOISE_SHELL) {
                    pEffect->m_effectAmount = pDStats->m_nTortoiseShellPoints;
                }
            }

            pos = pSprite->GetEquipedEffectList()->GetHeadPosition();
            while (pos != NULL) {
                CGameEffect* pEffect = pSprite->GetEquipedEffectList()->GetNext(pos);
                if (pEffect->m_effectID == CGAMEEFFECT_TORTOISE_SHELL) {
                    pEffect->m_effectAmount = pDStats->m_nTortoiseShellPoints;
                }
            }

            // HACK: the shell-hit feedback (flash + sound, 0x4AB0A0) is
            // unrecovered -- replaces the call at 0x4A7FF1.

            if (pDStats->m_nTortoiseShellPoints < 1) {
                pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
                    CGAMEEFFECT_TORTOISE_SHELL,
                    pSprite->GetTimedEffectList()->GetPosCurrent(),
                    -1);
                pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
                    CGAMEEFFECT_TORTOISE_SHELL,
                    pSprite->GetEquipedEffectList()->GetPosCurrent(),
                    -1);

                // HACK: the "shell destroyed" combat-log line (GetNameRef
                // strref 0x42C8 + DisplayTextRef, gated on the effect-text
                // option) is unrecovered -- replaces 0x4A8C8A..0x4A8D4B.
            }

            WakeOnDamage(pSprite);

            m_done = TRUE;
            return TRUE;
        }

        LONG nAmountBeforeReduction = m_effectAmount;

        // Per-type resistance.  The elemental types first reward the caster's
        // +20% damage feat, then subtract the target's resistance, then apply
        // the defensive spell-state halvings.
        CGameObject* pFeatSource = NULL;
        BYTE rcSource;
        do {
            rcSource = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                &pFeatSource,
                INFINITE);
        } while (rcSource == CGameObjectArray::SHARED || rcSource == CGameObjectArray::DENIED);

        switch (damageType) {
        case 0x0: // CRUSHING
            m_effectAmount -= pSprite->m_tempStats.m_nResistCrushing;
            break;
        case 0x10000: // ACID
            if (rcSource == CGameObjectArray::SUCCESS
                && pFeatSource->GetObjectType() == CGameObject::TYPE_SPRITE
                && static_cast<CGameSprite*>(pFeatSource)->HasFeat(CGAMESPRITE_FEAT_AQUA_MORTIS) == 1) {
                m_effectAmount = static_cast<LONG>(m_effectAmount * 1.2);
            }
            m_effectAmount -= pSprite->m_tempStats.m_nResistAcid;
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_IRON_BODY)) {
                m_effectAmount /= 2;
            }
            break;
        case 0x20000: // COLD
            if (rcSource == CGameObjectArray::SUCCESS
                && pFeatSource->GetObjectType() == CGameObject::TYPE_SPRITE
                && static_cast<CGameSprite*>(pFeatSource)->HasFeat(CGAMESPRITE_FEAT_AEGIS_OF_RIME) == 1) {
                m_effectAmount = static_cast<LONG>(m_effectAmount * 1.2);
            }
            m_effectAmount -= pSprite->m_tempStats.m_nResistCold;
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_RED_FIRESHIELD)) {
                m_effectAmount /= 2;
            }
            break;
        case 0x40000: // ELECTRICITY
            if (rcSource == CGameObjectArray::SUCCESS
                && pFeatSource->GetObjectType() == CGameObject::TYPE_SPRITE
                && static_cast<CGameSprite*>(pFeatSource)->HasFeat(CGAMESPRITE_FEAT_SCION_OF_STORMS) == 1) {
                m_effectAmount = static_cast<LONG>(m_effectAmount * 1.2);
            }
            m_effectAmount -= pSprite->m_tempStats.m_nResistElectricity;
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_IRON_BODY)) {
                m_effectAmount = 0;
            }
            break;
        case 0x80000: // FIRE
            if (rcSource == CGameObjectArray::SUCCESS
                && pFeatSource->GetObjectType() == CGameObject::TYPE_SPRITE
                && static_cast<CGameSprite*>(pFeatSource)->HasFeat(CGAMESPRITE_FEAT_SPIRIT_OF_FLAME) == 1) {
                m_effectAmount = static_cast<LONG>(m_effectAmount * 1.2);
            }
            m_effectAmount -= pSprite->m_tempStats.m_nResistFire;
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_BLUE_FIRESHIELD)) {
                m_effectAmount /= 2;
            }
            if (pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_IRON_BODY)) {
                m_effectAmount /= 2;
            }
            break;
        case 0x100000: // PIERCING
            m_effectAmount -= pSprite->m_tempStats.m_nResistPiercing;
            break;
        case 0x200000: // POISON
            m_effectAmount -= pSprite->m_tempStats.m_nResistPoison;
            break;
        case 0x400000: // MAGIC
            m_effectAmount -= pSprite->m_tempStats.m_nMagicDamageResistance;
            break;
        case 0x1000000: // SLASHING
            m_effectAmount -= pSprite->m_tempStats.m_nResistSlashing;
            break;
        case 0x2000000: // MAGICFIRE (sic: the binary subtracts the missile and
                        // piercing resistances, not the magic-fire one)
            m_effectAmount -= pSprite->m_tempStats.m_nResistMissile;
            m_effectAmount -= pSprite->m_tempStats.m_nResistPiercing;
            break;
        case 0x4000000: // MAGICCOLD (sic: missile + crushing, as above)
            m_effectAmount -= pSprite->m_tempStats.m_nResistMissile;
            m_effectAmount -= pSprite->m_tempStats.m_nResistCrushing;
            break;
        default: // MISSILE and everything else: no reduction
            break;
        }

        if (rcSource == CGameObjectArray::SUCCESS) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (m_effectAmount < 0) {
            m_effectAmount = 0;
        }

        // Frail party members (max HP below 14) are never one-shot from full:
        // the hit is capped to max - 1.
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId()) != -1) {
            SHORT nMaxHitPoints = pSprite->m_tempStats.m_nMaxHitPoints;
            if (nMaxHitPoints < m_effectAmount && nMaxHitPoints < 14) {
                m_effectAmount = nMaxHitPoints - 1;
            }
        }

        // Damage reduction (Stoneskin, Protection From Arrows, ...): unless the
        // attack's enchantment waives it, the strongest applicable entry on the
        // target's derived stats rewrites the amount.
        if (static_cast<LONG>(m_special) != -1) {
            m_effectAmount = pSprite->GetDerivedStats()->ReduceDamage(m_effectAmount,
                static_cast<LONG>(m_special),
                damageType);
        }

        if (m_effectAmount < 0) {
            m_effectAmount = 0;
        }

        (void)nAmountBeforeReduction; // resisted-amount feed for the HACKed
                                      // combat-log line (vtable +0x28)
    }

    // The damage just taken is remembered on the target; the casting
    // Concentration check reads it back (DC 10 + damage + spell level).
    if (m_effectAmount > 0) {
        pSprite->m_nDamageTaken = m_effectAmount;
    }

    // Pain vocal (sound slot 6).  Per-tick poison damage only grunts once
    // every 100 AI updates so the bark does not spam.
    if (damageType == 0x200000) {
        if (pSprite->m_poisonSoundTimer < 1) {
            pSprite->m_poisonSoundTimer = 100;

            CMessagePlaySound* pPlaySound = new CMessagePlaySound(CGameSprite::SOUND_DAMAGE,
                TRUE,
                TRUE,
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pPlaySound, FALSE);
        }
    } else {
        CMessagePlaySound* pPlaySound = new CMessagePlaySound(CGameSprite::SOUND_DAMAGE,
            TRUE,
            TRUE,
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pPlaySound, FALSE);
    }

    // --- subtract from hit points (HACK: the per-effect vtable[0x28] virtual
    // is omitted; it prints the combat-log damage line with the resisted
    // amount (0x4AA6D0) and does not change HP -- replaces the calls at
    // 0x4A8398 / 0x4A83D1). ---
    switch (mode) {
    case 0:
    case 3:
        pSprite->GetBaseStats()->m_hitPoints -= static_cast<SHORT>(m_effectAmount);
        break;
    case 1: {
        SHORT maxHP = pSprite->GetDerivedStats()->m_nMaxHitPoints;
        pSprite->GetBaseStats()->m_hitPoints =
            (m_effectAmount < maxHP) ? static_cast<SHORT>(m_effectAmount) : maxHP;
        break;
    }
    case 2:
        pSprite->GetBaseStats()->m_hitPoints =
            static_cast<SHORT>(pSprite->GetBaseStats()->m_hitPoints * m_effectAmount / 100);
        break;
    }

    // Damage-locator feedback: the portrait blood flash and the on-sprite
    // damage marker both get armed for 0x80 ticks.
    pSprite->m_bBloodFlashOn = TRUE;
    pSprite->m_nBloodFlashAmount = 0x80;
    pSprite->m_nDamageLocatorTime = 0x80;

    // HACK: the armor hit-thud is unrecovered -- it picks a HIT_0xx variant
    // from the target's armor animation type -- replaces the call at 0x4A8420
    // (0x4A8DF0).

    // On-hit feedback keyed on the damage type: the elemental types pair a
    // fixed cue with the matching flash overlay (which also queues the
    // spell-hit VFX), the physical types splatter blood away from the source
    // instead, magic only clinks, everything else stays silent.  The overlay
    // burns for 3 ticks per hit point lost.
    switch (damageType) {
    case 0x10000: // ACID
        PlaySound(CResRef("EFF_M47"), pSprite);
        pSprite->StartSpriteEffect(CGameSprite::SPRITE_EFFECT_ACID,
            CGameSprite::SPRITE_EFFECT_INTENSITY_LOW,
            static_cast<BYTE>(3 * (nOldHitPoints - pSprite->GetBaseStats()->m_hitPoints)),
            TRUE);
        break;
    case 0x20000: // COLD
        PlaySound(CResRef("EFF_M46"), pSprite);
        pSprite->StartSpriteEffect(CGameSprite::SPRITE_EFFECT_COLD,
            CGameSprite::SPRITE_EFFECT_INTENSITY_LOW,
            static_cast<BYTE>(3 * (nOldHitPoints - pSprite->GetBaseStats()->m_hitPoints)),
            TRUE);
        break;
    case 0x40000: // ELECTRICITY
        PlaySound(CResRef("MISC_02C"), pSprite);
        pSprite->StartSpriteEffect(CGameSprite::SPRITE_EFFECT_ELECTRICITY,
            CGameSprite::SPRITE_EFFECT_INTENSITY_LOW,
            static_cast<BYTE>(3 * (nOldHitPoints - pSprite->GetBaseStats()->m_hitPoints)),
            TRUE);
        break;
    case 0x80000: // FIRE
        PlaySound(CResRef("MISC_01C"), pSprite);
        pSprite->StartSpriteEffect(CGameSprite::SPRITE_EFFECT_FIRE,
            CGameSprite::SPRITE_EFFECT_INTENSITY_LOW,
            static_cast<BYTE>(3 * (nOldHitPoints - pSprite->GetBaseStats()->m_hitPoints)),
            TRUE);
        break;
    case 0x400000: // MAGIC
        PlaySound(CResRef("HIT_07"), pSprite);
        break;
    case 0x0:       // CRUSHING
    case 0x100000:  // PIERCING
    case 0x1000000: // SLASHING
    case 0x2000000: // MAGICFIRE (sic -- the original bleeds these two)
    case 0x4000000: // MAGICCOLD (sic)
        pSprite->AddBlood(0,
            nBloodDirection,
            nOldHitPoints - pSprite->GetBaseStats()->m_hitPoints < 6
                ? CBlood::GUSH_LOW
                : CBlood::GUSH_MEDIUM);
        break;
    default:
        break;
    }

    // Flinch unless the pre-damage animation already shows pain or worse.
    if (nSequenceBefore != CGameSprite::SEQ_DAMAGE
        && nSequenceBefore != CGameSprite::SEQ_DIE
        && nSequenceBefore != CGameSprite::SEQ_SLEEP
        && nSequenceBefore != CGameSprite::SEQ_TWITCH) {
        pSprite->SetSequence(CGameSprite::SEQ_DAMAGE);
    }

    // A minimum-hit-points effect (e.g. scripted plot armor) floors the result.
    if (pSprite->GetDerivedStats()->m_nMinHitPoints > 0
        && pSprite->GetBaseStats()->m_hitPoints < 1) {
        pSprite->GetBaseStats()->m_hitPoints =
            static_cast<SHORT>(pSprite->GetDerivedStats()->m_nMinHitPoints);
    }

    BOOL bSurvived = TRUE;
    if (pSprite->GetBaseStats()->m_hitPoints > 0) {
        // Survived.  Heroic Inspiration kicks in below half base hit points:
        // grant the lingering bonus effect once, with feedback on re-trigger.
        if (pSprite->HasFeat(CGAMESPRITE_FEAT_HEROIC_INSPIRATION) == 1
            && pSprite->GetBaseStats()->m_hitPoints
                <= static_cast<SHORT>(pSprite->GetBaseStats()->m_maxHitPointsBase >> 1)) {
            BOOL bFound = FALSE;
            POSITION pos = pSprite->GetTimedEffectList()->GetHeadPosition();
            while (pos != NULL) {
                CGameEffect* pEffect = pSprite->GetTimedEffectList()->GetNext(pos);
                if (pEffect->m_effectID == CGAMEEFFECT_HEROIC_INSPIRATION) {
                    bFound = TRUE;
                    break;
                }
            }

            if (!bFound) {
                ITEM_EFFECT heroic;
                heroic.effectID = CGAMEEFFECT_HEROIC_INSPIRATION;
                heroic.targetType = 0;
                heroic.spellLevel = 0;
                heroic.effectAmount = 0;
                heroic.dwFlags = 0;
                heroic.durationType = 1;
                heroic.duration = 0;
                heroic.probabilityUpper = 100;
                heroic.probabilityLower = 0;
                heroic.numDice = 0;
                heroic.diceSize = 0;
                heroic.savingThrow = 0;
                heroic.saveMod = 0;
                heroic.special = 0;

                CGameEffect* pHeroic = DecodeEffect(&heroic,
                    pSprite->GetPos(),
                    -1,
                    CPoint(-1, -1));
                pHeroic->m_flags &= ~1;

                CMessageAddEffect* pMessage = new CMessageAddEffect(pHeroic,
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            } else {
                pSprite->FeedBack(80, 0, 0, 0, -1, 0, 0);
            }
        }
    } else {
        // Lethal: build the death effect, picking the death-animation kind
        // from the damage type and how far below zero the target ended up.
        bSurvived = FALSE;

        CGameEffectDeath* pDeath = new CGameEffectDeath();
        pDeath->m_deathType = 0;
        pDeath->m_source = m_source;
        pDeath->m_sourceID = m_sourceID;
        CGameEffect* pEffectToApply = pDeath;

        SHORT nNewHitPoints = pSprite->GetBaseStats()->m_hitPoints;
        DWORD nKindType = damageType;
        BOOL bStunned = FALSE;

        if (nKindType == 0x8000000) { // STUNNING
            if (IcewindMisc::IsUndead(pSprite) == TRUE) {
                // Undead cannot be knocked out: they die a crushing death.
                nKindType = 0;
            } else if (pSprite->GetDerivedStats()->m_activeEffectOpcodes.find(CGAMEEFFECT_SLEEP)
                != pSprite->GetDerivedStats()->m_activeEffectOpcodes.end()) {
                // Naturally sleep-immune targets cannot be knocked out either:
                // they die outright (the death kind stays at the constructor
                // default, as in the binary).
            } else {
                delete pDeath;

                ITEM_EFFECT sleep;
                sleep.effectID = CGAMEEFFECT_SLEEP;
                sleep.targetType = 0;
                sleep.spellLevel = 0;
                sleep.effectAmount = 0;
                sleep.dwFlags = 0;
                sleep.durationType = 0;
                sleep.duration = 15 - 15 * nNewHitPoints;
                sleep.probabilityUpper = 100;
                sleep.probabilityLower = 0;
                sleep.numDice = 0;
                sleep.diceSize = 0;
                sleep.savingThrow = 0;
                sleep.saveMod = 0;
                sleep.special = 0;

                pEffectToApply = DecodeEffect(&sleep,
                    CPoint(-1, -1),
                    -1,
                    CPoint(-1, -1));
                pSprite->GetBaseStats()->m_hitPoints = 1;
                bStunned = TRUE;
            }
        }

        if (!bStunned) {
            switch (nKindType) {
            case 0x0:       // CRUSHING
            case 0x100000:  // PIERCING
            case 0x1000000: // SLASHING
            {
                // Gibs below -10; player characters hold together until -30.
                BOOL bPlayerCharacter = pSprite->GetAIType().m_nEnemyAlly == 2;
                if ((bPlayerCharacter && nNewHitPoints <= -31)
                    || (!bPlayerCharacter && nNewHitPoints <= -11)) {
                    pDeath->m_dwFlags = 8;
                } else {
                    pDeath->m_dwFlags = 4;
                }
                break;
            }
            case 0x10000: // ACID: dissolve below -5
                pDeath->m_dwFlags = nNewHitPoints > -6 ? 4 : 0;
                break;
            case 0x20000: // COLD: freeze-shatter below -5
                pDeath->m_dwFlags = nNewHitPoints > -6 ? 4 : 0x20;
                break;
            case 0x40000: // ELECTRICITY: charred death below -5
                pDeath->m_dwFlags = nNewHitPoints > -6 ? 4 : 0x100;
                break;
            case 0x80000: // FIRE: burnt death below -5
                pDeath->m_dwFlags = nNewHitPoints < -5 ? 1 : 4;
                break;
            case 0x200000:   // POISON
            case 0x400000:   // MAGIC
            case 0x2000000:  // MAGICFIRE
            case 0x4000000:  // MAGICCOLD
            case 0x20000000:
            case 0x40000000:
                pDeath->m_dwFlags = 4;
                break;
            case 0x10000000: // always chunked
                pDeath->m_dwFlags = 8;
                break;
            default:
                UTIL_ASSERT_MSG(FALSE, "WIERD damage");
            }
        }

        pEffectToApply->m_casterLevel = m_casterLevel;
        pSprite->AddEffect(pEffectToApply, CGameAIBase::EFFECT_LIST_TIMED, TRUE, TRUE);

        // HACK: the chunked-death debris pass for damage type 0x10000000 (the
        // EffSE spawner 0x586220 + per-chunk effect re-targeting) is
        // unrecovered -- replaces 0x4A89E5..0x4A8C46.
    }

    // If the cursor tooltip currently points at this creature, refresh it so
    // the hit-point line updates.
    if (g_pBaldurChitin->m_pObjectCursor->m_nCurrentCursor == 101
        && pSprite->m_pArea->m_iPicked == pSprite->GetId()) {
        pSprite->SetCharacterToolTip(g_pBaldurChitin->m_pObjectCursor->field_A02);
    }

    // "Badly wounded" vocal + autopause when the hit drops the target through
    // the 30% mark (always), or while already below it (1-in-10 per hit).
    SHORT nMaxHitPoints = pSprite->m_tempStats.m_nMaxHitPoints;
    SHORT nHitPointsNow = pSprite->GetBaseStats()->m_hitPoints;
    if (30 < (nOldHitPoints * 100) / nMaxHitPoints
        && (nHitPointsNow * 100) / nMaxHitPoints < 30
        && 0 < (nHitPointsNow * 100) / nMaxHitPoints) {
        CMessagePlaySound* pPlaySound = new CMessagePlaySound(CGameSprite::SOUND_BADLY_WOUNDED,
            TRUE,
            FALSE,
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pPlaySound, FALSE);
        pSprite->AutoPause(8);
    } else if ((nHitPointsNow * 100) / nMaxHitPoints < 30
        && 0 < (nHitPointsNow * 100) / nMaxHitPoints
        && rand() % 10 == 0) {
        CMessagePlaySound* pPlaySound = new CMessagePlaySound(CGameSprite::SOUND_BADLY_WOUNDED,
            TRUE,
            FALSE,
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pPlaySound, FALSE);
        pSprite->AutoPause(8);
    }

    if (bSurvived) {
        WakeOnDamage(pSprite);
    }

    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDeath::CGameEffectDeath()
{
    m_effectID = CGAMEEFFECT_DEATH;
    m_deathType = 1;
}

// NOTE: Inlined.
CGameEffectDeath::CGameEffectDeath(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
    m_deathType = 0;
}

// 0x494150
CGameEffectDeath::CGameEffectDeath(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target, int a5)
{
    m_deathType = a5;
}

// 0x452E70
CGameEffect* CGameEffectDeath::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDeath* copy = new CGameEffectDeath(effect, m_source, m_sourceID, m_target);
    copy->m_deathType = m_deathType;
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AB510
//
// CGameEffectDeath::ApplyEffect -- opcode #13, the death handler.  The damage
// opcode (and Slay, petrify, etc.) builds + applies this once a target's hit
// points reach <= 0; it is what actually transitions a creature to the dead
// state and plays its death animation.
//
// CORE RECOVERY.  Two faithful pieces are recovered:
//   * the on-death effect-list teardown (0x4AB884 + 0x4ABCE3): mark the death
//     animation running, then strip the effects that no longer make sense on a
//     corpse -- holds, portrait icons, disease/poison, and the grease/entangle/
//     web ground holds -- from both effect lists, and drop every remaining
//     non-permanent effect.  Each pass takes the list's GetPosCurrent() as
//     posLeave so the in-flight death effect (this) is preserved.
//   * the normal-death transition (the 0x4AC0DE case-4 path of the original's
//     death-type switch): zero the hit points, raise STATE_DEAD on both the base
//     and derived general-state words, and run the SEQ_DIE animation.  This is
//     what magic / generic damage produces and is enough for a slain creature to
//     fall.
//
// The rest of the original 0x4A7900-sized state machine is a documented stub
// pending its own arc:
//   * entry gates (0x4AB52A..0x4AB5B4): the animation-busy check and the global
//     no-death flag.
//   * the petrify/freeze SetSequence short-circuit (0x4AB5BA..0x4AB628).
//   * party bookkeeping (0x4AB62E..0x4AB882): the death reaction shout, the
//     six-slot party-wipe scan -> ReadyCharacterTerminationSequence (game over),
//     portrait / toolbar / selection cleanup, and the death feedback messages.
//   * the gore pipeline + death-type switch variants (0x4AB8EA..0x4ACD1F): blood
//     spatter, the seven gore-piece sub-objects, the burning/acid persistent
//     effects, and the gib / burnt / frozen / stone sequence + state-bit
//     selection (only the normal kind, case 4, is recovered above).
//   * the corpse teardown tail (0x4ACF71..0x4AD202): the dead AI-type set, the
//     death-variable bookkeeping, DropPath / DropSearchRequest, and the death
//     notification messages.  The XP award likewise lives here and is unrecovered.
// Replaces 0x4AB510.
BOOL CGameEffectDeath::ApplyEffect(CGameSprite* pSprite)
{
    // --- on-death effect-list teardown (0x4AB884) ---
    // The death animation is now running; strip the control / portrait-icon
    // effects that make no sense on a corpse from both effect lists.  posLeave
    // (GetPosCurrent) keeps this in-flight death effect off the removal sweep.
    pSprite->m_animationRunning = TRUE;
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_HOLDCREATURE,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_HOLDCREATURE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(), 13);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(), 13);

    // --- drop every remaining non-permanent effect (0x4ABCE3) ---
    pSprite->GetTimedEffectList()->RemoveAllEffectsIgnoreMoreThenPermanent(
        pSprite->GetTimedEffectList()->GetPosCurrent(), TRUE, FALSE, 0, 0);

    // 0x4ABCF0: a corpse is no longer poisoned or diseased.
    pSprite->GetBaseStats()->m_generalState &= ~(STATE_POISONED | STATE_DISEASED);
    pSprite->GetDerivedStats()->m_generalState &= ~(STATE_POISONED | STATE_DISEASED);

    // 0x4ABD2C: the lingering damage-over-time and portrait-icon effects.
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_DISEASE,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_DISEASE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(), -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_POISON,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_POISON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(), 6);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(), 6);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(), 7);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(), 7);

    // 0x4ABDCE: the grease / entangle / web ground holds.  The binary reuses the
    // timed list's iterator position as posLeave for all six passes, including
    // the equipped-list ones -- matched here.
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_GREASEPOOL,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_ENTANGLE,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_WEBHOLD,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_GREASEPOOL,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_ENTANGLE,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite, CGAMEEFFECT_WEBHOLD,
        pSprite->GetTimedEffectList()->GetPosCurrent(), -1);

    // --- normal-death transition (death-type switch case 4, 0x4AC0DE) ---
    pSprite->GetBaseStats()->m_hitPoints = 0;
    pSprite->GetBaseStats()->m_generalState |= STATE_DEAD;
    pSprite->GetDerivedStats()->m_generalState |= STATE_DEAD;
    pSprite->SetSequence(CGameSprite::SEQ_DIE);

    m_done = TRUE;
    return TRUE;
}

// 0x4ADC90
void CGameEffectDeath::DisplayString(CGameSprite* pSprite)
{
    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) == 0
        && (pSprite->GetBaseStats()->m_generalState & (STATE_STONE_DEATH | STATE_FROZEN_DEATH)) == 0
        && m_effectAmount != 1) {
        CGameEffect::DisplayString(pSprite);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDefrost::CGameEffectDefrost(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494190
CGameEffect* CGameEffectDefrost::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDefrost* copy = new CGameEffectDefrost(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4ADCC0
BOOL CGameEffectDefrost::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->GetDerivedStats()->m_generalState & STATE_FROZEN_DEATH) != 0) {
        pSprite->GetDerivedStats()->m_generalState &= ~STATE_FROZEN_DEATH;
        pSprite->ResetAITypeLive();
        pSprite->GetBaseStats()->m_hitPoints = 1;
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDEX::CGameEffectDEX(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494240
CGameEffect* CGameEffectDEX::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDEX* copy = new CGameEffectDEX(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4ADD10
BOOL CGameEffectDEX::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_DEXBase = min(max(pSprite->GetBaseStats()->m_DEXBase + static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Uninline.
            AdjustDEX(pSprite, m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_DEXBase = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nDEX = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_DEXBase = min(max(pSprite->GetBaseStats()->m_DEXBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nDEX = min(max(pSprite->GetBaseStats()->m_DEXBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_done = FALSE;
        }
        break;
    case 3:
        // Cat's Grace.
        if (pSprite->GetDerivedStats()->m_nDEX < 20) {
            if (m_effectAmount == 0) {
                m_effectAmount = rand() % 4 + 1;
            }

            pSprite->GetDerivedStats()->m_nDEX = min(pSprite->GetDerivedStats()->m_nDEX + static_cast<SHORT>(m_effectAmount), 20);

            if (m_secondaryType) {
                // NOTE: Uninline.
                DisplayStringRef(pSprite, 3798); // "Cat's Grace"
            }

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 75);

            pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_CATS_GRACE] = true;
        } else {
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 8038
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectHaste::CGameEffectHaste(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4942F0
CGameEffect* CGameEffectHaste::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHaste* copy = new CGameEffectHaste(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AE000
BOOL CGameEffectHaste::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->GetDerivedStats()->m_generalState & STATE_HASTED) == 0) {
        if (m_secondaryType != 0) {
            // NOTE: Uninline.
            DisplayStringRef(pSprite, 14023); // "Hasted"

            // NOTE: Probably should be outside of condition.
            if ((pSprite->GetDerivedStats()->m_generalState & STATE_SLOWED) != 0) {
                pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
                    CGAMEEFFECT_SLOW,
                    pSprite->GetEquipedEffectList()->GetPosCurrent(),
                    -1);
                pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
                    CGAMEEFFECT_SLOW,
                    pSprite->GetTimedEffectList()->GetPosCurrent(),
                    -1);
            }
        }

        pSprite->GetDerivedStats()->m_generalState |= STATE_HASTED;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 38);

        if (pSprite->GetDerivedStats()->m_nNumberOfAttacks < 5) {
            pSprite->GetDerivedStats()->m_nNumberOfAttacks += 1;
        }

        pSprite->m_derivedStats.m_nACDodgeBonus += 4;

        if (pSprite->GetAnimation()->GetMoveScale() != 0) {
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() * 2);

            if (m_secondaryType != 0) {
                pSprite->DropPath();
            }
        }

        AddColorEffect(pSprite, 64, 64, 64, 15);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectHeal::CGameEffectHeal()
{
    m_effectID = CGAMEEFFECT_HEAL;
}

// NOTE: Inlined.
CGameEffectHeal::CGameEffectHeal(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x4943A0
CGameEffect* CGameEffectHeal::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHeal* copy = new CGameEffectHeal(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectHitPoints::CGameEffectHitPoints(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x494450
CGameEffect* CGameEffectHitPoints::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHitPoints* copy = new CGameEffectHitPoints(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectINT::CGameEffectINT(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494500
CGameEffect* CGameEffectINT::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectINT* copy = new CGameEffectINT(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AEB10
BOOL CGameEffectINT::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_INTBase = min(max(pSprite->GetBaseStats()->m_INTBase + static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Uninline.
            AdjustINT(pSprite, m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_INTBase = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nINT = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_INTBase = min(max(pSprite->GetBaseStats()->m_INTBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nINT = min(max(pSprite->GetBaseStats()->m_INTBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 8713
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectInvisible::CGameEffectInvisible(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4945B0
CGameEffect* CGameEffectInvisible::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectInvisible* copy = new CGameEffectInvisible(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AED10
BOOL CGameEffectInvisible::ApplyEffect(CGameSprite* pSprite)
{
    // The invisibility variant is carried in m_dwFlags (the effect's param2):
    // 0/2 = plain Invisibility, 1 = Improved Invisibility.  The base-vs-derived
    // pass is selected by m_durationType (== 1 is the base-stats / equipped pass).
    // First pass over the effect emits the one-shot combat-feedback line;
    // DisplayStringRef self-gates on the effect-text option + load state.
    if (m_firstCall) {
        if (m_dwFlags == 0) {
            DisplayStringRef(pSprite, 14020); // "Invisible"
        } else if (m_dwFlags == 1) {
            DisplayStringRef(pSprite, 14783); // "Improved Invisibility"
        }
    }

    pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_INVISIBILITY] = true;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (m_dwFlags == 1) {
        // Improved invisibility (m_dwFlags 1).  m_durationType == 1 is the
        // base-stats / equipped pass; otherwise stamp the live derived state.
        if (m_durationType != 1) {
            pSprite->AddPortraitIcon(144);
            pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(pSprite->GetId()), 1);
            pSprite->GetDerivedStats()->m_generalState |= (STATE_INVISIBLE | STATE_IMPROVEDINVISIBILITY);
            if (IcewindMisc::IsEnemyNearby(pSprite)) {
                return TRUE;
            }
            // Clear the force-visible sanctuary bit CGameEffectForceVisible set.
            pSprite->GetBaseStats()->m_visible &= ~0x1;
            return TRUE;
        }
        pSprite->GetBaseStats()->m_generalState |= (STATE_INVISIBLE | STATE_IMPROVEDINVISIBILITY);
        m_done = TRUE;
        m_forceRepass = TRUE;
        return TRUE;
    }

    // m_dwFlags 0 and 2 share the plain-invisibility path; any other variant
    // is a no-op.
    if (m_dwFlags != 0 && m_dwFlags != 2) {
        return TRUE;
    }

    if (m_durationType != 1) {
        pSprite->AddPortraitIcon(110);
        pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(pSprite->GetId()), 1);
        pSprite->GetDerivedStats()->m_generalState |= STATE_INVISIBLE;
        return TRUE;
    }

    pSprite->GetBaseStats()->m_generalState |= STATE_INVISIBLE;
    m_done = TRUE;
    m_forceRepass = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectLore::CGameEffectLore(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494660
CGameEffect* CGameEffectLore::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectLore* copy = new CGameEffectLore(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectLuck::CGameEffectLuck(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4947C0
CGameEffect* CGameEffectLuck::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectLuck* copy = new CGameEffectLuck(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMorale::CGameEffectMorale(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494870
CGameEffect* CGameEffectMorale::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMorale* copy = new CGameEffectMorale(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AF420
BOOL CGameEffectMorale::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetBaseStats()->m_morale += static_cast<BYTE>(m_effectAmount);
        break;
    case 1:
        pSprite->GetBaseStats()->m_morale = static_cast<BYTE>(m_effectAmount);
        break;
    case 2:
        pSprite->GetBaseStats()->m_morale = pSprite->GetBaseStats()->m_morale * static_cast<BYTE>(m_effectAmount) / 100;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 9248
        UTIL_ASSERT(FALSE);
    }

    if (pSprite->GetBaseStats()->m_morale > 20) {
        pSprite->GetBaseStats()->m_morale = 20;
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPanic::CGameEffectPanic(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494960
CGameEffect* CGameEffectPanic::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPanic* copy = new CGameEffectPanic(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AF500
BOOL CGameEffectPanic::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14007);
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 36);

    if (m_dwFlags == 1) {
        pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS | STATE_STUNNED;
        pSprite->field_9D15 = 1;
    }

    pSprite->sub_761650();

    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_PANIC;
        m_forceRepass = TRUE;
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_generalState |= STATE_PANIC;
    }

    return TRUE;
}

// 0x4B5490
void CGameEffectPanic::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPoison::CGameEffectPoison(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494A10
CGameEffect* CGameEffectPoison::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPoison* copy = new CGameEffectPoison(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AF610
BOOL CGameEffectPoison::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_generalState |= STATE_POISONED;
    if (m_secondaryType != 0) {
        DisplayStringRef(pSprite, m_dwFlags == 8 ? 37696 : 14662);
    }
    AddPortraitIcon(pSprite, 6);

    if (m_secondaryType != 0 && m_dwFlags != 8) {
        m_special = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
    }
    DWORD duration = m_durationType == 1 ? 0 : m_duration;
    CPersistantEffect84C420* pEffect = NULL;
    switch (m_dwFlags) {
    case 2:
        pEffect = new CPersistantEffect84C420(0x200000, m_effectAmount, 15, m_special, duration);
        break;
    case 3:
        pEffect = new CPersistantEffect84C420(0x200000, 1, 15 * m_effectAmount, m_special, duration);
        break;
    case 6:
        if (m_secondaryType != 0) {
            DisplayStringRef(pSprite, 14102);
        }
        pSprite->field_9D15 = 1;
        AddPortraitIcon(pSprite, 13);
        pSprite->GetDerivedStats()->m_spellStates.set(7, true);
        pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;
        // This poison has the same damage cadence as mode four.
    case 4:
        pEffect = new CPersistantEffect84C420(0x200000, m_effectAmount, 105, m_special, duration);
        break;
    case 5:
        pEffect = new CPersistantEffect84C420(0x200000, m_effectAmount, 735, m_special, duration);
        break;
    case 7:
        pEffect = new CPersistantEffect84C420(0x200000, m_effectAmount, m_effectAmount2, m_special, duration);
        break;
    case 8:
        if (m_secondaryType != 0) {
            // The original initializes these fields only, as in OnRemove.
            ITEM_EFFECT effect;
            effect.effectID = CGAMEEFFECT_CON;
            effect.spellLevel = 10;
            effect.effectAmount = m_effectAmount;
            effect.probabilityUpper = 100;
            effect.probabilityLower = 0;
            effect.numDice = 0;
            effect.diceSize = 0;
            effect.saveMod = 0;
            effect.special = 0;
            effect.targetType = 2;
            effect.dwFlags = 0;
            effect.durationType = 0;
            effect.duration = 70;
            effect.savingThrow = 0;
            CGameEffect* pConstitution = DecodeEffect(&effect, CPoint(-1, -1), -1, CPoint(-1, -1));
            pConstitution->SetSource(pSprite->GetPos());
            pConstitution->SetSourceId(pSprite->GetId());
            pConstitution->m_flags &= ~1;
            g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessageAddEffect(pConstitution,
                pSprite->GetId(), pSprite->GetId()), FALSE);
        }
        return TRUE;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 9490
        UTIL_ASSERT(FALSE);
        return TRUE;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // The binary has an allocation assertion in each periodic-mode branch.
    UTIL_ASSERT(pEffect != NULL);
    if (pEffect != NULL) {
        pEffect->m_sourceID = m_sourceID;
        strcpy(reinterpret_cast<char*>(pEffect->field_29), m_sourceRes.GetResRefStr());
        pSprite->GetDerivedStats()->m_cRegeneratedPersistantEffectList.AddTail(pEffect);
    }
    return TRUE;
}

// 0x4AFBF0
void CGameEffectPoison::OnRemove(CGameSprite* pSprite)
{
    if (m_dwFlags == 8) {
        ITEM_EFFECT effect;
        effect.effectID = CGAMEEFFECT_CON;
        effect.spellLevel = 10;
        effect.effectAmount = m_effectAmount + m_special;
        effect.probabilityUpper = 100;
        effect.probabilityLower = 0;
        effect.numDice = 0;
        effect.diceSize = 0;
        effect.saveMod = 0;
        effect.special = 0;
        effect.targetType = 2;
        effect.dwFlags = 0;
        effect.durationType = 0;
        effect.duration = 70;
        effect.savingThrow = 4;

        CGameEffect* pEffect = DecodeEffect(&effect,
            CPoint(-1, -1),
            -1,
            CPoint(-1, -1));
        pEffect->SetSource(pSprite->GetPos());
        pEffect->SetSourceId(pSprite->GetId());
        pEffect->m_flags &= ~1;

        CMessage* message = new CMessageAddEffect(pEffect,
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRemoveCurse::CGameEffectRemoveCurse(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494AC0
CGameEffect* CGameEffectRemoveCurse::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRemoveCurse* copy = new CGameEffectRemoveCurse(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistAcid::CGameEffectResistAcid(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494B70
CGameEffect* CGameEffectResistAcid::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistAcid* copy = new CGameEffectResistAcid(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AFCF0
BOOL CGameEffectResistAcid::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistAcidBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistAcidBase > 100) {
                pSprite->GetBaseStats()->m_resistAcidBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistAcidBase < 0) {
                pSprite->GetBaseStats()->m_resistAcidBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistAcid += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistAcidBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistAcidBase > 100) {
                pSprite->GetBaseStats()->m_resistAcidBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistAcidBase < 0) {
                pSprite->GetBaseStats()->m_resistAcidBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistAcid < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistAcid = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistAcidBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistAcidBase / 100;
            if (pSprite->GetBaseStats()->m_resistAcidBase > 100) {
                pSprite->GetBaseStats()->m_resistAcidBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistAcidBase < 0) {
                pSprite->GetBaseStats()->m_resistAcidBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistAcid = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistAcidBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 9674
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistCold::CGameEffectResistCold(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494C20
CGameEffect* CGameEffectResistCold::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistCold* copy = new CGameEffectResistCold(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4AFEA0
BOOL CGameEffectResistCold::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistColdBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistColdBase > 100) {
                pSprite->GetBaseStats()->m_resistColdBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistColdBase < 0) {
                pSprite->GetBaseStats()->m_resistColdBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistCold += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistColdBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistColdBase > 100) {
                pSprite->GetBaseStats()->m_resistColdBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistColdBase < 0) {
                pSprite->GetBaseStats()->m_resistColdBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistCold < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistCold = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistColdBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistColdBase / 100;
            if (pSprite->GetBaseStats()->m_resistColdBase > 100) {
                pSprite->GetBaseStats()->m_resistColdBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistColdBase < 0) {
                pSprite->GetBaseStats()->m_resistColdBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistCold = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistColdBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 9805
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistElectricity::CGameEffectResistElectricity(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494CD0
CGameEffect* CGameEffectResistElectricity::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistElectricity* copy = new CGameEffectResistElectricity(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B0050
BOOL CGameEffectResistElectricity::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistElectricityBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistElectricityBase > 100) {
                pSprite->GetBaseStats()->m_resistElectricityBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistElectricityBase < 0) {
                pSprite->GetBaseStats()->m_resistElectricityBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistElectricity += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistElectricityBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistElectricityBase > 100) {
                pSprite->GetBaseStats()->m_resistElectricityBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistElectricityBase < 0) {
                pSprite->GetBaseStats()->m_resistElectricityBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistElectricity < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistElectricity = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistElectricityBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistElectricityBase / 100;
            if (pSprite->GetBaseStats()->m_resistElectricityBase > 100) {
                pSprite->GetBaseStats()->m_resistElectricityBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistElectricityBase < 0) {
                pSprite->GetBaseStats()->m_resistElectricityBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistElectricity = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistElectricityBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 9936
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistFire::CGameEffectResistFire(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494D80
CGameEffect* CGameEffectResistFire::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistFire* copy = new CGameEffectResistFire(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B0050
BOOL CGameEffectResistFire::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistFireBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistFireBase > 100) {
                pSprite->GetBaseStats()->m_resistFireBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistFireBase < 0) {
                pSprite->GetBaseStats()->m_resistFireBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistFire += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistFireBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistFireBase > 100) {
                pSprite->GetBaseStats()->m_resistFireBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistFireBase < 0) {
                pSprite->GetBaseStats()->m_resistFireBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistFire < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistFire = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistFireBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistFireBase / 100;
            if (pSprite->GetBaseStats()->m_resistFireBase > 100) {
                pSprite->GetBaseStats()->m_resistFireBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistFireBase < 0) {
                pSprite->GetBaseStats()->m_resistFireBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistFire = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistFireBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 10067
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistMagic::CGameEffectResistMagic(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494E30
CGameEffect* CGameEffectResistMagic::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistMagic* copy = new CGameEffectResistMagic(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B03B0
BOOL CGameEffectResistMagic::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_effectAmount >= 0 || pSprite->m_tempStats.m_nMagicDamageResistance != 100) {
            pSprite->m_bonusStats.m_nMagicDamageResistance += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (pSprite->GetDerivedStats()->m_nMagicDamageResistance < m_effectAmount) {
            pSprite->GetDerivedStats()->m_nMagicDamageResistance = static_cast<SHORT>(m_effectAmount);
        }
        m_done = FALSE;
        break;
    case 2:
        pSprite->GetDerivedStats()->m_nMagicDamageResistance = static_cast<SHORT>(m_effectAmount) * pSprite->GetDerivedStats()->m_nMagicDamageResistance / 100;
        m_done = FALSE;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 10154
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResurrect::CGameEffectResurrect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494EE0
CGameEffect* CGameEffectResurrect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResurrect* copy = new CGameEffectResurrect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSaveVsFortitude::CGameEffectSaveVsFortitude(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494F90
CGameEffect* CGameEffectSaveVsFortitude::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSaveVsFortitude* copy = new CGameEffectSaveVsFortitude(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B0D10
BOOL CGameEffectSaveVsFortitude::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSFortitudeBase += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_saveVSFortitudeBase > 40) {
                pSprite->GetBaseStats()->m_saveVSFortitudeBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSFortitudeBase < -40) {
                pSprite->GetBaseStats()->m_saveVSFortitudeBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nSaveVSFortitude += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSFortitudeBase = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_saveVSFortitudeBase > 40) {
                pSprite->GetBaseStats()->m_saveVSFortitudeBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSFortitudeBase < -40) {
                pSprite->GetBaseStats()->m_saveVSFortitudeBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nSaveVSFortitude = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetDerivedStats()->m_nSaveVSFortitude > 40) {
                pSprite->GetDerivedStats()->m_nSaveVSFortitude = 40;
            }

            if (pSprite->GetDerivedStats()->m_nSaveVSFortitude < -40) {
                pSprite->GetDerivedStats()->m_nSaveVSFortitude = -40;
            }

            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSFortitudeBase = pSprite->GetBaseStats()->m_saveVSFortitudeBase * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetBaseStats()->m_saveVSFortitudeBase > 40) {
                pSprite->GetBaseStats()->m_saveVSFortitudeBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSFortitudeBase < -40) {
                pSprite->GetBaseStats()->m_saveVSFortitudeBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nSaveVSFortitude = pSprite->GetBaseStats()->m_saveVSFortitudeBase * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetDerivedStats()->m_nSaveVSFortitude > 40) {
                pSprite->GetDerivedStats()->m_nSaveVSFortitude = 40;
            }

            if (pSprite->GetDerivedStats()->m_nSaveVSFortitude < -40) {
                pSprite->GetDerivedStats()->m_nSaveVSFortitude = -40;
            }

            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 10621
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSaveVsRelfex::CGameEffectSaveVsRelfex(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495040
CGameEffect* CGameEffectSaveVsRelfex::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSaveVsRelfex* copy = new CGameEffectSaveVsRelfex(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B0F20
BOOL CGameEffectSaveVsRelfex::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSReflexBase += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_saveVSReflexBase > 40) {
                pSprite->GetBaseStats()->m_saveVSReflexBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSReflexBase < -40) {
                pSprite->GetBaseStats()->m_saveVSReflexBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nSaveVSReflex += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSReflexBase = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_saveVSReflexBase > 40) {
                pSprite->GetBaseStats()->m_saveVSReflexBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSReflexBase < -40) {
                pSprite->GetBaseStats()->m_saveVSReflexBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nSaveVSReflex = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetDerivedStats()->m_nSaveVSReflex > 40) {
                pSprite->GetDerivedStats()->m_nSaveVSReflex = 40;
            }

            if (pSprite->GetDerivedStats()->m_nSaveVSReflex < -40) {
                pSprite->GetDerivedStats()->m_nSaveVSReflex = -40;
            }

            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSReflexBase = pSprite->GetBaseStats()->m_saveVSReflexBase * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetBaseStats()->m_saveVSReflexBase > 40) {
                pSprite->GetBaseStats()->m_saveVSReflexBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSReflexBase < -40) {
                pSprite->GetBaseStats()->m_saveVSReflexBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nSaveVSReflex = pSprite->GetBaseStats()->m_saveVSReflexBase * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetDerivedStats()->m_nSaveVSReflex > 40) {
                pSprite->GetDerivedStats()->m_nSaveVSReflex = 40;
            }

            if (pSprite->GetDerivedStats()->m_nSaveVSReflex < -40) {
                pSprite->GetDerivedStats()->m_nSaveVSReflex = -40;
            }

            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 10730
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSaveVsWill::CGameEffectSaveVsWill(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4950F0
CGameEffect* CGameEffectSaveVsWill::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSaveVsWill* copy = new CGameEffectSaveVsWill(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B1130
BOOL CGameEffectSaveVsWill::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSWillBase += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_saveVSWillBase > 40) {
                pSprite->GetBaseStats()->m_saveVSWillBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSWillBase < -40) {
                pSprite->GetBaseStats()->m_saveVSWillBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nSaveVSWill += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSWillBase = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_saveVSWillBase > 40) {
                pSprite->GetBaseStats()->m_saveVSWillBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSWillBase < -40) {
                pSprite->GetBaseStats()->m_saveVSWillBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nSaveVSWill = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetDerivedStats()->m_nSaveVSWill > 40) {
                pSprite->GetDerivedStats()->m_nSaveVSWill = 40;
            }

            if (pSprite->GetDerivedStats()->m_nSaveVSWill < -40) {
                pSprite->GetDerivedStats()->m_nSaveVSWill = -40;
            }

            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_saveVSWillBase = pSprite->GetBaseStats()->m_saveVSWillBase * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetBaseStats()->m_saveVSWillBase > 40) {
                pSprite->GetBaseStats()->m_saveVSWillBase = 40;
            }

            if (pSprite->GetBaseStats()->m_saveVSWillBase < -40) {
                pSprite->GetBaseStats()->m_saveVSWillBase = -40;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nSaveVSWill = pSprite->GetBaseStats()->m_saveVSWillBase * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetDerivedStats()->m_nSaveVSWill > 40) {
                pSprite->GetDerivedStats()->m_nSaveVSWill = 40;
            }

            if (pSprite->GetDerivedStats()->m_nSaveVSWill < -40) {
                pSprite->GetDerivedStats()->m_nSaveVSWill = -40;
            }

            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 10839
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSilence::CGameEffectSilence(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4951A0
CGameEffect* CGameEffectSilence::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSilence* copy = new CGameEffectSilence(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B1340
BOOL CGameEffectSilence::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14676);
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 34);

    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_SILENCED;
        pSprite->GetDerivedStats()->m_generalState |= STATE_SILENCED;
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_generalState |= STATE_SILENCED;
    }

    return TRUE;
}

// 0x4B5490
void CGameEffectSilence::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// 0x495250
CGameEffectSleep::CGameEffectSleep(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495290
CGameEffect* CGameEffectSleep::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSleep* copy = new CGameEffectSleep(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B1430
BOOL CGameEffectSleep::ApplyEffect(CGameSprite* pSprite)
{
    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_SLEEPING;
        pSprite->GetDerivedStats()->m_generalState |= STATE_SLEEPING;
        pSprite->GetBaseStats()->m_generalState |= STATE_HELPLESS;
        pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_generalState |= STATE_SLEEPING | STATE_HELPLESS;

        if (m_dwFlags == 1) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DOESNT_AWAKEN_ON_DAMAGE, true);
        }

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 14);

        if (m_secondaryType) {
            // NOTE: Uninline.
            DisplayStringRef(pSprite, 14001);
        }

        ITEM_EFFECT* effect = new ITEM_EFFECT;

        // NOTE: Uninline.
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

// NOTE: Inlined.
CGameEffectSlow::CGameEffectSlow(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495340
CGameEffect* CGameEffectSlow::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSlow* copy = new CGameEffectSlow(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B1650
BOOL CGameEffectSlow::ApplyEffect(CGameSprite* pSprite)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene != TRUE) {
        if (m_secondaryType != 0) {
            // NOTE: Uninline.
            DisplayStringRef(pSprite, 14668); // "Slowed"
        }

        AddSlowEffect(pSprite);
    } else {
        m_done = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSparkle::CGameEffectSparkle(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4953F0
CGameEffect* CGameEffectSparkle::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSparkle* copy = new CGameEffectSparkle(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSpellMemorizationMage::CGameEffectSpellMemorizationMage(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4954A0
CGameEffect* CGameEffectSpellMemorizationMage::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSpellMemorizationMage* copy = new CGameEffectSpellMemorizationMage(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectStoneToFlesh::CGameEffectStoneToFlesh()
{
    m_effectID = CGAMEEFFECT_STONETOFLESH;
}

// NOTE: Inlined.
CGameEffectStoneToFlesh::CGameEffectStoneToFlesh(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495600
CGameEffect* CGameEffectStoneToFlesh::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectStoneToFlesh* copy = new CGameEffectStoneToFlesh(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSTR::CGameEffectSTR(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4956B0
CGameEffect* CGameEffectSTR::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSTR* copy = new CGameEffectSTR(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2360
BOOL CGameEffectSTR::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_STRBase = min(max(pSprite->GetBaseStats()->m_STRBase + static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Uninline.
            AdjustSTR(pSprite, m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_STRBase = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nSTR = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_STRBase = min(max(pSprite->GetBaseStats()->m_STRBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nSTR = min(max(pSprite->GetBaseStats()->m_STRBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 11439
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectStun::CGameEffectStun(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495760
CGameEffect* CGameEffectStun::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectStun* copy = new CGameEffectStun(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2560
BOOL CGameEffectStun::ApplyEffect(CGameSprite* pSprite)
{
    if (m_dwFlags == 2) {
        return CheckStunRemove(pSprite);
    }

    if (m_dwFlags == 1) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_UNSTUN_ON_DAMAGE, TRUE);
    }

    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 1280); // "Stunned"
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 44);

    pSprite->sub_761650();

    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_STUNNED;
    }

    pSprite->GetDerivedStats()->m_generalState |= STATE_STUNNED;
    pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;
    pSprite->field_9D15 = 1;

    return TRUE;
}

// 0x4B2680
BOOL CGameEffectStun::CheckStunRemove(CGameSprite* pSprite)
{
    INT nHealth = pSprite->GetBaseStats()->m_hitPoints;
    int v1 = 0;

    if (nHealth <= 50) {
        for (int roll = 0; roll < 4; roll++) {
            v1 += CUtil::UtilRandInt(4, -pSprite->m_tempStats.m_nLuck) + 1;
        }
    } else if (nHealth <= 100) {
        for (int roll = 0; roll < 2; roll++) {
            v1 += CUtil::UtilRandInt(4, -pSprite->m_tempStats.m_nLuck) + 1;
        }
    } else if (nHealth <= 150) {
        v1 += CUtil::UtilRandInt(4, -pSprite->m_tempStats.m_nLuck) + 1;
    } else {
        FeedBackImmuneToResource(pSprite, m_sourceRes);
        m_done = TRUE;
        return TRUE;
    }

    CGameObject* pSource;

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
        CGameObjectArray::THREAD_ASYNCH,
        &pSource,
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (v1 > 0) {
        CGameEffect* pEffect = IcewindMisc::CreateEffectStun(pSource,
            7 * v1,
            static_cast<BYTE>(m_spellLevel),
            0);
        pEffect->m_flags |= 0x2;
        pEffect->m_sourceRes = m_sourceRes;
        pEffect->m_casterLevel = m_casterLevel;
        pSprite->AddEffect(pEffect,
            CGameAIBase::EFFECT_LIST_TIMED,
            TRUE,
            TRUE);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectUnstun::CGameEffectUnstun(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495810
CGameEffect* CGameEffectUnstun::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectUnstun* copy = new CGameEffectUnstun(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2810
BOOL CGameEffectUnstun::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_STUNNED;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_STUNNED;

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_STUN,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_STUN,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        44);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        44);

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_HOLDCREATURE,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_HOLDCREATURE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        13);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        13);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectVisible::CGameEffectVisible(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4958C0
CGameEffect* CGameEffectVisible::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectVisible* copy = new CGameEffectVisible(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectVocalize::CGameEffectVocalize(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495970
CGameEffect* CGameEffectVocalize::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectVocalize* copy = new CGameEffectVocalize(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2A40
BOOL CGameEffectVocalize::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14776); // "Vocalize"
    }

    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_VOCALIZE);

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 139);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectWIS::CGameEffectWIS(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495A20
CGameEffect* CGameEffectWIS::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectWIS* copy = new CGameEffectWIS(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2B20
BOOL CGameEffectWIS::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_WISBase = min(max(pSprite->GetBaseStats()->m_WISBase + static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Uninline.
            AdjustWIS(pSprite, m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_WISBase = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nWIS = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_WISBase = min(max(pSprite->GetBaseStats()->m_WISBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nWIS = min(max(pSprite->GetBaseStats()->m_WISBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 11877
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSingleColorPulseAll::CGameEffectSingleColorPulseAll(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493E80
CGameEffect* CGameEffectSingleColorPulseAll::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSingleColorPulseAll* copy = new CGameEffectSingleColorPulseAll(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A7360
BOOL CGameEffectSingleColorPulseAll::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->m_hasColorEffects = TRUE;

    CPersistantEffectColorEffect* pEffect = new CPersistantEffectColorEffect();
    pEffect->m_effectFinalRed = GetRValue(m_effectAmount >> 8);
    pEffect->m_effectFinalGreen = GetGValue(m_effectAmount >> 8);
    pEffect->m_effectFinalBlue = GetBValue(m_effectAmount >> 8);
    pEffect->m_effectDegreeRed = 0;
    pEffect->m_effectDegreeGreen = 0;
    pEffect->m_effectDegreeBlue = 0;
    pEffect->m_undo = TRUE;
    pEffect->m_colorEffect = 1;
    pEffect->m_period = (m_dwFlags >> 16) & 0xFFFF;
    pSprite->m_persistantEffects.AddTail(pEffect);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectColorTintSolid::CGameEffectColorTintSolid(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493D20
CGameEffect* CGameEffectColorTintSolid::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectColorTintSolid* copy = new CGameEffectColorTintSolid(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A7000
BOOL CGameEffectColorTintSolid::ApplyEffect(CGameSprite* pSprite)
{
    if (m_slotNum == pSprite->GetWeaponSlot()
        && (m_dwFlags & 0xF0) == CGameAnimationType::RANGE_WEAPON) {
        m_dwFlags &= 0xF;
        m_dwFlags |= CGameAnimationType::RANGE_SHIELD;
    }

    pSprite->m_hasColorEffects = TRUE;

    COLORREF tintColor = m_effectAmount >> 8;

    if (m_dwFlags == 255) {
        // FIXME: Odd selection of type, should be `BYTE`.
        //
        // 0x8A8EE0
        static int ranges[] = {
            0x00,
            0x01,
            0x02,
            0x03,
            0x04,
            0x05,
            0x06,
            0x14,
            0x15,
            0x21,
            0x22,
            0x24,
            0x25,
            0x31,
            0x32,
            0x34,
            0x35,
            -1,
        };

        int* range = ranges;
        while (*range != -1) {
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorEffect(0, static_cast<BYTE>(*range), tintColor, 1);

            CColorEffect* pColorEffect = new CColorEffect();
            pColorEffect->m_effectType = 0;
            pColorEffect->m_range = static_cast<BYTE>(*range);
            pColorEffect->m_tintColor = tintColor;
            pColorEffect->m_periodLength = 1;
            pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);

            range++;
        }
    } else {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorEffect(0, static_cast<BYTE>(m_dwFlags), tintColor, 1);

        CColorEffect* pColorEffect = new CColorEffect();
        pColorEffect->m_effectType = 0;
        pColorEffect->m_range = static_cast<BYTE>(m_dwFlags);
        pColorEffect->m_tintColor = tintColor;
        pColorEffect->m_periodLength = 1;
        pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectColorLightSolid::CGameEffectColorLightSolid(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x493DD0
CGameEffect* CGameEffectColorLightSolid::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectColorLightSolid* copy = new CGameEffectColorLightSolid(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A71B0
BOOL CGameEffectColorLightSolid::ApplyEffect(CGameSprite* pSprite)
{
    if (m_slotNum == pSprite->GetWeaponSlot()
        && (m_dwFlags & 0xF0) == CGameAnimationType::RANGE_WEAPON) {
        m_dwFlags &= 0xF;
        m_dwFlags |= CGameAnimationType::RANGE_SHIELD;
    }

    pSprite->m_hasColorEffects = TRUE;

    COLORREF tintColor = m_effectAmount >> 8;

    if (m_dwFlags == 255) {
        // FIXME: Odd selection of type, should be `BYTE`.
        //
        // 0x8A8F28
        static int ranges[] = {
            0x00,
            0x01,
            0x02,
            0x03,
            0x04,
            0x05,
            0x06,
            0x14,
            0x15,
            0x21,
            0x22,
            0x24,
            0x25,
            0x31,
            0x32,
            0x34,
            0x35,
            -1,
        };

        int* range = ranges;
        while (*range != -1) {
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorEffect(2, static_cast<BYTE>(*range), tintColor, 1);

            CColorEffect* pColorEffect = new CColorEffect();
            pColorEffect->m_effectType = 2;
            pColorEffect->m_range = static_cast<BYTE>(*range);
            pColorEffect->m_tintColor = tintColor;
            pColorEffect->m_periodLength = 1;
            pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);

            range++;
        }
    } else {
        // NOTE: Uninline.
        pSprite->GetAnimation()->SetColorEffect(2, static_cast<BYTE>(m_dwFlags), tintColor, 1);

        CColorEffect* pColorEffect = new CColorEffect();
        pColorEffect->m_effectType = 2;
        pColorEffect->m_range = static_cast<BYTE>(m_dwFlags);
        pColorEffect->m_tintColor = tintColor;
        pColorEffect->m_periodLength = 1;
        pSprite->GetDerivedStats()->m_appliedColorEffects.AddTail(pColorEffect);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAnimationChange::CGameEffectAnimationChange(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495AD0
CGameEffect* CGameEffectAnimationChange::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAnimationChange* copy = new CGameEffectAnimationChange(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBaseAttackBonus::CGameEffectBaseAttackBonus(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495BB0
CGameEffect* CGameEffectBaseAttackBonus::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBaseAttackBonus* copy = new CGameEffectBaseAttackBonus(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2D20
BOOL CGameEffectBaseAttackBonus::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_attackBase = min(max(pSprite->GetBaseStats()->m_attackBase + static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Uninline.
            AdjustTHAC0(pSprite, static_cast<SHORT>(m_effectAmount));
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_attackBase = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nTHAC0 = min(max(static_cast<SHORT>(m_effectAmount), 1), 40);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_attackBase = min(max(pSprite->GetBaseStats()->m_attackBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nTHAC0 = min(max(pSprite->GetBaseStats()->m_attackBase * static_cast<SHORT>(m_effectAmount) / 100, 1), 40);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 11968
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSlay::CGameEffectSlay(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495C60
CGameEffect* CGameEffectSlay::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSlay* copy = new CGameEffectSlay(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAlignmentReversal::CGameEffectAlignmentReversal(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495D10
CGameEffect* CGameEffectAlignmentReversal::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAlignmentReversal* copy = new CGameEffectAlignmentReversal(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B3840
BOOL CGameEffectAlignmentReversal::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType typeAI(pSprite->GetAIType());
    CAIObjectType liveTypeAI(pSprite->GetLiveAIType());

    // FIXME: Probably wrong (using live as start).
    CAIObjectType startTypeAI(pSprite->GetLiveAIType());

    if (m_durationType == 1) {
        // NOTE: Uninline.
        startTypeAI.SetAlignment(Reverse(startTypeAI.GetAlignment()));
        pSprite->m_startTypeAI = startTypeAI;
        m_done = TRUE;
    } else {
        m_done = FALSE;
    }

    // NOTE: Uninline.
    typeAI.SetAlignment(Reverse(typeAI.GetAlignment()));
    pSprite->SetAIType(typeAI, FALSE, FALSE);

    // NOTE: Uninline.
    liveTypeAI.SetAlignment(Reverse(liveTypeAI.GetAlignment()));
    pSprite->m_liveTypeAI = liveTypeAI;

    return TRUE;
}

// NOTE: Inlined.
BYTE CGameEffectAlignmentReversal::Reverse(BYTE alignment)
{
    BYTE newAlign;

    switch (alignment) {
    case 0:
        newAlign = 0;
        break;
    case CAIOBJECTTYPE_LAWFUL_GOOD:
        newAlign = CAIOBJECTTYPE_CHAOTIC_EVIL;
        break;
    case CAIOBJECTTYPE_LAWFUL_NEUTRAL:
        newAlign = CAIOBJECTTYPE_CHAOTIC_NEUTRAL;
        break;
    case CAIOBJECTTYPE_LAWFUL_EVIL:
        newAlign = CAIOBJECTTYPE_CHAOTIC_GOOD;
        break;
    case CAIOBJECTTYPE_NEUTRAL_GOOD:
        newAlign = CAIOBJECTTYPE_NEUTRAL_EVIL;
        break;
    case CAIOBJECTTYPE_NEUTRAL:
        newAlign = CAIOBJECTTYPE_NEUTRAL;
        break;
    case CAIOBJECTTYPE_NEUTRAL_EVIL:
        newAlign = CAIOBJECTTYPE_NEUTRAL_GOOD;
        break;
    case CAIOBJECTTYPE_CHAOTIC_GOOD:
        newAlign = CAIOBJECTTYPE_LAWFUL_EVIL;
        break;
    case CAIOBJECTTYPE_CHAOTIC_NEUTRAL:
        newAlign = CAIOBJECTTYPE_LAWFUL_NEUTRAL;
        break;
    case CAIOBJECTTYPE_CHAOTIC_EVIL:
        newAlign = CAIOBJECTTYPE_LAWFUL_GOOD;
        break;
    }

    return newAlign;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAlignmentChange::CGameEffectAlignmentChange(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495DC0
CGameEffect* CGameEffectAlignmentChange::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAlignmentChange* copy = new CGameEffectAlignmentChange(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B3BA0
BOOL CGameEffectAlignmentChange::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType typeAI(pSprite->GetAIType());
    CAIObjectType liveTypeAI(pSprite->GetLiveAIType());

    // FIXME: Probably wrong (using live as start).
    CAIObjectType startTypeAI(pSprite->GetLiveAIType());

    if (m_durationType == 1) {
        startTypeAI.SetAlignment(static_cast<BYTE>(m_dwFlags));
        pSprite->m_startTypeAI = startTypeAI;
        m_done = TRUE;
    } else {
        m_done = FALSE;
    }

    typeAI.SetAlignment(static_cast<BYTE>(m_dwFlags));
    pSprite->SetAIType(typeAI, FALSE, FALSE);

    liveTypeAI.SetAlignment(static_cast<BYTE>(m_dwFlags));
    pSprite->m_liveTypeAI = liveTypeAI;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDispelEffects::CGameEffectDispelEffects(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495E70
CGameEffect* CGameEffectDispelEffects::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelEffects* copy = new CGameEffectDispelEffects(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B3D00
BOOL CGameEffectDispelEffects::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_MIND)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_SWORD)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_MAGE)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_VENOM)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_THE_SPIRIT)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_FORTITUDE)));
    pSprite->RemoveSpecialAbility(CResRef(CString(RESREF_EYE_OF_STONE)));

    if (m_dwFlags != 0) {
        pSprite->GetTimedEffectList()->RemoveAllEffectsIgnoreMoreThenPermanent(pSprite->GetTimedEffectList()->GetPosCurrent(),
            TRUE,
            TRUE,
            rand() % 100,
            static_cast<BYTE>(m_casterLevel));
    } else {
        pSprite->GetTimedEffectList()->RemoveAllEffectsIgnoreMoreThenPermanent(pSprite->GetTimedEffectList()->GetPosCurrent(),
            TRUE,
            FALSE,
            0,
            0);
    }

    pSprite->UnequipAll(TRUE);
    StripItemSlot(pSprite, 42);
    pSprite->EquipAll(TRUE);
    pSprite->m_hasColorEffects = TRUE;
    pSprite->m_hasColorRangeEffects = TRUE;
    pSprite->field_5640 = 1;

    m_forceRepass = TRUE;
    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillStealth::CGameEffectSkillStealth(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495F20
CGameEffect* CGameEffectSkillStealth::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillStealth* copy = new CGameEffectSkillStealth(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B3F60
BOOL CGameEffectSkillStealth::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_MOVE_SILENTLY] += static_cast<BYTE>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else if (m_durationType == 9) {
            pSprite->m_bonusStats.m_nMoveSilentlyMTPBonus += m_effectAmount;
            m_done = FALSE;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_MOVE_SILENTLY] += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            // FIXME: Why nothing changed?
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_MOVE_SILENTLY] = static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_MOVE_SILENTLY] = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_MOVE_SILENTLY] * static_cast<SHORT>(m_effectAmount) / 100;
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_MOVE_SILENTLY] = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_MOVE_SILENTLY] * static_cast<SHORT>(m_effectAmount) / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 12565
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined
CGameEffectHideInShadows::CGameEffectHideInShadows(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495FD0
CGameEffect* CGameEffectHideInShadows::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHideInShadows* copy = new CGameEffectHideInShadows(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4080
BOOL CGameEffectHideInShadows::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 9) {
            pSprite->m_bonusStats.m_nHideInShadowsMTPBonus += m_effectAmount;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_HIDE] += static_cast<BYTE>(m_effectAmount);
        }
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_HIDE] = static_cast<BYTE>(m_effectAmount);
        break;
    case 2:
        pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_HIDE] = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_HIDE] * static_cast<SHORT>(m_effectAmount) / 100;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 12617
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCastingFailure::CGameEffectCastingFailure(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496080
CGameEffect* CGameEffectCastingFailure::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCastingFailure* copy = new CGameEffectCastingFailure(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4140
BOOL CGameEffectCastingFailure::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_nSpellFailureArcane += static_cast<SHORT>(m_effectAmount);
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nSpellFailureDivine += static_cast<SHORT>(m_effectAmount);
        break;
    case 2:
        pSprite->GetDerivedStats()->m_nSpellFailureArcane += static_cast<SHORT>(m_effectAmount);
        pSprite->GetDerivedStats()->m_nSpellFailureDivine += static_cast<SHORT>(m_effectAmount);
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillAlchemy::CGameEffectSkillAlchemy(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x494710
CGameEffect* CGameEffectSkillAlchemy::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillAlchemy* copy = new CGameEffectSkillAlchemy(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSpellMemoizationCleric::CGameEffectSpellMemoizationCleric(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x495550
CGameEffect* CGameEffectSpellMemoizationCleric::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSpellMemoizationCleric* copy = new CGameEffectSpellMemoizationCleric(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectInfravision::CGameEffectInfravision(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496130
CGameEffect* CGameEffectInfravision::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectInfravision* copy = new CGameEffectInfravision(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4B00
BOOL CGameEffectInfravision::ApplyEffect(CGameSprite* pSprite)
{
    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_INFRAVISION;
        pSprite->GetDerivedStats()->m_generalState |= STATE_INFRAVISION;
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_generalState |= STATE_INFRAVISION;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectInfravisionOff::CGameEffectInfravisionOff(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4961E0
CGameEffect* CGameEffectInfravisionOff::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectInfravisionOff* copy = new CGameEffectInfravisionOff(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4B40
BOOL CGameEffectInfravisionOff::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_INFRAVISION;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_INFRAVISION;

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INFRAVISION,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INFRAVISION,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBlur::CGameEffectBlur(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496290
CGameEffect* CGameEffectBlur::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBlur* copy = new CGameEffectBlur(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4A5DC0
BOOL CGameEffectBlur::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->GetDerivedStats()->m_generalState & STATE_BLUR) == 0) {
        pSprite->GetDerivedStats()->m_generalState |= STATE_BLUR;
        pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_BLUR] = true;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 109);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectTranslucent::CGameEffectTranslucent(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496600
CGameEffect* CGameEffectTranslucent::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectTranslucent* copy = new CGameEffectTranslucent(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BBB90
BOOL CGameEffectTranslucent::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_nTranslucent = static_cast<BYTE>(m_effectAmount);
        break;
    case 1:
        pSprite->GetBaseStats()->m_generalState |= STATE_FADE_IN;
        pSprite->GetBaseStats()->m_fadeSpeed = static_cast<BYTE>(m_effectAmount);
        m_done = TRUE;
        break;
    case 2:
        pSprite->GetBaseStats()->m_generalState |= STATE_FADE_OUT;
        pSprite->GetBaseStats()->m_fadeSpeed = static_cast<BYTE>(m_effectAmount);
        m_done = TRUE;
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillUnsummon::CGameEffectSkillUnsummon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4966B0
CGameEffect* CGameEffectSkillUnsummon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillUnsummon* copy = new CGameEffectSkillUnsummon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4BD0
BOOL CGameEffectSkillUnsummon::ApplyEffect(CGameSprite* pSprite)
{
    // Party members (portrait != -1) are never unsummoned; only summoned
    // creatures are.
    LONG portraitNum = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId());
    if (portraitNum == -1) {
        Icewind586B70::Instance()->Unsummon(pSprite);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessageUnsummon(pSprite->GetId(), pSprite->GetId(), pSprite->GetId()), FALSE);
        pSprite->m_removeFromArea = TRUE;
    }

    m_done = TRUE;
    return TRUE;
}

// 0x4B4BB0
void CGameEffectSkillUnsummon::DisplayString(CGameSprite* pSprite)
{
    if (m_effectAmount == 1) {
        CGameEffect::DisplayString(pSprite);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectNonDetection::CGameEffectNonDetection(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496760
CGameEffect* CGameEffectNonDetection::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectNonDetection* copy = new CGameEffectNonDetection(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4C60
BOOL CGameEffectNonDetection::ApplyEffect(CGameSprite* pSprite)
{
    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_NONDETECTION;
        pSprite->GetDerivedStats()->m_generalState |= STATE_NONDETECTION;
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_generalState |= STATE_NONDETECTION;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectEndNonDetection::CGameEffectEndNonDetection(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496810
CGameEffect* CGameEffectEndNonDetection::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectEndNonDetection* copy = new CGameEffectEndNonDetection(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B4CA0
BOOL CGameEffectEndNonDetection::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_NONDETECTION;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_NONDETECTION;

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_NONDETECTION,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_NONDETECTION,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSexChange::CGameEffectSexChange(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4968C0
CGameEffect* CGameEffectSexChange::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSexChange* copy = new CGameEffectSexChange(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAIChange::CGameEffectAIChange(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496970
CGameEffect* CGameEffectAIChange::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAIChange* copy = new CGameEffectAIChange(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B5000
BOOL CGameEffectAIChange::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType type(pSprite->GetAIType());
    CAIObjectType liveType(pSprite->m_liveTypeAI);
    CAIObjectType startType(pSprite->m_startTypeAI);

    switch (m_dwFlags) {
    case 0:
        type.SetEnemyAlly(static_cast<BYTE>(m_effectAmount));
        liveType.SetEnemyAlly(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetEnemyAlly(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    case 1:
        type.SetGeneral(static_cast<BYTE>(m_effectAmount));
        liveType.SetGeneral(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetGeneral(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    case 2:
        type.SetRace(static_cast<BYTE>(m_effectAmount));
        liveType.SetRace(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetRace(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    case 3:
        type.SetClass(static_cast<BYTE>(m_effectAmount));
        liveType.SetClass(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetClass(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    case 4:
        type.SetSpecific(static_cast<BYTE>(m_effectAmount));
        liveType.SetSpecific(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetSpecific(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    case 5:
        type.SetGender(static_cast<BYTE>(m_effectAmount));
        liveType.SetGender(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetGender(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    case 6:
        type.SetAlignment(static_cast<BYTE>(m_effectAmount));
        liveType.SetAlignment(static_cast<BYTE>(m_effectAmount));
        if (m_durationType == 1) {
            startType.SetAlignment(static_cast<BYTE>(m_effectAmount));
            m_done = TRUE;
        }
        break;
    }

    pSprite->SetAIType(type, FALSE, FALSE);
    pSprite->m_liveTypeAI.Set(liveType);
    pSprite->m_startTypeAI.Set(startType);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDamageMod::CGameEffectDamageMod(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496A20
CGameEffect* CGameEffectDamageMod::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDamageMod* copy = new CGameEffectDamageMod(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B5220
BOOL CGameEffectDamageMod::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_nDamageBonus += static_cast<SHORT>(m_effectAmount);
        if (pSprite->GetDerivedStats()->m_nDamageBonus > 20) {
            pSprite->GetDerivedStats()->m_nDamageBonus = 20;
        }
        if (pSprite->GetDerivedStats()->m_nDamageBonus < -20) {
            pSprite->GetDerivedStats()->m_nDamageBonus = -20;
        }
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nDamageModFire += static_cast<SHORT>(m_effectAmount);
        break;
    case 2:
        pSprite->GetDerivedStats()->m_nDamageModCold += static_cast<SHORT>(m_effectAmount);
        break;
    case 3:
        pSprite->GetDerivedStats()->m_nDamageModElectricity += static_cast<SHORT>(m_effectAmount);
        break;
    case 4:
        pSprite->GetDerivedStats()->m_nDamageModAcid += static_cast<SHORT>(m_effectAmount);
        break;
    case 5:
        pSprite->GetDerivedStats()->m_nDamageModMagic += static_cast<SHORT>(m_effectAmount);
        break;
    case 6:
        pSprite->GetDerivedStats()->m_nDamageModPoison += static_cast<SHORT>(m_effectAmount);
        break;
    case 7:
        pSprite->GetDerivedStats()->m_nDamageModSlashing += static_cast<SHORT>(m_effectAmount);
        break;
    case 8:
        pSprite->GetDerivedStats()->m_nDamageModPiercing += static_cast<SHORT>(m_effectAmount);
        break;
    case 9:
        pSprite->GetDerivedStats()->m_nDamageModCrushing += static_cast<SHORT>(m_effectAmount);
        break;
    case 10:
        pSprite->GetDerivedStats()->m_nDamageModMissile += static_cast<SHORT>(m_effectAmount);
        break;
    }

    m_done = FALSE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBlindness::CGameEffectBlindness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496AD0
CGameEffect* CGameEffectBlindness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBlindness* copy = new CGameEffectBlindness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B53A0
BOOL CGameEffectBlindness::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14674); // "Blinded"
    }

    if ((pSprite->GetDerivedStats()->m_generalState & STATE_BLIND) == 0) {
        pSprite->GetDerivedStats()->m_generalState |= STATE_BLIND;

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 8);

        int v1 = -2;
        if (pSprite->HasFeat(CGAMESPRITE_FEAT_BLIND_FIGHT)) {
            v1 = 0;
        }

        pSprite->GetDerivedStats()->m_nACDodgeBonus += v1;
    }

    return TRUE;
}

// 0x4B5490
void CGameEffectBlindness::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDayBlindness::CGameEffectDayBlindness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496B80
CGameEffect* CGameEffectDayBlindness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDayBlindness* copy = new CGameEffectDayBlindness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B54C0
BOOL CGameEffectDayBlindness::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 13510
    UTIL_ASSERT(pSprite != NULL);

    if (pSprite->GetArea() != NULL) {
        if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DAY_BLINDNESS]) {
            pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DAY_BLINDNESS] = true;
            SHORT nModifier = 0;
            if (pSprite->m_typeAI.m_nRace == CAIOBJECTTYPE_R_ELF
                && pSprite->m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_ELF_DROW) {
                nModifier = -1;
            } else if (pSprite->m_typeAI.m_nRace == CAIOBJECTTYPE_R_DWARF
                && pSprite->m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_DWARF_GRAY) {
                nModifier = -2;
            }

            pSprite->m_bonusStats.m_nSaveVSFortitude += nModifier;
            pSprite->m_bonusStats.m_nSaveVSReflex += nModifier;
            pSprite->m_bonusStats.m_nSaveVSWill += nModifier;

            for (int skill = 0; skill < CGAMESPRITE_SKILL_NUMSKILLS; skill++) {
                pSprite->m_bonusStats.m_nSkills[skill] += nModifier;
            }

            // NOTE: Uninline.
            if (pSprite->InControl()) {
                pSprite->m_bSendSpriteUpdate = TRUE;
            }
        } else {
            m_done = TRUE;
        }
    }

    return TRUE;
}

// 0x4B5490
void CGameEffectDayBlindness::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCureBlindness::CGameEffectCureBlindness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496C30
CGameEffect* CGameEffectCureBlindness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCureBlindness* copy = new CGameEffectCureBlindness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B55B0
BOOL CGameEffectCureBlindness::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_BLIND;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_BLIND;

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_BLINDNESS,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_BLINDNESS,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectFeebleMindedness::CGameEffectFeebleMindedness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496CE0
CGameEffect* CGameEffectFeebleMindedness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectFeebleMindedness* copy = new CGameEffectFeebleMindedness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B5610
BOOL CGameEffectFeebleMindedness::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14407); // "Feebleminded"
    }

    pSprite->GetDerivedStats()->m_generalState |= STATE_FEEBLEMINDED;

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 54);

    pSprite->GetDerivedStats()->m_nINT = 3;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCureFeebleMindedness::CGameEffectCureFeebleMindedness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496D90
CGameEffect* CGameEffectCureFeebleMindedness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCureFeebleMindedness* copy = new CGameEffectCureFeebleMindedness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B56F0
BOOL CGameEffectCureFeebleMindedness::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_FEEBLEMINDED;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_FEEBLEMINDED;

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_FEEBLEMINDEDNESS,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_FEEBLEMINDEDNESS,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        54);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        54);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDisease::CGameEffectDisease(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496E40
CGameEffect* CGameEffectDisease::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDisease* copy = new CGameEffectDisease(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B5790
BOOL CGameEffectDisease::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_generalState |= STATE_DISEASED;

    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 4389);
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 7);

    switch (m_dwFlags) {
    case 4:
        pSprite->m_bonusStats.m_nSTR -= static_cast<SHORT>(m_effectAmount);
        return TRUE;
    case 5:
        pSprite->m_bonusStats.m_nDEX -= static_cast<SHORT>(m_effectAmount);
        return TRUE;
    case 6:
        pSprite->m_bonusStats.m_nCON -= static_cast<SHORT>(m_effectAmount);
        return TRUE;
    case 7:
        pSprite->m_bonusStats.m_nINT -= static_cast<SHORT>(m_effectAmount);
        return TRUE;
    case 8:
        pSprite->m_bonusStats.m_nINT -= static_cast<SHORT>(m_effectAmount);
        return TRUE;
    case 9:
        pSprite->m_bonusStats.m_nCHR -= static_cast<SHORT>(m_effectAmount);
        return TRUE;
    case 10:
        AddSlowEffect(pSprite);
        return TRUE;
    case 11:
        ApplyDiseaseMoldTouch(pSprite);
        return TRUE;
    case 12:
        ApplyDiseaseSpawn(pSprite);
        return TRUE;
    case 13:
        pSprite->m_bonusStats.m_nSTR -= 2;
        pSprite->m_bonusStats.m_nDEX -= 2;
        pSprite->m_bonusStats.m_nCHR -= 2;
        AddSlowEffect(pSprite);
        return TRUE;
    case 14:
        ApplyDiseaseBlindness(pSprite);
        return TRUE;
    case 15:
        ApplyDiseaseSlow(pSprite);
        return TRUE;
    }

    if (m_secondaryType != 0) {
        m_special = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
    }

    DWORD duration = m_durationType != 1 ? m_duration : 0;

    CPersistantEffect84C420* pEffect;
    switch (m_dwFlags) {
    case 1:
        pEffect = new CPersistantEffect84C420(0x40000000,
            m_effectAmount,
            105,
            m_special,
            duration);
        break;
    case 2:
        pEffect = new CPersistantEffect84C420(0x40000000,
            m_effectAmount,
            15,
            m_special,
            duration);
        break;
    case 3:
        pEffect = new CPersistantEffect84C420(0x40000000,
            1,
            15 * m_effectAmount,
            m_special,
            duration);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 13882
        UTIL_ASSERT(FALSE);
    }

    pEffect->m_sourceID = m_sourceID;
    strcpy(reinterpret_cast<char*>(pEffect->field_29), m_sourceRes.GetResRefStr());
    pSprite->GetDerivedStats()->m_cRegeneratedPersistantEffectList.AddTail(pEffect);

    return TRUE;
}

// 0x4B5BF0
void CGameEffectDisease::ApplyDiseaseBlindness(CGameSprite* pSprite)
{
    pSprite->m_bonusStats.m_nSTR -= 3;
    pSprite->m_bonusStats.m_nDEX -= 3;

    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14674); // "Blinded"
    }

    pSprite->GetDerivedStats()->m_generalState |= STATE_BLIND;

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 8);

    pSprite->GetDerivedStats()->m_nACDodgeBonus -= 4;
    pSprite->GetDerivedStats()->m_nTHAC0 -= 4;

    if (m_secondaryType != 0) {
        CGameObject* pSource;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pSource,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            CGameEffect* pEffect = IcewindMisc::CreatePiercingDamageEffect(pSource,
                0,
                0,
                3,
                static_cast<BYTE>(m_spellLevel),
                0);

            CMessage* message = new CMessageAddEffect(pEffect,
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            m_done = TRUE;
        }
    }
}

// 0x4B5D90
void CGameEffectDisease::ApplyDiseaseSlow(CGameSprite* pSprite)
{
    AddSlowEffect(pSprite);

    if (m_secondaryType != 0) {
        m_special = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
    }

    DWORD duration = m_durationType != 1 ? m_duration : 0;

    CPersistantEffect84C420* pEffect = new CPersistantEffect84C420(0x40000000,
        1,
        15,
        m_special,
        duration);
    pSprite->GetDerivedStats()->m_cRegeneratedPersistantEffectList.AddTail(pEffect);
}

// 0x4B5E50
void CGameEffectDisease::ApplyDiseaseMoldTouch(CGameSprite* pSprite)
{
    if (IcewindMisc::IsDead(pSprite) != TRUE
        && !pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_MOLD_TOUCH]) {
        if (m_secondaryType != 0) {
            // NOTE: Uninline.
            DisplayStringRef(pSprite, 11501); // "Afflicted with Mold Touch"
        }

        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_MOLD_TOUCH] = true;

        pSprite->SetColorRange(2);

        CPersistantEffect84C484* pEffect = new CPersistantEffect84C484(m_effectAmount,
            "",
            100,
            105,
            m_special,
            m_duration);
        pEffect->m_sourceID = m_sourceID;
        strcpy(reinterpret_cast<char*>(pEffect->field_29), m_sourceRes.GetResRefStr());
        pSprite->GetDerivedStats()->m_cRegeneratedPersistantEffectList.AddTail(pEffect);
    } else {
        m_done = TRUE;
    }
}

// 0x4B5FF0
void CGameEffectDisease::ApplyDiseaseSpawn(CGameSprite* pSprite)
{
    if (IcewindMisc::IsDead(pSprite) != TRUE) {
        for (int index = 0; index < m_effectAmount; index += 7) {
            CGameEffect* copy = Copy();
            copy->m_dwFlags = 11;
            copy->m_durationType = 4;
            copy->m_duration = 7 * (index + 1);
            copy->m_effectAmount = m_effectAmount - index;
            copy->m_flags |= 0x2;
            copy->m_casterLevel = m_casterLevel;
            pSprite->AddEffect(copy,
                CGameAIBase::EFFECT_LIST_TIMED,
                TRUE,
                TRUE);
        }
    }
    m_done = TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCureDisease::CGameEffectCureDisease(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496EF0
CGameEffect* CGameEffectCureDisease::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCureDisease* copy = new CGameEffectCureDisease(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6090
BOOL CGameEffectCureDisease::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_DISEASE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_DISEASE,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        7);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        7);

    pSprite->GetBaseStats()->m_generalState &= ~STATE_DISEASED;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_DISEASED;

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDeafness::CGameEffectDeafness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496FA0
CGameEffect* CGameEffectDeafness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDeafness* copy = new CGameEffectDeafness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6130
BOOL CGameEffectDeafness::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DEAFENED, TRUE);

    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14073); // "Deafness"
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 83);

    m_done = FALSE;

    return TRUE;
}

// 0x4B5490
void CGameEffectDeafness::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCureDeafness::CGameEffectCureDeafness(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497050
CGameEffect* CGameEffectCureDeafness::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCureDeafness* copy = new CGameEffectCureDeafness(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6200
BOOL CGameEffectCureDeafness::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_DEAFNESS,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_DEAFNESS,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSetAIScript::CGameEffectSetAIScript(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497100
CGameEffect* CGameEffectSetAIScript::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSetAIScript* copy = new CGameEffectSetAIScript(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6250
BOOL CGameEffectSetAIScript::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType != 0
        && m_res != "") {
        CAIScript* script = new CAIScript(m_res);
        pSprite->SetScript(static_cast<SHORT>(m_dwFlags), script);
    }
    return TRUE;
}

// 0x4B6360
void CGameEffectSetAIScript::OnRemove(CGameSprite* pSprite)
{
    CAIScript* script = new CAIScript(CResRef(""));
    pSprite->SetScript(static_cast<SHORT>(m_dwFlags), script);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmunityToProjectile::CGameEffectImmunityToProjectile(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4971B0
CGameEffect* CGameEffectImmunityToProjectile::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunityToProjectile* copy = new CGameEffectImmunityToProjectile(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B63E0
BOOL CGameEffectImmunityToProjectile::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_cImmunitiesProjectile.AddTail(reinterpret_cast<DWORD*>(m_dwFlags));

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistSlashing::CGameEffectResistSlashing(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497260
CGameEffect* CGameEffectResistSlashing::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistSlashing* copy = new CGameEffectResistSlashing(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6400
BOOL CGameEffectResistSlashing::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistSlashingBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistSlashingBase > 100) {
                pSprite->GetBaseStats()->m_resistSlashingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistSlashingBase < 0) {
                pSprite->GetBaseStats()->m_resistSlashingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistSlashing += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistSlashingBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistSlashingBase > 100) {
                pSprite->GetBaseStats()->m_resistSlashingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistSlashingBase < 0) {
                pSprite->GetBaseStats()->m_resistSlashingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistSlashing < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistSlashing = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistSlashingBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistSlashingBase / 100;
            if (pSprite->GetBaseStats()->m_resistSlashingBase > 100) {
                pSprite->GetBaseStats()->m_resistSlashingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistSlashingBase < 0) {
                pSprite->GetBaseStats()->m_resistSlashingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistSlashing = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistSlashingBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 14586
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistCrushing::CGameEffectResistCrushing(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497310
CGameEffect* CGameEffectResistCrushing::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistCrushing* copy = new CGameEffectResistCrushing(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B68A0
BOOL CGameEffectResistCrushing::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistCrushingBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistCrushingBase > 100) {
                pSprite->GetBaseStats()->m_resistCrushingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistCrushingBase < 0) {
                pSprite->GetBaseStats()->m_resistCrushingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistCrushing += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistCrushingBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistCrushingBase > 100) {
                pSprite->GetBaseStats()->m_resistCrushingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistCrushingBase < 0) {
                pSprite->GetBaseStats()->m_resistCrushingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistCrushing < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistCrushing = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistCrushingBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistCrushingBase / 100;
            if (pSprite->GetBaseStats()->m_resistCrushingBase > 100) {
                pSprite->GetBaseStats()->m_resistCrushingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistCrushingBase < 0) {
                pSprite->GetBaseStats()->m_resistCrushingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistCrushing = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistCrushingBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 14900
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistPiercing::CGameEffectResistPiercing(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4973C0
CGameEffect* CGameEffectResistPiercing::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistPiercing* copy = new CGameEffectResistPiercing(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6A50
BOOL CGameEffectResistPiercing::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistPiercingBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistPiercingBase > 100) {
                pSprite->GetBaseStats()->m_resistPiercingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistPiercingBase < 0) {
                pSprite->GetBaseStats()->m_resistPiercingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistPiercing += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistPiercingBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistPiercingBase > 100) {
                pSprite->GetBaseStats()->m_resistPiercingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistPiercingBase < 0) {
                pSprite->GetBaseStats()->m_resistPiercingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistPiercing < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistPiercing = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistPiercingBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistPiercingBase / 100;
            if (pSprite->GetBaseStats()->m_resistPiercingBase > 100) {
                pSprite->GetBaseStats()->m_resistPiercingBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistPiercingBase < 0) {
                pSprite->GetBaseStats()->m_resistPiercingBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistPiercing = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistPiercingBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15021
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistMissile::CGameEffectResistMissile(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497470
CGameEffect* CGameEffectResistMissile::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistMissile* copy = new CGameEffectResistMissile(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6C00
BOOL CGameEffectResistMissile::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistMissileBase += static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistMissileBase > 100) {
                pSprite->GetBaseStats()->m_resistMissileBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistMissileBase < 0) {
                pSprite->GetBaseStats()->m_resistMissileBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nResistMissile += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistMissileBase = static_cast<BYTE>(m_effectAmount);
            if (pSprite->GetBaseStats()->m_resistMissileBase > 100) {
                pSprite->GetBaseStats()->m_resistMissileBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistMissileBase < 0) {
                pSprite->GetBaseStats()->m_resistMissileBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            if (pSprite->GetDerivedStats()->m_nResistMissile < m_effectAmount) {
                pSprite->GetDerivedStats()->m_nResistMissile = static_cast<BYTE>(m_effectAmount);
            }
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistMissileBase = static_cast<BYTE>(m_effectAmount) * pSprite->GetBaseStats()->m_resistMissileBase / 100;
            if (pSprite->GetBaseStats()->m_resistMissileBase > 100) {
                pSprite->GetBaseStats()->m_resistMissileBase = 100;
            }
            if (pSprite->GetBaseStats()->m_resistMissileBase < 0) {
                pSprite->GetBaseStats()->m_resistMissileBase = 0;
            }
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistMissile = static_cast<SHORT>(m_effectAmount) * pSprite->GetBaseStats()->m_resistMissileBase / 100;
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15142
        UTIL_ASSERT(FALSE);
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillLockPicking::CGameEffectSkillLockPicking(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497520
CGameEffect* CGameEffectSkillLockPicking::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillLockPicking* copy = new CGameEffectSkillLockPicking(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6DB0
BOOL CGameEffectSkillLockPicking::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK]) + static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_OPEN_LOCK] += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] = max(static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_OPEN_LOCK] = max(static_cast<signed char>(m_effectAmount), 0);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_OPEN_LOCK] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15232
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillTraps::CGameEffectSkillTraps(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4975D0
CGameEffect* CGameEffectSkillTraps::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillTraps* copy = new CGameEffectSkillTraps(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B6EE0
BOOL CGameEffectSkillTraps::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_SEARCH] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_SEARCH]) + static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_SEARCH] += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_SEARCH] = max(static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_SEARCH] = max(static_cast<signed char>(m_effectAmount), 0);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_SEARCH] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_SEARCH] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_SEARCH] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_SEARCH] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15326
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillPickPocket::CGameEffectSkillPickPocket(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497680
CGameEffect* CGameEffectSkillPickPocket::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillPickPocket* copy = new CGameEffectSkillPickPocket(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7010
BOOL CGameEffectSkillPickPocket::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET]) + static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_PICK_POCKET] += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET] = max(static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_PICK_POCKET] = max(static_cast<signed char>(m_effectAmount), 0);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_PICK_POCKET] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15420
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillUseMagicDevice::CGameEffectSkillUseMagicDevice(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497730
CGameEffect* CGameEffectSkillUseMagicDevice::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillUseMagicDevice* copy = new CGameEffectSkillUseMagicDevice(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7140
BOOL CGameEffectSkillUseMagicDevice::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE]) + static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = max(static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = max(static_cast<signed char>(m_effectAmount), 0);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15515
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectFatigue::CGameEffectFatigue(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4977E0
CGameEffect* CGameEffectFatigue::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectFatigue* copy = new CGameEffectFatigue(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7270
BOOL CGameEffectFatigue::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_fatigue += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_fatigue > 100) {
                pSprite->GetBaseStats()->m_fatigue = 100;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nFatigue += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_fatigue = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_fatigue > 100) {
                pSprite->GetBaseStats()->m_fatigue = 100;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nFatigue += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetDerivedStats()->m_nFatigue > 100) {
                pSprite->GetDerivedStats()->m_nFatigue = 100;
            }

            if (pSprite->GetDerivedStats()->m_nFatigue < 0) {
                pSprite->GetDerivedStats()->m_nFatigue = 0;
            }

            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_fatigue = pSprite->GetBaseStats()->m_fatigue * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetBaseStats()->m_fatigue > 100) {
                pSprite->GetBaseStats()->m_fatigue = 100;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nFatigue = pSprite->GetBaseStats()->m_fatigue * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetDerivedStats()->m_nFatigue > 100) {
                pSprite->GetDerivedStats()->m_nFatigue = 100;
            }

            if (pSprite->GetDerivedStats()->m_nFatigue < 0) {
                pSprite->GetDerivedStats()->m_nFatigue = 0;
            }

            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15630
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectIntoxication::CGameEffectIntoxication(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497890
CGameEffect* CGameEffectIntoxication::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectIntoxication* copy = new CGameEffectIntoxication(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7440
BOOL CGameEffectIntoxication::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_intoxication += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_intoxication > 100) {
                pSprite->GetBaseStats()->m_intoxication = 100;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nIntoxication += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_intoxication = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetBaseStats()->m_intoxication > 100) {
                pSprite->GetBaseStats()->m_intoxication = 100;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nIntoxication += static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetDerivedStats()->m_nIntoxication > 100) {
                pSprite->GetDerivedStats()->m_nIntoxication = 100;
            }

            if (pSprite->GetDerivedStats()->m_nIntoxication < 0) {
                pSprite->GetDerivedStats()->m_nIntoxication = 0;
            }

            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_intoxication = pSprite->GetBaseStats()->m_intoxication * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetBaseStats()->m_intoxication > 100) {
                pSprite->GetBaseStats()->m_intoxication = 100;
            }

            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nIntoxication = pSprite->GetBaseStats()->m_intoxication * static_cast<BYTE>(m_effectAmount) / 100;

            if (pSprite->GetDerivedStats()->m_nIntoxication > 100) {
                pSprite->GetDerivedStats()->m_nIntoxication = 100;
            }

            if (pSprite->GetDerivedStats()->m_nIntoxication < 0) {
                pSprite->GetDerivedStats()->m_nIntoxication = 0;
            }

            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15742
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSkillTracking::CGameEffectSkillTracking(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497940
CGameEffect* CGameEffectSkillTracking::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSkillTracking* copy = new CGameEffectSkillTracking(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7610
BOOL CGameEffectSkillTracking::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE]) + static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->m_bonusStats.m_nSkills[CGAMESPRITE_SKILL_WILDERNESS_LORE] += static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
        }
        break;
    case 1:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = max(static_cast<signed char>(m_effectAmount), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = max(static_cast<signed char>(m_effectAmount), 0);
            m_done = FALSE;
        }
        break;
    case 2:
        if (m_durationType == 1) {
            // NOTE: Signed compare.
            pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            // NOTE: Signed compare.
            pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = max(static_cast<signed char>(pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] * static_cast<SHORT>(m_effectAmount) / 100), 0);
            m_done = FALSE;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 15831
        UTIL_ASSERT(FALSE);
    }

    // NOTE: Uninline.
    if (pSprite->InControl()) {
        pSprite->m_bSendSpriteUpdate = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectLevel::CGameEffectLevel(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4979F0
CGameEffect* CGameEffectLevel::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectLevel* copy = new CGameEffectLevel(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSTRExtra::CGameEffectSTRExtra(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497AA0
CGameEffect* CGameEffectSTRExtra::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSTRExtra* copy = new CGameEffectSTRExtra(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7740
BOOL CGameEffectSTRExtra::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 15953
    UTIL_ASSERT(FALSE);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRegeneration::CGameEffectRegeneration(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497B50
CGameEffect* CGameEffectRegeneration::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRegeneration* copy = new CGameEffectRegeneration(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDurationCasting::CGameEffectDurationCasting(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497C00
CGameEffect* CGameEffectDurationCasting::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDurationCasting* copy = new CGameEffectDurationCasting(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7930
BOOL CGameEffectDurationCasting::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_nSpellDurationModMage = static_cast<BYTE>(m_effectAmount);
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nSpellDurationModPriest = static_cast<BYTE>(m_effectAmount);
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectProtectionFromCreature::CGameEffectProtectionFromCreature(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497CB0
CGameEffect* CGameEffectProtectionFromCreature::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectProtectionFromCreature* copy = new CGameEffectProtectionFromCreature(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7970
BOOL CGameEffectProtectionFromCreature::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 16143
    UTIL_ASSERT_MSG(FALSE, "CGameEffectProtectionCircle game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmunityToEffect::CGameEffectImmunityToEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497D60
CGameEffect* CGameEffectImmunityToEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunityToEffect* copy = new CGameEffectImmunityToEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B79A0
BOOL CGameEffectImmunityToEffect::ApplyEffect(CGameSprite* pSprite)
{
    ITEM_EFFECT* effect = new ITEM_EFFECT;
    effect->effectID = static_cast<WORD>(m_dwFlags);

    CPoint target(-1, -1);
    CPoint source(-1, -1);

    CGameEffect* pEffect = DecodeEffect(effect, source, -1, target);
    pEffect->m_compareIdOnly = TRUE;

    pSprite->GetDerivedStats()->m_cImmunitiesEffect.AddTail(pEffect);

    delete effect;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmunityToSpellLevel::CGameEffectImmunityToSpellLevel(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497E10
CGameEffect* CGameEffectImmunityToSpellLevel::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunityToSpellLevel* copy = new CGameEffectImmunityToSpellLevel(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7AC0
BOOL CGameEffectImmunityToSpellLevel::ApplyEffect(CGameSprite* pSprite)
{
    if (m_effectAmount < 10) {
        pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel.m_levels[m_effectAmount] = TRUE;
    }

    return true;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectName::CGameEffectName(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497EC0
CGameEffect* CGameEffectName::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectName* copy = new CGameEffectName(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7AE0
BOOL CGameEffectName::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_name = m_effectAmount;

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectXP::CGameEffectXP(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x497F70
CGameEffect* CGameEffectXP::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectXP* copy = new CGameEffectXP(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7B00
BOOL CGameEffectXP::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_xp += m_effectAmount;
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nXP += m_effectAmount;
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_xp = m_effectAmount;
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nXP = m_effectAmount;
            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_xp = pSprite->GetBaseStats()->m_xp * m_effectAmount / 100;
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nXP = pSprite->GetBaseStats()->m_xp * m_effectAmount / 100;
            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 16415
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectGold::CGameEffectGold(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498020
CGameEffect* CGameEffectGold::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectGold* copy = new CGameEffectGold(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7C30
BOOL CGameEffectGold::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_gold += static_cast<SHORT>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nXP += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_gold = static_cast<BYTE>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nXP = static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_gold = pSprite->GetBaseStats()->m_gold * static_cast<SHORT>(m_effectAmount) / 100;
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nXP = pSprite->GetBaseStats()->m_gold * static_cast<SHORT>(m_effectAmount) / 100;
            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 16504
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMoralBreak::CGameEffectMoralBreak(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4980D0
CGameEffect* CGameEffectMoralBreak::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMoralBreak* copy = new CGameEffectMoralBreak(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7D70
BOOL CGameEffectMoralBreak::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_moraleBreak += static_cast<BYTE>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nMoraleBreak += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_moraleBreak = static_cast<BYTE>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nMoraleBreak = static_cast<BYTE>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_moraleBreak = pSprite->GetBaseStats()->m_moraleBreak * static_cast<BYTE>(m_effectAmount) / 100;
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nMoraleBreak = pSprite->GetBaseStats()->m_moraleBreak * static_cast<BYTE>(m_effectAmount) / 100;
            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 16593
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPortrait::CGameEffectPortrait(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498180
CGameEffect* CGameEffectPortrait::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPortrait* copy = new CGameEffectPortrait(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7ED0
BOOL CGameEffectPortrait::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        m_res.GetResRef(pSprite->GetBaseStats()->m_portraitSmall);
        break;
    case 1:
        m_res.GetResRef(pSprite->GetBaseStats()->m_portraitLarge);
        break;
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectReputation::CGameEffectReputation(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498230
CGameEffect* CGameEffectReputation::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectReputation* copy = new CGameEffectReputation(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7F10
BOOL CGameEffectReputation::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_reputation += static_cast<BYTE>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->m_bonusStats.m_nReputation += static_cast<SHORT>(m_effectAmount);
            m_done = FALSE;
            break;
        }
        break;
    case 1:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_reputation = static_cast<BYTE>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nReputation = static_cast<BYTE>(m_effectAmount);

            if (pSprite->GetDerivedStats()->m_nReputation < 0) {
                pSprite->GetDerivedStats()->m_nReputation = 0;
            }

            m_done = FALSE;
            break;
        }
        break;
    case 2:
        switch (m_durationType) {
        case 1:
            pSprite->GetBaseStats()->m_reputation = pSprite->GetBaseStats()->m_reputation * static_cast<BYTE>(m_effectAmount) / 100;
            m_forceRepass = TRUE;
            m_done = TRUE;
            break;
        default:
            pSprite->GetDerivedStats()->m_nReputation = pSprite->GetBaseStats()->m_reputation * static_cast<BYTE>(m_effectAmount) / 100;
            m_done = FALSE;
            break;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 16716
        UTIL_ASSERT(FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4982E0
CGameEffectHoldCreature::CGameEffectHoldCreature(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498320
CGameEffect* CGameEffectHoldCreature::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHoldCreature* copy = new CGameEffectHoldCreature(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B8090
BOOL CGameEffectHoldCreature::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType != 0) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14102); // "Held"
    }

    pSprite->field_9D15 = 1;

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 13);

    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_HELD);
    pSprite->GetDerivedStats()->m_generalState |= STATE_HELPLESS;

    m_done = FALSE;

    return TRUE;
}

// 0x4B5490
void CGameEffectHoldCreature::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRetreatFrom::CGameEffectRetreatFrom(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4983D0
CGameEffect* CGameEffectRetreatFrom::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRetreatFrom* copy = new CGameEffectRetreatFrom(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B8180
BOOL CGameEffectRetreatFrom::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 16918
    UTIL_ASSERT_MSG(FALSE, "CGameEffectRetreatFrom game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCreateWeapon::CGameEffectCreateWeapon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498480
CGameEffect* CGameEffectCreateWeapon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCreateWeapon* copy = new CGameEffectCreateWeapon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDestroyWeapon::CGameEffectDestroyWeapon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498530
CGameEffect* CGameEffectDestroyWeapon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDestroyWeapon* copy = new CGameEffectDestroyWeapon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectEquipWeapon::CGameEffectEquipWeapon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4985E0
CGameEffect* CGameEffectEquipWeapon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectEquipWeapon* copy = new CGameEffectEquipWeapon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDither::CGameEffectDither(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498690
CGameEffect* CGameEffectDither::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDither* copy = new CGameEffectDither(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDispelInvisible::CGameEffectDispelInvisible(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498740
CGameEffect* CGameEffectDispelInvisible::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelInvisible* copy = new CGameEffectDispelInvisible(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2900
BOOL CGameEffectDispelInvisible::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->m_tempStats.m_generalState & STATE_NONDETECTION) == 0) {
        pSprite->GetBaseStats()->m_generalState &= ~STATE_INVISIBLE;
        pSprite->GetDerivedStats()->m_generalState &= ~STATE_INVISIBLE;

        pSprite->GetBaseStats()->m_generalState &= ~STATE_IMPROVEDINVISIBILITY;
        pSprite->GetDerivedStats()->m_generalState &= ~STATE_IMPROVEDINVISIBILITY;

        pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
            CGAMEEFFECT_INVISIBLE,
            pSprite->GetTimedEffectList()->GetPosCurrent(),
            -1);
        pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
            CGAMEEFFECT_INVISIBLE,
            pSprite->GetEquipedEffectList()->GetPosCurrent(),
            -1);
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectShowArea::CGameEffectShowArea(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4987F0
CGameEffect* CGameEffectShowArea::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectShowArea* copy = new CGameEffectShowArea(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B8820
BOOL CGameEffectShowArea::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetArea()->m_visibility.SetAreaExplored();

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectShowCreatures::CGameEffectShowCreatures(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4988A0
CGameEffect* CGameEffectShowCreatures::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectShowCreatures* copy = new CGameEffectShowCreatures(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMirrorImage::CGameEffectMirrorImage(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498950
CGameEffect* CGameEffectMirrorImage::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMirrorImage* copy = new CGameEffectMirrorImage(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B8850
BOOL CGameEffectMirrorImage::ApplyEffect(CGameSprite* pSprite)
{
    if (pSprite->GetDerivedStats()->m_nMirrorImages < 8) {
        if (m_secondaryType != 0) {
            // NOTE: Uninline.
            DisplayStringRef(pSprite, 14774);
        }

        switch (m_dwFlags) {
        case 0:
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_MIRROR_IMAGE);
            if (m_secondaryType != 0) {
                m_effectAmount += rand() % 4 + 1;
            }
            break;
        case 1:
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_REFLECTED_IMAGE);
            if (m_secondaryType != 0) {
                m_effectAmount = 1;
            }
            break;
        }

        pSprite->GetDerivedStats()->m_generalState |= STATE_MIRRORIMAGE;

        m_effectAmount = min(m_effectAmount, 8);

        if (pSprite->GetDerivedStats()->m_nMirrorImages < m_effectAmount) {
            pSprite->GetDerivedStats()->m_nMirrorImages = m_effectAmount;

            if (m_secondaryType != 0) {
                CMessage* message = new CMessageVisualEffect(3,
                    static_cast<BYTE>(pSprite->GetDerivedStats()->m_nMirrorImages),
                    pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        if (m_effectAmount == 0) {
            m_done = TRUE;
        }
    } else {
        m_done = TRUE;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmuneToWeapon::CGameEffectImmuneToWeapon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498AB0
CGameEffect* CGameEffectImmuneToWeapon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmuneToWeapon* copy = new CGameEffectImmuneToWeapon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B8A90
BOOL CGameEffectImmuneToWeapon::ApplyEffect(CGameSprite* pSprite)
{
    CWeaponIdentification* pWeaponId = new CWeaponIdentification();

    switch (m_dwFlags) {
    case 0:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0;
        pWeaponId->m_attributes = m_effectAmount;
        break;
    case 1:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0x40;
        pWeaponId->m_itemFlagMask = 0x40;
        pWeaponId->m_attributes = -1;
        break;
    case 2:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0x40;
        pWeaponId->m_attributes = -1;
        break;
    case 3:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0x100;
        pWeaponId->m_itemFlagMask = 0x100;
        pWeaponId->m_attributes = -1;
        break;
    case 4:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0x100;
        pWeaponId->m_attributes = -1;
        break;
    case 5:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0x100;
        pWeaponId->m_attributes = 0;
        break;
    case 6:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0x2;
        pWeaponId->m_itemFlagMask = 0x2;
        pWeaponId->m_attributes = -1;
        break;
    case 7:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0x2;
        pWeaponId->m_attributes = -1;
        break;
    case 8:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0x10;
        pWeaponId->m_itemFlagMask = 0x10;
        pWeaponId->m_attributes = -1;
        break;
    case 9:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0x10;
        pWeaponId->m_attributes = -1;
        break;
    case 0xA:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0x200;
        pWeaponId->m_itemFlagMask = 0x200;
        pWeaponId->m_attributes = -1;
        break;
    case 0xB:
        pWeaponId->m_itemType = 0;
        pWeaponId->m_itemFlags = 0;
        pWeaponId->m_itemFlagMask = 0x200;
        pWeaponId->m_attributes = -1;
        break;
    }

    pSprite->GetDerivedStats()->m_cImmunitiesWeapon.AddTail(pWeaponId);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectVisualAnimationEffect::CGameEffectVisualAnimationEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498B60
CGameEffect* CGameEffectVisualAnimationEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectVisualAnimationEffect* copy = new CGameEffectVisualAnimationEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCreateItem::CGameEffectCreateItem(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498C10
CGameEffect* CGameEffectCreateItem::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCreateItem* copy = new CGameEffectCreateItem(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDestroyItem::CGameEffectDestroyItem(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498CC0
CGameEffect* CGameEffectDestroyItem::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDestroyItem* copy = new CGameEffectDestroyItem(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectTeleport::CGameEffectTeleport(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498D70
CGameEffect* CGameEffectTeleport::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectTeleport* copy = new CGameEffectTeleport(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectKnock::CGameEffectKnock(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498E20
CGameEffect* CGameEffectKnock::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectKnock* copy = new CGameEffectKnock(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B9670
BOOL CGameEffectKnock::ApplyEffect(CGameSprite* pSprite)
{
    m_done = TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMovementRate::CGameEffectMovementRate(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498ED0
CGameEffect* CGameEffectMovementRate::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMovementRate* copy = new CGameEffectMovementRate(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B9680
BOOL CGameEffectMovementRate::ApplyEffect(CGameSprite* pSprite)
{
    if (pSprite->GetAnimation()->GetMoveScale() != 0) {
        switch (m_dwFlags) {
        case 0:
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() + static_cast<BYTE>(m_effectAmount));
            break;
        case 1:
            pSprite->GetAnimation()->SetMoveScale(static_cast<BYTE>(m_effectAmount));
            break;
        case 2:
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() * static_cast<BYTE>(m_effectAmount) / 100);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
            // __LINE__: 17997
            UTIL_ASSERT(FALSE);
        }
    }

    if (m_secondaryType != 0) {
        CMessage* message = new CMessageDropPath(pSprite->GetId(), pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x498F80
CGameEffectMovementRateWithPortrait::CGameEffectMovementRateWithPortrait(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498FC0
CGameEffect* CGameEffectMovementRateWithPortrait::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMovementRateWithPortrait* copy = new CGameEffectMovementRateWithPortrait(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B9870
BOOL CGameEffectMovementRateWithPortrait::ApplyEffect(CGameSprite* pSprite)
{
    if (pSprite->GetAnimation()->GetMoveScale() != 0) {
        switch (m_dwFlags) {
        case 0:
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() + static_cast<BYTE>(m_effectAmount));
            break;
        case 1:
            pSprite->GetAnimation()->SetMoveScale(static_cast<BYTE>(m_effectAmount));
            break;
        case 2:
            pSprite->GetAnimation()->SetMoveScale(pSprite->GetAnimation()->GetMoveScaleDefault() * static_cast<BYTE>(m_effectAmount) / 100);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
            // __LINE__: 18041
            UTIL_ASSERT(FALSE);
        }
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 41);

    if (m_secondaryType != 0) {
        CMessage* message = new CMessageDropPath(pSprite->GetId(), pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectConfusion::CGameEffectConfusion(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499380
CGameEffect* CGameEffectConfusion::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectConfusion* copy = new CGameEffectConfusion(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BA920
BOOL CGameEffectConfusion::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14782);
    }

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 3);

    pSprite->sub_761650();

    if (m_durationType == 1) {
        pSprite->GetBaseStats()->m_generalState |= STATE_CONFUSED;
        pSprite->GetDerivedStats()->m_generalState |= STATE_CONFUSED;
        pSprite->ClearActions(FALSE);
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_generalState |= STATE_CONFUSED;
    }

    return TRUE;
}

// 0x4B5490
void CGameEffectConfusion::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E05"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectNon_CumulativeAid::CGameEffectNon_CumulativeAid(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499430
CGameEffect* CGameEffectNon_CumulativeAid::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectNon_CumulativeAid* copy = new CGameEffectNon_CumulativeAid(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BAA40
BOOL CGameEffectNon_CumulativeAid::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14122);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_AID)) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_AID, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 57);

        AddColorEffect(pSprite, 50, 50, 50, 30);

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

    return TRUE;
}

// 0x55ECF0
void CGameEffectNon_CumulativeAid::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectNon_CumulativeBless::CGameEffectNon_CumulativeBless(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4994E0
CGameEffect* CGameEffectNon_CumulativeBless::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectNon_CumulativeBless* copy = new CGameEffectNon_CumulativeBless(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BAB70
BOOL CGameEffectNon_CumulativeBless::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14123);

        // NOTE: Uninline.
        RemoveAllOfType(pSprite, ICEWIND_CGAMEEFFECT_BANE, -1);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_BLESS]) {
        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 17);

        AddColorEffect(pSprite, 192, 128, 0, 30);

        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_BLESS] = true;
        pSprite->GetDerivedStats()->m_nTHAC0++;
        pSprite->GetBaseStats()->m_morale++;
    }

    return TRUE;
}

// 0x55ECF0
void CGameEffectNon_CumulativeBless::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectNon_CumulativeChant::CGameEffectNon_CumulativeChant(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499590
CGameEffect* CGameEffectNon_CumulativeChant::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectNon_CumulativeChant* copy = new CGameEffectNon_CumulativeChant(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BACA0
BOOL CGameEffectNon_CumulativeChant::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        // NOTE: Uses `test` (with bounds check).
        if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_BENEFICIAL_CHANT)) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_BENEFICIAL_CHANT, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 18);

            AddColorEffect(pSprite, 192, 128, 0, 30);

            pSprite->GetDerivedStats()->m_nDamageBonus++;
            pSprite->GetDerivedStats()->m_nTHAC0++;
            pSprite->GetDerivedStats()->m_nSaveVSFortitude++;
            pSprite->GetDerivedStats()->m_nSaveVSReflex++;
            pSprite->GetDerivedStats()->m_nSaveVSWill++;
        }
        break;
    case 1:
        // NOTE: Using `operator[]` (without bounds check).
        if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DETRIMENTAL_CHANT]) {
            pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DETRIMENTAL_CHANT, true);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 35);

            AddColorEffect(pSprite, 128, 0, 192, 30);

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
void CGameEffectNon_CumulativeChant::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E03"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectNon_CumulativeDrawUponHolyMight::CGameEffectNon_CumulativeDrawUponHolyMight(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499640
CGameEffect* CGameEffectNon_CumulativeDrawUponHolyMight::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectNon_CumulativeDrawUponHolyMight* copy = new CGameEffectNon_CumulativeDrawUponHolyMight(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BAE10
BOOL CGameEffectNon_CumulativeDrawUponHolyMight::ApplyEffect(CGameSprite* pSprite)
{
    if (m_secondaryType) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, 14125);
    }

    if (!pSprite->GetDerivedStats()->m_spellStates.test(SPLSTATE_DRAW_UPON_HOLY_MIGHT)) {
        pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_DRAW_UPON_HOLY_MIGHT, true);

        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 59);

        if (m_effectAmount > pSprite->GetDerivedStats()->field_128) {
            pSprite->m_bonusStats.m_nSTR += static_cast<SHORT>(m_effectAmount) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_128);
            pSprite->GetDerivedStats()->field_128 = m_effectAmount;
        }

        if (m_effectAmount > pSprite->GetDerivedStats()->field_130) {
            pSprite->m_bonusStats.m_nCON += static_cast<SHORT>(m_effectAmount) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_130);
            pSprite->GetDerivedStats()->field_130 = m_effectAmount;
        }

        if (m_effectAmount > pSprite->GetDerivedStats()->field_12C) {
            pSprite->m_bonusStats.m_nDEX += static_cast<SHORT>(m_effectAmount) - static_cast<SHORT>(pSprite->GetDerivedStats()->field_12C);
            pSprite->GetDerivedStats()->field_12C = m_effectAmount;
        }

        AddColorEffect(pSprite, 128, 128, 128, 30);

        m_done = FALSE;
    }

    return TRUE;
}

// 0x4BAF90
void CGameEffectNon_CumulativeDrawUponHolyMight::OnRemove(CGameSprite* pSprite)
{
    PlaySound(CResRef("EFF_E04"), pSprite);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectNon_CumulativeLuck::CGameEffectNon_CumulativeLuck(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4996F0
CGameEffect* CGameEffectNon_CumulativeLuck::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectNon_CumulativeLuck* copy = new CGameEffectNon_CumulativeLuck(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BAFC0
BOOL CGameEffectNon_CumulativeLuck::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_nLuck += static_cast<BYTE>(m_effectAmount);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPetrification::CGameEffectPetrification()
{
    m_effectID = CGAMEEFFECT_PETRIFICATION;
}

// NOTE: Inlined.
CGameEffectPetrification::CGameEffectPetrification(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4997A0
CGameEffect* CGameEffectPetrification::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPetrification* copy = new CGameEffectPetrification(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BAFE0
BOOL CGameEffectPetrification::ApplyEffect(CGameSprite* pSprite)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene != TRUE) {
        // FIXME: Should be on stack.
        ITEM_EFFECT* effect = new ITEM_EFFECT();
        effect->durationType = 1;
        effect->effectID = CGAMEEFFECT_DEATH;
        effect->targetType = 0;
        effect->spellLevel = 0;
        effect->effectAmount = 0;
        effect->probabilityUpper = 100;
        effect->probabilityLower = 0;
        effect->numDice = 0;
        effect->diceSize = 0;
        effect->savingThrow = 0;
        effect->saveMod = 0;
        effect->special = 0;
        effect->dwFlags = 0x10;
        effect->duration = 0;

        CGameEffect* pEffect = DecodeEffect(effect,
            pSprite->GetPos(),
            pSprite->GetId(),
            CPoint(-1, -1));

        CMessage* message = new CMessageAddEffect(pEffect,
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        delete effect;
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPolymorph::CGameEffectPolymorph(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499850
CGameEffect* CGameEffectPolymorph::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPolymorph* copy = new CGameEffectPolymorph(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectForceVisible::CGameEffectForceVisible(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499900
CGameEffect* CGameEffectForceVisible::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectForceVisible* copy = new CGameEffectForceVisible(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B2990
BOOL CGameEffectForceVisible::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->GetBaseStats()->m_generalState & STATE_IMPROVEDINVISIBILITY) != 0
        || (pSprite->GetDerivedStats()->m_generalState & STATE_IMPROVEDINVISIBILITY) != 0) {
        pSprite->GetBaseStats()->m_visible |= 0x1;
    } else {
        pSprite->GetBaseStats()->m_generalState &= ~STATE_INVISIBLE;
        pSprite->GetDerivedStats()->m_generalState &= ~STATE_INVISIBLE;
    }

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INVISIBLE,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        0);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INVISIBLE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        0);

    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INVISIBLE,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        2);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INVISIBLE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        2);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSetSequence::CGameEffectSetSequence(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4999B0
CGameEffect* CGameEffectSetSequence::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSetSequence* copy = new CGameEffectSetSequence(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BBC00
BOOL CGameEffectSetSequence::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->m_tempStats.m_generalState & STATE_DEAD) == 0) {
        pSprite->SetSequence(static_cast<BYTE>(m_dwFlags));

        m_done = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDisplayString::CGameEffectDisplayString(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499A60
CGameEffect* CGameEffectDisplayString::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDisplayString* copy = new CGameEffectDisplayString(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BBC40
BOOL CGameEffectDisplayString::ApplyEffect(CGameSprite* pSprite)
{
    STRREF str = static_cast<STRREF>(m_effectAmount);
    if (str != -1) {
        // NOTE: Uninline.
        DisplayStringRef(pSprite, str);
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectVisualSpellHit::CGameEffectVisualSpellHit(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499B30
CGameEffect* CGameEffectVisualSpellHit::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectVisualSpellHit* copy = new CGameEffectVisualSpellHit(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BBCB0
BOOL CGameEffectVisualSpellHit::ApplyEffect(CGameSprite* pSprite)
{
    CGameArea* area=pSprite->m_pArea;
    if (!area) { m_done=TRUE; return TRUE; }
    CProjectile* projectile=NULL;
    CString visual,sound,palette;
    // Unmapped selectors retain the incoming sprite pointer in the native
    // argument slot subsequently consumed as the message's WORD type.
    USHORT type=static_cast<USHORT>(reinterpret_cast<DWORD>(pSprite));
    switch (m_dwFlags) {
    case 32: case 33: case 34: {
        auto* boom=new CProjectileSPBOOM;
        boom->SetRange(0,m_dwFlags==32?0x43:m_dwFlags==33?0x41:0x45);
        CPoint point=pSprite->GetPos();
        point.y=static_cast<LONG>(static_cast<DWORD>(point.y)-2);
        boom->AddVisualToArea(area,point);
        m_done=TRUE;
        return TRUE;
    }
    case 35: case 36:
        projectile=new CProjectileCallLightning(CResRef(m_dwFlags==35?"FlmStrk":"HLYMITE"),CResRef(""),
            0xFF,m_dwFlags==35?0:1,0,static_cast<SHORT>(2*CTimerWorld::TIMESCALE_MSEC_PER_SEC));
        projectile->Fire(area,pSprite->m_id,pSprite->m_id,pSprite->GetPos(),projectile->DetermineHeight(pSprite),0);
        m_done=TRUE;
        return TRUE;
    case 37:
        visual="SHAir"; palette="SHAir7"; type=0x93;
        break;
    case 38:
        projectile=new CProjectilePhaseBAM(CString(_T("DDoorH")),CString(_T("")),CString(_T("")),0,0);
        projectile->Fire(area,pSprite->m_id,pSprite->m_id,pSprite->GetPos(),0,0);
        m_done=TRUE;
        return TRUE;
    case 39:
        visual="SPFDEATH"; type=0xCF;
        break;
    default:
        if (m_dwFlags<=30 && (m_dwFlags&3)!=3)
            type=static_cast<USHORT>(0x76+(m_dwFlags>>2)*3+(m_dwFlags&3));
        switch (m_dwFlags&3) {
        case 0: visual="SHAir"; palette="SHAir0"; break;
        case 1: visual="SHEarth"; palette="SHEarth0"; break;
        case 2: visual="SHWater"; palette="SHWater0"; break;
        default: UTIL_ASSERT(FALSE); break;
        }
        if ((m_dwFlags&3)!=3) {
            if ((m_dwFlags&0xFFFFFFFCu)==0) palette="";
            else palette.SetAt(palette.GetLength()-1,static_cast<char>((m_dwFlags>>2)+'0'));
        }
        break;
    }
    projectile=new CProjectileImpactBAM(visual,sound,palette,8,0);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessageFireProjectile(type,pSprite->m_id,pSprite->GetPos(),
        projectile->DetermineHeight(pSprite),pSprite->m_id,pSprite->m_id,0),FALSE);
    projectile->Fire(area,pSprite->m_id,pSprite->m_id,pSprite->GetPos(),projectile->DetermineHeight(pSprite),0);
    m_done=TRUE;
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPortraitIcon::CGameEffectPortraitIcon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499C00
CGameEffect* CGameEffectPortraitIcon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPortraitIcon* copy = new CGameEffectPortraitIcon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BC500
BOOL CGameEffectPortraitIcon::ApplyEffect(CGameSprite* pSprite)
{
    // NOTE: Uninline.
    AddPortraitIcon(pSprite, m_dwFlags);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectReplaceItem::CGameEffectReplaceItem(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499CB0
CGameEffect* CGameEffectReplaceItem::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectReplaceItem* copy = new CGameEffectReplaceItem(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDisableButton::CGameEffectDisableButton(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499D60
CGameEffect* CGameEffectDisableButton::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDisableButton* copy = new CGameEffectDisableButton(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BC560
BOOL CGameEffectDisableButton::ApplyEffect(CGameSprite* pSprite)
{
    unsigned int buttonIndex = static_cast<unsigned int>(m_dwFlags);
    if (buttonIndex < 40) {
        pSprite->GetDerivedStats()->m_disabledButtons[buttonIndex] = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDisableSpellType::CGameEffectDisableSpellType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499E10
CGameEffect* CGameEffectDisableSpellType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDisableSpellType* copy = new CGameEffectDisableSpellType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BC580
BOOL CGameEffectDisableSpellType::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_disabledSpellTypes[0] = TRUE;
        pSprite->GetDerivedStats()->m_disabledSpellTypes[1] = TRUE;
        pSprite->GetDerivedStats()->m_disabledSpellTypes[2] = TRUE;
        break;
    case 1:
        pSprite->GetDerivedStats()->m_disabledSpellTypes[0] = TRUE;
        pSprite->GetDerivedStats()->m_disabledSpellTypes[1] = TRUE;
        break;
    case 2:
        pSprite->GetDerivedStats()->m_disabledSpellTypes[0] = TRUE;
        break;
    case 3:
        pSprite->GetDerivedStats()->m_disabledSpellTypes[1] = TRUE;
        break;
    case 4:
        pSprite->GetDerivedStats()->m_disabledSpellTypes[2] = TRUE;
        break;
    }

    if (g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter() == g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(pSprite->GetId())) {
        g_pBaldurChitin->GetObjectGame()->m_cButtonArray.UpdateState();
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCastSpell::CGameEffectCastSpell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499EC0
CGameEffect* CGameEffectCastSpell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCastSpell* copy = new CGameEffectCastSpell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BC630
BOOL CGameEffectCastSpell::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType target = pSprite->GetAIType();

    // NOTE: Uninline.
    CAIAction action(CAIAction::FORCESPELL, target, 0, 0, 0);

    CString str;
    m_res.CopyToString(str);
    action.SetString1(str);

    CMessageInsertAction* pMessage = new CMessageInsertAction(action,
        pSprite->GetId(),
        m_sourceID);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectLearnSpell::CGameEffectLearnSpell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499F70
CGameEffect* CGameEffectLearnSpell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectLearnSpell* copy = new CGameEffectLearnSpell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCastSpellPoint::CGameEffectCastSpellPoint(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A020
CGameEffect* CGameEffectCastSpellPoint::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCastSpellPoint* copy = new CGameEffectCastSpellPoint(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BCDC0
BOOL CGameEffectCastSpellPoint::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType target = pSprite->GetAIType();

    // NOTE: Uninline.
    CAIAction action(CAIAction::FORCESPELLPOINT, target, 0, 0, 0);

    CString str;
    m_res.CopyToString(str);
    action.SetString1(str);

    action.m_specificID = m_effectAmount;
    action.m_dest = m_target;

    CMessageInsertAction* pMessage = new CMessageInsertAction(action,
        pSprite->GetId(),
        m_sourceID);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectIdentify::CGameEffectIdentify(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A0D0
CGameEffect* CGameEffectIdentify::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectIdentify* copy = new CGameEffectIdentify(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDetectTraps::CGameEffectDetectTraps(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A180
CGameEffect* CGameEffectDetectTraps::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDetectTraps* copy = new CGameEffectDetectTraps(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectReplaceSelf::CGameEffectReplaceSelf(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A230
CGameEffect* CGameEffectReplaceSelf::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectReplaceSelf* copy = new CGameEffectReplaceSelf(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPlayMovie::CGameEffectPlayMovie(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A2E0
CGameEffect* CGameEffectPlayMovie::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPlayMovie* copy = new CGameEffectPlayMovie(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BD540
BOOL CGameEffectPlayMovie::ApplyEffect(CGameSprite* pSprite)
{
    CString sFileName("");

    if (g_pBaldurChitin->m_pEngineProjector->ResolveMovieFileName(m_res, sFileName) == TRUE) {
        g_pBaldurChitin->m_pEngineWorld->ReadyMovie(m_res, FALSE);
    }

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSanctuary::CGameEffectSanctuary(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496340
CGameEffect* CGameEffectSanctuary::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSanctuary* copy = new CGameEffectSanctuary(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BD5E0
BOOL CGameEffectSanctuary::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_visualEffects |= IWD_VFX_SANCTUARY;
    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_SANCTUARY);

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 91);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectEntangle::CGameEffectEntangle(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4963F0
CGameEffect* CGameEffectEntangle::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectEntangle* copy = new CGameEffectEntangle(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BD650
BOOL CGameEffectEntangle::ApplyEffect(CGameSprite* pSprite)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene != TRUE) {
        pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_ENTANGLE] = true;

        if (m_secondaryType) {
            PlaySound(CResRef("CRE_P01"), pSprite);
        }

        if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ENTANGLE]) {
            pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_ENTANGLE] = true;

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 92);

            pSprite->GetDerivedStats()->m_nTHAC0 -= 2;
            AdjustDEX(pSprite, -4);
            Immobilize(pSprite);
        }
    } else {
        m_done = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectWebHold::CGameEffectWebHold(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4964A0
CGameEffect* CGameEffectWebHold::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectWebHold* copy = new CGameEffectWebHold(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BD740
BOOL CGameEffectWebHold::ApplyEffect(CGameSprite* pSprite)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene != TRUE) {
        if (!pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_WEB]) {
            pSprite->GetDerivedStats()->m_visualEffects[IWD_VFX_WEB] = true;

            if (m_secondaryType) {
                PlaySound(CResRef("CRE_M02"), pSprite);
            }

            pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_WEB] = true;

            AdjustDEX(pSprite, -4);

            // NOTE: Uninline
            Immobilize(pSprite);

            // NOTE: Uninline.
            AddPortraitIcon(pSprite, 95);

            // FIXME: Redundant, path already dropped in `Immobilize`.
            if (m_secondaryType) {
                CMessageDropPath* pMessage = new CMessageDropPath(pSprite->GetId(),
                    pSprite->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
    } else {
        m_done = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectGreasePool::CGameEffectGreasePool(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x496550
CGameEffect* CGameEffectGreasePool::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectGreasePool* copy = new CGameEffectGreasePool(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BD920
BOOL CGameEffectGreasePool::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_visualEffects.set(IWD_VFX_GREASE, true);

    if (m_secondaryType) {
        PlaySound(CResRef("CRE_M01"), pSprite);
    }

    pSprite->GetDerivedStats()->m_spellStates.set(SPLSTATE_GREASE, true);

    // NOTE: Uninline.
    AddPortraitIcon(pSprite, 94);

    if (pSprite->GetAnimation()->GetMoveScale() != 0) {
        pSprite->GetAnimation()->SetMoveScale(0);
    }

    if (m_secondaryType) {
        CMessageDropPath* pMessage = new CMessageDropPath(pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMirrorImageRun::CGameEffectMirrorImageRun(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x498A00
CGameEffect* CGameEffectMirrorImageRun::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMirrorImageRun* copy = new CGameEffectMirrorImageRun(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B89E0
BOOL CGameEffectMirrorImageRun::ApplyEffect(CGameSprite* pSprite)
{
    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) == 0) {
        pSprite->GetDerivedStats()->m_generalState |= STATE_MIRRORIMAGE;
        if (pSprite->GetDerivedStats()->m_nMirrorImages != m_effectAmount
            || pSprite->m_bForceVisualEffects) {
            CMessage* message = new CMessageVisualEffect(3,
                static_cast<BYTE>(m_effectAmount),
                pSprite->GetId(),
                pSprite->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (m_effectAmount == 0) {
            m_done = TRUE;
        }
    } else {
        m_done = TRUE;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDispelSanctuary::CGameEffectDispelSanctuary(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A390
CGameEffect* CGameEffectDispelSanctuary::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelSanctuary* copy = new CGameEffectDispelSanctuary(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDA60
BOOL CGameEffectDispelSanctuary::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_SANCTUARY,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_SANCTUARY,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x494920
CGameEffectRemovePanic::CGameEffectRemovePanic(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A440
CGameEffect* CGameEffectRemovePanic::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRemovePanic* copy = new CGameEffectRemovePanic(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDAB0
BOOL CGameEffectRemovePanic::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_generalState &= ~STATE_PANIC;
    pSprite->GetDerivedStats()->m_generalState &= ~STATE_PANIC;

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PANIC,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PANIC,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        36);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        36);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRemoveHold::CGameEffectRemoveHold(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A4F0
CGameEffect* CGameEffectRemoveHold::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRemoveHold* copy = new CGameEffectRemoveHold(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDB50
BOOL CGameEffectRemoveHold::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_HOLDCREATURE,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_HOLDCREATURE,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRestoreMovementRate::CGameEffectRestoreMovementRate(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A5A0
CGameEffect* CGameEffectRestoreMovementRate::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRestoreMovementRate* copy = new CGameEffectRestoreMovementRate(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDBA0
BOOL CGameEffectRestoreMovementRate::ApplyEffect(CGameSprite* pSprite)
{
    // NOTE: Uninline.
    pSprite->GetAnimation()->ResetMoveScale();

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_MOVEMENTRATE_176,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_MOVEMENTRATE_176,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRemoveIntoxication::CGameEffectRemoveIntoxication(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A650
CGameEffect* CGameEffectRemoveIntoxication::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRemoveIntoxication* copy = new CGameEffectRemoveIntoxication(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDC20
BOOL CGameEffectRemoveIntoxication::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetBaseStats()->m_intoxication = 0;
    pSprite->GetDerivedStats()->m_nIntoxication = 0;

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INTOXICATION,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        -1);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_INTOXICATION,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        -1);

    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        5);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        5);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPauseCaster::CGameEffectPauseCaster(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A700
CGameEffect* CGameEffectPauseCaster::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPauseCaster* copy = new CGameEffectPauseCaster(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDCB0
BOOL CGameEffectPauseCaster::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_bCasterHold = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistanceToMagic::CGameEffectResistanceToMagic(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A7B0
CGameEffect* CGameEffectResistanceToMagic::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistanceToMagic* copy = new CGameEffectResistanceToMagic(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x04BDCD0
BOOL CGameEffectResistanceToMagic::ApplyEffect(CGameSprite* pSprite)
{
    if (m_effectAmount > 0) {
        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 28);
    }

    switch (m_dwFlags) {
    case 0:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistMagicBase += static_cast<signed char>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistMagic += static_cast<SHORT>(m_effectAmount);
        }
        break;
    case 1:
        if (m_durationType == 1) {
            pSprite->GetBaseStats()->m_resistMagicBase = static_cast<signed char>(m_effectAmount);
            m_forceRepass = TRUE;
            m_done = TRUE;
        } else {
            pSprite->GetDerivedStats()->m_nResistMagic = static_cast<SHORT>(m_effectAmount);
        }
        break;
    }

    if (pSprite->GetBaseStats()->m_resistMagicBase < 0) {
        pSprite->GetBaseStats()->m_resistMagicBase = 0;
    } else if (pSprite->GetBaseStats()->m_resistMagicBase > 50) {
        pSprite->GetBaseStats()->m_resistMagicBase = 50;
    }

    if (pSprite->GetDerivedStats()->m_nResistMagic < 0) {
        pSprite->GetDerivedStats()->m_nResistMagic = 0;
    } else if (pSprite->GetDerivedStats()->m_nResistMagic > 50) {
        pSprite->GetDerivedStats()->m_nResistMagic = 50;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMissileTHAC0Bonus::CGameEffectMissileTHAC0Bonus(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A860
CGameEffect* CGameEffectMissileTHAC0Bonus::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMissileTHAC0Bonus* copy = new CGameEffectMissileTHAC0Bonus(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDE00
BOOL CGameEffectMissileTHAC0Bonus::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_nMissileTHAC0Bonus = static_cast<BYTE>(m_effectAmount);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDestroySelf::CGameEffectDestroySelf(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A910
CGameEffect* CGameEffectDestroySelf::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDestroySelf* copy = new CGameEffectDestroySelf(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDDE0
BOOL CGameEffectDestroySelf::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->m_removeFromArea = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmunityToPortraitIcon::CGameEffectImmunityToPortraitIcon(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49A9C0
CGameEffect* CGameEffectImmunityToPortraitIcon::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunityToPortraitIcon* copy = new CGameEffectImmunityToPortraitIcon(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B7A10
BOOL CGameEffectImmunityToPortraitIcon::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetEquipedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetEquipedEffectList()->GetPosCurrent(),
        m_dwFlags);
    pSprite->GetTimedEffectList()->RemoveAllOfType(pSprite,
        CGAMEEFFECT_PORTRAITICON,
        pSprite->GetTimedEffectList()->GetPosCurrent(),
        m_dwFlags);

    ITEM_EFFECT* effect = new ITEM_EFFECT;
    effect->effectID = CGAMEEFFECT_PORTRAITICON;
    effect->dwFlags = m_dwFlags;

    CPoint target(-1, -1);
    CPoint source(-1, -1);

    CGameEffect* pEffect = DecodeEffect(effect, source, -1, target);
    pEffect->m_compareIdAndFlagsOnly = TRUE;

    pSprite->GetDerivedStats()->m_cImmunitiesEffect.AddTail(pEffect);

    delete effect;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectDamageVisualEffect::CGameEffectDamageVisualEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AA70
CGameEffect* CGameEffectDamageVisualEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDamageVisualEffect* copy = new CGameEffectDamageVisualEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BDE20
BOOL CGameEffectDamageVisualEffect::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 1:
        pSprite->StartSpriteEffect(0, 0, 128, TRUE);
        break;
    case 2:
        pSprite->StartSpriteEffect(0, 1, 128, TRUE);
        break;
    case 3:
        pSprite->StartSpriteEffect(0, 2, 128, TRUE);
        break;
    case 4:
    case 5:
    case 6:
        pSprite->StartSpriteEffect(1, 0, 128, TRUE);
        break;
    case 7:
        pSprite->StartSpriteEffect(2, 0, 128, TRUE);
        break;
    case 8:
        pSprite->StartSpriteEffect(2, 1, 128, TRUE);
        break;
    case 9:
        pSprite->StartSpriteEffect(2, 2, 128, TRUE);
        break;
    case 10:
        pSprite->StartSpriteEffect(4, 0, 128, TRUE);
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAddInnateAbility::CGameEffectAddInnateAbility(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AB20
CGameEffect* CGameEffectAddInnateAbility::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAddInnateAbility* copy = new CGameEffectAddInnateAbility(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRemoveInnateAbility::CGameEffectRemoveInnateAbility(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49ABD0
CGameEffect* CGameEffectRemoveInnateAbility::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRemoveInnateAbility* copy = new CGameEffectRemoveInnateAbility(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE1B0
BOOL CGameEffectRemoveInnateAbility::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->RemoveSpecialAbility(m_res);
    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectResistanceToPoison::CGameEffectResistanceToPoison(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AC80
CGameEffect* CGameEffectResistanceToPoison::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectResistanceToPoison* copy = new CGameEffectResistanceToPoison(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE1D0
BOOL CGameEffectResistanceToPoison::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        if (pSprite->GetDerivedStats()->m_nResistPoison < m_effectAmount) {
            pSprite->GetDerivedStats()->m_nResistPoison = static_cast<BYTE>(m_effectAmount);
        }
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nResistPoison += static_cast<SHORT>(m_effectAmount);
        break;
    }

    if (pSprite->GetDerivedStats()->m_nResistPoison < 0) {
        pSprite->GetDerivedStats()->m_nResistPoison = 0;
    } else if (pSprite->GetDerivedStats()->m_nResistPoison > 100) {
        pSprite->GetDerivedStats()->m_nResistPoison = 100;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPlaySound::CGameEffectPlaySound(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AD30
CGameEffect* CGameEffectPlaySound::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPlaySound* copy = new CGameEffectPlaySound(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE240
BOOL CGameEffectPlaySound::ApplyEffect(CGameSprite* pSprite)
{
    PlaySound(m_res, pSprite);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectHoldCreatureSpell::CGameEffectHoldCreatureSpell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49ADE0
CGameEffect* CGameEffectHoldCreatureSpell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHoldCreatureSpell* copy = new CGameEffectHoldCreatureSpell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE270
BOOL CGameEffectHoldCreatureSpell::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 21645
    UTIL_ASSERT_MSG(FALSE, "CGameEffectHoldCreatureSpell game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectApplyEffect::CGameEffectApplyEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AE90
CGameEffect* CGameEffectApplyEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectApplyEffect* copy = new CGameEffectApplyEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSelectiveToHitBonus::CGameEffectSelectiveToHitBonus(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AF40
CGameEffect* CGameEffectSelectiveToHitBonus::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSelectiveToHitBonus* copy = new CGameEffectSelectiveToHitBonus(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE600
BOOL CGameEffectSelectiveToHitBonus::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType type(CAIObjectType::ANYONE);

    CSelectiveBonus* pBonus = new CSelectiveBonus;

    switch (m_dwFlags) {
    case 2:
        type.m_nEnemyAlly = static_cast<BYTE>(m_effectAmount);
        break;
    case 3:
        type.m_nGeneral = static_cast<BYTE>(m_effectAmount);
        break;
    case 4:
        type.m_nRace = static_cast<BYTE>(m_effectAmount);
        break;
    case 5:
        type.m_nClass = static_cast<BYTE>(m_effectAmount);
        break;
    case 6:
        type.m_nSpecific = static_cast<BYTE>(m_effectAmount);
        break;
    case 7:
        type.m_nGender = static_cast<BYTE>(m_effectAmount);
        break;
    case 8:
        type.m_nAlignment = static_cast<BYTE>(m_effectAmount);
        break;
    }

    pBonus->m_type.Set(type);
    pBonus->m_bonus = m_effectAmount2;

    pSprite->GetDerivedStats()->m_cToHitBonusList.AddTail(pBonus);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSelectiveDamageBonus::CGameEffectSelectiveDamageBonus(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49AFF0
CGameEffect* CGameEffectSelectiveDamageBonus::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSelectiveDamageBonus* copy = new CGameEffectSelectiveDamageBonus(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE740
BOOL CGameEffectSelectiveDamageBonus::ApplyEffect(CGameSprite* pSprite)
{
    CAIObjectType type(CAIObjectType::ANYONE);

    CSelectiveBonus* pBonus = new CSelectiveBonus;

    switch (m_dwFlags) {
    case 2:
        type.m_nEnemyAlly = static_cast<BYTE>(m_effectAmount);
        break;
    case 3:
        type.m_nGeneral = static_cast<BYTE>(m_effectAmount);
        break;
    case 4:
        type.m_nRace = static_cast<BYTE>(m_effectAmount);
        break;
    case 5:
        type.m_nClass = static_cast<BYTE>(m_effectAmount);
        break;
    case 6:
        type.m_nSpecific = static_cast<BYTE>(m_effectAmount);
        break;
    case 7:
        type.m_nGender = static_cast<BYTE>(m_effectAmount);
        break;
    case 8:
        type.m_nAlignment = static_cast<BYTE>(m_effectAmount);
        break;
    }

    pBonus->m_type.Set(type);
    pBonus->m_bonus = m_effectAmount2;

    pSprite->GetDerivedStats()->m_cDamageBonusList.AddTail(pBonus);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRestrictEquipItem::CGameEffectRestrictEquipItem(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B0A0
CGameEffect* CGameEffectRestrictEquipItem::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRestrictEquipItem* copy = new CGameEffectRestrictEquipItem(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE880
BOOL CGameEffectRestrictEquipItem::ApplyEffect(CGameSprite* pSprite)
{
    CImmunitiesItemEquip* pItemEquip = new CImmunitiesItemEquip;
    pItemEquip->m_error = m_effectAmount;
    pItemEquip->m_res = m_res;

    // FIXME: `pEffect` is not set, which will probably lead to crash, see
    // `CImmunitiesItemEquipList::OnList` and
    // `CImmunitiesItemEquipList::operator=` implementations.

    if (m_dwFlags != 0) {
        pSprite->GetDerivedStats()->m_cImmunitiesItemUse.AddTail(pItemEquip);
    } else {
        pSprite->GetDerivedStats()->m_cImmunitiesItemEquip.AddTail(pItemEquip);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectRestrictEquipItemType::CGameEffectRestrictEquipItemType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B150
CGameEffect* CGameEffectRestrictEquipItemType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRestrictEquipItemType* copy = new CGameEffectRestrictEquipItemType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BE930
BOOL CGameEffectRestrictEquipItemType::ApplyEffect(CGameSprite* pSprite)
{
    CImmunitiesItemTypeEquip* pItemTypeEquip = new CImmunitiesItemTypeEquip;
    pItemTypeEquip->m_error = m_effectAmount;
    pItemTypeEquip->m_type = m_dwFlags;

    // FIXME: `pEffect` is not set, which will probably lead to crash, see
    // `CGameEffectRestrictEquipItemType::OnList` and
    // `CGameEffectRestrictEquipItemType::operator=` implementations.

    if (m_dwFlags != 0) {
        pSprite->GetDerivedStats()->m_cImmunitiesItemTypeUse.AddTail(pItemTypeEquip);
    } else {
        pSprite->GetDerivedStats()->m_cImmunitiesItemTypeEquip.AddTail(pItemTypeEquip);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectApplyEffectEquipItem::CGameEffectApplyEffectEquipItem(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B200
CGameEffect* CGameEffectApplyEffectEquipItem::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectApplyEffectEquipItem* copy = new CGameEffectApplyEffectEquipItem(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectApplyEffectEquipItemType::CGameEffectApplyEffectEquipItemType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B2B0
CGameEffect* CGameEffectApplyEffectEquipItemType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectApplyEffectEquipItemType* copy = new CGameEffectApplyEffectEquipItemType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSetDoNoJump::CGameEffectSetDoNoJump(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B360
CGameEffect* CGameEffectSetDoNoJump::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSetDoNoJump* copy = new CGameEffectSetDoNoJump(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BECF0
BOOL CGameEffectSetDoNoJump::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_bDoNotJump = m_dwFlags;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectJumpToArea::CGameEffectJumpToArea(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B410
CGameEffect* CGameEffectJumpToArea::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectJumpToArea* copy = new CGameEffectJumpToArea(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BED10
BOOL CGameEffectJumpToArea::ApplyEffect(CGameSprite* pSprite)
{
    CString tempStr;
    m_res.CopyToString(tempStr);
    pSprite->JumpToArea(tempStr,
        m_source,
        static_cast<SHORT>(m_dwFlags),
        static_cast<SHORT>(m_effectAmount));

    if (CString(pSprite->GetArea()->GetHeader()->m_areaName) == tempStr) {
        m_done = TRUE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSetLocalVariable::CGameEffectSetLocalVariable(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B4C0
CGameEffect* CGameEffectSetLocalVariable::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSetLocalVariable* copy = new CGameEffectSetLocalVariable(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BEDF0
BOOL CGameEffectSetLocalVariable::ApplyEffect(CGameSprite* pSprite)
{
    CVariable var;
    CString strRes1;
    CString strRes2;
    CString strRes3;
    CString work;

    m_res.CopyToString(strRes1);
    m_res2.CopyToString(strRes2);
    m_res3.CopyToString(strRes3);

    strncpy(var.m_name, m_scriptName, SCRIPTNAME_SIZE);
    var.m_type = static_cast<WORD>(m_flags);
    var.m_dwValue = static_cast<DWORD>(m_dwFlags);
    var.m_intValue = static_cast<LONG>(m_effectAmount);
    var.m_resRefType = static_cast<WORD>(m_effectAmount2);
    var.m_floatValue = static_cast<float>(m_effectAmount3);
    work = strRes1 + strRes2 + strRes3;
    strncpy(var.m_stringValue, work, SCRIPTNAME_SIZE);
    pSprite->GetLocalVariables()->AddKey(var);

    m_done = TRUE;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectAuraCleansing::CGameEffectAuraCleansing(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B570
CGameEffect* CGameEffectAuraCleansing::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectAuraCleansing* copy = new CGameEffectAuraCleansing(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BEFC0
BOOL CGameEffectAuraCleansing::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_bAuraCleansing = m_dwFlags;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectMentalSpeed::CGameEffectMentalSpeed(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B620
CGameEffect* CGameEffectMentalSpeed::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMentalSpeed* copy = new CGameEffectMentalSpeed(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BEFE0
BOOL CGameEffectMentalSpeed::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_nMentalSpeed = static_cast<BYTE>(m_effectAmount);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectPhysicalSpeed::CGameEffectPhysicalSpeed(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B6D0
CGameEffect* CGameEffectPhysicalSpeed::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPhysicalSpeed* copy = new CGameEffectPhysicalSpeed(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF000
BOOL CGameEffectPhysicalSpeed::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_nPhysicalSpeed = static_cast<BYTE>(m_effectAmount);

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectCastingLevelBonus::CGameEffectCastingLevelBonus(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B780
CGameEffect* CGameEffectCastingLevelBonus::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectCastingLevelBonus* copy = new CGameEffectCastingLevelBonus(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF020
BOOL CGameEffectCastingLevelBonus::ApplyEffect(CGameSprite* pSprite)
{
    switch (m_dwFlags) {
    case 0:
        pSprite->GetDerivedStats()->m_nCastingLevelBonusMage = static_cast<BYTE>(m_effectAmount);
        break;
    case 1:
        pSprite->GetDerivedStats()->m_nCastingLevelBonusCleric = static_cast<BYTE>(m_effectAmount);
        break;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x499070
CGameEffectFeatCleave::CGameEffectFeatCleave(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4990B0
CGameEffect* CGameEffectFeatCleave::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectFindFamiliar* copy = new CGameEffectFindFamiliar(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectFindFamiliar::CGameEffectFindFamiliar(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499160
CGameEffect* CGameEffectFindFamiliar::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectFindFamiliar* copy = new CGameEffectFindFamiliar(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectSeeInvisible::CGameEffectSeeInvisible(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B830
CGameEffect* CGameEffectSeeInvisible::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSeeInvisible* copy = new CGameEffectSeeInvisible(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF060
BOOL CGameEffectSeeInvisible::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_bSeeInvisible = m_dwFlags;

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectIgnoreDialogPause::CGameEffectIgnoreDialogPause(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B8E0
CGameEffect* CGameEffectIgnoreDialogPause::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectIgnoreDialogPause* copy = new CGameEffectIgnoreDialogPause(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF080
BOOL CGameEffectIgnoreDialogPause::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 22409
    UTIL_ASSERT_MSG(FALSE, "CGameEffectIgnoreDialogPause game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectHitPointsOnDeath::CGameEffectHitPointsOnDeath(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x499210
CGameEffect* CGameEffectHitPointsOnDeath::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHitPointsOnDeath* copy = new CGameEffectHitPointsOnDeath(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BA6D0
BOOL CGameEffectHitPointsOnDeath::ApplyEffect(CGameSprite* pSprite)
{
    if (!pSprite->Animate()) {
        BYTE rc;
        CGameSprite* pProtagonist;
        ITEM_EFFECT* effect;
        CGameEffect* pEffect;
        CMessage* message;

        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(g_pBaldurChitin->GetObjectGame()->GetProtagonist(),
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pProtagonist),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(pProtagonist->GetNameRef(),
                8536,
                RGB(215, 215, 190),
                RGB(215, 215, 190),
                -1,
                pProtagonist->GetId(),
                pProtagonist->GetId());

            // FIXME: Should be on stack.
            effect = new ITEM_EFFECT();
            effect->effectAmount = 0;
            effect->durationType = 0;
            effect->effectID = CGAMEEFFECT_HITPOINTS;
            effect->targetType = 0;
            effect->spellLevel = 0;
            effect->dwFlags = 0;
            effect->duration = 0;
            effect->probabilityUpper = 100;
            effect->probabilityLower = 0;
            effect->numDice = 0;
            effect->diceSize = 0;
            effect->savingThrow = 0;
            effect->saveMod = 0;
            effect->special = 0;
            effect->effectAmount = m_effectAmount;
            effect->durationType = static_cast<WORD>(m_durationType);

            pEffect = DecodeEffect(effect,
                pSprite->GetPos(),
                pSprite->GetId(),
                CPoint(-1, -1));

            message = new CMessageAddEffect(pEffect,
                pSprite->GetId(),
                pProtagonist->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            delete effect;

            // FIXME: Should be on stack.
            effect = new ITEM_EFFECT();
            effect->effectAmount = 0;
            effect->durationType = 0;
            effect->effectID = CGAMEEFFECT_DAMAGE;
            effect->targetType = 0;
            effect->spellLevel = 0;
            effect->dwFlags = 0;
            effect->duration = 0;
            effect->probabilityUpper = 100;
            effect->probabilityLower = 0;
            effect->numDice = 0;
            effect->diceSize = 0;
            effect->savingThrow = 0;
            effect->saveMod = 0;
            effect->special = 0;
            effect->effectAmount = 0;
            effect->durationType = static_cast<WORD>(m_durationType);

            pEffect = DecodeEffect(effect,
                pSprite->GetPos(),
                pSprite->GetId(),
                CPoint(-1, -1));

            message = new CMessageAddEffect(pEffect,
                pSprite->GetId(),
                pProtagonist->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetProtagonist(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            delete effect;
        }
    } else {
        m_done = FALSE;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectFamiliarBlock::CGameEffectFamiliarBlock(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x4992C0
CGameEffect* CGameEffectFamiliarBlock::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectFamiliarBlock* copy = new CGameEffectFamiliarBlock(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4B9E70
BOOL CGameEffectFamiliarBlock::ApplyEffect(CGameSprite* pSprite)
{
    g_pBaldurChitin->GetObjectGame()->field_4204 = pSprite->Animate();
    m_done = !pSprite->Animate();

    return TRUE;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBounceProjectile::CGameEffectBounceProjectile(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49B990
CGameEffect* CGameEffectBounceProjectile::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceProjectile* copy = new CGameEffectBounceProjectile(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBounceEffect::CGameEffectBounceEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BA40
CGameEffect* CGameEffectBounceEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceEffect* copy = new CGameEffectBounceEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBounceLevel::CGameEffectBounceLevel(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BAF0
CGameEffect* CGameEffectBounceLevel::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceLevel* copy = new CGameEffectBounceLevel(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBounceLevelDecrement::CGameEffectBounceLevelDecrement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BBA0
CGameEffect* CGameEffectBounceLevelDecrement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceLevelDecrement* copy = new CGameEffectBounceLevelDecrement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmunityLevelDecrement::CGameEffectImmunityLevelDecrement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BC50
CGameEffect* CGameEffectImmunityLevelDecrement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunityLevelDecrement* copy = new CGameEffectImmunityLevelDecrement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF0B0
BOOL CGameEffectImmunityLevelDecrement::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 22530
    UTIL_ASSERT_MSG(FALSE, "CGameEffectImmunityLevelDecrement game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBounceSchool::CGameEffectBounceSchool(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BD00
CGameEffect* CGameEffectBounceSchool::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceSchool* copy = new CGameEffectBounceSchool(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectBounceSecondaryType::CGameEffectBounceSecondaryType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BDB0
CGameEffect* CGameEffectBounceSecondaryType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceSecondaryType* copy = new CGameEffectBounceSecondaryType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CGameEffectImmunitySchool::CGameEffectImmunitySchool(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BE60
CGameEffect* CGameEffectImmunitySchool::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunitySchool* copy = new CGameEffectImmunitySchool(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF0E0
BOOL CGameEffectImmunitySchool::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 22603
    UTIL_ASSERT_MSG(FALSE, "CGameEffectImmunitySchool game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49BF10
CGameEffectImmunitySecondaryType::CGameEffectImmunitySecondaryType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49BF50
CGameEffect* CGameEffectImmunitySecondaryType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunitySecondaryType* copy = new CGameEffectImmunitySecondaryType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF110
BOOL CGameEffectImmunitySecondaryType::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 22629
    UTIL_ASSERT_MSG(FALSE, "CGameEffectImmunitySecondaryType game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49C000
CGameEffectImmunitySpell::CGameEffectImmunitySpell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C040
CGameEffect* CGameEffectImmunitySpell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunitySpell* copy = new CGameEffectImmunitySpell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4BF140
BOOL CGameEffectImmunitySpell::Evaluate(CGameSprite* pSprite)
{
    CGameObject* pObject;
    BYTE rc;
    BOOL bResult;

    switch (m_dwFlags) {
    case 0:
        return FALSE;
    case 1:
        return !IcewindMisc::IsUndead(pSprite);
    case 2:
        return IcewindMisc::IsUndead(pSprite);
    case 3:
        return !IcewindMisc::IsImmuneToFire(pSprite);
    case 4:
        return IcewindMisc::IsImmuneToFire(pSprite);
    case 5:
        return !IcewindMisc::IsHumanoid(pSprite);
    case 6:
        return IcewindMisc::IsHumanoid(pSprite);
    case 7:
        return !IcewindMisc::IsAnimal(pSprite);
    case 8:
        return IcewindMisc::IsAnimal(pSprite);
    case 9:
        return !IcewindMisc::IsElemental(pSprite);
    case 10:
        return IcewindMisc::IsElemental(pSprite);
    case 11:
        return !IcewindMisc::IsFungus(pSprite);
    case 12:
        return IcewindMisc::IsFungus(pSprite);
    case 13:
        return !IcewindMisc::IsLarge(pSprite);
    case 14:
        return IcewindMisc::IsLarge(pSprite);
    case 15:
        return !IcewindMisc::IsElf(pSprite);
    case 16:
        return IcewindMisc::IsElf(pSprite);
    case 17:
        return !IcewindMisc::IsUmberhulk(pSprite);
    case 18:
        return IcewindMisc::IsUmberhulk(pSprite);
    case 19:
        return !IcewindMisc::IsHalfElf(pSprite);
    case 20:
        return IcewindMisc::IsHalfElf(pSprite);
    case 21:
        return !IcewindMisc::IsHumanoid(pSprite) && !IcewindMisc::IsAnimal(pSprite);
    case 22:
        return IcewindMisc::IsHumanoid(pSprite) || IcewindMisc::IsAnimal(pSprite);
    case 23:
        return !IcewindMisc::IsBlind(pSprite);
    case 24:
        return IcewindMisc::IsBlind(pSprite);
    case 25:
        return !IcewindMisc::IsImmuneToCold(pSprite);
    case 26:
        return IcewindMisc::IsImmuneToCold(pSprite);
    case 27:
        return !IcewindMisc::IsGolem(pSprite);
    case 28:
        return IcewindMisc::IsGolem(pSprite);
    case 29:
        return !IcewindMisc::IsMinotaur(pSprite);
    case 30:
        return IcewindMisc::IsMinotaur(pSprite);
    case 31:
        return !IcewindMisc::IsUndead(pSprite) && !IcewindMisc::IsFungus(pSprite);
    case 32:
        return IcewindMisc::IsUndead(pSprite) || IcewindMisc::IsFungus(pSprite);
    case 33:
        return !IcewindMisc::IsGood(pSprite);
    case 34:
        return IcewindMisc::IsGood(pSprite);
    case 35:
        return !IcewindMisc::IsNeutral(pSprite);
    case 36:
        return IcewindMisc::IsNeutral(pSprite);
    case 37:
        return !IcewindMisc::IsEvil(pSprite);
    case 38:
        return IcewindMisc::IsEvil(pSprite);
    case 39:
        return !IcewindMisc::IsPaladin(pSprite);
    case 40:
        return IcewindMisc::IsPaladin(pSprite);
    case 41:
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            bResult = !IcewindMisc::IsGoodEvilSame(pSprite, static_cast<CGameSprite*>(pObject));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bResult = FALSE;
        }
        return bResult;
    case 42:
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            bResult = IcewindMisc::IsGoodEvilSame(pSprite, static_cast<CGameSprite*>(pObject));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bResult = FALSE;
        }
        return bResult;
    case 43:
        return pSprite->GetId() != m_sourceID;
    case 44:
        return pSprite->GetId() == m_sourceID;
    case 45:
        return !IcewindMisc::IsAcquatic(pSprite);
    case 46:
        return IcewindMisc::IsAcquatic(pSprite);
    case 47:
        return !IcewindMisc::IsLiving(pSprite);
    case 48:
        return IcewindMisc::IsLiving(pSprite);
    case 49:
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            bResult = !IcewindMisc::AreAllies(pSprite, static_cast<CGameSprite*>(pObject));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bResult = FALSE;
        }
        return bResult;
    case 50:
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            bResult = IcewindMisc::AreAllies(pSprite, static_cast<CGameSprite*>(pObject));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bResult = FALSE;
        }
        return bResult;
    case 51:
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            bResult = !IcewindMisc::AreEnemies(pSprite, static_cast<CGameSprite*>(pObject));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bResult = FALSE;
        }
        return bResult;
    case 52:
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_sourceID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            bResult = IcewindMisc::AreEnemies(pSprite, static_cast<CGameSprite*>(pObject));
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_sourceID,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bResult = FALSE;
        }
        return bResult;
    case 53:
        return !IcewindMisc::IsImmuneToFire(pSprite) && !IcewindMisc::IsImmuneToCold(pSprite);
    case 54:
        return IcewindMisc::IsImmuneToFire(pSprite) || IcewindMisc::IsImmuneToCold(pSprite);
    case 55:
        return !IcewindMisc::sud_585070(pSprite);
    case 56:
        return IcewindMisc::sud_585070(pSprite);
    case 57:
        return !IcewindMisc::IsMale(pSprite);
    case 58:
        return IcewindMisc::IsMale(pSprite);
    case 59:
        return !IcewindMisc::IsLawful(pSprite);
    case 60:
        return IcewindMisc::IsLawful(pSprite);
    case 61:
        return !IcewindMisc::IsChaotic(pSprite);
    case 62:
        return IcewindMisc::IsChaotic(pSprite);
    case 63:
        return TRUE;
    case 64:
    case 65:
        return pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_ORC;
    case 66:
        return !pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DEAFENED];
    case 67:
        return pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_DEAFENED];
    case 68:
        return (pSprite->GetDerivedStats()->m_generalState & STATE_SUMMONED_CREATURE) == 0;
    case 69:
        return (pSprite->GetDerivedStats()->m_generalState & STATE_SUMMONED_CREATURE) != 0;
    case 70:
        return pSprite->GetAIType().GetRace() != CAIOBJECTTYPE_R_184;
    case 71:
        return pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_184;
    case 72:
        return TRUE;
    case 73:
        return TRUE;
    case 74:
        return pSprite->GetDerivedStats()->m_nINT >= m_effectAmount;
    case 75:
        return pSprite->GetDerivedStats()->m_nINT <= m_effectAmount;
    case 76:
        return pSprite->GetDerivedStats()->m_nINT > m_effectAmount;
    case 77:
        return pSprite->GetDerivedStats()->m_nINT < m_effectAmount;
    case 78:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 23134
        UTIL_ASSERT_MSG(FALSE, "CGameEffectImmunitySpell::ApplyEffect - Test for IF_BARD_SKALD is obsolete");
    case 79:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 23137
        UTIL_ASSERT_MSG(FALSE, "CGameEffectImmunitySpell::ApplyEffect - Test for IF_BARD_SKALD is obsolete");
    case 80:
        return !IcewindMisc::IsEnemyNearby(pSprite);
    case 81:
        return IcewindMisc::IsEnemyNearby(pSprite);
    case 82:
        return !(pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_ELF && pSprite->GetAIType().GetSubRace() == CAIOBJECTTYPE_SUBRACE_ELF_DROW);
    case 83:
        return pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_ELF && pSprite->GetAIType().GetSubRace() == CAIOBJECTTYPE_SUBRACE_ELF_DROW;
    case 84:
        return !(pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_DWARF && pSprite->GetAIType().GetSubRace() == CAIOBJECTTYPE_SUBRACE_DWARF_GRAY);
    case 85:
        return pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_DWARF && pSprite->GetAIType().GetSubRace() == CAIOBJECTTYPE_SUBRACE_DWARF_GRAY;
    case 86:
        return !g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDay() && !g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDawn();
    case 87:
        return g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDay() || g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDawn();
    case 88:
        return !(pSprite->GetArea() != NULL && (pSprite->GetArea()->GetHeader()->m_areaType & 0x3) != 0);
    case 89:
        return pSprite->GetArea() != NULL && (pSprite->GetArea()->GetHeader()->m_areaType & 0x3) != 0;
    case 90:
        return pSprite->GetAIType().GetRace() != CAIOBJECTTYPE_R_190;
    case 91:
        return pSprite->GetAIType().GetRace() == CAIOBJECTTYPE_R_190;
    case 92:
        return !IcewindMisc::IsOutsider(pSprite);
    case 93:
        return IcewindMisc::IsOutsider(pSprite);
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
        // __LINE__: 23214
        UTIL_ASSERT(FALSE);
    }
}

// 0x4BFE20
BOOL CGameEffectImmunitySpell::ApplyEffect(CGameSprite* pSprite)
{
    if (Evaluate(pSprite) == TRUE) {
        // The sprite already matches the effect's type predicate, so leave the
        // immunity list untouched and mark the effect spent.
        m_done = TRUE;
    } else {
        TString sResRef;
        sResRef = m_res.GetResRefStr();

        if (sResRef.Length() != 0) {
            pSprite->GetDerivedStats()->m_cImmunitiesSpell.insert(sResRef);

            // Opcode 290 additionally announces the resisted resource by name.
            // This mirrors CGameEffect::FeedBackImmuneToResource (0x4C4330) but
            // is inlined here and probes the key table with bWarningIfMissing =
            // FALSE.
            if (m_secondaryType != 0 && m_effectID == 290) {
                if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(m_sourceRes, 1005, FALSE)) {
                    CItem* pItem = new CItem(m_sourceRes, 1, 0, 0, 0, 0);
                    pSprite->FeedBack(CGameSprite::FEEDBACK_IMMUNE_TO_RESOURCE,
                        0,
                        0,
                        0,
                        pItem->GetIdentifiedName(),
                        0,
                        0);
                    delete pItem;
                } else if (g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(m_sourceRes, 1006, FALSE)) {
                    CSpell* pSpell = new CSpell(m_sourceRes);
                    pSprite->FeedBack(CGameSprite::FEEDBACK_IMMUNE_TO_RESOURCE,
                        0,
                        0,
                        0,
                        pSpell->GetGenericName(),
                        0,
                        0);
                    delete pSpell;
                }
            }
        } else {
            m_done = TRUE;
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49C0F0
CGameEffectBounceSpell::CGameEffectBounceSpell(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C130
CGameEffect* CGameEffectBounceSpell::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceSpell* copy = new CGameEffectBounceSpell(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49C1E0
CGameEffectMinHitPoints::CGameEffectMinHitPoints(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C220
CGameEffect* CGameEffectMinHitPoints::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMinHitPoints* copy = new CGameEffectMinHitPoints(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0100
BOOL CGameEffectMinHitPoints::ApplyEffect(CGameSprite* pSprite)
{
    if (pSprite->GetDerivedStats()->m_nMinHitPoints > 0) {
        m_done = TRUE;
    } else {
        pSprite->GetDerivedStats()->m_nMinHitPoints = m_effectAmount;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49C2D0
CGameEffectPowerWordKill::CGameEffectPowerWordKill(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C310
CGameEffect* CGameEffectPowerWordKill::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPowerWordKill* copy = new CGameEffectPowerWordKill(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0130
BOOL CGameEffectPowerWordKill::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24032
    UTIL_ASSERT_MSG(FALSE, "CGameEffectPowerWordKill game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49C3C0
CGameEffectPowerWordStun::CGameEffectPowerWordStun(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C400
CGameEffect* CGameEffectPowerWordStun::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPowerWordStun* copy = new CGameEffectPowerWordStun(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0160
BOOL CGameEffectPowerWordStun::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24057
    UTIL_ASSERT_MSG(FALSE, "CGameEffectPowerWordStun game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49C4B0
CGameEffectImprisonment::CGameEffectImprisonment(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C4F0
CGameEffect* CGameEffectImprisonment::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImprisonment* copy = new CGameEffectImprisonment(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0190
BOOL CGameEffectImprisonment::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24141
    UTIL_ASSERT_MSG(FALSE, "CGameEffectImprisonment game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49C5A0
CGameEffectFreedom::CGameEffectFreedom(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C5E0
CGameEffect* CGameEffectFreedom::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectFreedom* copy = new CGameEffectFreedom(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C01C0
BOOL CGameEffectFreedom::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24166
    UTIL_ASSERT_MSG(FALSE, "CGameEffectFreedom game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49C690
CGameEffectMaze::CGameEffectMaze(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C6D0
CGameEffect* CGameEffectMaze::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectMaze* copy = new CGameEffectMaze(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C01F0
BOOL CGameEffectMaze::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24214
    UTIL_ASSERT_MSG(FALSE, "CGameEffectMaze game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49C780
CGameEffectSecondaryCastList::CGameEffectSecondaryCastList(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C7C0
CGameEffect* CGameEffectSecondaryCastList::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSecondaryCastList* copy = new CGameEffectSecondaryCastList(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49C870
CGameEffectVisualEffect::CGameEffectVisualEffect(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C8B0
CGameEffect* CGameEffectVisualEffect::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectVisualEffect* copy = new CGameEffectVisualEffect(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49C960
CGameEffectLevelDrain::CGameEffectLevelDrain(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49C9A0
CGameEffect* CGameEffectLevelDrain::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectLevelDrain* copy = new CGameEffectLevelDrain(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49CA50
CGameEffectPowerWordSleep::CGameEffectPowerWordSleep(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49CA90
CGameEffect* CGameEffectPowerWordSleep::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPowerWordSleep* copy = new CGameEffectPowerWordSleep(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0220
BOOL CGameEffectPowerWordSleep::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24337
    UTIL_ASSERT_MSG(FALSE, "CGameEffectPowerWordSleep game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49CB40
CGameEffectStoneSkins::CGameEffectStoneSkins(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, TRUE)
{
}

// 0x49CB80
CGameEffect* CGameEffectStoneSkins::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectStoneSkins* copy = new CGameEffectStoneSkins(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0250
BOOL CGameEffectStoneSkins::ApplyEffect(CGameSprite* pSprite)
{
    if (m_dwFlags == 0) {
        // Stoneskin: a pool of physical-damage reduction.
        if (m_secondaryType != 0) {
            DisplayStringRef(pSprite, 0x323);
        }

        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_STONESKIN] = true;
        AddPortraitIcon(pSprite, 0x2E);

        if (m_secondaryType != 0) {
            DWORD nPool = static_cast<BYTE>(m_casterLevel) * 10;
            if (nPool > 149) {
                nPool = 150;
            }
            m_effectAmount2 = nPool;
        }

        DR_ENTRY entry(10, 5);
        entry.AddDamageType(0);
        entry.AddDamageType(0x100000);
        entry.AddDamageType(0x1000000);
        entry.AddDamageType(0x2000000);
        entry.AddDamageType(0x4000000);
        entry.AddDamageType(0x8000000);
        entry.SetSource(&m_effectAmount2, pSprite, this);
        pSprite->GetDerivedStats()->m_drReductions.push_back(entry);
    } else if (m_dwFlags == 1) {
        // Iron Skins: flat physical resistance plus a stone-skin pool.
        if (m_secondaryType != 0) {
            DisplayStringRef(pSprite, 0x324);
        }

        pSprite->GetDerivedStats()->m_spellStates[SPLSTATE_IRON_SKINS] = true;
        AddPortraitIcon(pSprite, 0x59);

        pSprite->GetDerivedStats()->m_nResistCrushing += 10;
        pSprite->GetDerivedStats()->m_nResistPiercing += 10;
        pSprite->GetDerivedStats()->m_nResistSlashing += 10;

        if (pSprite->GetDerivedStats()->m_nStoneSkins < m_effectAmount) {
            pSprite->GetDerivedStats()->m_nStoneSkins = m_effectAmount;
        }

        CMessage* message = new CMessageVisualEffect(0x17,
            static_cast<BYTE>(pSprite->GetDerivedStats()->m_nStoneSkins),
            pSprite->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    pSprite->SetColorRange(0xE);
    pSprite->GetDerivedStats()->m_activeEffectOpcodes.insert(218);
    m_sourceID = 0;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49CC30
CGameEffectProtectionCircle::CGameEffectProtectionCircle(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49CC70
CGameEffect* CGameEffectProtectionCircle::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectProtectionCircle* copy = new CGameEffectProtectionCircle(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0650
BOOL CGameEffectProtectionCircle::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24445
    UTIL_ASSERT_MSG(FALSE, "CGameEffectProtectionCircle game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49CD20
CGameEffectDispelSchool::CGameEffectDispelSchool(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49CD60
CGameEffect* CGameEffectDispelSchool::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelSchool* copy = new CGameEffectDispelSchool(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49CE10
CGameEffectDispelSecondaryType::CGameEffectDispelSecondaryType(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49CE50
CGameEffect* CGameEffectDispelSecondaryType::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelSecondaryType* copy = new CGameEffectDispelSecondaryType(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49CF00
CGameEffectRandomTeleport::CGameEffectRandomTeleport(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49CF40
CGameEffect* CGameEffectRandomTeleport::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRandomTeleport* copy = new CGameEffectRandomTeleport(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49CFF0
CGameEffectImmunitySchoolDecrement::CGameEffectImmunitySchoolDecrement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D030
CGameEffect* CGameEffectImmunitySchoolDecrement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectImmunitySchoolDecrement* copy = new CGameEffectImmunitySchoolDecrement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0680
BOOL CGameEffectImmunitySchoolDecrement::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24543
    UTIL_ASSERT_MSG(FALSE, "CGameEffectImmunitySchoolDecrement game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D0E0
CGameEffectLevelRestoration::CGameEffectLevelRestoration(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D120
CGameEffect* CGameEffectLevelRestoration::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectLevelRestoration* copy = new CGameEffectLevelRestoration(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49D1D0
CGameEffectRevealMagic::CGameEffectRevealMagic(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D210
CGameEffect* CGameEffectRevealMagic::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectRevealMagic* copy = new CGameEffectRevealMagic(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C06B0
BOOL CGameEffectRevealMagic::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24592
    UTIL_ASSERT_MSG(FALSE, "CGameEffectRevealMagic game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D2C0
CGameEffectSecondaryTypeDecrement::CGameEffectSecondaryTypeDecrement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D300
CGameEffect* CGameEffectSecondaryTypeDecrement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectSecondaryTypeDecrement* copy = new CGameEffectSecondaryTypeDecrement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C06E0
BOOL CGameEffectSecondaryTypeDecrement::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24620
    UTIL_ASSERT_MSG(FALSE, "CGameEffectSecondaryTypeDecrement game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D3B0
CGameEffectBounceSchoolDecrement::CGameEffectBounceSchoolDecrement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D3F0
CGameEffect* CGameEffectBounceSchoolDecrement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceSchoolDecrement* copy = new CGameEffectBounceSchoolDecrement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0710
BOOL CGameEffectBounceSchoolDecrement::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24645
    UTIL_ASSERT_MSG(FALSE, "CGameEffectBounceSchoolDecrement game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D4A0
CGameEffectBounceSecondaryTypeDecrement::CGameEffectBounceSecondaryTypeDecrement(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D4E0
CGameEffect* CGameEffectBounceSecondaryTypeDecrement::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectBounceSecondaryTypeDecrement* copy = new CGameEffectBounceSecondaryTypeDecrement(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0740
BOOL CGameEffectBounceSecondaryTypeDecrement::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24671
    UTIL_ASSERT_MSG(FALSE, "CGameEffectBounceSecondaryTypeDecrement game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D590
CGameEffectDispelSchoolOne::CGameEffectDispelSchoolOne(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D5D0
CGameEffect* CGameEffectDispelSchoolOne::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelSchoolOne* copy = new CGameEffectDispelSchoolOne(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C0770
BOOL CGameEffectDispelSchoolOne::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24696
    UTIL_ASSERT_MSG(FALSE, "CGameEffectDispelSchoolOne game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D680
CGameEffectDispelSecondaryTypeOne::CGameEffectDispelSecondaryTypeOne(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D6C0
CGameEffect* CGameEffectDispelSecondaryTypeOne::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDispelSecondaryTypeOne* copy = new CGameEffectDispelSecondaryTypeOne(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C07A0
BOOL CGameEffectDispelSecondaryTypeOne::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24696
    UTIL_ASSERT_MSG(FALSE, "CGameEffectDispelSecondaryTypeOne game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D770
CGameEffectTimeStop::CGameEffectTimeStop(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D7B0
CGameEffect* CGameEffectTimeStop::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectTimeStop* copy = new CGameEffectTimeStop(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C07D0
BOOL CGameEffectTimeStop::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 24747
    UTIL_ASSERT_MSG(FALSE, "CGameEffectTimeStop game effect not supported in Icewind II (-rjf)");
}

// -----------------------------------------------------------------------------

// 0x49D860
CGameEffectDisguise::CGameEffectDisguise(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
    field_18C = -1;
}

// 0x49D8A0
CGameEffect* CGameEffectDisguise::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectDisguise* copy = new CGameEffectDisguise(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// -----------------------------------------------------------------------------

// 0x49D980
CGameEffectHeroicInspiration::CGameEffectHeroicInspiration(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49D9C0
CGameEffect* CGameEffectHeroicInspiration::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectHeroicInspiration* copy = new CGameEffectHeroicInspiration(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C3E20
BOOL CGameEffectHeroicInspiration::ApplyEffect(CGameSprite* pSprite)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameEffect.cpp
    // __LINE__: 27295
    UTIL_ASSERT(pSprite != NULL);

    if (pSprite->GetBaseStats()->m_hitPoints > pSprite->GetBaseStats()->m_maxHitPointsBase / 2) {
        m_done = TRUE;
        PlaySound(CResRef("EFF_E05"), pSprite);
    } else {
        // NOTE: Uninline.
        AddPortraitIcon(pSprite, 138);

        pSprite->GetDerivedStats()->m_nTHAC0++;
        pSprite->GetDerivedStats()->m_nDamageBonus++;
        pSprite->GetDerivedStats()->m_nSaveVSFortitude++;
        pSprite->GetDerivedStats()->m_nSaveVSReflex++;
        pSprite->GetDerivedStats()->m_nSaveVSWill++;
        m_done = FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x49DA70
CGameEffectPreventAISlowdown::CGameEffectPreventAISlowdown(ITEM_EFFECT* effect, const CPoint& source, LONG sourceID, CPoint target)
    : CGameEffect(effect, source, sourceID, target, FALSE)
{
}

// 0x49DAB0
CGameEffect* CGameEffectPreventAISlowdown::Copy()
{
    ITEM_EFFECT* effect = GetItemEffect();
    CGameEffectPreventAISlowdown* copy = new CGameEffectPreventAISlowdown(effect, m_source, m_sourceID, m_target);
    delete effect;
    copy->CopyFromBase(this);
    return copy;
}

// 0x4C3F10
BOOL CGameEffectPreventAISlowdown::ApplyEffect(CGameSprite* pSprite)
{
    pSprite->GetDerivedStats()->m_bPreventAISlowDown = m_dwFlags;
    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x4AD250
CPersistantEffect::CPersistantEffect()
{
    m_done = FALSE;
    m_deleted = FALSE;
    m_sourceID = -1;
    m_numDamage = 0;
    m_persistantType = 0;
    m_counter = 0;
}

// NOTE: Inlined.
CPersistantEffect::CPersistantEffect(const CPersistantEffect& other)
{
    m_done = other.m_done;
    m_deleted = other.m_deleted;
    m_sourceID = other.m_sourceID;
    m_numDamage = other.m_numDamage;
    m_persistantType = other.m_persistantType;
    m_counter = other.m_counter;
}

// 0x4A7460
CPersistantEffect::~CPersistantEffect()
{
}

// 0x4A7470
void CPersistantEffect::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    // __FILE__: .\Include\CGameEffect.h
    // __LINE__: 3656
    UTIL_ASSERT(FALSE);
}

// 0x4A7440
CPersistantEffect* CPersistantEffect::Copy()
{
    // __FILE__: .\Include\CGameEffect.h
    // __LINE__: 3665
    UTIL_ASSERT_MSG(FALSE, "COPY missing");
}

// 0x4C1080
void CPersistantEffect::PlaySound(const CResRef& res, CGameSprite* pSprite)
{
    CSound cSound;

    CMessage* message = new CMessagePlaySoundRef(res, pSprite->GetId(), pSprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    cSound.SetResRef(res, TRUE, TRUE);

    if (cSound.m_nLooping == 0) {
        cSound.SetFireForget(TRUE);
    }

    cSound.SetChannel(14, reinterpret_cast<DWORD>(pSprite->GetArea()));
    cSound.Play(pSprite->GetPos().x, pSprite->GetPos().y, 0, FALSE);
}

// -----------------------------------------------------------------------------

// 0x4AD280
CPersistantEffectBurningDeath::CPersistantEffectBurningDeath()
{
    m_charredDegree = CVidPalette::NO_TINT;
    m_charredIncrement = 0;
}

// 0x4AD2C0
void CPersistantEffectBurningDeath::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    // __FILE__: .\Include\CGameEffect.h
    // __LINE__: 3681
    UTIL_ASSERT(FALSE);
}

// 0x4AD2E0
CPersistantEffect* CPersistantEffectBurningDeath::Copy()
{
    CPersistantEffectBurningDeath* copy = new CPersistantEffectBurningDeath();
    copy->m_charredDegree = m_charredDegree;
    copy->m_charredIncrement = m_charredIncrement;
    copy->m_counter = m_counter;
    copy->m_deleted = m_deleted;
    copy->m_done = m_done;
    copy->m_numDamage = m_numDamage;
    copy->m_persistantType = m_persistantType;
    copy->m_sourceID = m_sourceID;
    return copy;
}

// -----------------------------------------------------------------------------

// 0x4C1270
void CPersistantEffectFireDeath::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    if (m_counter++ == 0) {
        pSprite->SetSequence(CGameSprite::SEQ_DIE);
        m_numDamage = rand() % 6 + 4;
        PlaySound(CResRef("FIRE"), pSprite);
        pSprite->StartSpriteEffect(1, 0, static_cast<BYTE>(8 * m_numDamage), TRUE);
        pSprite->GetAnimation()->SetColorEffectAll(0,
            RGB(m_charredDegree, m_charredDegree, m_charredDegree),
            1);
        m_done = FALSE;
        m_charredIncrement = (CVidPalette::NO_TINT - 75) / (2 * m_numDamage);
        m_charredDegree -= m_charredIncrement;
    } else {
        if (m_numDamage != 0) {
            pSprite->GetAnimation()->SetColorEffectAll(0,
                RGB(m_charredDegree, m_charredDegree, m_charredDegree),
                1);
            m_charredDegree -= m_charredIncrement;
            if ((m_counter & 1) == 0) {
                m_numDamage--;
            }
        } else {
            pSprite->GetAnimation()->SetColorEffectAll(0, RGB(75, 75, 75), 1);
            m_done = TRUE;
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4C1430
void CPersistantEffectElectricDeath::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    if (m_counter++ == 0) {
        pSprite->SetSequence(CGameSprite::SEQ_DIE);
        m_numDamage = rand() % 6 + 4;
        PlaySound(CResRef("ELEC2"), pSprite);
        pSprite->StartSpriteEffect(2, 0, static_cast<BYTE>(8 * m_numDamage), TRUE);
        pSprite->GetAnimation()->SetColorEffectAll(0,
            RGB(m_charredDegree, m_charredDegree, m_charredDegree),
            1);
        m_done = FALSE;
        m_charredIncrement = (CVidPalette::NO_TINT - 75) / (2 * m_numDamage);
        m_charredDegree -= m_charredIncrement;
    } else {
        if (m_numDamage != 0) {
            pSprite->GetAnimation()->SetColorEffectAll(0,
                RGB(m_charredDegree, m_charredDegree, m_charredDegree),
                1);
            m_charredDegree -= m_charredIncrement;
            if ((m_counter & 1) == 0) {
                m_numDamage--;
            }
        } else {
            pSprite->GetAnimation()->SetColorEffectAll(0, RGB(75, 75, 75), 1);
            m_done = TRUE;
        }
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CPersistantEffectColorEffect::CPersistantEffectColorEffect()
{
    m_persistantType = 2;
    m_undo = FALSE;
    m_counter = 0;
    m_period = 0;
    m_effectDegreeBlue = CVidPalette::NO_TINT;
    m_effectDegreeGreen = CVidPalette::NO_TINT;
    m_effectDegreeRed = CVidPalette::NO_TINT;
    m_colorEffect = 0;
    m_done = FALSE;
    m_effectFinalBlue = 0;
    m_effectFinalGreen = 0;
    m_effectFinalRed = 0;
    m_effectIncrementBlue = 0;
    m_effectIncrementGreen = 0;
    m_effectIncrementRed = 0;
}

// 0x4C15F0
void CPersistantEffectColorEffect::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    if (m_period == 0) {
        m_period = 1;
    }

    if (m_counter++ == 0) {
        pSprite->GetAnimation()->SetColorEffectAll(m_colorEffect,
            RGB(m_effectDegreeRed, m_effectDegreeGreen, m_effectDegreeBlue),
            1);
        m_done = FALSE;

        switch (m_colorEffect) {
        case 0:
            m_effectIncrementRed = (CVidPalette::NO_TINT - m_effectFinalRed) / m_period;
            m_effectDegreeRed -= m_effectIncrementRed;
            m_effectIncrementGreen = (CVidPalette::NO_TINT - m_effectFinalGreen) / m_period;
            m_effectDegreeGreen -= m_effectIncrementGreen;
            m_effectIncrementBlue = (CVidPalette::NO_TINT - m_effectFinalBlue) / m_period;
            m_effectDegreeBlue -= m_effectIncrementBlue;
            break;
        case 1:
        case 2:
            m_effectIncrementRed = m_effectFinalRed / m_period;
            m_effectDegreeRed += m_effectIncrementRed;
            m_effectIncrementGreen = m_effectFinalGreen / m_period;
            m_effectDegreeGreen += m_effectIncrementGreen;
            m_effectIncrementBlue = m_effectFinalBlue / m_period;
            m_effectDegreeBlue += m_effectIncrementBlue;
            break;
        case 3:
            m_effectIncrementRed = m_effectFinalRed / m_period;
            m_effectDegreeRed -= m_effectIncrementRed;
            m_effectIncrementGreen = m_effectFinalGreen / m_period;
            m_effectDegreeGreen -= m_effectIncrementGreen;
            m_effectIncrementBlue = m_effectFinalBlue / m_period;
            m_effectDegreeBlue -= m_effectIncrementBlue;
            break;
        }
    } else {
        if (m_counter < m_period) {
            pSprite->GetAnimation()->SetColorEffectAll(m_colorEffect,
                RGB(m_effectDegreeRed, m_effectDegreeGreen, m_effectDegreeBlue),
                1);

            switch (m_colorEffect) {
            case 0:
            case 3:
                m_effectDegreeRed -= m_effectIncrementRed;
                m_effectDegreeGreen -= m_effectIncrementGreen;
                m_effectDegreeBlue -= m_effectIncrementBlue;
                break;
            default:
                m_effectDegreeRed += m_effectIncrementRed;
                m_effectDegreeGreen += m_effectIncrementGreen;
                m_effectDegreeBlue += m_effectIncrementBlue;
                break;
            }

        } else {
            m_done = TRUE;

            if (m_undo) {
                pSprite->GetAnimation()->ClearColorEffectsAll();
                pSprite->field_562C = 1;
            }
        }
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CPersistantEffectListRegenerated::CPersistantEffectListRegenerated()
{
    m_nCounter = 0;
}

// 0x51E8B0
//
// Tick every live regenerated persistant effect, purging deleted and
// finished entries along the way.  m_nCounter advances on each completed
// pass (empty list included) -- it cadences regeneration; a tick that
// leaves the owner dead aborts the walk without counting.
void CPersistantEffectListRegenerated::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        POSITION posCurrent = pos;
        CPersistantEffect* pEffect = GetNext(pos);

        if (pEffect->m_deleted) {
            RemoveAt(posCurrent);
            delete pEffect;
            continue;
        }

        pEffect->AIUpdate(pSprite, deltaT);

        if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) != 0) {
            return;
        }

        if (pEffect->m_deleted || pEffect->m_done) {
            RemoveAt(posCurrent);
            delete pEffect;
        }
    }

    m_nCounter++;
}

// -----------------------------------------------------------------------------

// 0x4C18A0
CPersistantEffect84C4A4::CPersistantEffect84C4A4(int a1, int a2, int a3)
{
    field_18 = a2;
    field_1C = a3;
    field_24 = a1;
    field_28 = 1;

    LONG delta = static_cast<LONG>(g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime - static_cast<ULONG>(a2));
    if (delta == 0 || delta == 1) {
        field_20 = a2;
    } else {
        field_20 = a2 + a1 * (delta / a1 + 1);
    }
}

// NOTE: Inlined.
CPersistantEffect84C4A4::CPersistantEffect84C4A4(const CPersistantEffect84C4A4& other)
    : CPersistantEffect(other)
{
    field_18 = other.field_18;
    field_1C = other.field_1C;
    field_20 = other.field_20;
    field_24 = other.field_24;
    field_28 = other.field_28;
    memcpy(field_29, other.field_29, sizeof(field_29));
    field_31 = other.field_31;
    field_32 = other.field_32;
    field_33 = other.field_33;
}

// 0x4C1920
void CPersistantEffect84C4A4::AIUpdate(CGameSprite* pSprite, LONG deltaT)
{
    int gameTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
    if (gameTime > field_20
        && (field_1C == 0 || gameTime <= field_1C)) {
        int time = gameTime - deltaT;
        int index = 0;
        while (time < gameTime) {
            time += field_24;
            index++;
        }
        field_20 += index * field_24;
        ApplyEffect(pSprite, index);
    }
}

// -----------------------------------------------------------------------------

// 0x4C1980
CPersistantEffect84C4B4::CPersistantEffect84C4B4(const char* a1, int a2, int a3, int a4, int a5)
    : CPersistantEffect84C4A4(a3, a4, a5)
{
    field_34 = a1;
    field_54 = a2;
}

// NOTE: Inlined.
CPersistantEffect84C4B4::CPersistantEffect84C4B4(const CPersistantEffect84C4B4& other)
    : CPersistantEffect84C4A4(other)
{
    field_34 = other.field_34;
    field_44 = other.field_44;
    field_54 = other.field_54;
    field_58 = other.field_58;
}

// 0x4C1B00
CPersistantEffect84C4B4::~CPersistantEffect84C4B4()
{
}

// 0x4C1C00
void CPersistantEffect84C4B4::ApplyEffect(CGameSprite* pSprite, int index)
{
    // TODO: Incomplete.
}

// 0x4C1F10
CPersistantEffect* CPersistantEffect84C4B4::Copy()
{
    return new CPersistantEffect84C4B4(*this);
}

// -----------------------------------------------------------------------------

// 0x4C2070
CPersistantEffect84C4C4::CPersistantEffect84C4C4(const char* a1, int a2, int a3, int a4)
    : CPersistantEffect84C4A4(a2, a3, a4)
{
    field_34 = a1;
}

// NOTE: Inlined.
CPersistantEffect84C4C4::CPersistantEffect84C4C4(const CPersistantEffect84C4C4& other)
    : CPersistantEffect84C4A4(other)
{
    field_34 = other.field_34;
    field_44 = other.field_44;
}

// 0x4C2160
CPersistantEffect84C4C4::~CPersistantEffect84C4C4()
{
}

// 0x4C21E0
void CPersistantEffect84C4C4::ApplyEffect(CGameSprite* pSprite, int index)
{
    // TODO: Incomplete.
}

// 0x4C2440
CPersistantEffect* CPersistantEffect84C4C4::Copy()
{
    return new CPersistantEffect84C4C4(*this);
}

// -----------------------------------------------------------------------------

// 0x4C2570
CPersistantEffect84C420::CPersistantEffect84C420(int a1, int a2, int a3, int a4, int a5)
    : CPersistantEffect84C4A4(a3, a4, a5)
{
    field_34 = a1;
    field_38 = a2;
}

// NOTE: Inlined.
CPersistantEffect84C420::CPersistantEffect84C420(const CPersistantEffect84C420& other)
    : CPersistantEffect84C4A4(other)
{
    field_34 = other.field_34;
    field_38 = other.field_38;
}

// 0x4C25B0
void CPersistantEffect84C420::ApplyEffect(CGameSprite* pSprite, int index)
{
    CGameEffect* pEffect = new CGameEffectDamage();
    pEffect->m_dwFlags = field_34;
    pEffect->m_sourceID = m_sourceID;
    pEffect->m_effectAmount = index * field_38;
    pSprite->AddEffect(pEffect,
        CGameAIBase::EFFECT_LIST_TIMED,
        TRUE,
        TRUE);
}

// 0x4C27F0
CPersistantEffect* CPersistantEffect84C420::Copy()
{
    return new CPersistantEffect84C420(*this);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CPersistantEffect84C494::CPersistantEffect84C494(int a1, int a2, int a3, int a4)
    : CPersistantEffect84C4A4(a2, a3, a4)
{
    field_34 = a1;
}

// NOTE: Inlined.
CPersistantEffect84C494::CPersistantEffect84C494(const CPersistantEffect84C494& other)
    : CPersistantEffect84C4A4(other)
{
    field_34 = other.field_34;
}

// 0x4C2890
void CPersistantEffect84C494::ApplyEffect(CGameSprite* pSprite, int index)
{
    if (pSprite->GetBaseStats()->m_hitPoints < pSprite->GetDerivedStats()->m_nMaxHitPoints) {
        CGameEffectHeal* pEffect = new CGameEffectHeal();
        pEffect->m_sourceID = m_sourceID;
        pEffect->m_effectAmount = index * field_34;
        pSprite->AddEffect(pEffect,
            CGameAIBase::EFFECT_LIST_TIMED,
            TRUE,
            TRUE);
    }
}

// 0x4C2990
CPersistantEffect* CPersistantEffect84C494::Copy()
{
    return new CPersistantEffect84C494(*this);
}

// -----------------------------------------------------------------------------

// 0x4C2A20
CPersistantEffect84C4D4::CPersistantEffect84C4D4(int a1, int a2, int a3, int a4)
    : CPersistantEffect84C4A4(a2, a3, a4)
{
    field_34 = "";
    field_54 = a1;
}

// NOTE: Inlined.
CPersistantEffect84C4D4::CPersistantEffect84C4D4(const CPersistantEffect84C4D4& other)
    : CPersistantEffect84C4A4(other)
{
    field_34 = other.field_34;
    field_44 = other.field_44;
    field_54 = other.field_54;
    field_58 = other.field_58;
}

// 0x4C2BB0
CPersistantEffect84C4D4::~CPersistantEffect84C4D4()
{
}

// 0x4C2CB0
void CPersistantEffect84C4D4::ApplyEffect(CGameSprite* pSprite, int index)
{
    // TODO: Incomplete.
}

// 0x4C3000
CPersistantEffect* CPersistantEffect84C4D4::Copy()
{
    return new CPersistantEffect84C4D4(*this);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CPersistantEffect84C484::CPersistantEffect84C484(int a1, const char* a2, int a3, int a4, int a5, int a6)
    : CPersistantEffect84C4B4(a2, a3, a4, a5, a6)
{
    field_68 = a1;
}

// NOTE: Inlined.
CPersistantEffect84C484::CPersistantEffect84C484(const CPersistantEffect84C484& other)
    : CPersistantEffect84C4B4(other)
{
    field_68 = other.field_68;
}

// 0x4C3180
CPersistantEffect84C484::~CPersistantEffect84C484()
{
}

// 0x4C3280
void CPersistantEffect84C484::ApplyEffect(CGameSprite* pSprite, int index)
{
    // TODO: Incomplete.
}

// 0x4C3570
CPersistantEffect* CPersistantEffect84C484::Copy()
{
    return new CPersistantEffect84C484(*this);
}
