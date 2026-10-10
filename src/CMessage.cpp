#include "CMessage.h"

#include "CBaldurChitin.h"
#include "CGameAIBase.h"
#include "CGameArea.h"
#include "CProjectile.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGameEffect.h"
#include "CGameSound.h"
#include "CGameSpawning.h"
#include "CGameSprite.h"
#include "CGameStatic.h"
#include "CGameTrigger.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CItem.h"
#include "CPathSearch.h"
#include "CScreenChapter.h"
#include "CScreenCharacter.h"
#include "CScreenConnection.h"
#include "CScreenCreateChar.h"
#include "CScreenLoad.h"
#include "CScreenInventory.h"
#include "CScreenMultiPlayer.h"
#include "CScreenSinglePlayer.h"
#include "CScreenStore.h"
#include "CScreenWorld.h"
#include "CStore.h"
#include "CUIPanel.h"
#include "CUtil.h"

// NOTE: Might be static in `CMessage`.
//
// 0x8ABE0C
static const DWORD STATICBUFFERSIZE = 0x4000;

// NOTE: Actual size might be bigger, but `STATICBUFFERSIZE` says it is at
// leats 0x4000 bytes.
//
// 0x8D8FE4
static BYTE STATICBUFFER[0x4000];

static inline BYTE* CreateBuffer(DWORD len)
{
    return len > STATICBUFFERSIZE ? new BYTE[len] : STATICBUFFER;
}

static inline void DestroyBuffer(BYTE* pBuffer)
{
    if (pBuffer != STATICBUFFER) {
        delete pBuffer;
    }
}

// 0x848208
const SHORT CMessage::SEND = 1;

// 0x84820A
const SHORT CMessage::BROADCAST = 2;

// 0x84820C
const SHORT CMessage::BROADCAST_OTHERS = 3;

// 0x84820E
const SHORT CMessage::BROADCAST_FORCED = 4;

// 0x848210
const SHORT CMessage::BROADCAST_FORCED_OTHERS = 5;

// 0x84CED4
const BYTE CBaldurMessage::MSG_TYPE_PROGRESSBAR = 66;

// 0x84CED5
const BYTE CBaldurMessage::MSG_SUBTYPE_PROGRESSBAR_STATUS = 83;

// 0x84CED6
const BYTE CBaldurMessage::MSG_TYPE_CMESSAGE = 67;

// 0x84CED7
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_ACTION = 0;

// 0x84CED8
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_EFFECT = 1;

// 0x84CEDA
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ANIMATION_CHANGE = 3;

// 0x84CEDB
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CHANGE_DIRECTION = 4;

// 0x84CEDC
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_ACTIONS = 5;

// 0x84CEDD
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_DIALOG_ACTIONS = 6;

// 0x84CEDE
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_GROUP_SLOT = 7;

// 0x84CEDF
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_TRIGGERS = 8;

// 0x84CEE0
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_CHANGE = 9;

// 0x84CEE1
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_RESET = 10;

// 0x84CEE2
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_UPDATE = 11;

// 0x84CEE3
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_ADD_ITEM = 12;

// 0x84CEE4
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_ITEMS = 13;

// 0x84CEE5
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_STATUS = 14;

// 0x84CEE6
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CUT_SCENE_MODE_STATUS = 15;

// 0x84CEE7
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXT = 16;

// 0x84CEE8
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXTREF = 17;

// 0x84CEEA
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_STATUS = 19;
// 0x84CEE9
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_OPEN = 18;

// 0x84CEEB
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DROP_PATH = 20;

// 0x84CEEC
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ENTER_DIALOG = 21;

// 0x84CEEE
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ENTER_STORE_MODE = 23;

// 0x84CEEF
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_EXIT_DIALOG_MODE = 24;

// 0x84CEF0
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_EXIT_STORE_MODE = 25;

// 0x84CEF1
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FIRE_PROJECTILE = 26;

// 0x84CEF2
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_INSERT_ACTION = 27;

// 0x84CEF3
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_INSERT_RESPONSE = 28;

// 0x84CF48
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTINUE_DIALOG = 113;

// 0x84CEF4
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_LEAVE_PARTY = 29;

// 0x84CEF5
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_LOAD_DIALOG = 30;

// 0x84CEF6
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PARTY_GOLD = 31;

// 0x84CEF7
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PLAY_SOUND = 32;

// 0x84CEF8
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PLAY_SOUND_REF = 33;

// 0x84CEFA
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_REMOVE_REPLIES = 35;

// 0x84CEFB
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_REPUTATION_CHANGE = 36;

// 0x84CEFC
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_ACTIVE = 37;

// 0x84CEFD
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AISPEED = 38;

// 0x84CEFE
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_COMMAND_PAUSE = 39;

// 0x84CEFF
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DIALOG_WAIT = 40;

// 0x84CF00
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DIRECTION = 41;

// 0x84CF01
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DRAW_POLY = 42;

// 0x84CF02
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_FORCE_ACTION_PICK = 43;

// 0x84CF03
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_HAPPINESS = 44;

// 0x84CF04
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_IN_CUT_SCENE = 45;

// 0x84CF05
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_LAST_ATTACKER = 46;

// 0x84CF07
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_NUM_TIMES_TALKED_TO = 48;

// 0x84CF08
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_PATH = 49;

// 0x84CF09
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_SEQUENCE = 50;

// 0x84CF0B
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_TRIGGER = 52;

// 0x84CF0C
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_VARIABLE = 53;

// 0x84CF06
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_LAST_OBJECT = 47;

// 0x84CF0D
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_DEATH = 54;

// 0x84CF0E
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_EQUIPMENT = 55;

// 0x84CF0F
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_PETRIFY = 56;

// 0x84CF10
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_UPDATE = 57;

// 0x84CF11
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_FOLLOW = 58;

// 0x84CF12
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_SCROLL = 59;

// 0x84CF11
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_ACTIONS = 60;

// 0x84CF12
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_FOLLOW = 61;

// 0x84CF15
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TRIGGER_STATUS = 62;

// 0x84CF16
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_UNLOCK = 63;

// 0x84CF17
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_UPDATE_REACTION = 64;

// 0x84CF18
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VERBAL_CONSTANT = 65;

// 0x84CF19
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VISIBILITY_MAP_MOVE = 66;

// 0x84CF1A
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VISUAL_EFFECT = 67;

// 0x84CF1B
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DIALOG_RESREF = 68;

// 0x84CF1C
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ESCAPE_AREA = 69;

// 0x84CF1D
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXTREF_SEND = 70;

// 0x84CF1E
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_CURRENT_ACTION_ID = 71;

// 0x84CF1F
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_MOVE_GLOBAL = 72;

// 0x84CF20
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FADE_COLOR = 73;

// 0x84CF21
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_TEXT_SCREEN = 74;

// 0x84CF22
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPAWNPT_ACTIVATE = 75;

// 0x84CF23
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPAWNPT_SPAWN = 76;

// 0x84CF24
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STATIC_START = 77;

// 0x84CF25
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_ADD_ITEM = 78;

// 0x84CF26
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_REMOVE_ITEM = 79;

// 0x84CF27
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FAMILIAR_ADD = 80;

// 0x84CF28
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FAMILIAR_REMOVE_RESREF = 81;

// 0x84CF29
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_ESCAPE_AREA = 82;

// 0x84CF2C
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_TIME_STOP = 85;

// 0x84CF2D
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PROJECTILE_TRAILING_VFX = 86;

// 0x84CF2E
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_RELEASE = 87;

// 0x84CF31
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_90 = 90;

// 0x84CF32
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FLOAT_TEXT = 91;

// 0x84CF33
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_92 = 92;

// 0x84CF34
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_PROTAGONIST = 93;

// 0x84CF35
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_COMBAT_MUSIC = 94;

// 0x84CF3A
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SCREENSHAKE = 99;

// 0x84CF3B
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_DEMAND = 100;

// 0x84CF3C
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_101 = 101;

// 0x84CF3D
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_WEAPON_IMMUNITIES_UPDATE = 102;

// 0x84CF3E
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_103 = 103;

// 0x84CF3F
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPELL_LEVEL_IMMUNITIES_UPDATE = 104;

// 0x84CF40
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_105 = 105;

// 0x84CF41
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TOGGLE_INTERFACE = 106;

// 0x84CF42
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_107 = 107;

// 0x84CF44
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_TYPE = 109;

// 0x84CF45
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_REST_ENCOUNTER = 110;

// 0x84CF46
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SAVE_GAME = 111;

// 0x84CF49
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_EXPLORED = 114;

// 0x84CF4B
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CHANGE_STAT = 116;

// 0x84CF4F
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_120 = 120;

// 0x84CF50
const BYTE CBaldurMessage::MSG_SUBTYPE_CMESSAGE_END_GAME = 121;

// 0x84CF51
const BYTE CBaldurMessage::MSG_TYPE_DIALOG = 68;

// 0x84CF52
const BYTE CBaldurMessage::MSG_SUBTYPE_DIALOG_PERMIT_REQUEST = 82;

// 0x84CF53
const BYTE CBaldurMessage::MSG_SUBTYPE_DIALOG_PERMIT_REPLY = 114;

// 0x84CF54
const BYTE CBaldurMessage::MSG_SUBTYPE_DIALOG_CANCEL_REQUEST = 67;

// 0x84CF55
const BYTE CBaldurMessage::MSG_SUBTYPE_DIALOG_KILL_OR_USE = 75;

// 0x84CF56
const BYTE CBaldurMessage::DIALOG_REQUESTPENDING = 2;

// 0x84CF57
const BYTE CBaldurMessage::DIALOG_NOT_PERMITTED = 3;

// 0x84CF58
const BYTE CBaldurMessage::MSG_TYPE_SWAPITEM = 73;

// 0x84CF59
const BYTE CBaldurMessage::MSG_SUBTYPE_SWAPITEM_REQUEST = 82;

// 0x84CF5A
const BYTE CBaldurMessage::MSG_SUBTYPE_SWAPITEM_REPLY = 114;

// 0x84CF5E
const BYTE CBaldurMessage::MSG_TYPE_CHAPTER = 74;

// 0x84CF60
const BYTE CBaldurMessage::MSG_SUBTYPE_CHAPTER_ANNOUNCE_STATUS = 97;

// 0x84CF64
const BYTE CBaldurMessage::MSG_TYPE_JOURNAL = 106;

// 0x84CF65
const BYTE CBaldurMessage::MSG_SUBTYPE_JOURNAL_ADD_ENTRY = 69;

// 0x84CF66
const BYTE CBaldurMessage::MSG_SUBTYPE_JOURNAL_ANNOUNCE = 65;

// 0x84CF67
const BYTE CBaldurMessage::MSG_SUBTYPE_JOURNAL_ADD_USER_ENTRY = 117;

// 0x84CF68
const BYTE CBaldurMessage::MSG_SUBTYPE_JOURNAL_ANNOUNCE_USER_ENTRY = 85;

// 0x84CF69
const BYTE CBaldurMessage::MSG_SUBTYPE_JOURNAL_CHANGE_ENTRY = 99;

// 0x84CF6A
const BYTE CBaldurMessage::MSG_SUBTYPE_JOURNAL_ANNOUNCE_CHANGE = 67;

// 0x84CF6B
const BYTE CBaldurMessage::MSG_TYPE_BIOGRAPHY = 98;

// 0x84CF6C
const BYTE CBaldurMessage::MSG_SUBTYPE_BIOGRAPHY_CHANGE = 99;

// 0x84CF6D
const BYTE CBaldurMessage::MSG_SUBTYPE_BIOGRAPHY_CHANGE_ANNOUNCE = 67;

// 0x84CF6E
const BYTE CBaldurMessage::MSG_TYPE_KICK_PLAYER = 75;

// 0x84CF6F
const BYTE CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_REQUEST = 82;

// 0x84CF6F
const BYTE CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_SERVER_SUPPORT = 83;

// 0x84CF6F
const BYTE CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_HOOFED_OUT = 72;

// 0x84CF76
const BYTE CBaldurMessage::DELETEAREA_EMPTY_VOTE = 101;

// 0x84CF77
const BYTE CBaldurMessage::DELETEAREA_PENDING_VOTE = 112;

// 0x84CF78
const BYTE CBaldurMessage::DELETEAREA_YES_VOTE = 121;

// 0x84CF79
const BYTE CBaldurMessage::DELETEAREA_NO_VOTE = 110;

// 0x84CF7A
const BYTE CBaldurMessage::MSG_TYPE_MPSETTINGS = 77;

// 0x84CF7B
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_FULLDEMAND = 68;

// 0x84CF7C
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_FULLSET = 83;

// 0x84CF7D
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_PERMISSION = 112;

// 0x84CF7E
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_PLAYER_READY = 121;

// 0x84CF7F
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_CHAR_READY = 114;

// 0x84CF80
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_CHAR_CONTROL = 99;

// 0x84CF81
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_IMPORTING = 105;

// 0x84CF82
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_LISTEN_JOIN = 106;

// 0x84CF83
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_SLOT_STATUS = 115;

// 0x84CF84
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_LOCK_STATUS = 108;

// 0x84CF85
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_LOCK_ALLOW_INPUT = 97;

// 0x84CF86
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_LOCK_REQUEST = 76;

// 0x84CF87
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_GORE_LEVEL = 54;

// 0x84CF88
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_RESTRICT_STORE = 57;

// 0x84CF89
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_NIGHTMAREMODE = 78;

// 0x84CF8A
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSETTINGS_DEMAND_NIGHTMAREMODE = 110;

// 0x84CF98
const BYTE CBaldurMessage::MSG_TYPE_OBJECT = 79;

// 0x84CF99
const BYTE CBaldurMessage::MSG_SUBTYPE_OBJECT_ADD = 65;

// 0x84CF9F
const BYTE CBaldurMessage::MSG_TYPE_PLAYERCHAR = 80;

// 0x84CFA0
const BYTE CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_UPDATE_DEMAND = 85;

// 0x84CFA1
const BYTE CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_UPDATE_REPLY = 117;

// 0x84CFA2
const BYTE CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_DEMAND_SLOT = 68;

// 0x84CFA3
const BYTE CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_DEMAND_REPLY = 100;

// 0x84CFA4
const BYTE CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_70 = 70;

// 0x84CFA5
const BYTE CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_102 = 102;

// 0x84CFAA
const BYTE CBaldurMessage::MSG_TYPE_PAUSING = 81;

// 0x84CFAB
const BYTE CBaldurMessage::MSG_SUBTYPE_PAUSING_PERMIT_REQUEST = 82;

// 0x84CFAC
const BYTE CBaldurMessage::MSG_SUBTYPE_PAUSING_ANNOUNCE = 65;

// 0x84CFB0
const BYTE CBaldurMessage::MSG_TYPE_RESOURCE = 82;

// 0x84CFB1
const BYTE CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND = 68;

// 0x84CFB8
const BYTE CBaldurMessage::MSG_TYPE_SIGNAL = 83;

// 0x84CFB9
const BYTE CBaldurMessage::MSG_SUBTYPE_SIGNAL = 83;

// 0x84CFBA
const BYTE CBaldurMessage::MSG_SUBTYPE_SIGNAL_REQUEST = 82;

// 0x84CFBB
const BYTE CBaldurMessage::SIGNAL_ALL_CLIENTS = 67;

// 0x84CFBC
const BYTE CBaldurMessage::SIGNAL_SERVER = 83;

// 0x84CFBD
const BYTE CBaldurMessage::SIGNAL_END_MAJOR_EVENT = 49;

// 0x84CFC2
const BYTE CBaldurMessage::SIGNAL_END_GAME = 54;

// 0x84CFC3
const BYTE CBaldurMessage::SIGNAL_MPSYNCH = 53;

// 0x84CFC4
const DWORD CBaldurMessage::SIGNAL_SECONDSTOMPTIMEOUT = 200;

// 0x84CFC8
const BYTE CBaldurMessage::MSG_TYPE_MPSYNCH = 115;

// 0x84CFC9
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSYNCH_REQUEST = 82;

// 0x84CFCA
const BYTE CBaldurMessage::MSG_SUBTYPE_MPSYNCH_REPLY = 80;

// 0x84CFCE
const BYTE CBaldurMessage::MSG_TYPE_VERSION = 86;

// 0x84CFCF
const BYTE CBaldurMessage::MSG_SUBTYPE_VERSION_SERVER = 115;

// 0x84CFD0
const BYTE CBaldurMessage::VERSION_NUM_FIELDS = 3;

// 0x84CFD3
const BYTE CBaldurMessage::MSG_TYPE_LEAVEAREALUA = 88;

// 0x84CFD4
const BYTE CBaldurMessage::MSG_SUBTYPE_LEAVEAREALUA_PERMIT_REQUEST = 82;

// 0x84CFDB
const BYTE CBaldurMessage::MSG_TYPE_LEAVEAREANAME = 120;

// 0x84CFDC
const BYTE CBaldurMessage::MSG_SUBTYPE_LEAVEAREANAME_PERMIT_REQUEST = 82;

// 0x4294F0
CBaldurMessage::CBaldurMessage()
{
    m_bInHandleBlockingMessages = FALSE;
    m_nVersionControlShutdownReason = 0;
    m_bInOnObjectAdd = FALSE;
    m_bInOnObjectDelete = FALSE;
    m_bInOnRestAnnounce = FALSE;
    m_bInOnJournalAnnounce = FALSE;
    m_bInOnAnnounceBiography = FALSE;
    m_bInOnAreaMapNoteAnnounce = FALSE;
    m_bDialogRequestPending = FALSE;
    m_bDialogReplyReturned = FALSE;
    m_bDialogReplyValue = FALSE;
    m_nDialogReplyUpdates = 0;
    m_bLeaveAreaLuaRequestPending = FALSE;
    m_bLeaveAreaLuaReplyReturned = FALSE;
    m_nLeaveAreaLuaReplyValue = 0;
    m_bLeaveAreaNameRequestPending = FALSE;
    m_bLeaveAreaNameReplyReturned = FALSE;
    m_nLeaveAreaNameReplyValue = 0;
    m_sDeleteAreaString = "";
    m_bDeleteAreaPolling = FALSE;
    m_dwDeleteAreaTimeout = 0;

    for (int k = 0; k < 6; k++) {
        m_pnDeleteAreaVoters[k] = 0;
        m_pnDeleteAreaVotes[k] = DELETEAREA_EMPTY_VOTE;
    }

    m_bMultiplayerSynchClientPending = FALSE;
    m_bMultiplayerSynchClientFinished = FALSE;
    m_bMultiplayerSynchServerPending = FALSE;
    m_bMultiplayerSynchServerFinished = FALSE;
    m_nMultiplayerSynchClientLocation = -1;
    m_nSignalQueueStart = 0;
    m_nSignalQueueEnd = 0;
    m_nSignalQueueSize = 24;

    for (BYTE l = 0; l < m_nSignalQueueSize; l++) {
        m_pnSignalFrom[l] = -1;
        m_pnSignalData[l] = 0;
        m_pnSignalType[l] = 0;
    }

    m_bRemovedPlayerID = 0;

    for (int m = 0; m < 6; m++) {
        m_pRemovedPlayerID[m] = 0;
    }

    m_bVersionControlShutdown = FALSE;
    m_dwSignalSecondsToTimeout = SIGNAL_SECONDSTOMPTIMEOUT;
    m_bPlayerShutdown = FALSE;
    m_bMultiplayerSessionShutdown = FALSE;
    m_bInReputationChange = FALSE;
    m_bInMessageSetDrawPoly = FALSE;
}

// 0x429720
CBaldurMessage::~CBaldurMessage()
{
}

// 0x4297C0
BOOLEAN CBaldurMessage::DemandResourceFromServer(const CString& sResName, UINT nType, BOOLEAN bSendMessage, BOOLEAN bWaitForMessage, BOOLEAN bSaveToTemp)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x429A90
BOOLEAN CBaldurMessage::OnResourceDemanded(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x42AA10
BOOLEAN CBaldurMessage::ResourceSuggestLoad(const CString& sResName, UINT nType, DWORD dwFlags)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x42B1F0
BOOL CBaldurMessage::OnSettingsFullDemanded(INT nMsgFrom)
{
    CString sPlayerName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetPlayerName(nMsgFrom, sPlayerName);
    SendFullSettingsToClients(sPlayerName);

    return TRUE;
}

// 0x42B2B0
BOOL CBaldurMessage::SendFullSettingsToClients(const CString& sPlayerName)
{
    BYTE* pMessage;
    DWORD dwSize;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    CMultiplayerSettings* pSettings = g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings();

    dwSize = pSettings->GetDataSize();
    pSettings->Marshal(&pMessage, &dwSize);

    if (sPlayerName != "") {
        g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
            CNetwork::SEND_GUARANTEED,
            MSG_TYPE_MPSETTINGS,
            MSG_SUBTYPE_MPSETTINGS_FULLSET,
            pMessage,
            dwSize);
    } else {
        g_pChitin->cNetwork.SendSpecificMessage(CString(""),
            CNetwork::SEND_ALL_PLAYERS | CNetwork::SEND_GUARANTEED | CNetwork::SEND_JOINING_PLAYERS,
            MSG_TYPE_MPSETTINGS,
            MSG_SUBTYPE_MPSETTINGS_FULLSET,
            pMessage,
            dwSize);
    }

    delete pMessage;

    return TRUE;
}

// 0x42B420
BOOL CBaldurMessage::OnSettingsFullSet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
        return FALSE;
    }

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->Unmarshal(pMessage, dwSize);

    return TRUE;
}

// 0x42B490
BOOL CBaldurMessage::SendPermissionToServer(const CString& sPlayerName, INT nPermission, BOOLEAN bValue)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    PLAYER_ID playerID;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    playerID = g_pChitin->cNetwork.FindPlayerIDByName(sPlayerName, FALSE);

    dwSize = sizeof(PLAYER_ID)
        + sizeof(INT)
        + sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<PLAYER_ID*>(pMessage + nMsgPtr) = playerID;
    nMsgPtr += sizeof(PLAYER_ID);

    *reinterpret_cast<INT*>(pMessage + nMsgPtr) = nPermission;
    nMsgPtr += sizeof(INT);

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bValue;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_PERMISSION,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42B5E0
BOOL CBaldurMessage::OnSettingsPermissionSet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    PLAYER_ID playerID;
    INT nPlayerToFind;
    INT nPermission;
    BOOLEAN bValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
            return FALSE;
        }
    } else {
        if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
            return FALSE;
        }
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(PLAYER_ID) + sizeof(INT) + sizeof(BOOLEAN)) {
        return FALSE;
    }

    playerID = *reinterpret_cast<PLAYER_ID*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(PLAYER_ID);

    nPermission = *reinterpret_cast<INT*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(INT);

    bValue = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    if (playerID != 0) {
        nPlayerToFind = g_pChitin->cNetwork.FindPlayerLocationByID(playerID, FALSE);
    } else {
        nPlayerToFind = -1;
    }

    if (nPlayerToFind == g_pChitin->cNetwork.m_idLocalPlayer) {
        g_pBaldurChitin->GetObjectGame()->m_singlePlayerPermissions.SetSinglePermission(nPermission, bValue);
    }

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetPermission(nPlayerToFind,
        nPermission,
        bValue,
        TRUE);

    return TRUE;
}

// 0x42B700
BOOLEAN CBaldurMessage::SendPlayerReadyToServer(PLAYER_ID idPlayer, BOOLEAN bValue)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(PLAYER_ID) + sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<PLAYER_ID*>(pMessage + nMsgPtr) = idPlayer;
    nMsgPtr += sizeof(PLAYER_ID);

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bValue;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_PLAYER_READY,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42B850
BOOLEAN CBaldurMessage::OnSettingsPlayerReadySet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    PLAYER_ID playerID;
    BOOLEAN bValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(PLAYER_ID) + sizeof(BOOLEAN)) {
        return FALSE;
    }

    playerID = *reinterpret_cast<PLAYER_ID*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(PLAYER_ID);

    bValue = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetPlayerReady(playerID,
        bValue,
        TRUE);

    return TRUE;
}

// 0x42B8B0
BOOL CBaldurMessage::SendCharacterControlToServer(CString& sPlayerName, INT nCharacterSlot, BOOLEAN bFlush, BOOLEAN bCharacterUpdate)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    PLAYER_ID playerID;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    if (sPlayerName != "") {
        playerID = g_pChitin->cNetwork.FindPlayerIDByName(sPlayerName, FALSE);
    } else {
        playerID = 0;
    }

    dwSize = sizeof(INT)
        + sizeof(PLAYER_ID)
        + sizeof(BOOLEAN)
        + sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<INT*>(pMessage + nMsgPtr) = nCharacterSlot;
    nMsgPtr += sizeof(INT);

    *reinterpret_cast<PLAYER_ID*>(pMessage + nMsgPtr) = playerID;
    nMsgPtr += sizeof(PLAYER_ID);

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bFlush;
    nMsgPtr += sizeof(BOOLEAN);

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bCharacterUpdate;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_CHAR_CONTROL,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42BA40
BOOL CBaldurMessage::OnSettingsCharacterControlSet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    PLAYER_ID playerID;
    INT nPlayerToFind;
    INT nCharacterSlot;
    BOOLEAN bFlush;
    BOOLEAN bCharacterUpdate;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
            return FALSE;
        } else {
            if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
                return FALSE;
            }
        }
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(INT) + sizeof(PLAYER_ID) + sizeof(BOOLEAN) + sizeof(BOOLEAN)) {
        return FALSE;
    }

    nCharacterSlot = *reinterpret_cast<INT*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(INT);

    playerID = *reinterpret_cast<PLAYER_ID*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(PLAYER_ID);

    bFlush = *reinterpret_cast<INT*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(INT);

    bCharacterUpdate = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    nPlayerToFind = g_pChitin->cNetwork.FindPlayerLocationByID(playerID, FALSE);
    if (playerID != 0 && nPlayerToFind != -1) {
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetCharacterControlledByPlayer(nCharacterSlot,
            nPlayerToFind,
            bFlush,
            bCharacterUpdate);
    } else {
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetCharacterControlledByPlayer(nCharacterSlot,
            -1,
            bFlush,
            bCharacterUpdate);
    }

    SendFullSettingsToClients(CString(""));

    return TRUE;
}

// 0x42BC00
BOOL CBaldurMessage::SendCharacterReadyToServer(INT nCharacterSlot, BOOLEAN bCharacterReady)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(INT) + sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<INT*>(pMessage + nMsgPtr) = nCharacterSlot;
    nMsgPtr += sizeof(INT);

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bCharacterReady;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_CHAR_READY,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42BD50
BOOL CBaldurMessage::OnSettingsCharacterReadySet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    INT nCharacterSlot;
    BOOLEAN bValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (!g_pChitin->cNetwork.GetSessionHosting()) {
        if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
            return FALSE;
        }
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(INT) + sizeof(BOOLEAN)) {
        return FALSE;
    }

    nCharacterSlot = *reinterpret_cast<INT*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(INT);

    bValue = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetCharacterControlledByPlayer(nCharacterSlot) != nMsgFrom) {
        return FALSE;
    }

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetCharacterReady(nCharacterSlot,
        bValue,
        TRUE);

    SendFullSettingsToClients(CString(""));

    return TRUE;
}

// 0x42BEB0
BOOL CBaldurMessage::SendGoreRequirementToServer(DWORD nGore, DWORD nGoreOption)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(DWORD) + sizeof(DWORD);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<DWORD*>(pMessage + nMsgPtr) = nGore;
    nMsgPtr += sizeof(DWORD);

    *reinterpret_cast<DWORD*>(pMessage + nMsgPtr) = nGoreOption;
    nMsgPtr += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_GORE_LEVEL,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42C000
BOOL CBaldurMessage::OnSettingsGoreRequirement(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    DWORD nMsgPtr;
    DWORD nClientGore;
    DWORD nClientGoreOption;
    BOOLEAN bModifiedGore;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nClientGore = *reinterpret_cast<DWORD*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(DWORD);

    nClientGoreOption = *reinterpret_cast<DWORD*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    DWORD nServerGoreOption = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nGoreOption;
    DWORD nServerGore = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nGore;

    if (nClientGoreOption < nServerGoreOption) {
        g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nGoreOption = nClientGoreOption;
        bModifiedGore = TRUE;
    }

    if (nClientGore < nServerGore) {
        g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nGore = nClientGore;
        bModifiedGore = TRUE;
    }

    if (bModifiedGore) {
        SendFullSettingsToClients(CString(""));
    }

    return TRUE;
}

// 0x42C0E0
BOOL CBaldurMessage::SendImportingOptionToServer(BYTE nImportingBitField)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(BYTE);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BYTE*>(pMessage + nMsgPtr) = nImportingBitField;
    nMsgPtr += sizeof(BYTE);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_IMPORTING,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42C230
BOOL CBaldurMessage::OnSettingsImportingOptionSet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BYTE nValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
            return FALSE;
        } else {
            if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
                return FALSE;
            }
        }
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(BYTE)) {
        return FALSE;
    }

    nValue = *reinterpret_cast<BYTE*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BYTE);

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetImportingCharacterOption(nValue);

    SendFullSettingsToClients(CString(""));

    return TRUE;
}

// 0x42C390
BOOL CBaldurMessage::SendRestrictStoreOptionToServer(BOOLEAN bRestrictStore)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bRestrictStore;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_RESTRICT_STORE,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42C4E0
BOOL CBaldurMessage::OnSettingsRestrictStoreOptionSet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BOOLEAN bValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
            return FALSE;
        } else {
            if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
                return FALSE;
            }
        }
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(BOOLEAN)) {
        return FALSE;
    }

    bValue = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetRestrictStoreOption(bValue);

    SendFullSettingsToClients(CString(""));

    return TRUE;
}

// 0x42C640
BOOL CBaldurMessage::SendJoinRequestOptionToServer(BOOLEAN bListenToJoin)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bListenToJoin;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_LISTEN_JOIN,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42C790
BOOL CBaldurMessage::OnSettingsJoinRequestOptionSet(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BOOLEAN bValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
            return FALSE;
        } else {
            if (nMsgFrom != g_pChitin->cNetwork.FindPlayerLocationByID(g_pChitin->cNetwork.GetHostPlayerID(), FALSE)) {
                return FALSE;
            }
        }
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(BOOLEAN)) {
        return FALSE;
    }

    bValue = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetListenToJoinOption(bValue, TRUE);

    SendFullSettingsToClients(CString(""));

    return TRUE;
}

// 0x42C8F0
BOOLEAN CBaldurMessage::SendCharacterSlotStatusToServer(SHORT nCharacterSlot, BYTE nStatus)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(SHORT) + sizeof(BYTE);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<SHORT*>(pMessage + nMsgPtr) = nCharacterSlot;
    nMsgPtr += sizeof(SHORT);

    *reinterpret_cast<BYTE*>(pMessage + nMsgPtr) = nStatus;
    nMsgPtr += sizeof(BYTE);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_SLOT_STATUS,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42CA40
BOOLEAN CBaldurMessage::OnSettingsCharacterSlotStatus(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    SHORT nCharacterSlot;
    BYTE nStatus;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    if (dwSize != nMsgPtr + sizeof(SHORT) + sizeof(BYTE)) {
        return FALSE;
    }

    nCharacterSlot = *reinterpret_cast<SHORT*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(SHORT);

    nStatus = *reinterpret_cast<BYTE*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BYTE);

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetCharacterControlledByPlayer(nCharacterSlot) != nMsgFrom) {
        return FALSE;
    }

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SignalCharacterStatus(nCharacterSlot,
        nStatus,
        FALSE,
        TRUE);

    return TRUE;
}

// 0x42CAD0
BOOLEAN CBaldurMessage::SendArbitrationLockRequestToServer(BOOLEAN bLockValue)
{
    CString sHostName;
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bLockValue;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_LOCK_REQUEST,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42CC20
BOOLEAN CBaldurMessage::OnSettingsArbitrationLockRequest(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BOOLEAN bLockValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bLockValue = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetArbitrationLockStatus(bLockValue, 0);
    }

    return TRUE;
}

// 0x42CCA0
BOOLEAN CBaldurMessage::SendArbitrationLockStatus(BOOLEAN bStatus, unsigned char a2)
{
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN) + sizeof(unsigned char);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bStatus;
    nMsgPtr += sizeof(BOOLEAN);

    *reinterpret_cast<unsigned char*>(pMessage + nMsgPtr) = a2;
    nMsgPtr += sizeof(unsigned char);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_ALL_PLAYERS | CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_LOCK_STATUS,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42CDA0
BOOLEAN CBaldurMessage::OnSettingsArbitrationLockStatus(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BOOLEAN bStatus;
    unsigned char v1;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bStatus = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    v1 = *reinterpret_cast<unsigned char*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(unsigned char);

    if (g_pChitin->cNetwork.GetHostPlayerID() != g_pChitin->cNetwork.GetPlayerID(nMsgFrom)) {
        return FALSE;
    }

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetArbitrationLockStatus(bStatus, v1);

    if (bStatus == TRUE) {
        if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineLoad
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCreateChar
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineMultiPlayer
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineConnection) {
            g_pBaldurChitin->GetObjectGame()->SetTempCursor(4);

            g_pBaldurChitin->m_pEngineMultiPlayer->m_nLobbyMode = 1;
            g_pBaldurChitin->m_pEngineMultiPlayer->StartMultiPlayer(1);
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineMultiPlayer);
        }

        if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineMultiPlayer
            && g_pBaldurChitin->m_pEngineMultiPlayer->m_nLobbyMode != 1) {
            g_pBaldurChitin->m_pEngineMultiPlayer->m_nLobbyMode = 1;
            g_pBaldurChitin->m_pEngineMultiPlayer->StartMultiPlayer(1);
        }
    }

    return TRUE;
}

// 0x42CEF0
BOOLEAN CBaldurMessage::SendArbitrationLockAllowInput(BOOLEAN bAllowInput)
{
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) = bAllowInput;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_ALL_PLAYERS | CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_LOCK_ALLOW_INPUT,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42CFF0
BOOLEAN CBaldurMessage::OnSettingsArbitrationLockAllowInput(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BOOLEAN bStatus;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bStatus = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    if (g_pChitin->cNetwork.GetHostPlayerID() != g_pChitin->cNetwork.GetPlayerID(nMsgFrom)) {
        return FALSE;
    }

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetArbitrationLockAllowInput(bStatus);

    return TRUE;
}

