#include "CScreenStore.h"

#include "CBaldurChitin.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CIcon.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CScreenInventory.h"
#include "CScreenSpellbook.h"
#include "CScreenWorld.h"
#include "CSpell.h"
#include "CStore.h"
#include "CUIControlTextDisplay.h"
#include "CUIPanel.h"
#include "CUtil.h"

// 0x859330
const INT CScreenStore::STORE_SLOTS = 6;

// 0x859334
const INT CScreenStore::GROUP_SLOTS = 6;

// 0x859338
const INT CScreenStore::SPELL_SLOTS = 6;

// 0x85933C
const INT CScreenStore::DRINK_SLOTS = 8;

// 0x859340
const INT CScreenStore::IDENTIFY_SLOTS = 6;

// 0x859344
const INT CScreenStore::NUM_BOTTOMBUTTONS = CSCREENSTORE_NUM_BOTTOMBUTTONS;

// 0x859348
const WORD CScreenStore::SEQUENCE_BUYSELL = 0;

// 0x85934A
const WORD CScreenStore::SEQUENCE_IDENTIFY = 1;

// 0x85934C
const WORD CScreenStore::SEQUENCE_STEAL = 2;

// 0x85934E
const WORD CScreenStore::SEQUENCE_BUYSPELL = 3;

// 0x859350
const WORD CScreenStore::SEQUENCE_DONATE = 4;

// 0x859352
const WORD CScreenStore::SEQUENCE_BUYDRINK = 5;

// 0x859354
const WORD CScreenStore::SEQUENCE_RENTROOM = 6;

// 0x859358
const DWORD CScreenStore::ROOMTYPE_NONE = 0;

// 0x85935C
const DWORD CScreenStore::ROOMTYPE_PEASANT = 1;

// 0x859360
const DWORD CScreenStore::ROOMTYPE_MERCHANT = 2;

// 0x859364
const DWORD CScreenStore::ROOMTYPE_NOBLE = 3;

// 0x859368
const DWORD CScreenStore::ROOMTYPE_ROYAL = 4;

// 0x85936C
const DWORD CScreenStore::NUM_ROOMTYPES = 4;

// 0x859370
const DWORD CScreenStore::SELL_MAXNUM_STORETOBAG = 999;

// 0x8F8194
const CString CScreenStore::TOKEN_HP("HP");

// 0x8F8158
const CString CScreenStore::TOKEN_ITEMNAME("ITEMNAME");

// 0x8F8188
const CString CScreenStore::TOKEN_ITEMCOST("ITEMCOST");

// 0x8F8168
CRITICAL_SECTION CScreenStore::m_critSect;

// 0x8F8198
BOOLEAN CScreenStore::m_bInCancelEngine;

// -----------------------------------------------------------------------------

// 0x672360
CScreenStoreItem::CScreenStoreItem()
{
    m_cResSpell = "";
    m_pItem = NULL;
    m_bEnabled = FALSE;
    m_bSelected = FALSE;
    m_nValue = 0;
    m_nSingleValue = 0;
    m_nStoreCount = 1;
    m_nMaxCount = 1;
    m_nCount = 1;
    m_nSlot = -1;
}

// -----------------------------------------------------------------------------

// 0x6709E0
CScreenStore::CScreenStore()
{
    m_dwRoomType = 0;
    field_57C = 0;
    field_5A4 = 0;
    field_5A8 = 0;
    m_nErrorState = 0;
    m_nNumErrorButtons = 0;
    field_5D8 = 0;
    field_5D9 = 0;
    field_14DA = NULL;
    field_14DE = 0;
    field_14E2 = 0;
    field_14E6 = 0;

    SetVideoMode(0);

    m_pVirtualKeys[0] = CKeyInfo(VK_ESCAPE, -1, 0);
    m_pVirtualKeys[1] = CKeyInfo('1', 0, 0);
    m_pVirtualKeys[2] = CKeyInfo('2', 0, 0);
    m_pVirtualKeys[3] = CKeyInfo('3', 0, 0);
    m_pVirtualKeys[4] = CKeyInfo('4', 0, 0);
    m_pVirtualKeys[5] = CKeyInfo('5', 0, 0);
    m_pVirtualKeys[6] = CKeyInfo('6', 0, 0);
    m_pVirtualKeys[7] = CKeyInfo('7', 0, 0);
    m_pVirtualKeys[8] = CKeyInfo('8', 0, 0);
    m_pVirtualKeys[9] = CKeyInfo('9', 0, 0);
    m_pVirtualKeys[10] = CKeyInfo(VK_SPACE, -1, 0);
    m_pVirtualKeys[11] = CKeyInfo('A', 0, 0);
    m_pVirtualKeys[12] = CKeyInfo('B', 0, 0);
    m_pVirtualKeys[13] = CKeyInfo('C', 0, 0);
    m_pVirtualKeys[14] = CKeyInfo('D', 0, 0);
    m_pVirtualKeys[15] = CKeyInfo('E', 0, 0);
    m_pVirtualKeys[16] = CKeyInfo('F', 0, 0);
    m_pVirtualKeys[17] = CKeyInfo('G', 0, 0);
    m_pVirtualKeys[18] = CKeyInfo('H', 0, 0);
    m_pVirtualKeys[19] = CKeyInfo('I', 0, 0);
    m_pVirtualKeys[20] = CKeyInfo('J', 0, 0);
    m_pVirtualKeys[21] = CKeyInfo('K', 0, 0);
    m_pVirtualKeys[22] = CKeyInfo('L', 0, 0);
    m_pVirtualKeys[23] = CKeyInfo('M', 0, 0);
    m_pVirtualKeys[24] = CKeyInfo('N', 0, 0);
    m_pVirtualKeys[25] = CKeyInfo('O', 0, 0);
    m_pVirtualKeys[26] = CKeyInfo('P', 0, 0);
    m_pVirtualKeys[27] = CKeyInfo('Q', 0, 0);
    m_pVirtualKeys[28] = CKeyInfo('R', 0, 0);
    m_pVirtualKeys[29] = CKeyInfo('S', 0, 0);
    m_pVirtualKeys[30] = CKeyInfo('T', 0, 0);
    m_pVirtualKeys[31] = CKeyInfo('U', 0, 0);
    m_pVirtualKeys[32] = CKeyInfo('V', 0, 0);
    m_pVirtualKeys[33] = CKeyInfo('W', 0, 0);
    m_pVirtualKeys[34] = CKeyInfo('X', 0, 0);
    m_pVirtualKeys[35] = CKeyInfo('Y', 0, 0);
    m_pVirtualKeys[36] = CKeyInfo('Z', 0, 0);
    m_pVirtualKeys[37] = CKeyInfo('0', 0, 0);
    m_pVirtualKeys[38] = CKeyInfo(VK_BACK, 0, 0);
    m_pVirtualKeys[39] = CKeyInfo(VK_END, -1, 0);
    m_pVirtualKeys[40] = CKeyInfo(VK_HOME, -1, 0);
    m_pVirtualKeys[41] = CKeyInfo(VK_LEFT, 0, 0);
    m_pVirtualKeys[42] = CKeyInfo(VK_UP, 0, 0);
    m_pVirtualKeys[43] = CKeyInfo(VK_RIGHT, 0, 0);
    m_pVirtualKeys[44] = CKeyInfo(VK_DOWN, 0, 0);
    m_pVirtualKeys[45] = CKeyInfo(VK_SNAPSHOT, -1, 0);
    m_pVirtualKeys[46] = CKeyInfo(VK_INSERT, -1, 0);
    m_pVirtualKeys[47] = CKeyInfo(VK_DELETE, 0, 0);
    m_pVirtualKeys[48] = CKeyInfo(VK_NUMPAD0, -1, 0);
    m_pVirtualKeys[49] = CKeyInfo(VK_NUMPAD1, -1, 0);
    m_pVirtualKeys[50] = CKeyInfo(VK_NUMPAD2, -1, 0);
    m_pVirtualKeys[51] = CKeyInfo(VK_NUMPAD3, -1, 0);
    m_pVirtualKeys[52] = CKeyInfo(VK_NUMPAD4, -1, 0);
    m_pVirtualKeys[53] = CKeyInfo(VK_NUMPAD5, -1, 0);
    m_pVirtualKeys[54] = CKeyInfo(VK_NUMPAD6, -1, 0);
    m_pVirtualKeys[55] = CKeyInfo(VK_NUMPAD7, -1, 0);
    m_pVirtualKeys[56] = CKeyInfo(VK_NUMPAD8, -1, 0);
    m_pVirtualKeys[57] = CKeyInfo(VK_NUMPAD9, -1, 0);
    m_pVirtualKeys[58] = CKeyInfo(VK_MULTIPLY, -1, 0);
    m_pVirtualKeys[59] = CKeyInfo(VK_ADD, -1, 0);
    m_pVirtualKeys[60] = CKeyInfo(VK_SEPARATOR, -1, 0);
    m_pVirtualKeys[61] = CKeyInfo(VK_SUBTRACT, -1, 0);
    m_pVirtualKeys[62] = CKeyInfo(VK_DECIMAL, -1, 0);
    m_pVirtualKeys[63] = CKeyInfo(VK_DIVIDE, -1, 0);
    m_pVirtualKeys[64] = CKeyInfo(VK_F1, -1, 0);
    m_pVirtualKeys[65] = CKeyInfo(VK_F2, -1, 0);
    m_pVirtualKeys[66] = CKeyInfo(VK_F3, -1, 0);
    m_pVirtualKeys[67] = CKeyInfo(VK_F4, -1, 0);
    m_pVirtualKeys[68] = CKeyInfo(VK_F5, -1, 0);
    m_pVirtualKeys[69] = CKeyInfo(VK_F6, -1, 0);
    m_pVirtualKeys[70] = CKeyInfo(VK_F7, -1, 0);
    m_pVirtualKeys[71] = CKeyInfo(VK_F8, -1, 0);
    m_pVirtualKeys[72] = CKeyInfo(VK_F9, -1, 0);
    m_pVirtualKeys[73] = CKeyInfo(VK_F10, -1, 0);
    m_pVirtualKeys[74] = CKeyInfo(VK_F11, -1, 0);
    m_pVirtualKeys[75] = CKeyInfo(VK_F12, -1, 0);
    m_pVirtualKeys[76] = CKeyInfo(VK_NUMLOCK, -1, 0);
    m_pVirtualKeys[77] = CKeyInfo(VK_RETURN, -1, 0);
    m_pVirtualKeys[78] = CKeyInfo(VK_OEM_3, 0, 0);
    m_pVirtualKeys[79] = CKeyInfo(VK_OEM_MINUS, 0, 0);
    m_pVirtualKeys[80] = CKeyInfo(VK_OEM_PLUS, 0, 0);
    m_pVirtualKeys[81] = CKeyInfo(VK_OEM_4, 0, 0);
    m_pVirtualKeys[82] = CKeyInfo(VK_OEM_6, 0, 0);
    m_pVirtualKeys[83] = CKeyInfo(VK_OEM_5, 0, 0);
    m_pVirtualKeys[84] = CKeyInfo(VK_OEM_1, 0, 0);
    m_pVirtualKeys[85] = CKeyInfo(VK_OEM_7, 0, 0);
    m_pVirtualKeys[86] = CKeyInfo(VK_OEM_COMMA, 0, 0);
    m_pVirtualKeys[87] = CKeyInfo(VK_OEM_PERIOD, 0, 0);
    m_pVirtualKeys[88] = CKeyInfo(VK_OEM_2, 0, 0);
    m_pVirtualKeys[89] = CKeyInfo(VK_TAB, -1, 0);

    // FIXME: Buffer overrun.
    m_pVirtualKeys[90] = CKeyInfo(VK_PROCESSKEY, -1, 0);

    m_bCtrlKeyDown = FALSE;
    m_bShiftKeyDown = FALSE;
    m_bCapsLockKeyOn = FALSE;
    m_pMainPanel = NULL;
    m_nTopGroupItem = 0;
    m_nTopStoreItem = 0;
    m_nTopSpellItem = 0;
    m_nTopDrinkItem = 0;
    m_nTopIdentifyItem = 0;
    m_nStoreCost = 0;
    m_nGroupCost = 0;
    m_dwSpellCost = 0;
    m_dwIdentifyCost = 0;
    m_pStore = NULL;
    m_pBag = NULL;
    m_cResBag = "";
    m_cResStore = "";
    m_nDrinkRumorIndex = -1;
    field_584 = -1;
    m_pButtonBar = NULL;
    m_pChatDisplay = NULL;
    m_nChatMessageCount = 0;

    InitializeCriticalSection(&m_critSect);
}

// 0x49FC40
BOOL CScreenStore::CheckSystemKeyShift()
{
    return TRUE;
}

// 0x671790
void CScreenStore::SetSystemKeyShift(BOOL bValue)
{
    m_bShiftKeyDown = bValue;
}

// 0x6717A0
BOOL CScreenStore::GetShiftKey()
{
    return m_bShiftKeyDown;
}

// 0x49FC40
BOOL CScreenStore::CheckSystemKeyCapsLock()
{
    return TRUE;
}

// 0x6717B0
void CScreenStore::SetSystemKeyCapsLock(BOOL bValue)
{
    m_bCapsLockKeyOn = bValue;
}

// 0x6717C0
BOOL CScreenStore::GetCapsLockKey()
{
    return m_bCapsLockKeyOn;
}

// 0x49FC40
BOOL CScreenStore::CheckSystemKeyCtrl()
{
    return TRUE;
}

// 0x6717D0
void CScreenStore::SetSystemKeyCtrl(BOOLEAN bValue)
{
    m_bCtrlKeyDown = bValue;
}

// 0x6717E0
BOOL CScreenStore::GetCtrlKey()
{
    return m_bCtrlKeyDown;
}

// 0x49FC40
BOOL CScreenStore::CheckMouseMove()
{
    return TRUE;
}

// 0x49FC40
BOOL CScreenStore::CheckMouseLButton()
{
    return TRUE;
}

// 0x5D76C0
void CScreenStore::OnLButtonDblClk(CPoint pt)
{
    m_cUIManager.OnLButtonDblClk(pt);
}

// 0x636970
void CScreenStore::OnLButtonDown(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 1;
    m_cUIManager.OnLButtonDown(pt);
}

// 0x6369A0
void CScreenStore::OnLButtonUp(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 0;
    m_cUIManager.OnLButtonUp(pt);
}

// 0x49FC40
BOOL CScreenStore::CheckMouseRButton()
{
    return TRUE;
}

// 0x6369D0
void CScreenStore::OnRButtonDown(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 1;
    m_cUIManager.OnRButtonDown(pt);
}

// 0x636A00
void CScreenStore::OnRButtonUp(CPoint pt)
{
    g_pBaldurChitin->GetObjectCursor()->m_nState = 0;
    m_cUIManager.OnRButtonUp(pt);
}

// 0x6717F0
SHORT CScreenStore::GetNumVirtualKeys()
{
    return CSCREENSTORE_VIRTUAL_KEYS;
}

// 0x699610
CKeyInfo* CScreenStore::GetVirtualKeys()
{
    return m_pVirtualKeys;
}

// 0x671800
BYTE* CScreenStore::GetVirtualKeysFlags()
{
    return m_pVirtualKeysFlags;
}

// 0x671830
CScreenStore::~CScreenStore()
{
    if (m_pStore != NULL) {
        delete m_pStore;
        m_pStore = NULL;
    }

    m_cResStore = "";
    m_cResBag = "";

    DestroyGroupItems(TRUE);

    // NOTE: Uninlie.
    DestroySpellItems();

    // NOTE: Uninline.
    DestroyIdentifyItems();

    // NOTE: Uninline.
    DestroyStoreItems(TRUE);

    DeleteCriticalSection(&m_critSect);
}

// 0x671A20
void CScreenStore::EngineActivated()
{
    if (CChitin::byte_8FB950
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE
        && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
    }

    if (m_cUIManager.m_bInitialized) {
        // NOTE: Unused.
        CResRef v1;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 364
        UTIL_ASSERT(m_pButtonBar != NULL);

        m_preLoadFontRealms.SetResRef(CResRef("REALMS"), FALSE, TRUE);
        m_preLoadFontRealms.RegisterFont();

        m_preLoadFontStnSml.SetResRef(CResRef("STONESML"), FALSE, TRUE);
        m_preLoadFontStnSml.RegisterFont();

        m_preLoadFontTool.SetResRef(CResRef("TOOLFONT"), FALSE, TRUE);
        m_preLoadFontTool.RegisterFont();

        for (INT nButtonIndex = 0; nButtonIndex < CSCREENSTORE_NUM_BOTTOMBUTTONS; nButtonIndex++) {
            CUIControlButton* pButton = static_cast<CUIControlButton*>(m_pButtonBar->GetControl(nButtonIndex + 1));

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 386
            UTIL_ASSERT(pButton != NULL);

            pButton->SetToolTipStrRef(GetPanelButtonToolTip(nButtonIndex), -1, -1);
        }

        if (m_pChatDisplay != NULL) {
            m_nChatMessageCount = g_pBaldurChitin->GetBaldurMessage()->m_cChatBuffer.UpdateTextDisplay(m_pChatDisplay,
                m_nChatMessageCount);
            m_pChatDisplay->ScrollToBottom();
        }

        if (GetTopPopup() == NULL) {
            UpdateMainPanel();
        }

        // NOTE: Uninline.
        CheckEnableButtonBar();

        CheckEnablePortaits(1);
        CheckEnableLeftPanel();
        g_pBaldurChitin->GetObjectCursor()->m_bVisible = TRUE;
        UpdateCursorShape(0);
        m_cUIManager.InvalidateRect(NULL);
    }
}

// 0x671CC0
void CScreenStore::EngineDeactivated()
{
    if (CChitin::byte_8FB950) {
        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
            && g_pChitin->cNetwork.GetSessionHosting() == TRUE
            && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            if (!g_pBaldurChitin->m_pEngineWorld->m_bPaused) {
                g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 1, 0);
            }
        }
    }

    m_preLoadFontRealms.Unload();
    m_preLoadFontStnSml.Unload();
    m_preLoadFontTool.Unload();

    // NOTE: Uninline.
    m_cUIManager.KillCapture();
}

// 0x671D60
void CScreenStore::EngineGameInit()
{
    m_cUIManager.fInit(this, CResRef("GUISTORE"), g_pBaldurChitin->m_bUseNewGui);

    CPoint pt;
    if (g_pBaldurChitin->m_bUseNewGui) {
        pt.x = CVideo::SCREENWIDTH / 2 - CBaldurChitin::DEFAULT_SCREEN_WIDTH;
        pt.y = CVideo::SCREENHEIGHT / 2 - CBaldurChitin::DEFAULT_SCREEN_HEIGHT;
    } else {
        pt.x = (CVideo::SCREENWIDTH - CBaldurChitin::DEFAULT_SCREEN_WIDTH) / 2;
        pt.y = (CVideo::SCREENHEIGHT - CBaldurChitin::DEFAULT_SCREEN_HEIGHT) / 2;
    }

    m_cUIManager.ShiftPanels(pt);

    for (int side = 0; side < 4; side++) {
        m_cUIManager.AddPanel(&(g_pBaldurChitin->m_aBorderPanels[side]));
    }

    m_nSelectedCharacter = 0;
    m_pCurrentScrollBar = NULL;
    m_pMainPanel = NULL;
    m_nTopGroupItem = 0;
    m_nTopStoreItem = 0;
    m_nTopSpellItem = 0;
    m_nTopDrinkItem = 0;
    m_nTopIdentifyItem = 0;
    m_nStoreCost = 0;
    m_nGroupCost = 0;
    m_dwSpellCost = 0;
    m_dwIdentifyCost = 0;
    m_pStore = 0;
    m_pBag = NULL;
    m_nDrinkRumorIndex = -1;
    field_584 = -1;
    m_pButtonBar = NULL;
    m_pChatDisplay = NULL;
    m_nChatMessageCount = 0;

    m_cUIManager.GetPanel(4)->SetActive(FALSE);
    m_cUIManager.GetPanel(2)->SetActive(FALSE);
    m_cUIManager.GetPanel(5)->SetActive(FALSE);
    m_cUIManager.GetPanel(7)->SetActive(FALSE);
    m_cUIManager.GetPanel(8)->SetActive(FALSE);
    m_cUIManager.GetPanel(10)->SetActive(FALSE);
    m_cUIManager.GetPanel(11)->SetActive(FALSE);
    m_cUIManager.GetPanel(12)->SetActive(FALSE);
    m_cUIManager.GetPanel(14)->SetActive(FALSE);
    m_cUIManager.GetPanel(3)->SetActive(FALSE);
    m_cUIManager.GetPanel(15)->SetActive(FALSE);
    m_cUIManager.GetPanel(20)->SetActive(FALSE);
}

// 0x671FA0
void CScreenStore::EngineGameUninit()
{
    m_pMainPanel = NULL;

    // NOTE: Uninline.
    m_cUIManager.fUninit();
}

// 0x671FC0
void CScreenStore::OnKeyDown(SHORT nKeysFlags)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 661
    UTIL_ASSERT(pGame != NULL);

    for (SHORT nKeyFlag = 0; nKeyFlag < nKeysFlags; nKeyFlag++) {
        if (!m_cUIManager.OnKeyDown(m_pVirtualKeysFlags[nKeyFlag])) {
            switch (m_pVirtualKeysFlags[nKeyFlag]) {
            case VK_TAB:
                m_cUIManager.ForceToolTip();
                break;
            case 13:
                if (GetTopPopup() == NULL) {
                    if (g_pBaldurChitin->m_pEngineWorld->m_bInControlOfStore) {
                        if (m_pStore->GetType() == 4) {
                            SelectEngine(g_pBaldurChitin->m_pEngineInventory);
                        } else {
                            SelectEngine(g_pBaldurChitin->m_pEngineWorld);
                        }

                        g_pBaldurChitin->m_pEngineWorld->StopStore();
                    }
                } else {
                    OnDoneButtonClick();
                }
                break;
            case VK_ESCAPE:
                if (GetTopPopup() == NULL) {
                    if (g_pBaldurChitin->m_pEngineWorld->m_bInControlOfStore) {
                        SelectEngine(g_pBaldurChitin->m_pEngineWorld);
                        g_pBaldurChitin->m_pEngineWorld->StopStore();
                    }
                } else {
                    OnCancelButtonClick();
                }
                break;
            case VK_SNAPSHOT:
                g_pBaldurChitin->GetCurrentVideoMode()->PrintScreen();
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
                if (GetTopPopup() == NULL) {
                    OnPortraitLClick(m_pVirtualKeysFlags[nKeyFlag] - '1');
                }
                break;
            case VK_ADD:
                if (GetTopPopup() != NULL
                    && GetTopPopup()->m_nID == 20) {
                    CScreenStoreItem cItem;
                    if (field_14E6) {
                        GetStoreItem(field_14E2, cItem);
                    } else {
                        GetGroupItem(field_14E2, cItem);
                    }

                    if (field_14DE < cItem.m_nMaxCount) {
                        field_14DE += 1;
                        UpdateRequesterPanel();
                    }
                }
                break;
            case VK_SUBTRACT:
                if (GetTopPopup() != NULL
                    && GetTopPopup()->m_nID == 20) {
                    // NOTE: Signed compare.
                    if (static_cast<int>(field_14DE) > 0) {
                        field_14DE -= 1;
                        UpdateRequesterPanel();
                    }
                }
                break;
            default:
                for (SHORT index = 0; index < CINFGAME_KEYMAP_SIZE; index) {
                    if (pGame->GetKeymap(index) == m_pVirtualKeysFlags[nKeyFlag]
                        && pGame->GetKeymapFlag(index) == m_bCtrlKeyDown) {
                        switch (index) {
                        case 24:
                            FocusChatEditBox();
                            break;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

// 0x6723B0
void CScreenStore::OnMouseMove(CPoint pt)
{
    CUIPanel* pPanel;
    CUIControlBase* pControl;
    CUIControlScrollBar* pScrollBar;

    // FIXME: Calls `GetPanel` with the same ID.
    if (m_cUIManager.GetPanel(2)->m_bActive) {
        pPanel = m_cUIManager.GetPanel(2);
        pControl = pPanel->GetControl(11);

        // NOTE: Odd condition - only checks `x`.
        if (pControl != NULL
            && pt.x - pPanel->m_ptOrigin.x < pControl->m_ptOrigin.x + pControl->m_size.cx) {
            pScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(11));
        } else {
            pScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(12));
        }
        field_14DA = pScrollBar;
        m_pCurrentScrollBar = pScrollBar;
    } else if (m_cUIManager.GetPanel(4)->m_bActive) {
        pPanel = m_cUIManager.GetPanel(4);
        pControl = pPanel->GetControl(23);
        if (pControl != NULL && pControl->IsOver(pt - pPanel->m_ptOrigin)) {
            pScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(24));
        } else {
            pScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(7));
        }
        m_pCurrentScrollBar = pScrollBar;
    }

    m_cUIManager.OnMouseMove(pt);
}

// 0x6724D0
void CScreenStore::OnPortraitLClick(DWORD nPortrait)
{
    LONG nNewCharacterId;
    LONG nOldCharacterId;
    CGameSprite* pNewSprite;
    CGameSprite* pOldSprite;
    BYTE rc;
    CMessage* pMessage;

    if (m_cUIManager.GetPanel(1)->m_bActive
        && (IsCharacterInRange(static_cast<SHORT>(nPortrait))
            || (m_pMainPanel->m_nID == 5
                && !IsCharacterAlive(static_cast<SHORT>(nPortrait))))) {
        DWORD nOldSelectedCharacter = m_nSelectedCharacter;
        m_nSelectedCharacter = nPortrait;

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            // NOTE: Uninline.
            LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait));

            for (SHORT index = 0; index < CINFGAME_MAXCHARACTERS; index++) {
                if (g_pBaldurChitin->GetObjectGame()->m_characters[index] == nCharacterId) {
                    g_pBaldurChitin->GetBaldurMessage()->UpdateDemandCharacters(0, index, 0);
                }
            }
        }

        // NOTE: Uninline.
        nNewCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nPortrait));

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nNewCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pNewSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // NOTE: Uninline.
            nOldCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(static_cast<SHORT>(nOldSelectedCharacter));

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nOldCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pOldSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                m_cAICustomer.Set(pNewSprite->GetAIType());

                pMessage = new CMessage101(TRUE,
                    pNewSprite->GetId(),
                    pNewSprite->GetId(),
                    FALSE);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

                pMessage = new CMessage101(FALSE,
                    pOldSprite->GetId(),
                    pOldSprite->GetId(),
                    FALSE);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nOldCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                // FIXME: Leaks share if new sprite if old not found.
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nNewCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        if (m_pBag != NULL) {
            CloseBag(TRUE);
            UpdateStoreItems();
            CheckEnablePortaits(1);
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1040
        UTIL_ASSERT(m_pMainPanel != NULL);

        switch (m_pMainPanel->m_nID) {
        case 2:
            UpdateGroupItems();

            // NOTE: Uninline.
            SelectAllGroupItems(FALSE);

            // NOTE: Uninline.
            SetTopGroupItem(0);

            // NOTE: Uninline.
            UpdateGroupCost();

            break;
        case 4:
            UpdateIdentifyItems();

            // NOTE: Uninline.
            SelectAllIdentifyItems(FALSE);

            // NOTE: Uninline.
            SetTopIdentifyItem(0);

            // NOTE: Uninline.
            UpdateIdentifyCost();

            break;
        case 5:
            UpdateSpellItems();

            // NOTE: Uninline.
            UpdateSpellCost();

            break;
        }

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
            && !g_pChitin->cNetwork.GetSessionHosting()
            && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            pMessage = new CMessage101(TRUE,
                m_nSelectedCharacter,
                m_nSelectedCharacter,
                FALSE);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

            pMessage = new CMessage101(FALSE,
                nOldSelectedCharacter,
                nOldSelectedCharacter,
                FALSE);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }

        UpdateMainPanel();

        m_cUIManager.GetPanel(1)->GetControl(nOldSelectedCharacter)->InvalidateRect();
        m_cUIManager.GetPanel(1)->GetControl(m_nSelectedCharacter)->InvalidateRect();
        m_cUIManager.InvalidateRect(NULL);
    }
}

