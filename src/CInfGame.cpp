#include "CInfGame.h"
#include <process.h>

#include "CAIScript.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CBaldurProjector.h"
#include "CCacheStatus.h"
#include "CGameAnimationType.h"
#include "CGameArea.h"
#include "CGameContainer.h"
#include "CGameEffect.h"
#include "CGameObject.h"
#include "CGameSprite.h"
#include "CInfCursor.h"
#include "CItem.h"
#include "CMessage.h"
#include "CPathSearch.h"
#include "CResFile.h"
#include "CScreenChapter.h"
#include "CScreenCharacter.h"
#include "CScreenInventory.h"
#include "CScreenJournal.h"
#include "CScreenLoad.h"
#include "CScreenMap.h"
#include "CScreenMultiPlayer.h"
#include "CScreenOptions.h"
#include "CScreenSave.h"
#include "CScreenSinglePlayer.h"
#include "CScreenSpellbook.h"
#include "CScreenStore.h"
#include "CScreenWorld.h"
#include "CScreenWorldMap.h"
#include "CSearchBitmap.h"
#include "CSpell.h"
#include "CStore.h"
#include "CUIControlBase.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVidInf.h"
#include "Icewind586B70.h"
#include "IcewindMisc.h"

#define FIFTY_THREE 53

// 0x851884
const SHORT CInfGame::KILL_INNOCENT = 0;

// 0x851886
const SHORT CInfGame::INJURE_INNOCENT = 1;

// 0x851888
const SHORT CInfGame::STEAL = 2;

// 0x85188A
const SHORT CInfGame::KILL_FIST = 3;

// 0x85188C
const SHORT CInfGame::MONEY_CHURCH = 4;

// 0x85188E
const SHORT CInfGame::MONEY_BEGGAR = 5;

// 0x851890
const SHORT CInfGame::MAGIC_CHURCH = 6;

// 0x851892
const SHORT CInfGame::KILL_EVIL = 7;

// 0x851894
const SHORT CInfGame::BASE_CHURCH = 8;

// 0x851896
const SHORT CInfGame::REPUTATION_MULTIPLIER = 10;

// 0x8518AC
const DWORD CInfGame::PROGRESSBAR_AREA_ADDITIONAL = 3000000;

// 0x8518B0
const DWORD CInfGame::PROGRESSBAR_GAME_ADDITIONAL = 5000000;

// 0x8518B4
const DWORD CInfGame::PROGRESSBAR_REQUESTS_ADDITIONAL = 5000000;

// 0x8518B8
const DWORD CInfGame::PROGRESSBAR_CACHING_ADDITIONAL = 3000000;

// 0x8518BE
const SHORT CInfGame::FEEDBACK_GOLD = 1;

// 0x8518C0
const SHORT CInfGame::FEEDBACK_EXPERIENCE = 2;

// 0x8518C2
const SHORT CInfGame::FEEDBACK_ITEMGAINED = 3;

// 0x8518C4
const SHORT CInfGame::FEEDBACK_ITEMLOST = 4;

// 0x8518C6
const SHORT CInfGame::FEEDBACK_REPUTATION = 5;

// 0x8518C8
const SHORT CInfGame::FEEDBACK_TIMEPASSEDJOURNEY = 6;

// 0x8518CA
const SHORT CInfGame::FEEDBACK_TIMEPASSEDREST = 7;

// 0x8518CC
const SHORT CInfGame::FEEDBACK_INVENTORYFULL_ITEMDROPPED = 8;

// 0x8518CE
const SHORT CInfGame::FEEDBACK_JOURNAL_UPDATE = 9;

// 0x8518D0
const SHORT CInfGame::FEEDBACK_WORLDMAP_UPDATE = 10;

// 0x8AFC50
static const char* off_8AFC50[FIFTY_THREE] = {
    "Inventory",
    "Character Record",
    "Return to Game",
    "Journal",
    "Map",
    "Spellbook",
    "Options",
    "Character Arbitration",
    "Guard / Protect",
    "Turn Undead",
    "Battle Song",
    "Cast Spell",
    "Use Item",
    "Dialog",
    "Thieving",
    "Stealth",
    "Trap Detection",
    "Group Stop",
    "Group Attack",
    "Special Abilities",
    "Equip Melee Weapon",
    "Equip Range Weapon",
    "Quick Load",
    "Hide Interface",
    "Chat Window",
    "Location",
    "Quick Save",
    "Toggle AI",
    "Toggle Interface",
    "Rest",
    "Toggle Portrait HP",
    "Formation",
    "Multiselect 1-2",
    "Multiselect 3-4",
    "Multiselect 5-6",
    "Multiselect 1-3",
    "Multiselect 4-6",
    "Multiselect All",
    "Quick Spell Slot1",
    "Quick Spell Slot2",
    "Quick Spell Slot3",
    "Quick Weapon Slot1",
    "Quick Weapon Slot2",
    "Quick Weapon Slot3",
    "Quick Weapon Slot4",
    "Quick Item Slot1",
    "Quick Item Slot2",
    "Quick Item Slot3",
    "Quick Formation Slot1",
    "Quick Formation Slot2",
    "Quick Formation Slot3",
    "Quick Formation Slot4",
    "Quick Formation Slot5",
};

static BOOL WriteBmpResource(const CString& sDirName, const CResRef& cResRef, const BYTE* pData, DWORD nData)
{
    CVidBitmap bitmap;
    bitmap.SetResRef(cResRef, TRUE, TRUE);

    CRes* pRes = static_cast<CRes*>(bitmap.GetRes());
    if (pRes == NULL) {
        return FALSE;
    }

    return pRes->Write(sDirName, const_cast<BYTE*>(pData), nData);
}

// 0x8AFD24
static const char* off_8AFD24[FIFTY_THREE] = {
    "I",
    "R",
    "G",
    "J",
    "M",
    "S",
    "O",
    "C",
    "",
    "",
    "",
    "",
    "",
    "D",
    "",
    "",
    "",
    "",
    "",
    "V",
    "",
    "",
    "L",
    "H",
    "T",
    "X",
    "Q",
    "A",
    "Y",
    "Z",
    ".",
    "F",
    "7",
    "8",
    "9",
    "0",
    "-",
    "=",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

// 0x8B4F84
static const char* off_8B4F84[] = {
    "SPPR101",
    "SPPR102",
    "SPPR103",
    "SPPR105",
    "SPPR106",
    "SPPR107",
    "SPPR108",
    "SPPR109",
    "SPPR110",
    "SPPR111",
    "SPPR112",
    "SPPR113",
    "SPPR114",
    "SPPR115",
    "SPPR116",
    "SPPR117",
    "SPPR201",
    "SPPR202",
    "SPPR203",
    "SPPR204",
    "SPPR205",
    "SPPR206",
    "SPPR207",
    "SPPR208",
    "SPPR210",
    "SPPR211",
    "SPPR212",
    "SPPR213",
    "SPPR214",
    "SPPR215",
    "SPPR216",
    "SPPR217",
    "SPPR218",
    "SPPR219",
    "SPPR220",
    "SPPR221",
    "SPPR222",
    "SPPR301",
    "SPPR302",
    "SPPR303",
    "SPPR304",
    "SPPR305",
    "SPPR306",
    "SPPR307",
    "SPPR308",
    "SPPR309",
    "SPPR310",
    "SPPR311",
    "SPPR312",
    "SPPR313",
    "SPPR314",
    "SPPR315",
    "SPPR316",
    "SPPR318",
    "SPPR319",
    "SPPR320",
    "SPPR322",
    "SPPR323",
    "SPPR324",
    "SPPR325",
    "SPPR326",
    "SPPR327",
    "SPPR328",
    "SPPR401",
    "SPPR402",
    "SPPR403",
    "SPPR404",
    "SPPR405",
    "SPPR406",
    "SPPR407",
    "SPPR408",
    "SPPR409",
    "SPPR410",
    "SPPR411",
    "SPPR414",
    "SPPR415",
    "SPPR416",
    "SPPR417",
    "SPPR418",
    "SPPR420",
    "SPPR422",
    "SPPR423",
    "SPPR424",
    "SPPR425",
    "SPPR426",
    "SPPR427",
    "SPPR428",
    "SPPR429",
    "SPPR501",
    "SPPR502",
    "SPPR503",
    "SPPR504",
    "SPPR507",
    "SPPR508",
    "SPPR510",
    "SPPR511",
    "SPPR512",
    "SPPR513",
    "SPPR514",
    "SPPR515",
    "SPPR516",
    "SPPR517",
    "SPPR518",
    "SPPR519",
    "SPPR520",
    "SPPR521",
    "SPPR522",
    "SPPR523",
    "SPPR524",
    "SPPR602",
    "SPPR605",
    "SPPR606",
    "SPPR607",
    "SPPR608",
    "SPPR609",
    "SPPR610",
    "SPPR611",
    "SPPR612",
    "SPPR613",
    "SPPR614",
    "SPPR615",
    "SPPR616",
    "SPPR702",
    "SPPR704",
    "SPPR705",
    "SPPR707",
    "SPPR709",
    "SPPR712",
    "SPPR714",
    "SPPR715",
    "SPPR716",
    "SPPR717",
    "SPPR718",
    "SPPR719",
    "SPPR720",
    "SPPR721",
    "SPPR722",
    "SPPR723",
    "SPPR724",
    "SPPR725",
    "SPPR726",
    "SPPR727",
    "SPPR728",
    "SPPR729",
    "SPWI101",
    "SPWI102",
    "SPWI103",
    "SPWI104",
    "SPWI105",
    "SPWI107",
    "SPWI108",
    "SPWI112",
    "SPWI113",
    "SPWI114",
    "SPWI115",
    "SPWI116",
    "SPWI117",
    "SPWI118",
    "SPWI119",
    "SPWI120",
    "SPWI121",
    "SPWI122",
    "SPWI201",
    "SPWI203",
    "SPWI205",
    "SPWI206",
    "SPWI207",
    "SPWI209",
    "SPWI210",
    "SPWI211",
    "SPWI212",
    "SPWI213",
    "SPWI214",
    "SPWI215",
    "SPWI217",
    "SPWI218",
    "SPWI219",
    "SPWI220",
    "SPWI221",
    "SPWI222",
    "SPWI223",
    "SPWI224",
    "SPWI225",
    "SPWI226",
    "SPWI227",
    "SPWI228",
    "SPWI302",
    "SPWI303",
    "SPWI304",
    "SPWI305",
    "SPWI306",
    "SPWI308",
    "SPWI309",
    "SPWI310",
    "SPWI311",
    "SPWI312",
    "SPWI313",
    "SPWI314",
    "SPWI316",
    "SPWI317",
    "SPWI318",
    "SPWI319",
    "SPWI320",
    "SPWI321",
    "SPWI322",
    "SPWI323",
    "SPWI401",
    "SPWI402",
    "SPWI404",
    "SPWI405",
    "SPWI406",
    "SPWI407",
    "SPWI408",
    "SPWI410",
    "SPWI411",
    "SPWI412",
    "SPWI413",
    "SPWI414",
    "SPWI417",
    "SPWI418",
    "SPWI419",
    "SPWI420",
    "SPWI421",
    "SPWI422",
    "SPWI423",
    "SPWI424",
    "SPWI425",
    "SPWI426",
    "SPWI427",
    "SPWI428",
    "SPWI501",
    "SPWI503",
    "SPWI504",
    "SPWI505",
    "SPWI507",
    "SPWI508",
    "SPWI509",
    "SPWI510",
    "SPWI511",
    "SPWI512",
    "SPWI513",
    "SPWI514",
    "SPWI515",
    "SPWI516",
    "SPWI517",
    "SPWI518",
    "SPWI519",
    "SPWI520",
    "SPWI521",
    "SPWI522",
    "SPWI523",
    "SPWI524",
    "SPWI525",
    "SPWI526",
    "SPWI601",
    "SPWI603",
    "SPWI605",
    "SPWI606",
    "SPWI607",
    "SPWI608",
    "SPWI609",
    "SPWI610",
    "SPWI611",
    "SPWI612",
    "SPWI613",
    "SPWI614",
    "SPWI615",
    "SPWI616",
    "SPWI617",
    "SPWI618",
    "SPWI619",
    "SPWI620",
    "SPWI621",
    "SPWI622",
    "SPWI623",
    "SPWI624",
    "SPWI626",
    "SPWI627",
    "SPWI628",
    "SPWI629",
    "SPWI630",
    "SPWI702",
    "SPWI703",
    "SPWI704",
    "SPWI705",
    "SPWI706",
    "SPWI707",
    "SPWI708",
    "SPWI709",
    "SPWI710",
    "SPWI711",
    "SPWI712",
    "SPWI713",
    "SPWI714",
    "SPWI715",
    "SPWI716",
    "SPWI717",
    "SPWI718",
    "SPWI719",
    "SPWI802",
    "SPWI803",
    "SPWI804",
    "SPWI805",
    "SPWI806",
    "SPWI807",
    "SPWI808",
    "SPWI809",
    "SPWI810",
    "SPWI902",
    "SPWI903",
    "SPWI905",
    "SPWI906",
    "SPWI907",
    "SPWI908",
    "SPWI909",
    "SPWI910",
    "SPWI911",
    NULL,
    NULL,
};

// 0x8ABA24
static char byte_8ABA24[CINFGAME_SPECIAL_SYMBOLS] = {
    ';',
    '=',
    ',',
    '-',
    '.',
    '/',
    '`',
    '[',
    '\\',
    ']',
    '\'',
    '\0',
};

// 0x8ABA3C
static BYTE byte_8ABA3C[CINFGAME_SPECIAL_SYMBOLS] = {
    VK_OEM_1,
    VK_OEM_PLUS,
    VK_OEM_COMMA,
    VK_OEM_MINUS,
    VK_OEM_PERIOD,
    VK_OEM_2,
    VK_OEM_3,
    VK_OEM_4,
    VK_OEM_5,
    VK_OEM_6,
    VK_OEM_7,
    0,
};

// 0x8E707C
const CString CInfGame::FOGOWAR_RESREF("FOGOWAR");

// 0x8E74AC
const CString CInfGame::SAVE_OBJECT_LIST_NAME("SVTRIOBJ");

// 0x8E74F4
const CString CInfGame::CHAPTER_GLOBAL("CHAPTER");

// 0x8E74E8
const CString CInfGame::TOKEN_CURRENT_CHAPTER("CurrentChapter");

// 0x8E7510
const CString CInfGame::TOKEN_CHARACTER("CHARNAME");

// 0x8E7498
const CString CInfGame::TOKEN_GABBER("GABBER");

// 0x8E7064
const CString CInfGame::TOKEN_SIRMAAM("PROTAGONIST_SIRMAAM");

// 0x8E7084
const CString CInfGame::TOKEN_GIRLBOY("PROTAGONIST_GIRLBOY");

// 0x8E74FC
const CString CInfGame::TOKEN_BROTHERSISTER("PROTAGONIST_BROTHERSISTER");

// 0x8E7060
const CString CInfGame::TOKEN_LADYLORD("PROTAGONIST_LADYLORD");

// 0x8E7520
const CString CInfGame::TOKEN_MALEFEMALE("PROTAGONIST_MALEFEMALE");

// 0x8E7518
const CString CInfGame::TOKEN_HESHE("PROTAGONIST_HESHE");

// 0x8E7504
const CString CInfGame::TOKEN_HISHER("PROTAGONIST_HISHER");

// 0x8E74DC
const CString CInfGame::TOKEN_RACE("PROTAGONIST_RACE");

// 0x8E74D0
const CString CInfGame::TOKEN_HIMHER("PROTAGONIST_HIMHER");

// 0x8E7074
const CString CInfGame::TOKEN_MANWOMAN("PROTAGONIST_MANWOMAN");

// 0x8E74C4
const CString CInfGame::TOKEN_GABBER_SIRMAAM("SIRMAAM");

// 0x8E7070
const CString CInfGame::TOKEN_GABBER_GIRLBOY("GIRLBOY");

// 0x8E74E4
const CString CInfGame::TOKEN_GABBER_BROTHERSISTER("BROTHERSISTER");

// 0x8E7088
const CString CInfGame::TOKEN_GABBER_LADYLORD("LADYLORD");

// 0x8E7080
const CString CInfGame::TOKEN_GABBER_MALEFEMALE("MALEFEMALE");

// 0x8E751C
const CString CInfGame::TOKEN_GABBER_HESHE("HESHE");

// 0x8E74EC
const CString CInfGame::TOKEN_GABBER_HISHER("HISHER");

// 0x8E708C
const CString CInfGame::TOKEN_GABBER_RACE("RACE");

// 0x8E74C8
const CString CInfGame::TOKEN_GABBER_HIMHER("HIMHER");

// 0x8E74F0
const CString CInfGame::TOKEN_GABBER_MANWOMAN("MANWOMAN");

// 0x8E7494
const CString CInfGame::TOKEN_PLAYER1("PLAYER1");

// 0x8E74BC
const CString CInfGame::TOKEN_PLAYER2("PLAYER2");

// 0x8E7490
const CString CInfGame::TOKEN_PLAYER3("PLAYER3");

// 0x8E74A4
const CString CInfGame::TOKEN_PLAYER4("PLAYER4");

// 0x8E74C0
const CString CInfGame::TOKEN_PLAYER5("PLAYER5");

// 0x8E749C
const CString CInfGame::TOKEN_PLAYER6("PLAYER6");

// 0x8E74D8
const CString CInfGame::TOKEN_SURFACE_UNDERDARK("SURFACE_UNDERDARK");

// 0x8E74D4
const CString CInfGame::AUTO_SAVE_NAME("000000000-Auto-Save");

// 0x8E7508
const CString CInfGame::QUICK_SAVE_NAME("000000002-Quick-Save");

// 0x8E750C
const CString CInfGame::QUICK_SAVE_BACKUP_NAME("000000001-Quick-Save-Backup");

// 0x8E706C
const CString CInfGame::SILHOUETTE_PORTRAIT_SM("noportsm");

// 0x8E705C
const CString CInfGame::SILHOUETTE_PORTRAIT_LG("noportlg");

// 0x8E7524
BOOL CInfGame::m_bHealPartyOnRest;

// 0x8E7528
BOOLEAN CInfGame::byte_8E7528;

// 0x8E752C
int CInfGame::dword_8E752C;

// 0x8E52EC
int CInfGame::dword_8E52EC;

// 0x8E52F0
int CInfGame::dword_8E52F0;

// 0x59CC30
CInfGame::CInfGame()
    : m_rgbMasterBitmap(CResRef("MPALETTE"), FALSE, 24)
{
    m_nUniqueAreaID = 0;
    m_nReputation = 0;
    m_nProtagonistId = 0;
    m_bPartyAI = FALSE;
    m_bShowAreaNotes = FALSE;
    m_dwLastProgressRenderTickCount = 0;
    m_dwLastProgressMsgTickCount = 0;
    m_nLastSaveTime = 0;
    m_lastTarget = 0;
    m_bTriggerOutline = 0;
    m_nCharacterTerminationSequenceDelay = 0;
    m_nAIIndex = 0;
    m_nTimeStop = 0;
    m_nTimeStopCaster = 0;
    memset(m_appearanceColorsWeapon, 0, sizeof(m_appearanceColorsWeapon));
    memset(m_appearanceColorsBody, 0, sizeof(m_appearanceColorsBody));
    memset(m_appearanceColorsShield, 0, sizeof(m_appearanceColorsShield));
    memset(m_appearanceColorsHelmet, 0, sizeof(m_appearanceColorsHelmet));
    m_nDifficultyLevel = 0;
    m_nCharacters = 0;
    for (INT nIndex = 0; nIndex < CINFGAME_MAXCHARACTERS; nIndex++) {
        m_characters[nIndex] = CGameObjectArray::INVALID_INDEX;
        m_characterPortraits[nIndex] = CGameObjectArray::INVALID_INDEX;
    }

    m_nState = 0;
    m_tempCursor = 4;
    m_iconIndex = -1;
    m_bGameLoaded = FALSE;
    m_bInLoadGame = FALSE;
    m_bInLoadArea = FALSE;
    m_bFromNewGame = FALSE;
    m_bInDestroyGame = FALSE;
    m_bInAreaTransition = FALSE;
    m_visibleArea = 0;
    memset(m_gameAreas, 0, sizeof(m_gameAreas));
    m_pGameAreaMaster = NULL;
    m_pathSearch = NULL;
    m_listGrid = NULL;
    m_searchRequestListEmpty = TRUE;
    m_iContainer = CGameObjectArray::INVALID_INDEX;
    m_iContainerSprite = CGameObjectArray::INVALID_INDEX;
    m_bForceDither = FALSE;
    m_sSaveDir = ".\\save\\";
    m_sMultiplayerSaveDir = ".\\mpsave\\";
    m_sTempDir = ".\\temp\\";
    m_sTempSaveDir = ".\\tempsave\\";
    m_sScriptsDir = ".\\scripts\\";
    m_sPortraitsDir = ".\\portraits\\";
    m_sSoundsDir = ".\\sounds\\";
    m_sCharactersDir = ".\\characters\\";

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempDir)) {
        UTIL_ASSERT_MSG(FALSE, "Problems removing files from Temp directory, delete the directory and try again");
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)) {
        UTIL_ASSERT_MSG(FALSE, "Problems removing files from TempSave directory, delete the directory and try again");
    }

    g_pChitin->cDimm.AddToDirectoryList(m_sScriptsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sSoundsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sPortraitsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sCharactersDir, TRUE);

    m_gameSave.m_curFormation = TRUE;
    m_gameSave.m_bArenaMode = FALSE;

    m_zoomLevel = 48;
    m_zoomEnabled = 48;
    m_gameSave.m_mode = -1;
    m_gameSave.m_cutScene = FALSE;
    m_currArmor = '1';
    field_4A46 = -1;
    m_currAnimation = -1;
    m_vcLocator.SetResRef(CResRef("locater"), FALSE, TRUE, TRUE);
    memset(m_defaultFamiliarResRefs, 0, sizeof(m_defaultFamiliarResRefs));
    field_4204 = 0;
    m_nCharacterOverflowCount = 0;
    m_saveObjectList.LoadList(CResRef(SAVE_OBJECT_LIST_NAME), FALSE);
    m_listGrid = new CPathNode*[CPathSearch::GRID_ACTUALX * CPathSearch::GRID_ACTUALY];
    m_pathSearch = new CPathSearch(m_listGrid);
    m_searchShutdown = FALSE;
    m_hSearchThread = CreateSemaphoreA(NULL, 0, 1, NULL);
    if (m_hSearchThread != NULL) {
        m_nTravelScreenImageToUse = 0;
        m_bPlayerScriptStyle = FALSE;
        field_4B38 = 0;
        m_bSaveScreen = 0;
        m_nAreaFirstObject = 0;

        m_INISounds.SetFileName(CString("sounds"));
        if (m_INISounds.Load(CString("sounds")) == 0) {
            UTIL_ASSERT_MSG(FALSE, "Problem loading the sounds.ini file!");
        }

        memset(m_aServerStore, 0, sizeof(m_aServerStore));
        memset(m_nServerStoreDemands, 0, sizeof(m_nServerStoreDemands));

        field_50D8 = FALSE;
        field_50DC = 0;

        CString sFileName;
        g_pChitin->cDimm.WriteSetUp(m_sTempDir + "foo.bar", sFileName);
        g_pChitin->cDimm.WriteSetUp(m_sTempSaveDir + "foo.bar", sFileName);

        field_4BD4 = 0;
        m_bExpansion = FALSE;
        field_4BD6 = FALSE;

        C2DArray tSpells;
        tSpells.Load(CResRef(CRuleTables::LISTSPLL));
        m_spells.Load(tSpells, 7);

        for (unsigned int spellcasterClassIndex = 0; spellcasterClassIndex < 7; spellcasterClassIndex++) {
            m_spellsByClass[spellcasterClassIndex].Load(tSpells,
                spellcasterClassIndex,
                7,
                NULL);
        }

        C2DArray tDomainSpells;
        tDomainSpells.Load(CResRef(CRuleTables::LISTDOMN));

        for (unsigned int domainIndex = 0; domainIndex < 9; domainIndex++) {
            m_spellsByDomain[domainIndex].Load(tDomainSpells,
                domainIndex,
                9,
                &m_spells);
        }

        C2DArray tInnateSpells;
        tInnateSpells.Load(CResRef(CRuleTables::LISTINNT));
        m_innateSpells.Load(tInnateSpells, 0);

        C2DArray tSongs;
        tSongs.Load(CResRef(CRuleTables::LISTSONG));
        m_songs.Load(tSongs, 0);

        C2DArray tShapeshifts;
        tShapeshifts.Load(CResRef(CRuleTables::LISTSHAP));
        m_shapeshifts.Load(tShapeshifts, 0);

        CString sError(" ");
        UINT nIndex;

        for (nIndex = 0; nIndex < m_spells.m_nCount; nIndex++) {
            CSpell cSpell(m_spells.Get(nIndex));

            sError.Format("Unable to load Class Spell file: %s\n",
                (LPCSTR)m_spells.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Class Spell level: %s - %d\n",
                    (LPCSTR)m_spells.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }

        for (nIndex = 0; nIndex < m_innateSpells.m_nCount; nIndex++) {
            CSpell cSpell(m_innateSpells.Get(nIndex));

            sError.Format("Unable to load Innate Spell file: %s\n",
                (LPCSTR)m_innateSpells.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Innate Spell level: %s - %d\n",
                    (LPCSTR)m_innateSpells.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }

        for (nIndex = 0; nIndex < m_songs.m_nCount; nIndex++) {
            CSpell cSpell(m_songs.Get(nIndex));

            sError.Format("Unable to load Song file: %s\n",
                (LPCSTR)m_songs.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Song level: %s - %d\n",
                    (LPCSTR)m_songs.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }

        for (nIndex = 0; nIndex < m_shapeshifts.m_nCount; nIndex++) {
            CSpell cSpell(m_shapeshifts.Get(nIndex));

            sError.Format("Unable to load Shapeshift file: %s\n",
                (LPCSTR)m_shapeshifts.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Shapeshift level: %s - %d\n",
                    (LPCSTR)m_shapeshifts.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }
    }
}

// 0x59ECB0
CInfGame::~CInfGame()
{
    CSingleLock searchLock(&field_1B58, FALSE);

    if (m_bGameLoaded) {
        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pChitin->cNetwork.CloseSession(TRUE);
        }
        DestroyGame(FALSE, FALSE);
    }

    if (m_hSearchThread != NULL) {
        searchLock.Lock(INFINITE);
        m_searchShutdown = TRUE;
        ReleaseSemaphore(m_hSearchThread, 1, NULL);
        searchLock.Unlock();

        while (m_hSearchThread != NULL) {
            SleepEx(1, FALSE);
        }

        if (m_pathSearch != NULL) {
            delete m_pathSearch;
            m_pathSearch = NULL;
        }

        if (m_listGrid != NULL) {
            delete[] m_listGrid;
            m_listGrid = NULL;
        }

        m_allies.RemoveAll();
        m_familiars.RemoveAll();

        g_pChitin->cDimm.RemoveFromDirectoryList(m_sScriptsDir, FALSE);
        g_pChitin->cDimm.RemoveFromDirectoryList(m_sSoundsDir, FALSE);
        g_pChitin->cDimm.RemoveFromDirectoryList(m_sPortraitsDir, FALSE);
        g_pChitin->cDimm.RemoveFromDirectoryList(m_sCharactersDir, FALSE);

        Icewind586B70::Destroy();
    }
}

// 0x59F500
void CInfGame::StartSearchThread()
{
    if (_beginthread(SearchThreadMain, 0, NULL) == -1) {
        CloseHandle(m_hSearchThread);
        m_hSearchThread = NULL;
    }
}

// 0x59F540
void CInfGame::InitGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    m_cVRamPool.AttachSurfaces(g_pBaldurChitin->GetCurrentVideoMode());

    if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL) {
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::AREA_TRANSITION, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::CHAR_RECORDS, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::DIALOG, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::GROUP_POOL, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::LEADER, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::MODIFY_CHARS, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::PAUSING, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::PURCHASING, TRUE);
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::LEADER, TRUE);
    }

    LoadKeymap();
    LoadOptions();
    g_pBaldurChitin->EnginesGameInit();

    m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);

    g_pBaldurChitin->GetCurrentVideoMode()->rgbGlobalTint = RGB(255, 255, 255);

    m_worldTime.m_gameTime = CTimerWorld::TIME_DAY + 1;
    m_worldTime.CheckForTriggerEventPast();

    g_pBaldurChitin->GetTlkTable().Fetch(16484, m_soundNeedParty);
    g_pBaldurChitin->GetTlkTable().Fetch(15307, m_soundAreaTransitionRefused);

    for (BYTE nIndex = 0; nIndex < 6; nIndex++) {
        EnablePortrait(nIndex, FALSE);
    }

    m_gameSave.m_nGroupInventoryNumber = 0;
    memset(m_gameSave.m_groupInventory, 0, sizeof(m_gameSave.m_groupInventory));
    m_gameSave.m_mode = -1;
    m_gameSave.m_cutScene = FALSE;
    m_appearanceWeapon = "";
    m_appearanceBody = "";
    m_appearanceShield = "";
    m_appearanceHelmet = "";

    m_pGameAreaMaster = NULL;
    memset(m_gameAreas, 0, sizeof(m_gameAreas));
    m_visibleArea = 0;
    m_bPartyAI = TRUE;
    m_bShowAreaNotes = TRUE;
    m_bTriggerOutline = 0;
    m_bGameLoaded = TRUE;

    m_allies.RemoveAll();
    m_familiars.RemoveAll();

    memset(m_defaultFamiliarResRefs, 0, sizeof(m_defaultFamiliarResRefs));

    field_4204 = 0;

    m_cJournal.ClearAllEntries();

    m_variables.Resize(2048);
    m_namedCreatures.Resize(2048);
    m_variables.ClearAll();
    m_namedCreatures.ClearAll();

    m_entanglePalette.SetType(CVidPalette::TYPE_RANGE);
    m_entanglePalette.SetRange(0, 54, m_rgbMasterBitmap);

    m_webHoldPalette.SetType(CVidPalette::TYPE_RANGE);
    m_webHoldPalette.SetRange(0, 65, m_rgbMasterBitmap);

    ValidateCache();

    m_nCharacterTerminationSequenceDelay = 0;

    m_cMoveList.ClearAll();
    m_cLimboList.ClearAll();

    CGameAIGame* pAIGame = new CGameAIGame();
    CAIScript* pScript = new CAIScript(CResRef("BALDUR"));

    // NOTE: Uninline.
    pAIGame->SetDefaultScript(pScript);

    m_nAIIndex = pAIGame->m_id;
    m_nTimeStop = 0;
    m_nTimeStopCaster = -1;

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3213
        UTIL_ASSERT_MSG(FALSE, "Could not clean out Temp directory");
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3215
        UTIL_ASSERT_MSG(FALSE, "Could not clean out TempSave directory");
    }

    // TODO: Use loop.
    memset(m_aServerStore, 0, sizeof(m_aServerStore));
    memset(m_nServerStoreDemands, 0, sizeof(m_nServerStoreDemands));

    dword_8E752C = 0;

    ResetMultiPlayerPermissions();
}

// 0x59F9A0
void CInfGame::BeginListManipulation(CGameArea* pArea)
{
    if (pArea != NULL) {
        EnterCriticalSection(&(pArea->field_214));
        pArea->SetListManipulationThreadId(GetCurrentThreadId());
    }
}

// 0x59F9D0
INT CInfGame::EndListManipulation(CGameArea* pArea)
{
    INT nCounter = 0;

    if (pArea != NULL) {
        nCounter = pArea->SetListManipulationThreadId(0);
        LeaveCriticalSection(&(pArea->field_214));
    }

    return nCounter;
}

// 0x59FA00
void CInfGame::WaitForEngine(BOOL a1)
{
    CGameArea* pArea = GetVisibleArea();
    if (pArea != NULL) {
        INT nCounter = pArea->m_nListManipulationThreadCounter;
        if (pArea->m_ListManipulationThreadId == GetCurrentThreadId()) {
            while (1) {
                CGameArea* pArea = GetVisibleArea();
                if (pArea == NULL) {
                    break;
                }

                INT nRemainingCounter = pArea->SetListManipulationThreadId(0);
                LeaveCriticalSection(&(pArea->field_214));

                if (nRemainingCounter == 0) {
                    break;
                }
            }

            while (g_pChitin->m_bDisplayStale == TRUE
                || (g_pChitin->m_bInSynchronousUpdate == TRUE && a1)) {
                SleepEx(5, FALSE);
            }

            while (nCounter != 0) {
                CGameArea* pArea = GetVisibleArea();
                if (pArea != NULL) {
                    EnterCriticalSection(&(pArea->field_214));
                    pArea->SetListManipulationThreadId(GetCurrentThreadId());
                }
                nCounter--;
            }
        } else {
            while (g_pChitin->m_bInSynchronousUpdate == TRUE
                || (g_pChitin->m_bDisplayStale == TRUE && a1)) {
                SleepEx(5, FALSE);
            }
        }
    } else {
        while (g_pChitin->m_bInSynchronousUpdate == TRUE
            || (g_pChitin->m_bDisplayStale == TRUE && a1)) {
            SleepEx(5, FALSE);
        }
    }
}

// 0x59FB50
void CInfGame::DestroyGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    DWORD dwPerSegment;
    BYTE cnt;

    if (!bProgressBarInPlace) {
        if (bProgressBarRequired == TRUE) {
            WaitForEngine(TRUE);
            g_pChitin->SetProgressBar(TRUE,
                11831,
                0,
                0,
                FALSE,
                0,
                FALSE,
                0,
                FALSE,
                FALSE,
                255);
            g_pChitin->cProgressBar.m_nActionProgress = 0;
            g_pChitin->cProgressBar.m_nActionTarget = 5000000;
            g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        }
    }

    dwPerSegment = 555555;

    CSingleLock lock(&(g_pBaldurChitin->GetObjectGame()->field_1B58), TRUE);
    m_bInDestroyGame = TRUE;

    for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            while (m_gameAreas[cnt]->m_bInPathSearch != FALSE) {
                SleepEx(25, FALSE);
            }
        }
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    if (g_pBaldurChitin->GetScreenWorld() != NULL) {
        g_pBaldurChitin->GetScreenWorld()->m_weather.CancelCurrentWeather(NULL, 0);
    }

    g_pChitin->cDimm.RemoveFromDirectoryList(m_sTempDir, TRUE);
    m_cObjectArray.Clean();

    EnterCriticalSection(&(g_pBaldurChitin->GetScreenWorld()->field_106));
    for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            while (m_gameAreas[cnt]->m_bInPathSearch != FALSE) {
                SleepEx(25, FALSE);
            }

            delete m_gameAreas[cnt];
            m_gameAreas[cnt] = NULL;
        }
    }
    m_visibleArea = 0;
    LeaveCriticalSection(&(g_pBaldurChitin->GetScreenWorld()->field_106));

    lock.Unlock();

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    for (cnt = 0; cnt < CINFGAME_MAXCHARACTERS; cnt++) {
        EnablePortrait(cnt, FALSE);
        if (m_characterPortraits[cnt] != -1) {
            CGameObject* pObject;
            if (m_cObjectArray.Delete(m_characterPortraits[cnt], CGameObjectArray::THREAD_ASYNCH, &pObject, -1) == CGameObjectArray::SUCCESS
                && pObject != NULL) {
                delete pObject;
            }
            m_characterPortraits[cnt] = -1;
        }
    }

    memset(m_characters, -1, sizeof(m_characters));
    m_nCharacters = 0;

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    while (!m_lstGlobalCreatures.IsEmpty()) {
        LONG iObject = reinterpret_cast<LONG>(m_lstGlobalCreatures.RemoveHead());
        CGameObject* pObject;
        if (m_cObjectArray.Delete(iObject, CGameObjectArray::THREAD_ASYNCH, &pObject, -1) == CGameObjectArray::SUCCESS
            && pObject != NULL) {
            delete pObject;
        }
    }

    for (cnt = 0; cnt < 9; cnt++) {
        while (!m_familiarResRefs[cnt]->IsEmpty()) {
            delete m_familiarResRefs[cnt]->RemoveHead();
        }
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    m_zoomLevel = 48;
    m_zoomEnabled = 48;
    m_nState = 0;
    m_tempCursor = 4;
    m_iconIndex = -1;
    m_searchRequestListEmpty = TRUE;
    m_currArmor = '1';
    field_4A46 = -1;
    m_currAnimation = -1;
    m_bGameLoaded = FALSE;

    for (cnt = 0; cnt < 100; cnt++) {
        if (m_gameSave.m_groupInventory[cnt] != NULL) {
            delete m_gameSave.m_groupInventory[cnt];
            m_gameSave.m_groupInventory[cnt] = NULL;
        }
    }

    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    m_group.m_memberList.RemoveAll();
    m_group.m_groupChanged = TRUE;
    m_allies.RemoveAll();
    m_familiars.RemoveAll();
    g_pBaldurChitin->EnginesGameUninit();

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
        // FIXME: Redundant.
        if (bProgressBarInPlace || bProgressBarRequired) {
            ProgressBarCallback(dwPerSegment, FALSE);
        }
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3592
        UTIL_ASSERT_MSG(FALSE, "Could not remove files from Temp directory");
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3296
        UTIL_ASSERT_MSG(FALSE, "Could not remove files from TempSave directory");
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    m_bInDestroyGame = FALSE;

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        g_pChitin->m_bDisplayStale = TRUE;
        WaitForEngine(TRUE);
        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }

    m_cVRamPool.DetachSurfaces();

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL) {
            delete m_aServerStore[cnt];
        }
        m_aServerStore[cnt] = NULL;
    }

    POSITION pos = field_4BDC.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        CSavedGameStoredLocation* node = field_4BDC.GetNext(pos);
        if (node != NULL) {
            delete node;
        }
        field_4BDC.RemoveAt(posOld);
    }
}