// 0x42D070
BOOLEAN CBaldurMessage::KickPlayerRequest(const CString& sPlayerName)
{
    DWORD dwSize;
    BYTE* pMessage;
    INT nMsgPtr;
    CString sHostName;
    CString sCopyPlayerName;

    // NOTE: Uninline.
    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "" || sHostName == sPlayerName) {
        return FALSE;
    }

    dwSize = sPlayerName.GetLength() + sizeof(BYTE);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    // FIXME: Always true.
    if (dwSize != 0) {
        memset(pMessage, 0, dwSize);
    }

    nMsgPtr = 0;

    *reinterpret_cast<BYTE*>(pMessage + nMsgPtr) = sPlayerName.GetLength();
    nMsgPtr += sizeof(BYTE);

    sCopyPlayerName = sPlayerName;
    memcpy(pMessage + nMsgPtr,
        sCopyPlayerName.GetBuffer(sCopyPlayerName.GetLength()),
        sCopyPlayerName.GetLength());

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (g_pChitin->cNetwork.FindPlayerLocationByName(sPlayerName, FALSE) == -1) {
            // FIXME: Leaking `pMessage`.
            return FALSE;
        }

        PLAYER_ID playerID = g_pChitin->cNetwork.FindPlayerIDByName(sPlayerName, FALSE);

        KickPlayerServerSupport(sPlayerName);
        g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
            CNetwork::SEND_GUARANTEED,
            CBaldurMessage::MSG_TYPE_KICK_PLAYER,
            CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_HOOFED_OUT,
            NULL,
            0);

        SleepEx(500, FALSE);

        if (playerID != 0) {
            g_pChitin->cNetwork.KickPlayer(playerID, TRUE);
        }
    } else {
        g_pChitin->cNetwork.SendSpecificMessage(sHostName,
            CNetwork::SEND_GUARANTEED,
            CBaldurMessage::MSG_TYPE_KICK_PLAYER,
            CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_REQUEST,
            pMessage,
            dwSize);
    }

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42D310
BOOL CBaldurMessage::OnKickPlayerRequest(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()
        || !g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::LEADER)) {
        return FALSE;
    }

    INT nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    BYTE nNameLength = *reinterpret_cast<BYTE*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BYTE);

    char szPlayerName[32] = { 0 };
    memcpy(szPlayerName, pMessage + nMsgPtr, nNameLength);

    CString sPlayerName(szPlayerName, nNameLength);

    if (g_pChitin->cNetwork.FindPlayerLocationByName(sPlayerName, FALSE) == -1
        || g_pChitin->cNetwork.FindPlayerIDByName(sPlayerName, FALSE) == g_pChitin->cNetwork.m_idLocalPlayer) {
        return FALSE;
    }

    PLAYER_ID playerID = g_pChitin->cNetwork.FindPlayerIDByName(sPlayerName, FALSE);

    KickPlayerServerSupport(sPlayerName);
    g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
        CNetwork::SEND_GUARANTEED,
        CBaldurMessage::MSG_TYPE_KICK_PLAYER,
        CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_HOOFED_OUT,
        NULL,
        0);

    SleepEx(500, FALSE);

    if (playerID != 0) {
        g_pChitin->cNetwork.KickPlayer(playerID, TRUE);
    }

    return TRUE;
}

// 0x42D4E0
void CBaldurMessage::KickPlayerServerSupport(const CString& sPlayerName)
{
    DWORD dwSize;
    BYTE* pMessage;
    CString sSendTo;

    PLAYER_ID idToKick = g_pChitin->cNetwork.FindPlayerIDByName(sPlayerName, FALSE);
    if (idToKick != 0) {
        dwSize = sizeof(PLAYER_ID);
        pMessage = CreateBuffer(dwSize);
        if (pMessage != NULL) {
            *reinterpret_cast<PLAYER_ID*>(pMessage) = idToKick;

            for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
                PLAYER_ID playerID = g_pChitin->cNetwork.GetPlayerID(nPlayer);
                if (playerID != 0
                    && playerID != g_pChitin->cNetwork.m_idLocalPlayer
                    && playerID != idToKick) {
                    g_pChitin->cNetwork.GetPlayerName(nPlayer, sSendTo);
                    g_pChitin->cNetwork.SendSpecificMessage(sSendTo,
                        CNetwork::SEND_GUARANTEED,
                        CBaldurMessage::MSG_TYPE_KICK_PLAYER,
                        CBaldurMessage::MSG_SUBTYPE_KICK_PLAYER_SERVER_SUPPORT,
                        pMessage,
                        dwSize);
                }
            }

            DestroyBuffer(pMessage);
        }
    }
}

// 0x42D600
BOOLEAN CBaldurMessage::OnKickPlayerServerSupport(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetPlayerID(nMsgFrom) != g_pChitin->cNetwork.GetHostPlayerID()) {
        return FALSE;
    }

    PLAYER_ID playerID = *reinterpret_cast<PLAYER_ID*>(pMessage + CNetwork::SPEC_MSG_HEADER_LENGTH);
    g_pChitin->cNetwork.KickPlayer(playerID, TRUE);

    return TRUE;
}

// 0x42D670
BOOL CBaldurMessage::OnKickPlayerHoofedOut(INT nMsgFrom)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetPlayerID(nMsgFrom) != g_pChitin->cNetwork.GetHostPlayerID()) {
        return FALSE;
    }

    g_pChitin->cNetwork.OnCloseSession();

    return TRUE;
}

// 0x42D6E0
BOOL CBaldurMessage::AddDroppedPlayer(PLAYER_ID playerID)
{
    CSingleLock lock(&field_9A, FALSE);
    lock.Lock(INFINITE);

    // NOTE: Original code is slightly different but does the same thing.
    for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
        if (m_pRemovedPlayerID[nPlayer] == 0) {
            m_pRemovedPlayerID[nPlayer] = playerID;
            m_bRemovedPlayerID = TRUE;
            break;
        }
    }

    lock.Unlock();

    return TRUE;
}

// 0x42D7A0
BOOL CBaldurMessage::PurgeDroppedPlayers()
{
    CSingleLock lock(&field_9A, FALSE);

    if (m_bRemovedPlayerID == TRUE) {
        lock.Lock(INFINITE);

        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            if (m_pRemovedPlayerID[nPlayer] != 0) {
                g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->RemovePlayer(m_pRemovedPlayerID[nPlayer]);
                m_pRemovedPlayerID[nPlayer] = 0;
            }
        }

        m_bRemovedPlayerID = FALSE;

        lock.Unlock();
    }

    return TRUE;
}

// 0x42D860
void CBaldurMessage::UpdateDemandCharacters(unsigned char a1, int a2, unsigned char a3)
{
    // TODO: Incomplete.
}

// 0x42DCF0
void CBaldurMessage::BroadcastUpdateCharacterSlot(BOOLEAN bUpdateAllCharacters, INT nUpdatedCharacterSlot, BOOLEAN bProgressBarInPlace)
{
    // TODO: Incomplete.
}

// 0x42E0C0
BOOLEAN CBaldurMessage::OnUpdateDemandCharacterSlot(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x42F100
BOOLEAN CBaldurMessage::DemandCharacterSlot(SHORT nCharacterSlot, BOOLEAN bDemandFromHost, SHORT nPlayerSlot)
{
    CString sSendTo;
    DWORD dwSize;
    BYTE* pMessage;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (bDemandFromHost == TRUE) {
        if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
            return FALSE;
        }

        g_pChitin->cNetwork.GetHostPlayerName(sSendTo);
    } else {
        g_pChitin->cNetwork.GetPlayerName(nPlayerSlot, sSendTo);
    }

    if (sSendTo == "") {
        return FALSE;
    }

    dwSize = sizeof(SHORT);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    *reinterpret_cast<SHORT*>(pMessage) = nCharacterSlot;

    g_pChitin->cNetwork.SendSpecificMessage(sSendTo,
        CNetwork::SEND_GUARANTEED,
        CBaldurMessage::MSG_TYPE_PLAYERCHAR,
        CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_DEMAND_SLOT,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    if (bDemandFromHost) {
        while (!g_pChitin->cNetwork.PeekSpecificMessage(sSendTo, CBaldurMessage::MSG_TYPE_PLAYERCHAR, CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_DEMAND_REPLY)
            && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pBaldurChitin->GetBaldurMessage()->HandleBlockingMessages();

            g_pChitin->m_bDisplayStale = TRUE;
            SleepEx(60, FALSE);
        }

        if (!g_pChitin->cNetwork.GetSessionOpen()) {
            return FALSE;
        }

        pMessage = g_pChitin->cNetwork.FetchSpecificMessage(sSendTo,
            CBaldurMessage::MSG_TYPE_PLAYERCHAR,
            CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_DEMAND_REPLY,
            dwSize);
        OnDemandCharacterSlotReply(nPlayerSlot, pMessage, dwSize);
        delete pMessage;
    }

    return TRUE;
}

// 0x42F390
BOOLEAN CBaldurMessage::OnDemandCharacterSlot(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    CString sSendTo;
    SHORT nCharacterSlot;
    BOOLEAN bSendCharInfo;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        nCharacterSlot = *reinterpret_cast<SHORT*>(pMessage + CNetwork::SPEC_MSG_HEADER_LENGTH);
        bSendCharInfo = g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(nCharacterSlot) != CGameObjectArray::INVALID_INDEX;
        g_pChitin->cNetwork.GetRawPlayerName(nMsgFrom, sSendTo);
        DemandCharacterSlotReply(sSendTo, nCharacterSlot, bSendCharInfo);
        return TRUE;
    } else {
        return FALSE;
    }
}

// 0x42F480
BOOLEAN CBaldurMessage::BroadcastDemandCharacterSlotReply(BOOLEAN bUpdateAllCharacters, INT nUpdatedCharacterSlot, BOOLEAN bProgressBarInPlace)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x42F750
BOOLEAN CBaldurMessage::DemandCharacterSlotReply(CString& sSendTo, SHORT nCharacterSlot, BOOLEAN bSendCharInfo)
{
    DWORD dwSize;
    BYTE* pMessage;
    DWORD nCustomBio = 0;
    STR_RES strRes;
    CSavedGamePartyCreature partyCreature;
    DWORD dwCreatureDataLoc;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    dwSize = sizeof(SHORT) + sizeof(BOOLEAN);

    if (bSendCharInfo == TRUE) {
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(nCharacterSlot);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Marshal(partyCreature, TRUE);
            dwSize += sizeof(CSavedGamePartyCreature);

            dwCreatureDataLoc = dwSize;

            if (g_pBaldurChitin->GetTlkTable().m_override.Fetch(pSprite->GetBaseStats()->m_biography, strRes)) {
                nCustomBio = strRes.szText.GetLength();
                dwSize += nCustomBio;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bSendCharInfo = FALSE;
        }
    }

    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    DWORD cnt = 0;

    *reinterpret_cast<SHORT*>(pMessage + cnt) = nCharacterSlot;
    cnt += sizeof(SHORT);

    *reinterpret_cast<BOOLEAN*>(pMessage + cnt) = bSendCharInfo;
    cnt += sizeof(BOOLEAN);

    if (bSendCharInfo == TRUE) {
        BYTE* pCreature = reinterpret_cast<BYTE*>(partyCreature.m_creatureOffset);
        partyCreature.m_creatureOffset = dwCreatureDataLoc;

        memcpy(pMessage + cnt, &partyCreature, sizeof(CSavedGamePartyCreature));
        cnt += sizeof(CSavedGamePartyCreature);

        memcpy(pMessage + cnt, pCreature, partyCreature.m_creatureSize);
        cnt += partyCreature.m_creatureSize;

        delete pCreature;

        *reinterpret_cast<DWORD*>(pMessage + cnt) = nCustomBio;
        cnt += sizeof(DWORD);

        if (nCustomBio != 0) {
            memcpy(pMessage + cnt, (LPCSTR)strRes.szText, nCustomBio);
        }
    }

    g_pChitin->cNetwork.SendSpecificMessage(sSendTo,
        CNetwork::SEND_GUARANTEED,
        CBaldurMessage::MSG_TYPE_PLAYERCHAR,
        CBaldurMessage::MSG_SUBTYPE_PLAYERCHAR_DEMAND_REPLY,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x42FA60
BOOLEAN CBaldurMessage::OnDemandCharacterSlotReply(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x42FE20
BOOL CBaldurMessage::ObjectAdd(LONG localObjectID, BYTE nObjectType, BYTE* pObjectData, DWORD dwObjectDataSize)
{
    DWORD dwSize;
    BYTE* pByteMessage;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || m_bInOnObjectAdd == TRUE) {
        return FALSE;
    }

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(localObjectID,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
    LONG remoteObjectID = pObject->m_remoteObjectID;

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(localObjectID,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(BYTE)
        + sizeof(DWORD)
        + dwObjectDataSize;
    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage != NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(pByteMessage + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(pByteMessage + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nObjectType;
    cnt += sizeof(BYTE);

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = dwObjectDataSize;
    cnt += sizeof(DWORD);

    if (dwObjectDataSize != 0) {
        memcpy(pByteMessage + cnt, pObjectData, dwObjectDataSize);
        cnt += dwObjectDataSize;
    }

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_OBJECT,
        MSG_SUBTYPE_OBJECT_ADD,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x430720
BOOL CBaldurMessage::ObjectControlSuggestion(PLAYER_ID playerToSuggestID, LONG localObjectID)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x430980
BOOL CBaldurMessage::ObjectControlRequest(LONG localObjectID)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x430C50
BOOL CBaldurMessage::ObjectControl()
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x431560
BOOLEAN CBaldurMessage::SwapItemRequest(BYTE nArea, LONG nObjectID, SHORT nSlotNum, CItem*& pNewItem, STRREF& errorCode, WORD wCount)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4319C0
BOOLEAN CBaldurMessage::OnSwapItemRequest(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    // MULTIPLAYER: host-side handler for a client's inventory item-swap request.
    // UNRECOVERED — pairs with SwapItemReply / OnSwapItemReply and the
    // CMessageStore* bag-sync. The single-player give-to-portrait / inventory
    // swap paths work without it; this only matters in multiplayer sessions.
    return FALSE;
}

// 0x432C50
BOOLEAN CBaldurMessage::SendChatMessage(CString& sChatMessage)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x433080
BOOLEAN CBaldurMessage::OnChatMessage(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    CString sChatString;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        // FIXME: Unncessesary temporary.
        sChatString = CString(reinterpret_cast<char*>(pByteMessage + 1), dwSize - 1);
        m_cChatBuffer.AddMessage(sChatString);
        return TRUE;
    } else {
        return FALSE;
    }
}

// 0x433150
void CBaldurMessage::SetSignalDefaultSecondsToTimeout()
{
    m_dwSignalSecondsToTimeout = GetPrivateProfileIntA("Multiplayer",
        "Client Timeout",
        SIGNAL_SECONDSTOMPTIMEOUT,
        g_pBaldurChitin->GetIniFileName());

    if (m_dwSignalSecondsToTimeout <= 30 || m_dwSignalSecondsToTimeout > 9999) {
        m_dwSignalSecondsToTimeout = SIGNAL_SECONDSTOMPTIMEOUT;
    }
}

// 0x4331A0
BOOLEAN CBaldurMessage::RequestClientSignal(BYTE signalToSend)
{
    DWORD dwSize;
    INT nMsgPtr;
    BYTE* pData;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    dwSize = sizeof(BYTE) + sizeof(BOOLEAN);
    pData = CreateBuffer(dwSize);
    if (pData == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BYTE*>(pData + nMsgPtr) = signalToSend;
    nMsgPtr += sizeof(BYTE);

    *reinterpret_cast<BOOLEAN*>(pData + nMsgPtr) = CInfGame::byte_8E7528;
    nMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_SIGNAL,
        MSG_SUBTYPE_SIGNAL_REQUEST,
        pData,
        dwSize);

    DestroyBuffer(pData);

    return TRUE;
}

// 0x4332B0
BOOLEAN CBaldurMessage::OnRequestClientSignal(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    BYTE signalType;
    INT nMsgPtr;

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    signalType = *reinterpret_cast<BYTE*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BYTE);

    CInfGame::byte_8E7528 = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr) != 0;
    nMsgPtr += sizeof(BOOLEAN);

    if (signalType == SIGNAL_END_GAME) {
        g_pBaldurChitin->m_pEngineConnection->ReadyEndCredits();
    }

    return SendSignal(SIGNAL_SERVER, signalType);
}

// 0x433310
BOOLEAN CBaldurMessage::RemoveSignalsFromQueue(BYTE signalType, BYTE signalData)
{
    BYTE nStartNewQueue;
    BYTE nEndNewQueue;
    BYTE cnt;

    nStartNewQueue = 0;
    nEndNewQueue = 0;
    for (cnt = 0; cnt < m_nSignalQueueSize; cnt++) {
        if (m_pnSignalFrom[cnt] != -1) {
            if (m_pnSignalType[cnt] == signalType && m_pnSignalData[cnt] == signalData) {
                m_pnSignalFrom[cnt] = -1;
                m_pnSignalType[cnt] = 0;
                m_pnSignalData[cnt] = 0;
            } else {
                if (cnt != nEndNewQueue) {
                    m_pnSignalFrom[nEndNewQueue] = m_pnSignalFrom[cnt];
                    m_pnSignalType[nEndNewQueue] = m_pnSignalType[cnt];
                    m_pnSignalData[nEndNewQueue] = m_pnSignalData[cnt];

                    m_pnSignalFrom[cnt] = -1;
                    m_pnSignalType[cnt] = 0;
                    m_pnSignalData[cnt] = 0;
                }
                nEndNewQueue++;
            }
        }
    }

    m_nSignalQueueStart = nStartNewQueue;
    m_nSignalQueueEnd = nEndNewQueue;

    return TRUE;
}

// 0x4333C0
BOOLEAN CBaldurMessage::SendSignal(BYTE signalType, BYTE signalToSend)
{
    CString sSendTo;
    DWORD dwSize;
    INT nMsgPtr;
    BYTE* pData;
    DWORD dwFlags;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    dwSize = sizeof(BYTE) + sizeof(BYTE);
    pData = CreateBuffer(dwSize);
    if (pData == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<BYTE*>(pData + nMsgPtr) = signalType;
    nMsgPtr += sizeof(BYTE);

    *reinterpret_cast<BYTE*>(pData + nMsgPtr) = signalToSend;
    nMsgPtr += sizeof(BYTE);

    if (signalType == SIGNAL_SERVER) {
        g_pChitin->cNetwork.GetHostPlayerName(sSendTo);
        dwFlags = CNetwork::SEND_GUARANTEED;
    } else {
        sSendTo = "";
        dwFlags = CNetwork::SEND_ALL_PLAYERS | CNetwork::SEND_GUARANTEED;
    }

    g_pChitin->cNetwork.SendSpecificMessage(sSendTo,
        dwFlags,
        MSG_TYPE_SIGNAL,
        MSG_SUBTYPE_SIGNAL,
        pData,
        dwSize);

    DestroyBuffer(pData);

    return FALSE;
}

// 0x433530
BOOLEAN CBaldurMessage::OnSignal(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    BYTE signalType;
    BYTE signalData;

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    signalType = *reinterpret_cast<BYTE*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BYTE);

    signalData = *reinterpret_cast<BYTE*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BYTE);

    m_pnSignalFrom[m_nSignalQueueEnd] = nMsgFrom;
    m_pnSignalType[m_nSignalQueueEnd] = signalType;
    m_pnSignalData[m_nSignalQueueEnd] = signalData;
    m_nSignalQueueEnd = (m_nSignalQueueEnd + 1) % m_nSignalQueueSize;

    return TRUE;
}

// 0x433580
BOOLEAN CBaldurMessage::NonBlockingWaitForSignal(BYTE signalType, BYTE signalToWaitFor)
{
    CString sPlayerName;
    BOOLEAN signalReceived[CNETWORK_MAX_PLAYERS];
    DWORD dwSize;
    BYTE* pMessage;
    BYTE nLocalPlayer;
    BYTE nQueue;
    INT nQueueChecked;
    BOOLEAN bComplete;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return -1;
    }

    nLocalPlayer = static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer);
    if (nLocalPlayer == 0xFF) {
        nLocalPlayer = 0;
    }

    if (signalType == SIGNAL_SERVER) {
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            signalReceived[nPlayer] = FALSE;
        }
        signalReceived[nLocalPlayer] = TRUE;
    } else {
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            signalReceived[nPlayer] = TRUE;
        }
        signalReceived[nLocalPlayer] = FALSE;
    }

    nQueue = m_nSignalQueueStart;
    nQueueChecked = 0;
    if (nQueue != m_nSignalQueueEnd) {
        do {
            if (nQueueChecked >= m_nSignalQueueSize) {
                break;
            }

            if (m_pnSignalType[nQueue] == signalType
                && m_pnSignalData[nQueue] == signalToWaitFor) {
                signalReceived[m_pnSignalFrom[nQueue]] = TRUE;
            }

            nQueue = (nQueue + 1) % m_nSignalQueueSize;
            nQueueChecked++;
        } while (nQueue != m_nSignalQueueEnd);
    }

    bComplete = TRUE;
    for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
        if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0
            && signalReceived[nPlayer] == FALSE
            && bComplete == TRUE) {
            bComplete = FALSE;
            g_pChitin->cNetwork.GetPlayerName(nPlayer, sPlayerName);
        }
    }

    if (bComplete == TRUE) {
        RemoveSignalsFromQueue(signalType, signalToWaitFor);
    } else if (g_pChitin->cNetwork.PeekSpecificMessage(sPlayerName, MSG_TYPE_SIGNAL, MSG_SUBTYPE_SIGNAL) == TRUE) {
        pMessage = g_pChitin->cNetwork.FetchSpecificMessage(sPlayerName,
            MSG_TYPE_SIGNAL,
            MSG_SUBTYPE_SIGNAL,
            dwSize);

        m_pnSignalFrom[m_nSignalQueueEnd] = static_cast<BYTE>(g_pChitin->cNetwork.FindPlayerLocationByName(sPlayerName, FALSE));
        m_pnSignalType[m_nSignalQueueEnd] = *reinterpret_cast<BYTE*>(pMessage + CNetwork::SPEC_MSG_HEADER_LENGTH);
        m_pnSignalData[m_nSignalQueueEnd] = *reinterpret_cast<BYTE*>(pMessage + CNetwork::SPEC_MSG_HEADER_LENGTH + sizeof(BYTE));
        m_nSignalQueueEnd = (m_nSignalQueueEnd + 1) % m_nSignalQueueSize;

        delete pMessage;
    }

    return bComplete;
}

// 0x4337D0
BYTE CBaldurMessage::KickOutWaitingForSignal(BYTE signalType, BYTE signalToWaitFor)
{
    CString sPlayerName;
    BOOLEAN signalReceived[CNETWORK_MAX_PLAYERS];
    BYTE nLocalPlayer;
    BYTE nQueue;
    INT nQueueChecked;
    BOOLEAN bComplete;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return -1;
    }

    nLocalPlayer = static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer);
    if (nLocalPlayer == 0xFF) {
        nLocalPlayer = 0;
    }

    if (signalType == SIGNAL_SERVER) {
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            signalReceived[nPlayer] = FALSE;
        }
        signalReceived[nLocalPlayer] = TRUE;
    } else {
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            signalReceived[nPlayer] = TRUE;
        }
        signalReceived[nLocalPlayer] = FALSE;
    }

    nQueue = m_nSignalQueueStart;
    nQueueChecked = 0;
    if (nQueue != m_nSignalQueueEnd) {
        do {
            if (nQueueChecked >= m_nSignalQueueSize) {
                break;
            }

            if (m_pnSignalType[nQueue] == signalType
                && m_pnSignalData[nQueue] == signalToWaitFor) {
                signalReceived[m_pnSignalFrom[nQueue]] = TRUE;
            }

            nQueue = (nQueue + 1) % m_nSignalQueueSize;
            nQueueChecked++;
        } while (nQueue != m_nSignalQueueEnd);
    }

    bComplete = TRUE;
    for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
        if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0
            && signalReceived[nPlayer] == FALSE
            && bComplete == TRUE) {
            bComplete = FALSE;
            g_pChitin->cNetwork.GetPlayerName(nPlayer, sPlayerName);
            KickPlayerRequest(sPlayerName);
        }
    }

    return TRUE;
}

// 0x433950
BOOLEAN CBaldurMessage::WaitForSignal(BYTE signalType, BYTE signalToWaitFor)
{
    CString sPlayerName;
    BOOLEAN signalReceived[CNETWORK_MAX_PLAYERS];
    DWORD dwSize;
    BYTE* pMessage;
    BYTE nLocalPlayer;
    BYTE nQueue;
    INT nQueueChecked;
    BOOLEAN bComplete;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    nLocalPlayer = static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer);
    if (nLocalPlayer == 0xFF) {
        nLocalPlayer = 0;
    }

    if (signalType == SIGNAL_SERVER) {
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            signalReceived[nPlayer] = FALSE;
        }
        signalReceived[nLocalPlayer] = TRUE;
    } else {
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            signalReceived[nPlayer] = TRUE;
        }
        signalReceived[nLocalPlayer] = FALSE;
    }

    do {
        nQueue = m_nSignalQueueStart;
        nQueueChecked = 0;
        if (nQueue != m_nSignalQueueEnd) {
            do {
                if (nQueueChecked >= m_nSignalQueueSize) {
                    break;
                }

                if (m_pnSignalType[nQueue] == signalType
                    && m_pnSignalData[nQueue] == signalToWaitFor) {
                    signalReceived[m_pnSignalFrom[nQueue]] = TRUE;
                }

                nQueue = (nQueue + 1) % m_nSignalQueueSize;
                nQueueChecked++;
            } while (nQueue != m_nSignalQueueEnd);
        }

        bComplete = TRUE;
        for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
            if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0
                && signalReceived[nPlayer] == FALSE
                && bComplete == TRUE) {
                bComplete = FALSE;
                g_pChitin->cNetwork.GetPlayerName(nPlayer, sPlayerName);
            }
        }

        if (bComplete != TRUE) {
            if (g_pChitin->cNetwork.PeekSpecificMessage(sPlayerName, MSG_TYPE_SIGNAL, MSG_SUBTYPE_SIGNAL) == TRUE) {
                pMessage = g_pChitin->cNetwork.FetchSpecificMessage(sPlayerName,
                    MSG_TYPE_SIGNAL,
                    MSG_SUBTYPE_SIGNAL,
                    dwSize);

                m_pnSignalFrom[m_nSignalQueueEnd] = static_cast<BYTE>(g_pChitin->cNetwork.FindPlayerLocationByName(sPlayerName, FALSE));
                m_pnSignalType[m_nSignalQueueEnd] = *reinterpret_cast<BYTE*>(pMessage + CNetwork::SPEC_MSG_HEADER_LENGTH);
                m_pnSignalData[m_nSignalQueueEnd] = *reinterpret_cast<BYTE*>(pMessage + CNetwork::SPEC_MSG_HEADER_LENGTH + sizeof(BYTE));
                m_nSignalQueueEnd = (m_nSignalQueueEnd + 1) % m_nSignalQueueSize;

                delete pMessage;
            } else {
                g_pChitin->m_bDisplayStale = TRUE;
                SleepEx(60, FALSE);
            }
        }

        if (!g_pChitin->cNetwork.GetSessionOpen()) {
            return FALSE;
        }
    } while (bComplete != TRUE);

    RemoveSignalsFromQueue(signalType, signalToWaitFor);

    return TRUE;
}

// 0x433BE0
BOOLEAN CBaldurMessage::SendProgressBarStatus(LONG nActionProgress, LONG nActionTarget, BOOLEAN bWaiting, LONG nWaitingReason, BOOLEAN bTimeoutVisible, DWORD nSecondsToTimeout)
{
    DWORD dwSize;
    INT nMsgPtr;
    BYTE* pData;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    dwSize = sizeof(LONG)
        + sizeof(LONG)
        + sizeof(BOOLEAN)
        + sizeof(LONG)
        + sizeof(BOOLEAN)
        + sizeof(DWORD);
    pData = CreateBuffer(dwSize);
    if (pData == NULL) {
        return FALSE;
    }

    nMsgPtr = 0;

    *reinterpret_cast<LONG*>(pData + nMsgPtr) = nActionProgress;
    nMsgPtr += sizeof(LONG);

    *reinterpret_cast<LONG*>(pData + nMsgPtr) = nActionTarget;
    nMsgPtr += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(pData + nMsgPtr) = bWaiting;
    nMsgPtr += sizeof(BOOLEAN);

    *reinterpret_cast<LONG*>(pData + nMsgPtr) = nWaitingReason;
    nMsgPtr += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(pData + nMsgPtr) = bTimeoutVisible;
    nMsgPtr += sizeof(BOOLEAN);

    *reinterpret_cast<DWORD*>(pData + nMsgPtr) = nSecondsToTimeout;
    nMsgPtr += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_PROGRESSBAR,
        MSG_SUBTYPE_PROGRESSBAR_STATUS,
        pData,
        dwSize);

    DestroyBuffer(pData);

    return TRUE;
}

// 0x433D30
BOOLEAN CBaldurMessage::OnProgressBarStatus(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    SHORT nSender;
    LONG nActionProgress;
    LONG nActionTarget;
    BOOLEAN bWaiting;
    LONG nWaitingReason;
    BOOLEAN bTimeoutVisible;
    DWORD nSecondsToTimeout;

    nSender = static_cast<SHORT>(nMsgFrom);

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nActionProgress = *reinterpret_cast<LONG*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(LONG);

    nActionTarget = *reinterpret_cast<LONG*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(LONG);

    bWaiting = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    nWaitingReason = *reinterpret_cast<LONG*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(LONG);

    bTimeoutVisible = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    nSecondsToTimeout = *reinterpret_cast<DWORD*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(DWORD);

    g_pChitin->cProgressBar.SetRemoteActionProgress(nSender, nActionProgress);
    g_pChitin->cProgressBar.SetRemoteActionTarget(nSender, nActionTarget);
    g_pChitin->cProgressBar.SetRemoteWaiting(nSender, bWaiting);
    g_pChitin->cProgressBar.SetRemoteWaitingReason(nSender, nWaitingReason);

    if (nMsgFrom == g_pChitin->cNetwork.m_nHostPlayer) {
        g_pChitin->cProgressBar.m_bTimeoutVisible = bTimeoutVisible;
        g_pChitin->cProgressBar.m_nSecondsToTimeout = nSecondsToTimeout;
    }

    return TRUE;
}

// 0x433DF0
BOOLEAN CBaldurMessage::SendDialogRequestToServer(BOOLEAN bTestPermission)
{
    DWORD dwSize;
    INT nMsgPtr;
    BYTE* pData;
    BOOLEAN bValue;
    CString sHostName;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        m_bDialogRequestPending = FALSE;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        if (g_pBaldurChitin->m_pEngineWorld->m_bPaused == TRUE) {
            m_bDialogRequestPending = FALSE;
            return FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 386
            || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 1282) {
            m_bDialogRequestPending = FALSE;
            return FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog == TRUE) {
            m_bDialogRequestPending = FALSE;
            return DIALOG_NOT_PERMITTED;
        }

        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_nHostPermittedDialogDelay = 0;
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog = TRUE;
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog = g_pChitin->cNetwork.m_idLocalPlayer;
        m_bDialogRequestPending = TRUE;
        m_bDialogReplyValue = TRUE;
        m_bDialogReplyReturned = TRUE;
        m_nDialogReplyUpdates = 5;

        return TRUE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        m_bDialogRequestPending = FALSE;
        return FALSE;
    }

    if (!m_bDialogRequestPending) {
        dwSize = sizeof(BOOLEAN);
        pData = CreateBuffer(dwSize);
        if (pData == NULL) {
            m_bDialogRequestPending = FALSE;
            return FALSE;
        }

        nMsgPtr = 0;

        *reinterpret_cast<BOOLEAN*>(pData + nMsgPtr) = bTestPermission;
        nMsgPtr += sizeof(BOOLEAN);

        g_pChitin->cNetwork.SendSpecificMessage(sHostName,
            CNetwork::SEND_GUARANTEED,
            MSG_TYPE_DIALOG,
            MSG_SUBTYPE_DIALOG_PERMIT_REQUEST,
            pData,
            dwSize);

        DestroyBuffer(pData);

        m_bDialogRequestPending = TRUE;
        m_bDialogReplyReturned = FALSE;
        m_nDialogReplyUpdates = 0;
        return DIALOG_REQUESTPENDING;
    }

    if (m_bDialogReplyReturned == TRUE) {
        m_bDialogReplyReturned = FALSE;
        m_nDialogReplyUpdates = 0;
        m_bDialogRequestPending = 0;
        return m_bDialogReplyValue;
    }

    if (!g_pChitin->cNetwork.PeekSpecificMessage(sHostName, MSG_TYPE_DIALOG, MSG_SUBTYPE_DIALOG_PERMIT_REPLY)) {
        return DIALOG_REQUESTPENDING;
    }

    pData = g_pChitin->cNetwork.FetchSpecificMessage(sHostName,
        MSG_TYPE_DIALOG,
        MSG_SUBTYPE_DIALOG_PERMIT_REPLY,
        dwSize);
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        bValue = FALSE;
    } else {
        // NOTE: Uninline.
        bValue = OnDialogReplyFromServer(pData, dwSize);
    }

    m_bDialogReplyReturned = FALSE;

    delete pData;

    m_bDialogRequestPending = FALSE;
    return bValue;
}

// 0x434110
BOOLEAN CBaldurMessage::OnDialogRequestToServer(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    INT nMsgPtr;
    CString sPlayerName;
    BOOLEAN bTestPermission;
    BOOLEAN bPermitDialog;
    BYTE* pReturnMessage;
    DWORD dwReturnSize;
    INT nRetMsgPtr;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetPlayerName(nMsgFrom, sPlayerName);

    bPermitDialog = g_pBaldurChitin->GetBaldurMessage()->m_bInHandleBlockingMessages != TRUE;
    if (g_pBaldurChitin->m_pEngineWorld->m_bPaused == TRUE) {
        bPermitDialog = FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 386
        || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 1282) {
        bPermitDialog = FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog == TRUE) {
        bPermitDialog = FALSE;
    }

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bTestPermission = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    if (bTestPermission == TRUE) {
        if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::DIALOG)) {
            bPermitDialog = DIALOG_NOT_PERMITTED;
        }
    }

    if (bPermitDialog == TRUE) {
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_nHostPermittedDialogDelay = 0;
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog = TRUE;
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog = g_pChitin->cNetwork.GetPlayerID(nMsgFrom);
    }

    dwReturnSize = sizeof(BOOLEAN);
    pReturnMessage = CreateBuffer(dwReturnSize);
    if (pReturnMessage == NULL) {
        return FALSE;
    }

    nRetMsgPtr = 0;

    // FIXME: Redunant.
    *reinterpret_cast<BOOLEAN*>(pReturnMessage + nRetMsgPtr) = 0;

    *reinterpret_cast<BOOLEAN*>(pReturnMessage + nRetMsgPtr) = bPermitDialog;
    nRetMsgPtr += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_DIALOG,
        MSG_SUBTYPE_DIALOG_PERMIT_REPLY,
        pReturnMessage,
        dwReturnSize);

    DestroyBuffer(pReturnMessage);

    return TRUE;
}

// 0x434300
BOOLEAN CBaldurMessage::OnDialogReplyFromServer(BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    BOOLEAN bValue;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bValue = *reinterpret_cast<BOOLEAN*>(pByteMessage + cnt);
    cnt += sizeof(BOOLEAN);

    m_bDialogReplyValue = bValue;
    m_bDialogReplyReturned = TRUE;
    m_nDialogReplyUpdates = 5;

    return bValue;
}

