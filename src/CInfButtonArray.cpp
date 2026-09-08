#include "CInfButtonArray.h"

#include "CBaldurChitin.h"
#include "CButtonData.h"
#include "CGameButtonList.h"
#include "CGameEffect.h"
#include "CGameObjectArray.h"
#include "CGameOptions.h"
#include "CGameSave.h"
#include "CGameSprite.h"
#include "CGameSpriteEquipment.h"
#include "CIcon.h"
#include "CInfGame.h"
#include "IcewindCGameEffects.h"
#include "CMessage.h"
#include "CSound.h"
#include "CItem.h"
#include "CScreenWorld.h"
#include "CUIControlBase.h"
#include "CUIControlButton.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"

// 0x851700
const BYTE CInfButtonArray::STATE_NONE = 0;

// 0x8E6820
static CGameButtonList* g_pButtonArrayPickerList = NULL;

// 0x587960
CInfButtonSettings::CInfButtonSettings()
{
    m_nIconSequence = 0;
    m_bSelected = 0;
    m_nCount = 0;
    field_0 = 0;
    m_bActive = 0;
    m_bHasOverlay = 0;
    m_bActiveWeaponSet = 0;
    m_bGreyOut = FALSE;
    m_nIconNormalFrame = -1;
    m_nIconSelectedFrame = -1;
}

// 0x587B80
CInfButtonSettings::~CInfButtonSettings()
{
}

// 0x5879E0
CInfButtonArray::CInfButtonArray()
{
    // TODO: Incomplete.

    for (INT nButton = 0; nButton < 12; nButton++) {
        m_buttonTypes[nButton] = 100;
        m_buttonArray[nButton].m_bGreyOut = FALSE;
    }

    m_nCustomizeSlot = 0;
    m_nSelectedButton = 100;
    field_16E0 = -1;
    field_16E4 = 0;
    m_nState = STATE_NONE;

    // 0x19B6..0x19D6, copied from selected sprite by SelectToolbar.
    m_customButtonTypes[0] = 5;
    m_customButtonTypes[1] = 3;
    m_customButtonTypes[2] = 0x46;
    m_customButtonTypes[3] = 0x47;
    m_customButtonTypes[4] = 0x50;
    m_customButtonTypes[5] = 0x51;
    m_customButtonTypes[6] = 0x5A;
    m_customButtonTypes[7] = 0x5B;
    m_customButtonTypes[8] = 10;
    m_nCurrentSelectedSpellClass = 0;
    m_nCurrentSelectedSpellLevel = 0;
    m_currentAbilityResRef = "";
    m_nQuickWeaponSlot = 0;
    m_nListStartIndex = 0;
}

// NOTE: Convenience.
void CInfButtonArray::ClearPickerList()
{
    if (g_pButtonArrayPickerList != NULL) {
        while (!g_pButtonArrayPickerList->IsEmpty()) {
            delete g_pButtonArrayPickerList->RemoveHead();
        }
        delete g_pButtonArrayPickerList;
        g_pButtonArrayPickerList = NULL;
    }
}

// Build one of the six picker lists for the party's first member.  SetState
// (0x589110) calls this on every picker state and stores the result in
// g_pButtonArrayPickerList; the caller owns the returned list.
//
// nSlot is already biased by the caller for kind 1 (quick-weapon button index
// + SLOT_WEAPON); kind 3 applies its own SLOT_MISC bias here.  a5 selects
// between the per-slot list and the whole-inventory list, and its polarity is
// deliberately opposite in the two item-backed kinds -- that is what the
// binary does.
//
// 0x587C20
CGameButtonList* CInfButtonArray::BuildPickerList(INT nSlot, INT nListType, const BYTE& nClass,
    DWORD nSpecialization, BOOL a5)
{
    CGameButtonList* pButtons = NULL;

    if (nListType != CINFBUTTONARRAY_PICKER_SPELL
        && nListType != CINFBUTTONARRAY_PICKER_QUICK_WEAPON
        && nListType != CINFBUTTONARRAY_PICKER_QUICK_ITEM
        && nListType != CINFBUTTONARRAY_PICKER_INNATE
        && nListType != CINFBUTTONARRAY_PICKER_INTERNAL
        && nListType != CINFBUTTONARRAY_PICKER_SONG) {
        return NULL;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return NULL;
    }

    LONG* pGroupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = pGroupList[0];
    delete pGroupList;

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return NULL;
    }

    switch (nListType) {
    case CINFBUTTONARRAY_PICKER_QUICK_WEAPON:
        if (a5) {
            pButtons = pSprite->GetItemUsages(static_cast<SHORT>(nSlot),
                static_cast<WORD>(nListType),
                -1);
        } else {
            // NOTE: The binary calls 0x717250 here, a byte-identical copy of
            // GetAllItemUsages (0x716E80) the linker did not fold.
            pButtons = pSprite->GetAllItemUsages(FALSE);
        }
        break;
    case CINFBUTTONARRAY_PICKER_SPELL:
        if (nClass == CAIOBJECTTYPE_C_CLERIC && nSpecialization != 0) {
            pButtons = pSprite->GetDomainSpellsButtonList(nClass, nSpecialization);
        } else {
            pButtons = pSprite->GetSpellsButtonList(nClass);
        }
        break;
    case CINFBUTTONARRAY_PICKER_QUICK_ITEM:
        if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bQuickItemMapping && !a5) {
            pButtons = pSprite->GetItemUsages(
                static_cast<SHORT>(nSlot + CGameSpriteEquipment::SLOT_MISC),
                static_cast<WORD>(nListType),
                -1);
        } else {
            pButtons = pSprite->GetAllItemUsages(FALSE);
        }
        break;
    case CINFBUTTONARRAY_PICKER_INNATE:
        pButtons = pSprite->GetInnateSpellsButtonList();
        break;
    case CINFBUTTONARRAY_PICKER_INTERNAL:
        pButtons = pSprite->GetInternalButtonList();
        break;
    case CINFBUTTONARRAY_PICKER_SONG:
        pButtons = pSprite->GetSongsButtonList();
        break;
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return pButtons;
}

