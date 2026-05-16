#include "CInfButtonArray.h"
#include "debuglog.h"

#include "CBaldurChitin.h"
#include "CButtonData.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CScreenWorld.h"
#include "CUIPanel.h"
#include "CUIControlButton.h"

// 0x851700
const BYTE CInfButtonArray::STATE_NONE = 0;

// 0x587960
CInfButtonSettings::CInfButtonSettings()
{
    field_1C8 = 0;
    field_1CC = 0;
    field_1D8 = 0;
    field_0 = 0;
    field_4 = 0;
    field_8 = 0;
    field_1D0 = 0;
    m_bGreyOut = FALSE;
    field_C = -1;
    field_10 = -1;
}

// 0x587B80
CInfButtonSettings::~CInfButtonSettings()
{
}

// 0x5879E0
CInfButtonArray::CInfButtonArray()
{
    // TODO: Incomplete.
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

// 0x588FF0
BOOL CInfButtonArray::ResetState()
{
    // Minimal: set to STATE_NONE (no buttons)
    m_nSelectedButton = 100;
    SetState(STATE_NONE, 0);
    return TRUE;
}

// 0x589100
void CInfButtonArray::UpdateState()
{
    // Minimal: determine state from group selection count
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame != NULL) {
        CAIGroup* pGroup = pGame->GetGroup();
        INT nGroupCount = pGroup->GetCount();
        DBG("UpdateState called\n");
        if (nGroupCount > 1) {
            SetState(0x6E, 0);  // Group mode: Guard, Attack, Stop, Formations
        } else if (nGroupCount == 1) {
            SetState(0x65, 0);  // Single character mode
        } else {
            SetState(STATE_NONE, 0);
        }
    }
}

// 0x589110
BOOL CInfButtonArray::SetState(INT nState, int a2)
{
    // TODO: Incomplete — minimal implementation

    // Load BAM resources (matching Ghidra decomp: FUN_0078a990 = load BAM)
    // GUIBTBUT: button backgrounds, GUIBTACT: action icons (240 frames)
    field_16E8.SetResRef(CResRef("GUIBTBUT"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
    field_17C2.SetResRef(CResRef("GUIBTACT"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);
    field_189C.SetResRef(CResRef("GUIBTBUT"), g_pBaldurChitin->field_4A2C, TRUE, TRUE);

    // Button type IDs (matching Ghidra decomp)
    // Group mode (0x6E): Guard, Attack, Stop, Formations 1-5, 4 empty
    // Single char mode (0x65): Action buttons 0x15-0x20 (21-32)

    switch (nState) {
    case 0x65: // Single character
        m_buttonTypes[0] = 0x15;
        m_buttonTypes[1] = 0x16;
        m_buttonTypes[2] = 0x17;
        m_buttonTypes[3] = 0x18;
        m_buttonTypes[4] = 0x19;
        m_buttonTypes[5] = 0x1A;
        m_buttonTypes[6] = 0x1B;
        m_buttonTypes[7] = 0x1C;
        m_buttonTypes[8] = 0x1D;
        m_buttonTypes[9] = 0x1E;
        m_buttonTypes[10] = 0x1F;
        m_buttonTypes[11] = 0x20;
        m_nState = 0x65;
        break;
    case 0x6E: // Group selected
        m_buttonTypes[0] = 7;   // Guard
        m_buttonTypes[1] = 8;   // Attack
        m_buttonTypes[2] = 0xF; // Stop
        m_buttonTypes[3] = 0x10; // Formation 1
        m_buttonTypes[4] = 0x11; // Formation 2
        m_buttonTypes[5] = 0x12; // Formation 3
        m_buttonTypes[6] = 0x13; // Formation 4
        m_buttonTypes[7] = 0x14; // Formation 5
        m_buttonTypes[8] = 100;  // Empty
        m_buttonTypes[9] = 100;  // Empty
        m_buttonTypes[10] = 100; // Empty
        m_buttonTypes[11] = 100; // Empty
        m_nState = 0x6E;
        break;
    default:
        // Unsupported state — set all to empty
        for (int i = 0; i < 12; i++) {
            m_buttonTypes[i] = 100;
        }
        m_nState = nState;
        break;
    }

    UpdateButtons();
    return TRUE;
}

// 0x58A340
void CInfButtonArray::UpdateButtons()
{
    // TODO: Incomplete — sets correct BAM frames for each button type
    // Icon mapping (GUIBTACT.BAM: 60 icons, 4 frames each):
    //   Icon 0 (frames 0-3)   = Guard
    //   Icon 3 (frames 12-15)  = Attack
    //   Icon 11 (frames 44-47) = Stop
    //   Icons for formations: TBD

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame == NULL) return;

    // Get panel 3 which has 12 button controls (indices 6-17)
    CUIPanel* pPanel = g_pBaldurChitin->GetScreenWorld()->GetManager()->GetPanel(3);
    if (pPanel == NULL) {
        DBG("UpdateButtons: Panel 3 is NULL — trying panel 4");
        pPanel = g_pBaldurChitin->GetScreenWorld()->GetManager()->GetPanel(4);
    }
    if (pPanel == NULL) {
        DBG("UpdateButtons: Panel 4 also NULL");
        // Try to find which panels exist
        for (DWORD id = 0; id < 30; id++) {
            if (g_pBaldurChitin->GetScreenWorld()->GetManager()->GetPanel(id) != NULL) {
                DBG("UpdateButtons: Panel %d exists", id);
            }
        }
        return;
    }
    pPanel->SetActive(TRUE);

    DBG("UpdateButtons: Panel 3 found, setting buttons\n");

    for (int i = 0; i < 12; i++) {
        CUIControlButton* pButton = static_cast<CUIControlButton*>(pPanel->GetControl(6 + i));
        if (pButton == NULL) {
            DBG("UpdateButtons: button is NULL\n");
            continue;
        }

        int nType = m_buttonTypes[i];
        int nIcon = -1;

        switch (nType) {
        case 7:  nIcon = 0;  break;  // Guard
        case 8:  nIcon = 3;  break;  // Attack
        case 0xF: nIcon = 11; break;  // Stop
        default: break;
        }

        if (nIcon >= 0) {
            pButton->m_nNormalFrame = nIcon * 4;
            pButton->m_nPressedFrame = nIcon * 4 + 1;
            pButton->SetActive(TRUE);
        } else {
            pButton->SetActive(FALSE);
        }
        pButton->InvalidateRect();
    }
    
    pPanel->InvalidateRect(NULL);
}

// 0x452C50
void CInfButtonArray::SetSelectedButton(INT nSelectedButton)
{
    m_nSelectedButton = nSelectedButton;
}

// 0x58FF20
void CInfButtonArray::OnLButtonPressed(int buttonID)
{
    // TODO: Incomplete.
}

// 0x594720
void CInfButtonArray::OnRButtonPressed(int buttonID)
{
    // TODO: Incomplete.
}