// 0x434350
BOOLEAN CBaldurMessage::DialogRequestKillOrUse()
{
    INT location;
    CString sPlayerName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    location = g_pChitin->cNetwork.FindPlayerLocationByID(g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog, FALSE);
    if (location != -1) {
        g_pChitin->cNetwork.GetPlayerName(location, sPlayerName);
        g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
            CNetwork::SEND_GUARANTEED,
            MSG_TYPE_DIALOG,
            MSG_SUBTYPE_DIALOG_KILL_OR_USE,
            NULL,
            0);
    } else {
        CancelDialogRequestToServer();
    }

    return TRUE;
}

// 0x434480
BOOLEAN CBaldurMessage::OnDialogRequestKillOrUse(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode != 386
        && g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode != 1282) {
        CancelDialogRequestToServer();
    }

    return TRUE;
}

// 0x4344D0
BOOLEAN CBaldurMessage::CancelDialogRequestToServer()
{
    BYTE* pMessage;
    DWORD dwSize;
    INT nMsgPtr;
    CString sHostName;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting()) {
        if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog == g_pChitin->cNetwork.m_idLocalPlayer) {
            g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_nHostPermittedDialogDelay = 0;
            g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog = FALSE;
            g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog = 0;
        }
    } else {
        g_pChitin->cNetwork.GetHostPlayerName(sHostName);

        dwSize = sizeof(BYTE);
        pMessage = CreateBuffer(dwSize);
        if (pMessage == NULL) {
            return FALSE;
        }

        nMsgPtr = 0;

        *reinterpret_cast<BYTE*>(pMessage + nMsgPtr) = 1;
        nMsgPtr += sizeof(BYTE);

        g_pChitin->cNetwork.SendSpecificMessage(sHostName,
            CNetwork::SEND_GUARANTEED,
            MSG_TYPE_DIALOG,
            MSG_SUBTYPE_DIALOG_CANCEL_REQUEST,
            pMessage,
            dwSize);

        DestroyBuffer(pMessage);
    }

    m_bDialogRequestPending = FALSE;
    m_bDialogReplyValue = FALSE;
    m_bDialogReplyReturned = FALSE;
    m_nDialogReplyUpdates = 0;

    return TRUE;
}

// 0x434680
BOOLEAN CBaldurMessage::OnCancelDialogRequestToServer(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    // FIXME: Unused.
    CString sPlayerName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog == g_pChitin->cNetwork.GetPlayerID(nMsgFrom)) {
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_nHostPermittedDialogDelay = 0;
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog = FALSE;
        g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_idHostPermittedDialog = 0;
    }

    return TRUE;
}

// 0x434770
BOOLEAN CBaldurMessage::SendPauseRequestToServer(BOOLEAN bVisualPause, BOOLEAN bPauseToBeSetTo)
{
    BYTE* pMessage;
    DWORD cnt;
    DWORD dwSize;
    CString sHostName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        m_bDialogRequestPending = FALSE;
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN) + sizeof(BOOLEAN);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + cnt) = bVisualPause;
    cnt += sizeof(BOOLEAN);

    *reinterpret_cast<BOOLEAN*>(pMessage + cnt) = bPauseToBeSetTo;
    cnt += sizeof(BOOLEAN);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_PAUSING,
        MSG_SUBTYPE_PAUSING_PERMIT_REQUEST,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x4348C0
BOOLEAN CBaldurMessage::OnPauseRequestToServer(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    BOOLEAN bPauseToBeSetTo;
    DWORD nMsgPtr;
    CString sPlayerName;
    BOOLEAN bVisualPause;
    BOOLEAN bPermitPause;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetPlayerName(nMsgFrom, sPlayerName);

    nMsgPtr = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bVisualPause = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    bPauseToBeSetTo = *reinterpret_cast<BOOLEAN*>(pMessage + nMsgPtr);
    nMsgPtr += sizeof(BOOLEAN);

    bPermitPause = TRUE;
    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 386
        || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 1282) {
        bPermitPause = FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog == TRUE) {
        bPermitPause = FALSE;
    }

    if (!g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(nMsgFrom, CGamePermission::PAUSING)) {
        bPermitPause = FALSE;
    }

    if (g_pBaldurChitin->m_pEngineWorld->m_bPaused == TRUE
        && g_pBaldurChitin->m_pEngineWorld->m_bHardPaused == TRUE) {
        bPermitPause = FALSE;
    }

    if (g_pBaldurChitin->m_pEngineWorld->m_bPaused != bPauseToBeSetTo
        && bPermitPause == TRUE) {
        g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(bVisualPause,
            1,
            g_pChitin->cNetwork.GetPlayerID(nMsgFrom));
    }

    return TRUE;
}

// 0x434A20
BOOLEAN CBaldurMessage::PauseAnnounceStatus(BOOLEAN bHardPauseStatus, BOOLEAN bPauseStatus, PLAYER_ID idPauser)
{
    DWORD dwSize;
    DWORD cnt;
    BYTE* pMessage;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    dwSize = sizeof(BOOLEAN)
        + sizeof(BOOLEAN)
        + sizeof(PLAYER_ID);
    pMessage = CreateBuffer(dwSize);
    if (pMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<BOOLEAN*>(pMessage + cnt) = bHardPauseStatus;
    cnt += sizeof(BOOLEAN);

    *reinterpret_cast<BOOLEAN*>(pMessage + cnt) = bPauseStatus;
    cnt += sizeof(BOOLEAN);

    *reinterpret_cast<PLAYER_ID*>(pMessage + cnt) = idPauser;
    cnt += sizeof(PLAYER_ID);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_PAUSING,
        MSG_SUBTYPE_PAUSING_ANNOUNCE,
        pMessage,
        dwSize);

    DestroyBuffer(pMessage);

    return TRUE;
}

// 0x434B30
BOOLEAN CBaldurMessage::OnPauseAnnounceStatus(INT nMsgFrom, BYTE* pMessage, DWORD dwSize)
{
    DWORD cnt;
    BOOLEAN bHardPausingStatus;
    BOOLEAN bPausingStatus;
    PLAYER_ID idPausingPlayer;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    bHardPausingStatus = *reinterpret_cast<BOOLEAN*>(pMessage + cnt);
    cnt += sizeof(BOOLEAN);

    bPausingStatus = *reinterpret_cast<BOOLEAN*>(pMessage + cnt);
    cnt += sizeof(BOOLEAN);

    idPausingPlayer = *reinterpret_cast<PLAYER_ID*>(pMessage + cnt);
    cnt += sizeof(PLAYER_ID);

    if (g_pBaldurChitin->m_pEngineWorld->m_bPaused == bPausingStatus) {
        g_pBaldurChitin->m_pEngineWorld->m_bPaused = !bPausingStatus;
    }

    g_pBaldurChitin->m_pEngineWorld->TogglePauseGame(0, 0, idPausingPlayer);

    return TRUE;
}

// 0x434EB0
BOOLEAN CBaldurMessage::ChapterAnnounceStatus(BYTE nChapter, CResRef cResRef)
{
    if (g_pChitin->cNetwork.GetSessionOpen() != TRUE
        || g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return FALSE;
    }

    CString sResRef;
    cResRef.CopyToString(sResRef);

    BYTE nResRefLength = static_cast<BYTE>(sResRef.GetLength());
    DWORD dwSize = sizeof(BYTE) + sizeof(BYTE) + nResRefLength;
    BYTE* pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    DWORD cnt = 0;

    pByteMessage[cnt] = nChapter;
    cnt += sizeof(BYTE);

    pByteMessage[cnt] = nResRefLength;
    cnt += sizeof(BYTE);

    memcpy(pByteMessage + cnt, sResRef.GetBuffer(nResRefLength), nResRefLength);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_CHAPTER,
        MSG_SUBTYPE_CHAPTER_ANNOUNCE_STATUS,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x435360
BOOLEAN CBaldurMessage::SendMapWorldCancelRequestToServer()
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4354E0
BOOLEAN CBaldurMessage::SendMapWorldAnnounceStatus(BOOLEAN bActive, PLAYER_ID idController, LONG nLeavingEdge)
{
    // TODO: Incomplete.

    return FALSE;
}

// FIXME: `sAreaResRef` should be reference.
//
// 0x436050
BOOLEAN CBaldurMessage::SendMapWorldRevealArea(CString sAreaResRef)
{
    // TODO: Incomplete.

    return FALSE;
}

// FIXME: `cResMovie` should be reference.
//
// 0x436690
BOOLEAN CBaldurMessage::SendMovieRequestToServer(CResRef cResMovie)
{
    // TODO: Incomplete.

    return FALSE;
}

// FIXME: `cResMovie` should be reference.
//
// 0x4369E0
BOOLEAN CBaldurMessage::MovieAnnounceStatus(CResRef cResMovie)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x436DF0
BOOLEAN CBaldurMessage::SendReformPartyAnnounceStatus()
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x436F50
void CBaldurMessage::WeatherBroadcast(WORD wWeatherFlags)
{
    // TODO: Incomplete.
}

// 0x4370B0
void CBaldurMessage::TimeSynchBroadcast(ULONG nGameTime, BOOLEAN bCompressTime)
{
    // TODO: Incomplete.
}

// 0x437240
void CBaldurMessage::TimeChangeToServer(DWORD deltaTime)
{
    // TODO: Incomplete.
}

// 0x437950
BOOLEAN CBaldurMessage::OnLeaveAreaLuaRequestToServer(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x438BF0
BOOLEAN CBaldurMessage::OnLeaveAreaNameRequestToServer(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x4381A0
void CBaldurMessage::CleanLeaveAreaLuaRequest()
{
    m_bLeaveAreaLuaRequestPending = FALSE;
    m_nLeaveAreaLuaReplyValue = 0;
    m_bLeaveAreaLuaReplyReturned = FALSE;
}

// 0x4393E0
void CBaldurMessage::CleanLeaveAreaNameRequest()
{
    m_bLeaveAreaNameRequestPending = FALSE;
    m_nLeaveAreaNameReplyValue = 0;
    m_bLeaveAreaNameReplyReturned = FALSE;
}

// 0x439400
BOOLEAN CBaldurMessage::SendJournalEntryToServer(STRREF strText, INT nChapter, LONG nTime)
{
    CString sHostName;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    dwSize = sizeof(STRREF)
        + sizeof(INT)
        + sizeof(LONG);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<STRREF*>(pByteMessage + cnt) = strText;
    cnt += sizeof(STRREF);

    *reinterpret_cast<INT*>(pByteMessage + cnt) = nChapter;
    cnt += sizeof(INT);

    *reinterpret_cast<LONG*>(pByteMessage + cnt) = nTime;
    cnt += sizeof(LONG);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_JOURNAL,
        MSG_SUBTYPE_JOURNAL_ADD_ENTRY,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x4395C0
BOOLEAN CBaldurMessage::OnSendJournalEntry(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    STRREF strText;
    INT nChapter;
    LONG nTime;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    strText = *reinterpret_cast<STRREF*>(pByteMessage + cnt);
    cnt += sizeof(STRREF);

    nChapter = *reinterpret_cast<INT*>(pByteMessage + cnt);
    cnt += sizeof(INT);

    nTime = *reinterpret_cast<LONG*>(pByteMessage + cnt);
    cnt += sizeof(LONG);

    g_pBaldurChitin->GetObjectGame()->GetJournal()->AddEntry(strText, nChapter, nTime, 0);

    return TRUE;
}

// 0x439610
BOOLEAN CBaldurMessage::AnnounceJournalEntry(STRREF strText, INT nChapter, LONG nTime)
{
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    dwSize = sizeof(STRREF)
        + sizeof(INT)
        + sizeof(LONG);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<STRREF*>(pByteMessage + cnt) = strText;
    cnt += sizeof(STRREF);

    *reinterpret_cast<INT*>(pByteMessage + cnt) = nChapter;
    cnt += sizeof(INT);

    *reinterpret_cast<LONG*>(pByteMessage + cnt) = nTime;
    cnt += sizeof(LONG);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_JOURNAL,
        MSG_SUBTYPE_JOURNAL_ANNOUNCE,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x439740
BOOLEAN CBaldurMessage::OnAnnounceJournalEntry(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    STRREF strText;
    INT nChapter;
    LONG nTime;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    strText = *reinterpret_cast<STRREF*>(pByteMessage + cnt);
    cnt += sizeof(STRREF);

    nChapter = *reinterpret_cast<INT*>(pByteMessage + cnt);
    cnt += sizeof(INT);

    nTime = *reinterpret_cast<LONG*>(pByteMessage + cnt);
    cnt += sizeof(LONG);

    m_bInOnJournalAnnounce = TRUE;
    g_pBaldurChitin->GetObjectGame()->GetJournal()->AddEntry(strText, nChapter, nTime, 0);
    m_bInOnJournalAnnounce = FALSE;

    return TRUE;
}

// 0x4397A0
BOOLEAN CBaldurMessage::SendJournalUserEntry(CString szText, BYTE nCharacter, INT nChapter, LONG nTime, DWORD nIndex)
{
    CString sHostName;
    DWORD nTextLength;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    nTextLength = szText.GetLength();
    dwSize = sizeof(DWORD)
        + nTextLength
        + sizeof(BYTE)
        + sizeof(INT)
        + sizeof(LONG)
        + sizeof(DWORD);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nTextLength;
    cnt += sizeof(DWORD);

    memcpy(pByteMessage + cnt, szText, nTextLength);
    cnt += nTextLength;

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nCharacter;
    cnt += sizeof(BYTE);

    *reinterpret_cast<INT*>(pByteMessage + cnt) = nChapter;
    cnt += sizeof(INT);

    *reinterpret_cast<LONG*>(pByteMessage + cnt) = nTime;
    cnt += sizeof(LONG);

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nIndex;
    cnt += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_JOURNAL,
        MSG_SUBTYPE_JOURNAL_ADD_USER_ENTRY,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x439990
BOOLEAN CBaldurMessage::OnSendJournalUserEntry(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    DWORD nTextLength;
    BYTE nCharacter;
    INT nChapter;
    LONG nTime;
    DWORD nIndex;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nTextLength = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    CString szText(reinterpret_cast<char*>(pByteMessage + cnt), nTextLength);
    cnt += nTextLength;

    nCharacter = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    nChapter = *reinterpret_cast<INT*>(pByteMessage + cnt);
    cnt += sizeof(INT);

    nTime = *reinterpret_cast<LONG*>(pByteMessage + cnt);
    cnt += sizeof(LONG);

    nIndex = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    g_pBaldurChitin->GetObjectGame()->GetJournal()->InsertEntryAfter(szText,
        nIndex,
        nChapter,
        nTime,
        nCharacter,
        0);

    return TRUE;
}

// 0x439A80
BOOLEAN CBaldurMessage::AnnounceJournalUserEntry(CString szText, BYTE nCharacter, INT nChapter, LONG nTime, DWORD nIndex)
{
    DWORD nTextLength;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    nTextLength = szText.GetLength();
    dwSize = sizeof(DWORD)
        + nTextLength
        + sizeof(BYTE)
        + sizeof(INT)
        + sizeof(LONG)
        + sizeof(DWORD);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nTextLength;
    cnt += sizeof(DWORD);

    memcpy(pByteMessage + cnt, szText, nTextLength);
    cnt += nTextLength;

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nCharacter;
    cnt += sizeof(BYTE);

    *reinterpret_cast<INT*>(pByteMessage + cnt) = nChapter;
    cnt += sizeof(INT);

    *reinterpret_cast<LONG*>(pByteMessage + cnt) = nTime;
    cnt += sizeof(LONG);

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nIndex;
    cnt += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_JOURNAL,
        MSG_SUBTYPE_JOURNAL_ANNOUNCE_USER_ENTRY,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x439C20
BOOLEAN CBaldurMessage::OnAnnounceJournalUserEntry(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    DWORD nTextLength;
    BYTE nCharacter;
    INT nChapter;
    LONG nTime;
    DWORD nIndex;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nTextLength = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    CString szText(reinterpret_cast<char*>(pByteMessage + cnt), nTextLength);
    cnt += nTextLength;

    nCharacter = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    nChapter = *reinterpret_cast<INT*>(pByteMessage + cnt);
    cnt += sizeof(INT);

    nTime = *reinterpret_cast<LONG*>(pByteMessage + cnt);
    cnt += sizeof(LONG);

    nIndex = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    m_bInOnJournalAnnounce = TRUE;
    g_pBaldurChitin->GetObjectGame()->GetJournal()->InsertEntryAfter(szText,
        nIndex,
        nChapter,
        nTime,
        nCharacter,
        0);
    m_bInOnJournalAnnounce = FALSE;

    return TRUE;
}

// 0x439D00
BOOLEAN CBaldurMessage::SendJournalEntryChangeToServer(CString szText, BYTE nCharacter, INT nChapter, DWORD nIndex)
{
    CString sHostName;
    DWORD nTextLength;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    nTextLength = szText.GetLength();
    dwSize = sizeof(DWORD)
        + nTextLength
        + sizeof(BYTE)
        + sizeof(INT)
        + sizeof(DWORD);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nTextLength;
    cnt += sizeof(DWORD);

    memcpy(pByteMessage + cnt, szText, nTextLength);
    cnt += nTextLength;

    *reinterpret_cast<INT*>(pByteMessage + cnt) = nChapter;
    cnt += sizeof(INT);

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nIndex;
    cnt += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_JOURNAL,
        MSG_SUBTYPE_JOURNAL_CHANGE_ENTRY,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x439EE0
BOOLEAN CBaldurMessage::OnSendEntryJournalChange(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    DWORD nTextLength;
    BYTE nCharacter;
    INT nChapter;
    DWORD nIndex;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nTextLength = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    CString szText(reinterpret_cast<char*>(pByteMessage + cnt), nTextLength);
    cnt += nTextLength;

    nCharacter = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    nChapter = *reinterpret_cast<INT*>(pByteMessage + cnt);
    cnt += sizeof(INT);

    nIndex = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    g_pBaldurChitin->GetObjectGame()->GetJournal()->ChangeEntry(nIndex,
        szText,
        nChapter,
        nCharacter);

    return TRUE;
}

// 0x439FB0
BOOLEAN CBaldurMessage::AnnounceJournalEntryChange(CString szText, BYTE nCharacter, INT nChapter, DWORD nIndex)
{
    DWORD nTextLength;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    nTextLength = szText.GetLength();
    dwSize = sizeof(DWORD)
        + nTextLength
        + sizeof(BYTE)
        + sizeof(INT)
        + sizeof(DWORD);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nTextLength;
    cnt += sizeof(DWORD);

    memcpy(pByteMessage + cnt, szText, nTextLength);
    cnt += nTextLength;

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nCharacter;
    cnt += sizeof(BYTE);

    *reinterpret_cast<INT*>(pByteMessage + cnt) = nChapter;
    cnt += sizeof(INT);

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nIndex;
    cnt += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_JOURNAL,
        MSG_SUBTYPE_JOURNAL_ANNOUNCE_CHANGE,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x43A140
BOOLEAN CBaldurMessage::OnAnnounceJournalEntryChange(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    DWORD nTextLength;
    BYTE nCharacter;
    INT nChapter;
    DWORD nIndex;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nTextLength = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    CString szText(reinterpret_cast<char*>(pByteMessage + cnt), nTextLength);
    cnt += nTextLength;

    nCharacter = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    nChapter = *reinterpret_cast<INT*>(pByteMessage + cnt);
    cnt += sizeof(INT);

    nIndex = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    m_bInOnJournalAnnounce = TRUE;
    g_pBaldurChitin->GetObjectGame()->GetJournal()->ChangeEntry(nIndex,
        szText,
        nChapter,
        nCharacter);
    m_bInOnJournalAnnounce = FALSE;

    return TRUE;
}

// 0x43A910
BOOLEAN CBaldurMessage::SendBiographyToServer(BYTE nFixedPartyId, CString szBio)
{
    CString sHostName;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;
    DWORD nTextLength;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    nTextLength = szBio.GetLength();
    dwSize = nTextLength
        + sizeof(DWORD)
        + sizeof(BYTE);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nTextLength;
    cnt += sizeof(DWORD);

    memcpy(pByteMessage + cnt, szBio.GetBuffer(nTextLength), nTextLength);
    cnt += nTextLength;

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nFixedPartyId;
    cnt += sizeof(BYTE);

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_BIOGRAPHY,
        MSG_SUBTYPE_BIOGRAPHY_CHANGE,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x43AAD0
BOOLEAN CBaldurMessage::OnSendBiography(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    DWORD nTextLength;
    BYTE nFixedPartyId;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nTextLength = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    CString szText(reinterpret_cast<char*>(pByteMessage + cnt), nTextLength);
    cnt += nTextLength;

    nFixedPartyId = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    g_pBaldurChitin->GetObjectGame()->ChangeBiography(nFixedPartyId, szText);

    return TRUE;
}

// 0x43AB90
BOOLEAN CBaldurMessage::AnnounceBiography(BYTE nFixedPartyId, CString szBio)
{
    CString sHostName;
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;
    DWORD nTextLength;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    nTextLength = szBio.GetLength();
    dwSize = nTextLength
        + sizeof(DWORD)
        + sizeof(BYTE);

    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = nTextLength;
    cnt += sizeof(DWORD);

    memcpy(pByteMessage + cnt, szBio.GetBuffer(nTextLength), nTextLength);
    cnt += nTextLength;

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nFixedPartyId;
    cnt += sizeof(BYTE);

    g_pChitin->cNetwork.SendSpecificMessage(CString(""),
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
        MSG_TYPE_BIOGRAPHY,
        MSG_SUBTYPE_BIOGRAPHY_CHANGE_ANNOUNCE,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x43AD80
BOOLEAN CBaldurMessage::OnAnnounceBiography(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    DWORD cnt;
    DWORD nTextLength;
    BYTE nFixedPartyId;

    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    nTextLength = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    CString szBio(reinterpret_cast<char*>(pByteMessage + cnt), nTextLength);
    cnt += nTextLength;

    nFixedPartyId = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    m_bInOnAnnounceBiography = TRUE;

    g_pBaldurChitin->GetObjectGame()->ChangeBiography(nFixedPartyId, szBio);

    m_bInOnAnnounceBiography = FALSE;

    return TRUE;
}

// FIXME: `sAreaString` should be reference.
//
// 0x43B030
void CBaldurMessage::SetDeleteAreaString(CString sAreaString)
{
    m_sDeleteAreaString = sAreaString;
}

// 0x43B4C0
CString CBaldurMessage::GetDeleteAreaString()
{
    return m_sDeleteAreaString;
}

// 0x43B780
BOOLEAN CBaldurMessage::CancelDeleteAreaPoll()
{
    BYTE cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    m_sDeleteAreaString = "";
    m_bDeleteAreaPolling = FALSE;
    m_dwDeleteAreaTimeout = 0;

    for (cnt = 0; cnt < 6; cnt++) {
        m_pnDeleteAreaVoters[cnt] = 0;
        m_pnDeleteAreaVotes[cnt] = DELETEAREA_EMPTY_VOTE;
    }

    return TRUE;
}

// 0x43B980
BOOLEAN CBaldurMessage::DeleteAreaBallotResult()
{
    BYTE cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    for (cnt = 0; cnt < CMultiplayerSettings::MAX_PLAYERS; cnt++) {
        if (cnt != g_pChitin->cNetwork.m_nLocalPlayer) {
            if (m_pnDeleteAreaVoters[cnt] != g_pChitin->cNetwork.GetPlayerID(cnt)) {
                return FALSE;
            }
        }
    }

    BOOLEAN bAggregateVote = TRUE;

    for (cnt = 0; cnt < CMultiplayerSettings::MAX_PLAYERS; cnt++) {
        if (cnt != g_pChitin->cNetwork.m_nLocalPlayer) {
            if (m_pnDeleteAreaVoters[cnt] != 0
                && m_pnDeleteAreaVotes[cnt] != DELETEAREA_YES_VOTE) {
                bAggregateVote = FALSE;
            }
        }
    }

    return bAggregateVote;
}

// 0x43BA50
BOOLEAN CBaldurMessage::MultiplayerSynchRequest()
{
    CString sHostName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetHostPlayerName(sHostName);

    if (sHostName == "") {
        return FALSE;
    }

    g_pChitin->cNetwork.SendSpecificMessage(sHostName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSYNCH,
        MSG_SUBTYPE_MPSYNCH_REQUEST,
        NULL,
        0);

    return TRUE;
}

// 0x43BB60
BOOLEAN CBaldurMessage::OnMultiplayerSynchRequest(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    m_nMultiplayerSynchClientLocation = static_cast<BYTE>(nMsgFrom);
    m_bMultiplayerSynchClientPending = FALSE;
    m_bMultiplayerSynchServerPending = TRUE;
    m_bMultiplayerSynchServerFinished = FALSE;

    // FIXME: Global.
    g_pBaldurChitin->GetBaldurMessage()->RequestClientSignal(SIGNAL_MPSYNCH);

    return TRUE;
}

// 0x43BBC0
BOOLEAN CBaldurMessage::MultiplayerSynchReply()
{
    CString sPlayerName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetPlayerName(m_nMultiplayerSynchClientLocation, sPlayerName);

    if (sPlayerName == "") {
        return FALSE;
    }

    g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
        CNetwork::SEND_GUARANTEED,
        MSG_TYPE_MPSYNCH,
        MSG_SUBTYPE_MPSYNCH_REPLY,
        NULL,
        0);

    m_bMultiplayerSynchServerPending = FALSE;
    m_bMultiplayerSynchServerFinished = FALSE;
    m_nMultiplayerSynchClientLocation = -1;

    return TRUE;
}

// 0x43BCD0
BOOLEAN CBaldurMessage::OnMultiplayerSynchReply(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    m_bMultiplayerSynchClientFinished = TRUE;

    return TRUE;
}

// FIXME: `sPlayerName` should be reference.
//
// 0x43BD00
BOOLEAN CBaldurMessage::VersionServer(CString sPlayerName)
{
    BYTE* pByteMessage;
    DWORD dwSize;
    DWORD cnt;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    CString sLocalPlayerName;

    sLocalPlayerName = g_pChitin->cNetwork.m_sLocalPlayerName;
    if (sPlayerName == sLocalPlayerName) {
        return FALSE;
    }

    CString sFinalVersion;
    CString sVersion(CChitin::m_sVersionNumber);
    CString sBuild(CChitin::m_sBuildNumber);

    INT nMajor;
    INT nMinor;
    sscanf(sVersion, "%d, %d", &nMajor, &nMinor);

    sFinalVersion.Format("v%d.%d.%s", nMajor, nMinor, (LPCSTR)sBuild);

    BYTE nFinalVersionLength = static_cast<BYTE>(sFinalVersion.GetLength());

    dwSize = sizeof(BYTE)
        + sizeof(BYTE)
        + nFinalVersionLength
        + sizeof(BYTE)
        + sizeof(DWORD);
    pByteMessage = CreateBuffer(dwSize);
    if (pByteMessage == NULL) {
        return FALSE;
    }

    cnt = 0;

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = VERSION_NUM_FIELDS;
    cnt += sizeof(BYTE);

    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = nFinalVersionLength;
    cnt += sizeof(BYTE);

    memcpy(pByteMessage + cnt, sFinalVersion.GetBuffer(nFinalVersionLength), nFinalVersionLength);
    cnt += nFinalVersionLength;

    // Expansion pack.
    *reinterpret_cast<BYTE*>(pByteMessage + cnt) = 0;
    cnt += sizeof(BYTE);

    *reinterpret_cast<DWORD*>(pByteMessage + cnt) = CChitin::TIMER_UPDATES_PER_SECOND;
    cnt += sizeof(DWORD);

    g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
        CNetwork::SEND_GUARANTEED | CNetwork::SEND_JOINING_PLAYERS,
        MSG_TYPE_VERSION,
        MSG_SUBTYPE_VERSION_SERVER,
        pByteMessage,
        dwSize);

    DestroyBuffer(pByteMessage);

    return TRUE;
}

// 0x43BF80
BOOLEAN CBaldurMessage::OnVersionServer(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    CString sClientVersion;
    CString sVersion(CChitin::m_sVersionNumber);
    CString sBuild(CChitin::m_sBuildNumber);

    INT nMajor;
    INT nMinor;
    sscanf(sVersion, "%d, %d", &nMajor, &nMinor);

    sClientVersion.Format("v%d.%d.%s", nMajor, nMinor, (LPCSTR)sBuild);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    // Skip num fields.
    cnt++;

    BYTE nServerVersionLength = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    CString sServerVersion(reinterpret_cast<char*>(pByteMessage + cnt), nServerVersionLength);
    cnt += nServerVersionLength;

    if (sClientVersion != sServerVersion) {
        m_bVersionControlShutdown = TRUE;
        m_nVersionControlShutdownReason = 1;
        SetVersionControlShutdownServerString(sServerVersion);
        SetVersionControlShutdownClientString(sClientVersion);
        g_pChitin->cNetwork.CloseSession(TRUE);
        return FALSE;
    }

    BYTE nServerExpansion = *reinterpret_cast<BYTE*>(pByteMessage + cnt);
    cnt += sizeof(BYTE);

    if (nServerExpansion != 0) {
        CString sClientExpansion;
        CString sServerExpansion;

        m_bVersionControlShutdown = TRUE;
        m_nVersionControlShutdownReason = 2;

        sServerExpansion.Format("%d", nServerExpansion);
        sClientExpansion.Format("%d", 0);

        SetVersionControlShutdownServerString(sServerExpansion);
        SetVersionControlShutdownClientString(sClientExpansion);

        m_bVersionControlShutdown = TRUE;
        g_pChitin->cNetwork.CloseSession(TRUE);
        return FALSE;
    }

    DWORD nServerFrameRate = *reinterpret_cast<DWORD*>(pByteMessage + cnt);
    cnt += sizeof(DWORD);

    if (nServerFrameRate != CChitin::TIMER_UPDATES_PER_SECOND) {
        CString sClientFrameRate;
        CString sServerFrameRate;

        m_bVersionControlShutdown = TRUE;
        m_nVersionControlShutdownReason = 2;

        sServerFrameRate.Format("%d", nServerFrameRate);
        sClientFrameRate.Format("%d", CChitin::TIMER_UPDATES_PER_SECOND);

        SetVersionControlShutdownServerString(sServerFrameRate);
        SetVersionControlShutdownClientString(sClientFrameRate);

        m_bVersionControlShutdown = TRUE;
        g_pChitin->cNetwork.CloseSession(TRUE);
        return FALSE;
    }

    return TRUE;
}

// FIXME: `sValue` should be reference.
//
// 0x43C2F0
void CBaldurMessage::SetVersionControlShutdownServerString(CString sValue)
{
    m_sVersionControlShutdownServerString = sValue;
}

// FIXME: `sValue` should be reference.
//
// 0x43C340
void CBaldurMessage::SetVersionControlShutdownClientString(CString sValue)
{
    m_sVersionControlShutdownClientString = sValue;
}

// 0x43C390
BYTE* CBaldurMessage::PollSpecificMessageType(BYTE nMsgType, BYTE nMsgSubType, INT& nMsgFrom, DWORD& dwSize)
{
    for (INT nPlayerNumber = 0; nPlayerNumber < 6; nPlayerNumber++) {
        if (g_pChitin->cNetwork.GetPlayerID(nPlayerNumber) != 0) {
            CString sPlayerName;
            g_pChitin->cNetwork.GetPlayerName(nPlayerNumber, sPlayerName);

            if (g_pChitin->cNetwork.PeekSpecificMessage(sPlayerName, nMsgType, nMsgSubType) == TRUE) {
                nMsgFrom = nPlayerNumber;
                return g_pChitin->cNetwork.FetchSpecificMessage(sPlayerName,
                    nMsgType,
                    nMsgSubType,
                    dwSize);
            }
        }
    }

    return NULL;
}

// 0x43C4A0
void CBaldurMessage::HandleBlockingMessages()
{
    INT nMsgFrom;
    BYTE* pData;
    DWORD dwSize;

    m_bInHandleBlockingMessages = TRUE;

    pData = PollSpecificMessageType(MSG_TYPE_RESOURCE,
        MSG_SUBTYPE_RESOURCE_DEMAND,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnResourceDemanded(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_FULLDEMAND,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnSettingsFullDemanded(nMsgFrom);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_PLAYERCHAR,
        MSG_SUBTYPE_PLAYERCHAR_UPDATE_DEMAND,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnUpdateDemandCharacterSlot(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_PLAYERCHAR,
        MSG_SUBTYPE_PLAYERCHAR_DEMAND_SLOT,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnDemandCharacterSlot(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_SWAPITEM,
        MSG_SUBTYPE_SWAPITEM_REQUEST,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnSwapItemRequest(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_DIALOG,
        MSG_SUBTYPE_DIALOG_PERMIT_REQUEST,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnDialogRequestToServer(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_LEAVEAREALUA,
        MSG_SUBTYPE_LEAVEAREALUA_PERMIT_REQUEST,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnLeaveAreaLuaRequestToServer(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_LEAVEAREANAME,
        MSG_SUBTYPE_LEAVEAREANAME_PERMIT_REQUEST,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnLeaveAreaNameRequestToServer(nMsgFrom, pData, dwSize);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_MPSETTINGS,
        MSG_SUBTYPE_MPSETTINGS_DEMAND_NIGHTMAREMODE,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnDemandSettingsNightmareMode(nMsgFrom);
        delete pData;
    }

    pData = PollSpecificMessageType(MSG_TYPE_PLAYERCHAR,
        MSG_SUBTYPE_PLAYERCHAR_70,
        nMsgFrom,
        dwSize);
    if (pData != NULL) {
        OnDemandSettingsNightmareMode(nMsgFrom);
        delete pData;
    }

    m_bInHandleBlockingMessages = FALSE;
}

// 0x43C750
BOOLEAN CBaldurMessage::GetGameSpyQueryBasic(CString& sResponse)
{
    CString sGameName;
    CString sGameVersion;

    g_pChitin->GetGameSpyGameName(sGameName);
    sGameVersion = CChitin::m_sBuildNumber;

    CString sLocation;
    GetPrivateProfileStringA("GameSpy",
        "Location",
        "US",
        sLocation.GetBuffer(128),
        128,
        g_pBaldurChitin->GetIniFileName());
    sLocation.ReleaseBuffer();

    sResponse.Format("\\gamename\\%s\\gamever\\%s\\location\\%s",
        sGameName,
        sGameVersion,
        sLocation);

    return TRUE;
}

// 0x43C860
BOOLEAN CBaldurMessage::GetGameSpyQueryInfo(CString& sResponse)
{
    CString sSessionName;
    CString sPortNumber;
    CString sIPAddress;
    CString sChapter;
    CString sResRef;
    CString sMapName;
    CString sGameType;
    CString sNumPlayers;
    CString sNumCharacters;
    CString sMaxCharacters;
    CString sMaxPlayers;
    CString sGameMode;

    sSessionName = g_pChitin->cNetwork.m_sSessionNameToMake;

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sSessionName);

    sPortNumber = "5123";

    // NOTE: Uninline.
    INT nChapter = g_pBaldurChitin->GetObjectGame()->GetCurrentChapter();

    sResRef = "chapters";
    CList<DWORD, DWORD>* pList = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetChapterText(CResRef(sResRef), nChapter);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CBaldurMessage.cpp
    // __LINE__: 16551
    UTIL_ASSERT(pList != NULL);

    if (pList->GetCount() > 0) {
        sChapter = CBaldurEngine::FetchString(pList->GetHead());
    }

    delete pList;

    sMapName = sChapter;

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sMapName);

    sGameType = "cooperative";

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sGameType);

    INT nNumCharacters = 0;
    for (INT nCharacter = 0; nCharacter < CMultiplayerSettings::MAX_CHARACTERS; nCharacter++) {
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterId(nCharacter) != CGameObjectArray::INVALID_INDEX) {
            nNumCharacters++;
        }
    }

    sNumCharacters.Format("%d", nNumCharacters);
    sMaxCharacters.Format("%d", CMultiplayerSettings::MAX_CHARACTERS);

    INT nNumPlayers = 0;
    for (INT nPlayer = 0; nPlayer < CMultiplayerSettings::MAX_PLAYERS; nPlayer) {
        if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0) {
            nNumPlayers++;
        }
    }

    sNumPlayers.Format("%d", nNumPlayers);
    sMaxPlayers.Format("%d", CMultiplayerSettings::MAX_PLAYERS);

    if (g_pChitin->cNetwork.m_bAllowNewConnections == TRUE) {
        sGameMode = "openplaying";
    } else {
        sGameMode = "closedplaying";
    }

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sGameMode);

    sResponse.Format("\\hostname\\%s"
                     "\\hostport\\%s"
                     "\\mapname\\%s"
                     "\\gametype\\%s"
                     "\\numcharacters\\%s"
                     "\\maxcharacters\\%s"
                     "\\numplayers\\%s"
                     "\\maxplayers\\%s"
                     "\\gamemode\\%s",
        sSessionName,
        sPortNumber,
        sMapName,
        sGameType,
        sNumCharacters,
        sMaxCharacters,
        sNumPlayers,
        sMaxPlayers,
        sGameMode);

    return TRUE;
}

// 0x43CC90
BOOLEAN CBaldurMessage::GetGameSpyQueryRules(CString& sResponse)
{
    CString sPassword;
    CString sPasswordProtection;
    CString sFrameRate;
    CString sReputation;
    CString sDifficulty;
    CString sModifyCharacters;
    CString sBuyAndSell;
    CString sAreaTransition;
    CString sDialog;
    CString sViewCharacters;
    CString sPausing;
    CString sLeader;
    CString sImportXP;
    CString sImportItems;

    sPassword = g_pChitin->cNetwork.m_sSessionPassword;
    sPasswordProtection.Format("%d", sPassword != "");
    sFrameRate.Format("%d", CChitin::TIMER_UPDATES_PER_SECOND);

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sFrameRate);

    SHORT nReputation = g_pBaldurChitin->GetObjectGame()->GetReputation();
    STRREF strReputation = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetReputationDescription(nReputation);
    sReputation = CBaldurEngine::FetchString(strReputation);

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sReputation);

    STRREF strDifficulty;
    if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nMPDifficultyMultiplier <= -50) {
        strDifficulty = 11308;
    } else if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nMPDifficultyMultiplier <= -25) {
        strDifficulty = 11309;
    } else if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nMPDifficultyMultiplier <= 0) {
        strDifficulty = 11311;
    } else if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nMPDifficultyMultiplier <= 50) {
        strDifficulty = 11312;
    } else {
        strDifficulty = 11313;
    }
    sDifficulty = CBaldurEngine::FetchString(strDifficulty);

    // NOTE: Uninline.
    GameSpyRemoveBadCharacters(sDifficulty);

    BYTE nImportingBitField = g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_nImportingBitField;
    sImportXP.Format("%d", (nImportingBitField & CMultiplayerSettings::IMPORT_EXPERIENCE) != 0);
    sImportItems.Format("%d", (nImportingBitField & CMultiplayerSettings::IMPORT_ITEMS) != 0);

    for (BYTE nPermission = 0; nPermission < CGamePermission::TOTAL_PERMISSIONS; nPermission) {
        BOOLEAN bPermission = g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetPermission(-1, nPermission);
        switch (nPermission) {
        case 0:
            sBuyAndSell.Format("%d", bPermission != FALSE);
            break;
        case 1:
            sAreaTransition.Format("%d", bPermission != FALSE);
            break;
        case 2:
            sDialog.Format("%d", bPermission != FALSE);
            break;
        case 3:
            sViewCharacters.Format("%d", bPermission != FALSE);
            break;
        case 4:
            sPausing.Format("%d", bPermission != FALSE);
            break;
        case 6:
            sLeader.Format("%d", bPermission != FALSE);
            break;
        case 7:
            sModifyCharacters.Format("%d", bPermission != FALSE);
            break;
        }
    }

    sResponse.Format("\\PasswordProtection\\%s"
                     "\\FrameRate\\%s"
                     "\\PartyReputation\\%s"
                     "\\DifficultyLevel\\%s"
                     "\\Import:XP\\%s"
                     "\\Import:Items\\%s"
                     "\\Permit:ModifyCharacters\\%s"
                     "\\Permit:Buy&Sell\\%s"
                     "\\Permit:AreaTransition\\%s"
                     "\\Permit:Dialog\\%s"
                     "\\Permit:ViewCharacters\\%s"
                     "\\Permit:Pausing\\%s"
                     "\\Permit:Leader\\%s",
        sPasswordProtection,
        sFrameRate,
        sReputation,
        sDifficulty,
        sImportXP,
        sImportItems,
        sModifyCharacters,
        sBuyAndSell,
        sAreaTransition,
        sDialog,
        sViewCharacters,
        sPausing,
        sLeader);

    return TRUE;
}

// 0x43D120
BOOLEAN CBaldurMessage::GetGameSpyQueryPlayers(CString& sResponse)
{
    CString sName;
    CString sRace;
    CString sClass;
    CString sLevel;
    CString sPlayerName;
    CString string;
    INT nCharacter;
    INT nPlayer;
    INT nCharacters = 0;

    sResponse = "";

    for (nCharacter = 0; nCharacter < CMultiplayerSettings::MAX_CHARACTERS; nCharacter++) {
        sName = "";
        sRace = "";
        sClass = "";
        sLevel = "";
        sPlayerName = "";

        if (g_pBaldurChitin->GetObjectGame()->GetGameSpyCharacterInformation(nCharacter, sName, sRace, sClass, sLevel) == TRUE) {
            nPlayer = g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetCharacterControlledByPlayer(nCharacter);
            g_pChitin->cNetwork.GetPlayerName(nPlayer, sPlayerName);

            // NOTE: Uninline.
            GameSpyRemoveBadCharacters(sName);

            // NOTE: Uninline.
            GameSpyRemoveBadCharacters(sRace);

            // NOTE: Uninline.
            GameSpyRemoveBadCharacters(sClass);

            // NOTE: Uninline.
            GameSpyRemoveBadCharacters(sLevel);

            // NOTE: Uninline.
            GameSpyRemoveBadCharacters(sPlayerName);

            string.Format("\\player_%d\\%s\\team_%d\\%s\\race_%d\\%s\\class_%d\\%s\\level_%d\\%s",
                nCharacters,
                sName,
                sPlayerName,
                sRace,
                sClass,
                sLevel);

            sResponse += string;

            nCharacters++;
        }
    }

    if (nCharacters == 0) {
        sResponse.Format("\\player_0\\No Game Loaded");
    }

    return TRUE;
}

// NOTE: Inlined.
void CBaldurMessage::GameSpyRemoveBadCharacters(CString& sString)
{
    for (int index = 0; index < sString.GetLength(); index++) {
        if (sString[index] == '\\') {
            sString.SetAt(index, '/');
        }
    }
}

// 0x43D3D0
BOOL CBaldurMessage::SendSettingsNightmareMode(const CString& sPlayerName)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    BOOL bNightmareMode = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nNightmareMode;

    if (sPlayerName != "") {
        g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
            CNetwork::SEND_GUARANTEED,
            MSG_TYPE_MPSETTINGS,
            MSG_SUBTYPE_MPSETTINGS_NIGHTMAREMODE,
            &bNightmareMode,
            sizeof(bNightmareMode));
    } else {
        g_pChitin->cNetwork.SendSpecificMessage(CString(""),
            CNetwork::SEND_ALL_PLAYERS | CNetwork::SEND_GUARANTEED | CNetwork::SEND_JOINING_PLAYERS,
            MSG_TYPE_MPSETTINGS,
            MSG_SUBTYPE_MPSETTINGS_NIGHTMAREMODE,
            &bNightmareMode,
            sizeof(bNightmareMode));
    }

    return TRUE;
}

// 0x43D520
BOOLEAN CBaldurMessage::OnSettingsNightmareMode(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        return FALSE;
    }

    if (nMsgFrom != g_pChitin->cNetwork.GetHostPlayerID()) {
        return FALSE;
    }

    g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nNightmareMode = pByteMessage[CNetwork::SPEC_MSG_HEADER_LENGTH];

    return TRUE;
}

// 0x43D590
BOOLEAN CBaldurMessage::DemandSettingsNightmareMode(BOOLEAN wait)
{
    // TODO: Incomplete.

    return FALSE;
}

// 0x43D810
BOOL CBaldurMessage::OnDemandSettingsNightmareMode(INT nMsgFrom)
{
    CString sPlayerName;

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || !g_pChitin->cNetwork.GetSessionHosting()) {
        return FALSE;
    }

    g_pChitin->cNetwork.GetPlayerName(nMsgFrom, sPlayerName);
    SendSettingsNightmareMode(sPlayerName);

    return TRUE;
}

