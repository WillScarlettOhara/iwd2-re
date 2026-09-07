#include "CItem.h"

#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CGameAnimationType.h"
#include "CGameEffect.h"
#include "CGameEffectList.h"
#include "CGameSprite.h"
#include "CImmunities.h"
#include "CInfGame.h"
#include "CUtil.h"

namespace {

const BYTE ITEM_ANIMATION_NONE = 0;
const BYTE ITEM_ANIMATION_ARMOR = 1;
const BYTE ITEM_ANIMATION_HELMET = 2;
const BYTE ITEM_ANIMATION_SHIELD = 3;
const BYTE ITEM_ANIMATION_WEAPON = 4;

BYTE g_emptyColorRangeValues[7] = { 0, 0, 0, 0, 0, 0, 0 };
const WORD g_defaultAttackProbability[6] = { 0x22, 0x21, 0x21, 0, 0, 0 };

CString GetItemAnimationString(const ITEM_HEADER* pHeader)
{
    char szAnimation[3];
    szAnimation[0] = static_cast<char>(pHeader->animationType[0]);
    szAnimation[1] = static_cast<char>(pHeader->animationType[1]);
    szAnimation[2] = '\0';

    return CString(szAnimation);
}

BOOL IsSpecialWeaponAnimation(const CString& sAnimation)
{
    static const char* SPECIAL_WEAPON_ANIMATIONS[] = {
        "S1",
        "FS",
        "S2",
        "AX",
        "BW",
        "CL",
        "FL",
        "WH",
        "HB",
        "MC",
        "MS",
        "SP",
        "SL",
        "CB",
        "DD",
        "QS",
        "SS",
    };

    for (INT index = 0; index < sizeof(SPECIAL_WEAPON_ANIMATIONS) / sizeof(SPECIAL_WEAPON_ANIMATIONS[0]); index++) {
        if (sAnimation.CompareNoCase(SPECIAL_WEAPON_ANIMATIONS[index]) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

BYTE GetItemAnimationType(CItem* pItem, CString& sAnimation, CGameSprite* pSprite)
{
    BYTE nAnimationType = ITEM_ANIMATION_NONE;

    sAnimation = "";

    ITEM_HEADER* pHeader = pItem->pRes->m_pHeader;
    switch (pHeader->itemType) {
    case 0:
        sAnimation = GetItemAnimationString(pHeader);
        if (IsSpecialWeaponAnimation(sAnimation)) {
            nAnimationType = ITEM_ANIMATION_WEAPON;
        }
        break;
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 40:
    case 42:
    case 43:
    case 45:
    case 46:
    case 48:
    case 50:
    case 51:
    case 52:
    case 54:
    case 55:
    case 56:
    case 58:
    case 59:
    case 70:
    case 71:
    case 73:
        break;
    default:
        UTIL_ASSERT(FALSE);
        break;
    case 7:
    case 72:
        nAnimationType = ITEM_ANIMATION_HELMET;
        sAnimation = GetItemAnimationString(pHeader);
        break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 25:
    case 26:
    case 27:
    case 29:
    case 30:
    case 44:
    case 57:
    case 69:
        nAnimationType = ITEM_ANIMATION_WEAPON;
        sAnimation = GetItemAnimationString(pHeader);
        break;
    case 24:
    case 28:
        nAnimationType = ITEM_ANIMATION_WEAPON;
        break;
    case 41:
    case 47:
    case 49:
    case 53:
        nAnimationType = ITEM_ANIMATION_SHIELD;
        sAnimation = GetItemAnimationString(pHeader);
        break;
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
        nAnimationType = ITEM_ANIMATION_ARMOR;
        if (((pHeader->animationType[1] == 'W') && ((pSprite->m_baseStats.m_animationType & 0xF00) != 0x200))
            || ((pHeader->animationType[1] != 'W') && ((pSprite->m_baseStats.m_animationType & 0xF00) == 0x200))) {
            sAnimation = "1";
        } else {
            sAnimation = CString(static_cast<char>(pHeader->animationType[0]), 1);
        }
        break;
    }

    sAnimation.TrimRight();
    sAnimation.TrimLeft();

    return nAnimationType;
}

BOOL RemoveEffectFromList(CGameEffectList* pList, CGameEffect* pEffect)
{
    POSITION pos = pList->GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        CGameEffect* pListEffect = pList->GetNext(pos);
        if (pListEffect != NULL && pListEffect->Compare(*pEffect)) {
            pList->RemoveAt(posOld);
            delete pListEffect;
            return TRUE;
        }
    }

    return FALSE;
}

}

// 0x8D828C
const CString CItem::VALUE("VALUE");

// 0x4E7840
CItem::CItem()
{
    m_flags = 0;
    m_nAbilities = 0;
    m_numSounds = 0;
    m_wear = 0;
    m_useCount1 = 0;
    m_useCount2 = 0;
    m_useCount3 = 0;
}

// 0x4E7900
CItem::CItem(const CItem& item)
{
    m_flags = 0;
    m_nAbilities = 0;
    m_numSounds = 0;
    m_wear = 0;
    m_useCount1 = 0;
    m_useCount2 = 0;
    m_useCount3 = 0;

    *this = item;
    GetAbilityCount();

    if (GetLoreValue() == 0) {
        m_flags |= 0x1;
    }
}

// 0x4E7B60
CItem::CItem(const CCreatureFileItem& item)
{
    CString sResRef(CResRef(const_cast<BYTE*>(item.m_itemId)).GetResRefStr());
    sResRef.MakeUpper();

    m_flags = 0;
    m_nAbilities = 0;
    m_numSounds = 0;
    m_wear = item.m_wear;
    m_useCount1 = item.m_usageCount[0];
    m_useCount2 = item.m_usageCount[1];
    m_useCount3 = item.m_usageCount[2];

    if (sResRef.CompareNoCase("NO_DROP") == 0) {
        sResRef = "";
    }

    if (sResRef.CompareNoCase("MISC07") == 0) {
        if (m_useCount1 == 0) {
            m_useCount1 = 1;
        } else if (m_useCount2 != 0) {
            m_useCount1 = static_cast<WORD>(m_useCount1 + rand() % m_useCount2);
        }
        m_useCount2 = 0;
    }

    SetResRef(CResRef(sResRef), TRUE);

    if (GetMaxStackable() > 1) {
        m_useCount1 = max(m_useCount1, 1);
        m_useCount2 = max(m_useCount2, 1);
        m_useCount3 = max(m_useCount3, 1);
    }

    m_flags = item.m_dynamicFlags;

    GetAbilityCount();

    if (GetLoreValue() == 0) {
        m_flags |= 0x1;
    }

    if ((m_flags & 0x1) == 0) {
        m_flags |= 0x8;
    }
}

// FIXME: `id` should be reference.
//
// 0x4E7E90
CItem::CItem(CResRef id, WORD useCount1, WORD useCount2, WORD useCount3, int wear, DWORD flags)
{
    CString sResRef;
    id.CopyToString(sResRef);
    sResRef.MakeUpper();

    m_useCount1 = useCount1;
    m_useCount2 = useCount2;
    m_useCount3 = useCount3;

    if (sResRef.CompareNoCase("NO_DROP") == 0) {
        sResRef = "";
    }

    if (sResRef.CompareNoCase("MISC07") == 0) {
        if (m_useCount1 == 0) {
            m_useCount1 = 1;
        } else if (m_useCount2 != 0) {
            m_useCount1 = static_cast<WORD>(m_useCount1 + rand() % m_useCount2);
        }
        m_useCount2 = 0;
    }

    // Ghidra 0x4E7E90: constructor must bind the ITM resource before
    // querying stack size/lore/abilities.  Without this, imported CRE/CHR
    // equipment exists but has empty cResRef, so inventory/action-bar icons
    // and weight all disappear.
    SetResRef(CResRef(sResRef), TRUE);

    // NOTE: Uninline.
    if (GetMaxStackable() > 1) {
        m_useCount1 = max(m_useCount1, 1);
        m_useCount2 = max(m_useCount2, 1);
        m_useCount3 = max(m_useCount3, 1);
    }

    m_flags = flags;
    m_wear = wear;

    // NOTE: Uninline.
    GetAbilityCount();

    // NOTE: Uninline.
    if (GetLoreValue() == 0) {
        m_flags |= 0x1;
    }

    if ((m_flags & 0x1) == 0) {
        m_flags |= 0x8;
    }
}

// 0x4E8180
CItem::~CItem()
{
    // TODO: Incomplete.
}

// 0x4E8240
CCreatureFileItem CItem::GetItemFile()
{
    CCreatureFileItem temp;

    cResRef.GetResRef(temp.m_itemId);
    temp.m_dynamicFlags = m_flags;
    temp.m_usageCount[0] = m_useCount1;
    temp.m_usageCount[1] = m_useCount2;
    temp.m_usageCount[2] = m_useCount3;
    temp.m_wear = m_wear;

    return temp;
}

// 0x4E82B0
BOOL CItem::Demand()
{
    if (cResRef == "") {
        return FALSE;
    }

    if (pRes == NULL) {
        return FALSE;
    }

    pRes->Demand();

    return pRes != NULL;
}

// 0x4E82F0
BOOL CItem::Release()
{
    if (pRes == NULL) {
        return FALSE;
    }

    pRes->Release();

    return TRUE;
}

// 0x4E8310
BOOL CItem::ReleaseAll()
{
    if (pRes == NULL) {
        return FALSE;
    }

    while (pRes->GetDemands() > 0) {
        pRes->Release();
    }

    return TRUE;
}

// 0x4E8350
void CItem::SetResRef(const CResRef& cNewResRef, BOOL bSetAutoRequest)
{
    CResHelper<CResItem, 1005>::SetResRef(cNewResRef, bSetAutoRequest, TRUE);
}

// 0x4E8440
INT CItem::GetAbilityCount()
{
    if (cResRef == "") {
        m_nAbilities = 0;
    } else {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 810
            UTIL_ASSERT(pRes->Demand());

            m_nAbilities = pRes->m_pHeader->abilityCount;
            pRes->Release();
        }
    }

    return m_nAbilities;
}

// 0x4E85F0
WORD CItem::GetMaxUsageCount(INT nAbility)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 904
    UTIL_ASSERT(pRes->Demand());