// 0x588240
void CInfButtonArray::GetSelectedQuickWeaponData(CButtonData& cButtonData)
{
    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() != 0) {
        LONG* groupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
        LONG nCharacterId = groupList[0];
        delete groupList;

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->GetSelectedWeaponButton(cButtonData);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5883C0
BYTE CInfButtonArray::GetSelectedModalMode()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BYTE modalState = 0;

    if (pGame->GetGroup()->GetCount() != 0) {
        LONG nCharacterId = pGame->GetGroup()->GetGroupLeader();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            modalState = pSprite->GetModalState();

            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return modalState;
}

// 0x588460
BYTE CInfButtonArray::GetButtonId(INT buttonType)
{
    for (BYTE id = 0; id < 12; id++) {
        if (m_buttonTypes[id] == buttonType && !m_buttonArray[id].m_bGreyOut) {
            return id;
        }
    }
    return -1;
}

// Dispatch a picked item ability onto the party leader.  `bUseNow` inverts into
// CGameSprite::ReadyOffInternalList's `firstCall`: a "use now" click readies the
// ability for an immediate target pick, a customise click only records it.
//
// 0x5884B0
BOOLEAN CInfButtonArray::UseItemAction(const CButtonData* pButtonData, BOOL bUseNow)
{
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_group.m_memberList.GetCount() == 0) {
        return FALSE;
    }

    LONG nLeader = pGame->m_group.GetGroupLeader();
    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nLeader,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSprite->ReadyOffInternalList(*pButtonData, !bUseNow);
    pGame->m_cObjectArray.ReleaseDeny(nLeader,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// Fire quick slot `nButton` of the party leader, picking the action from the
// quick-slot family `nMode` (1 weapon, 2 spell, 3 item, 4 innate, 6 song).
// Mode 5 -- and any other value -- takes the leader lock and releases it again
// without doing anything.
//
// 0x588570
void CInfButtonArray::ReadyQuickSlotByMode(SHORT nButton, INT nMode)
{
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_group.m_memberList.GetCount() == 0) {
        return;
    }

    LONG nLeader = pGame->m_group.GetGroupLeader();

    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nLeader,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    switch (nMode) {
    case 1:
        pSprite->SetSelectedWeaponButton(nButton);
        break;
    case 2:
        pSprite->ReadySpell(nButton, 2, 0);
        break;
    case 3:
        pSprite->ReadyItem(nButton, 0);
        break;
    case 4:
        pSprite->ReadySpell(nButton, 4, 0);
        break;
    case 6:
        pSprite->ReadySpell(nButton, 6, 0);
        break;
    }

    pGame->m_cObjectArray.ReleaseDeny(nLeader, CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// Dispatch a picked spell onto the party leader.  Same shape as UseItemAction,
// through CGameSprite::UseButtonAction.
//
// 0x5886A0
BOOLEAN CInfButtonArray::UseSpellAction(const CButtonData* pButtonData, BOOL bUseNow)
{
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_group.m_memberList.GetCount() == 0) {
        return FALSE;
    }

    LONG nLeader = pGame->m_group.GetGroupLeader();
    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nLeader,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSprite->UseButtonAction(*pButtonData, !bUseNow);
    pGame->m_cObjectArray.ReleaseDeny(nLeader,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// Dispatch a picked innate ability onto the party leader.  Same shape as
// UseItemAction, through CGameSprite::UseButtonItem.
//
// 0x588760
BOOLEAN CInfButtonArray::UseInnateAction(const CButtonData* pButtonData, BOOL bUseNow)
{
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_group.m_memberList.GetCount() == 0) {
        return FALSE;
    }

    LONG nLeader = pGame->m_group.GetGroupLeader();
    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nLeader,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSprite->UseButtonItem(*pButtonData, !bUseNow);
    pGame->m_cObjectArray.ReleaseDeny(nLeader,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// Toggle the group leader's bard song from a song CButtonData: locate the song,
// and -- unless the leader is silenced -- either stop an active song (modal
// state 1) or start one (set the last-song index, enter modal state 1, play the
// ACT_01 cue and queue a SmallWait so the singer pauses).  A silenced leader
// just gets the "cannot sing" feedback and the bar resets.
//
// `bUseNow` is the same picker flag the three static Use*Action helpers take: a
// customise click (state 0x71) passes 0 and only takes and drops the lock, a
// play click (state 0x7A) passes 1 and reaches the toggle.
//
// 0x588820
BOOL CInfButtonArray::UseSongAction(const CButtonData* pButtonData, BOOL bUseNow)
{
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_group.m_memberList.GetCount() == 0) {
        return FALSE;
    }

    LONG nLeader = pGame->m_group.GetGroupLeader();
    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nLeader,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    UINT nSongID = 0;
    if (pGame->m_songs.Find(pButtonData->m_abilityId.m_res, nSongID) && bUseNow) {
        if ((pSprite->m_derivedStats.m_generalState & STATE_SILENCED) == 0
            && (pSprite->m_baseStats.m_generalState & STATE_SILENCED) == 0) {
            if (pSprite->m_nModalState == 1) {
                pSprite->SetModalState(0, FALSE);
                m_nSelectedButton = 100;
            } else {
                pSprite->m_nLastSong = static_cast<BYTE>(nSongID);
                pSprite->SetModalState(1, FALSE);
                m_nSelectedButton = 2;

                CSound sound;
                sound.SetResRef(CResRef("ACT_01"), TRUE, TRUE);
                sound.SetFireForget(TRUE);
                sound.SetChannel(5, 0);
                sound.Play(FALSE);

                CAIAction action(CAIAction::SMALLWAIT,
                    CAIObjectType(0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0),
                    1, -1, -1);
                CMessage* pMessage = new CMessageAddAction(action, pSprite->m_id, pSprite->m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        } else {
            pSprite->FeedBack(51 /* 0x85C10C */, 0, 0, 0, -1, 0, 0);
            pSprite->SetModalState(0, FALSE);
            m_nSelectedButton = 100;
            pGame->m_nState = 0;
            UpdateButtons();
        }
    }

    pGame->m_cObjectArray.ReleaseDeny(nLeader, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    return TRUE;
}

// Store `buttonData` into quick slot `nButton` of the first party member,
// picking the destination array from `nMode` (1 weapon, 2 spell, 3 item,
// 4 innate, 6 song).  Mode 5 -- and any other value -- takes the member lock
// and releases it again without doing anything.
//
// Unlike ReadyQuickSlotByMode this resolves the member through GetGroupList()[0]
// rather than GetGroupLeader().
//
// 0x588CB0
void CInfButtonArray::CustomizeQuickSlot(const CButtonData* pButtonData, BYTE nButton, INT nMode)
{
    CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
    if (pGame->m_group.m_memberList.GetCount() == 0) {
        return;
    }

    LONG* pGroupList = pGame->m_group.GetGroupList();
    LONG nCharacterId = pGroupList[0];
    delete pGroupList;

    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = pGame->m_cObjectArray.GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    switch (nMode) {
    case 1:
        pSprite->SetQuickWeapon(nButton, *pButtonData);

        // The clamped store below is dead: the unconditional one that follows
        // it writes the same field from the same source.  Both are in the
        // binary (three inlined SetQuickWeapon(BYTE, BYTE) bodies, all from
        // ObjCreature.h line 2031).
        if (pButtonData->m_abilityId.m_itemNum >= CGameSpriteEquipment::SLOT_AMMO
            && pButtonData->m_abilityId.m_itemNum <= CGameSpriteEquipment::SLOT_AMMO + 3) {
            pSprite->SetQuickWeapon(nButton, static_cast<BYTE>(pButtonData->m_abilityId.m_itemNum));
        } else {
            pSprite->SetQuickWeapon(nButton, static_cast<BYTE>(0));
        }

        pSprite->SetQuickWeapon(nButton, static_cast<BYTE>(pButtonData->m_abilityId.m_itemNum));
        break;
    case 2:
        pSprite->SetQuickSpell(nButton, *pButtonData);
        break;
    case 3:
        pSprite->SetQuickItem(nButton, *pButtonData);
        break;
    case 4:
        pSprite->SetQuickAbility(nButton, *pButtonData);
        break;
    case 6:
        pSprite->SetQuickSong(nButton, *pButtonData);
        break;
    }

    pGame->m_cObjectArray.ReleaseDeny(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// 0x588FF0
BOOL CInfButtonArray::ResetState()
{
    // Pops one level off the action-bar state stack and re-applies it.  With
    // an empty stack it leaves the current bar untouched: it must NOT force
    // SetState(STATE_NONE), which blanked the bar when WorldEngineActivated
    // ran ResetState after the bar had already been built on load / new game.
    if (!m_stateStack.empty()) {
        INT nState = m_stateStack.front();
        m_stateStack.pop_front();
        SetState(nState, 0);
    }

    return TRUE;
}

// Walk the action-bar state stack back.  a3 == 1 unwinds the whole sequence in
// one go, landing on the bar it started from; anything else steps back a single
// level.  Both re-apply the state they land on, and neither pushes.
//
// 0x589FF0
void CInfButtonArray::PopState(int a2, char a3)
{
    if (m_stateStack.empty()) {
        return;
    }

    if (a3 == 1) {
        // The binary spells this out as an erase of the whole deque, choosing
        // whichever end is cheaper to rotate; the state it lands on is the one
        // at the bottom.
        INT nState = m_stateStack.back();
        m_stateStack.clear();
        SetState(nState, 0);
        return;
    }

    INT nState = m_stateStack.front();
    m_stateStack.pop_front();
    SetState(nState, a2);
}

// 0x589100
void CInfButtonArray::UpdateState()
{
    SetState(m_nState, 0);
}

// 0x589110
BOOL CInfButtonArray::SetState(INT nState, int a2)
{
    // TODO: Incomplete.

    // The picker list is torn down on every transition; the picker branch
    // below rebuilds it when the new state is itself a picker.
    ClearPickerList();

    // a2 asks for the state being replaced to be remembered, so that a submenu
    // or picker can walk back to it.
    if (a2 == 1 && nState != m_nState && m_nState != 0) {
        m_stateStack.push_front(m_nState);
    }

    switch (nState) {
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x70:
    case 0x71:
    case 0x7A:
    case 0x7B: {
        // Picker states (weapon / spell / item / innate / song / feat points).
        // Each state selects one builder; the layout that follows is shared.
        // When N <= 12 the slots use the submenu types 0x15..0x20 (entry =
        // buttonType - 0x15).  When N > 12 the layout switches to paging
        // buttons: slot 0 = 0x21 (page-up arrow), slots 1..10 = 0x15..0x1E
        // (entries), slot 11 = 0x22 (page-down).  m_nListStartIndex holds the
        // index of the first displayed entry.  0x7B never reaches the paging
        // layout: its list is at most MAX_FEAT_POINTS + 1 entries long.
        BYTE nNoClass = CAIOBJECTTYPE_C_NONE;

        switch (nState) {
        case 0x65:
            g_pButtonArrayPickerList = BuildPickerList(
                m_nCustomizeSlot + CGameSpriteEquipment::SLOT_WEAPON,
                CINFBUTTONARRAY_PICKER_QUICK_WEAPON, nNoClass, 0, TRUE);
            break;
        case 0x66:
        case 0x67:
            UTIL_ASSERT(m_nCurrentSelectedSpellClass != CAIOBJECTTYPE_C_NONE);

            g_pButtonArrayPickerList = BuildPickerList(m_nCustomizeSlot,
                CINFBUTTONARRAY_PICKER_SPELL, m_nCurrentSelectedSpellClass,
                m_nCurrentSelectedSpellLevel, FALSE);
            break;
        case 0x68:
        case 0x69:
            g_pButtonArrayPickerList = BuildPickerList(m_nCustomizeSlot,
                CINFBUTTONARRAY_PICKER_QUICK_ITEM, nNoClass, 0, nState == 0x69);
            break;
        case 0x6A:
        case 0x6B:
            g_pButtonArrayPickerList = BuildPickerList(m_nCustomizeSlot,
                CINFBUTTONARRAY_PICKER_INNATE, nNoClass, 0, FALSE);
            break;
        case 0x70:
            g_pButtonArrayPickerList = BuildPickerList(m_nCustomizeSlot,
                CINFBUTTONARRAY_PICKER_INTERNAL, nNoClass, 0, FALSE);
            break;
        case 0x71:
        case 0x7A:
            g_pButtonArrayPickerList = BuildPickerList(m_nCustomizeSlot,
                CINFBUTTONARRAY_PICKER_SONG, nNoClass, 0, FALSE);
            break;
        case 0x7B:
            g_pButtonArrayPickerList = BuildFeatPointsPickerList(m_currentAbilityResRef);
            break;
        }

        if (g_pButtonArrayPickerList != NULL
            && g_pButtonArrayPickerList->GetCount() > 12) {
            // Paging layout - slot 0 = page up, slots 1-10 = entries
            // (filled by UpdateButtons via m_nListStartIndex), slot 11 = page
            // down.  Type 0x21 / 0x22 already in UpdateButtons.
            m_buttonTypes[0] = 0x21;
            for (INT nButton = 1; nButton < 11; nButton++) {
                m_buttonTypes[nButton] = 0x15 + (nButton - 1);
            }
            m_buttonTypes[11] = 0x22;
        } else {
            for (INT nButton = 0; nButton < 12; nButton++) {
                m_buttonTypes[nButton] = 0x15 + nButton;
            }
        }

        if (m_nState != nState) {
            m_nListStartIndex = 0;
        }

        m_nState = nState;
        UpdateButtons();
        return TRUE;
    }
    case 0x73:
    case 0x74:
        // Skills submenu (Stealth / Search / Thieving / Wilderness Lore / Animal Empathy).
        m_buttonTypes[0] = 0x0B;
        m_buttonTypes[1] = 4;
        m_buttonTypes[2] = 0x0C;
        m_buttonTypes[3] = 0x77;
        m_buttonTypes[4] = 0x0D;
        for (INT nButton = 5; nButton < 12; nButton++) {
            m_buttonTypes[nButton] = 100;
        }
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x78:
        // Quick-item picker (3 slots + empty).
        m_buttonTypes[0] = 0x50;
        m_buttonTypes[1] = 0x51;
        m_buttonTypes[2] = 0x52;
        for (INT nButton = 3; nButton < 12; nButton++) {
            m_buttonTypes[nButton] = 100;
        }
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x79:
        // Quick-weapon picker â€” 4 weapon-set rows, each (main, off, empty).
        m_buttonTypes[0] = 0x3C;
        m_buttonTypes[1] = 0x3D;
        m_buttonTypes[2] = 100;
        m_buttonTypes[3] = 0x3E;
        m_buttonTypes[4] = 0x3F;
        m_buttonTypes[5] = 100;
        m_buttonTypes[6] = 0x40;
        m_buttonTypes[7] = 0x41;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 0x42;
        m_buttonTypes[10] = 0x43;
        m_buttonTypes[11] = 100;
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x6E:
        // Group / generic creature action bar.
        m_buttonTypes[0] = 7;
        m_buttonTypes[1] = 8;
        m_buttonTypes[2] = 0x0F;
        m_buttonTypes[3] = 0x10;
        m_buttonTypes[4] = 0x11;
        m_buttonTypes[5] = 0x12;
        m_buttonTypes[6] = 0x13;
        m_buttonTypes[7] = 0x14;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 100;
        m_buttonTypes[10] = 100;
        m_buttonTypes[11] = 100;
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x6F:
        // Empty action bar (used while certain modes are transitioning).
        for (INT nButton = 0; nButton < 12; nButton++) {
            m_buttonTypes[nButton] = 100;
        }
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x6C:
    case 0x6D:
        for (INT nButton = 0; nButton < 12; nButton++) {
            m_buttonTypes[nButton] = 0x15 + nButton;
        }
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x72:
        // Single PC action bar.  Matches Ghidra default layout: protect,
        // weapon pair, then the nine custom button slots copied from sprite
        // offset 0x3D14 by SelectToolbar.
        m_buttonTypes[0] = 7;
        m_buttonTypes[1] = m_nQuickWeaponSlot * 2 + 0x3C;
        m_buttonTypes[2] = m_nQuickWeaponSlot * 2 + 0x3D;
        m_buttonTypes[3] = m_customButtonTypes[0];
        m_buttonTypes[4] = m_customButtonTypes[1];
        m_buttonTypes[5] = m_customButtonTypes[2];
        m_buttonTypes[6] = m_customButtonTypes[3];
        m_buttonTypes[7] = m_customButtonTypes[4];
        m_buttonTypes[8] = m_customButtonTypes[5];
        m_buttonTypes[9] = m_customButtonTypes[6];
        m_buttonTypes[10] = m_customButtonTypes[7];
        m_buttonTypes[11] = m_customButtonTypes[8];
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    case 0x75: {
        m_buttonTypes[0] = 0x23;

        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        LONG nCharacterId = pGame->GetGroup()->GetGroupLeader();
        CGameSprite* pSprite = NULL;
        BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        BOOL bSpellcaster = rc == CGameObjectArray::SUCCESS && pSprite != NULL && pSprite->IsSpellcaster();
        BOOL bBard = rc == CGameObjectArray::SUCCESS && pSprite != NULL && pSprite->IsBard();

        if (rc == CGameObjectArray::SUCCESS) {
            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (bSpellcaster) {
            m_buttonTypes[1] = 0x24;
            m_buttonTypes[2] = 0x25;
            m_buttonTypes[3] = 0x26;
            m_buttonTypes[4] = 0x27;
            m_buttonTypes[5] = bBard ? 0x28 : 100;
        } else {
            m_buttonTypes[1] = 0x25;
            m_buttonTypes[2] = 0x26;
            m_buttonTypes[3] = 0x27;
            m_buttonTypes[4] = bBard ? 0x28 : 100;
            m_buttonTypes[5] = 100;
        }

        m_buttonTypes[6] = 100;
        m_buttonTypes[7] = 100;
        m_buttonTypes[8] = 100;
        m_buttonTypes[9] = 100;
        m_buttonTypes[10] = 0x29;
        m_buttonTypes[11] = 0x2A;
        m_nState = nState;
        UpdateButtons();
        return TRUE;
    }
    case 0x76:
    case 0x77: {
        // Spell class picker.  Ghidra probes classes 2,3,4,7,8,10,11 and
        // maps them to buttons 0x32..0x38 (+ 0x39 domain for class 3).
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        LONG nCharacterId = pGame->GetGroup()->GetGroupLeader();
        CGameSprite* pSprite = NULL;
        BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        INT nButton = 0;
        if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
            const BYTE classes[] = { 2, 3, 4, 7, 8, 10, 11 };
            const INT buttons[] = { 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 };
            for (INT nIndex = 0; nIndex < 7 && nButton < 12; nIndex++) {
                BYTE nClass = classes[nIndex];
                CGameSpriteGroupedSpellList* pSpells = pSprite->GetSpells(nClass);
                if (pSpells->m_nHighestLevel != 0) {
                    m_buttonTypes[nButton++] = buttons[nIndex];
                    if (nClass == 3 && nButton < 12) {
                        m_buttonTypes[nButton++] = 0x39;
                    }
                }
            }

            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        while (nButton < 12) {
            m_buttonTypes[nButton++] = 100;
        }

        m_nState = nState;
        UpdateButtons();
        return TRUE;
    }
    case STATE_NONE:
    default:
        for (INT nButton = 0; nButton < 12; nButton++) {
            m_buttonTypes[nButton] = 100;
        }
        m_nState = STATE_NONE;
        UpdateButtons();
        return TRUE;
    }
}

// 0x58A340
void CInfButtonArray::UpdateButtons()
{
    // TODO: Incomplete.

    CScreenWorld* pWorld = g_pBaldurChitin->GetScreenWorld();
    if (pWorld == NULL) {
        return;
    }

    CUIPanel* pPanel = pWorld->m_cUIManager.GetPanel(1);
    if (pPanel == NULL) {
        return;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CGameSprite* pSprite = NULL;
    LONG nLeaderId = CGameObjectArray::INVALID_INDEX;
    BYTE rc = 0xFF;
    if (pGame->GetGroup()->GetCount() != 0) {
        nLeaderId = pGame->GetGroup()->GetGroupLeader();
        do {
            rc = pGame->GetObjectArray()->GetShare(nLeaderId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    }

    // Shared array-level overlay cells.  GUIBTACT (field_17C2) supplies every
    // m_bHasOverlay slot's 38x38 action icon -- RenderButtonOverlay picks the
    // per-button frame (m_nIconNormalFrame) into this one cell.  GUIBTBUT
    // (field_16E8) supplies the per-button selection marker.  Loaded here
    // rather than in the ctor because m_bDoubleSize is only known once the
    // panel exists.
    field_17C2.SetResRef(CResRef("GUIBTACT"), pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);
    field_16E8.SetResRef(CResRef("GUIBTBUT"), pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);

    for (INT nButton = 0; nButton < 12; nButton++) {
        CUIControlBase* pControl = pPanel->GetControl(nButton + 6);
        if (pControl == NULL) {
            continue;
        }

        CUIControlButton* pButton = static_cast<CUIControlButton*>(pControl);
        CInfButtonSettings& settings = m_buttonArray[nButton];

        CResRef cIconResRef("GUIBTACT");
        SHORT nIconNormalFrame = -1;
        SHORT nIconSelectedFrame = -1;
        SHORT nIconSequence = 0;
        STRREF nToolTip = -1;
        USHORT nHotKey = 0xFFFF;
        BOOL bEnabled = TRUE;
        BOOL bActive = TRUE;
        BOOL bGreyOut = FALSE;
        BOOL bActiveWeaponSet = FALSE;
        SHORT nCount = 0;
        // m_bHasOverlay selects the render path in CUIControlButtonAction::Render:
        //   1 = GUIBTACT-style overlay (Protect/Attack/Cast etc.) â€” the BAM
        //       bakes its own stone bezel, so CUIControlButton::Render base
        //       must be skipped.
        //   0 = STON*-style icon (small silhouette over a regular GUIBTBUT
        //       bezel) â€” base GUIBTBUT must be painted underneath.
        // Original UpdateButtons at 0x58A340 sets m_bHasOverlay per case.
        BOOL bHasOverlay = TRUE;
        // Item icon BAMs (IBLUN/ISHD/SW1H/IPOTN...) carry two cycles:
        //   cycle 0 = inventory icon (large, e.g. 53x43)
        //   cycle 1 = action-bar icon (small, e.g. 26x27)
        // STON*/FORMx/GUIBTACT have one cycle so 0 is the safe default.
        // Item-icon cases below override to 1.

        settings.m_bSelected = m_nSelectedButton == m_buttonTypes[nButton];
        settings.m_bGreyOut = FALSE;

        switch (m_buttonTypes[nButton]) {
        case 7:
            nIconNormalFrame = 0;
            nIconSelectedFrame = 2;
            nToolTip = 0x3E35;
            break;
        case 8:
            nIconNormalFrame = 0x0C;
            nIconSelectedFrame = 0x0E;
            nToolTip = 0x123A;
            nHotKey = 0x12;
            break;
        case 0x0F:
            nIconNormalFrame = 0x2C;
            nIconSelectedFrame = 0x2C;
            nToolTip = 0x3E34;
            nHotKey = 0x11;
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14: {
            // Quick formation slot.  Per Ghidra UpdateButtons (case 0x10-0x14
            // at 0x58A340) m_bHasOverlay = 0: FORMx is a small ~24x22 icon
            // that sits inside a GUIBTBUT bezel, not a full-button overlay.
            INT nFormationButton = m_buttonTypes[nButton] - 0x10;
            SHORT nFormation = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_quickFormations[nFormationButton];
            if (nFormation < 0) {
                cIconResRef = CResRef("STONFORM");
            } else if (nFormation < 10) {
                CString sResRef;
                sResRef.Format("FORM%d", nFormation);
                cIconResRef = CResRef(sResRef);
            } else {
                CString sResRef;
                sResRef.Format("FORM%c", static_cast<char>(nFormation + '7'));
                cIconResRef = CResRef(sResRef);
            }
            nIconNormalFrame = 0;
            nIconSelectedFrame = 0;
            nToolTip = 0x1347;
            nHotKey = static_cast<USHORT>(0x20 + nFormationButton);
            bHasOverlay = FALSE;
            // Ghidra `piVar8[0x73] = (uint)(piStack_51c == apiStack_4e8[0])`:
            // formation slot's selection highlight is driven by whether its
            // configured formation matches CGameSave::m_curFormation (NOT the
            // generic m_nSelectedButton match).  Override here.
            settings.m_bSelected = (nFormation == g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_curFormation) ? 1 : 0;
            break;
        }
        case 2:
            // Bard song.  Ghidra UpdateButtons case 2 sets m_nIconSequence (=
            // settings.m_nIconSequence) and may set m_bSelected if currently in song
            // modal. Tooltip 0x1336, hotkey 0xA.
            nIconNormalFrame = 0x14;
            nIconSelectedFrame = 0x16;
            nToolTip = 0x1336;
            nHotKey = 0xA;
            // The song button's selection highlight tracks the PERSISTENT
            // song-modal state of the group leader, not the transient
            // m_nSelectedButton match -- override the generic m_bSelected set
            // above (0x58A340: clear piVar8[0x73], then set it to 1 when the
            // shared leader's m_nModalState == 1).
            settings.m_bSelected = (GetSelectedModalMode() == 1);
            break;
        case 3:
            nIconNormalFrame = 8;
            nIconSelectedFrame = 10;
            nToolTip = 0x1250;
            nHotKey = 0xB;
            break;
        case 4:
            // Search modal. Ghidra case 4 frames 0x24/0x26, tooltip 0x133F.
            nIconNormalFrame = 0x24;
            nIconSelectedFrame = 0x26;
            nToolTip = 0x133F;
            nHotKey = 0x10;
            // Selection highlight tracks the leader's persistent search modal
            // (state 2), not the transient m_nSelectedButton match -- override
            // the generic set above (0x58A340 case 4: clear piVar8[0x73], set it
            // to 1 only when the shared leader's m_nModalState == 2).
            settings.m_bSelected = (GetSelectedModalMode() == 2);
            break;
        case 5:
            // Skills button. Ghidra case 5 frames 0x60/0x62, tooltip 0x1345.
            nIconNormalFrame = 0x60;
            nIconSelectedFrame = 0x62;
            nToolTip = 0x1345;
            nHotKey = 0xD;
            break;
        case 9:
            // Shapeshift â€” frame 0x28, tooltip 0x135E.
            nIconNormalFrame = 0x28;
            nIconSelectedFrame = 0x28;
            nToolTip = 0x135E;
            break;
        case 10:
            // Special Abilities. Ghidra case 10 frames 0x28/0x2A, tooltip 0x135A.
            nIconNormalFrame = 0x28;
            nIconSelectedFrame = 0x2A;
            nToolTip = 0x135A;
            nHotKey = 0x13;
            break;
        case 0x0B:
            // Stealth. Ghidra case 0x0B frames 0x1C/0x1E, tooltip 0x1368.
            nIconNormalFrame = 0x1C;
            nIconSelectedFrame = 0x1E;
            nToolTip = 0x1368;
            nHotKey = 0xF;
            // Grey the button out while the Stealth slot of the disabled-buttons
            // array is set (m_disabledButtons[0], at +0x16CC) or the post-reveal
            // grey-out timer is still ticking (m_nStealthGreyOut > 0), so stealth
            // cannot be re-armed mid-cooldown (0x58A340 case 0xB: set piVar8[0x77]
            // / m_bGreyOut).  Confirmed on the original via Frida: re-clicking
            // Stealth during the ~90-tick cooldown is swallowed by
            // OnLButtonPressed's grey-out gate (no SetModalState).  AIUpdate counts
            // the timer down and calls UpdateState() at 0, which re-runs
            // SetState(m_nState) -> UpdateButtons and clears the grey.
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL
                && (pSprite->GetDerivedStats()->m_disabledButtons[0] == TRUE
                    || pSprite->m_nStealthGreyOut > 0)) {
                bGreyOut = TRUE;
            }
            // Selection highlight tracks the leader's persistent stealth modal
            // (state 3), not the transient m_nSelectedButton match -- override
            // the generic set above (0x58A340 case 0xB: clear piVar8[0x73], set
            // it to 1 only when the shared leader's m_nModalState == 3).
            settings.m_bSelected = (GetSelectedModalMode() == 3);
            break;
        case 0x0C:
            // Thieving. Ghidra case 0x0C frames 0x18/0x1A, tooltip 0x136B.
            nIconNormalFrame = 0x18;
            nIconSelectedFrame = 0x1A;
            nToolTip = 0x136B;
            nHotKey = 0xE;
            break;
        case 0x0D:
            // Animal Empathy. These are GUIBTACT logical frames; the BAM cycle
            // lookup maps 0x7C/0x7E to the animal head physical frames.
            nIconNormalFrame = 0x7C;
            nIconSelectedFrame = 0x7E;
            nToolTip = 0x136E;
            nHotKey = 0x9;
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL
                && static_cast<signed char>(pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_ANIMAL_EMPATHY]) < 1) {
                bGreyOut = TRUE;
            }
            break;
        case 0x0E:
            // Use Item. Ghidra case 0x0E frames 0x10/0x12, tooltip 0x1372.
            nIconNormalFrame = 0x10;
            nIconSelectedFrame = 0x12;
            nToolTip = 0x1372;
            nHotKey = 0xC;
            break;
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20: {
            // Formation picker sub-grid (state 0x6C / 0x6D) â€” Ghidra sets
            // m_bHasOverlay = 0 here too: small FORMx icon over GUIBTBUT.
            if (m_nState == 0x6C || m_nState == 0x6D) {
                INT nFormation = m_buttonTypes[nButton] - 0x15;
                CString sResRef;
                if (nFormation < 10) {
                    sResRef.Format("FORM%d", nFormation);
                } else {
                    sResRef.Format("FORM%c", static_cast<char>('A' + nFormation - 10));
                }
                cIconResRef = CResRef(sResRef);
                nIconNormalFrame = 0;
                nIconSelectedFrame = 0;
                bHasOverlay = FALSE;
            } else if (g_pButtonArrayPickerList != NULL) {
                // Picker list entry â€” pull icon + tooltip from the
                // CGameButtonList built in RebuildPickerList.  Two layouts:
                //   The binary starts from FindIndex(m_nListStartIndex) and fills
                //   the slots in order, so m_nListStartIndex is an entry index and
                //   the slot's own offset is buttonType - 0x15.  With more
                //   than 12 entries slots 0 + 11 hold the 0x21/0x22 arrows and
                //   fall through to their own UpdateButtons cases.
                INT nListCount = static_cast<INT>(g_pButtonArrayPickerList->GetCount());
                INT nEntry = m_nListStartIndex + (m_buttonTypes[nButton] - 0x15);
                POSITION pos = (nEntry >= 0 && nEntry < nListCount)
                    ? g_pButtonArrayPickerList->FindIndex(nEntry)
                    : NULL;
                CButtonData* pEntry = (pos != NULL) ? g_pButtonArrayPickerList->GetAt(pos) : NULL;
                if (pEntry != NULL && pEntry->m_icon != "") {
                    cIconResRef = pEntry->m_icon;
                    nIconNormalFrame = 0;
                    nIconSelectedFrame = 0;
                    nIconSequence = 1;  // item-style icon BAM
                    nToolTip = pEntry->m_name;
                    bGreyOut = pEntry->m_bDisabled;
                    bHasOverlay = FALSE;
                    if (pEntry->m_bDisplayCount) {
                        nCount = pEntry->m_count;
                    }
                } else {
                    bActive = FALSE;
                    bEnabled = FALSE;
                    cIconResRef = CResRef("");
                }
            } else {
                bActive = FALSE;
                bEnabled = FALSE;
                cIconResRef = CResRef("");
            }
            break;
        }
        case 0x23:
            // Customize: Skills.
            nIconNormalFrame = 0x60;
            nIconSelectedFrame = 0x62;
            nToolTip = 0x1345;
            break;
        case 0x24:
            // Customize: Cast Spell. Original greys it and shows "No Spells"
            // when the leader is not a caster.
            nIconNormalFrame = 8;
            nIconSelectedFrame = 10;
            nToolTip = 0x1250;
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL && !pSprite->IsSpellcaster()) {
                nToolTip = 0x924A;
                bGreyOut = TRUE;
            }
            break;
        case 0x25:
            // Customize: Use Item.
            nIconNormalFrame = 0x10;
            nIconSelectedFrame = 0x12;
            nToolTip = 0x1372;
            break;
        case 0x26:
            // Customize: Quick Item.
            nIconNormalFrame = 0x70;
            nIconSelectedFrame = 0x72;
            nToolTip = 0x1349;
            break;
        case 0x27:
            // Customize: Special Abilities.
            nIconNormalFrame = 0x28;
            nIconSelectedFrame = 0x2A;
            nToolTip = 0x135A;
            break;
        case 0x28:
            // Customize: Battle Song.
            nIconNormalFrame = 0x14;
            nIconSelectedFrame = 0x16;
            nToolTip = 0x1336;
            break;
        case 0x29:
            // Customize: Clear Button.
            nIconNormalFrame = 0x74;
            nIconSelectedFrame = 0x76;
            nToolTip = 0x9B2B;
            break;
        case 0x2A:
            // Customize: Restore Default Buttons.
            nIconNormalFrame = 0x78;
            nIconSelectedFrame = 0x7A;
            nToolTip = 0x9B2C;
            break;
        case 0x21:
            // Page-up arrow (submenu paging) â€” frame 0x30.
            nIconNormalFrame = 0x30;
            nIconSelectedFrame = 0x30;
            break;
        case 0x22:
            // Page-down arrow â€” frame 0x34.
            nIconNormalFrame = 0x34;
            nIconSelectedFrame = 0x34;
            break;
        case 0x32:
            // Class picker: Bard Spell.
            nIconNormalFrame = 0x38;
            nIconSelectedFrame = 0x3A;
            nToolTip = 0x9B1D;
            nHotKey = 0xB;
            break;
        case 0x33:
            // Class picker: Cleric Spell.
            nIconNormalFrame = 0x3C;
            nIconSelectedFrame = 0x3E;
            nToolTip = 0x9B1E;
            nHotKey = 0xB;
            break;
        case 0x34:
            // Class picker: Druid Spell.
            nIconNormalFrame = 0x40;
            nIconSelectedFrame = 0x42;
            nToolTip = 0x9B1F;
            nHotKey = 0xB;
            break;
        case 0x35:
            // Class picker: Paladin Spell.
            nIconNormalFrame = 0x44;
            nIconSelectedFrame = 0x46;
            nToolTip = 0x9B20;
            nHotKey = 0xB;
            break;
        case 0x36:
            // Class picker: Ranger Spell.
            nIconNormalFrame = 0x48;
            nIconSelectedFrame = 0x4A;
            nToolTip = 0x9B21;
            nHotKey = 0xB;
            break;
        case 0x37:
            // Class picker: Sorcerer Spell.
            nIconNormalFrame = 0x4C;
            nIconSelectedFrame = 0x4E;
            nToolTip = 0x9B22;
            nHotKey = 0xB;
            break;
        case 0x38:
            // Class picker: Wizard Spell.
            nIconNormalFrame = 0x50;
            nIconSelectedFrame = 0x52;
            nToolTip = 0x9B23;
            nHotKey = 0xB;
            break;
        case 0x39:
            // Class picker: Domain Spell.
            nIconNormalFrame = 0x54;
            nIconSelectedFrame = 0x56;
            nToolTip = 0x9C04;
            nHotKey = 0xB;
            break;
        case 0x77:
            // Wilderness Lore. Ghidra case 0x77 frames 0x5C/0x5E, tooltip
            // 0x7DBA, hotkey 0x36 ('6').
            nIconNormalFrame = 0x5C;
            nIconSelectedFrame = 0x5E;
            nToolTip = 0x7DBA;
            nHotKey = 0x36;
            break;
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43: {
            CButtonData buttonData;
            BYTE nWeaponSlot = static_cast<BYTE>(m_buttonTypes[nButton] - 0x3C);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL && pGame->m_bGameLoaded) {
                pSprite->GetQuickWeapon(nWeaponSlot, buttonData);
            }
            if (buttonData.m_icon != "") {
                cIconResRef = buttonData.m_icon;
                nIconNormalFrame = 0;
                nIconSelectedFrame = 0;
                nIconSequence = 1;  // small action-bar cycle
                nToolTip = buttonData.m_name;
                bGreyOut = buttonData.m_bDisabled;
            } else {
                // Off-hand slot (odd index) â†’ STONSHIL.  Main hand â†’ STONWEAP.
                cIconResRef = (m_buttonTypes[nButton] & 1) ? CResRef("STONSHIL") : CResRef("STONWEAP");
                nIconNormalFrame = 0;
                nIconSelectedFrame = 0;
                nToolTip = 0x1356;
            }
            nHotKey = static_cast<USHORT>(0x19 + (m_buttonTypes[nButton] - 0x3C));
            // Green border: this slot matches the sprite's currently-active
            // weapon set (sprite.m_quickWeaponSet stores the set index, each
            // set occupies a main+off pair).  Drives settings.m_bActiveWeaponSet.
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL
                && static_cast<BYTE>(nWeaponSlot >> 1) == pSprite->m_nWeaponSet) {
                bActiveWeaponSet = TRUE;
            }
            bHasOverlay = FALSE;
            break;
        }
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E: {
            // Quick spell.  Empty slot keeps STONSPEL stone visible so the user
            // can right-click to assign â€” original UpdateButtons at 0x58A340
            // never marks these inactive.  Ghidra default tooltip 0x1250 ("Cast
            // Spell").
            CButtonData buttonData;
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL && pGame->m_bGameLoaded) {
                pSprite->GetQuickSpell(static_cast<BYTE>(m_buttonTypes[nButton] - 0x46), buttonData);
            }
            if (buttonData.m_icon != "") {
                cIconResRef = buttonData.m_icon;
                nIconSequence = 1;
                nToolTip = buttonData.m_name;
                bGreyOut = buttonData.m_bDisabled;
                if (buttonData.m_bDisplayCount) {
                    nCount = buttonData.m_count;
                }
            } else {
                cIconResRef = CResRef("STONSPEL");
                nToolTip = 0x1250;
            }
            nIconNormalFrame = 0;
            nIconSelectedFrame = 0;
            bHasOverlay = FALSE;
            break;
        }
        case 0x50:
        case 0x51:
        case 0x52: {
            // Quick item.  STONITEM fallback; tooltip 0x1372.
            CButtonData buttonData;
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL && pGame->m_bGameLoaded) {
                pSprite->GetQuickItem(static_cast<BYTE>(m_buttonTypes[nButton] - 0x50), buttonData);
            }
            if (buttonData.m_icon != "") {
                cIconResRef = buttonData.m_icon;
                nIconSequence = 1;
                nToolTip = buttonData.m_name;
                bGreyOut = buttonData.m_bDisabled;
                if (buttonData.m_bDisplayCount) {
                    nCount = buttonData.m_count;
                }
            } else {
                cIconResRef = CResRef("STONITEM");
                nToolTip = 0x1372;
            }
            nIconNormalFrame = 0;
            nIconSelectedFrame = 0;
            nHotKey = static_cast<USHORT>(0x2D + (m_buttonTypes[nButton] - 0x50));
            bHasOverlay = FALSE;
            break;
        }
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62: {
            // Quick ability (innate / feat / special).  STONSPEC fallback;
            // tooltip 0x135A ("Special Abilities").
            CButtonData buttonData;
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL && pGame->m_bGameLoaded) {
                pSprite->GetQuickAbility(static_cast<BYTE>(m_buttonTypes[nButton] - 0x5A), buttonData);
            }
            if (buttonData.m_icon != "") {
                cIconResRef = buttonData.m_icon;
                nIconSequence = 1;
                nToolTip = buttonData.m_name;
                bGreyOut = buttonData.m_bDisabled;
                if (buttonData.m_bDisplayCount) {
                    nCount = buttonData.m_count;
                }
            } else {
                cIconResRef = CResRef("STONSPEC");
                nToolTip = 0x135A;
            }
            nIconNormalFrame = 0;
            nIconSelectedFrame = 0;
            bHasOverlay = FALSE;
            break;
        }
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76: {
            // Quick song (bard).  STONSONG fallback; tooltip 0x923C.
            CButtonData buttonData;
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL && pGame->m_bGameLoaded) {
                pSprite->GetQuickSong(static_cast<BYTE>(m_buttonTypes[nButton] - 0x6E), buttonData);
            }
            if (buttonData.m_icon != "") {
                cIconResRef = buttonData.m_icon;
                nIconSequence = 1;
                nToolTip = buttonData.m_name;
                bGreyOut = buttonData.m_bDisabled;
            } else {
                cIconResRef = CResRef("STONSONG");
                nToolTip = 0x923C;
            }
            nIconNormalFrame = 0;
            nIconSelectedFrame = 0;
            bHasOverlay = FALSE;
            break;
        }
        case 100:
        default:
            bActive = FALSE;
            bEnabled = FALSE;
            cIconResRef = CResRef("");
            break;
        }

        settings.field_0 = bActive ? 1 : 0;
        settings.m_bActive = bActive ? 1 : 0;
        settings.m_bHasOverlay = bHasOverlay ? 1 : 0;
        settings.m_nIconNormalFrame = nIconNormalFrame;
        settings.m_nIconSelectedFrame = nIconSelectedFrame;
        settings.m_bActiveWeaponSet = bActiveWeaponSet ? 1 : 0;
        settings.m_nCount = nCount;
        settings.m_bGreyOut = !bEnabled || bGreyOut;
        // Overlay slots draw their icon from the shared GUIBTACT cell
        // (field_17C2) at m_nIconNormalFrame, so their per-button m_iconCell
        // stays empty.  Non-overlay slots carry their own 32x32 icon here.
        if (bHasOverlay) {
            settings.m_iconCell.SetResRef(CResRef(""), pPanel->m_pManager->m_bDoubleSize, FALSE, FALSE);
        } else {
            settings.m_iconCell.SetResRef(cIconResRef, pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);
            settings.m_iconCell.SequenceSet(nIconSequence);
            if (nIconNormalFrame >= 0) {
                settings.m_iconCell.FrameSet(nIconNormalFrame);
            }
        }
        // Stash sequence in m_nIconSequence so RenderButton can re-apply it when
        // swapping between normal/selected frames without resetting to 0.
        settings.m_nIconSequence = nIconSequence;
        settings.m_countCell.SetResRef(CResRef(""), pPanel->m_pManager->m_bDoubleSize, FALSE, FALSE);

        // Selection highlight: GUIBTBUT cycle entries 0x18+ map to frame 2
        // (red border) per the BAM lookup table.  Original RenderButton at
        // 0x5957C0 swaps to these frames when settings.m_bSelected is set
        // (m_nSelectedButton matches this button's type).
        SHORT nNormalFrame = static_cast<SHORT>(nButton * 2);
        SHORT nPressedFrame = static_cast<SHORT>(nButton * 2 + 1);
        if (settings.m_bSelected != 0) {
            nNormalFrame = static_cast<SHORT>(nButton * 2 + 0x18);
            nPressedFrame = static_cast<SHORT>(nButton * 2 + 0x19);
        }

        pButton->m_nNormalFrame = nNormalFrame;
        pButton->m_nPressedFrame = nPressedFrame;
        pButton->m_nDisabledFrame = static_cast<SHORT>(nButton * 2);
        pButton->m_cVidCell.SetResRef(CResRef("GUIBTBUT"), pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);
        pButton->m_cVidCell.SequenceSet(0);
        pButton->m_cVidCell.FrameSet(pButton->m_nNormalFrame);
        pButton->SetToolTipStrRef(nToolTip, -1, -1);
        pButton->SetToolTipHotKey(nHotKey, 0xFFFF, CString(""));
        // Visibility is driven by settings.field_0: when 0 the original
        // FUN_005957C0 + FUN_005950F0 both no-op, so CUIControlButtonAction::
        // Render checks that to short-circuit. Keep m_bEnabled = TRUE so
        // right-click customization works on slots the user could populate.
        pButton->SetEnabled(TRUE);
    }

    if (rc == CGameObjectArray::SUCCESS) {
        pGame->GetObjectArray()->ReleaseShare(nLeaderId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (g_pBaldurChitin->m_pEngineWorld->m_nPopupState == -1) {
        pPanel->InvalidateRect(NULL);
    }
}

// 0x5950F0
//
// Paint one action-bar slot.  Faithful reconstruction of the binary:
//   1. Gate on the active engine + its video mode being live.
//   2. Compute the icon rect (slot + 3px, + another 2px while pressed) and
//      clip it to the dirty rect.
//   3. Skip inactive slots (field_0 == 0); a live slot with no button
//      (m_bActive == 0) bails out.
//   4. Active-weapon-set highlight ring (HIGHLGHT) when selected-off.
//   5. Background cell (m_countCell, normally empty) painted first.
//   6. Foreground icon (m_iconCell) via CIcon::RenderIcon, branched on the
//      slot's button type (m_buttonTypes):
//        - spell / ability / song (0x46-0x4E, 0x5A-0x62): count badge, with a
//          forced count digit for the generic STONSPEL / STONSPEC slot art.
//        - quick item (0x50-0x52): resolve the looter's quick item, run
//          CheckItemUsable, then tint -- UMD-usable (==2) gets a yellow icon
//          tint plus a STORTIN4 overlay, unusable (==0) gets a STORTINT
//          overlay, freely usable (==1) gets neither.
//        - anything else: a plain icon.
// The greyout flag is the value 2, which CIcon::RenderIcon maps to its
// TINT_INVALID path -- it suppresses the yellow tint, so a greyed quick item
// reads grey, not yellow.
BOOL CInfButtonArray::RenderButton(CPoint pt, const CRect& rClip, BOOL bPressed, INT nButton)
{
    INT nScale = g_pBaldurChitin->field_4A2C != 0 ? 2 : 1;
    CSize size(nScale * CIcon::ICON_SIZE_SM.cx, nScale * CIcon::ICON_SIZE_SM.cy);

    if (nButton < 0 || nButton >= 12) {
        return TRUE;
    }

    CInfButtonSettings& settings = m_buttonArray[nButton];

    // Display not live yet -> paint nothing.
    if (g_pBaldurChitin->pActiveEngine == NULL
        || g_pBaldurChitin->pActiveEngine->pVidMode == NULL) {
        return FALSE;
    }

    CPoint ptIcon(pt.x + 3 * nScale, pt.y + 3 * nScale);
    if (bPressed) {
        ptIcon.x += 2 * nScale;
        ptIcon.y += 2 * nScale;
    }

    CRect rIcon(ptIcon, size);
    CRect rClipIcon;
    rClipIcon.IntersectRect(rIcon, rClip);

    if (settings.field_0 == 0) {
        return TRUE;
    }
    if (settings.m_bActive == 0) {
        return FALSE;
    }

    // Active-weapon-set highlight ring (HIGHLGHT), suppressed while selected.
    // The original also clears m_bCacheHeader on the loaded cell, but that is
    // always FALSE here (the resref is never empty), so it is a no-op.
    if (settings.m_bActiveWeaponSet != 0 && settings.m_bSelected == 0) {
        // dwFlags = m_bGreyOut ? 0x80000 : 0 (binary 0x5951a5: edi loaded from
        // m_bGreyOut, neg/sbb/and 0x80000, reused at the HIGHLGHT Render).
        DWORD dwHighlightFlags = settings.m_bGreyOut ? 0x80000 : 0;
        CVidCell cHighlight;
        // bDoubleSize tracks the display double-size mode (binary 0x5952d9 reads
        // g_pBaldurChitin->field_4A2C into the cell), NOT a constant -- hardcoding
        // TRUE loaded the 2x HIGHLGHT in single-size mode, so the border rendered
        // twice button size (only its top-left quarter visible inside the slot).
        cHighlight.SetResRef(CResRef("HIGHLGHT"), nScale == 2, TRUE, FALSE);
        cHighlight.Render(0, ptIcon.x, ptIcon.y, rClipIcon, NULL, 0, dwHighlightFlags, -1);
    }

    // m_bGreyOut -> CIcon::RenderIcon flag 2 (TINT_INVALID), else no flags.
    DWORD dwFlags = settings.m_bGreyOut ? 2 : 0;

    // Background cell, painted first.  m_countCell is the slot's secondary
    // layer (UpdateButtons leaves it empty for most slots, so this no-ops).
    CIcon::RenderIcon(0, ptIcon, size, rClipIcon, settings.m_countCell.GetResRef(),
        nScale == 2, dwFlags, 0, FALSE, 0, FALSE, 0);

    INT nType = m_buttonTypes[nButton];
    if ((nType >= 0x46 && nType <= 0x4E) || (nType >= 0x5A && nType <= 0x62)) {
        // Spell / ability / song.  The generic STONSPEL / STONSPEC slot art
        // forces the count digit to render even at zero.
        BOOL bForceCount = settings.m_iconCell.GetResRef() == CResRef("STONSPEL")
            || settings.m_iconCell.GetResRef() == CResRef("STONSPEC");
        CIcon::RenderIcon(0, ptIcon, size, rClipIcon, settings.m_iconCell.GetResRef(),
            nScale == 2, dwFlags, static_cast<WORD>(settings.m_nCount), bForceCount, 0, FALSE, 0);
    } else if (nType == 0x50 || nType == 0x51 || nType == 0x52) {
        // Quick item.  Resolve the looter (selected portrait's sprite),
        // fetch the quick item in this slot and check usability.
        CInfGame* pGame = g_pBaldurChitin->m_pObjectGame;
        SHORT nPortrait = g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter();
        LONG nLooterId = -1;
        if (nPortrait < pGame->m_nCharacters) {
            nLooterId = pGame->m_characterPortraits[nPortrait];
        }

        INT nUsable = 1;
        CGameSprite* pLooter;
        BYTE share;
        do {
            pLooter = NULL;
            share = pGame->GetObjectArray()->GetShare(nLooterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pLooter), INFINITE);
        } while (share == CGameObjectArray::SHARED || share == CGameObjectArray::DENIED);

        if (share == CGameObjectArray::SUCCESS) {
            if (pLooter != NULL) {
                CItem* pItem = pLooter->GetQuickItem(static_cast<BYTE>(nType - 0x50));
                if (pItem != NULL) {
                    nUsable = pGame->CheckItemUsable(pLooter, pItem);
                }
            }
            pGame->GetObjectArray()->ReleaseShare(nLooterId,
                CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }

        // Binary 0x5955a6: rgbTint = (nUsable == 2) ? *(COLORREF*)0x84ebb0 : 0,
        // and *(COLORREF*)0x84ebb0 == 0x0000FFFF (yellow).  The UMD-usable icon
        // is tinted yellow; a normal/unusable icon is not.
        COLORREF rgbTint = (nUsable == 2) ? 0x0000FFFF : 0;
        CIcon::RenderIcon(0, ptIcon, size, rClipIcon, settings.m_iconCell.GetResRef(),
            nScale == 2, dwFlags, static_cast<WORD>(settings.m_nCount), FALSE, 0, FALSE, rgbTint);

        if (nUsable == 2 || nUsable == 0) {
            // Both overlays blit translucent (binary 0x595662 / 0x595714 both
            // push dwFlags=2=CVIDIMG_TRANSLUCENT, nTransVal=0xC0) -- the STORTINT
            // path hardcodes 2, not nUsable.
            CVidCell cTint(CResRef(nUsable == 2 ? "STORTIN4" : "STORTINT"),
                g_pBaldurChitin->m_pEngineWorld->GetManager()->m_bDoubleSize);
            cTint.Render(0, ptIcon.x, ptIcon.y, rClipIcon, NULL, 0, 2, 0xC0);
        }
    } else {
        CIcon::RenderIcon(0, ptIcon, size, rClipIcon, settings.m_iconCell.GetResRef(),
            nScale == 2, dwFlags, static_cast<WORD>(settings.m_nCount), FALSE, 0, FALSE, 0);
    }

    return TRUE;
}

// 0x5957C0
//
// Overlay-bezel companion to RenderButton, called first by
// CUIControlButtonAction::Render.  It paints the 38x38 bezel layer and
// reports (via the return value) whether the caller still needs to paint the
// plain GUIBTBUT base bezel underneath:
//   - GUIBTACT-style action slots (m_bHasOverlay, normal frame >= 0) bake
//     their own bezel into field_17C2 -> paint it, return TRUE (skip base).
//   - other slots paint nothing here unless selected, in which case the
//     selection marker (field_16E8) is drawn and TRUE returned (so the base
//     bezel -- which would otherwise show its own selection square -- is
//     skipped).  Unselected non-overlay slots return FALSE so the caller
//     paints the base bezel and RenderButton lays the icon on top.
// A greyed-out slot tints both overlay cells grey (0x00B4B4B4).
BOOL CInfButtonArray::RenderButtonOverlay(CPoint pt, const CRect& rClip, BOOL bPressed, INT nButton)
{
    if (nButton < 0 || nButton >= 12) {
        return TRUE;
    }

    CInfButtonSettings& settings = m_buttonArray[nButton];

    DWORD dwFlags;
    if (settings.m_bGreyOut != 0) {
        field_17C2.SetTintColor(0x00B4B4B4);
        field_16E8.SetTintColor(0x00B4B4B4);
        dwFlags = 0xA0000;
    } else {
        dwFlags = 0;
    }

    // Display not live yet -> paint nothing, let the caller draw the base.
    if (g_pBaldurChitin->pActiveEngine == NULL
        || g_pBaldurChitin->pActiveEngine->pVidMode == NULL) {
        return FALSE;
    }

    INT nScale = g_pBaldurChitin->field_4A2C != 0 ? 2 : 1;
    CRect rOverlay(pt, CSize(38 * nScale, 38 * nScale));
    CRect rClipOverlay;
    rClipOverlay.IntersectRect(rOverlay, rClip);

    if (settings.field_0 == 0) {
        return TRUE;
    }

    if (settings.m_bHasOverlay != 0 && settings.m_nIconNormalFrame >= 0) {
        // Action slot: the bezel + icon are one BAM in field_17C2.
        field_17C2.SequenceSet(0);
        INT nFrame = settings.m_bSelected != 0
            ? settings.m_nIconSelectedFrame
            : settings.m_nIconNormalFrame;
        if (bPressed) {
            nFrame++;
        }
        field_17C2.FrameSet(static_cast<SHORT>(nFrame));
        field_17C2.Render(0, pt.x, pt.y, rClipOverlay, NULL, 0, dwFlags, -1);
        return TRUE;
    }

    // Non-overlay slot: only the selection marker, and only while selected.
    if (settings.m_bSelected == 0) {
        return FALSE;
    }

    field_16E8.SequenceSet(0);
    INT nFrame = nButton * 2 + 0x18;
    if (bPressed) {
        nFrame++;
    }
    field_16E8.FrameSet(static_cast<SHORT>(nFrame));
    field_16E8.Render(0, pt.x, pt.y, rClipOverlay, NULL, 0, dwFlags, -1);
    return TRUE;
}

// 0x595E70
void CInfButtonArray::SetCustomButtonTypes(const INT* pButtonList)
{
    // TODO: Incomplete.

    if (pButtonList == NULL) {
        return;
    }

    for (INT nButton = 0; nButton < 9; nButton++) {
        m_customButtonTypes[nButton] = pButtonList[nButton];
    }
}

// 0x595F70
void CInfButtonArray::SetQuickWeaponSlot(BYTE nSlot)
{
    // TODO: Incomplete.

    if (nSlot > 3) {
        nSlot = 0;
    }

    m_nQuickWeaponSlot = nSlot;
}

// 0x452C50
void CInfButtonArray::SetSelectedButton(INT nSelectedButton)
{
    m_nSelectedButton = nSelectedButton;
}

// Cap on how many attack-bonus points a modal feat may take.
//
// 0x85BCB4
static const INT MAX_FEAT_POINTS = 5;

// The state 0x7B picker: how many attack-bonus points to sink into Power
// Attack or Expertise.  Each feat offers "Off" plus one entry per point, up to
// the sprite's base attack bonus and never more than MAX_FEAT_POINTS.  Any
// other ability leaves the list empty.
//
// 0x587DF0
CGameButtonList* CInfButtonArray::BuildFeatPointsPickerList(const CResRef& resRef)
{
    CGameButtonList* pButtons = NULL;

    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() == 0) {
        return NULL;
    }

    LONG* pGroupList = g_pBaldurChitin->GetObjectGame()->GetGroup()->GetGroupList();
    LONG nCharacterId = pGroupList[0];
    delete pGroupList;

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return NULL;
    }

    // Only nBaseAttackBonus is used here; the other two outputs are scratch,
    // which is why the two calls pass them in opposite order.
    INT nBaseAttackBonus;
    INT nAttackCount;
    INT nAttackDivisor;
    BOOL bHasFeat = FALSE;

    if (resRef == CGameSprite::SPIN275) {
        if (pSprite->HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK)) {
            g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetBaseCombatValues(pSprite,
                nBaseAttackBonus, nAttackCount, nAttackDivisor, FALSE);
            bHasFeat = TRUE;
        }
    } else if (resRef == CGameSprite::SPIN276) {
        if (pSprite->HasFeat(CGAMESPRITE_FEAT_EXPERTISE)) {
            g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetBaseCombatValues(pSprite,
                nBaseAttackBonus, nAttackDivisor, nAttackCount, FALSE);
            bHasFeat = TRUE;
        }
    }

    if (bHasFeat) {
        INT nPoints = nBaseAttackBonus;
        if (nPoints > MAX_FEAT_POINTS) {
            nPoints = MAX_FEAT_POINTS;
        }

        pButtons = pSprite->GetFeatPointsButtonList(resRef, nPoints + 1);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return pButtons;
}

// Keyboard route into the action bar: CScreenWorld::OnKeyDown maps a shortcut
// key to a button type and calls this.  It only acts while the bar is in its
// normal state, and only for the seven types DispatchActionBarClick owns.  The
// return value tells the caller whether the bar took the key.
//
// 0x594170
BOOL CInfButtonArray::CheckActivation(LONG nButtonType)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (m_nState != 0x72) {
        return FALSE;
    }

    LONG nLeader = pGame->GetGroup()->GetGroupLeader();
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetDeny(nLeader,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    switch (nButtonType) {
    case 3:
    case 5:
    case 10:
    case 0xE:
    case 0x50:
    case 0x51:
    case 0x52:
        DispatchActionBarClick(nButtonType, pSprite);
        break;
    }

    pGame->GetObjectArray()->ReleaseDeny(nLeader,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// Handle the seven action-bar button types that need the party leader's sprite:
// Cast Spell, Skills, Special Abilities, Use Item and the three quick-item
// slots.  Reached from OnLButtonPressed and from the hotkey path at 0x594170.
//
// 0x594280
void CInfButtonArray::DispatchActionBarClick(INT nButtonType, CGameSprite* pSprite)
{
    UTIL_ASSERT(pSprite != NULL);

    switch (nButtonType) {
    case 3: {
        // Cast Spell.  Count the spellcasting classes that still have a
        // memorised level, plus the domain pool.  More than one caster source
        // opens the class picker (0x76); exactly one goes straight to that
        // class's spellbook (0x67); none does nothing at all.
        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        UpdateButtons();

        BYTE nClass = 0;
        UINT nCount = 0;

        for (UINT nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
            if (nCount > 1) {
                break;
            }

            UINT nLevel = 0;
            BOOL bHasSpells = FALSE;
            while (nLevel < pSprite->m_spells.m_spellsByClass[nClassIndex].m_nHighestLevel) {
                if (pSprite->m_spells.Get(nClassIndex)->GetSpellsAtLevel(nLevel)->m_nSharedCurrent != 0) {
                    bHasSpells = TRUE;
                    break;
                }

                nLevel++;
            }

            if (bHasSpells) {
                nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nClassIndex);
                nCount++;
            }
        }

        if (pSprite->m_domainSpells.m_nHighestLevel != 0) {
            for (UINT nLevel = 0; nLevel < pSprite->m_domainSpells.m_nHighestLevel; nLevel++) {
                if (pSprite->m_domainSpells.GetSpellsAtLevel(nLevel)->m_nSharedCurrent != 0) {
                    if (nCount == 0) {
                        // A cleric whose only memorised spells are domain
                        // spells still goes straight to the spellbook, with
                        // the specialization steering it to the domain list.
                        nClass = CAIOBJECTTYPE_C_CLERIC;
                        m_nCurrentSelectedSpellLevel = pSprite->m_baseStats.m_specialization;
                    }

                    nCount++;
                    break;
                }

                m_nCurrentSelectedSpellLevel = 0;
            }
        }

        if (nCount > 1) {
            SetState(0x76, 1);
            return;
        }

        if (nCount == 1) {
            m_nCurrentSelectedSpellClass = nClass;
            SetState(0x67, 1);
            return;
        }

        break;
    }
    case 5:
        // Skills.  While a modal skill is running the button just cancels it;
        // otherwise it opens the skills submenu.
        if (pSprite->m_nModalState != 0) {
            if (pSprite->m_nModalState == 3) {
                pSprite->SetModalState(0, 0);

                // 0x85BD1C
                pSprite->m_nStealthGreyOut = 90;
            } else {
                pSprite->SetModalState(0, 0);
            }

            m_nSelectedButton = 100;
            UpdateButtons();
            return;
        }

        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        UpdateButtons();
        SetState(0x73, 1);
        return;
    case 10:
        // Special Abilities.
        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        UpdateButtons();
        SetState(0x6A, 1);
        return;
    case 0xE:
        // Use Item.
        g_pBaldurChitin->GetObjectGame()->m_nState = 0;
        m_nSelectedButton = 0xE;
        pSprite->SetModalState(0, 0);
        UpdateButtons();
        SetState(0x69, 1);
        return;
    case 0x50:
    case 0x51:
    case 0x52: {
        // Quick item.  An item with two or more usable abilities opens the
        // ability picker; a single-ability item is readied straight away.
        CGameButtonList* pUsages = pSprite->GetItemUsages(
            static_cast<SHORT>(nButtonType - 0x41), 3, -1);

        if (pUsages != NULL && pUsages->GetCount() > 1) {
            m_nSelectedButton = nButtonType;
            m_nCustomizeSlot = nButtonType - 0x50;
            SetState(0x68, 1);
        } else {
            INT nPreviousButton = m_nSelectedButton;
            g_pBaldurChitin->GetObjectGame()->m_nState = 0;

            if (nPreviousButton == nButtonType) {
                m_nSelectedButton = 100;
                UpdateButtons();
            } else {
                m_nSelectedButton = nButtonType;
                pSprite->SetModalState(0, 0);
                ReadyQuickSlotByMode(static_cast<SHORT>(nButtonType - 0x50), 3);

                if (g_pBaldurChitin->GetObjectGame()->m_nState == 0) {
                    m_nSelectedButton = 100;
                }
            }

            // No UpdateButtons() on this arm.  The ready path sets
            // m_nSelectedButton but never re-syncs, so the slot's m_bSelected
            // stays clear and a quick item shows no red selection square
            // (Frida on the original 2026-06-20: m_nSelectedButton=0x50 yet the
            // button's m_bSelected=0).  The eventual cast-completion
            // UpdateState resets the selection.
        }

        if (pUsages != NULL) {
            while (pUsages->GetCount() != 0) {
                delete pUsages->RemoveHead();
            }

            pUsages->RemoveAll();
            delete pUsages;
        }

        break;
    }
    }
}

// Page the spellbook picker down by a whole memorised level instead of by ten
// entries: walk forward from the current page and stop on the first entry
// whose level differs from the page's own.  For spell entries m_bCanUse
// carries the level.  The list is passed in only to be checked; the walk
// itself runs on g_pButtonArrayPickerList.
//
// 0x595FB0
INT CInfButtonArray::GetNextPickerPage(CGameButtonList* pButtonList)
{
    UTIL_ASSERT(pButtonList != NULL);

    INT nSeen = 0;
    INT nPageLevel = 0;

    POSITION pos = g_pButtonArrayPickerList->FindIndex(m_nListStartIndex);
    while (pos != NULL) {
        CButtonData* pButtonData = g_pButtonArrayPickerList->GetNext(pos);
        if (pButtonData == NULL) {
            continue;
        }

        if (nSeen == 0) {
            nPageLevel = pButtonData->m_abilityId.m_bCanUse;
        } else if (pButtonData->m_abilityId.m_bCanUse != nPageLevel) {
            return m_nListStartIndex + nSeen;
        }

        nSeen++;
    }

    return 0;
}

// Page the spellbook picker up by a whole memorised level instead of by ten
// entries: walk backwards from the current page until the entry level changes
// twice, and land on the first entry of that previous level.  For spell
// entries m_bCanUse carries the level.  The list is passed in only to be
// checked; the walk itself runs on g_pButtonArrayPickerList.
//
// 0x596040
INT CInfButtonArray::GetPreviousPickerPage(CGameButtonList* pButtonList)
{
    UTIL_ASSERT(pButtonList != NULL);

    INT nSeen = 0;
    INT nPageLevel = 0;
    INT nPreviousLevel = -1;

    POSITION pos = g_pButtonArrayPickerList->FindIndex(m_nListStartIndex);
    while (pos != NULL) {
        CButtonData* pButtonData = g_pButtonArrayPickerList->GetPrev(pos);
        if (pButtonData == NULL) {
            continue;
        }

        if (nSeen == 0) {
            nPageLevel = pButtonData->m_abilityId.m_bCanUse;
        } else if (nPreviousLevel == -1
            && pButtonData->m_abilityId.m_bCanUse != nPageLevel) {
            nPreviousLevel = pButtonData->m_abilityId.m_bCanUse;
        } else if (pButtonData->m_abilityId.m_bCanUse != nPreviousLevel) {
            INT nPage = m_nListStartIndex - nSeen + 1;
            return nPage < 0 ? 0 : nPage;
        }

        nSeen++;
    }

    return 0;
}

// 0x58FF20
void CInfButtonArray::OnLButtonPressed(int buttonID)
{
    // TODO: Incomplete.

    if (buttonID < 0 || buttonID >= 12) {
        return;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    INT nButtonType = m_buttonTypes[buttonID];

    // Greyout gate per Ghidra OnLButtonPressed entry: a greyed-out slot
    // only blocks the click in state 0x72 (action bar) and in state 0x66
    // for non-picker button types.  In every other submenu state the
    // click must reach the handler â€” its default case is what pops the
    // submenu back to the action bar when the user clicks an empty slot.
    if (m_buttonArray[buttonID].m_bGreyOut) {
        if (m_nState == 0x72) {
            return;
        }
        if (m_nState == 0x66 && (nButtonType < 0x15 || nButtonType > 0x20)) {
            return;
        }
    }

    switch (m_nState) {
    case 0x6C:
        // Ghidra OnLButtonPressed state 0x6C (FUN_0058ff20): formation picker
        // reached by right-clicking a quick-formation slot.  Rebind the stashed
        // quick slot (m_nCustomizeSlot @ +0x1976, set by OnRButtonPressed state
        // 0x6E) to the chosen formation, then make it current:
        //   gameSave->m_quickFormations[slot] = buttonID;        // +0x423e
        //   gameSave->m_curFormation = m_quickFormations[slot];  // +0x423c
        if (buttonID < 12) {
            pGame->GetGameSave()->m_quickFormations[m_nCustomizeSlot] = static_cast<SHORT>(buttonID);
            pGame->GetGameSave()->m_curFormation = pGame->GetGameSave()->m_quickFormations[m_nCustomizeSlot];
            SetState(0x6E, 0);
        }
        return;
    case 0x6D:
        // Ghidra state 0x6D: formation selector that does NOT rebind a quick
        // slot â€” it only sets the current formation (gameSave +0x423c).
        if (buttonID < 12) {
            pGame->GetGameSave()->m_curFormation = static_cast<SHORT>(buttonID);
            SetState(0x6E, 0);
        }
        return;
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x70:
    case 0x71:
    case 0x7A:
    case 0x7B:
        // Page-up / page-down clicks - move m_nListStartIndex and re-render without
        // changing the state.  m_nListStartIndex is an entry index, not a page
        // number, so a page step is ten entries.  Holding shift in a spellbook
        // picker steps by a whole memorised level instead.
        if (nButtonType == 0x21) {
            if (m_nListStartIndex <= 0) {
                return;
            }

            if ((m_nState == 0x67 || m_nState == 0x66)
                && g_pBaldurChitin->pActiveEngine->GetShiftKey() == 1) {
                m_nListStartIndex = GetPreviousPickerPage(g_pButtonArrayPickerList);
                UpdateButtons();
                return;
            }

            INT nPage = m_nListStartIndex - 10;
            if (nPage < 0) {
                nPage = 0;
            }

            m_nListStartIndex = nPage;
            UpdateButtons();
            return;
        }
        if (nButtonType == 0x22) {
            if (g_pButtonArrayPickerList == NULL) {
                return;
            }

            INT nLastPage = static_cast<INT>(g_pButtonArrayPickerList->GetCount()) - 10;
            if (m_nListStartIndex >= nLastPage) {
                return;
            }

            if ((m_nState == 0x67 || m_nState == 0x66)
                && g_pBaldurChitin->pActiveEngine->GetShiftKey() == 1) {
                m_nListStartIndex = GetNextPickerPage(g_pButtonArrayPickerList);
                UpdateButtons();
                return;
            }

            if (nLastPage >= m_nListStartIndex + 10) {
                m_nListStartIndex = m_nListStartIndex + 10;
            } else {
                m_nListStartIndex = nLastPage;
            }

            UpdateButtons();
            return;
        }
        // Picker click.  Ghidra OnLButtonPressed 0x5913b3..0x5917df.  The click
        // is split in two: states 0x66 / 0x68 / 0x71 ASSIGN the picked entry to
        // the quick slot stashed in m_nCustomizeSlot, and then -- unless the
        // clicked cell is greyed out -- every state falls through to the same
        // "use it" dispatch.  A customise click therefore both binds the slot
        // and fires, which is why bUseNow below is false for exactly those
        // three states: the four Use*Action helpers turn it into
        // CGameSprite's `firstCall`, so a customise click readies rather than
        // executes.
        //
        //   0x66 / 0x67 / 0x6A / 0x6B -> UseSpellAction  (0x5886A0)
        //   0x68 / 0x69               -> UseItemAction   (0x5884B0)
        //   0x71 / 0x7A               -> UseSongAction   (0x588820)
        //   0x70                      -> UseInnateAction (0x588760, the default)
        //
        // The innate states 0x6A / 0x6B are a second, near-identical arm in the
        // binary (0x5924c9): same walk, same customise-then-fire shape, but its
        // own bUseNow (0x59242b) and no greyout gate on the fire.
        if (nButtonType >= 0x15 && nButtonType <= 0x20 && g_pButtonArrayPickerList != NULL) {
            BOOL bUseNow = (m_nState == 0x67 || m_nState == 0x69 || m_nState == 0x6A
                || m_nState == 0x70 || m_nState == 0x7A);

            // Resolve the clicked cell by walking the list from the current
            // page: the walk counts BUTTON slots, so it starts at 1 whenever
            // the list is long enough for slot 0 to be the page-up arrow.
            INT nIndex = (g_pButtonArrayPickerList->GetCount() > 12) ? 1 : 0;
            POSITION pos = g_pButtonArrayPickerList->FindIndex(m_nListStartIndex);
            CButtonData* pEntry = NULL;
            while (pos != NULL) {
                CButtonData* pCandidate = g_pButtonArrayPickerList->GetNext(pos);
                if (nIndex == buttonID && pCandidate != NULL) {
                    pEntry = pCandidate;
                    break;
                }
                nIndex++;
            }

            BOOL bFeatPointPicker = FALSE;
            BOOL bModalFeatToggled = FALSE;
            if (pEntry != NULL) {
                // NOTE: unrecovered -- in state 0x67 the binary first demands
                // the CSpell, builds a specialization mask and gates the whole
                // dispatch on CGameSprite::CanCast, feeding back "cannot cast"
                // instead (0x591502..0x5915d5).
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    // The slot binding goes through CustomizeQuickSlot, which
                    // takes its own Deny on GetGroupList()[0].  Nesting is what
                    // the binary does -- it holds one leader Deny across the
                    // whole of OnLButtonPressed (0x590027) -- and it is safe
                    // because GetDeny only refuses a lock held by ANOTHER
                    // thread; a re-entrant take just bumps m_denyCounts.
                    INT nSlot = m_nCustomizeSlot;
                    if (m_nState == 0x66) {
                        CustomizeQuickSlot(pEntry, static_cast<BYTE>(nSlot), 2);
                        m_customButtonTypes[nSlot] = nSlot + 0x46;
                        pSprite->SetCustomButtonValue(static_cast<BYTE>(nSlot), nSlot + 0x46);
                    } else if (m_nState == 0x68) {
                        CustomizeQuickSlot(pEntry, static_cast<BYTE>(nSlot), 3);
                        m_customButtonTypes[nSlot] = nSlot + 0x50;
                        pSprite->SetCustomButtonValue(static_cast<BYTE>(nSlot), nSlot + 0x50);
                    } else if (m_nState == 0x71) {
                        CustomizeQuickSlot(pEntry, static_cast<BYTE>(nSlot), 6);
                        m_customButtonTypes[nSlot] = nSlot + 0x6E;
                        pSprite->SetCustomButtonValue(static_cast<BYTE>(nSlot), nSlot + 0x6E);
                    } else if (m_nState == 0x6B) {
                        // The innate picker is its own arm in the binary
                        // (0x5924c9), with the same shape.
                        CustomizeQuickSlot(pEntry, static_cast<BYTE>(nSlot), 4);
                        m_customButtonTypes[nSlot] = nSlot + 0x5A;
                        pSprite->SetCustomButtonValue(static_cast<BYTE>(nSlot), nSlot + 0x5A);
                    } else if (m_nState == 0x6A) {
                        // Power Attack and Expertise do not fire from here:
                        // they open their point picker instead, with the
                        // chosen ability stashed for BuildFeatPointsPickerList.
                        if ((pEntry->m_abilityId.m_res == CGameSprite::SPIN275
                                && pSprite->HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK))
                            || (pEntry->m_abilityId.m_res == CGameSprite::SPIN276
                                && pSprite->HasFeat(CGAMESPRITE_FEAT_EXPERTISE))) {
                            m_currentAbilityResRef = pEntry->m_abilityId.m_res;
                            bFeatPointPicker = TRUE;
                        } else if (pEntry->m_abilityId.m_res == CGameSprite::SPIN277) {
                            // The other three modal feats have no picker: the
                            // click flips the feat rank and posts the matching
                            // effect at the sprite's own position.
                            pSprite->SetFeatRank(CGAMESPRITE_FEAT_ARTERIAL_STRIKE,
                                pSprite->GetFeatRank(CGAMESPRITE_FEAT_ARTERIAL_STRIKE) > 0 ? 0 : 1);

                            ITEM_EFFECT effect;
                            CGameEffect::ClearItemEffect(&effect,
                                ICEWIND_CGAMEEFFECT_FEATARTERIALSTRIKE);
                            effect.durationType = 1;

                            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                                pSprite->GetPos(), pSprite->GetId(), CPoint(-1, -1));
                            CMessage* pMsg = new CMessageAddEffect(pEffect,
                                pSprite->GetId(), pSprite->GetId());
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
                            bModalFeatToggled = TRUE;
                        } else if (pEntry->m_abilityId.m_res == CGameSprite::SPIN278) {
                            pSprite->SetFeatRank(CGAMESPRITE_FEAT_HAMSTRING,
                                pSprite->GetFeatRank(CGAMESPRITE_FEAT_HAMSTRING) > 0 ? 0 : 1);

                            ITEM_EFFECT effect;
                            CGameEffect::ClearItemEffect(&effect,
                                ICEWIND_CGAMEEFFECT_FEATHAMSTRING);
                            effect.durationType = 1;

                            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                                pSprite->GetPos(), pSprite->GetId(), CPoint(-1, -1));
                            CMessage* pMsg = new CMessageAddEffect(pEffect,
                                pSprite->GetId(), pSprite->GetId());
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
                            bModalFeatToggled = TRUE;
                        } else if (pEntry->m_abilityId.m_res == CGameSprite::SPIN279) {
                            pSprite->SetFeatRank(CGAMESPRITE_FEAT_RAPID_SHOT,
                                pSprite->GetFeatRank(CGAMESPRITE_FEAT_RAPID_SHOT) > 0 ? 0 : 1);

                            ITEM_EFFECT effect;
                            CGameEffect::ClearItemEffect(&effect,
                                ICEWIND_CGAMEEFFECT_FEATRAPIDSHOT);
                            effect.durationType = 1;

                            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                                pSprite->GetPos(), pSprite->GetId(), CPoint(-1, -1));
                            CMessage* pMsg = new CMessageAddEffect(pEffect,
                                pSprite->GetId(), pSprite->GetId());
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);
                            bModalFeatToggled = TRUE;
                        }
                    }

                    // A greyed-out cell can still be bound to a quick slot, but
                    // it never fires -- except in the innate arm, which has no
                    // such gate.  The binary keeps each helper's return in a
                    // flag that only the unrecovered tails read (0x59182f for
                    // this arm, 0x5924f0 for the innate one).
                    BOOL bInnateArm = (m_nState == 0x6A || m_nState == 0x6B);
                    if (!bFeatPointPicker && !bModalFeatToggled
                        && (bInnateArm || !m_buttonArray[buttonID].m_bGreyOut)) {
                        switch (m_nState) {
                        case 0x66:
                        case 0x67:
                        case 0x6A:
                        case 0x6B:
                            // NOTE: unrecovered -- state 0x6A first matches the
                            // entry's resref against two fixed ones and gates
                            // on CGameSprite::HasFeat(0x2F) (0x59256c).
                            UseSpellAction(pEntry, bUseNow);
                            break;
                        case 0x68:
                        case 0x69:
                            UseItemAction(pEntry, bUseNow);
                            break;
                        case 0x71:
                        case 0x7A:
                            UseSongAction(pEntry, bUseNow);
                            break;
                        default:
                            UseInnateAction(pEntry, bUseNow);
                            break;
                        }
                    }

                    pGame->GetObjectArray()->ReleaseDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }

            if (bFeatPointPicker) {
                SetState(0x7B, 1);
                return;
            }

            if (pEntry != NULL) {
                // Something was picked, so the whole sequence is done: walk
                // straight back to the bar it started from.
                PopState(0, 1);
                return;
            }
        }

        // An empty or unknown cell only steps back one level.
        PopState(0, 0);
        SetSelectedButton(100);
        UpdateButtons();
        return;
    case 0x6E:
        switch (nButtonType) {
        case 7:
            if (pGame->GetState() == 3) {
                pGame->SetState(0);
                SetSelectedButton(100);
            } else {
                pGame->SetState(3);
                SetSelectedButton(7);
            }
            UpdateButtons();
            return;
        case 8:
            if (pGame->GetState() == 2 && pGame->GetIconIndex() == 0x0C) {
                pGame->SetState(0);
                SetSelectedButton(100);
            } else {
                pGame->SetState(2);
                pGame->SetIconIndex(0x0C);
                SetSelectedButton(8);
            }
            UpdateButtons();
            return;
        case 0x0F:
            pGame->SetState(0);
            SetSelectedButton(100);
            pGame->GetGroup()->ClearActions();
            UpdateButtons();
            return;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14: {
            INT nFormationButton = nButtonType - 0x10;
            pGame->GetGameSave()->m_curFormation = pGame->GetGameSave()->m_quickFormations[nFormationButton];
            SetState(0x6E, 0);
            return;
        }
        default:
            break;
        }
        break;
    case 0x75:
        // Customize menu â€” left click writes the chosen button type into
        // m_customButtonTypes[m_nCustomizeSlot] and mirrors it onto the
        // sprite via SetCustomButtonValue.  Per Ghidra OnLButtonPressed
        // state 0x75 (FUN_0058FF20 around 0x592b14).  No state change after
        // write â€” the menu stays open and UpdateButtons re-paints the slot
        // with the new icon.  Slot index was stashed by OnRButtonPressed
        // state 0x72 customize-entry.
        {
            INT nNewType = -1;
            switch (nButtonType) {
            case 0x23: nNewType = 5;   break; // Attack
            case 0x24: nNewType = 3;   break; // Cast Spell
            case 0x25: nNewType = 0xE; break;
            case 0x26:
                SetState(0x78, 1);
                return;
            case 0x27: nNewType = 10;  break; // Innate ability
            case 0x28: nNewType = 2;   break; // Bard Song
            case 0x29: nNewType = 100; break; // No action
            case 0x2A: {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    for (BYTE i = 0; i < 9; i++) {
                        pSprite->SetCustomButtonValue(i, 0);
                    }
                    pSprite->ResetQuickSlots();
                    for (BYTE i = 0; i < 9; i++) {
                        m_customButtonTypes[i] = pSprite->GetCustomButtonValue(i);
                    }
                    pGame->GetObjectArray()->ReleaseDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                SetState(0x72, 0);
                return;
            }
            default:
                // Empty / unknown click in customize menu â€” pop back to
                // the action bar like every other submenu default exit.
                SetState(0x72, 0);
                return;
            }

            if (m_nCustomizeSlot >= 0 && m_nCustomizeSlot < 9) {
                m_customButtonTypes[m_nCustomizeSlot] = nNewType;
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    pSprite->SetCustomButtonValue(static_cast<BYTE>(m_nCustomizeSlot), nNewType);
                    pGame->GetObjectArray()->ReleaseDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            // Close the menu so the new icon shows.  NOTE: unrecovered -- the
            // binary walks the state stack back here rather than naming 0x72.
            SetState(0x72, 0);
        }
        return;
    case 0x73:
        // Skills submenu (entered from state 0x72 button 5). Per Ghidra
        // OnLButtonPressed state 0x73 handles Stealth, Search, Thieving,
        // Wilderness Lore, and Animal Empathy, then pops back to state 0x72
        // via the state-stack mechanism. Any other click exits the submenu.
        {
            LONG nLeader = pGame->GetGroup()->GetGroupLeader();
            CGameSprite* pSprite = NULL;
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                BYTE modal = pSprite->GetModalState();
                switch (nButtonType) {
                case 4: // Search
                    if (modal == 2) {
                        pSprite->SetModalState(0, 0);
                        SetSelectedButton(100);
                    } else {
                        pSprite->SetModalState(2, 0);
                        SetSelectedButton(5);
                    }
                    break;
                case 0xB: // Stealth
                    if (modal == 3) {
                        pSprite->SetModalState(0, 0);
                        SetSelectedButton(100);
                    } else {
                        pSprite->SetModalState(3, 0);
                        SetSelectedButton(5);
                    }
                    break;
                case 0xC: // Thieving
                    if (pGame->GetState() == 2
                        && (pGame->GetIconIndex() == 0x24 || pGame->GetIconIndex() == 0x28)) {
                        pGame->SetState(0);
                        SetSelectedButton(100);
                    } else {
                        pGame->SetState(2);
                        pGame->SetIconIndex(0x24);
                        SetSelectedButton(0xC);
                    }
                    // 0x593295: entering thieving/disarm mode cancels any active
                    // modal (Search/Stealth) on the leader -- unconditional.
                    pSprite->SetModalState(0, 0);
                    break;
                case 0xD: // Animal Empathy
                    // Per Ghidra OnL state 0x73 case 0xD: require Animal
                    // Empathy > 0, build DAT_008f8e60 ("SPIN108", Charm
                    // Animal), then dispatch via UseSpellAction.
                    if (static_cast<signed char>(pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_ANIMAL_EMPATHY]) > 0) {
                        CButtonData bd;
                        bd.m_abilityId.m_res = CResRef("SPIN108");
                        bd.m_abilityId.m_targetType = -1;
                        bd.m_abilityId.m_strDescription = -1;
                        bd.m_bDisabled = FALSE;
                        bd.m_bDisplayCount = TRUE;
                        pSprite->UseButtonAction(bd, 1);
                    }
                    break;
                case 0x77: { // TODO: Wilderness Lore in Ghidra; this port still cycles weapon sets.
                    BYTE nNext = static_cast<BYTE>((pSprite->m_nWeaponSet + 1) & 0x3);
                    pSprite->SetWeaponSet(nNext);
                    m_nQuickWeaponSlot = nNext;
                    break;
                }
                default:
                    break;
                }
                pGame->GetObjectArray()->ReleaseDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
            SetState(0x72, 0);
        }
        return;
    case 0x74:
        // Skills submenu in customize mode (entered from state 0x75 case
        // 0x23 right-click).  Click writes the chosen button type into
        // m_customButtonTypes[m_nCustomizeSlot] and the sprite mirror,
        // then exits to state 0x72.  Empty / unknown clicks just exit.
        if ((nButtonType == 4 || nButtonType == 0xB || nButtonType == 0xC
                || nButtonType == 0xD || nButtonType == 0x77)
            && m_nCustomizeSlot >= 0 && m_nCustomizeSlot < 9) {
            m_customButtonTypes[m_nCustomizeSlot] = nButtonType;
            LONG nLeader = pGame->GetGroup()->GetGroupLeader();
            CGameSprite* pSprite = NULL;
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                pSprite->SetCustomButtonValue(static_cast<BYTE>(m_nCustomizeSlot), nButtonType);
                pGame->GetObjectArray()->ReleaseDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        SetState(0x72, 0);
        return;
    case 0x77:
        // Customize class picker â€” entered from state 0x75 case 0x24 via
        // OnRButtonPressed.  Per Ghidra OnLButtonPressed state 0x77: each
        // class button (0x32-0x39) writes its own type into the customize
        // slot's m_customButtonTypes entry + sprite mirror, so the action
        // bar slot becomes a "per-class quick spell" launcher.  Other
        // clicks (empty slot, etc.) just pop back to 0x72.
        if (nButtonType >= 0x32 && nButtonType <= 0x39
            && m_nCustomizeSlot >= 0 && m_nCustomizeSlot < 9) {
            m_customButtonTypes[m_nCustomizeSlot] = nButtonType;
            LONG nLeader = pGame->GetGroup()->GetGroupLeader();
            CGameSprite* pSprite = NULL;
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                pSprite->SetCustomButtonValue(static_cast<BYTE>(m_nCustomizeSlot), nButtonType);
                pGame->GetObjectArray()->ReleaseDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        SetState(0x72, 0);
        return;
    case 0x79:
        // Quick-weapon picker (entered from state 0x72 right-click on a
        // weapon slot).  Per Ghidra OnLButtonPressed state 0x79: a click
        // on a 0x3C-0x43 button calls CGameSprite::SetWeaponSet with the
        // set index (buttonType - 0x3C) / 2, updates the array's tracked
        // quick-weapon slot, then exits to 0x72.  Any other click just
        // exits.
        if (nButtonType >= 0x3C && nButtonType <= 0x43) {
            LONG nLeader = pGame->GetGroup()->GetGroupLeader();
            CGameSprite* pSprite = NULL;
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                BYTE nNewSet = static_cast<BYTE>((nButtonType - 0x3C) / 2);
                pSprite->SetWeaponSet(nNewSet);
                m_nQuickWeaponSlot = pSprite->m_nWeaponSet;
                pGame->GetObjectArray()->ReleaseDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        SetState(0x72, 0);
        return;
    case 0x78:
        // Quick-item picker sub-menu reached from state 0x75 case 0x26.
        // Per Ghidra OnLButtonPressed state 0x78: a click on a 0x50-0x52
        // button writes that button type into m_customButtonTypes[slot]
        // (and the sprite mirror) and pops back to the action bar.  Any
        // other click just dismisses the picker.
        if (nButtonType >= 0x50 && nButtonType <= 0x52
            && m_nCustomizeSlot >= 0 && m_nCustomizeSlot < 9) {
            m_customButtonTypes[m_nCustomizeSlot] = nButtonType;
            LONG nLeader = pGame->GetGroup()->GetGroupLeader();
            CGameSprite* pSprite = NULL;
            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                pSprite->SetCustomButtonValue(static_cast<BYTE>(m_nCustomizeSlot), nButtonType);
                pGame->GetObjectArray()->ReleaseDeny(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        SetState(0x72, 0);
        return;
    default:
        switch (nButtonType) {
        case 3:
        case 5:
        case 10:
        case 0xE:
        case 0x50:
        case 0x51:
        case 0x52:
            // The seven types DispatchActionBarClick owns.  They are grouped
            // here rather than left scattered across the switch because they
            // all need the same thing: the party leader's sprite.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    DispatchActionBarClick(nButtonType, pSprite);
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            return;
        case 7:
            if (pGame->GetState() == 3) {
                pGame->SetState(0);
                SetSelectedButton(100);
            } else {
                pGame->SetState(3);
                SetSelectedButton(7);
            }
            // 0x5939e1: clears any active modal (Search/Stealth) on the leader.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    pSprite->SetModalState(0, 0);
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            UpdateButtons();
            return;
        case 8:
            if (pGame->GetState() == 2 && pGame->GetIconIndex() == 0x0C) {
                pGame->SetState(0);
                SetSelectedButton(100);
            } else {
                pGame->SetState(2);
                pGame->SetIconIndex(0x0C);
                SetSelectedButton(8);
            }
            // 0x593173: clears any active modal (Search/Stealth) on the leader.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    pSprite->SetModalState(0, 0);
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            UpdateButtons();
            return;
        case 0x0F:
            pGame->SetState(0);
            SetSelectedButton(100);
            pGame->GetGroup()->ClearActions();
            UpdateButtons();
            return;
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43: {
            // Shift opens the weapon ability picker (0x592D92).
            if (g_pBaldurChitin->pActiveEngine->GetShiftKey() == TRUE) {
                pGame->SetState(0);
                SetSelectedButton(100);
                m_nCustomizeSlot = nButtonType - 0x3C;
                UpdateButtons();
                SetState(0x65, 1);
                return;
            }
            INT nEffective = nButtonType;
            // Off-hand collapses to its main when the main is empty or holds a
            // 2H/ranged/sling/crossbow (ITEMTYPE 0x29/0x2F/0x31/0x35).  Ghidra
            // address calc: m_equipment.m_items[(button - 0x3C) + 43] for the
            // odd off-hand slot resolves to the same offset as the main slot.
            if ((nButtonType & 1) != 0) {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    INT nProbeSlot = (nButtonType - 0x3C) + 43;
                    CItem* pMain = pSprite->m_equipment.m_items[nProbeSlot];
                    if (pMain == NULL) {
                        nEffective = nButtonType - 1;
                    } else {
                        WORD itemType = pMain->GetItemType();
                        if (itemType == 0x2F || itemType == 0x35
                            || itemType == 0x31 || itemType == 0x29) {
                            nEffective = nButtonType - 1;
                        }
                    }
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            if (m_nSelectedButton != nEffective) {
                pGame->SetState(0);
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite), INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    pSprite->SetModalState(0, 0);
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }
                SetSelectedButton(nEffective);
                ReadyQuickSlotByMode(static_cast<SHORT>(nEffective - 0x3C), 1);
                if (pGame->GetState() == 0) {
                    SetSelectedButton(100);
                }
                UpdateButtons();
            } else {
                pGame->SetState(0);
                SetSelectedButton(100);
                UpdateButtons();
            }
            return;
        }
        case 2:
            // Bard song toggle.  Per Ghidra 0x58FF20 default case 2: read
            // sprite's modal state, if already singing (state 1) clear it,
            // otherwise enter song-select state 0x7A.  Simplified: toggle
            // modal state directly.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    if (pSprite->GetModalState() == 1) {
                        pSprite->SetModalState(0, 0);
                        SetSelectedButton(100);
                    } else {
                        pGame->SetState(0);
                        UpdateButtons();
                        SetState(0x7A, 1);
                    }
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                UpdateButtons();
            }
            return;
        case 4:
            // Search modal toggle.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    if (pSprite->GetModalState() == 2) {
                        pSprite->SetModalState(0, 0);
                        SetSelectedButton(100);
                    } else {
                        // 0x5930b3: entering Search announces "Searching" in the
                        // combat feedback before the modal state flips on.
                        pSprite->FeedBack(CGameSprite::FEEDBACK_SEARCHSTART, 0, 0, 0, -1, 0, 0);
                        pSprite->SetModalState(2, 0);
                        SetSelectedButton(5);
                    }
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                UpdateButtons();
            }
            return;
        case 0xB:
            // Stealth modal toggle (0x592EEC).  Toggling the modal alone is not
            // enough: entering stealth also queues an immediate Hide() action so
            // the sprite attempts to hide on the spot (CGameSprite::ExecuteAction
            // action 18 -> the detection pass) instead of waiting for the next
            // three-cycle modal upkeep, and leaving stealth applies a
            // FORCEVISIBLE effect so it reappears at once.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    if (pSprite->GetModalState() == 3) {
                        pSprite->SetModalState(0, 0);
                        pSprite->SetStealthGreyOut(90);
                        SetSelectedButton(100);

                        ITEM_EFFECT effect;
                        CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);
                        effect.durationType = 1;
                        CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                            pSprite->GetPos(),
                            pSprite->m_id,
                            CPoint(-1, -1));
                        CMessage* pMessage = new CMessageAddEffect(pEffect,
                            pSprite->m_id, pSprite->m_id);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                    } else {
                        pSprite->SetModalState(3, 0);
                        SetSelectedButton(5);

                        CAIAction action(18 /* Hide, ACTION.IDS */, CPoint(-1, -1), 0, -1);
                        CMessage* pMessage = new CMessageAddAction(action,
                            pSprite->m_id, pSprite->m_id);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
                    }
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                UpdateButtons();
            }
            return;
        case 0xC:
            // Thieving mode. Ghidra default case 0xC: SetState(2) and
            // IconIndex 0x24 (or toggle off when already in that mode).
            if (pGame->GetState() == 2
                && (pGame->GetIconIndex() == 0x24 || pGame->GetIconIndex() == 0x28)) {
                pGame->SetState(0);
                SetSelectedButton(100);
            } else {
                pGame->SetState(2);
                pGame->SetIconIndex(0x24);
                SetSelectedButton(0xC);
            }
            // 0x593177: entering thieving/disarm mode cancels any active modal
            // (Search/Stealth) on the leader -- unconditional.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    pSprite->SetModalState(0, 0);
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            UpdateButtons();
            return;
        case 0xD:
            // Animal Empathy: require skill > 0, then use SPIN108 (Charm Animal).
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    if (static_cast<signed char>(pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_ANIMAL_EMPATHY]) > 0) {
                        CButtonData bd;
                        bd.m_abilityId.m_res = CResRef("SPIN108");
                        bd.m_abilityId.m_targetType = -1;
                        bd.m_abilityId.m_strDescription = -1;
                        bd.m_bDisabled = FALSE;
                        bd.m_bDisplayCount = TRUE;
                        pSprite->UseButtonAction(bd, 1);
                    }
                    pGame->GetObjectArray()->ReleaseDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                UpdateButtons();
            }
            return;
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
            // Quick song click.  Ghidra default case 0x6E-0x76 reads the
            // assigned CButtonData via GetQuickSong, then calls
            // FUN_00588820(buttonData, 1) which dispatches through the AI
            // action table.  UseButtonAction is our generic equivalent â€” it
            // handles spell / item / ability / song uniformly.
            {
                CButtonData buttonData;
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    pSprite->GetQuickSong(static_cast<BYTE>(nButtonType - 0x6E), buttonData);
                    if (buttonData.m_icon != "") {
                        pSprite->UseButtonAction(buttonData, 0);
                    } else if (pSprite->GetModalState() == 1) {
                        // No assigned song â€” toggle out of song-modal mode.
                        pSprite->SetModalState(0, 0);
                    }
                    pGame->GetObjectArray()->ReleaseDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                pGame->SetState(0);
                SetSelectedButton(100);
                UpdateButtons();
            }
            return;
        case 0x77:
            // TODO: Ghidra identifies type 0x77 as Wilderness Lore (tooltip
            // 0x7DBA) and applies effect 0x11E. This port still cycles to the
            // next quick-weapon set.
            {
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    BYTE nNext = static_cast<BYTE>((pSprite->m_nWeaponSet + 1) & 0x3);
                    pSprite->SetWeaponSet(nNext);
                    m_nQuickWeaponSlot = nNext;
                    pGame->GetObjectArray()->ReleaseDeny(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                pGame->SetState(0);
                SetSelectedButton(100);
                UpdateState();
            }
            return;
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E: {
            // Quick spell cast (0x593559).
            pGame->SetState(0);
            SetSelectedButton(nButtonType);

            ReadyQuickSlotByMode(static_cast<SHORT>(nButtonType - 0x46), 2);

            if (pGame->GetState() == 0) {
                SetSelectedButton(100);
            }
            UpdateButtons();
            return;
        }
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62: {
            // Quick ability use (0x593902).
            pGame->SetState(0);
            SetSelectedButton(nButtonType);

            ReadyQuickSlotByMode(static_cast<SHORT>(nButtonType - 0x5A), 4);

            if (pGame->GetState() == 0) {
                SetSelectedButton(100);
            }
            UpdateButtons();
            return;
        }
        case 0x32:
            m_nCurrentSelectedSpellClass = 2;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        case 0x33:
            // Cleric â€” regular spells (m_nCurrentSelectedSpellLevel = 0
            // signals the picker dispatcher to use m_spells.m_spellsByClass).
            m_nCurrentSelectedSpellClass = 3;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        case 0x39:
            // Cleric DOMAIN.  Per Ghidra OnLButton case 0x39:
            //   m_nCurrentSelectedSpellClass = 3
            //   m_nCurrentSelectedSpellLevel = FUN_005940d0() == sprite[0x80C]
            // Any non-zero level acts as the "domain" sentinel for FUN_00587c20
            // case 2's dispatch; we use 1 here since the picker iterates ALL
            // levels of m_domainSpells anyway.
            m_nCurrentSelectedSpellClass = 3;
            m_nCurrentSelectedSpellLevel = 1;
            SetState(0x67, 1);
            return;
        case 0x34:
            m_nCurrentSelectedSpellClass = 4;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        case 0x35:
            m_nCurrentSelectedSpellClass = 7;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        case 0x36:
            m_nCurrentSelectedSpellClass = 8;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        case 0x37:
            m_nCurrentSelectedSpellClass = 10;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        case 0x38:
            m_nCurrentSelectedSpellClass = 11;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x67, 1);
            return;
        default:
            // Unhandled button click.  In submenu states (anything other
            // than the main action bar 0x72 or the group bar 0x6E), this
            // is how Ghidra exits â€” empty/unknown slot click pops the
            // state stack back to 0x72 via the default branch.  We don't
            // implement the state stack, so SetState(0x72, 0) directly.
            if (m_nState != 0x72 && m_nState != 0x6E) {
                SetState(0x72, 0);
            }
            return;
        }
        break;
    }
}