// 0x43DF40
BOOLEAN CBaldurMessage::OnMovieEndCredits(INT nMsgFrom, BYTE* pByteMessage, DWORD dwSize)
{
    CInfGame::byte_8E7528 = TRUE;
    g_pBaldurChitin->m_pEngineWorld->ReadyEndCredits(FALSE);
    return TRUE;
}

// 0x43DF60
BOOL CBaldurMessage::DisplayText(const CString& sName, const CString& sText, COLORREF rgbNameColor, COLORREF rgbTextColor, LONG lMarker, LONG caller, LONG target)
{
    if (g_pBaldurChitin->GetObjectGame()->m_bInLoadGame == TRUE) {
        return FALSE;
    }

    CMessageDisplayText* pMessage = new CMessageDisplayText(sName,
        sText,
        rgbNameColor,
        rgbTextColor,
        lMarker,
        caller,
        target);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return TRUE;
}

// 0x43E0E0
BOOL CBaldurMessage::DisplayTextRef(STRREF name, STRREF text, COLORREF rgbNameColor, COLORREF rgbTextColor, LONG lMarker, LONG caller, LONG target)
{
    if (g_pBaldurChitin->GetObjectGame()->m_bInLoadGame == TRUE) {
        return FALSE;
    }

    CMessageDisplayTextRef* pMessage = new CMessageDisplayTextRef(name,
        text,
        rgbNameColor,
        rgbTextColor,
        lMarker,
        caller,
        target);

    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return TRUE;
}

// 0x605AA0
CString CBaldurMessage::GetVersionControlShutdownServerString()
{
    return m_sVersionControlShutdownServerString;
}

// 0x605AD0
CString CBaldurMessage::GetVersionControlShutdownClientString()
{
    return m_sVersionControlShutdownClientString;
}

// 0x4F4F50
BOOLEAN CBaldurMessage::GetDialogRequestPending()
{
    return m_bDialogRequestPending;
}

// 0x4F4F50
BOOLEAN CBaldurMessage::GetDialogReplyReturned()
{
    return m_bDialogReplyReturned;
}

// 0x4F4F70
INT CBaldurMessage::GetDialogReplyUpdates()
{
    return m_nDialogReplyUpdates;
}

// 0x4F4F80
void CBaldurMessage::SetDialogReplyUpdates(INT nValue)
{
    m_nDialogReplyUpdates = nValue;
}

// -----------------------------------------------------------------------------

// 0x45DEA0
CMessage::CMessage(LONG caller, LONG target)
{
    m_sourceId = caller;
    m_targetId = target;
}

// 0x4051F0
CMessage::~CMessage()
{
}

// 0x405140
SHORT CMessage::GetCommType()
{
    // __FILE__: .\Include\CMessage.h
    // __LINE__: 965
    UTIL_ASSERT(FALSE);
}

// 0x405160
BYTE CMessage::GetMsgType()
{
    // __FILE__: .\Include\CMessage.h
    // __LINE__: 966
    UTIL_ASSERT(FALSE);
}

// 0x405180
BYTE CMessage::GetMsgSubType()
{
    // __FILE__: .\Include\CMessage.h
    // __LINE__: 967
    UTIL_ASSERT(FALSE);
}

// 0x4051A0
void CMessage::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    *pData = NULL;
    *dwSize = 0;
}

// 0x4051C0
BOOL CMessage::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    return FALSE;
}

// 0x4051D0
void CMessage::Run()
{
    // __FILE__: .\Include\CMessage.h
    // __LINE__: 972
    UTIL_ASSERT(FALSE);
}

// -----------------------------------------------------------------------------

// 0x4F7470
CMessageHandler::CMessageHandler()
{
    m_bLastArbitrationLockStatus = FALSE;
}

// 0x4F7490
CMessageHandler::~CMessageHandler()
{
    // TODO: Incomplete.
}

static BOOL Iwd2MessageRunHeldBack(BYTE subType);

// 0x4EE020
void CMessageHandler::AsynchronousUpdate()
{
    while (!m_messageList.IsEmpty()) {
        CMessage* pMsg = m_messageList.RemoveHead();
        if (pMsg != NULL) {
            if (!Iwd2MessageRunHeldBack(pMsg->GetMsgSubType())) {
                pMsg->Run();
            }
            delete pMsg;
        }
    }
}

// 0x4F7110
void CMessageHandler::PostAsynchronousUpdate()
{
    if (g_pChitin->field_1932 == TRUE) {
        return;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        g_pChitin->cGameSpy.Update();
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame == NULL) {
        return;
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        STRREF strError;
        if (g_pBaldurChitin->GetObjectGame()->m_bAnotherPlayerJoinedGame == TRUE
            && (g_pBaldurChitin->GetObjectGame()->CanSaveGame(strError, FALSE, FALSE) == TRUE
                || g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bArbitrationLockStatus == TRUE)) {
            if ((g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineLoad
                    && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCreateChar
                    && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineMultiPlayer
                    && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineSinglePlayer
                    && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineConnection)
                || (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineMultiPlayer
                    && g_pBaldurChitin->m_pEngineMultiPlayer->m_nLobbyMode != 1)) {
                g_pBaldurChitin->GetObjectGame()->MultiplayerSetCharacterCreationLocation();

                g_pBaldurChitin->GetObjectGame()->m_tempCursor = 4;

                g_pBaldurChitin->m_pEngineMultiPlayer->m_nLobbyMode = 1;
                g_pBaldurChitin->m_pEngineMultiPlayer->StartMultiPlayer(1);
                g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineMultiPlayer);

                g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetArbitrationLockStatus(1, 0);
            }

            if (g_pBaldurChitin->GetObjectGame()->m_bAnotherPlayerJoinedGame == TRUE) {
                g_pBaldurChitin->GetBaldurMessage()->SendFullSettingsToClients(CString(""));
                g_pBaldurChitin->GetBaldurMessage()->UpdateDemandCharacters(1, 0, 0);
            } else {
                if (!m_bLastArbitrationLockStatus
                    && g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bArbitrationLockStatus != TRUE) {
                    g_pBaldurChitin->GetBaldurMessage()->UpdateDemandCharacters(1, 0, 0);
                }
            }

            g_pBaldurChitin->GetObjectGame()->m_bAnotherPlayerJoinedGame = FALSE;
        }
    }

    m_bLastArbitrationLockStatus = g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bArbitrationLockStatus;

    if (g_pBaldurChitin->GetBaldurMessage()->m_bMultiplayerSessionShutdown == TRUE) {
        g_pBaldurChitin->GetBaldurMessage()->m_bMultiplayerSessionShutdown = FALSE;
        g_pChitin->cNetwork.CloseSession(TRUE);
    }

    if (g_pBaldurChitin->GetBaldurMessage()->m_bPlayerShutdown == TRUE) {
        if (g_pBaldurChitin->GetObjectGame()->m_bGameLoaded) {
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineConnection);

            if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                g_pBaldurChitin->m_pEngineConnection->ShowSessionTerminatedMessage();
            }

            if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
                && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
                g_pBaldurChitin->GetObjectGame()->SaveGame(0, 0, 1);
            }

            g_pBaldurChitin->GetObjectGame()->DestroyGame(TRUE, FALSE);
        }

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pChitin->cNetwork.CloseSession(TRUE);
        }

        g_pBaldurChitin->GetBaldurMessage()->m_bPlayerShutdown = FALSE;
    }
}

// 0x4F73F0
BOOL CMessageHandler::ImportantMessage(BYTE* pData, DWORD dwSize)
{
    if (pData[CNetwork::SPEC_MSG_FLAG] == CNetwork::SPEC_MSG_FLAG_ENABLED
        && pData[CNetwork::SPEC_MSG_TYPE] == 67) {
        switch (pData[CNetwork::SPEC_MSG_SUBTYPE]) {
        case 1:
        case 5:
        case 8:
        case 12:
        case 18:
        case 27:
        case 28:
        case 39:
        case 40:
        case 44:
        case 45:
        case 52:
        case 57:
        case 58:
            return TRUE;
        }
    }

    return FALSE;
}

// 0x4F7500
SHORT CMessageHandler::AddMessage(CMessage* message, BOOL bForcePassThrough)
{
    return AddMessage(message, bForcePassThrough, message->GetCommType());
}

// 0x4F7530
SHORT CMessageHandler::AddMessage(CMessage* message, BOOL bForcePassThrough, SHORT nCommType)
{
    switch (nCommType) {
    case CMessage::SEND:
        return Send(message);
    case CMessage::BROADCAST:
        if (bForcePassThrough) {
            m_messageList.AddTail(message);
            return 1;
        }

        return Broadcast(message, TRUE, FALSE);
    case CMessage::BROADCAST_OTHERS:
        if (bForcePassThrough) {
            m_messageList.AddTail(message);
            return 1;
        }

        return Broadcast(message, FALSE, FALSE);
    case CMessage::BROADCAST_FORCED:
        if (bForcePassThrough) {
            m_messageList.AddTail(message);
            return 1;
        }

        return Broadcast(message, TRUE, TRUE);
    case CMessage::BROADCAST_FORCED_OTHERS:
        if (bForcePassThrough) {
            m_messageList.AddTail(message);
            return 1;
        }

        return Broadcast(message, FALSE, TRUE);
    default:
        return -1;
    }
}

// The binary's drain (0x4EE083-0x4EE0AB) calls Run() unconditionally -- it
// dereferences the message at 0x4EE093 before any test, and evaluates no
// predicate between RemoveHead and Run.  This list is ours, not the engine's:
// it withholds the subtypes whose Run() we have not yet cleared to execute,
// and it shrinks to nothing as they are cleared, at which point both it and
// its call site go away.
//
// A subtype that is absent from this list runs, which is the binary's
// behaviour.  Every subtype the message system can deliver has been audited
// against the binary, so absence here is a decision, not an oversight.
static BOOL Iwd2MessageRunHeldBack(BYTE subType)
{
    // SAFE -- faithful body, recovered callees, but never observed reaching
    // the drain on any save we can drive; released once one does.
    return subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_92
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_ACTIONS
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_CHANGE
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_STATUS
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_STATUS
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FADE_COLOR
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PROJECTILE_TRAILING_VFX
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_ACTIVE
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_EXPLORED
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_COMMAND_PAUSE
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DRAW_POLY
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_FORCE_ACTION_PICK
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_LAST_OBJECT
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_FOLLOW
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_SCROLL
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_ACTIONS
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_FOLLOW
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TOGGLE_INTERFACE
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_UNLOCK
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VERBAL_CONSTANT

    // DIVERGENT -- body knowingly differs from the binary; fix before releasing
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ANIMATION_CHANGE
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_RESET
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_ITEMS
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SCREENSHAKE

    // CALLS-STUB -- Run() is faithful, its payload callee is still unimplemented
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_MOVE_GLOBAL

    // NULLDEREF -- Run() casts the target to CGameTrigger with no type check
    // and writes m_dwFlags / m_trapActivated / m_trapDetected at +0x5D6,
    // +0x612 and +0x614, which is faithful.  The problem is the caller set:
    // CGameContainer::AddEffect posts one targeting the container itself (the
    // binary installs the same vtable at 0x848C3C and stores the container id
    // in both the source and target slots), and CGameContainer puts
    // m_pileVidCell[3] at +0x5D4, so the write lands on a live CVidCell that
    // is rendered every frame.  A scan for the +0x5D6 displacement finds only
    // CGameTrigger methods, so the container has nothing meaningful there.
        || subType == CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TRIGGER_STATUS

;
}

// 0x4F7620
SHORT CMessageHandler::Broadcast(CMessage* message, BOOLEAN bSendMessageToSelf, BOOLEAN bIgnoreObjectControl)
{
    LONG targetId = message->m_targetId;

    CGameObject* pObject;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        pObject = NULL;
    }

    // Object-control gate: send over the wire only for an object this station
    // owns, unless the caller waives the check.  Both arms of the binary's
    // branch run the same queue/delete tail, so it is hoisted below.
    if (bIgnoreObjectControl == TRUE
        || (pObject != NULL
            && (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                || g_pChitin->cNetwork.m_idLocalPlayer == pObject->m_remotePlayerID))) {
        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
            && g_pChitin->cNetwork.GetSessionOpen() == TRUE
            && (bIgnoreObjectControl == TRUE
                || (pObject != NULL
                    && g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
                    && pObject->m_bLocalControl == 0))) {
            BYTE* pData = STATICBUFFER;
            DWORD dwSize = 0;
            message->MarshalMessage(&pData, &dwSize);

            if (dwSize != 0) {
                CString sPlayerName;
                g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
                    CNetwork::SEND_GUARANTEED | CNetwork::SEND_ALL_PLAYERS,
                    message->GetMsgType(),
                    message->GetMsgSubType(),
                    pData,
                    dwSize);
            }
        }
    }

    if (bSendMessageToSelf == TRUE) {
        m_messageList.AddTail(message);
    } else {
        delete message;
    }

    if (rc == CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    return 0;
}

// 0x4F7830
SHORT CMessageHandler::Send(CMessage* message)
{
    // NOTE: The binary reloads message->m_targetId for the ReleaseShare calls
    // below, which reads freed memory on the path that deletes the message
    // first.  The value cannot change, so it is captured once here instead.
    LONG targetId = message->m_targetId;

    if (targetId != -1) {
        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(targetId,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            BOOLEAN bDeleteMessage = FALSE;

            if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                || g_pChitin->cNetwork.m_idLocalPlayer == pObject->m_remotePlayerID) {
                // The target is ours: run the message locally.
                m_messageList.AddTail(message);
            } else {
                CString sPlayerName;
                BYTE* pData = STATICBUFFER;
                DWORD dwSize = 0;

                bDeleteMessage = TRUE;
                message->MarshalMessage(&pData, &dwSize);

                if (dwSize != 0) {
                    PLAYER_ID idOwner = pObject->m_remotePlayerID;
                    INT nPlayer = g_pChitin->cNetwork.FindPlayerLocationByID(idOwner, FALSE);

                    if (nPlayer == -1) {
                        // The owner is not a connected player.  An area id means
                        // the object belongs to an area nobody is holding, so the
                        // message is dropped; otherwise it is relayed to the host.
                        if (g_pBaldurChitin->GetObjectGame()->FindAreaID(idOwner) == TRUE) {
                            delete message;
                        } else {
                            PLAYER_ID idHost = g_pChitin->cNetwork.m_nHostPlayer == -1
                                ? 0
                                : g_pChitin->cNetwork.m_pPlayerID[g_pChitin->cNetwork.m_nHostPlayer];
                            nPlayer = g_pChitin->cNetwork.FindPlayerLocationByID(idHost, FALSE);
                        }

                        if (nPlayer == -1) {
                            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(targetId,
                                CGameObjectArray::THREAD_ASYNCH,
                                INFINITE);
                            return -1;
                        }
                    }

                    g_pChitin->cNetwork.GetPlayerName(nPlayer, sPlayerName);

                    BYTE nSubType = message->GetMsgSubType();
                    BYTE nType = message->GetMsgType();

                    if (g_pChitin->cNetwork.SendSpecificMessage(sPlayerName,
                            CNetwork::SEND_GUARANTEED,
                            nType,
                            nSubType,
                            pData,
                            dwSize)
                            == FALSE
                        && g_pChitin->cNetwork.m_idLocalPlayer == idOwner) {
                        m_messageList.AddTail(message);
                        bDeleteMessage = FALSE;
                    }
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            if (bDeleteMessage) {
                delete message;
            }

            return 0;
        }
    }

    delete message;
    return -1;
}

// -----------------------------------------------------------------------------

// 0x4F4F90
CMessageAddAction::CMessageAddAction(const CAIAction& action, LONG caller, LONG target)
    : CMessage(caller, target)
{
    // NOTE: Uninline.
    m_action = action;
}

// 0x405240
CMessageAddAction::~CMessageAddAction()
{
}

// 0x40A0D0
SHORT CMessageAddAction::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageAddAction::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x405210
BYTE CMessageAddAction::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_ACTION;
}

// 0x4F83C0
void CMessageAddAction::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->AddAction(m_action);
            pSprite->m_interrupt = TRUE;
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }}

// -----------------------------------------------------------------------------

// 0x4F51D0
CMessageAnimationChange::CMessageAnimationChange(WORD animationId, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_animationId = animationId;
}

// 0x4641A0
CMessageAnimationChange::~CMessageAnimationChange()
{
}

// 0x40A0D0
SHORT CMessageAnimationChange::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageAnimationChange::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4B4FF0
BYTE CMessageAnimationChange::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ANIMATION_CHANGE;
}

// 0x4F93B0
void CMessageAnimationChange::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 5756
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG) + sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 5777
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_animationId;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 5799
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4F94E0
BOOL CMessageAnimationChange::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 5827
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_animationId = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 5853
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4F9590
void CMessageAnimationChange::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->UnequipAll(TRUE);
            if (pSprite->GetArea() != NULL) {
                // The guard reads the animation's intrinsic list type
                // (CGameAnimationType vtable +0x3C), not the object's current
                // m_listType.
                if (pSprite->GetAnimation()->GetListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.RemoveObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                                  pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->m_bBumpable,
                        pSprite->m_bOnSearchMap);
                }
            }
            pSprite->GetAnimation()->SetAnimationType(m_animationId,
                pSprite->GetBaseStats()->m_colors,
                pSprite->GetDirection());
            if (pSprite->GetArea() != NULL) {
                // 0x4F964B / 0x4F9748: the guard reads the animation's intrinsic
                // list type (CGameAnimationType vtable +0x3C), not the object's
                // current m_listType.
                if (pSprite->GetAnimation()->GetListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.AddObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                               pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->m_bBumpable,
                        pSprite->m_bOnSearchMap);
                }
            }
            pSprite->EquipAll(TRUE);
            pSprite->JumpToPoint(pSprite->GetPos(), TRUE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4533D0
CMessageAddEffect::CMessageAddEffect(CGameEffect* effect, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_effect = effect;
}

// 0x453410
CMessageAddEffect::~CMessageAddEffect()
{
    if (m_effect != NULL) {
        delete m_effect;
        m_effect = NULL;
    }
}

// 0x40A0D0
SHORT CMessageAddEffect::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageAddEffect::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4533C0
BYTE CMessageAddEffect::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ADD_EFFECT;
}

// 0x4F8CA0
void CMessageAddEffect::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->AddEffect(m_effect, CGameAIBase::EFFECT_LIST_TIMED, FALSE, TRUE);
            m_effect = NULL;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5200
CMessageChangeDirection::CMessageChangeDirection(SHORT face, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_face = face;
}

// 0x453510
SHORT CMessageChangeDirection::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageChangeDirection::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5230
BYTE CMessageChangeDirection::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CHANGE_DIRECTION;
}

// 0x4F9890
void CMessageChangeDirection::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 5951
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG) + sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 5972
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_face;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 5994
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4F99C0
BOOL CMessageChangeDirection::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6022
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_face = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6048
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4F9A70
void CMessageChangeDirection::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->SetDirection(m_face);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5240
CMessageClearActions::CMessageClearActions(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x40A0D0
SHORT CMessageClearActions::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageClearActions::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4637A0
BYTE CMessageClearActions::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_ACTIONS;
}

// 0x4F9B10
void CMessageClearActions::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6119
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6139
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6159
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4F9C30
BOOL CMessageClearActions::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6187
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6209
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4F9CD0
void CMessageClearActions::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->ClearActions(FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5260
CMessageClearDialogActions::CMessageClearDialogActions(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x40A0D0
SHORT CMessageClearDialogActions::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageClearDialogActions::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5280
BYTE CMessageClearDialogActions::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_DIALOG_ACTIONS;
}

// 0x4F9D70
void CMessageClearDialogActions::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6281
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6301
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6321
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4F9E90
BOOL CMessageClearDialogActions::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6349
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6371
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4F9F30
void CMessageClearDialogActions::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->ClearDialogActions();
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5290
CMessageClearGroupSlot::CMessageClearGroupSlot(SHORT slotNum, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_slotNum = slotNum;
}

// 0x453510
SHORT CMessageClearGroupSlot::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageClearGroupSlot::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F52C0
BYTE CMessageClearGroupSlot::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_DIALOG_ACTIONS;
}

// 0x4F9FD0
void CMessageClearGroupSlot::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6444
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6449
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<SHORT*>(*pData + cnt) = m_slotNum;
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6466
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FA060
BOOL CMessageClearGroupSlot::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6490
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_slotNum = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6498
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FA0C0
void CMessageClearGroupSlot::Run()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->GetGameSave()->m_groupInventory[m_slotNum] != NULL) {
        delete pGame->GetGameSave()->m_groupInventory[m_slotNum];
        pGame->GetGameSave()->m_groupInventory[m_slotNum] = NULL;
    }
}

// -----------------------------------------------------------------------------

// 0x4F52D0
CMessageClearTriggers::CMessageClearTriggers(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x40A0D0
SHORT CMessageClearTriggers::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageClearTriggers::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F52F0
BYTE CMessageClearTriggers::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CLEAR_TRIGGERS;
}

// 0x4FA110
void CMessageClearTriggers::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6559
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6579
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6599
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FA230
BOOL CMessageClearTriggers::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6627
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6649
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FA2D0
void CMessageClearTriggers::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->ClearTriggers();
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5300
CMessageColorChange::CMessageColorChange(BYTE* colors, LONG caller, LONG target)
    : CMessage(caller, target)
{
    for (int index = 0; index < 7; index++) {
        m_colors[index] = colors[index];
    }
}

// 0x4088A0
SHORT CMessageColorChange::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageColorChange::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5340
BYTE CMessageColorChange::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_CHANGE;
}

// 0x4FA370
void CMessageColorChange::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6721
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + 7;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6743
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        memcpy(*pData + cnt, m_colors, 7);
        cnt += 7;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6766
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FA4B0
BOOL CMessageColorChange::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6794
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    memcpy(m_colors, pData + cnt, 7);
    cnt += 7;

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6819
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FA570
void CMessageColorChange::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            for (int index = 0; index < 7; index++) {
                pSprite->GetBaseStats()->m_colors[index] = m_colors[index];

                // NOTE: Uninline.
                pSprite->GetAnimation()->SetColorRange(index, m_colors[index]);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5350
CMessageColorReset::CMessageColorReset(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x4088A0
SHORT CMessageColorReset::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageColorReset::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5370
BYTE CMessageColorReset::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_RESET;
}

// 0x4FA680
void CMessageColorReset::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6896
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pSprite->m_remotePlayerID;
        LONG remoteObjectID = pSprite->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6916
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 6936
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FA7A0
BOOL CMessageColorReset::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6964
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 6986
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FA840
void CMessageColorReset::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            BYTE* colors = pSprite->GetBaseStats()->m_colors;
            for (int index = 0; index < 7; index++) {
                // NOTE: Uninline.
                pSprite->GetAnimation()->SetColorRange(index, colors[index]);
            }

            // The binary's loop bound is 8, not 7: it runs an eighth range
            // whose value is the byte after m_colors[6], i.e. m_effectVersion.
            // NOTE: Uninline.
            pSprite->GetAnimation()->SetColorRange(7, pSprite->GetBaseStats()->m_effectVersion);

            // NOTE: Uninline.
            pSprite->GetAnimation()->ClearColorEffectsAll();
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined
CMessageContainerAddItem::CMessageContainerAddItem(const CItem& item, SHORT slotNum, BOOLEAN bCompressContainer, LONG caller, LONG target)
    : CMessage(caller, target)
{
    //
    m_item = item;
    m_slotNum = slotNum;
    m_bCompressContainer = bCompressContainer;
}

// 0x47DFC0
CMessageContainerAddItem::~CMessageContainerAddItem()
{
}

// 0x40A0D0
SHORT CMessageContainerAddItem::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageContainerAddItem::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47DF90
BYTE CMessageContainerAddItem::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_ADD_ITEM;
}