// 0x672A00
void CScreenStore::TimerAsynchronousUpdate()
{
    if (m_pChatDisplay != NULL) {
        m_nChatMessageCount = g_pBaldurChitin->GetBaldurMessage()->m_cChatBuffer.UpdateTextDisplay(m_pChatDisplay,
            m_nChatMessageCount);
    }

    CGameSprite* pCustomer = static_cast<CGameSprite*>(m_cAICustomer.GetObjectWithType(NULL, CGameObject::TYPE_SPRITE, FALSE));
    if (m_pMainPanel != NULL && m_pMainPanel->m_nID != 5) {
        if (pCustomer != NULL
            && (pCustomer->GetBaseStats()->m_generalState & STATE_DEAD) == 0
            && (pCustomer->GetDerivedStats()->m_generalState & STATE_DEAD) == 0) {
            if (!pCustomer->InControl()) {
                SelectEngine(g_pBaldurChitin->GetScreenWorld());
                g_pBaldurChitin->GetScreenWorld()->StopStore();
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        } else {
            SelectEngine(g_pBaldurChitin->GetScreenWorld());
            g_pBaldurChitin->GetScreenWorld()->StopStore();
            if (pCustomer != NULL) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    } else {
        if (pCustomer != NULL) {
            if (m_pStore->GetType() == 4) {
                if (pCustomer->FindItemPersonal(m_pStore->m_resRef.GetResRefStr(), 0, FALSE) == -1) {
                    SelectEngine(g_pBaldurChitin->GetScreenWorld());
                    g_pBaldurChitin->GetScreenWorld()->StopStore();
                    if (pCustomer != NULL) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                    return;
                }
            }

            if (m_pBag != NULL) {
                if (pCustomer->FindItemPersonal(m_pBag->m_resRef.GetResRefStr(), 0, FALSE) == -1) {
                    SelectEngine(g_pBaldurChitin->GetScreenWorld());
                    g_pBaldurChitin->GetScreenWorld()->StopStore();
                    if (pCustomer != NULL) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                    return;
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            CGameSprite* pProprietor = static_cast<CGameSprite*>(m_cAIProprietor.GetObjectWithType(NULL, CGameObject::TYPE_SPRITE, FALSE));
            if (pProprietor != NULL
                && pProprietor->GetAIType().GetEnemyAlly() != CAIObjectType::EA_ENEMY
                && (pProprietor->GetBaseStats()->m_generalState & STATE_DEAD) == 0
                && (pProprietor->GetDerivedStats()->m_generalState & STATE_DEAD) == 0) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                g_pBaldurChitin->GetScreenWorld()->AsynchronousUpdate(FALSE);
                UpdateCursorShape(0);
                m_cUIManager.TimerAsynchronousUpdate();
                g_pBaldurChitin->GetObjectCursor()->CursorUpdate(pVidMode);
            } else {
                SelectEngine(g_pBaldurChitin->GetScreenWorld());
                g_pBaldurChitin->GetScreenWorld()->StopStore();
                if (pCustomer != NULL) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        } else {
            SelectEngine(g_pBaldurChitin->GetScreenWorld());
            g_pBaldurChitin->GetScreenWorld()->StopStore();
            if (pCustomer != NULL) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(pCustomer->GetId(),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }
}

// 0x672D50
void CScreenStore::TimerSynchronousUpdate()
{
    EnterCriticalSection(&m_critSect);

    g_pBaldurChitin->GetObjectGame()->SynchronousUpdate();
    m_cUIManager.Render();

    for (INT nIndex = 0; nIndex < m_lPopupStack.GetCount(); nIndex++) {
        POSITION pos = m_lPopupStack.FindIndex(nIndex);
        if (pos != NULL) {
            CUIPanel* pPanel = m_lPopupStack.GetAt(pos);
            pPanel->SetActive(TRUE);
            pPanel->SetInactiveRender(TRUE);
            pPanel->InvalidateRect(NULL);

            // NOTE: Inlining?
            if (pPanel->m_nID != 20 && pPanel->m_nID == 12) {
                CUIPanel* pPanel = m_cUIManager.GetPanel(12);
                CUIControlBase* pControl = pPanel->GetControl(9);
                pControl->SetActive(field_5A8);
                pControl->SetInactiveRender(field_5A8);
            }
        }
    }

    LeaveCriticalSection(&m_critSect);

    pVidMode->Flip(TRUE);
}

// 0x672E20
void CScreenStore::EnableMainPanel(BOOL bEnable)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1284
    UTIL_ASSERT(m_pButtonBar != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1285
    UTIL_ASSERT(m_pMainPanel != NULL);

    if (bEnable) {
        PlayGUISound(RESREF_SOUND_WINDOWCLOSE);
    }

    if (m_pMainPanel->m_nID == 2) {
        m_pMainPanel->GetControl(50)->SetInactiveRender(m_pBag != NULL);
    }

    m_pButtonBar->SetEnabled(bEnable);

    if (CVideo::SCREENWIDTH / (g_pBaldurChitin->m_bUseNewGui ? 2 : 1) != CBaldurChitin::DEFAULT_SCREEN_WIDTH) {
        m_cUIManager.GetPanel(-5)->SetEnabled(bEnable);
        m_cUIManager.GetPanel(-4)->SetEnabled(bEnable);
        m_cUIManager.GetPanel(-3)->SetEnabled(bEnable);
        m_cUIManager.GetPanel(-2)->SetEnabled(bEnable);
    }

    CheckEnablePanels(bEnable);
}

// NOTE: Inlined.
CUIPanel* CScreenStore::GetTopPopup()
{
    return m_lPopupStack.GetTailPosition() != NULL ? m_lPopupStack.GetTail() : NULL;
}

// NOTE: Inlined.
void CScreenStore::ShowPopupPanel(DWORD dwPanelId, BOOL bShow)
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPanelId);

    pPanel->SetActive(bShow);
    pPanel->SetInactiveRender(bShow);

    if (bShow) {
        pPanel->InvalidateRect(NULL);
        PlayGUISound(RESREF_SOUND_WINDOWOPEN);
    }
}

// NOTE: Inlined.
void CScreenStore::EnablePopupPanel(DWORD dwPanelId, BOOL bEnable)
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPanelId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1375
    UTIL_ASSERT(pPanel != NULL);

    pPanel->SetEnabled(bEnable);
}

// NOTE: Inlined.
void CScreenStore::ResetPopupPanel(DWORD dwPanelId)
{
    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPanelId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1402
    UTIL_ASSERT(pPanel != NULL);

    switch (pPanel->m_nID) {
    case 10:
    case 11:
        ResetErrorPanel(pPanel);
        break;
    case 12:
        UpdateItemPanel(pPanel);
        break;
    case 14:
        UpdateSpellPanel(pPanel);
        break;
    case 20:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1423
        UTIL_ASSERT(FALSE);
    }
}

// NOTE: Inlined.
void CScreenStore::UpdatePopupPanel(DWORD dwPanelId)
{
    switch (dwPanelId) {
    case 12:
        if (1) {
            CUIControlBase* pControl = m_cUIManager.GetPanel(12)->GetControl(9);
            pControl->SetActive(field_5A8);
            pControl->SetInactiveRender(field_5A8);
        }
        break;
    case 20:
        UpdateRequesterPanel();
        break;
    }
}

// 0x672F50
void CScreenStore::SummonPopup(DWORD dwPopupId)
{
    // NOTE: Uninline.
    m_cUIManager.KillCapture();

    if (!m_lPopupStack.IsEmpty()) {
        CUIPanel* pPanel = m_lPopupStack.GetTail();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1547
        UTIL_ASSERT(pPanel != NULL);

        // NOTE: Uninline.
        EnablePopupPanel(pPanel->m_nID, FALSE);
    } else {
        EnableMainPanel(FALSE);
    }

    CUIPanel* pPanel = m_cUIManager.GetPanel(dwPopupId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1553
    UTIL_ASSERT(pPanel != NULL);

    m_lPopupStack.AddTail(pPanel);

    // NOTE: Uninline.
    ResetPopupPanel(pPanel->m_nID);

    // NOTE: Uninline.
    ShowPopupPanel(pPanel->m_nID, TRUE);

    // NOTE: Uninline.
    EnablePopupPanel(pPanel->m_nID, TRUE);

    // NOTE: Uninline.
    UpdatePopupPanel(pPanel->m_nID);
}

// 0x673170
void CScreenStore::DismissPopup()
{
    // NOTE: Uninline.
    m_cUIManager.KillCapture();

    CUIPanel* pPanel = m_lPopupStack.RemoveTail();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1598
    UTIL_ASSERT(pPanel != NULL);

    // NOTE: Uninline.
    ShowPopupPanel(pPanel->m_nID, FALSE);

    m_pMainPanel->InvalidateRect(NULL);

    if (m_lPopupStack.GetTailPosition() != NULL) {
        CUIPanel* pPanel = m_lPopupStack.GetTail();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1615
        UTIL_ASSERT(pPanel != NULL);

        // NOTE: Uninline.
        ShowPopupPanel(pPanel->m_nID, TRUE);

        // NOTE: Uninline.
        EnablePopupPanel(pPanel->m_nID, TRUE);

        // NOTE: Uninline.
        UpdatePopupPanel(pPanel->m_nID);
    } else {
        EnableMainPanel(TRUE);
        UpdateMainPanel();
    }
}

// 0x6732F0
void CScreenStore::ResetErrorPanel(CUIPanel* pPanel)
{
    switch (pPanel->m_nID) {
    case 10:
        m_nNumErrorButtons = 1;
        break;
    case 11:
        m_nNumErrorButtons = 2;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1655
        UTIL_ASSERT(FALSE);
    }

    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(m_strErrorText, strRes);

    strRes.cSound.SetChannel(0, 0);
    strRes.cSound.SetFireForget(TRUE);
    strRes.cSound.Play(FALSE);

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(3));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1668
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();
    UpdateText(pText, "%s", strRes.szText);

    for (INT nButton = 0; nButton < m_nNumErrorButtons; nButton++) {
        CUIControlButton* pButton = static_cast<CUIControlButton*>(pPanel->GetControl(nButton));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1677
        UTIL_ASSERT(pButton != NULL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8744
        UTIL_ASSERT(0 <= nButton && nButton < CSCREENSTORE_ERROR_BUTTONS);

        pButton->SetText(FetchString(m_strErrorButtonText[nButton]));
    }
}

// 0x6734F0
void CScreenStore::UpdateItemPanel(CUIPanel* pPanel)
{
    // FIXME: Unused.
    CString v1;

    CItem* pItem = field_5A4;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1713
    UTIL_ASSERT(pItem != NULL);

    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(6));

    CUIControlButtonInventoryHistoryImage* pImage = static_cast<CUIControlButtonInventoryHistoryImage*>(pPanel->GetControl(7));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1719
    UTIL_ASSERT(pImage != NULL);

    pImage->SetImage(pItem->GetDescriptionPicture());

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(5));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1724
    UTIL_ASSERT(pText != NULL);

    pText->m_sNameSeparator = " ";

    CSize frameSize;
    pText->m_labelFont.GetFrameSize(64, 0, frameSize, FALSE);

    SHORT nFontHeight = pText->m_labelFont.GetFontHeight(FALSE);
    pText->field_AB4 = static_cast<SHORT>((nFontHeight + frameSize.cy - 1) / nFontHeight);

    pText->RemoveAll();
    pItem->FormatItemDescription(pText, RGB(200, 200, 0));
    pText->SetTopString(pText->m_plstStrings->FindIndex(0));

    CUIControlButtonInventoryHistoryIcon* pIcon = static_cast<CUIControlButtonInventoryHistoryIcon*>(pPanel->GetControl(2));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1738
    UTIL_ASSERT(pIcon != NULL);

    pIcon->SetItem(pItem);

    UpdateLabel(pPanel,
        0x10000000,
        "%s",
        pItem->GetGenericName());

    if ((pItem->m_flags & 0x1) == 0) {
        // "NOT IDENTIFIED"
        UpdateLabel(pPanel,
            0x10000007,
            "%s",
            FetchString(17108));
    } else {
        UpdateLabel(pPanel, 0x10000007, "");
    }
}

// 0x673740
void CScreenStore::UpdateSpellPanel(CUIPanel* pPanel)
{
    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(pPanel->GetControl(4));

    CSpell cSpell;
    cSpell.SetResRef(m_cResInfoSpell, TRUE, TRUE);
    cSpell.Demand();

    UpdateLabel(pPanel,
        0x10000000,
        "%s",
        FetchString(cSpell.GetGenericName()));

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(3));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1787
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();

    UpdateText(pText,
        "%s",
        FetchString(cSpell.GetDescription()));

    CUIControlButtonSpellbookSpellInfoIcon* pIcon = static_cast<CUIControlButtonSpellbookSpellInfoIcon*>(pPanel->GetControl(2));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 1794
    UTIL_ASSERT(pIcon != NULL);

    pIcon->SetSpell(m_cResInfoSpell);

    cSpell.Release();
}

// 0x673930
BOOL CScreenStore::IsCharacterAlive(SHORT nPortraitNum)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BOOL bAlive;

    // NOTE: Uninline.
    LONG nCharacterId = pGame->GetCharacterId(nPortraitNum);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetDerivedStats()->m_generalState & STATE_DEAD) != 0) {
            bAlive = FALSE;
        } else {
            bAlive = TRUE;
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    return bAlive;
}

// NOTE: Inlined.
void CScreenStore::CheckEnableButtonBar()
{
    for (INT nButtonIndex = 0; nButtonIndex < CSCREENSTORE_NUM_BOTTOMBUTTONS; nButtonIndex++) {
        CUIControlButton* pButton = static_cast<CUIControlButton*>(m_pButtonBar->GetControl(nButtonIndex + 1));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 1967
        UTIL_ASSERT(pButton != NULL);

        // NOTE: Uninline.
        DWORD dwId = GetPanelButtonPanelId(nButtonIndex);

        pButton->SetActive(dwId != -1);
        pButton->SetInactiveRender(dwId != -1);
    }
}

// 0x673A00
void CScreenStore::SwitchMainPanel(DWORD dwMainPanelId)
{
    EnterCriticalSection(&m_critSect);

    if (m_pBag != NULL) {
        CloseBag(TRUE);
        UpdateStoreItems();
    }

    if (m_pMainPanel != NULL) {
        m_pMainPanel->SetActive(FALSE);
    }

    m_pMainPanel = m_cUIManager.GetPanel(dwMainPanelId);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2013
    UTIL_ASSERT(m_pMainPanel != NULL);

    m_pMainPanel->SetActive(TRUE);
    m_pMainPanel->InvalidateRect(NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2017
    UTIL_ASSERT(m_pButtonBar != NULL);

    for (INT nButtonIndex = 0; nButtonIndex < CSCREENSTORE_NUM_BOTTOMBUTTONS; nButtonIndex++) {
        CUIControlButton3State* pButton = static_cast<CUIControlButton3State*>(m_pButtonBar->GetControl(nButtonIndex + 1));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2022
        UTIL_ASSERT(pButton != NULL);

        // NOTE: Uninline.
        DWORD dwId = GetPanelButtonPanelId(nButtonIndex);

        pButton->SetSelected(dwId == dwMainPanelId);
    }

    ResetMainPanel();
    UpdateMainPanel();

    LeaveCriticalSection(&m_critSect);
}

// 0x673B60
void CScreenStore::CheckEnablePanels(BOOL bEnable)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2055
    UTIL_ASSERT(m_pMainPanel != NULL);

    CUIPanel* pRightPanel = m_cUIManager.GetPanel(1);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2057
    UTIL_ASSERT(pRightPanel != NULL);

    CUIPanel* pLeftPanel = m_cUIManager.GetPanel(0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2059
    UTIL_ASSERT(pLeftPanel != NULL);

    pLeftPanel->SetEnabled(FALSE);

    if (bEnable) {
        switch (m_pMainPanel->m_nID) {
        case 2:
        case 4:
        case 5:
        case 8:
            pRightPanel->SetEnabled(TRUE);
            break;
        case 7:
            pRightPanel->SetEnabled(FALSE);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 2100
            UTIL_ASSERT(FALSE);
        }
    } else {
        pRightPanel->SetEnabled(FALSE);
    }

    // NOTE: Uninline.
    CheckEnableButtonBar();

    CheckEnablePortaits(1);
    CheckEnableLeftPanel();
}

// 0x673D20
void CScreenStore::ResetMainPanel()
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2133
    UTIL_ASSERT(m_pMainPanel != NULL);

    switch (m_pMainPanel->m_nID) {
    case 2:
        UpdateGroupItems();
        UpdateStoreItems();

        // NOTE: Uninline.
        SelectAllGroupItems(FALSE);

        // NOTE: Uninline.
        SelectAllStoreItems(FALSE);

        // NOTE: Uninline.
        SetTopGroupItem(0);

        // NOTE: Uninline.
        SetTopStoreItem(0);

        // NOTE: Uninline.
        UpdateGroupCost();

        // NOT: Uninline.
        UpdateStoreCost();

        CheckEnablePanels(TRUE);
        break;
    case 4:
        UpdateIdentifyItems();

        // NOTE: Uninline.
        SelectAllIdentifyItems(FALSE);

        // NOTE: Uninline.
        SetTopIdentifyItem(0);

        // NOTE: Uninline.
        UpdateIdentifyCost();

        CheckEnablePanels(TRUE);
        break;
    case 5:
        UpdateSpellItems();

        // NOTE: Uninline.
        SelectAllSpellItems(FALSE);

        SetTopSpellItem(0);
        UpdateSpellCost();

        CheckEnablePanels(TRUE);
        break;
    case 7:
        m_dwRoomType = 1;

        CheckEnablePanels(TRUE);
        break;
    case 8:
        SetTopDrinkItem(0);

        CheckEnablePanels(TRUE);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2287
        UTIL_ASSERT(FALSE);
    }
}

// 0x673FE0
void CScreenStore::UpdateMainPanel()
{
    if (m_pStore != NULL) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2221
        UTIL_ASSERT(m_pMainPanel != NULL);

        m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(m_pMainPanel->GetControl(7));

        CUIControlButton* pButton = static_cast<CUIControlButton*>(m_pButtonBar->GetControl(0));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2225
        UTIL_ASSERT(pButton != NULL);

        pButton->SetEnabled(g_pBaldurChitin->m_pEngineWorld->m_bInControlOfStore);

        switch (m_pMainPanel->m_nID) {
        case 2:
            UpdateBuySellPanel();
            break;
        case 4:
            UpdateIdentifyPanel();
            break;
        case 5:
            UpdateBuySpellPanel();
            break;
        case 7:
            UpdateRentRoomPanel();
            break;
        case 8:
            UpdateBuyDrinksPanel();
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 2259
            UTIL_ASSERT(FALSE);
        }
    }
}

// 0x6740F0
void CScreenStore::UpdateBuySellPanel()
{
    STR_RES strRes;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2316
    UTIL_ASSERT(pGame != NULL);

    CGameSave* pSave = pGame->GetGameSave();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2318
    UTIL_ASSERT(pSave != NULL);

    BOOL bCharacterViewable = IsCharacterViewable(static_cast<SHORT>(m_nSelectedCharacter));

    if (field_14DA == NULL) {
        field_14DA = static_cast<CUIControlScrollBar*>(m_pMainPanel->GetControl(11));
    }

    m_pCurrentScrollBar = field_14DA;

    CString sTitle;
    INT nWeight = 0;
    INT nMaxWeight = 0;
    INT nUsedSlots = 0;
    INT nTotalSlots = 0;

    if (m_pBag == NULL) {
        LONG nCharacterId = CGameObjectArray::INVALID_INDEX;
        if (m_nSelectedCharacter < pGame->m_nCharacters) {
            nCharacterId = pGame->m_characterPortraits[m_nSelectedCharacter];
        }

        CGameObject* pObject;

        BYTE rc;
        do {
            rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

            if (pSprite->m_baseStats.m_name == -1) {
                sTitle = pSprite->GetName();
            } else {
                sTitle = FetchString(pSprite->m_baseStats.m_name);
            }

            nWeight = static_cast<INT>(pSprite->GetCarriedWeight());

            SHORT nWeightAllowance = static_cast<SHORT>(atol(pGame->m_ruleTables.m_tStrengthMod.GetAt(CPoint(3, pSprite->m_derivedStats.m_nSTR))));

            nMaxWeight = static_cast<INT>(
                static_cast<float>(pGame->m_ruleTables.GetEncumbranceMod(pSprite)) / 100.0f
                * static_cast<float>(nWeightAllowance));

            pSprite->GetNumInventoryPersonalSlots(nUsedSlots, nTotalSlots);

            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            sTitle = "";
        }
    } else {
        sTitle = FetchString(m_pBag->m_header.m_strName);

        nUsedSlots = m_pBag->GetNumItems();
        nTotalSlots = m_pBag->m_header.m_nCapacity;
        if (nTotalSlots == 0) {
            nTotalSlots = 0x7FFFFFFF;
        }
    }

    CUIControlEncumbrance* pEncumbrance = static_cast<CUIControlEncumbrance*>(m_pMainPanel->GetControl(44));

    if (bCharacterViewable) {
        pEncumbrance->SetEncumbrance(nWeight, nMaxWeight);
        pEncumbrance->SetVolume(nUsedSlots, nTotalSlots);
    } else {
        pEncumbrance->SetEncumbrance(0, 0);
        pEncumbrance->SetVolume(0, 0);
    }

    DWORD nPartyGold = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold;

    BOOL bShowGold = TRUE;

    if (m_pStore->m_header.m_nStoreType == 4) {
        UpdateLabel(m_pMainPanel,
            0x0FFFFFFF,
            "%s",
            (LPCTSTR)FetchString(26291));

        UpdateLabel(m_pMainPanel,
            0x1000002D,
            "%s",
            (LPCTSTR)FetchString(m_pStore->m_header.m_strName));

        if (m_pStore->m_header.m_nBuyMarkUp == 0 && m_pStore->m_header.m_nSellMarkDown == 0) {
            UpdateLabel(m_pMainPanel, 0x1000002A, "");
            bShowGold = FALSE;
        }
    } else {
        UpdateLabel(m_pMainPanel,
            0x0FFFFFFF,
            "%s",
            (LPCTSTR)FetchString(13978));

        UpdateLabel(m_pMainPanel,
            0x1000002D,
            "%s",
            (LPCTSTR)FetchString(13701));
    }

    if (bShowGold) {
        UpdateLabel(m_pMainPanel,
            0x1000002A,
            "%d",
            nPartyGold);
    }

    UpdateLabel(m_pMainPanel,
        0x1000002E,
        "%s",
        (LPCTSTR)sTitle);

    UpdateLabel(m_pMainPanel,
        0x10000003,
        "%s",
        (LPCTSTR)FetchString(m_pStore->m_header.m_strName));

    BOOL bBuyAllowed = bCharacterViewable && (m_pStore->m_header.m_nStoreFlags & 0x1) != 0;
    BOOL bSellAllowed = bCharacterViewable && (m_pStore->m_header.m_nStoreFlags & 0x2) != 0;

    for (DWORD nLabel = 0x10000012; nLabel < 0x10000018; nLabel++) {
        CScreenStoreItem cItem;
        GetStoreItem(m_nTopStoreItem + nLabel - 0x10000012, cItem);

        CUIControlButtonStoreStoreItem* pStoreItem = static_cast<CUIControlButtonStoreStoreItem*>(m_pMainPanel->GetControl(nLabel - 0x1000000D));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2436
        UTIL_ASSERT(pStoreItem != NULL);

        if (cItem.m_pItem == NULL) {
            pStoreItem->m_item.SetResRef(CResRef(""), TRUE);
            pStoreItem->InvalidateRect();
        } else if (!(pStoreItem->m_item == *(cItem.m_pItem))) {
            pStoreItem->m_item.SetResRef(cItem.m_pItem->GetResRef(), TRUE);
            pStoreItem->m_item.m_useCount1 = cItem.m_pItem->m_useCount1;
            pStoreItem->m_item.m_useCount2 = cItem.m_pItem->m_useCount2;
            pStoreItem->m_item.m_useCount3 = cItem.m_pItem->m_useCount3;
            pStoreItem->m_item.m_wear = cItem.m_pItem->m_wear;
            pStoreItem->m_item.m_flags = cItem.m_pItem->m_flags;
            pStoreItem->InvalidateRect();
        }

        pStoreItem->SetSelected(cItem.m_bSelected);
        pStoreItem->SetEnabled(bBuyAllowed && cItem.m_bEnabled);

        STRREF strError;
        pStoreItem->SetValid(pGame->CanCharacterUseItem(static_cast<SHORT>(m_nSelectedCharacter),
            cItem.m_pItem,
            strError,
            TRUE));

        if (cItem.m_pItem == NULL) {
            UpdateLabel(m_pMainPanel, nLabel, "");
            continue;
        }

        CString sText;

        sText.Format(_T("%s"), (LPCTSTR)FetchString(cItem.m_pItem->GetGenericName()));
        g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMNAME, sText);

        if (m_pStore->m_header.m_nStoreType == 4) {
            sText = FetchString(24890);
        } else {
            sText.Format(_T("%d"), cItem.m_nValue);
            g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMCOST, sText);

            sText = FetchString(10162);
        }

        if (cItem.m_nStoreCount != 0xFFFF) {
            sText.Format(_T("%s (%d)"), CString(sText), cItem.m_nStoreCount);
        }

        UpdateLabel(m_pMainPanel,
            nLabel,
            "%s",
            (LPCTSTR)sText);

        if (pStoreItem->m_bEnabled) {
            HighlightLabel(m_pMainPanel, nLabel, FALSE, COLOR_LABEL_HIGHLIGHT_BONUS);
        } else {
            HighlightLabel(m_pMainPanel, nLabel, TRUE, COLOR_LABEL_DISABLE);
        }
    }

    CUIControlScrollBarStoreStore* pStoreBar = static_cast<CUIControlScrollBarStoreStore*>(m_pMainPanel->GetControl(11));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2503
    UTIL_ASSERT(pStoreBar != NULL);

    // NOTE: Uninline.
    pStoreBar->UpdateScrollBar();

    CUIControlButton* pBuy = static_cast<CUIControlButton*>(m_pMainPanel->GetControl(2));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2508
    UTIL_ASSERT(pBuy != NULL);

    pBuy->SetEnabled(IsBuyItemButtonClickable() && bBuyAllowed);

    if (m_pStore->m_header.m_nStoreType == 4) {
        g_pBaldurChitin->GetTlkTable().Fetch(26287, strRes);
        pBuy->SetText(strRes.szText);

        if (m_pStore->m_header.m_nBuyMarkUp == 0 && m_pStore->m_header.m_nSellMarkDown == 0) {
            UpdateLabel(m_pMainPanel, 0x1000002F, "");
            UpdateLabel(m_pMainPanel, 0x1000002B, "");
        } else {
            UpdateLabel(m_pMainPanel,
                0x1000002F,
                "%s",
                (LPCTSTR)FetchString(13705));

            UpdateLabel(m_pMainPanel,
                0x1000002B,
                "%d",
                m_nStoreCost);
        }
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(13703, strRes);
        pBuy->SetText(strRes.szText);

        UpdateLabel(m_pMainPanel,
            0x1000002F,
            "%s",
            (LPCTSTR)FetchString(13705));

        UpdateLabel(m_pMainPanel,
            0x1000002B,
            "%d",
            m_nStoreCost);
    }

    for (DWORD nLabel = 0x1000001E; nLabel < 0x10000024; nLabel++) {
        CScreenStoreItem cItem;
        GetGroupItem(m_nTopGroupItem + nLabel - 0x1000001E, cItem);

        CUIControlButtonStoreGroupItem* pGroupItem = static_cast<CUIControlButtonStoreGroupItem*>(m_pMainPanel->GetControl(nLabel - 0x10000011));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2545
        UTIL_ASSERT(pGroupItem != NULL);

        if (!bCharacterViewable) {
            pGroupItem->m_item.SetResRef(CResRef(""), TRUE);
            pGroupItem->InvalidateRect();
            pGroupItem->SetSelected(FALSE);
            pGroupItem->SetEnabled(FALSE);
            UpdateLabel(m_pMainPanel, nLabel, "");
            continue;
        }

        if (cItem.m_pItem == NULL) {
            pGroupItem->m_item.SetResRef(CResRef(""), TRUE);
            pGroupItem->InvalidateRect();
        } else if (pGroupItem->m_item.m_useCount1 != cItem.m_pItem->m_useCount1
            || pGroupItem->m_item.m_useCount2 != cItem.m_pItem->m_useCount2
            || pGroupItem->m_item.m_useCount3 != cItem.m_pItem->m_useCount3
            || pGroupItem->m_item.m_wear != cItem.m_pItem->m_wear
            || pGroupItem->m_item.m_flags != cItem.m_pItem->m_flags
            || pGroupItem->m_item.GetResRef() != cItem.m_pItem->GetResRef()) {
            pGroupItem->m_item.SetResRef(cItem.m_pItem->GetResRef(), TRUE);
            pGroupItem->m_item.m_useCount1 = cItem.m_pItem->m_useCount1;
            pGroupItem->m_item.m_useCount2 = cItem.m_pItem->m_useCount2;
            pGroupItem->m_item.m_useCount3 = cItem.m_pItem->m_useCount3;
            pGroupItem->m_item.m_wear = cItem.m_pItem->m_wear;
            pGroupItem->m_item.m_flags = cItem.m_pItem->m_flags;
            pGroupItem->InvalidateRect();
        }

        pGroupItem->SetSelected(cItem.m_bSelected);

        if (cItem.m_pItem != NULL && cItem.m_pItem->GetItemType() == 58) {
            pGroupItem->SetEnabled(cItem.m_bEnabled != FALSE);
        } else {
            pGroupItem->SetEnabled(bSellAllowed && cItem.m_bEnabled);
        }

        STRREF strError;
        pGroupItem->SetValid(pGame->CanCharacterUseItem(static_cast<SHORT>(m_nSelectedCharacter),
            cItem.m_pItem,
            strError,
            TRUE));

        if (cItem.m_pItem == NULL) {
            UpdateLabel(m_pMainPanel, nLabel, "");
            continue;
        }

        CString sText;

        sText.Format(_T("%s"), (LPCTSTR)FetchString(cItem.m_pItem->GetGenericName()));
        g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMNAME, sText);

        if (m_pStore->m_header.m_nStoreType == 4) {
            sText = FetchString(24890);
        } else {
            sText.Format(_T("%d"), cItem.m_nValue);
            g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMCOST, sText);

            sText = FetchString(10162);
        }

        if (m_pBag != NULL && cItem.m_nStoreCount != 0xFFFF) {
            sText.Format(_T("%s (%d)"), CString(sText), cItem.m_nStoreCount);
        }

        UpdateLabel(m_pMainPanel,
            nLabel,
            "%s",
            (LPCTSTR)sText);

        if (pGroupItem->m_bEnabled) {
            HighlightLabel(m_pMainPanel, nLabel, FALSE, COLOR_LABEL_HIGHLIGHT_BONUS);
        } else {
            HighlightLabel(m_pMainPanel, nLabel, TRUE, COLOR_LABEL_DISABLE);
        }
    }

    CUIControlScrollBarStoreGroup* pGroupBar = static_cast<CUIControlScrollBarStoreGroup*>(m_pMainPanel->GetControl(12));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2613
    UTIL_ASSERT(pGroupBar != NULL);

    // NOTE: Uninline.
    pGroupBar->UpdateScrollBar();

    CUIControlButton* pSell = static_cast<CUIControlButton*>(m_pMainPanel->GetControl(3));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2618
    UTIL_ASSERT(pSell != NULL);

    pSell->SetEnabled(IsSellItemButtonClickable() && bSellAllowed);

    if (m_pStore->m_header.m_nStoreType == 4) {
        g_pBaldurChitin->GetTlkTable().Fetch(26288, strRes);
        pSell->SetText(strRes.szText);

        if (m_pStore->m_header.m_nBuyMarkUp == 0 && m_pStore->m_header.m_nSellMarkDown == 0) {
            UpdateLabel(m_pMainPanel, 0x10000030, "");
            UpdateLabel(m_pMainPanel, 0x1000002C, "");
        } else {
            UpdateLabel(m_pMainPanel,
                0x10000030,
                "%s",
                (LPCTSTR)FetchString(13706));

            UpdateLabel(m_pMainPanel,
                0x1000002C,
                "%d",
                m_nGroupCost);
        }
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(13704, strRes);
        pSell->SetText(strRes.szText);

        UpdateLabel(m_pMainPanel,
            0x10000030,
            "%s",
            (LPCTSTR)FetchString(13706));

        UpdateLabel(m_pMainPanel,
            0x1000002C,
            "%d",
            m_nGroupCost);
    }

    CUIControlBase* pBagIcon = m_pMainPanel->GetControl(50);
    pBagIcon->SetActive(m_pBag != NULL);

    pBagIcon = m_pMainPanel->GetControl(50);
    pBagIcon->SetInactiveRender(m_pBag != NULL);

    pBagIcon = m_pMainPanel->GetControl(50);
    pBagIcon->InvalidateRect();
}