// 0x5A0160
void CInfGame::ValidateCache()
{
    CString sSearchPath = g_pChitin->cDimm.cResCache.m_sDirName + "data\\*.*";
    CString sResolvedSearchPath;
    if (g_pChitin->lAliases.ResolveFileName(sSearchPath, sResolvedSearchPath) == FALSE) {
        sResolvedSearchPath = sSearchPath;
    }

    CFileFind fileFind;
    BOOLEAN bWorking = fileFind.FindFile(sResolvedSearchPath, 0);
    while (bWorking) {
        bWorking = fileFind.FindNextFile();

        CString sBifFileName = fileFind.GetFilePath();
        INT nSlash = sBifFileName.Find('\\');
        while (nSlash != -1) {
            sBifFileName = sBifFileName.Right(sBifFileName.GetLength() - nSlash - 1);
            nSlash = sBifFileName.Find('\\');
        }

        CString sBifName = sBifFileName.Left(sBifFileName.GetLength() - 4);
        CString sExtension = sBifFileName.Right(4);
        sExtension.MakeUpper();
        if (sExtension.Compare(".BIF") == 0) {
            CString sMinimumSize;
            CPoint ptLocation;
            if (m_ruleTables.m_tCacheValidation.Find(sBifName, ptLocation, FALSE) == TRUE) {
                if (m_ruleTables.m_tCacheValidation.GetWidth() < 2
                    || m_ruleTables.m_tCacheValidation.GetHeight() <= ptLocation.y
                    || ptLocation.y < 0) {
                    sMinimumSize = m_ruleTables.m_tCacheValidation.GetDefault();
                } else {
                    sMinimumSize = m_ruleTables.m_tCacheValidation.GetAt(CPoint(1, ptLocation.y));
                }
            } else {
                sMinimumSize = m_ruleTables.m_tCacheValidation.GetDefault();
            }

            g_pChitin->cDimm.cResCache.ValidateFile(g_pChitin->cDimm.GetResFileID(sBifName),
                atol(sMinimumSize));
        }
    }

    fileFind.Close();
}

// 0x5A04B0
LONG CInfGame::CachingRequirements(const CString& areaName)
{
    CAreaFile cAreaFile;

    cAreaFile.SetResRef(CResRef(areaName), TRUE, TRUE);

    BYTE* pAreaData = cAreaFile.GetData();
    if (pAreaData == NULL) {
        return 0;
    }

    RESID nWedID = g_pChitin->cDimm.GetResID(CResRef(pAreaData + 8), 1003);
    CResFile* pResFile = g_pChitin->cDimm.GetResFilePtr(nWedID);
    cAreaFile.ReleaseData();

    if (pResFile == NULL) {
        return 0;
    }

    LONG nTotal;

    if (!g_pChitin->cNetwork.m_bConnectionEstablished) {
        nTotal = pResFile->GetCacheSize();
    } else {
        CString sCacheKey;
        pResFile->GetCacheKey(sCacheKey);

        CPoint ptLocation;
        CString sXLabel;
        CString sYLabel;
        BOOLEAN bFoundLocation = m_ruleTables.m_tAreaLinkageCaching.Find(sCacheKey, ptLocation, TRUE);
        BOOLEAN bFoundLabels = m_ruleTables.m_tAreaLinkageCaching.Find(sCacheKey, sXLabel, sYLabel, TRUE);
        if ((bFoundLocation & bFoundLabels) == 1) {
            CString sCount = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(0, ptLocation.y));
            LONG nLinked = atol(sCount.GetBuffer(0));
            if (nLinked != 0) {
                // Sum the cache size of every resfile linked to the area.
                LONG nSum = 0;
                int nColumn = 0;
                UINT nResFileID = g_pChitin->cDimm.GetResFileID(sYLabel);
                if (nLinked >= 0) {
                    CString sResFile;
                    do {
                        CResFile* pLinked = g_pChitin->cDimm.GetResFilePtr(nResFileID);
                        if (pLinked == NULL) {
                            break;
                        }
                        nSum += pLinked->GetCacheSize();
                        nColumn++;
                        sResFile = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(nColumn, ptLocation.y));
                        nResFileID = g_pChitin->cDimm.GetResFileID(sResFile);
                    } while (nColumn <= nLinked);
                }
                nTotal = nSum;
            } else {
                nTotal = pResFile->GetCacheSize();
            }
        } else {
            nTotal = pResFile->GetCacheSize();
        }
    }

    return nTotal;
}

// Loading-screen minibar caching registry.  As an area's linked resfiles are
// cached, their names are recorded here so the loading screen can draw a
// progress minibar for each.  Slot 0 holds the area name and g_nCachingResFiles
// counts the valid entries.
//
// 0x8FB90C
CString g_aCachingResFiles[8];
// 0x8FB92C
int g_nCachingResFiles;

// 0x5A0950
BOOL CInfGame::CacheResFileWithResource(const CString& areaName)
{
    CAreaFile cAreaFile;

    for (int i = 1; i < 7; i++) {
        g_aCachingResFiles[i] = "";
    }
    g_aCachingResFiles[0] = areaName;
    g_nCachingResFiles = 1;

    // Demand the area resource and read its file so the WED reference embedded
    // in the header can be resolved to the resfile that backs the area.
    cAreaFile.SetResRef(CResRef(areaName), TRUE, TRUE);

    BYTE* pAreaData = cAreaFile.GetData();
    RESID nWedID = g_pChitin->cDimm.GetResID(CResRef(pAreaData + 8), 1003);
    CResFile* pResFile = g_pChitin->cDimm.GetResFilePtr(nWedID);
    cAreaFile.ReleaseData();

    if (pResFile == NULL) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.m_bConnectionEstablished) {
        CString sCacheKey;
        pResFile->GetCacheKey(sCacheKey);

        CPoint ptLocation;
        CString sXLabel;
        CString sYLabel;
        BOOLEAN bFoundLocation = m_ruleTables.m_tAreaLinkageCaching.Find(sCacheKey, ptLocation, TRUE);
        BOOLEAN bFoundLabels = m_ruleTables.m_tAreaLinkageCaching.Find(sCacheKey, sXLabel, sYLabel, TRUE);
        if ((bFoundLocation & bFoundLabels) == 1) {
            CString sCount = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(0, ptLocation.y));
            LONG nLinked = atol(sCount.GetBuffer(0));
            if (nLinked != 0) {
                CString sResFile;

                // First pass: cache every linked resfile that is not resident.
                int nColumn = 0;
                UINT nResFileID = g_pChitin->cDimm.GetResFileID(sYLabel);
                if (nLinked >= 0) {
                    do {
                        CResFile* pLinked = g_pChitin->cDimm.GetResFilePtr(nResFileID);
                        if (pLinked == NULL) {
                            return FALSE;
                        }
                        if (pLinked->GetCacheSize() == 0) {
                            if (pLinked->Cache() == 1) {
                                pLinked->UnCache();
                            }
                        }
                        nColumn++;
                        sResFile = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(nColumn, ptLocation.y));
                        nResFileID = g_pChitin->cDimm.GetResFileID(sResFile);
                    } while (nColumn <= nLinked);
                }

                // Second pass: register each linked resfile for the minibar and
                // re-cache those that are already resident.
                nColumn = 0;
                nResFileID = g_pChitin->cDimm.GetResFileID(sYLabel);
                if (nLinked >= 0) {
                    do {
                        CResFile* pLinked = g_pChitin->cDimm.GetResFilePtr(nResFileID);
                        if (pLinked == NULL) {
                            return FALSE;
                        }
                        g_aCachingResFiles[g_nCachingResFiles++] = sResFile;
                        if (pLinked->GetCacheSize() != 0) {
                            if (pLinked->Cache() == 1) {
                                pLinked->UnCache();
                            }
                        }
                        nColumn++;
                        sResFile = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(nColumn, ptLocation.y));
                        nResFileID = g_pChitin->cDimm.GetResFileID(sResFile);
                    } while (nColumn <= nLinked);
                }

                return TRUE;
            }
        }
    }

    // Fall back to caching only the area's own resfile.
    if (pResFile->Cache() == 1) {
        pResFile->UnCache();
    }
    return TRUE;
}

// 0x5A0F50
LONG CInfGame::ImportCharacter(const CString& sFileName, INT nIndex)
{
    CString sPath = sFileName;
    if (sPath.Right(4).CompareNoCase(".chr") != 0) {
        sPath += ".chr";
    }

    FILE* stream = fopen(GetDirCharacters() + sPath, "rb");
    if (stream == NULL) {
        stream = fopen(sPath, "rb");
    }

    if (stream == NULL) {
        return 0;
    }

    fseek(stream, 0, SEEK_END);
    LONG nChrSize = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    if (nChrSize <= 0) {
        fclose(stream);
        return 0;
    }

    BYTE* pChrData = new BYTE[nChrSize];
    size_t nRead = fread(pChrData, 1, nChrSize, stream);
    fclose(stream);

    if (nRead != static_cast<size_t>(nChrSize)) {
        delete[] pChrData;
        return 0;
    }

    if (nChrSize < 0x30 || memcmp(pChrData, "CHR V2.2", 8) != 0) {
        delete[] pChrData;
        return 0;
    }

    DWORD nCreatureOffset = *reinterpret_cast<DWORD*>(pChrData + 0x28);
    DWORD nCreatureSize = *reinterpret_cast<DWORD*>(pChrData + 0x2C);

    if (nCreatureOffset == 0 || nCreatureSize == 0 || nCreatureOffset + nCreatureSize > static_cast<DWORD>(nChrSize)) {
        if (nChrSize > 0x224) {
            nCreatureOffset = 0x224;
            nCreatureSize = nChrSize - 0x224;
        } else {
            delete[] pChrData;
            return 0;
        }
    }

    if (nCreatureSize < 8 || memcmp(pChrData + nCreatureOffset, "CRE V2.2", 8) != 0) {
        delete[] pChrData;
        return 0;
    }

    CGameSprite* pSprite = new CGameSprite(pChrData + nCreatureOffset,
        nCreatureSize,
        0,
        0,
        -1,
        0,
        0,
        INT_MAX,
        CPoint(-1, -1),
        0);

    if (pSprite == NULL) {
        delete[] pChrData;
        return 0;
    }

    char szName[33];
    memcpy(szName, pChrData + 0x8, 32);
    szName[32] = '\0';
    for (INT nIndex = 31; nIndex >= 0; nIndex--) {
        if (szName[nIndex] == '\0' || szName[nIndex] == ' ') {
            szName[nIndex] = '\0';
        } else {
            break;
        }
    }

    if (pSprite->m_baseStats.m_name == -1 && szName[0] != '\0') {
        pSprite->m_sName = szName;
    }

    if (nChrSize >= 0x164) {
        memcpy(pSprite->field_3D14, pChrData + 0x13E, sizeof(pSprite->field_3D14));
        pSprite->m_nLastSpellbookClassIndex = pChrData[0x162];
        pSprite->m_nLastSpellbookSpellLevel = pChrData[0x163];
    }

    if (nChrSize >= 0x1A4) {
        pSprite->m_secondarySounds = CResRef(pChrData + 0x17C);
        memcpy(pSprite->field_725A, pChrData + 0x184, sizeof(pSprite->field_725A));
        pSprite->field_725A[sizeof(pSprite->field_725A) - 1] = '\0';
    }

    pSprite->m_bGlobal = TRUE;
    pSprite->SetResRef(CResRef(sFileName));
    pSprite->AddKnownDivineSpells(CAIObjectType::C_CLERIC);
    pSprite->AddKnownDivineSpells(CAIObjectType::C_DRUID);
    pSprite->AddKnownDivineSpells(CAIObjectType::C_PALADIN);
    pSprite->AddKnownDivineSpells(CAIObjectType::C_RANGER);
    AddPartyGold(GetRuleTables().GetStartingGold(pSprite));

    LONG nCharacterId = pSprite->m_id;
    delete[] pChrData;

    return nCharacterId;
}

// 0x5A1A80
CString CInfGame::GetDirSounds()
{
    return m_sSoundsDir;
}

// 0x5A1AB0
void CInfGame::CharacterExport(LONG nCharacterId, CString sFileName)
{
    // TODO: Incomplete.
}

// 0x5A24D0
CGameArea* CInfGame::LoadArea(CString areaName, BYTE nTravelScreenImageToUse, BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    CAreaFile areaData;
    CString sLevel;
    CString sResRef;
    LONG highRequests;
    LONG mediumRequests;
    LONG lowRequests;
    DWORD nTotalCachingRequirements;
    BYTE cnt;
    CGameArea* pArea;

    areaName.MakeUpper();

    pArea = GetArea(areaName);
    if (pArea != NULL) {
        if (bProgressBarInPlace) {
            ProgressBarCallback(3000000, FALSE);
        }
        return pArea;
    }

    if (!bProgressBarInPlace) {
        g_pChitin->m_bDisplayStale = TRUE;
        SleepEx(25, FALSE);
    }

    BYTE nAverageLevel = 0;
    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        CGameSprite* pSprite;
        LONG nCharacterId = m_characterPortraits[nPortrait];
        BYTE rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            nAverageLevel += pSprite->GetDerivedStats()->m_nLevel;
            m_cObjectArray.ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (m_nCharacters > 0) {
        sLevel.Format("%d", nAverageLevel / m_nCharacters);
    }

    highRequests = g_pChitin->cDimm.RequestsPendingCount(CDimm::PRIORITY_HIGH);
    mediumRequests = g_pChitin->cDimm.RequestsPendingCount(CDimm::PRIORITY_MEDIUM);
    lowRequests = g_pChitin->cDimm.RequestsPendingCount(CDimm::PRIORITY_LOW);

    cnt = 0;
    while (cnt < CINFGAME_MAX_AREAS) {
        if (m_gameAreas[cnt] == NULL) {
            break;
        }
        cnt++;
    }

    if (cnt >= CINFGAME_MAX_AREAS) {
        return NULL;
    }

    m_gameAreas[cnt] = new CGameArea(cnt);
    if (m_gameAreas[cnt] == NULL) {
        return NULL;
    }

    if (nTravelScreenImageToUse != -1) {
        m_nTravelScreenImageToUse = nTravelScreenImageToUse;
    } else {
        m_nTravelScreenImageToUse = rand() % 5;
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetListenToJoinOption() == TRUE
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineLoad) {
            g_pBaldurChitin->GetBaldurMessage()->ResourceSuggestLoad(areaName,
                1010,
                m_nTravelScreenImageToUse | (bProgressBarInPlace | bProgressBarRequired) << 8);
        }
        SleepEx(200, FALSE);
    }

    g_pChitin->m_bDisplayStale = TRUE;
    WaitForEngine(TRUE);

    nTotalCachingRequirements = CachingRequirements(areaName);
    if (bProgressBarRequired == TRUE || nTotalCachingRequirements > 0) {
        bProgressBarRequired = TRUE;
        nTotalCachingRequirements += 5000000;
    } else {
        bProgressBarRequired = FALSE;
    }

    nTotalCachingRequirements += 3000000;

    if (bProgressBarInPlace == TRUE) {
        if (bProgressBarRequired == TRUE) {
            g_pChitin->cProgressBar.AddActionTarget(nTotalCachingRequirements - 3000000);
        } else {
            ProgressBarCallback(3000000 - nTotalCachingRequirements, FALSE);
        }
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !g_pChitin->cNetwork.GetSessionHosting()
        && !g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(areaName, 1010, TRUE, FALSE, TRUE)) {
        g_pChitin->cNetwork.CloseSession(TRUE);
        m_bInDestroyGame = TRUE;
        delete m_gameAreas[cnt];
        m_gameAreas[cnt] = NULL;
        m_bInDestroyGame = FALSE;
        return NULL;
    }

    if (bProgressBarRequired == TRUE
        && !bProgressBarInPlace) {
        field_1B86 = areaName;
        g_pChitin->SetProgressBar(TRUE,
            9886,
            0,
            0,
            TRUE,
            9885,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = nTotalCachingRequirements;
    }

    CacheResFileWithResource(areaName);

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (!g_pChitin->cNetwork.GetSessionHosting()) {
            if (bProgressBarInPlace || bProgressBarRequired) {
                if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                    g_pChitin->cProgressBar.m_nWaitingReason = 16513;
                    g_pChitin->cProgressBar.m_bWaiting = TRUE;
                }

                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(areaName, 1010, FALSE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    g_pChitin->SetProgressBar(FALSE,
                        0,
                        0,
                        0,
                        FALSE,
                        0,
                        FALSE,
                        0,
                        FALSE,
                        FALSE,
                        255);
                    m_bInDestroyGame = TRUE;
                    delete m_gameAreas[cnt];
                    m_gameAreas[cnt] = NULL;
                    m_bInDestroyGame = FALSE;
                }

                if (bProgressBarInPlace || bProgressBarRequired) {
                    g_pChitin->cProgressBar.m_bWaiting = FALSE;
                    g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();
                }
            }
        }

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            m_gameAreas[cnt]->m_nInitialAreaID = 0;
            g_pBaldurChitin->GetObjectGame()->m_nAreaFirstObject = -1;

            // TODO: Unclear.
            DWORD idPlayer = -1;
            BOOLEAN bIDInUse = TRUE;
            while (bIDInUse) {
                bIDInUse = g_pChitin->cNetwork.FindPlayerLocationByID(idPlayer, FALSE) != -1;
                for (INT nArea = 0; nArea < CINFGAME_MAX_AREAS; nArea++) {
                    if (m_gameAreas[nArea] != NULL
                        && m_gameAreas[nArea]->m_nInitialAreaID == idPlayer) {
                        bIDInUse = TRUE;
                        break;
                    }
                }
                if (bIDInUse) {
                    idPlayer--;
                }
            }

            m_gameAreas[cnt]->m_nInitialAreaID = idPlayer;
            m_nUniqueAreaID = idPlayer;
        }
    }

    WaitForEngine(TRUE);

    m_bInLoadArea = TRUE;

    areaData.SetResRef(CResRef(areaName), TRUE, TRUE);
    m_gameAreas[cnt]->m_resRef = areaName;
    m_gameAreas[cnt]->m_cGameAreaNotes.m_rArea = CResRef(areaName);
    m_gameAreas[cnt]->Unmarshal(areaData.GetData(),
        areaData.GetDataSize(),
        areaName,
        bProgressBarInPlace);

    areaData.ReleaseData();

    if (g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE
        && !g_pBaldurChitin->GetObjectGame()->m_bInLoadGame) {
        m_gameAreas[cnt]->SortLists();
        m_gameAreas[cnt]->EnterSpawn();
    }

    POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
    while (pos != NULL) {
        CGameSprite* pSprite;
        LONG iSprite = reinterpret_cast<LONG>(m_lstGlobalCreatures.GetNext(pos));
        BYTE rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->m_currentArea == areaName) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 4774
                UTIL_ASSERT(pSprite->GetArea() == NULL);

                rc = m_cObjectArray.GetDeny(iSprite,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) != 0
                        && pSprite->GetAnimation()->CanLieDown()) {
                        pSprite->AddToArea(m_gameAreas[cnt],
                            pSprite->GetPos(),
                            0,
                            CGameObject::LIST_BACK);
                    } else {
                        pSprite->AddToArea(m_gameAreas[cnt],
                            pSprite->GetPos(),
                            0,
                            CGameObject::LIST_FRONT);
                    }
                    m_cObjectArray.ReleaseDeny(iSprite,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    m_bInLoadArea = FALSE;

    m_gameAreas[cnt]->m_nFirstObject = g_pBaldurChitin->GetObjectGame()->m_nAreaFirstObject;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->ChangeControlOnLoadGame();
    }

    if (bProgressBarRequired) {
        if (g_pChitin->field_4C == 1 && !g_pChitin->m_bExitRSThread) {
            int v1 = highRequests + mediumRequests + lowRequests;
            int v6 = 0;
            g_pChitin->cDimm.MoveRequests(CDimm::PRIORITY_HIGH, CDimm::PRIORITY_LOW, highRequests);
            g_pChitin->cDimm.MoveRequests(CDimm::PRIORITY_MEDIUM, CDimm::PRIORITY_LOW, mediumRequests);
            g_pChitin->cDimm.MoveRequests(CDimm::PRIORITY_LOW, CDimm::PRIORITY_LOW, lowRequests);
            int v2 = g_pChitin->cDimm.RequestsPendingCount() - v1;
            int v3 = v2;
            while (v3 > 0) {
                if (v6 >= 100) {
                    break;
                }

                if (!g_pChitin->cDimm.ResumeServicing()
                    || g_pChitin->cDimm.MemoryAlmostFull()) {
                    int v4 = v1 - g_pChitin->cDimm.RequestsPendingCount();
                    g_pChitin->cDimm.ReduceServicedList();
                    v1 = g_pChitin->cDimm.RequestsPendingCount() + v4;
                    g_pChitin->cDimm.ResumeServicing();
                }

                g_pChitin->cDimm.field_0 = 0;
                g_pChitin->cDimm.field_4 = 0;
                SleepEx(50, FALSE);

                int v5 = g_pChitin->cDimm.RequestsPendingCount() - v1;
                if (v5 != v3) {
                    g_pChitin->cProgressBar.m_nActionProgress += (v3 - v5) * (5000000 / v2);
                    v3 = v5;
                    g_pChitin->m_bDisplayStale = TRUE;
                }

                v6++;
            }
        }

        if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
            g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
            g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
            g_pChitin->m_bDisplayStale = TRUE;
            WaitForEngine(TRUE);
            g_pChitin->SetProgressBar(FALSE,
                0,
                0,
                0,
                FALSE,
                0,
                FALSE,
                0,
                FALSE,
                FALSE,
                255);
        }
    }

    m_nTravelScreenImageToUse = rand() % 5;

    return m_gameAreas[cnt];
}

// 0x5AC110
BOOLEAN CInfGame::CanSaveGame(STRREF& strError, BOOLEAN bInRestGame, BOOLEAN bInStore)
{
    if (!bInRestGame
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !g_pChitin->cNetwork.GetSessionHosting()) {
        // "Client machines are not allowed to save in a multiplayer game."
        strError = 17194;
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 386
        || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 1282) {
        // "Cannot save game while in dialog mode."
        strError = 19253;
        return FALSE;
    }

    if (g_pBaldurChitin->GetScreenWorld()->m_comingOutOfDialog > 0 && !bInStore) {
        // "Cannot save game while in dialog mode."
        strError = 19253;
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog == TRUE) {
        // "Cannot save because dialog is about to start."
        strError = 19254;
        return FALSE;
    }

    if (g_pBaldurChitin->GetScreenWorld()->m_bHardPaused == TRUE) {
        // "Cannot save during a rest, chapter information or movie."
        strError = 19255;
        return FALSE;
    }

    if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineProjector
        || g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineChapter
        || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene) {
        // "Cannot save during a rest, chapter information or movie."
        strError = 19255;
        return FALSE;
    }

    if (!bInRestGame
        && g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineStore) {
        // "Cannot save while someone is in the Store Screen."
        strError = 10841;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && !bInRestGame
        && CountNPCs() > 0) {
        // "Cannot save while someone is in the Store Screen."
        strError = 10841;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && !bInRestGame
        && g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->CountReadyPlayers() > 0) {
        // "Cannot save while someone is levelling up."
        strError = 41397;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && !bInRestGame
        && g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->CountViewedCharacters() > 0) {
        // "Cannot save while someone is in the Inventory Screen."
        strError = 26546;
        return FALSE;
    }

    for (BYTE cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            if (!m_gameAreas[cnt]->CanSaveGame(strError)) {
                return FALSE;
            }
        }
    }

    strError = -1;
    return TRUE;
}

// 0x5AC340
BOOL CInfGame::BackupQuickSave()
{
    BOOL bResult = FALSE;

    m_sSaveGame = QUICK_SAVE_BACKUP_NAME;
    CScreenCharacter::SAVE_NAME = QUICK_SAVE_BACKUP_NAME;

    CString sDirSave = GetDirSave();
    if (g_pChitin->cDimm.DirectoryRemoveFiles(sDirSave)) {
        m_sSaveGame = QUICK_SAVE_NAME;
        CScreenCharacter::SAVE_NAME = QUICK_SAVE_NAME;
        bResult = g_pChitin->cDimm.DirectoryCopyFiles(GetDirSave(), sDirSave);
    }

    return bResult;
}

// 0x5AC430
BOOL CInfGame::SaveGame(unsigned char bProgressBarRequired, unsigned char bProgressBarInPlace, unsigned char bSaveScreen)
{
    BYTE* pGame = NULL;
    DWORD nGame = 0;
    const BOOLEAN bProgressBar = bProgressBarRequired || bProgressBarInPlace;


    for (BYTE nArea = 0; nArea < CINFGAME_MAX_AREAS; nArea++) {
        if (m_gameAreas[nArea] != NULL) {
            m_gameAreas[nArea]->SaveMusicPosition();
        }
    }

    const BOOLEAN bMultiplayerHostSave = g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pBaldurChitin->m_pEngineMultiPlayer != NULL
        && g_pBaldurChitin->m_pEngineMultiPlayer->m_bMultiplayerStartup == FALSE;

    if (bMultiplayerHostSave) {
        g_pBaldurChitin->m_pEngineWorld->StartSaveGameMultiplayerHost();
        g_pBaldurChitin->GetBaldurMessage()->UpdateDemandCharacters(TRUE,
            0,
            bProgressBar);
        g_pBaldurChitin->GetBaldurMessage()->PurgeDroppedPlayers();
        g_pBaldurChitin->GetBaldurMessage()->BroadcastUpdateCharacterSlot(TRUE,
            0,
            bProgressBar);
        g_pBaldurChitin->GetBaldurMessage()->RequestClientSignal(CBaldurMessage::SIGNAL_END_MAJOR_EVENT);
    }

    if (bSaveScreen != FALSE) {
        m_bSaveScreen = TRUE;
    }

    Icewind586B70::Instance()->SaveSummonLinks();

    CBaldurEngine* pActiveEngine = g_pBaldurChitin->GetActiveEngine();
    pActiveEngine->CancelEngine();
    pActiveEngine->GetManager()->InvalidateRect(NULL);
    g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();

    // TODO INCOMPLETE: 0x5AC430 also drives disk-space failure shutdown,
    // progress-bar target accounting, and area/store resource writes. The
    // recovered path below keeps the multiplayer hard-pause/signaling behavior
    // and existing game marshal/write flow.
    for (BYTE nStore = 0; nStore < CINFGAME_MAX_AREAS; nStore++) {
        if (m_aServerStore[nStore] != NULL) {
            m_aServerStore[nStore]->Marshal(m_sTempDir);
        }
    }

    Marshal(&pGame, &nGame, bProgressBar);
    BOOL bResult = FALSE;

    if (pGame != NULL && nGame != 0) {
        CGameFile cGameFile;
        cGameFile.SetResRef(CResRef("ICEWIND2"), FALSE, TRUE);

        CRes* pRes = static_cast<CRes*>(cGameFile.GetRes());
        if (pRes != NULL) {
            bResult = pRes->Write(m_sTempDir, pGame, nGame);
        }
    }

    if (pGame != NULL) {
        delete[] pGame;
    }

    for (BYTE nArea = 0; nArea < CINFGAME_MAX_AREAS; nArea++) {
        CGameArea* pArea = m_gameAreas[nArea];
        if (pArea != NULL && (pArea->m_header.m_flags & 0x1) == 0) {
            BYTE* pAreaData = NULL;
            DWORD nAreaData = 0;
            pArea->Marshal(&pAreaData, &nAreaData, bProgressBar);
            if (pAreaData != NULL && nAreaData != 0) {
                CAreaFile cAreaFile;
                cAreaFile.SetResRef(pArea->m_resRef, FALSE, TRUE);

                CRes* pRes = static_cast<CRes*>(cAreaFile.GetRes());
                if (pRes != NULL) {
                    bResult = pRes->Write(m_sTempDir, pAreaData, nAreaData) && bResult;
                }
            } else {
                bResult = FALSE;
            }

            if (pAreaData != NULL) {
                delete[] pAreaData;
            }
        }
    }

    m_cWorldMap.Marshal(m_sTempDir);

    g_pBaldurChitin->GetTlkTable().m_override.Save();
    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    if (bResult) {
        CString sDirSave = GetDirSave();
        bResult = g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)
            && g_pChitin->cDimm.DirectoryCopyFiles(m_sTempDir, m_sTempSaveDir)
            && g_pChitin->cDimm.CompressDirectory(m_sTempSaveDir)
            && g_pChitin->cDimm.DirectoryRemoveFiles(sDirSave)
            && g_pChitin->cDimm.DirectoryCopyFiles(m_sTempSaveDir, sDirSave);
        g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir);
    }

    g_pBaldurChitin->GetTlkTable().OpenOverride(CString("temp/default.toh"),
        CString("temp/default.tot"));

    EnterCriticalSection(&(g_pChitin->m_critSectDimm));
    g_pChitin->cDimm.m_cKeyTable.RescanEverything();
    LeaveCriticalSection(&(g_pChitin->m_critSectDimm));

    SaveOptions();
    g_pBaldurChitin->SaveOptions();

    m_nLastSaveTime = m_worldTime.m_gameTime;

    if (bMultiplayerHostSave) {
        while (g_pBaldurChitin->m_pEngineWorld->m_bHardPaused == TRUE
            && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pBaldurChitin->m_pEngineWorld->CheckEndOfHardPause();
            g_pBaldurChitin->GetBaldurMessage()->HandleBlockingMessages();
            g_pChitin->m_bDisplayStale = TRUE;
            if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineWorld) {
                SynchronousUpdate();
            } else {
                SleepEx(60, FALSE);
            }
        }

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();
        }
    }


    return bResult;
}

// 0x5A3230
void CInfGame::Marshal(BYTE** pGame, DWORD* nGame, BOOLEAN bProgressBarInPlace)
{
    UTIL_ASSERT(pGame != NULL && nGame != NULL);

    CSavedGamePartyCreature partyCreatures[CINFGAME_MAXCHARACTERS];
    BYTE* partyCreatureData[CINFGAME_MAXCHARACTERS];
    DWORD partyCreatureDataSize[CINFGAME_MAXCHARACTERS];
    SHORT nViewedPartyMember = 0;
    SHORT nPartyCreatures = 0;

    memset(partyCreatureData, 0, sizeof(partyCreatureData));
    memset(partyCreatureDataSize, 0, sizeof(partyCreatureDataSize));

    for (SHORT nSlot = 0; nSlot < CINFGAME_MAXCHARACTERS; nSlot++) {
        LONG nCharacterId = m_characters[nSlot];
        if (nCharacterId == CGameObjectArray::INVALID_INDEX) {
            continue;
        }

        CGameSprite* pSprite = NULL;
        BYTE rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        if (rc != CGameObjectArray::SUCCESS || pSprite == NULL) {
            UTIL_ASSERT(FALSE);
            continue;
        }

        pSprite->Marshal(partyCreatures[nPartyCreatures], FALSE);
        partyCreatureData[nPartyCreatures] = reinterpret_cast<BYTE*>(partyCreatures[nPartyCreatures].m_creatureOffset);
        partyCreatureDataSize[nPartyCreatures] = partyCreatures[nPartyCreatures].m_creatureSize;

        if (pSprite->m_pArea != NULL && pSprite->m_pArea == GetVisibleArea()) {
            nViewedPartyMember = nPartyCreatures;
        }

        m_cObjectArray.ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        nPartyCreatures++;
    }

    CVariable* pVariables = NULL;
    WORD nVariables = 0;
    m_variables.Marshal(&pVariables, &nVariables);
    for (WORD nIndex = 0; nIndex < nVariables && nIndex < 8; nIndex++) {
    }

    WORD nJournalEntries = m_cJournal.CountEntries();

    DWORD nGameSize = 8 + sizeof(CSavedGameHeader);
    nGameSize += nPartyCreatures * sizeof(CSavedGamePartyCreature);
    for (SHORT nIndex = 0; nIndex < nPartyCreatures; nIndex++) {
        nGameSize += partyCreatureDataSize[nIndex];
    }
    nGameSize += nVariables * sizeof(CVariable);
    nGameSize += nJournalEntries * sizeof(CSavedGameJournalEntry);

    // Familiars (0x5A35AD): a fixed 0x190 block -- 9 default resrefs (0x48), the
    // packed-data offset (0x04) and the 81 per-list counts (0x144) -- plus eight
    // bytes for every resref held in the 9x9 lists.  The block is unconditional:
    // an empty game still writes the fixed part, which is why a GAM the original
    // engine wrote always carries a non-zero familiars offset.
    nGameSize += 0x190;
    for (int nAlignment = 0; nAlignment < 9; nAlignment++) {
        for (int nLevel = 0; nLevel < 9; nLevel++) {
            nGameSize += m_familiarResRefs[nAlignment][nLevel].GetCount() * 8;
        }
    }

    // Pocket-plane stored locations (0x5A35F8): the count dword plus, per record,
    // the 0x1C-byte fixed header and the area name's bytes.  Also unconditional --
    // Unmarshal's final `cnt == nGame` assert requires the trailing dword even
    // when the list is empty.
    DWORD nStoredLocationsSize = 4;
    for (POSITION pos = field_4BDC.GetHeadPosition(); pos != NULL;) {
        CSavedGameStoredLocation* pLocation = field_4BDC.GetNext(pos);
        nStoredLocationsSize += 0x1C + pLocation->m_areaName.GetLength();
    }
    nGameSize += nStoredLocationsSize;

    *pGame = new BYTE[nGameSize];
    *nGame = nGameSize;
    memset(*pGame, 0, nGameSize);
    memcpy(*pGame, "GAMEV2.2", 8);

    CSavedGameHeader* pHeader = reinterpret_cast<CSavedGameHeader*>(*pGame + 8);
    pHeader->m_worldTime = m_worldTime.m_gameTime / CTimerWorld::TIMESCALE_MSEC_PER_SEC;
    pHeader->m_curFormation = 0;
    for (SHORT nFormation = 0; nFormation < CGAMESAVE_NUM_QUICK_FORMATIONS; nFormation++) {
        pHeader->m_quickFormations[nFormation] = m_gameSave.m_quickFormations[nFormation];
        if (m_gameSave.m_quickFormations[nFormation] == m_gameSave.m_curFormation) {
            pHeader->m_curFormation = nFormation;
        }
    }
    pHeader->m_nPartyGold = m_gameSave.m_nPartyGold;
    pHeader->m_nPCAreaViewed = nViewedPartyMember;
    pHeader->m_partyCreatureTableOffset = 8 + sizeof(CSavedGameHeader);
    pHeader->m_partyCreatureTableCount = nPartyCreatures;
    m_gameSave.m_cResCurrentWorldArea.GetResRef(pHeader->m_currentWorldArea);
    pHeader->m_currentLink = m_gameSave.m_nCurrentWorldLink;
    pHeader->m_reputation = m_nReputation;
    pHeader->m_versionNumber = 3;

    if (m_pGameAreaMaster != NULL) {
        m_pGameAreaMaster->m_resRef.GetResRef(pHeader->m_masterArea);
    }

    DWORD nOffset = pHeader->m_partyCreatureTableOffset
        + nPartyCreatures * sizeof(CSavedGamePartyCreature);

    for (SHORT nIndex = 0; nIndex < nPartyCreatures; nIndex++) {
        CSavedGamePartyCreature savedCreature = partyCreatures[nIndex];
        savedCreature.m_creatureOffset = nOffset;
        memcpy(*pGame + pHeader->m_partyCreatureTableOffset + nIndex * sizeof(CSavedGamePartyCreature),
            &savedCreature,
            sizeof(savedCreature));

        if (partyCreatureDataSize[nIndex] != 0) {
            memcpy(*pGame + nOffset, partyCreatureData[nIndex], partyCreatureDataSize[nIndex]);
            nOffset += partyCreatureDataSize[nIndex];
        }

        delete[] partyCreatureData[nIndex];
    }

    if (nVariables != 0) {
        pHeader->m_globalVariablesOffset = nOffset;
        pHeader->m_globalVariablesCount = nVariables;
        memcpy(*pGame + nOffset, pVariables, nVariables * sizeof(CVariable));
        nOffset += nVariables * sizeof(CVariable);
    }

    delete[] pVariables;

    // The journal offset and count are written unconditionally (0x5A3BAA): the
    // binary stores the running offset, then asks the journal for its entry count
    // and marshals into the block.  An empty journal still records the offset.
    pHeader->m_journalEntriesOffset = nOffset;
    pHeader->m_journalEntriesCount = nJournalEntries;
    m_cJournal.Marshal(reinterpret_cast<CSavedGameJournalEntry*>(*pGame + nOffset));
    nOffset += nJournalEntries * sizeof(CSavedGameJournalEntry);

    // --- Familiars (0x5A3BDA) ------------------------------------------------
    // The nightmare flag shares this stretch of the header and is stored first.
    pHeader->m_nNightmareMode = m_cOptions.m_nNightmareMode;
    pHeader->m_familiarsOffset = nOffset;

    BYTE* pFamiliars = *pGame + nOffset;
    memcpy(pFamiliars, m_defaultFamiliarResRefs, sizeof(m_defaultFamiliarResRefs));

    // The packed resref data begins after the fixed 0x190 block, and the offset
    // stored at +0x48 is measured from the start of the buffer.
    nOffset += 0x190;
    *reinterpret_cast<DWORD*>(pFamiliars + 0x48) = nOffset;

    DWORD* pCounts = reinterpret_cast<DWORD*>(pFamiliars + 0x4C);
    for (int nAlignment = 0; nAlignment < 9; nAlignment++) {
        for (int nLevel = 0; nLevel < 9; nLevel++) {
            *pCounts = m_familiarResRefs[nAlignment][nLevel].GetCount();

            for (POSITION pos = m_familiarResRefs[nAlignment][nLevel].GetHeadPosition(); pos != NULL;) {
                CResRef* pResRef = m_familiarResRefs[nAlignment][nLevel].GetNext(pos);
                memcpy(*pGame + nOffset, pResRef->GetResRef(), 8);
                nOffset += 8;
            }

            pCounts++;
        }
    }

    UTIL_ASSERT(nOffset <= nGameSize);

    // --- Pocket-plane stored locations (0x5A3CA1) ----------------------------
    // Written last and carrying no header offset: Unmarshal finds the section by
    // its own size accumulator.  Each record is the total size, the name length,
    // the packed fields, then the raw area-name bytes.
    BYTE* pStored = *pGame + nOffset;
    *reinterpret_cast<DWORD*>(pStored) = field_4BDC.GetCount();

    BYTE* pRecord = pStored + 4;
    for (POSITION pos = field_4BDC.GetHeadPosition(); pos != NULL;) {
        CSavedGameStoredLocation* pLocation = field_4BDC.GetNext(pos);
        int nNameLen = pLocation->m_areaName.GetLength();

        *reinterpret_cast<DWORD*>(pRecord + 0x00) = 0x18 + nNameLen;
        *reinterpret_cast<DWORD*>(pRecord + 0x04) = nNameLen;
        *reinterpret_cast<DWORD*>(pRecord + 0x08) = pLocation->m_flags;
        *reinterpret_cast<DWORD*>(pRecord + 0x0C) = pLocation->m_value;
        *reinterpret_cast<DWORD*>(pRecord + 0x0D) = pLocation->field_9;
        *reinterpret_cast<DWORD*>(pRecord + 0x11) = pLocation->field_D;
        *reinterpret_cast<DWORD*>(pRecord + 0x15) = pLocation->field_11;
        *reinterpret_cast<WORD*>(pRecord + 0x19) = pLocation->field_15;
        pRecord[0x1B] = pLocation->field_17;
        memcpy(pRecord + 0x1C, static_cast<const char*>(pLocation->m_areaName), nNameLen);

        pRecord += 0x1C + nNameLen;
    }

    nOffset += nStoredLocationsSize;

    UTIL_ASSERT(nOffset == nGameSize);
}