    // NOTE: Uninline.
    GetAbilityCount();

    WORD nMaxUsageCount = 0;
    if (nAbility >= 0 && nAbility < m_nAbilities) {
        // NOTE: Uninline.
        if (GetMaxStackable() <= 1) {
            // NOTE: Uninline.
            ITEM_ABILITY* ability = GetAbility(nAbility);
            if (ability != NULL) {
                nMaxUsageCount = ability->maxUsageCount;
            }
        }
    }

    pRes->Release();
    return nMaxUsageCount;
}

// 0x4E84B0
WORD CItem::GetUsageCount(INT nAbility)
{
    // NOTE: Uninline.
    GetAbilityCount();

    if (nAbility < 0) {
        return 0;
    }

    if (nAbility < m_nAbilities) {
        // NOTE: Uninline.
        // NOTE: Not used.
        GetMaxStackable();

        switch (nAbility) {
        case 1:
            return m_useCount2;
        case 2:
            return m_useCount3;
        default:
            return m_useCount1;
        }
    }

    if (nAbility == 0) {
        // NOTE: Uninline.
        if (GetMaxStackable() > 1) {
            return m_useCount1;
        }
    }

    return 0;
}

// 0x4E8760
void CItem::SetUsageCount(INT nAbility, WORD nUseCount)
{
    // NOTE: Uninline.
    GetAbilityCount();

    if (nAbility < 0) {
        return;
    }

    if (nAbility < m_nAbilities) {
        switch (nAbility) {
        case 1:
            m_useCount2 = nUseCount;
            return;
        case 2:
            m_useCount3 = nUseCount;
            return;
        default:
            m_useCount1 = nUseCount;
            return;
        }
    }

    if (nAbility == 0) {
        // NOTE: Uninline.
        if (GetMaxStackable() > 1) {
            m_useCount1 = nUseCount;
        }
    }
}