// 0x675380
void CScreenStore::UpdateIdentifyPanel()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2688
    UTIL_ASSERT(pGame != NULL);

    CGameSave* pSave = pGame->GetGameSave();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2690
    UTIL_ASSERT(pSave != NULL);

    BOOL bCharacterViewable = IsCharacterViewable(static_cast<SHORT>(m_nSelectedCharacter));

    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(m_pMainPanel->GetControl(7));

    LONG nCharacterId = CGameObjectArray::INVALID_INDEX;
    if (m_nSelectedCharacter < pGame->m_nCharacters) {
        nCharacterId = pGame->m_characterPortraits[m_nSelectedCharacter];
    }

    CString sCharacterName;

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

        if (pSprite->m_baseStats.m_name == -1) {
            sCharacterName = pSprite->GetName();
        } else {
            sCharacterName = FetchString(pSprite->m_baseStats.m_name);
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        sCharacterName = "";
    }

    DWORD nPartyGold = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold;

    UpdateLabel(m_pMainPanel,
        0x10000005,
        "%s",
        (LPCTSTR)sCharacterName);

    UpdateLabel(m_pMainPanel,
        0x10000000,
        "%s",
        (LPCTSTR)FetchString(m_pStore->m_header.m_strName));

    UpdateLabel(m_pMainPanel,
        0x10000001,
        "%d",
        nPartyGold);

    for (DWORD nLabel = 0x1000000D; nLabel < 0x10000013; nLabel++) {
        CScreenStoreItem cItem;
        GetIdentifyItem(m_nTopIdentifyItem + nLabel - 0x1000000D, cItem);

        CUIControlButtonStoreGroupItem* pGroupItem = static_cast<CUIControlButtonStoreGroupItem*>(m_pMainPanel->GetControl(nLabel - 0x10000005));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2737
        UTIL_ASSERT(pGroupItem != NULL);

        if (bCharacterViewable) {
            if (cItem.m_pItem == NULL) {
                pGroupItem->m_item.SetResRef(CResRef(""), TRUE);
                pGroupItem->InvalidateRect();
            } else if (!(pGroupItem->m_item == *(cItem.m_pItem))) {
                pGroupItem->m_item.SetResRef(cItem.m_pItem->GetResRef(), TRUE);
                pGroupItem->m_item.m_useCount1 = cItem.m_pItem->m_useCount1;
                pGroupItem->m_item.m_useCount2 = cItem.m_pItem->m_useCount2;
                pGroupItem->m_item.m_useCount3 = cItem.m_pItem->m_useCount3;
                pGroupItem->m_item.m_wear = cItem.m_pItem->m_wear;
                pGroupItem->m_item.m_flags = cItem.m_pItem->m_flags;
                pGroupItem->InvalidateRect();
            }

            pGroupItem->SetSelected(cItem.m_bSelected);
            pGroupItem->SetEnabled(cItem.m_bEnabled);

            if (cItem.m_pItem != NULL) {
                CString sToken;

                sToken.Format(_T("%s"), (LPCTSTR)FetchString(cItem.m_pItem->GetGenericName()));
                g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMNAME, sToken);

                sToken.Format(_T("%d"), cItem.m_nValue);
                g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMCOST, sToken);

                UpdateLabel(m_pMainPanel,
                    nLabel,
                    "%s",
                    (LPCTSTR)FetchString(10162));
                continue;
            }
        } else {
            pGroupItem->m_item.SetResRef(CResRef(""), TRUE);
            pGroupItem->InvalidateRect();
            pGroupItem->SetSelected(FALSE);
            pGroupItem->SetEnabled(FALSE);
        }

        UpdateLabel(m_pMainPanel, nLabel, "");
    }

    CUIControlScrollBarStoreIdentify* pGroupBar = static_cast<CUIControlScrollBarStoreIdentify*>(m_pMainPanel->GetControl(7));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2773
    UTIL_ASSERT(pGroupBar != NULL);

    // NOTE: Uninline.
    pGroupBar->UpdateScrollBar();

    UpdateLabel(m_pMainPanel,
        0x10000003,
        "%d",
        m_dwIdentifyCost);

    CUIControlButton* pIdentify = static_cast<CUIControlButton*>(m_pMainPanel->GetControl(5));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2782
    UTIL_ASSERT(pIdentify != NULL);

    BOOL bIdentifyEnabled = FALSE;

    POSITION pos = m_lIdentifyItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pIdentifyItem = m_lIdentifyItems.GetNext(pos);
        if (pIdentifyItem->m_pItem != NULL) {
            bIdentifyEnabled = bCharacterViewable != FALSE;
            break;
        }
    }

    pIdentify->SetEnabled(bIdentifyEnabled);
}

// 0x675940
void CScreenStore::UpdateBuySpellPanel()
{
    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(m_pMainPanel->GetControl(7));

    DWORD nPartyGold = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold;

    UpdateLabel(m_pMainPanel,
        0x10000000,
        "%s",
        (LPCTSTR)FetchString(m_pStore->m_header.m_strName));

    UpdateLabel(m_pMainPanel,
        0x10000001,
        "%d",
        nPartyGold);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2823
    UTIL_ASSERT(pGame != NULL);

    CGameSave* pSave = pGame->GetGameSave();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2825
    UTIL_ASSERT(pSave != NULL);

    for (DWORD nLabel = 0x1000000D; nLabel < 0x10000013; nLabel++) {
        CScreenStoreItem cItem;
        GetSpellItem(m_nTopSpellItem + nLabel - 0x1000000D, cItem);

        CUIControlButtonStoreStoreSpell* pSpellItem = static_cast<CUIControlButtonStoreStoreSpell*>(m_pMainPanel->GetControl(nLabel - 0x10000005));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 2836
        UTIL_ASSERT(pSpellItem != NULL);

        if (pSpellItem->m_resRef != cItem.m_cResSpell) {
            pSpellItem->m_resRef = cItem.m_cResSpell;
            pSpellItem->InvalidateRect();
        }

        pSpellItem->SetSelected(cItem.m_bSelected);
        pSpellItem->SetEnabled(cItem.m_bEnabled);

        if (cItem.m_cResSpell == "") {
            UpdateLabel(m_pMainPanel, nLabel, "");
            continue;
        }

        CSpell cSpell(cItem.m_cResSpell);
        cSpell.Demand();

        BOOL bUsable = CheckSpellUsable(m_nSelectedCharacter, cItem.m_cResSpell);
        pSpellItem->SetValid(bUsable);

        CString sText;

        sText.Format(_T("%s"), (LPCTSTR)FetchString(cSpell.GetGenericName()));
        g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMNAME, sText);

        sText.Format(_T("%d"), cItem.m_nValue);
        g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_ITEMCOST, sText);

        UpdateLabel(m_pMainPanel,
            nLabel,
            "%s",
            (LPCTSTR)FetchString(10162));

        pSpellItem->SetEnabled(bUsable);

        cSpell.Release();
    }

    CUIControlScrollBarStoreSpell* pSpellBar = static_cast<CUIControlScrollBarStoreSpell*>(m_pMainPanel->GetControl(7));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2885
    UTIL_ASSERT(pSpellBar != NULL);

    // NOTE: Uninline.
    pSpellBar->UpdateScrollBar();

    UpdateLabel(m_pMainPanel,
        0x10000003,
        "%d",
        m_dwSpellCost);

    CUIControlButton* pBuy = static_cast<CUIControlButton*>(m_pMainPanel->GetControl(5));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 2894
    UTIL_ASSERT(pBuy != NULL);

    BOOL bBuyEnabled = FALSE;

    POSITION pos = m_lSpellItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pSpellStoreItem = m_lSpellItems.GetNext(pos);
        if (pSpellStoreItem->m_bSelected) {
            // The buy-spell panel is the one temple service a dead character
            // can still be the target of, so it accepts an out-of-range pick
            // when the character is not alive.
            if (IsCharacterInRange(static_cast<SHORT>(m_nSelectedCharacter))
                || (m_pMainPanel->m_nID == 5
                    && !IsCharacterAlive(static_cast<SHORT>(m_nSelectedCharacter)))) {
                bBuyEnabled = TRUE;
            }
            break;
        }
    }

    pBuy->SetEnabled(bBuyEnabled);
}

// 0x675E80
void CScreenStore::UpdateRentRoomPanel()
{
    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(m_pMainPanel->GetControl(13));

    DWORD nInnFlags = m_pStore->m_header.m_nInnFlags;
    DWORD nPartyGold = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold;

    UpdateLabel(m_pMainPanel,
        0x10000008,
        "%s",
        (LPCSTR)FetchString(m_pStore->m_header.m_strName));

    UpdateLabel(m_pMainPanel,
        0x10000009,
        "%d",
        nPartyGold);

    // NOTE: Uninline.
    UpdateLabel(m_pMainPanel, 0x1000000D, "%d", GetRoomCost());

    for (INT nIndex = 0; nIndex < NUM_ROOMTYPES; nIndex++) {
        CUIControlButtonStoreRentRoomRoomPicture* pPicture = static_cast<CUIControlButtonStoreRentRoomRoomPicture*>(m_pMainPanel->GetControl(nIndex));

        BOOL bEnabled;
        switch (nIndex) {
        case 0:
            bEnabled = (nInnFlags & 0x1) != 0;
            break;
        case 1:
            bEnabled = (nInnFlags & 0x2) != 0;
            break;
        case 2:
            bEnabled = (nInnFlags & 0x4) != 0;
            break;
        case 3:
            bEnabled = (nInnFlags & 0x8) != 0;
            break;
        }

        // NOTE: Uninline.
        pPicture->SetSelected(pPicture->GetRoomType() == m_dwRoomType);

        pPicture->SetEnabled(bEnabled);

        CUIControlButtonStoreRentRoomRoomSelect* pSelect = static_cast<CUIControlButtonStoreRentRoomRoomSelect*>(m_pMainPanel->GetControl(nIndex + 4));

        // NOTE: Uninline.
        pSelect->SetSelected(pSelect->GetRoomType() == m_dwRoomType);

        pSelect->SetEnabled(bEnabled);
    }

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(m_pMainPanel->GetControl(12));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3184
    UTIL_ASSERT(pText != NULL);

    switch (m_dwRoomType) {
    case ROOMTYPE_NONE:
        break;
    case ROOMTYPE_PEASANT:
        pText->RemoveAll();
        UpdateText(pText, "%s", (LPCSTR)FetchString(17389));
        break;
    case ROOMTYPE_MERCHANT:
        pText->RemoveAll();
        UpdateText(pText, "%s", (LPCSTR)FetchString(17517));
        break;
    case ROOMTYPE_NOBLE:
        pText->RemoveAll();
        UpdateText(pText, "%s", (LPCSTR)FetchString(17521));
        break;
    case ROOMTYPE_ROYAL:
        pText->RemoveAll();
        UpdateText(pText, "%s", (LPCSTR)FetchString(17519));
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 3211
        UTIL_ASSERT(FALSE);
    }

    CUIControlButton* pRent = static_cast<CUIControlButton*>(m_pMainPanel->GetControl(11));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3218
    UTIL_ASSERT(pRent != NULL);

    pRent->SetEnabled(m_dwRoomType != ROOMTYPE_NONE);
}

// 0x676300
void CScreenStore::UpdateBuyDrinksPanel()
{
    // FIXME: Unused.
    CString v1;

    m_pCurrentScrollBar = static_cast<CUIControlScrollBar*>(m_pMainPanel->GetControl(5));

    DWORD nPartyGold = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold;

    UpdateLabel(m_pMainPanel,
        0x10000011,
        "%s",
        (LPCSTR)FetchString(m_pStore->m_header.m_strName));

    UpdateLabel(m_pMainPanel,
        0x10000012,
        "%d",
        nPartyGold);

    BOOL bCharacterViewable = IsCharacterViewable(static_cast<SHORT>(m_nSelectedCharacter));

    for (INT nIndex = 0; nIndex < DRINK_SLOTS; nIndex++) {
        CUIControlButton* pButton = static_cast<CUIControlButton*>(m_pMainPanel->GetControl(nIndex + 40));
        STRREF strName;
        DWORD dwCost;
        DWORD dwRumorChance;
        if (IsCharacterInRange(m_nSelectedCharacter)
            && m_pStore->GetDrink(m_nTopDrinkItem + nIndex, strName, dwCost, dwRumorChance)) {
            pButton->SetText(FetchString(strName));
            pButton->SetEnabled(bCharacterViewable);
            UpdateLabel(m_pMainPanel, nIndex + 0x1000001D, "%d", dwCost);
        } else {
            pButton->SetText(CString(""));
            pButton->SetEnabled(FALSE);
            UpdateLabel(m_pMainPanel, nIndex + 0x1000001D, "");
        }
    }

    CUIControlScrollBarStoreBuyDrinksDrink* pDrinkBar = static_cast<CUIControlScrollBarStoreBuyDrinksDrink*>(m_pMainPanel->GetControl(5));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3289
    UTIL_ASSERT(pDrinkBar != NULL);

    // NOTE: Uninline.
    pDrinkBar->UpdateScrollBar();
}

// NOTE: Inlined.
INT CScreenStore::GetNumGroupItems()
{
    return m_lGroupItems.GetCount();
}

// NOTE: Inlined
void CScreenStore::SetTopGroupItem(INT nTopGroupItem)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3360
    UTIL_ASSERT(0 <= nTopGroupItem && nTopGroupItem <= GetNumGroupItems());

    m_nTopGroupItem = nTopGroupItem;
}

// NOTE: Inlined.
INT CScreenStore::GetNumIdentifyItems()
{
    return m_lIdentifyItems.GetCount();
}

// NOTE: Inlined
void CScreenStore::SetTopIdentifyItem(INT nTopIdentifyItem)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3387
    UTIL_ASSERT(0 <= nTopIdentifyItem && nTopIdentifyItem <= GetNumIdentifyItems());

    m_nTopIdentifyItem = nTopIdentifyItem;
}

// NOTE: Inlined.
INT CScreenStore::GetNumStoreItems()
{
    return m_lStoreItems.GetCount();
}

// NOTE: Inlined
void CScreenStore::SetTopStoreItem(INT nTopStoreItem)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3414
    UTIL_ASSERT(0 <= nTopStoreItem && nTopStoreItem <= GetNumStoreItems());

    m_nTopStoreItem = nTopStoreItem;
}

// NOTE: Inlined.
INT CScreenStore::GetNumSpellItems()
{
    return m_lSpellItems.GetCount();
}

// 0x676590
void CScreenStore::SetTopSpellItem(INT nTopSpellItem)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3441
    UTIL_ASSERT(0 <= nTopSpellItem && nTopSpellItem <= GetNumSpellItems());

    m_nTopSpellItem = nTopSpellItem;
}

// NOTE: Inlined.
INT CScreenStore::GetNumDrinkItems()
{
    return m_pStore->m_nDrinks;
}

// 0x6765D0
void CScreenStore::SetTopDrinkItem(INT nTopDrinkItem)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3468
    UTIL_ASSERT(0 <= nTopDrinkItem && nTopDrinkItem <= GetNumDrinkItems());

    m_nTopDrinkItem = nTopDrinkItem;
}

// 0x676610
void CScreenStore::GetGroupItem(INT nIndex, CScreenStoreItem& cItem)
{
    CScreenStoreItem cDefaultItem;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3499
    UTIL_ASSERT(pGame != NULL);

    // TODO: Obtain `pSave` at 0x422C, not sure about type. Not used.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3451
    // UTIL_ASSERT(pSave != NULL);

    if (nIndex >= 0 && nIndex < m_lGroupItems.GetCount()) {
        CScreenStoreItem* pItem = m_lGroupItems.GetAt(m_lGroupItems.FindIndex(nIndex));
        cItem.m_cResSpell = pItem->m_cResSpell;
        cItem.m_pItem = pItem->m_pItem;
        cItem.m_bSelected = pItem->m_bSelected;
        cItem.m_bEnabled = pItem->m_bEnabled;
        cItem.m_nSlot = pItem->m_nSlot;
        cItem.m_nValue = pItem->m_nValue;
        cItem.m_nSingleValue = pItem->m_nSingleValue;
        cItem.m_nCount = pItem->m_nCount;
        cItem.m_nMaxCount = pItem->m_nMaxCount;
        cItem.m_nStoreCount = pItem->m_nStoreCount;
    } else {
        cItem.m_cResSpell = cDefaultItem.m_cResSpell;
        cItem.m_pItem = cDefaultItem.m_pItem;
        cItem.m_bSelected = cDefaultItem.m_bSelected;
        cItem.m_bEnabled = cDefaultItem.m_bEnabled;
        cItem.m_nSlot = cDefaultItem.m_nSlot;
        cItem.m_nValue = cDefaultItem.m_nValue;
        cItem.m_nSingleValue = cDefaultItem.m_nSingleValue;
        cItem.m_nCount = cDefaultItem.m_nCount;
        cItem.m_nMaxCount = cDefaultItem.m_nMaxCount;
        cItem.m_nStoreCount = cDefaultItem.m_nStoreCount;
    }
}

// 0x676780
void CScreenStore::UpdateGroupItems()
{
    CTypedPtrList<CPtrList, CScreenStoreItem*> lSelectedItems;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3644
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3646
    // UTIL_ASSERT(pSave != NULL);

    CGameSprite* pSprite;
    LONG nCharacterId;

    if (m_pBag == NULL) {
        // NOTE: Uninline.
        nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

        BYTE rc;
        do {
            rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 3657
            UTIL_ASSERT(FALSE);
            return;
        }
    } else {
        m_pBag->CompressItems();
    }

    POSITION pos = m_lGroupItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pOldItem = m_lGroupItems.GetNext(pos);
        if (!pOldItem->m_bSelected) {
            delete pOldItem;
        } else {
            lSelectedItems.AddTail(pOldItem);
        }
    }
    m_lGroupItems.RemoveAll();

    if (m_pBag == NULL) {
        unsigned char nSavedMarkDown = field_5D9;
        field_5D9 = pSprite->HasFeat(CGAMESPRITE_FEAT_MERCANTILE_BACKGROUND) ? 5 : 0;

        for (INT nIndex = 0; nIndex < CScreenInventory::PERSONAL_INVENTORY_SIZE; nIndex++) {
            CItem* pItem = pSprite->GetEquipment()->m_items[18 + nIndex];
            if (pItem == NULL) {
                continue;
            }

            // A bag cannot be put inside another bag.
            if (m_pStore->GetType() == CSTOREFILEHEADER_STORETYPE_BAG
                && pItem->GetItemType() == 0x3A) {
                continue;
            }

            CScreenStoreItem* pGroupItem = new CScreenStoreItem;

            pGroupItem->m_bSelected = FALSE;
            pGroupItem->m_pItem = pItem;
            pGroupItem->m_nSlot = nIndex;
            pGroupItem->m_nSingleValue = m_pStore->GetItemSellValue(*pItem, field_5D9);
            pGroupItem->m_nValue = pGroupItem->m_nSingleValue;

            if (pItem->GetItemType() == 0x3A) {
                pGroupItem->m_bEnabled = TRUE;
            } else if (m_pStore->GetType() == CSTOREFILEHEADER_STORETYPE_BAG) {
                pGroupItem->m_bEnabled = m_pStore->IsValidSellType(pItem);
            } else {
                pGroupItem->m_bEnabled = m_pStore->IsValidSellType(pItem)
                    && pGroupItem->m_nValue != 0;
            }

            if (pItem->GetMaxStackable() < 2) {
                pGroupItem->m_nMaxCount = 1;
            } else {
                pGroupItem->m_nMaxCount = pItem->GetUsageCount(0);
            }

            CScreenStoreItem* pSelectedItem = NULL;
            POSITION posSelected = lSelectedItems.GetHeadPosition();
            while (posSelected != NULL) {
                CScreenStoreItem* pCandidate = lSelectedItems.GetNext(posSelected);
                if (pCandidate->m_pItem == pItem) {
                    pSelectedItem = pCandidate;
                }
            }

            if (pSelectedItem != NULL) {
                pGroupItem->m_bSelected = TRUE;
                pGroupItem->m_nCount = min(pSelectedItem->m_nCount, pGroupItem->m_nMaxCount);
            }

            m_lGroupItems.AddTail(pGroupItem);
        }

        field_5D9 = nSavedMarkDown;

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        for (INT nIndex = 0; nIndex < m_pBag->m_lInventory.GetCount(); nIndex++) {
            CItem* pItem = new CItem;

            if (!m_pBag->GetItem(nIndex, *pItem)) {
                delete pItem;
                continue;
            }

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 3767
            UTIL_ASSERT(pItem->GetItemType() != 0x3A);

            CScreenStoreItem* pGroupItem = new CScreenStoreItem;

            pGroupItem->m_nSlot = nIndex;
            pGroupItem->m_bSelected = FALSE;
            pGroupItem->m_pItem = pItem;
            pGroupItem->m_nValue = m_pStore->GetItemSellValue(*pItem, field_5D9);
            pGroupItem->m_nStoreCount = m_pBag->GetItemNumInStock(nIndex);
            pGroupItem->m_bEnabled = m_pStore->IsValidSellType(pItem)
                && pGroupItem->m_nValue != 0;
            pGroupItem->m_nSingleValue = pGroupItem->m_nValue;

            if (pItem->GetMaxStackable() < 2) {
                pGroupItem->m_nMaxCount = 1;
            } else {
                pGroupItem->m_nMaxCount = pItem->GetUsageCount(0);
            }

            CScreenStoreItem* pSelectedItem = NULL;
            POSITION posSelected = lSelectedItems.GetHeadPosition();
            while (posSelected != NULL) {
                CScreenStoreItem* pCandidate = lSelectedItems.GetNext(posSelected);
                if (pCandidate->m_pItem == pItem) {
                    pSelectedItem = pCandidate;
                }
            }

            if (pSelectedItem != NULL) {
                pGroupItem->m_bSelected = TRUE;
                pGroupItem->m_nCount = min(pSelectedItem->m_nCount, pGroupItem->m_nMaxCount);
            }

            m_lGroupItems.AddTail(pGroupItem);
        }
    }

    POSITION posSelected = lSelectedItems.GetHeadPosition();
    while (posSelected != NULL) {
        delete lSelectedItems.GetNext(posSelected);
    }
    lSelectedItems.RemoveAll();
}

// 0x676D10
void CScreenStore::DestroyGroupItems(BOOL bShutDown)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    POSITION pos = m_lGroupItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lGroupItems.GetAt(pos);

        if (m_pBag != NULL) {
            if (bShutDown || pGame == NULL) {
                if (pItem->m_pItem != NULL) {
                    delete pItem->m_pItem;
                }
            } else {
                pGame->AddDisposableItem(pItem->m_pItem);
            }
        }

        delete pItem;

        m_lGroupItems.GetNext(pos);
    }

    m_lGroupItems.RemoveAll();
}

// 0x676D80
void CScreenStore::GetStoreItem(INT nIndex, CScreenStoreItem& cItem)
{
    CScreenStoreItem cDefaultItem;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3896
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 3898
    // UTIL_ASSERT(pSave != NULL);

    if (nIndex >= 0 && nIndex < m_lStoreItems.GetCount()) {
        CScreenStoreItem* pItem = m_lStoreItems.GetAt(m_lStoreItems.FindIndex(nIndex));
        cItem.m_cResSpell = pItem->m_cResSpell;
        cItem.m_pItem = pItem->m_pItem;
        cItem.m_bSelected = pItem->m_bSelected;
        cItem.m_bEnabled = pItem->m_bEnabled;
        cItem.m_nSlot = pItem->m_nSlot;
        cItem.m_nValue = pItem->m_nValue;
        cItem.m_nSingleValue = pItem->m_nSingleValue;
        cItem.m_nCount = pItem->m_nCount;
        cItem.m_nMaxCount = pItem->m_nMaxCount;
        cItem.m_nStoreCount = pItem->m_nStoreCount;
    } else {
        cItem.m_cResSpell = cDefaultItem.m_cResSpell;
        cItem.m_pItem = cDefaultItem.m_pItem;
        cItem.m_bSelected = cDefaultItem.m_bSelected;
        cItem.m_bEnabled = cDefaultItem.m_bEnabled;
        cItem.m_nSlot = cDefaultItem.m_nSlot;
        cItem.m_nValue = cDefaultItem.m_nValue;
        cItem.m_nSingleValue = cDefaultItem.m_nSingleValue;
        cItem.m_nCount = cDefaultItem.m_nCount;
        cItem.m_nMaxCount = cDefaultItem.m_nMaxCount;
        cItem.m_nStoreCount = cDefaultItem.m_nStoreCount;
    }
}