// 0x594720
void CInfButtonArray::OnRButtonPressed(int buttonID)
{
    // TODO: Incomplete â€” picker states (0x66/0x71/0x78/0x79) need SetState
    // helpers that build the per-class spell / item / ability lists.  When a
    // sub-list cannot be built, the right-click is currently silent.

    if (buttonID < 0 || buttonID >= 12) {
        return;
    }

    INT nButtonType = m_buttonTypes[buttonID];

    // Empty quick-slot (type 100) renders inactive so its bezel can stay
    // transparent, but right-clicking it in state 0x72 must still bring up
    // the customize menu â€” otherwise erased slots become permanent dead
    // zones.  UpdateButtons forces m_bGreyOut for type 100; bypass that
    // gate for the customize entry path.
    BOOL bCustomizeEntry = (m_nState == 0x72 && nButtonType == 100);
    if (m_buttonArray[buttonID].m_bGreyOut && !bCustomizeEntry) {
        return;
    }

    switch (m_nState) {
    case 0x6E:
        if (nButtonType >= 0x10 && nButtonType <= 0x14) {
            // Stash the right-clicked quick-formation slot (0-4) so the picker
            // (state 0x6C) can rebind it.  Ghidra OnRButtonPressed state 0x6E
            // writes m_nCustomizeSlot (+0x1976) â€” the same scratch field the
            // 0x6C picker reads.
            m_nCustomizeSlot = nButtonType - 0x10;
            SetState(0x6C, 1);
        }
        return;
    case 0x72:
        switch (nButtonType) {
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
            // Ghidra also calls CheckWeaponUsability() + sprite[0x4B80] check
            // first.  Both denials produce a feedback string then no state
            // change.  We skip the check and unconditionally enter state 0x79
            // (the quick-weapon picker); the picker itself isn't built yet so
            // this is a no-op until SetState(0x79) lands.
            m_nCurrentSelectedSpellLevel = nButtonType - 0x3C;
            SetState(0x79, 1);
            return;
        case 2:
        case 3:
        case 4:
        case 5:
        case 10:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 100:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
            // Customize the slot.  Ghidra stores (buttonID - 3) at
            // offset 0x1976 so state 0x75 can write back to the right
            // m_customButtonTypes entry.
            m_nCustomizeSlot = buttonID - 3;
            SetState(0x75, 1);
            return;
        }
        return;
    case 0x75:
        switch (nButtonType) {
        case 0x23:
            SetState(0x74, 1);
            return;
        case 0x26:
            SetState(0x78, 1);
            return;
        case 0x27:
            // Innate ability customize â€” Ghidra OnR state 0x75 case 0x27
            // checks sprite[0x4A94] (m_innateSpells internal head pointer)
            // is non-zero before opening the picker.  We use the
            // std::vector emptiness check on m_innateSpells.m_List.
            {
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                BOOL bHasInnate = FALSE;
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    bHasInnate = !pSprite->m_innateSpells.m_List.empty();
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                if (bHasInnate) {
                    SetState(0x6B, 1);
                }
            }
            return;
        case 0x28:
            // Bard Song customize â€” Ghidra OnR state 0x75 case 0x28 checks
            // the song list head + (end - head) / 16 (entry size).  We
            // approximate with std::vector emptiness on m_songs.m_List.
            {
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                BOOL bHasSong = FALSE;
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    bHasSong = !pSprite->m_songs.m_List.empty();
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                if (bHasSong) {
                    SetState(0x71, 1);
                }
            }
            return;
        case 0x24:
            // Cast Spell â€” pick state 0x66 (customize spellbook) for a
            // single-class caster and state 0x77 (customize-class-picker)
            // for multi-class.  Mirrors Ghidra OnRButtonPressed state 0x75
            // case 0x24 which iterates classes 2/3/4/7/8/10/11 and counts
            // those with memorised spells (plus the cleric domain pool).
            {
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
                LONG nLeader = pGame->GetGroup()->GetGroupLeader();
                CGameSprite* pSprite = NULL;
                BYTE rc = pGame->GetObjectArray()->GetShare(nLeader,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                INT nCasterCount = 0;
                BYTE nOnlyClass = 0;
                BOOL bDomainContributed = FALSE;
                if (rc == CGameObjectArray::SUCCESS && pSprite != NULL) {
                    static const BYTE classes[] = { 2, 3, 4, 7, 8, 10, 11 };
                    for (size_t i = 0; i < sizeof(classes) / sizeof(classes[0]); i++) {
                        CGameSpriteGroupedSpellList* grouped = pSprite->GetSpells(classes[i]);
                        if (grouped != NULL && grouped->m_nHighestLevel != 0) {
                            nCasterCount++;
                            nOnlyClass = classes[i];
                        }
                    }
                    if (pSprite->m_domainSpells.m_nHighestLevel != 0) {
                        if (nCasterCount == 0) {
                            nOnlyClass = 3;
                            bDomainContributed = TRUE;
                        }
                        nCasterCount++;
                    }
                    pGame->GetObjectArray()->ReleaseShare(nLeader,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
                if (nCasterCount == 1) {
                    m_nCurrentSelectedSpellClass = nOnlyClass;
                    m_nCurrentSelectedSpellLevel = bDomainContributed ? 1 : 0;
                    SetState(0x66, 1);
                } else if (nCasterCount >= 2) {
                    SetState(0x77, 1);
                }
            }
            return;
        }
        return;
    case 0x77:
        switch (nButtonType) {
        case 0x32:
            m_nCurrentSelectedSpellClass = 2;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x33:
            m_nCurrentSelectedSpellClass = 3;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x34:
            m_nCurrentSelectedSpellClass = 4;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x35:
            m_nCurrentSelectedSpellClass = 7;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x36:
            m_nCurrentSelectedSpellClass = 8;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x37:
            m_nCurrentSelectedSpellClass = 10;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x38:
            m_nCurrentSelectedSpellClass = 11;
            m_nCurrentSelectedSpellLevel = 0;
            SetState(0x66, 1);
            return;
        case 0x39:
            m_nCurrentSelectedSpellClass = 3;
            SetState(0x66, 1);
            return;
        }
        return;
    }
}