// 0x4FB230
void CMessageContainerAddItem::Run()
{
    CGameContainer* pContainer;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pContainer->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            CItem* pItem;

            if (m_item.cResRef != "") {
                pItem = new CItem(m_item);
            } else {
                pItem = NULL;
            }

            pContainer->SetItem(m_slotNum, pItem);

            if (m_bCompressContainer) {
                pContainer->CompressContainer();
            } else {
                CMessageContainerItems* pMessage = new CMessageContainerItems(pContainer,
                    pContainer->GetId(),
                    pContainer->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4FB3C0
CMessageContainerItems::CMessageContainerItems(CGameContainer* pContainer, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pContainer != NULL && pContainer->m_lstItems.GetCount() != 0) {
        m_nItems = pContainer->m_lstItems.GetCount();
        m_pItems = new CItem*[m_nItems];

        for (SHORT cnt = 0; cnt < m_nItems; cnt++) {
            m_pItems[cnt] = pContainer->GetItem(cnt);
        }
    } else {
        m_nItems = NULL;
        m_pItems = NULL;
    }
}

// 0x4FB4B0
CMessageContainerItems::~CMessageContainerItems()
{
    for (SHORT cnt = 0; cnt < m_nItems; cnt++) {
        m_pItems[cnt] = NULL;
    }

    if (m_pItems != NULL) {
        delete m_pItems;
    }
}

// 0x4088A0
SHORT CMessageContainerItems::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageContainerItems::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4FB480
BYTE CMessageContainerItems::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_ITEMS;
}

// 0x4FBA30
void CMessageContainerItems::Run()
{
    CGameContainer* pContainer;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        SHORT maxcnt = max(m_nItems, pContainer->m_lstItems.GetCount());
        for (SHORT cnt = 0; cnt < maxcnt; cnt++) {
            CItem* oldItem = pContainer->GetItem(cnt);
            if (cnt < m_nItems) {
                pContainer->SetItem(cnt, m_pItems[cnt]);
            } else {
                pContainer->SetItem(cnt, NULL);
            }

            if (oldItem != NULL && oldItem != m_pItems[cnt]) {
                g_pBaldurChitin->GetObjectGame()->AddDisposableItem(oldItem);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4FA960
CMessageColorUpdate::CMessageColorUpdate(CGameSprite* pSprite, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_appliedColorRanges = pSprite->GetDerivedStats()->m_appliedColorRanges;
    m_appliedColorEffects = pSprite->GetDerivedStats()->m_appliedColorEffects;
}

// 0x4F6FC0
CMessageColorUpdate::~CMessageColorUpdate()
{
}

// 0x453510
SHORT CMessageColorUpdate::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageColorUpdate::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4FAA10
BYTE CMessageColorUpdate::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_COLOR_UPDATE;
}

// 0x4FAA40
void CMessageColorUpdate::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 7084
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        BYTE* ranges;
        ULONG rangeSize = m_appliedColorRanges.Marshal(&ranges);

        BYTE* effects;
        DWORD effectSize = m_appliedColorEffects.Marshal(&effects);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(ULONG)
            + rangeSize
            + sizeof(ULONG)
            + effectSize;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 7116
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<ULONG*>(*pData + cnt) = rangeSize;
        cnt += sizeof(ULONG);

        memcpy(*pData + cnt, ranges, rangeSize);
        cnt += rangeSize;

        *reinterpret_cast<ULONG*>(*pData + cnt) = effectSize;
        cnt += sizeof(ULONG);

        memcpy(*pData + cnt, effects, effectSize);
        cnt += effectSize;

        if (ranges != NULL) {
            delete ranges;
        }

        if (effects != NULL) {
            delete effects;
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 7158
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FAC10
BOOL CMessageColorUpdate::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 7186
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    ULONG rangeSize = *reinterpret_cast<ULONG*>(pData + cnt);
    cnt += sizeof(ULONG);

    m_appliedColorRanges.Unmarshal(pData + cnt, rangeSize);
    cnt += rangeSize;

    ULONG effectSize = *reinterpret_cast<ULONG*>(pData + cnt);
    cnt += sizeof(ULONG);

    m_appliedColorEffects.Unmarshal(pData + cnt, effectSize);
    cnt += effectSize;

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 7223
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FACF0
void CMessageColorUpdate::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            BYTE* colors = pSprite->GetBaseStats()->m_colors;
            for (int index = 0; index < 7; index++) {
                // NOTE: Uninline.
                pSprite->GetAnimation()->SetColorRange(index, colors[index]);
            }

            m_appliedColorRanges.Apply(pSprite);
            m_appliedColorEffects.Apply(pSprite);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5380
CMessageContainerStatus::CMessageContainerStatus(DWORD flags, WORD trapActivated, WORD trapDetected, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_dwFlags = flags;
    m_trapActivated = trapActivated;
    m_trapDetected = trapDetected;
}

// 0x43E170
SHORT CMessageContainerStatus::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageContainerStatus::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47D7A0
BYTE CMessageContainerStatus::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTAINER_STATUS;
}

// 0x4FBB50
void CMessageContainerStatus::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8053
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(remoteObjectID)
            + sizeof(LONG)
            + sizeof(DWORD)
            + sizeof(WORD)
            + sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 8076
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_dwFlags;
        cnt += sizeof(DWORD);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_trapActivated;
        cnt += sizeof(WORD);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_trapDetected;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 8106
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FBC90
BOOL CMessageContainerStatus::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8134
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_dwFlags = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    m_trapActivated = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    m_trapDetected = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8167
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FBD50
void CMessageContainerStatus::Run()
{
    CGameContainer* pContainer;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pContainer->SetFlags(m_dwFlags);
        pContainer->SetTrapActivated(m_trapActivated);
        pContainer->SetTrapDetected(m_trapDetected);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F53C0
CMessageCutSceneModeStatus::CMessageCutSceneModeStatus(BOOLEAN mode, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_cutSceneMode = mode;
}

// 0x43E170
SHORT CMessageCutSceneModeStatus::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageCutSceneModeStatus::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453470
BYTE CMessageCutSceneModeStatus::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CUT_SCENE_MODE_STATUS;
}

// 0x4FBE00
void CMessageCutSceneModeStatus::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8247
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(remoteObjectID)
            + sizeof(LONG)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 8268
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_cutSceneMode;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 8106
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FBF30
BOOL CMessageCutSceneModeStatus::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8319
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_cutSceneMode = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8351
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FBFD0
void CMessageCutSceneModeStatus::Run()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->GetGameSave()->m_mode == 1282) {
        pGame->GetGameSave()->m_cutScene = FALSE;

        // NOTE: Unused.
        CString sError("Trying to start a cut scene while in a dialog.");
    } else {
        if (m_cutSceneMode) {
            CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;
            if (pWorld->m_bPaused == TRUE
                && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
                if (!pWorld->TogglePauseGame(0, 1, 0)) {
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
                    // __LINE__: 8396
                    UTIL_ASSERT(FALSE);
                }
            }

            pGame->GetGameSave()->m_mode = 322;
            pGame->GetGameSave()->m_cutScene = TRUE;

            g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);

            pGame->SetTempCursor(4);
            pGame->GetVisibleArea()->m_nScrollState = 0;

            g_pBaldurChitin->GetActiveEngine()->SelectEngine(pWorld);
        } else {
            if (pGame->GetGameSave()->m_mode == 322) {
                pGame->GetGameSave()->m_mode = -1;

                g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(1)->SetActive(TRUE);
                g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(1)->InvalidateRect(NULL);
            }
            pGame->GetGameSave()->m_cutScene = FALSE;
        }
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CMessageDisplayText::CMessageDisplayText(const CString& name, const CString& text, COLORREF nameColor, COLORREF textColor, LONG marker, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_name = name;
    m_text = text;
    m_nameColor = nameColor;
    m_textColor = textColor;
    m_marker = marker;
    m_moveToTop = FALSE;
}

// 0x43E090
CMessageDisplayText::~CMessageDisplayText()
{
}

// 0x43E170
SHORT CMessageDisplayText::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageDisplayText::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x43E060
BYTE CMessageDisplayText::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXT;
}

// 0x4FC4F0
void CMessageDisplayText::Run()
{
    g_pBaldurChitin->m_pEngineWorld->DisplayText(m_name,
        m_text,
        m_nameColor,
        m_textColor,
        m_marker,
        m_moveToTop);
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CMessageDisplayTextRef::CMessageDisplayTextRef(STRREF name, STRREF text, COLORREF nameColor, COLORREF textColor, LONG marker, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_name = name;
    m_text = text;
    m_nameColor = nameColor;
    m_textColor = textColor;
    m_marker = marker;
    m_moveToTop = FALSE;
}

// 0x43E170
SHORT CMessageDisplayTextRef::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageDisplayTextRef::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x43E180
BYTE CMessageDisplayTextRef::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXTREF;
}

// 0x4FC520
void CMessageDisplayTextRef::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8677
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(STRREF)
        + sizeof(STRREF)
        + sizeof(COLORREF)
        + sizeof(COLORREF)
        + sizeof(LONG)
        + sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8705
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<STRREF*>(*pData + cnt) = m_name;
    cnt += sizeof(STRREF);

    *reinterpret_cast<STRREF*>(*pData + cnt) = m_text;
    cnt += sizeof(STRREF);

    *reinterpret_cast<COLORREF*>(*pData + cnt) = m_nameColor;
    cnt += sizeof(COLORREF);

    *reinterpret_cast<COLORREF*>(*pData + cnt) = m_textColor;
    cnt += sizeof(COLORREF);

    *reinterpret_cast<LONG*>(*pData + cnt) = m_marker;
    cnt += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_moveToTop;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8746
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FC670
BOOL CMessageDisplayTextRef::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8774
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_name = *reinterpret_cast<STRREF*>(pData + cnt);
    cnt += sizeof(STRREF);

    m_text = *reinterpret_cast<STRREF*>(pData + cnt);
    cnt += sizeof(STRREF);

    m_nameColor = *reinterpret_cast<COLORREF*>(pData + cnt);
    cnt += sizeof(COLORREF);

    m_textColor = *reinterpret_cast<COLORREF*>(pData + cnt);
    cnt += sizeof(COLORREF);

    m_marker = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_moveToTop = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8819
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FC740
void CMessageDisplayTextRef::Run()
{
    STR_RES strResName;
    STR_RES strResText;
    CString sName("");

    BYTE rc;
    CGameSprite* pSprite;

    if (m_name >= -7 && m_name != -1) {
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(-2 - m_name);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            sName = pSprite->GetName();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(m_name, strResName);
        sName = strResName.szText;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(m_text, strResText);

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (strResText.cSound.GetRes() != NULL) {
            if (strResText.cSound.m_nLooping == 0) {
                strResText.cSound.SetFireForget(TRUE);
            }

            strResText.cSound.SetChannel(14, reinterpret_cast<DWORD>(pSprite->GetArea()));

            // FIXME: Calls `GetPos` twice.
            strResText.cSound.Play(pSprite->GetPos().x,
                pSprite->GetPos().y,
                0,
                FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    g_pBaldurChitin->m_pEngineWorld->DisplayText(sName,
        strResText.szText,
        m_nameColor,
        m_textColor,
        m_marker,
        m_moveToTop);
}

// -----------------------------------------------------------------------------

// 0x4FD370
CMessageDoorStatus::CMessageDoorStatus(CGameDoor* pDoor, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pDoor != NULL) {
        m_bOpen = (pDoor->m_dwFlags & 0x1) != 0;
        m_dwFlags = pDoor->m_dwFlags;
        m_trapDetected = pDoor->m_trapDetected;
        m_trapActivated = pDoor->m_trapActivated;
    } else {
        m_bOpen = FALSE;
        m_dwFlags = 0;
        m_trapDetected = 0;
        m_trapActivated = 0;
    }
}

// 0x43E170
SHORT CMessageDoorStatus::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageDoorStatus::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4FD3D0
BYTE CMessageDoorStatus::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_STATUS;
}

// 0x4FD3E0
void CMessageDoorStatus::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9469
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(remoteObjectID)
            + sizeof(LONG)
            + sizeof(BOOLEAN)
            + sizeof(DWORD)
            + sizeof(WORD)
            + sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 9493
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bOpen;
        cnt += sizeof(BOOLEAN);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_dwFlags;
        cnt += sizeof(DWORD);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_trapActivated;
        cnt += sizeof(WORD);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_trapDetected;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 9525
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FD530
BOOL CMessageDoorStatus::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9553
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_bOpen = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    m_dwFlags = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    m_trapActivated = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    m_trapDetected = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9588
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FD5F0
void CMessageDoorStatus::Run()
{
    CGameDoor* pDoor;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pDoor),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pDoor->OnDoorStatusUpdate(m_bOpen,
            m_dwFlags,
            m_trapActivated,
            m_trapDetected);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F54C0
CMessageEnterDialog::CMessageEnterDialog(DWORD entryNum, BOOLEAN a2, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_entryNum = entryNum;
    field_10 = a2;
}

// 0x453510
SHORT CMessageEnterDialog::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageEnterDialog::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x43E170
BYTE CMessageEnterDialog::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ENTER_DIALOG;
}

// 0x4FE100
void CMessageEnterDialog::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 10139
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(DWORD)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 10161
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_entryNum;
        cnt += sizeof(DWORD);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = field_10;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 10189
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x4FE230
BOOL CMessageEnterDialog::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 10217
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        localObjectID = CGameObjectArray::INVALID_INDEX;
    }

    m_targetId = localObjectID;

    m_entryNum = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    field_10 = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 10248
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FE2E0
void CMessageEnterDialog::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.EnterDialog(m_entryNum,
                static_cast<CGameSprite*>(pObject),
                field_10);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4BD900
CMessageDropPath::CMessageDropPath(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x453510
SHORT CMessageDropPath::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageDropPath::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4B34E0
BYTE CMessageDropPath::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DROP_PATH;
}

// 0x4FD6A0
void CMessageDropPath::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9662
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 9682
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // NOTE: Missing trailing control seen in other functions.
    } else {
        *dwSize = 0;
    }
}

// 0x4FD7A0
BOOL CMessageDropPath::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9733
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing control seen in other functions.

    return TRUE;
}

// 0x4FD830
void CMessageDropPath::Run()
{
    CGameSprite* pSprite;


    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->DropPath();
            pSprite->DropSearchRequest();

            if (pSprite->m_nSequence == CGameSprite::SEQ_WALK) {
                pSprite->SetSequence(CGameSprite::SEQ_READY);
            }

            // NOTE: Uninline.
            if (pSprite->InControl()) {
                pSprite->m_bSendSpriteUpdate = TRUE;
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// FIXME: `cResStore` should be reference.
//
// 0x4F5520
CMessageEnterStoreMode::CMessageEnterStoreMode(const CAIObjectType& cAIProprietor, const CAIObjectType& cAICustomer, const CResRef cResStore, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_cAIProprietor.Set(cAIProprietor);
    m_cAICustomer.Set(cAICustomer);
    m_cResStore = cResStore;
}

// 0x4F5630
CMessageEnterStoreMode::~CMessageEnterStoreMode()
{
}

// 0x4536E0
SHORT CMessageEnterStoreMode::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageEnterStoreMode::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5600
BYTE CMessageEnterStoreMode::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ENTER_STORE_MODE;
}

// 0x4FF480
void CMessageEnterStoreMode::Run()
{
    g_pBaldurChitin->m_pEngineWorld->StartStore(m_cAIProprietor,
        m_cAICustomer,
        m_cResStore,
        FALSE);
}

// -----------------------------------------------------------------------------

// 0x4F56E0
CMessageExitDialogMode::CMessageExitDialogMode(BOOLEAN bButtonPushed, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bButtonPushed = bButtonPushed;
}

// 0x4536E0
SHORT CMessageExitDialogMode::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageExitDialogMode::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5700
BYTE CMessageExitDialogMode::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_EXIT_DIALOG_MODE;
}

// 0x4FF970
void CMessageExitDialogMode::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11426
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(BOOLEAN)
        + sizeof(unsigned char);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11454
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bButtonPushed;
    cnt += sizeof(BOOLEAN);

    *reinterpret_cast<unsigned char*>(*pData + cnt) = g_pBaldurChitin->m_pEngineWorld->field_1204;
    cnt += sizeof(unsigned char);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11485
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FFAB0
BOOL CMessageExitDialogMode::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11513
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_bButtonPushed = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    g_pBaldurChitin->m_pEngineWorld->field_1204 = *reinterpret_cast<unsigned char*>(pData + cnt);
    cnt += sizeof(unsigned char);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11548
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FFB70
void CMessageExitDialogMode::Run()
{
    g_pBaldurChitin->m_pEngineWorld->m_bDialogButtonClicked = 0;
    g_pBaldurChitin->m_pEngineWorld->EndDialog(1, 1);
    g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_bDialogActive = 0;
}

// -----------------------------------------------------------------------------

// 0x4F5710
CMessageExitStoreMode::CMessageExitStoreMode(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x4536E0
SHORT CMessageExitStoreMode::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageExitStoreMode::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5730
BYTE CMessageExitStoreMode::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_EXIT_STORE_MODE;
}

// 0x4FFBB0
void CMessageExitStoreMode::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11617
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11640
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11660
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FFCD0
BOOL CMessageExitStoreMode::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11688
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11711
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FFD70
void CMessageExitStoreMode::Run()
{
    g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
    g_pBaldurChitin->m_pEngineWorld->StopStore();
}

// -----------------------------------------------------------------------------

// 0x4F5830
CMessageInsertAction::CMessageInsertAction(const CAIAction& action, LONG caller, LONG target)
    : CMessage(caller, target)
{
    // NOTE: Uninline.
    m_action = action;
}

// 0x45BD40
CMessageInsertAction::~CMessageInsertAction()
{
}

// 0x40A0D0
SHORT CMessageInsertAction::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageInsertAction::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x45BD10
BYTE CMessageInsertAction::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_INSERT_ACTION;
}

// 0x501730
void CMessageInsertAction::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->InsertAction(m_action);
            pSprite->m_interrupt = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F59D0 (out-of-line ctor; binary also inlines this at FUN_00485750 / FUN_00462f90).
CMessageInsertResponse::CMessageInsertResponse(const CAIResponse& response,
    int checkCurrentResponse,
    int clearActions,
    int field38,
    LONG caller,
    LONG target)
    : CMessage(caller, target)
{
    m_response.Set(response);
    m_checkCurrentResponse = checkCurrentResponse;
    m_clearActions = clearActions;
    field_38 = field38;
}

// 0x463290
CMessageInsertResponse::~CMessageInsertResponse()
{
}

// 0x40A0D0
SHORT CMessageInsertResponse::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageInsertResponse::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x463280
BYTE CMessageInsertResponse::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_INSERT_RESPONSE;
}

// 0x5017E0
void CMessageInsertResponse::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // TODO: marshal m_response + 3 trailing ints. Binary at 0x5017E0.
    (void)pData;
    (void)dwSize;
}

// 0x501FE0
BOOL CMessageInsertResponse::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // TODO: unmarshal companion to MarshalMessage. Binary at 0x501FE0.
    (void)pData;
    (void)dwSize;
    return FALSE;
}

// 0x502570
void CMessageInsertResponse::Run()
{
    CGameAIBase* pSprite;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
        // TODO: when field_38 == 1 the binary at 0x5025F0..0x5026F0 walks
        // m_response.m_actionList, copies each CAIAction with the sprite's
        // own AIObjectType slots overlaid (m_aiObjectType / m_lastObject /
        // ...), then calls InsertResponse for each. Skipped: that path
        // exists for scripted FireSpell-style responses where the caster's
        // object refs need to be substituted in; the simpler unconditional
        // InsertResponse below still applies the response, just without
        // the per-action object rewrite.

        if (m_response.m_actionList.GetCount() > 0) {
            pSprite->InsertResponse(m_response,
                m_checkCurrentResponse,
                m_clearActions);
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// -----------------------------------------------------------------------------

// 0x4F5A60
CMessageLeaveParty::CMessageLeaveParty(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x40A0D0
SHORT CMessageLeaveParty::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageLeaveParty::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5A80
BYTE CMessageLeaveParty::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_LEAVE_PARTY;
}

// 0x5028E0
void CMessageLeaveParty::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14116
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14139
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14159
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x502A00
BOOL CMessageLeaveParty::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14187
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14210
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x502AA0
void CMessageLeaveParty::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->LeaveParty();
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5A90
CMessageLoadDialog::CMessageLoadDialog(const CString& dialog, LONG npcId, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_dialogRes = dialog;
    m_NPCId = npcId;
}

// 0x43E170
SHORT CMessageLoadDialog::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageLoadDialog::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5B10
BYTE CMessageLoadDialog::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_LOAD_DIALOG;
}

// 0x502B40
void CMessageLoadDialog::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // TODO: marshal m_dialogRes + m_NPCId. Binary at 0x502B40.
    (void)pData;
    (void)dwSize;
}

// 0x502D60
BOOL CMessageLoadDialog::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // TODO: unmarshal companion. Binary at 0x502D60.
    (void)pData;
    (void)dwSize;
    return FALSE;
}

// 0x502EB0
void CMessageLoadDialog::Run()
{
    if (m_targetId == CGameObjectArray::INVALID_INDEX
        || m_NPCId == CGameObjectArray::INVALID_INDEX) {
        return;
    }

    CGameSprite* pTarget;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pTarget),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameSprite* pNpc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_NPCId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pNpc),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // Binary at 0x502EB0 also calls CAIObjectType::Set on both
            // sprites here before releasing the NPC lock; skipped pending
            // CAIObjectType recovery.
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_NPCId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            CResRef cResRef;
            cResRef = m_dialogRes;
            g_pBaldurChitin->GetScreenWorld()->GetInternalLoadedDialog()->Initialize(cResRef,
                m_targetId,
                m_NPCId);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5B90
CMessagePartyGold::CMessagePartyGold(BOOLEAN bFeedback, BOOLEAN bAdj, LONG gold, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bFeedback = bFeedback;
    m_bAdjustment = bAdj;
    m_gold = gold;
}

// 0x43E170
SHORT CMessagePartyGold::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessagePartyGold::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x465190
BYTE CMessagePartyGold::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PARTY_GOLD;
}

// 0x503040
void CMessagePartyGold::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14529
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(LONG)
        + sizeof(BYTE)
        + sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14534
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<LONG*>(*pData + cnt) = m_gold;
    cnt += sizeof(LONG);

    *reinterpret_cast<BYTE*>(*pData + cnt) = m_bAdjustment;
    cnt += sizeof(BYTE);

    *reinterpret_cast<BYTE*>(*pData + cnt) = m_bFeedback;
    cnt += sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14557
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5030E0
BOOL CMessagePartyGold::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14582
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_gold = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_bAdjustment = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    m_bFeedback = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14596
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x503150
void CMessagePartyGold::Run()
{
    if (m_bAdjustment == TRUE) {
        if (m_bFeedback) {
            if (!g_pChitin->cNetwork.GetSessionOpen()
                || g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
                g_pBaldurChitin->GetObjectGame()->FeedBack(CInfGame::FEEDBACK_GOLD,
                    m_gold,
                    TRUE);
            }
        }

        if (m_gold < 0) {
            // NOTE: Unsigned compare.
            if (static_cast<DWORD>(-m_gold) > g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold) {
                g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold = 0;
            } else {
                g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold += m_gold;
            }
        } else {
            g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold += m_gold;
        }
    } else {
        g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_nPartyGold = m_gold;
    }

    CBaldurEngine* pActiveEngine = g_pBaldurChitin->GetActiveEngine();
    if (pActiveEngine != NULL) {
        pActiveEngine->UpdatePartyGoldStatus();
    }
}

// -----------------------------------------------------------------------------

// 0x4F5BC0
CMessagePlaySound::CMessagePlaySound(BYTE soundId, BOOL text, BOOL circle, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_soundId = soundId;
    m_showText = text;
    m_showCircle = circle;
}

// 0x43E170
SHORT CMessagePlaySound::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessagePlaySound::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4A8DE0
BYTE CMessagePlaySound::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PLAY_SOUND;
}

// 0x503210
void CMessagePlaySound::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14673
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BYTE)
            + sizeof(BOOLEAN)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 14694
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = m_soundId;
        cnt += sizeof(BYTE);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_showText == TRUE;
        cnt += sizeof(BOOLEAN);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_showCircle == TRUE;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 14743
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x503360
BOOL CMessagePlaySound::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14773
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_soundId = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    m_showText = *reinterpret_cast<BOOLEAN*>(pData + cnt) == TRUE;
    cnt += sizeof(BOOLEAN);

    m_showCircle = *reinterpret_cast<BOOLEAN*>(pData + cnt) == TRUE;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14822
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x503430
void CMessagePlaySound::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->PlaySound(m_soundId, m_showText, m_showCircle, FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4A9AD0
CMessagePlaySoundRef::CMessagePlaySoundRef(CResRef cResSound, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_cResSound = cResSound;
    m_nChannel = 14;
    m_bPositioned = TRUE;
}

// 0x4536E0
SHORT CMessagePlaySoundRef::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessagePlaySoundRef::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x48AC70
BYTE CMessagePlaySoundRef::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PLAY_SOUND_REF;
}

// 0x5034E0
void CMessagePlaySoundRef::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    CString sResRef;

    // FIXME: Unused.
    CAIObjectType v1;

    PLAYER_ID remotePlayerID = 0;
    LONG remoteObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14894
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    m_cResSound.CopyToString(sResRef);
    BYTE stringLen = static_cast<BYTE>(sResRef.GetLength());

    *dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(BYTE)
        + stringLen
        + sizeof(BYTE)
        + sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14923
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<BYTE*>(*pData + cnt) = stringLen;
    cnt += sizeof(BYTE);

    memcpy(*pData + cnt, sResRef.GetBuffer(sResRef.GetLength()), sResRef.GetLength());
    cnt += sResRef.GetLength();

    *reinterpret_cast<BYTE*>(*pData + cnt) = m_nChannel;
    cnt += sizeof(BYTE);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bPositioned;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14956
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5036E0
BOOL CMessagePlaySoundRef::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // FIXME: Unused.
    CString v1;
    CAIObjectType v2;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 14993
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    BYTE stringLen = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    m_cResSound = CString(reinterpret_cast<char*>(pData + cnt), stringLen);
    cnt += stringLen;

    m_nChannel = *reinterpret_cast<BYTE*>(pData + cnt) == TRUE;
    cnt += sizeof(BYTE);

    m_bPositioned = *reinterpret_cast<BOOLEAN*>(pData + cnt) == TRUE;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15034
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x503880
void CMessagePlaySoundRef::Run()
{
    CSound cSound;
    cSound.SetResRef(m_cResSound, TRUE, TRUE);

    if (cSound.m_nLooping == 0) {
        cSound.SetFireForget(TRUE);
    }

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        cSound.SetChannel(m_nChannel,
            reinterpret_cast<DWORD>(pObject->GetArea()));
        cSound.Play(pObject->GetPos().x,
            pObject->GetPos().y,
            0,
            FALSE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        cSound.SetChannel(14,
            reinterpret_cast<DWORD>(g_pBaldurChitin->GetObjectGame()->GetVisibleArea()));
        cSound.Play(FALSE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5CA0
CMessageRemoveReplies::CMessageRemoveReplies(LONG entryIndex, LONG marker, COLORREF nameColor, const CString& name, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_entryIndex = entryIndex;
    m_marker = marker;
    m_nameColor = nameColor;
    m_name = name;
}

// 0x484570
CMessageRemoveReplies::~CMessageRemoveReplies()
{
}

// 0x43E170
SHORT CMessageRemoveReplies::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageRemoveReplies::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x484540
BYTE CMessageRemoveReplies::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_REMOVE_REPLIES;
}

// 0x504740
void CMessageRemoveReplies::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15506
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        SHORT stringLen = static_cast<SHORT>(m_name.GetLength());

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(SHORT)
            + stringLen
            + sizeof(LONG)
            + sizeof(LONG)
            + sizeof(COLORREF);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 15531
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<SHORT*>(*pData + cnt) = stringLen;
        cnt += sizeof(SHORT);

        memcpy(*pData + cnt, m_name.GetBuffer(stringLen), stringLen);
        cnt += stringLen;

        *reinterpret_cast<LONG*>(*pData + cnt) = m_entryIndex;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_marker;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_nameColor;
        cnt += sizeof(COLORREF);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 15571
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5048E0
BOOL CMessageRemoveReplies::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15599
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        localObjectID = CGameObjectArray::INVALID_INDEX;
    }

    m_targetId = localObjectID;

    SHORT stringLen = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    m_name = CString(reinterpret_cast<char*>(pData + cnt), stringLen);
    cnt += stringLen;

    m_entryIndex = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_marker = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_nameColor = *reinterpret_cast<COLORREF*>(pData + cnt);
    cnt += sizeof(COLORREF);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15641
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x504A00
void CMessageRemoveReplies::Run()
{
    g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_dialogEntries[m_entryIndex]->RemoveReplies(m_marker,
        m_nameColor,
        m_name);
}

// -----------------------------------------------------------------------------

// 0x4F5400 (out-of-line ctor; binary also inlines this in
// CGameDialogSprite::AsynchronousUpdate at 0x4841a2..0x484248).
CMessageContinueDialog::CMessageContinueDialog(LONG entryIndex,
    LONG marker,
    COLORREF nameColor,
    const CString& name,
    LONG sourceSpriteId,
    const CString& nextDialog,
    LONG talkerIndex,
    LONG characterIndex,
    LONG nextEntryIndex,
    BYTE flag,
    LONG caller,
    LONG target)
    : CMessage(caller, target)
{
    m_name = name;
    m_entryIndex = entryIndex;
    m_marker = marker;
    m_nameColor = nameColor;
    m_sourceSpriteId = sourceSpriteId;
    m_nextDialog = nextDialog;
    m_talkerIndex = talkerIndex;
    m_characterIndex = characterIndex;
    m_nextEntryIndex = nextEntryIndex;
    m_flag = flag;
}

// 0x4844D0
CMessageContinueDialog::~CMessageContinueDialog()
{
}

// 0x40A0D0
SHORT CMessageContinueDialog::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageContinueDialog::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4844C0
BYTE CMessageContinueDialog::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CONTINUE_DIALOG;
}

// 0x4FD930
void CMessageContinueDialog::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // TODO: marshal layout per binary at 0x4FD930; mirrors the CMessage
    // remote-id + CString pattern used in CMessageRemoveReplies but with
    // the extra ResRef/index pair.
    (void)pData;
    (void)dwSize;
}

// 0x4FDD30
BOOL CMessageContinueDialog::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // TODO: unmarshal companion. Binary at 0x4FDD30.
    (void)pData;
    (void)dwSize;
    return FALSE;
}

// 0x4FDF80
void CMessageContinueDialog::Run()
{
    // Repost three smaller messages that together drive the state
    // transition on the receiving side: clear the reply list, load the new
    // dialog file under deny locks, then enter the new state by index.

    CMessageRemoveReplies* pRemove = new CMessageRemoveReplies(m_entryIndex,
        m_marker,
        m_nameColor,
        m_name,
        m_sourceSpriteId,
        m_sourceSpriteId);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pRemove, FALSE);

    CMessageLoadDialog* pLoad = new CMessageLoadDialog(m_nextDialog,
        m_talkerIndex,
        m_characterIndex,
        m_characterIndex);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pLoad, FALSE);

    CMessageEnterDialog* pEnter = new CMessageEnterDialog(m_nextEntryIndex,
        m_flag,
        m_targetId,
        m_targetId);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pEnter, FALSE);
}

// -----------------------------------------------------------------------------

// 0x4F5D30
CMessageReputationChange::CMessageReputationChange(SHORT deltaRep, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_deltaRep = deltaRep;
}

// 0x4536E0
SHORT CMessageReputationChange::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageReputationChange::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5D60
BYTE CMessageReputationChange::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_REPUTATION_CHANGE;
}

// 0x504A30
void CMessageReputationChange::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15714
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 15735
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<SHORT*>(*pData + cnt) = m_deltaRep;
        cnt += sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 15758
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x504B60
BOOL CMessageReputationChange::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15786
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_deltaRep = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 15806
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x504C00
void CMessageReputationChange::Run()
{
    g_pBaldurChitin->GetBaldurMessage()->m_bInReputationChange = TRUE;
    g_pBaldurChitin->GetObjectGame()->ReputationAdjustment(m_deltaRep, FALSE);
    g_pBaldurChitin->GetBaldurMessage()->m_bInReputationChange = FALSE;
}

// -----------------------------------------------------------------------------

// 0x4534C0
CMessageScreenShake::CMessageScreenShake(WORD duration, CHAR magnitudeX, CHAR magnitudeY, BOOLEAN bOverride, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_duration = duration;
    m_magnitudeX = magnitudeX;
    m_magnitudeY = magnitudeY;
    m_bOverride = bOverride;
}

// 0x43E170
SHORT CMessageScreenShake::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageScreenShake::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453500
BYTE CMessageScreenShake::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SCREENSHAKE;
}

// 0x505020
void CMessageScreenShake::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16043
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(WORD)
        + sizeof(CHAR)
        + sizeof(CHAR)
        + sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16051
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<WORD*>(*pData + cnt) = m_duration;
    cnt += sizeof(WORD);

    *reinterpret_cast<CHAR*>(*pData + cnt) = m_magnitudeX;
    cnt += sizeof(CHAR);

    *reinterpret_cast<CHAR*>(*pData + cnt) = m_magnitudeY;
    cnt += sizeof(CHAR);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bOverride;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16077
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5050C0
BOOL CMessageScreenShake::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16100
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_duration = *reinterpret_cast<WORD*>(*pData + cnt);
    cnt += sizeof(WORD);

    m_magnitudeX = *reinterpret_cast<CHAR*>(*pData + cnt);
    cnt += sizeof(CHAR);

    m_magnitudeY = *reinterpret_cast<CHAR*>(*pData + cnt);
    cnt += sizeof(CHAR);

    m_bOverride = *reinterpret_cast<BOOLEAN*>(*pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16117
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x505140
void CMessageScreenShake::Run()
{
    if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bAllScreenShake) {
        // The binary jumps to the exit when a shake is already running: an
        // override always shakes, otherwise one in progress is left alone
        // rather than restarted.
        if (m_bOverride
            || !g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->GetInfinity()->m_bScreenShake) {
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->GetInfinity()->SetScreenShake(TRUE,
                m_duration,
                CPoint(m_magnitudeX, m_magnitudeY));
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4F5D70
CMessageSetActive::CMessageSetActive(BOOL active, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_active = active;
}

// 0x453510
SHORT CMessageSetActive::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetActive::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453520
BYTE CMessageSetActive::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_ACTIVE;
}

// 0x5051C0
void CMessageSetActive::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16189
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16210
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOL*>(*pData + cnt) = m_active;
        cnt += sizeof(BOOL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16233
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5052F0
BOOL CMessageSetActive::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16261
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_active = *reinterpret_cast<BOOL*>(pData + cnt);
    cnt += sizeof(BOOL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16287
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x5053A0
void CMessageSetActive::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->m_activeAI = m_active;
        } else if (pObject->GetObjectType() == CGameObject::TYPE_STATIC) {
            CGameStatic* pStatic = static_cast<CGameStatic*>(pObject);
            if (m_active) {
                pStatic->m_header.m_dwFlags |= 0x1;
            } else {
                pStatic->m_header.m_dwFlags &= ~0x1;
            }
        } else if (pObject->GetObjectType() == CGameObject::TYPE_SOUND) {
            CGameSound* pSound = static_cast<CGameSound*>(pObject);
            pSound->SetActive(m_active);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5D90
CMessageSetAISpeed::CMessageSetAISpeed(BYTE speed, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nAISpeed = speed;
}

// 0x453510
SHORT CMessageSetAISpeed::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetAISpeed::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5DB0
BYTE CMessageSetAISpeed::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AISPEED;
}

// 0x505490
void CMessageSetAISpeed::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16389
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16410
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = m_nAISpeed;
        cnt += sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16433
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5055C0
BOOL CMessageSetAISpeed::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16461
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_nAISpeed = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16487
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x505660
void CMessageSetAISpeed::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_AISpeed = m_nAISpeed;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5DC0
CMessageSetCommandPause::CMessageSetCommandPause(SHORT pause, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_commandPause = pause;
}

// 0x40A0D0
SHORT CMessageSetCommandPause::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetCommandPause::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x485A00
BYTE CMessageSetCommandPause::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_COMMAND_PAUSE;
}

// 0x505700
void CMessageSetCommandPause::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16559
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16580
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<SHORT*>(*pData + cnt) = m_commandPause;
        cnt += sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16603
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x505830
BOOL CMessageSetCommandPause::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16631
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_commandPause = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16657
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x5058E0
void CMessageSetCommandPause::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_nCommandPause = m_commandPause;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5EB0
CMessageSetDialogWait::CMessageSetDialogWait(LONG wait, LONG waitTarget, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_wait = wait;
    m_waitTarget = waitTarget;
}

// 0x40A0D0
SHORT CMessageSetDialogWait::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetDialogWait::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x485A10
BYTE CMessageSetDialogWait::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DIALOG_WAIT;
}

// 0x5062C0
void CMessageSetDialogWait::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_dialogWait = m_wait;
            pSprite->m_dialogWaitTarget = m_waitTarget;
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// FIXME: `face` should be reference.
//
// 0x4F5EE0
CMessageSetDirection::CMessageSetDirection(CPoint face, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_face = face;
}

// 0x453510
SHORT CMessageSetDirection::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetDirection::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5F10
BYTE CMessageSetDirection::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DIRECTION;
}