// 0x676EF0
void CScreenStore::UpdateStoreItems()
{
    CTypedPtrList<CPtrList, CScreenStoreItem*> lSelectedItems;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4039
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4041
    // UTIL_ASSERT(pSave != NULL);

    CItem cItem;

    m_pStore->CompressItems();

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pOldItem = m_lStoreItems.GetNext(pos);
        if (!pOldItem->m_bSelected) {
            pGame->AddDisposableItem(pOldItem->m_pItem);
            delete pOldItem;
        } else {
            lSelectedItems.AddTail(pOldItem);
        }
    }
    m_lStoreItems.RemoveAll();

    for (INT nIndex = 0; nIndex < m_pStore->m_lInventory.GetCount(); nIndex++) {
        CScreenStoreItem* pStoreItem = new CScreenStoreItem;

        m_pStore->GetItem(nIndex, cItem);
        if (!m_pStore->GetItemExt(nIndex, cItem)) {
            continue;
        }

        CItem* pItem = new CItem(cItem);

        // NOTE: Unused.
        pItem->GetAbilityCount();

        if (pItem->GetLoreValue() == 0) {
            pItem->m_flags |= 0x1;
        }

        pStoreItem->m_bSelected = FALSE;
        pStoreItem->m_pItem = pItem;
        pStoreItem->m_nSlot = nIndex;
        pStoreItem->m_nValue = m_pStore->GetItemBuyValue(*pItem,
            field_5D8,
            pGame->m_nReputation / CInfGame::REPUTATION_MULTIPLIER,
            field_5D9);

        pStoreItem->m_nStoreCount = m_pStore->GetItemNumInStock(nIndex);

        if (pStoreItem->m_nStoreCount == 65535) {
            pStoreItem->m_nMaxCount = 65535;
        } else if (m_pStore->GetType() == CSTOREFILEHEADER_STORETYPE_BAG) {
            if (pItem->GetMaxStackable() < 2) {
                pStoreItem->m_nMaxCount = 1;
            } else {
                pStoreItem->m_nMaxCount = pItem->GetUsageCount(0);
            }
        } else {
            DWORD nMaxCount;

            if (pItem->GetMaxStackable() < 2) {
                if (m_pStore->GetItemNumInStock(nIndex) < CScreenInventory::PERSONAL_INVENTORY_SIZE) {
                    nMaxCount = m_pStore->GetItemNumInStock(nIndex);
                } else {
                    nMaxCount = CScreenInventory::PERSONAL_INVENTORY_SIZE;
                }
            } else if (pItem->GetUsageCount(0) == pItem->GetMaxStackable()) {
                if (m_pStore->GetItemNumInStock(nIndex)
                    < static_cast<DWORD>(pItem->GetMaxStackable() * CScreenInventory::PERSONAL_INVENTORY_SIZE)) {
                    nMaxCount = m_pStore->GetItemNumInStock(nIndex);
                } else {
                    nMaxCount = pItem->GetMaxStackable() * CScreenInventory::PERSONAL_INVENTORY_SIZE;
                }
            } else if (pItem->GetItemType() == 9
                || pItem->GetItemType() == 71
                || pItem->GetItemType() == 11
                || pItem->GetItemType() == 13
                || pItem->GetItemType() == 34
                || pItem->GetItemType() == 0
                || pStoreItem->m_nStoreCount > 1) {
                nMaxCount = pStoreItem->m_nStoreCount;
                if (static_cast<DWORD>(pItem->GetMaxStackable() * CScreenInventory::PERSONAL_INVENTORY_SIZE) <= nMaxCount) {
                    nMaxCount = pItem->GetMaxStackable() * CScreenInventory::PERSONAL_INVENTORY_SIZE;
                }
            } else {
                if (pItem->GetUsageCount(0)
                    < pItem->GetMaxStackable() * CScreenInventory::PERSONAL_INVENTORY_SIZE) {
                    nMaxCount = pItem->GetUsageCount(0);
                } else {
                    nMaxCount = pItem->GetMaxStackable() * CScreenInventory::PERSONAL_INVENTORY_SIZE;
                }
            }

            pStoreItem->m_nMaxCount = nMaxCount;
        }

        pStoreItem->m_nSingleValue = pStoreItem->m_nValue;

        if (m_pBag == NULL) {
            pStoreItem->m_bEnabled = TRUE;
        } else {
            pStoreItem->m_bEnabled = m_pBag->IsValidSellType(pItem);
        }

        CScreenStoreItem* pSelectedItem = NULL;
        POSITION posSelected = lSelectedItems.GetHeadPosition();
        while (posSelected != NULL) {
            CScreenStoreItem* pCandidate = lSelectedItems.GetNext(posSelected);
            if (pCandidate->m_pItem->GetResRef() == pStoreItem->m_pItem->GetResRef()) {
                pSelectedItem = pCandidate;
            }
        }

        if (pSelectedItem != NULL) {
            pStoreItem->m_bSelected = TRUE;
            pStoreItem->m_nCount = min(pSelectedItem->m_nCount, pStoreItem->m_nMaxCount);
            pStoreItem->m_nValue = pSelectedItem->m_nSingleValue * pSelectedItem->m_nCount;
        }

        m_lStoreItems.AddTail(pStoreItem);
    }

    POSITION posSelected = lSelectedItems.GetHeadPosition();
    while (posSelected != NULL) {
        CScreenStoreItem* pSelectedItem = lSelectedItems.GetNext(posSelected);
        pGame->AddDisposableItem(pSelectedItem->m_pItem);
        delete pSelectedItem;
    }
    lSelectedItems.RemoveAll();
}

// 0x677430
void CScreenStore::DestroyStoreItems(BOOL bShutDown)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lStoreItems.GetAt(pos);

        if (bShutDown || pGame == NULL) {
            if (pItem->m_pItem != NULL) {
                delete pItem->m_pItem;
            }
        } else {
            pGame->AddDisposableItem(pItem->m_pItem);
        }

        delete pItem;

        m_lStoreItems.GetNext(pos);
    }

    m_lStoreItems.RemoveAll();
}

// 0x6774A0
void CScreenStore::GetSpellItem(INT nIndex, CScreenStoreItem& cItem)
{
    CScreenStoreItem cDefaultItem;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4258
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4260
    // UTIL_ASSERT(pSave != NULL);

    if (nIndex >= 0 && nIndex < m_lSpellItems.GetCount()) {
        CScreenStoreItem* pItem = m_lSpellItems.GetAt(m_lSpellItems.FindIndex(nIndex));
        cItem.m_cResSpell = pItem->m_cResSpell;
        cItem.m_pItem = pItem->m_pItem;
        cItem.m_bSelected = pItem->m_bSelected;
        cItem.m_bEnabled = pItem->m_bEnabled;
        cItem.m_nSlot = pItem->m_nSlot;
        cItem.m_nValue = pItem->m_nValue;
        cItem.m_nSingleValue = pItem->m_nSingleValue;
        cItem.m_nCount = pItem->m_nCount;
        cItem.m_nMaxCount = pItem->m_nMaxCount;
        cItem.m_nStoreCount = pItem->m_nStoreCount;
    } else {
        cItem.m_cResSpell = cDefaultItem.m_cResSpell;
        cItem.m_pItem = cDefaultItem.m_pItem;
        cItem.m_bSelected = cDefaultItem.m_bSelected;
        cItem.m_bEnabled = cDefaultItem.m_bEnabled;
        cItem.m_nSlot = cDefaultItem.m_nSlot;
        cItem.m_nValue = cDefaultItem.m_nValue;
        cItem.m_nSingleValue = cDefaultItem.m_nSingleValue;
        cItem.m_nCount = cDefaultItem.m_nCount;
        cItem.m_nMaxCount = cDefaultItem.m_nMaxCount;
        cItem.m_nStoreCount = cDefaultItem.m_nStoreCount;
    }
}

// 0x677610
void CScreenStore::UpdateSpellItems()
{
    CTypedPtrList<CPtrList, CScreenStoreItem*> lSelectedItems;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4367
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4369
    // UTIL_ASSERT(pSave != NULL);

    POSITION pos = m_lSpellItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pOldItem = m_lSpellItems.GetNext(pos);
        if (!pOldItem->m_bSelected) {
            delete pOldItem;
        } else {
            lSelectedItems.AddTail(pOldItem);
        }
    }
    m_lSpellItems.RemoveAll();

    for (INT nIndex = 0; nIndex < static_cast<INT>(m_pStore->m_nSpells); nIndex++) {
        CScreenStoreItem* pSpellItem = new CScreenStoreItem;

        DWORD dwCost;
        m_pStore->GetSpell(nIndex, pSpellItem->m_cResSpell, dwCost);

        pSpellItem->m_nValue = dwCost;
        pSpellItem->m_bSelected = FALSE;
        pSpellItem->m_pItem = NULL;
        pSpellItem->m_nSlot = nIndex;
        pSpellItem->m_bEnabled = TRUE;

        if (pSpellItem->m_cResSpell == "SPPR504") {
            // NOTE: Uninline.
            LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                pSpellItem->m_nValue = pGame->GetRuleTables().GetRaiseDeadCost(pSprite->m_liveTypeAI,
                    pSprite->m_derivedStats);

                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        } else if (pSpellItem->m_cResSpell == "SPPR712") {
            // NOTE: Uninline.
            LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                pSpellItem->m_nValue = pGame->GetRuleTables().GetRaiseDeadCost(pSprite->m_liveTypeAI,
                    pSprite->m_derivedStats)
                    * 2;

                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        CScreenStoreItem* pSelectedItem = NULL;
        POSITION posSelected = lSelectedItems.GetHeadPosition();
        while (posSelected != NULL) {
            CScreenStoreItem* pCandidate = lSelectedItems.GetNext(posSelected);
            if (pCandidate->m_cResSpell == pSpellItem->m_cResSpell) {
                pSelectedItem = pCandidate;
            }
        }

        if (pSelectedItem != NULL) {
            pSpellItem->m_bSelected = TRUE;
        }

        m_lSpellItems.AddTail(pSpellItem);
    }

    POSITION posSelected = lSelectedItems.GetHeadPosition();
    while (posSelected != NULL) {
        delete lSelectedItems.GetNext(posSelected);
    }
    lSelectedItems.RemoveAll();
}

// 0x677950
void CScreenStore::DestroySpellItems()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    POSITION pos = m_lSpellItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lSpellItems.GetAt(pos);
        delete pItem;

        m_lSpellItems.GetNext(pos);
    }

    m_lSpellItems.RemoveAll();
}

// 0x677980
void CScreenStore::GetIdentifyItem(INT nIndex, CScreenStoreItem& cItem)
{
    CScreenStoreItem cDefaultItem;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4534
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4536
    // UTIL_ASSERT(pSave != NULL);

    if (nIndex >= 0 && nIndex < m_lIdentifyItems.GetCount()) {
        CScreenStoreItem* pItem = m_lIdentifyItems.GetAt(m_lIdentifyItems.FindIndex(nIndex));
        cItem.m_cResSpell = pItem->m_cResSpell;
        cItem.m_pItem = pItem->m_pItem;
        cItem.m_bSelected = pItem->m_bSelected;
        cItem.m_bEnabled = pItem->m_bEnabled;
        cItem.m_nSlot = pItem->m_nSlot;
        cItem.m_nValue = pItem->m_nValue;
        cItem.m_nSingleValue = pItem->m_nSingleValue;
        cItem.m_nCount = pItem->m_nCount;
        cItem.m_nMaxCount = pItem->m_nMaxCount;
        cItem.m_nStoreCount = pItem->m_nStoreCount;
    } else {
        cItem.m_cResSpell = cDefaultItem.m_cResSpell;
        cItem.m_pItem = cDefaultItem.m_pItem;
        cItem.m_bSelected = cDefaultItem.m_bSelected;
        cItem.m_bEnabled = cDefaultItem.m_bEnabled;
        cItem.m_nSlot = cDefaultItem.m_nSlot;
        cItem.m_nValue = cDefaultItem.m_nValue;
        cItem.m_nSingleValue = cDefaultItem.m_nSingleValue;
        cItem.m_nCount = cDefaultItem.m_nCount;
        cItem.m_nMaxCount = cDefaultItem.m_nMaxCount;
        cItem.m_nStoreCount = cDefaultItem.m_nStoreCount;
    }
}

// 0x677AF0
void CScreenStore::UpdateIdentifyItems()
{
    CTypedPtrList<CPtrList, CScreenStoreItem*> lSelectedItems;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4644
    UTIL_ASSERT(pGame != NULL);

    // TODO: `See CScreenStore::GetGroupItem`.

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 4646
    // UTIL_ASSERT(pSave != NULL);

    // NOTE: Uninline.
    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        POSITION pos = m_lIdentifyItems.GetHeadPosition();
        while (pos != NULL) {
            CScreenStoreItem* pOldItem = m_lIdentifyItems.GetNext(pos);
            if (!pOldItem->m_bSelected) {
                delete pOldItem;
            } else {
                lSelectedItems.AddTail(pOldItem);
            }
        }
        m_lIdentifyItems.RemoveAll();

        for (INT nIndex = 0; nIndex < CScreenInventory::PERSONAL_INVENTORY_SIZE; nIndex++) {
            CItem* pItem = pSprite->GetEquipment()->m_items[18 + nIndex];
            if (pItem == NULL) {
                continue;
            }

            CScreenStoreItem* pIdentifyItem = new CScreenStoreItem;

            pIdentifyItem->m_bSelected = FALSE;
            pIdentifyItem->m_pItem = pItem;
            pIdentifyItem->m_nSlot = nIndex;
            pIdentifyItem->m_nValue = m_pStore->m_header.m_nIdentifyCost;
            pIdentifyItem->m_bEnabled = m_pStore->IsValidIdentifyItem(pItem);

            CScreenStoreItem* pSelectedItem = NULL;
            POSITION posSelected = lSelectedItems.GetHeadPosition();
            while (posSelected != NULL) {
                CScreenStoreItem* pCandidate = lSelectedItems.GetNext(posSelected);
                if (pCandidate->m_pItem == pItem) {
                    pSelectedItem = pCandidate;
                }
            }

            if (pSelectedItem != NULL) {
                pIdentifyItem->m_bSelected = TRUE;
            }

            m_lIdentifyItems.AddTail(pIdentifyItem);
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        POSITION posSelected = lSelectedItems.GetHeadPosition();
        while (posSelected != NULL) {
            delete lSelectedItems.GetNext(posSelected);
        }
        lSelectedItems.RemoveAll();
    } else {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 4655
        UTIL_ASSERT(FALSE);
    }
}

// 0x677DA0
void CScreenStore::DestroyIdentifyItems()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    POSITION pos = m_lIdentifyItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lIdentifyItems.GetAt(pos);
        delete pItem;

        m_lIdentifyItems.GetNext(pos);
    }

    m_lIdentifyItems.RemoveAll();
}

// 0x677DD0
void CScreenStore::UpdateGroupCost()
{
    m_nGroupCost = 0;

    POSITION pos = m_lGroupItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lGroupItems.GetAt(pos);
        if (pItem->m_bSelected) {
            m_nGroupCost += pItem->m_nValue;
        }

        m_lGroupItems.GetNext(pos);
    }
}

// NOTE: Inlined.
void CScreenStore::UpdateIdentifyCost()
{
    m_dwIdentifyCost = 0;

    POSITION pos = m_lIdentifyItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lIdentifyItems.GetAt(pos);
        if (pItem->m_bSelected) {
            m_dwIdentifyCost += pItem->m_nValue;
        }

        m_lIdentifyItems.GetNext(pos);
    }
}

// 0x677E00
void CScreenStore::UpdateStoreCost()
{
    m_nStoreCost = 0;

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lStoreItems.GetAt(pos);
        if (pItem->m_bSelected) {
            m_nStoreCost += pItem->m_nValue;
        }

        m_lStoreItems.GetNext(pos);
    }
}

// 0x677E30
void CScreenStore::UpdateSpellCost()
{
    m_dwSpellCost = 0;

    POSITION pos = m_lSpellItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lSpellItems.GetAt(pos);
        if (pItem->m_bSelected) {
            m_dwSpellCost += pItem->m_nValue;
        }

        m_lSpellItems.GetNext(pos);
    }
}

// 0x677E60
BOOL CScreenStore::IsBuyItemButtonClickable()
{
    DWORD nItems = 0;

    if ((m_pStore->m_header.m_nStoreFlags & 0x1) == 0) {
        return FALSE;
    }

    if (m_pBag != NULL && (m_pBag->m_header.m_nStoreFlags & 0x2) == 0) {
        return FALSE;
    }

    POSITION pos = m_lStoreItems.GetHeadPosition();
    if (pos == NULL) {
        return FALSE;
    }

    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lStoreItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            if (pStoreItem->m_pItem->GetMaxStackable() > 1) {
                nItems += (pStoreItem->m_pItem->GetMaxStackable() + pStoreItem->m_nCount - 1) / pStoreItem->m_pItem->GetMaxStackable();
            } else {
                nItems += pStoreItem->m_nCount;
            }
        }

        m_lStoreItems.GetNext(pos);
    }

    if (nItems == 0) {
        return FALSE;
    }

    if (m_pBag != NULL
        && m_pBag->m_header.m_nCapacity != 0
        && m_pBag->GetNumItems() + nItems > m_pBag->m_header.m_nCapacity) {
        return FALSE;
    }

    if (m_pBag != NULL) {
        return TRUE;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5020
    UTIL_ASSERT(pGame != NULL);

    CGameSave* pSave = pGame->GetGameSave();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5022
    UTIL_ASSERT(pSave != NULL);

    LONG nCharacterId = CGameObjectArray::INVALID_INDEX;
    if (m_nSelectedCharacter < g_pBaldurChitin->GetObjectGame()->m_nCharacters) {
        nCharacterId = g_pBaldurChitin->GetObjectGame()->m_characterPortraits[m_nSelectedCharacter];
    }

    CGameObject* pObject;

    // NOTE: Unlike every other GetShare loop in this screen, the original only
    // spins on DENIED, and it never calls ReleaseShare on the way out.
    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 5031
        UTIL_ASSERT(FALSE);
    }

    CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

    if (pSprite->m_equipment.GetUsedSlotsCount() + nItems > CScreenInventory::PERSONAL_INVENTORY_SIZE) {
        return FALSE;
    }

    return TRUE;
}

// 0x678050
void CScreenStore::OnBuyItemButtonClick()
{
    CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
    renderLock.Lock(INFINITE);
    if (IsBuyItemButtonClickable()) {
        // NOTE: These are three independent conditions, not the else-chain
        // OnSellItemButtonClick uses: buying out of a bag store while a bag is
        // open runs BuyBagItemsExt, BuyPersonalItemsExt and BuyBagItems in turn.
        if (m_pStore->GetType() == 4 && m_pBag != NULL) {
            BuyBagItemsExt();
        }

        if (m_pStore->GetType() == 4) {
            BuyPersonalItemsExt();
        }

        if (m_pBag != NULL) {
            BuyBagItems();
        } else {
            BuyPersonalItems();
        }

        // FIXME: Wrong placement for explicit unlock, should be outside of
        // this condition.
        renderLock.Unlock();
    }
}

// 0x678110
void CScreenStore::BuyPersonalItems()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5099
    UTIL_ASSERT(pGame != NULL);

    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    CItem* pSoundItem = NULL;
    LONG nPartyGold = pGame->GetGameSave()->m_nPartyGold;

    if (nPartyGold < m_nStoreCost) {
        m_nErrorState = 2;
        m_strErrorText = 11047;
        m_strErrorButtonText[0] = 11973;
        SummonPopup(10);
        return;
    }

    INT nIndex = 0;

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lStoreItems.GetAt(pos);
        if (!pStoreItem->m_bSelected) {
            nIndex++;
        } else {
            CItem cItem;
            CItem cStoreItem;

            m_pStore->GetItem(nIndex, cStoreItem);

            cItem.SetResRef(cStoreItem.GetResRef(), TRUE);
            cItem.m_useCount1 = cStoreItem.m_useCount1;
            cItem.m_useCount2 = cStoreItem.m_useCount2;
            cItem.m_useCount3 = cStoreItem.m_useCount3;
            cItem.m_wear = cStoreItem.m_wear;
            cItem.m_flags = cStoreItem.m_flags;

            if (m_pStore->GetItemExt(nIndex, cItem)) {
                DWORD dwFlags = cItem.m_flags;
                BOOLEAN bRemoved = FALSE;

                INT nRemaining;
                if (cItem.GetMaxStackable() < 2) {
                    nRemaining = pStoreItem->m_nCount;
                } else if (pStoreItem->m_nStoreCount < 2 || cItem.GetUsageCount(0) != 0) {
                    nRemaining = cItem.GetUsageCount(0) * pStoreItem->m_nCount;
                } else {
                    nRemaining = pStoreItem->m_nCount;
                }

                SHORT nSlot = pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter));

                if (nSlot >= 0) {
                    while (TRUE) {
                        CItem* pNewItem = new CItem(cItem);

                        INT nTransfer;
                        if (cItem.GetMaxStackable() < 2) {
                            nRemaining--;
                            nTransfer = 1;
                        } else {
                            INT nStack = cItem.GetMaxStackable();

                            // Everything that is not ammunition, a potion, a
                            // scroll or a gem is bought half a stack at a time.
                            if (cItem.GetItemType() != 9
                                && cItem.GetItemType() != 0x47
                                && cItem.GetItemType() != 0xB
                                && cItem.GetItemType() != 0xD
                                && cItem.GetItemType() != 0x22
                                && cItem.GetItemType() != 0) {
                                nStack /= 2;
                            }

                            pNewItem->SetUsageCount(0, static_cast<WORD>(min(nStack, nRemaining)));

                            nTransfer = min(nStack, nRemaining);
                            nRemaining -= nTransfer;
                        }

                        STRREF strError;
                        if (!pGame->SwapItemPersonalInventory(static_cast<SHORT>(m_nSelectedCharacter),
                                pNewItem,
                                nSlot,
                                strError,
                                0xFFFF,
                                FALSE,
                                TRUE)) {
                            if (pNewItem != NULL) {
                                delete pNewItem;
                            }

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5239
                            UTIL_ASSERT(FALSE);
                            break;
                        }

                        if (m_pStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, &bRemoved) != 1) {
                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5232
                            UTIL_ASSERT(FALSE);
                            break;
                        }

                        if (pSoundItem == NULL) {
                            pSoundItem = new CItem(cItem);
                        }

                        CMessage* message = new CMessageStoreRemoveItem(m_pStore->m_resRef,
                            CResRef(cItem.GetResRef().GetResRef()),
                            dwFlags,
                            nIndex,
                            nTransfer,
                            CGameObjectArray::INVALID_INDEX,
                            CGameObjectArray::INVALID_INDEX);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        if (!g_pChitin->cNetwork.m_bIsHost) {
                            CStore::InvalidateStore(m_pStore->m_resRef);
                        } else {
                            CStore* pServerStore = pGame->GetServerStore(m_pStore->m_resRef);

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5211
                            UTIL_ASSERT(pServerStore != NULL);

                            pServerStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, NULL);
                        }

                        if (cItem.GetItemType() == 9
                            || cItem.GetItemType() == 0x47
                            || cItem.GetItemType() == 0xB
                            || cItem.GetItemType() == 0xD
                            || cItem.GetItemType() == 0x22
                            || cItem.GetItemType() == 0) {
                            nPartyGold -= pStoreItem->m_nSingleValue * nTransfer;
                        } else {
                            nPartyGold -= pStoreItem->m_nSingleValue;
                        }

                        if (bRemoved || nRemaining == 0) {
                            break;
                        }

                        nSlot = pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter));
                        if (nSlot < 0) {
                            break;
                        }
                    }
                }

                pStoreItem->m_bSelected = FALSE;

                if (!bRemoved) {
                    nIndex++;
                }
            }
        }

        m_lStoreItems.GetNext(pos);
    }

    pGame->AddPartyGold(nPartyGold - pGame->GetGameSave()->m_nPartyGold);

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(nTopGroupItem);
    }

    if (pSoundItem != NULL) {
        g_pBaldurChitin->m_pEngineInventory->PlaySwapSound(pSoundItem, NULL);
        delete pSoundItem;
    }

    UpdateMainPanel();
}

// 0x678900
void CScreenStore::BuyBagItems()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5308
    UTIL_ASSERT(pGame != NULL);

    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    LONG nPartyGold = pGame->GetGameSave()->m_nPartyGold;
    CItem* pSoundItem = NULL;

    if (nPartyGold < m_nStoreCost) {
        m_nErrorState = 2;
        m_strErrorText = 11047;
        m_strErrorButtonText[0] = 11973;
        SummonPopup(10);
        return;
    }

    INT nIndex = 0;

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lStoreItems.GetAt(pos);
        if (!pStoreItem->m_bSelected) {
            nIndex++;
        } else {
            CItem cItem;
            CItem cStoreItem;

            m_pStore->GetItem(nIndex, cStoreItem);

            cItem.SetResRef(cStoreItem.GetResRef(), TRUE);
            cItem.m_useCount1 = cStoreItem.m_useCount1;
            cItem.m_useCount2 = cStoreItem.m_useCount2;
            cItem.m_useCount3 = cStoreItem.m_useCount3;
            cItem.m_wear = cStoreItem.m_wear;
            cItem.m_flags = cStoreItem.m_flags;

            if (m_pStore->GetItemExt(nIndex, cItem)) {
                DWORD dwFlags = cItem.m_flags;
                BOOLEAN bRemoved = FALSE;

                INT nRemaining;
                if (cItem.GetMaxStackable() < 2) {
                    nRemaining = pStoreItem->m_nCount;
                } else if (pStoreItem->m_nStoreCount < 2 || cItem.GetUsageCount(0) != 0) {
                    nRemaining = cItem.GetUsageCount(0) * pStoreItem->m_nCount;
                } else {
                    nRemaining = pStoreItem->m_nCount;
                }

                if (pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter)) >= 0) {
                    while (TRUE) {
                        INT nTransfer;

                        // Unlike BuyPersonalItems there is no half-stack rule
                        // here: a bag takes a full maximum stack at a time.
                        if (cItem.GetMaxStackable() < 2) {
                            nRemaining--;
                            nTransfer = 1;
                        } else {
                            cItem.SetUsageCount(0,
                                static_cast<WORD>(cItem.GetMaxStackable() <= nRemaining
                                        ? cItem.GetMaxStackable()
                                        : nRemaining));

                            if (nRemaining < cItem.GetMaxStackable()) {
                                nTransfer = nRemaining;
                                nRemaining = 0;
                            } else {
                                nTransfer = cItem.GetMaxStackable();
                                nRemaining -= nTransfer;
                            }
                        }

                        if (m_pBag->AddItemExt(cItem, 0x1) == -1) {
                            break;
                        }

                        CMessage* message = new CMessageStoreAddItem(m_pBag->m_resRef,
                            cItem.GetItemFile(),
                            CGameObjectArray::INVALID_INDEX,
                            CGameObjectArray::INVALID_INDEX);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        if (!g_pChitin->cNetwork.m_bIsHost) {
                            CStore::InvalidateStore(m_pBag->m_resRef);
                        } else {
                            CStore* pServerBag = pGame->GetServerStore(m_pBag->m_resRef);

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5404
                            UTIL_ASSERT(pServerBag != NULL);

                            pServerBag->AddItemExt(cItem, 0x1);
                        }

                        if (m_pStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, &bRemoved) != 1) {
                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5448
                            UTIL_ASSERT(FALSE);
                            break;
                        }

                        if (pSoundItem == NULL) {
                            pSoundItem = new CItem(cItem);
                        }

                        message = new CMessageStoreRemoveItem(m_pStore->m_resRef,
                            CResRef(cItem.GetResRef().GetResRef()),
                            dwFlags,
                            nIndex,
                            nTransfer,
                            CGameObjectArray::INVALID_INDEX,
                            CGameObjectArray::INVALID_INDEX);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        if (!g_pChitin->cNetwork.m_bIsHost) {
                            CStore::InvalidateStore(m_pStore->m_resRef);
                        } else {
                            CStore* pServerStore = pGame->GetServerStore(m_pStore->m_resRef);

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5427
                            UTIL_ASSERT(pServerStore != NULL);

                            pServerStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, NULL);
                        }

                        if (cItem.GetItemType() == 9
                            || cItem.GetItemType() == 0x47
                            || cItem.GetItemType() == 0xB
                            || cItem.GetItemType() == 0xD
                            || cItem.GetItemType() == 0x22
                            || cItem.GetItemType() == 0) {
                            nPartyGold -= pStoreItem->m_nSingleValue * nTransfer;
                        } else {
                            nPartyGold -= pStoreItem->m_nSingleValue;
                        }

                        if (bRemoved
                            || nRemaining == 0
                            || pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter)) < 0) {
                            break;
                        }
                    }
                }

                pStoreItem->m_bSelected = FALSE;

                if (!bRemoved) {
                    nIndex++;
                }
            }
        }

        m_lStoreItems.GetNext(pos);
    }

    pGame->AddPartyGold(nPartyGold - pGame->GetGameSave()->m_nPartyGold);

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(nTopGroupItem);
    }

    if (pSoundItem != NULL) {
        g_pBaldurChitin->m_pEngineInventory->PlaySwapSound(pSoundItem, NULL);
        delete pSoundItem;
    }

    UpdateMainPanel();
}