// 0x4E8860
void CItem::Equip(CGameSprite* pSprite, LONG slotNum, BOOL animationOnly)
{
    if (pSprite == NULL || cResRef == "") {
        return;
    }

    BOOL bInEquip = pSprite->field_7544;
    if (!bInEquip) {
        pSprite->field_7540 = 0;
        pSprite->field_7544 = 1;
    }

    if (pRes != NULL) {
        UTIL_ASSERT(pRes->Demand());

        CString sAnimation;
        if (pSprite->GetAnimation()->m_animation != NULL) {
            switch (GetItemAnimationType(this, sAnimation, pSprite)) {
            case ITEM_ANIMATION_ARMOR:
                if (sAnimation == "") {
                    pSprite->GetAnimation()->m_animation->EquipArmor('1', pSprite->m_baseStats.m_colors);
                } else {
                    pSprite->GetAnimation()->m_animation->EquipArmor(sAnimation[0], pSprite->m_baseStats.m_colors);
                }
                break;
            case ITEM_ANIMATION_HELMET:
                pSprite->GetAnimation()->m_animation->EquipHelmet(sAnimation, pSprite->m_baseStats.m_colors);
                break;
            case ITEM_ANIMATION_SHIELD:
                pSprite->GetAnimation()->m_animation->EquipShield(sAnimation, pSprite->m_baseStats.m_colors);
                break;
            case ITEM_ANIMATION_WEAPON:
                if (slotNum == 43 || slotNum == 45 || slotNum == 47 || slotNum == 49 || slotNum == 42) {
                    WORD nAbility = pSprite->m_equipment.m_selectedWeaponAbility;
                    ITEM_ABILITY* pAbility = pRes->GetAbility(nAbility);
                    if (pAbility != NULL && pAbility->type != 0) {
                        pSprite->GetAnimation()->m_animation->EquipWeapon(sAnimation,
                            pSprite->m_baseStats.m_colors,
                            pRes->m_pHeader->itemFlags,
                            pAbility->attackProbability);
                    } else {
                        UTIL_ASSERT(FALSE);
                    }
                } else if (slotNum == 44 || slotNum == 46 || slotNum == 48 || slotNum == 50) {
                    sAnimation += "O";

                    GetAbilityCount();

                    WORD nAbility = pSprite->m_equipment.m_selectedWeaponAbility;
                    ITEM_ABILITY* pAbility;
                    if (nAbility < m_nAbilities) {
                        pAbility = pRes->GetAbility(nAbility);
                    } else {
                        pAbility = pRes->GetAbility(0);
                    }

                    if (pAbility != NULL && pAbility->type != 0) {
                        pSprite->GetAnimation()->m_animation->EquipWeapon(sAnimation,
                            pSprite->m_baseStats.m_colors,
                            pRes->m_pHeader->itemFlags | 0x400,
                            pAbility->attackProbability);
                    } else {
                        UTIL_ASSERT(FALSE);
                    }
                }
                break;
            }
        }

        if (!animationOnly) {
            WORD nStart = pRes->m_pHeader->equipedStartingEffect;
            WORD nCount = pRes->m_pHeader->equipedEffectCount;
            for (WORD nEffect = 0; nEffect < nCount; nEffect++) {
                ITEM_EFFECT* pItemEffect = &(pRes->m_pEffects[nStart + nEffect]);
                CGameEffect* pEffect = CGameEffect::DecodeEffect(pItemEffect,
                    CPoint(-1, -1),
                    pSprite->GetId(),
                    CPoint(-1, -1));
                if (pEffect != NULL) {
                    pEffect->field_188 = 1;
                    pEffect->m_sourceID = pSprite->GetId();
                    pEffect->m_flags |= 0x2;
                    pEffect->m_casterLevel = 10;
                    pEffect->m_sourceRes = cResRef;
                    pEffect->m_sourceType = 2;

                    BYTE list = pEffect->m_durationType == 2
                        ? CGameAIBase::EFFECT_LIST_EQUIPED
                        : CGameAIBase::EFFECT_LIST_TIMED;
                    pSprite->AddEffect(pEffect, list, FALSE, TRUE);
                }
            }
        }

        pRes->Release();
    }

    if (!bInEquip) {
        if (pSprite->field_7540 != 0) {
            Equip(pSprite, slotNum, TRUE);
        }
        pSprite->field_7544 = 0;
    }
}