// 0x506370
void CMessageSetDirection::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17333
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17353
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_face.x;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_face.y;
        cnt += sizeof(LONG);

        // NOTE: Missing trailing guard.
    } else {
        *dwSize = 0;
    }
}

// 0x506490
BOOL CMessageSetDirection::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C17408Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16631
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // NOTE: Other functions set additional properties after validating
    // target.

    m_face.x = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_face.y = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x506530
void CMessageSetDirection::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->SetDirection(m_face);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x47D7B0
CMessageSetDrawPoly::CMessageSetDrawPoly(SHORT time, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_time = time;
}

// 0x453510
SHORT CMessageSetDrawPoly::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetDrawPoly::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47D7E0
BYTE CMessageSetDrawPoly::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DRAW_POLY;
}

// 0x5065D0
void CMessageSetDrawPoly::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17503
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17524
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<SHORT*>(*pData + cnt) = m_time;
        cnt += sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17547
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x506700
BOOL CMessageSetDrawPoly::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17575
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_time = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17601
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x5067B0
void CMessageSetDrawPoly::Run()
{
    CGameAIBase* pAIBase;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pAIBase),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly = TRUE;

        if (pAIBase->GetObjectType() == CGameObject::TYPE_DOOR) {
            static_cast<CGameDoor*>(pAIBase)->SetDrawPoly(m_time);
        } else if (pAIBase->GetObjectType() == CGameObject::TYPE_TRIGGER) {
            static_cast<CGameTrigger*>(pAIBase)->SetDrawPoly(m_time);
        } else if (pAIBase->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            static_cast<CGameContainer*>(pAIBase)->SetDrawPoly(m_time);
        }

        g_pBaldurChitin->GetBaldurMessage()->m_bInMessageSetDrawPoly = FALSE;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5F20
CMessageSetForceActionPick::CMessageSetForceActionPick(BOOLEAN forceActionPick, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_forceActionPick = forceActionPick;
}

// 0x40A0D0
SHORT CMessageSetForceActionPick::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetForceActionPick::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x466F70
BYTE CMessageSetForceActionPick::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_FORCE_ACTION_PICK;
}

// 0x5068C0
void CMessageSetForceActionPick::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17683
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17704
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_forceActionPick;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17727
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5069F0
BOOL CMessageSetForceActionPick::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17755
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_forceActionPick = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17781
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x506A90
void CMessageSetForceActionPick::Run()
{
    CGameAIBase* pAIBase;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pAIBase),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pAIBase->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pAIBase->m_forceActionPick = m_forceActionPick;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5F40
CMessageSetHappiness::CMessageSetHappiness(SHORT happiness, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_happiness = happiness;
}

// 0x40A0D0
SHORT CMessageSetHappiness::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetHappiness::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5F70
BYTE CMessageSetHappiness::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_HAPPINESS;
}

// 0x506B40
void CMessageSetHappiness::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 18858
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17879
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<SHORT*>(*pData + cnt) = m_happiness;
        cnt += sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 17902
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x506C70
BOOL CMessageSetHappiness::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17930
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_happiness = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 17956
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x506D20
void CMessageSetHappiness::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_nHappiness = m_happiness;
            pSprite->m_bHappinessChanged = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5F80
CMessageSetInCutScene::CMessageSetInCutScene(BOOL status, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_status = status;
}

// 0x40A0D0
SHORT CMessageSetInCutScene::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetInCutScene::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x463300
BYTE CMessageSetInCutScene::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_IN_CUT_SCENE;
}

// 0x506DD0
void CMessageSetInCutScene::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 18026
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 18047
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOL*>(*pData + cnt) = m_status;
        cnt += sizeof(BOOL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 18070
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x506F00
BOOL CMessageSetInCutScene::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 18098
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_status = *reinterpret_cast<BOOL*>(pData + cnt);
    cnt += sizeof(BOOL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 18124
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x506FB0
void CMessageSetInCutScene::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->m_inCutScene = m_status;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5FA0
CMessageSetLastAttacker::CMessageSetLastAttacker(const CAIObjectType& lAttacker, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_lAttacker.Set(lAttacker);
}

// 0x4F6060
CMessageSetLastAttacker::~CMessageSetLastAttacker()
{
}

// 0x453510
SHORT CMessageSetLastAttacker::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetLastAttacker::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6030
BYTE CMessageSetLastAttacker::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_LAST_ATTACKER;
}

// 0x5075A0
void CMessageSetLastAttacker::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_lAttacker.Set(m_lAttacker);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x45B780
CMessageSetLastObject::CMessageSetLastObject(const CAIObjectType& lAttacker, WORD type, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_lAttacker.Set(lAttacker);
    m_type = type;
}

// 0x45B840
CMessageSetLastObject::~CMessageSetLastObject()
{
}

// 0x40A0D0
SHORT CMessageSetLastObject::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetLastObject::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x45B810
BYTE CMessageSetLastObject::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_LAST_OBJECT;
}

// 0x507BA0
void CMessageSetLastObject::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            switch (m_type) {
            case 2:
                pSprite->m_lAttacker.Set(m_lAttacker);
                break;
            case 3:
                pSprite->m_lHelp.Set(m_lAttacker);
                break;
            case 6:
                pSprite->m_lOrderedBy.Set(m_lAttacker);
                break;
            case 7:
                pSprite->m_lTalkedTo.Set(m_lAttacker);
                break;
            case 32:
                pSprite->m_lHitter.Set(m_lAttacker);
                break;
            case 47:
                pSprite->m_lHeard.Set(m_lAttacker);
                break;
            case 16412:
                pSprite->m_lSeen.Set(m_lAttacker);
                break;
            default:
                pSprite->m_lTrigger.Set(m_lAttacker);
                break;
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F60B0
CMessageSetNumTimesTalkedTo::CMessageSetNumTimesTalkedTo(LONG times, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nNumTimesTalkedTo = times;
}

// 0x43E170
SHORT CMessageSetNumTimesTalkedTo::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageSetNumTimesTalkedTo::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F60D0
BYTE CMessageSetNumTimesTalkedTo::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_NUM_TIMES_TALKED_TO;
}

// 0x507CF0
void CMessageSetNumTimesTalkedTo::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 18936
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 18957
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_nNumTimesTalkedTo;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 18980
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x507E20
BOOL CMessageSetNumTimesTalkedTo::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19008
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_nNumTimesTalkedTo = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19034
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x507ED0
void CMessageSetNumTimesTalkedTo::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_nNumberOfTimesTalkedTo = m_nNumTimesTalkedTo;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x507F70
CMessageSetPath::CMessageSetPath(LONG position, LONG* pPath, SHORT nPath, SHORT currPath, CPoint currDest, LONG caller, LONG target)
    : CMessage(caller, target)
{
    CResRef areaResRef;
    CString sAreaString;

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetArea() != NULL) {
            areaResRef = pObject->GetArea()->m_resRef;
            areaResRef.CopyToString(sAreaString);
        } else {
            sAreaString = "NO_AREA";
        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(target,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        sAreaString = "NO_AREA";
    }

    m_currDest = currDest;
    m_position = position;
    m_sAreaString = sAreaString;
    if (nPath > 0) {
        m_currPath = currPath;
        m_pPath = new LONG[nPath];
        memcpy(m_pPath, pPath, sizeof(LONG) * nPath);
        m_nPath = nPath;
    } else {
        m_currPath = 0;
        m_pPath = NULL;
        m_nPath = 0;
    }
}

// 0x4F60E0
CMessageSetPath::~CMessageSetPath()
{
    if (m_pPath != NULL) {
        delete m_pPath;
    }
}

// 0x453510
SHORT CMessageSetPath::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetPath::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6140
BYTE CMessageSetPath::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_PATH;
}

// 0x5084C0
void CMessageSetPath::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

            if (pSprite->GetArea() != NULL) {
                CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(m_sAreaString);
                if (pArea == pSprite->GetArea()) {
                    CPoint pathStart;
                    CPathSearch::PositionToPoint(m_position, &pathStart);

                    CPoint spritePos = pSprite->GetPos();
                    if (spritePos.x / CPathSearch::GRID_SQUARE_SIZEX != pathStart.x
                        || spritePos.y / CPathSearch::GRID_SQUARE_SIZEY != pathStart.y) {
                        if (g_pChitin->cNetwork.GetSessionOpen()) {
                            pSprite->JumpToPoint(CPoint(pathStart.x * CPathSearch::GRID_SQUARE_SIZEX,
                                                     pathStart.y * CPathSearch::GRID_SQUARE_SIZEY),
                                FALSE);
                        } else {
                            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                                CGameObjectArray::THREAD_ASYNCH,
                                INFINITE);
                            return;
                        }
                    }

                    pSprite->SetPath(m_pPath, m_nPath);
                    pSprite->m_currPath = m_currPath;
                    pSprite->InitializeWalkingSound();
                    m_pPath = NULL;
                }            }        }
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }}

// -----------------------------------------------------------------------------

// 0x453530
CMessageSetSequence::CMessageSetSequence(BYTE sequence, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_sequence = sequence;
}

// 0x453510
SHORT CMessageSetSequence::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetSequence::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453550
BYTE CMessageSetSequence::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_SEQUENCE;
}

// 0x5086D0
void CMessageSetSequence::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19503
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 19523
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = m_sequence;
        cnt += sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 19546
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x508800
BOOL CMessageSetSequence::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19574
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_sequence = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19600
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x5088A0
void CMessageSetSequence::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);

            // NOTE: Uninline.
            if (pSprite->GetAnimation()->GetAnimationId() != 0xEC0B) {
                pSprite->CheckSequence(m_sequence);
            }

            pSprite->SetSequence(m_sequence);

            // NOTE: Uninline.
            if (pSprite->InControl()) {
                pSprite->m_bSendSpriteUpdate = TRUE;
            }
        } else if (pObject->GetObjectType() == CGameObject::TYPE_STATIC) {
            CGameStatic* pStatic = static_cast<CGameStatic*>(pObject);
            pStatic->m_vidCell.SequenceSet(m_sequence);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F61B0
CMessageSetTrigger::CMessageSetTrigger(const CAITrigger& trigger, LONG caller, LONG target)
    : CMessage(caller, target)
    , m_trigger(CAITrigger::NO_TRIGGER, 0)
{
    m_trigger = trigger;
}

// 0x409370
CMessageSetTrigger::~CMessageSetTrigger()
{
}

// 0x40A0D0
SHORT CMessageSetTrigger::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSetTrigger::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x409340
BYTE CMessageSetTrigger::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_TRIGGER;
}

// 0x50A360
void CMessageSetTrigger::Run()
{
    CGameAIBase* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if ((pSprite->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pSprite->SetTrigger(m_trigger);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined
CMessageSpriteDeath::CMessageSpriteDeath(DWORD nDeathType, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nDeathType = nDeathType;
}

// 0x4088A0
SHORT CMessageSpriteDeath::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageSpriteDeath::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4AD360
BYTE CMessageSpriteDeath::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_DEATH;
}

// 0x50B1C0
void CMessageSpriteDeath::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21835
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(DWORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 21856
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_nDeathType;
        cnt += sizeof(DWORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 21879
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x50B2F0
BOOL CMessageSpriteDeath::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21907
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_nDeathType = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21933
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x50B3A0
void CMessageSpriteDeath::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameEffectDeath death;
            death.m_dwFlags = m_nDeathType;
            death.ApplyEffect(pSprite);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x50B510
CMessageSpriteEquipment::CMessageSpriteEquipment(CGameSprite* pSprite, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pSprite != NULL) {
        CGameSpriteEquipment* pEquipment = pSprite->GetEquipment();
        for (int index = 0; index < 51; index++) {
            m_equipment.m_items[index] = pEquipment->m_items[index];
        }
        m_equipment.m_selectedWeapon = pEquipment->m_selectedWeapon;
        m_equipment.m_selectedWeaponAbility = pEquipment->m_selectedWeaponAbility;
        m_nWeaponSet = pSprite->m_nWeaponSet;
    }

    m_equipment.m_pSprite = NULL;
}

// 0x50B620
CMessageSpriteEquipment::~CMessageSpriteEquipment()
{
    memset(&m_equipment, 0, sizeof(m_equipment));
}

// 0x4088A0
SHORT CMessageSpriteEquipment::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageSpriteEquipment::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x50B5F0
BYTE CMessageSpriteEquipment::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_EQUIPMENT;
}

// -----------------------------------------------------------------------------

// 0x4F62C0
CMessageSpritePetrify::CMessageSpritePetrify(BOOLEAN bPetrify, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bPetrify = bPetrify;
}

// 0x4088A0
SHORT CMessageSpritePetrify::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageSpritePetrify::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F62E0
BYTE CMessageSpritePetrify::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_PETRIFY;
}

// 0x50BE80
void CMessageSpritePetrify::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 22552
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 22573
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bPetrify;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 22597
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x50BFB0
BOOL CMessageSpritePetrify::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 22625
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_bPetrify = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 22651
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x50C050
void CMessageSpritePetrify::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            if (m_bPetrify == TRUE) {
                CGameEffectPetrification petrify;
                petrify.ApplyEffect(pSprite);
            } else {
                CGameEffectStoneToFlesh unpetrify;
                unpetrify.ApplyEffect(pSprite);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x50C260
CMessageSpriteUpdate::CMessageSpriteUpdate(CGameSprite* pSprite, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nTranslucent = 0;
    m_nFadeSpeed = 0;
    field_64 = 0;
    field_66 = 0;
    field_68 = 0;
    field_6A = 0;
    m_nACCrushingMod = 0;
    m_nACMissileMod = 0;
    m_nACPiercingMod = 0;
    m_nACSlashingMod = 0;
    m_nEnemyAlly = 0;
    m_nEnemyAllyLive = 0;
    m_nEnemyAllyStart = 0;
    m_bMoralFailure = 0;
    m_nMoveScale = 0;
    m_nHPCONBonusTotalOld = 0;
    m_bLevelUp = FALSE;
    m_pPath = 0;
    m_nPath = 0;
    field_74 = 0;
    field_76 = NULL;
    m_currPath = 0;
    field_108 = 0;
    field_56 = 0;

    if (pSprite != NULL) {
        m_nFadeSpeed = pSprite->GetBaseStats()->m_fadeSpeed;
        m_nTranslucent = static_cast<BYTE>(pSprite->GetDerivedStats()->m_nTranslucent);
        m_visualEffects = pSprite->GetDerivedStats()->m_visualEffects;
        m_spellStates = pSprite->GetDerivedStats()->m_spellStates;
        memcpy(m_baseSkills, pSprite->GetBaseStats()->m_skills, sizeof(m_baseSkills));
        memcpy(m_skills, pSprite->GetDerivedStats()->m_nSkills, sizeof(m_skills));
        field_106 = pSprite->GetBaseStats()->field_2FF;
        m_nSequence = pSprite->m_nSequence;
        m_ptPosition = pSprite->GetPos();
        m_nFacing = pSprite->m_nDirection;
        m_dwState = pSprite->GetDerivedStats()->m_generalState;
        m_nHitPoints = pSprite->GetBaseStats()->m_hitPoints;
        m_nMaxHitPoints = pSprite->GetDerivedStats()->m_nMaxHitPoints;
        field_60 = pSprite->GetBaseStats()->m_flags;
        field_64 = pSprite->GetDerivedStats()->m_nACArmorBonus;
        field_66 = pSprite->GetDerivedStats()->m_nACNaturalBonus;
        field_68 = pSprite->GetDerivedStats()->m_nACDeflectionBonus;
        field_6A = pSprite->GetDerivedStats()->m_nACDodgeBonus;
        m_nACCrushingMod = pSprite->GetDerivedStats()->m_nACCrushingMod;
        m_nACMissileMod = pSprite->GetDerivedStats()->m_nACMissileMod;
        m_nACPiercingMod = pSprite->GetDerivedStats()->m_nACPiercingMod;
        m_nACSlashingMod = pSprite->GetDerivedStats()->m_nACSlashingMod;
        m_nEnemyAlly = pSprite->GetAIType().GetEnemyAlly();
        m_nEnemyAllyLive = pSprite->GetLiveAIType().GetEnemyAlly();
        m_nEnemyAllyStart = pSprite->m_startTypeAI.GetEnemyAlly();
        m_bMoralFailure = pSprite->m_moraleFailure;
        m_nMoveScale = pSprite->GetAnimation()->GetMoveScale();
        m_bLevelUp = static_cast<BOOLEAN>(pSprite->m_bLevelUp);
        m_nHPCONBonusTotalOld = pSprite->m_nHPCONBonusTotalOld;
        m_ptDest = pSprite->m_curDest;
        field_108 = 0;
        if (pSprite->GetArea() != NULL) {
            m_nPath = pSprite->m_nPath;
            m_currPath = pSprite->m_currPath;
            if (pSprite->m_pPath == NULL) {
                m_nPath = 0;
                m_currPath;
            }
            if (m_nPath > 0) {
                m_pPath = new LONG[m_nPath];
                memcpy(m_pPath, pSprite->m_pPath, sizeof(LONG) * m_nPath);
            }
        } else {
            m_nPath = 0;
            m_pPath = NULL;
            m_currPath = 0;
        }
        field_85 = 0;
    }
}

// 0x4F6300
CMessageSpriteUpdate::~CMessageSpriteUpdate()
{
    if (m_pPath != NULL) {
        delete m_pPath;
    }

    if (field_76 != NULL) {
        delete field_76;
    }
}

// 0x4088A0
SHORT CMessageSpriteUpdate::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageSpriteUpdate::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F62F0
BYTE CMessageSpriteUpdate::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_UPDATE;
}

// -----------------------------------------------------------------------------

// 0x4F6390
CMessageStartFollow::CMessageStartFollow(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x40A0D0
SHORT CMessageStartFollow::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageStartFollow::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x40A0F0
BYTE CMessageStartFollow::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_FOLLOW;
}

// 0x50EEE0
void CMessageStartFollow::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 24971
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 24991
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // NOTE: Missing trailing guard.
    } else {
        *dwSize = 0;
    }
}

// 0x50EFE0
BOOL CMessageStartFollow::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25043
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50F060
void CMessageStartFollow::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_followStart = 1;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// FIXME: `src` and `dest` should references.
//
// 0x4F63B0
CMessageStartScroll::CMessageStartScroll(CGameArea* pArea, CPoint src, CPoint dest, BYTE speed, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pArea != NULL) {
        pArea->m_resRef.CopyToString(m_sAreaString);
    } else {
        m_sAreaString = "";
    }

    m_src = src;
    m_dest = dest;
    m_speed = speed;
}

// 0x45F580
CMessageStartScroll::~CMessageStartScroll()
{
}

// 0x453510
SHORT CMessageStartScroll::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageStartScroll::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x40A0F0
BYTE CMessageStartScroll::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_SCROLL;
}

// 0x50F100
void CMessageStartScroll::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25144
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        BYTE nAreaStringLength = static_cast<BYTE>(m_sAreaString.GetLength());

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BYTE)
            + nAreaStringLength
            + sizeof(LONG)
            + sizeof(LONG)
            + sizeof(LONG)
            + sizeof(LONG)
            + sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 25171
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = nAreaStringLength;
        cnt += sizeof(BYTE);

        memcpy(*pData + cnt, m_sAreaString.GetBuffer(nAreaStringLength), nAreaStringLength);
        cnt += nAreaStringLength;

        *reinterpret_cast<LONG*>(*pData + cnt) = m_src.x;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_src.y;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_dest.x;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_dest.y;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = m_speed;
        cnt += sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 25210
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x50F2C0
BOOL CMessageStartScroll::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25241
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    BYTE nAreaStringLength = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    CHAR szAreaString[9];
    memcpy(szAreaString, pData + cnt, nAreaStringLength);
    cnt += nAreaStringLength;

    CString sTempString(szAreaString, nAreaStringLength);
    m_sAreaString = sTempString;

    m_src.x = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_src.y = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_dest.x = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_dest.y = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_speed = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50F420
void CMessageStartScroll::Run()
{
    if (m_sAreaString.GetLength() > 0) {
        CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetArea(m_sAreaString);
        if (pArea != NULL || g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(m_sAreaString), 1010, TRUE) == NULL) {
            if (pArea != g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
                g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_bPicked = FALSE;
                g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_iPicked = CGameObjectArray::INVALID_INDEX;
                g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_nToolTip = 0;
                g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnDeactivation();
                g_pBaldurChitin->GetObjectGame()->m_visibleArea = pArea->m_id;
                g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnActivation();
            }
        } else {
            pArea = g_pBaldurChitin->GetObjectGame()->LoadArea(m_sAreaString, -1, FALSE, FALSE);
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_bPicked = FALSE;
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_iPicked = CGameObjectArray::INVALID_INDEX;
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_nToolTip = 0;
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnDeactivation();
            g_pBaldurChitin->GetObjectGame()->m_visibleArea = pArea->m_id;
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnActivation();
        }

        if (pArea != NULL) {
            pArea->GetInfinity()->SetViewPosition(m_src.x, m_src.y, TRUE);
        }
    }

    g_pBaldurChitin->m_pEngineWorld->StartScroll(m_dest, m_speed);
}

// -----------------------------------------------------------------------------

// 0x4F6480
CMessageStopActions::CMessageStopActions(LONG caller, LONG target, CGameSprite* pSprite)
    : CMessage(caller, target)
{
    if (pSprite != NULL) {
        pSprite->DropPath();
        pSprite->DropSearchRequest();
        pSprite->ClearActions(FALSE);
        pSprite->SetCurrAction(CAIAction::NULL_ACTION);
        pSprite->SetIdleSequence();
        pSprite->m_targetPoint.x = -1;
        pSprite->m_targetPoint.y = -1;
    }
}

// 0x4088A0
SHORT CMessageStopActions::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageStopActions::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4088B0
BYTE CMessageStopActions::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_ACTIONS;
}

// 0x50F860
void CMessageStopActions::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25556
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 25576
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 25596
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x50F980
BOOL CMessageStopActions::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25624
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50F9F0
void CMessageStopActions::Run()
{
    CGameAIBase* pAIBase;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pAIBase),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pAIBase->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pAIBase);
            pSprite->DropPath();
            pSprite->DropSearchRequest();
            pSprite->ClearActions(FALSE);
            pSprite->SetCurrAction(CAIAction::NULL_ACTION);
            pSprite->SetIdleSequence();
            pSprite->m_targetPoint.x = -1;
            pSprite->m_targetPoint.y = -1;
        } else if ((pAIBase->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
            pAIBase->ClearActions(FALSE);
            pAIBase->SetCurrAction(CAIAction::NULL_ACTION);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6520
CMessageStopFollow::CMessageStopFollow(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x40A0D0
SHORT CMessageStopFollow::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageStopFollow::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x409810
BYTE CMessageStopFollow::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_FOLLOW;
}

// 0x50FB00
void CMessageStopFollow::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25731
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 25751
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        // NOTE: Missing trailing guard.
    } else {
        *dwSize = 0;
    }
}

// 0x50FC00
BOOL CMessageStopFollow::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25803
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50FC80
void CMessageStopFollow::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_followStart = 0;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6720
CMessageTriggerStatus::CMessageTriggerStatus(DWORD flags, WORD trapActivated, WORD trapDetected, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_dwFlags = flags;
    m_trapActivated = trapActivated;
    m_trapDetected = trapDetected;
}

// 0x43E170
SHORT CMessageTriggerStatus::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageTriggerStatus::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x466210
BYTE CMessageTriggerStatus::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TRIGGER_STATUS;
}

// 0x5105B0
void CMessageTriggerStatus::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26399
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(remoteObjectID)
            + sizeof(LONG)
            + sizeof(DWORD)
            + sizeof(WORD)
            + sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26422
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_dwFlags;
        cnt += sizeof(DWORD);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_trapActivated;
        cnt += sizeof(WORD);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_trapDetected;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26452
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5106F0
BOOL CMessageTriggerStatus::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26480
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_dwFlags = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    m_trapActivated = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    m_trapDetected = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x510790
void CMessageTriggerStatus::Run()
{
    CGameTrigger* pTrigger;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pTrigger),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pTrigger->m_dwFlags = m_dwFlags;
        pTrigger->m_trapActivated = m_trapActivated;
        pTrigger->m_trapDetected = m_trapDetected;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6760
CMessageUnlock::CMessageUnlock(DWORD flags, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_dwFlags = flags;
}

// 0x40A0D0
SHORT CMessageUnlock::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageUnlock::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x466F80
BYTE CMessageUnlock::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_UNLOCK;
}

// 0x510840
void CMessageUnlock::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26595
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(DWORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26616
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_dwFlags;
        cnt += sizeof(DWORD);

        // NOTE: Missing trailing guard.
    } else {
        *dwSize = 0;
    }
}

// 0x510950
BOOL CMessageUnlock::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 22625
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_dwFlags = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 22651
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x5109E0
void CMessageUnlock::Run()
{
    CGameAIBase* pAIBase;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pAIBase),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pAIBase->GetObjectType() == CGameObject::TYPE_DOOR) {
            CGameDoor* pDoor = static_cast<CGameDoor*>(pAIBase);

            // TODO: Unclear.
            pDoor->m_dwFlags ^= (pDoor->m_dwFlags ^ m_dwFlags) & 0x2;

            CMessageDoorStatus* pMessage = new CMessageDoorStatus(pDoor,
                m_targetId,
                m_targetId);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        } else if (pAIBase->GetObjectType() == CGameObject::TYPE_CONTAINER) {
            CGameContainer* pContainer = static_cast<CGameContainer*>(pAIBase);

            pContainer->SetFlags(m_dwFlags);

            CMessageContainerStatus* pMessage = new CMessageContainerStatus(pContainer->m_dwFlags,
                pContainer->m_trapActivated,
                pContainer->m_trapDetected,
                m_targetId,
                m_targetId);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6780
CMessageUpdateReaction::CMessageUpdateReaction(LONG nReaction, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nReaction = nReaction;
}

// 0x453510
SHORT CMessageUpdateReaction::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageUpdateReaction::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x45DEC0
BYTE CMessageUpdateReaction::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_UPDATE_REACTION;
}

// 0x510BA0
void CMessageUpdateReaction::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26786
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26807
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_nReaction;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26830
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x510CD0
BOOL CMessageUpdateReaction::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26858
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_nReaction = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x510D50
void CMessageUpdateReaction::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_reactionRoll = static_cast<BYTE>(m_nReaction);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F67A0
CMessageVerbalConstant::CMessageVerbalConstant(LONG verbalConstant, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_verbalConstant = verbalConstant;
}

// 0x453510
SHORT CMessageVerbalConstant::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageVerbalConstant::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4535E0
BYTE CMessageVerbalConstant::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VERBAL_CONSTANT;
}

// 0x510DF0
void CMessageVerbalConstant::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26957
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26978
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_verbalConstant;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 27001
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x510F20
BOOL CMessageVerbalConstant::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 27029
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_verbalConstant = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x510FA0
void CMessageVerbalConstant::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->VerbalConstant(m_verbalConstant);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F67C0
CMessageVisibilityMapMove::CMessageVisibilityMapMove(BOOLEAN moveOn, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_moveOntoList = moveOn;
}

// 0x453510
SHORT CMessageVisibilityMapMove::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageVisibilityMapMove::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4AD370
BYTE CMessageVisibilityMapMove::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VISIBILITY_MAP_MOVE;
}

// 0x511040
void CMessageVisibilityMapMove::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 27127
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 27148
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_moveOntoList;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 27171
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x511170
BOOL CMessageVisibilityMapMove::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 27199
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_moveOntoList = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5111F0
void CMessageVisibilityMapMove::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            if (m_moveOntoList) {
                CGameArea* pArea = pSprite->GetArea();
                if (pArea != NULL) {
                    pArea->m_visibility.AddCharacter(pSprite->GetPos(),
                        pSprite->GetId(),
                        pSprite->GetVisibleTerrainTable());
                    pSprite->m_posLastVisMapEntry = pSprite->GetPos();
                }
            } else {
                CGameArea* pArea = pSprite->GetArea();
                if (pArea != NULL) {
                    pArea->m_visibility.RemoveCharacter(pSprite->GetPos(),
                        pSprite->GetId(),
                        pSprite->GetVisibleTerrainTable());
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F67E0
CMessageVisualEffect::CMessageVisualEffect(BYTE nEffectType, BYTE nEffectProperty, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nEffectType = nEffectType;
    m_nEffectProperty = nEffectProperty;
}

// 0x453510
SHORT CMessageVisualEffect::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageVisualEffect::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4AA6C0
BYTE CMessageVisualEffect::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_VISUAL_EFFECT;
}

// 0x511310
void CMessageVisualEffect::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 27347
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BYTE)
            + sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 27369
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = m_nEffectType;
        cnt += sizeof(BYTE);

        *reinterpret_cast<BYTE*>(*pData + cnt) = m_nEffectProperty;
        cnt += sizeof(BYTE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 27395
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x511440
BOOL CMessageVisualEffect::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 27423
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_nEffectType = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    m_nEffectProperty = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5114D0
void CMessageVisualEffect::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            if (m_nEffectType == 3) {
                pSprite->GetDerivedStats()->m_nMirrorImages = m_nEffectProperty;
            } else if (m_nEffectType == 23) {
                pSprite->GetDerivedStats()->m_nStoneSkins = m_nEffectProperty;

                if (m_nEffectProperty == 0) {
                    // The binary's loop bound really is 8, not 7 -- and since
                    // m_colors is BYTE[7] the eighth value is the byte that
                    // follows it in the stats block, m_effectVersion.  Written
                    // as an explicit eighth call rather than an over-read, same
                    // observable effect without the UB.  CMessageColorReset
                    // does the same thing.
                    for (BYTE colorRange = 0; colorRange < 7; colorRange++) {
                        pSprite->GetAnimation()->SetColorRange(colorRange,
                            pSprite->GetBaseStats()->m_colors[colorRange]);
                    }
                    pSprite->GetAnimation()->SetColorRange(7,
                        pSprite->GetBaseStats()->m_effectVersion);

                    pSprite->GetAnimation()->ClearColorEffectsAll();
                } else {
                    pSprite->SetColorRange(14);
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// FIXME: `dialog` should be reference.
//
// 0x4F5E30
CMessageSetDialogResRef::CMessageSetDialogResRef(CResRef dialog, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_cResRefDialog = dialog;
}

// 0x4088A0
SHORT CMessageSetDialogResRef::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageSetDialogResRef::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4B2330
BYTE CMessageSetDialogResRef::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_DIALOG_RESREF;
}

// 0x505F50
void CMessageSetDialogResRef::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_dialog = m_cResRefDialog;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5680
CMessageEscapeArea::CMessageEscapeArea(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x4536E0
SHORT CMessageEscapeArea::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageEscapeArea::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F56A0
BYTE CMessageEscapeArea::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_ESCAPE_AREA;
}

// 0x4FF4B0
void CMessageEscapeArea::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11068
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11089
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11109
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FF5D0
BOOL CMessageEscapeArea::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11137
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11160
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FF670
void CMessageEscapeArea::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_bEscapingArea = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x48AC30
CMessageDisplayTextRefSend::CMessageDisplayTextRefSend(STRREF name, STRREF text, COLORREF nameColor, COLORREF textColor, LONG marker, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_name = name;
    m_text = text;
    m_nameColor = nameColor;
    m_textColor = textColor;
    m_marker = marker;
    m_moveToTop = FALSE;
}

// 0x40A0D0
SHORT CMessageDisplayTextRefSend::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageDisplayTextRefSend::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x489670
BYTE CMessageDisplayTextRefSend::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DISPLAY_TEXTREF_SEND;
}

// 0x4FCA10
void CMessageDisplayTextRefSend::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8934
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(STRREF)
        + sizeof(STRREF)
        + sizeof(COLORREF)
        + sizeof(COLORREF)
        + sizeof(LONG)
        + sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 8962
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<STRREF*>(*pData + cnt) = m_name;
    cnt += sizeof(STRREF);

    *reinterpret_cast<STRREF*>(*pData + cnt) = m_text;
    cnt += sizeof(STRREF);

    *reinterpret_cast<COLORREF*>(*pData + cnt) = m_nameColor;
    cnt += sizeof(COLORREF);

    *reinterpret_cast<COLORREF*>(*pData + cnt) = m_textColor;
    cnt += sizeof(COLORREF);

    *reinterpret_cast<LONG*>(*pData + cnt) = m_marker;
    cnt += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_moveToTop;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9003
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FCB60
BOOL CMessageDisplayTextRefSend::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9031
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_name = *reinterpret_cast<STRREF*>(pData + cnt);
    cnt += sizeof(STRREF);

    m_text = *reinterpret_cast<STRREF*>(pData + cnt);
    cnt += sizeof(STRREF);

    m_nameColor = *reinterpret_cast<COLORREF*>(pData + cnt);
    cnt += sizeof(COLORREF);

    m_textColor = *reinterpret_cast<COLORREF*>(pData + cnt);
    cnt += sizeof(COLORREF);

    m_marker = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_moveToTop = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 9076
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FCC30
void CMessageDisplayTextRefSend::Run()
{
    STR_RES strResName;
    STR_RES strResText;
    CString sName("");

    BYTE rc;
    CGameSprite* pSprite;

    if (m_name >= -7 && m_name != -1) {
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(-2 - m_name);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            sName = pSprite->GetName();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(m_name, strResName);
        sName = strResName.szText;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(m_text, strResText);

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (strResText.cSound.GetRes() != NULL) {
            if (strResText.cSound.m_nLooping == 0) {
                strResText.cSound.SetFireForget(TRUE);
            }

            strResText.cSound.SetChannel(14, reinterpret_cast<DWORD>(pSprite->GetArea()));

            // FIXME: Calls `GetPos` twice.
            strResText.cSound.Play(pSprite->GetPos().x,
                pSprite->GetPos().y,
                0,
                FALSE);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    g_pBaldurChitin->m_pEngineWorld->DisplayText(sName,
        strResText.szText,
        m_nameColor,
        m_textColor,
        m_marker,
        m_moveToTop);
}

// -----------------------------------------------------------------------------

// 0x4F5DF0
CMessageSetCurrentActionId::CMessageSetCurrentActionId(SHORT actionId, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_actionId = actionId;
}

// 0x453510
SHORT CMessageSetCurrentActionId::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetCurrentActionId::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5E20
BYTE CMessageSetCurrentActionId::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_CURRENT_ACTION_ID;
}

// 0x505980
void CMessageSetCurrentActionId::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16735
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16756
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<SHORT*>(*pData + cnt) = m_actionId;
        cnt += sizeof(SHORT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 16779
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x505AB0
BOOL CMessageSetCurrentActionId::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16807
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_actionId = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 16833
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x505B60
void CMessageSetCurrentActionId::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_currentActionId = m_actionId;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F5740
CMessageFadeColor::CMessageFadeColor(BOOLEAN bFadeTo, BYTE redFade, BYTE greenFade, BYTE blueFade, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bFadeTo = bFadeTo;
    m_redFade = redFade;
    m_greenFade = greenFade;
    m_blueFade = blueFade;
}

// 0x43E170
SHORT CMessageFadeColor::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageFadeColor::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4678F0
BYTE CMessageFadeColor::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FADE_COLOR;
}

// 0x4FFDA0
void CMessageFadeColor::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11772
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(BOOLEAN)
        + sizeof(BYTE)
        + sizeof(BYTE)
        + sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11780
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    (*pData)[cnt++] = m_bFadeTo;
    (*pData)[cnt++] = m_redFade;
    (*pData)[cnt++] = m_greenFade;
    (*pData)[cnt++] = m_blueFade;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11806
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FFE40
BOOL CMessageFadeColor::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11829
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_bFadeTo = pData[cnt++];
    m_redFade = pData[cnt++];
    m_greenFade = pData[cnt++];
    m_blueFade = pData[cnt++];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11846
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FFEB0
void CMessageFadeColor::Run()
{
    g_pBaldurChitin->m_pEngineWorld->m_nBlackOutCountDown = 0;

    // NOTE: Uninlnie.
    g_pChitin->GetCurrentVideoMode()->SetFade(m_bFadeTo, m_redFade);

    CScreenWorld::dword_8F85BC = 3;

    // NOTE: `m_redFade` name is misleading.
    if (m_redFade < 10) {
        for (int i = 0; i < 20; i++) {
            // NOTE: Uninline.
            g_pChitin->GetCurrentVideoMode()->UpdateFade();

            g_pChitin->m_bDisplayStale = TRUE;
            SleepEx(60, FALSE);
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4F6810
CMessageStartTextScreen::CMessageStartTextScreen(const CResRef& ref, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_screen = ref;
}

// 0x4088A0
SHORT CMessageStartTextScreen::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageStartTextScreen::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4535F0
BYTE CMessageStartTextScreen::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_TEXT_SCREEN;
}

// 0x508D30
void CMessageStartTextScreen::Run()
{
    // NOTE: Unused.
    CString sScreen;

    m_screen.CopyToString(sScreen);

    g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineChapter);
}

// -----------------------------------------------------------------------------

// 0x453600
CMessage90::CMessage90(LONG caller, LONG target, int a3)
    : CMessage(caller, target)
{
    field_C = a3;
}

// 0x66F250
SHORT CMessage90::GetCommType()
{
    // NOTE: Strange case.
    return 5;
}

// 0x40A0E0
BYTE CMessage90::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453620
BYTE CMessage90::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_90;
}

// 0x513270
void CMessage90::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29093
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(int);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29114
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<int*>(*pData + cnt) = field_C;
        cnt += sizeof(int);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29137
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5133A0
BOOL CMessage90::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29151
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    field_C = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x513420
void CMessage90::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            static_cast<CGameSprite*>(pObject)->SetStealthState(field_C);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6BE0
CMessageFloatText::CMessageFloatText(LONG caller, LONG target, STRREF strRef, BOOLEAN bSpell)
    : CMessage(caller, target)
{
    m_strRef = strRef;
    m_bSpell = bSpell;
}

// 0x453510
SHORT CMessageFloatText::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageFloatText::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453630
BYTE CMessageFloatText::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FLOAT_TEXT;
}

// 0x5134C0
void CMessageFloatText::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29222
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(STRREF)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29244
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<STRREF*>(*pData + cnt) = m_strRef;
        cnt += sizeof(STRREF);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bSpell;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29266
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5135F0
BOOL CMessageFloatText::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29280
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_strRef = *reinterpret_cast<STRREF*>(pData + cnt);
    cnt += sizeof(STRREF);

    m_bSpell = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x513680
void CMessageFloatText::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE
            || pObject->GetObjectType() == CGameObject::TYPE_DOOR) {
            if (m_bSpell) {
                STR_RES strRes;
                g_pBaldurChitin->GetTlkTable().Fetch(m_strRef, strRes);
                g_pBaldurChitin->GetTlkTable().SetToken(CRuleTables::CASTSPELL, strRes.szText);
                pObject->FloatText(39799, 10, 5);
            } else {
                pObject->FloatText(m_strRef, 0, 5);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x453640
CMessage92::CMessage92(LONG caller, LONG target, int a3)
    : CMessage(caller, target)
{
    field_C = a3;
}

// 0x453510
SHORT CMessage92::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessage92::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453660
BYTE CMessage92::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_92;
}

// 0x513800
void CMessage92::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29361
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(int);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29382
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<int*>(*pData + cnt) = field_C;
        cnt += sizeof(int);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29405
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x513930
BOOL CMessage92::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29419
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    field_C = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5139B0
void CMessage92::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            static_cast<CGameSprite*>(pObject)->SetExplorationRange(field_C);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Inlined
CMessageMoveGlobal::CMessageMoveGlobal(const CString& sArea, const CPoint& ptStart, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_sArea = sArea;
    m_ptStart = ptStart;
}

// 0x453510
SHORT CMessageMoveGlobal::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageMoveGlobal::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x467090
BYTE CMessageMoveGlobal::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_MOVE_GLOBAL;
}