// 0x679170
void CScreenStore::BuyPersonalItemsExt()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5517
    UTIL_ASSERT(pGame != NULL);

    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    CItem* pSoundItem = NULL;
    INT nIndex = 0;

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lStoreItems.GetAt(pos);
        if (!pStoreItem->m_bSelected) {
            nIndex++;
        } else {
            CItem cItem;
            CItem cStoreItem;

            m_pStore->GetItem(nIndex, cStoreItem);

            cItem.SetResRef(cStoreItem.GetResRef(), TRUE);
            cItem.m_useCount1 = cStoreItem.m_useCount1;
            cItem.m_useCount2 = cStoreItem.m_useCount2;
            cItem.m_useCount3 = cStoreItem.m_useCount3;
            cItem.m_wear = cStoreItem.m_wear;
            cItem.m_flags = cStoreItem.m_flags;

            if (m_pStore->GetItemExt(nIndex, cItem)) {
                DWORD dwFlags = cItem.m_flags;
                BOOLEAN bRemoved = FALSE;

                // A bag transfer is free of charge and counts whole rows, so
                // there is none of the usage-count arithmetic BuyPersonalItems
                // does.
                INT nRemaining = 1;
                if (cItem.GetMaxStackable() > 1) {
                    nRemaining = pStoreItem->m_nCount;
                }

                SHORT nSlot = pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter));

                if (nSlot >= 0) {
                    while (TRUE) {
                        CItem* pNewItem = new CItem(cItem);

                        INT nTransfer;
                        if (cItem.GetMaxStackable() < 2) {
                            nRemaining--;
                            nTransfer = 1;
                        } else {
                            pNewItem->SetUsageCount(0,
                                static_cast<WORD>(cItem.GetMaxStackable() <= nRemaining
                                        ? cItem.GetMaxStackable()
                                        : nRemaining));

                            if (nRemaining < cItem.GetMaxStackable()) {
                                nTransfer = nRemaining;
                                nRemaining = 0;
                            } else {
                                nTransfer = cItem.GetMaxStackable();
                                nRemaining -= nTransfer;
                            }
                        }

                        STRREF strError;
                        if (!pGame->SwapItemPersonalInventory(static_cast<SHORT>(m_nSelectedCharacter),
                                pNewItem,
                                nSlot,
                                strError,
                                0xFFFF,
                                FALSE,
                                TRUE)) {
                            if (pNewItem != NULL) {
                                delete pNewItem;
                            }

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5614
                            UTIL_ASSERT(FALSE);
                            break;
                        }

                        if (m_pStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, &bRemoved) != 1) {
                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5607
                            UTIL_ASSERT(FALSE);
                            break;
                        }

                        if (pSoundItem == NULL) {
                            pSoundItem = new CItem(cItem);
                        }

                        CMessage* message = new CMessageStoreRemoveItem(m_pStore->m_resRef,
                            CResRef(cItem.GetResRef().GetResRef()),
                            dwFlags,
                            nIndex,
                            nTransfer,
                            CGameObjectArray::INVALID_INDEX,
                            CGameObjectArray::INVALID_INDEX);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        if (!g_pChitin->cNetwork.m_bIsHost) {
                            CStore::InvalidateStore(m_pStore->m_resRef);
                        } else {
                            CStore* pServerStore = pGame->GetServerStore(m_pStore->m_resRef);

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5596
                            UTIL_ASSERT(pServerStore != NULL);

                            pServerStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, NULL);
                        }

                        if (bRemoved || nRemaining == 0) {
                            break;
                        }

                        nSlot = pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter));
                        if (nSlot < 0) {
                            break;
                        }
                    }
                }

                pStoreItem->m_bSelected = FALSE;

                if (!bRemoved) {
                    nIndex++;
                }
            }
        }

        m_lStoreItems.GetNext(pos);
    }

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(nTopGroupItem);
    }

    if (pSoundItem != NULL) {
        g_pBaldurChitin->m_pEngineInventory->PlaySwapSound(pSoundItem, NULL);
        delete pSoundItem;
    }

    UpdateMainPanel();
}

// 0x679810
void CScreenStore::BuyBagItemsExt()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5681
    UTIL_ASSERT(pGame != NULL);

    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    CItem* pSoundItem = NULL;
    INT nIndex = 0;

    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lStoreItems.GetAt(pos);
        if (!pStoreItem->m_bSelected) {
            nIndex++;
        } else {
            CItem cItem;
            CItem cStoreItem;

            m_pStore->GetItem(nIndex, cStoreItem);

            cItem.SetResRef(cStoreItem.GetResRef(), TRUE);
            cItem.m_useCount1 = cStoreItem.m_useCount1;
            cItem.m_useCount2 = cStoreItem.m_useCount2;
            cItem.m_useCount3 = cStoreItem.m_useCount3;
            cItem.m_wear = cStoreItem.m_wear;
            cItem.m_flags = cStoreItem.m_flags;

            if (m_pStore->GetItemExt(nIndex, cItem)) {
                DWORD dwFlags = cItem.m_flags;
                BOOLEAN bRemoved = FALSE;

                INT nRemaining = 1;
                if (cItem.GetMaxStackable() > 1) {
                    nRemaining = pStoreItem->m_nCount;
                }

                if (pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter)) >= 0) {
                    while (TRUE) {
                        INT nTransfer;
                        if (cItem.GetMaxStackable() < 2) {
                            nRemaining--;
                            nTransfer = 1;
                        } else {
                            cItem.SetUsageCount(0,
                                static_cast<WORD>(cItem.GetMaxStackable() <= nRemaining
                                        ? cItem.GetMaxStackable()
                                        : nRemaining));

                            if (nRemaining < cItem.GetMaxStackable()) {
                                nTransfer = nRemaining;
                                nRemaining = 0;
                            } else {
                                nTransfer = cItem.GetMaxStackable();
                                nRemaining -= nTransfer;
                            }
                        }

                        if (m_pBag->AddItemExt(cItem, 0x1) == -1) {
                            break;
                        }

                        CMessage* message = new CMessageStoreAddItem(m_pBag->m_resRef,
                            cItem.GetItemFile(),
                            CGameObjectArray::INVALID_INDEX,
                            CGameObjectArray::INVALID_INDEX);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        if (!g_pChitin->cNetwork.m_bIsHost) {
                            CStore::InvalidateStore(m_pBag->m_resRef);
                        } else {
                            CStore* pServerBag = pGame->GetServerStore(m_pBag->m_resRef);

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5751
                            UTIL_ASSERT(pServerBag != NULL);

                            pServerBag->AddItemExt(cItem, 0x1);
                        }

                        if (m_pStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, &bRemoved) != 1) {
                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5785
                            UTIL_ASSERT(FALSE);
                            break;
                        }

                        if (pSoundItem == NULL) {
                            pSoundItem = new CItem(cItem);
                        }

                        message = new CMessageStoreRemoveItem(m_pStore->m_resRef,
                            CResRef(cItem.GetResRef().GetResRef()),
                            dwFlags,
                            nIndex,
                            nTransfer,
                            CGameObjectArray::INVALID_INDEX,
                            CGameObjectArray::INVALID_INDEX);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        if (!g_pChitin->cNetwork.m_bIsHost) {
                            CStore::InvalidateStore(m_pStore->m_resRef);
                        } else {
                            CStore* pServerStore = pGame->GetServerStore(m_pStore->m_resRef);

                            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                            // __LINE__: 5774
                            UTIL_ASSERT(pServerStore != NULL);

                            pServerStore->RemoveItemExt(cStoreItem.GetResRef(), dwFlags, nIndex, nTransfer, NULL);
                        }

                        if (bRemoved
                            || nRemaining == 0
                            || pGame->FindFirstFreeInventorySlot(static_cast<SHORT>(m_nSelectedCharacter)) < 0) {
                            break;
                        }
                    }
                }

                pStoreItem->m_bSelected = FALSE;

                if (!bRemoved) {
                    nIndex++;
                }
            }
        }

        m_lStoreItems.GetNext(pos);
    }

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(nTopGroupItem);
    }

    if (pSoundItem != NULL) {
        g_pBaldurChitin->m_pEngineInventory->PlaySwapSound(pSoundItem, NULL);
        delete pSoundItem;
    }

    UpdateMainPanel();
}

// 0x679F80
BOOL CScreenStore::IsSellItemButtonClickable()
{
    DWORD nItems = 0;

    if ((m_pStore->m_header.m_nStoreFlags & 0x2) == 0) {
        return FALSE;
    }

    if (m_pBag != NULL && (m_pBag->m_header.m_nStoreFlags & 0x1) == 0) {
        return FALSE;
    }

    POSITION pos = m_lGroupItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lGroupItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            if (pStoreItem->m_pItem->GetMaxStackable() > 1) {
                nItems += (pStoreItem->m_pItem->GetMaxStackable() + pStoreItem->m_nCount - 1) / pStoreItem->m_pItem->GetMaxStackable();
            } else {
                nItems += pStoreItem->m_nCount;
            }
        }
        m_lGroupItems.GetNext(pos);
    }

    if (nItems == 0) {
        return FALSE;
    }

    if (m_pStore->m_header.m_nCapacity != 0
        && nItems + m_pStore->GetNumItems() > m_pStore->m_header.m_nCapacity) {
        return FALSE;
    }

    return TRUE;
}

// 0x67A070
void CScreenStore::OnSellItemButtonClick()
{
    CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
    renderLock.Lock(INFINITE);
    if (IsSellItemButtonClickable()) {
        if (m_pStore->GetType() == 4 && m_pBag != NULL) {
            SellBagItemsExt();
        } else if (m_pStore->GetType() == 4) {
            SellPersonalItemsExt();
        } else if (m_pBag != NULL) {
            SellBagItems();
        } else {
            SellPersonalItems();
        }
        // FIXME: Wrong placement for explicit unlock, should be outside of
        // this condition.
        renderLock.Unlock();
    }
}

// 0x67A130
void CScreenStore::SellPersonalItems()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 5971
    UTIL_ASSERT(pGame != NULL);

    POSITION pos = m_lGroupItems.GetHeadPosition();
    BOOLEAN bPlaySound = FALSE;
    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    DWORD dwCost = 0;
    INT nIndex = 0;
    CMessage* message;

    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lGroupItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            CItem* pItem = NULL;
            STRREF description;
            CResRef cResIcon;
            CResRef cResItem;
            WORD wCount;

            pGame->InventoryInfoPersonal(static_cast<SHORT>(m_nSelectedCharacter),
                pStoreItem->m_nSlot + 18,
                pItem,
                description,
                cResIcon,
                cResItem,
                wCount,
                TRUE);

            if (pItem != NULL) {
                DWORD dwFlags = pItem->m_flags;

                if ((pItem->m_flags & 0x4) != 0) {
                    m_nErrorState = 2;
                    m_strErrorText = 20644;
                    m_strErrorButtonText[0] = 11973;
                    SummonPopup(10);
                    break;
                }

                CItem* pNewItem = NULL;
                STRREF strError;
                if (pGame->SwapItemPersonalInventory(static_cast<SHORT>(m_nSelectedCharacter), pNewItem, pStoreItem->m_nSlot + 18, strError, -1, FALSE, TRUE)) {
                    if (pNewItem != NULL) {
                        if (!bPlaySound) {
                            bPlaySound = 1;
                        }

                        pNewItem->m_flags = dwFlags | 0x1;

                        if (m_pStore->AddItemExt(*pNewItem, 0x1) != -1) {
                            message = new CMessageStoreAddItem(m_pStore->m_resRef,
                                pNewItem->GetItemFile(),
                                CGameObjectArray::INVALID_INDEX,
                                CGameObjectArray::INVALID_INDEX);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                            if (g_pChitin->cNetwork.GetSessionHosting()) {
                                CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_pStore->m_resRef);

                                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                                // __LINE__: 6055
                                UTIL_ASSERT(pStore != NULL);

                                pStore->AddItemExt(*pNewItem, 0x1);
                            } else {
                                CStore::InvalidateStore(m_pStore->m_resRef);
                            }

                            delete pNewItem;

                            dwCost += pStoreItem->m_nSingleValue;
                            pStoreItem->m_bSelected = FALSE;
                        }
                    }
                }
            }
        }

        m_lGroupItems.GetNext(pos);
    }

    pGame->AddPartyGold(static_cast<LONG>(dwCost));

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(0);
    }

    if (bPlaySound) {
        PlayGUISound(CResRef("ACT_03D"));
    }

    UpdateMainPanel();
}

// 0x67A5B0
void CScreenStore::SellBagItems()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 6125
    UTIL_ASSERT(pGame != NULL);

    POSITION pos = m_lGroupItems.GetHeadPosition();
    BOOLEAN bPlaySound = FALSE;
    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    DWORD dwCost = 0;
    INT nIndex = 0;
    CMessage* message;

    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lGroupItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            CItem cItem;
            m_pBag->GetItem(nIndex, cItem);

            if ((cItem.m_flags & 0x4) != 0) {
                m_nErrorState = 2;
                m_strErrorText = 20644;
                m_strErrorButtonText[0] = 11973;
                SummonPopup(10);
                break;
            }

            DWORD dwFlags = cItem.m_flags;
            WORD wCount = 1;
            if (cItem.GetMaxStackable() > 1) {
                wCount = cItem.GetUsageCount(0);
            }

            cItem.m_flags |= 0x1;

            if (m_pStore->AddItemExt(cItem, 0x1) == -1) {
                break;
            }

            message = new CMessageStoreAddItem(m_pStore->m_resRef,
                cItem.GetItemFile(),
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            if (g_pChitin->cNetwork.GetSessionHosting()) {
                CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_pStore->m_resRef);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 6194
                UTIL_ASSERT(pStore != NULL);

                pStore->AddItemExt(cItem, 0x1);
            } else {
                CStore::InvalidateStore(m_pStore->m_resRef);
            }

            if (cItem.GetMaxStackable() > 1) {
                cItem.SetUsageCount(0, wCount);
            }

            BOOLEAN v1;
            if (m_pBag->RemoveItemExt(cItem.GetResRef(), dwFlags, nIndex, wCount, &v1) != 1) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 6230
                UTIL_ASSERT(FALSE);
            }

            if (!bPlaySound) {
                bPlaySound = TRUE;
            }

            message = new CMessageStoreRemoveItem(m_pBag->m_resRef,
                cItem.GetResRef(),
                dwFlags,
                nIndex,
                wCount,
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            if (g_pChitin->cNetwork.GetSessionHosting()) {
                CStore* pBag = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_pBag->m_resRef);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 6222
                UTIL_ASSERT(pBag != NULL);

                pBag->RemoveItemExt(cItem.GetResRef(), dwFlags, nIndex, wCount, NULL);
            } else {
                CStore::InvalidateStore(m_pBag->m_resRef);
            }

            dwCost += pStoreItem->m_nSingleValue;

            if (!v1) {
                nIndex++;
            }

            pStoreItem->m_bSelected = FALSE;
        }

        m_lGroupItems.GetNext(pos);
    }

    pGame->AddPartyGold(static_cast<LONG>(dwCost));

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(0);
    }

    if (bPlaySound) {
        PlayGUISound(CResRef("ACT_03D"));
    }

    UpdateMainPanel();
}

// 0x67ABF0
void CScreenStore::SellPersonalItemsExt()
{
    INT nIndex = 0;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 6292
    UTIL_ASSERT(pGame != NULL);

    POSITION pos = m_lGroupItems.GetHeadPosition();
    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    CItem* pSwapSoundItem = NULL;
    CMessage* message;

    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lGroupItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            CItem* pItem = NULL;
            STRREF description;
            CResRef cResIcon;
            CResRef cResItem;
            WORD wCount;

            pGame->InventoryInfoPersonal(static_cast<SHORT>(m_nSelectedCharacter),
                pStoreItem->m_nSlot + 18,
                pItem,
                description,
                cResIcon,
                cResItem,
                wCount,
                TRUE);

            if (pItem != NULL) {
                DWORD dwFlags = pItem->m_flags;

                CItem* pNewItem = NULL;
                STRREF strError;
                if (pGame->SwapItemPersonalInventory(static_cast<SHORT>(m_nSelectedCharacter), pNewItem, pStoreItem->m_nSlot + 18, strError, -1, FALSE, TRUE)) {
                    if (pNewItem != NULL) {
                        if (pSwapSoundItem == NULL) {
                            pSwapSoundItem = new CItem(*pNewItem);
                        }

                        pNewItem->m_flags = dwFlags | 0x1;

                        if (m_pStore->AddItemExt(*pNewItem, 0x1) != -1) {
                            message = new CMessageStoreAddItem(m_pStore->m_resRef,
                                pNewItem->GetItemFile(),
                                CGameObjectArray::INVALID_INDEX,
                                CGameObjectArray::INVALID_INDEX);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                            if (g_pChitin->cNetwork.GetSessionHosting()) {
                                CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_pStore->m_resRef);

                                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                                // __LINE__: 6362
                                UTIL_ASSERT(pStore != NULL);

                                pStore->AddItemExt(*pNewItem, 0x1);
                            } else {
                                CStore::InvalidateStore(m_pStore->m_resRef);
                            }

                            delete pNewItem;
                            pStoreItem->m_bSelected = FALSE;
                        }
                    }
                }
            }
        }

        m_lGroupItems.GetNext(pos);
    }

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(0);
    }

    if (pSwapSoundItem != NULL) {
        g_pBaldurChitin->m_pEngineInventory->PlaySwapSound(pSwapSoundItem, NULL);
        delete pSwapSoundItem;
    }

    UpdateMainPanel();
}

// 0x67B030
void CScreenStore::SellBagItemsExt()
{
    INT nIndex = 0;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 6429
    UTIL_ASSERT(pGame != NULL);

    POSITION pos = m_lGroupItems.GetHeadPosition();
    INT nTopStoreItem = m_nTopStoreItem;
    INT nTopGroupItem = m_nTopGroupItem;
    CItem* pSwapSoundItem = NULL;
    CMessage* message;

    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lGroupItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            CItem cItem;
            m_pBag->GetItem(nIndex, cItem);

            DWORD dwFlags = cItem.m_flags;
            WORD wCount = 1;
            if (cItem.GetMaxStackable() > 1) {
                wCount = cItem.GetUsageCount(0);
            }

            if (m_pStore->AddItemExt(cItem, 0x1) == -1) {
                break;
            }

            message = new CMessageStoreAddItem(m_pStore->m_resRef,
                cItem.GetItemFile(),
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            if (g_pChitin->cNetwork.GetSessionHosting()) {
                CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_pStore->m_resRef);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 6482
                UTIL_ASSERT(pStore != NULL);

                pStore->AddItemExt(cItem, 0x1);
            } else {
                CStore::InvalidateStore(m_pStore->m_resRef);
            }

            if (cItem.GetMaxStackable() > 1) {
                cItem.SetUsageCount(0, wCount);
            }

            BOOLEAN v1;
            if (m_pBag->RemoveItemExt(cItem.GetResRef(), dwFlags, nIndex, wCount, &v1) != 1) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 6516
                UTIL_ASSERT(FALSE);
            }

            if (pSwapSoundItem == NULL) {
                pSwapSoundItem = new CItem(cItem);
            }

            message = new CMessageStoreRemoveItem(m_pBag->m_resRef,
                cItem.GetResRef(),
                dwFlags,
                nIndex,
                wCount,
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            if (g_pChitin->cNetwork.GetSessionHosting()) {
                CStore* pBag = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_pBag->m_resRef);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 6510
                UTIL_ASSERT(pBag != NULL);

                pBag->RemoveItemExt(cItem.GetResRef(), dwFlags, nIndex, wCount, NULL);
            } else {
                CStore::InvalidateStore(m_pBag->m_resRef);
            }

            if (v1) {
                nIndex++;
            }

            pStoreItem->m_bSelected = FALSE;
        }

        m_lGroupItems.GetNext(pos);
    }

    UpdateStoreItems();
    UpdateGroupItems();

    // NOTE: Uninline.
    UpdateStoreCost();

    // NOTE: Uninline.
    UpdateGroupCost();

    if (nTopStoreItem >= 0) {
        if (m_nTopStoreItem <= nTopStoreItem && nTopStoreItem < m_nTopStoreItem + 6) {
            nTopStoreItem = m_nTopStoreItem;
        }

        nTopStoreItem = min(max(m_lStoreItems.GetCount() - 6, 0), nTopStoreItem);

        // NOTE: Uninline.
        SetTopStoreItem(nTopStoreItem);
    }

    if (nTopGroupItem >= 0) {
        if (m_nTopGroupItem <= nTopGroupItem && nTopGroupItem < m_nTopGroupItem + 6) {
            nTopGroupItem = m_nTopGroupItem;
        }

        nTopGroupItem = min(max(m_lGroupItems.GetCount() - 6, 0), nTopGroupItem);

        // NOTE: Uninline.
        SetTopGroupItem(0);
    }

    if (pSwapSoundItem != NULL) {
        g_pBaldurChitin->m_pEngineInventory->PlaySwapSound(pSwapSoundItem, NULL);
        delete pSwapSoundItem;
    }

    UpdateMainPanel();
}

// 0x67B640
void CScreenStore::OnIdentifyItemButtonClick()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 6643
    UTIL_ASSERT(pGame != NULL);

    CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    DWORD nPartyGold = pGame->GetGameSave()->m_nPartyGold;
    POSITION pos = m_lIdentifyItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lIdentifyItems.GetNext(pos);
        if (pStoreItem->m_bSelected) {
            break;
        }
    }

    if (pos != NULL) {
        if (m_dwIdentifyCost <= nPartyGold) {
            CUIPanel* pPanel = m_cUIManager.GetPanel(4);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 6665
            UTIL_ASSERT(pPanel != NULL);

            CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(23));

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 6667
            UTIL_ASSERT(pText != NULL);

            pos = m_lIdentifyItems.GetHeadPosition();
            while (pos != NULL) {
                CScreenStoreItem* pStoreItem = m_lIdentifyItems.GetNext(pos);
                if (pStoreItem->m_bSelected
                    && static_cast<LONG>(nPartyGold) >= pStoreItem->m_nValue) {
                    pStoreItem->m_pItem->m_flags |= 0x1;
                    nPartyGold -= pStoreItem->m_nValue;
                    UpdateText(pText, "%s", FetchString(pStoreItem->m_pItem->GetDescription()));
                    UpdateText(pText, "");
                    UpdateText(pText, "-----------------------------");
                    UpdateText(pText, "");
                }
            }

            pGame->AddPartyGold(nPartyGold - pGame->GetGameSave()->m_nPartyGold);
            UpdateIdentifyItems();

            // NOTE: Uninline.
            SetTopIdentifyItem(0);

            // NOTE: Uninline.
            UpdateIdentifyCost();

            UpdateMainPanel();
        } else {
            m_nErrorState = 2;
            m_strErrorText = 11050;
            m_strErrorButtonText[0] = 11973;
            SummonPopup(10);
        }
    }

    renderLock.Unlock();
}