// 0x4E8DF0
void CItem::Unequip(CGameSprite* pSprite, LONG slotNum, BOOL recalculateEffects, BOOL animationOnly)
{
    if (pSprite == NULL || cResRef == "" || pRes == NULL) {
        return;
    }

    UTIL_ASSERT(pRes->Demand());

    if (!animationOnly) {
        WORD nStart = pRes->m_pHeader->equipedStartingEffect;
        WORD nCount = pRes->m_pHeader->equipedEffectCount;
        for (WORD nEffect = 0; nEffect < nCount; nEffect++) {
            ITEM_EFFECT* pItemEffect = &(pRes->m_pEffects[nStart + nEffect]);
            CGameEffect* pEffect = CGameEffect::DecodeEffect(pItemEffect,
                CPoint(-1, -1),
                -1,
                CPoint(-1, -1));
            if (pEffect != NULL) {
                pEffect->m_sourceID = pSprite->GetId();
                pEffect->m_sourceRes = cResRef;
                if (pEffect->m_durationType == 2) {
                    RemoveEffectFromList(pSprite->GetEquipedEffectList(), pEffect);
                }
                delete pEffect;
            }
        }

        if (nCount > 0) {
            pSprite->field_562C = 1;
            if (recalculateEffects
                && g_pBaldurChitin != NULL
                && g_pBaldurChitin->GetObjectGame() != NULL
                && !g_pBaldurChitin->GetObjectGame()->m_bInLoadGame) {
                pSprite->sub_72DE60();
            }
        }
    }

    CString sAnimation;
    if (pSprite->GetAnimation()->m_animation != NULL) {
        switch (GetItemAnimationType(this, sAnimation, pSprite)) {
        case ITEM_ANIMATION_ARMOR:
            pSprite->GetAnimation()->m_animation->EquipArmor('1', pSprite->m_baseStats.m_colors);
            break;
        case ITEM_ANIMATION_HELMET:
            sAnimation = "";
            pSprite->GetAnimation()->m_animation->EquipHelmet(sAnimation, g_emptyColorRangeValues);
            break;
        case ITEM_ANIMATION_SHIELD:
            sAnimation = "";
            pSprite->GetAnimation()->m_animation->EquipShield(sAnimation, g_emptyColorRangeValues);
            break;
        case ITEM_ANIMATION_WEAPON:
            sAnimation = "";
            if (slotNum == (pSprite->GetWeaponSlot() & 0xFF)) {
                pSprite->GetAnimation()->m_animation->EquipWeapon(sAnimation,
                    g_emptyColorRangeValues,
                    0x400,
                    g_defaultAttackProbability);
                pSprite->GetAnimation()->m_animation->EquipShield(sAnimation, g_emptyColorRangeValues);
            } else {
                pSprite->GetAnimation()->m_animation->EquipWeapon(sAnimation,
                    g_emptyColorRangeValues,
                    0,
                    g_defaultAttackProbability);
            }
            break;
        }
    }

    pRes->Release();
}