// 0x508DA0
void CMessageMoveGlobal::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19886
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        BYTE nAreaLength = static_cast<BYTE>(m_sArea.GetLength());

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BYTE)
            + nAreaLength
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 19907
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BYTE*>(*pData + cnt) = nAreaLength;
        cnt += sizeof(BYTE);

        memcpy(*pData + cnt, m_sArea, nAreaLength);
        cnt += nAreaLength;

        *reinterpret_cast<LONG*>(*pData + cnt) = m_ptStart.x;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_ptStart.y;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 19937
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x508F30
BOOL CMessageMoveGlobal::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 19965
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    BYTE nAreaLength = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    m_sArea = CString(reinterpret_cast<char*>(pData + cnt), nAreaLength);
    cnt += nAreaLength;

    m_ptStart.x = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_ptStart.y = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 20000
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x509060
void CMessageMoveGlobal::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            static_cast<CGameSprite*>(pObject)->MoveGlobal(m_sArea, m_ptStart);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6270
CMessageSpawnPtActivate::CMessageSpawnPtActivate(BOOLEAN bActivate, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bActivate = bActivate;
}

// 0x453510
SHORT CMessageSpawnPtActivate::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSpawnPtActivate::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x467A70
BYTE CMessageSpawnPtActivate::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPAWNPT_ACTIVATE;
}

// 0x50AD10
void CMessageSpawnPtActivate::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21488
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 21509
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bActivate;
        cnt += sizeof(BOOLEAN);

        // NOTE: Missing trailing guard.
    } else {
        *dwSize = 0;
    }
}

// 0x50AE20
BOOL CMessageSpawnPtActivate::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21564
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_bActivate = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50AEC0
void CMessageSpawnPtActivate::Run()
{
    CGameSpawning* pSpawning;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSpawning),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSpawning->GetObjectType() == CGameObject::TYPE_SPAWNING) {
            pSpawning->m_spawningObject.m_activated = m_bActivate;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6290
CMessageSpawnPtSpawn::CMessageSpawnPtSpawn(const CPoint& ptFacingTowards, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_ptFacingTowards = ptFacingTowards;
}

// 0x40A0D0
SHORT CMessageSpawnPtSpawn::GetCommType()
{
    return SEND;
}

// 0x40A0E0
BYTE CMessageSpawnPtSpawn::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x467BA0
BYTE CMessageSpawnPtSpawn::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPAWNPT_SPAWN;
}

// 0x50AF60
void CMessageSpawnPtSpawn::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21658
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 21679
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_ptFacingTowards.x;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_ptFacingTowards.y;
        cnt += sizeof(LONG);

        // NOTE: Missing trailing guard.
    } else {
        *dwSize = 0;
    }
}

// 0x50B080
BOOL CMessageSpawnPtSpawn::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 21735
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_ptFacingTowards.x = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    m_ptFacingTowards.y = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50B120
void CMessageSpawnPtSpawn::Run()
{
    CGameSpawning* pSpawning;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSpawning),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSpawning->GetObjectType() == CGameObject::TYPE_SPAWNING) {
            pSpawning->Spawn(m_ptFacingTowards, 0);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6460
CMessageStaticStart::CMessageStaticStart(BOOLEAN bStart, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bStart = bStart;
}

// 0x43E170
SHORT CMessageStaticStart::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageStaticStart::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x467C40
BYTE CMessageStaticStart::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STATIC_START;
}

// 0x50F610
void CMessageStaticStart::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25385
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25406
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bStart;
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.
}

// 0x50F720
BOOL CMessageStaticStart::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25462
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_bStart = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50F7B0
void CMessageStaticStart::Run()
{
    CGameStatic* pStatic;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pStatic),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pStatic->GetObjectType() == CGameObject::TYPE_STATIC) {
            if (m_bStart) {
                pStatic->m_header.m_dwFlags &= ~0x8;
            } else {
                pStatic->m_header.m_dwFlags |= 0x8;
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F65C0
CMessageStoreAddItem::CMessageStoreAddItem(const CResRef& store, const CCreatureFileItem& item, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_store = store;
    memcpy(&m_item, &item, sizeof(CCreatureFileItem));
}

// 0x4536E0
SHORT CMessageStoreAddItem::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageStoreAddItem::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6660
BYTE CMessageStoreAddItem::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_ADD_ITEM;
}

// 0x50FE30
void CMessageStoreAddItem::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26002
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = RESREF_SIZE
        + sizeof(CCreatureFileItem);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26011
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    memcpy(*pData + cnt, m_store.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;

    memcpy(*pData + cnt, &m_item, sizeof(CCreatureFileItem));
    cnt += sizeof(CCreatureFileItem);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26032
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x50FED0
BOOL CMessageStoreAddItem::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26056
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_store = pData + cnt;
    cnt += RESREF_SIZE;

    memcpy(&m_item, pData + cnt, sizeof(CCreatureFileItem));
    cnt += sizeof(CCreatureFileItem);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50FF30
void CMessageStoreAddItem::Run()
{
    CStore* pStore;

    if (g_pChitin->cNetwork.GetSessionHosting()) {
        pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_store);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26111
        UTIL_ASSERT(pStore != NULL);

        pStore->AddItemExt(CItem(m_item), 1);
    } else if (g_pChitin->cNetwork.GetSessionOpen()) {
        CStore::InvalidateStore(m_store);
    }

    pStore = g_pBaldurChitin->m_pEngineStore->m_pStore;
    if (pStore != NULL) {
        if (pStore->m_resRef == m_store) {
            CItem item(m_item);
            pStore->AddItemExt(item, 1);
            g_pBaldurChitin->m_pEngineStore->UpdateStoreItems();
            g_pBaldurChitin->m_pEngineStore->UpdateStoreCost();

            LONG nCharacterId = g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_characterIndex;
            if (nCharacterId != CGameObjectArray::BAD_INDEX) {
                CGameObject* pObject;

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        &pObject,
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    // NOTE: Uninline.
                    if (pObject->InControl()) {
                        g_pBaldurChitin->m_pEngineStore->UpdateMainPanel();
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        } else {
            pStore = g_pBaldurChitin->m_pEngineStore->m_pBag;
            if (pStore != NULL && pStore->m_resRef == m_store) {
                CItem item(m_item);
                pStore->AddItemExt(item, 1);
                g_pBaldurChitin->m_pEngineStore->UpdateGroupItems();
                g_pBaldurChitin->m_pEngineStore->UpdateGroupCost();

                LONG nCharacterId = g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_characterIndex;
                if (nCharacterId != CGameObjectArray::BAD_INDEX) {
                    CGameObject* pObject;

                    BYTE rc;
                    do {
                        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            &pObject,
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        // NOTE: Uninline.
                        if (pObject->InControl()) {
                            g_pBaldurChitin->m_pEngineStore->UpdateMainPanel();
                        }

                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4F6670
CMessageStoreRemoveItem::CMessageStoreRemoveItem(const CResRef& store, const CResRef& itemId, int a3, int a4, int a5, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_store = store;
    m_itemId = itemId;
    field_1C = a3;
    field_20 = a4;
    field_24 = a5;
}

// 0x848F48
SHORT CMessageStoreRemoveItem::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageStoreRemoveItem::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47E020
BYTE CMessageStoreRemoveItem::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_REMOVE_ITEM;
}

// 0x5102B0
void CMessageStoreRemoveItem::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26219
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = RESREF_SIZE
        + RESREF_SIZE
        + sizeof(int)
        + sizeof(int)
        + sizeof(int);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26225
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    memcpy(*pData + cnt, m_store.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;

    memcpy(*pData + cnt, m_itemId.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;

    *reinterpret_cast<int*>(*pData + cnt) = field_1C;
    cnt += sizeof(int);

    *reinterpret_cast<int*>(*pData + cnt) = field_20;
    cnt += sizeof(int);

    *reinterpret_cast<int*>(*pData + cnt) = field_24;
    cnt += sizeof(int);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26247
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x510370
BOOL CMessageStoreRemoveItem::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 26270
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_store = pData + cnt;
    cnt += RESREF_SIZE;

    m_itemId = pData + cnt;
    cnt += RESREF_SIZE;

    field_1C = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    field_20 = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    field_24 = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5103F0
void CMessageStoreRemoveItem::Run()
{
    CStore* pStore;

    if (g_pChitin->cNetwork.GetSessionHosting()) {
        pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(m_store);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 26322
        UTIL_ASSERT(pStore != NULL);

        if (pStore->GetItemIndex(m_itemId) != INT_MAX) {
            pStore->RemoveItemExt(m_itemId,
                field_1C,
                field_20,
                field_24,
                NULL);
        }
    } else if (g_pChitin->cNetwork.GetSessionOpen()) {
        CStore::InvalidateStore(m_store);
    }

    pStore = g_pBaldurChitin->m_pEngineStore->m_pStore;
    if (pStore != NULL) {
        if (pStore->m_resRef == m_store) {
            if (pStore->GetItemIndex(m_itemId) != INT_MAX) {
                pStore->RemoveItemExt(m_itemId,
                    field_1C,
                    field_20,
                    field_24,
                    NULL);

                g_pBaldurChitin->m_pEngineStore->UpdateStoreItems();
                g_pBaldurChitin->m_pEngineStore->UpdateStoreCost();
                g_pBaldurChitin->m_pEngineStore->UpdateMainPanel();
            }
        } else {
            pStore = g_pBaldurChitin->m_pEngineStore->m_pBag;
            if (pStore != NULL && pStore->m_resRef == m_store) {
                if (pStore->GetItemIndex(m_itemId) != INT_MAX) {
                    pStore->RemoveItemExt(m_itemId,
                        field_1C,
                        field_20,
                        field_24,
                        NULL);

                    g_pBaldurChitin->m_pEngineStore->UpdateGroupItems();
                    g_pBaldurChitin->m_pEngineStore->UpdateGroupCost();
                    g_pBaldurChitin->m_pEngineStore->UpdateMainPanel();
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4F5780
CMessageFamiliarAdd::CMessageFamiliarAdd(BOOLEAN bFamiliarAdd, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bFamiliarAdd = bFamiliarAdd;
}

// 0x43E170
SHORT CMessageFamiliarAdd::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageFamiliarAdd::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453480
BYTE CMessageFamiliarAdd::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FAMILIAR_ADD;
}

// 0x4FFF90
void CMessageFamiliarAdd::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11921
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID)
        + sizeof(LONG)
        + sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11944
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bFamiliarAdd;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11967
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5000C0
BOOL CMessageFamiliarAdd::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11994
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    m_bFamiliarAdd = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 12021
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x500170
void CMessageFamiliarAdd::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            if (pSprite->m_bGlobal) {
                if (m_bFamiliarAdd) {
                    pSprite->SetAIType(CAIObjectType(CAIObjectType::EA_FAMILIAR), TRUE, FALSE);
                    g_pBaldurChitin->GetObjectGame()->AddCharacterToFamiliars(m_targetId);
                } else {
                    pSprite->SetAIType(CAIObjectType(CAIObjectType::EA_NEUTRAL), TRUE, FALSE);
                    g_pBaldurChitin->GetObjectGame()->RemoveCharacterFromFamiliars(m_targetId);
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F57A0
CMessageFamiliarRemoveResRef::CMessageFamiliarRemoveResRef(const CResRef& resRef, BYTE nAlignment, BYTE nLevel, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_resRef = resRef;
    m_nAlignment = nAlignment;
    m_nLevel = nLevel;
}

// 0x4536E0
SHORT CMessageFamiliarRemoveResRef::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageFamiliarRemoveResRef::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F5820
BYTE CMessageFamiliarRemoveResRef::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FAMILIAR_REMOVE_RESREF;
}

// 0x500320
void CMessageFamiliarRemoveResRef::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 12100
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = RESREF_SIZE
        + sizeof(BYTE)
        + sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 12107
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    memcpy(*pData + cnt, m_resRef.GetResRef(), RESREF_SIZE);

    *reinterpret_cast<BYTE*>(*pData + cnt) = m_nAlignment;
    cnt += sizeof(BYTE);

    *reinterpret_cast<BYTE*>(*pData + cnt) = m_nLevel;
    cnt += sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 12129
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5003C0
BOOL CMessageFamiliarRemoveResRef::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 12152
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_resRef = pData + cnt;
    cnt += RESREF_SIZE;

    m_nAlignment = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    m_nLevel = *reinterpret_cast<BYTE*>(pData + cnt);
    cnt += sizeof(BYTE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 12165
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x500440
void CMessageFamiliarRemoveResRef::Run()
{
    g_pBaldurChitin->GetObjectGame()->RemoveFamiliarResRef(m_resRef,
        m_nAlignment,
        m_nLevel);
}

// -----------------------------------------------------------------------------

// 0x500470
CMessageFireProjectile::CMessageFireProjectile(WORD projectileType, LONG projectileTargetId, const CPoint& projectileTarget, LONG height, LONG caller, LONG target, unsigned char a7)
    : CMessage(caller, target)
{
    // 0x500481: the binary increments the projectile type before storing it
    // (inc ecx). Senders pass the projectile's m_projectileType, which DecodeProjectile
    // stores as (factory type - 1); the +1 restores the factory type so Run's
    // DecodeProjectile(m_projectileType) rebuilds the same projectile. Without it the
    // handler decodes one type lower -- e.g. Ice Storm (0x62) replicated as Color
    // Spray (0x61).
    m_projectileType = static_cast<WORD>(projectileType + 1);
    m_projectileTargetId = projectileTargetId;
    m_projectileTarget = projectileTarget;
    m_height = height;
    field_1E = a7;
    field_20 = 0;
}

// 0x453510
SHORT CMessageFireProjectile::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageFireProjectile::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x5004C0
BYTE CMessageFireProjectile::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_FIRE_PROJECTILE;
}

// 0x5007F0
//
// The deferred projectile launch. The cast queues this message with caller ==
// target == the caster id, so m_targetId resolves the caster. After an MP-
// ownership / world-state gate it builds the projectile from the factory and
// fires it: Fire(caster area, caster id, projectile target id, target point,
// height, 0).
//
// The original calls Fire unconditionally; our factory still returns NULL for
// the projectile types whose leaves are unrecovered, so the launch is guarded (a
// NULL here would call through a null vtable). The projType 0x130 seed
// (projectile +0x356) is not modelled and is skipped.
void CMessageFireProjectile::Run()
{
    CGameObject* pCaster;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &pCaster, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    // 0x500853: the binary inlines CGameObject::InControl (0x4530C0) -- the launch is
    // skipped on the machine that controls the caster, which already fired the projectile
    // locally (CGameSprite::Spell). The original tests cNetwork's service provider and
    // m_idLocalPlayer; recovering those as raw CChitin offsets (+0x96E / +0x104C) read the
    // wrong words in our CChitin layout, so the gate never skipped in single-player and the
    // handler fired a duplicate projectile alongside the local one.
    BOOL skip = pCaster->InControl();
    if (skip || pCaster->m_pArea == NULL) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, INFINITE);
        return;
    }

    CGameAIBase* pProjCaster = (pCaster->GetObjectType() & 1)
        ? static_cast<CGameAIBase*>(pCaster) : NULL;
    CProjectile* pProjectile = CProjectile::DecodeProjectile(
        m_projectileType, pProjCaster, field_1E);
    if (pProjectile != NULL) {
        pProjectile->Fire(pCaster->m_pArea, pCaster->m_id, m_projectileTargetId,
            m_projectileTarget, m_height, 0);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// -----------------------------------------------------------------------------

// 0x4F56B0
CMessageStopEscapeArea::CMessageStopEscapeArea(LONG caller, LONG target)
    : CMessage(caller, target)
{
}

// 0x4536E0
SHORT CMessageStopEscapeArea::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageStopEscapeArea::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F56D0
BYTE CMessageStopEscapeArea::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STOP_ESCAPE_AREA;
}

// 0x4FF710
void CMessageStopEscapeArea::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11247
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        remotePlayerID = 0;
        remoteObjectID = CGameObjectArray::INVALID_INDEX;
    }

    *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11268
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11288
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FF830
BOOL CMessageStopEscapeArea::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11316
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) == TRUE) {
        m_targetId = localObjectID;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 11339
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x4FF8D0
void CMessageStopEscapeArea::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite->GetObjectType() == CGameObject::TYPE_SPRITE) {
            pSprite->m_bEscapingArea = FALSE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6AF0
CMessageSetTimeStop::CMessageSetTimeStop(LONG time, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_time = time;
}

// 0x453510
SHORT CMessageSetTimeStop::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetTimeStop::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6B10
BYTE CMessageSetTimeStop::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_TIME_STOP;
}

// 0x512160
void CMessageSetTimeStop::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 28267
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 28288
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_time;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 28311
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x512290
BOOL CMessageSetTimeStop::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 28339
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_time = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x799CA0
void CMessageSetTimeStop::Run()
{
}

// -----------------------------------------------------------------------------

// 0x4F6540
CMessageStoreRelease::CMessageStoreRelease(const CResRef& store, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_store = store;
}

// 0x43E170
SHORT CMessageStoreRelease::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageStoreRelease::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47E010
BYTE CMessageStoreRelease::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_RELEASE;
}

// 0x50FD20
void CMessageStoreRelease::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25889
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = RESREF_SIZE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25894
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    memcpy(*pData + cnt, m_store.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25910
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x50FDB0
BOOL CMessageStoreRelease::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 25934
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_store = pData + cnt;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x50FE00
void CMessageStoreRelease::Run()
{
    if (g_pChitin->cNetwork.GetSessionHosting()) {
        g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(m_store);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6C10
CMessageSetProtagonist::CMessageSetProtagonist(LONG nId, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_nId = nId;
}

// 0x453510
SHORT CMessageSetProtagonist::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetProtagonist::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4824E0
BYTE CMessageSetProtagonist::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_PROTAGONIST;
}

// 0x513A50
void CMessageSetProtagonist::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29490
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29511
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<LONG*>(*pData + cnt) = m_nId;
        cnt += sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29534
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x513B80
BOOL CMessageSetProtagonist::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29548
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_nId = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x513C00
void CMessageSetProtagonist::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pObject),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            g_pBaldurChitin->GetObjectGame()->SetProtagonist(m_nId);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6C30
CMessageStartCombatMusic::CMessageStartCombatMusic(int slot, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_slot = slot;
}

// 0x4088A0
SHORT CMessageStartCombatMusic::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageStartCombatMusic::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47A010
BYTE CMessageStartCombatMusic::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_START_COMBAT_MUSIC;
}

// 0x513CB0
void CMessageStartCombatMusic::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29620
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    PLAYER_ID remotePlayerID;
    LONG remoteObjectID;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        remotePlayerID = pObject->m_remotePlayerID;
        remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID) + sizeof(LONG);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29641
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<int*>(*pData + cnt) = m_slot;
        cnt += sizeof(int);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 29664
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x513DE0
BOOL CMessageStartCombatMusic::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 29678
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_slot = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x513E60
void CMessageStartCombatMusic::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pObject),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pObject->GetArea()->PlaySong(m_slot, 0x10000 | 0x100 | 0x80 | 0x40 | 0x20 | 0x10 | 0x04);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6CC0
CMessageStoreDemand::CMessageStoreDemand(const CResRef& store, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_store = store;
}

// 0x43E170
SHORT CMessageStoreDemand::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageStoreDemand::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x47E030
BYTE CMessageStoreDemand::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_STORE_DEMAND;
}

// 0x5143F0
void CMessageStoreDemand::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30020
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = RESREF_SIZE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30025
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    memcpy(*pData + cnt, m_store.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30041
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x514480
BOOL CMessageStoreDemand::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30065
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_store = pData + cnt;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5144D0
void CMessageStoreDemand::Run()
{
    if (g_pChitin->cNetwork.GetSessionHosting()) {
        g_pBaldurChitin->GetObjectGame()->DemandServerStore(m_store, FALSE);
    }
}

// -----------------------------------------------------------------------------

// NOTE: Very rare case - parameter after `caller` / `target` pair.
//
// 0x4F6D40
CMessage101::CMessage101(BOOLEAN a1, LONG caller, LONG target, BOOLEAN a4)
    : CMessage(caller, target)
{
    field_C = a1;
    field_D = a4;
}

// 0x43E170
SHORT CMessage101::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessage101::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6D70
BYTE CMessage101::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_101;
}

// 0x514500
void CMessage101::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30131
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOLEAN)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30153
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = field_C;
        cnt += sizeof(BOOLEAN);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = field_D;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30180
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x514630
BOOL CMessage101::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30207
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    field_C = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    field_D = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5146C0
void CMessage101::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->SetHideState(field_C, field_D);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x514770
CMessageWeaponImmumityUpdate::CMessageWeaponImmumityUpdate(CGameSprite* pSprite, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pSprite != NULL) {
        m_weaponImmunities = pSprite->GetDerivedStats()->m_cImmunitiesWeapon;
    }
}

// 0x4F7050
CMessageWeaponImmumityUpdate::~CMessageWeaponImmumityUpdate()
{
}

// 0x4088A0
SHORT CMessageWeaponImmumityUpdate::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageWeaponImmumityUpdate::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x514800
BYTE CMessageWeaponImmumityUpdate::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_WEAPON_IMMUNITIES_UPDATE;
}

// 0x514830
void CMessageWeaponImmumityUpdate::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30314
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        BYTE* immunities;
        ULONG immunitySize = m_weaponImmunities.Marshal(&immunities);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(ULONG)
            + immunitySize;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30342
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<ULONG*>(*pData + cnt) = immunitySize;
        cnt += sizeof(ULONG);

        memcpy(*pData + cnt, immunities, immunitySize);
        cnt += immunitySize;

        if (immunities != NULL) {
            delete immunities;
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30373
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5149A0
BOOL CMessageWeaponImmumityUpdate::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30388
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    ULONG immunitySize = *reinterpret_cast<ULONG*>(pData + cnt);
    cnt += sizeof(ULONG);

    m_weaponImmunities.Unmarshal(pData + cnt, immunitySize);
    cnt += immunitySize;

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x514A30
void CMessageWeaponImmumityUpdate::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->GetDerivedStats()->m_cImmunitiesWeapon = m_weaponImmunities;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x516CD0
CMessageSpellLevelImmumityUpdate::CMessageSpellLevelImmumityUpdate(CGameSprite* pSprite, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pSprite != NULL) {
        m_spellLevelImmunities = pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel;
    }
}

// 0x4B2340
CMessageSpellLevelImmumityUpdate::~CMessageSpellLevelImmumityUpdate()
{
}

// 0x4088A0
SHORT CMessageSpellLevelImmumityUpdate::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageSpellLevelImmumityUpdate::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x516D40
BYTE CMessageSpellLevelImmumityUpdate::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPELL_LEVEL_IMMUNITIES_UPDATE;
}

// 0x516D50
void CMessageSpellLevelImmumityUpdate::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 32328
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(m_spellLevelImmunities.m_levels);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 32349
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        memcpy(*pData + cnt, m_spellLevelImmunities.m_levels, sizeof(m_spellLevelImmunities.m_levels));
        cnt += sizeof(m_spellLevelImmunities.m_levels);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 32363
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x516E90
BOOL CMessageSpellLevelImmumityUpdate::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 32378
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    memcpy(m_spellLevelImmunities.m_levels, pData + cnt, sizeof(m_spellLevelImmunities.m_levels));
    cnt += sizeof(m_spellLevelImmunities.m_levels);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    return TRUE;
}

// 0x516F10
void CMessageSpellLevelImmumityUpdate::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->GetDerivedStats()->m_cImmunitiesSpellLevel = m_spellLevelImmunities;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x514F60
CMessageProjectileImmumityUpdate::CMessageProjectileImmumityUpdate(CGameSprite* pSprite, LONG caller, LONG target)
    : CMessage(caller, target)
{
    if (pSprite != NULL) {
        m_projectileImmunities = pSprite->GetDerivedStats()->m_cImmunitiesProjectile;
    }
}

// 0x514FF0
CMessageProjectileImmumityUpdate::~CMessageProjectileImmumityUpdate()
{
}

// 0x4088A0
SHORT CMessageProjectileImmumityUpdate::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageProjectileImmumityUpdate::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x516D40
BYTE CMessageProjectileImmumityUpdate::GetMsgSubType()
{
    // The binary's projectile-immunity GetMsgSubType is folded (/OPT:ICF) with
    // CMessageSpellLevelImmumityUpdate::GetMsgSubType at 0x516D40: both return
    // the spell-level subtype (104). The projectile message therefore reports
    // the same wire subtype as the spell-level message rather than its own.
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPELL_LEVEL_IMMUNITIES_UPDATE;
}

// 0x515010
void CMessageProjectileImmumityUpdate::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30714
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        BYTE* immunities;
        ULONG immunitySize = m_projectileImmunities.Marshal(&immunities);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(ULONG)
            + immunitySize;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30742
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<ULONG*>(*pData + cnt) = immunitySize;
        cnt += sizeof(ULONG);

        memcpy(*pData + cnt, immunities, immunitySize);
        cnt += immunitySize;

        if (immunities != NULL) {
            delete immunities;
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30773
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x515180
BOOL CMessageProjectileImmumityUpdate::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30788
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    ULONG immunitySize = *reinterpret_cast<ULONG*>(pData + cnt);
    cnt += sizeof(ULONG);

    m_projectileImmunities.Unmarshal(pData + cnt, immunitySize);
    cnt += immunitySize;

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    return TRUE;
}

// 0x515210
void CMessageProjectileImmumityUpdate::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            CGameSprite* pSprite = static_cast<CGameSprite*>(pObject);
            pSprite->GetDerivedStats()->m_cImmunitiesProjectile = m_projectileImmunities;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x5152C0
//
// Build the message and take over the source list's sub-messages.  The source
// list is the engine's circular sentinel+count list (see the class comment);
// pSourceHead is its sentinel node.  Only the CMessage* pointers are copied --
// this message owns them from here on.
CMessage105::CMessage105(Node* pSourceHead, LONG caller, LONG target)
    : CMessage(caller, target)
{
    field_C = static_cast<BYTE>(caller);

    // Start empty: a sentinel node that points to itself.
    m_pNodeHead = new Node;
    m_pNodeHead->m_pNext = m_pNodeHead;
    m_pNodeHead->m_pPrev = m_pNodeHead;
    m_nCount = 0;

    // Self-copy guard (param_1+3 != param_2 in the binary), then append a node
    // per source element at the tail.  The binary inlines a full list operator=
    // (reuse/free/append); against the freshly emptied list only the append
    // branch runs.
    if (pSourceHead != m_pNodeHead) {
        for (Node* pSrc = pSourceHead->m_pNext; pSrc != pSourceHead; pSrc = pSrc->m_pNext) {
            Node* pNode = new Node;
            pNode->m_pNext = m_pNodeHead;
            pNode->m_pPrev = m_pNodeHead->m_pPrev;
            m_pNodeHead->m_pPrev->m_pNext = pNode;
            m_pNodeHead->m_pPrev = pNode;
            pNode->m_pMessage = pSrc->m_pMessage;
            m_nCount++;
        }
    }
}

// 0x515430 (the scalar-deleting destructor thunk is at 0x515410)
CMessage105::~CMessage105()
{
    // Delete every carried sub-message, then free the list nodes and sentinel.
    for (Node* pNode = m_pNodeHead->m_pNext; pNode != m_pNodeHead; pNode = pNode->m_pNext) {
        delete pNode->m_pMessage;
        pNode->m_pMessage = NULL;
    }

    Node* pNode = m_pNodeHead->m_pNext;
    while (pNode != m_pNodeHead) {
        Node* pNext = pNode->m_pNext;
        delete pNode;
        pNode = pNext;
    }

    delete m_pNodeHead;
    m_pNodeHead = NULL;
    m_nCount = 0;
}

// 0x40A0D0 (folded by /OPT:ICF with the other CMessage::GetCommType overrides)
SHORT CMessage105::GetCommType()
{
    return SEND;
}

// 0x40A0E0 (folded by /OPT:ICF)
BYTE CMessage105::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x515400
BYTE CMessage105::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_105;
}

// 0x5154E0
//
// Network form: resolve the target's remote ids, write a 13-byte header (remote
// player id, remote object id, sub-message count, per-sub-message marshalled
// size), then let each sub-message marshal its own bytes after it.  The
// per-sub-message serialisation belongs to CMessageAddEffect (vtable slot 0x10,
// not yet recovered); this routine only frames the list.
void CMessage105::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        *dwSize = 0;
        return;
    }

    PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
    LONG remoteObjectID = pObject->m_remoteObjectID;

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    // Marshalled size of one sub-message (a CMessageAddEffect).
    const DWORD SUBMESSAGE_SIZE = 0x59;

    *dwSize = sizeof(PLAYER_ID) + sizeof(LONG) + sizeof(BYTE) + sizeof(DWORD)
        + m_nCount * SUBMESSAGE_SIZE;
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
    cnt += sizeof(PLAYER_ID);

    *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
    cnt += sizeof(LONG);

    *(*pData + cnt) = static_cast<BYTE>(m_nCount);
    cnt += sizeof(BYTE);

    *reinterpret_cast<DWORD*>(*pData + cnt) = SUBMESSAGE_SIZE;
    cnt += sizeof(DWORD);

    for (Node* pNode = m_pNodeHead->m_pNext; pNode != m_pNodeHead; pNode = pNode->m_pNext) {
        BYTE* pSubData = *pData + cnt;
        DWORD subSize = 0;
        pNode->m_pMessage->MarshalMessage(&pSubData, &subSize);
        cnt += subSize;
    }

    UTIL_ASSERT(cnt == *dwSize);
}