// 0x67B8A0
void CScreenStore::OnBuySpellButtonClick()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 6999
    UTIL_ASSERT(pGame != NULL);

    DWORD nPartyGold = pGame->GetGameSave()->m_nPartyGold;

    CTypedPtrList<CPtrList, CAIAction*> lActions;
    CString sSpell;

    CSingleLock renderLock(&(GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    POSITION pos = m_lSpellItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pStoreItem = m_lSpellItems.GetAt(pos);
        if (pStoreItem->m_bSelected) {
            break;
        }

        m_lSpellItems.GetNext(pos);
    }

    if (pos != NULL) {
        if (IsCharacterInRange(static_cast<SHORT>(m_nSelectedCharacter))
            || (m_pMainPanel->m_nID == 5
                && !IsCharacterAlive(static_cast<SHORT>(m_nSelectedCharacter)))) {
            if (m_dwSpellCost <= nPartyGold) {
                LONG nProprietorId = m_cAIProprietor.GetInstance();

                CGameObject* pProprietor;

                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetDeny(nProprietorId,
                        CGameObjectArray::THREAD_ASYNCH,
                        &pProprietor,
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    // NOTE: Uninline.
                    LONG nCharacterId = pGame->GetCharacterId(static_cast<SHORT>(m_nSelectedCharacter));

                    CGameSprite* pSprite;

                    do {
                        rc = pGame->GetObjectArray()->GetDeny(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc != CGameObjectArray::SUCCESS) {
                        renderLock.Unlock();
                        pGame->GetObjectArray()->ReleaseDeny(nProprietorId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        return;
                    }

                    pos = m_lSpellItems.GetHeadPosition();
                    while (pos != NULL) {
                        CScreenStoreItem* pStoreItem = m_lSpellItems.GetAt(pos);
                        if (pStoreItem->m_bSelected
                            && static_cast<LONG>(nPartyGold) >= pStoreItem->m_nValue) {
                            nPartyGold -= pStoreItem->m_nValue;
                            pStoreItem->m_bSelected = FALSE;
                            pStoreItem->m_cResSpell.CopyToString(sSpell);

                            CAIAction* pAction = new CAIAction();
                            pAction->m_actionID = CAIAction::APPLYSPELL;
                            pAction->m_acteeID.Set(pSprite->GetAIType());
                            pAction->SetString1(sSpell);
                            pAction->m_specificID3 = pSprite->GetDerivedStats()->GetClassMaskLevel(0xFFF);
                            pAction->m_internalFlags |= 0x20000000;
                            lActions.AddTail(pAction);

                            CSound cSound;
                            if (_stricmp(sSpell, "SPPR103") == 0) {
                                cSound.SetResRef(CResRef("EFF_P26"), TRUE, TRUE);
                            } else if (_stricmp(sSpell, "SPPR303") == 0) {
                                cSound.SetResRef(CResRef("EFF_M10"), TRUE, TRUE);
                            } else if (_stricmp(sSpell, "SPPR307") == 0) {
                                cSound.SetResRef(CResRef("EFF_M10"), TRUE, TRUE);
                            } else if (_stricmp(sSpell, "SPPR401") == 0) {
                                cSound.SetResRef(CResRef("EFF_P26"), TRUE, TRUE);
                            } else if (_stricmp(sSpell, "SPPR212") == 0) {
                                cSound.SetResRef(CResRef("EFF_P17"), TRUE, TRUE);
                            } else if (_stricmp(sSpell, "SPPR504") == 0) {
                                cSound.SetResRef(CResRef("EFF_P18"), TRUE, TRUE);
                            }

                            PlayGUISound(cSound.GetResRef());
                        }

                        m_lSpellItems.GetNext(pos);
                    }

                    POSITION posAction = lActions.GetHeadPosition();
                    while (posAction != NULL) {
                        CAIAction* pAction = lActions.GetNext(posAction);

                        CMessage* message = new CMessageAddAction(*pAction,
                            nProprietorId,
                            nCharacterId);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                        delete pAction;
                    }

                    lActions.RemoveAll();

                    pSprite->field_562C = 1;
                    pSprite->ProcessEffectList();

                    // NOTE: Uninline.
                    BOOLEAN bInControl = pSprite->InControl();

                    pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    pGame->GetObjectArray()->ReleaseDeny(nProprietorId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    pGame->AddPartyGold(nPartyGold - pGame->GetGameSave()->m_nPartyGold);

                    if (!bInControl
                        && (_stricmp(sSpell, "SPPR504") == 0
                            || _stricmp(sSpell, "SPPR712") == 0)) {
                        for (INT nPortraitNum = 0; nPortraitNum < 6; nPortraitNum++) {
                            if (IsCharacterInRange(static_cast<SHORT>(nPortraitNum))
                                && IsCharacterAlive(static_cast<SHORT>(nPortraitNum)) == 1) {
                                DWORD nOldPortraitNum = m_nSelectedCharacter;

                                OnPortraitLClick(nPortraitNum);
                                CheckEnablePortaits(1);

                                CUIPanel* pPanel = m_cUIManager.GetPanel(1);
                                CUIControlBase* pControl = pPanel->GetControl(nOldPortraitNum);
                                pControl->SetActive(FALSE);
                                pControl->SetInactiveRender(FALSE);
                                pPanel->InvalidateRect(0);
                                break;
                            }
                        }
                    }

                    UpdateSpellItems();

                    // NOTE: Uninline.
                    SetTopSpellItem(0);

                    // NOTE: Uninline.
                    UpdateSpellCost();

                    UpdateMainPanel();
                }
            } else {
                m_nErrorState = 2;
                m_strErrorText = 11048;
                m_strErrorButtonText[0] = 11973;
                SummonPopup(10);
            }
        }
    }

    renderLock.Unlock();
}

// 0x67C570
void CScreenStore::OnRentRoomButtonClick()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 7233
    UTIL_ASSERT(pGame != NULL);

    DWORD nPartyGold = pGame->GetGameSave()->m_nPartyGold;

    // NOTE: Uninline.
    DWORD nCost = GetRoomCost();

    CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    if (m_dwRoomType != 0) {
        if (nCost > nPartyGold) {
            m_nErrorState = 2;
            m_strErrorText = 11051;
            m_strErrorButtonText[0] = 11973;
            SummonPopup(10);
        } else {
            STRREF strError;
            if (pGame->CanRestParty(strError, 1, 0, 0)) {
                m_nErrorState = 0;
                m_strErrorText = 15358;
                m_strErrorButtonText[0] = 17199;
                m_strErrorButtonText[1] = 13727;
                SummonPopup(11);
            } else {
                m_nErrorState = 1;
                m_strErrorText = strError;
                m_strErrorButtonText[0] = 11973;
                SummonPopup(10);
            }
        }
    }

    renderLock.Unlock();
}

// 0x67C710
void CScreenStore::OnBuyDrinkButtonClick(INT nButton)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 7302
    UTIL_ASSERT(pGame != NULL);

    CSingleLock renderLock(&(GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    STR_RES strRes;

    DWORD nPartyGold = pGame->GetGameSave()->m_nPartyGold;
    INT nDrinkIndex = m_nTopDrinkItem + nButton;
    STRREF strName;
    DWORD dwCost;
    DWORD dwRumorChance;

    if (IsCharacterInRange(static_cast<SHORT>(m_nSelectedCharacter))
        && m_pStore->GetDrink(nDrinkIndex, strName, dwCost, dwRumorChance)) {
        if (dwCost <= nPartyGold) {
            LONG nCharacterId = pGame->GetCharacterId(static_cast<SHORT>(m_nSelectedCharacter));

            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                INT nIntoxicationRate;
                INT nRecoveryRate;
                INT nIntoxicationCap;

                pGame->GetRuleTables().GetIntoxicationInfo(static_cast<BYTE>(pSprite->GetDerivedStats()->m_nCON),
                    nIntoxicationRate,
                    nRecoveryRate,
                    nIntoxicationCap);

                CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(m_pMainPanel->GetControl(13));

                if (pSprite->GetBaseStats()->m_intoxication < nIntoxicationCap) {
                    PlayGUISound(CResRef("GAM_07"));
                    pSprite->GetBaseStats()->m_intoxication += nIntoxicationRate;
                    pSprite->field_562C = 1;
                    pSprite->field_562C = 1;
                    pSprite->ProcessEffectList();

                    // NOTE: Unsigned compare.
                    if (static_cast<DWORD>(rand() % 100 + 1) <= dwRumorChance) {
                        CGameDialogSprite cDialogSprite;
                        cDialogSprite.FetchRumor(CResRef(m_pStore->m_header.m_resRumor),
                            pSprite,
                            m_nDrinkRumorIndex,
                            strRes);

                        UpdateText(pText, "%s", strRes.szText);
                        UpdateText(pText, "");
                    }

                    pGame->AddPartyGold(-static_cast<LONG>(dwCost));
                } else {
                    UpdateText(pText,
                        "%s",
                        FetchString(10832));
                    UpdateText(pText, "");
                }

                pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                UpdateMainPanel();
            }
        } else {
            m_nErrorState = 2;
            m_strErrorText = 11049;
            m_strErrorButtonText[0] = 11973;
            SummonPopup(10);
        }
    }

    renderLock.Unlock();
}

// 0x67CBD0
void CScreenStore::OnDoneButtonClick()
{
    CUIPanel* pPanel = GetTopPopup();
    if (pPanel != NULL) {
        if (pPanel->m_nID == 10 || pPanel->m_nID == 11) {
            OnErrorButtonClick(0);
        } else {
            CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
            renderLock.Lock(INFINITE);

            switch (pPanel->m_nID) {
            case 12:
                field_5A4 = 0;
                field_5A8 = 0;
                break;
            case 14:
                m_cResInfoSpell = CResRef("");
                break;
            case 20:
                if (field_14E6) {
                    // NOTE: Uninline.
                    SelectStoreItem(field_14E2, TRUE);

                    // NOTE: Uninline.
                    SetStoreItemCount(field_14E2, field_14DE);

                    // NOTE: Uninline.
                    UpdateStoreCost();
                } else {
                    // NOTE: Uninline.
                    SelectGroupItem(field_14E2, TRUE);

                    // NOTE: Uninline.
                    SetStoreItemCount(field_14E2, field_14DE);

                    // NOTE: Uninline.
                    UpdateGroupCost();
                }

                field_14E2 = -1;
                field_14E6 = 1;
                UpdateMainPanel();
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 7642
                UTIL_ASSERT(FALSE);
            }

            DismissPopup();

            renderLock.Unlock();
        }
    }
}

// 0x67CE40
void CScreenStore::OnCancelButtonClick()
{
    CUIPanel* pPanel = GetTopPopup();
    if (pPanel != NULL) {
        CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
        renderLock.Lock(INFINITE);

        if (g_pBaldurChitin->m_pEngineWorld->m_bInControlOfStore
            || m_bInCancelEngine) {
            switch (pPanel->m_nID) {
            case 10:
            case 11:
                DismissPopup();
                break;
            case 12:
                field_5A4 = NULL;
                field_5A8 = 0;
                DismissPopup();
                break;
            case 14:
                m_cResInfoSpell = CResRef("");
                DismissPopup();
                break;
            case 20:
                if (field_14E6) {
                    // NOTE: Uninline.
                    SelectStoreItem(field_14E2, FALSE);

                    // NOTE: Uninline.
                    UpdateStoreCost();
                } else {
                    // NOTE: Uninline.
                    SelectGroupItem(field_14E2, FALSE);

                    // NOTE: Uninline.
                    UpdateGroupCost();
                }

                field_14E2 = -1;
                field_14E6 = 1;
                UpdateMainPanel();
                DismissPopup();
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
                // __LINE__: 7735
                UTIL_ASSERT(FALSE);
            }
        }

        renderLock.Unlock();
    }
}

// 0x67D050
void CScreenStore::StartStore(const CAIObjectType& proprietor, const CAIObjectType& customer, const CResRef& cResStore)
{
    if (m_pBag != NULL) {
        CloseBag(TRUE);
    }

    if (m_pStore != NULL) {
        // NOTE: Uninline.
        DeleteStore();
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 7777
    UTIL_ASSERT(cResStore != "");

    m_cResStore = cResStore;
    m_cAIProprietor = proprietor;
    m_cAICustomer = customer;
    m_nDrinkRumorIndex = -1;
    field_584 = -1;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->DemandServerStore(cResStore, TRUE);
        } else {
            if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                g_pChitin->cNetwork.CloseSession(TRUE);
                return;
            }
        }
    }

    CMessage* message = new CMessage101(TRUE, customer.GetInstance(), customer.GetInstance(), FALSE);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    m_pStore = new CStore(cResStore);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 7828
    UTIL_ASSERT(pGame != NULL);

    SHORT nPortrait = pGame->GetCharacterPortraitNum(customer.GetInstance());

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 7832
    UTIL_ASSERT(nPortrait >= 0);

    SetSelectedCharacter(nPortrait);

    field_5D8 = 9;
    field_5D9 = 0;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        for (SHORT nPortrait = 0; nPortrait < CMultiplayerSettings::MAX_CHARACTERS; nPortrait++) {
            LONG nCharacterId = pGame->GetCharacterId(nPortrait);
            if (nCharacterId == customer.GetInstance()) {
                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    field_5D8 = static_cast<BYTE>(pSprite->GetDerivedStats()->m_nCHR);
                    field_5D9 = pSprite->HasFeat(CGAMESPRITE_FEAT_MERCANTILE_BACKGROUND) ? 5 : 0;

                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        }
    } else {
        for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters(); nPortrait++) {
            if (IsCharacterInRange(nPortrait)) {
                LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    field_5D8 = static_cast<BYTE>(pSprite->GetDerivedStats()->m_nCHR);
                    field_5D9 = pSprite->HasFeat(CGAMESPRITE_FEAT_MERCANTILE_BACKGROUND) ? 5 : 0;

                    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        }
    }

    m_adwButtonPanelId[0] = -1;
    m_adwButtonPanelId[1] = -1;
    m_adwButtonPanelId[2] = -1;
    m_adwButtonPanelId[3] = -1;

    int buttonIndex = 0;

    switch (m_pStore->GetType()) {
    case 0:
        m_adwButtonPanelId[buttonIndex++] = 2;
        if ((m_pStore->m_header.m_nStoreFlags & 0x4) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 4;
        }
        if ((m_pStore->m_header.m_nStoreFlags & 0x20) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 5;
        }
        break;
    case 1:
        m_adwButtonPanelId[buttonIndex++] = 8;
        if ((m_pStore->m_header.m_nStoreFlags & 0x3) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 2;
        }
        if ((m_pStore->m_header.m_nStoreFlags & 0x4) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 4;
        }
        break;
    case 3:
        m_adwButtonPanelId[buttonIndex++] = 5;
        if ((m_pStore->m_header.m_nStoreFlags & 0x3) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 2;
        }
        if ((m_pStore->m_header.m_nStoreFlags & 0x4) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 4;
        }
        break;
    case 4:
        m_adwButtonPanelId[buttonIndex++] = 2;
        if ((m_pStore->m_header.m_nStoreFlags & 0x4) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 4;
        }
        if ((m_pStore->m_header.m_nStoreFlags & 0x20) != 0) {
            m_adwButtonPanelId[buttonIndex++] = 5;
        }
        break;
    case 5:
        m_adwButtonPanelId[0] = -1;
        m_adwButtonPanelId[1] = -1;
        m_adwButtonPanelId[2] = -1;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8037
        UTIL_ASSERT(FALSE);
    }

    CUIPanel* pBarSingle = m_cUIManager.GetPanel(3);
    CUIPanel* pBarMulti = m_cUIManager.GetPanel(15);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8047
    UTIL_ASSERT((pBarSingle != NULL) && (pBarMulti != NULL));

    pBarSingle->SetActive(FALSE);
    pBarMulti->SetActive(FALSE);

    if (g_pBaldurChitin->cNetwork.GetSessionOpen()
        && g_pBaldurChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        m_pButtonBar = pBarMulti;
        m_pButtonBar->SetActive(TRUE);
        m_pChatDisplay = static_cast<CUIControlTextDisplay*>(m_pButtonBar->GetControl(6));
    } else {
        m_pButtonBar = pBarSingle;
        m_pButtonBar->SetActive(TRUE);
        m_pChatDisplay = NULL;
    }

    CUIPanel* pPanel;
    CUIControlTextDisplay* pText;

    pPanel = m_cUIManager.GetPanel(8);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8066
    UTIL_ASSERT(pPanel != NULL);

    pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(13));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8068
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();

    pPanel = m_cUIManager.GetPanel(5);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8080
    UTIL_ASSERT(pPanel != NULL);

    pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(23));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8082
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();

    pPanel = m_cUIManager.GetPanel(4);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8087
    UTIL_ASSERT(pPanel != NULL);

    pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(23));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8089
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();

    pPanel = m_cUIManager.GetPanel(7);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8094
    UTIL_ASSERT(pPanel != NULL);

    pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(12));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8096
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();
}

// 0x67D7B0
void CScreenStore::StopStore()
{
    if (m_pStore != NULL) {
        if (m_pBag != NULL) {
            CloseBag(TRUE);
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(m_pStore->m_resRef);
        } else {
            CMessageStoreRelease* pMessageStoreRelease = new CMessageStoreRelease(m_pStore->m_resRef,
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessageStoreRelease, FALSE);
        }

        CMessage101* pMessage101 = new CMessage101(FALSE,
            m_cAICustomer.m_nInstance,
            m_cAICustomer.m_nInstance,
            FALSE);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage101, FALSE);

        // NOTE: Uninline.
        DeleteStore();
    }
}

// NOTE: Inlined.
void CScreenStore::DeleteStore()
{
    if (m_pStore != NULL) {
        delete m_pStore;
        m_pStore = NULL;
    }

    m_pButtonBar = NULL;
    m_pChatDisplay = NULL;
    m_cResStore = "";

    DestroyGroupItems(FALSE);

    // NOTE: Uninline.
    DestroyStoreItems(FALSE);

    // NOTE: Uninline.
    DestroySpellItems();

    // NOTE: Uninline.
    DestroyIdentifyItems();
}

// NOTE: Inlined.
DWORD CScreenStore::GetPanelButtonPanelId(INT nButtonIndex)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8251
    UTIL_ASSERT(0 <= nButtonIndex && nButtonIndex < CSCREENSTORE_NUM_BOTTOMBUTTONS);

    if (m_cResStore != "") {
        return m_adwButtonPanelId[nButtonIndex];
    }

    return MAXDWORD;
}

// 0x67D9D0
SHORT CScreenStore::GetPanelButtonSequence(INT nButtonIndex)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8283
    UTIL_ASSERT(0 <= nButtonIndex && nButtonIndex < CSCREENSTORE_NUM_BOTTOMBUTTONS);

    if (m_cResStore == "") {
        return 0;
    }

    SHORT nSequence;
    switch (m_adwButtonPanelId[nButtonIndex]) {
    case -1:
        nSequence = 0;
        break;
    case 2:
        nSequence = 0;
        break;
    case 4:
        nSequence = 1;
        break;
    case 5:
        nSequence = 3;
        break;
    case 7:
        nSequence = 6;
        break;
    case 8:
        nSequence = 5;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8323
        UTIL_ASSERT(FALSE);
    }

    return nSequence;
}

// 0x67DA90
STRREF CScreenStore::GetPanelButtonToolTip(INT nButtonIndex)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8353
    UTIL_ASSERT(0 <= nButtonIndex && nButtonIndex < CSCREENSTORE_NUM_BOTTOMBUTTONS);

    if (m_cResStore == "") {
        return -1;
    }

    STRREF strText;
    switch (m_adwButtonPanelId[nButtonIndex]) {
    case -1:
        strText = -1;
        break;
    case 2:
        strText = 14288; // "Buy and Sell"
        break;
    case 4:
        strText = 14292; // "Identify"
        break;
    case 5:
        strText = 12138; // "Healing"
        break;
    case 7:
        strText = 14287; // "Rooms"
        break;
    case 8:
        strText = 14289; // "Drinks"
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8393
        UTIL_ASSERT(FALSE);
    }

    return strText;
}

// 0x67E840
void CScreenStore::CancelEngine()
{
    m_bInCancelEngine = TRUE;

    while (GetTopPopup() != NULL) {
        OnCancelButtonClick();
    }

    m_bInCancelEngine = FALSE;
}

// 0x67DB60
BOOL CScreenStore::IsCharacterInRange(SHORT nPortraitNum)
{
    LONG nSprite;
    CGameSprite* pSprite;
    BYTE rc;
    CGameArea* pSrcArea;
    CGameArea* pDstArea;
    BOOL bDead;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenInventory.cpp
    // __LINE__: 8488
    UTIL_ASSERT(pGame != NULL);

    nSprite = pGame->GetCharacterId(m_cAIProprietor.GetInstance());

    do {
        rc = pGame->GetObjectArray()->GetShare(nSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    pSrcArea = pSprite->GetArea();

    pGame->GetObjectArray()->ReleaseShare(nSprite,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    nSprite = pGame->GetCharacterId(nPortraitNum);

    do {
        rc = pGame->GetObjectArray()->GetShare(nSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    BOOL bResult;
    if (pSprite->InControl()) {
        pDstArea = pSprite->GetArea();
        bDead = pSprite->GetBaseStats()->m_hitPoints <= 0;

        pGame->GetObjectArray()->ReleaseShare(nSprite,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        if (m_pMainPanel != NULL && m_pMainPanel->m_nID == 5) {
            bResult = bDead || pDstArea == pSrcArea;
        } else {
            bResult = pDstArea == pSrcArea;
        }
    } else {
        pGame->GetObjectArray()->ReleaseShare(nSprite,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
        bResult = FALSE;
    }

    return bResult;
}

// NOTE: Inlined.
DWORD CScreenStore::GetRoomCost()
{
    DWORD dwCost;

    switch (m_dwRoomType) {
    case ROOMTYPE_NONE:
        dwCost = 0;
        break;
    case ROOMTYPE_PEASANT:
        dwCost = m_pStore->m_header.m_nRoomCostPeasant;
        break;
    case ROOMTYPE_MERCHANT:
        dwCost = m_pStore->m_header.m_nRoomCostMerchant;
        break;
    case ROOMTYPE_NOBLE:
        dwCost = m_pStore->m_header.m_nRoomCostNoble;
        break;
    case ROOMTYPE_ROYAL:
        dwCost = m_pStore->m_header.m_nRoomCostRoyal;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8602
        UTIL_ASSERT(FALSE);
    }

    return dwCost;
}

// 0x67DD30
void CScreenStore::OnErrorButtonClick(INT nButton)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8632
    UTIL_ASSERT(0 <= nButton && nButton < GetNumErrorButtons());

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8634
    UTIL_ASSERT(pGame != NULL);

    CSingleLock renderLock(&(m_cUIManager.m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    switch (m_nErrorState) {
    case 0:
        switch (nButton) {
        case 0:
            DismissPopup();
            RestParty();
            UpdateMainPanel();
            break;
        case 1:
            DismissPopup();
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 8656
            UTIL_ASSERT(FALSE);
        }
        break;
    case 1:
        switch (nButton) {
        case 0:
            DismissPopup();
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 8669
            UTIL_ASSERT(FALSE);
        }
        break;
    case 2:
        switch (nButton) {
        case 0:
            DismissPopup();
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 8682
            UTIL_ASSERT(FALSE);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8688
        UTIL_ASSERT(FALSE);
    }

    renderLock.Unlock();
}

// 0x67DEA0
void CScreenStore::RestParty()
{
    CString sToken;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8778
    UTIL_ASSERT(pGame != NULL);

    DWORD nPartyGold = pGame->GetGameSave()->m_nPartyGold;

    // NOTE: Uninline.
    DWORD nCost = GetRoomCost();

    DWORD dwRoomType = m_dwRoomType;
    m_dwRoomType = 0;

    if (nCost > nPartyGold) {
        return;
    }

    if (nCost != 0) {
        pGame->AddPartyGold(-static_cast<LONG>(nCost));
    }

    INT nHP;
    switch (dwRoomType) {
    case ROOMTYPE_PEASANT:
        nHP = 1;
        break;
    case ROOMTYPE_MERCHANT:
        nHP = 2;
        break;
    case ROOMTYPE_NOBLE:
        nHP = 3;
        break;
    case ROOMTYPE_ROYAL:
        nHP = 4;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 8824
        UTIL_ASSERT(FALSE);
    }

    sToken.Format("%d", CRuleTables::TIME_RESTING / CTimerWorld::TIMESCALE_MIN_PER_HOUR / CTimerWorld::TIMESCALE_SEC_PER_MIN / CTimerWorld::TIMESCALE_MSEC_PER_SEC);
    g_pBaldurChitin->GetTlkTable().SetToken(CTimerWorld::TOKEN_HOUR, sToken);

    sToken.Format("%d", nHP);
    g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_HP, sToken);

    CUIPanel* pPanel = m_cUIManager.GetPanel(7);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8835
    UTIL_ASSERT(pPanel != NULL);

    CUIControlTextDisplay* pText = static_cast<CUIControlTextDisplay*>(pPanel->GetControl(12));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8837
    UTIL_ASSERT(pText != NULL);

    pText->RemoveAll();

    UpdateText(pText,
        "%s",
        FetchString(16476));

    CInfGame::m_bHealPartyOnRest = TRUE;

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        LONG nCharacterId = GetCustomer().GetInstance();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->InControl()) {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                pGame->RestParty(nHP, TRUE);
            } else {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    } else {
        pGame->RestParty(nHP, TRUE);
    }

    CInfGame::m_bHealPartyOnRest = FALSE;
}

// 0x67E2A0
BOOL CScreenStore::IsCharacterViewable(SHORT nPortraitNum)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    BOOL bViewable = FALSE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 8998
    UTIL_ASSERT(pGame != NULL);

    // NOTE: Uninline.
    LONG nCharacterId = pGame->GetCharacterId(m_nSelectedCharacter);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Uninline.
        bViewable = pSprite->InControl();

        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    return bViewable;
}

// 0x67E380
void CScreenStore::FocusChatEditBox()
{
    if (g_pBaldurChitin->cNetwork.GetSessionOpen()
        && g_pBaldurChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        if (GetTopPopup() == NULL) {
            CUIPanel* pPanel = m_cUIManager.GetPanel(15);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 9055
            UTIL_ASSERT(pPanel != NULL);

            CUIControlEdit* pEdit = static_cast<CUIControlEdit*>(pPanel->GetControl(8));

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 9057
            UTIL_ASSERT(pEdit != NULL);

            m_cUIManager.SetCapture(pEdit, CUIManager::KEYBOARD);
        }
    }
}

// 0x67E420
void CScreenStore::GetChatEditBoxStatus(CString& sChatText, BOOL& bInputCapture)
{
    if (m_pButtonBar->m_nID == 15) {
        CUIControlEdit* pEdit = static_cast<CUIControlEdit*>(m_pButtonBar->GetControl(8));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 9095
        UTIL_ASSERT(pEdit != NULL);

        sChatText = pEdit->GetText();
        bInputCapture = m_cUIManager.m_pFocusedControl == pEdit;
    } else {
        sChatText = "";
        bInputCapture = FALSE;
    }
}

// 0x67E4F0
void CScreenStore::SetChatEditBoxStatus(const CString& sChatText, BOOL bInputCapture)
{
    if (m_pButtonBar->m_nID == 15) {
        CUIControlEdit* pEdit = static_cast<CUIControlEdit*>(m_pButtonBar->GetControl(8));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 9133
        UTIL_ASSERT(pEdit != NULL);

        pEdit->SetText(sChatText);

        if (bInputCapture) {
            m_cUIManager.SetCapture(pEdit, CUIManager::KEYBOARD);
        }
    }
}

// 0x67E560
BOOL CScreenStore::CheckSpellUsable(INT nPortraitNum, const CResRef& cResSpell)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9165
    UTIL_ASSERT(pGame != NULL);

    BOOL bUsable = FALSE;

    LONG nCharacterId = CGameObjectArray::INVALID_INDEX;
    if (static_cast<SHORT>(nPortraitNum) < pGame->m_nCharacters) {
        nCharacterId = pGame->m_characterPortraits[static_cast<SHORT>(nPortraitNum)];
    }

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

    // The temple services are hard-coded by resref: each one is offered only
    // while the selected character is in the state it cures.
    if (cResSpell == CResRef("SPPR101")           // Bless
        || cResSpell == CResRef("SPPR103")) {     // Cure Light Wounds
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0;
    } else if (cResSpell == CResRef("SPPR303")) { // Dispel Magic
        if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
            POSITION pos = pSprite->m_timedEffectList.GetHeadPosition();
            while (pos != NULL) {
                CGameEffect* pEffect = pSprite->m_timedEffectList.GetNext(pos);
                if ((pEffect->m_effectAmount2 & 0x1) != 0) {
                    bUsable = TRUE;
                    break;
                }
            }

            if (!bUsable) {
                pos = pSprite->m_equipedEffectList.GetHeadPosition();
                while (pos != NULL) {
                    CGameEffect* pEffect = pSprite->m_equipedEffectList.GetNext(pos);
                    if ((pEffect->m_effectAmount2 & 0x1) != 0) {
                        bUsable = TRUE;
                        break;
                    }
                }
            }
        }
    } else if (cResSpell == CResRef("SPPR307")) { // Remove Curse
        if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
            for (INT nSlot = 0; nSlot < CGameSpriteEquipment::NUM_SLOT; nSlot++) {
                if (pSprite->m_equipment.m_items[nSlot] != NULL
                    && (pSprite->m_equipment.m_items[nSlot]->GetFlagsFile() & 0x10) != 0) {
                    bUsable = TRUE;
                    break;
                }
            }
        }
    } else if (cResSpell == CResRef("SPPR308")) { // Remove Paralysis
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0
            && (pSprite->m_derivedStats.m_generalState & STATE_STUNNED) != 0;
    } else if (cResSpell == CResRef("SPPR214")            // Cure Moderate Wounds
        || cResSpell == CResRef("SPPR401")                // Cure Serious Wounds
        || cResSpell == CResRef("SPWI407")) {             // Summon Monster IV
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0;
    } else if (cResSpell == CResRef("SPPR314")) { // Remove Disease
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0
            && (pSprite->m_derivedStats.m_generalState & STATE_DISEASED) != 0;
    } else if (cResSpell == CResRef("SPPR404")) { // Neutralize Poison
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0
            && (pSprite->m_derivedStats.m_generalState & STATE_POISONED) != 0;
    } else if (cResSpell == CResRef("SPPR212")) { // Delay Poison
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0
            && (pSprite->m_derivedStats.m_generalState & STATE_POISONED) != 0;
    } else if (cResSpell == CResRef("SPPR504")) { // Raise Dead
        bUsable = pSprite->m_derivedStats.m_generalState & STATE_DEAD;

        // Elves cannot be raised, only resurrected.
        if (pSprite->m_liveTypeAI.m_nRace == 2) {
            bUsable = FALSE;
        }
    } else if (cResSpell == CResRef("SPPR712")) { // Resurrection
        bUsable = pSprite->m_derivedStats.m_generalState & STATE_DEAD;
    } else {
        bUsable = (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0;
    }

    pGame->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bUsable;
}

// 0x67E870
void CScreenStore::UpdatePartyGoldStatus()
{
    if (GetTopPopup() == NULL) {
        UpdateMainPanel();
    }
}

// 0x684A20
void CScreenStore::OpenBag(const CResRef& resRef)
{
    m_pBag = new CStore();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13588
    UTIL_ASSERT(m_pBag);

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->DemandServerStore(resRef, TRUE);
            m_pBag->SetResRef(resRef);
        } else {
            m_pBag->SetResRef(resRef);

            if (m_pBag->m_bLocalCopy && memcmp(m_pBag->m_pVersion, "STORV9.0", 8) == 0) {
                CMessageStoreDemand* pMessage = new CMessageStoreDemand(resRef,
                    CGameObjectArray::INVALID_INDEX,
                    CGameObjectArray::INVALID_INDEX);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            } else {
                BOOL bResult = g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(resRef.GetResRefStr(),
                    1014,
                    TRUE,
                    TRUE,
                    TRUE);
                if (!bResult) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return;
                }

                m_pBag->SetResRef(resRef);
            }
        }
    } else {
        m_pBag->SetResRef(resRef);
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13623
    UTIL_ASSERT_MSG(m_pBag->GetType() == CSTOREFILEHEADER_STORETYPE_BAG,
        (LPCSTR)(resRef.GetResRefStr() + " store file is not type BAG"));
}

// 0x684CD0
void CScreenStore::CloseBag(BOOL bSaveFile)
{
    if (g_pChitin->cNetwork.GetSessionOpen()) {
        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(m_pBag->m_resRef);
        } else {
            CMessageStoreRelease* message = new CMessageStoreRelease(m_pBag->m_resRef,
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    } else {
        if (bSaveFile) {
            m_pBag->Marshal(g_pBaldurChitin->GetObjectGame()->m_sTempDir);
        }
    }

    delete m_pBag;
    m_pBag = NULL;
}

// 0x67E890
void CScreenStore::UpdateRequesterPanel()
{
    CScreenStoreItem cItem;

    if (field_14E6 == 0) {
        GetGroupItem(field_14E2, cItem);
    } else {
        GetStoreItem(field_14E2, cItem);
    }

    CUIPanel* pPanel = m_cUIManager.GetPanel(20);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9423
    UTIL_ASSERT(pPanel != NULL);

    CUIControlButtonStoreRequesterItem* pButton = static_cast<CUIControlButtonStoreRequesterItem*>(pPanel->GetControl(0));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9425
    UTIL_ASSERT(pButton != NULL);

    CUIControlEdit* pEdit = static_cast<CUIControlEdit*>(pPanel->GetControl(6));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9427
    UTIL_ASSERT(pButton != NULL);

    field_14DE = min(field_14DE, cItem.m_nMaxCount);

    char szCount[4];
    sprintf(szCount, "%d", field_14DE);

    pEdit->SetText(CString(szCount));
    pEdit->InvalidateRect();

    if (cItem.m_pItem != NULL) {
        pButton->m_resRef = cItem.m_pItem->GetItemIcon();
        pButton->InvalidateRect();
    }
}

// 0x6849B0
DWORD CScreenStore::GetDefaultMainPanel()
{
    switch (m_pStore->m_header.m_nStoreType) {
    case 0:
    case 4:
        return 2;
    case 1:
        return 8;
    case 2:
        return 7;
    case 3:
        return 5;
    case 5:
        return -1;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 13563
        UTIL_ASSERT(FALSE);
    }
}

// 0x67E250
CAIObjectType CScreenStore::GetCustomer()
{
    return m_cAICustomer;
}

// NOTE: Inlined.
void CScreenStore::SelectGroupItem(INT nIndex, BOOL bSelected)
{
    if (nIndex >= 0 && nIndex < m_lGroupItems.GetCount()) {
        CScreenStoreItem* pItem = m_lGroupItems.GetAt(m_lGroupItems.FindIndex(nIndex));
        pItem->m_bSelected = bSelected;
        if (!bSelected) {
            pItem->m_nCount = 1;
            pItem->m_nValue = pItem->m_nSingleValue;
        }
    }
}

// NOTE: Inlined.
void CScreenStore::SelectIdentifyItem(INT nIndex, BOOL bSelected)
{
    if (nIndex >= 0 && nIndex < m_lIdentifyItems.GetCount()) {
        CScreenStoreItem* pItem = m_lIdentifyItems.GetAt(m_lIdentifyItems.FindIndex(nIndex));
        pItem->m_bSelected = bSelected;
        if (!bSelected) {
            pItem->m_nCount = 1;
            pItem->m_nValue = pItem->m_nSingleValue;
        }
    }
}

// NOTE: Inlined.
void CScreenStore::SelectStoreItem(INT nIndex, BOOL bSelected)
{
    if (nIndex >= 0 && nIndex < m_lStoreItems.GetCount()) {
        CScreenStoreItem* pItem = m_lStoreItems.GetAt(m_lStoreItems.FindIndex(nIndex));
        pItem->m_bSelected = bSelected;
        if (!bSelected) {
            pItem->m_nCount = 1;
            pItem->m_nValue = pItem->m_nSingleValue;
        }
    }
}

// NOTE: Inlined.
void CScreenStore::SelectSpellItem(INT nIndex, BOOL bSelected)
{
    // NOTE: Uninline.
    SelectAllSpellItems(FALSE);

    if (nIndex >= 0 && nIndex < m_lSpellItems.GetCount()) {
        CScreenStoreItem* pItem = m_lSpellItems.GetAt(m_lSpellItems.FindIndex(nIndex));
        pItem->m_bSelected = TRUE;
    }
}

// NOTE: Inlined.
void CScreenStore::SelectAllGroupItems(BOOL bSelected)
{
    POSITION pos = m_lGroupItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lGroupItems.GetNext(pos);
        pItem->m_bSelected = bSelected;
    }
}

// NOTE: Inlined.
void CScreenStore::SelectAllIdentifyItems(BOOL bSelected)
{
    POSITION pos = m_lIdentifyItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lIdentifyItems.GetNext(pos);
        pItem->m_bSelected = bSelected;
    }
}

// NOTE: Inlined.
void CScreenStore::SelectAllStoreItems(BOOL bSelected)
{
    POSITION pos = m_lStoreItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lStoreItems.GetNext(pos);
        pItem->m_bSelected = bSelected;
    }
}