// GAM party/NPC record version upgraders, used by CInfGame::Unmarshal when a
// loaded save predates the current V2.2 record layout.  Each copies an older
// fixed-size record into the next version's larger layout in source order,
// leaving the fields introduced by the newer version untouched -- the caller
// zero-fills the destination buffer first, so the new fields read back as zero.
// IWD2 only ever writes V2.2 saves, so these conversions are dead for native
// saves but are reproduced from the binary.  Offsets are the ground truth
// (the destination gaps are the fields each version added).

// 0x5A79E0
// V2.0 party-creature record (0x180 bytes) -> V2.1 layout (0x220 bytes).
static void UpgradePartyCreatureRecord20To21(const BYTE* pSrc, BYTE* pDst)
{
    memcpy(pDst + 0x000, pSrc + 0x000, 0x94); // header .. quick-weapon item slots
    memcpy(pDst + 0x09C, pSrc + 0x094, 0x08); // quick-weapon ability slots
    memcpy(pDst + 0x0AC, pSrc + 0x09C, 0x18); // quick spells
    memcpy(pDst + 0x0DC, pSrc + 0x0B4, 0x0C); // quick items
    memcpy(pDst + 0x160, pSrc + 0x0C0, 0xC0); // name, stats and kill records
}

// 0x5A7BF0
// V2.1 party-creature record (0x220 bytes) -> V2.2 layout (0x340 bytes), the
// in-memory CSavedGamePartyCreature.
static void UpgradePartyCreatureRecord21To22(const BYTE* pSrc, BYTE* pDst)
{
    memcpy(pDst + 0x000, pSrc + 0x000, 0xDC); // header, quick weapons, leading quick spells
    memcpy(pDst + 0x0FE, pSrc + 0x0DC, 0x3C); // quick items, leading quick innates
    memcpy(pDst + 0x152, pSrc + 0x118, 0x30); // leading quick songs
    memcpy(pDst + 0x19A, pSrc + 0x148, 0x18); // leading custom-button section
    memcpy(pDst + 0x1BE, pSrc + 0x160, 0xC0); // name, stats and kill records
}

// 0x5A7E40
BOOL CInfGame::Unmarshal(BYTE* pGame, LONG nGame, BOOLEAN bProgressBarInPlace)
{

    if (pGame == NULL || nGame < 1) {
        return FALSE;
    }

    // GAM V2.0 header:
    // 0x00: Signature "GAME" (4 bytes)
    // 0x04: Version "V2.0" (4 bytes)
    // 0x08: Elapsed game time in seconds (4 bytes)
    // 0x0C: Selected formation (2 bytes)
    // 0x0E: Formation buttons 1-5 (5 x 2 = 10 bytes)
    // 0x18: Party gold (4 bytes)
    // 0x1C: View area of party member (2 bytes)
    // 0x1E: Weather (2 bytes)
    // 0x20: Party members offset (4 bytes)
    // 0x24: Number of party members (4 bytes)
    // 0x28: Party inventory offset (4 bytes)
    // 0x2C: Number of party inventory items (4 bytes)
    // 0x30: Non-party characters offset (4 bytes)
    // 0x34: Number of non-party characters (4 bytes)
    // 0x38: Global variables offset (4 bytes)
    // 0x3C: Number of global variables (4 bytes)
    // 0x40: World area RESREF (8 bytes)
    // 0x48: Current link (4 bytes)
    // 0x4C: Number of journal entries (4 bytes)
    // 0x50: Journal entries offset (4 bytes)
    // 0x54: Reputation (4 bytes)
    // 0x58: Master area RESREF (8 bytes)
    // 0x60: Configuration (4 bytes)
    // 0x64: Unknown section count (4 bytes)
    // 0x68: Unknown section offset (4 bytes)
    // 0x6C: Nightmare mode (4 bytes)
    // 0x70: Unknown (68 bytes)
    // Total header: 0xB4 bytes

    if (nGame < 0xB4) {
        return FALSE;
    }

    int* pData = reinterpret_cast<int*>(pGame);

    // Validate signature
    if (memcmp(pGame, "GAME", 4) != 0) {
        return FALSE;
    }

    // Version.  The signature carries the format version ("GAMEV2.0/1/2"); the
    // loader upgrades older party-creature records to the V2.2 layout on the
    // fly.  nVersionMode mirrors the binary's internal tag (0x14=V2.0,
    // 0x15=V2.1, 0x16=V2.2).
    int nVersionMode = 0x14;
    if (memcmp(pGame, "GAMEV2.1", 8) == 0) {
        nVersionMode = 0x15;
    } else if (memcmp(pGame, "GAMEV2.2", 8) == 0) {
        nVersionMode = 0x16;
    } else if (memcmp(pGame + 4, "V2.", 3) != 0) {
        return FALSE;
    }

    // Running size accumulator.  The loader walks the GAM strictly in physical
    // order, advancing cnt past every section and asserting cnt <= nGame at each
    // step; the final cnt must equal nGame (the whole buffer is consumed).  The
    // pocket-plane section carries no header offset -- it sits at pGame + cnt --
    // so the accumulator is the only thing that can locate it.  (binary: the
    // uStack_6f4 local, seeded with the 0xB4 header size.)
    DWORD cnt = 0xB4;

    // Game time: pData[2] is elapsed seconds, convert to game ticks (*15)
    m_worldTime.m_gameTime = pData[2] * 15;

    // Selected formation: GAM offset 0x0C is the INDEX (0-4) into the five
    // formation buttons at 0x0E, not a formation id.  The active formation is
    // quickFormations[index].  (binary 0x5A7FD5:
    //   curFormation = *(WORD*)(pGame + 0x0E + 2 * (SHORT)pGame[0x0C]).)
    SHORT nFormationIndex = *reinterpret_cast<SHORT*>(pGame + 0x0C);
    m_gameSave.m_curFormation = *reinterpret_cast<SHORT*>(pGame + 0x0E + 2 * nFormationIndex);

    // Formation buttons 1-5 (offset 0x0E-0x16)
    memcpy(m_gameSave.m_quickFormations, pGame + 0x0E, sizeof(m_gameSave.m_quickFormations));

    // Party gold (offset 0x18)
    m_gameSave.m_nPartyGold = *reinterpret_cast<DWORD*>(pGame + 0x18);

    // Nightmare mode (offset 0x6C)
    m_cOptions.m_nNightmareMode = pData[0x1B];

    // Heart of Fury (nightmare mode == 1) forces the maximum difficulty level on
    // load and persists it to the INI, so a HoF save can never be reopened at a
    // lower difficulty.  The difficulty multipliers are reset on the host only.
    // The m_nDifficultyLevel != 5 guard makes the rewrite idempotent.
    // (binary 0x5A8035.)
    if (m_cOptions.m_nNightmareMode == 1 && m_cOptions.m_nDifficultyLevel != 5) {
        m_cOptions.m_nDifficultyLevel = 5;
        if (g_pBaldurChitin->cNetwork.m_bIsHost == 1) {
            m_cOptions.m_nDifficultyMultiplier = 100;
            m_cOptions.m_nMPDifficultyMultiplier = 100;
        }

        CString sDifficulty;
        sDifficulty.Format("%d", 5);
        WritePrivateProfileStringA("Game Options", "Difficulty Level", sDifficulty,
            g_pBaldurChitin->GetIniFileName());
    }

    // View area of party member (offset 0x1C)
    SHORT nViewAreaMember = *reinterpret_cast<SHORT*>(pGame + 0x1C);

    // Weather flags (offset 0x1E)
    WORD wWeatherFlags = *reinterpret_cast<WORD*>(pGame + 0x1E);

    if (bProgressBarInPlace) {
        ProgressBarCallback(312500, FALSE);
    }

    if (bProgressBarInPlace && pData[9] > 1) {
        // ProgressBarCallback for each area
    }

    g_pBaldurChitin->m_pEngineWorld->m_weather.Unmarshal(wWeatherFlags);

    // Number of party members (offset 0x24)
    int nPartyMembers = pData[9]; // offset 0x24 / 4 = index 9
    int partyOffset = pData[8];   // offset 0x20 / 4 = index 8

    // Number of non-party characters (offset 0x34)
    int nNonParty = pData[13]; // offset 0x34 / 4 = index 13

    // World area (offset 0x40, 8 bytes RESREF)
    char areaResRef[9];
    memcpy(areaResRef, pGame + 0x40, 8);
    areaResRef[8] = 0;
    m_gameSave.m_cResCurrentWorldArea = CResRef(areaResRef);
    m_gameSave.m_nCurrentWorldLink = static_cast<DWORD>(pData[0x12]);

    // Area notes visibility, from the configuration flags (offset 0x60, bit 7,
    // inverted).
    m_bShowAreaNotes = (~(pData[0x18] >> 7)) & 1;

    // Reputation (offset 0x54)
    m_nReputation = static_cast<SHORT>(pData[21]); // offset 0x54 / 4 = 21

    // Master area (offset 0x58, 8 bytes)
    // Current link (offset 0x48)

    if (bProgressBarInPlace) {
        ProgressBarCallback(312500, FALSE);
    }

    // Master area (header offset 0x58): load it if the save names one.
    if (pGame[0x58] != '\0') {
        char masterArea[9];
        memcpy(masterArea, pGame + 0x58, 8);
        masterArea[8] = '\0';
        LoadArea(CString(masterArea), 0xFF, FALSE, bProgressBarInPlace);
    }

    // --- Non-party / "global" creatures (header 0x34 count, 0x30 offset) ------
    // Persistent creatures not in the current area; the binary tracks them in
    // m_lstGlobalCreatures.  They share the party-creature record format (so
    // creOffset/creSize live at +4/+8 in every version) and the on-disk records
    // are contiguous at the version's record size.
    //   The creature-loading body is NOT reproduced: the slot-3 test save holds
    //   zero of these so it cannot be exercised, and the record-sourcing in the
    //   Ghidra lift is ambiguous (it appears to alias the party base).  Only the
    //   size accounting is recovered, which the final cnt == nGame assert needs;
    //   it is exact for both record kinds (resref records carry creSize == 0).
    if (nNonParty > 0) {
        int recordSize = (nVersionMode == 0x16) ? 0x340
                       : (nVersionMode == 0x15) ? 0x220
                                                : 0x180;
        BYTE* pRecord = pGame + pData[0x0C]; // 0x30 / 4 = 0x0C
        for (int i = 0; i < nNonParty; i++) {
            cnt += recordSize;
            UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));
            cnt += *reinterpret_cast<DWORD*>(pRecord + 8); // embedded CRE size, 0 if resref
            UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));
            // TODO: instantiate CGameSprite + register in m_lstGlobalCreatures.
            pRecord += recordSize;
        }
    }

    // TODO: Load party members from partyOffset with CCreatureFile
    // Each party member is 0x340 (832) bytes in V2.2, 0x220 in V2.1, 0x180 in V2.0
    if (nPartyMembers > 0 && partyOffset > 0) {
        int memberSize = (nVersionMode == 0x16) ? 0x340
                       : (nVersionMode == 0x15) ? 0x220
                                                : 0x180;

        BYTE* pMember = pGame + partyOffset;
        for (int i = 0; i < nPartyMembers && i < 6; i++) {
            // Upgrade pre-V2.2 records to the canonical 0x340 layout; V2.2 reads
            // the on-disk record in place.  All field offsets below are V2.2.
            CSavedGamePartyCreature canonical;
            BYTE* pRecord;
            if (nVersionMode == 0x14) {
                BYTE record21[0x220] = { 0 };
                UpgradePartyCreatureRecord20To21(pMember, record21);
                UpgradePartyCreatureRecord21To22(record21, reinterpret_cast<BYTE*>(&canonical));
                pRecord = reinterpret_cast<BYTE*>(&canonical);
            } else if (nVersionMode == 0x15) {
                UpgradePartyCreatureRecord21To22(pMember, reinterpret_cast<BYTE*>(&canonical));
                pRecord = reinterpret_cast<BYTE*>(&canonical);
            } else {
                pRecord = pMember;
            }

            // Advance the accumulator past the on-disk record, then (below) past
            // the embedded CRE blob -- the binary's per-member size accounting.
            cnt += memberSize;
            UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));

            // Verified offsets from hex dump:
            // +0x00: u16 selectionState, +0x02: u16 slotIndex (0-based)
            // +0x04: u32 creOffset, +0x08: u32 creSize
            // +0x14: u32 facing, +0x18: char[8] areaRef
            // +0x20: u16 posX, +0x22: u16 posY

            int slotIndex = *reinterpret_cast<unsigned short*>(pRecord + 2);
            int creOffset = *reinterpret_cast<int*>(pRecord + 4);
            int creSize   = *reinterpret_cast<int*>(pRecord + 8);

            // Embedded CRE blob size (0 for a resref-loaded record).
            cnt += creSize;
            UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));
            short facing  = static_cast<short>(*reinterpret_cast<int*>(pRecord + 0x14));

            char areaRef[9];
            memcpy(areaRef, pRecord + 0x18, 8);
            areaRef[8] = 0;

            short posX = *reinterpret_cast<short*>(pRecord + 0x20);
            short posY = *reinterpret_cast<short*>(pRecord + 0x22);

            // Create CGameSprite from embedded CRE data
            if (creOffset > 0 && creSize > 0 && creOffset + creSize <= nGame) {
                BYTE* pCreData = pGame + creOffset;

                if (memcmp(pCreData, "CRE V2.2", 8) == 0) {
                    CGameSprite* pSprite = new CGameSprite(pCreData, creSize, 0,
                        0, -1, 0, 0, INT_MAX,
                        CPoint(posX, posY), facing);

                    if (pSprite != NULL && pSprite->m_id != CGameObjectArray::INVALID_INDEX) {
                        pSprite->SetResRef(CResRef(reinterpret_cast<char*>(pCreData + 0x280)));

                        char szName[SCRIPTNAME_SIZE + 1];
                        memcpy(szName, pRecord + 0x1BE, SCRIPTNAME_SIZE);
                        szName[SCRIPTNAME_SIZE] = '\0';
                        pSprite->m_sName = szName;

                        pSprite->Unmarshal(reinterpret_cast<CSavedGamePartyCreature*>(pRecord), TRUE, bProgressBarInPlace);

                        if (i == nViewAreaMember && pSprite->m_pArea != NULL) {
                            m_visibleArea = pSprite->m_pArea->m_id;
                        }

                        // Party slot -> char-id map for the portrait UI.  The
                        // visibility-map registration and m_characterPortraits /
                        // m_nCharacters bookkeeping are done by AddCharacterToParty
                        // inside pSprite->Unmarshal (called above, before the sprite's
                        // AddToArea); do NOT pre-bump m_nCharacters here -- doing so
                        // tripped AddCharacterToParty's `m_nCharacters >= MAX` guard for
                        // the later party members, so they never registered in the
                        // visibility map (invisible enemies / unrevealed bolt).
                        LONG nIndex = pSprite->m_id;
                        if (slotIndex >= 0 && slotIndex < 6) {
                            m_characters[slotIndex] = nIndex;
                        }
                    }
                }
            }

            pMember += memberSize;
        }
    }

    SelectToolbar();
    SetProtagonist(m_characters[0]);
    // The reputation is forced to the default (100 == 10.0) on load, except for
    // a saved campaign (header 0x64 == 3), which keeps the value read above.
    if (pData[0x19] != 3) {
        ReputationAdjustment(100, TRUE);
    }

    // --- View / interface restore (host, installed) ---------------------------
    // The host rebuilds the world viewport and the two world-screen panels from
    // the saved configuration flags (header 0x60).  Bits 0x20 / 0x40 select one
    // of two mutually-exclusive panel layouts -- each re-derives the view rect
    // from GetNewViewSize() and a CInfinity reference rect, forces the resize,
    // and invalidates the UI -- while bit 0x10 independently snaps the view to
    // the full 800x600 rect.  Gated on m_bIsHost && m_nInstallType.  Untestable
    // on the slot-3 save (its configuration flags are 0, so every branch skips);
    // recovered faithfully from 0x5A8F40.
    if (g_pBaldurChitin->cNetwork.m_bIsHost == 1 && g_pBaldurChitin->m_nInstallType != 0) {
        CScreenWorld* pScreen = g_pBaldurChitin->m_pEngineWorld;
        UTIL_ASSERT(pScreen != NULL);

        CRect rViewSize = pScreen->GetNewViewSize();
        CUIPanel* pPanel1 = pScreen->GetManager()->GetPanel(1);
        CUIPanel* pPanel2 = pScreen->GetManager()->GetPanel(pScreen->GetPanel_22_0());

        DWORD nConfiguration = static_cast<DWORD>(pData[0x18]); // header offset 0x60

        if (nConfiguration & 0x20) {
            pScreen->field_11F0 = 0;
            pScreen->field_11F8 = rViewSize.bottom;
            pPanel1->m_bActive = FALSE;
            CInfinity::stru_8E79B8.bottom = CInfinity::stru_8E79E0.bottom;
            pScreen->SetNewViewSize(
                CRect(rViewSize.left, rViewSize.top, rViewSize.right, CInfinity::stru_8E79E0.bottom),
                FALSE);
            pScreen->GetManager()->InvalidateRect(NULL);
        } else if (nConfiguration & 0x40) {
            pScreen->field_11FC = rViewSize.bottom;
            pScreen->field_11F4 = 0;
            pPanel2->m_bActive = FALSE;
            pPanel1->m_ptOrigin.x = pPanel1->m_ptBaseOrigin.x;
            pPanel1->m_ptOrigin.y = pPanel1->m_ptBaseOrigin.y + pPanel2->m_size.cy;
            pPanel1->m_bActive = TRUE;
            CInfinity::stru_8E79B8.bottom = CInfinity::stru_8E79C8.bottom;
            pScreen->SetNewViewSize(
                CRect(rViewSize.left, rViewSize.top, rViewSize.right, CInfinity::stru_8E79C8.bottom),
                FALSE);
            pScreen->GetManager()->InvalidateRect(NULL);
        }

        if (nConfiguration & 0x10) {
            pScreen->GetManager()->m_bHidden = TRUE;
            pScreen->field_11D0 = rViewSize;
            pScreen->SetNewViewSize(CInfinity::stru_8E7548, FALSE);
        }
    }

    // Party inventory (header 0x2C count, 0x14-byte records): the binary accounts
    // its size here but loads it elsewhere.  (cnt += pData[0x2C] * 0x14.)
    cnt += static_cast<DWORD>(pData[0x0B]) * 0x14; // 0x2C / 4 = 0x0B
    UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));

    DWORD nGlobalVariablesOffset = pData[0x0E];
    DWORD nGlobalVariables = pData[0x0F];
    if (nGlobalVariables != 0
        && nGlobalVariablesOffset > 0
        && nGlobalVariablesOffset + nGlobalVariables * sizeof(CVariable) <= static_cast<DWORD>(nGame)) {
        CVariable* pVariables = reinterpret_cast<CVariable*>(pGame + nGlobalVariablesOffset);
        for (DWORD nIndex = 0; nIndex < nGlobalVariables; nIndex++) {
            m_variables.AddKey(pVariables[nIndex]);
        }
    }
    // Each on-disk global variable is 0x54 bytes (the binary advances cnt one
    // record at a time inside the load loop).
    cnt += nGlobalVariables * 0x54;
    UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));

    // Load journal entries (header offsets 0x4C=count, 0x50=offset)
    DWORD nJournalCount = pData[0x13];  // 0x4C/4 = 0x13
    DWORD nJournalOffset = pData[0x14]; // 0x50/4 = 0x14
    if (nJournalCount > 0 && nJournalOffset > 0 && nJournalOffset + nJournalCount * 12 <= (DWORD)nGame) {
        m_cJournal.ClearAllEntries();
        m_cJournal.Unmarshal(reinterpret_cast<CSavedGameJournalEntry*>(pGame + nJournalOffset), nJournalCount);
        // Each on-disk journal entry is 0xC bytes.
        cnt += nJournalCount * 0xC;
        UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));
    }

    // Familiars (header offset 0x68).  Section at pGame + offset:
    //   +0x00  default familiar resrefs (9 x RESREF = 0x48 bytes)
    //   +0x48  offset (from pGame) to the packed resref data block
    //   +0x4C  per-list counts (9 alignments x 9 levels = 81 DWORDs)
    //   data   resref entries, 8 bytes each, packed in [alignment][level] order
    // and indexed by a single running counter shared across every list.
    DWORD nFamiliarsOffset = static_cast<DWORD>(pData[0x1A]); // 0x68 / 4 = 0x1A
    if (nFamiliarsOffset != 0) {
        BYTE* pFamiliars = pGame + nFamiliarsOffset;
        memcpy(m_defaultFamiliarResRefs, pFamiliars, sizeof(m_defaultFamiliarResRefs));

        DWORD nResRefDataOffset = *reinterpret_cast<DWORD*>(pFamiliars + 0x48);
        DWORD* pCounts = reinterpret_cast<DWORD*>(pFamiliars + 0x4C);
        BYTE* pResRefData = pGame + nResRefDataOffset;

        int nResRef = 0;
        for (int nAlignment = 0; nAlignment < 9; nAlignment++) {
            for (int nLevel = 0; nLevel < 9; nLevel++) {
                for (DWORD nCount = *pCounts; nCount != 0; nCount--) {
                    CResRef* pResRef = new CResRef(
                        reinterpret_cast<char*>(pResRefData + nResRef * 8));
                    m_familiarResRefs[nAlignment][nLevel].AddTail(pResRef);
                    nResRef++;
                }
                pCounts++;
            }
        }

        // 0x190 = default resrefs (0x48) + data offset (0x04) + 81 counts (0x144);
        // then 8 bytes per packed resref entry.
        cnt += 0x190 + static_cast<DWORD>(nResRef) * 8;
        UTIL_ASSERT(cnt <= static_cast<DWORD>(nGame));
    }

    // --- Pocket-plane "stored locations" (header offset 0x70/0x74 mirror it, but
    // the loader reads the section sequentially, at pGame + cnt).  Gated by a
    // version compare that is always satisfied for an IWD2 save: the binary calls
    // _stricmp(version, "V1.1") and parses the section when they differ, and the
    // version is always "V2.x".  ---
    if (CString(reinterpret_cast<char*>(pGame + 4), 4).CompareNoCase("V1.1") != 0) {
        // Drop any locations from a previously-loaded game.
        while (!field_4BDC.IsEmpty()) {
            delete field_4BDC.RemoveHead();
        }

        BYTE* pStored = pGame + cnt;
        int nStored = *reinterpret_cast<int*>(pStored);
        BYTE* pRecord = pStored + 4; // records follow the count
        for (int i = 0; i < nStored; i++) {
            int nTotalSize = *reinterpret_cast<int*>(pRecord + 0x00);
            int nNameLen   = *reinterpret_cast<int*>(pRecord + 0x04);

            CSavedGameStoredLocation* pLocation = new CSavedGameStoredLocation();
            pLocation->m_flags  = *reinterpret_cast<DWORD*>(pRecord + 0x08);
            pLocation->m_value  = pRecord[0x0C];
            pLocation->field_9  = *reinterpret_cast<DWORD*>(pRecord + 0x0D);
            pLocation->field_D  = *reinterpret_cast<DWORD*>(pRecord + 0x11);
            pLocation->field_11 = *reinterpret_cast<DWORD*>(pRecord + 0x15);
            pLocation->field_15 = *reinterpret_cast<WORD*>(pRecord + 0x19);
            pLocation->field_17 = pRecord[0x1B];

            // The area name is a length-prefixed blob at +0x1C (nNameLen bytes).
            char* pBuffer = pLocation->m_areaName.GetBufferSetLength(nNameLen);
            memcpy(pBuffer, pRecord + 0x1C, nNameLen);
            pLocation->m_areaName.ReleaseBuffer(nNameLen);

            field_4BDC.AddTail(pLocation);
            pRecord += nTotalSize + 4;
        }

        // Recompute the section's on-disk size from the loaded list: the count
        // dword plus, per record, the 0x1C-byte fixed header and the name bytes.
        DWORD nStoredSize = 4;
        for (POSITION pos = field_4BDC.GetHeadPosition(); pos != NULL; ) {
            CSavedGameStoredLocation* pLocation = field_4BDC.GetNext(pos);
            nStoredSize += 0x1C + pLocation->m_areaName.GetLength();
        }
        cnt += nStoredSize;
    }

    // The whole buffer must have been consumed exactly.
    UTIL_ASSERT(cnt == static_cast<DWORD>(nGame));

    return TRUE;
}

// 0x5A9680
void CInfGame::ProgressBarCallback(DWORD dwSize, BOOLEAN bInitialize)
{
    if (bInitialize == TRUE) {
        m_dwLastProgressRenderTickCount = GetTickCount() - 250;
        m_dwLastProgressMsgTickCount = GetTickCount() - 5000;
    }

    g_pChitin->cProgressBar.m_nActionProgress += dwSize;

    if (GetTickCount() - m_dwLastProgressMsgTickCount >= 1000) {
        m_dwLastProgressMsgTickCount = GetTickCount();

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pBaldurChitin->GetBaldurMessage()->SendProgressBarStatus(g_pChitin->cProgressBar.m_nActionProgress,
                g_pChitin->cProgressBar.m_nActionTarget,
                g_pChitin->cProgressBar.m_bWaiting,
                g_pChitin->cProgressBar.m_nWaitingReason,
                g_pChitin->cProgressBar.m_bTimeoutVisible,
                g_pChitin->cProgressBar.m_nSecondsToTimeout);
        }
    }

    if (GetTickCount() - m_dwLastProgressRenderTickCount >= 250) {
        m_dwLastProgressRenderTickCount = GetTickCount();

        g_pChitin->m_bDisplayStale = TRUE;
        g_pChitin->cDimm.field_0 = 1;
        g_pChitin->cDimm.field_4 = 1;
        SleepEx(25, TRUE);

        WaitForEngine(FALSE);
    }
}

// 0x5A9780
char CInfGame::VirtualKeyToChar(BYTE nKey)
{
    if (nKey >= 'A' && nKey <= 'Z') {
        return static_cast<char>(nKey);
    }

    if (nKey >= '0' && nKey <= '9') {
        return static_cast<char>(nKey);
    }

    for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
        if (byte_8ABA3C[sym] == nKey) {
            return byte_8ABA24[sym];
        }
    }

    return -1;
}

// 0x5A97D0
BYTE CInfGame::CharToVirtualKey(char ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<BYTE>(ch - 32);
    }

    if (ch >= 'A' && ch <= 'Z') {
        return static_cast<BYTE>(ch);
    }

    if (ch >= '0' && ch <= '9') {
        return static_cast<BYTE>(ch);
    }

    for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
        if (byte_8ABA24[sym] == ch) {
            return byte_8ABA3C[sym];
        }
    }

    return -1;
}

// 0x5A9830
BYTE CInfGame::GetKeyBinding(int index)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7779
    UTIL_ASSERT(index >= 0 && index < CINFGAME_KEYMAP_SIZE);

    if (index >= FIFTY_THREE) {
        return 0;
    }

    // NOTE: Uninline.
    return CharToVirtualKey(off_8AFD24[index][0]);
}

// 0x5A98F0
void CInfGame::LoadKeymap()
{
    CString sKey;

    memset(m_pKeymap, 0, sizeof(m_pKeymap));
    memset(m_pKeymapFlags, 0, sizeof(m_pKeymapFlags));

    // NOTE: Unsigned compare in both loops.
    WORD index;
    for (index = 0; index < FIFTY_THREE; index++) {
        char* buffer = sKey.GetBuffer(128);
        GetPrivateProfileStringA("Keymap",
            off_8AFC50[index],
            off_8AFD24[index],
            buffer,
            128,
            ".\\Keymap.ini");
        sKey.ReleaseBuffer();
        LoadKey(sKey, index);
    }

    for (; index < CINFGAME_KEYMAP_SIZE; index++) {
        char* buffer = sKey.GetBuffer(128);
        if (index >= 199) {
            GetPrivateProfileStringA("Wizard Spells",
                off_8B4F84[index - FIFTY_THREE],
                "",
                buffer,
                128,
                ".\\Keymap.ini");
        } else {
            GetPrivateProfileStringA("Priest Spells",
                off_8B4F84[index - FIFTY_THREE],
                "",
                buffer,
                128,
                ".\\Keymap.ini");
        }
        sKey.ReleaseBuffer();
        LoadKey(sKey, index);
    }
}

// 0x5A9A30
void CInfGame::LoadKey(const CString& sKey, SHORT index)
{
    INT nOffset = 0;
    if (sKey.GetLength() > 0) {
        if (sKey.Find("CTRL-") == 0
            || sKey.Find("ctrl-") == 0
            || sKey.Find("Ctrl-") == 0) {
            nOffset = strlen("CTRL-");
            m_pKeymapFlags[index] = 1;
        }

        if (sKey[nOffset] >= 'a' && sKey[nOffset] <= 'z') {
            m_pKeymap[index] = sKey[nOffset] - 32;
        } else {
            m_pKeymap[index] = sKey[nOffset];

            for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
                if (byte_8ABA24[sym] == sKey[nOffset]) {
                    m_pKeymap[index] = byte_8ABA3C[sym];
                    break;
                }
            }
        }
    }
}

// 0x5A9B00
void CInfGame::SetKey(INT nIndex, BYTE nKey, BYTE nKeyFlag)
{
    if (nIndex >= 0 && nIndex < CINFGAME_KEYMAP_SIZE) {
        m_pKeymap[nIndex] = nKey;
        m_pKeymapFlags[nIndex] = nKeyFlag;

        for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
            if (byte_8ABA3C[sym] == nKey) {
                nKey = byte_8ABA24[sym];
                break;
            }
        }

        if (nIndex < FIFTY_THREE) {
            char buffer[16];
            sprintf(buffer, "%s%c", nKeyFlag ? "CTRL-" : "", nKey);
            WritePrivateProfileStringA("Keymap",
                off_8AFC50[nIndex],
                buffer,
                ".\\Keymap.ini");
        }
    }
}