// 0x515670
//
// Inverse of MarshalMessage: resolve the local target id, then rebuild one
// CMessageAddEffect per serialised entry.  Each entry's bytes are copied into a
// scratch buffer past the spec-message header that the sub-message's own
// UnmarshalMessage (vtable slot 0x14) skips.
BOOL CMessage105::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt + sizeof(PLAYER_ID));

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }
    m_targetId = localObjectID;

    BYTE nCount = *(pData + cnt + sizeof(PLAYER_ID) + sizeof(LONG));
    DWORD subSize = *reinterpret_cast<DWORD*>(pData + cnt + sizeof(PLAYER_ID) + sizeof(LONG) + sizeof(BYTE));
    DWORD dataOffset = cnt + sizeof(PLAYER_ID) + sizeof(LONG) + sizeof(BYTE) + sizeof(DWORD);

    BYTE* pSubBuf = new BYTE[CNetwork::SPEC_MSG_HEADER_LENGTH + subSize];

    for (BYTE i = 0; i < nCount; i++) {
        memcpy(pSubBuf + CNetwork::SPEC_MSG_HEADER_LENGTH, pData + dataOffset, subSize);
        dataOffset += subSize;

        CMessageAddEffect* pSub = new CMessageAddEffect(NULL, 0, 0);
        pSub->UnmarshalMessage(pSubBuf, CNetwork::SPEC_MSG_HEADER_LENGTH + subSize);

        Node* pNode = new Node;
        pNode->m_pNext = m_pNodeHead;
        pNode->m_pPrev = m_pNodeHead->m_pPrev;
        m_pNodeHead->m_pPrev->m_pNext = pNode;
        m_pNodeHead->m_pPrev = pNode;
        pNode->m_pMessage = pSub;
        m_nCount++;
    }

    delete[] pSubBuf;
    return TRUE;
}

// 0x5157F0
void CMessage105::Run()
{
    for (Node* pNode = m_pNodeHead->m_pNext; pNode != m_pNodeHead; pNode = pNode->m_pNext) {
        pNode->m_pMessage->Run();
    }
}

// -----------------------------------------------------------------------------

// 0x4F6D80
CMessage103::CMessage103(BOOLEAN a1, PLAYER_ID idPlayer, INT nPortrait, LONG caller, LONG target)
    : CMessage(caller, target)
{
    field_C = a1;
    m_idPlayer = idPlayer;
    m_nCharacterPortraitSlotNumber = nPortrait;
}

// 0x43E170
SHORT CMessage103::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessage103::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6DB0
BYTE CMessage103::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_103;
}

// 0x514AE0
void CMessage103::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30472
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(BOOLEAN)
            + sizeof(PLAYER_ID)
            + sizeof(INT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30494
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = field_C;
        cnt += sizeof(BOOLEAN);

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = m_idPlayer;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<INT*>(*pData + cnt) = m_nCharacterPortraitSlotNumber;
        cnt += sizeof(INT);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30509
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x514C20
BOOL CMessage103::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30525
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    field_C = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    m_idPlayer = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    m_nCharacterPortraitSlotNumber = *reinterpret_cast<INT*>(pData + cnt);
    cnt += sizeof(INT);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x514CB0
void CMessage103::Run()
{
    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30569
        UTIL_ASSERT(pGame != NULL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30572
        UTIL_ASSERT(m_nCharacterPortraitSlotNumber >= -1 && m_nCharacterPortraitSlotNumber < CMultiplayerSettings::MAX_CHARACTERS);

        pGame->GetMultiplayerSettings()->SetViewedCharacter(m_idPlayer, m_nCharacterPortraitSlotNumber);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6DC0
CMessage120::CMessage120(PLAYER_ID idPlayer, BOOLEAN a2, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_idPlayer = idPlayer;
    field_10 = a2;
}

// 0x43E170
SHORT CMessage120::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessage120::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6DF0
BYTE CMessage120::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_120;
}

// 0x514D40
void CMessage120::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30594
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(PLAYER_ID)
            + sizeof(unsigned char);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30615
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = m_idPlayer;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = field_10;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30628
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x514E70
BOOL CMessage120::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 30644
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_idPlayer = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    field_10 = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x514CB0
void CMessage120::Run()
{
    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 30686
        UTIL_ASSERT(pGame != NULL);

        pGame->GetMultiplayerSettings()->SetPlayerReady(m_idPlayer, field_10);
    }
}

// -----------------------------------------------------------------------------

// 0x4536B0
CMessageSetAreaType::CMessageSetAreaType(WORD areaType, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_areaType = areaType;
}

// 0x4536E0
SHORT CMessageSetAreaType::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageSetAreaType::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4536F0
BYTE CMessageSetAreaType::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_TYPE;
}

// 0x515F60
void CMessageSetAreaType::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31544
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameArea* pArea = pObject->GetArea();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 31563
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        // NOTE: Unclear why it obtains `areaType` from target's area instead of
        // using `m_areaType` which is set in constructor.
        *reinterpret_cast<WORD*>(*pData + cnt) = pArea->m_header.m_areaType;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 31570
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x516080
BOOL CMessageSetAreaType::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31594
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_areaType = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31601
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x5160E0
void CMessageSetAreaType::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameArea* pArea = pObject->GetArea();
        if (pArea != NULL) {
            pArea->m_header.m_areaType = m_areaType;
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->GetInfinity()->m_areaType = m_areaType;
            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_header.m_areaType = m_areaType;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x453700
CMessageSetAreaRestEncounter::CMessageSetAreaRestEncounter(WORD probDay, WORD probNight, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_probDay = probDay;
    m_probNight = probNight;
}

// 0x4536E0
SHORT CMessageSetAreaRestEncounter::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageSetAreaRestEncounter::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453730
BYTE CMessageSetAreaRestEncounter::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_REST_ENCOUNTER;
}

// 0x5161D0
void CMessageSetAreaRestEncounter::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31673
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameArea* pArea = pObject->GetArea();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(WORD) + sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 31695
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        // NOTE: Unclear why it obtains `probDay` from target's area instead of
        // using `m_probDay` which is set in constructor.
        *reinterpret_cast<WORD*>(*pData + cnt) = pArea->GetHeaderRestEncounter()->m_probDay;
        cnt += sizeof(WORD);

        // NOTE: Unclear why it obtains `probNight` from target's area instead of
        // using `m_probNight` which is set in constructor.
        *reinterpret_cast<WORD*>(*pData + cnt) = pArea->GetHeaderRestEncounter()->m_probNight;
        cnt += sizeof(WORD);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 31704
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x516300
BOOL CMessageSetAreaRestEncounter::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31728
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_probDay = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    m_probNight = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31737
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x516370
void CMessageSetAreaRestEncounter::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        CGameArea* pArea = pObject->GetArea();
        if (pArea != NULL) {
            pArea->GetHeaderRestEncounter()->m_probDay = m_probDay;
            pArea->GetHeaderRestEncounter()->m_probNight = m_probNight;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x453490
CMessageSaveGame::CMessageSaveGame(STRREF strRef, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_strRef = strRef;
}

// 0x43E170
SHORT CMessageSaveGame::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageSaveGame::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4534B0
BYTE CMessageSaveGame::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SAVE_GAME;
}

// 0x504C40
void CMessageSaveGame::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(STRREF);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    // Binary 0x504C40 stores only the low byte despite advertising a
    // four-byte payload; keep the original wire layout.
    **pData = static_cast<BYTE>(m_strRef);
}

// 0x504C80
BOOL CMessageSaveGame::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_strRef = *reinterpret_cast<STRREF*>(pData + cnt);
    cnt += sizeof(STRREF);

    // NOTE: Binary omits the trailing size guard here.

    return TRUE;
}

// 0x504CA0
void CMessageSaveGame::Run()
{
    if (g_pChitin->cNetwork.GetSessionOpen()
        && g_pChitin->cNetwork.GetSessionHosting() != TRUE) {
        return;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    STR_RES strRes;
    BOOLEAN bStrRefOn = CTlkTable::STRREF_ON;
    if (bStrRefOn == TRUE) {
        CTlkTable::STRREF_ON = FALSE;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(m_strRef, strRes);

    CString sSaveName;
    sSaveName.Format("000000000-%s - %s",
        static_cast<LPCSTR>(CBaldurEngine::FetchString(0x9BFF)),
        static_cast<LPCSTR>(strRes.szText));

    if (pGame->GetOptions()->m_nNightmareMode == 1) {
        STR_RES strNightmareMode;
        if (g_pBaldurChitin->GetTlkTable().Fetch(0xA193, strNightmareMode) == TRUE) {
            CString sSuffix;
            sSuffix.Format(" (%s)", static_cast<LPCSTR>(strNightmareMode.szText));
            sSaveName += sSuffix;
        }
    }

    if (bStrRefOn == TRUE) {
        CTlkTable::STRREF_ON = TRUE;
    }

    DWORD areaFlags[CINFGAME_MAX_AREAS];
    memset(areaFlags, 0, sizeof(areaFlags));

    for (SHORT nArea = 0; nArea < CINFGAME_MAX_AREAS; nArea++) {
        CGameArea* pArea = pGame->m_gameAreas[nArea];
        if (pArea != NULL) {
            areaFlags[nArea] = pArea->m_header.m_flags;
            pArea->m_header.m_flags &= ~1u;
        }
    }

    pGame->m_sSaveGame = sSaveName;
    CScreenCharacter::SAVE_NAME = sSaveName;

    pGame->m_bSaveScreen = 1;
    pGame->field_50DC = 0;

    BOOL bRestoreDisplayStale = FALSE;

    if (!g_pChitin->cVideo.Is3dAccelerated()) {
        if (pGame->field_50D8 == FALSE) {
            // The original pump serializes this path. In the RE build the
            // message thread can overlap the render-thread synchronous update;
            // hold the pending repaint so an older capture cannot consume the
            // save-screen flag armed below for the deferred copy pass.
            bRestoreDisplayStale = g_pChitin->m_bDisplayStale;
            g_pChitin->m_bDisplayStale = FALSE;
            while (g_pChitin->m_bInSynchronousUpdate) {
                SleepEx(10, FALSE);
            }
            pGame->SynchronousUpdate();
        }
        pGame->field_50D8 = TRUE;
    }

    if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineWorld) {
        pGame->field_50D8 = TRUE;
    } else {
        pGame->field_50D8 = FALSE;
    }


    pGame->SaveGame(0, 0, 1);

    if (bRestoreDisplayStale || (pGame->m_bSaveScreen != 0 && pGame->field_50D8 != FALSE)) {
        g_pChitin->m_bDisplayStale = TRUE;
    }


    for (SHORT nArea = 0; nArea < CINFGAME_MAX_AREAS; nArea++) {
        CGameArea* pArea = pGame->m_gameAreas[nArea];
        if (pArea != NULL) {
            pArea->m_header.m_flags = areaFlags[nArea];
        }
    }
}

// -----------------------------------------------------------------------------

// 0x453670
CMessageToggleInterface::CMessageToggleInterface(BOOLEAN bHide, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bHide = bHide;
}

// 0x453510
SHORT CMessageToggleInterface::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageToggleInterface::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453690
BYTE CMessageToggleInterface::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_TOGGLE_INTERFACE;
}

// 0x515810
void CMessageToggleInterface::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31130
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31135
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bHide;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31152
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5158A0
BOOL CMessageToggleInterface::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31176
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_bHide = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x5158E0
void CMessageToggleInterface::Run()
{
    if (m_bHide) {
        if (!g_pBaldurChitin->GetScreenWorld()->GetManager()->m_bHidden) {
            g_pBaldurChitin->GetScreenWorld()->HideInterface();
            g_pBaldurChitin->GetScreenWorld()->m_nAutoUnhideInterface++;
        }
    } else {
        if (g_pBaldurChitin->GetScreenWorld()->m_nAutoUnhideInterface > 0) {
            g_pBaldurChitin->GetScreenWorld()->UnhideInterface();
            g_pBaldurChitin->GetScreenWorld()->m_nAutoUnhideInterface--;
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4F6E20
CMessage107::CMessage107(unsigned char a1, LONG caller, LONG target)
    : CMessage(caller, target)
{
    field_C = a1;
}

// 0x43E170
SHORT CMessage107::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessage107::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6E40
BYTE CMessage107::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_107;
}

// 0x515950
void CMessage107::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31259
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(PLAYER_ID)
            + sizeof(unsigned char);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 31279
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<unsigned char*>(*pData + cnt) = field_C;
        cnt += sizeof(unsigned char);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 31302
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x515A80
BOOL CMessage107::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31318
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    field_C = *reinterpret_cast<unsigned char*>(pData + cnt);
    cnt += sizeof(unsigned char);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    // NOTE: Missing trailing guard.

    return TRUE;
}

// 0x515B00
void CMessage107::Run()
{
    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
            static_cast<CGameSprite*>(pObject)->field_580 = field_C;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// -----------------------------------------------------------------------------

// 0x4F6E00
CMessageSetAreaExplored::CMessageSetAreaExplored(BOOLEAN bExplored, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_bExplored = bExplored;
}

// 0x453510
SHORT CMessageSetAreaExplored::GetCommType()
{
    return BROADCAST;
}

// 0x40A0E0
BYTE CMessageSetAreaExplored::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4536A0
BYTE CMessageSetAreaExplored::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_AREA_EXPLORED;
}

// 0x516420
void CMessageSetAreaExplored::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31807
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31812
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_bExplored;
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31820
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5164B0
BOOL CMessageSetAreaExplored::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31844
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    m_bExplored = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31851
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x516510
void CMessageSetAreaExplored::Run()
{
    CGameArea* pArea = g_pBaldurChitin->GetObjectGame()->GetVisibleArea();
    if (pArea != NULL) {
        if (m_bExplored) {
            pArea->m_visibility.SetAreaExplored();
        } else {
            pArea->m_visibility.SetAreaUnexplored();
        }
    }
}

// -----------------------------------------------------------------------------

// 0x4531F0
CMessageEndGame::CMessageEndGame(int a1, int a2, LONG caller, LONG target)
    : CMessage(caller, target)
{
    field_C = a1;
    field_10 = a2;
}

// 0x453330
CMessageEndGame::~CMessageEndGame()
{
}

// 0x4088A0
SHORT CMessageEndGame::GetCommType()
{
    return BROADCAST_OTHERS;
}

// 0x40A0E0
BYTE CMessageEndGame::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453300
BYTE CMessageEndGame::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_END_GAME;
}

// 0x516550
void CMessageEndGame::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31915
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    *dwSize = sizeof(int) + sizeof(int);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31921
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

    DWORD cnt = 0;

    *reinterpret_cast<int*>(*pData + cnt) = field_C;
    cnt += sizeof(int);

    *reinterpret_cast<int*>(*pData + cnt) = field_10;
    cnt += sizeof(int);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31931
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5165E0
BOOL CMessageEndGame::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31955
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    field_C = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    field_10 = *reinterpret_cast<int*>(pData + cnt);
    cnt += sizeof(int);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 31964
    UTIL_ASSERT(cnt == dwSize);

    return TRUE;
}

// 0x516650
void CMessageEndGame::Run()
{
    g_pBaldurChitin->GetObjectGame()->ReadyCharacterTerminationSequence(field_C, field_10);
}

// -----------------------------------------------------------------------------

// 0x453740
CMessageChangeStat::CMessageChangeStat(WORD stat, DWORD value, BOOLEAN modify, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_stat = stat;
    m_value = value;
    m_modify = modify;
}

// 0x4536E0
SHORT CMessageChangeStat::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessageChangeStat::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453770
BYTE CMessageChangeStat::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_CHANGE_STAT;
}

// 0x516670
void CMessageChangeStat::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 32006
    UTIL_ASSERT(pData != NULL && dwSize != NULL);

    CGameObject* pObject;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        PLAYER_ID remotePlayerID = pObject->m_remotePlayerID;
        LONG remoteObjectID = pObject->m_remoteObjectID;

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        *dwSize = sizeof(PLAYER_ID)
            + sizeof(LONG)
            + sizeof(WORD)
            + sizeof(DWORD)
            + sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 32028
        UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);

        DWORD cnt = 0;

        *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = remotePlayerID;
        cnt += sizeof(PLAYER_ID);

        *reinterpret_cast<LONG*>(*pData + cnt) = remoteObjectID;
        cnt += sizeof(LONG);

        *reinterpret_cast<WORD*>(*pData + cnt) = m_stat;
        cnt += sizeof(WORD);

        *reinterpret_cast<DWORD*>(*pData + cnt) = m_value;
        cnt += sizeof(DWORD);

        *reinterpret_cast<BOOLEAN*>(*pData + cnt) = m_modify;
        cnt += sizeof(BOOLEAN);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
        // __LINE__: 32044
        UTIL_ASSERT(cnt == *dwSize);
    } else {
        *dwSize = 0;
    }
}

// 0x5167B0
BOOL CMessageChangeStat::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 32071
    UTIL_ASSERT(pData != NULL);

    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;

    PLAYER_ID remotePlayerID = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);

    LONG remoteObjectID = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);

    LONG localObjectID;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(remotePlayerID, remoteObjectID, localObjectID) != TRUE) {
        return FALSE;
    }

    m_targetId = localObjectID;

    m_stat = *reinterpret_cast<WORD*>(pData + cnt);
    cnt += sizeof(WORD);

    m_value = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);

    m_modify = *reinterpret_cast<BOOLEAN*>(pData + cnt);
    cnt += sizeof(BOOLEAN);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CMessage.cpp
    // __LINE__: 32099
    UTIL_ASSERT(pData != NULL);

    return TRUE;
}

// 0x516870
void CMessageChangeStat::Run()
{
    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pSprite->SetAtOffset(m_stat,
            m_value,
            m_modify);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// ---------------------------------------------------------------------------
// CMessageProjectileTrailingVFX (vtable 0x84D328)
// ---------------------------------------------------------------------------

// 0x554D20 (factory stub — see CProjectile.cpp)
CMessageProjectileTrailingVFX::CMessageProjectileTrailingVFX(LONG target, LONG flightDistSq,
    const CResRef& resRef, LONG launchX, LONG launchY,
    LONG nHeight, LONG nType, SHORT launchHeight)
    : CMessage(target, target)
{
    // m_sourceId is repurposed as m_flightDistSq.
    m_sourceId = flightDistSq;

    m_resRef = resRef;
    m_resRef2 = "";
    m_launchX = launchX;
    m_launchY = launchY;
    m_nHeight = nHeight;
    m_nType = nType;
    m_launchHeight = launchHeight;
}

// 0x4088A0
SHORT CMessageProjectileTrailingVFX::GetCommType()
{
    return CMessage::SEND;
}

// 0x40A0E0
BYTE CMessageProjectileTrailingVFX::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6A70
BYTE CMessageProjectileTrailingVFX::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_PROJECTILE_TRAILING_VFX;
}

// 0x511D00
void CMessageProjectileTrailingVFX::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    // STUB: network serialisation not yet recovered (0x511D00, ~100 bytes).
    // Not exercised by single-player / local-object messages.
}

// 0x511F40
BOOL CMessageProjectileTrailingVFX::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    // STUB: network deserialisation not yet recovered (0x511F40, ~111 bytes).
    // Not exercised by single-player / local-object messages.
    return FALSE;
}

// 0x799CA0
void CMessageProjectileTrailingVFX::Run()
{
    // No-op; the message is a data carrier. The actual attachment work
    // happens in the factory (0x554D20) and the post-dispatch object-flag
    // modification in IcewindCProjectileTravellingVFX::Fire.
}

// Constructor inlined in IcewindCGameEffectWhenStruck::ApplyEffect.
CMessage117::CMessage117(const CResRef& sourceRes, const CResRef& spellRes,
    DWORD level, const CPoint& targetPoint, LONG caller, LONG target)
    : CMessage(caller, target)
{
    m_sourceRes = sourceRes;
    m_spellRes = spellRes;
    m_level = level;
    m_target = targetPoint;
}

// 0x4536E0
SHORT CMessage117::GetCommType()
{
    return BROADCAST_FORCED_OTHERS;
}

// 0x40A0E0
BYTE CMessage117::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x4F6FA0
BYTE CMessage117::GetMsgSubType()
{
    return 117;
}

// 0x516910
void CMessage117::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    UTIL_ASSERT(pData != NULL && dwSize != NULL);
    CGameObject* object;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        *dwSize = 0;
        return;
    }
    PLAYER_ID player = object->m_remotePlayerID;
    LONG remote = object->m_remoteObjectID;
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
    *dwSize = 36;
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);
    DWORD cnt = 0;
    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = player;
    cnt += sizeof(PLAYER_ID);
    *reinterpret_cast<LONG*>(*pData + cnt) = remote;
    cnt += sizeof(LONG);
    memcpy(*pData + cnt, m_sourceRes.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;
    memcpy(*pData + cnt, m_spellRes.GetResRef(), RESREF_SIZE);
    cnt += RESREF_SIZE;
    *reinterpret_cast<DWORD*>(*pData + cnt) = m_level;
    cnt += sizeof(DWORD);
    *reinterpret_cast<LONG*>(*pData + cnt) = m_target.x;
    cnt += sizeof(LONG);
    *reinterpret_cast<LONG*>(*pData + cnt) = m_target.y;
    cnt += sizeof(LONG);
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x516A80
BOOL CMessage117::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    UTIL_ASSERT(pData != NULL);
    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;
    PLAYER_ID player = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);
    LONG remote = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);
    LONG local;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(player, remote, local) != TRUE) {
        return FALSE;
    }
    m_targetId = local;
    m_sourceRes = pData + cnt;
    cnt += RESREF_SIZE;
    m_spellRes = pData + cnt;
    cnt += RESREF_SIZE;
    m_level = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);
    m_target.x = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);
    m_target.y = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);
    UTIL_ASSERT(cnt == dwSize);
    return TRUE;
}

// 0x516B70
void CMessage117::Run()
{
    CGameObject* object;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }
    if (object->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* sprite = static_cast<CGameSprite*>(object);
        sprite->m_derivedStats.m_onHitSpells.clear();
        COnHitSpell spell;
        spell.m_sourceRes = m_sourceRes;
        spell.m_spellRes = m_spellRes;
        spell.m_level = m_level;
        spell.m_target = m_target;
        sprite->m_derivedStats.m_onHitSpells.push_back(spell);
    }
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// Constructor is inlined at 0x6F5235..0x6F52AC.
CMessageSetVariable::CMessageSetVariable(const CString& area, const CString& name,
    LONG value, BYTE increment, LONG caller, LONG target)
    : CMessage(caller, target)
    , m_sAreaName(area)
    , m_sGlobalName(name)
    , m_nValue(value)
    , m_bIncrement(increment)
    , m_flags(0)
{
}

// 0x43E170
SHORT CMessageSetVariable::GetCommType()
{
    return BROADCAST_FORCED;
}

// 0x40A0E0
BYTE CMessageSetVariable::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}

// 0x453560
BYTE CMessageSetVariable::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SET_VARIABLE;
}

// 0x50A4D0
void CMessageSetVariable::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    UTIL_ASSERT(pData != NULL && dwSize != NULL);
    CGameObject* object;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        *dwSize = 0;
        return;
    }
    PLAYER_ID player = object->m_remotePlayerID;
    LONG remote = object->m_remoteObjectID;
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
    *dwSize = 21 + m_sGlobalName.GetLength() + m_sAreaName.GetLength();
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);
    DWORD cnt = 0;
    *reinterpret_cast<PLAYER_ID*>(*pData + cnt) = player;
    cnt += sizeof(PLAYER_ID);
    *reinterpret_cast<LONG*>(*pData + cnt) = remote;
    cnt += sizeof(LONG);
    SHORT length = static_cast<SHORT>(m_sGlobalName.GetLength());
    *reinterpret_cast<SHORT*>(*pData + cnt) = length;
    cnt += sizeof(SHORT);
    memcpy(*pData + cnt, m_sGlobalName.GetBuffer(length), length);
    cnt += length;
    length = static_cast<SHORT>(m_sAreaName.GetLength());
    *reinterpret_cast<SHORT*>(*pData + cnt) = length;
    cnt += sizeof(SHORT);
    memcpy(*pData + cnt, m_sAreaName.GetBuffer(length), length);
    cnt += length;
    *reinterpret_cast<LONG*>(*pData + cnt) = m_nValue;
    cnt += sizeof(LONG);
    (*pData)[cnt++] = m_bIncrement;
    *reinterpret_cast<DWORD*>(*pData + cnt) = m_flags;
    cnt += sizeof(DWORD);
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x50A6C0
BOOL CMessageSetVariable::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    UTIL_ASSERT(pData != NULL);
    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;
    PLAYER_ID player = *reinterpret_cast<PLAYER_ID*>(pData + cnt);
    cnt += sizeof(PLAYER_ID);
    LONG remote = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);
    LONG local;
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(player, remote, local) == TRUE) {
        m_targetId = local;
    } else {
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }
    SHORT length = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);
    m_sGlobalName = CString(reinterpret_cast<const char*>(pData + cnt), length);
    cnt += length;
    length = *reinterpret_cast<SHORT*>(pData + cnt);
    cnt += sizeof(SHORT);
    m_sAreaName = CString(reinterpret_cast<const char*>(pData + cnt), length);
    cnt += length;
    m_nValue = *reinterpret_cast<LONG*>(pData + cnt);
    cnt += sizeof(LONG);
    m_bIncrement = pData[cnt++];
    m_flags = *reinterpret_cast<DWORD*>(pData + cnt);
    cnt += sizeof(DWORD);
    UTIL_ASSERT(cnt == dwSize);
    return TRUE;
}

// 0x50A820
void CMessageSetVariable::Run()
{
    CGameObject* object = NULL;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
            CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (object != NULL && (m_flags & 6) != 0) {
        if ((m_flags & 2) != 0) {
            CGameArea* area = g_pBaldurChitin->GetObjectGame()->GetArea(m_sAreaName);
            if (area != NULL) {
                area->GetNamedCreatures()->RemoveKey(m_sGlobalName, TRUE, object->m_id);
            }
        }
        if ((m_flags & 4) != 0) {
            g_pBaldurChitin->GetObjectGame()->GetNamedCreatures()->RemoveKey(m_sGlobalName, TRUE, object->m_id);
        }
    } else {
        auto update = [&](CVariableHashBase* variables, const CString& name) {
            CVariable* variable = variables->FindKey(name);
            if (variable == NULL) {
                variable = new CVariable;
                variable->SetName(name);
                variable->m_intValue = m_nValue;
                variables->AddKey(*variable);
                delete variable;
            } else if (m_bIncrement == 1) {
                variable->m_intValue += m_nValue;
            } else {
                variable->m_intValue = m_nValue;
            }
        };
        if (strcmp(m_sAreaName, "GLOBAL") == 0) {
            update(g_pBaldurChitin->GetObjectGame()->GetVariables(), m_sGlobalName);
        }
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }
        if (strcmp(m_sAreaName, "LOCALS") == 0) {
            if (object != NULL && object->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CString name(m_sGlobalName);
                update(static_cast<CGameSprite*>(object)->GetLocalVariables(), name);
            }
        } else {
            if (strcmp(m_sAreaName, "MYAREA") == 0) {
                m_sAreaName = reinterpret_cast<const char*>(object->m_pArea->m_header.m_areaName);
            }
            CGameArea* area = g_pBaldurChitin->GetObjectGame()->GetArea(m_sAreaName);
            if (area != NULL) {
                update(area->GetVariables(), m_sGlobalName);
            }
        }
    }
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
        CGameObjectArray::THREAD_ASYNCH, INFINITE);
}


// 0x4F8D60
void CMessageAddItem::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    CResRef res;
    CString name;
    UTIL_ASSERT(pData != NULL && dwSize != NULL);
    CGameObjectArray* array = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
    CGameObject* object;
    BYTE rc;
    do {
        rc = array->GetShare(m_targetId, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        *dwSize = 0;
        return;
    }
    PLAYER_ID player = object->m_remotePlayerID;
    LONG id = object->m_remoteObjectID;
    array->ReleaseShare(m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    res = m_item.cResRef;
    res.CopyToString(name);
    BYTE length = static_cast<BYTE>(name.GetLength());
    *dwSize = 21 + length;
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);
    DWORD cnt = 0;
    memcpy(*pData + cnt, &player, sizeof(player)); cnt += sizeof(player);
    memcpy(*pData + cnt, &id, sizeof(id)); cnt += sizeof(id);
    (*pData)[cnt++] = length;
    memcpy(*pData + cnt, name.GetBuffer(name.GetLength()), name.GetLength()); cnt += name.GetLength();
    for (INT ability = 0; ability < 3; ability++) {
        WORD uses = m_item.GetUsageCount(ability);
        memcpy(*pData + cnt, &uses, sizeof(uses)); cnt += sizeof(uses);
    }
    memcpy(*pData + cnt, &m_item.m_wear, sizeof(m_item.m_wear)); cnt += sizeof(m_item.m_wear);
    memcpy(*pData + cnt, &m_item.m_flags, sizeof(m_item.m_flags)); cnt += sizeof(m_item.m_flags);
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4F8FA0
BOOL CMessageAddItem::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    UTIL_ASSERT(pData != NULL);
    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;
    PLAYER_ID player;
    LONG id, local;
    memcpy(&player, pData + cnt, sizeof(player)); cnt += sizeof(player);
    memcpy(&id, pData + cnt, sizeof(id)); cnt += sizeof(id);
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(player, id, local) != TRUE) return FALSE;
    m_targetId = local;
    BYTE length = pData[cnt++];
    char name[9] = {0};
    memcpy(name, pData + cnt, length); cnt += length;
    WORD uses[3], wear;
    DWORD flags;
    memcpy(uses, pData + cnt, sizeof(uses)); cnt += sizeof(uses);
    memcpy(&wear, pData + cnt, sizeof(wear)); cnt += sizeof(wear);
    memcpy(&flags, pData + cnt, sizeof(flags)); cnt += sizeof(flags);
    {
        CItem item(CResRef(reinterpret_cast<BYTE*>(name)), uses[0], uses[1], uses[2], wear, flags);
        m_item.SetResRef(item.cResRef, TRUE);
        m_item.m_useCount1 = item.m_useCount1;
        m_item.m_useCount2 = item.m_useCount2;
        m_item.m_useCount3 = item.m_useCount3;
        m_item.m_wear = item.m_wear;
        m_item.m_flags = item.m_flags;
    }
    UTIL_ASSERT(cnt == dwSize);
    return TRUE;
}

// 0x504070
void CMessageRemoveItem::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    UTIL_ASSERT(pData != NULL && dwSize != NULL);
    CGameObjectArray* array = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
    CGameObject* object;
    BYTE rc;
    do {
        rc = array->GetShare(m_targetId, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) {
        *dwSize = 0;
        return;
    }
    PLAYER_ID player = object->m_remotePlayerID;
    LONG id = object->m_remoteObjectID;
    array->ReleaseShare(m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    *dwSize = sizeof(player) + sizeof(id) + sizeof(m_slot);
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);
    DWORD cnt = 0;
    memcpy(*pData + cnt, &player, sizeof(player)); cnt += sizeof(player);
    memcpy(*pData + cnt, &id, sizeof(id)); cnt += sizeof(id);
    memcpy(*pData + cnt, &m_slot, sizeof(m_slot)); cnt += sizeof(m_slot);
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x5041A0
BOOL CMessageRemoveItem::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    UTIL_ASSERT(pData != NULL);
    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;
    PLAYER_ID player;
    LONG id, local;
    memcpy(&player, pData + cnt, sizeof(player)); cnt += sizeof(player);
    memcpy(&id, pData + cnt, sizeof(id)); cnt += sizeof(id);
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(player, id, local) != TRUE) return FALSE;
    m_targetId = local;
    memcpy(&m_slot, pData + cnt, sizeof(m_slot)); cnt += sizeof(m_slot);
    UTIL_ASSERT(cnt == dwSize);
    return TRUE;
}

// 0x4F9150
void CMessageAddItem::Run()
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* array = game->GetObjectArray();
    CGameObject* object;
    BYTE rc;
    do {
        rc = array->GetDeny(m_targetId, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) return;
    if (object->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* sprite = static_cast<CGameSprite*>(object);
        BOOL placed = FALSE;
        for (int index = 0; index < CScreenInventory::PERSONAL_INVENTORY_SIZE && !placed; index++) {
            if (sprite->m_equipment.m_items[index + 18] == NULL) {
                sprite->m_equipment.m_items[index + 18] = new CItem(m_item);
                sprite->Equip(static_cast<SHORT>(index + 18));
                placed = TRUE;
            }
        }
        if (!placed) {
            CItem* item = new CItem(m_item);
            sprite->FeedBack(CGameSprite::FEEDBACK_ITEM_DROPPED_FULL, 0, 0, 0, -1, 0, 0);
            sprite->PutItemGround(item);
            if (game->GetCharacterPortraitNum(sprite->m_id) != -1) game->FeedBack(CInfGame::FEEDBACK_INVENTORYFULL_ITEMDROPPED, 0, TRUE);
        } else {
            CMessage* message = new CMessageSpriteEquipment(sprite, sprite->m_id, sprite->m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE, message->GetCommType());
        }
        static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine)->UpdatePersonalItemStatus(m_targetId);
    }
    array->ReleaseDeny(m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
}

// 0x504250
void CMessageRemoveItem::Run()
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    CGameObjectArray* array = game->GetObjectArray();
    CGameObject* object;
    BYTE rc;
    do {
        rc = array->GetDeny(m_targetId, CGameObjectArray::THREAD_ASYNCH, &object, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc != CGameObjectArray::SUCCESS) return;
    if (object->GetObjectType() == CGameObject::TYPE_SPRITE) {
        CGameSprite* sprite = static_cast<CGameSprite*>(object);
        if (sprite->m_equipment.m_items[m_slot] != NULL) {
            sprite->Unequip(m_slot);
            game->AddDisposableItem(sprite->m_equipment.m_items[m_slot]);
            sprite->m_equipment.m_items[m_slot] = NULL;
            if (m_slot == sprite->m_equipment.m_selectedWeapon) sprite->EquipMostDamagingMelee();
        }
        CMessage* message = new CMessageSpriteEquipment(sprite, sprite->m_id, sprite->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE, message->GetCommType());
        static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine)->UpdatePersonalItemStatus(m_targetId);
        if (m_slot >= 43 && m_slot <= 50) {
            sprite->SetQuickWeapon(static_cast<BYTE>(m_slot - 43), CButtonData());
            sprite->SetQuickWeapon(static_cast<BYTE>(m_slot - 43), static_cast<BYTE>(0));
        } else if (m_slot >= 15 && m_slot <= 17) sprite->SetQuickItem(static_cast<BYTE>(m_slot - 15), CButtonData());
        if ((static_cast<int>(m_slot) - 43) / 2 == sprite->m_nWeaponSet) {
            sprite->UpdateQuickWeaponData();
            if (game->GetCharacterPortraitNum(sprite->m_id) == g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter()) game->GetButtonArray()->UpdateButtons();
        }
    }
    array->ReleaseDeny(m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
}