// NOTE: Inlined.
void CScreenStore::SelectAllSpellItems(BOOL bSelected)
{
    POSITION pos = m_lSpellItems.GetHeadPosition();
    while (pos != NULL) {
        CScreenStoreItem* pItem = m_lSpellItems.GetNext(pos);
        pItem->m_bSelected = bSelected;
    }
}

// NOTE: Inlined.
void CScreenStore::SetGroupItemCount(INT nIndex, DWORD nCount)
{
    if (nIndex >= 0 && nIndex < m_lGroupItems.GetCount()) {
        CScreenStoreItem* pItem = m_lGroupItems.GetAt(m_lGroupItems.FindIndex(nIndex));
        pItem->m_nCount = min(nCount, pItem->m_nMaxCount);
        pItem->m_nValue = pItem->m_nCount * pItem->m_nSingleValue;
    }
}

// NOTE: Inlined.
void CScreenStore::SetStoreItemCount(INT nIndex, DWORD nCount)
{
    if (nIndex >= 0 && nIndex < m_lStoreItems.GetCount()) {
        CScreenStoreItem* pItem = m_lStoreItems.GetAt(m_lStoreItems.FindIndex(nIndex));
        pItem->m_nCount = min(nCount, pItem->m_nMaxCount);
        pItem->m_nValue = pItem->m_nCount * pItem->m_nSingleValue;
    }
}

// -----------------------------------------------------------------------------

// 0x67EA20
CUIControlButtonStoreBarDone::CUIControlButtonStoreBarDone(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(11973, strRes);
    SetText(strRes.szText);
}

// 0x67EB10
CUIControlButtonStoreBarDone::~CUIControlButtonStoreBarDone()
{
}

// 0x67EBB0
void CUIControlButtonStoreBarDone::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9575
    UTIL_ASSERT(pStore != NULL);

    if (g_pBaldurChitin->m_pEngineWorld->m_bInControlOfStore) {
        if (pStore->m_pStore->m_header.m_nStoreType == 4) {
            pStore->SelectEngine(g_pBaldurChitin->m_pEngineInventory);
        } else {
            pStore->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
        }

        g_pBaldurChitin->m_pEngineWorld->StopStore();
    }
}

// -----------------------------------------------------------------------------

// 0x67EC40
CUIControlButtonStoreBarPanel::CUIControlButtonStoreBarPanel(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON, 0)
{
    m_nNotSelectedFrame = m_nNormalFrame;
    m_nSelectedFrame = 0;
}

// 0x67ECB0
CUIControlButtonStoreBarPanel::~CUIControlButtonStoreBarPanel()
{
}

// 0x67ED50
void CUIControlButtonStoreBarPanel::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9634
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    // NOTE: Uninline.
    DWORD dwPanelId = pStore->GetPanelButtonPanelId(m_nID - 1);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9641
    UTIL_ASSERT(dwPanelId != MAXDWORD);

    pStore->SwitchMainPanel(dwPanelId);

    renderLock.Unlock();
}

// 0x67EE60
BOOL CUIControlButtonStoreBarPanel::Render(BOOL bForce)
{
    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9681
    UTIL_ASSERT(pStore != NULL);

    m_cVidCell.SequenceSet(pStore->GetPanelButtonSequence(m_nID - 1));

    return CUIControlButton3State::Render(bForce);
}

// -----------------------------------------------------------------------------

// 0x67EEE0
CUIControlButtonStoreBuySellBuy::CUIControlButtonStoreBuySellBuy(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(13703, strRes); // "Buy"
    SetText(strRes.szText);
}

// 0x67EFD0
CUIControlButtonStoreBuySellBuy::~CUIControlButtonStoreBuySellBuy()
{
}

// 0x67F070
void CUIControlButtonStoreBuySellBuy::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9738
    UTIL_ASSERT(pStore != NULL);

    pStore->OnBuyItemButtonClick();
}

// -----------------------------------------------------------------------------

// 0x67F0B0
CUIControlButtonStoreBuySellSell::CUIControlButtonStoreBuySellSell(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(13704, strRes); // "Sell"
    SetText(strRes.szText);
}

// 0x67F1A0
CUIControlButtonStoreBuySellSell::~CUIControlButtonStoreBuySellSell()
{
}

// 0x67F240
void CUIControlButtonStoreBuySellSell::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9794
    UTIL_ASSERT(pStore != NULL);

    pStore->OnSellItemButtonClick();
}

// -----------------------------------------------------------------------------

// 0x67F280
CUIControlButtonStoreIdentifyIdentify::CUIControlButtonStoreIdentifyIdentify(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(14133, strRes); // "Identify"
    SetText(strRes.szText);
}

// 0x67F370
CUIControlButtonStoreIdentifyIdentify::~CUIControlButtonStoreIdentifyIdentify()
{
}

// 0x67F410
void CUIControlButtonStoreIdentifyIdentify::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9850
    UTIL_ASSERT(pStore != NULL);

    pStore->OnIdentifyItemButtonClick();
}

// -----------------------------------------------------------------------------

// 0x67F450
CUIControlButtonStoreBuySpellBuy::CUIControlButtonStoreBuySpellBuy(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(13703, strRes); // "Buy"
    SetText(strRes.szText);
}

// 0x67F540
CUIControlButtonStoreBuySpellBuy::~CUIControlButtonStoreBuySpellBuy()
{
}

// 0x67F5E0
void CUIControlButtonStoreBuySpellBuy::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 9963
    UTIL_ASSERT(pStore != NULL);

    pStore->OnBuySpellButtonClick();
}

// -----------------------------------------------------------------------------

// 0x67F620
CUIControlScrollBarStoreBuyDrinksDrink::CUIControlScrollBarStoreBuyDrinksDrink(CUIPanel* panel, UI_CONTROL_SCROLLBAR* controlInfo)
    : CUIControlScrollBar(panel, controlInfo)
{
}

// 0x632C00
CUIControlScrollBarStoreBuyDrinksDrink::~CUIControlScrollBarStoreBuyDrinksDrink()
{
}

// NOTE: Inlined.
void CUIControlScrollBarStoreBuyDrinksDrink::UpdateScrollBar()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10015
    UTIL_ASSERT(pStore != NULL);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10017
    UTIL_ASSERT(pGame != NULL);

    AdjustScrollBar(pStore->m_nTopDrinkItem, pStore->GetNumDrinkItems(), 8);
}