// 0x5A9BA0
void CInfGame::LoadOptions()
{
    m_cOptions.m_nTilesPrecachePercent = GetPrivateProfileIntA("Game Options",
        "Tiles Precache Percent",
        100,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nTilesPrecachePercent > 100) {
        m_cOptions.m_nTilesPrecachePercent = 100;
    }

    m_cOptions.m_bDuplicateFloatingText = GetPrivateProfileIntA("Game Options",
        "Duplicate Floating Text",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bTranslucentBlts = GetPrivateProfileIntA("Game Options",
        "Translucent Blts",
        1,
        g_pBaldurChitin->GetIniFileName());
    CInfinity::TRANSLUCENT_BLTS_ON = m_cOptions.m_bTranslucentBlts;
    CVidCell::TRANSLUCENT_BLTS_ON = m_cOptions.m_bTranslucentBlts;

    m_cOptions.m_bMaxHP = GetPrivateProfileIntA("Game Options",
        "Maximum HP",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bAutoPauseOnTrap = GetPrivateProfileIntA("Game Options",
        "Pause On Trap",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.field_AC = 0;

    m_cOptions.m_bEnvironmentalAudio = GetPrivateProfileIntA("Game Options",
        "Environmental Audio",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bCheatsEnabled = GetPrivateProfileIntA("Game Options",
        "Cheats",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nGoreOption = GetPrivateProfileIntA("Game Options",
        "Grant 2000",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nGoreOption) {
        m_cOptions.m_nGore = GetPrivateProfileIntA("Game Options",
            "Chucknomitron",
            0,
            g_pBaldurChitin->GetIniFileName());
    } else {
        m_cOptions.m_nGore = 0;
    }

    m_cOptions.m_bFootStepsSounds = GetPrivateProfileIntA("Game Options",
        "Footsteps",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nMouseScrollSpeed = GetPrivateProfileIntA("Game Options",
        "Mouse Scroll Speed",
        28,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nMouseScrollSpeed < 6 || m_cOptions.m_nMouseScrollSpeed > 50) {
        m_cOptions.m_nMouseScrollSpeed = 28;
    }

    m_cOptions.m_nGuiFeedbackLevel = GetPrivateProfileIntA("Game Options",
        "GUI Feedback Level",
        5,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nGuiFeedbackLevel == 0 || m_cOptions.m_nGuiFeedbackLevel > 5) {
        m_cOptions.m_nGuiFeedbackLevel = 5;
    }

    m_cOptions.m_nLocatorFeedbackLevel = GetPrivateProfileIntA("Game Options",
        "Locator Feedback Level",
        3,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nLocatorFeedbackLevel == 0 || m_cOptions.m_nLocatorFeedbackLevel > 5) {
        m_cOptions.m_nLocatorFeedbackLevel = 3;
    }

    m_cOptions.m_nBoredTime = GetPrivateProfileIntA("Game Options",
        "Bored Timeout",
        CScreenWorld::BORED_TIME,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nBoredTime == 0 || m_cOptions.m_nBoredTime > CScreenWorld::BORED_TIME) {
        m_cOptions.m_nBoredTime = CScreenWorld::BORED_TIME;
    }

    m_cOptions.m_bAlwaysDither = GetPrivateProfileIntA("Game Options",
        "Always Dither",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bSubtitles = GetPrivateProfileIntA("Game Options",
        "Subtitles",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nKeyboardScrollSpeed = GetPrivateProfileIntA("Game Options",
        "Keyboard Scroll Speed",
        36,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nKeyboardScrollSpeed < 18 || m_cOptions.m_nKeyboardScrollSpeed > 150) {
        m_cOptions.m_nKeyboardScrollSpeed = 36;
    }

    m_cOptions.m_nCommandSoundsFrequency = GetPrivateProfileIntA("Game Options",
        "Command Sounds Frequency",
        2,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nCommandSoundsFrequency == 0 || m_cOptions.m_nCommandSoundsFrequency > 3) {
        m_cOptions.m_nCommandSoundsFrequency = 2;
    }

    m_cOptions.m_nSelectionSoundsFrequency = GetPrivateProfileIntA("Game Options",
        "Selection Sounds Frequency",
        3,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nSelectionSoundsFrequency == 0 || m_cOptions.m_nSelectionSoundsFrequency > 3) {
        m_cOptions.m_nSelectionSoundsFrequency = 3;
    }

    m_cOptions.m_nEffectTextLevel = GetPrivateProfileIntA("Game Options",
        "Effect Text Level",
        62,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bDarkvision = GetPrivateProfileIntA("Game Options",
        "Darkvision",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bWeatherEnabled = GetPrivateProfileIntA("Game Options",
        "Weather",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nTutorialState = GetPrivateProfileIntA("Game Options",
        "Tutorial State",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nTutorialState > 1) {
        m_cOptions.m_nTutorialState = 1;
    }

    m_cOptions.m_bAttackSounds = GetPrivateProfileIntA("Game Options",
        "Attack Sounds",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_bAttackSounds > 1) {
        m_cOptions.m_bAttackSounds = 1;
    }

    m_cOptions.m_nAutoPauseState = GetPrivateProfileIntA("Game Options",
        "Auto Pause State",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bAutoPauseCenter = GetPrivateProfileIntA("Game Options",
        "Auto Pause Center",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE) {
        m_cOptions.m_nDifficultyLevel = GetPrivateProfileIntA("Game Options",
            "Difficulty Level",
            3,
            g_pBaldurChitin->GetIniFileName());

        if (m_cOptions.m_nDifficultyLevel < 1 || m_cOptions.m_nDifficultyLevel > 5) {
            m_cOptions.m_nDifficultyLevel = 3;
        }

        switch (m_cOptions.m_nDifficultyLevel) {
        case 1:
            m_cOptions.m_nDifficultyMultiplier = -50;
            break;
        case 2:
            m_cOptions.m_nDifficultyMultiplier = -25;
            break;
        case 3:
            m_cOptions.m_nDifficultyMultiplier = 0;
            break;
        case 4:
            m_cOptions.m_nDifficultyMultiplier = 50;
            break;
        case 5:
            m_cOptions.m_nDifficultyMultiplier = 100;
            break;
        }

        m_cOptions.m_nMPDifficultyMultiplier = m_cOptions.m_nDifficultyMultiplier;
        m_nDifficultyLevel = -1;
    } else {
        m_nDifficultyLevel = GetPrivateProfileIntA("Game Options",
            "Difficulty Level",
            3,
            g_pBaldurChitin->GetIniFileName());

        // NOTE: Difficulty level is not normalized to 1..5 range as in the
        // code path above.

        m_cOptions.m_nDifficultyLevel = m_nDifficultyLevel;
    }

    m_cOptions.m_bQuickItemMapping = GetPrivateProfileIntA("Game Options",
        "Quick Item Mapping",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_bQuickItemMapping > 1) {
        m_cOptions.m_bQuickItemMapping = 1;
    }

    m_cOptions.m_bTerrainHugging = GetPrivateProfileIntA("Game Options",
        "Terrain Hugging",
        0,
        g_pBaldurChitin->GetIniFileName());

    CGameSprite::SHOW_CHARACTER_HP = GetPrivateProfileIntA("Game Options",
        "Show Character HP",
        0,
        g_pBaldurChitin->GetIniFileName());

    CGameSprite::GRAVITY_IS_DOWN = GetPrivateProfileIntA("Game Options",
        "Gravity Is Down",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bShowQuestXP = GetPrivateProfileIntA("Game Options",
        "Show Quest XP",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bStaticAnimations = GetPrivateProfileIntA("Game Options",
        "Static Animations",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bCriticalHitScreenShake = GetPrivateProfileIntA("Game Options",
        "Critical Hit Screen Shake",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bSuppressExtraDifficultyDamage = GetPrivateProfileIntA("Game Options",
        "Suppress Extra Difficulty Damage",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (!g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        g_pBaldurChitin->m_cBaldurMessage.DemandSettingsNightmareMode(FALSE);
    }

    m_cOptions.m_nDefaultCharGenPointsPool = GetPrivateProfileIntA("Game Options",
        "DefaultCharGenPointsPool",
        16,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nDefaultCharGenPointsPool < 10 || m_cOptions.m_nDefaultCharGenPointsPool > 108) {
        m_cOptions.m_nDefaultCharGenPointsPool = 16;
    }

    m_cOptions.m_nDefaultCharGenPoints = GetPrivateProfileIntA("Game Options",
        "DefaultCharGenPoints",
        10,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nDefaultCharGenPoints < 3 || m_cOptions.m_nDefaultCharGenPoints > 18) {
        m_cOptions.m_nDefaultCharGenPoints = 10;
    }

    m_cOptions.m_bHotkeysOnToolTips = GetPrivateProfileIntA("Game Options",
        "Hotkeys On Tooltips",
        1,
        g_pBaldurChitin->GetIniFileName());

    g_pBaldurChitin->m_cBaldurMessage.SetSignalDefaultSecondsToTimeout();

    m_cOptions.m_bAllScreenShake = GetPrivateProfileIntA("Game Options",
        "All Screen Shake",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nOldPortraitHealth = GetPrivateProfileIntA("Game Options",
        "Old Portrait Health",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nHitPointBarSequence = GetPrivateProfileIntA("Game Options",
        "Hit Point Bar Sequence",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nHitPointBarSequence < 0 || m_cOptions.m_nHitPointBarSequence > 5) {
        m_cOptions.m_nHitPointBarSequence = 0;
    }
}

// 0x5AA350
void CInfGame::SaveOptions()
{
    CString sValue;

    sValue.Format("%d", m_cOptions.m_bFootStepsSounds);
    WritePrivateProfileStringA("Game Options", "Footsteps", sValue, g_pBaldurChitin->GetIniFileName());

    if ((!g_pChitin->cNetwork.GetSessionOpen() || g_pChitin->cNetwork.GetSessionHosting())
        && m_cOptions.m_nGoreOption != 0) {
        sValue.Format("%d", m_cOptions.m_nGoreOption);
        WritePrivateProfileStringA("Game Options", "Grant 2000", sValue, g_pBaldurChitin->GetIniFileName());

        sValue.Format("%d", m_cOptions.m_nGore);
        WritePrivateProfileStringA("Game Options", "Chucknomitron", sValue, g_pBaldurChitin->GetIniFileName());
    }

    sValue.Format("%d", m_cOptions.m_nMouseScrollSpeed);
    WritePrivateProfileStringA("Game Options", "Mouse Scroll Speed", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nGuiFeedbackLevel);
    WritePrivateProfileStringA("Game Options", "GUI Feedback Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nLocatorFeedbackLevel);
    WritePrivateProfileStringA("Game Options", "Locator Feedback Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nBoredTime);
    WritePrivateProfileStringA("Game Options", "Bored Timeout", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAlwaysDither);
    WritePrivateProfileStringA("Game Options", "Always Dither", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bSubtitles);
    WritePrivateProfileStringA("Game Options", "Subtitles", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nKeyboardScrollSpeed);
    WritePrivateProfileStringA("Game Options", "Keyboard Scroll Speed", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nCommandSoundsFrequency);
    WritePrivateProfileStringA("Game Options", "Command Sounds Frequency", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nSelectionSoundsFrequency);
    WritePrivateProfileStringA("Game Options", "Selection Sounds Frequency", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nEffectTextLevel);
    WritePrivateProfileStringA("Game Options", "Effect Text Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bDarkvision);
    WritePrivateProfileStringA("Game Options", "Darkvision", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bWeatherEnabled);
    WritePrivateProfileStringA("Game Options", "Weather", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nTutorialState);
    WritePrivateProfileStringA("Game Options", "Tutorial State", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAttackSounds);
    WritePrivateProfileStringA("Game Options", "Attack Sounds", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nAutoPauseState);
    WritePrivateProfileStringA("Game Options", "Auto Pause State", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAutoPauseCenter);
    WritePrivateProfileStringA("Game Options", "Auto Pause Center", sValue, g_pBaldurChitin->GetIniFileName());

    if (m_nDifficultyLevel == -1) {
        sValue.Format("%d", m_cOptions.m_nDifficultyLevel);
    } else {
        sValue.Format("%d", m_nDifficultyLevel);
    }

    WritePrivateProfileStringA("Game Options", "Difficulty Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bQuickItemMapping);
    WritePrivateProfileStringA("Game Options", "Quick Item Mapping", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bEnvironmentalAudio);
    WritePrivateProfileStringA("Game Options", "Environmental Audio", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bTerrainHugging);
    WritePrivateProfileStringA("Game Options", "Terrain Hugging", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bStaticAnimations);
    WritePrivateProfileStringA("Game Options", "Static Animations", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bTranslucentBlts);
    WritePrivateProfileStringA("Game Options", "Translucent Blts", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bMaxHP);
    WritePrivateProfileStringA("Game Options", "Maximum HP", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAutoPauseOnTrap);
    WritePrivateProfileStringA("Game Options", "Pause On Trap", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", CGameSprite::SHOW_CHARACTER_HP);
    WritePrivateProfileStringA("Game Options", "Show Character HP", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bShowQuestXP);
    WritePrivateProfileStringA("Game Options", "Show Quest XP", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", g_pBaldurChitin->field_1A0);
    WritePrivateProfileStringA("Program Options", "Double Byte Character Support", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bCriticalHitScreenShake);
    WritePrivateProfileStringA("Game Options", "Critical Hit Screen Shake", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bHotkeysOnToolTips);
    WritePrivateProfileStringA("Game Options", "Hotkeys On Tooltips", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bDuplicateFloatingText);
    WritePrivateProfileStringA("Game Options", "Duplicate Floating Text", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nTilesPrecachePercent);
    WritePrivateProfileStringA("Game Options", "Tiles Precache Percent", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bSuppressExtraDifficultyDamage);
    WritePrivateProfileStringA("Game Options", "Suppress Extra Difficulty Damage", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAllScreenShake);
    WritePrivateProfileStringA("Game Options", "All Screen Shake", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nHitPointBarSequence);
    WritePrivateProfileStringA("Game Options", "Hit Point Bar Sequence", sValue, g_pBaldurChitin->GetIniFileName());
}

// 0x5AAC30
void CInfGame::LoadMultiPlayerPermissions()
{
    if (!g_pBaldurChitin->cNetwork.GetSessionOpen()
        || !g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        return;
    }

    CMultiplayerSettings* pSettings = GetMultiplayerSettings();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 8323
    UTIL_ASSERT(pSettings != NULL);

    CString sPermissions;
    CString sDefault("");

    GetPrivateProfileStringA("Multiplayer",
        "Default Permissions",
        sDefault,
        sPermissions.GetBuffer(10),
        10,
        g_pBaldurChitin->GetIniFileName());
    sPermissions.ReleaseBuffer(-1);

    if (sPermissions.GetLength() == 8) {
        BOOL bPurchasing = sPermissions[0] == '1';
        BOOL bAreaTransition = sPermissions[1] == '1';
        BOOL bDialog = sPermissions[2] == '1';
        BOOL bCharRecords = sPermissions[3] == '1';
        BOOL bPausing = sPermissions[4] == '1';
        BOOL bGroupPool = sPermissions[5] == '1';
        BOOL bLeader = sPermissions[6] == '1';
        BOOL bModifyChars = sPermissions[7] == '1';

        pSettings->SetPermission(-1,
            CGamePermission::PURCHASING,
            bPurchasing,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::AREA_TRANSITION,
            bAreaTransition,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::DIALOG,
            bDialog,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::CHAR_RECORDS,
            bCharRecords,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::PAUSING,
            bPausing,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::GROUP_POOL,
            bGroupPool,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::LEADER,
            bLeader,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::MODIFY_CHARS,
            bModifyChars,
            TRUE);
    }

    INT nRestrictStore = GetPrivateProfileIntA("Multiplayer",
        "Restrict Stores",
        0,
        g_pBaldurChitin->GetIniFileName());
    pSettings->SetRestrictStoreOption(nRestrictStore != 0);

    BYTE nImportingBitField = static_cast<BYTE>(GetPrivateProfileIntA("Multiplayer",
        "Import Character",
        CMultiplayerSettings::IMPORT_ALL,
        g_pBaldurChitin->GetIniFileName()));
    pSettings->SetImportingCharacterOption(nImportingBitField);
}

// 0x5AAEC0
void CInfGame::SaveMultiPlayerPermissions()
{
    if (!g_pBaldurChitin->cNetwork.GetSessionOpen()
        || !g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        return;
    }

    CMultiplayerSettings* pSettings = GetMultiplayerSettings();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 3290
    UTIL_ASSERT(pSettings != NULL);

    BOOL bPurchasing = pSettings->GetPermission(-1, CGamePermission::PURCHASING);
    BOOL bAreaTransition = pSettings->GetPermission(-1, CGamePermission::AREA_TRANSITION);
    BOOL bDialog = pSettings->GetPermission(-1, CGamePermission::DIALOG);
    BOOL bCharRecords = pSettings->GetPermission(-1, CGamePermission::CHAR_RECORDS);
    BOOL bPausing = pSettings->GetPermission(-1, CGamePermission::PAUSING);
    BOOL bGroupPool = pSettings->GetPermission(-1, CGamePermission::GROUP_POOL);
    BOOL bLeader = pSettings->GetPermission(-1, CGamePermission::LEADER);
    BOOL bModifyChars = pSettings->GetPermission(-1, CGamePermission::MODIFY_CHARS);

    CString sPermissions("00000000");
    sPermissions.SetAt(0, bPurchasing + '0');
    sPermissions.SetAt(1, bAreaTransition + '0');
    sPermissions.SetAt(2, bDialog + '0');
    sPermissions.SetAt(3, bCharRecords + '0');
    sPermissions.SetAt(4, bPausing + '0');
    sPermissions.SetAt(5, bGroupPool + '0');
    sPermissions.SetAt(6, bLeader + '0');
    sPermissions.SetAt(7, bModifyChars + '0');

    WritePrivateProfileStringA("Multiplayer",
        "Default Permissions",
        sPermissions,
        g_pBaldurChitin->GetIniFileName());

    CString sTemp;

    sTemp.Format("%d", pSettings->m_bRestrictStoreOption);
    WritePrivateProfileStringA("Multiplayer",
        "Restrict Stores",
        sTemp,
        g_pBaldurChitin->GetIniFileName());

    sTemp.Format("%d", pSettings->m_nImportingBitField);
    WritePrivateProfileStringA("Multiplayer",
        "Import Character",
        sTemp,
        g_pBaldurChitin->GetIniFileName());
}

// 0x5AB190
void CInfGame::LoadGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 8447
    UTIL_ASSERT(m_sSaveGame != "");



    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->ClearViewedCharacters();
    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->ClearPlayerReady();

    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    dword_8E52EC = 0;
    dword_8E52F0 = 0;

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        WaitForEngine(TRUE);
        g_pChitin->SetProgressBar(TRUE,
            9888,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = 8000000;
        g_pChitin->cProgressBar.m_bDisableMinibars = 1;
    }

    g_pChitin->m_bDisplayStale = TRUE;
    SleepEx(25, FALSE);

    if (bProgressBarInPlace || bProgressBarRequired) {
        g_pChitin->cProgressBar.AddActionTarget(625000);
    }

    m_bInLoadGame = TRUE;

    BOOL bMusicThreadPriorityChanged = FALSE;
    int nMusicThreadPriority = GetThreadPriority(g_pChitin->m_hMusicThread);

    if (GetPrivateProfileIntA("Program Options", "Volume Music", 0, g_pChitin->GetIniFileName())) {
        bMusicThreadPriorityChanged = SetThreadPriority(g_pChitin->m_hMusicThread, 15);
    }

    InitGame(FALSE, FALSE);

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    m_bFromNewGame = FALSE;

    CString sDir = GetDirSave();
    if (sDir == m_sSaveDir + "default\\") {
        sDir = m_sMultiplayerSaveDir + m_sSaveGame + "\\";
    }

    if (!g_pChitin->cDimm.DirectoryCopyFiles(sDir, m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8524
        UTIL_ASSERT_MSG(FALSE, "Couldn't copy files from Temp to Save directories");
    }

    if (!g_pChitin->cDimm.UncompressDirectory(m_sTempDir, sDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8526
        UTIL_ASSERT_MSG(FALSE, "Couldn't uncompress save game into Temp directory");
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "ICEWIND2.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8541
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT0.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8548
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT1.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8555
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT2.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8562
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT3.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8569
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT4.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8576
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT5.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8583
        UTIL_ASSERT(FALSE);
    }

    g_pChitin->cDimm.AddToDirectoryList(m_sTempDir, TRUE);

    g_pBaldurChitin->GetTlkTable().OpenOverride(CString("temp/default.toh"),
        CString("temp/default.tot"));

    m_cWorldMap.SetResRef(CResRef("WORLDMAP"));

    CGameFile cGameFile;
    cGameFile.SetResRef(CResRef("ICEWIND2"), TRUE, TRUE);

    Unmarshal(cGameFile.GetData(),
        cGameFile.GetDataSize(),
        bProgressBarInPlace | bProgressBarRequired);

    g_pBaldurChitin->cSoundMixer.StartSong(-1, 0x1 | 0x2);
    SleepEx(500, FALSE);
    g_pBaldurChitin->cSoundMixer.StopMusic(TRUE);

    cGameFile.Release();

    m_bAnotherPlayerJoinedGame = FALSE;

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        m_cRemoteObjectArray.ChangeControlOnLoadGame();
    }

    m_bInLoadGame = FALSE;

    if (bMusicThreadPriorityChanged) {
        SetThreadPriority(g_pChitin->m_hMusicThread, nMusicThreadPriority);
    }

    m_nLastSaveTime = m_worldTime.m_gameTime;

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;

        g_pChitin->m_bDisplayStale = TRUE;
        WaitForEngine(TRUE);

        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }


    for (SHORT cnt = 0; cnt < m_nCharacters; cnt++) {
    }

    SelectAll(FALSE);


    g_pBaldurChitin->GetObjectGame()->m_cButtonArray.UpdateState();


}

// 0x5ABA20
void CInfGame::NewGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        WaitForEngine(TRUE);

        g_pChitin->SetProgressBar(TRUE,
            9889,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = 8000000;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        g_pChitin->cProgressBar.AddActionTarget(625000);
    }

    m_bInLoadGame = TRUE;

    BOOL bMusicThreadPriorityChanged = FALSE;
    int nMusicThreadPriority = GetThreadPriority(g_pChitin->m_hMusicThread);

    if (GetPrivateProfileIntA("Program Options", "Volume Music", 0, g_pChitin->GetIniFileName())) {
        bMusicThreadPriorityChanged = SetThreadPriority(g_pChitin->m_hMusicThread, 15);
    }

    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    if (g_pChitin->cDimm.DirectoryRemoveFiles(CString(".\\mpsave\\default\\"))) {
        g_pChitin->cDimm.m_cKeyTable.RescanEverything();
    }

    InitGame(FALSE, FALSE);

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(156250, FALSE);
    }

    CVariable chapter;
    chapter.SetName(CHAPTER_GLOBAL);
    chapter.m_intValue = -1;
    m_variables.AddKey(chapter);

    m_bFromNewGame = TRUE;

    g_pChitin->cDimm.AddToDirectoryList(m_sScriptsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sSoundsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sPortraitsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sCharactersDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sTempDir, TRUE);

    g_pBaldurChitin->GetTlkTable().OpenOverride(CString("temp/default.toh"), CString("temp/default.tot"));

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(156250, FALSE);
    }

    m_cWorldMap.SetResRef(CResRef("WORLDMAP"));

    CGameFile cGameFile;
    cGameFile.SetResRef(CResRef("ICEWIND2"), TRUE, TRUE);

    Unmarshal(cGameFile.GetData(),
        cGameFile.GetDataSize(),
        bProgressBarInPlace | bProgressBarRequired);

    m_cOptions.m_nNightmareMode = GetPrivateProfileIntA("Game Options",
        "Nightmare Mode",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nNightmareMode == 1 && m_cOptions.m_nDifficultyLevel != 5) {
        m_cOptions.m_nDifficultyLevel = 5;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 8769
    UTIL_ASSERT(m_nCharacters == 0);

    m_bAnotherPlayerJoinedGame = FALSE;

    m_sSaveGame = "default";
    CScreenCharacter::SAVE_NAME = "default";

    m_bInLoadGame = FALSE;

    if (bMusicThreadPriorityChanged == TRUE) {
        SetThreadPriority(g_pChitin->m_hMusicThread, nMusicThreadPriority);
    }

    m_nLastSaveTime = m_worldTime.m_gameTime;

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(156250, FALSE);
    }

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        g_pChitin->m_bDisplayStale = TRUE;

        WaitForEngine(TRUE);

        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }

    m_nReputation = -10;
}

// 0x5AC0A0
BOOLEAN CInfGame::IsAreaSaveAllowed(CGameArea* pArea)
{
    if (!m_bInLoadGame
        && pArea != NULL
        && pArea->m_bAreaLoaded) {
        return pArea->field_B16;
    } else {
        return FALSE;
    }
}

// 0x5AC0D0
void CInfGame::UpdateAreaSaveStatus()
{
    STRREF strError;

    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL && m_gameAreas[index]->m_bAreaLoaded) {
            m_gameAreas[index]->field_B16 = !m_gameAreas[index]->CanSaveGame(strError);
        }
    }
}

// 0x5AD470
void CInfGame::SelectAll(BOOLEAN bPlaySound)
{
    LONG iSprite;
    CGameSprite* pSprite;
    BYTE rc;
    SHORT cnt;
    POSITION pos;


    for (cnt = 0; cnt < m_nCharacters; cnt++) {
        iSprite = m_characterPortraits[cnt];
        do {
            rc = m_cObjectArray.GetShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CGameArea* pVisibleArea = GetVisibleArea();

            if (GetVisibleArea() != NULL && pSprite->m_pArea == GetVisibleArea()) {
                if (pSprite->Orderable(FALSE) && !pSprite->m_bSelected) {
                    do {
                        rc = m_cObjectArray.GetDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->Select();
                        m_cObjectArray.ReleaseDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        UpdatePortrait(cnt, 1);
                    }                }
                if (pSprite->Orderable(FALSE) && bPlaySound) {
                    bPlaySound = FALSE;
                    pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
                }
            } else if (pSprite->m_bSelected) {
                do {
                    rc = m_cObjectArray.GetDeny(iSprite,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    pSprite->Unselect();
                    m_cObjectArray.ReleaseDeny(iSprite,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    UpdatePortrait(cnt, 1);
                }            }
            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }    }

    pos = m_familiars.GetHeadPosition();
    while (pos != NULL) {
        iSprite = reinterpret_cast<LONG>(m_familiars.GetNext(pos));
        rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->GetArea() == GetVisibleArea()) {
                if (pSprite->Orderable(FALSE) && !pSprite->m_bSelected) {
                    do {
                        rc = m_cObjectArray.GetDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->Select();
                        m_cObjectArray.ReleaseDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
                if (pSprite->Orderable(FALSE) && bPlaySound) {
                    bPlaySound = FALSE;
                    pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
                }
            }
            rc = m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    pos = m_allies.GetHeadPosition();
    while (pos != NULL) {
        iSprite = reinterpret_cast<LONG>(m_allies.GetNext(pos));
        rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->GetArea() == GetVisibleArea()) {
                if (pSprite->Orderable(FALSE) && !pSprite->m_bSelected) {
                    do {
                        rc = m_cObjectArray.GetDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->Select();
                        m_cObjectArray.ReleaseDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
                if (pSprite->Orderable(FALSE) && bPlaySound) {
                    bPlaySound = FALSE;
                    pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
                }
            }
            rc = m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    SelectToolbar();

}

// 0x5AD7E0
void CInfGame::UnselectAll()
{
    LONG* pGroup = m_group.GetGroupList();
    SHORT nGroup = m_group.GetCount();
    CGameSprite* pSprite;

    for (SHORT cnt = 0; cnt < nGroup; cnt++) {
        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(pGroup[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Unselect();

            m_cObjectArray.ReleaseDeny(pGroup[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            // NOTE: Uninline.
            UpdatePortrait(GetFixedOrderCharacterPortraitNum(pGroup[cnt]), 1);
        }
    }

    delete pGroup;
}

// 0x5AD8A0
BOOL CInfGame::SelectCharacter(LONG characterId, BOOLEAN bPlaySelectSound)
{
    CGameSprite* pSprite;
    BYTE rc;
    SHORT nPortrait = GetCharacterPortraitNum(characterId);

    if (nPortrait == -1) {
        if (!IsAlly(characterId) || !IsFamiliar(characterId)) {
            return FALSE;
        }
    }

    do {
        rc = m_cObjectArray.GetShare(characterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (pSprite->m_bSelected
        && !g_pBaldurChitin->GetScreenWorld()->GetShiftKey()) {
        do {
            rc = m_cObjectArray.GetDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Unselect();

            m_cObjectArray.ReleaseDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else if ((GetVisibleArea() == pSprite->GetArea()
                   || GetGroup()->GetCount() == 0)
        && pSprite->Orderable(FALSE)
        && pSprite->GetArea() != NULL) {
        if (GetVisibleArea() != pSprite->GetArea()) {
            if (GetVisibleArea() != NULL) {
                GetVisibleArea()->m_bPicked = FALSE;
                GetVisibleArea()->m_iPicked = -1;
                GetVisibleArea()->m_nToolTip = 0;
                GetVisibleArea()->OnDeactivation();
            }

            m_visibleArea = pSprite->GetArea()->GetId();

            GetVisibleArea()->OnActivation();
        }

        do {
            rc = m_cObjectArray.GetDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Select();

            m_cObjectArray.ReleaseDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (bPlaySelectSound && pSprite->Orderable(FALSE)) {
        pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
    }

    m_cObjectArray.ReleaseShare(characterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    if (nPortrait != -1) {
        UpdatePortrait(nPortrait, 1);
    }

    return TRUE;
}

// 0x5ADAE0
void CInfGame::SelectToolbar()
{
    // TODO: Incomplete.

    m_nState = 0;
    m_cButtonArray.SetSelectedButton(100);

    BYTE nGroupCount = m_group.GetCount();
    if (nGroupCount == 0) {
        m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);
        return;
    }

    if (nGroupCount > 1) {
        m_cButtonArray.SetState(0x6E, 0);
        return;
    }

    LONG* pGroupList = m_group.GetGroupList();
    if (pGroupList == NULL) {
        m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);
        return;
    }

    LONG nCharacterId = pGroupList[0];
    delete[] pGroupList;

    CGameSprite* pSprite = NULL;
    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS || pSprite == NULL) {
        m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);
        return;
    }

    if (IcewindMisc::IsPC(pSprite)) {
        // Matches Ghidra SelectToolbar (0x5ADAE0): the nine custom button slots
        // come straight from the sprite (offset 0x3D14, read via
        // GetCustomButtonValue).  No ResetQuickSlots / fallback here — the slots
        // are populated at character creation.
        INT customButtons[9];
        for (BYTE nButton = 0; nButton < 9; nButton++) {
            customButtons[nButton] = pSprite->GetCustomButtonValue(nButton);
        }

        m_cButtonArray.SetCustomButtonTypes(customButtons);
        m_cButtonArray.SetQuickWeaponSlot(pSprite->m_nWeaponSet);
        m_cButtonArray.SetState(0x72, 0);
    } else {
        m_cButtonArray.SetQuickWeaponSlot(pSprite->m_nWeaponSet);
        m_cButtonArray.SetState(0x6E, 0);
    }

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x5ADC90
void CInfGame::DemandServerStore(const CResRef& store, BOOL bSaveToDisk)
{
    int cnt;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 9987
    UTIL_ASSERT(g_pChitin->cNetwork.GetSessionOpen() && g_pChitin->cNetwork.GetSessionHosting());

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL
            && store == m_aServerStore[cnt]->m_resRef) {
            if (bSaveToDisk) {
                m_aServerStore[cnt]->Marshal(m_sTempDir);
            }

            m_nServerStoreDemands[cnt]++;

            return;
        }
    }

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] == NULL) {
            m_aServerStore[cnt] = new CStore(store);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 10007
            UTIL_ASSERT(m_aServerStore[cnt] != NULL);

            if (m_aServerStore[cnt]->m_resRef == "") {
                delete m_aServerStore[cnt];
                m_aServerStore[cnt] = NULL;
            } else {
                m_nServerStoreDemands[cnt] = 1;
            }

            return;
        }
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10019
    UTIL_ASSERT(FALSE);
}

// 0x5ADE90
void CInfGame::ReleaseServerStore(const CResRef& store)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10042
    UTIL_ASSERT(g_pChitin->cNetwork.GetSessionOpen() && g_pChitin->cNetwork.GetSessionHosting());

    for (int cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL
            && store == m_aServerStore[cnt]->m_resRef) {
            m_nServerStoreDemands[cnt]--;

            if (m_nServerStoreDemands[cnt] == 0) {
                m_aServerStore[cnt]->Marshal(m_sTempDir);

                delete m_aServerStore[cnt];

                m_aServerStore[cnt] = NULL;
            }
        }
    }
}

// 0x5ADF60
CStore* CInfGame::GetServerStore(const CResRef& store)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10079
    UTIL_ASSERT(g_pChitin->cNetwork.GetSessionOpen() && g_pChitin->cNetwork.GetSessionHosting());

    for (int cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL
            && store == m_aServerStore[cnt]->m_resRef) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 10084
            UTIL_ASSERT(m_nServerStoreDemands[cnt] > 0);

            return m_aServerStore[cnt];
        }
    }

    return NULL;
}

// 0x5AF160
void CInfGame::SwapCharacters(DWORD pos1, DWORD pos2)
{
    // TODO: Incomplete.
}

// 0x5AF360
void CInfGame::UpdatePortrait(SHORT nPortrait, DWORD dwPanelId)
{
    if (nPortrait != -1) {
        CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
        if (pEngine != g_pBaldurChitin->m_pEngineProjector
            && pEngine != g_pBaldurChitin->m_pEngineMultiPlayer
            && pEngine != g_pBaldurChitin->m_pEngineSinglePlayer
            && pEngine != g_pBaldurChitin->m_pEngineWorldMap
            && pEngine != g_pBaldurChitin->m_pEngineSave) {
            CUIManager* pManager = pEngine->GetManager();
            CUIPanel* pPanel = pManager->GetPanel(dwPanelId);
            if (pPanel != NULL) {
                CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                if (pControl != NULL) {
                    pControl->InvalidateRect();
                }
            } else {
                // NOTE: Obtaining active engine second time, probably some
                // inlining. Looks similar to `CBaldurEngine::OnPortraitLClick`.
                CUIManager* pManager = g_pBaldurChitin->GetActiveEngine()->GetManager();
                CUIPanel* pPanel = pManager->GetPanel(1);
                if (pPanel != NULL) {
                    CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                    if (pControl != NULL) {
                        pControl->InvalidateRect();
                    }
                }
            }
        }
    }
}

// 0x5AF420
void CInfGame::ProcessPortraitAction(SHORT nPortrait, DWORD dwPanelId)
{
    if (nPortrait != -1) {
        CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
        if (pEngine != g_pBaldurChitin->m_pEngineProjector
            && pEngine != g_pBaldurChitin->m_pEngineMultiPlayer
            && pEngine != g_pBaldurChitin->m_pEngineSinglePlayer
            && pEngine != g_pBaldurChitin->m_pEngineWorldMap
            && pEngine != g_pBaldurChitin->m_pEngineSave) {
            CUIManager* pManager = pEngine->GetManager();
            CUIPanel* pPanel = pManager->GetPanel(dwPanelId);
            if (pPanel != NULL) {
                CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                if (pControl != NULL) {
                    pControl->func_54();
                }
            } else {
                CUIManager* pManager = g_pBaldurChitin->GetActiveEngine()->GetManager();
                CUIPanel* pPanel = pManager->GetPanel(1);
                if (pPanel != NULL) {
                    CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                    if (pControl != NULL) {
                        pControl->func_54();
                    }
                }
            }
        }
    }
}

// 0x5AF4E0
SHORT CInfGame::GetCharacterPortraitNum(LONG nCharacterId)
{
    if (nCharacterId != -1) {
        for (SHORT nPortrait = 0; nPortrait < 6; nPortrait++) {
            if (m_characterPortraits[nPortrait] == nCharacterId) {
                return nPortrait;
            }
        }
    }

    return -1;
}

// 0x5AF510
SHORT CInfGame::GetFixedOrderCharacterPortraitNum(LONG nCharacterId)
{
    if (nCharacterId != -1) {
        for (SHORT nPortrait = 0; nPortrait < 6; nPortrait++) {
            if (m_characters[nPortrait] == nCharacterId) {
                return nPortrait;
            }
        }
    }

    return -1;
}

// 0x5AF700
void CInfGame::RemoveFamiliarResRef(const CResRef& resRef, BYTE nAlignment, BYTE nLevel)
{
    if (nAlignment < 9 && nLevel < 9) {
        POSITION pos = m_familiarResRefs[nAlignment][nLevel].GetHeadPosition();
        while (pos != NULL) {
            POSITION curr = pos;
            CResRef* pResRef = m_familiarResRefs[nAlignment][nLevel].GetNext(pos);
            if (*pResRef == resRef) {
                m_familiarResRefs[nAlignment][nLevel].RemoveAt(curr);
                break;
            }
        }
    }
}

// 0x5AF770
void CInfGame::RenderPortrait(DWORD portraitId, const CPoint& cpRenderPosition, const CSize& szControl, BOOL bPressed, BOOL reorderHighlight, BOOL selectFromMarker, const CRect& rClip, BOOL bDoubleSize)
{
    LONG iSprite = GetCharacterId(static_cast<SHORT>(portraitId));

    if (iSprite != CGameObjectArray::INVALID_INDEX) {
        CGameSprite* pSprite;

        BYTE rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->RenderPortrait(cpRenderPosition,
                szControl,
                bPressed,
                reorderHighlight,
                selectFromMarker,
                rClip,
                bDoubleSize);

            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5AF7F0
void CInfGame::WorldEngineActivated(CVidMode* pVidMode)
{

    m_nState = 0;
    m_cButtonArray.m_nSelectedButton = 100;
    m_cVRamPool.DetachSurfaces();
    m_cVRamPool.AttachSurfaces(pVidMode);
    if (m_visibleArea >= 0 && m_visibleArea < CINFGAME_MAX_AREAS && m_gameAreas[m_visibleArea] != NULL) {
        m_gameAreas[m_visibleArea]->OnActivation();
    }    m_cButtonArray.ResetState();

}

// 0x5AF850
void CInfGame::WorldEngineDeactivated()
{
    if (m_gameAreas[m_visibleArea] != NULL) {
        m_gameAreas[m_visibleArea]->OnDeactivation();
    }
}

// 0x5AF870
void CInfGame::OnPortraitLClick(DWORD id)
{
    if (m_nState != 0 && GetVisibleArea()->m_iPicked != -1) {
        CGameObject* pObject;
        BYTE rc = m_cObjectArray.GetShare(GetVisibleArea()->m_iPicked,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (m_nState == 1) {
                GetVisibleArea()->OnActionButtonClickGround(pObject->GetPos());
            } else {
                pObject->OnActionButton(pObject->GetPos());
            }
            m_cObjectArray.ReleaseShare(GetVisibleArea()->m_iPicked,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else {
        if (static_cast<SHORT>(id) < GetNumCharacters()) {
            CGameSprite* pSprite;
            LONG nCharacterId = m_characterPortraits[id];
            BYTE rc = m_cObjectArray.GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if (!pSprite->InControl()) {
                    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() > 0
                        && pSprite->GetArea() == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
                        CAIAction action(CAIAction::MOVETOOBJECTFOLLOW, pSprite->GetAIType(), 0, 0, 0);
                        g_pBaldurChitin->GetObjectGame()->GetGroup()->GroupAction(action, TRUE, NULL);
                    } else if (pSprite->GetArea() != NULL) {
                        UnselectAll();

                        if (GetVisibleArea() != NULL) {
                            GetVisibleArea()->m_bPicked = FALSE;
                            GetVisibleArea()->m_iPicked = -1;
                            GetVisibleArea()->m_nToolTip = 0;
                            GetVisibleArea()->OnDeactivation();
                        }

                        m_visibleArea = pSprite->GetArea()->m_id;

                        GetVisibleArea()->OnActivation();
                    }
                }

                m_cObjectArray.ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            if (!g_pBaldurChitin->GetScreenWorld()->GetCtrlKey()
                && !g_pBaldurChitin->GetScreenWorld()->GetShiftKey()) {
                UnselectAll();
            }

            SelectCharacter(m_characterPortraits[id], TRUE);
            SelectToolbar();
        }
    }
}

// 0x5AFCB0
void CInfGame::OnPortraitLDblClick(DWORD id)
{
    LONG iSprite = GetCharacterId(static_cast<SHORT>(id));
    if (iSprite != CGameObjectArray::INVALID_INDEX) {
        CGameSprite* pSprite;

        BYTE rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        if (rc == CGameObjectArray::SUCCESS) {
            CGameArea* pArea = pSprite->GetArea();
            if (pArea != NULL) {
                CRect rView(pArea->GetInfinity()->rViewPort);
                INT x = rView.Width() / -2;
                INT y = rView.Height() / -2;

                CPoint& pos = pSprite->GetPos();

                pArea->GetInfinity()->SetViewPosition(x + pos.x, y + pos.y, TRUE);
            }

            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5AFD90
void CInfGame::EnablePortrait(BYTE nControlId, BOOL bEnable)
{
    g_pBaldurChitin->m_pEngineCharacter->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineInventory->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineJournal->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineMap->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineOptions->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineSpellbook->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineStore->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineWorld->EnablePortrait(1, nControlId, bEnable);
}

// 0x5AFE60
BOOLEAN CInfGame::ExtendedDayNightAreaActive()
{
    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if ((m_gameAreas[index]->m_header.m_areaType & 0x40) != 0) {
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5AFE90
void CInfGame::UpdatePortraitToolTip(DWORD dwControlId, STRREF dwStrId)
{
    CUIManager* pManager;
    CUIPanel* pPanel;
    CUIControlBase* pControl;

    pManager = g_pBaldurChitin->m_pEngineCharacter->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineInventory->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineJournal->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineMap->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineOptions->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineSpellbook->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineStore->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineWorld->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);
}

// 0x5B0020
void CInfGame::StepAnimation(BYTE direction)
{
    // FIXME: Should be static global.
    WORD animationList[ANIM_TYPE_NUM] = {
        ANIM_DEATH_KNIGHT,
        ANIM_STATIC_NOBLE_MAN_CHAIR,
        ANIM_STATIC_NOBLE_MAN_MATTE,
        ANIM_STATIC_NOBLE_WOMAN_CHAIR,
        ANIM_STATIC_NOBLE_WOMAN_MATTE,
        ANIM_STATIC_PEASANT_MAN_CHAIR,
        ANIM_STATIC_PEASANT_MAN_STOOL,
        ANIM_STATIC_PEASANT_MAN_MATTE,
        ANIM_STATIC_PEASANT_WOMAN_CHAIR,
        ANIM_STATIC_PEASANT_WOMAN_MATTE,
        ANIM_STATIC_HUMANCLERIC_MAN_CHAIR,
        ANIM_SLEEPING_MAN_HUMAN,
        ANIM_SLEEPING_WOMAN_HUMAN,
        ANIM_SLEEPING_FAT_MAN_HUMAN,
        ANIM_SLEEPING_DWARF,
        ANIM_SLEEPING_MAN_ELF,
        ANIM_SLEEPING_WOMAN_ELF,
        ANIM_SLEEPING_MAN_HALFLING,
        ANIM_CLERIC_MALE_HUMAN,
        ANIM_CLERIC_MALE_ELF,
        ANIM_CLERIC_MALE_DWARF,
        ANIM_CLERIC_MALE_HALFLING,
        ANIM_CLERIC_MALE_GNOME,
        ANIM_CLERIC_MALE_HALFORC,
        ANIM_CLERIC_FEMALE_HUMAN,
        ANIM_CLERIC_FEMALE_ELF,
        ANIM_CLERIC_FEMALE_DWARF,
        ANIM_CLERIC_FEMALE_HALFLING,
        ANIM_CLERIC_FEMALE_GNOME,
        ANIM_CLERIC_FEMALE_HALFORC,
        ANIM_FIGHTER_MALE_HUMAN,
        ANIM_FIGHTER_MALE_ELF,
        ANIM_FIGHTER_MALE_DWARF,
        ANIM_FIGHTER_MALE_HALFLING,
        ANIM_FIGHTER_MALE_GNOME,
        ANIM_FIGHTER_MALE_HALFORC,
        ANIM_FIGHTER_FEMALE_HUMAN,
        ANIM_FIGHTER_FEMALE_ELF,
        ANIM_FIGHTER_FEMALE_DWARF,
        ANIM_FIGHTER_FEMALE_HALFLING,
        ANIM_FIGHTER_FEMALE_GNOME,
        ANIM_FIGHTER_FEMALE_HALFORC,
        ANIM_MAGE_MALE_HUMAN,
        ANIM_MAGE_MALE_ELF,
        ANIM_MAGE_MALE_DWARF,
        ANIM_MAGE_MALE_GNOME,
        ANIM_MAGE_MALE_HALFORC,
        ANIM_MAGE_FEMALE_HUMAN,
        ANIM_MAGE_FEMALE_ELF,
        ANIM_MAGE_FEMALE_DWARF,
        ANIM_MAGE_FEMALE_GNOME,
        ANIM_MAGE_FEMALE_HALFORC,
        ANIM_THIEF_MALE_HUMAN,
        ANIM_THIEF_MALE_ELF,
        ANIM_THIEF_MALE_DWARF,
        ANIM_THIEF_MALE_HALFLING,
        ANIM_THIEF_MALE_GNOME,
        ANIM_THIEF_MALE_HALFORC,
        ANIM_THIEF_FEMALE_HUMAN,
        ANIM_THIEF_FEMALE_ELF,
        ANIM_THIEF_FEMALE_DWARF,
        ANIM_THIEF_FEMALE_HALFLING,
        ANIM_THIEF_FEMALE_GNOME,
        ANIM_THIEF_FEMALE_HALFORC,
        ANIM_MONK_MALE_HUMAN,
        ANIM_MONK_FEMALE_HUMAN,
        ANIM_MONK,
        ANIM_SKELETON,
        ANIM_DOOM_GUARD,
        ANIM_BEAR_BLACK,
        ANIM_BEAR_BROWN,
        ANIM_BEAR_CAVE,
        ANIM_BEAR_POLAR,
        ANIM_SPIDER_GIANT,
        ANIM_SPIDER_HUGE,
        ANIM_SPIDER_PHASE,
        ANIM_SPIDER_SWORD,
        ANIM_SPIDER_WRAITH,
        ANIM_WOLF,
        ANIM_WOLF_WINTER,
        ANIM_WEREWOLF,
        ANIM_WEREWOLF_GREATER,
        ANIM_OGRE,
        ANIM_CARRION_CRAWLER,
        ANIM_COW,
        ANIM_STATIC_BEGGAR_MAN,
        ANIM_STATIC_HARLOT_WOMAN,
        ANIM_STATIC_BOY,
        ANIM_STATIC_GIRL,
        ANIM_STATIC_FAT_MAN,
        ANIM_STATIC_FAT_WOMAN,
        ANIM_STATIC_PEASANT_MAN,
        ANIM_STATIC_PEASANT_WOMAN,
        ANIM_STATIC_NOBLE_MAN,
        ANIM_STATIC_NOBLE_WOMAN,
        ANIM_STATIC_SLAVE,
        ANIM_BAT_INSIDE,
        ANIM_CAT,
        ANIM_CHICKEN,
        ANIM_RAT,
        ANIM_SQUIRREL,
        ANIM_BAT_OUTSIDE,
        ANIM_BEGGAR_MAN,
        ANIM_HARLOT_WOMAN,
        ANIM_BOY,
        ANIM_GIRL,
        ANIM_FAT_MAN,
        ANIM_FAT_WOMAN,
        ANIM_PEASANT_MAN,
        ANIM_PEASANT_WOMAN,
        ANIM_NOBLE_MAN,
        ANIM_NOBLE_WOMAN,
        ANIM_SLAVE,
        ANIM_EAGLE,
        ANIM_VULTURE,
        ANIM_BIRD,
        ANIM_BIRD_INSIDE,
        ANIM_BEETLE,
        ANIM_BEETLE,
        ANIM_BEETLE_BOMBADIER,
        ANIM_BEETLE_BORING,
        ANIM_BEETLE_FIRE,
        ANIM_BEETLE_RHINOCEROUS,
        ANIM_CYCLOPS,
        ANIM_ETTIN,
        ANIM_GIANT_FIRE,
        ANIM_GIANT_FROST,
        ANIM_VERBEEG,
        ANIM_ELEMENTAL_EARTH,
        ANIM_ELEMENTAL_FIRE,
        ANIM_ELEMENTAL_WATER,
        ANIM_GHOUL,
        ANIM_GHOUL_2,
        ANIM_WIGHT,
        ANIM_ZOMBIE,
        ANIM_ZOMBIE_2,
        ANIM_WIGHT_2,
        ANIM_WIGHT_3,
        ANIM_MUMMY,
        ANIM_MUMMY_HISTACHII,
        ANIM_GOBLIN_W_AXE,
        ANIM_GOBLIN_W_BOW,
        ANIM_GOBLIN_ELITE_W_AXE,
        ANIM_GOBLIN_ELITE_W_BOW,
        ANIM_SVIRFNEBLIN,
        ANIM_GOLEM_IRON,
        ANIM_SKELETON_BONEGUARD,
        ANIM_MINOTAUR,
        ANIM_TROLL,
        ANIM_TROLL_ICE,
        ANIM_TROLL_SNOW,
        ANIM_UMBER_HULK,
        ANIM_TUNDRA_YETI,
        ANIM_ORC_W_AXE,
        ANIM_ORC_W_BOW,
        ANIM_ORC_ELITE_W_AXE,
        ANIM_ORC_ELITE_W_BOW,
        ANIM_ORC_SHAMAN,
        ANIM_OROG,
        ANIM_OROG_ELITE,
        ANIM_OROG_CHIEFTAN,
        ANIM_LIZARD_MAN_KING,
        ANIM_YUAN_TI_PRIEST,
        ANIM_SHADOW,
        ANIM_GHOST,
        ANIM_ANIMATED_PLATE,
        ANIM_ANIMATED_PLATE_2,
        ANIM_ANIMATED_PLATE_3,
        ANIM_BELHIFET,
        ANIM_BELHIFET_PRIEST,
        ANIM_LICH_LARREL,
        ANIM_MYCONID_RED,
        ANIM_MYCONID_BLUE,
        ANIM_SHRIEKER,
        ANIM_TANARI,
        ANIM_SALAMANDER,
        ANIM_SALAMANDER_FROST,
        ANIM_SKELETON_ARMORED,
        ANIM_YUAN_TI,
        ANIM_YUAN_TI_ELITE,
        ANIM_SKELETON_TATTERED,
        ANIM_LIZARD_MAN_2H,
        ANIM_LIZARD_MAN_1H,
        ANIM_ARUNDEL,
        ANIM_CREEPING_DOOM,
        ANIM_LICH_WHITE,
        ANIM_SVIRVNEBLIN_NOAXE,
        ANIM_BARBARIAN_1,
        ANIM_BARBARIAN_2,
        ANIM_BARBARIAN_3,
        ANIM_BARBARIAN_SHAMAN_1,
        ANIM_BARBARIAN_SHAMAN_2,
        ANIM_BARBARIAN_SHAMAN_3,
        ANIM_BARBARIAN_ICASARACHT,
        ANIM_CORNUGON,
        ANIM_DRAGON_WHITE,
        ANIM_DROWNED_DEAD,
        ANIM_GLABREZU,
        ANIM_GOLEM_ICE,
        ANIM_REMORHAZ,
        ANIM_SAHUAGIN,
        ANIM_SAHUAGIN_LARGE,
        ANIM_SEER,
        ANIM_ELEMENTAL_SHAMBLING_MOUND,
        ANIM_TROLL_SCRAG,
        ANIM_UMBER_HULK_VODYANOI,
        ANIM_WAILING_VIRGIN,
        ANIM_WATER_WEIRD,
        ANIM_WIGHT_BARROW,
        ANIM_WYVERN,
        ANIM_WYVERN_BIG_WHITE,
        ANIM_DOG_WILD,
        ANIM_DOG_WAR,
        ANIM_DOG_MOON,
        ANIM_HARPY,
        ANIM_BEHOLDER,
        ANIM_RAKSHASA,
        ANIM_SLIME_GREEN,
        ANIM_SLIME_OLIVE,
        ANIM_SLIME_MUSTARD,
        ANIM_SLIME_OCHRE,
        ANIM_DEER,
        ANIM_DJINNI,
        ANIM_DJINNI_LEGS,
        ANIM_DOPPLEGANGER,
        ANIM_DOPPLEGANGER_GREATER,
        ANIM_DRAGON_BLACK,
        ANIM_ELEMENTAL_AIR,
        ANIM_ELEMENTAL_AIR_SMALL,
        ANIM_ELEMENTAL_FIRE_BG2,
        ANIM_ETTERCAP,
        ANIM_GNOLL,
        ANIM_GOLEM_CLAY,
        ANIM_GOLEM_IRON_BG2,
        ANIM_GREATCAT_PANTHER,
        ANIM_GREATCAT_LEOPARD,
        ANIM_HOBGOBLIN,
        ANIM_IMP,
        ANIM_MINDFLAYER,
        ANIM_OGRE_HALF,
        ANIM_OTYUGH,
        ANIM_RAVER,
        ANIM_SLAYER,
        ANIM_SNAKE,
        ANIM_SPIDER_SMALL,
        ANIM_ABISHAI_WHITE,
        ANIM_BOAR_ARCTIC,
        ANIM_BUGBEAR,
        ANIM_BUGBEAR_CAPTAIN,
        ANIM_CHIMERA,
        ANIM_DARKTREE,
        ANIM_DRAGON_WHITE_BABY,
        ANIM_DRIDER_MALE,
        ANIM_DRIDER_FEMALE,
        ANIM_DROW,
        ANIM_DROW_FEMALE,
        ANIM_ELEMENTAL_WATER_SMALL,
        ANIM_FEYR,
        ANIM_FEYR_GREATER,
        ANIM_GIANT_FOMORIAN,
        ANIM_GOBLIN_CAPTAIN,
        ANIM_GOBLIN_SHAMAN,
        ANIM_GOBLIN_WORG,
        ANIM_GOBLIN_WORG_CAPTAIN,
        ANIM_HALFDRAGON_BLACK,
        ANIM_HALFDRAGON_RED,
        ANIM_HOOK_HORROR,
        ANIM_INVISIBLE_TINY,
        ANIM_INVISIBLE_SMALL,
        ANIM_INVISIBLE_MEDIUM,
        ANIM_INVISIBLE_LARGE,
        ANIM_INVISIBLE_GIANT,
        ANIM_INVISIBLE_HUGE,
        ANIM_ISAIR,
        ANIM_KEG_1,
        ANIM_KEG_2,
        ANIM_KEG_3,
        ANIM_LEMURE,
        ANIM_MADAE,
        ANIM_MALARITE,
        ANIM_ORC_W_AXE_UNDEAD,
        ANIM_ORC_SHAMAN_UNDEAD,
        ANIM_OROG_SHAMAN,
        ANIM_SHENRICAL,
        ANIM_WEREBADGER,
        ANIM_WERERAT,
        ANIM_WORG,
        ANIM_WILL_O_WISP,
        ANIM_YUAN_TI_HALF,
    };

    // FIXME: Should be static global.
    CString animationListNames[ANIM_TYPE_NUM] = {
        "DEATH_KNIGHT",
        "STATIC_NOBLE_MAN_CHAIR",
        "STATIC_NOBLE_MAN_MATTE",
        "STATIC_NOBLE_WOMAN_CHAIR",
        "STATIC_NOBLE_WOMAN_MATTE",
        "STATIC_PEASANT_MAN_CHAIR",
        "STATIC_PEASANT_MAN_STOOL",
        "STATIC_PEASANT_MAN_MATTE",
        "STATIC_PEASANT_WOMAN_CHAIR",
        "STATIC_PEASANT_WOMAN_MATTE",
        "STATIC_HUMANCLERIC_MAN_CHAIR",
        "SLEEPING_MAN_HUMAN",
        "SLEEPING_WOMAN_HUMAN",
        "SLEEPING_FAT_MAN_HUMAN",
        "SLEEPING_DWARF",
        "SLEEPING_MAN_ELF",
        "SLEEPING_WOMAN_ELF",
        "SLEEPING_MAN_HALFLING",
        "CLERIC_MALE_HUMAN",
        "CLERIC_MALE_ELF",
        "CLERIC_MALE_DWARF",
        "CLERIC_MALE_HALFLING",
        "CLERIC_MALE_GNOME",
        "CLERIC MALE HALFORC",
        "CLERIC_FEMALE_HUMAN",
        "CLERIC_FEMALE_ELF",
        "CLERIC_FEMALE_DWARF",
        "CLERIC_FEMALE_HALFLING",
        "CLERIC_FEMALE_GNOME",
        "CLERIC FEMALE HALFORC",
        "FIGHTER_MALE_HUMAN",
        "FIGHTER_MALE_ELF",
        "FIGHTER_MALE_DWARF",
        "FIGHTER_MALE_HALFLING",
        "FIGHTER_MALE_GNOME",
        "FIGHTER MALE HALFORC",
        "FIGHTER_FEMALE_HUMAN",
        "FIGHTER_FEMALE_ELF",
        "FIGHTER_FEMALE_DWARF",
        "FIGHTER_FEMALE_HALFLING",
        "FIGHTER_FEMALE_GNOME",
        "FIGHTER FEMALE HALFORC",
        "MAGE_MALE_HUMAN",
        "MAGE_MALE_ELF",
        "MAGE_MALE_DWARF",
        "MAGE_MALE_GNOME",
        "MAGE MALE HALFORC",
        "MAGE_FEMALE_HUMAN",
        "MAGE_FEMALE_ELF",
        "MAGE_FEMALE_DWARF",
        "MAGE_FEMALE_GNOME",
        "MAGE FEMALE HALFORC",
        "THIEF_MALE_HUMAN",
        "THIEF_MALE_ELF",
        "THIEF_MALE_DWARF",
        "THIEF_MALE_HALFLING",
        "THIEF_MALE_GNOME",
        "THIEF MALE HALFORC",
        "THIEF_FEMALE_HUMAN",
        "THIEF_FEMALE_ELF",
        "THIEF_FEMALE_DWARF",
        "THIEF_FEMALE_HALFLING",
        "THIEF_FEMALE_GNOME",
        "THIEF FEMALE HALFORC",
        "MONK MALE HUMAN",
        "MONK FEMALE HUMAN",
        "MONK",
        "SKELETON",
        "DOOM_GUARD",
        "BEAR_BLACK",
        "BEAR_BROWN",
        "BEAR_CAVE",
        "BEAR_POLAR",
        "SPIDER_GIANT",
        "SPIDER_HUGE",
        "SPIDER_PHASE",
        "SPIDER_SWORD",
        "SPIDER_WRAITH",
        "WOLF",
        "WOLF_WINTER",
        "WEREWOLF",
        "WEREWOLF_GREATER",
        "OGRE",
        "CARRION_CRAWLER",
        "COW",
        "STATIC_BEGGAR_MAN",
        "STATIC_HARLOT_WOMAN",
        "STATIC_BOY",
        "STATIC_GIRL",
        "STATIC_FAT_MAN",
        "STATIC_FAT_WOMAN",
        "STATIC_PEASANT_MAN",
        "STATIC_PEASANT_WOMAN",
        "STATIC_NOBLE_MAN",
        "STATIC_NOBLE_WOMAN",
        "STATIC_SLAVE",
        "BAT_INSIDE",
        "CAT",
        "CHICKEN",
        "RAT",
        "SQUIRREL",
        "BAT_OUTSIDE",
        "BEGGAR_MAN",
        "HARLOT_WOMAN",
        "BOY",
        "GIRL",
        "FAT_MAN",
        "FAT_WOMAN",
        "PEASANT_MAN",
        "PEASANT_WOMAN",
        "NOBLE_MAN",
        "NOBLE_WOMAN",
        "SLAVE",
        "EAGLE",
        "VULTURE",
        "BIRD",
        "BIRD_INSIDE",
        "BEETLE",
        "BEETLE",
        "BEETLE_BOMBADIER",
        "BEETLE_BORING",
        "BEETLE_FIRE",
        "BEETLE_RHINOCEROUS",
        "CYCLOPS",
        "ETTIN",
        "GIANT_FIRE",
        "GIANT_FROST",
        "VERBEEG",
        "ELEMENTAL_EARTH",
        "ELEMENTAL_FIRE",
        "ELEMENTAL_WATER",
        "GHOUL",
        "GHOUL_2",
        "WIGHT",
        "ZOMBIE",
        "ZOMBIE_2",
        "WIGHT_2",
        "WIGHT_3",
        "MUMMY",
        "MUMMY_HISTACHII",
        "GOBLIN_W_AXE",
        "GOBLIN_W_BOW",
        "GOBLIN_ELITE_W_AXE",
        "GOBLIN_ELITE_W_BOW",
        "SVIRFNEBLIN",
        "GOLEM_IRON",
        "SKELETON_BONEGUARD",
        "MINOTAUR",
        "TROLL",
        "TROLL_ICE",
        "TROLL_SNOW",
        "UMBER_HULK",
        "TUNDRA_YETI",
        "ORC_W_AXE",
        "ORC_W_BOW",
        "ORC_ELITE_W_AXE",
        "ORC_ELITE_W_BOW",
        "ORC_SHAMAN",
        "OROG",
        "OROG_ELITE",
        "OROG_CHIEFTAN",
        "LIZARD_MAN_KING",
        "YUAN_TI_PRIEST",
        "SHADOW",
        "GHOST",
        "ANIMATED_PLATE",
        "ANIMATED_PLATE_2",
        "ANIMATED_PLATE_3",
        "BELHIFET",
        "BELHIFET_PRIEST",
        "LICH_LARREL",
        "MYCONID_RED",
        "MYCONID_BLUE",
        "SHRIEKER",
        "TANARI",
        "SALAMANDER",
        "SALAMANDER_FROST",
        "SKELETON_ARMORED",
        "YUAN_TI",
        "YUAN_TI_ELITE",
        "SKELETON_TATTERED",
        "LIZARD_MAN_2H",
        "LIZARD_MAN_1H",
        "ARUNDEL",
        "CREEPING_DOOM",
        "LICH_WHITE",
        "SVIRVNEBLIN_NOAXE",
        "BARBARIAN_1",
        "BARBARIAN_2",
        "BARBARIAN_3",
        "BARBARIAN_SHAMAN_1",
        "BARBARIAN_SHAMAN_2",
        "BARBARIAN_SHAMAN_3",
        "BARBARIAN_ICASARACHT",
        "CORNUGON",
        "DRAGON_WHITE",
        "DROWNED_DEAD",
        "GLABREZU",
        "GOLEM_ICE",
        "REMORHAZ",
        "SAHUAGIN",
        "SAHUAGIN_LARGE",
        "SEER",
        "ELEMENTAL_SHAMBLING_MOUND",
        "TROLL_SCRAG",
        "UMBER_HULK_VODYANOI",
        "WAILING_VIRGIN",
        "WATER_WEIRD",
        "WIGHT_BARROW",
        "WYVERN",
        "WYVERN_BIG_WHITE",
        "DOG_WILD",
        "DOG_WAR",
        "DOG_MOON",
        "HARPY",
        "BEHOLDER",
        "RAKSHASA",
        "SLIME_GREEN",
        "SLIME_OLIVE",
        "SLIME_MUSTARD",
        "SLIME_OCHRE",
        "DEER",
        "DJINNI",
        "DJINNI_LEGS",
        "DOPPLEGANGER",
        "DOPPLEGANGER_GREATER",
        "DRAGON_BLACK",
        "ELEMENTAL_AIR",
        "ELEMENTAL_AIR_SMALL",
        "ELEMENTAL_FIRE_BG2",
        "ETTERCAP",
        "GNOLL",
        "GOLEM_CLAY",
        "GOLEM_IRON_BG2",
        "GREATCAT_PANTHER",
        "GREATCAT_LEOPARD",
        "HOBGOBLIN",
        "IMP",
        "MINDFLAYER",
        "OGRE_HALF",
        "OTYUGH",
        "RAVER",
        "SLAYER",
        "SNAKE",
        "SPIDER_SMALL",
        "ABISHAI_WHITE",
        "BOAR_ARCTIC",
        "BUGBEAR",
        "BUGBEAR_CAPTAIN",
        "CHIMERA",
        "DARKTREE",
        "DRAGON_WHITE_BABY",
        "DRIDER_MALE",
        "DRIDER_FEMALE",
        "DROW",
        "DROW_FEMALE",
        "ELEMENTAL_WATER_SMALL",
        "FEYR",
        "FEYR_GREATER",
        "GIANT_FOMORIAN",
        "GOBLIN_CAPTAIN",
        "GOBLIN_SHAMAN",
        "GOBLIN_WORG",
        "GOBLIN_WORG_CAPTAIN",
        "HALFDRAGON_BLACK",
        "HALFDRAGON_RED",
        "HOOK_HORROR",
        "INVISIBLE_TINY",
        "INVISIBLE_SMALL",
        "INVISIBLE_MEDIUM",
        "INVISIBLE_LARGE",
        "INVISIBLE_GIANT",
        "INVISIBLE_HUGE",
        "ISAIR",
        "KEG_1",
        "KEG_2",
        "KEG_3",
        "LEMURE",
        "MADAE",
        "MALARITE",
        "ORC_W_AXE_UNDEAD",
        "ORC_SHAMAN_UNDEAD",
        "OROG_SHAMAN",
        "SHENRICAL",
        "WEREBADGER",
        "WERERAT",
        "WORG",
        "WILL_O_WISP",
        "YUAN_TI_HALF",
    };

    if (!m_group.m_memberList.IsEmpty()) {
        CRect rFx;
        CPoint ptRef;

        LONG nCharacterId = m_group.GetGroupLeader();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            switch (direction) {
            case '1':
                m_currArmor++;
                if (m_currArmor > '4') {
                    m_currArmor = '1';
                }

                pSprite->GetAnimation()->EquipArmor(static_cast<CHAR>(m_currArmor),
                    pSprite->GetBaseStats()->m_colors);

                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    CString("Armor Level ") + static_cast<CHAR>(m_currArmor),
                    -1,
                    0);
                break;
            case '4':
                for (int index = 0; index < ANIM_TYPE_NUM; index++) {
                    pSprite->UnequipAll(FALSE);
                    pSprite->GetAnimation()->SetAnimationType(animationList[index],
                        pSprite->GetBaseStats()->m_colors,
                        0);
                    pSprite->EquipAll(FALSE);

                    for (SHORT nSequence = 0; nSequence <= 15; nSequence++) {
                        if (nSequence != 8) {
                            pSprite->GetAnimation()->SetSequence(nSequence);
                            pSprite->SetSequence(nSequence);

                            for (SHORT nDirection = 0; nDirection <= 15; nDirection++) {
                                pSprite->GetAnimation()->ChangeDirection(nDirection);

                                for (INT nFrame = 0; nFrame < 40; nFrame--) {
                                    pSprite->GetAnimation()->CalculateFxRect(rFx, ptRef, pSprite->m_posZ);
                                    pSprite->GetAnimation()->IncrementFrame();
                                }
                            }
                        }
                    }
                }
                break;
            case '5':
                for (int index = 0; index < ANIM_TYPE_NUM; index++) {
                    pSprite->UnequipAll(FALSE);
                    pSprite->GetAnimation()->SetAnimationType(animationList[index],
                        pSprite->GetBaseStats()->m_colors,
                        0);
                    pSprite->EquipAll(FALSE);

                    for (SHORT nSequence = 0; nSequence <= 15; nSequence++) {
                        if (nSequence == 8) {
                            pSprite->GetAnimation()->SetSequence(nSequence);
                            pSprite->SetSequence(nSequence);

                            for (SHORT nDirection = 0; nDirection <= 15; nDirection++) {
                                pSprite->GetAnimation()->ChangeDirection(nDirection);

                                for (INT nFrame = 0; nFrame < 40; nFrame--) {
                                    pSprite->GetAnimation()->CalculateFxRect(rFx, ptRef, pSprite->m_posZ);
                                    pSprite->GetAnimation()->IncrementFrame();
                                }
                            }
                        }
                    }
                }
                break;
            case '6':
                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.RemoveObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                                  pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->m_bBumpable,
                        pSprite->m_bOnSearchMap);
                }

                m_currAnimation--;
                if (m_currAnimation < 0) {
                    m_currAnimation = ANIM_TYPE_NUM - 1;
                }

                pSprite->UnequipAll(FALSE);
                pSprite->GetAnimation()->SetAnimationType(animationList[m_currAnimation],
                    pSprite->GetBaseStats()->m_colors,
                    0);
                pSprite->EquipAll(FALSE);

                pSprite->GetAnimation()->SetSequence(pSprite->m_nSequence);

                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    animationListNames[m_currAnimation],
                    -1,
                    FALSE);

                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.AddObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                               pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->m_bBumpable,
                        pSprite->m_bOnSearchMap);
                }
                break;
            case '7':
                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.RemoveObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                                  pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->m_bBumpable,
                        pSprite->m_bOnSearchMap);
                }

                m_currAnimation++;
                if (m_currAnimation == ANIM_TYPE_NUM) {
                    m_currAnimation = 0;
                }

                pSprite->UnequipAll(FALSE);
                pSprite->GetAnimation()->SetAnimationType(animationList[m_currAnimation],
                    pSprite->GetBaseStats()->m_colors,
                    0);
                pSprite->EquipAll(FALSE);

                pSprite->GetAnimation()->SetSequence(pSprite->m_nSequence);

                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    animationListNames[m_currAnimation],
                    -1,
                    FALSE);

                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.AddObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                               pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->m_bBumpable,
                        pSprite->m_bOnSearchMap);
                }
                break;
            }

            m_cObjectArray.ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// FIXME: Use reference instead of object instance.
//
// 0x5B7510
CGameArea* CInfGame::GetArea(CString sName)
{
    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL) {
            if (m_gameAreas[index]->m_resRef == sName) {
                return m_gameAreas[index];
            }
        }
    }

    return NULL;
}

// 0x68DE70
CGameArea* CInfGame::GetArea(SHORT nArea)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1416
    UTIL_ASSERT(nArea < CINFGAME_MAX_AREAS);

    return m_gameAreas[nArea];
}

// 0x5B75C0
LONG CInfGame::GetGroundPile(LONG iSprite)
{
    CGameSprite* pSprite;

    BYTE rc = m_cObjectArray.GetShare(iSprite,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);

    if (rc != CGameObjectArray::SUCCESS) {
        return CGameObjectArray::INVALID_INDEX;
    }

    CPoint ptSprite = pSprite->GetPos();
    LONG iGroundPile = pSprite->GetArea()->GetGroundPile(ptSprite);
    if (iGroundPile == CGameObjectArray::INVALID_INDEX) {
        ptSprite.x += CPathSearch::GRID_SQUARE_SIZEX / 2
            - ptSprite.x % CPathSearch::GRID_SQUARE_SIZEX;
        ptSprite.y += CPathSearch::GRID_SQUARE_SIZEY / 2
            - ptSprite.y % CPathSearch::GRID_SQUARE_SIZEY;

        CGameContainer* pContainer = new CGameContainer(pSprite->GetArea(),
            CRect(ptSprite.x - CPathSearch::GRID_SQUARE_SIZEX / 2,
                ptSprite.y - CPathSearch::GRID_SQUARE_SIZEY / 2,
                ptSprite.x + CPathSearch::GRID_SQUARE_SIZEX / 2 + 1,
                ptSprite.y + CPathSearch::GRID_SQUARE_SIZEY / 2 + 1));
        iGroundPile = pContainer->GetId();
    }

    m_cObjectArray.ReleaseShare(iSprite,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return iGroundPile;
}

// 0x5C7440
//
// An item that could not be placed where the loot/give code intended must not
// leak.  Droppable items go on the disposable list; an undroppable (flag 0x1)
// item is force-fit into a free personal slot on the looter -- bumping a
// droppable occupant if necessary -- so quest items can never be lost.
void CInfGame::PutBackItem(CItem* pItem, LONG nLooterId)
{
    if ((pItem->GetFlagsFile() & 0x1) == 0) {
        AddDisposableItem(pItem);
        return;
    }

    CGameSprite* pLooter;
    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nLooterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pLooter),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    INT nEmptySlot = -1;
    INT nBumpSlot = -1;
    for (INT nSlot = 18; nSlot < 34; nSlot++) {
        CItem* pSlotItem = pLooter->GetEquipment()->m_items[nSlot];
        if (pSlotItem == NULL) {
            nEmptySlot = nSlot;
        } else if ((pSlotItem->GetFlagsFile() & 0x1) == 0) {
            nBumpSlot = nSlot;
        }
    }

    if (nEmptySlot < 0) {
        if (nBumpSlot < 0) {
            AddDisposableItem(pItem);
            m_cObjectArray.ReleaseShare(nLooterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return;
        }
        CItem* pBumped = pLooter->GetEquipment()->m_items[nBumpSlot];
        if (pBumped != NULL) {
            AddDisposableItem(pBumped);
        }
        pLooter->GetEquipment()->m_items[nBumpSlot] = pItem;
    } else {
        pLooter->GetEquipment()->m_items[nEmptySlot] = pItem;
    }

    CMessage* message = new CMessageSpriteEquipment(pLooter, nLooterId, nLooterId);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    m_cObjectArray.ReleaseShare(nLooterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// 0x5B7730
BOOL CInfGame::CanTakeContainerItem(LONG nContainerId, SHORT nSlotNum, CGameSprite* pLooter, SHORT nPortrait)
{
    CResRef cResIcon;
    CResRef cResItem;

    CGameContainer* pContainer;
    if (m_cObjectArray.GetDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE) != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    CItem* pItem = pContainer->GetItem(nSlotNum);

    m_cObjectArray.ReleaseDeny(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    if (pItem == NULL) {
        return FALSE;
    }

    // Gold (MISC07) merges into party gold, so it can always be taken.
    if (pItem->GetResRef() == "MISC07") {
        return TRUE;
    }

    // Otherwise the looter needs a free backpack slot (18 .. 18 + PERSONAL_INVENTORY_SIZE).
    CItem* pScanItem;
    STRREF description;
    WORD wCount;
    SHORT nSlot = 18;
    do {
        InventoryInfoPersonal(nPortrait, nSlot, pScanItem, description, cResIcon, cResItem, wCount, TRUE);
        if (cResIcon != "") {
            nSlot++;
        }
    } while (cResIcon != "" && nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18);

    return nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18;
}

// 0x5BA230
INT CInfGame::CanDragItemToSlot(SHORT nPortraitNum, INT nSlotNum, CItem*& pItem, STRREF& errorCode)
{
    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortraitNum);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        errorCode = 0x249D;
        return 0;
    }

    INT nResult = pSprite->CanEquipItemInSlot(nSlotNum, pItem, errorCode);

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return nResult;
}

// 0x5B7850
BOOL CInfGame::SwapItemGround(LONG nContainerId, SHORT nSlotNum, CItem*& pItem, STRREF& errorCode, WORD wCount, BOOLEAN bFromServer)
{
    CGameContainer* pContainer;

    if (m_cObjectArray.GetDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE) != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    // Not controlling this container in a multiplayer session: forward to the
    // server and return its result.
    if (!pContainer->InControl()
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && bFromServer == FALSE) {
        BOOLEAN bForwarded = g_pBaldurChitin->GetBaldurMessage()->SwapItemRequest(
            0x47, // 0x0084CF5B
            nContainerId,
            nSlotNum,
            pItem,
            errorCode,
            wCount);

        m_cObjectArray.ReleaseDeny(nContainerId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        return bForwarded;
    }

    CItem* pExisting = pContainer->GetItem(nSlotNum);

    if (pExisting != NULL) {
        if (pItem == NULL
            && pExisting->GetMaxStackable() > 1
            && wCount < pExisting->GetUsageCount(0)) {
            // Take part of the container's stack into the hand.
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 12276
            UTIL_ASSERT(wCount != 0);

            pExisting->SetUsageCount(0, pExisting->GetUsageCount(0) - wCount);

            pItem = new CItem(pExisting->GetResRef(), 0, 0, 0, 0, 0);
            pItem->SetUsageCount(0, wCount);
            pItem->m_flags = ((pExisting->m_flags ^ pItem->m_flags) & 1) ^ pItem->m_flags;

            goto done;
        }

        if (pItem != NULL) {
            if (pExisting->GetMaxStackable() > 1
                && pItem->GetResRef() == pExisting->GetResRef()
                && ((pItem->m_flags ^ pExisting->m_flags) & 1) == 0) {
                // Merge the held stack into the container's matching stack.
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 12286
                UTIL_ASSERT(wCount != 0);

                WORD nRoom = pExisting->GetMaxStackable() - pExisting->GetUsageCount(0);
                WORD nTransfer = (pItem->GetUsageCount(0) < nRoom) ? pItem->GetUsageCount(0) : nRoom;
                if (wCount < nTransfer) {
                    nTransfer = wCount;
                }

                pExisting->SetUsageCount(0, pExisting->GetUsageCount(0) + nTransfer);

                if (pItem->GetUsageCount(0) == nTransfer) {
                    // NOTE: Uninline.
                    AddDisposableItem(pItem);
                    pItem = NULL;
                } else {
                    pItem->SetUsageCount(0, pItem->GetUsageCount(0) - nTransfer);
                }

                goto done;
            }

            // Swap the held item with the container's contents.
            goto placeHeld;
        }

        // Take the whole stack.
        goto clearSlot;
    }

placeHeld:
    if (pItem == NULL) {
        goto clearSlot;
    }

    // Dropping gold (MISC07) onto a slot that holds gold folds it into the
    // party purse and empties the slot.
    if (pExisting != NULL && pExisting->GetResRef() == "MISC07") {
        AddPartyGold(pExisting->GetUsageCount(0));

        // NOTE: Uninline.
        AddDisposableItem(pExisting);

        pContainer->SetItem(nSlotNum, NULL);

        CMessage* message = new CMessageContainerAddItem(CItem(),
            nSlotNum,
            FALSE,
            pContainer->m_id,
            pContainer->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        goto done;
    }

    {
        CMessage* message = new CMessageContainerAddItem(*pItem,
            nSlotNum,
            FALSE,
            pContainer->m_id,
            pContainer->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    if (pItem != NULL) {
        // NOTE: Uninline.
        AddDisposableItem(pItem);
    }

    pItem = pExisting;
    goto done;

clearSlot:
    pContainer->SetItem(nSlotNum, NULL);

    {
        CMessage* message = new CMessageContainerAddItem(CItem(),
            nSlotNum,
            FALSE,
            pContainer->m_id,
            pContainer->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    pItem = pExisting;

done:
    m_cObjectArray.ReleaseDeny(nContainerId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    errorCode = -1;
    return TRUE;
}

// 0x5B7FF0
SHORT CInfGame::GetNumGroundSlots(LONG nContainerId)
{
    SHORT nSlots;

    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);

    if (rc == CGameObjectArray::SUCCESS) {
        nSlots = static_cast<SHORT>(pContainer->m_lstItems.GetCount());

        m_cObjectArray.ReleaseShare(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        nSlots = 0;
    }

    return nSlots;
}

// 0x5B8040
void CInfGame::InventoryInfoGround(LONG nContainerId, SHORT nSlotNum, CItem*& pItem, STRREF& description, CResRef& cResIcon, CResRef& cResItem, WORD& wCount)
{
    description = -1;
    cResIcon = "";
    cResItem = "";
    wCount = 0;

    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);

    if (rc == CGameObjectArray::SUCCESS) {
        pItem = pContainer->GetItem(nSlotNum);
        if (pItem != NULL) {
            description = pItem->GetGenericName();
            cResIcon = pItem->GetItemIcon();
            cResItem = pItem->GetResRef();

            if (pItem->GetMaxStackable() > 1) {
                wCount = pItem->GetUsageCount(0);
            }
        }

        m_cObjectArray.ReleaseShare(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5B8130
WORD CInfGame::GetContainerType(LONG nContainerId)
{
    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return -1;
    }

    WORD nContainerType = pContainer->m_containerType;

    m_cObjectArray.ReleaseShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return nContainerType;
}

// 0x5B8180
void CInfGame::CompressContainer(LONG nContainerId)
{
    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetDeny(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        pContainer->CompressContainer();

        m_cObjectArray.ReleaseDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5BA080
INT CInfGame::CheckItemUsable(CGameSprite* pSprite, CItem* pItem)
{
    INT nType = pItem->GetItemType();
    if (nType != 35 && nType != 11) {
        return 1;
    }

    DWORD nNotUsable1 = pItem->GetNotUsableBy();
    DWORD nNotUsable2 = pItem->GetNotUsableBy2();
    DWORD nClassMask = pSprite->GetDerivedStats()->m_classMask;
    DWORD nSpecialization = pSprite->GetDerivedStats()->m_nSpecialization;

    if ((nNotUsable1 & 0x1) == 0
        && (nClassMask & CLASSMASK_BARBARIAN) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x2) == 0
        && (nClassMask & CLASSMASK_BARD) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x4) == 0
        && (nClassMask & CLASSMASK_CLERIC) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_CLERIC) == 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x8) == 0
        && (nClassMask & CLASSMASK_DRUID) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x10) == 0
        && (nClassMask & CLASSMASK_FIGHTER) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x20) == 0
        && (nClassMask & CLASSMASK_MONK) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_MONK) == 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x40) == 0
        && (nClassMask & CLASSMASK_PALADIN) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_PALADIN) == 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x80) == 0
        && (nClassMask & CLASSMASK_RANGER) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x100) == 0
        && (nClassMask & CLASSMASK_ROGUE) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x200) == 0
        && (nClassMask & CLASSMASK_SORCERER) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x400) == 0
        && (nClassMask & CLASSMASK_WIZARD) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_WIZARD) == 0) {
        return 1;
    }

    if ((nClassMask & CLASSMASK_ROGUE) != 0
        && pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] > 0) {
        return 2;
    }

    if ((nClassMask & CLASSMASK_BARD) != 0
        && pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] > 0) {
        return 2;
    }

    return 0;
}

// 0x5B9C60
// Resolves the character shown at portrait slot nPortrait (m_characterPortraits) and
// reports whether they can use pItem. a4 selects the GetShare thread (THREAD_ASYNCH
// when non-zero, else THREAD_1) and is forwarded to CanUseItem, which ignores it.
INT CInfGame::CanCharacterUseItem(SHORT nPortrait, CItem* pItem, STRREF& errorCode, BOOL a4)
{
    LONG nObjectId = CGameObjectArray::INVALID_INDEX;
    if (nPortrait < m_nCharacters) {
        nObjectId = m_characterPortraits[nPortrait];
    }

    BYTE threadNum = (a4 == 0) ? CGameObjectArray::THREAD_1 : CGameObjectArray::THREAD_ASYNCH;

    CGameSprite* pSprite;
    BYTE share;
    do {
        share = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(
            nObjectId, threadNum, reinterpret_cast<CGameObject**>(&pSprite), INFINITE);
    } while (share == CGameObjectArray::SHARED || share == CGameObjectArray::DENIED);

    if (share != CGameObjectArray::SUCCESS) {
        errorCode = 0x249D;
        return 0;
    }

    INT nResult = pSprite->CanUseItem(pSprite, pItem, errorCode, a4);
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nObjectId, threadNum, INFINITE);
    return nResult;
}

// 0x5BACE0
SHORT CInfGame::GetNumQuickWeaponSlots(SHORT nPortrait)
{
    SHORT nSlots;

    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortrait);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        nSlots = static_cast<SHORT>(pSprite->GetNumQuickWeaponSlots());
    } else {
        nSlots = 0;
    }

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return nSlots;
}

// Defined in CGameSprite.cpp (no header declaration in the original).
void RefreshWeaponSetButtons(CGameSprite* pSprite);

// 0x5B81C0
BOOL CInfGame::SwapItemPersonal(SHORT nPortraitNum, SHORT nSlotNum, CItem*& pItem, STRREF& errorCode, WORD wCount, BOOLEAN bFromServer)
{
    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortraitNum);

    CItem* pNewItem = pItem;

    BOOL bResult = TRUE;
    BOOL bUsable = TRUE;
    BOOL bGenericEquip = TRUE;
    BOOL bQuickItemSlot = FALSE;

    WORD nNewItemType = 0;
    if (pNewItem != NULL) {
        nNewItemType = pNewItem->GetItemType();
    }

    errorCode = -1;

    if (CGameSpriteEquipment::NUM_SLOT <= nSlotNum) {
        errorCode = 0x249E;
        return FALSE;
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        errorCode = 0x249D;
        return FALSE;
    }

    // Equipment may only be changed on a sprite this machine controls.
    if (!pSprite->InControl()) {
        m_cObjectArray.ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        errorCode = 0x50D7;
        return FALSE;
    }

    CAIObjectType cType;
    cType.Set(pSprite->GetAIType());

    CGameSpriteEquipment* pEquip = pSprite->GetEquipment();

    if (pNewItem != NULL) {
        DWORD nNotUsableBy = pNewItem->GetNotUsableBy();

        // Alignment restrictions.
        if ((nNotUsableBy & 0x1000) != 0
            && (cType.m_nAlignment == CAIOBJECTTYPE_CHAOTIC_GOOD
                || cType.m_nAlignment == CAIOBJECTTYPE_CHAOTIC_NEUTRAL
                || cType.m_nAlignment == CAIOBJECTTYPE_CHAOTIC_EVIL)) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x2000) != 0
            && (cType.m_nAlignment == CAIOBJECTTYPE_CHAOTIC_EVIL
                || cType.m_nAlignment == CAIOBJECTTYPE_NEUTRAL_EVIL
                || cType.m_nAlignment == CAIOBJECTTYPE_LAWFUL_EVIL)) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x4000) != 0
            && (cType.m_nAlignment == CAIOBJECTTYPE_CHAOTIC_GOOD
                || cType.m_nAlignment == CAIOBJECTTYPE_NEUTRAL_GOOD
                || cType.m_nAlignment == CAIOBJECTTYPE_LAWFUL_GOOD)) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x8000) != 0
            && (cType.m_nAlignment == CAIOBJECTTYPE_CHAOTIC_NEUTRAL
                || cType.m_nAlignment == CAIOBJECTTYPE_NEUTRAL
                || cType.m_nAlignment == CAIOBJECTTYPE_LAWFUL_NEUTRAL)) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x10000) != 0
            && (cType.m_nAlignment == CAIOBJECTTYPE_LAWFUL_GOOD
                || cType.m_nAlignment == CAIOBJECTTYPE_LAWFUL_NEUTRAL
                || cType.m_nAlignment == CAIOBJECTTYPE_LAWFUL_EVIL)) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x20000) != 0
            && (cType.m_nAlignment == CAIOBJECTTYPE_NEUTRAL_EVIL
                || cType.m_nAlignment == CAIOBJECTTYPE_NEUTRAL
                || cType.m_nAlignment == CAIOBJECTTYPE_NEUTRAL_GOOD)) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }

        // Race restrictions.
        if ((nNotUsableBy & 0x800000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_ELF) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x1000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_DWARF) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x2000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HALF_ELF) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x4000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HALFLING) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x8000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HUMAN) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x10000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_GNOME) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if ((nNotUsableBy & 0x20000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HALF_ORC) {
            bResult = FALSE;
            bUsable = FALSE;
            errorCode = 0x24A6;
        }

        if (bResult == FALSE) {
            goto releaseShareAndReturn;
        }

        if (GetRuleTables().ShouldCheckItemRequirements(pNewItem)) {
            if (pSprite->m_derivedStats.m_nLevel < pNewItem->GetMinLevelRequired()) {
                bResult = FALSE;
                bUsable = FALSE;
                errorCode = 0x24A6;
            }
            if (pSprite->m_derivedStats.m_nSTR < static_cast<SHORT>(pNewItem->GetMinSTRRequired())) {
                bResult = FALSE;
                bUsable = FALSE;
                errorCode = 0x24A6;
            }
            if (pSprite->m_derivedStats.m_nINT < static_cast<SHORT>(pNewItem->GetMinINTRequired())) {
                bResult = FALSE;
                bUsable = FALSE;
                errorCode = 0x24A6;
            }
            if (pSprite->m_derivedStats.m_nDEX < static_cast<SHORT>(pNewItem->GetMinDEXRequired())) {
                bResult = FALSE;
                bUsable = FALSE;
                errorCode = 0x24A6;
            }
            if (pSprite->m_derivedStats.m_nWIS < static_cast<SHORT>(pNewItem->GetMinWISRequired())) {
                bResult = FALSE;
                bUsable = FALSE;
                errorCode = 0x24A6;
            }
            if (pSprite->m_derivedStats.m_nCON < static_cast<SHORT>(pNewItem->GetMinCONRequired())) {
                bResult = FALSE;
                bUsable = FALSE;
                errorCode = 0x24A6;
            }
            if (pSprite->m_derivedStats.m_nCHR < static_cast<SHORT>(pNewItem->GetMinCHRRequired())) {
                bResult = FALSE;
                errorCode = 0x24A6;
                goto releaseShareAndReturn;
            }
        }

        if (bResult == FALSE) {
            goto releaseShareAndReturn;
        }
    }

    // Validate that the item type is allowed in the target slot.
    switch (nSlotNum) {
    case 0: // amulet
        if (pNewItem != NULL && nNewItemType != 1 && nNewItemType != 0x46) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 1: // armor
        if (pNewItem != NULL
            && nNewItemType != 0x3C && nNewItemType != 0x3D && nNewItemType != 0x42
            && nNewItemType != 0x3E && nNewItemType != 0x3F && nNewItemType != 0x40
            && nNewItemType != 0x41 && nNewItemType != 0x43 && nNewItemType != 0x44) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 2: // belt
        if (pNewItem != NULL && nNewItemType != 3) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 3: // boots
        if (pNewItem != NULL && nNewItemType != 4) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 4: // cloak
        if (pNewItem != NULL && nNewItemType != 0x20) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 5: // gauntlets
        if (pNewItem != NULL && nNewItemType != 6 && nNewItemType != 0x49) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 6: // helmet
        if (pNewItem != NULL && nNewItemType != 7 && nNewItemType != 0x48) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 7: // left ring
    case 8: // right ring
        if (pNewItem != NULL && nNewItemType != 10) {
            errorCode = 0x249F;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        break;
    case 10: // fist (cannot hold an item)
        errorCode = 0x249F;
        bResult = FALSE;
        goto releaseShareAndReturn;
    case 0xB: // ammo quiver
    case 0xC:
    case 0xD:
    case 0xE:
        if (pNewItem != NULL && nNewItemType != 5 && nNewItemType != 0x1F && nNewItemType != 0xE) {
            bResult = FALSE;
            errorCode = 0x249F;
            bUsable = FALSE;
        }
        bGenericEquip = FALSE;
        break;
    case 0xF: // quick item slots
    case 0x10:
    case 0x11:
        bQuickItemSlot = TRUE;
        if (pNewItem != NULL) {
            if (nNewItemType != 0 && nNewItemType != 8 && nNewItemType != 9
                && nNewItemType != 0xB && nNewItemType != 0x23 && nNewItemType != 0x47) {
                bResult = FALSE;
                errorCode = 0x249F;
                bUsable = FALSE;
                bGenericEquip = FALSE;
                break;
            }
            if (pNewItem->GetResRef() == "SPWI110Z") {
                errorCode = 0x249F;
                bResult = FALSE;
                bUsable = FALSE;
            } else if (pNewItem->GetResRef() == "MISC86") {
                errorCode = 0x249F;
                bResult = FALSE;
                bUsable = FALSE;
            } else if (pNewItem->GetAbilityCount() == 0) {
                errorCode = 0x249F;
                bResult = FALSE;
                bUsable = FALSE;
            } else if (!CheckItemUsable(pSprite, pNewItem)) {
                errorCode = 0x24A6;
                bResult = FALSE;
                bUsable = FALSE;
            }
        }
        bGenericEquip = FALSE;
        break;
    case 0x2B: // main-hand weapon, sets 0-3
    case 0x2D:
    case 0x2F:
    case 0x31:
        if (pSprite->m_nWeaponSet != (nSlotNum - 0x2B) / 2) {
            bGenericEquip = FALSE;
        }
        if (pNewItem == NULL) {
            goto weaponOffhandCheck;
        }
        if (nNewItemType != 0xF) {
            if (nNewItemType != 0x10 && nNewItemType != 0x11 && nNewItemType != 0x2C
                && nNewItemType != 0x12 && nNewItemType != 0x13 && nNewItemType != 0x14
                && nNewItemType != 0x39 && nNewItemType != 0x45 && nNewItemType != 0x15
                && nNewItemType != 0x16 && nNewItemType != 0x17 && nNewItemType != 0x18
                && nNewItemType != 0x19 && nNewItemType != 0x1A && nNewItemType != 0x1B
                && nNewItemType != 0x1D && nNewItemType != 0x1E) {
                bResult = FALSE;
                errorCode = 0x249F;
                bUsable = FALSE;
            }
            goto weaponOffhandCheck;
        }
        // Two-handed weapon (type 0xF): the off-hand slot must be free or hold a
        // compatible "set" placeholder.
        if (pEquip->m_items[nSlotNum + 1] != NULL) {
            SHORT nOffhandType = pEquip->m_items[nSlotNum + 1]->GetItemType();
            if (nOffhandType != 0x2F && nOffhandType != 0x35
                && nOffhandType != 0x31 && nOffhandType != 0x29) {
                errorCode = 0x705D;
                bResult = FALSE;
                goto releaseShareAndReturn;
            }
            if (nNewItemType != 0x12) {
                bResult = FALSE;
                errorCode = 0x24A4;
                bUsable = FALSE;
            }
        }
        bGenericEquip = FALSE;
        break;
    weaponOffhandCheck:
        if (nNewItemType != 0xF && nNewItemType != 0x1B && nNewItemType != 0x12) {
            CItem* pOffhand = pEquip->m_items[nSlotNum + 1];
            if (pOffhand != NULL
                && pOffhand->GetItemType() != 0x2F && pOffhand->GetItemType() != 0x35
                && pOffhand->GetItemType() != 0x31 && pOffhand->GetItemType() != 0x29
                && pNewItem != NULL) {
                pNewItem->Demand();
                INT nAbilities = pNewItem->GetAbilityCount();
                for (INT nAbility = 0; nAbility < nAbilities && bResult != FALSE; nAbility++) {
                    ITEM_ABILITY* pAbility = pNewItem->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        if ((pAbility->type & 0xFF) == 2) {
                            errorCode = 0x705D;
                            bResult = FALSE;
                        } else if ((pAbility->type & 0xFF) == 4) {
                            errorCode = 0x705E;
                            bResult = FALSE;
                        }
                    }
                    nAbilities = pNewItem->GetAbilityCount();
                }
                pNewItem->Release();
                if (bResult == FALSE) {
                    goto releaseShareAndReturn;
                }
            }
            if (pNewItem != NULL && (pNewItem->GetFlagsFile() & 2) != 0
                && pEquip->m_items[nSlotNum + 1] != NULL) {
                SHORT nOffhandType = pEquip->m_items[nSlotNum + 1]->GetItemType();
                if (nOffhandType == 0x2F || nOffhandType == 0x35
                    || nOffhandType == 0x31 || nOffhandType == 0x29) {
                    errorCode = 0x24A4;
                } else {
                    errorCode = 0x7056;
                }
                bResult = FALSE;
                goto releaseShareAndReturn;
            }
        }
        break;
    case 0x2C: // off-hand / shield, sets 0-3
    case 0x2E:
    case 0x30:
    case 0x32:
        if (pSprite->m_nWeaponSet != (nSlotNum - 0x2B) / 2) {
            bGenericEquip = FALSE;
        }
        if (pNewItem != NULL) {
            if (nNewItemType == 0x10 || nNewItemType == 0x11 || nNewItemType == 0x2C
                || nNewItemType == 0x12 || nNewItemType == 0x13 || nNewItemType == 0x14
                || nNewItemType == 0x45 || nNewItemType == 0x39 || nNewItemType == 0x15
                || nNewItemType == 0x16 || nNewItemType == 0x17 || nNewItemType == 0x18
                || nNewItemType == 0x19 || nNewItemType == 0x1A || nNewItemType == 0x1B
                || nNewItemType == 0x1D || nNewItemType == 0x1E) {
                if ((pNewItem->GetFlagsFile() & 2) != 0) {
                    errorCode = 0x66E6;
                    bResult = FALSE;
                    goto releaseShareAndReturn;
                }
                pNewItem->Demand();
                INT nAbilities = pNewItem->GetAbilityCount();
                for (INT nAbility = 0; nAbility < nAbilities && bResult != FALSE; nAbility++) {
                    ITEM_ABILITY* pAbility = pNewItem->GetAbility(nAbility);
                    if (pAbility != NULL
                        && ((pAbility->type & 0xFF) == 2 || (pAbility->type & 0xFF) == 4)) {
                        errorCode = 0x522;
                        bResult = FALSE;
                    }
                    nAbilities = pNewItem->GetAbilityCount();
                }
                pNewItem->Release();
            } else if (nNewItemType != 0x2F && nNewItemType != 0x31
                       && nNewItemType != 0x35 && nNewItemType != 0x29) {
                errorCode = 0x249F;
                bResult = FALSE;
                goto releaseShareAndReturn;
            }
        }
        if (pEquip->m_items[42] != NULL
            && (pNewItem != NULL && (pNewItem->GetFlagsFile() & 2) != 0)) {
            errorCode = 0x24A5;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
        {
            CItem* pMainhand = pEquip->m_items[nSlotNum - 1];
            if (pMainhand != NULL && pNewItem != NULL) {
                if ((pNewItem->GetFlagsFile() & 2) != 0 && pMainhand != pEquip->m_items[10]) {
                    errorCode = 0x24A5;
                    bResult = FALSE;
                    goto releaseShareAndReturn;
                }
                SHORT nMainType = pMainhand->GetItemType();
                if (nMainType != 0x2F && nMainType != 0x35 && nMainType != 0x31 && nMainType != 0x29) {
                    pMainhand->Demand();
                    INT nAbilities = pMainhand->GetAbilityCount();
                    for (INT nAbility = 0; nAbility < nAbilities && bResult != FALSE; nAbility++) {
                        ITEM_ABILITY* pAbility = pMainhand->GetAbility(nAbility);
                        if (pAbility != NULL) {
                            if ((pAbility->type & 0xFF) == 2) {
                                errorCode = 0x51E;
                                bResult = FALSE;
                            } else if ((pAbility->type & 0xFF) == 4) {
                                errorCode = 0x51F;
                                bResult = FALSE;
                            }
                        }
                        nAbilities = pMainhand->GetAbilityCount();
                    }
                    pMainhand->Release();
                }
            }
        }
        break;
    default: // backpack and unused slots
        bGenericEquip = FALSE;
        break;
    }

    if (bResult == FALSE) {
        goto releaseShareAndReturn;
    }

    // Item-instance and item-type equip immunities (e.g. cursed-item lists).
    {
        STRREF nImmunityError;
        CGameEffect* pImmunityEffect;
        if (pNewItem != NULL
            && pSprite->m_derivedStats.m_cImmunitiesItemEquip.OnList(pNewItem->GetResRef(), nImmunityError, pImmunityEffect)) {
            bResult = FALSE;
            errorCode = nImmunityError;
            if (pImmunityEffect != NULL) {
                delete pImmunityEffect;
            }
            goto releaseShareAndReturn;
        }
        if (pNewItem != NULL
            && pSprite->m_derivedStats.m_cImmunitiesItemTypeEquip.OnList(pNewItem->GetItemType(), nImmunityError, pImmunityEffect)) {
            bResult = FALSE;
            errorCode = nImmunityError;
            if (pImmunityEffect != NULL) {
                delete pImmunityEffect;
            }
            goto releaseShareAndReturn;
        }
    }

    // Slot 42 blocks equipping/changing weapons and ammo.
    if (pEquip->m_items[42] != NULL
        && (nSlotNum == 0x2B || nSlotNum == 0x2C || nSlotNum == 0x2D || nSlotNum == 0x2E
            || nSlotNum == 0x2F || nSlotNum == 0x30 || nSlotNum == 0x31 || nSlotNum == 0x32
            || nSlotNum == 0xB || nSlotNum == 0xC || nSlotNum == 0xD || nSlotNum == 0xE)) {
        errorCode = 0x279D;
        bResult = FALSE;
        goto releaseShareAndReturn;
    }

    if (pNewItem != NULL && IsItemExclusive(pSprite, nSlotNum, pNewItem, errorCode)) {
        bResult = FALSE;
        goto releaseShareAndReturn;
    }

    // BG2 NPC-bound items: those characters cannot remove their personal item.
    // Inert in IWD2 (no MINSC/ALORA/EDWIN sprites), kept from the shared codebase.
    if (pEquip->m_items[nSlotNum] != NULL) {
        CItem* pExisting = pEquip->m_items[nSlotNum];
        CString sScriptName(pSprite->GetScriptName());
        if (sScriptName.CompareNoCase("MINSC") == 0 && pExisting->GetResRef() == "MISC84") {
            errorCode = 0x27EA;
            bResult = FALSE;
            bUsable = FALSE;
        } else {
            if (sScriptName.CompareNoCase("ALORA") == 0 && pExisting->GetResRef() == "MISC88") {
                errorCode = 0x27EB;
                bResult = FALSE;
                bUsable = FALSE;
            }
            if (sScriptName.CompareNoCase("EDWIN") == 0 && pExisting->GetResRef() == "MISC89") {
                errorCode = 0x27EE;
                bResult = FALSE;
                bUsable = FALSE;
            }
        }
    }

    if (bResult == FALSE) {
        goto releaseShareAndReturn;
    }

    // The item currently in the slot must not be cursed/unremovable.
    if (pEquip->m_items[nSlotNum] != NULL) {
        DWORD nFlagsFile = pEquip->m_items[nSlotNum]->GetFlagsFile();
        if ((nFlagsFile & 0x10) != 0) {
            bResult = FALSE;
            pEquip->m_items[nSlotNum]->m_flags |= 1;
            errorCode = 0x3FB0;
            bUsable = FALSE;
        }
        if ((nFlagsFile & 4) == 0) {
            errorCode = 0x6461;
            pEquip->m_items[nSlotNum]->m_flags |= 1;
            bResult = FALSE;
            goto releaseShareAndReturn;
        }
    }

    if (bResult == FALSE) {
        goto releaseShareAndReturn;
    }

    // Quick-item slots: the item's first ability must be usable given the
    // item's identification state (the ability "type" high byte carries the
    // unidentified/identified usage restriction bits).
    if (bQuickItemSlot && pNewItem != NULL) {
        pNewItem->Demand();
        ITEM_ABILITY* pAbility = pNewItem->GetAbility(0);
        if (pAbility == NULL) {
            errorCode = 0x43A5;
            bResult = FALSE;
            bUsable = FALSE;
        } else if ((pNewItem->m_flags & 1) == 0) {
            if (((pAbility->type >> 8) & 1) != 0) {
                errorCode = 0x43A4;
                bResult = FALSE;
                bUsable = FALSE;
            }
        } else {
            if (((pAbility->type >> 8) & 2) != 0) {
                errorCode = 0x43A4;
                bResult = FALSE;
                bUsable = FALSE;
            }
        }
        pNewItem->Release();
    }

    if (bResult == FALSE) {
        goto releaseShareAndReturn;
    }

    // A cursed item being equipped becomes "known" and undroppable.
    if (pNewItem != NULL && (pNewItem->GetFlagsFile() & 0x10) != 0) {
        pNewItem->m_flags |= 1;
        errorCode = 0x3FB0;
        bGenericEquip = TRUE;
    }

    BYTE rcDeny;
    do {
        rcDeny = m_cObjectArray.GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rcDeny == CGameObjectArray::SHARED || rcDeny == CGameObjectArray::DENIED);

    if (rcDeny != CGameObjectArray::SUCCESS) {
        m_cObjectArray.ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        errorCode = 0x249D;
        return FALSE;
    }

    pEquip = pSprite->GetEquipment();

    // Multiplayer client forwards the request to the host (unreachable in practice:
    // the owner check above already rejects non-controlled sprites).
    if (!pSprite->InControl()
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && bFromServer == FALSE) {
        BOOLEAN bForwarded = g_pBaldurChitin->GetBaldurMessage()->SwapItemRequest(
            0x50, // 0x0084CF5D
            nCharacterId,
            nSlotNum,
            pItem,
            errorCode,
            wCount);

        m_cObjectArray.ReleaseDeny(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        m_cObjectArray.ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

        return bForwarded;
    }

    {
        CItem* pExisting = pEquip->m_items[nSlotNum];

        BOOL bSwap = (pExisting == NULL);
        if (pExisting != NULL) {
            WORD nMaxStack = pExisting->GetMaxStackable();
            if (pNewItem == NULL && nMaxStack > 1 && wCount < pExisting->GetUsageCount(0)) {
                // Pick up part of an existing stack.
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 13383
                UTIL_ASSERT(wCount != 0);

                pExisting->SetUsageCount(0, pExisting->GetUsageCount(0) - wCount);

                pItem = new CItem(pExisting->GetResRef(), 0, 0, 0, 0, 0);
                pNewItem = pItem;
                pNewItem->SetUsageCount(0, wCount);
                pNewItem->m_flags = ((pExisting->m_flags ^ pNewItem->m_flags) & 1) ^ pNewItem->m_flags;

                goto afterEquip;
            }

            if (pNewItem != NULL && pExisting->GetMaxStackable() >= 2
                && pNewItem->GetResRef() == pExisting->GetResRef()
                && ((pNewItem->m_flags ^ pExisting->m_flags) & 1) == 0) {
                // Merge the held stack into the matching one already in the slot.
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 13393
                UTIL_ASSERT(wCount != 0);

                WORD nRoom = pExisting->GetMaxStackable() - pExisting->GetUsageCount(0);
                WORD nTransfer = (pNewItem->GetUsageCount(0) < nRoom) ? pNewItem->GetUsageCount(0) : nRoom;
                if (wCount < nTransfer) {
                    nTransfer = wCount;
                }

                pExisting->SetUsageCount(0, pExisting->GetUsageCount(0) + nTransfer);

                if (pNewItem->GetUsageCount(0) == nTransfer) {
                    // NOTE: Uninline.
                    AddDisposableItem(pNewItem);
                    pItem = NULL;
                    pNewItem = NULL;
                } else {
                    pNewItem->SetUsageCount(0, pNewItem->GetUsageCount(0) - nTransfer);
                }

                goto afterEquip;
            }

            bSwap = TRUE;
        }

        if (bSwap) {
            if (pEquip->m_selectedWeapon == nSlotNum) {
                pEquip->m_selectedWeapon = CGameSpriteEquipment::SLOT_FIST;
                pEquip->m_selectedWeaponAbility = 0;

                if (!bGenericEquip && pEquip->m_items[nSlotNum] != NULL) {
                    if (nSlotNum > 0x2A && nSlotNum < 0x33) {
                        BYTE nButton = static_cast<BYTE>(nSlotNum - 0x2B);
                        pSprite->SetQuickWeapon(nButton, CButtonData());

                        // __FILE__: .\Include\ObjCreature.h
                        // __LINE__: 2031
                        UTIL_ASSERT(nButton < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22);
                        pSprite->field_3D3A[nButton] = 0;

                        bResult = bUsable;
                    }

                    pExisting->Demand();
                    pExisting->Unequip(pSprite, nSlotNum, TRUE, FALSE);
                    ITEM_ABILITY* pAbility = pExisting->GetAbility(pEquip->m_selectedWeaponAbility);
                    SHORT nLauncherSlot;
                    if (pAbility != NULL && pSprite->GetLauncher(pAbility, nLauncherSlot) != NULL) {
                        pExisting->Unequip(pSprite, nLauncherSlot, TRUE, FALSE);
                    }
                    pExisting->Release();
                }
            } else {
                CItem* pSelectedWeapon = pEquip->m_items[pEquip->m_selectedWeapon];
                pSelectedWeapon->Demand();
                ITEM_ABILITY* pAbility = pSelectedWeapon->GetAbility(pEquip->m_selectedWeaponAbility);
                SHORT nLauncherSlot;
                CItem* pLauncher = (pAbility != NULL) ? pSprite->GetLauncher(pAbility, nLauncherSlot) : NULL;
                if (pLauncher != NULL && pLauncher == pEquip->m_items[nSlotNum]) {
                    pEquip->m_items[nSlotNum]->Unequip(pSprite, nSlotNum, TRUE, FALSE);
                    pEquip->m_items[nSlotNum] = NULL;
                    pSelectedWeapon->Unequip(pSprite, pEquip->m_selectedWeapon, TRUE, FALSE);
                    pEquip->m_selectedWeapon = CGameSpriteEquipment::SLOT_FIST;
                    pEquip->m_selectedWeaponAbility = 0;
                }
                pSelectedWeapon->Release();
            }

            if (bGenericEquip) {
                if (pEquip->m_items[nSlotNum] != NULL) {
                    pEquip->m_items[nSlotNum]->Unequip(pSprite, nSlotNum, TRUE, FALSE);
                    pEquip->m_items[nSlotNum] = NULL;
                }
                if (pNewItem != NULL) {
                    pNewItem->Equip(pSprite, nSlotNum, FALSE);
                }
            }

            pEquip->m_items[nSlotNum] = pNewItem;
            pItem = pExisting;

            if (nSlotNum == 0x2B || nSlotNum == 0x2C || nSlotNum == 0x2D || nSlotNum == 0x2E
                || nSlotNum == 0x2F || nSlotNum == 0x30 || nSlotNum == 0x31 || nSlotNum == 0x32) {
                if (bGenericEquip != TRUE
                    && pSprite->m_nWeaponSet != (nSlotNum - 0x2B) / 2) {
                    RefreshWeaponSetButtons(pSprite);
                    goto afterEquip;
                }

                BOOL bOffhand = (nSlotNum == 0x2C || nSlotNum == 0x2E
                    || nSlotNum == 0x30 || nSlotNum == 0x32);
                if (pExisting == NULL || pSprite->CheckWeaponUsability(bOffhand)) {
                    bResult = TRUE;
                    pSprite->SetWeaponSet(static_cast<BYTE>((nSlotNum - 0x2B) / 2));
                } else {
                    bResult = FALSE;
                }
            }
        }
    }

afterEquip:
    g_pBaldurChitin->GetActiveEngine()->UpdatePersonalItemStatus(pSprite->GetId());

    if (pSprite->InControl()
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        CMessage* message = new CMessageSpriteEquipment(pSprite,
            nCharacterId,
            nCharacterId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    // Recompute the base number of attacks after the equipment change. (0x723830)
    pSprite->GetAIType();
    {
        int nBaseAttack;
        int nAttacks;
        int nFeat;
        g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetBaseCombatValues(pSprite,
            nBaseAttack,
            nAttacks,
            nFeat,
            FALSE);
        pSprite->m_baseStats.m_numberOfAttacksBase = static_cast<BYTE>(nAttacks);
        if (pSprite->m_baseStats.m_numberOfAttacksBase > 5) {
            pSprite->m_baseStats.m_numberOfAttacksBase = 5;
        }
    }

    m_cObjectArray.ReleaseDeny(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

releaseShareAndReturn:
    m_cObjectArray.ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

    return bResult;
}

// 0x5BAD70
BOOL CInfGame::SwapItemPersonalInventory(SHORT nPortraitNum, CItem*& pNewItem, SHORT nSlotNum, STRREF& errorCode, WORD wCount, BOOLEAN bFromServer, BOOL bAutoStacking)
{
    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortraitNum);

    BOOL bResult = TRUE;

    errorCode = -1;

    if (((nSlotNum < 18) || (CScreenInventory::PERSONAL_INVENTORY_SIZE + 18 <= nSlotNum))
        && nSlotNum != 0x7FFF) {
        errorCode = 0x249E;
        return FALSE;
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        errorCode = 0x249D;
        return FALSE;
    }

    // Not controlling this sprite in a multiplayer session: forward the request
    // to the server and return its result.
    if (!pSprite->InControl()
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && bFromServer == FALSE) {
        BOOLEAN bForwarded = g_pBaldurChitin->GetBaldurMessage()->SwapItemRequest(
            0x67, // 0x0084CF5C
            nCharacterId,
            nSlotNum,
            pNewItem,
            errorCode,
            wCount);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        return bForwarded;
    }

    // Dropping an item onto a bag/container item already in the slot deposits it
    // into that item's store.
    if (nSlotNum != 0x7FFF) {
        CItem* pSlotItem = pSprite->GetEquipment()->m_items[nSlotNum];
        if (pNewItem != NULL
            && pSlotItem != NULL
            && pSlotItem->GetItemType() == 0x3A) {
            WORD nDeposited = SwapItemBag(pSlotItem->GetResRef(), pNewItem, errorCode);
            if (nDeposited != 0) {
                if (pNewItem->GetMaxStackable() < 2 || pNewItem->GetUsageCount(0) == nDeposited) {
                    if (pNewItem != NULL) {
                        // NOTE: Uninline.
                        AddDisposableItem(pNewItem);
                    }

                    pNewItem = NULL;
                } else {
                    pNewItem->SetUsageCount(0, pNewItem->GetUsageCount(0) - nDeposited);
                }

                g_pBaldurChitin->GetActiveEngine()->UpdatePersonalItemStatus(pSprite->GetId());

                if (pSprite->InControl()
                    && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
                    CMessage* message = new CMessageSpriteEquipment(pSprite,
                        nCharacterId,
                        nCharacterId);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                return TRUE;
            }

            bResult = FALSE;
            goto placeRemainder;
        }
    }

    // Auto-stacking: top off existing matching stacks before placing the remainder.
    if (bAutoStacking
        && pNewItem != NULL
        && pNewItem->GetMaxStackable() > 1) {
        for (SHORT nSlot = 18; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18; nSlot++) {
            CItem* pSlotItem = pSprite->GetEquipment()->m_items[nSlot];
            if (pSlotItem != NULL
                && pSlotItem->GetResRef() == pNewItem->GetResRef()
                && ((pNewItem->m_flags ^ pSlotItem->m_flags) & 1) == 0
                && pSlotItem->GetUsageCount(0) < pSlotItem->GetMaxStackable()) {
                if (pSlotItem->GetUsageCount(0) + pNewItem->GetUsageCount(0) <= pSlotItem->GetMaxStackable()) {
                    nSlotNum = nSlot;
                    break;
                }

                pNewItem->SetUsageCount(0,
                    pNewItem->GetUsageCount(0) - (pSlotItem->GetMaxStackable() - pSlotItem->GetUsageCount(0)));
                pSlotItem->SetUsageCount(0, pSlotItem->GetMaxStackable());
            }
        }
    }

placeRemainder:
    // Resolve an explicit empty slot when the caller asked for "first free" (0x7FFF).
    if (nSlotNum == 0x7FFF) {
        for (SHORT nSlot = 18; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18; nSlot++) {
            if (pSprite->GetEquipment()->m_items[nSlot] == NULL) {
                nSlotNum = nSlot;
                break;
            }
        }

        if (nSlotNum == 0x7FFF) {
            errorCode = 0x464F;
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return FALSE;
        }
    }

    CItem* pExisting = pSprite->GetEquipment()->m_items[nSlotNum];

    if (bResult == FALSE) {
        goto releaseAndReturn;
    }

    if (pExisting != NULL) {
        if ((pExisting->GetFlagsFile() & 4) == 0 || (pExisting->m_flags & 0x10) != 0) {
            bResult = FALSE;
            errorCode = 0x6461;
            goto releaseAndReturn;
        }

        if (pNewItem == NULL
            && pExisting->GetMaxStackable() > 1
            && wCount < pExisting->GetUsageCount(0)) {
            // Pick up part of an existing stack.
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 14884
            UTIL_ASSERT(wCount != 0);

            pExisting->SetUsageCount(0, pExisting->GetUsageCount(0) - wCount);

            pNewItem = new CItem(pExisting->GetResRef(), 0, 0, 0, 0, 0);
            pNewItem->SetUsageCount(0, wCount);
            pNewItem->m_flags = ((pExisting->m_flags ^ pNewItem->m_flags) & 1) ^ pNewItem->m_flags;

            goto notifyChange;
        }

        if (pNewItem != NULL
            && pExisting->GetMaxStackable() >= 2
            && pNewItem->GetResRef() == pExisting->GetResRef()
            && ((pNewItem->m_flags ^ pExisting->m_flags) & 1) == 0) {
            // Merge the held stack into the matching one already in the slot.
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 14894
            UTIL_ASSERT(wCount != 0);

            WORD nRoom = pExisting->GetMaxStackable() - pExisting->GetUsageCount(0);
            WORD nTransfer = (pNewItem->GetUsageCount(0) < nRoom) ? pNewItem->GetUsageCount(0) : nRoom;
            if (wCount < nTransfer) {
                nTransfer = wCount;
            }

            pExisting->SetUsageCount(0, pExisting->GetUsageCount(0) + nTransfer);

            if (pNewItem->GetUsageCount(0) == nTransfer) {
                // NOTE: Uninline.
                AddDisposableItem(pNewItem);
                pNewItem = NULL;
            } else {
                pNewItem->SetUsageCount(0, pNewItem->GetUsageCount(0) - nTransfer);
            }

            goto notifyChange;
        }
    }

    // Swap the held item with the slot's contents (the old item, possibly NULL,
    // is returned through `pNewItem`).
    pSprite->GetEquipment()->m_items[nSlotNum] = pNewItem;
    pNewItem = pExisting;

notifyChange:
    g_pBaldurChitin->GetActiveEngine()->UpdatePersonalItemStatus(pSprite->GetId());

    if (pSprite->InControl()
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        CMessage* message = new CMessageSpriteEquipment(pSprite,
            nCharacterId,
            nCharacterId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

releaseAndReturn:
    pSprite->field_562C = 1;
    pSprite->ProcessEffectList();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bResult;
}

// 0x5BE840
SHORT CInfGame::FindFirstFreeInventorySlot(SHORT nPortrait)
{
    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortrait);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        for (SHORT nSlot = 0; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE; nSlot++) {
            if (pSprite->GetEquipment()->m_items[18 + nSlot] == NULL) {
                m_cObjectArray.ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                return nSlot + 18;
            }
        }

        m_cObjectArray.ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    return -1;
}

// 0x5C7B10
WORD CInfGame::SwapItemBag(const CResRef& bagResRef, CItem* pItem, STRREF& errorCode)
{
    errorCode = -1;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        // TODO: Incomplete. Multiplayer store-sync deposit path: demands the
        // server-side bag store, posts the CMessageStore* updates, then releases
        // it. Needs those store message classes recovered. The host path below
        // is complete.
        return 0;
    }

    WORD nDeposited = 0;

    CStore store(bagResRef);

    // Store type 4 is a bag/container.
    if (store.GetType() == 4 && store.IsValidSellType(pItem)) {
        WORD nCapacity = store.m_header.m_nCapacity;

        if (nCapacity == 0 || nCapacity == store.GetNumItems()) {
            errorCode = 0x6139;
        } else {
            nDeposited = 1;
            if (pItem->GetMaxStackable() > 1) {
                nDeposited = pItem->GetUsageCount(0);
            }

            if (store.GetNumItems() + nDeposited > nCapacity) {
                nDeposited = static_cast<WORD>(nCapacity - store.GetNumItems());
            }

            pItem->SetUsageCount(0, nDeposited);
            store.AddItemExt(*pItem, 1);
        }
    } else {
        errorCode = 0x66AD;
    }

    if (errorCode == -1) {
        store.Marshal(m_sTempDir);
    }

    return nDeposited;
}

// 0x5BB600
void CInfGame::InventoryInfoPersonal(SHORT nPortraitNum, SHORT nSlotNum, CItem*& pItem, STRREF& description, CResRef& cResIcon, CResRef& cResItem, WORD& wCount, BOOL a8)
{
    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortraitNum);

    description = -1;
    cResIcon = "";
    cResItem = "";
    wCount = 0;

    if (nSlotNum < CGameSpriteEquipment::NUM_SLOT) {
        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (nSlotNum == 43
                || nSlotNum == 45
                || nSlotNum == 47
                || nSlotNum == 49) {
                if (pSprite->GetEquipment()->m_items[nSlotNum] == NULL && a8 == TRUE) {
                    if (pSprite->GetEquipment()->m_items[nSlotNum + 1] != NULL) {
                        // FIXME: Calls `GetItemType` four times.
                        if (pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 47
                            || pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 53
                            || pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 49
                            || pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 41) {
                            nSlotNum = 10;
                        }
                    } else {
                        if (pSprite->m_nWeaponSet == (nSlotNum - 43) / 2) {
                            nSlotNum = 10;
                        }
                    }
                }
            }

            pItem = pSprite->GetEquipment()->m_items[nSlotNum];
            if (pItem != NULL) {
                description = pItem->GetGenericName();
                cResIcon = pItem->GetItemIcon();
                cResItem = pItem->GetResRef();

                if (pItem->GetMaxStackable() > 1) {
                    wCount = pItem->GetUsageCount(0);
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5BB800
BOOL CInfGame::Is3DSound(int nSoundChannel)
{
    return m_ruleTables.Is3DSound(nSoundChannel);
}

// 0x5BB810
float CInfGame::GetSoundReverbMix(int nSoundChannel, int nReverb)
{
    return m_ruleTables.GetSoundReverbMix(nSoundChannel, nReverb);
}

// 0x5BB830
CResRef CInfGame::GetAnimationBam(SHORT nPortrait, BYTE range)
{
    CString sResRef;

    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortrait);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return CResRef("");
    }

    pSprite->GetAnimation()->GetAnimationResRef(sResRef, range);

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return CResRef(sResRef);
}

// 0x5BB960
BYTE CInfGame::GetFrameRate()
{
    return static_cast<BYTE>(g_pBaldurChitin->GetActiveEngine()->pVidMode->field_94);
}

// 0x5BE900
void CInfGame::SynchronousUpdate()
{

    if (m_bSaveScreen == 0) {
        DestroyDisposableItems();
        return;
    }

    CString sDefaultDir = GetDirSaveRoot() + "default\\";

    if (field_50D8 != FALSE && field_50DC == 0) {
        CString sDirSave = GetDirSave();

        CString sSource = sDefaultDir + "ICEWIND2.BMP";
        CString sTarget = sDirSave + "ICEWIND2.BMP";
        BOOL bCopied = CopyFileA(sSource, sTarget, FALSE);
        static_cast<void>(bCopied);
        DeleteFileA(sSource);

        for (SHORT nPortrait = 0; nPortrait < CINFGAME_MAXCHARACTERS; nPortrait++) {
            sSource.Format("%sdefault\\PORTRT%d.BMP",
                static_cast<LPCSTR>(GetDirSaveRoot()),
                nPortrait);
            sTarget.Format("%sPORTRT%d.BMP",
                static_cast<LPCSTR>(sDirSave),
                nPortrait);
            bCopied = CopyFileA(sSource, sTarget, FALSE);
            static_cast<void>(bCopied);
            DeleteFileA(sSource);
        }

        m_bSaveScreen = 0;
        field_50D8 = FALSE;
        DestroyDisposableItems();
        return;
    }

    CVidMode* pVidMode = g_pChitin->GetCurrentVideoMode();
    CGameArea* pArea = GetVisibleArea();
    if (pVidMode == NULL || g_pBaldurChitin->m_pEngineWorld == NULL || pArea == NULL) {
        DestroyDisposableItems();
        return;
    }

    BeginListManipulation(pArea);
    EnterCriticalSection(&pArea->field_1FC);

    Sleep(100);

    EnterCriticalSection(&g_pBaldurChitin->m_pEngineWorld->field_106);

    const BYTE nPrevBrightnessCorrection = pVidMode->m_nBrightnessCorrection;
    const BYTE nPrevGammaCorrection = pVidMode->m_nGammaCorrection;
    pVidMode->m_nBrightnessCorrection = 0;
    pVidMode->m_nGammaCorrection = 0;

    m_cVRamPool.InvalidateAll();
    pVidMode->LoadFogOWarSurfaces(FOGOWAR_RESREF);

    m_bSaveScreen = 0;

    const BOOL bPrevDither = CCacheStatus::dword_8D0BA8;
    CCacheStatus::dword_8D0BA8 = FALSE;
    g_pBaldurChitin->m_pEngineWorld->SaveScreen();
    CCacheStatus::dword_8D0BA8 = bPrevDither;

    CString sWriteDir;
    if (field_50DC == 0 && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineWorld) {
        sWriteDir = GetDirSave();
    } else {
        if (g_pBaldurChitin->GetActiveEngine() == NULL) {
            sWriteDir = sDefaultDir;
        } else if (field_50DC == 0) {
            sWriteDir = sDefaultDir;
            field_50D8 = TRUE;
        } else {
            sWriteDir = GetDirSave();
            field_50DC = 0;
            field_50D8 = FALSE;
        }
    }

    g_pChitin->cDimm.AddToDirectoryList(sWriteDir, TRUE);

    LPBYTE pScreenData = NULL;
    LONG nScreenData = 0;
    if (pVidMode->PrintSurfaceToBmp(pScreenData,
            CVIDINF_SURFACE_BACK,
            CInfinity::stru_8E79A8,
            nScreenData,
            5)) {
        WriteBmpResource(sWriteDir, CResRef("ICEWIND2"), pScreenData, nScreenData);
    }

    if (pScreenData != NULL) {
        delete[] pScreenData;
    }

    SHORT nPortraitFile = 0;
    for (SHORT nSlot = 0; nSlot < CINFGAME_MAXCHARACTERS; nSlot++) {
        LONG nCharacterId = m_characters[nSlot];
        if (nCharacterId == CGameObjectArray::INVALID_INDEX) {
            continue;
        }

        CGameSprite* pSprite = NULL;
        BYTE rc;
        do {
            rc = m_cObjectArray.GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS || pSprite == NULL) {
            continue;
        }

        CVidBitmap vbPortrait;
        vbPortrait.SetResRef(CResRef(pSprite->GetBaseStats()->m_portraitSmall), TRUE, TRUE);

        if (vbPortrait.GetBitCount(FALSE) != 24 && vbPortrait.GetBitCount(FALSE) != 8) {
            vbPortrait.SetResRef(CResRef(SILHOUETTE_PORTRAIT_SM), TRUE, TRUE);
        }

        m_cObjectArray.ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        CSize portraitSize;
        vbPortrait.GetImageDimensions(portraitSize, FALSE);

        CRect rPortrait(0, 0, portraitSize.cx, portraitSize.cy);
        vbPortrait.RenderDirect(CVIDINF_SURFACE_2, 0, 0, rPortrait, 0, FALSE);

        LPBYTE pPortraitData = NULL;
        LONG nPortraitData = 0;
        if (pVidMode->PrintSurfaceToBmp(pPortraitData,
                CVIDINF_SURFACE_2,
                rPortrait,
                nPortraitData,
                2)) {
            CString sPortraitResRef;
            sPortraitResRef.Format("PORTRT%d", nPortraitFile);
            WriteBmpResource(sWriteDir, CResRef(sPortraitResRef), pPortraitData, nPortraitData);
            nPortraitFile++;
        }

        if (pPortraitData != NULL) {
            delete[] pPortraitData;
        }
    }

    g_pChitin->cDimm.RemoveFromDirectoryList(GetDirSave(), TRUE);

    pVidMode->m_nBrightnessCorrection = nPrevBrightnessCorrection;
    pVidMode->m_nGammaCorrection = nPrevGammaCorrection;
    m_cVRamPool.InvalidateAll();
    pVidMode->LoadFogOWarSurfaces(FOGOWAR_RESREF);

    LeaveCriticalSection(&g_pBaldurChitin->m_pEngineWorld->field_106);

    LeaveCriticalSection(&pArea->field_1FC);
    EndListManipulation(pArea);

    DestroyDisposableItems();
}

// 0x5C0520
CStringList* CInfGame::GetSaveGames()
{
    CStringList* pList = new CStringList();

    CString sFileName;

    WIN32_FIND_DATAA ffd;
    HANDLE hFindFile = FindFirstFileA(GetDirSaveRoot() + "*.*", &ffd);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = ffd.cFileName;
            if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                && sFileName != "."
                && sFileName != ".."
                && SaveGameExists(sFileName)) {
                POSITION pos = pList->GetHeadPosition();
                while (pos != NULL) {
                    if (sFileName < pList->GetAt(pos)) {
                        break;
                    }
                    pList->GetNext(pos);
                }

                if (pos != NULL) {
                    pList->InsertBefore(pos, sFileName);
                } else {
                    pList->AddTail(sFileName);
                }
            }
        } while (FindNextFileA(hFindFile, &ffd));

        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C0730
BOOL CInfGame::DeleteSaveGame(const CString& sFileName)
{
    CString sDirectoryName = GetDirSaveRoot() + sFileName;

    CFileStatus cFileStatus;
    if (!CFile::GetStatus(sDirectoryName, cFileStatus)) {
        return FALSE;
    }

    sDirectoryName += "\\";
    if (!g_pChitin->cDimm.DirectoryRemoveFiles(sDirectoryName)) {
        return FALSE;
    }

    WIN32_FIND_DATAA ffd;
    HANDLE hFindFile = FindFirstFileA(sDirectoryName + "*.*", &ffd);
    if (hFindFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 || ffd.cFileName[0] != '.') {
            FindClose(hFindFile);
            return FALSE;
        }
    } while (FindNextFileA(hFindFile, &ffd));

    FindClose(hFindFile);

    return RemoveDirectoryA(sDirectoryName);
}

// 0x5C0890
BOOL CInfGame::SaveGameExists(const CString& sSaveGame)
{
    CString sFileName = GetDirSaveRoot() + sSaveGame + "\\ICEWIND2.GAM";

    HANDLE hFile = CreateFileA(sFileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // NOTE: Original code is slightly different (conditions inverted).
    BOOL bSuccess = FALSE;
    if (hFile != INVALID_HANDLE_VALUE) {
        unsigned char signature[8];
        DWORD dwBytesRead;
        ReadFile(hFile, signature, sizeof(signature), &dwBytesRead, NULL);
        if (dwBytesRead == sizeof(signature)
            && memcmp(signature, "GAMEV2.2", sizeof(signature)) == 0) {
            bSuccess = TRUE;
        }

        CloseHandle(hFile);
    }

    return bSuccess;
}

// 0x5C0EA0
void CInfGame::ApplyVolumeSliders(BOOLEAN a2)
{
    g_pBaldurChitin->cSoundMixer.SetMusicVolume(m_cOptions.m_nVolumeMusic);

    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(0, m_cOptions.m_nVolumeVoices);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(1, m_cOptions.m_nVolumeAmbients);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(2, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(3, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(4, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(5, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(2, m_cOptions.m_nVolumeVoices);

    for (int k = 7; k < 13; k++) {
        g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(k, m_cOptions.m_nVolumeVoices);
    }

    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(13, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(14, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(15, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(20, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(18, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(19, m_cOptions.m_nVolumeSFX);

    if (a2 == TRUE) {
        g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(16, m_cOptions.m_nVolumeAmbients);
        g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(17, m_cOptions.m_nVolumeAmbients);
    }

    g_pBaldurChitin->cSoundMixer.UpdateSoundList();
}

// 0x5BB980
void CInfGame::UseMagicOnGround(CPoint pt)
{
    CAIAction action;

    LONG nCharacterId = reinterpret_cast<LONG>(m_group.m_memberList.GetHead());

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastClick(CPoint(-1, -1));

        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        BOOL addAction = FALSE;
        switch (pSprite->m_currentUseButton.m_abilityId.m_itemType) {
        case 2:
            action = CAIAction(CAIAction::USEITEMPOINT,
                pt,
                pSprite->m_currentUseButton.m_abilityId.m_itemNum,
                pSprite->m_currentUseButton.m_abilityId.m_abilityNum);
            addAction = TRUE;
            break;
        case 3:
            if (1) {
                pSprite->m_castCounter = -1;

                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELLPOINTNODEC,
                    string,
                    pt,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.m_nTooltip << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.m_bCanUse;
                addAction = TRUE;
            }
            break;
        case 4:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::FORCESPELLPOINT,
                    string,
                    pt,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.m_nTooltip << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.m_bCanUse;
                addAction = TRUE;
            }
            break;
        case 5:
            pSprite->FireSpellPoint(pSprite->m_currentUseButton.m_abilityId.m_res, pt);
            break;
        default:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELLPOINT,
                    string,
                    pt,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.m_nTooltip << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.m_bCanUse;
                addAction = TRUE;
            }
            break;
        }

        if (addAction) {
            pSprite->ClearActions(FALSE);
            pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
            pSprite->m_triggerId = -1;
            pSprite->AddAction(action);
            pSprite->m_interrupt = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5BC450
void CInfGame::UseMagicOnObject(LONG target)
{
    CAIAction action;
    CAIObjectType targetType(0, 0, 0, 0, 0, 0, 0, 0, target, 0, 0);

    LONG nCharacterId = reinterpret_cast<LONG>(m_group.m_memberList.GetHead());

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastClick(CPoint(-1, -1));

        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        BOOL addAction = FALSE;
        switch (pSprite->m_currentUseButton.m_abilityId.m_itemType) {
        case 2:
            action = CAIAction(CAIAction::USEITEM,
                targetType,
                pSprite->m_currentUseButton.m_abilityId.m_itemNum,
                pSprite->m_currentUseButton.m_abilityId.m_abilityNum,
                0);
            addAction = TRUE;
            break;
        case 3:
            if (1) {
                pSprite->m_castCounter = -1;

                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELLNODEC,
                    targetType,
                    string,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.m_nTooltip << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.m_bCanUse;
                addAction = TRUE;
            }
            break;
        case 4:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::FORCESPELL,
                    targetType,
                    string,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.m_nTooltip << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.m_bCanUse;
                addAction = TRUE;
            }
            break;
        case 5:
            if (1) {
                CGameObject* pTarget;

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
                        CGameObjectArray::THREAD_ASYNCH,
                        &pTarget,
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    pSprite->FireSpell(pSprite->m_currentUseButton.m_abilityId.m_res, pTarget);

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(target,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            break;
        default:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELL,
                    targetType,
                    string,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.m_nTooltip << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.m_bCanUse;
                addAction = TRUE;
            }
            break;
        }

        if (addAction) {
            pSprite->ClearActions(FALSE);
            pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
            pSprite->m_triggerId = -1;
            pSprite->AddAction(action);
            pSprite->m_interrupt = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5BD070
BOOL CInfGame::AddCharacterToParty(LONG id, SHORT nPortraitNumber)
{
    // NOTE: Uninline.
    if (GetCharacterPortraitNum(id) != -1) {
        return FALSE;
    }

    if (m_nCharacters >= CINFGAME_MAXCHARACTERS) {
        return FALSE;
    }

    if (m_allies.Find(reinterpret_cast<int*>(id)) != NULL) {
        // NOTE: Uninline.
        RemoveCharacterFromAllies(id);
    }

    if (m_familiars.Find(reinterpret_cast<int*>(id)) != NULL) {
        // NOTE: Uninline.
        RemoveCharacterFromFamiliars(id);
    }

    POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        if (reinterpret_cast<int>(m_lstGlobalCreatures.GetNext(pos)) == id) {
            m_lstGlobalCreatures.RemoveAt(posOld);
            break;
        }
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (nPortraitNumber == -1 || nPortraitNumber >= CINFGAME_MAXCHARACTERS) {
        nPortraitNumber = m_nCharacters;
    }

    m_characterPortraits[nPortraitNumber] = id;

    UpdatePortraitToolTip(nPortraitNumber, pSprite->GetNameRef());
    pSprite->SetFootstepChannel();

    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) != 0) {
        pSprite->m_canBeSeen = 4 * (CGameObject::VISIBLE_DELAY + 1);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    EnablePortrait(static_cast<BYTE>(nPortraitNumber), TRUE);

    m_nCharacters++;

    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;
    pWorld->GetManager()->GetPanel(pWorld->GetPanel_22_0())->InvalidateRect(NULL);

    return TRUE;
}

// 0x5BD270
BOOL CInfGame::RemoveCharacterFromParty(LONG id, BOOL& overflow)
{
    CGameSprite* pSprite;
    BYTE rc;
    SHORT cnt;

    // NOTE: Uninline.
    SHORT nPortrait = GetCharacterPortraitNum(id);

    overflow = FALSE;

    if (nPortrait == -1) {
        for (int index = 0; index < m_characterOverflow.GetCount(); index++) {
            if (reinterpret_cast<int>(m_characterOverflow.GetAt(index)) == id) {
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(id,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    overflow = TRUE;
                    m_characterOverflow.RemoveAt(index);

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(id,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    return TRUE;
                }
            }
        }
        return FALSE;
    }

    for (cnt = 0; cnt < CINFGAME_MAXCHARACTERS; cnt++) {
        if (m_characters[cnt] == id) {
            m_characters[cnt] = -1;
        }
    }

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    m_group.Remove(pSprite);
    pSprite->SetFootstepChannel();
    pSprite->m_cGameStats.RecordLeaveParty();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    for (cnt = 0; cnt < m_nCharacters - 1; cnt++) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_characterPortraits[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            UpdatePortraitToolTip(cnt, pSprite->GetNameRef());
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_characterPortraits[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    m_lstGlobalCreatures.AddHead(reinterpret_cast<int*>(id));

    m_nCharacters--;
    m_characterPortraits[m_nCharacters] = -1;

    EnablePortrait(static_cast<BYTE>(m_nCharacters), FALSE);

    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;
    pWorld->GetManager()->GetPanel(pWorld->GetPanel_22_0())->InvalidateRect(NULL);

    return TRUE;
}

// 0x5BD4E0
BOOL CInfGame::SetCharacterSlot(INT nCharacterSlot, LONG nCharacterId)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 15670
    UTIL_ASSERT(0 <= nCharacterSlot && nCharacterSlot < CINFGAME_MAXCHARACTERS);

    if (m_characters[nCharacterSlot] != -1) {
        return FALSE;
    }

    m_characters[nCharacterSlot] = nCharacterId;

    return TRUE;
}

// 0x5BD530
BOOL CInfGame::ClearCharacterSlot(INT nCharacterSlot)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 15717
    UTIL_ASSERT(0 <= nCharacterSlot && nCharacterSlot < CINFGAME_MAXCHARACTERS);

    LONG id = m_characters[nCharacterSlot];
    if (id == -1) {
        return FALSE;
    }

    m_characters[nCharacterSlot] = -1;

    BOOL overflow;
    RemoveCharacterFromParty(id, overflow);

    POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        if (reinterpret_cast<LONG>(m_lstGlobalCreatures.GetNext(pos)) == id) {
            m_lstGlobalCreatures.RemoveAt(posOld);
            break;
        }
    }

    CGameSprite* pSprite;
    if (m_cObjectArray.GetDeny(id, CGameObjectArray::THREAD_ASYNCH, reinterpret_cast<CGameObject**>(&pSprite), INFINITE) != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (pSprite->GetArea() != NULL) {
        pSprite->GetArea()->m_visibility.RemoveCharacter(pSprite->GetPos(),
            pSprite->GetId(),
            pSprite->GetTerrainTable());

        if (pSprite->GetArea()->m_nCharacters != -1) {
            pSprite->GetArea()->m_nCharacters--;
        }

        pSprite->RemoveFromArea();
    }

    m_cObjectArray.ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);

    return TRUE;
}

// 0x5BD650
LONG CInfGame::GetCharacterSlot(INT nCharacterSlot)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 15787
    UTIL_ASSERT(0 <= nCharacterSlot && nCharacterSlot < CINFGAME_MAXCHARACTERS);

    return m_characters[nCharacterSlot];
}

// 0x5BD690
INT CInfGame::GetCharacterSlotFromId(LONG nCharacterId)
{
    for (int index = 0; index < CINFGAME_MAXCHARACTERS; index++) {
        if (m_characters[index] == nCharacterId) {
            return index;
        }
    }

    return -1;
}

// 0x5BD6B0
void CInfGame::UpdateCharacterSlots()
{
    for (SHORT nPortrait = 0; nPortrait < CINFGAME_MAXCHARACTERS; nPortrait++) {
        if (m_characterPortraits[nPortrait] != -1) {
            EnablePortrait(static_cast<BYTE>(nPortrait), FALSE);
            m_characterPortraits[nPortrait] = -1;
        }
    }
    m_nCharacters = 0;
    for (SHORT nSlot = 0; nSlot < CINFGAME_MAXCHARACTERS; nSlot++) {
        LONG nCharacterId = m_characters[nSlot];
        if (nCharacterId != -1) {
            AddCharacterToParty(nCharacterId, -1);
        }
    }
}

// 0x5BD9D0
BOOL CInfGame::AddCharacterToAllies(LONG nCharacterId)
{
    // NOTE: Uninline.
    if (GetCharacterPortraitNum(nCharacterId) != -1) {
        return FALSE;
    }

    if (m_allies.Find(reinterpret_cast<int*>(nCharacterId)) != NULL) {
        return FALSE;
    }

    if (m_familiars.Find(reinterpret_cast<int*>(nCharacterId)) != NULL) {
        return FALSE;
    }

    m_allies.AddTail(reinterpret_cast<int*>(nCharacterId));

    return TRUE;
}

// 0x5BDA50
BOOL CInfGame::RemoveCharacterFromAllies(LONG nCharacterId)
{
    POSITION pos = m_allies.Find(reinterpret_cast<int*>(nCharacterId));
    if (pos == NULL) {
        return FALSE;
    }

    m_allies.RemoveAt(pos);

    // FIXME: What for?
    if (m_allies.GetCount() == 0) {
        m_allies.RemoveAll();
    }

    return TRUE;
}

// 0x5BDAA0
BOOL CInfGame::AddCharacterToFamiliars(LONG nCharacterId)
{
    // NOTE: Uninline.
    if (GetCharacterPortraitNum(nCharacterId) != -1) {
        return FALSE;
    }

    if (m_allies.Find(reinterpret_cast<int*>(nCharacterId)) != NULL) {
        return FALSE;
    }

    if (m_familiars.Find(reinterpret_cast<int*>(nCharacterId)) != NULL) {
        return FALSE;
    }

    m_familiars.AddTail(reinterpret_cast<int*>(nCharacterId));

    return TRUE;
}

// 0x5BDB20
BOOL CInfGame::RemoveCharacterFromFamiliars(LONG nCharacterId)
{
    POSITION pos = m_familiars.Find(reinterpret_cast<int*>(nCharacterId));
    if (pos == NULL) {
        return FALSE;
    }

    m_familiars.RemoveAt(pos);

    // FIXME: What for?
    if (m_familiars.GetCount() == 0) {
        m_familiars.RemoveAll();
    }

    return TRUE;
}

// 0x5BDB70
DWORD CInfGame::GetScrollSpeed()
{
    if (m_gameAreas[m_visibleArea]->m_nKeyScrollState != 0) {
        return m_cOptions.m_nKeyboardScrollSpeed;
    } else {
        return m_cOptions.m_nMouseScrollSpeed;
    }
}

// 0x5BDBA0
void CInfGame::SetReputation(int a1, BOOL a2)
{
    // TODO: Incomplete.
}

// 0x5BDC00
void CInfGame::ReputationAdjustment(SHORT reputationAdjustment, BOOL set)
{
    // TODO: Incomplete.
}

// 0x5BDF80
void CInfGame::ChangeReputation(SHORT eventCode)
{
    LONG repMod = atol(m_ruleTables.m_tReputationMod.GetAt(CPoint(eventCode, m_nReputation / REPUTATION_MULTIPLIER - 1)));
    ReputationAdjustment(static_cast<SHORT>(repMod), FALSE);
}

// 0x5BF4B0
void CInfGame::AddDisposableItem(CItem* pItem)
{
    if (pItem != NULL) {
        CSingleLock disposableItemsLock(&m_disposableItemsCritSect, FALSE);
        disposableItemsLock.Lock(INFINITE);

        m_lDisposableItems.AddTail(pItem);

        disposableItemsLock.Unlock();
    }
}

// 0x5BF540
void CInfGame::DestroyDisposableItems()
{
    if (m_lDisposableItems.GetCount() > 0) {
        CSingleLock disposableItemsLock(&m_disposableItemsCritSect, FALSE);
        disposableItemsLock.Lock(INFINITE);

        POSITION pos = m_lDisposableItems.GetHeadPosition();
        while (pos != NULL) {
            CItem* pItem = m_lDisposableItems.GetAt(pos);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 16931
            UTIL_ASSERT(pItem != NULL);

            delete pItem;

            // NOTE: Meaningless.
            m_lDisposableItems.SetAt(pos, NULL);

            m_lDisposableItems.GetNext(pos);
        }

        m_lDisposableItems.RemoveAll();

        disposableItemsLock.Unlock();
    }
}

// 0x5BF610
void CInfGame::AddPartyGold(LONG dwAddPartyGold)
{
    if (dwAddPartyGold != 0) {
        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            CMessagePartyGold* pMessage = new CMessagePartyGold(FALSE,
                TRUE,
                dwAddPartyGold,
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        } else {
            if (dwAddPartyGold >= 0 || static_cast<DWORD>(-dwAddPartyGold) <= m_gameSave.m_nPartyGold) {
                m_gameSave.m_nPartyGold += dwAddPartyGold;
            } else {
                m_gameSave.m_nPartyGold = 0;
            }
        }
    }
}

// 0x5BF6A0
void CInfGame::SetupCharacters(BOOLEAN bProgressBarInPlace)
{
    CResRef cResArea;
    CPoint ptView;
    GetRuleTables().GetStartArea(cResArea, ptView);

    CString sAreaName;
    CGameArea* pArea;
    if (!m_bFromNewGame) {
        // Load path: area name comes from MultiplayerSettings so LoadArea is a cache hit; don't touch m_visibleArea (Unmarshal already set it from save header).
        sAreaName = GetMultiplayerSettings()->GetAreaStartName();
        pArea = LoadArea(sAreaName, -1, TRUE, bProgressBarInPlace);
        if (pArea == NULL) {
            return;
        }
    } else {
        cResArea.CopyToString(sAreaName);
        pArea = LoadArea(sAreaName, -1, TRUE, bProgressBarInPlace);
        if (pArea == NULL) {
            return;
        }
        m_visibleArea = pArea->m_id;
        pArea->m_visibility.SetAreaExplored();
        pArea->m_visibility.SetAreaVisible(TRUE);
    }

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        LONG nCharacterId = m_characterPortraits[nPortrait];
        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;
            BYTE rc = m_cObjectArray.GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                CPoint ptPos = pSprite->GetPos();
                if (ptPos.x == -1 && ptPos.y == -1) {
                    CPoint ptStart;
                    if (m_bFromNewGame) {
                        ptStart = GetRuleTables().GetStartPoint(nPortrait);
                        WORD nFacing = static_cast<WORD>(GetRuleTables().GetStartRotation(nPortrait));
                        pSprite->SetFacing(nFacing);
                    } else {
                        ptStart = CPoint(-1, -1);
                    }
                    pSprite->AddToArea(pArea, ptStart, 0, CGAMEOBJECT_LIST_FRONT);
                    pSprite->SetIdleSequence();
                    pSprite->m_canBeSeen = 4 * (CGameObject::VISIBLE_DELAY + 1);
                }
                m_cObjectArray.ReleaseDeny(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }

    if (m_bFromNewGame) {
        CInfinity* pInfinity = pArea->GetInfinity();
        CRect rView(pInfinity->rViewPort);
        pInfinity->SetViewPosition(ptView.x + rView.Width() / -2, ptView.y + rView.Height() / -2, TRUE);
    }
}

// 0x5BFC40
LONG CInfGame::GetProtagonist()
{
    if (m_nCharacters > 0) {
        return m_characterPortraits[0];
    }
    return CGameObjectArray::INVALID_INDEX;
}

// 0x5BFE10
void CInfGame::SetProtagonist(LONG nId)
{
    STR_RES strRes;

    if (g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_bDialogActive) {
        nId = g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_characterIndex;
    }

    STRREF strSurfaceUnderdark = 25688; // "sun-lander"

    if (nId == -1) {
        return;
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(g_pBaldurChitin->GetObjectGame()->m_characters[0],
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    // NOTE: Original code is slightly different. It sets token only if
    // `GetName` returns non-NULL, but this implementation already returns
    // reference.
    g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_CHARACTER, pSprite->GetName());

    STRREF strRace;
    switch (pSprite->GetAIType().m_nRace) {
    case CAIOBJECTTYPE_R_HUMAN:
        strRace = 7193;
        break;
    case CAIOBJECTTYPE_R_ELF:
        strRace = 7194;
        break;
    case CAIOBJECTTYPE_R_HALF_ELF:
        strRace = 7197;
        break;
    case CAIOBJECTTYPE_R_DWARF:
        strRace = 7182;
        if (pSprite->GetAIType().m_nSubRace == 2
            || pSprite->GetAIType().m_nSubRace == 1) {
            strSurfaceUnderdark = 25689; // "outsider"
        }
        break;
    case CAIOBJECTTYPE_R_HALFLING:
        strRace = 7195;
        break;
    case CAIOBJECTTYPE_R_GNOME:
        strRace = 7196;
        if (pSprite->GetAIType().m_nSubRace == 1) {
            strSurfaceUnderdark = 25689; // "outsider"
        }
        break;
    case CAIOBJECTTYPE_R_HALF_ORC:
        strRace = 22;
        break;
    default:
        strRace = -1;
        break;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(strRace, strRes);
    g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_RACE, strRes.szText);

    g_pBaldurChitin->GetTlkTable().Fetch(strSurfaceUnderdark, strRes);
    g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_SURFACE_UNDERDARK, strRes.szText);

    if (pSprite->GetAIType().m_nGender == CAIObjectType::SEX_FEMALE) {
        g_pBaldurChitin->GetTlkTable().Fetch(27475, strRes); // "ma'am"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_SIRMAAM, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27476, strRes); // "girl"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_GIRLBOY, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27479, strRes); // "sister"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_BROTHERSISTER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27480, strRes); // "lady"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_LADYLORD, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27483, strRes); // "female"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MALEFEMALE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27485, strRes); // "she"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HESHE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27487, strRes); // "her"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HISHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27487, strRes); // "her"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HIMHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27490, strRes); // "woman"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MANWOMAN, strRes.szText);
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(27473, strRes); // "sir"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_SIRMAAM, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27477, strRes); // "boy"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_GIRLBOY, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27478, strRes); // "brother"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_BROTHERSISTER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27481, strRes); // "lord"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_LADYLORD, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27482, strRes); // "male"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MALEFEMALE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27484, strRes); // "he"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HESHE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27486, strRes); // "his"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HISHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27488, strRes); // "him"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HIMHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27489, strRes); // "man"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MANWOMAN, strRes.szText);
    }

    m_nProtagonistId = nId;

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->m_characters[0],
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x5C04F0
BOOLEAN CInfGame::FindAreaID(DWORD nAreaId)
{
    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL) {
            if (m_gameAreas[index]->m_nInitialAreaID == nAreaId) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

// 0x5C09C0
CString CInfGame::GetDirSaveRoot()
{
    CString sResolvedFileName;
    CString sFileName;

    sFileName = m_sMultiplayerSaveDir;
    if (!g_pBaldurChitin->lAliases.ResolveFileName(sFileName, sResolvedFileName)) {
        sResolvedFileName = sFileName;
    }

    return sResolvedFileName;
}

// 0x5C0A80
CString CInfGame::GetDirSave()
{
    return m_sMultiplayerSaveDir + m_sSaveGame + "\\";
}

// TODO: Somehow its different from function above. Figure out.
//
// 0x5C0B30
CString CInfGame::GetSaveGameDirectory()
{
    return m_sMultiplayerSaveDir + m_sSaveGame + "\\";
}

// 0x5C0BB0
//
// Divide an experience award among the living party and deliver it -- the
// engine's kill / quest XP payout (GemRB models this routine as Game::ShareXP
// with SX_DIVIDE, plus SX_CR for combat kills).  Pass one walks the party,
// counting the members that are not dead and summing their levels.  When
// bCombat is set the raw award is first rescaled against the party's average
// level through the monster-rate table and then halved; otherwise it is used
// as-is.  The pool is split evenly between the living members, with the
// division remainder handed out a point at a time to the earliest members, and
// pass two posts each living member an opcode-104 (XP) effect carrying its
// share -- after docking that share by the member's multiclassing penalty.
// Returns the total actually handed out and announces it through FeedBack.
LONG CInfGame::ShareXP(LONG nXP, BOOLEAN bFeedback, BOOLEAN bCombat)
{
    // --- pass 1: count the living members and sum their levels ---
    SHORT nLiving = 0;
    SHORT nLevelSum = 0;

    for (SHORT i = 0; i < m_nCharacters; i++) {
        LONG nMemberId = GetCharacterId(i);

        CGameSprite* pMember;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nMemberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pMember),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if ((pMember->GetDerivedStats()->m_generalState & STATE_DEAD) == 0) {
                nLiving++;
                nLevelSum += pMember->GetDerivedStats()->m_nLevel;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nMemberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (nLiving == 0) {
        nLiving = 1;
    }

    // --- the combat path rescales by the party average level, then halves ---
    LONG nPool;
    if (bCombat) {
        nPool = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetMoncRate(
                    static_cast<WORD>(nXP),
                    static_cast<WORD>(nLevelSum / nLiving))
            >> 1;
    } else {
        nPool = nXP;
    }

    LONG nPerMember = nPool / nLiving;
    LONG nRemainder = nPool % nLiving;

    // --- pass 2: deliver each living member its (penalty-adjusted) share ---
    ITEM_EFFECT effect;
    CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_XP);
    effect.durationType = 1;
    effect.effectAmount = nPerMember;

    LONG nTotal = 0;
    for (SHORT i = 0; i < m_nCharacters; i++) {
        LONG nMemberId = GetCharacterId(i);

        CGameSprite* pMember;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nMemberId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pMember),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if ((pMember->GetDerivedStats()->m_generalState & STATE_DEAD) == 0) {
                effect.effectAmount = nPerMember;
                if (nRemainder > 0) {
                    nRemainder--;
                    effect.effectAmount = nPerMember + 1;
                }

                INT nPenalty = pMember->GetMulticlassingPenalty();
                if (nPenalty > 0) {
                    effect.effectAmount = (100 - nPenalty) * effect.effectAmount / 100;
                }

                nTotal += effect.effectAmount;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    CPoint(-1, -1),
                    -1,
                    CPoint(-1, -1));

                CMessage* message = new CMessageAddEffect(pEffect, nMemberId, nMemberId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nMemberId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    FeedBack(FEEDBACK_EXPERIENCE, nTotal, bFeedback);
    return nTotal;
}

// 0x5C20E0
BOOL CInfGame::CanEnterStore(STRREF& strError)
{
    BOOL bScattered = FALSE;
    CResRef cResArea;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        if (bScattered) {
            break;
        }

        LONG nCharacterId = GetCharacterId(nPortrait);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->Animate()) {
                if (cResArea != "") {
                    if (cResArea != pSprite->GetArea()->m_resRef) {
                        bScattered = TRUE;
                    }
                } else {
                    cResArea = pSprite->GetArea()->m_resRef;
                }
            }

            m_cObjectArray.ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bScattered = TRUE;
        }
    }

    if (!CanSaveGame(strError, TRUE, TRUE)) {
        if (strError == 16501) {
            // "You cannot enter the store; there are monsters nearby."
            strError = 20049;
        } else if (strError == 16502) {
            // "You may not enter the store at this time."
            strError = 261;
        } else {
            // "Not permitted to buy/sell items."
            strError = 10847;
        }
        return FALSE;
    }

    if (bScattered) {
        // "You may not enter the store when your party is scattered."
        strError = 20690;
        return FALSE;
    }

    return TRUE;
}

// 0x5C2250
BOOL CInfGame::AddCharacterToOverflow(LONG id)
{
    m_characterOverflow.Add(reinterpret_cast<int*>(id));
    m_nCharacterOverflowCount++;
    return TRUE;
}

// 0x5C2280
CStringList* CInfGame::GetScripts()
{
    CStringList* pList = new CStringList();

    CString sFileName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirScripts() + "*.*";

    INT nCount = 0;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = findFileData.cFileName;
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0
                && sFileName != "."
                && sFileName != "..") {
                sFileName = sFileName.SpanExcluding(".");
                if (sFileName.GetLength() <= 8) {
                    if (nCount >= 16383) {
                        break;
                    }

                    sFileName.MakeUpper();

                    POSITION pos = pList->GetHeadPosition();
                    while (pos != NULL) {
                        sTemp = pList->GetAt(pos);
                        if (sFileName == sTemp) {
                            break;
                        }

                        if (sFileName < pList->GetAt(pos)) {
                            pList->InsertBefore(pos, sFileName);
                            nCount++;
                            break;
                        }

                        pList->GetNext(pos);
                    }

                    if (pos == NULL) {
                        pList->AddTail(sFileName);
                        nCount++;
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C24C0
CString CInfGame::GetDirScripts()
{
    return m_sScriptsDir;
}

// 0x5C24F0
void CInfGame::StartCharacterTerminationSequence()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_gameSave.m_mode == 322) {
        pGame->m_gameSave.m_mode = -1;
    }

    pGame->m_gameSave.m_cutScene = FALSE;

    if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineWorld) {
        g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
    }

    g_pBaldurChitin->m_pEngineWorld->m_bGameOverPanel = TRUE;
}

// 0x5C2570
void CInfGame::ReadyCharacterTerminationSequence(int a1, int a2)
{
    if (!m_gameSave.m_bArenaMode) {
        // FIXME: What for (this function is not static as
        // `StartCharacterTerminationSequence`).
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        pGame->m_gameSave.m_mode = 322;
        pGame->m_gameSave.m_cutScene = TRUE;

        g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);

        pGame->m_tempCursor = 4;
        pGame->m_gameAreas[pGame->m_visibleArea]->m_nScrollState = 0;
        pGame->m_nCharacterTerminationSequenceDelay = 150;

        g_pBaldurChitin->m_pEngineWorld->field_10F0 = a1;
        g_pBaldurChitin->m_pEngineWorld->field_10F4 = a2;
    }
}

// 0x5C2630
void CInfGame::FeedBack(SHORT feedBackId, LONG value, BOOLEAN a3)
{
    CString sTime;

    if (!a3) {
        a3 = m_cOptions.m_bShowQuestXP;
    }

    switch (feedBackId) {
    case FEEDBACK_GOLD:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            CString sText;
            STR_RES strRes;

            if (value != 0) {
                if (value > 0) {
                    // "The party has gained gold"
                    g_pBaldurChitin->GetTlkTable().Fetch(17572, strRes);
                } else {
                    // "The party has lost gold"
                    g_pBaldurChitin->GetTlkTable().Fetch(17573, strRes);
                    value = -value;
                }
                sText.Format("%d", value);

                if (strRes.cSound.GetRes() != NULL) {
                    if (!strRes.cSound.GetLooping()) {
                        strRes.cSound.SetFireForget(TRUE);
                    }
                    strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(GetVisibleArea()));
                    strRes.cSound.Play(FALSE);
                }

                if (strRes.szText != "") {
                    if (a3) {
                        sText = strRes.szText + ": " + sText;
                    } else {
                        sText = strRes.szText + ".";
                    }
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sText,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_EXPERIENCE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            CString sText;
            STR_RES strRes;

            if (value != 0) {
                if (value > 0) {
                    // "The party has gained experience"
                    g_pBaldurChitin->GetTlkTable().Fetch(17574, strRes);
                } else {
                    // "The party has lost experience"
                    g_pBaldurChitin->GetTlkTable().Fetch(17575, strRes);
                    value = -value;
                }
                sText.Format("%d", value);

                if (strRes.cSound.GetRes() != NULL) {
                    if (!strRes.cSound.GetLooping()) {
                        strRes.cSound.SetFireForget(TRUE);
                    }
                    strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(GetVisibleArea()));
                    strRes.cSound.Play(FALSE);
                }

                if (strRes.szText != "") {
                    if (a3) {
                        sText = strRes.szText + ": " + sText;
                    } else {
                        sText = strRes.szText + ".";
                    }
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sText,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_ITEMGAINED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "The party has gained an item."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                17576,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_ITEMLOST:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "The party has lost an item."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                17577,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_REPUTATION:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            CString sText;
            STR_RES strRes;

            if (value != 0) {
                if (value > 0) {
                    // "The party's reputation has increased"
                    g_pBaldurChitin->GetTlkTable().Fetch(19686, strRes);
                } else {
                    // "The party's reputation has decreased"
                    g_pBaldurChitin->GetTlkTable().Fetch(19687, strRes);
                    value = -value;
                }
                sText.Format("%d", value);

                if (strRes.cSound.GetRes() != NULL) {
                    if (!strRes.cSound.GetLooping()) {
                        strRes.cSound.SetFireForget(TRUE);
                    }
                    strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(GetVisibleArea()));
                    strRes.cSound.Play(FALSE);
                }

                if (strRes.szText != "") {
                    if (a3) {
                        sText = strRes.szText + ": " + sText;
                    } else {
                        sText = strRes.szText + ".";
                    }
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sText,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_TIMEPASSEDJOURNEY:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // NOTE: Signed compare.
            if (value >= static_cast<LONG>(CTimerWorld::TIMESCALE_MSEC_PER_HOUR)) {
                // "The journey took <DURATION>."
                CTimerWorld::GetCurrentTimeString(value, 10689, sTime);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sTime,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_TIMEPASSEDREST:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // NOTE: Signed compare.
            if (value >= static_cast<LONG>(CTimerWorld::TIMESCALE_MSEC_PER_HOUR)) {
                // "You have rested for <DURATION>."
                CTimerWorld::GetCurrentTimeString(value, 10690, sTime);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sTime,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_INVENTORYFULL_ITEMDROPPED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "Inventory Full: The item has been dropped."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                10959,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_JOURNAL_UPDATE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "Your journal has been updated."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                11359,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_WORLDMAP_UPDATE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "Your map has been updated."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                11360,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    }
}

// 0x5C3210
CString CInfGame::GetDirPortraits()
{
    return m_sPortraitsDir;
}

// 0x5C3510
CString CInfGame::GetDirCharacters()
{
    return m_sCharactersDir;
}

// 0x5C1090
BYTE CInfGame::GetCharactersControlled()
{
    BYTE bControlled = 0;
    CGameObject* pObject;

    for (SHORT nIndex = 0; nIndex < m_nCharacters; nIndex++) {
        LONG id = m_characterPortraits[nIndex];

        BYTE rc;
        do {
            // NOTE: Using global. Implying inlined static function.
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(id,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // NOTE: Uninline.
            if (pObject->InControl()) {
                bControlled |= 1 << nIndex;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return bControlled;
}

// 0x5C1160
BOOLEAN CInfGame::RestParty(int a2, unsigned char a3)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x5C1350
BOOLEAN CInfGame::RestPartyPrivate(INT nHP, BOOLEAN bRenting)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x5C1E30
BOOL CInfGame::CanRestParty(STRREF& strError, unsigned char a2, unsigned char a3, unsigned char a4)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x5C3010
CStringList* CInfGame::GetPortraits()
{
    CStringList* pList = new CStringList();

    CString sFileName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirPortraits() + "*.BMP";

    INT nCount = 0;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = findFileData.cFileName;
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                sFileName = sFileName.SpanExcluding(".");
                if (sFileName.GetLength() <= 8) {
                    if (nCount >= 32767) {
                        break;
                    }

                    POSITION pos = pList->GetHeadPosition();
                    while (pos != NULL) {
                        sTemp = pList->GetAt(pos);
                        if (sFileName == sTemp) {
                            break;
                        }

                        if (sFileName < pList->GetAt(pos)) {
                            pList->InsertBefore(pos, sFileName);
                            nCount++;
                            break;
                        }

                        pList->GetNext(pos);
                    }

                    if (pos == NULL) {
                        pList->AddTail(sFileName);
                        nCount++;
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C3240
CStringList* CInfGame::GetImportCharacters()
{
    CStringList* pList = new CStringList();

    CString sFileName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirCharacters() + "*.chr";

    INT nCount = 0;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = findFileData.cFileName;
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                sFileName = sFileName.SpanExcluding(".");
                if (sFileName.GetLength() <= 8) {
                    if (nCount >= 32767) {
                        break;
                    }

                    FILE* stream = fopen(GetDirCharacters() + findFileData.cFileName, "rb");
                    if (stream != NULL) {
                        unsigned char data[8];
                        size_t bytesRead = fread(data, 1, sizeof(data), stream);
                        fclose(stream);

                        if (bytesRead == 8 && memcmp(data, "CHR V2.2", 8) == 0) {
                            sFileName.MakeUpper();

                            POSITION pos = pList->GetHeadPosition();
                            while (pos != NULL) {
                                sTemp = pList->GetAt(pos);
                                if (sFileName == sTemp) {
                                    break;
                                }

                                if (sFileName < pList->GetAt(pos)) {
                                    pList->InsertBefore(pos, sFileName);
                                    nCount++;
                                    break;
                                }

                                pList->GetNext(pos);
                            }

                            if (pos == NULL) {
                                pList->AddTail(sFileName);
                                nCount++;
                            }
                        }
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C3540
CStringList* CInfGame::GetSounds()
{
    CStringList* pList = new CStringList();

    CString sSoundSetDirName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirSounds() + "*";
    INT nCount = 0;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sSoundSetDirName = findFileData.cFileName;
            if (sSoundSetDirName != "."
                && sSoundSetDirName != ".."
                && (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                sSoundSetDirName.MakeUpper();

                INT nLength = sSoundSetDirName.GetLength();
                if (nLength > 0 && nLength < 32) {
                    if (nCount >= 32766) {
                        break;
                    }

                    POSITION pos = pList->GetHeadPosition();
                    while (pos != NULL) {
                        sTemp = pList->GetAt(pos);
                        if (sSoundSetDirName == sTemp) {
                            break;
                        }

                        if (sSoundSetDirName < pList->GetAt(pos)) {
                            pList->InsertBefore(pos, sSoundSetDirName);
                            nCount++;
                            break;
                        }

                        pList->GetNext(pos);
                    }

                    if (pos == NULL) {
                        pList->AddTail(sSoundSetDirName);
                        nCount++;
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C3770
STRREF CInfGame::GetAreaStrRef(CString a1)
{
    return 9885;
}

// 0x5C3790
void CInfGame::ChangeBiography(BYTE nFixedPartyId, CString szBiography)
{
    // TODO: Incomplete.
}

// 0x5C72D0
BOOLEAN CInfGame::GetGameSpyCharacterInformation(INT nCharacterSlot, CString& sName, CString& sRace, CString& sClass, CString& sLevel)
{
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();
    CString sTemp;

    LONG nCharacterId = GetFixedOrderCharacterId(nCharacterSlot);

    if (nCharacterId == CGameObjectArray::INVALID_INDEX) {
        return FALSE;
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    BYTE nClass = pSprite->m_derivedStats.GetBestClass();

    sName = pSprite->GetName();
    ruleTables.GetRaceStringMixed(pSprite->m_liveTypeAI.m_nRace, sClass, 0);
    ruleTables.GetClassStringGameSpy(nClass, pSprite->m_baseStats.m_specialization, sClass);

    sTemp.Format("/%d", pSprite->m_derivedStats.m_nLevel);
    sLevel += sTemp;

    m_cObjectArray.ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x5C76B0
void CInfGame::MultiplayerSetCharacterCreationLocation()
{
    CResRef areaResRef;
    CString sAreaString;
    BOOLEAN bValueSet = FALSE;

    for (BYTE nIndex = 0; nIndex < 6; nIndex++) {
        if (bValueSet) {
            break;
        }

        // NOTE: Uninline `GetFixedOrderCharacterId`.
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterId(nIndex);

        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                CGameArea* pArea = pSprite->GetArea();
                if (pArea != NULL) {
                    areaResRef = pArea->m_resRef;
                    areaResRef.CopyToString(sAreaString);

                    CPoint ptStart = pSprite->GetPos();
                    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetCharacterCreationLocation(sAreaString,
                        ptStart);

                    bValueSet = TRUE;

                    // TODO: Probably wrong, object released only when area is
                    // not null.
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        }
    }
}

// 0x5C7830
BOOL CInfGame::IsItemExclusive(CGameSprite* pSprite, INT slotNum, CItem* pItem, STRREF& strError)
{
    const CRuleTables& cRule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    strError = -1;

    INT slots[] = {
        7,
        8,
        1,
        4,
        0,
    };

    if (pItem == NULL || !cRule.IsItemExclusive(pItem)) {
        return FALSE;
    }

    for (int index = 0; index < 5; index++) {
        if (slotNum != slots[index]
            && pSprite->GetEquipment()->m_items[slots[index]] != NULL
            && cRule.IsItemExclusive(pSprite->GetEquipment()->m_items[slots[index]])) {
            strError = 20685;
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5C78F0
CWorldMap* CInfGame::GetWorldMap(CString sArea)
{
    CString v1;

    // NOTE: Not sure what first condition is for.
    if (sArea
        && sArea[0] != '\0'
        && sArea[1] != '\0'
        && sArea[2] != '\0') {
        v1 = sArea.Mid(2);
        if (atoi(v1) >= 9000) {
            return &m_cOtherWorldMap;
        }
    }

    return &m_cWorldMap;
}

// 0x5C79C0
BOOL CInfGame::IsSpecialArea(CString sArea)
{
    CString v1;

    // NOTE: Not sure what first condition is for.
    if (sArea
        && sArea[0] != '\0'
        && sArea[1] != '\0'
        && sArea[2] != '\0') {
        v1 = sArea.Mid(2);
        if (atoi(v1) >= 9000) {
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5C8190
DWORD CInfGame::FindItemInStore(const CResRef& cResStore, const CResRef& cResItem, BOOL checkForIdentified)
{
    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;
        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(cResStore, TRUE);
            cStore.SetResRef(cResStore);
        } else {
            cStore.SetResRef(cResStore);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return 0;
                }
                cStore.SetResRef(cResStore);
                bStoreDemanded = TRUE;
            }
        }

        CResRef curItemRef;
        DWORD inStock = 0;
        CItem tempItem;

        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            if (curItemRef == cResItem) {
                if (checkForIdentified) {
                    cStore.GetItem(nIndex, tempItem);
                    if ((tempItem.m_flags & 0x1) != 0) {
                        inStock += cStore.GetItemNumInStock(nIndex);
                    }
                } else {
                    inStock += cStore.GetItemNumInStock(nIndex);
                }
            }
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            ReleaseServerStore(cResStore);
        } else {
            if (bStoreDemanded) {
                CMessage* message = new CMessageStoreRelease(cResStore, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        return inStock;
    } else {
        CStore cStore;
        CResRef curItemRef;
        DWORD inStock = 0;

        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            if (curItemRef == cResItem) {
                inStock += cStore.GetItemNumInStock(nIndex);
            }
        }

        return inStock;
    }
}

// 0x5C8510
SHORT CInfGame::TakeItemFromStore(const CResRef& cResStore, const CResRef& cResItem, LONG number)
{
    CItem tempItem;
    CMessage* message;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(cResStore, TRUE);
            cStore.SetResRef(cResStore);
        } else {
            cStore.SetResRef(cResStore);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return 0;
                }

                cStore.SetResRef(cResStore);
                bStoreDemanded = TRUE;
            }
        }

        CResRef curResItem;
        CStore* pStore;
        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curResItem = cStore.GetItemId(nIndex);
            if (curResItem == cResItem) {
                DWORD inStock = cStore.GetItemNumInStock(nIndex);
                cStore.GetItem(nIndex, tempItem);

                if (!g_pChitin->cNetwork.GetSessionHosting() && !bStoreDemanded) {
                    bStoreDemanded = TRUE;
                    message = new CMessageStoreDemand(cResStore, -1, -1);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                if (g_pChitin->cNetwork.GetSessionHosting()) {
                    pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(cResStore);

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                    // __LINE__: 22412
                    UTIL_ASSERT(pStore != NULL);
                }

                // FIXME: Looks wrong, `removed` is reset on every iteration,
                // but probably should be accumulated.
                DWORD removed = 0;
                // NOTE: Unsigned compare.
                while (removed < inStock && removed < static_cast<DWORD>(number)) {
                    message = new CMessageStoreRemoveItem(cResStore,
                        cResItem,
                        tempItem.m_flags,
                        -1,
                        1,
                        -1,
                        -1);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                    if (g_pChitin->cNetwork.GetSessionHosting()) {
                        pStore->RemoveItemExt(cResItem, tempItem.m_flags, -1, 1, NULL);
                    }
                }

                if (!g_pChitin->cNetwork.GetSessionHosting()) {
                    CStore::InvalidateStore(cResStore);
                }

                // NOTE: Unsigned compare.
                if (removed >= static_cast<DWORD>(number)) {
                    if (g_pChitin->cNetwork.GetSessionHosting()) {
                        g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cResStore);
                    } else {
                        if (bStoreDemanded) {
                            message = new CMessageStoreRelease(cResStore, -1, -1);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                    }

                    return static_cast<SHORT>(removed);
                }
            }
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cResStore);
        } else {
            if (bStoreDemanded) {
                message = new CMessageStoreRelease(cResStore, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        return 0;
    } else {
        CStore cStore(cResStore);
        CResRef curItemRef;
        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            cStore.GetItem(nIndex, tempItem);
            if (curItemRef == cResItem) {
                DWORD inStock = cStore.GetItemNumInStock(nIndex);
                // FIXME: Looks wrong, `removed` is reset on every iteration,
                // but probably should be accumulated.
                DWORD removed = 0;
                // NOTE: Unsigned compare.
                while (removed < static_cast<DWORD>(number) && removed < inStock) {
                    cStore.RemoveItemExt(cResItem, tempItem.m_flags, -1, 1, NULL);
                    removed++;
                }

                // NOTE: Unsigned compare.
                if (removed >= static_cast<DWORD>(number)) {
                    cStore.Marshal(m_sTempDir);
                    return static_cast<SHORT>(removed);
                }
            }
        }
        cStore.Marshal(m_sTempDir);
        return 0;
    }
}

// 0x5C93E0
INT CInfGame::CountNPCs()
{
    INT nCount = 0;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        LONG nCharacterId = GetCharacterId(nPortrait);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->m_bHiding == 1) {
                nCount++;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return nCount;
}

// NOTE: Somewhat broken version of `LoadMultiPlayerPermissions`.
//
// 0x5C9490
void CInfGame::ResetMultiPlayerPermissions()
{
    if (!g_pBaldurChitin->cNetwork.GetSessionOpen()
        || !g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        return;
    }

    CMultiplayerSettings* pSettings = GetMultiplayerSettings();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 22952
    UTIL_ASSERT(pSettings != NULL);

    CString sPermissions;
    CString sDefault("");

    GetPrivateProfileStringA("Multiplayer",
        "Default Permissions",
        sDefault,
        sPermissions.GetBuffer(10),
        10,
        g_pBaldurChitin->GetIniFileName());
    sPermissions.ReleaseBuffer(-1);

    if (sPermissions.GetLength() == 8) {
        BOOL bPurchasing = sPermissions[0] == '1';
        BOOL bAreaTransition = sPermissions[1] == '1';
        BOOL bDialog = sPermissions[2] == '1';
        BOOL bCharRecords = sPermissions[3] == '1';
        BOOL bPausing = sPermissions[4] == '1';
        BOOL bGroupPool = sPermissions[5] == '1';
        BOOL bLeader = sPermissions[6] == '1';
        BOOL bModifyChars = sPermissions[7] == '1';

        // FIXME: Individual permission values are used as player slots. So
        // depending on the value appropriate permission is set either on
        // player 0 or 1.

        pSettings->SetPermission(bPurchasing,
            CGamePermission::PURCHASING,
            bPurchasing,
            TRUE);
        pSettings->SetPermission(bAreaTransition,
            CGamePermission::AREA_TRANSITION,
            bAreaTransition,
            TRUE);
        pSettings->SetPermission(bDialog,
            CGamePermission::DIALOG,
            bDialog,
            TRUE);
        pSettings->SetPermission(bCharRecords,
            CGamePermission::CHAR_RECORDS,
            bCharRecords,
            TRUE);
        pSettings->SetPermission(bPausing,
            CGamePermission::PAUSING,
            bPausing,
            TRUE);
        pSettings->SetPermission(bGroupPool,
            CGamePermission::GROUP_POOL,
            bGroupPool,
            TRUE);
        pSettings->SetPermission(bLeader,
            CGamePermission::LEADER,
            bLeader,
            TRUE);
        pSettings->SetPermission(bModifyChars,
            CGamePermission::MODIFY_CHARS,
            bModifyChars,
            TRUE);
    }

    INT nRestrictStore = GetPrivateProfileIntA("Multiplayer",
        "Restrict Stores",
        0,
        g_pBaldurChitin->GetIniFileName());
    pSettings->SetRestrictStoreOption(nRestrictStore != 0);

    BYTE nImportingBitField = static_cast<BYTE>(GetPrivateProfileIntA("Multiplayer",
        "Import Character",
        CMultiplayerSettings::IMPORT_ALL,
        g_pBaldurChitin->GetIniFileName()));
    pSettings->SetImportingCharacterOption(nImportingBitField);
}

// 0x5C9720
INT CInfGame::GetNumberOfItemsInBag(const CResRef& storeResRef, CString a2)
{
    CMessage* message;
    INT nNumberOfItemsInBag = 0;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 23072
        UTIL_ASSERT_MSG(FALSE, "Very bad things are about to happen in GetNumberOfItemsInBag");
    }

    BOOL demanded = FALSE;
    CStore store;
    if (g_pChitin->cNetwork.GetSessionHosting()) {
        DemandServerStore(storeResRef, TRUE);
        store.SetResRef(storeResRef);
    } else {
        store.SetResRef(storeResRef);
        if (!store.m_bLocalCopy || memcmp(store.m_pVersion, "STORV9.0", 8) != 0) {
            if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(storeResRef.GetResRefStr(),
                    1014,
                    TRUE,
                    TRUE,
                    TRUE)) {
                g_pChitin->cNetwork.CloseSession(TRUE);
                return FALSE;
            }

            store.SetResRef(storeResRef);
            demanded = TRUE;
        }
    }

    CResRef itemResRef;

    if (g_pChitin->cNetwork.GetSessionHosting() || demanded) {
        store.InvalidateStore(storeResRef);
    } else {
        demanded = TRUE;
        message = new CMessageStoreDemand(storeResRef, -1, -1);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    for (INT nIndex = 0; nIndex < store.GetNumItems(); nIndex++) {
        itemResRef = store.GetItemId(nIndex);

        // FIXME: Temporary instance, should be on stack.
        CItem* pItem = new CItem();
        store.GetItem(nIndex, *pItem);
        if (pItem->GetResRef() != a2) {
            nNumberOfItemsInBag += store.GetItemNumInStock(nIndex);
        }

        // FIXME: Probably leaking `pItem`.
    }

    if (g_pChitin->cNetwork.GetSessionHosting()) {
        g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(storeResRef);
    } else {
        if (demanded) {
            message = new CMessageStoreRelease(storeResRef, -1, -1);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    return nNumberOfItemsInBag;
}

// 0x5C9AC0
LONG CInfGame::GetAveragePartyLevel()
{
    LONG nAverageLevel = 0;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        // NOTE: Uninline.
        LONG nCharacterId = GetCharacterId(nPortrait);

        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;
            m_cObjectArray.GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);

            nAverageLevel += pSprite->GetLevel();

            m_cObjectArray.ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (nAverageLevel == 0 || nAverageLevel / m_nCharacters < 1) {
        return 1;
    }

    return nAverageLevel / m_nCharacters;
}

// NOTE: Unclear why `nClass` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA5D0
UINT CInfGame::GetSpellcasterIndex(const BYTE& nClass)
{
    switch (nClass) {
    case CAIOBJECTTYPE_C_BARD:
        return 0;
    case CAIOBJECTTYPE_C_CLERIC:
        return 1;
    case CAIOBJECTTYPE_C_DRUID:
        return 2;
    case CAIOBJECTTYPE_C_PALADIN:
        return 3;
    case CAIOBJECTTYPE_C_RANGER:
        return 4;
    case CAIOBJECTTYPE_C_SORCERER:
        return 5;
    case CAIOBJECTTYPE_C_WIZARD:
        return 6;
    }

    CString sError;
    sError.Format("Invalid spell caster index: %d", nClass);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23583
    UTIL_ASSERT_MSG(FALSE, sError);
}

// NOTE: Unclear why `nList` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA730
BYTE CInfGame::GetSpellcasterClass(const UINT& nIndex)
{
    switch (nIndex) {
    case 0:
        return CAIOBJECTTYPE_C_BARD;
    case 1:
        return CAIOBJECTTYPE_C_CLERIC;
    case 2:
        return CAIOBJECTTYPE_C_DRUID;
    case 3:
        return CAIOBJECTTYPE_C_PALADIN;
    case 4:
        return CAIOBJECTTYPE_C_RANGER;
    case 5:
        return CAIOBJECTTYPE_C_SORCERER;
    case 6:
        return CAIOBJECTTYPE_C_WIZARD;
    }

    CString sError;
    sError.Format("Invalid spell caster class: %d", nIndex);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23624
    UTIL_ASSERT_MSG(FALSE, sError);
}

// NOTE: Unclear why `nIndex` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA870
DWORD CInfGame::GetSpellcasterClassMask(const UINT& nIndex)
{
    switch (nIndex) {
    case 0:
        return CLASSMASK_BARD;
    case 1:
        return CLASSMASK_CLERIC;
    case 2:
        return CLASSMASK_DRUID;
    case 3:
        return CLASSMASK_PALADIN;
    case 4:
        return CLASSMASK_RANGER;
    case 5:
        return CLASSMASK_SORCERER;
    case 6:
        return CLASSMASK_WIZARD;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23663
    UTIL_ASSERT_MSG(FALSE, "Invalid spell caster class mask");
}

// NOTE: Unclear why `a1` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA910
UINT CInfGame::GetSpellcasterDomainIndex(const DWORD& nSpecialization)
{
    if ((nSpecialization & SPECMASK_CLERIC_ILMATER) != 0) {
        return 0;
    } else if ((nSpecialization & SPECMASK_CLERIC_LATHANDER) != 0) {
        return 1;
    } else if ((nSpecialization & SPECMASK_CLERIC_SELUNE) != 0) {
        return 2;
    } else if ((nSpecialization & SPECMASK_CLERIC_HELM) != 0) {
        return 3;
    } else if ((nSpecialization & SPECMASK_CLERIC_OGHMA) != 0) {
        return 4;
    } else if ((nSpecialization & SPECMASK_CLERIC_TEMPUS) != 0) {
        return 5;
    } else if ((nSpecialization & SPECMASK_CLERIC_BANE) != 0) {
        return 6;
    } else if ((nSpecialization & SPECMASK_CLERIC_MASK) != 0) {
        return 7;
    } else if ((nSpecialization & SPECMASK_CLERIC_TALOS) != 0) {
        return 8;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23714
    UTIL_ASSERT_MSG(FALSE, "Invalid spell caster domain index");
}

// 0x5CA9E0
DWORD CInfGame::GetClassMask(const BYTE& nClass)
{
    switch (nClass) {
    case CAIOBJECTTYPE_C_BARBARIAN:
        return CLASSMASK_BARBARIAN;
    case CAIOBJECTTYPE_C_BARD:
        return CLASSMASK_BARD;
    case CAIOBJECTTYPE_C_CLERIC:
        return CLASSMASK_CLERIC;
    case CAIOBJECTTYPE_C_DRUID:
        return CLASSMASK_DRUID;
    case CAIOBJECTTYPE_C_FIGHTER:
        return CLASSMASK_FIGHTER;
    case CAIOBJECTTYPE_C_MONK:
        return CLASSMASK_MONK;
    case CAIOBJECTTYPE_C_PALADIN:
        return CLASSMASK_PALADIN;
    case CAIOBJECTTYPE_C_RANGER:
        return CLASSMASK_RANGER;
    case CAIOBJECTTYPE_C_ROGUE:
        return CLASSMASK_ROGUE;
    case CAIOBJECTTYPE_C_SORCERER:
        return CLASSMASK_SORCERER;
    case CAIOBJECTTYPE_C_WIZARD:
        return CLASSMASK_WIZARD;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23834
    UTIL_ASSERT_MSG(FALSE, "CInfGame::GetClassMask() - Invalid class mask!");
}

// 0x5CAAD0
BYTE CInfGame::GetSpellType(const CResRef& resRef)
{
    UINT nID;

    // NOTE: Uninline.
    if (m_songs.Find(resRef, nID)) {
        return 3;
    }

    // NOTE: Uninline.
    if (m_shapeshifts.Find(resRef, nID)) {
        return 4;
    }

    if (m_innateSpells.Find(resRef, nID)) {
        return 2;
    }

    if (m_spells.Find(resRef, nID)) {
        return 1;
    }

    CString sError(" ");
    sError.Format("*** Invalid spell: %s\n", (LPCSTR)resRef.GetResRefStr());

    // TODO: Report error.

    return 0;
}

// 0x5CAC30
INT CInfGame::GetSpellLevel(const CResRef& resRef, BYTE nClass, DWORD nSpecialization)
{
    UINT nID;

    // NOTE: Uninline.
    if (!m_spells.Find(resRef, nID)) {
        CString sError;
        sError.Format("Spell not in master list - Invalid spell: %s", (LPCSTR)resRef.GetResRefStr());

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 23940
        UTIL_ASSERT_MSG(FALSE, (LPCSTR)sError);
    }

    if (nSpecialization != 0) {
        UINT nDomainIndex = GetSpellcasterDomainIndex(nSpecialization);

        for (UINT nLevel = 0; nLevel < m_spellsByDomain[nDomainIndex].m_nHighestLevel; nLevel++) {
            CSpellList* pSpells = &(m_spellsByDomain[nDomainIndex].m_lists[nLevel]);
            for (UINT nIndex = 0; nIndex < pSpells->m_nCount; nIndex++) {
                if (pSpells->m_pList[nIndex].m_nID == nID) {
                    return nLevel;
                }
            }
        }
    } else {
        UINT nClassIndex = GetSpellcasterIndex(nClass);

        for (UINT nLevel = 0; nLevel < m_spellsByClass[nClassIndex].m_nHighestLevel; nLevel++) {
            CSpellList* pSpells = &(m_spellsByClass[nClassIndex].m_lists[nLevel]);
            for (UINT nIndex = 0; nIndex < pSpells->m_nCount; nIndex++) {
                if (pSpells->m_pList[nIndex].m_nID == nID) {
                    return nLevel;
                }
            }
        }
    }

    return CSPELLLIST_MAX_LEVELS;
}

// 0x5CADF0
void CInfGame::MarkAliveCharacters()
{
    // FIXME: What for?
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
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
            if ((pSprite->m_baseStats.m_generalState & STATE_DEAD) == 0
                && (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                pSprite->field_9D15 = 1;
            }

            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// NOTE: Odd location.
//
// 0x428620
INT CInfGame::GetCurrentChapter()
{
    CVariable* pVar = GetVariables()->FindKey(CHAPTER_GLOBAL);
    if (pVar != NULL) {
        return pVar->m_intValue;
    }
    return 0;
}

// NOTE: Odd location
//
// 0x435110
void CInfGame::SetCurrentChapter(INT nChapter)
{
    CVariable* pVar = GetVariables()->FindKey(CHAPTER_GLOBAL);
    if (pVar != NULL) {
        pVar->m_intValue = nChapter;
    }
}

// 0x452FD0
CInfButtonArray* CInfGame::GetButtonArray()
{
    return &m_cButtonArray;
}

// 0x452FE0
LONG CInfGame::GetCharacterId(SHORT nPortrait)
{
    if (nPortrait < m_nCharacters) {
        return m_characterPortraits[nPortrait];
    }

    return CGameObjectArray::INVALID_INDEX;
}

// 0x453010
CTimerWorld* CInfGame::GetWorldTimer()
{
    return &m_worldTime;
}

// 0x453020
SHORT CInfGame::GetNumCharacters()
{
    return m_nCharacters;
}

// 0x453030
CGameObjectArray* CInfGame::GetObjectArray()
{
    return &m_cObjectArray;
}

// 0x453040
CGameArea* CInfGame::GetVisibleArea()
{
    return m_gameAreas[m_visibleArea];
}

// 0x453050
CGameSave* CInfGame::GetGameSave()
{
    return &m_gameSave;
}

// 0x453060
CMultiplayerSettings* CInfGame::GetMultiplayerSettings()
{
    return &m_multiplayerSettings;
}

// 0x439A70
CGameJournal* CInfGame::GetJournal()
{
    return &m_cJournal;
}

// 0x475DB0
CAIGroup* CInfGame::GetGroup()
{
    return &m_group;
}

// 0x4286A0
CVariableHash* CInfGame::GetVariables()
{
    return &m_variables;
}

// 0x6E4E80
CVidBitmap* CInfGame::GetMasterBitmap()
{
    return &m_rgbMasterBitmap;
}

// NOTE: Inlined.
LONG CInfGame::GetFixedOrderCharacterId(SHORT nSlot)
{
    if (nSlot < 6) {
        return m_characters[nSlot];
    }

    return CGameObjectArray::INVALID_INDEX;
}

// 0x40CAE0
CNamedCreatureVariableHash* CInfGame::GetNamedCreatures()
{
    return &m_namedCreatures;
}

// 0x68C010
void CInfGame::SetTempCursor(BYTE tempCursor)
{
    m_tempCursor = tempCursor;
}

// 0x45B630
CGameOptions* CInfGame::GetOptions()
{
    return &m_cOptions;
}

// 0x45B640
SHORT CInfGame::GetReputation()
{
    return m_nReputation / REPUTATION_MULTIPLIER;
}

// 0x45B660
CTypedPtrArray<CPtrArray, int*>& CInfGame::GetCharacterOverflow()
{
    return m_characterOverflow;
}

// 0x594070
SHORT CInfGame::GetState()
{
    return m_nState;
}

// 0x594080
void CInfGame::SetState(SHORT nState)
{
    m_nState = nState;
}

// 0x594090
BYTE CInfGame::GetIconIndex()
{
    return m_iconIndex;
}

// 0x5940A0
void CInfGame::SetIconIndex(BYTE iconIndex)
{
    m_iconIndex = iconIndex;
    m_iconResRef = "";
}

// 0x58FEC0
CSpellResRefList& CInfGame::GetMasterSongLookup()
{
    return m_songs;
}

// 0x5F67F0
CSpellResRefList* CInfGame::GetInnateSpells()
{
    return &m_innateSpells;
}

// 0x5F6800
CSpellResRefList* CInfGame::GetShapeshifts()
{
    return &m_innateSpells;
}

// 0x64D660
CString CInfGame::GetSaveGame()
{
    return m_sSaveGame;
}

// 0x71AB80
void CInfGame::SetLastTarget(LONG id)
{
    m_lastClick.x = -1;
    m_lastClick.y = -1;
    m_lastTarget = id;
}

// NOTE: Inlined.
void CInfGame::SetLastClick(const CPoint& pt)
{
    m_lastClick = pt;
    m_lastTarget = CGameObjectArray::INVALID_INDEX;
}

// NOTE: Inlined.
BYTE CInfGame::GetKeymap(SHORT index)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1486
    UTIL_ASSERT(index >= 0 && index < CINFGAME_KEYMAP_SIZE);

    return m_pKeymap[index];
}

// NOTE: Inlined.
BYTE CInfGame::GetKeymapFlag(SHORT index)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1487
    UTIL_ASSERT(index >= 0 && index < CINFGAME_KEYMAP_SIZE);

    return m_pKeymapFlags[index];
}

// NOTE: Inlined
void CInfGame::SetQuickFormation(SHORT buttonNum, SHORT formation)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1499
    UTIL_ASSERT(buttonNum < CGAMESAVE_NUM_QUICK_FORMATIONS);

    m_gameSave.m_quickFormations[buttonNum] = formation;
}

// NOTE: Inlined.
void CInfGame::SetFormationToQuickFormation(SHORT buttonNum)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1500
    UTIL_ASSERT(buttonNum < CGAMESAVE_NUM_QUICK_FORMATIONS);

    m_gameSave.m_curFormation = m_gameSave.m_quickFormations[buttonNum];
}

// NOTE: Inlined.
void CInfGame::SetFormation(SHORT formation)
{
    m_gameSave.m_curFormation = formation;
}

// NOTE: Inlined.
BOOL CInfGame::IsFamiliar(LONG id)
{
    return m_familiars.Find(reinterpret_cast<int*>(id)) != NULL;
}

// NOTE: Inlined.
BOOL CInfGame::IsAlly(LONG id)
{
    return m_allies.Find(reinterpret_cast<int*>(id)) != NULL;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CSpellResRefList::CSpellResRefList()
{
    m_pList = NULL;
    m_nCount = 0;
}

// NOTE: Inlined.
CSpellResRefList::~CSpellResRefList()
{
    if (m_pList != NULL) {
        delete m_pList;
    }
    m_nCount = 0;
}

// FIXME: `nResRefColumn` should not be reference.
//
// 0x5C9FD0
void CSpellResRefList::Load(const C2DArray& Table, const INT& nResRefColumn)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23354
    UTIL_ASSERT(Table.GetWidth() > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23355
    UTIL_ASSERT(Table.GetHeight() > 0);

    // FIXME: Uses unsigned compare.
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23356
    UTIL_ASSERT(nResRefColumn < Table.GetWidth());

    if (m_pList != NULL) {
        delete m_pList;
        m_nCount = 0;
    }

    m_pList = new CResRef[Table.GetHeight()];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23369
    UTIL_ASSERT(m_pList != NULL);

    for (int index = 0; index < Table.GetHeight(); index++) {
        m_pList[index] = Table.GetAt(CPoint(nResRefColumn, index));
    }

    m_nCount = Table.GetHeight();
}

// 0x5CA160
bool CSpellResRefList::Find(const CResRef& resRef, UINT& nID)
{
    for (UINT nIndex = 0; nIndex < m_nCount; nIndex++) {
        if (m_pList[nIndex] == resRef) {
            nID = nIndex;
            return true;
        }
    }

    nID = 0;
    return false;
}

// NOTE: Inlined. Unclear why it's different from the one below.
const CResRef& CSpellResRefList::Get636(UINT nIndex)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 636
    UTIL_ASSERT(nIndex < m_nCount);

    // __FILE__: .\Include\InfGame.h
    // __LINE__: 637
    UTIL_ASSERT(m_pList != NULL);

    return m_pList[nIndex];
}

// FIXME: `nIndex` should not be reference.
// TODO: Is it `operator[]`?
//
// 0x5F6790
const CResRef& CSpellResRefList::Get(UINT& nIndex)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 652
    UTIL_ASSERT(nIndex < m_nCount);

    // __FILE__: .\Include\InfGame.h
    // __LINE__: 653
    UTIL_ASSERT(m_pList != NULL);

    return m_pList[nIndex];
}

// -----------------------------------------------------------------------------

// 0x59EB90
CSpellList::CSpellList()
{
    m_pList = NULL;
    m_nCount = 0;
}

// 0x59EBA0
CSpellList::~CSpellList()
{
    if (m_pList != NULL) {
        delete m_pList;
    }
    m_nCount = 0;
}

// 0x5CA1B0
void CSpellList::Load(const std::vector<CSpellListEntry>& spells)
{
    if (spells.empty()) {
        return;
    }

    size_t size = spells.size();
    if (size == 0) {
        return;
    }

    m_pList = new CSpellListEntry[size];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23430
    UTIL_ASSERT(m_pList != NULL);

    for (size_t index = 0; index < size; index++) {
        m_pList[index] = spells[index];
    }

    m_nCount = size;
}

// 0x5CA250
bool CSpellList::GetSpellLevel(const UINT& nID, UINT& nLevel)
{
    for (UINT nIndex = 0; nIndex < m_nCount; nIndex++) {
        if (m_pList[nIndex].m_nID == nID) {
            nLevel = m_pList[nIndex].m_nLevel;
            return true;
        }
    }

    nLevel = 0;
    return false;
}

// TODO: Is it `operator[]`?
//
// NOTE: Inlined.
UINT CSpellList::Get(UINT nIndex)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 724
    UTIL_ASSERT(nIndex < m_nCount);

    // __FILE__: .\Include\InfGame.h
    // __LINE__: 725
    UTIL_ASSERT(m_pList != NULL);

    return m_pList[nIndex].m_nID;
}

// -----------------------------------------------------------------------------

// 0x59EB60
CGroupedSpellList::CGroupedSpellList()
{
    m_nHighestLevel = 0;
}

// 0x59ECA0
CGroupedSpellList::~CGroupedSpellList()
{
}

// FIXME: `nClassColumn` should not be reference.
// FIXME: `nResRefColumn` should not be reference.
//
// 0x5CA2A0
void CGroupedSpellList::Load(const C2DArray& Table, const INT& nClassColumn, const INT& nResRefColumn, CSpellResRefList* pSpells)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23495
    UTIL_ASSERT(Table.GetWidth() > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23496
    UTIL_ASSERT(Table.GetHeight() > 0);

    // FIXME: Uses unsigned compare.
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23497
    UTIL_ASSERT(nClassColumn < Table.GetWidth());

    // FIXME: Uses unsigned compare.
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23498
    UTIL_ASSERT(nResRefColumn < Table.GetWidth());

    std::vector<CSpellListEntry> lists[9];

    for (int index = 0; index < Table.GetHeight(); index++) {
        int nLevel = atol(Table.GetAt(CPoint(nClassColumn, index)));
        UINT nID = index;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 23509
        UTIL_ASSERT(nLevel <= CSPELLLIST_MAX_LEVELS);

        CString sName = Table.GetAt(CPoint(nResRefColumn, index));
        CResRef resRef(sName);

        sName = sName.Left(strlen("**"));
        if (nLevel > 0 && sName != "**") {
            if (pSpells != NULL) {
                bool result = pSpells->Find(resRef, nID);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 23524
                UTIL_ASSERT(result == true);
            }

            CSpellListEntry entry;
            entry.m_nID = nID;
            entry.m_nLevel = nLevel;
            lists[nLevel - 1].push_back(entry);
        }
    }

    // NOTE: Unsigned compare, implying `m_nHighestLevel` is also unsigned.
    for (UINT nLevel = 1; nLevel <= CSPELLLIST_MAX_LEVELS; nLevel++) {
        if (lists[nLevel - 1].size() != 0) {
            m_lists[nLevel - 1].Load(lists[nLevel - 1]);

            if (nLevel > m_nHighestLevel) {
                m_nHighestLevel = nLevel;
            }
        }
    }
}

// -----------------------------------------------------------------------------

// 0x5C7AC0
BYTE* CGameFile::GetData()
{
    if (pRes != NULL) {
        return reinterpret_cast<BYTE*>(pRes->Demand());
    } else {
        return NULL;
    }
}

// 0x5C7AD0
DWORD CGameFile::GetDataSize()
{
    if (pRes == NULL) {
        return 0;
    }

    if (pRes->Demand() == NULL) {
        return 0;
    }

    DWORD nSize = pRes->m_nResSizeActual;

    pRes->Release();

    return nSize;
}