// 0x4E91F0
WORD CItem::GetAnimationType()
{
    if (cResRef == "") {
        return NULL;
    }

    if (pRes == NULL) {
        return NULL;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1268
    UTIL_ASSERT(pRes->Demand());

    WORD animationType = pRes->m_pHeader->animationType[1] | (pRes->m_pHeader->animationType[0] << 8);
    pRes->Release();

    return animationType;
}

// 0x4E9610
ITEM_ABILITY* CItem::GetAbility(INT nAbility)
{
    if (cResRef == "") {
        return NULL;
    }

    if (pRes == NULL) {
        return NULL;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1478
    UTIL_ASSERT(pRes->Demand());

    ITEM_ABILITY* ability = pRes->GetAbility(nAbility);
    if (ability == NULL) {
        return NULL;
    }

    if (ability->type == 0) {
        return NULL;
    }

    return ability;
}

// 0x4E9680
CGameEffect* CItem::GetAbilityEffect(LONG abilityNum, LONG effectNum, CGameObject* pObject)
{
    if (cResRef == "") {
        return NULL;
    }

    if (pRes == NULL) {
        return NULL;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1521
    UTIL_ASSERT(pRes->Demand());

    if (effectNum >= pRes->GetAbilityEffectNo(abilityNum)) {
        pRes->Release();
        return NULL;
    }

    CGameEffect* pEffect = CGameEffect::DecodeEffect(pRes->GetAbilityEffect(abilityNum, effectNum),
        CPoint(-1, -1),
        -1,
        CPoint(-1, -1));
    if (pEffect != NULL) {
        // NOTE: Uninline.
        ITEM_ABILITY* ability = GetAbility(abilityNum);
        if (ability != NULL) {
            pEffect->m_school = ability->school;
            pEffect->m_sourceType = 2;
            // FIXME: Should it be `m_secondaryType`?
            pEffect->field_48 = ability->secondaryType;
            pEffect->m_sourceRes = cResRef;
            pEffect->m_sourceFlags = ability->abilityFlags;
            pEffect->m_casterLevel = 10;
        }
    }

    pRes->Release();
    return pEffect;
}

// 0x4E97E0
WORD CItem::GetItemType()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1595
    UTIL_ASSERT(pRes->Demand());

    WORD nType = pRes->m_pHeader->itemType;
    pRes->Release();

    return nType;
}

// 0x4EA3F0
INT CItem::GetMaxEffectSpellLevel()
{
    DWORD nMaxLevel = 0;

    for (DWORD i = 0; i < (DWORD)m_nAbilities; i++) {
        if (cResRef != "" && pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 1478
            UTIL_ASSERT(pRes->GetDemands() > 0);

            ITEM_ABILITY* pAbility = pRes->GetAbility(i);
            if (pAbility != NULL && pAbility->type != 0) {
                for (INT j = 0;; j++) {
                    CGameEffect* pEffect = GetAbilityEffect(i, j, NULL);
                    if (pEffect == NULL) {
                        break;
                    }

                    if (pEffect->m_spellLevel > nMaxLevel) {
                        nMaxLevel = pEffect->m_spellLevel;
                    }
                }
            }
        }
    }

    return (INT)nMaxLevel;
}

// 0x4E9840
SHORT CItem::GetCriticalRangeBonus(const ITEM_ABILITY* pAbility)
{
    SHORT nBonus = 0;
    switch (GetItemType()) {
    case 16:
    case 19:
    case 20:
    case 23:
    case 27:
    case 31:
    case 57:
    case 69:
        nBonus = 1;
        break;
    }
    if (pAbility != NULL && (pAbility->abilityFlags & 0x20000) != 0) {
        nBonus++;
    }
    return nBonus;
}

// 0x4E9910
DWORD CItem::GetCriticalHitMultiplier()
{
    // NOTE: Uninline.
    WORD nType = GetItemType();
    switch (nType) {
    case 5:
    case 15:
    case 21:
    case 25:
    case 29:
    case 30:
        return 3;
    default:
        return 2;
    }
}

// 0x4E99B0
DWORD CItem::GetWeight()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1688
    UTIL_ASSERT(pRes->Demand());

    DWORD nWeight = pRes->m_pHeader->baseWeight;
    pRes->Release();

    // NOTE: Uninline.
    if (GetMaxStackable() > 1) {
        nWeight *= max(m_useCount1, 1);
    }

    return nWeight;
}

// 0x4E9A80
CResRef CItem::GetUsedUpItemId()
{
    CResRef usedResRef("");

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 1732
            UTIL_ASSERT(pRes->Demand());

            usedResRef = pRes->m_pHeader->usedUpItemID;
            pRes->Release();
        }
    }

    return usedResRef;
}

// 0x4E9B10
STRREF CItem::GetGenericName()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1799
    UTIL_ASSERT(pRes->Demand());

    STRREF strName;
    if ((m_flags & 0x1) != 0) {
        strName = pRes->m_pHeader->identifiedName;
    } else {
        strName = pRes->m_pHeader->genericName;
    }

    pRes->Release();

    return strName;
}

// 0x4E9B80
STRREF CItem::GetIdentifiedName()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1799
    UTIL_ASSERT(pRes->Demand());

    STRREF strName = pRes->m_pHeader->identifiedName;
    pRes->Release();

    return strName;
}

// 0x4E9BE0
DWORD CItem::GetFlagsFile()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1840
    UTIL_ASSERT(pRes->Demand());

    DWORD dwFlags = pRes->m_pHeader->itemFlags;
    pRes->Release();

    return dwFlags;
}

// 0x4E9C40
DWORD CItem::GetNotUsableBy()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1879
    UTIL_ASSERT(pRes->Demand());

    DWORD dwNotUsableBy = pRes->m_pHeader->notUsableBy;
    pRes->Release();

    return dwNotUsableBy;
}

// 0x4E9CA0
DWORD CItem::GetNotUsableBy2()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 1920
    UTIL_ASSERT(pRes->Demand());

    BYTE bNotUsableBy2a = pRes->m_pHeader->notUsableBy2a;
    BYTE bNotUsableBy2b = pRes->m_pHeader->notUsableBy2b;
    BYTE bNotUsableBy2c = pRes->m_pHeader->notUsableBy2c;
    BYTE bNotUsableBy2d = pRes->m_pHeader->notUsableBy2d;
    pRes->Release();

    return bNotUsableBy2d | (bNotUsableBy2c << 8) | (bNotUsableBy2b << 16) | (bNotUsableBy2a << 24);
}

// 0x4E9D40
CResRef CItem::GetGroundIcon()
{
    CResRef iconResRef("");

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 1973
            UTIL_ASSERT(pRes->Demand());

            iconResRef = pRes->m_pHeader->groundIcon;
            pRes->Release();
        }
    }

    if (iconResRef == "") {
        iconResRef = "gsack01";
    }

    return iconResRef;
}