// 0x67F660
void CUIControlScrollBarStoreBuyDrinksDrink::OnScrollUp()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10047
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopDrinkItem = max(pStore->m_nTopDrinkItem - 1, 0);
    if (nNewTopDrinkItem != pStore->m_nTopDrinkItem) {
        // NOTE: Uninline.
        pStore->SetTopDrinkItem(nNewTopDrinkItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67F760
void CUIControlScrollBarStoreBuyDrinksDrink::OnScrollDown()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10083
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopDrinkItem = max(min(pStore->m_nTopDrinkItem + 1, pStore->GetNumDrinkItems() - 8), 0);
    if (nNewTopDrinkItem != pStore->m_nTopDrinkItem) {
        // NOTE: Uninline.
        pStore->SetTopDrinkItem(nNewTopDrinkItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67F870
void CUIControlScrollBarStoreBuyDrinksDrink::OnPageUp(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10119
    UTIL_ASSERT(pStore != NULL);

    INT nStep = min(nLines, 7);
    INT nNewTopDrinkItem = max(pStore->m_nTopDrinkItem - nStep, 0);
    if (nNewTopDrinkItem != pStore->m_nTopDrinkItem) {
        // NOTE: Uninline.
        pStore->SetTopDrinkItem(nNewTopDrinkItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67F980
void CUIControlScrollBarStoreBuyDrinksDrink::OnPageDown(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10159
    UTIL_ASSERT(pStore != NULL);

    INT nStep = nStep = min(nLines, 7);
    INT nNewTopDrinkItem = max(min(pStore->m_nTopDrinkItem + nStep, pStore->GetNumDrinkItems() - 8), 0);
    if (nNewTopDrinkItem != pStore->m_nTopDrinkItem) {
        // NOTE: Uninline.
        pStore->SetTopDrinkItem(nNewTopDrinkItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67FAA0
void CUIControlScrollBarStoreBuyDrinksDrink::OnScroll()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10199
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->SetTopDrinkItem(max(field_144 * (pStore->GetNumDrinkItems() - 8), 0) / field_142);

    InvalidateItems();

    // NOTE: Uninline.
    UpdateScrollBar();
}

// 0x67FBA0
void CUIControlScrollBarStoreBuyDrinksDrink::InvalidateItems()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10231
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    pStore->UpdateMainPanel();

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x67FC40
CUIControlScrollBarStoreStore::CUIControlScrollBarStoreStore(CUIPanel* panel, UI_CONTROL_SCROLLBAR* controlInfo)
    : CUIControlScrollBar(panel, controlInfo)
{
}

// 0x632C00
CUIControlScrollBarStoreStore::~CUIControlScrollBarStoreStore()
{
}

// NOTE: Inlined.
void CUIControlScrollBarStoreStore::UpdateScrollBar()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10291
    UTIL_ASSERT(pStore != NULL);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10293
    UTIL_ASSERT(pGame != NULL);

    AdjustScrollBar(pStore->m_nTopStoreItem, pStore->GetNumStoreItems(), 6);
}

// 0x67FC60
void CUIControlScrollBarStoreStore::OnScrollUp()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10323
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopStoreItem = max(pStore->m_nTopStoreItem - 1, 0);
    if (nNewTopStoreItem != pStore->m_nTopStoreItem) {
        // NOTE: Uninline.
        pStore->SetTopStoreItem(nNewTopStoreItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67FD50
void CUIControlScrollBarStoreStore::OnScrollDown()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10359
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopStoreItem = max(min(pStore->m_nTopStoreItem + 1, pStore->GetNumStoreItems() - 6), 0);
    if (nNewTopStoreItem != pStore->m_nTopStoreItem) {
        // NOTE: Uninline.
        pStore->SetTopStoreItem(nNewTopStoreItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67FE50
void CUIControlScrollBarStoreStore::OnPageUp(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10395
    UTIL_ASSERT(pStore != NULL);

    INT nStep = min(nLines, 5);
    INT nNewTopStoreItem = max(pStore->m_nTopStoreItem - nStep, 0);
    if (nNewTopStoreItem != pStore->m_nTopStoreItem) {
        // NOTE: Uninline.
        pStore->SetTopStoreItem(nNewTopStoreItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x67FF50
void CUIControlScrollBarStoreStore::OnPageDown(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10435
    UTIL_ASSERT(pStore != NULL);

    INT nStep = nStep = min(nLines, 5);
    INT nNewTopStoreItem = max(min(pStore->m_nTopStoreItem + nStep, pStore->GetNumStoreItems() - 6), 0);
    if (nNewTopStoreItem != pStore->m_nTopStoreItem) {
        // NOTE: Uninline.
        pStore->SetTopStoreItem(nNewTopStoreItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680070
void CUIControlScrollBarStoreStore::OnScroll()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10475
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->SetTopStoreItem(max(field_144 * (pStore->GetNumStoreItems() - 6), 0) / field_142);

    InvalidateItems();

    // NOTE: Uninline.
    UpdateScrollBar();
}

// 0x680160
void CUIControlScrollBarStoreStore::InvalidateItems()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10507
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    pStore->UpdateMainPanel();

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x680200
CUIControlScrollBarStoreIdentify::CUIControlScrollBarStoreIdentify(CUIPanel* panel, UI_CONTROL_SCROLLBAR* controlInfo)
    : CUIControlScrollBar(panel, controlInfo)
{
}

// 0x632C00
CUIControlScrollBarStoreIdentify::~CUIControlScrollBarStoreIdentify()
{
}

// NOTE: Inlined.
void CUIControlScrollBarStoreIdentify::UpdateScrollBar()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10567
    UTIL_ASSERT(pStore != NULL);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10569
    UTIL_ASSERT(pGame != NULL);

    AdjustScrollBar(pStore->m_nTopIdentifyItem, pStore->GetNumIdentifyItems(), 6);
}

// 0x680220
void CUIControlScrollBarStoreIdentify::OnScrollUp()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10599
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopIdentifyItem = max(pStore->m_nTopIdentifyItem - 1, 0);
    if (nNewTopIdentifyItem != pStore->m_nTopIdentifyItem) {
        // NOTE: Uninline.
        pStore->SetTopIdentifyItem(nNewTopIdentifyItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680310
void CUIControlScrollBarStoreIdentify::OnScrollDown()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10635
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopIdentifyItem = max(min(pStore->m_nTopIdentifyItem + 1, pStore->GetNumIdentifyItems() - 6), 0);
    if (nNewTopIdentifyItem != pStore->m_nTopIdentifyItem) {
        // NOTE: Uninline.
        pStore->SetTopIdentifyItem(nNewTopIdentifyItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680410
void CUIControlScrollBarStoreIdentify::OnPageUp(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10671
    UTIL_ASSERT(pStore != NULL);

    INT nStep = min(nLines, 5);
    INT nNewTopIdentifyItem = max(pStore->m_nTopIdentifyItem - nStep, 0);
    if (nNewTopIdentifyItem != pStore->m_nTopIdentifyItem) {
        // NOTE: Uninline.
        pStore->SetTopIdentifyItem(nNewTopIdentifyItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680510
void CUIControlScrollBarStoreIdentify::OnPageDown(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10711
    UTIL_ASSERT(pStore != NULL);

    INT nStep = nStep = min(nLines, 5);
    INT nNewTopIdentifyItem = max(min(pStore->m_nTopIdentifyItem + nStep, pStore->GetNumIdentifyItems() - 6), 0);
    if (nNewTopIdentifyItem != pStore->m_nTopIdentifyItem) {
        // NOTE: Uninline.
        pStore->SetTopIdentifyItem(nNewTopIdentifyItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680630
void CUIControlScrollBarStoreIdentify::OnScroll()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10751
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->SetTopIdentifyItem(max(field_144 * (pStore->GetNumIdentifyItems() - 6), 0) / field_142);

    InvalidateItems();

    // NOTE: Uninline.
    UpdateScrollBar();
}

// 0x680720
void CUIControlScrollBarStoreIdentify::InvalidateItems()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10783
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    pStore->UpdateMainPanel();

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x6807C0
CUIControlScrollBarStoreGroup::CUIControlScrollBarStoreGroup(CUIPanel* panel, UI_CONTROL_SCROLLBAR* controlInfo)
    : CUIControlScrollBar(panel, controlInfo)
{
}

// 0x632C00
CUIControlScrollBarStoreGroup::~CUIControlScrollBarStoreGroup()
{
}

// NOTE: Inlined.
void CUIControlScrollBarStoreGroup::UpdateScrollBar()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10844
    UTIL_ASSERT(pStore != NULL);

    AdjustScrollBar(pStore->m_nTopGroupItem, pStore->GetNumGroupItems(), 6);
}

// 0x6807E0
void CUIControlScrollBarStoreGroup::OnScrollUp()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10873
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopGroupItem = max(pStore->m_nTopGroupItem - 1, 0);
    if (nNewTopGroupItem != pStore->m_nTopGroupItem) {
        // NOTE: Uninline.
        pStore->SetTopGroupItem(nNewTopGroupItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x6808A0
void CUIControlScrollBarStoreGroup::OnScrollDown()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10909
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopGroupItem = max(min(pStore->m_nTopGroupItem + 1, pStore->GetNumGroupItems() - 6), 0);
    if (nNewTopGroupItem != pStore->m_nTopGroupItem) {
        // NOTE: Uninline.
        pStore->SetTopGroupItem(nNewTopGroupItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680980
void CUIControlScrollBarStoreGroup::OnPageUp(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10945
    UTIL_ASSERT(pStore != NULL);

    INT nStep = min(nLines, 5);
    INT nNewTopGroupItem = max(pStore->m_nTopGroupItem - nStep, 0);
    if (nNewTopGroupItem != pStore->m_nTopGroupItem) {
        // NOTE: Uninline.
        pStore->SetTopGroupItem(nNewTopGroupItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680A50
void CUIControlScrollBarStoreGroup::OnPageDown(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 10985
    UTIL_ASSERT(pStore != NULL);

    INT nStep = nStep = min(nLines, 5);
    INT nNewTopGroupItem = max(min(pStore->m_nTopGroupItem + nStep, pStore->GetNumGroupItems() - 6), 0);
    if (nNewTopGroupItem != pStore->m_nTopGroupItem) {
        // NOTE: Uninline.
        pStore->SetTopGroupItem(nNewTopGroupItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680B40
void CUIControlScrollBarStoreGroup::OnScroll()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11025
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->SetTopGroupItem(max(field_144 * (pStore->GetNumGroupItems() - 6), 0) / field_142);

    InvalidateItems();

    // NOTE: Uninline.
    UpdateScrollBar();
}

// 0x680C10
void CUIControlScrollBarStoreGroup::InvalidateItems()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11057
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    pStore->UpdateMainPanel();

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x680CB0
CUIControlScrollBarStoreSpell::CUIControlScrollBarStoreSpell(CUIPanel* panel, UI_CONTROL_SCROLLBAR* controlInfo)
    : CUIControlScrollBar(panel, controlInfo)
{
}

// 0x632C00
CUIControlScrollBarStoreSpell::~CUIControlScrollBarStoreSpell()
{
}

// NOTE: Inlined.
void CUIControlScrollBarStoreSpell::UpdateScrollBar()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11118
    UTIL_ASSERT(pStore != NULL);

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11120
    UTIL_ASSERT(pGame != NULL);

    AdjustScrollBar(pStore->m_nTopSpellItem, pStore->GetNumSpellItems(), 6);
}

// 0x680CD0
void CUIControlScrollBarStoreSpell::OnScrollUp()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11150
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopSpellItem = max(pStore->m_nTopSpellItem - 1, 0);
    if (nNewTopSpellItem != pStore->m_nTopSpellItem) {
        // NOTE: Uninline.
        pStore->SetTopSpellItem(nNewTopSpellItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680DC0
void CUIControlScrollBarStoreSpell::OnScrollDown()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11186
    UTIL_ASSERT(pStore != NULL);

    INT nNewTopSpellItem = max(min(pStore->m_nTopSpellItem + 1, pStore->GetNumSpellItems() - 6), 0);
    if (nNewTopSpellItem != pStore->m_nTopSpellItem) {
        // NOTE: Uninline.
        pStore->SetTopSpellItem(nNewTopSpellItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680EC0
void CUIControlScrollBarStoreSpell::OnPageUp(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11222
    UTIL_ASSERT(pStore != NULL);

    INT nStep = min(nLines, 5);
    INT nNewTopSpellItem = max(pStore->m_nTopSpellItem - nStep, 0);
    if (nNewTopSpellItem != pStore->m_nTopSpellItem) {
        // NOTE: Uninline.
        pStore->SetTopSpellItem(nNewTopSpellItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x680FC0
void CUIControlScrollBarStoreSpell::OnPageDown(DWORD nLines)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11262
    UTIL_ASSERT(pStore != NULL);

    INT nStep = nStep = min(nLines, 5);
    INT nNewTopSpellItem = max(min(pStore->m_nTopSpellItem + nStep, pStore->GetNumSpellItems() - 6), 0);
    if (nNewTopSpellItem != pStore->m_nTopSpellItem) {
        // NOTE: Uninline.
        pStore->SetTopSpellItem(nNewTopSpellItem);

        InvalidateItems();

        // NOTE: Uninline.
        UpdateScrollBar();
    }
}

// 0x6810E0
void CUIControlScrollBarStoreSpell::OnScroll()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11302
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->SetTopSpellItem(max(field_144 * (pStore->GetNumSpellItems() - 6), 0) / field_142);

    InvalidateItems();

    // NOTE: Uninline.
    UpdateScrollBar();
}

// 0x6811D0
void CUIControlScrollBarStoreSpell::InvalidateItems()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11334
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    pStore->UpdateMainPanel();

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x681270
CUIControlButtonStoreStoreItem::CUIControlButtonStoreStoreItem(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON | RBUTTON, 0)
{
    m_nSelectedFrame = 0;
}

// 0x681310
CUIControlButtonStoreStoreItem::~CUIControlButtonStoreStoreItem()
{
}

// 0x6813C0
void CUIControlButtonStoreStoreItem::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11431
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cItem;
    INT nIndex;

    switch (m_pPanel->m_nID) {
    case 2:
        nIndex = pStore->m_nTopStoreItem + m_nID - 5;
        pStore->GetStoreItem(nIndex, cItem);

        // NOTE: Uninline.
        pStore->SelectStoreItem(nIndex, !cItem.m_bSelected);

        pStore->UpdateStoreCost();
        pStore->UpdateMainPanel();
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11455
        UTIL_ASSERT(FALSE);
    }
}

// 0x681500
void CUIControlButtonStoreStoreItem::OnLButtonDoubleClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11479
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cItem;
    INT nIndex;

    switch (m_pPanel->m_nID) {
    case 2:
        nIndex = pStore->m_nTopStoreItem + m_nID - 5;
        pStore->GetStoreItem(nIndex, cItem);

        // NOTE: Original code is slightly different. There is only one
        // `CSingleLock`.
        if (pStore->m_pStore->m_header.m_nStoreType == 4) {
            if (cItem.m_pItem->GetMaxStackable() > 1
                && cItem.m_pItem->GetUsageCount(0) > 1) {
                pStore->field_14DE = cItem.m_pItem->GetUsageCount(0);
                pStore->field_14E2 = nIndex;
                pStore->field_14E6 = 1;

                CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
                renderLock.Lock(INFINITE);

                pStore->SummonPopup(20);

                renderLock.Unlock();
            }
        } else {
            if (cItem.m_nStoreCount != -1) {
                CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
                renderLock.Lock(INFINITE);

                pStore->SummonPopup(20);

                renderLock.Unlock();
            }
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11523
        UTIL_ASSERT(FALSE);
    }
}

// 0x681690
void CUIControlButtonStoreStoreItem::OnRButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11551
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    INT nIndex;
    POSITION pos;

    switch (m_pPanel->m_nID) {
    case 2:
        nIndex = pStore->m_nTopStoreItem + m_nID - 5;
        pos = pStore->m_lStoreItems.FindIndex(nIndex);
        if (pos != NULL) {
            pStore->field_5A4 = pStore->m_lStoreItems.GetAt(pos)->m_pItem;
            pStore->field_5A8 = 0;
            pStore->SummonPopup(12);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11570
        UTIL_ASSERT(FALSE);
    }

    renderLock.Unlock();
}

// 0x6817A0
BOOL CUIControlButtonStoreStoreItem::Render(BOOL bForce)
{
    CResRef cResIcon;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (!CUIControlButton3State::Render(bForce)) {
        return FALSE;
    }

    if (m_item.GetResRef() == "") {
        return TRUE;
    }

    cResIcon = m_item.GetItemIcon();

    INT nCount = m_item.GetUsageCount(0);
    if (m_item.GetMaxStackable() < 2) {
        nCount = 0;
    }

    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11666
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cStoreItem;
    pStore->GetStoreItem(m_nID + pStore->m_nTopStoreItem - 5, cStoreItem);

    INT nSelectedCount = cStoreItem.m_nCount;
    if (static_cast<WORD>(cStoreItem.m_nCount) < 2) {
        nSelectedCount = 0;
    }

    CRect rFrame(m_pPanel->m_ptOrigin.x + m_ptOrigin.x,
        m_pPanel->m_ptOrigin.y + m_ptOrigin.y,
        m_pPanel->m_ptOrigin.x + m_ptOrigin.x + m_size.cx,
        m_pPanel->m_ptOrigin.y + m_ptOrigin.y + m_size.cy);

    if (m_bPressed) {
        rFrame.OffsetRect(field_63E, field_642);
    }

    CPoint ptIcon(rFrame.left, rFrame.top);

    CRect rClip;
    rClip.IntersectRect(rFrame, m_rDirty);

    DWORD dwFlags = 1;
    if (m_bEnabled) {
        dwFlags = 0;
    }

    CIcon::RenderIcon(0, ptIcon, m_size, rClip, cResIcon,
        m_pPanel->m_pManager->m_bDoubleSize,
        dwFlags,
        static_cast<WORD>(nCount),
        FALSE,
        static_cast<WORD>(nSelectedCount),
        FALSE,
        0);

    if ((m_item.m_flags & 0x1) == 0) {
        // Unidentified.
        CVidCell vcOverlay(CResRef("STORTIN2"), m_pPanel->m_pManager->m_bDoubleSize);
        vcOverlay.Render(0,
            m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
            m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
            rClip, NULL, 0, 2, 0xC0);
    } else if (!m_bValid) {
        CVidCell vcOverlay(CResRef("STORTINT"), m_pPanel->m_pManager->m_bDoubleSize);
        vcOverlay.Render(0,
            m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
            m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
            rClip, NULL, 0, 2, 0xC0);
    } else {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11719
        UTIL_ASSERT(pGame != NULL);

        // NOTE: Uninline.
        LONG nCharacterId = pGame->GetCharacterId(pStore->GetSelectedCharacter());

        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                INT nUsable = pGame->CheckItemUsable(pSprite, &m_item);

                if (nUsable == 2) {
                    CVidCell vcOverlay(CResRef("STORTIN4"), m_pPanel->m_pManager->m_bDoubleSize);
                    vcOverlay.Render(0,
                        m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
                        m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
                        rClip, NULL, 0, 2, 0xC0);
                } else if (nUsable == 0) {
                    CVidCell vcOverlay(CResRef("STORTINT"), m_pPanel->m_pManager->m_bDoubleSize);
                    vcOverlay.Render(0,
                        m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
                        m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
                        rClip, NULL, 0, 2, 0xC0);
                }

                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    if ((m_item.m_flags & 0x8) != 0 && (m_item.m_flags & 0x1) != 0) {
        // Magical.
        CVidCell vcOverlay(CResRef("STORTIN3"), m_pPanel->m_pManager->m_bDoubleSize);
        vcOverlay.Render(0,
            m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
            m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
            rClip, NULL, 0, 2, 0xC0);
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x681D90
CUIControlButtonStoreGroupItem::CUIControlButtonStoreGroupItem(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON | RBUTTON, 0)
{
    m_nSelectedFrame = 0;
}

// 0x681E30
CUIControlButtonStoreGroupItem::~CUIControlButtonStoreGroupItem()
{
}

// 0x681EE0
void CUIControlButtonStoreGroupItem::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11850
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cItem;
    INT nIndex;

    switch (m_pPanel->m_nID) {
    case 2:
        nIndex = pStore->m_nTopGroupItem + m_nID - 13;
        pStore->GetGroupItem(nIndex, cItem);

        if (cItem.m_pItem->GetItemType() == 58) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
            // __LINE__: 11874
            UTIL_ASSERT(pStore->GetBag() == NULL);

            pStore->OpenBag(cItem.m_pItem->GetResRef());
            INT nInventory = pStore->m_pBag->m_lInventory.GetCount();
            pStore->CloseBag(FALSE);

            if (nInventory == 0) {
                // NOTE: Uninline.
                pStore->SelectGroupItem(nIndex, !cItem.m_bSelected);

                // NOTE: Uninline.
                pStore->UpdateGroupCost();

                pStore->UpdateMainPanel();
            }
        } else {
            // NOTE: Uninline.
            pStore->SelectGroupItem(nIndex, !cItem.m_bSelected);

            // NOTE: Uninline.
            pStore->UpdateGroupCost();

            pStore->UpdateMainPanel();
        }
        break;
    case 4:
        nIndex = pStore->m_nTopIdentifyItem + m_nID - 8;
        pStore->GetIdentifyItem(nIndex, cItem);

        // NOTE: Uninline.
        pStore->SelectIdentifyItem(nIndex, !cItem.m_bSelected);

        // NOTE: Uninline.
        pStore->UpdateIdentifyCost();

        pStore->UpdateMainPanel();
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11894
        UTIL_ASSERT(FALSE);
    }
}

// NOTE: Odd location.
//
// 0x685090
void CUIControlButtonStoreGroupItem::OnLButtonDoubleClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13747
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cItem;
    INT nIndex;

    switch (m_pPanel->m_nID) {
    case 2:
        nIndex = pStore->m_nTopGroupItem + m_nID - 13;
        pStore->GetGroupItem(nIndex, cItem);

        if (cItem.m_pItem->GetItemType() == 58
            && pStore->m_pBag == NULL
            && cItem.m_pItem->GetResRef() != pStore->m_pStore->m_resRef) {
            pStore->OpenBag(cItem.m_pItem->GetResRef());

            pStore->UpdateStoreItems();

            // NOTE: Uninline.
            pStore->SetTopStoreItem(0);

            // NOTE: Uninline.
            pStore->UpdateStoreCost();

            pStore->UpdateGroupItems();

            // NOTE: Uninline.
            pStore->SetTopGroupItem(0);

            // NOTE: Uninline.
            pStore->UpdateGroupCost();

            pStore->UpdateMainPanel();
        }
        break;
    case 4:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 13810
        UTIL_ASSERT(FALSE);
    }
}

// 0x6821A0
void CUIControlButtonStoreGroupItem::OnRButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11922
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    INT nIndex;
    POSITION pos;
    CScreenStoreItem* pItem;

    switch (m_pPanel->m_nID) {
    case 2:
        nIndex = pStore->m_nTopGroupItem + m_nID - 13;
        pos = pStore->m_lGroupItems.FindIndex(nIndex);
        if (pos != NULL) {
            pItem = pStore->m_lGroupItems.GetAt(pos);
            pStore->field_5A4 = pItem->m_pItem;
            pStore->field_5A8 = pStore->m_pBag == NULL && pItem->m_pItem->GetItemType() == 58;
            pStore->SummonPopup(12);
        }
        break;
    case 4:
        nIndex = pStore->m_nTopIdentifyItem + m_nID - 8;
        pos = pStore->m_lIdentifyItems.FindIndex(nIndex);
        if (pos != NULL) {
            pItem = pStore->m_lIdentifyItems.GetAt(pos);
            pStore->field_5A4 = pItem->m_pItem;
            pStore->SummonPopup(12);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11957
        UTIL_ASSERT(FALSE);
    }

    renderLock.Unlock();
}

// 0x6822F0
BOOL CUIControlButtonStoreGroupItem::Render(BOOL bForce)
{
    CResRef cResIcon;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (!CUIControlButton3State::Render(bForce)) {
        return FALSE;
    }

    if (m_item.GetResRef() == "") {
        return TRUE;
    }

    cResIcon = m_item.GetItemIcon();

    INT nCount = m_item.GetUsageCount(0);
    if (m_item.GetMaxStackable() < 2) {
        nCount = 0;
    }

    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 11923
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cGroupItem;
    pStore->GetGroupItem(m_nID + pStore->m_nTopGroupItem - 13, cGroupItem);

    INT nSelectedCount = cGroupItem.m_nCount;
    if (static_cast<WORD>(cGroupItem.m_nCount) < 2) {
        nSelectedCount = 0;
    }

    CRect rFrame(m_pPanel->m_ptOrigin.x + m_ptOrigin.x,
        m_pPanel->m_ptOrigin.y + m_ptOrigin.y,
        m_pPanel->m_ptOrigin.x + m_ptOrigin.x + m_size.cx,
        m_pPanel->m_ptOrigin.y + m_ptOrigin.y + m_size.cy);

    if (m_bPressed) {
        rFrame.OffsetRect(field_63E, field_642);
    }

    CPoint ptIcon(rFrame.left, rFrame.top);

    CRect rClip;
    rClip.IntersectRect(rFrame, m_rDirty);

    DWORD dwFlags = 1;
    if (m_bEnabled) {
        dwFlags = 0;
    }

    CIcon::RenderIcon(0, ptIcon, m_size, rClip, cResIcon,
        m_pPanel->m_pManager->m_bDoubleSize,
        dwFlags,
        static_cast<WORD>(nCount),
        FALSE,
        static_cast<WORD>(nSelectedCount),
        FALSE,
        0);

    if ((m_item.m_flags & 0x1) == 0) {
        // Unidentified.
        CVidCell vcOverlay(CResRef("STORTIN2"), m_pPanel->m_pManager->m_bDoubleSize);
        vcOverlay.Render(0,
            m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
            m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
            rClip, NULL, 0, 2, 0xC0);
    } else if (!m_bValid) {
        CVidCell vcOverlay(CResRef("STORTINT"), m_pPanel->m_pManager->m_bDoubleSize);
        vcOverlay.Render(0,
            m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
            m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
            rClip, NULL, 0, 2, 0xC0);
    } else {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 11943
        UTIL_ASSERT(pGame != NULL);

        // NOTE: Uninline.
        LONG nCharacterId = pGame->GetCharacterId(pStore->GetSelectedCharacter());

        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                INT nUsable = pGame->CheckItemUsable(pSprite, &m_item);

                if (nUsable == 2) {
                    CVidCell vcOverlay(CResRef("STORTIN4"), m_pPanel->m_pManager->m_bDoubleSize);
                    vcOverlay.Render(0,
                        m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
                        m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
                        rClip, NULL, 0, 2, 0xC0);
                } else if (nUsable == 0) {
                    CVidCell vcOverlay(CResRef("STORTINT"), m_pPanel->m_pManager->m_bDoubleSize);
                    vcOverlay.Render(0,
                        m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
                        m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
                        rClip, NULL, 0, 2, 0xC0);
                }

                pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    if ((m_item.m_flags & 0x8) != 0 && (m_item.m_flags & 0x1) != 0) {
        // Magical.
        CVidCell vcOverlay(CResRef("STORTIN3"), m_pPanel->m_pManager->m_bDoubleSize);
        vcOverlay.Render(0,
            m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
            m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
            rClip, NULL, 0, 2, 0xC0);
    }

    return TRUE;
}


// -----------------------------------------------------------------------------

// 0x6828E0
CUIControlButtonStoreStoreSpell::CUIControlButtonStoreStoreSpell(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON | RBUTTON, 0)
{
    m_nSelectedFrame = 0;
    m_resRef = "";
}

// 0x682990
CUIControlButtonStoreStoreSpell::~CUIControlButtonStoreStoreSpell()
{
}

// 0x682A30
void CUIControlButtonStoreStoreSpell::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12197
    UTIL_ASSERT(pStore != NULL);

    CString sSpell;
    CString sDescription;
    INT nIndex;
    CUIControlTextDisplay* pText;

    switch (m_pPanel->m_nID) {
    case 5:
        nIndex = pStore->m_nTopSpellItem + m_nID - 8;

        // NOTE: Uninline.
        pStore->SelectSpellItem(nIndex, TRUE);

        // NOTE: Uninline.
        pStore->UpdateSpellCost();

        pStore->UpdateMainPanel();

        pText = static_cast<CUIControlTextDisplay*>(m_pPanel->GetControl(23));

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 12216
        UTIL_ASSERT(pText != NULL);

        pText->RemoveAll();

        m_resRef.CopyToString(sSpell);
        g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpellDescription(sSpell,
            sDescription);

        pText->DisplayString(CString(""),
            sDescription,
            pText->m_rgbLabelColor,
            pText->m_rgbTextColor,
            -1,
            FALSE,
            TRUE);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 12224
        UTIL_ASSERT(FALSE);
    }
}

// 0x682C00
void CUIControlButtonStoreStoreSpell::OnRButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12252
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    INT nIndex;
    POSITION pos;
    CScreenStoreItem* pItem;

    switch (m_pPanel->m_nID) {
    case 5:
        nIndex = pStore->m_nTopSpellItem + m_nID - 8;
        pos = pStore->m_lGroupItems.FindIndex(nIndex);
        if (pos != NULL) {
            pItem = pStore->m_lGroupItems.GetAt(pos);
            pStore->m_cResInfoSpell = pItem->m_cResSpell;
            pStore->SummonPopup(14);
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 12274
        UTIL_ASSERT(FALSE);
    }

    renderLock.Unlock();
}

// 0x682D10
BOOL CUIControlButtonStoreStoreSpell::Render(BOOL bForce)
{
    CResRef cResIcon;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (!CUIControlButton3State::Render(bForce)) {
        return FALSE;
    }

    if (m_resRef != "") {
        CResRef cResSpell = m_resRef;

        CSpell cSpell;
        cSpell.SetResRef(cResSpell, TRUE, TRUE);
        cSpell.Demand();

        BYTE resIcon[8];
        cSpell.GetIcon(resIcon);
        cResIcon = resIcon;

        cSpell.Release();

        // The store slot uses the `B` variant of the spell icon.
        CString sResIcon;
        cResIcon.CopyToString(sResIcon);
        sResIcon.SetAt(sResIcon.GetLength() - 1, 'B');
        cResIcon = sResIcon;

        CRect rFrame(m_pPanel->m_ptOrigin.x + m_ptOrigin.x,
            m_pPanel->m_ptOrigin.y + m_ptOrigin.y,
            m_pPanel->m_ptOrigin.x + m_ptOrigin.x + m_size.cx,
            m_pPanel->m_ptOrigin.y + m_ptOrigin.y + m_size.cy);

        if (m_bPressed) {
            rFrame.OffsetRect(field_63E, field_642);
        }

        CPoint ptIcon(rFrame.left, rFrame.top);

        CRect rClip;
        rClip.IntersectRect(rFrame, m_rDirty);

        DWORD dwFlags = 1;
        if (m_bEnabled) {
            dwFlags = 0;
        }

        CIcon::RenderIcon(0, ptIcon, m_size, rClip, cResIcon,
            m_pPanel->m_pManager->m_bDoubleSize,
            dwFlags,
            0,
            FALSE,
            0,
            FALSE,
            0);

        if (!m_bValid) {
            CVidCell vcOverlay(CResRef("STORTINT"), m_pPanel->m_pManager->m_bDoubleSize);
            vcOverlay.Render(0,
                m_ptOrigin.x + m_pPanel->m_ptOrigin.x,
                m_ptOrigin.y + m_pPanel->m_ptOrigin.y,
                rClip, NULL, 0, 2, 0xC0);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x683080
CUIControlButtonStoreRentRoomRoomSelect::CUIControlButtonStoreRentRoomRoomSelect(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON, 0)
{
    m_nSelectedFrame = 0;
    m_nNotSelectedFrame = 1;
    m_nPressedFrame = 2;
    m_nDisabledFrame = 3;

    STR_RES strRes;

    STRREF strText;
    switch (m_nID) {
    case 4:
        strText = 14294; // "Peasant"
        break;
    case 5:
        strText = 14295; // "Merchant"
        break;
    case 6:
        strText = 14296; // "Noble"
        break;
    case 7:
        strText = 14297; // "Royal"
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 12420
        UTIL_ASSERT(FALSE);
    }

    g_pBaldurChitin->GetTlkTable().Fetch(strText, strRes);
    SetText(strRes.szText);
}

// 0x6831E0
CUIControlButtonStoreRentRoomRoomSelect::~CUIControlButtonStoreRentRoomRoomSelect()
{
}

// 0x683280
void CUIControlButtonStoreRentRoomRoomSelect::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12503
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->m_dwRoomType = GetRoomType();

    pStore->UpdateMainPanel();
}

// NOTE: Inlined.
DWORD CUIControlButtonStoreRentRoomRoomSelect::GetRoomType()
{
    DWORD dwRoomType;

    switch (m_nID) {
    case 4:
        dwRoomType = 1;
        break;
    case 5:
        dwRoomType = 2;
        break;
    case 6:
        dwRoomType = 3;
        break;
    case 7:
        dwRoomType = 4;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 12472
        UTIL_ASSERT(FALSE);
    }

    return dwRoomType;
}

// -----------------------------------------------------------------------------

// 0x683360
CUIControlButtonStoreRentRoomRoomPicture::CUIControlButtonStoreRentRoomRoomPicture(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton3State(panel, controlInfo, LBUTTON, 0)
{
    m_nSelectedFrame = 0;
    m_nNotSelectedFrame = 0;
    m_nPressedFrame = 0;
    m_nDisabledFrame = 1;
}

// 0x6833D0
CUIControlButtonStoreRentRoomRoomPicture::~CUIControlButtonStoreRentRoomRoomPicture()
{
}

// 0x683470
void CUIControlButtonStoreRentRoomRoomPicture::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12612
    UTIL_ASSERT(pStore != NULL);

    // NOTE: Uninline.
    pStore->m_dwRoomType = GetRoomType();

    pStore->UpdateMainPanel();
}

// NOTE: Inlined.
DWORD CUIControlButtonStoreRentRoomRoomPicture::GetRoomType()
{
    DWORD dwRoomType;

    switch (m_nID) {
    case 0:
        dwRoomType = 1;
        break;
    case 1:
        dwRoomType = 2;
        break;
    case 2:
        dwRoomType = 3;
        break;
    case 3:
        dwRoomType = 4;
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 12581
        UTIL_ASSERT(FALSE);
    }

    return dwRoomType;
}

// -----------------------------------------------------------------------------

// 0x683550
CUIControlButtonStoreRentRoomRent::CUIControlButtonStoreRentRoomRent(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(14293, strRes); // "Rent"
    SetText(strRes.szText);
}

// 0x683640
CUIControlButtonStoreRentRoomRent::~CUIControlButtonStoreRentRoomRent()
{
}

// 0x6836E0
void CUIControlButtonStoreRentRoomRent::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12669
    UTIL_ASSERT(pStore != NULL);

    pStore->OnRentRoomButtonClick();
}

// -----------------------------------------------------------------------------

// 0x683720
CUIControlPortraitStore::CUIControlPortraitStore(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
}

// 0x683770
CUIControlPortraitStore::~CUIControlPortraitStore()
{
}

// 0x683810
void CUIControlPortraitStore::OnLButtonClick(CPoint pt)
{
    g_pBaldurChitin->GetActiveEngine()->OnPortraitLClick(m_nID);
}

// 0x683830
BOOL CUIControlPortraitStore::Render(BOOL bForce)
{
    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (!CUIControlButton::Render(bForce)) {
        return FALSE;
    }

    CPoint pt = m_pPanel->m_ptOrigin + m_ptOrigin;
    // NOTE: The binary passes the control's DIRTY rect (`lea [this+0x22]` at the
    // callsite 0x6838C5) as the clip, NOT a full control rect -- cursor-region
    // repaints hand RenderPortrait cursor-sized clip slivers.
    g_pBaldurChitin->GetObjectGame()->RenderPortrait(m_nID,
        pt,
        m_size,
        m_bPressed,
        0,
        0,
        m_rDirty,
        m_pPanel->m_pManager->m_bDoubleSize);

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x6838E0
CUIControlButtonStoreBuyDrinksDrink::CUIControlButtonStoreBuyDrinksDrink(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(14293, strRes); // "Rent"
    SetText(strRes.szText);
}

// 0x683930
CUIControlButtonStoreBuyDrinksDrink::~CUIControlButtonStoreBuyDrinksDrink()
{
}

// 0x6839D0
void CUIControlButtonStoreBuyDrinksDrink::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12811
    UTIL_ASSERT(pStore != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 12817
    UTIL_ASSERT(CRESUI_CONTROLBUTTONID_STORE_BUYDRINK_DRINK0 <= m_nID && m_nID <= CRESUI_CONTROLBUTTONID_STORE_BUYDRINK_DRINK7);

    pStore->OnBuyDrinkButtonClick(m_nID - CRESUI_CONTROLBUTTONID_STORE_BUYDRINK_DRINK0);

    renderLock.Unlock();
}

// -----------------------------------------------------------------------------

// 0x683AA0
CUIControlButtonStorePopupDone::CUIControlButtonStorePopupDone(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(11973, strRes); // "Done"
    SetText(strRes.szText);
}

// 0x683B50
CUIControlButtonStorePopupDone::~CUIControlButtonStorePopupDone()
{
}

// 0x683BF0
void CUIControlButtonStorePopupDone::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13079
    UTIL_ASSERT(pStore != NULL);

    pStore->OnDoneButtonClick();
}

// -----------------------------------------------------------------------------

// 0x683C30
CUIControlButtonStoreError::CUIControlButtonStoreError(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(11973, strRes); // "Done"
    SetText(strRes.szText);
}

// 0x683CE0
CUIControlButtonStoreError::~CUIControlButtonStoreError()
{
}

// 0x683D80
void CUIControlButtonStoreError::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13133
    UTIL_ASSERT(pStore != NULL);

    pStore->OnErrorButtonClick(m_nID);
}

// -----------------------------------------------------------------------------

// 0x683DC0
CUIControlButtonStoreRequesterItem::CUIControlButtonStoreRequesterItem(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 1)
{
    m_resRef = "";
}

// 0x683E60
CUIControlButtonStoreRequesterItem::~CUIControlButtonStoreRequesterItem()
{
}

// 0x683F00
BOOL CUIControlButtonStoreRequesterItem::Render(BOOL bForce)
{
    CVidCell vcIcon;

    if (!m_bActive && !m_bInactiveRender) {
        return FALSE;
    }

    if (m_nRenderCount == 0 && !bForce) {
        return FALSE;
    }

    if (m_nRenderCount != 0) {
        CSingleLock lock(&(m_pPanel->m_pManager->field_56), FALSE);
        lock.Lock(INFINITE);
        m_nRenderCount--;
        lock.Unlock();
    }

    CVidMode* pVidMode = g_pBaldurChitin->GetCurrentVideoMode();

    // NOTE: Uninline.
    vcIcon.SetResRef(m_resRef, m_pPanel->m_pManager->m_bDoubleSize, TRUE, TRUE);

    vcIcon.SequenceSet(1);
    vcIcon.FrameSet(0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13249
    UTIL_ASSERT(vcIcon.GetRes() != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13250
    UTIL_ASSERT(pVidMode != NULL);

    if (vcIcon.pRes == NULL) {
        return FALSE;
    }

    // NOTE: Original code is slightly different.
    INT x = m_pPanel->m_ptOrigin.x + m_ptOrigin.x;
    INT y = m_pPanel->m_ptOrigin.y + m_ptOrigin.y;
    LONG nWidth = CIcon::ICON_SIZE_SM.cx * (m_pPanel->m_pManager->m_bDoubleSize ? 2 : 1);
    LONG nHeight = CIcon::ICON_SIZE_SM.cy * (m_pPanel->m_pManager->m_bDoubleSize ? 2 : 1);

    if (m_size.cx > nWidth) {
        x += (m_size.cx - nWidth) / 2;
    }

    if (m_size.cy > nHeight) {
        y += (m_size.cy - nHeight) / 2;
    }

    CSize iconSize;
    vcIcon.GetCurrentFrameSize(iconSize, FALSE);

    CRect rFrame(x, y, x + nWidth, y + nHeight);

    CRect rClip;
    rClip.IntersectRect(rFrame, m_rDirty);

    vcIcon.Render(0, x, y, rClip, NULL, 0, 0, -1);

    return TRUE;
}

// -----------------------------------------------------------------------------

// 0x684210
CUIControlButtonStoreRequesterDone::CUIControlButtonStoreRequesterDone(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(11973, strRes); // "Done"
    SetText(strRes.szText);
}

// 0x684300
CUIControlButtonStoreRequesterDone::~CUIControlButtonStoreRequesterDone()
{
}

// 0x6843A0
void CUIControlButtonStoreRequesterDone::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13324
    UTIL_ASSERT(pStore != NULL);

    pStore->OnDoneButtonClick();
}

// -----------------------------------------------------------------------------

// 0x6843E0
CUIControlButtonStoreRequesterCancel::CUIControlButtonStoreRequesterCancel(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(13727, strRes); // "Cancel"
    SetText(strRes.szText);
}

// 0x6844D0
CUIControlButtonStoreRequesterCancel::~CUIControlButtonStoreRequesterCancel()
{
}

// 0x684570
void CUIControlButtonStoreRequesterCancel::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13380
    UTIL_ASSERT(pStore != NULL);

    pStore->OnCancelButtonClick();
}

// -----------------------------------------------------------------------------

// 0x6845B0
CUIControlButtonStoreRequesterPlusMinus::CUIControlButtonStoreRequesterPlusMinus(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButtonPlusMinus(panel, controlInfo)
{
}

// 0x6845F0
CUIControlButtonStoreRequesterPlusMinus::~CUIControlButtonStoreRequesterPlusMinus()
{
}

// 0x684690
void CUIControlButtonStoreRequesterPlusMinus::AdjustValue()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13432
    UTIL_ASSERT(pStore != NULL);

    switch (m_nID) {
    case 3:
        if (1) {
            CScreenStoreItem cItem;

            if (pStore->field_14E6 == 0) {
                pStore->GetGroupItem(pStore->field_14E2, cItem);
            } else {
                pStore->GetStoreItem(pStore->field_14E2, cItem);
            }

            // NOTE: Unsigned compare.
            if (pStore->field_14DE < cItem.m_nMaxCount) {
                pStore->field_14DE++;
                pStore->UpdateRequesterPanel();
            }
        }
        break;
    case 4:
        // NOTE: Signed compare.
        if (static_cast<int>(pStore->field_14DE) > 1) {
            pStore->field_14DE--;
            pStore->UpdateRequesterPanel();
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
        // __LINE__: 13445
        UTIL_ASSERT(FALSE);
    }
}

// -----------------------------------------------------------------------------

// 0x6847B0
CUIControlEditStoreRequesterAmount::CUIControlEditStoreRequesterAmount(CUIPanel* panel, UI_CONTROL_EDIT* controlInfo)
    : CUIControlEdit(panel, controlInfo, 0)
{
}

// 0x684800
CUIControlEditStoreRequesterAmount::~CUIControlEditStoreRequesterAmount()
{
}

// 0x6848D0
void CUIControlEditStoreRequesterAmount::KillFocus()
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13498
    UTIL_ASSERT(pStore != NULL);

    CScreenStoreItem cItem;

    if (pStore->field_14E6) {
        pStore->GetStoreItem(pStore->field_14E2, cItem);
    } else {
        pStore->GetGroupItem(pStore->field_14E2, cItem);
    }

    DWORD nValue = atol(m_sText);

    // NOTE: Unsigned compare.
    if (nValue > 0 && nValue <= cItem.m_nMaxCount) {
        pStore->field_14DE = nValue;
    }

    pStore->UpdateRequesterPanel();

    CUIControlEdit::KillFocus();
}

// -----------------------------------------------------------------------------

// 0x684E00
CUIControlButtonStoreCloseBag::CUIControlButtonStoreCloseBag(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(24892, strRes); // "Close Bag"
    SetText(strRes.szText);
}

// 0x684EF0
CUIControlButtonStoreCloseBag::~CUIControlButtonStoreCloseBag()
{
}

// 0x684F90
void CUIControlButtonStoreCloseBag::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13714
    UTIL_ASSERT(pStore != NULL);

    pStore->CloseBag(TRUE);
    pStore->UpdateStoreItems();
    pStore->UpdateGroupItems();

    // NOTE: Uninline.
    pStore->SelectAllGroupItems(FALSE);

    // NOTE: Uninline.
    pStore->SetTopGroupItem(0);

    // NOTE: Uninline.
    pStore->UpdateGroupCost();

    pStore->UpdateMainPanel();

    CRect rControlFrame(m_pPanel->m_ptOrigin + m_ptOrigin, m_size);
    m_pPanel->InvalidateRect(&rControlFrame);

    pStore->CheckEnablePortaits(1);
}

// -----------------------------------------------------------------------------

// 0x685290
CUIControlButtonStoreOpenBag::CUIControlButtonStoreOpenBag(CUIPanel* panel, UI_CONTROL_BUTTON* controlInfo)
    : CUIControlButton(panel, controlInfo, LBUTTON, 0)
{
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(24891, strRes); // "Open Container"
    SetText(strRes.szText);
}

// 0x685340
CUIControlButtonStoreOpenBag::~CUIControlButtonStoreOpenBag()
{
}

// 0x6853E0
void CUIControlButtonStoreOpenBag::OnLButtonClick(CPoint pt)
{
    CScreenStore* pStore = g_pBaldurChitin->m_pEngineStore;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13847
    UTIL_ASSERT(pStore != NULL);

    CItem* pItem = pStore->field_5A4;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfScreenStore.cpp
    // __LINE__: 13851
    UTIL_ASSERT(pItem != NULL);

    CSingleLock renderLock(&(pStore->GetManager()->m_critSect), FALSE);
    renderLock.Lock(INFINITE);

    pStore->OpenBag(pItem->GetResRef());
    pStore->UpdateStoreItems();

    // NOTE: Uninline.
    pStore->SelectAllStoreItems(FALSE);

    // NOTE: Uninline.
    pStore->SetTopStoreItem(0);

    // NOTE: Uninline.
    pStore->UpdateStoreCost();

    pStore->UpdateGroupItems();

    // NOTE: Uninline.
    pStore->SelectAllGroupItems(FALSE);

    // NOTE: Uninline.
    pStore->SetTopGroupItem(0);

    // NOTE: Uninline.
    pStore->UpdateGroupCost();

    pStore->UpdateMainPanel();
    pStore->OnDoneButtonClick();

    renderLock.Unlock();
}