// 0x4E9DF0
CResRef CItem::GetItemIcon()
{
    CResRef iconResRef("");

    if (cResRef != "") {
        if (IsBadReadPtr(pRes, 0x60)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2019
            UTIL_ASSERT_MSG(FALSE, "Bad item!");
        }

        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2023
            UTIL_ASSERT(pRes->Demand());

            iconResRef = pRes->m_pHeader->itemIcon;
            pRes->Release();
        }
    }

    return iconResRef;
}
// 0x4E9EB0
WORD CItem::GetMaxStackable()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2061
    UTIL_ASSERT(pRes->Demand());

    WORD nMaxStack = pRes->m_pHeader->maxStackable;
    pRes->Release();

    return nMaxStack;
}

// 0x4E9F10
DWORD CItem::GetBaseValue()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2099
    UTIL_ASSERT(pRes->Demand());

    DWORD nBaseValue = pRes->m_pHeader->baseValue;
    pRes->Release();

    return nBaseValue;
}

// 0x4E9F70
WORD CItem::GetLoreValue()
{
    if (cResRef == "") {
        return 0;
    }

    if (pRes == NULL) {
        return 0;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2140
    UTIL_ASSERT(pRes->Demand());

    WORD nLoreValue = pRes->m_pHeader->loreValue;
    pRes->Release();

    return nLoreValue;
}

// NOTE: Convenience.
INT CItem::GetEquippedACBonus()
{
    // FIXME: Temporary fallback until equipped effects/AddEffect are fully
    // reconstructed.  ITM equipped effect opcode 0 is the AC bonus used by
    // starter armor such as 00LEAT01.
    if (cResRef == "" || pRes == NULL) {
        return 0;
    }

    UTIL_ASSERT(pRes->Demand());

    INT nBonus = 0;
    WORD nStart = pRes->m_pHeader->equipedStartingEffect;
    WORD nCount = pRes->m_pHeader->equipedEffectCount;
    for (WORD nEffect = 0; nEffect < nCount; nEffect++) {
        ITEM_EFFECT* pEffect = &(pRes->m_pEffects[nStart + nEffect]);
        if (pEffect->effectID == 0) {
            nBonus += pEffect->effectAmount;
        }
    }

    pRes->Release();

    return nBonus;
}

// 0x4E9FD0
STRREF CItem::GetDescription()
{
    if (cResRef == "") {
        return -1;
    }

    if (pRes == NULL) {
        return -1;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2176
    UTIL_ASSERT(pRes->Demand());

    STRREF strDescription;
    if ((m_flags & 0x1) != 0 && pRes->m_pHeader->identifiedDescription != -1) {
        strDescription = pRes->m_pHeader->identifiedDescription;
    } else {
        strDescription = pRes->m_pHeader->genericDescription;
    }

    pRes->Release();
    return strDescription;
}

// 0x4EA050
CResRef CItem::GetDescriptionPicture()
{
    CResRef iconResRef("");

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2218
            UTIL_ASSERT(pRes->Demand());

            iconResRef = pRes->m_pHeader->descriptionPicture;
            pRes->Release();
        }
    }

    return iconResRef;
}

// 0x4EA0E0
void CItem::LoadWeaponIdentification(CWeaponIdentification& weaponId)
{
    if (pRes == NULL) {
        return;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
    // __LINE__: 2425
    UTIL_ASSERT(pRes->Demand());

    weaponId.m_itemType = pRes->m_pHeader->itemType;
    weaponId.m_itemFlags = pRes->m_pHeader->itemFlags;
    weaponId.m_itemFlagMask = 0;
    weaponId.m_attributes = pRes->m_pHeader->attributes;

    pRes->Release();
}

// 0x4EA150
BYTE CItem::GetMinLevelRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2464
            UTIL_ASSERT(pRes->Demand());

            nValue = static_cast<BYTE>(pRes->m_pHeader->minLevelRequired);
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA1B0
BYTE CItem::GetMinSTRRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2483
            UTIL_ASSERT(pRes->Demand());

            nValue = static_cast<BYTE>(pRes->m_pHeader->minSTRRequired);
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA210
BYTE CItem::GetMinINTRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2521
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minINTRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA270
BYTE CItem::GetMinDEXRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2540
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minDEXRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA2D0
BYTE CItem::GetMinWISRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2559
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minWISRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA330
BYTE CItem::GetMinCONRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2578
            UTIL_ASSERT(pRes->Demand());

            nValue = pRes->m_pHeader->minCONRequired;
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA390
BYTE CItem::GetMinCHRRequired()
{
    BYTE nValue = 0;

    if (cResRef != "") {
        if (pRes != NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\CItem.cpp
            // __LINE__: 2597
            UTIL_ASSERT(pRes->Demand());

            nValue = static_cast<BYTE>(pRes->m_pHeader->minCHRRequired);
            pRes->Release();
        }
    }

    return nValue;
}

// 0x4EA580
void CItem::FormatItemDescription(CUIControlTextDisplay* pText, COLORREF rgbColor)
{
    // NOTE: Uninline. Return value discarded.
    GetItemType();
    // NOTE: Uninline. Refreshes m_nAbilities for FormatItemStats().
    GetAbilityCount();

    if (g_pBaldurChitin->pActiveEngine != NULL) {
        CBaldurEngine::UpdateText(pText, "%s", CBaldurEngine::FetchString(GetDescription()));
        CBaldurEngine::UpdateText(pText, "");
        FormatItemStats(pText, RGB(200, 200, 0));
    }
}

// 0x4EA750
void CItem::FormatItemStats(CUIControlTextDisplay* pText, COLORREF rgbColor)
{
    DWORD dwNotUsableBy = GetNotUsableBy();
    DWORD dwNotUsableBy2 = GetNotUsableBy2();

    CString sUsableBy;
    CString sNotUsableBy;
    CString sLine;
    CString sAbjurer;
    CString sConjurer;
    CString sDiviner;
    CString sEnchanter;
    CString sEvoker;
    CString sIllusionist;
    CString sNecromancer;
    CString sTransmuter;

    if ((dwNotUsableBy == 0 && dwNotUsableBy2 == 0) ||
        g_pBaldurChitin->pActiveEngine == NULL) {
        return;
    }

    // Re-read after the early-out, matching the binary.
    dwNotUsableBy = GetNotUsableBy();

    // Set once the item proves usable by a "key" category; selects which list
    // (Usable By / Not Usable By) is shown and suppresses redundant Not-Usable lines.
    int nUsable = 0;

    // Alignment, good axis.
    if (dwNotUsableBy & 0xE000) {
        CString sGood    = "  " + CBaldurEngine::FetchString(0x7B2C) + "\n";  // Good
        CString sNeutral = "  " + CBaldurEngine::FetchString(0x7B2E) + "\n";  // Neutral
        CString sEvil    = "  " + CBaldurEngine::FetchString(0x7B30) + "\n";  // Evil
        if (dwNotUsableBy & 0x4000) sNotUsableBy += sGood;    else sUsableBy += sGood;
        if (dwNotUsableBy & 0x8000) sNotUsableBy += sNeutral; else sUsableBy += sNeutral;
        if (dwNotUsableBy & 0x2000) sNotUsableBy += sEvil;    else sUsableBy += sEvil;
    }

    // Alignment, law axis.
    if (dwNotUsableBy & 0x31000) {
        CString sLawful  = "  " + CBaldurEngine::FetchString(0x7B2D) + "\n";  // Lawful
        CString sNeutral = "  " + CBaldurEngine::FetchString(0x7B2E) + "\n";  // Neutral
        CString sChaotic = "  " + CBaldurEngine::FetchString(0x7B2F) + "\n";  // Chaotic
        if (dwNotUsableBy & 0x10000) sNotUsableBy += sLawful;  else sUsableBy += sLawful;
        if (dwNotUsableBy & 0x20000) sNotUsableBy += sNeutral; else sUsableBy += sNeutral;
        if (dwNotUsableBy & 0x1000)  sNotUsableBy += sChaotic; else sUsableBy += sChaotic;
    }

    // Race.
    if (dwNotUsableBy & 0x3F800000) {
        CString sElves     = "  " + CBaldurEngine::FetchString(0x7B52) + "\n";  // Elves
        CString sDwarves   = "  " + CBaldurEngine::FetchString(0x7B56) + "\n";  // Dwarves
        CString sHalfElves = "  " + CBaldurEngine::FetchString(0x7B55) + "\n";  // Half-elves
        CString sHalflings = "  " + CBaldurEngine::FetchString(0x7B53) + "\n";  // Halflings
        CString sHalfOrcs  = "  " + CBaldurEngine::FetchString(0x7B58) + "\n";  // Half-orcs
        CString sHumans    = "  " + CBaldurEngine::FetchString(0x7B51) + "\n";  // Humans
        CString sGnomes    = "  " + CBaldurEngine::FetchString(0x7B54) + "\n";  // Gnomes

        if (dwNotUsableBy & 0x800000) sNotUsableBy += sElves; else sUsableBy += sElves;
        if (dwNotUsableBy & 0x1000000) {
            sNotUsableBy += sDwarves;
        } else {
            sUsableBy += sDwarves;
            nUsable = 1;
        }
        if (dwNotUsableBy & 0x2000000)  sNotUsableBy += sHalfElves; else sUsableBy += sHalfElves;
        if (dwNotUsableBy & 0x4000000)  sNotUsableBy += sHalflings; else sUsableBy += sHalflings;
        if (dwNotUsableBy & 0x8000000)  sNotUsableBy += sHumans;    else sUsableBy += sHumans;
        if (dwNotUsableBy & 0x10000000) sNotUsableBy += sGnomes;    else sUsableBy += sGnomes;
        if (dwNotUsableBy & 0x20000000) sNotUsableBy += sHalfOrcs;  else sUsableBy += sHalfOrcs;
    }

    // Single classes. A restricted class is listed only while nothing usable has been found.
    sLine = "  " + CBaldurEngine::FetchString(0x22) + "\n";  // Barbarian
    if ((dwNotUsableBy & 0x1) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x43B) + "\n";  // Bard
    if ((dwNotUsableBy & 0x2) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x437) + "\n";  // Cleric
    if ((dwNotUsableBy & 0x4) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x438) + "\n";  // Druid
    if ((dwNotUsableBy & 0x8) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x419F) + "\n";  // Fighter
    if ((dwNotUsableBy & 0x10) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x21) + "\n";  // Monk
    if ((dwNotUsableBy & 0x20) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x436) + "\n";  // Paladin
    if ((dwNotUsableBy & 0x40) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x435) + "\n";  // Ranger
    if ((dwNotUsableBy & 0x80) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x43A) + "\n";  // Rogue
    if ((dwNotUsableBy & 0x100) == 0) sUsableBy += sLine; else if (nUsable == 0) sNotUsableBy += sLine;

    sLine = "  " + CBaldurEngine::FetchString(0x20) + "\n";  // Sorcerer
    if ((dwNotUsableBy & 0x200) == 0) {
        sUsableBy += sLine;
        nUsable = 1;
    } else if (nUsable == 0) {
        sNotUsableBy += sLine;
    }

    // Wizard and the eight specialist schools (school flags live in GetNotUsableBy2).
    sLine        = "  " + CBaldurEngine::FetchString(0x2703) + "\n";  // Wizard
    sAbjurer     = "  " + CBaldurEngine::FetchString(0x1F6) + "\n";   // Abjurer
    sConjurer    = "  " + CBaldurEngine::FetchString(0x1F8) + "\n";   // Conjurer
    sDiviner     = "  " + CBaldurEngine::FetchString(0x7DC) + "\n";   // Diviner
    sEnchanter   = "  " + CBaldurEngine::FetchString(0x7E6) + "\n";   // Enchanter
    sEvoker      = "  " + CBaldurEngine::FetchString(0x31F2) + "\n";  // Evoker
    sIllusionist = "  " + CBaldurEngine::FetchString(0x31F1) + "\n";  // Illusionist
    sNecromancer = "  " + CBaldurEngine::FetchString(0x31F3) + "\n";  // Necromancer
    sTransmuter  = "  " + CBaldurEngine::FetchString(0x31F4) + "\n";  // Transmuter

    CString sHeader;
    if ((dwNotUsableBy & 0x400) == 0) {
        // Usable by wizards: list each restricted school under Not Usable By.
        if (dwNotUsableBy2 & 0x40)   sNotUsableBy += sAbjurer;
        if (dwNotUsableBy2 & 0x80)   sNotUsableBy += sConjurer;
        if (dwNotUsableBy2 & 0x100)  sNotUsableBy += sDiviner;
        if (dwNotUsableBy2 & 0x200)  sNotUsableBy += sEnchanter;
        if (dwNotUsableBy2 & 0x400)  sNotUsableBy += sEvoker;
        if (dwNotUsableBy2 & 0x800)  sNotUsableBy += sIllusionist;
        if (dwNotUsableBy2 & 0x1000) sNotUsableBy += sNecromancer;
        if (dwNotUsableBy2 & 0x2000) sNotUsableBy += sTransmuter;
        sUsableBy += sLine;  // Wizard
        sHeader = CBaldurEngine::FetchString(nUsable != 0 ? 0x7B2B : 0x7B2A);
    } else {
        // Not usable by wizards: list each usable school under Usable By.
        if ((dwNotUsableBy2 & 0x40) == 0)   { sUsableBy += sAbjurer;     nUsable = 1; }
        if ((dwNotUsableBy2 & 0x80) == 0)   { sUsableBy += sConjurer;    nUsable = 1; }
        if ((dwNotUsableBy2 & 0x100) == 0)  { sUsableBy += sDiviner;     nUsable = 1; }
        if ((dwNotUsableBy2 & 0x200) == 0)  { sUsableBy += sEnchanter;   nUsable = 1; }
        if ((dwNotUsableBy2 & 0x400) == 0)  { sUsableBy += sEvoker;      nUsable = 1; }
        if ((dwNotUsableBy2 & 0x800) == 0)  { sUsableBy += sIllusionist; nUsable = 1; }
        if ((dwNotUsableBy2 & 0x1000) == 0) { sUsableBy += sNecromancer; nUsable = 1; }
        if ((dwNotUsableBy2 & 0x2000) == 0) { sUsableBy += sTransmuter;  nUsable = 1; }
        if (nUsable == 0) {
            sNotUsableBy += sLine;  // Wizard
            sHeader = CBaldurEngine::FetchString(0x7B2A);  // Not Usable By:
        } else {
            sHeader = CBaldurEngine::FetchString(0x7B2B);  // Usable By:
        }
    }

    CBaldurEngine::UpdateTextForceColor(pText, rgbColor, "%s", (LPCSTR)sHeader);
    const CString& sList = (nUsable != 0) ? sUsableBy : sNotUsableBy;
    CBaldurEngine::UpdateTextNoTrim(pText, "%s", (LPCSTR)sList);
}

// 0x464130
CItem& CItem::operator=(const CItem& other)
{
    SetResRef(other.GetResRef(), TRUE);
    m_useCount1 = other.m_useCount1;
    m_useCount2 = other.m_useCount2;
    m_useCount3 = other.m_useCount3;
    m_wear = other.m_wear;
    m_flags = other.m_flags;
    return *this;
}

// 0x675320
bool CItem::operator==(const CItem& other)
{
    return m_useCount1 == other.m_useCount1
        && m_useCount2 == other.m_useCount2
        && m_useCount3 == other.m_useCount3
        && m_wear == other.m_wear
        && m_flags == other.m_flags
        && cResRef == other.cResRef;
}
