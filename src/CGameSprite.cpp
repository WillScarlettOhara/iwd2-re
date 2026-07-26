#include "CGameSprite.h"

#include "CAIScript.h"
#include "CAIUtil.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CBlood.h"
#include "CGameAnimationType.h"
#include "CGameArea.h"
#include "CGameButtonList.h"
#include "CGameContainer.h"
#include "CGameDoor.h"
#include "CGamePermission.h"
#include "CGameTimer.h"
#include "CGameTrigger.h"
#include "DebugLog.h"
#include "CInfCursor.h"
#include "CInfGame.h"
#include "CItem.h"
#include "CPathSearch.h"
#include "CProjectile.h"
#include "CScreenCharacter.h"
#include "CScreenCreateChar.h"
#include "CScreenInventory.h"
#include "CScreenMap.h"
#include "CScreenWorld.h"
#include "CScreenWorldMap.h"
#include "CSoundMixer.h"
#include "CSpawn.h"
#include "CSpell.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "CVariableHash.h"
#include "CVidInf.h"
#include "Icewind586B70.h"
#include "IcewindCGameEffects.h"
#include "IcewindCVisualEffect.h"
#include "IcewindMisc.h"

// 0x85BB38
const BYTE CGameSprite::DIR_S = 0;

// 0x85BB39
const BYTE CGameSprite::DIR_SW = 2;

// 0x85BB3A
const BYTE CGameSprite::DIR_W = 4;

// 0x85BB3B
const BYTE CGameSprite::DIR_NW = 6;

// 0x85BB3C
const BYTE CGameSprite::DIR_N = 8;

// 0x85BB3D
const BYTE CGameSprite::DIR_NE = 10;

// 0x85BB3E
const BYTE CGameSprite::DIR_E = 12;

// 0x85BB3F
const BYTE CGameSprite::DIR_SE = 14;

// 0x85BB40
const BYTE CGameSprite::DIR_SSW = 1;

// 0x85BB41
const BYTE CGameSprite::DIR_SWW = 3;

// 0x85BB42
const BYTE CGameSprite::DIR_NWW = 5;

// 0x85BB43
const BYTE CGameSprite::DIR_NNW = 7;

// 0x85BB44
const BYTE CGameSprite::DIR_NNE = 9;

// 0x85BB45
const BYTE CGameSprite::DIR_NEE = 11;

// 0x85BB46
const BYTE CGameSprite::DIR_SEE = 13;

// 0x85BB47
const BYTE CGameSprite::DIR_SSE = 15;

// 0x85BB48
const BYTE CGameSprite::NUM_DIR = 16;

// 0x85BB4A
const BYTE CGameSprite::SPRITE_EFFECT_FIRE = 1;

// 0x85BB4B
const BYTE CGameSprite::SPRITE_EFFECT_ELECTRICITY = 2;

// 0x85BB4C
const BYTE CGameSprite::SPRITE_EFFECT_COLD = 3;

// 0x85BB4D
const BYTE CGameSprite::SPRITE_EFFECT_ACID = 4;

// 0x85BB4E
const BYTE CGameSprite::SPRITE_EFFECT_INTENSITY_LOW = 0;

// 0x85BB69
const BYTE CGameSprite::SOUND_DAMAGE = 6;

// 0x85BB6A
const BYTE CGameSprite::SOUND_DYING = 7;

// 0x85BB6B
const BYTE CGameSprite::SOUND_BADLY_WOUNDED = 8;

// 0x85BB6C
const BYTE CGameSprite::SOUND_SELECT = 9;

// 0x85BB6E
const BYTE CGameSprite::SOUND_SELECT_ACTION = 11;

// 0x85BB70
const BYTE CGameSprite::SOUND_REACT_TO_DEATH = 13;

// 0x85BB74
const SHORT CGameSprite::EXACT_SCALE = 10;

// 0x85BB9A
const SHORT CGameSprite::READY_COUNT = 75;

// 0x85BB9C
const SHORT CGameSprite::USER_OVERRIDE_COUNT = 75;

// 0x85BBB0
const BYTE CGameSprite::SEQ_ATTACK = CGAMESPRITE_SEQ_ATTACK;

// 0x85BBB1
const BYTE CGameSprite::SEQ_AWAKE = CGAMESPRITE_SEQ_AWAKE;

// 0x85BBB2
const BYTE CGameSprite::SEQ_CAST = CGAMESPRITE_SEQ_CAST;

// 0x85BBB3
const BYTE CGameSprite::SEQ_CONJURE = CGAMESPRITE_SEQ_CONJURE;

// 0x85BBB4
const BYTE CGameSprite::SEQ_DAMAGE = CGAMESPRITE_SEQ_DAMAGE;

// 0x85BBB5
const BYTE CGameSprite::SEQ_DIE = CGAMESPRITE_SEQ_DIE;

// 0x85BBB6
const BYTE CGameSprite::SEQ_EMERGE = CGAMESPRITE_SEQ_EMERGE;

// 0x85BBB7
const BYTE CGameSprite::SEQ_HEAD_TURN = CGAMESPRITE_SEQ_HEAD_TURN;

// 0x85BBB8
const BYTE CGameSprite::SEQ_HIDE = CGAMESPRITE_SEQ_HIDE;

// 0x85BBB9
const BYTE CGameSprite::SEQ_READY = CGAMESPRITE_SEQ_READY;

// 0x85BBBA
const BYTE CGameSprite::SEQ_SHOOT = CGAMESPRITE_SEQ_SHOOT;

// 0x85BBBB
const BYTE CGameSprite::SEQ_SLEEP = CGAMESPRITE_SEQ_SLEEP;

// 0x85BBBC
const BYTE CGameSprite::SEQ_TWITCH = CGAMESPRITE_SEQ_TWITCH;

// 0x85BBBD
const BYTE CGameSprite::SEQ_WALK = CGAMESPRITE_SEQ_WALK;

// 0x85BBBE
const BYTE CGameSprite::SEQ_ATTACK_SLASH = CGAMESPRITE_SEQ_ATTACK_SLASH;

// 0x85BBBF
const BYTE CGameSprite::SEQ_ATTACK_BACKSLASH = CGAMESPRITE_SEQ_ATTACK_BACKSLASH;

// 0x85BBC0
const BYTE CGameSprite::SEQ_ATTACK_JAB = CGAMESPRITE_SEQ_ATTACK_JAB;

// 0x85BBD0
const ITEM_ABILITY CGameSprite::DEFAULT_ATTACK = {
    1, // type
    1, // quickSlotType
    0, // largeDamageDice
    { 0 }, // quickSlotIcon
    1, // actionType
    1, // actionCount
    1, // range
    0, // launcherType
    0, // largeDamageDiceCount
    0, // speedFactor
    0, // largeDamageDiceBonus
    0, // thac0Bonus
    2, // damageDice
    9, // school
    1, // damageDiceCount
    0, // secondaryType
    0, // damageDiceBonus
    2, // damageType
    0, // effectCount
    0, // startingEffect
    WORD_MAX, // maxUsageCount
    0, // usageFlags
    0, // abilityFlags
    0, // missileType
    { 0 }, // attackProbability
};

// 0x85BCA0
const LONG CGameSprite::STANDARD_VERBAL_CONSTANT_LENGTH = 300;

// 0x84C7B8
const LONG CGameSprite::LEAVEAREA_GATHER_RANGE = 0x10000;

// 0x85C09C
const SHORT CGameSprite::FLY_RIGHT = -1;

// 0x85C09E
const SHORT CGameSprite::FLY_LEFT = 1;

// 0x85C0A0
const SHORT CGameSprite::FLY_RADIUS_RANDOM = 0x380;

// 0x85C0A2
const SHORT CGameSprite::FLY_RADIUS_BASE = 0x80;

// 0x85C0A4
const SHORT CGameSprite::FLY_0 = 0;

// 0x85C0A6
const SHORT CGameSprite::FLY_90 = 1;

// 0x85C0A8
const SHORT CGameSprite::FLY_180 = 2;

// 0x85C0AA
const SHORT CGameSprite::FLY_270 = 3;

// 0x85C0AC
const SHORT CGameSprite::FLY_360 = 4;

// 0x85C0B4
const WORD CGameSprite::FEEDBACK_BACKSTAB = 1;

// 0x85C0B6
const WORD CGameSprite::FEEDBACK_CRITICALHIT = 2;

// 0x85C0B8
const WORD CGameSprite::FEEDBACK_CRITICALMISS = 3;

// 0x85C0BA
const WORD CGameSprite::FEEDBACK_TOHIT = 4;

// 0x85C0BC
const WORD CGameSprite::FEEDBACK_SPELL = 5;

// 0x85C0BE
const WORD CGameSprite::FEEDBACK_ATTACKS = 6;

// 0x85C0C0
const WORD CGameSprite::FEEDBACK_MODAL = 7;

// 0x85C0C2
const WORD CGameSprite::FEEDBACK_LOCKPICKSUCCEEDED = 8;

// 0x85C0C4
const WORD CGameSprite::FEEDBACK_LOCKPICKFAILED = 9;

// 0x85C0C6
const WORD CGameSprite::FEEDBACK_TRAPDISARMED = 10;

// 0x85C0C8
const WORD CGameSprite::FEEDBACK_TRAPDETECTED = 11;

// 0x85C0CA
const WORD CGameSprite::FEEDBACK_ITEMCURSED = 12;

// 0x85C0CC
const WORD CGameSprite::FEEDBACK_HIDEFAILED = 13;

// 0x85C0CE
const WORD CGameSprite::FEEDBACK_TOOHEAVY_SLOWED = 14;

// 0x85C0D0
const WORD CGameSprite::FEEDBACK_TOOHEAVY_STOPPED = 15;

// 0x85C0D2
const WORD CGameSprite::FEEDBACK_16 = 16;

// 0x85C0D4
const WORD CGameSprite::FEEDBACK_HIDESUCCEEDED = 17;

// 0x85C0D6
const WORD CGameSprite::FEEDBACK_MORALEFAILURE_PANIC = 18;

// 0x85C0D8
const WORD CGameSprite::FEEDBACK_MORALEFAILURE_RUN = 19;

// 0x85C0DA
const WORD CGameSprite::FEEDBACK_MORALEFAILURE_BERSERK = 20;

// 0x85C0DC
const WORD CGameSprite::FEEDBACK_LEVELUP = 21;

// 0x85C0DE
const WORD CGameSprite::FEEDBACK_SPELLFAILED_INDOORS = 22;

// 0x85C0E0
const WORD CGameSprite::FEEDBACK_LEAVING_SHADOWS = 23;

// 0x85C0E2
const WORD CGameSprite::FEEDBACK_BASHDOORSUCCEEDED = 24;

// 0x85C0E4
const WORD CGameSprite::FEEDBACK_BASHDOORFAILED = 25;

// 0x85C0E6
const WORD CGameSprite::FEEDBACK_BASHCHESTSUCCEEDED = 26;

// 0x85C0E8
const WORD CGameSprite::FEEDBACK_BASHCHESTFAILED = 27;

// 0x85C0EA
const WORD CGameSprite::FEEDBACK_BACKSTAB_UNUSABLE_WEAPON = 28;

// 0x85C0EC
const WORD CGameSprite::FEEDBACK_29 = 29;

// 0x85C0EE
const WORD CGameSprite::FEEDBACK_PICKPOCKETFAILED_ARMOR = 30;

// 0x85C0F0
const WORD CGameSprite::FEEDBACK_PICKPOCKETFAILED_HOSTILE = 31;

// 0x85C0F2
const WORD CGameSprite::FEEDBACK_PICKPOCKETFAILED_SKILL = 32;

// 0x85C0F4
const WORD CGameSprite::FEEDBACK_PICKPOCKETFAILED_NOITEMS = 33;

// 0x85C0F6
const WORD CGameSprite::FEEDBACK_PICKPOCKETFAILED_INVENTORYFULL = 34;

// 0x85C0F8
const WORD CGameSprite::FEEDBACK_PICKPOCKETSUCCEEDED = 35;

// 0x85C0FA
const WORD CGameSprite::FEEDBACK_GAINEDSPECIALABILITY = 36;

// 0x85C0FC
const WORD CGameSprite::FEEDBACK_IMMUNE_TO_WEAPON = 37;

// 0x85C0FE
const WORD CGameSprite::FEEDBACK_CHARACTER_CONTROL_SEIZURE = 38;

// 0x85C100
const WORD CGameSprite::FEEDBACK_TRAPDISARMEDFAILED = 39;

// 0x85C102
const WORD CGameSprite::FEEDBACK_CRITICALHITAVOIDED = 40;

// 0x85C104
const WORD CGameSprite::FEEDBACK_AURA_CLEANSED = 41;

// 0x85C106
const WORD CGameSprite::FEEDBACK_ITEM_DROPPED_FULL = 48;

// 0x85C108
const WORD CGameSprite::FEEDBACK_LOCK_UNPICKABLE = 49;

// 0x85C10A
const WORD CGameSprite::FEEDBACK_TOOMANYSUMMONED = 50;

// 0x85C10C
const WORD CGameSprite::FEEDBACK_SILENCED = 51;

// 0x85C10E
const WORD CGameSprite::FEEDBACK_CRIPPLINGSTRIKESUCCEEDED = 52;

// 0x85C110
const WORD CGameSprite::FEEDBACK_SNEAK_UNUSABLE_WEAPON = 53;

// 0x85C112
const WORD CGameSprite::FEEDBACK_EFFECTSEVADED = 54;

// 0x85C114
const WORD CGameSprite::FEEDBACK_SPELLFAILED_DISRUPTED = 55;

// 0x85C116
const WORD CGameSprite::FEEDBACK_GAINEDBARDSONG = 56;

// 0x85C118
const WORD CGameSprite::FEEDBACK_STATICCHARGEEND = 57;

// 0x85C11A
const WORD CGameSprite::FEEDBACK_IMMUNE_TO_RESOURCE = 58;

// 0x85C11C
const WORD CGameSprite::FEEDBACK_LEAVEFAILED_INVENTORY = 59;

// 0x85C11E
const WORD CGameSprite::FEEDBACK_BACKSTAB_AVOIDED = 60;

// 0x85C120
const WORD CGameSprite::FEEDBACK_SPELLFAILED_CASTFAILURE = 61;

// 0x85C122
const WORD CGameSprite::FEEDBACK_62 = 62;

// 0x85C124
const WORD CGameSprite::FEEDBACK_63 = 63;

// 0x85C126
const WORD CGameSprite::FEEDBACK_CRIPPLINGSTRIKEFAILED = 64;

// 0x85C128
const WORD CGameSprite::FEEDBACK_USEMAGICDEVICEFAILED = 65;

// 0x85C12A
const WORD CGameSprite::FEEDBACK_USEMAGICDEVICESUCCEEDED = 66;

// 0x85C12C
const WORD CGameSprite::FEEDBACK_SEARCHSTART = 67;

// 0x85C12E
const WORD CGameSprite::FEEDBACK_SEARCHEND = 68;

// 0x85C130
const WORD CGameSprite::FEEDBACK_69 = 69;

// 0x85C132
const WORD CGameSprite::FEEDBACK_TURNUNDEADEND = 70;

// 0x85C134
const WORD CGameSprite::FEEDBACK_71 = 71;

// 0x85C136
const WORD CGameSprite::FEEDBACK_BATTLESONGSTART = 72;

// 0x85C138
const WORD CGameSprite::FEEDBACK_BATTLESONGEND = 73;

// 0x85C13A
const WORD CGameSprite::FEEDBACK_ARTERIALSTRIKESUCCEEDED = 75;

// 0x85C13C
const WORD CGameSprite::FEEDBACK_HAMSTRINGSUCCEEDED = 76;

// 0x85C13E
const WORD CGameSprite::FEEDBACK_HEROICINSPIRATION = 80;

// 0x85C140
const WORD CGameSprite::FEEDBACK_TARGETCONCEALED = 81;

// 0x85C142
const WORD CGameSprite::FEEDBACK_UNCANNYDODGESUCCEEDED = 82;

// 0x85C144
const WORD CGameSprite::FEEDBACK_ROLL = 83;

// 0x85C146
const WORD CGameSprite::FEEDBACK_SPELLFAILURE_CONCENTRATION = 84;

// 0x85C148
const WORD CGameSprite::FEEDBACK_SPONTANEOUSCASTING = 85;

// 0x85C14A
const WORD CGameSprite::FEEDBACK_TOGGLEFEAT = 86;

// 0x85C14C
const WORD CGameSprite::FEEDBACK_CLEAVE = 87;

// 0x85C14E
const WORD CGameSprite::FEEDBACK_LINGERINGSONG = 88;

// 0x85C150
const WORD CGameSprite::FEEDBACK_89 = 89;

// 0x85C152
const WORD CGameSprite::FEEDBACK_CALLLIGHTNINGEND = 90;

// 0x85C154
const WORD CGameSprite::FEEDBACK_MAGICALWEAPONINUSE = 91;

// 0x85C156
const WORD CGameSprite::FEEDBACK_LEAVEFAILED_LEVELUP = 92;

// 0x85C158
const WORD CGameSprite::FEEDBACK_LEAVEFAILED_STORE = 93;

// 0x8F94B8
const CPoint CGameSprite::PORTRAIT_ICON_SIZE(10, 10);

// 0x8F93F8
const CResRef CGameSprite::SPIN110("SPIN110");

// 0x8F92A0
const CResRef CGameSprite::SPIN111("SPIN111");

// 0x8F9328
const CResRef CGameSprite::SPIN107("SPIN107");

// 0x8F93E8
const CResRef CGameSprite::SPIN141("SPIN141");

// 0x8F92D0
const CResRef CGameSprite::SPIN142("SPIN142");

// 0x8F9468
const CResRef CGameSprite::SPIN143("SPIN143");

// 0x8F92C8
const CResRef CGameSprite::SPIN244("SPIN244");

// 0x8F94A0
const CResRef CGameSprite::SPIN245("SPIN245");

// 0x8F9488
const CResRef CGameSprite::SPIN246("SPIN246");

// 0x8F8E60
const CResRef CGameSprite::SPIN108("SPIN108");

// 0x8F92E0
const CResRef CGameSprite::SPIN109("SPIN109");

// 0x8F92B8
const CResRef CGameSprite::SPIN120("SPIN120");

// 0x8F9298
const CResRef CGameSprite::SPIN121("SPIN121");

// 0x8F9320
const CResRef CGameSprite::SPIN125("SPIN125");

// 0x8F9330
const CResRef CGameSprite::SPIN139("SPIN139");

// 0x8F9478
const CResRef CGameSprite::SPIN152("SPIN152");

// 0x8F9338
const CResRef CGameSprite::SPIN275("SPIN275");

// 0x8F8E70
const CResRef CGameSprite::SPIN276("SPIN276");

// 0x8F92F8
const CResRef CGameSprite::SPIN277("SPIN277");

// 0x8F92E8
const CResRef CGameSprite::SPIN278("SPIN278");

// 0x8F93D0
const CResRef CGameSprite::SPIN279("SPIN279");

// 0x8F8E18
const CResRef CGameSprite::SPIN970("SPIN970");

// 0x8F94B0
const CResRef CGameSprite::SPIN197("SPIN197");

// 0x8F8E90
const CResRef CGameSprite::SPPR327("SPPR327");

// 0x8F94D0
const CResRef CGameSprite::SPPR328("SPPR328");

// 0x8F92A8
const CResRef CGameSprite::SPPR429("SPPR429");

// 0x8F9460
const CResRef CGameSprite::SPIN198("SPIN198");

// 0x8F9448
const CResRef CGameSprite::SPIN199("SPIN199");

// 0x8F9300
const CResRef CGameSprite::SPIN200("SPIN200");

// 0x8F8E30
const CResRef CGameSprite::SPIN243("SPIN243");

// 0x8F9438
const CResRef CGameSprite::SPIN201("SPIN201");

// 0x8F9308
const CResRef CGameSprite::SPIN202("SPIN202");

// 0x8F8E28
const CResRef CGameSprite::SPIN203("SPIN203");

// 0x8F9418
const CResRef CGameSprite::SPIN247("SPIN247");

// 0x8F9348
const CResRef CGameSprite::SPIN204("SPIN204");

// 0x8F9490
const CResRef CGameSprite::SPIN205("SPIN205");

// 0x8F92B0
const CResRef CGameSprite::SPIN206("SPIN206");

// 0x8F9458
const CResRef CGameSprite::SPIN207("SPIN207");

// 0x8F9470
const CResRef CGameSprite::SPIN208("SPIN208");

// 0x8F9498
const CResRef CGameSprite::SPIN209("SPIN209");

// 0x8F8E88
const CResRef CGameSprite::SPIN210("SPIN210");

// 0x8F94C0
const CResRef CGameSprite::SPIN231("SPIN231");

// 0x8F94A8
const CResRef CGameSprite::SPIN191("SPIN191");

// 0x8F8E48
const CResRef CGameSprite::SPIN211("SPIN211");

// 0x8F8E78
const CResRef CGameSprite::SPIN212("SPIN212");

// 0x8F8E40
const CResRef CGameSprite::SPIN213("SPIN213");

// 0x8F9428
const CResRef CGameSprite::SPIN214("SPIN214");

// 0x8F8E58
const CResRef CGameSprite::SPIN215("SPIN215");

// 0x8F94E8
const CResRef CGameSprite::SPIN216("SPIN216");

// 0x8F92C0
const CResRef CGameSprite::SPIN217("SPIN217");

// 0x8F9400
const CResRef CGameSprite::SPIN218("SPIN218");

// 0x8F9310
const CResRef CGameSprite::SPIN229("SPIN229");

// 0x8F94D8
const CResRef CGameSprite::SPIN232("SPIN232");

// 0x8F93E0
const CResRef CGameSprite::SPIN233("SPIN233");

// 0x8F8E68
const CResRef CGameSprite::SPIN234("SPIN234");

// 0x8F93D8
const CResRef CGameSprite::SPIN235("SPIN235");

// 0x8F9410
const CResRef CGameSprite::SPIN236("SPIN236");

// 0x8F8E20
const CResRef CGameSprite::SPIN260("SPIN260");

// 0x8F9340
const CString CGameSprite::FA("FA_");

// 0x8F9430
const CString CGameSprite::GA("GA_");

// 0x8F94E0
const CString CGameSprite::RA("RA_");

// 0x8F94C8
const CString CGameSprite::AP("AP_");

// 0x8F9480
const CResRef CGameSprite::SPWI110("SPWI110");

// 0x8F9408
const CResRef CGameSprite::SPIN122("SPIN122");

// 0x8F93F0
const CResRef CGameSprite::SPIN285("SPIN285");

// 0x8F9924
const CString CGameSprite::TOKEN_SPECIALABILITY("SPECIALABILITYNAME");

// 0x8F9350
BYTE CGameSprite::STATICBUFFER[STATICBUFFERSIZE_CGAMESPRITE];

// 0x8F950C
BOOLEAN CGameSprite::SHOW_CHARACTER_HP;

// 0x8F950D
BOOLEAN CGameSprite::GRAVITY_IS_DOWN;

// 0x8B85E0
INT CGameSprite::m_bRollFeedbackEnabled = -1;

// 0x8F9A18
CAIAction CGameSprite::m_aiDoAction;

// 0x8F9AF4: LeaveArea()'s "already played the no-sound feedback this call"
// latch, shared by its three "party not ready" message sites.
static BOOLEAN s_bLeaveAreaMessageShown;

// 0x6EF990
CGameSprite::CGameSprite(BYTE* pCreature, LONG creatureSize, int a3, WORD type, DWORD expirationTime, WORD huntingRange, WORD followRange, DWORD timeOfDayVisible, CPoint startPos, WORD facing)
    : m_portraitIconVidCell(CResRef("STATES"), g_pBaldurChitin->m_bUseNewGui)
    , m_spriteSplashPalette(CVidPalette::TYPE_RANGE)
{
    int index;

    field_7540 = 0;
    field_7544 = 0;
    m_spriteEffectSequenceLength = 0;
    m_spriteEffectBaseIntensity = 0;
    m_spriteEffectRandomIntensity = 0;
    m_bEscapingArea = 0;
    m_nSndWalk = 0;
    m_currSndWalk = 0;
    m_effectExtendDirection = 0;
    m_animationRunning = FALSE;
    m_posZDelta = 0;
    m_circleFacing = 0;
    m_radius = 0;
    m_fDirectionOffset = 0.0;
    m_skipDeltaDirection = 0;
    m_deltaDirection = 0;
    m_walkBackwards = FALSE;
    m_turningAbout = FALSE;
    m_bInCasting = FALSE;
    m_moveCount = 0;
    m_curDest.x = 0;
    m_curDest.y = 0;
    m_posLastVisMapEntry.x = 0;
    m_posLastVisMapEntry.y = 0;
    m_nCommandPause = 0;
    field_557A = 0;
    field_557E = 0;
    field_558E = 0;
    field_559E = 0;
    field_55A0 = 0;
    m_speedFactor = 0;
    m_lastActionID = 0;
    field_560E = 0;
    field_5610 = 0;
    field_5618 = 0;
    field_561C = 0;
    m_noActionCount = 0;
    field_5630 = 0;
    field_5632 = 0;
    field_5636 = 0;
    field_7118 = 0;
    field_711C = 0;
    m_firstActionSound = FALSE;
    m_bForceVisualEffects = FALSE;
    m_currentActionId = 0;
    field_72A8 = 0;
    m_bInUnmarshal = FALSE;
    field_72D6 = 0;
    field_72DE = 0;
    field_7532 = 0;
    field_7536 = 0;
    field_9D08 = 0;
    field_9D0C = 0;
    field_9D10 = 0;
    m_objectType = TYPE_SPRITE;
    m_resRef = "";
    m_fCircleChange = 0.0;
    m_fCurrCircleChange = 1.0;
    m_bGlobal = FALSE;
    m_posExact.x = 0;
    m_posExact.y = 0;
    m_posDelta.x = 0;
    m_posDelta.y = 0;
    m_posDest.x = 0;
    m_posDest.y = 0;
    m_posOld.x = 0;
    m_posOld.y = 0;
    field_536A = 0;
    field_536E = 0;
    m_nSequence = -1;
    m_nNewDirection = facing & 0xF;
    m_nDirection = facing & 0xF;
    m_pPath = 0;
    m_nPath = 0;
    m_currPath = 0;
    m_pathSearchInvalidDest = FALSE;
    m_selectedSound = 17;
    field_53D2 = 0;
    m_currentSearchRequest = 0;
    m_lastCharacterCount = 0;
    m_sequenceTest = FALSE;
    m_removeFromArea = FALSE;
    m_talkingCounter = 0;
    m_moveToFrontQueue = 0;
    m_moveToBackQueue = 0;
    m_curResponseNum = -1;
    m_curResponseSetNum = -1;
    m_curScriptNum = -1;
    m_curAction = CAIAction::NULL_ACTION;
    m_interrupt = FALSE;
    m_endOfDamageSeq = FALSE;
    m_inFormation = FALSE;
    m_lastRGBColor = 0x8000;
    m_bVisibilityUpdated = TRUE;
    m_targetId = CGameObjectArray::INVALID_INDEX;
    m_targetPoint.x = -1;
    m_targetPoint.y = -1;
    m_targetAreaSize = 0;
    m_destMarker.SetType(CMarker::RECTICLE);
    field_72E2 = CGameObjectArray::INVALID_INDEX;
    m_typeAI.Set(CAIObjectType::ANYONE);
    m_liveTypeAI.Set(CAIObjectType::ANYONE);
    m_startTypeAI.Set(CAIObjectType::ANYONE);
    m_overrideScript = 0;
    m_special1Script = 0;
    m_teamScript = 0;
    m_special2Script = 0;
    m_combatScript = 0;
    m_special3Script = 0;
    m_movementScript = NULL;
    m_followLeader = FALSE;
    m_followLeaderAdditive = FALSE;
    m_followLeaderNext = CGameObjectArray::INVALID_INDEX;
    m_followStart = 0;
    m_userCommandPause = 0;
    m_recoilFrame = 0;
    field_561E = 0;
    field_44A = 0;
    field_54C = 0;
    field_560C = 0;
    field_5616 = 0;
    field_5612 = 0;
    field_5614 = 0;
    m_castCounter = -1;
    m_bStartedCasting = FALSE;
    m_attackFrame = -2;
    field_72A2 = 0;
    m_bAllowEffectListCall = TRUE;
    field_72AA = 0;
    m_bForceVisualEffects = FALSE;
    m_nDamageTaken = 0;
    field_9D14 = 0;
    field_9D15 = 0;
    m_hasColorEffects = FALSE;
    m_hasColorRangeEffects = FALSE;
    field_5640 = 0;
    m_groupPosition = 0;
    m_groupMove = 0;
    field_562C = 0;
    m_curProjectile = 0;
    m_curSpell = 0;
    m_curItem = 0;
    m_nNumberOfTimesTalkedTo = 0;
    m_bSeenPartyBefore = 0;
    field_710A = 0;
    field_710C = 0;
    field_710E = 0;
    field_7110 = 0x7FFF;
    m_moraleFailure = FALSE;
    m_nHappiness = 0;
    m_bHappinessChanged = FALSE;
    m_bDeleteOnRemove = TRUE;
    m_lastRegenerationTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;

    CButtonData buttonData;

    for (index = 0; index < 8; index++) {
        m_quickWeapons[index] = buttonData;
    }

    for (index = 0; index < 9; index++) {
        m_quickSpells[index] = buttonData;
    }

    for (index = 0; index < 3; index++) {
        m_quickItems[index] = buttonData;
    }

    for (index = 0; index < 9; index++) {
        m_quickInnates[index] = buttonData;
    }

    for (index = 0; index < 9; index++) {
        m_quickSongs[index] = buttonData;
    }

    for (index = 0; index < 9; index++) {
        field_3D14[index] = 0;
    }

    field_70F6 = rand() % 20 + 1;
    field_70F7 = rand() % 20 + 1;
    field_70F8 = rand() % 20 + 1;
    field_70F9 = rand() % 20 + 1;
    field_70FA = rand() % 100;
    field_55F2 = 0;
    field_55F6 = 0;
    field_55FA = 0;
    field_55FE = 0;
    field_5602 = 0;

    for (index = 0; index < 20; index++) {
        field_55A2[index] = -65538;
    }

    memset(m_nNumberOfTimesInteractedWith, 0, sizeof(m_nNumberOfTimesInteractedWith));

    field_54B8 = 0;
    m_bBumpable = 0;
    m_bBumped = FALSE;
    m_ptBumpedFrom.x = -1;
    m_ptBumpedFrom.y = -1;
    m_bVisibleMonster = FALSE;
    m_bSelected = FALSE;
    m_bInfravisionOn = FALSE;
    m_bPortraitUpdate = FALSE;
    m_nBloodFlashAmount = 0;
    m_nDamageLocatorTime = 0;
    m_nDamageLocatorColor = 0x1E1EFF;
    m_bBloodFlashOn = 0;
    field_53E6 = 0;
    m_nTwitches = 0;

    // NOTE: See `CGameArea` note on default terrain tables.
    memcpy(m_terrainTable, DEFAULT_TERRAIN_TABLE, 16);
    memcpy(m_visibleTerrainTable, DEFAULT_VISIBLE_TERRAIN_TABLE, 16);
    memcpy(m_flightTerrainTable, DEFAULT_VISIBLE_TERRAIN_TABLE, 16);

    if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Add(&m_id, this, INFINITE) == CGameObjectArray::SUCCESS) {
        m_pSpriteEffectArray = NULL;
        m_pSpriteEffectArrayPosition = NULL;
        m_spriteEffectDuration = 0;
        m_spriteEffectFlags = 0;
        field_5304 = 0;
        m_doBounce = 3;
        m_nModalState = 0;
        m_nFeatRanks[0] = 0;
        m_nFeatRanks[1] = 0;
        m_nFeatRanks[2] = 0;
        m_nFeatRanks[3] = 0;
        m_nFeatRanks[4] = 0;
        m_nWeaponSet = 0;

        m_aVfxCells[IWD_VFX_SANCTUARY].SetResRef(CResRef("SanctuC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_ENTANGLE].SetResRef(CResRef("EntangC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_MINOR_GLOBE_OF_INVULNERABILITY].SetResRef(CResRef("MGoInvC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_SHIELD].SetResRef(CResRef("ShieldC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_GREASE].SetResRef(CResRef("GreaseC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_WEB].SetResRef(CResRef("WebC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_GLOBE_OF_INVULNERABILITY].SetResRef(CResRef("GoInvuC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_SHROUD_OF_FLAME].SetResRef(CResRef("SoFlamC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_ANTIMAGIC_MISSILE].SetResRef(CResRef("AMShelC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_OTILUKES_RESILIENT_SPHERE].SetResRef(CResRef("ORSpheC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_PROTECTION_FROM_MISSILES].SetResRef(CResRef("PFNMisC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_CLOAK_OF_FEAR].SetResRef(CResRef("CoFearC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_ENTROPY_SHIELD].SetResRef(CResRef("EShielC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_FIRE_AURA].SetResRef(CResRef("FiAuraC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_FROST_AURA].SetResRef(CResRef("FrAuraC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_INSECT_PLAGUE].SetResRef(CResRef("IPlaguC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_STORM_SHELL].SetResRef(CResRef("SShellC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_SHIELD_OF_LATHANDER].SetResRef(CResRef("SoLatC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_SHIELD_OF_LATHANDER_2].SetResRef(CResRef("SoLatC2"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_GREATER_SHIELD_OF_LATHANDER].SetResRef(CResRef("GSoLaC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_GREATER_SHIELD_OF_LATHANDER_2].SetResRef(CResRef("GSoLaC2"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_SEVEN_EYES].SetResRef(CResRef("SEyesC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_SEVEN_EYES_2].SetResRef(CResRef("SEyesC2"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_FIRE_SHIELD_RED].SetResRef(CResRef("FShiRC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_28].SetResRef(CResRef("FShiRC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_FIRE_SHIELD_BLUE].SetResRef(CResRef("FShiBC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_29].SetResRef(CResRef("FShiBC1"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_DEATH_ARMOR].SetResRef(CResRef("DArmorC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_TORTOISE_SHELL].SetResRef(CResRef("TShellC"), FALSE, TRUE, TRUE);
        m_aVfxCells[IWD_VFX_WISP].SetResRef(CResRef("Wisp"), FALSE, TRUE, TRUE);

        m_aVfxSounds[IWD_VFX_GLOBE_OF_INVULNERABILITY].SetResRef(CResRef("AFT_M05"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_SHROUD_OF_FLAME].SetResRef(CResRef("MISC_01C"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_ANTIMAGIC_MISSILE].SetResRef(CResRef("AFT_M04"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_OTILUKES_RESILIENT_SPHERE].SetResRef(CResRef("AFT_M03"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_PROTECTION_FROM_MISSILES].SetResRef(CResRef("AFT_M01"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_CLOAK_OF_FEAR].SetResRef(CResRef("AFT_P02"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_ENTROPY_SHIELD].SetResRef(CResRef("AFT_P03"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_STORM_SHELL].SetResRef(CResRef("AFT_P25"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_SHIELD_OF_LATHANDER].SetResRef(CResRef("AFT_P20"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_GREATER_SHIELD_OF_LATHANDER].SetResRef(CResRef("AFT_P26"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_SANCTUARY].SetResRef(CResRef("AFT_P01"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_MINOR_GLOBE_OF_INVULNERABILITY].SetResRef(CResRef("AFT_M02"), TRUE, TRUE);
        m_aVfxSounds[IWD_VFX_SHIELD].SetResRef(CResRef("EFF_M11C"), TRUE, TRUE);

        m_type = type;
        m_expirationTime = expirationTime;
        m_huntingRange = huntingRange;
        m_followRange = followRange;
        m_posStart = startPos;
        m_timeOfDayVisible = timeOfDayVisible;
        m_active = ((timeOfDayVisible >> g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->GetCurrentHour()) & 0x1) != 0;
        m_activeAI = TRUE;
        m_activeImprisonment = TRUE;

        memset(&m_baseStats, 0, sizeof(m_baseStats));

        m_pDialogData = NULL;
        m_nDialogData = 0;
        m_dialogWait = 0;
        m_dialogWaitTarget = CGameObjectArray::INVALID_INDEX;
        m_bAllowDialogInterrupt = TRUE;
        field_56E4 = "";
        m_talkingRenderCount = 0;
        m_clearAIOnRemoveFromArea = TRUE;

        if (m_portraitIconVidCell.pRes != NULL) {
            m_portraitIconVidCell.pRes->Request();
        }

        field_722A = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
        m_interactingWith.Set(CAIObjectType::NOONE);
        m_nHPCONBonusTotalOld = 0;
        m_bHPCONBonusTotalUpdate = TRUE;
        m_modalCounter = 0;
        m_bHiding = FALSE;
        m_secondarySounds = "";

        memset(field_725A, 0, sizeof(field_725A));

        field_727E = 0;
        m_pLocalVariables = new CVariableHash(16);
        m_internalButtonList = NULL;

        SleepEx(10, FALSE);
        Unmarshal(pCreature, creatureSize, facing, a3);
        SleepEx(10, FALSE);
        UnmarshalScripts();
        SleepEx(10, FALSE);

        // NOTE: Inlining as `STR_RES` object is immediately destroyed.
        if (1) {
            STR_RES strRes;
            g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_name, strRes);
            m_sName = strRes.szText;
        }

        // NOTE: Uninline.
        FetchCommonStrings();

        for (index = 0; index < 2; index++) {
            m_sndWalk[index].m_nVolumeVariance = 50;
        }

        m_currSndArmor = 0;
        m_sndArmor[m_currSndArmor].m_nVolumeVariance = 50;
        m_animationRunning = (m_baseStats.m_animationType & 0xF000) != 0;
        m_triggerId = CGameObjectArray::INVALID_INDEX;
        field_562C = TRUE;
        m_firstCall = FALSE;
        m_berserkActive = FALSE;
        m_attackSoundDeadzone = 0;
        m_nTempSelectedWeapon = m_equipment.m_selectedWeapon;
        m_nTempSelectedWeaponAbility = static_cast<BYTE>(m_equipment.m_selectedWeaponAbility);

        SetSequence(SEQ_HEAD_TURN);
        CheckLoadState();

        m_bLevelUp = FALSE;
        m_nUnselectableCounter = 0;

        // NOTE: Uninline.
        m_inControlLastTime = InControl();

        m_nStealthGreyOut = 0;
        m_baseStats.m_flags &= ~0x80000000;

        INT bonusNew = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetHPCONBonusTotal(m_typeAI,
            m_derivedStats,
            static_cast<BYTE>(m_derivedStats.m_nCON));
        if (bonusNew > m_nHPCONBonusTotalOld) {
            if ((m_derivedStats.m_generalState & (STATE_DEAD | STATE_FROZEN_DEATH | STATE_STONE_DEATH)) == 0) {
                m_baseStats.m_hitPoints += static_cast<SHORT>(bonusNew) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
            } else {
                m_baseStats.m_hitPoints = 0;
            }
            m_derivedStats.m_nMaxHitPoints += static_cast<SHORT>(bonusNew) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
            m_baseStats.m_maxHitPointsBase += static_cast<SHORT>(bonusNew) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
        } else if (bonusNew < m_nHPCONBonusTotalOld) {
            // NOTE: The code in this branch looks exactly the same as above. It
            // might be different in earlier games.
            if ((m_derivedStats.m_generalState & (STATE_DEAD | STATE_FROZEN_DEATH | STATE_STONE_DEATH)) == 0) {
                m_baseStats.m_hitPoints += static_cast<SHORT>(bonusNew) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
            } else {
                m_baseStats.m_hitPoints = 0;
            }
            m_derivedStats.m_nMaxHitPoints += static_cast<SHORT>(bonusNew) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
            m_baseStats.m_maxHitPointsBase += static_cast<SHORT>(bonusNew) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
        }
        m_nHPCONBonusTotalOld = bonusNew;
        m_bHPCONBonusTotalUpdate = FALSE;
        m_derivedStats.m_nMaxHitPoints = max(m_derivedStats.m_nMaxHitPoints, m_derivedStats.m_nLevel);

        if (m_baseStats.m_hitPoints <= 0
            && (m_derivedStats.m_generalState & STATE_DEAD) == 0
            && (m_derivedStats.m_generalState & (STATE_STONE_DEATH | STATE_FROZEN_DEATH)) == 0
            && (m_baseStats.m_generalState & STATE_DEAD) != 0) {
            m_derivedStats.m_generalState |= STATE_DEAD;
        }

        field_7282 = -1;
        field_7283 = -1;
        field_7284 = -1;
        field_728E = -1;
        m_bPlayedEncumberedStopped = 0;
        m_bPlayedEncumberedSlowed = 0;
        m_poisonSoundTimer = 0;
        m_nBounceCounter = 0;
        field_7106 = 0;

        LoadSoundEntries();

        field_70EE = 0;
        m_bOnSearchMap = 0;
        m_bInvisible = 0;
        if (IcewindMisc::IsLarge(this) == TRUE) {
            m_aVfxCells[IWD_VFX_OTILUKES_RESILIENT_SPHERE].SetResRef(CResRef("ORSpheB"), FALSE, TRUE, TRUE);
        }

        field_753A = 0;
        field_753C = 0;
    } else {
        // FIXME: Does not look cool.
        delete this;
    }
}

// 0x6F2990
BOOL CGameSprite::DoesIntersect(CRect r)
{
    r.InflateRect(0, 0, 1, 1);

    CRect rEllipse = m_animation.GetEllipseRect();
    rEllipse.OffsetRect(m_pos);

    CRect rIntersection;
    rIntersection.IntersectRect(r, rEllipse);

    return !rIntersection.IsRectNull();
}

// 0x6F2A90
BOOL CGameSprite::IsOver(const CPoint& pt)
{
    if (m_active
        && m_activeAI
        && m_activeImprisonment
        && Animate()
        && m_baseStats.m_bStealthMode == 0
        && (m_typeAI.GetEnemyAlly() <= CAIObjectType::EA_GOODCUTOFF
            || (m_derivedStats.m_generalState & STATE_INVISIBLE) == 0
            || (m_baseStats.m_critSectService & 0x1) != 0)) {
        const CRect& rEllipse = m_animation.GetEllipseRect();
        CPoint ptRelative(pt.x - m_pos.x, pt.y - m_pos.y);
        return rEllipse.PtInRect(ptRelative);
    } else {
        return FALSE;
    }
}

// 0x6F2B50
const BYTE* CGameSprite::GetVisibleTerrainTable()
{
    return m_visibleTerrainTable;
}

// 0x6F2B60
BOOL CGameSprite::OnSearchMap()
{
    return ((m_listType == LIST_FRONT || (m_derivedStats.m_generalState & STATE_SLEEPING) != 0))
        && m_pArea != NULL
        && m_pos.x >= 0
        && m_pos.y >= 0;
}

// 0x6F2B90
void CGameSprite::SetAIType(const CAIObjectType& typeAI, BOOL updateLive, BOOL updateStart)
{
    m_typeAI.Set(typeAI);

    if (updateLive) {
        m_liveTypeAI.Set(typeAI);
    }

    if (updateStart) {
        m_startTypeAI.Set(typeAI);
    }
}

// 0x6F2BD0
void CGameSprite::ResetAITypeLive()
{
    m_typeAI.m_nGeneral = m_liveTypeAI.m_nGeneral;
    m_startTypeAI.m_nGeneral = m_liveTypeAI.m_nGeneral;
}

// 0x6F2BF0
void CGameSprite::ResetAIType()
{
    m_typeAI.Set(m_startTypeAI);

    BYTE prev = m_liveTypeAI.m_nGeneral;
    m_liveTypeAI.Set(m_startTypeAI);
    m_liveTypeAI.m_nGeneral = prev;
}

// 0x6F2C50
LONG CGameSprite::GetTargetId()
{
    return m_targetId;
}

// 0x6F2C60
const BYTE* CGameSprite::GetTerrainTable()
{
    return m_terrainTable;
}

// 0x6F2C70
BOOL CGameSprite::GetCanSeeInvisible()
{
    return m_curAction.m_actionID == CAIAction::DIALOGUE
        || m_curAction.m_actionID == CAIAction::STARTDIALOG
        || m_curAction.m_actionID == CAIAction::STARTDIALOGUENOSET
        || m_derivedStats.m_bSeeInvisible
        || (m_baseStats.m_flags & 0x10000) != 0;
}

// 0x6F40F0
void CGameSprite::AddToArea(CGameArea* pNewArea, const CPoint& pos, LONG posZ, BYTE listType)
{
    BOOLEAN v1 = FALSE;

    CGameObject::AddToArea(pNewArea, pos, posZ, listType);
    m_posExact.x = m_pos.x << 10;
    m_posExact.y = (m_pos.y << 12) / 3;
    m_posOld = m_pos;
    field_562C = 1;

    if (m_active && m_activeAI && m_activeImprisonment) {
        CheckIfVisible();
    }

    switch (listType) {
    case CGAMEOBJECT_LIST_FRONT:
        if (m_active
            && m_activeAI
            && m_activeImprisonment
            && (m_derivedStats.m_generalState & STATE_DEAD) == 0) {
            // NOTE: Uninline.
            pNewArea->AddToMarkers(m_id);

            m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                            m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
        }
        break;
    case CGAMEOBJECT_LIST_BACK:
        if (m_active && m_activeAI && m_activeImprisonment) {
            pNewArea->IncrHeightDynamic(m_pos);
        }
        break;
    case CGAMEOBJECT_LIST_FLIGHT:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 1528
        UTIL_ASSERT(FALSE);
    }

    SetFootstepChannel();

    if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0) {
        if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bFootStepsSounds
            || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1) {
            char* pSndReady = m_animation.GetSndReady();
            if (*pSndReady != '\0') {
                m_sndReady.SetResRef(CResRef(pSndReady), TRUE, TRUE);
                delete pSndReady;
                m_sndReady.SetLoopingFlag(1);
                m_sndReady.Play(m_pos.x, m_pos.y, m_posZ, FALSE);
            }
        }
    }

    m_currentArea = pNewArea->m_resRef;

    if (m_bGlobal) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 1548
        UTIL_ASSERT(pGame != NULL);

        if (pGame->GetCharacterPortraitNum(m_id) != -1
            && Animate()) {
            m_posLastVisMapEntry = pos;
            pNewArea->m_visibility.AddCharacter(m_posLastVisMapEntry,
                m_id,
                m_visibleTerrainTable);
            if (pNewArea->m_nCharacters != -1) {
                pNewArea->m_nCharacters++;
            } else {
                pNewArea->m_nCharacters = 1;
            }
        }
    } else if (m_type == 1) {
        pNewArea->m_nRandomMonster++;
    }

    if (!g_pBaldurChitin->GetObjectGame()->m_bInLoadGame
        && !g_pBaldurChitin->GetObjectGame()->m_bInLoadArea
        && !g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition
        && !g_pBaldurChitin->GetBaldurMessage()->m_bInOnObjectAdd) {
        BYTE* pData = STATICBUFFER;
        DWORD dwSize;
        MarshalMessage(&pData, &dwSize);
        g_pBaldurChitin->GetBaldurMessage()->ObjectAdd(m_id, GetObjectType(), pData, dwSize);

        if (InControl()
            && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            m_cLastSpriteUpdate.Initialize(TRUE);
            CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        v1 = TRUE;
    }

    if (m_pArea == NULL) {
        // FIXME: Unused.
        CString sText("Sprite ");
        if (m_sName.GetLength() != 0) {
            sText += m_sName;
        } else {
            sText += "Unknown";
        }
        sText += " has a NULL m_pArea member in AddToArea() function!";
    }

    if (pNewArea != NULL) {
        if (g_pBaldurChitin->GetObjectGame()->m_bInLoadGame
            && pNewArea->mpSpawner->m_bInSpawn == TRUE
            && !v1) {
            BYTE* pData = STATICBUFFER;
            DWORD dwSize;
            MarshalMessage(&pData, &dwSize);
            g_pBaldurChitin->GetBaldurMessage()->ObjectAdd(m_id, GetObjectType(), pData, dwSize);
        }
    }

    if (m_baseStats.m_bStealthMode) {
        m_baseStats.m_bStealthMode = FALSE;
        SetStealthState(1);
        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            CMessage* message = new CMessage90(m_id, m_id, 1);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    if (m_baseStats.m_bRemoveFromArea) {
        m_removeFromArea = TRUE;
    }

    if (!m_baseStats.field_2E2) {
        SavePositionToBaseStats();
        m_baseStats.field_2E2 = TRUE;
    }

    Icewind586B70::Instance()->RestoreSummonLinks(this);
}

// 0x6F47F0
void CGameSprite::RemoveReplacementFromArea()
{
    if (m_pArea != NULL) {
        m_sndReady.Stop();
        DropSearchRequest();
        if (m_bInfravisionOn) {
            m_bInfravisionOn = FALSE;
            m_pArea->m_nInfravision--;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1) {
            if (!g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode || InControl()) {
                if (Animate()) {
                    m_pArea->m_visibility.RemoveCharacter(m_pos,
                        m_id,
                        m_visibleTerrainTable);
                }

                if (m_pArea->m_nCharacters != -1) {
                    m_pArea->m_nCharacters--;
                }
            }
        }

        ClearStoredPaths();

        if (m_pPath != NULL) {
            delete m_pPath;
            m_pPath = NULL;
        }

        if (m_clearAIOnRemoveFromArea) {
            ClearAI(TRUE);
        }

        m_nNewDirection = m_nDirection;
        g_pBaldurChitin->GetObjectGame()->RemoveCharacterFromAllies(m_id);

        switch (m_listType) {
        case CGAMEOBJECT_LIST_FRONT:
            if (m_active
                && m_activeAI
                && m_activeImprisonment) {
                if ((m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    // NOTE: Uninline.
                    m_pArea->RemoveFromMarkers(m_id);
                }

                m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                   m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
            }
            break;
        case CGAMEOBJECT_LIST_BACK:
            if (m_active
                && m_activeAI
                && m_activeImprisonment) {
                if ((m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
                    // NOTE: Uninline.
                    m_pArea->RemoveFromMarkers(m_id);

                    m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                       m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                        m_typeAI.GetEnemyAlly(),
                        m_animation.GetPersonalSpace(),
                        m_bBumpable,
                        m_bOnSearchMap);
                } else {
                    m_pArea->DecrHeightDynamic(m_pos);
                }
            }
            break;
        case CGAMEOBJECT_LIST_FLIGHT:
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 1773
            UTIL_ASSERT(FALSE);
        }

        if (m_bGlobal) {
            m_pArea = NULL;
            m_pos.x = -1;
            m_pos.y = -1;
            m_posZ = 0;
        } else {
            if (m_type == 1) {
                m_pArea->m_nRandomMonster--;
            }

            CGameObject::RemoveFromArea();

            BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                CGameObjectArray::THREAD_ASYNCH,
                NULL,
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                // __LINE__: 1783
                UTIL_ASSERT(FALSE);
            }

            delete this;
        }
    }
}

// 0x6F4B90
void CGameSprite::RemoveFromArea()
{
    if (m_pArea != NULL) {
        POSITION pos = m_lstBlood.GetHeadPosition();
        while (pos != NULL) {
            CBlood* pBlood = m_lstBlood.GetNext(pos);
            delete pBlood;
        }
        m_lstBlood.RemoveAll();

        m_sndReady.Stop();
        DropSearchRequest();

        if (m_bBumped) {
            m_ptBumpedFrom.x = -1;
            m_ptBumpedFrom.y = -1;
            m_bBumped = FALSE;
        }

        if (m_bVisibleMonster) {
            m_pArea->m_nVisibleMonster--;
            m_bVisibleMonster = FALSE;
        }

        if (m_bInfravisionOn) {
            m_pArea->m_nInfravision--;
            m_bInfravisionOn = FALSE;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1) {
            if (!g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode || InControl()) {
                if (Animate()) {
                    m_pArea->m_visibility.RemoveCharacter(m_pos,
                        m_id,
                        m_visibleTerrainTable);
                }

                if (m_pArea->m_nCharacters != -1) {
                    m_pArea->m_nCharacters--;
                }
            }
        }

        ClearStoredPaths();

        if (m_pPath != NULL) {
            delete m_pPath;
            m_pPath = NULL;
        }

        if (m_clearAIOnRemoveFromArea) {
            ClearAI(TRUE);
        }

        m_nNewDirection = m_nDirection;
        g_pBaldurChitin->GetObjectGame()->RemoveCharacterFromAllies(m_id);

        switch (m_listType) {
        case CGAMEOBJECT_LIST_FRONT:
            if (m_active
                && m_activeAI
                && m_activeImprisonment) {
                if ((m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    // NOTE: Uninline.
                    m_pArea->RemoveFromMarkers(m_id);
                }

                m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                   m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
            }
            break;
        case CGAMEOBJECT_LIST_BACK:
            if (m_active
                && m_activeAI
                && m_activeImprisonment) {
                if ((m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
                    // NOTE: Uninline.
                    m_pArea->RemoveFromMarkers(m_id);

                    m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                       m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                        m_typeAI.GetEnemyAlly(),
                        m_animation.GetPersonalSpace(),
                        m_bBumpable,
                        m_bOnSearchMap);
                } else {
                    m_pArea->DecrHeightDynamic(m_pos);
                }
            }
            break;
        case CGAMEOBJECT_LIST_FLIGHT:
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 1922
            UTIL_ASSERT(FALSE);
        }

        if (m_bGlobal) {
            CPoint pos(m_pos);

            CGameObject::RemoveFromArea();

            if (InControl() && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
                if (!g_pBaldurChitin->GetObjectGame()->m_bInDestroyGame
                    && !g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition) {
                    m_cLastSpriteUpdate.Initialize(TRUE);

                    CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }

            m_pos = pos;
        } else {
            if (m_type == 1) {
                m_pArea->m_nRandomMonster--;
            }

            CGameObject::RemoveFromArea();

            if (m_bDeleteOnRemove) {
                BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
                    CGameObjectArray::THREAD_ASYNCH,
                    NULL,
                    INFINITE);
                if (rc != CGameObjectArray::SUCCESS) {
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                    // __LINE__: 1934
                    UTIL_ASSERT(FALSE);
                }

                delete this;
            }
        }
    }
}

// 0x6F5FF0
void CGameSprite::AIUpdate()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (m_inControlLastTime != InControl()) {
        JumpToPoint(m_pos, TRUE);
    }

    m_inControlLastTime = InControl();

    if (pGame->GetWorldTimer()->m_active) {
        if ((m_baseStats.m_generalState & STATE_FADE_OUT) != 0) {
            if (static_cast<int>(m_baseStats.field_2F9 + m_baseStats.m_fadeSpeed) > 255) {
                m_baseStats.m_generalState &= ~STATE_FADE_OUT;
                m_derivedStats.m_generalState &= ~STATE_FADE_OUT;
                m_baseStats.field_2F9 = -1;
            } else {
                m_baseStats.field_2F9 += m_baseStats.m_fadeSpeed;
            }
        } else if ((m_baseStats.m_generalState & STATE_FADE_IN) != 0) {
            if (static_cast<int>(m_baseStats.field_2F9 - m_baseStats.m_fadeSpeed) < 0) {
                m_baseStats.m_generalState &= ~STATE_FADE_IN;
                m_derivedStats.m_generalState &= ~STATE_FADE_IN;
                m_baseStats.field_2F9 = -1;
            } else {
                m_baseStats.field_2F9 -= m_baseStats.m_fadeSpeed;
            }
        }

        field_70F6 = rand() % 20 + 1;
        field_70F7 = rand() % 20 + 1;
        field_70F8 = rand() % 20 + 1;
        field_70F9 = rand() % 20 + 1;
        field_70FA = rand() % 100;

        if (m_nBounceCounter <= 0) {
            m_nBounceCounter = 25;
            m_lBounceList.Process(this);
        }
        m_nBounceCounter--;

        if (m_nStealthGreyOut > 0) {
            m_nStealthGreyOut--;

            if (g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter() == pGame->GetCharacterPortraitNum(m_id)) {
                INT nState = pGame->GetButtonArray()->m_nState;
                if (nState != 102
                    && nState != 101
                    && nState != 104
                    && nState != 103
                    && nState != 105
                    && nState != 112
                    && nState != 115
                    && nState != 116
                    && nState != 117
                    && nState != 118
                    && nState != 119
                    && nState != 120
                    && nState != 121
                    && nState != 122
                    && nState != 123
                    && nState != 106) {
                    pGame->GetButtonArray()->ResetState();
                }
            }

            if (m_nStealthGreyOut == 0) {
                if (g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter() == pGame->GetCharacterPortraitNum(m_id)) {
                    INT nState = pGame->GetButtonArray()->m_nState;
                    if (nState != 102
                        && nState != 101
                        && nState != 104
                        && nState != 103
                        && nState != 105
                        && nState != 112
                        && nState != 115
                        && nState != 116
                        && nState != 117
                        && nState != 118
                        && nState != 119
                        && nState != 120
                        && nState != 121
                        && nState != 122
                        && nState != 123
                        && nState != 106) {
                        pGame->GetButtonArray()->UpdateState();
                    }
                }
            }
        }

        if (m_attackSoundDeadzone > 0) {
            m_attackSoundDeadzone--;
        }

        if (m_expirationTime < pGame->GetWorldTimer()->m_gameTime
            && (!m_active || m_canBeSeen <= 0)) {
            RemoveFromArea();
            return;
        }

        if (m_active) {
            if (((m_timeOfDayVisible >> pGame->GetWorldTimer()->GetCurrentHour()) & 0x1) == 0
                && m_canBeSeen <= 0) {
                m_active = FALSE;
                if (m_pPath != NULL) {
                    delete m_pPath;
                    m_pPath = NULL;
                }

                DropSearchRequest();

                switch (m_listType) {
                case CGAMEOBJECT_LIST_FRONT:
                    if ((m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                        // NOTE: Uninline.
                        m_pArea->RemoveFromMarkers(m_id);
                    }

                    m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                       m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                        m_typeAI.GetEnemyAlly(),
                        m_animation.GetPersonalSpace(),
                        m_bBumpable,
                        m_bOnSearchMap);
                    break;
                case CGAMEOBJECT_LIST_BACK:
                    if ((m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
                        // NOTE: Uninline.
                        m_pArea->RemoveFromMarkers(m_id);

                        m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                           m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                            m_typeAI.GetEnemyAlly(),
                            m_animation.GetPersonalSpace(),
                            m_bBumpable,
                            m_bOnSearchMap);
                    } else {
                        m_pArea->IncrHeightDynamic(m_pos);
                    }
                    break;
                case CGAMEOBJECT_LIST_FLIGHT:
                    break;
                default:
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                    // __LINE__: 2424
                    UTIL_ASSERT(FALSE);
                }
            }
        } else {
            if (((m_timeOfDayVisible >> pGame->GetWorldTimer()->GetCurrentHour()) & 0x1) != 0) {
                if (m_canBeSeen > 0) {
                    m_canBeSeen--;
                    if (m_canBeSeen == 0) {
                        if (m_bVisibleMonster) {
                            m_bVisibleMonster = FALSE;
                            m_pArea->m_nVisibleMonster--;
                        }
                    }
                }

                if ((pGame->GetWorldTimer()->m_gameTime & VISIBLE_DELAY) == (m_id & VISIBLE_DELAY)) {
                    CheckIfVisible();
                }

                if (m_canBeSeen <= 0) {
                    m_active = TRUE;

                    switch (m_listType) {
                    case CGAMEOBJECT_LIST_FRONT:
                        if (m_baseStats.m_bStealthMode != TRUE) {
                            if ((m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                                // NOTE: Uninline.
                                m_pArea->AddToMarkers(m_id);
                            }

                            m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                            m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                                m_typeAI.GetEnemyAlly(),
                                m_animation.GetPersonalSpace(),
                                m_bBumpable,
                                m_bOnSearchMap);

                            JumpToPoint(m_pos, TRUE);
                        }
                        break;
                    case CGAMEOBJECT_LIST_BACK:
                        if (m_baseStats.m_bStealthMode != TRUE) {
                            if ((m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
                                m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                                    m_typeAI.GetEnemyAlly(),
                                    m_animation.GetPersonalSpace(),
                                    m_bBumpable,
                                    m_bOnSearchMap);

                                // NOTE: Uninline.
                                m_pArea->AddToMarkers(m_id);

                                JumpToPoint(m_pos, TRUE);
                            } else {
                                m_pArea->IncrHeightDynamic(m_pos);
                            }
                        }
                        break;
                    case CGAMEOBJECT_LIST_FLIGHT:
                        break;
                    default:
                        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                        // __LINE__: 2488
                        UTIL_ASSERT(FALSE);
                    }
                }
            }
        }
    }

    if ((m_derivedStats.m_generalState & STATE_HELPLESS) == 0
        || (m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
        if ((m_derivedStats.m_generalState & STATE_DEAD) == 0 || m_nSequence == SEQ_DIE) {
            m_animationRunning = TRUE;
        }
    } else {
        m_animationRunning = FALSE;
    }

    if (m_AIInhibitor) {
        return;
    }

    if (pGame->GetCharacterPortraitNum(m_id) != CGameObjectArray::INVALID_INDEX
        && (m_derivedStats.m_generalState & STATE_DEAD) == 0) {
        if ((m_typeAI.GetRace() == CAIOBJECTTYPE_R_ELF
                && m_typeAI.GetSubRace() == CAIOBJECTTYPE_SUBRACE_ELF_DROW)
            || (m_typeAI.GetRace() == CAIOBJECTTYPE_R_DWARF
                && m_typeAI.GetSubRace() == CAIOBJECTTYPE_SUBRACE_DWARF_GRAY)) {
            if ((pGame->GetWorldTimer()->IsDay() || pGame->GetWorldTimer()->IsDawn())
                && m_pArea != NULL
                && (m_pArea->GetHeader()->m_areaType & 0x1) != 0
                && (m_pArea->GetHeader()->m_areaType & 0x2) != 0
                && (m_pArea->GetHeader()->m_areaType & 0x40) == 0) {
                // NOTE: Uninline.
                AddPortraitIcon(137);

                if (InControl() && !m_timedEffectList.IsTypeOnList(ICEWIND_CGAMEEFFECT_DAYBLINDNESS)) {
                    ITEM_EFFECT effect;
                    CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_DAYBLINDNESS);

                    CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                        GetPos(),
                        m_id,
                        CPoint(-1, -1));

                    CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            } else {
                if (InControl() && m_timedEffectList.IsTypeOnList(ICEWIND_CGAMEEFFECT_DAYBLINDNESS)) {
                    m_timedEffectList.RemoveAllOfType(this,
                        ICEWIND_CGAMEEFFECT_DAYBLINDNESS,
                        m_timedEffectList.GetPosCurrent(),
                        -1);
                }
            }
        }
    }

    ProcessAI();

    if (m_bSendSpriteUpdate == TRUE) {
        CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        m_bSendSpriteUpdate = FALSE;
    }

    if (m_talkingCounter > 0) {
        m_talkingCounter--;
    }

    if (m_removeFromArea) {
        if (m_bSelected) {
            Unselect();
            pGame->SelectToolbar();
        }
        RemoveFromArea();
        return;
    }

    if (m_pArea == NULL || m_baseStats.m_bStealthMode == TRUE) {
        return;
    }

    if (m_pArea->m_iPicked == m_id) {
        m_bPortraitUpdate = TRUE;
        pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(m_id), 1);
    } else if (m_bPortraitUpdate) {
        m_bPortraitUpdate = FALSE;
        pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(m_id), 1);
    }

    m_marker.AsynchronousUpdate(this);

    if (Orderable(FALSE)
        && (m_bSelected || m_id == m_pArea->m_iPicked)
        && m_targetPoint.x != -1) {
        m_destMarker.AsynchronousUpdate(this);
    }

    if (((m_pArea->GetInfinity()->m_areaType & 0x20) != 0
            || ((m_pArea->GetInfinity()->m_areaType & 0x2) != 0
                && pGame->GetWorldTimer()->IsNight()))
        && m_bSelected
        && pGame->GetCharacterPortraitNum(m_id) != -1
        && (pGame->GetOptions()->m_bDarkvision
            || pGame->GetGroup()->GetCount() == 1)
        && (m_derivedStats.m_generalState & STATE_SLEEPING) == 0
        && (m_derivedStats.m_generalState & STATE_INFRAVISION) != 0) {
        CPoint pt;
        pt.x = m_pos.x;
        pt.y = m_pos.y + m_posZ + m_pArea->GetHeightOffset(m_pos, m_listType);
        COLORREF rgbTintColor = m_pArea->GetTintColor(pt, m_listType);
        BYTE v3 = static_cast<BYTE>((299 * GetRValue(CInfinity::RGB_NIGHT_COLOR) + 587 * GetGValue(CInfinity::RGB_NIGHT_COLOR) + 114 * GetBValue(CInfinity::RGB_NIGHT_COLOR)) / 2550);
        BYTE v4;
        if ((m_pArea->GetInfinity()->m_areaType & 0x2) == 0
            || ((m_pArea->GetInfinity()->m_areaType & 0x40) != 0
                && (m_pArea->GetInfinity()->m_renderDayNightCode & 0x2) != 0)) {
            v4 = static_cast<BYTE>((299 * GetRValue(rgbTintColor) + 587 * GetGValue(rgbTintColor) + 114 * GetBValue(rgbTintColor)) / 2550);
        } else {
            // TODO: Unclear math (merging global tint with area tint).
            v4 = 0;
        }

        if (v4 > v3) {
            if (m_bInfravisionOn) {
                m_bInfravisionOn = FALSE;
                m_pArea->m_nInfravision--;
            }
        } else {
            if (!m_bInfravisionOn) {
                m_bInfravisionOn = TRUE;
                m_pArea->m_nInfravision++;
            }
        }
    } else {
        if (m_bInfravisionOn) {
            m_bInfravisionOn = FALSE;
            m_pArea->m_nInfravision--;
        }
    }

    if (!pGame->GetWorldTimer()->m_active) {
        if (m_active
            && m_activeAI
            && m_activeImprisonment
            && m_lastCharacterCount != m_pArea->m_nCharacters) {
            CheckIfVisible();
        }
        m_lastCharacterCount = m_pArea->m_nCharacters;
        return;
    }

    BOOL v1 = CanAnimate();
    if (v1 != m_bBumpable
        && Animate()
        && !m_baseStats.m_bStealthMode) {
        if (!v1) {
            m_bBumped = FALSE;
            m_ptBumpedFrom.x = -1;
            m_ptBumpedFrom.y = -1;
        }

        m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                           m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
            m_typeAI.GetEnemyAlly(),
            m_animation.GetPersonalSpace(),
            m_bBumpable,
            m_bOnSearchMap);

        m_bBumpable = v1;

        m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                        m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
            m_typeAI.GetEnemyAlly(),
            m_animation.GetPersonalSpace(),
            m_bBumpable,
            m_bOnSearchMap);
    }

    if (m_bBumped && !m_baseStats.m_bStealthMode) {
        m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                           m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
            m_typeAI.GetEnemyAlly(),
            m_animation.GetPersonalSpace(),
            m_bBumpable,
            m_bOnSearchMap);

        SHORT nTableIndex;
        if (m_pArea->m_search.GetCost(m_ptBumpedFrom, GetTerrainTable(), m_animation.GetPersonalSpace(), nTableIndex, TRUE) != CPathSearch::COST_IMPASSABLE) {
            m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                            m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
            JumpToPoint(CPoint(m_ptBumpedFrom.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2,
                            m_ptBumpedFrom.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2),
                TRUE);
            m_bBumped = FALSE;
            m_ptBumpedFrom.x = -1;
            m_ptBumpedFrom.y = -1;
        } else {
            if (!m_baseStats.m_bStealthMode) {
                m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
            }
        }
    }

    if (m_nBloodFlashAmount > 0) {
        m_nBloodFlashAmount -= 5;
        if (m_nBloodFlashAmount < 0) {
            m_nBloodFlashAmount = 0;
        }
        pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(m_id), 1);
    }

    if (m_nDamageLocatorTime > 0) {
        ULONG nQuarterPeriod = CMarker::PICKED_FLASH_PERIOD / 4;
        ULONG nAsyncCounter = g_pBaldurChitin->nAUCounter % CMarker::PICKED_FLASH_PERIOD;
        COLORREF rgbDamageColor = RGB(255, 30, 30);
        BYTE red;
        BYTE green;
        BYTE blue;

        // TODO: Check, probably wrong.
        if (nAsyncCounter < nQuarterPeriod) {
            red = static_cast<BYTE>(nAsyncCounter * GetRValue(rgbDamageColor) / nQuarterPeriod);
            green = static_cast<BYTE>(nAsyncCounter * GetGValue(rgbDamageColor) / nQuarterPeriod);
            blue = static_cast<BYTE>(nAsyncCounter * GetBValue(rgbDamageColor) / nQuarterPeriod);
        } else if (nAsyncCounter < nQuarterPeriod * 2) {
            nAsyncCounter -= nQuarterPeriod;
            red = static_cast<BYTE>(GetRValue(rgbDamageColor) + nAsyncCounter * (255 - GetRValue(rgbDamageColor)) / nQuarterPeriod);
            green = static_cast<BYTE>(GetGValue(rgbDamageColor) + nAsyncCounter * (255 - GetGValue(rgbDamageColor)) / nQuarterPeriod);
            blue = static_cast<BYTE>(GetBValue(rgbDamageColor) + nAsyncCounter * (255 - GetBValue(rgbDamageColor)) / nQuarterPeriod);
        } else if (nAsyncCounter < nQuarterPeriod * 3) {
            nAsyncCounter -= nQuarterPeriod * 2;
            red = static_cast<BYTE>(255 - nAsyncCounter * (255 - GetRValue(rgbDamageColor)) / nQuarterPeriod);
            green = static_cast<BYTE>(255 - nAsyncCounter * (255 - GetGValue(rgbDamageColor)) / nQuarterPeriod);
            blue = static_cast<BYTE>(255 - nAsyncCounter * (255 - GetBValue(rgbDamageColor)) / nQuarterPeriod);
        } else {
            nAsyncCounter -= nQuarterPeriod * 3;
            red = static_cast<BYTE>(GetRValue(rgbDamageColor) - nAsyncCounter * GetRValue(rgbDamageColor) / nQuarterPeriod);
            green = static_cast<BYTE>(GetGValue(rgbDamageColor) - nAsyncCounter * GetGValue(rgbDamageColor) / nQuarterPeriod);
            blue = static_cast<BYTE>(GetBValue(rgbDamageColor) - nAsyncCounter * GetBValue(rgbDamageColor) / nQuarterPeriod);
        }

        m_nDamageLocatorColor = RGB(red, green, blue);

        m_nDamageLocatorTime--;
    }

    if (!m_lstBlood.IsEmpty()) {
        POSITION pos = m_lstBlood.GetHeadPosition();
        while (pos != NULL) {
            POSITION posOld = pos;
            CBlood* pBlood = m_lstBlood.GetNext(pos);
            if (!pBlood->AsynchronousUpdate()) {
                m_lstBlood.RemoveAt(posOld);
                delete pBlood;
            }
        }
    }

    if (m_AISpeed == AI_SPEED_SLOWED) {
        if (m_active
            && m_activeAI
            && m_activeImprisonment
            && ((pGame->GetWorldTimer()->m_gameTime / 2) & VISIBLE_DELAY) == ((m_id / 2) & VISIBLE_DELAY)) {
            CheckIfVisible();
        }
    } else {
        if (m_active
            && m_activeAI
            && m_activeImprisonment
            && (pGame->GetWorldTimer()->m_gameTime & VISIBLE_DELAY) == (m_id & VISIBLE_DELAY)) {
            CheckIfVisible();
        }
    }

    m_lastCharacterCount = m_pArea->m_nCharacters;

    if (m_canBeSeen > 0) {
        m_canBeSeen--;
        if (m_canBeSeen == 0) {
            if (m_bVisibleMonster) {
                m_bVisibleMonster = FALSE;
                m_pArea->m_nVisibleMonster--;
            }
        }
    }

    UpdateSpriteEffect();

    if (m_animationRunning) {
        if (m_posZ < 0) {
            m_posZ -= m_posZDelta;
            m_posZDelta--;
            if (m_posZ >= 0) {
                m_posZ = 0;
                if (m_doBounce > 0) {
                    m_posZ = -1;
                    m_posZDelta = m_doBounce * (rand() % 3 + 1);
                    m_doBounce--;
                } else {
                    m_animationRunning = FALSE;
                }
            }
        }

        if (m_active) {
            if (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id) {
                ChangeDirection();
                if (m_nSequence == CGAMESPRITE_SEQ_ATTACK
                    || m_nSequence == CGAMESPRITE_SEQ_SHOOT
                    || m_nSequence == CGAMESPRITE_SEQ_ATTACK_SLASH
                    || m_nSequence == CGAMESPRITE_SEQ_ATTACK_BACKSLASH
                    || m_nSequence == CGAMESPRITE_SEQ_ATTACK_JAB) {
                    SHORT lSlot;
                    CItem* pItem = m_equipment.m_items[m_equipment.m_selectedWeapon];
                    pItem->Demand();
                    sub_756930(pItem, GetLauncher(pItem->GetAbility(m_equipment.m_selectedWeaponAbility), lSlot));
                    pItem->Release();
                }
                switch (m_nSequence) {
                case CGAMESPRITE_SEQ_ATTACK:
                    if (m_animation.IsEndOfSequence()) {
                        SetSequence(CGAMESPRITE_SEQ_READY);
                    } else {
                        if (m_animation.GetCurrentFrame() == field_740C) {
                            CSound cSound;
                            cSound.SetResRef(field_7408, TRUE, TRUE);
                            cSound.m_nPitchVariance = 5;
                            cSound.m_nVolumeVariance = 20;
                            cSound.SetChannel(3, reinterpret_cast<DWORD>(m_pArea));
                            if (!cSound.GetLooping()) {
                                cSound.SetFireForget(TRUE);
                            }
                            cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
                        }
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_AWAKE:
                    if (m_animation.GetAnimationId() < 0xE000) {
                        if (m_animation.IsBeginningOfSequence()) {
                            // NOTE: Uninline.
                            SetIdleSequence();
                        } else {
                            m_animation.DecrementFrame();
                        }
                    } else {
                        if (m_animation.IsEndOfSequence()) {
                            // NOTE: Uninline.
                            SetIdleSequence();
                        } else {
                            m_animation.IncrementFrame();
                        }
                    }
                    break;
                case CGAMESPRITE_SEQ_CAST:
                    if (m_animation.IsEndOfSequence()) {
                        SetSequence(GetIdleSequence());
                    } else {
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_CONJURE:
                    m_animation.IncrementFrame();
                    break;
                case CGAMESPRITE_SEQ_DAMAGE:
                    if (m_animation.IsBeginningOfSequence()) {
                        CString animationResRef;
                        m_animation.GetAnimationResRef(animationResRef, CGameAnimationType::RANGE_BODY);
                        if (animationResRef == "MWAV") {
                            CGameEffect* pEffect = IcewindMisc::CreateEffectVisualSpellHit(this, 99, 1, 0);
                            AddEffect(pEffect, EFFECT_LIST_TIMED, TRUE, TRUE);
                        }
                    }
                    if (m_animation.IsEndOfSequence()) {
                        m_endOfDamageSeq = TRUE;
                        if (m_pPath != NULL) {
                            SetSequence(CGAMESPRITE_SEQ_WALK);
                        } else {
                            // NOTE: Uninline.
                            SetIdleSequence();
                        }
                    } else {
                        if (field_70FD
                            && m_animation.GetCurrentFrame() == field_7414) {
                            CSound cSound;
                            cSound.SetResRef(CResRef(field_7410), TRUE, TRUE);
                            cSound.m_nPitchVariance = 5;
                            cSound.m_nVolumeVariance = 20;
                            cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                            if (!cSound.GetLooping()) {
                                cSound.SetFireForget(TRUE);
                            }
                            cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
                        }
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_DIE:
                    if (m_animation.IsBeginningOfSequence()) {
                        CString animationResRef;
                        m_animation.GetAnimationResRef(animationResRef, CGameAnimationType::RANGE_BODY);
                        if (animationResRef == "MWAV") {
                            CGameEffect* pEffect = IcewindMisc::CreateEffectVisualSpellHit(this, 97, 1, 0);
                            AddEffect(pEffect, EFFECT_LIST_TIMED, TRUE, TRUE);
                        }
                    }
                    if (m_animation.IsBeginningOfSequence()
                        && field_70FE
                        && !field_7398.IsEmpty()) {
                        INT nIndex = rand() % field_7398.GetCount();
                        POSITION pos = field_7398.GetHeadPosition();
                        while (pos != NULL && nIndex != 0) {
                            nIndex--;
                            field_7398.GetNext(pos);
                        }
                        CGameSpriteSoundEntry& pEntry = field_7398.GetAt(pos);
                        field_7418 = pEntry.field_0;
                        field_741C = pEntry.field_4;
                    }
                    if (m_animation.IsBeginningOfSequence()
                        && field_7101
                        && !field_73EC.IsEmpty()) {
                        INT nIndex = rand() % field_73EC.GetCount();
                        POSITION pos = field_73EC.GetHeadPosition();
                        while (pos != NULL && nIndex != 0) {
                            nIndex--;
                            field_73EC.GetNext(pos);
                        }
                        CGameSpriteSoundEntry& pEntry = field_73EC.GetAt(pos);
                        field_7428 = pEntry.field_0;
                        field_742C = pEntry.field_4;
                    }
                    if (field_1C != 0) {
                        pGame->GetObjectArray()->Delete(field_1C,
                            CGameObjectArray::THREAD_ASYNCH,
                            NULL,
                            INFINITE);
                        field_1C = 0;
                    }
                    m_nSequence = CGAMESPRITE_SEQ_DIE;
                    if (m_animation.IsEndOfSequence()) {
                        if (m_bBloodFlashOn == TRUE) {
                            m_bBloodFlashOn = FALSE;
                            m_nBloodFlashAmount = 128;
                            m_nDamageLocatorTime = 128;
                            pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(m_id), 1);
                        }
                        m_nTwitches = rand() % 4 + rand() % 4;
                        SetSequence(CGAMESPRITE_SEQ_TWITCH);
                    } else {
                        if (field_70FE
                            && m_animation.GetCurrentFrame() == field_741C) {
                            CSound cSound;
                            cSound.SetResRef(CResRef(field_7418), TRUE, TRUE);
                            cSound.m_nPitchVariance = 5;
                            cSound.m_nVolumeVariance = 20;
                            cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                            if (!cSound.GetLooping()) {
                                cSound.SetFireForget(TRUE);
                            }
                            cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
                        }
                        if (field_7101
                            && m_animation.GetCurrentFrame() == field_742C) {
                            CSound cSound;
                            cSound.SetResRef(CResRef(field_7428), TRUE, TRUE);
                            cSound.m_nPitchVariance = 5;
                            cSound.m_nVolumeVariance = 20;
                            cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                            if (!cSound.GetLooping()) {
                                cSound.SetFireForget(TRUE);
                            }
                            cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
                        }
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_HEAD_TURN:
                case CGAMESPRITE_SEQ_READY:
                    if (m_dialogWait <= 0
                        || m_typeAI.GetEnemyAlly() >= CAIObjectType::EA_EVILCUTOFF
                        || !m_bAllowDialogInterrupt
                        || m_derivedStats.m_bIgnoreDialogPause) {
                        AIUpdateWalk();
                    }

                    // NOTE: Uninline.
                    SetIdleSequence();

                    if (m_animation.IsBeginningOfSequence()
                        && field_70FF
                        && m_nSequence == CGAMESPRITE_SEQ_HEAD_TURN
                        && !field_73B4.IsEmpty()) {
                        INT nIndex = rand() % field_73B4.GetCount();
                        POSITION pos = field_73B4.GetHeadPosition();
                        while (pos != NULL && nIndex != 0) {
                            nIndex--;
                            field_73B4.GetNext(pos);
                        }
                        CGameSpriteSoundEntry& pEntry = field_73B4.GetAt(pos);
                        field_7420 = pEntry.field_0;
                        field_7424 = pEntry.field_4;
                    }
                    if (pGame->GetVisibleArea() == m_pArea
                        && field_70FF
                        && m_animation.GetCurrentFrame() == field_7424
                        && m_nSequence == CGAMESPRITE_SEQ_HEAD_TURN
                        && !m_baseStats.m_bStealthMode) {
                        if (m_animation.GetCurrentFrame() == field_740C) {
                            CSound cSound;
                            cSound.SetResRef(field_7420, TRUE, TRUE);
                            cSound.m_nPitchVariance = 5;
                            cSound.m_nVolumeVariance = 20;
                            cSound.SetChannel(13, reinterpret_cast<DWORD>(m_pArea));
                            if (!cSound.GetLooping()) {
                                cSound.SetFireForget(TRUE);
                            }
                            cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
                        }
                    }
                    m_animation.IncrementFrame();
                    break;
                case CGAMESPRITE_SEQ_SHOOT:
                    if (m_animation.IsEndOfSequence()) {
                        // NOTE: Uninline.
                        SetIdleSequence();
                    } else {
                        if (m_animation.GetCurrentFrame() == field_740C) {
                            CSound cSound;
                            cSound.SetResRef(field_7408, TRUE, TRUE);
                            cSound.m_nPitchVariance = 5;
                            cSound.m_nVolumeVariance = 20;
                            cSound.SetChannel(3, reinterpret_cast<DWORD>(m_pArea));
                            if (!cSound.GetLooping()) {
                                cSound.SetFireForget(TRUE);
                            }
                            cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
                        }
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_TWITCH:
                    if (field_1C != 0) {
                        pGame->GetObjectArray()->Delete(field_1C,
                            CGameObjectArray::THREAD_ASYNCH,
                            NULL,
                            INFINITE);
                        field_1C = 0;
                    }
                    if (m_nTwitches != 0) {
                        if (m_animation.IsEndOfSequence()) {
                            if (rand() % 10 == 0) {
                                m_nTwitches--;
                            }
                        } else {
                            m_animation.IncrementFrame();
                        }
                    } else {
                        if ((pGame->GetCharacterPortraitNum(m_id) != -1
                                && (m_derivedStats.m_generalState & STATE_DEAD) != 0)
                            || (pGame->GetCharacterPortraitNum(m_id) != -1
                                && (m_derivedStats.m_generalState & STATE_STONE_DEATH) != 0)) {
                            if (InControl()) {
                                // RemoveFromArea (virtual) clears m_pArea via
                                // CGameObject::RemoveFromArea, so cache the area
                                // first -- the binary saves it (mov edi,[esi+0x12])
                                // before the call and reads m_nCharacters off the
                                // saved pointer afterward.
                                CGameArea* pArea = m_pArea;
                                RemoveFromArea();
                                if (pArea->m_nCharacters == 0) {
                                    pGame->SelectCharacter(pGame->GetProtagonist(), FALSE);
                                    pGame->SelectToolbar();
                                    if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->GetScreenWorld()
                                        && g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter() == pGame->GetCharacterPortraitNum(m_id)) {
                                        g_pBaldurChitin->GetActiveEngine()->OnPortraitLClick(pGame->GetCharacterPortraitNum(pGame->GetProtagonist()));
                                    }
                                }
                            }
                        }
                    }
                    break;
                case CGAMESPRITE_SEQ_WALK:
                    AIUpdateFly();

                    if (m_dialogWait > 0
                        && m_typeAI.GetEnemyAlly() < CAIObjectType::EA_EVILCUTOFF
                        && m_bAllowDialogInterrupt
                        && !m_derivedStats.m_bIgnoreDialogPause) {
                        if (m_nSequence != GetIdleSequence()) {
                            CMessage* message = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()),
                                m_id,
                                m_id);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                    } else {
                        AIUpdateWalk();
                    }

                    if (m_walkBackwards || m_turningAbout) {
                        m_animation.DecrementFrame();
                    } else {
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_EMERGE:
                    if (m_animation.IsEndOfSequence()) {
                        // NOTE: Uninline.
                        SetIdleSequence();
                    } else {
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_HIDE:
                    if (m_animation.IsEndOfSequence()) {
                        // NOTE: Uninline.
                        SetIdleSequence();
                    } else {
                        m_animation.IncrementFrame();
                    }
                    break;
                case CGAMESPRITE_SEQ_SLEEP:
                    if (field_1C != 0) {
                        pGame->GetObjectArray()->Delete(field_1C,
                            CGameObjectArray::THREAD_ASYNCH,
                            NULL,
                            INFINITE);
                        field_1C = 0;
                    }
                    m_nSequence = CGAMESPRITE_SEQ_SLEEP;
                    if (m_animation.IsEndOfSequence()) {
                        if (m_bBloodFlashOn == TRUE) {
                            m_bBloodFlashOn = FALSE;
                            m_nBloodFlashAmount = 128;
                            m_nDamageLocatorTime = 128;
                            pGame->UpdatePortrait(pGame->GetCharacterPortraitNum(m_id), 1);
                        }
                        m_nTwitches = 0;
                    } else {
                        m_animation.IncrementFrame();
                    }
                    break;
                default:
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                    // __LINE__: 3274
                    UTIL_ASSERT(FALSE);
                }

                if (pGame->GetCharacterPortraitNum(m_id) != CGameObjectArray::INVALID_INDEX) {
                    if ((m_derivedStats.m_generalState & STATE_DEAD) == 0
                        && m_typeAI.GetRace() == CAIOBJECTTYPE_R_GNOME
                        && m_typeAI.GetSubRace() == CAIOBJECTTYPE_SUBRACE_GNOME_DEEP) {
                        if (InControl()
                            && (m_baseStats.m_generalState & STATE_NONDETECTION) == 0) {
                            ITEM_EFFECT effect;
                            CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_NONDETECTION);

                            CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                                GetPos(),
                                m_id,
                                CPoint(-1, -1));

                            CMessage* message = new CMessageAddEffect(pEffect,
                                m_id,
                                m_id);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }

                        // NOTE: Uninline.
                        AddPortraitIcon(31);
                    }

                    BOOL bHavePaladinNearby = FALSE;
                    for (SHORT nPortrait = 0; nPortrait < pGame->GetNumCharacters() && !bHavePaladinNearby; nPortrait++) {
                        LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
                            CGameSprite* pPaladin;

                            BYTE rc;
                            do {
                                rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                                    CGameObjectArray::THREAD_ASYNCH,
                                    reinterpret_cast<CGameObject**>(&pPaladin),
                                    INFINITE);
                            } while (rc != CGameObjectArray::SUCCESS);

                            CPoint ptPaladinPos = pPaladin->GetPos();
                            CPoint ptPos = GetPos();
                            INT nPaladinLevel = pPaladin->GetDerivedStats()->GetClassLevel(CAIOBJECTTYPE_C_PALADIN);
                            BOOL bPaladinAnimate = pPaladin->Animate();
                            BOOL bAnimate = Animate();

                            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                                CGameObjectArray::THREAD_ASYNCH,
                                INFINITE);

                            if (bPaladinAnimate
                                && bAnimate
                                && nPaladinLevel >= 2
                                && (ptPos.x - ptPaladinPos.x) * (ptPos.x - ptPaladinPos.x) + (ptPos.y - ptPaladinPos.y) * (ptPos.y - ptPaladinPos.y) <= 10000) {
                                bHavePaladinNearby = TRUE;
                            }
                        }
                    }

                    if (bHavePaladinNearby == TRUE) {
                        // NOTE: Uninline.
                        AddPortraitIcon(67);
                    } else {
                        // NOTE: Uninline.
                        RemovePortraitIcon(67);
                    }
                }
            }
        }
    }
}

// 0x6F2D80
CGameSprite::~CGameSprite()
{
    POSITION pos;

    pos = m_lstBlood.GetHeadPosition();
    while (pos != NULL) {
        CBlood* blood = m_lstBlood.GetNext(pos);
        if (blood != NULL) {
            delete blood;
        }
    }
    m_lstBlood.RemoveAll();

    pos = m_equipedEffectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* effect = m_equipedEffectList.GetNext(pos);
        if (effect != NULL) {
            delete effect;
        }
    }
    m_equipedEffectList.RemoveAll();

    pos = m_timedEffectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* effect = m_timedEffectList.GetNext(pos);
        if (effect != NULL) {
            delete effect;
        }
    }
    m_timedEffectList.RemoveAll();

    pos = m_persistantEffects.GetHeadPosition();
    while (pos != NULL) {
        CPersistantEffect* effect = m_persistantEffects.GetNext(pos);
        if (effect != NULL) {
            delete effect;
        }
    }
    m_persistantEffects.RemoveAll();

    ClearStoredPaths();

    // NOTE: Uninline.
    DropPath();

    if (m_currentSearchRequest != NULL) {
        delete m_currentSearchRequest;
    }

    if (m_curProjectile != NULL) {
        delete m_curProjectile;
    }
    m_curProjectile = NULL;

    if (m_pSpriteEffectArray != NULL) {
        delete m_pSpriteEffectArray;
        delete m_pSpriteEffectArrayPosition;
    }

    if (m_curSpell != NULL) {
        delete m_curSpell;
        m_curSpell = NULL;
    }

    if (m_curItem != NULL) {
        m_curItem = NULL;
    }

    if (m_pLocalVariables != NULL) {
        delete m_pLocalVariables;
    }

    m_portraitIconVidCell.CancelRequest();

    ClearMarshal(FALSE);
}

// 0x6F3A30
void CGameSprite::AddBlood(SHORT nHeight, SHORT nDirection, SHORT nType)
{
    if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nGoreOption == 0) {
        if (nType != CBlood::EXPLODING_DEATH) {
            StartSpriteEffect(0, static_cast<BYTE>(nType), 0, 1);
        }
    } else {
        if (nType == CBlood::EXPLODING_DEATH) {
            CRect rFx;
            CPoint ptReference;

            // NOTE: Uninline.
            m_animation.CalculateFxRect(rFx, ptReference, m_posZ);

            CBlood* pBlood = new CBlood(m_pArea,
                m_pos.x,
                m_pos.y,
                5 * rFx.Height() / 10,
                nDirection,
                nType,
                rFx.Height());
            m_lstBlood.AddTail(pBlood);
        } else {
            StartSpriteEffect(0, static_cast<BYTE>(nType), 0, 1);
        }
    }
}

// 0x6F9040
void CGameSprite::AIUpdateWalk()
{
    CSingleLock pathLock(&(g_pBaldurChitin->GetObjectGame()->field_1B58), FALSE);
    CMessage* message;

    if (m_pPath == NULL && m_currentSearchRequest == NULL) {
        return;
    }

    if (m_pPath == NULL && m_currentSearchRequest != NULL) {
        pathLock.Lock(INFINITE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 3435
        UTIL_ASSERT(m_currentSearchRequest->m_serviceState != CSearchRequest::STATE_STALE);

        if (m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_WAITING) {
            if (m_currentSearchRequest->m_collisionDelay > 1) {
                m_currentSearchRequest->m_collisionDelay--;
                pathLock.Unlock();
                return;
            }

            if (m_currentSearchRequest->m_collisionDelay == 1) {
                m_currentSearchRequest->m_collisionDelay = 0;
                if (m_currentSearchRequest->m_frontList == CSearchRequest::LIST_FRONT) {
                    g_pBaldurChitin->GetObjectGame()->m_searchRequests.AddTail(m_currentSearchRequest);
                    g_pBaldurChitin->GetObjectGame()->m_searchRequestListEmpty = FALSE;
                } else {
                    g_pBaldurChitin->GetObjectGame()->m_searchRequestsBack.AddTail(m_currentSearchRequest);
                }
                ReleaseSemaphore(g_pBaldurChitin->GetObjectGame()->m_hSearchThread, 1, NULL);
                pathLock.Unlock();
                return;
            }
        }

        if (m_currentSearchRequest->m_frontList == CSearchRequest::LIST_BACK_PROMOTE
            && m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_WAITING) {
            m_currentSearchRequest->m_serviceState = CSearchRequest::STATE_STALE;
            m_currentSearchRequest = new CSearchRequest(*m_currentSearchRequest);
            pathLock.Unlock();
            if (m_currentSearchRequest != NULL) {
                SetTarget(m_currentSearchRequest, FALSE, CSearchRequest::LIST_FRONT);
            } else {
                SetSequence(CGAMESPRITE_SEQ_HEAD_TURN);
            }
            return;
        }

        if (m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_ERROR
            || m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_NO_TARGET) {
            if (m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_NO_TARGET) {
                m_pathSearchInvalidDest = TRUE;
            }
            pathLock.Unlock();
            delete m_currentSearchRequest;
            m_currentSearchRequest = NULL;
            SetSequence(CGAMESPRITE_SEQ_HEAD_TURN);
            return;
        }

        if (m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_DONE) {
            LONG* pPath = m_currentSearchRequest->m_pPath;
            SHORT nPath = m_currentSearchRequest->m_nPath;

            m_currentSearchRequest->m_pPath = NULL;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 3490
            UTIL_ASSERT(nPath > 0 && pPath != NULL);

            CPoint pt(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);
            message = new CMessageSetPath(CPathSearch::PointToPosition(&pt),
                pPath,
                nPath,
                TRUE,
                m_curDest,
                m_id,
                m_id);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            if (m_currentSearchRequest->m_searchRc != 0 || nPath == 1) {
                delete m_currentSearchRequest;
                m_currentSearchRequest = NULL;

                if (nPath == 1) {
                    pathLock.Unlock();

                    message = new CMessageDropPath(m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                    if (m_nSequence != GetIdleSequence()) {
                        message = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()),
                            m_id,
                            m_id);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    }

                    delete pPath;
                    return;
                }
            } else {
                // NOTE: Uninline.
                CPathSearch::PositionToPoint(pPath[nPath - 1], &(m_currentSearchRequest->m_sourcePt));
                SetTarget(m_currentSearchRequest, FALSE, CSearchRequest::LIST_BACK_PROMOTE);
            }

            SetSequence(CGAMESPRITE_SEQ_WALK);
            InitializeWalkingSound();

            if (m_followLeader) {
                g_pBaldurChitin->GetObjectGame()->GetGroup()->HandleFollowPath(pPath,
                    nPath,
                    m_curDest,
                    m_pos,
                    m_followLeaderAdditive);
                m_followLeader = FALSE;
                m_followLeaderAdditive = FALSE;
            }

            if (m_followLeaderNext != CGameObjectArray::INVALID_INDEX) {
                message = new CMessageStartFollow(m_id, m_followLeaderNext);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                m_followLeaderNext = CGameObjectArray::INVALID_INDEX;
            }

            delete pPath;
        }

        pathLock.Unlock();

        if (m_pPath == NULL) {
            return;
        }
    }

    if ((m_posDest.x / 8 == (m_pos.x + 4) / 8 && m_posDest.y / 6 == (m_pos.y + 3) / 6)
        || (m_pos.y - m_posDest.y) * (m_pos.y - m_posDest.y) + (m_pos.x - m_posDest.x) * (m_pos.x - m_posDest.x) >= (m_posOld.y - m_posDest.y) * (m_posOld.y - m_posDest.y) + (m_posOld.x - m_posDest.x) * (m_posOld.x - m_posDest.x)) {
        if (m_currPath == m_nPath) {
            delete m_pPath;
            m_pPath = NULL;

            // NOTE: Uninline.
            SetIdleSequence();

            if (!m_bVisibilityUpdated) {
                m_pArea->m_visibility.UpDate(m_posLastVisMapEntry,
                    m_pos,
                    m_id,
                    m_visibleTerrainTable);
                m_posLastVisMapEntry = m_pos;
                m_bVisibilityUpdated = TRUE;
            }
            return;
        }

        CPoint goal;
        CPathSearch::PositionToPoint(m_pPath[m_currPath++], &goal);
        m_posDest.x = goal.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
        m_posDest.y = goal.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2;

        // Scale, delta and facing run in search-grid space, where both axes use
        // SIZEX-wide squares: the goal y is goal.y*SIZEX (not the SIZEY-based
        // m_posDest.y) and the screen m_pos.y is converted via 4/3. Matches the
        // binary 0x6F9040 (iStack_34 = goal.y*SIZEX, iStack_1c = 4*m_pos.y/3).
        LONG searchGoalY = goal.y * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
        LONG searchPosY = 4 * m_pos.y / 3;

        int scale = static_cast<int>(sqrt((double)((m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x) + (searchGoalY - searchPosY) * (searchGoalY - searchPosY))));
        if (scale == 0) {
            // NOTE: Uninline.
            DropPath();

            // NOTE: Uninline.
            SetIdleSequence();

            return;
        }

        if (m_animation.GetMoveScale() > 8) {
            if (scale >= m_animation.GetMoveScale()) {
                scale = (scale + m_animation.GetMoveScale() / 2) / m_animation.GetMoveScale();
            } else {
                scale = 1;
            }

            m_posDelta.x = ((m_posDest.x - m_pos.x) << EXACT_SCALE) / scale;
            m_posDelta.y = ((searchGoalY - searchPosY) << EXACT_SCALE) / scale;
        } else {
            m_posDelta.x = ((m_animation.GetMoveScale() * (m_posDest.x - m_pos.x)) << EXACT_SCALE) / scale;
            m_posDelta.y = ((m_animation.GetMoveScale() * (searchGoalY - searchPosY)) << EXACT_SCALE) / scale;
        }

        // m_posDest (screen-space y) is correct here: GetDirection converts both
        // m_pos.y and the target y by 4/3 into grid space, so passing the already
        // grid-scaled searchGoalY would double-convert. Binary 0x6F9040 likewise
        // reassigns iStack_34 to goal.y*SIZEY before this SetDirection.
        SetDirection(m_posDest);
    }

    m_turningAbout = !m_walkBackwards
        && abs(m_nNewDirection - m_nDirection) >= 5
        && abs(m_nNewDirection - m_nDirection) <= 8;

    SetSequence(CGAMESPRITE_SEQ_WALK);

    CPoint posExactOld(m_posExact);

    m_posExact += m_posDelta;
    m_posOld = m_pos;
    m_pos.x = m_posExact.x >> EXACT_SCALE;
    m_pos.y = (3 * m_posExact.y / 4) >> EXACT_SCALE;

    CPoint ptOldSearch(m_posOld.x / CPathSearch::GRID_SQUARE_SIZEX,
        m_posOld.y / CPathSearch::GRID_SQUARE_SIZEY);
    CPoint ptSearch(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
        m_pos.y / CPathSearch::GRID_SQUARE_SIZEY);

    if (ptOldSearch == ptSearch
        || m_animation.GetListType() == LIST_FLIGHT) {
        if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
            && (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bFootStepsSounds
                || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1)
            && m_pArea == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
            INT nSndWalk = m_nSndWalk % m_animation.GetSndWalkFreq();
            m_nSndWalk++;

            if (nSndWalk == 0) {
                BYTE currSndWalk = m_currSndWalk;
                m_currSndWalk = (m_currSndWalk + 1) % 2;

                CPoint ear;
                LONG earZ;
                g_pBaldurChitin->cSoundMixer.GetListenPosition(ear, earZ);

                m_sndWalk[m_currSndWalk].Stop();
                m_sndWalk[m_currSndWalk].SetResRef(m_sndWalk[currSndWalk].GetResRef(), TRUE, TRUE);

                LONG priority = max(99 - 99 * ((ear.y - m_pos.y) * (ear.y - m_pos.y) / 144 + (ear.x - m_pos.x) * (ear.x - m_pos.x) / 256) / 6400, 0);
                m_sndWalk[m_currSndWalk].SetPriority(static_cast<BYTE>(priority));

                m_sndWalk[m_currSndWalk].Play(m_pos.x, m_pos.y, m_posZ, FALSE);
            }
        }

        m_sndSpriteEffect.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        m_sndReady.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
    } else {
        m_pArea->m_search.RemoveObject(ptOldSearch,
            m_typeAI.GetEnemyAlly(),
            m_animation.GetPersonalSpace(),
            m_bBumpable,
            m_bOnSearchMap);

        // Binary 0x6f9040 gates the bump only on an impassable next cell that
        // ClearBumpPath cannot clear -- there is NO InControl() check here. The
        // extra guard was invented; it would suppress bumping whenever the sprite
        // is not under local control. (Single-player InControl() is always TRUE,
        // so this is behaviour-neutral there; the binary's form is restored.)
        if (m_pArea->m_search.GetMobileCost(ptSearch, m_terrainTable, m_animation.GetPersonalSpace(), TRUE) == CPathSearch::COST_IMPASSABLE
            && !ClearBumpPath(ptOldSearch, ptSearch)) {
            if (!m_baseStats.m_bStealthMode) {
                m_pArea->m_search.AddObject(ptOldSearch,
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);

                m_posExact = posExactOld;
                m_pos = m_posOld;

                if (m_currentSearchRequest != NULL) {
                    pathLock.Lock(INFINITE);

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                    // __LINE__: 3776
                    UTIL_ASSERT(m_currentSearchRequest->m_serviceState != CSearchRequest::STATE_STALE);

                    if ((m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_WAITING
                            && m_currentSearchRequest->m_collisionDelay == 0)
                        || m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_PROCESSING) {
                        m_currentSearchRequest->m_serviceState = CSearchRequest::STATE_STALE;
                        m_currentSearchRequest = new CSearchRequest(*m_currentSearchRequest);
                        if (m_currentSearchRequest != NULL) {
                            SetTarget(m_currentSearchRequest, TRUE, CSearchRequest::LIST_FRONT);
                        } else {
                            SetSequence(CGAMESPRITE_SEQ_HEAD_TURN);
                        }
                    } else {
                        if (m_currentSearchRequest->m_pPath != NULL) {
                            delete m_currentSearchRequest->m_pPath;
                            m_currentSearchRequest->m_pPath = NULL;
                        }
                        SetTarget(m_currentSearchRequest, TRUE, CSearchRequest::LIST_FRONT);
                    }

                    pathLock.Unlock();
                } else {
                    // Binary 0x6f9040 calls this virtually: (*(vtable+0xb8))(&pt, TRUE).
                    // CGameSprite vtable slot 0xb8 is SetTarget(const CPoint&, BOOL)
                    // (CGameSprite.h /* 00B8 */ = 0x707980), NOT JumpToPoint (which is
                    // non-virtual, 0x745950). The blocked sprite re-issues a search to its
                    // path end; it does not teleport. (JumpToPoint here would jump to
                    // m_pPath[m_nPath-1] = the far dest = a visible teleport absent from IWD2.exe.)
                    CPoint pt;
                    CPathSearch::PositionToPoint(m_pPath[m_nPath - 1], &pt);
                    SetTarget(CPoint(pt.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2,
                                  pt.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2),
                        TRUE);
                }
            }
        } else {
            if (!m_baseStats.m_bStealthMode) {
                m_pArea->m_search.AddObject(ptSearch,
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
            }

            char* pSndWalk = m_animation.GetSndWalk(m_pArea->m_search.GetTableIndex(ptOldSearch));
            if (pSndWalk != NULL) {
                m_sndWalk[m_currSndWalk].SetResRef(CResRef(pSndWalk), TRUE, TRUE);

                if (*pSndWalk != '\0') {
                    delete pSndWalk;
                }
            }

            if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
                && (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bFootStepsSounds
                    || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1)
                && m_pArea == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
                INT nSndWalk = m_nSndWalk % m_animation.GetSndWalkFreq();
                m_nSndWalk++;

                if (nSndWalk == 0) {
                    BYTE currSndWalk = m_currSndWalk;
                    m_currSndWalk = (m_currSndWalk + 1) % 2;

                    CPoint ear;
                    LONG earZ;
                    g_pBaldurChitin->cSoundMixer.GetListenPosition(ear, earZ);

                    m_sndWalk[m_currSndWalk].Stop();
                    m_sndWalk[m_currSndWalk].SetResRef(m_sndWalk[currSndWalk].GetResRef(), TRUE, TRUE);

                    LONG priority = max(99 - 99 * ((ear.y - m_pos.y) * (ear.y - m_pos.y) / 144 + (ear.x - m_pos.x) * (ear.x - m_pos.x) / 256) / 6400, 0);
                    m_sndWalk[m_currSndWalk].SetPriority(static_cast<BYTE>(priority));

                    m_sndWalk[m_currSndWalk].Play(m_pos.x, m_pos.y, m_posZ, FALSE);
                }
            }

            m_sndSpriteEffect.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
            m_sndReady.SetCoordinates(m_pos.x, m_pos.y, m_posZ);
        }
    }

    if ((m_posLastVisMapEntry.x / CVisibilityMap::SQUARE_SIZEX != m_pos.x / CVisibilityMap::SQUARE_SIZEX
            || m_posLastVisMapEntry.y / CVisibilityMap::SQUARE_SIZEY != m_pos.y / CVisibilityMap::SQUARE_SIZEY)
        && g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1
        && (!g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode || InControl())) {
        SHORT nTableIndex;
        if (m_pArea->m_search.GetLOSCost(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX, m_pos.y / CPathSearch::GRID_SQUARE_SIZEY), m_terrainTable, nTableIndex, FALSE) != CPathSearch::COST_IMPASSABLE) {
            m_pArea->m_visibility.UpDate(m_posLastVisMapEntry,
                m_pos,
                m_id,
                m_visibleTerrainTable);
            m_posLastVisMapEntry = m_pos;
            m_bVisibilityUpdated = TRUE;
            if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
                && g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineMap) {
                SHORT nPortrait = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id);
                CUIControlButtonMapAreaMap* pMap = static_cast<CUIControlButtonMapAreaMap*>(g_pBaldurChitin->m_pEngineMap->GetManager()->GetPanel(2)->GetControl(2));
                pMap->field_7DA |= 1 << nPortrait;
            }
        } else {
            m_bVisibilityUpdated = FALSE;
        }
    }
}

// 0x6FA810
void CGameSprite::SetPath(LONG* pPath, SHORT nPath)
{
    CPoint start;
    CPoint goal;

    // NOTE: Uninline.
    DropPath();

    m_nPath = nPath;
    m_pPath = pPath;
    m_currPath = 1;
    m_posDest.x = m_pos.x + 4;
    m_posDest.y = m_pos.y + 3;

    // NOTE: Uninline.
    CPathSearch::PositionToPoint(m_pPath[0], &start);

    // NOTE: Uninline.
    CPathSearch::PositionToPoint(m_pPath[nPath - 1], &goal);

    if (abs(goal.x - start.x) <= 4 && abs(goal.y - start.y) <= 4 && nPath <= 4) {
        m_walkBackwards = TRUE;
    }

    SetSequence(CGAMESPRITE_SEQ_WALK);
}

// 0x6FA900
BOOL CGameSprite::ClearBumpPath(const CPoint& start, const CPoint& goal)
{
    if (GetAIType().GetEnemyAlly() > CAIObjectType::EA_GOODCUTOFF) {
        return FALSE;
    }

    CSearchBitmap& search = m_pArea->m_search;
    CTypedPtrList<CPtrList, LONG*> obstacles;
    BOOL bGathered = FALSE;

    // abStack_ac[12] @ 0x6FA900. The two indexed writes use the FULL signed
    // index 3*dy+dx+4 (the binary indexes abStack_ac[<int expr>], not the
    // truncated dirTable[0]/[8] byte), and the goal write happens BEFORE
    // dirTable[8] is assigned. The index lands in [0,8] only when start/goal are
    // adjacent (one walk step, the binary's precondition). When a step spans >1
    // cell the index leaves the array; the binary scribbles dead frame locals
    // (harmless), but our standalone array would overflow the stack and corrupt
    // the caller frame, so the indexed writes are guarded to the array bounds.
    // Only dirTable[0..8] are ever read, and those are identical to the binary
    // for every adjacent input.
    BYTE dirTable[12];
    int idxGoal = 3 * (goal.y - start.y) + (goal.x - start.x) + 4;
    int idxBack = 3 * (start.y - goal.y) + (start.x - goal.x) + 4;
    dirTable[0] = (BYTE)idxGoal;
    dirTable[1] = 1;
    dirTable[2] = 2;
    dirTable[3] = 3;
    dirTable[4] = 0xFF;
    dirTable[5] = 5;
    dirTable[6] = 6;
    dirTable[7] = 7;
    if (idxGoal >= 0 && idxGoal < (int)sizeof(dirTable)) {
        dirTable[idxGoal] = 0;
    }
    dirTable[8] = (BYTE)idxBack;
    if (idxBack >= 0 && idxBack < (int)sizeof(dirTable)) {
        dirTable[idxBack] = 8;
    }

    if (start.x == goal.x && start.y == goal.y) {
        return TRUE;
    }

    BYTE personalSpace = m_animation.GetPersonalSpace();
    SHORT nTableIndex;
    if (search.GetCost(goal, GetTerrainTable(), personalSpace, nTableIndex, FALSE) == CPathSearch::COST_IMPASSABLE) {
        return FALSE;
    }

    personalSpace = m_animation.GetPersonalSpace();
    SHORT radius = (SHORT)(personalSpace - 2) / 2;

    for (SHORT a = -radius; a <= radius; a++) {
        int cellX = a + goal.x;
        if (cellX < 0 || cellX >= search.m_GridSquareDimensions.cx) {
            continue;
        }

        for (SHORT b = -radius; b <= radius; b++) {
            int cellY = b + goal.y;
            if (cellY < 0 || cellY >= search.m_GridSquareDimensions.cx) {
                continue;
            }

            int absA = (a < 0) ? -a : a;
            int absB = (b < 0) ? -b : b;
            if (absA + absB > radius + 1) {
                continue;
            }

            BYTE dynByte = search.m_pDynamicCost[cellY * search.m_GridSquareDimensions.cx + cellX];
            BYTE obstacleCount = dynByte >> 1;
            if (obstacleCount > 7) {
                return FALSE;
            }

            if (!bGathered) {
                bGathered = TRUE;
                BYTE ps = m_animation.GetPersonalSpace();
                CPoint worldCenter;
                worldCenter.x = goal.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
                worldCenter.y = goal.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2;
                m_pArea->GetCloseObjects(m_posVertList, worldCenter, CAIObjectType::ANYONE,
                    ((ps + 11) / 2) * CPathSearch::GRID_SQUARE_SIZEX,
                    m_terrainTable, obstacles, FALSE, FALSE);
            }

            POSITION pos = obstacles.GetHeadPosition();
            while (pos != NULL) {
                if (obstacleCount == 0) {
                    break;
                }

                POSITION posCurrent = pos;
                LONG objId = reinterpret_cast<LONG>(obstacles.GetNext(pos));

                CGameObject* pObject;
                BYTE rc = m_pArea->m_pGame->GetObjectArray()->GetShare(objId,
                    CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
                if (rc != CGameObjectArray::SUCCESS) {
                    continue;
                }

                CGameSprite* pObstacle = static_cast<CGameSprite*>(pObject);
                if (pObstacle->m_bBumpable == 0 || pObstacle->field_54B8 != 0) {
                    m_pArea->m_pGame->GetObjectArray()->ReleaseShare(objId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                    continue;
                }

                int obsCellX = pObstacle->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX;
                int obsCellY = pObstacle->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY;

                BYTE obsPS = pObstacle->m_animation.GetPersonalSpace();
                SHORT obsRadius = (SHORT)(obsPS >> 1);

                BOOL bFound = FALSE;
                for (SHORT fy = -obsRadius; fy <= obsRadius && !bFound; fy++) {
                    for (SHORT fx = -obsRadius; fx <= obsRadius && !bFound; fx++) {
                        if (obsCellX + fx == cellX && obsCellY + fy == cellY) {
                            bFound = TRUE;
                        }
                    }
                }

                if (!bFound) {
                    m_pArea->m_pGame->GetObjectArray()->ReleaseShare(objId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                    continue;
                }

                int origObsCellX = pObstacle->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX;
                int origObsCellY = pObstacle->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY;

                search.AddObject(goal,
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);

                BYTE denyRc;
                do {
                    denyRc = m_pArea->m_pGame->GetObjectArray()->GetDeny(objId,
                        CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
                } while (denyRc == CGameObjectArray::SHARED);

                if (denyRc == CGameObjectArray::SUCCESS) {
                    pObstacle = static_cast<CGameSprite*>(pObject);

                    BYTE obsEA = pObstacle->GetAIType().GetEnemyAlly();
                    BYTE obsPS2 = pObstacle->m_animation.GetPersonalSpace();
                    CPoint obsCell(origObsCellX, origObsCellY);

                    search.RemoveObject(obsCell,
                        obsEA,
                        obsPS2,
                        pObstacle->m_bBumpable,
                        pObstacle->m_bOnSearchMap);

                    BOOL bPlaced = FALSE;
                    for (int d = 0; d < 9 && !bPlaced; d++) {
                        BYTE dirCode = dirTable[d];
                        if (dirCode == 0xFF) {
                            continue;
                        }

                        int nx = (dirCode % 3 - 1) + origObsCellX;
                        int ny = (dirCode / 3 - 1) + origObsCellY;
                        CPoint neighborPt(nx, ny);

                        BYTE obsPS3 = pObstacle->m_animation.GetPersonalSpace();
                        SHORT obsTableIdx;
                        if (search.GetCost(neighborPt, pObstacle->GetTerrainTable(),
                                obsPS3, obsTableIdx, TRUE) == CPathSearch::COST_IMPASSABLE) {
                            continue;
                        }

                        BYTE obsPS4 = pObstacle->m_animation.GetPersonalSpace();
                        BYTE obsEA2 = pObstacle->GetAIType().GetEnemyAlly();
                        search.AddObject(obsCell, obsEA2, obsPS4,
                            pObstacle->m_bBumpable, pObstacle->m_bOnSearchMap);

                        CPoint jumpDest;
                        jumpDest.x = nx * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
                        jumpDest.y = ny * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2;
                        pObstacle->JumpToPoint(jumpDest, TRUE);

                        if (!pObstacle->m_bBumped) {
                            pObstacle->m_bBumped = TRUE;
                            pObstacle->m_ptBumpedFrom.x = origObsCellX;
                            pObstacle->m_ptBumpedFrom.y = origObsCellY;
                        }

                        obstacles.RemoveAt(posCurrent);
                        obstacleCount--;
                        bPlaced = TRUE;
                    }

                    if (!bPlaced) {
                        field_54B8 = 1;

                        for (int d = 0; d < 9 && !bPlaced; d++) {
                            BYTE dirCode = dirTable[d];
                            if (dirCode == 0xFF) {
                                continue;
                            }

                            int nx = (dirCode % 3 - 1) + origObsCellX;
                            int ny = (dirCode / 3 - 1) + origObsCellY;
                            CPoint fromPt(origObsCellX, origObsCellY);
                            CPoint toPt(nx, ny);

                            if (!pObstacle->ClearBumpPath(fromPt, toPt)) {
                                continue;
                            }

                            BYTE obsPS5 = pObstacle->m_animation.GetPersonalSpace();
                            BYTE obsEA3 = pObstacle->GetAIType().GetEnemyAlly();
                            search.AddObject(obsCell, obsEA3, obsPS5,
                                pObstacle->m_bBumpable, pObstacle->m_bOnSearchMap);

                            CPoint jumpDest;
                            jumpDest.x = nx * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
                            jumpDest.y = ny * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2;
                            pObstacle->JumpToPoint(jumpDest, TRUE);

                            if (!pObstacle->m_bBumped) {
                                pObstacle->m_bBumped = TRUE;
                                pObstacle->m_ptBumpedFrom.x = origObsCellX;
                                pObstacle->m_ptBumpedFrom.y = origObsCellY;
                            }

                            obstacles.RemoveAt(posCurrent);
                            obstacleCount--;
                            bPlaced = TRUE;
                        }

                        field_54B8 = 0;

                        if (!bPlaced) {
                            BYTE obsPS6 = pObstacle->m_animation.GetPersonalSpace();
                            BYTE obsEA4 = pObstacle->GetAIType().GetEnemyAlly();
                            search.AddObject(obsCell, obsEA4, obsPS6,
                                pObstacle->m_bBumpable, pObstacle->m_bOnSearchMap);
                        }
                    }

                    m_pArea->m_pGame->GetObjectArray()->ReleaseDeny(objId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }

                search.RemoveObject(goal,
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);

                m_pArea->m_pGame->GetObjectArray()->ReleaseShare(objId,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }

            if (obstacleCount != 0) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

// 0x6FB440
BOOL CGameSprite::CanAnimate()
{
    if (!InControl()) {
        return FALSE;
    }

    if (m_animation.GetMoveScale() == 0
        || m_typeAI.GetEnemyAlly() == CAIObjectType::EA_ENEMY
        || !Animate()) {
        return FALSE;
    }

    if (m_typeAI.GetEnemyAlly() > CAIObjectType::EA_GOODCUTOFF
        || m_curAction.m_actionID != CAIAction::NO_ACTION) {
        // 0x8F94F0
        static SHORT word_8F94F0[] = {
            CAIAction::RANDOMWALK,
            CAIAction::RANDOMWALKCONTINUOUS,
            CAIAction::WAIT,
            CAIAction::FACE,
            CAIAction::ATTACK,
            CAIAction::GROUPATTACK,
            CAIAction::ATTACKNOSOUND,
            CAIAction::ATTACKONEROUND,
            CAIAction::ATTACKREEVALUATE,
        };

        // 0x8F9504
        static SHORT word_8F9504[] = {
            CAIAction::RANDOMWALK,
            CAIAction::RANDOMWALKCONTINUOUS,
            CAIAction::WAIT,
            CAIAction::FACE,
        };

        WORD wType = 0;
        CItem* pItem = m_equipment.m_items[m_equipment.m_selectedWeapon];
        if (pItem != NULL) {
            pItem->Demand();
            const ITEM_ABILITY* ability = pItem->GetAbility(m_equipment.m_selectedWeaponAbility);
            if (ability != NULL) {
                wType = ability->type;
            }
            pItem->Release();
        }

        if (wType == 4 || wType == 2) {
            for (int index = 0; index < sizeof(word_8F94F0) / sizeof(word_8F94F0[0]); index++) {
                if (word_8F94F0[index] == m_curAction.m_actionID) {
                    return TRUE;
                }
            }
            return FALSE;
        } else {
            for (int index = 0; index < sizeof(word_8F9504) / sizeof(word_8F9504[0]); index++) {
                if (word_8F9504[index] == m_curAction.m_actionID) {
                    return TRUE;
                }
            }
            return FALSE;
        }
    }

    return TRUE;
}

// 0x6FB630
void CGameSprite::AIUpdateFly()
{
    if (m_fCurrCircleChange < m_fCircleChange) {
        double fCurrCircleChange = min(m_fCircleChange, m_fCurrCircleChange + 1.0);

        if (m_circleFacing != 0) {
            double fAngle = static_cast<double>(m_animation.GetMoveScale())
                    * static_cast<double>(m_circleFacing)
                    * fCurrCircleChange
                    / static_cast<double>(m_radius)
                + m_fDirectionOffset;

            m_pos.x = m_posOld.x + static_cast<LONG>(cos(fAngle) * static_cast<double>(m_radius));
            m_pos.y = m_posOld.y - static_cast<LONG>(sin(fAngle) * static_cast<double>(m_radius) * 3.0 / 4.0);

            if (m_circleFacing == FLY_LEFT) {
                if (m_pos != m_posOld) {
                    SetDirection((GetDirection(CPoint(m_pos.x, 4 * m_pos.y / 3),
                                      CPoint(m_posOld.x, 4 * m_posOld.y / 3))
                                     + 4)
                        % 16);
                } else {
                    SetDirection(m_nNewDirection);
                }
            } else {
                if (m_pos != m_posOld) {
                    SetDirection((GetDirection(CPoint(m_pos.x, 4 * m_pos.y / 3),
                                      CPoint(m_posOld.x, 4 * m_posOld.y / 3))
                                     + 12)
                        % 16);
                } else {
                    SetDirection(m_nNewDirection);
                }
            }
        } else {
            switch (m_nDirection) {
            case 0:
                m_pos.y = m_posOld.y + static_cast<LONG>(static_cast<double>(m_radius) * fCurrCircleChange / m_fCircleChange * 3.0 / 4.0);
                break;
            case 4:
                m_pos.x = m_posOld.x - static_cast<LONG>(static_cast<double>(m_radius) * fCurrCircleChange / m_fCircleChange);
                break;
            case 8:
                m_pos.y = m_posOld.y - static_cast<LONG>(static_cast<double>(m_radius) * fCurrCircleChange / m_fCircleChange * 3.0 / 4.0);
                break;
            case 12:
                m_pos.x = m_posOld.x + static_cast<LONG>(static_cast<double>(m_radius) * fCurrCircleChange / m_fCircleChange);
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                // __LINE__: 4554
                UTIL_ASSERT(FALSE);
            }
        }

        m_fCurrCircleChange += 1.0;
        if (m_fCurrCircleChange >= m_fCircleChange) {
            SetSequence(SEQ_READY);
        }
    }
}

// 0x6FBA50
void CGameSprite::GetNextWaypoint(CPoint* pt)
{
    if (m_pPath != NULL) {
        CPathSearch::PositionToPoint(m_pPath[m_currPath - 1], pt);

        pt->x *= CPathSearch::GRID_SQUARE_SIZEX;
        pt->x += CPathSearch::GRID_SQUARE_SIZEX / 2;

        pt->y *= CPathSearch::GRID_SQUARE_SIZEY;
        pt->y += CPathSearch::GRID_SQUARE_SIZEY / 2;
    } else {
        *pt = m_pos;
    }
}

// 0x6FBAD0
void CGameSprite::ChangeDirection()
{
    if (m_nNewDirection == m_nDirection) {
        return;
    }

    if (m_skipDeltaDirection > 0) {
        m_skipDeltaDirection -= 2;
        if (m_skipDeltaDirection <= 1) {
            m_nDirection = (m_nDirection + 2 * m_deltaDirection + 16) % 16;
        }
    }

    m_nDirection = (m_nDirection + m_deltaDirection + 16) % 16;
    if (m_nNewDirection != m_nDirection) {
        m_nDirection = (m_nDirection + m_deltaDirection + 16) % 16;
    }

    m_animation.ChangeDirection(m_nDirection);

    if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
        && (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bFootStepsSounds
            || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1)
        && m_pArea == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
        m_currSndArmor = 0;
        m_sndArmor[m_currSndArmor].Stop();

        // NOTE: Uninline.
        char* sndArmor = m_animation.GetSndArmor();

        if (sndArmor[0] != '\0') {
            // NOTE: Uninline.
            m_sndArmor[m_currSndArmor].SetResRef(CResRef(sndArmor), TRUE, TRUE);
            delete sndArmor;

            CPoint ear;
            LONG earZ;
            g_pBaldurChitin->cSoundMixer.GetListenPosition(ear, earZ);

            LONG priority = max(99 - 99 * ((ear.y - m_pos.y) * (ear.y - m_pos.y) / 144 + (ear.x - m_pos.x) * (ear.x - m_pos.x) / 256) / 6400, 0);
            m_sndArmor[m_currSndArmor].SetPriority(static_cast<BYTE>(priority));

            m_sndArmor[m_currSndArmor].Play(m_pos.x, m_pos.y, m_posZ, FALSE);
        }
    }
}

// 0x6FBF10
void CGameSprite::CheckIfVisible()
{
    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1
        || m_animation.GetListType() == LIST_FLIGHT) {
        if (Animate()) {
            m_canBeSeen = 4 * (VISIBLE_DELAY + 1);
            return;
        }
    }

    for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->GetNumCharacters(); nPortrait++) {
        CGameSprite* pSprite;

        BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait),
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->GetArea() == m_pArea
                && pSprite->Animate()
                && m_pArea->CheckLOS(m_pos, pSprite->GetPos(), m_visibleTerrainTable, TRUE)) {
                if (!m_bVisibleMonster
                    && m_typeAI.GetEnemyAlly() >= CAIObjectType::EA_EVILCUTOFF
                    && Animate()
                    && ((m_derivedStats.m_generalState & STATE_INVISIBLE) == 0 || (m_baseStats.m_critSectService & 0x1) != 0)
                    && !m_baseStats.m_bStealthMode
                    && (m_baseStats.m_flags & 0x8000) == 0) {
                    if (m_pArea->m_nVisibleMonster == 0) {
                        AutoPause(0x200);
                    }

                    m_bVisibleMonster = TRUE;
                    m_pArea->m_nVisibleMonster++;
                }

                if (m_canBeSeen == 0) {
                    CAITrigger becameVisible(CAITRIGGER_BECAMEVISIBLE, 0);
                    m_bSeenPartyBefore = TRUE;
                    CMessage* message = new CMessageSetTrigger(becameVisible, m_id, pSprite->GetId());
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                m_canBeSeen = 4 * (VISIBLE_DELAY + 1);

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                break;
            } else {
                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait),
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }
}

// 0x6FF2F0
void CGameSprite::DropPath()
{
    if (m_pPath != NULL) {
        delete m_pPath;
        m_pPath = NULL;
    }
}

// 0x6FF320
void CGameSprite::DropSearchRequest()
{
    if (m_currentSearchRequest != NULL) {
        CSingleLock searchLock(&(g_pBaldurChitin->GetObjectGame()->field_1B58), TRUE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 5590
        UTIL_ASSERT(m_currentSearchRequest->m_serviceState != CSearchRequest::STATE_STALE);

        if ((m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_WAITING
                && m_currentSearchRequest->m_collisionDelay == 0)
            || m_currentSearchRequest->m_serviceState == CSearchRequest::STATE_PROCESSING) {
            m_currentSearchRequest->m_serviceState = CSearchRequest::STATE_STALE;
        } else {
            delete m_currentSearchRequest;
        }

        m_currentSearchRequest = NULL;

        searchLock.Unlock();
    }
}

// 0x6FF410
BOOL CGameSprite::MoveToBack()
{
    if (m_animation.CanLieDown()) {
        if (m_listType == LIST_BACK) {
            if ((m_derivedStats.m_generalState & STATE_SLEEPING) == 0) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                // __LINE__: 5669
                UTIL_ASSERT(FALSE);
            }

            // NOTE: Uninline.
            m_pArea->RemoveFromMarkers(m_id);

            m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                               m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
            m_pArea->IncrHeightDynamic(m_pos);
            m_derivedStats.m_generalState &= ~STATE_SLEEPING;
            return FALSE;
        }

        if (m_posVertList != NULL) {
            m_pArea->RemoveObject(m_posVertList, m_listType, m_id);
            m_posVertList = NULL;

            if ((m_derivedStats.m_generalState & STATE_DEAD) != 0) {
                // NOTE: Uninline.
                m_pArea->RemoveFromMarkers(m_id);

                m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                   m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
                m_pArea->IncrHeightDynamic(m_pos);
            }

            m_listType = LIST_BACK;
            m_pArea->AddObject(m_id, LIST_BACK);
            return TRUE;
        }

        if (m_moveToFrontQueue > 0) {
            m_moveToFrontQueue--;
        } else {
            m_moveToBackQueue++;
        }
        return FALSE;
    }

    if ((m_derivedStats.m_generalState & STATE_DEAD) != 0) {
        // NOTE: Uninline.
        m_pArea->RemoveFromMarkers(m_id);
    }
    return TRUE;
}

// 0x6FF6D0
BOOL CGameSprite::MoveToFront()
{
    if (m_animation.CanLieDown()) {
        if (m_listType == LIST_FRONT) {
            return FALSE;
        }

        if (m_posVertList != NULL) {
            m_pArea->RemoveObject(m_posVertList, m_listType, m_id);
            m_posVertList = NULL;

            if ((m_derivedStats.m_generalState & STATE_SLEEPING) == 0) {
                // NOTE: Uninline.
                m_pArea->AddToMarkers(m_id);

                m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
                m_pArea->DecrHeightDynamic(m_pos);
            }

            m_listType = LIST_FRONT;
            m_pArea->AddObject(m_id, LIST_FRONT);
            return TRUE;
        }

        if (m_moveToBackQueue > 0) {
            m_moveToBackQueue--;
        } else {
            m_moveToFrontQueue++;
        }
        return FALSE;
    }

    if ((m_derivedStats.m_generalState & STATE_SLEEPING) == 0) {
        // NOTE: Uninline.
        m_pArea->AddToMarkers(m_id);
    }
    return TRUE;
}

// 0x6FF8A0
void CGameSprite::OnActionButton(const CPoint& pt)
{
    if (m_canBeSeen > 0) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        CAIGroup* pGroup = pGame->GetGroup();
        CAIAction action1;
        CAIAction action2;
        CAIObjectType targetType(0, 0, 0, 0, 0, 0, 0, 0, m_id, 0, 0);

        switch (pGame->GetState()) {
        case 0:
            if (pGame->GetCharacterPortraitNum(m_id) != -1
                || pGame->IsAlly(m_id)
                || pGame->IsFamiliar(m_id)) {
                if (!pGame->GetGameSave()->m_bArenaMode || InControl()) {
                    if (InControl()) {
                        // NOTE: Uninline.
                        pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

                        if (!m_moraleFailure) {
                            if (!g_pBaldurChitin->GetScreenWorld()->GetCtrlKey()
                                && !g_pBaldurChitin->GetScreenWorld()->GetShiftKey()) {
                                pGame->UnselectAll();
                            }
                            pGame->SelectCharacter(m_id, !m_bSelected);
                            pGame->SelectToolbar();
                        }
                    } else {
                        if (pGroup->GetCount() > 0
                            && m_pArea == pGame->GetVisibleArea()) {
                            action1 = CAIAction(CAIAction::MOVETOOBJECTFOLLOW,
                                targetType,
                                0,
                                0,
                                0);
                            pGroup->GroupAction(action1, TRUE, NULL);
                        } else {
                            pGame->UnselectAll();
                            pGame->GetVisibleArea()->m_bPicked = FALSE;
                            pGame->GetVisibleArea()->m_iPicked = CGameObjectArray::INVALID_INDEX;
                            pGame->GetVisibleArea()->m_nToolTip = 0;
                            pGame->GetVisibleArea()->OnDeactivation();
                            pGame->m_visibleArea = m_pArea->m_id;
                            pGame->GetVisibleArea()->OnActivation();
                        }
                    }
                }
            } else {
                if (pGroup->GetCount() > 0) {
                    if (pGroup->m_groupChanged
                        || pGame->m_lastTarget != m_id
                        || m_typeAI.GetEnemyAlly() < CAIObjectType::EA_EVILCUTOFF) {
                        if (m_typeAI.GetEnemyAlly() < CAIObjectType::EA_EVILCUTOFF) {
                            if (pGroup->IsPartyLeader() && (~m_baseStats.m_flags & STATE_DISEASED) != 0) {
                                CMessage* message = new CMessageSetDialogWait(150,
                                    pGroup->GetGroupLeader(),
                                    m_id,
                                    m_id);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                                action2 = CAIAction(CAIAction::PLAYERDIALOG,
                                    targetType,
                                    0,
                                    0,
                                    0);
                                pGroup->GroupAction(action1, TRUE, &action2);
                            }
                        } else {
                            pGroup->m_groupChanged = FALSE;

                            // NOTE: Uninline.
                            pGame->SetLastTarget(m_id);

                            pGroup->GroupSetTarget(m_id);
                        }
                    } else if (pGame->field_38A6) {
                        if (m_typeAI.GetEnemyAlly() >= CAIObjectType::EA_EVILCUTOFF) {
                            pGroup->m_groupChanged = FALSE;

                            // NOTE: Uninline.
                            pGame->SetLastTarget(m_id);

                            pGroup->GroupSetTarget(m_id);
                        }
                        pGame->field_38A6 = FALSE;
                    }
                } else {
                    // NOTE: Uninline.
                    pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

                    PlaySound(9, TRUE, FALSE, FALSE);
                }
            }
            break;
        case 2:
            // NOTE: Uninline.
            pGame->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            switch (pGame->GetIconIndex()) {
            case 12:
                if (!m_bSelected || pGroup->GetCount() != 1) {
                    pGroup->GroupSetTarget(m_id);
                    pGame->SetState(0);
                    pGame->GetButtonArray()->SetSelectedButton(100);
                    pGame->GetButtonArray()->UpdateState();
                }
                break;
            case 18:
                if (pGroup->IsPartyLeader()) {
                    CMessage* message = new CMessageSetDialogWait(150,
                        pGroup->GetGroupLeader(),
                        m_id,
                        m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                    action2 = CAIAction(CAIAction::PLAYERDIALOG,
                        targetType,
                        0,
                        0,
                        0);
                    pGroup->GroupAction(action1, TRUE, &action2);

                    pGame->SetState(0);
                    pGame->GetButtonArray()->SetSelectedButton(100);
                    pGame->GetButtonArray()->UpdateState();
                }
                break;
            case 20:
            case -1:
                pGame->UseMagicOnObject(m_id);

                pGame->SetState(0);
                pGame->GetButtonArray()->SetSelectedButton(100);
                pGame->GetButtonArray()->UpdateState();
                break;
            case 36:
            case 40:
                if (!m_bSelected) {
                    SHORT nPortrait = g_pBaldurChitin->GetScreenWorld()->GetSelectedCharacter();
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
                        BYTE nPickPockets = pSprite->GetBaseStats()->m_skills[CGAMESPRITE_SKILL_PICK_POCKET];

                        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);

                        if (nPickPockets == 0) {
                            break;
                        }
                    }

                    action2 = CAIAction(CAIAction::PICKPOCKETS,
                        targetType,
                        0,
                        0,
                        0);
                    pGroup->GroupAction(action1, TRUE, &action2);

                    pGame->SetState(0);
                    pGame->GetButtonArray()->SetSelectedButton(100);
                    pGame->GetButtonArray()->UpdateState();
                }
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                // __LINE__: 5921
                UTIL_ASSERT(FALSE);
            }
            break;
        case 3:
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 5951
            UTIL_ASSERT(FALSE);
        }
    } else {
        CGameObject::OnActionButton(pt);
    }
}

// 0x700BB0
void CGameSprite::OnFormationButton(const CPoint& pt)
{
    if (m_canBeSeen > 0) {
        PlaySound(9, 1, 0, 1);
    } else {
        CGameObject::OnFormationButton(pt);
    }
}

// 0x700BE0
void CGameSprite::OnLightningStrike()
{
    CRect rView;
    rView.left = m_pArea->GetInfinity()->nNewX;
    rView.top = m_pArea->GetInfinity()->nNewY;
    rView.right = m_pArea->GetInfinity()->nNewX + m_pArea->GetInfinity()->rViewPort.Width();
    rView.bottom = m_pArea->GetInfinity()->nNewY + m_pArea->GetInfinity()->rViewPort.Height();

    BYTE nProb = 30;

    if (rView.PtInRect(m_pos)) {
        CItem* pItem = m_equipment.m_items[CGameSpriteEquipment::SLOT_ARMOR];
        if (pItem != NULL) {
            WORD animType = pItem->GetAnimationType();
            if (static_cast<BYTE>(animType) != 87) {
                switch (static_cast<BYTE>(animType >> 8)) {
                case 51:
                    nProb += 35;
                    break;
                case 52:
                    nProb += 40;
                    break;
                }
            }
        }

        if (rand() % 100 < nProb) {
            CAIAction strike(CAIAction::REALLYFORCESPELL, CPoint(-1, -1), 0, -1);
            strike.m_acteeID.Set(m_typeAI);
            strike.SetString1(CString("SpWi938"));

            CMessage* message = new CMessageAddAction(strike, m_pArea->m_nAIIndex, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }
}

// 0x700F20
void CGameSprite::MakeGlobal()
{
    if (!m_bGlobal) {
        m_bGlobal = TRUE;

        g_pBaldurChitin->GetObjectGame()->m_lstGlobalCreatures.AddTail(reinterpret_cast<int*>(m_id));

        CVariable v;
        v.SetName(CString(GetScriptName()));
        v.m_intValue = m_id;
        g_pBaldurChitin->GetObjectGame()->GetNamedCreatures()->AddKey(v);
    }
}

// 0x700FE0
SHORT CGameSprite::GetNumSounds(SHORT nOffset, SHORT nMaxNum)
{
    SHORT nSounds = 0;

    if (m_secondarySounds == "") {
        for (INT nIndex = 0; nIndex < nMaxNum; nIndex++) {
            if (m_baseStats.m_speech[nOffset + nIndex] == -1) {
                break;
            }

            nSounds++;
        }
    } else {
        nSounds = nMaxNum;
    }

    return nSounds;
}

// 0x7010A0
BOOL CGameSprite::CanSpeak(BOOL ignoreDeath, BOOL ignoreSilence)
{
    if (!m_active || !m_activeAI || !m_activeImprisonment) {
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene == TRUE) {
        return TRUE;
    }

    if (!ignoreSilence && (m_derivedStats.m_generalState & STATE_SILENCED) != 0) {
        return FALSE;
    }

    USHORT animationType = m_animation.GetAnimationId();

    if ((m_derivedStats.m_generalState & STATE_SLEEPING) != 0
        && ((animationType & 0xF000) != 0x4000 || animationType < 0x4400)) {
        return FALSE;
    }

    if (!ignoreDeath
        && ((m_derivedStats.m_generalState & STATE_DEAD) != 0
            || (m_derivedStats.m_generalState & STATE_STONE_DEATH) != 0
            || (m_derivedStats.m_generalState & STATE_FROZEN_DEATH) != 0)) {
        return FALSE;
    }

    if ((m_derivedStats.m_generalState & STATE_HELPLESS) != 0) {
        return FALSE;
    }

    if (m_derivedStats.m_spellStates.test(SPLSTATE_HELD)
        || m_derivedStats.m_spellStates.test(SPLSTATE_HOPELESSNESS)
        || m_derivedStats.m_spellStates.test(SPLSTATE_OTILUKES_RESILIENT_SPHERE)) {
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->m_nTimeStop != 0
        && g_pBaldurChitin->GetObjectGame()->m_nTimeStopCaster != m_id) {
        return FALSE;
    }

    if (!m_bAllowDialogInterrupt) {
        return FALSE;
    }

    if ((m_derivedStats.m_generalState & STATE_CHARMED) != 0) {
        return FALSE;
    }

    return TRUE;
}

// 0x7011E0
void CGameSprite::PlaySound(BYTE soundID, BOOL showText, BOOL showCircle, BOOL overrideOption)
{
    // TODO: Incomplete.
}

// 0x702900
void CGameSprite::VerbalConstant(LONG verbalConstant)
{
    STR_RES strRes;

    // NOTE: Uninline.
    BYTE channel = GetChannel();

    if (CanSpeak(verbalConstant >= 36 && verbalConstant < 38, FALSE)
        && verbalConstant >= 0 && verbalConstant < 64) {
        g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_speech[verbalConstant], strRes);
        COLORREF nameColor = CVidPalette::RANGE_COLORS[m_baseStats.m_colors[CVIDPALETTE_RANGE_MAIN_CLOTH]];
        if (m_secondarySounds != "") {
            CString sSoundName;
            CString sSoundSetName;
            LONG nNumber = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetCustomSound(sSoundName, static_cast<BYTE>(verbalConstant));
            m_secondarySounds.CopyToString(sSoundSetName);
            sSoundName.TrimLeft();
            sSoundName.TrimRight();
            sSoundName = sSoundSetName + sSoundName;

            strRes.cSound.SetResRef(CResRef(sSoundName), TRUE, TRUE);
        }

        strRes.szText.TrimLeft();

        BOOL show = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bSubtitles != FALSE;
        if (strRes.cSound.GetRes() == NULL) {
            show = TRUE;
        }

        if (m_typeAI.GetEnemyAlly() == CAIObjectType::EA_PC) {
            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                if (strRes.cSound.Play(FALSE)) {
                    m_talkingCounter = min(strRes.cSound.GetPlayTime() / 66, STANDARD_VERBAL_CONSTANT_LENGTH);
                }
            }

            CString sText;
            sText = strRes.szText;
            if (sText != "" && show) {
                g_pBaldurChitin->GetScreenWorld()->DisplayText(m_sName,
                    sText,
                    nameColor,
                    RGB(160, 200, 215),
                    -1,
                    FALSE);
            }
        } else {
            if (!strRes.cSound.GetLooping()) {
                strRes.cSound.SetFireForget(TRUE);
            }
            if (strRes.cSound.Play(FALSE)) {
                m_talkingCounter = min(strRes.cSound.GetPlayTime() / 66, STANDARD_VERBAL_CONSTANT_LENGTH);
            }

            CString sText;
            sText = strRes.szText;
            if (sText != "" && show) {
                g_pBaldurChitin->GetScreenWorld()->DisplayText(m_sName,
                    sText,
                    nameColor,
                    RGB(160, 200, 215),
                    -1,
                    FALSE);
            }
        }
    }
}

// 0x702E60
LONG CGameSprite::GetLength(LONG verbalConstant)
{
    STR_RES strRes;

    if (verbalConstant < 0 || verbalConstant >= 64) {
        return 0;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_speech[verbalConstant], strRes);

    if (m_secondarySounds != "") {
        CString sSoundName;
        CString sSoundSetName;
        LONG nNumber = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetCustomSound(sSoundName, static_cast<BYTE>(verbalConstant));
        m_secondarySounds.CopyToString(sSoundSetName);
        sSoundName.TrimLeft();
        sSoundName.TrimRight();
        sSoundName = sSoundSetName + sSoundName;

        if (nNumber == 0
            || g_pBaldurChitin->cDimm.m_cKeyTable.FindKey(CResRef(sSoundName), 4, TRUE) != NULL) {
            strRes.cSound.SetResRef(CResRef(sSoundName), TRUE, TRUE);
        }
    }

    // FIXME: Calls `GetPlayTime` twice.
    return min(strRes.cSound.GetPlayTime() / 33, 300);
}

// 0x703170
BOOL CGameSprite::IsSpellSchool(BYTE spellSchool)
{
    return ((GetSpecialization() & SPECMASK_WIZARD_ABJURER) != 0 && spellSchool == 1)
        || ((GetSpecialization() & SPECMASK_WIZARD_CONJURER) != 0 && spellSchool == 2)
        || ((GetSpecialization() & SPECMASK_WIZARD_DIVINER) != 0 && spellSchool == 3)
        || ((GetSpecialization() & SPECMASK_WIZARD_ENCHANTER) != 0 && spellSchool == 4)
        || ((GetSpecialization() & SPECMASK_WIZARD_ILLUSIONIST) != 0 && spellSchool == 5)
        || ((GetSpecialization() & SPECMASK_WIZARD_EVOKER) != 0 && spellSchool == 6)
        || ((GetSpecialization() & SPECMASK_WIZARD_NECROMANCER) != 0 && spellSchool == 7)
        || ((GetSpecialization() & SPECMASK_WIZARD_TRANSMUTER) != 0 && spellSchool == 8)
        || ((GetSpecialization() & SPECMASK_WIZARD_UNIVERSAL) != 0 && spellSchool == 9);
}

// 0x703270
void CGameSprite::SetFootstepChannel()
{
    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1) {
        for (int index = 0; index < 2; index++) {
            m_sndWalk[index].SetChannel(19, reinterpret_cast<DWORD>(m_pArea));
            m_sndArmor[index].SetChannel(20, reinterpret_cast<DWORD>(m_pArea));
        }
        m_sndReady.SetChannel(19, reinterpret_cast<DWORD>(m_pArea));
        m_sndDeath.SetChannel(15, reinterpret_cast<DWORD>(m_pArea));
    } else {
        for (int index = 0; index < 2; index++) {
            m_sndWalk[index].SetChannel(18, reinterpret_cast<DWORD>(m_pArea));
            m_sndArmor[index].SetChannel(20, reinterpret_cast<DWORD>(m_pArea));
        }
        m_sndReady.SetChannel(18, reinterpret_cast<DWORD>(m_pArea));
        m_sndDeath.SetChannel(15, reinterpret_cast<DWORD>(m_pArea));
    }
}

// 0x703330
BYTE CGameSprite::GetSound(BYTE soundID)
{
    // NOTE: Unused.
    STR_RES strRes;

    SHORT nNumSounds;
    BYTE nSound;

    switch (soundID) {
    case 2:
        nNumSounds = GetNumSounds(7, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 7;
        } else {
            nSound = 7;
        }
        break;
    case 3:
        nNumSounds = GetNumSounds(9, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 9;
        } else {
            nSound = 9;
        }
        break;
    case 4:
        nNumSounds = GetNumSounds(11, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 11;
        } else {
            nSound = 11;
        }
        break;
    case 5:
        nNumSounds = GetNumSounds(2, 5);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 2;
        } else {
            nSound = 2;
        }
        break;
    case 6:
        nNumSounds = GetNumSounds(33, 3);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 33;
        } else {
            nSound = 33;
        }
        break;
    case 7:
        nNumSounds = GetNumSounds(36, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 36;
        } else {
            nSound = 36;
        }
        break;
    case 8:
        nNumSounds = GetNumSounds(13, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 13;
        } else {
            nSound = 13;
        }
        break;
    case 9:
        if (field_710C > 8) {
            nSound = field_710E + 29;
            field_710E++;
            if (field_710E >= GetNumSounds(29, 4)) {
                field_710E = 0;
            }
            field_710A = 0;
            field_710C = 0;
        } else {
            nNumSounds = GetNumSounds(15, 7);
            if (nNumSounds > 0) {
                nSound = field_710A % nNumSounds;
            } else {
                nSound = 0;
            }
            field_710A++;
            field_710C++;
        }
        break;
    case 10:
        nNumSounds = GetNumSounds(15, 7);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 15;
        } else {
            nSound = 15;
        }
        break;
    case 11:
        nNumSounds = GetNumSounds(22, 7);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 22;
        } else {
            nSound = 22;
        }
        break;
    case 12:
        nNumSounds = GetNumSounds(29, 4);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 29;
        } else {
            nSound = 29;
        }
        break;
    case 13:
        nNumSounds = GetNumSounds(38, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 38;
        } else {
            nSound = 38;
        }
        break;
    case 14:
        nNumSounds = GetNumSounds(40, 4);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 40;
        } else {
            nSound = 40;
        }
        break;
    case 15:
        // FIXME: Not sure if it can overrun `m_speech` and treat `m_biography`
        // as sound?
        nNumSounds = GetNumSounds(44, 20);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 44;
        } else {
            nSound = 44;
        }
        break;
    case 16:
        nNumSounds = GetNumSounds(40, 2);
        if (nNumSounds != 0) {
            nSound = rand() % nNumSounds + 40;
        } else {
            nSound = 40;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 7658
        UTIL_ASSERT(FALSE);
    }

    return nSound;
}

// 0x703700
void CGameSprite::Render(CGameArea* pArea, CVidMode* pVidMode, INT nSurface)
{
    BOOLEAN bDithered = FALSE;
    BOOLEAN bFadeOut = FALSE;
    DWORD dwRenderFlags = 0x20000;
    CVisibilityMap* pVisibility;
    COLORREF rgbTint;
    COLORREF rgbGlobalTint;
    BYTE transparency;
    CSearchBitmap* pSearch;
    CRect rFX;
    CRect rGCBounds;
    CRect rViewRect;
    CPoint ptReference;
    CPoint newPos;
    POSITION bloodLstPos;
    CBlood* pBlood;
    SHORT nBloodDir;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 7687
    UTIL_ASSERT(pVidMode != NULL);

    if (m_active
        && m_activeAI
        && m_activeImprisonment
        && ((m_derivedStats.m_generalState & STATE_DEAD) != 0
            || m_canBeSeen > 0)
        && m_pArea == pArea
        && (m_typeAI.GetEnemyAlly() <= CAIObjectType::EA_CONTROLCUTOFF
            || (m_derivedStats.m_generalState & STATE_INVISIBLE) == 0)
        && m_baseStats.m_bStealthMode != 1
        && m_baseStats.m_animationType != 0) {
        pSearch = &(m_pArea->m_search);
        pVisibility = &(m_pArea->m_visibility);

        newPos.x = min(max(m_pos.x, 0), m_pArea->GetInfinity()->nAreaX - 1);
        newPos.y = min(max(m_pos.y, 0), m_pArea->GetInfinity()->nAreaY - 1);

        if (pVisibility->IsTileExplored(pVisibility->PointToTile(newPos))) {
            if (m_canBeSeen < VISIBLE_DELAY + 1
                || !pVisibility->IsTileVisible(pVisibility->PointToTile(newPos))) {
                bFadeOut = TRUE;
            }

            rViewRect.left = m_pArea->GetInfinity()->nCurrentX;
            rViewRect.top = m_pArea->GetInfinity()->nCurrentY;
            rViewRect.right = rViewRect.left + m_pArea->GetInfinity()->rViewPort.Width();
            rViewRect.bottom = rViewRect.top + m_pArea->GetInfinity()->rViewPort.Height();

            if (!m_lstBlood.IsEmpty()) {
                bloodLstPos = m_lstBlood.GetHeadPosition();
                while (bloodLstPos != NULL) {
                    pBlood = m_lstBlood.GetNext(bloodLstPos);
                    nBloodDir = pBlood->GetDirection();
                    if (pBlood->BloodLeft()
                        && nBloodDir >= 4
                        && nBloodDir <= 12) {
                        pBlood->Render(pVidMode, nSurface);
                    }
                }
            }

            // NOTE: Uninline.
            m_animation.CalculateFxRect(rFX, ptReference, m_posZ);

            if (m_id == m_pArea->m_iPicked
                || g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bAlwaysDither
                || g_pBaldurChitin->GetObjectGame()->m_bForceDither) {
                bDithered = TRUE;
            }

            rgbTint = m_pArea->GetTintColor(CPoint(m_pos.x, m_pos.y + m_posZ),
                m_listType);

            if (m_pArea->m_nInfravision
                && m_animation.DetectedByInfravision()
                && Animate()) {
                if ((m_pArea->GetInfinity()->m_areaType & 0x2) != 0
                    && ((m_pArea->GetInfinity()->m_areaType & 0x40) == 0
                        || (m_pArea->GetInfinity()->m_renderDayNightCode & 0x2) == 0)) {
                    rgbGlobalTint = g_pBaldurChitin->GetCurrentVideoMode()->GetGlobalTintColor();
                } else {
                    rgbGlobalTint = RGB(255, 255, 255);
                }

                if (GetRValue(rgbTint) * GetRValue(rgbGlobalTint) / 255 > GetRValue(CInfinity::RGB_NIGHT_COLOR)
                    || GetGValue(rgbTint) * GetGValue(rgbGlobalTint) / 255 > GetGValue(CInfinity::RGB_NIGHT_COLOR)
                    || GetBValue(rgbTint) * GetBValue(rgbGlobalTint) / 255 > GetBValue(CInfinity::RGB_NIGHT_COLOR)) {
                    if ((m_pArea->GetInfinity()->m_areaType & 0x2) != 0
                        && ((m_pArea->GetInfinity()->m_areaType & 0x40) == 0
                            || (m_pArea->GetInfinity()->m_renderDayNightCode & 0x2) == 0)) {
                        dwRenderFlags |= 0x10000;
                    }
                } else {
                    rgbTint = RGB(200, 200, 200);
                }
            } else {
                if ((m_pArea->GetInfinity()->m_areaType & 0x2) != 0
                    && ((m_pArea->GetInfinity()->m_areaType & 0x40) == 0
                        || (m_pArea->GetInfinity()->m_renderDayNightCode & 0x2) == 0)) {
                    dwRenderFlags |= 0x10000;
                }
            }

            transparency = 0;

            if (m_derivedStats.m_visualEffects[IWD_VFX_BLUR]) {
                transparency = 180;
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_INVISIBILITY]) {
                transparency = static_cast<BYTE>(abs(g_pBaldurChitin->nAUCounter % 80 - 40)) + 140;
            }

            // NOTE: Unsigned compare.
            if (m_derivedStats.m_nTranslucent > 0 && transparency < static_cast<BYTE>(m_derivedStats.m_nTranslucent)) {
                transparency = static_cast<BYTE>(m_derivedStats.m_nTranslucent);
            }

            if (m_derivedStats.m_nMirrorImages > 0) {
                if (transparency < 96) {
                    transparency = 96;
                }
            }

            if (m_baseStats.field_2F9 > 0 && transparency < m_baseStats.field_2F9) {
                transparency = m_baseStats.field_2F9;
            }

            if (m_derivedStats.m_nMirrorImages > 0) {
                if (m_derivedStats.m_nMirrorImages > 2) {
                    RenderMirrorImage(2,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }

                if (m_derivedStats.m_nMirrorImages > 4) {
                    RenderMirrorImage(4,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }

                if (m_derivedStats.m_nMirrorImages > 6) {
                    RenderMirrorImage(6,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }

                if (m_derivedStats.m_nMirrorImages > 0) {
                    RenderMirrorImage(0,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }

                if (m_derivedStats.m_nMirrorImages > 1) {
                    RenderMirrorImage(1,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }
            }

            IcewindCVisualEffect vfx;
            vfx.SetCopyFromBack(TRUE);

            if (m_derivedStats.m_visualEffects[IWD_VFX_GREASE]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_GREASE]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SHIELD_OF_LATHANDER_2]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SHIELD_OF_LATHANDER_2]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_GREATER_SHIELD_OF_LATHANDER_2]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_GREATER_SHIELD_OF_LATHANDER_2]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SEVEN_EYES_2]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SEVEN_EYES_2]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_28]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_28]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_29]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_29]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_BLUR]
                && m_pos != m_posOld) {
                LONG dx = (m_pos.x - m_posOld.x) / 2;
                LONG dy = (m_pos.y - m_posOld.y) / 2;
                for (int cnt = 0; cnt < 4; cnt++) {
                    m_animation.CalculateGCBoundsRect(rGCBounds,
                        newPos,
                        ptReference,
                        m_posZ,
                        rFX.Width(),
                        rFX.Height());

                    if (!IsRectEmpty(rViewRect & rGCBounds)) {
                        m_animation.Render(m_pArea->GetInfinity(),
                            pVidMode,
                            nSurface,
                            rFX,
                            newPos,
                            ptReference,
                            dwRenderFlags,
                            rgbTint,
                            rGCBounds,
                            bDithered,
                            bFadeOut,
                            m_posZ,
                            transparency);
                    }

                    newPos.x -= dx;
                    newPos.y -= dy;
                }
            } else {
                m_animation.CalculateGCBoundsRect(rGCBounds,
                    newPos,
                    ptReference,
                    m_posZ,
                    rFX.Width(),
                    rFX.Height());

                if (!IsRectEmpty(rViewRect & rGCBounds)) {
                    m_animation.Render(m_pArea->GetInfinity(),
                        pVidMode,
                        nSurface,
                        rFX,
                        newPos,
                        ptReference,
                        dwRenderFlags,
                        rgbTint,
                        rGCBounds,
                        bDithered,
                        bFadeOut,
                        m_posZ,
                        transparency);
                }
            }

            if (m_derivedStats.m_nMirrorImages > 0) {
                if (m_derivedStats.m_nMirrorImages > 7) {
                    RenderMirrorImage(7,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }

                if (m_derivedStats.m_nMirrorImages > 5) {
                    RenderMirrorImage(5,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }

                if (m_derivedStats.m_nMirrorImages > 3) {
                    RenderMirrorImage(3,
                        rFX,
                        rGCBounds,
                        rViewRect,
                        ptReference,
                        pSearch,
                        pVisibility,
                        pVidMode,
                        nSurface,
                        rgbTint,
                        bDithered,
                        bFadeOut,
                        dwRenderFlags);
                }
            }

            RenderSpriteEffect(pVidMode, nSurface);

            if (m_derivedStats.m_visualEffects[IWD_VFX_ENTANGLE]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_ENTANGLE]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_WEB]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_WEB]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SHROUD_OF_FLAME]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SHROUD_OF_FLAME]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_ANTIMAGIC_MISSILE]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_ANTIMAGIC_MISSILE]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_OTILUKES_RESILIENT_SPHERE]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_OTILUKES_RESILIENT_SPHERE]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_PROTECTION_FROM_MISSILES]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_PROTECTION_FROM_MISSILES]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_CLOAK_OF_FEAR]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_CLOAK_OF_FEAR]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_ENTROPY_SHIELD]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_ENTROPY_SHIELD]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_FIRE_AURA]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_FIRE_AURA]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_FROST_AURA]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_FROST_AURA]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SANCTUARY]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SANCTUARY]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_GLOBE_OF_INVULNERABILITY]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_GLOBE_OF_INVULNERABILITY]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_INSECT_PLAGUE]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_INSECT_PLAGUE]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_STORM_SHELL]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_STORM_SHELL]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SHIELD_OF_LATHANDER]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SHIELD_OF_LATHANDER]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_GREATER_SHIELD_OF_LATHANDER]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_GREATER_SHIELD_OF_LATHANDER]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SEVEN_EYES]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SEVEN_EYES]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_MINOR_GLOBE_OF_INVULNERABILITY]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_MINOR_GLOBE_OF_INVULNERABILITY]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_SHIELD]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_SHIELD]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_FIRE_SHIELD_RED]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_FIRE_SHIELD_RED]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_FIRE_SHIELD_BLUE]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_FIRE_SHIELD_BLUE]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_TORTOISE_SHELL]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_TORTOISE_SHELL]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_DEATH_ARMOR]) {
                vfx.SetCopyFromBack(FALSE);
                vfx.SetTransparency(TRUE, 192);
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_DEATH_ARMOR]),
                    vfx);
            }

            if (m_derivedStats.m_visualEffects[IWD_VFX_WISP]) {
                RenderSpriteCover(pVidMode,
                    nSurface,
                    &(m_aVfxCells[IWD_VFX_WISP]),
                    vfx);
            }

            if (!m_lstBlood.IsEmpty()) {
                bloodLstPos = m_lstBlood.GetHeadPosition();
                while (bloodLstPos != NULL) {
                    CBlood* blood = m_lstBlood.GetNext(bloodLstPos);
                    pBlood = m_lstBlood.GetNext(bloodLstPos);
                    nBloodDir = pBlood->GetDirection();
                    if (pBlood->BloodLeft()
                        && (nBloodDir < 4 || nBloodDir > 12)) {
                        pBlood->Render(pVidMode, nSurface);
                    }
                }
            }
        }
    }
}

// 0x704770
void CGameSprite::RenderMarkers(CVidMode* pVidMode, INT nSurface)
{
    CPoint pt;
    pt.x = min(max(m_pos.x, 0), m_pArea->GetInfinity()->nAreaX - 1);
    pt.y = min(max(m_pos.y, 0), m_pArea->GetInfinity()->nAreaY - 1);

    if (m_pArea->m_visibility.IsTileExplored(m_pArea->m_visibility.PointToTile(pt))
        && (m_typeAI.m_nEnemyAlly <= CAIObjectType::EA_CONTROLCUTOFF
            || (m_derivedStats.m_generalState & STATE_INVISIBLE) == 0
            || (m_baseStats.m_critSectService & 0x1) != 0)
        && m_baseStats.m_bStealthMode != 1) {
        DWORD level = g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nGuiFeedbackLevel;
        if (g_pBaldurChitin->GetScreenWorld()->field_14A) {
            level = 5;
        }
        if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene) {
            level = 1;
        }

        BOOLEAN dialogTalker = FALSE;
        if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 386
            || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 1282) {
            CRect rViewPort(m_pArea->GetInfinity()->rViewPort);

            INT x;
            INT y;
            m_pArea->GetInfinity()->GetViewPosition(x, y);

            if (abs(GetPos().x - (x + rViewPort.Width() / 2)) < 16
                && abs(GetPos().y - (y + rViewPort.Height() / 2)) < 12) {
                dialogTalker = TRUE;
            }
        }

        switch (level) {
        case 1:
            if (m_talkingCounter > 0
                || dialogTalker
                || m_id == m_pArea->m_iPicked
                || m_id == m_pArea->m_iPickedTarget
                || m_marker.m_nRecticleForceRenderTarget != 0) {
                m_marker.Render(pVidMode, nSurface, this);
            }

            if (m_id == m_pArea->m_iPicked
                || m_id == m_pArea->m_iPickedTarget
                || m_marker.m_nRecticleForceRender != 0
                || m_groupMove) {
                if (Orderable(FALSE) && m_targetPoint.x != -1) {
                    m_destMarker.Render(pVidMode,
                        nSurface,
                        m_pArea->GetInfinity(),
                        m_targetPoint,
                        CMarker::RECTICLE_DEST_SIZE,
                        3 * CMarker::RECTICLE_DEST_SIZE / 4);
                }
            }
            break;
        case 2:
            if (m_bSelected
                || m_id == m_pArea->m_iPicked
                || m_id == m_pArea->m_iPickedTarget
                || m_marker.m_nRecticleForceRender != 0) {
                m_marker.Render(pVidMode, nSurface, this);

                if (Orderable(FALSE) && m_targetPoint.x != -1) {
                    m_destMarker.Render(pVidMode,
                        nSurface,
                        m_pArea->GetInfinity(),
                        m_targetPoint,
                        CMarker::RECTICLE_DEST_SIZE,
                        3 * CMarker::RECTICLE_DEST_SIZE / 4);
                }
            } else if (m_talkingCounter > 0) {
                m_marker.Render(pVidMode, nSurface, this);
            }
            break;
        case 3:
            if (Orderable(FALSE)
                || m_id == m_pArea->m_iPicked
                || m_id == m_pArea->m_iPickedTarget
                || m_marker.m_nRecticleForceRender != 0) {
                m_marker.Render(pVidMode, nSurface, this);

                if (Orderable(FALSE)
                    && (m_bSelected
                        || m_id == m_pArea->m_iPicked
                        || m_id == m_pArea->m_iPickedTarget)
                    && m_targetPoint.x != -1) {
                    m_destMarker.Render(pVidMode,
                        nSurface,
                        m_pArea->GetInfinity(),
                        m_targetPoint,
                        CMarker::RECTICLE_DEST_SIZE,
                        3 * CMarker::RECTICLE_DEST_SIZE / 4);
                }
            } else if (m_talkingCounter > 0) {
                m_marker.Render(pVidMode, nSurface, this);
            }
            break;
        case 4:
            if (Orderable(FALSE)
                || m_marker.m_type == CMarker::RECTICLE
                || m_id == m_pArea->m_iPicked
                || m_id == m_pArea->m_iPickedTarget) {
                m_marker.Render(pVidMode, nSurface, this);

                if (Orderable(FALSE)
                    && (m_bSelected
                        || m_id == m_pArea->m_iPicked
                        || m_id == m_pArea->m_iPickedTarget)
                    && m_targetPoint.x != -1) {
                    m_destMarker.Render(pVidMode,
                        nSurface,
                        m_pArea->GetInfinity(),
                        m_targetPoint,
                        CMarker::RECTICLE_DEST_SIZE,
                        3 * CMarker::RECTICLE_DEST_SIZE / 4);
                }
            } else if (m_talkingCounter > 0) {
                m_marker.Render(pVidMode, nSurface, this);
            }
            break;
        case 5:
            m_marker.Render(pVidMode, nSurface, this);

            if (Orderable(FALSE)
                && (m_bSelected
                    || m_id == m_pArea->m_iPicked
                    || m_id == m_pArea->m_iPickedTarget)
                && m_targetPoint.x != -1) {
                m_destMarker.Render(pVidMode,
                    nSurface,
                    m_pArea->GetInfinity(),
                    m_targetPoint,
                    CMarker::RECTICLE_DEST_SIZE,
                    3 * CMarker::RECTICLE_DEST_SIZE / 4);
            }
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 8300
            UTIL_ASSERT(FALSE);
        }
    }
}

// 0x704D40
void CGameSprite::RenderPortrait(const CPoint& cpRenderPosition, const CSize& szControl, BOOL bPressed, BOOL reorderHighlight, BOOL selectFromMarker, const CRect& rClip, BOOL bDoubleSize)
{
    BOOL bDead = FALSE;
    BOOLEAN bInControl = InControl();
    BOOL bSequenceMode = g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode;

    if (bSequenceMode
        && !bInControl) {
        return;
    }

    m_vbPortraitSmall.SetResRef(CResRef(m_baseStats.m_portraitSmall), TRUE, FALSE);
    m_vbPortraitSmall.m_bDoubleSize = bDoubleSize;

    if (CResRef(m_baseStats.m_portraitSmall) == "") {
        m_vbPortraitSmall.SetResRef(CResRef(CInfGame::SILHOUETTE_PORTRAIT_SM), TRUE, FALSE);
        m_vbPortraitSmall.m_bDoubleSize = bDoubleSize;
    }

    if (CResRef(m_baseStats.m_portraitSmall) != "" && m_vbPortraitSmall.GetRes() == NULL) {
        m_vbPortraitSmall.SetResRef(CResRef(CInfGame::SILHOUETTE_PORTRAIT_SM), TRUE, FALSE);
        m_vbPortraitSmall.m_bDoubleSize = bDoubleSize;
    }

    if (m_vbPortraitSmall.GetBitCount(FALSE) != 24
        && m_vbPortraitSmall.GetBitCount(FALSE) != 8) {
        m_vbPortraitSmall.SetResRef(CResRef(CInfGame::SILHOUETTE_PORTRAIT_SM), TRUE, TRUE);
        m_vbPortraitSmall.m_bDoubleSize = bDoubleSize;
    }

    INT nScale = bDoubleSize ? 2 : 1;

    CRect r1(cpRenderPosition.x,
        cpRenderPosition.y,
        cpRenderPosition.x + 44 * nScale,
        cpRenderPosition.y + 44 * nScale);

    INT v1;
    if (m_derivedStats.m_spellStates[SPLSTATE_SUPPRESS_HP_INFO]) {
        v1 = 42 * nScale;
    } else {
        v1 = 42 * nScale * (m_derivedStats.m_nMaxHitPoints - m_baseStats.m_hitPoints) / m_derivedStats.m_nMaxHitPoints;
    }

    if (m_baseStats.m_hitPoints <= 0) {
        bDead = TRUE;
    }

    COLORREF rgbColor;
    if (selectFromMarker) {
        if ((m_pArea != NULL && m_id == m_pArea->m_iPicked)
            || m_talkingCounter > 0) {
            rgbColor = m_marker.m_rgbColor;
        } else {
            rgbColor = CMarker::PC_SELECTED_COLOR;
            if (!m_bSelected) {
                rgbColor = RGB(0, 0, 0);
            }
        }
    } else {
        if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineMap) {
            rgbColor = GetMapScreenColor();
        } else {
            if (InControl() && m_nUnselectableCounter == 0) {
                rgbColor = CMarker::PC_SELECTED_COLOR;
            } else {
                rgbColor = CMarker::PC_NONECONTROLED_SELECTED_COLOR;
            }
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter()) {
                rgbColor = RGB(0, 0, 0);
            }
        }
    }

    INT v2 = 42 * nScale;

    CRect r2(cpRenderPosition.x + 2 * nScale,
        cpRenderPosition.y + 2 * nScale,
        cpRenderPosition.x + 2 * nScale + 42 * nScale,
        cpRenderPosition.y + 2 * nScale + 42 * nScale);
    CRect r3(r2);

    if (bDead) {
        m_vbPortraitSmall.SetTintColor(RGB(180, 180, 180));
        m_vbPortraitSmall.RenderDirect(CVIDINF_SURFACE_BACK,
            r2.left,
            r2.top,
            r2 & rClip,
            0xA0000,
            FALSE);
    } else {
        if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nOldPortraitHealth == TRUE) {
            if (m_bBloodFlashOn) {
                m_vbPortraitSmall.SetTintColor(RGB(120 + static_cast<BYTE>(m_nBloodFlashAmount), 30, 30));
                m_vbPortraitSmall.RenderDirect(CVIDINF_SURFACE_BACK,
                    r2.left,
                    r2.top,
                    r2 & rClip,
                    0x20000,
                    FALSE);
            } else {
                if (field_53E6) {
                    m_vbPortraitSmall.SetTintColor(RGB(50, 255, 50));
                } else {
                    m_vbPortraitSmall.SetTintColor(RGB(120, 30, 30));
                }
                m_vbPortraitSmall.RenderDirect(CVIDINF_SURFACE_BACK,
                    r2.left,
                    r2.top,
                    r2 & rClip,
                    0x20000,
                    FALSE);
            }
        }
    }

    if (!GRAVITY_IS_DOWN
        && g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nOldPortraitHealth == TRUE) {
        r2.bottom = r2.top + v2 - v1;
    }

    if (!bDead) {
        m_vbPortraitSmall.SetTintColor(RGB(255, 255, 255));

        if (GRAVITY_IS_DOWN) {
            CRect r3(rClip);

            if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nOldPortraitHealth == TRUE) {
                r3.top = r2.bottom + v1 - v2;
            }

            m_vbPortraitSmall.RenderDirect(CVIDINF_SURFACE_BACK,
                r2.left,
                r2.top,
                r2 & r3,
                0x20000,
                FALSE);
        } else {
            m_vbPortraitSmall.RenderDirect(CVIDINF_SURFACE_BACK,
                r2.left,
                r2.top,
                r2 & r2,
                0x20000,
                FALSE);
        }
    }

    CRect rLineClip(rClip);
    if (rgbColor != 0) {
        r1.right++;
        rLineClip.right++;

        g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.left,
            r1.top,
            r1.left,
            r1.bottom,
            CVIDINF_SURFACE_BACK,
            rLineClip & r1,
            rgbColor);

        if (rLineClip.bottom != r1.bottom) {
            g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.left,
                r1.bottom,
                r1.right,
                r1.bottom,
                CVIDINF_SURFACE_BACK,
                rLineClip & r1,
                rgbColor);
        }

        if (rLineClip.right != r1.right) {
            g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.right,
                r1.bottom,
                r1.right,
                r1.top,
                CVIDINF_SURFACE_BACK,
                rLineClip & r1,
                rgbColor);
        }

        g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.right,
            r1.top,
            r1.left,
            r1.top,
            CVIDINF_SURFACE_BACK,
            rLineClip & r1,
            rgbColor);
    }

    if (reorderHighlight) {
        g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.left,
            r1.top,
            r1.left,
            r1.bottom,
            CVIDINF_SURFACE_BACK,
            r1 & rLineClip,
            RGB(255, 255, 0));

        g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.left,
            r1.bottom,
            r1.right,
            r1.bottom,
            CVIDINF_SURFACE_BACK,
            r1 & rLineClip,
            RGB(255, 255, 0));

        g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.right,
            r1.bottom,
            r1.right,
            r1.top,
            CVIDINF_SURFACE_BACK,
            r1 & rLineClip,
            RGB(255, 255, 0));

        g_pBaldurChitin->GetCurrentVideoMode()->DrawLine(r1.right,
            r1.top,
            r1.left,
            r1.top,
            CVIDINF_SURFACE_BACK,
            r1 & rLineClip,
            RGB(255, 255, 0));
    }

    CPoint ptIcon(r3.left, r3.bottom);
    POSITION pos = m_portraitIcons.GetHeadPosition();
    while (pos != NULL) {
        SHORT nIcon = static_cast<SHORT>(reinterpret_cast<INT>(m_portraitIcons.GetNext(pos)));
        if (nIcon < m_portraitIconVidCell.GetNumberSequences(FALSE)) {
            m_portraitIconVidCell.SequenceSet(nIcon + 65);
            m_portraitIconVidCell.FrameSet(0);
            m_portraitIconVidCell.Render(CVIDINF_SURFACE_BACK,
                ptIcon.x,
                ptIcon.y,
                r3 & rClip,
                NULL,
                0,
                0,
                -1);
            ptIcon.x += PORTRAIT_ICON_SIZE.x * nScale;
            if (ptIcon.x > r3.right - PORTRAIT_ICON_SIZE.x * nScale) {
                ptIcon.x = r3.left;
                ptIcon.y -= PORTRAIT_ICON_SIZE.y * nScale;
            }
        }
    }

    CVidFont font;
    if (!bDead) {
        if (SHOW_CHARACTER_HP) {
            font.SetResRef(CResRef("NUMFONT"), bDoubleSize, TRUE);
            font.SequenceSet(0);
            if (font.GetRes()->Demand() != NULL) {
                // 0x85BD4C
                static const INT HEALTH_VALUE[4] = {
                    100,
                    75,
                    50,
                    25,
                };

                // 0x85BD5C
                static const COLORREF HEALTH_COLOR[4] = {
                    RGB(255, 255, 255),
                    RGB(0, 250, 0),
                    RGB(255, 255, 0),
                    RGB(255, 128, 0),
                };

                CString sHealth;
                INT iHealthPercent = 100 * m_baseStats.m_hitPoints / m_derivedStats.m_nMaxHitPoints;
                COLORREF rgbColor = RGB(255, 0, 0);

                for (int index = 0; index < 4; index++) {
                    if (iHealthPercent >= HEALTH_VALUE[index]) {
                        rgbColor = HEALTH_COLOR[index];
                        break;
                    }
                }

                if (m_derivedStats.m_spellStates[SPLSTATE_SUPPRESS_HP_INFO]) {
                    sHealth.Format("%c/%c", '-', '-');
                    rgbColor = RGB(255, 0, 0);
                } else {
                    sHealth.Format("%d/%d", m_baseStats.m_hitPoints, m_derivedStats.m_nMaxHitPoints);
                }

                if (m_baseStats.m_hitPoints == 0) {
                    rgbColor = RGB(128, 128, 128);
                }

                font.SetColor(rgbColor, RGB(0, 0, 0), TRUE);
                font.TextOut(sHealth,
                    r3.left,
                    cpRenderPosition.y + font.GetFontHeight(FALSE),
                    rClip,
                    CVIDINF_SURFACE_BACK);
                font.GetRes()->Release();
            }
        }
    }

    if (m_bLevelUp) {
        m_portraitIconVidCell.SequenceSet(254);
        m_portraitIconVidCell.FrameSet(0);
        m_portraitIconVidCell.Render(CVIDINF_SURFACE_BACK,
            r3.right - PORTRAIT_ICON_SIZE.x * nScale,
            r3.top + PORTRAIT_ICON_SIZE.y * nScale,
            r3 & rClip,
            NULL,
            0,
            0,
            -1);
    }
}

// 0x705AD0
void CGameSprite::RenderToMapScreen(const CRect& rClipBase, const CPoint& ptCharPos)
{
    CRect rClip(rClipBase);
    INT nScale = g_pBaldurChitin->m_bUseNewGui ? 2 : 1;
    if (!g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode
        || InControl()) {
        if (!IcewindMisc::IsDead(this)) {
            CPoint center;
            CSize axes;
            COLORREF rgbColor;

            center.x = nScale * (4 * ptCharPos.x / 32) - rClip.left;
            center.y = nScale * (4 * ptCharPos.y / 32) - rClip.top;
            rClip.OffsetRect(-rClip.left, -rClip.top);

            axes.cx = GetAnimation()->GetPersonalSpace();
            if (!g_pBaldurChitin->m_bUseNewGui) {
                axes.cx = max(axes.cx - 2, 3);
            }
            axes.cy = 3 * GetAnimation()->GetPersonalSpace() / 4;

            rgbColor = g_pBaldurChitin->GetCurrentVideoMode()->ApplyBrightnessContrast(GetMapScreenColor());

            static_cast<CVidInf*>(g_pBaldurChitin->GetCurrentVideoMode())->BKRenderEllipse(center, axes, rClip, rgbColor);
        }
    }
}

// 0x705CC0
COLORREF CGameSprite::GetMapScreenColor()
{
    COLORREF rgbColor;
    BYTE enemyAlly = m_liveTypeAI.GetEnemyAlly();
    if (enemyAlly == CAIObjectType::EA_PC
        || enemyAlly == CAIObjectType::EA_FAMILIAR
        || enemyAlly == CAIObjectType::EA_ALLY
        || enemyAlly == CAIObjectType::EA_0x847C3A
        || enemyAlly == CAIObjectType::EA_CONTROLLED
        || enemyAlly == CAIObjectType::EA_GOODCUTOFF) {
        if (m_moraleFailure) {
            rgbColor = CMarker::PC_MORALE_FAILURE_COLOR;
        } else if (InControl() && m_nUnselectableCounter == 0) {
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter()) {
                rgbColor = CMarker::PC_SELECTED_COLOR;
            } else {
                rgbColor = CMarker::PC_COLOR;
            }
        } else {
            if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter()) {
                rgbColor = CMarker::PC_NONECONTROLED_SELECTED_COLOR;
            } else {
                rgbColor = CMarker::PC_NONECONTROLED_COLOR;
            }
        }
    } else if (enemyAlly == CAIObjectType::EA_ENEMY
        || enemyAlly == CAIObjectType::EA_CHARMED_PC) {
        rgbColor = CMarker::ENEMY_COLOR;
    } else if (enemyAlly == CAIObjectType::EA_NEUTRAL) {
        rgbColor = CMarker::NEUTRAL_COLOR;
    } else {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 8754
        UTIL_ASSERT(FALSE);
    }

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == g_pBaldurChitin->GetActiveEngine()->GetPickedCharacter()) {
        ULONG nQuarterPeriod = CMarker::PICKED_FLASH_PERIOD / 4;
        ULONG nAsyncCounter = g_pBaldurChitin->nAUCounter % CMarker::PICKED_FLASH_PERIOD;
        BYTE red;
        BYTE green;
        BYTE blue;

        if (nAsyncCounter < nQuarterPeriod) {
            red = static_cast<BYTE>(nAsyncCounter * GetRValue(rgbColor) / nQuarterPeriod);
            green = static_cast<BYTE>(nAsyncCounter * GetGValue(rgbColor) / nQuarterPeriod);
            blue = static_cast<BYTE>(nAsyncCounter * GetBValue(rgbColor) / nQuarterPeriod);
        } else if (nAsyncCounter < nQuarterPeriod * 2) {
            nAsyncCounter -= nQuarterPeriod;
            red = static_cast<BYTE>(GetRValue(rgbColor) + nAsyncCounter * (255 - GetRValue(rgbColor)) / nQuarterPeriod);
            green = static_cast<BYTE>(GetGValue(rgbColor) + nAsyncCounter * (255 - GetGValue(rgbColor)) / nQuarterPeriod);
            blue = static_cast<BYTE>(GetBValue(rgbColor) + nAsyncCounter * (255 - GetBValue(rgbColor)) / nQuarterPeriod);
        } else if (nAsyncCounter < nQuarterPeriod * 3) {
            nAsyncCounter -= nQuarterPeriod * 2;
            red = static_cast<BYTE>(255 - nAsyncCounter * (255 - GetRValue(rgbColor)) / nQuarterPeriod);
            green = static_cast<BYTE>(255 - nAsyncCounter * (255 - GetGValue(rgbColor)) / nQuarterPeriod);
            blue = static_cast<BYTE>(255 - nAsyncCounter * (255 - GetBValue(rgbColor)) / nQuarterPeriod);
        } else {
            nAsyncCounter -= nQuarterPeriod * 3;
            red = static_cast<BYTE>(GetRValue(rgbColor) - nAsyncCounter * GetRValue(rgbColor) / nQuarterPeriod);
            green = static_cast<BYTE>(GetGValue(rgbColor) - nAsyncCounter * GetGValue(rgbColor) / nQuarterPeriod);
            blue = static_cast<BYTE>(GetBValue(rgbColor) - nAsyncCounter * GetBValue(rgbColor) / nQuarterPeriod);
        }

        rgbColor = RGB(red, green, blue);
    }

    return rgbColor;
}

// 0x705FD0
void CGameSprite::Select()
{
    m_bSelected = TRUE;
    m_firstActionSound = TRUE;
    m_pArea->m_pGame->GetGroup()->Add(this);

    if (m_targetId != CGameObjectArray::INVALID_INDEX && Orderable(FALSE)) {
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
                static_cast<CGameSprite*>(pObject)->m_marker.SetType(CMarker::RECTICLE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x7060C0
void CGameSprite::Unselect()
{
    CGameSprite* pSprite = NULL;

    m_bSelected = FALSE;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (pSprite != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetGroup()->Remove(pSprite);
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        if (m_targetId != CGameObjectArray::INVALID_INDEX && Orderable(FALSE)) {
            CGameObject* pObject;

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pObject,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if (pObject->GetObjectType() == CGameObject::TYPE_SPRITE) {
                    static_cast<CGameSprite*>(pObject)->m_marker.SetType(CMarker::ELLIPSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
    }
}

// 0x706220
void CGameSprite::SetCursor(LONG nToolTip)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (m_canBeSeen > 0) {
        switch (pGame->GetState()) {
        case 0:
            // NOTE: Unsigned compare.
            if (static_cast<DWORD>(nToolTip) < static_cast<DWORD>(pGame->GetOptions()->m_nTooltips)) {
                if (pGame->GetGroup()->GetCount() != 0) {
                    if ((pGame->GetCharacterPortraitNum(m_id) != -1
                            || pGame->IsAlly(m_id)
                            || pGame->IsFamiliar(m_id)
                            || m_typeAI.GetEnemyAlly() <= CAIObjectType::EA_CONTROLCUTOFF)
                        && (!g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode || InControl())) {
                        if (InControl()) {
                            g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);
                        } else {
                            g_pBaldurChitin->GetObjectCursor()->SetCursor(46, FALSE);
                        }
                    } else {
                        if (m_typeAI.GetEnemyAlly() < CAIObjectType::EA_EVILCUTOFF) {
                            if (pGame->GetGroup()->IsPartyLeader()) {
                                if ((m_baseStats.m_flags & 0x80000) != 0) {
                                    g_pBaldurChitin->GetObjectCursor()->SetCursor(6, FALSE);
                                } else {
                                    g_pBaldurChitin->GetObjectCursor()->SetCursor(18, FALSE);
                                }
                            } else {
                                g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);
                            }
                        } else {
                            g_pBaldurChitin->GetObjectCursor()->SetCursor(12, FALSE);
                        }
                    }
                } else {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);
                }
            } else {
                SetCharacterToolTip(NULL);
            }
            break;
        case 1:
            if (pGame->GetIconIndex() != -1) {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(pGame->GetIconIndex(), FALSE);
            } else {
                g_pBaldurChitin->GetObjectCursor()->SetCursor(20, FALSE);
            }
            break;
        case 2:
            switch (pGame->GetIconIndex()) {
            case 12:
                g_pBaldurChitin->GetObjectCursor()->SetCursor(12, FALSE);
                if (m_bSelected && pGame->GetGroup()->GetCount() == 1) {
                    g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                }
                break;
            case 18:
                g_pBaldurChitin->GetObjectCursor()->SetCursor(18, FALSE);
                if (!pGame->GetGroup()->IsPartyLeader()) {
                    g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                }
                break;
            case 36:
            case 40:
                if (1) {
                    g_pBaldurChitin->GetObjectCursor()->SetCursor(40, FALSE);
                    if (m_bSelected) {
                        g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                    }

                    SHORT nPortrait = g_pBaldurChitin->GetScreenWorld()->GetSelectedCharacter();
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
                        if (pSprite->m_baseStats.m_skills[CGAMESPRITE_SKILL_PICK_POCKET] == 0) {
                            g_pBaldurChitin->GetObjectCursor()->SetGreyScale(TRUE);
                            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                                CGameObjectArray::THREAD_ASYNCH,
                                INFINITE);
                        }
                    }
                }
                break;
            case 255:
                g_pBaldurChitin->GetObjectCursor()->SetCursor(20, FALSE);
                break;
            default:
                g_pBaldurChitin->GetObjectCursor()->SetCursor(pGame->GetIconIndex(), FALSE);
                break;
            }
            break;
        case 3:
            g_pBaldurChitin->GetObjectCursor()->SetCursor(6, FALSE);
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 9053
            UTIL_ASSERT(FALSE);
        }
    } else {
        CGameObject::SetCursor(nToolTip);
    }
}

// 0x706720
void CGameSprite::SetCharacterToolTip(CUIControlBase* pControl)
{
    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1) {
        if (m_baseStats.m_name != -1) {
            CString sHitPoints;
            if (!m_derivedStats.m_spellStates[SPLSTATE_SUPPRESS_HP_INFO]) {
                sHitPoints.Format("%d/%d",
                    m_baseStats.m_hitPoints,
                    m_derivedStats.m_nMaxHitPoints);
            } else {
                sHitPoints.Format("%c/%c", '?', '?');
            }
            g_pBaldurChitin->GetObjectCursor()->SetToolTip(m_baseStats.m_name,
                pControl,
                sHitPoints);
        } else {
            CString sHitPoints;
            if (!m_derivedStats.m_spellStates[SPLSTATE_SUPPRESS_HP_INFO]) {
                sHitPoints.Format("%s\n%d/%d",
                    (LPCSTR)m_sName,
                    m_baseStats.m_hitPoints,
                    m_derivedStats.m_nMaxHitPoints);
            } else {
                sHitPoints.Format("%s\n%c/%c", (LPCSTR)m_sName, '?', '?');
            }
            g_pBaldurChitin->GetObjectCursor()->SetToolTip(m_baseStats.m_name,
                pControl,
                sHitPoints);
        }
    } else {
        BOOLEAN bReturn = FALSE;
        STR_RES strRes;
        if ((m_baseStats.m_flags & 0x1000) == 0) {
            if (m_typeAI.GetEnemyAlly() >= CAIObjectType::EA_EVILCUTOFF
                || m_typeAI.GetEnemyAlly() <= CAIObjectType::EA_GOODCUTOFF) {
                if (m_baseStats.m_hitPoints == m_derivedStats.m_nMaxHitPoints) {
                    bReturn = g_pBaldurChitin->GetTlkTable().Fetch(2943, strRes);
                } else {
                    INT nHealthPercent = 100 * m_baseStats.m_hitPoints / m_derivedStats.m_nMaxHitPoints;
                    if (nHealthPercent > 75) {
                        bReturn = g_pBaldurChitin->GetTlkTable().Fetch(2944, strRes);
                    } else if (nHealthPercent > 50) {
                        bReturn = g_pBaldurChitin->GetTlkTable().Fetch(2945, strRes);
                    } else if (nHealthPercent > 25) {
                        bReturn = g_pBaldurChitin->GetTlkTable().Fetch(2946, strRes);
                    } else {
                        bReturn = g_pBaldurChitin->GetTlkTable().Fetch(2947, strRes);
                    }
                }
            }
        }

        if ((m_baseStats.m_flags & 0x1) != 0) {
            if (m_baseStats.m_name != -1) {
                g_pBaldurChitin->GetObjectCursor()->SetToolTip(m_baseStats.m_name,
                    NULL,
                    strRes.szText);
            } else {
                if (bReturn) {
                    g_pBaldurChitin->GetObjectCursor()->SetToolTip(-1,
                        NULL,
                        m_sName + "\n" + strRes.szText);
                } else {
                    g_pBaldurChitin->GetObjectCursor()->SetToolTip(-1,
                        NULL,
                        m_sName);
                }
            }
        } else if (m_baseStats.m_apparentName != -1) {
            g_pBaldurChitin->GetObjectCursor()->SetToolTip(m_baseStats.m_apparentName,
                NULL,
                strRes.szText);
        } else {
            if (bReturn) {
                g_pBaldurChitin->GetObjectCursor()->SetToolTip(-1,
                    NULL,
                    m_sName + "\n" + strRes.szText);
            } else {
                g_pBaldurChitin->GetObjectCursor()->SetToolTip(-1,
                    NULL,
                    m_sName + "\n" + strRes.szText);
            }
        }
    }
}

// 0x706B40
void CGameSprite::SetDirection(const CPoint& posDest)
{
    if (m_pos == posDest) {
        // NOTE: Uninline.
        SetDirection(m_nNewDirection);
    } else {
        // NOTE: Uninline.
        SetDirection(GetDirection(posDest));
    }
}

// 0x706C80
void CGameSprite::SetDirection(SHORT direction)
{
    m_nNewDirection = direction;
    m_deltaDirection = direction > m_nDirection ? 1 : -1;

    if (direction - m_nDirection > 8 || direction - m_nDirection <= -8) {
        // Turn the short way: when the target is more than half the 16-step
        // compass away, flip the step sign so we rotate across the 0/15 seam
        // (binary: `neg edx` at 0x706cb9) instead of the long way around.
        m_deltaDirection = -m_deltaDirection;
    }

    if (abs(direction - m_nDirection) == 8) {
        m_skipDeltaDirection = 4;
    } else {
        m_skipDeltaDirection = 0;
    }

    if (m_nSequence == CGAMESPRITE_SEQ_WALK && m_walkBackwards) {
        if (abs(direction - m_nDirection) == 8) {
            m_nNewDirection = (direction + 8) % 16;
            m_deltaDirection = -m_deltaDirection;
        } else {
            m_walkBackwards = FALSE;
        }
    }
}

// 0x706F80
void CGameSprite::SetFacing(SHORT direction)
{
    m_nDirection = direction;
    m_nNewDirection = direction;

    // NOTE: Uninline.
    GetAnimation()->ChangeDirection(direction);

    if (m_derivedStats.m_visualEffects[IWD_VFX_BLUR]) {
        InitBlur();
    }
}

// 0x706FE0
void CGameSprite::InitBlur()
{
    m_posOld = m_pos;

    if (m_nDirection >= 2 && m_nDirection <= 7) {
        // NOTE: Uninline.
        m_posOld.x += GetAnimation()->GetMoveScale();
    } else if (m_nDirection >= 10) {
        // NOTE: Uninline.
        m_posOld.x -= GetAnimation()->GetMoveScale();
    }

    if (m_nDirection <= 3 || m_nDirection >= 14) {
        // NOTE: Uninline.
        m_posOld.y -= 3 * GetAnimation()->GetMoveScale() / 4;
    } else if (m_nDirection >= 6 && m_nDirection <= 11) {
        // NOTE: Uninline.
        m_posOld.y += 3 * GetAnimation()->GetMoveScale() / 4;
    }
}

// 0x707170
void CGameSprite::SetIdleSequence()
{
    // NOTE: Uninline.
    SHORT nIdleSequence = GetIdleSequence();

    // NOTE: Uninline.
    BOOL bIsEndOfSequence = m_animation.IsEndOfSequence();

    if (m_nSequence != nIdleSequence || bIsEndOfSequence) {
        SetSequence(nIdleSequence);
    }
}

// 0x7071F0
SHORT CGameSprite::GetIdleSequence()
{
    if (m_pArea != NULL
        && m_pArea->m_bAreaLoaded
        && g_pBaldurChitin->GetObjectGame()->IsAreaSaveAllowed(m_pArea)) {
        return 7; // SEQ_READY
    } else {
        return 6; // SEQ_HEAD_TURN
    }
}

// 0x707230
void CGameSprite::SetSequence(SHORT nSequence)
{
    if (nSequence == SEQ_WALK) {
        // NOTE: Uninline.
        if (m_animation.GetMoveScale() == 0) {
            return;
        }
    }

    if (m_nSequence == nSequence) {
        // NOTE: Uninline.
        if (m_nSequence != GetIdleSequence()) {
            return;
        }
    }

    if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
        && (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bFootStepsSounds
            || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1)
        && m_nSequence != nSequence
        && m_pArea == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()
        && m_pArea != NULL) {
        m_currSndArmor = 0;
        m_sndArmor[m_currSndArmor].Stop();

        // NOTE: Uninline.
        char* sndArmor = m_animation.GetSndArmor();

        if (sndArmor[0] != '\0') {
            // NOTE: Uninline.
            m_sndArmor[m_currSndArmor].SetResRef(CResRef(sndArmor), TRUE, TRUE);
            delete sndArmor;

            CPoint ear;
            LONG earZ;
            g_pBaldurChitin->cSoundMixer.GetListenPosition(ear, earZ);

            LONG priority = max(99 - 99 * ((ear.y - m_pos.y) * (ear.y - m_pos.y) / 144 + (ear.x - m_pos.x) * (ear.x - m_pos.x) / 256) / 6400, 0);
            m_sndArmor[m_currSndArmor].SetPriority(static_cast<BYTE>(priority));

            m_sndArmor[m_currSndArmor].Play(m_pos.x, m_pos.y, m_posZ, FALSE);
        }
    }

    if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
        && (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bFootStepsSounds
            || g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == -1)
        && m_nSequence != nSequence
        && m_pArea == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()
        && m_pArea != NULL
        && nSequence != SEQ_DIE
        && nSequence != SEQ_SLEEP
        && nSequence != SEQ_TWITCH
        && nSequence != SEQ_WALK
        && nSequence != SEQ_AWAKE) {
        if (!m_sndReady.IsSoundPlaying()) {
            // NOTE: Uninline.
            char* sndReady = m_animation.GetSndReady();

            if (sndReady[0] != '\0') {
                // NOTE: Uninline.
                m_sndReady.SetResRef(CResRef(sndReady), TRUE, TRUE);

                delete sndReady;

                m_sndReady.SetLoopingFlag(1);
                m_sndReady.Play(m_pos.x, m_pos.y, m_posZ, FALSE);
            }
        }
    } else {
        m_sndReady.Stop();
    }

    SHORT nDirection = m_nDirection & 0xFE;
    CItem* curWeapon;
    const ITEM_ABILITY* curAttack;

    switch (nSequence) {
    case SEQ_ATTACK:
        curWeapon = m_equipment.m_items[m_equipment.m_selectedWeapon];
        if (curWeapon != NULL) {
            curWeapon->Demand();
            curAttack = curWeapon->GetAbility(m_equipment.m_selectedWeaponAbility);
        } else {
            SelectWeaponAbility(CGameSpriteEquipment::SLOT_FIST, 0, 0, 1);
            curAttack = &DEFAULT_ATTACK;
        }

        if (curAttack == NULL) {
            curAttack = &DEFAULT_ATTACK;
        }

        if (curAttack->type != 2) {
            LONG totalProb = 0;
            LONG prob = rand() % 100 + 1;
            for (int index = 0; index < 6 && prob > totalProb; index++) {
                totalProb += curAttack->attackProbability[index];
                if (prob <= totalProb) {
                    switch (index) {
                    case 0:
                        nSequence = SEQ_ATTACK_SLASH;
                        break;
                    case 1:
                        nSequence = SEQ_ATTACK_BACKSLASH;
                        break;
                    case 2:
                        nSequence = SEQ_ATTACK_JAB;
                        break;
                    default:
                        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                        // __LINE__: 9748
                        UTIL_ASSERT(FALSE);
                    }
                    break;
                }
            }
        } else {
            nSequence = SEQ_SHOOT;
        }

        if (curWeapon != NULL) {
            curWeapon->Release();
        }

        if (nSequence == SEQ_HEAD_TURN) {
            // NOTE: Uninline.
            nSequence = GetIdleSequence();
        }
        break;
    case SEQ_DAMAGE:
        m_nBloodFlashAmount = 128;
        m_nDamageLocatorTime = 128;
        m_bBloodFlashOn = TRUE;
        g_pBaldurChitin->GetObjectGame()->UpdatePortrait(g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id), 1);
        break;
    case SEQ_TWITCH:
        m_nTwitches = 0;
        break;
    case SEQ_WALK:
        nDirection = m_nDirection;
        break;
    }

    if (nSequence != SEQ_DIE || m_nSequence != SEQ_SLEEP) {
        // NOTE: Uninline.
        m_nSequence = m_animation.SetSequence(nSequence);
    }

    if (m_nNewDirection == m_nDirection) {
        m_nDirection = nDirection;
        m_nNewDirection = nDirection;
    } else {
        m_nDirection = nDirection;
    }
}

// 0x707980
void CGameSprite::SetTarget(const CPoint& target, BOOL collisionPath)
{
    m_pathSearchInvalidDest = FALSE;
    if (m_pPath != NULL) {
        delete m_pPath;
        m_pPath = NULL;
    }

    DropSearchRequest();

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && InControl()) {
        CMessage* message = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE, CMessage::BROADCAST_OTHERS);
    }

    m_currentSearchRequest = new CSearchRequest();
    if (m_currentSearchRequest != NULL) {
        m_currentSearchRequest->m_searchBitmap = &(m_pArea->m_search);

        if (m_animation.GetListType() == LIST_FLIGHT) {
            memcpy(m_currentSearchRequest->m_terrainTable, m_flightTerrainTable, sizeof(m_currentSearchRequest->m_terrainTable));
        } else {
            memcpy(m_currentSearchRequest->m_terrainTable, m_terrainTable, sizeof(m_currentSearchRequest->m_terrainTable));
        }

        m_currentSearchRequest->m_collisionSearch = collisionPath;
        if (collisionPath) {
            m_currentSearchRequest->m_collisionDelay = rand() % 15;
        } else {
            m_currentSearchRequest->m_collisionDelay = 0;
        }
        m_currentSearchRequest->m_removeSelf = m_animation.GetListType() != LIST_FLIGHT;
        m_currentSearchRequest->m_pathSmooth = m_animation.GetPathSmooth();
        m_currentSearchRequest->m_sourceId = m_id;
        m_currentSearchRequest->m_nTargetPoints = 1;
        m_currentSearchRequest->m_targetPoints = new POINT[m_currentSearchRequest->m_nTargetPoints];
        if (m_currentSearchRequest->m_targetPoints) {
            m_currentSearchRequest->m_targetPoints[0].x = target.x;
            m_currentSearchRequest->m_targetPoints[0].y = target.y;
            if (m_currentSearchRequest->m_collisionDelay == 0) {
                CSingleLock lock(&(g_pBaldurChitin->GetObjectGame()->field_1B58), TRUE);
                g_pBaldurChitin->GetObjectGame()->m_searchRequests.AddTail(m_currentSearchRequest);
                g_pBaldurChitin->GetObjectGame()->m_searchRequestListEmpty = FALSE;
                ReleaseSemaphore(g_pBaldurChitin->GetObjectGame()->m_hSearchThread, 1, NULL);
                lock.Unlock();
            }

            // NOTE: Uninline.
            SetIdleSequence();
        } else {
            delete m_currentSearchRequest;
            m_currentSearchRequest = NULL;
        }
    }
}

// 0x707D40
void CGameSprite::SetTarget(CSearchRequest* pSearchRequest, BOOL collisionPath, BYTE frontList)
{
    m_pathSearchInvalidDest = FALSE;
    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && InControl()) {
        CMessage* message = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE, CMessage::BROADCAST_OTHERS);
    }

    if (frontList == CSearchRequest::LIST_FRONT) {
        if (m_pPath != NULL) {
            delete m_pPath;
            m_pPath = NULL;
        }
    }

    if (m_currentSearchRequest != pSearchRequest) {
        DropSearchRequest();
    }
    m_currentSearchRequest = pSearchRequest;

    m_currentSearchRequest->m_serviceState = CSearchRequest::STATE_WAITING;
    m_currentSearchRequest->m_collisionSearch = collisionPath;
    if (collisionPath) {
        m_currentSearchRequest->m_collisionDelay = rand() % 15;
    } else {
        m_currentSearchRequest->m_collisionDelay = 0;
    }
    m_currentSearchRequest->m_removeSelf = pSearchRequest->m_removeSelf;
    m_currentSearchRequest->m_frontList = frontList;

    if (!m_currentSearchRequest->m_collisionDelay) {
        CSingleLock lock(&(g_pBaldurChitin->GetObjectGame()->field_1B58), TRUE);
        if (frontList == CSearchRequest::LIST_FRONT) {
            g_pBaldurChitin->GetObjectGame()->m_searchRequests.AddTail(m_currentSearchRequest);
            g_pBaldurChitin->GetObjectGame()->m_searchRequestListEmpty = FALSE;
        } else {
            g_pBaldurChitin->GetObjectGame()->m_searchRequestsBack.AddTail(m_currentSearchRequest);
        }
        ReleaseSemaphore(g_pBaldurChitin->GetObjectGame()->m_hSearchThread, 1, NULL);
        lock.Unlock();
    }

    // NOTE: Uninline.
    SetIdleSequence();
}

// 0x707FB0
void CGameSprite::SetTargetFly(BYTE circleType, BYTE facing, LONG radius)
{
    m_pathSearchInvalidDest = FALSE;
    if (m_pPath != NULL) {
        delete m_pPath;
        m_pPath = NULL;
    }

    DropSearchRequest();

    m_radius = static_cast<SHORT>(radius);
    m_circleFacing = facing;
    m_fCurrCircleChange = 0.0;
    m_posOld = m_pos;
    m_turningAbout = FALSE;
    m_walkBackwards = FALSE;
    SetSequence(CGAMESPRITE_SEQ_WALK);

    if (circleType == FLY_0) {
        m_circleFacing = 0;
        m_fCircleChange = static_cast<double>(m_radius / m_animation.GetMoveScale());
    } else {
        m_fCircleChange = static_cast<double>(m_radius) * static_cast<double>(circleType) * 3.1415926535 / static_cast<double>(m_animation.GetMoveScale()) / 2.0;

        switch (abs(4 * ((m_nDirection + 2) / 4))) {
        case 0:
            m_posOld.x += m_radius * m_circleFacing;
            break;
        case 4:
            m_posOld.y += 3 * m_radius * m_circleFacing / 4;
            break;
        case 8:
            m_posOld.x -= m_radius * m_circleFacing;
            break;
        case 12:
            m_posOld.y -= 3 * m_radius * m_circleFacing / 4;
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 10009
            UTIL_ASSERT(FALSE);
        }

        m_fDirectionOffset = static_cast<double>(8 - m_nDirection) * 3.1415926535 / 8.0;
        if (m_circleFacing == FLY_RIGHT) {
            m_fDirectionOffset += 3.1415926535;
        }
    }
}

// 0x708280
void CGameSprite::StartSpriteEffect(BYTE spriteEffect, BYTE intensityLevel, BYTE effectDuration, BOOLEAN a4)
{
    m_spriteEffectDuration = 0;
    if (m_pSpriteEffectArray != NULL) {
        delete m_pSpriteEffectArray;
        m_pSpriteEffectArray = NULL;
        delete m_pSpriteEffectArrayPosition;
        m_pSpriteEffectArrayPosition = NULL;
    }

    switch (spriteEffect) {
    case 0:
        field_5304 = 0;
        m_spriteEffectFlags = 0;

        if (m_animation.GetColorBlood() != 255) {
            m_spriteSplashPalette.SetRange(0,
                m_animation.GetColorBlood(),
                *g_pBaldurChitin->GetObjectGame()->GetMasterBitmap());
            m_spriteSplashVidCell.SetPalette(m_spriteSplashPalette);
            if (intensityLevel == CBlood::GUSH_LOW) {
                m_spriteSplashVidCell.SetResRef(CResRef("BLOODS"), FALSE, TRUE, TRUE);
            } else if (intensityLevel == CBlood::GUSH_MEDIUM) {
                m_spriteSplashVidCell.SetResRef(CResRef("BLOODM"), FALSE, TRUE, TRUE);
            } else if (intensityLevel == CBlood::GUSH_HIGH
                || intensityLevel == CBlood::EXPLODING_DEATH) {
                m_spriteSplashVidCell.SetResRef(CResRef("BLOODL"), FALSE, TRUE, TRUE);
            } else if (intensityLevel == CBlood::PULSATING_ARTERY) {
                m_spriteSplashVidCell.SetResRef(CResRef("BLOODCR"), FALSE, TRUE, TRUE);
            } else {
                m_spriteEffectDuration = 0;
                return;
            }

            m_spriteSplashVidCell.SequenceSet(0);
            m_spriteSplashVidCell.FrameSet(0);
            m_spriteEffectSequenceLength = 1;
            m_spriteEffectBaseIntensity = 0;
            m_spriteEffectRandomIntensity = 1;
        } else {
            m_spriteEffectDuration = 0;
            return;
        }
        break;
    case 1:
        if (a4 == TRUE) {
            AddEffect(IcewindMisc::CreateEffectVisualSpellHit(this, 0x3F, 10, 0),
                EFFECT_LIST_TIMED,
                TRUE,
                TRUE);
        }
        m_spriteEffectFlags = 0x200;
        m_spriteEffectVidCell.SetResRef(CResRef("FireL"), FALSE, TRUE, TRUE);
        m_spriteEffectVidCell.SequenceSet(0);
        m_spriteEffectSequenceNumber = 1;
        m_spriteEffectSequenceLength = m_spriteEffectVidCell.GetSequenceLength(m_spriteEffectVidCell.m_nCurrentSequence, FALSE);
        m_effectExtendDirection = 1;
        switch (intensityLevel) {
        case 0:
            m_spriteEffectBaseIntensity = -2;
            m_spriteEffectRandomIntensity = 4;
            break;
        case 1:
            m_spriteEffectBaseIntensity = -1;
            m_spriteEffectRandomIntensity = 3;
            break;
        case 2:
            m_spriteEffectBaseIntensity = 0;
            m_spriteEffectRandomIntensity = 2;
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 10204
            UTIL_ASSERT(FALSE);
        }
        break;
    case 2:
        if (a4 == TRUE) {
            AddEffect(IcewindMisc::CreateEffectVisualSpellHit(this, 0x41, 10, 0),
                EFFECT_LIST_TIMED,
                TRUE,
                TRUE);
        }
        m_spriteEffectFlags = 0x200;
        m_spriteEffectVidCell.SetResRef(CResRef("ElectrL"), FALSE, TRUE, TRUE);
        m_spriteEffectVidCell.SequenceSet(0);
        m_spriteEffectSequenceNumber = static_cast<BYTE>(m_spriteEffectVidCell.GetNumberSequences(FALSE));
        m_spriteEffectSequenceLength = m_spriteEffectVidCell.GetSequenceLength(m_spriteEffectVidCell.m_nCurrentSequence, FALSE);
        m_effectExtendDirection = 0;
        switch (intensityLevel) {
        case 0:
            m_spriteEffectBaseIntensity = -2;
            m_spriteEffectRandomIntensity = 4;
            break;
        case 1:
            m_spriteEffectBaseIntensity = -1;
            m_spriteEffectRandomIntensity = 3;
            break;
        case 2:
            m_spriteEffectBaseIntensity = 0;
            m_spriteEffectRandomIntensity = 2;
            break;
        default:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 10259
            UTIL_ASSERT(FALSE);
        }
        break;
    case 3:
        if (a4 == TRUE) {
            AddEffect(IcewindMisc::CreateEffectVisualSpellHit(this, 0x40, 10, 0),
                EFFECT_LIST_TIMED,
                TRUE,
                TRUE);
        }
        m_spriteEffectFlags = 0x200;
        m_spriteEffectVidCell.SetResRef(CResRef("ColdL"), FALSE, TRUE, TRUE);
        m_spriteEffectVidCell.SequenceSet(0);
        m_spriteEffectSequenceNumber = 1;
        m_spriteEffectSequenceLength = m_spriteEffectVidCell.GetSequenceLength(m_spriteEffectVidCell.m_nCurrentSequence, FALSE);
        m_effectExtendDirection = 1;
        m_spriteEffectBaseIntensity = 0;
        m_spriteEffectRandomIntensity = 1;
        break;
    case 4:
        if (a4 == TRUE) {
            AddEffect(IcewindMisc::CreateEffectVisualSpellHit(this, 0x42, 10, 0),
                EFFECT_LIST_TIMED,
                TRUE,
                TRUE);
        }
        m_spriteEffectFlags = 0x200;
        m_spriteEffectVidCell.SetResRef(CResRef("None"), FALSE, TRUE, TRUE);
        m_spriteEffectVidCell.SequenceSet(0);
        m_spriteEffectSequenceNumber = static_cast<BYTE>(m_spriteEffectVidCell.GetNumberSequences(FALSE));
        m_spriteEffectSequenceLength = m_spriteEffectVidCell.GetSequenceLength(m_spriteEffectVidCell.m_nCurrentSequence, FALSE);
        m_effectExtendDirection = 0;
        // FIXME: `m_spriteEffectBaseIntensity` and `m_spriteEffectRandomIntensity` not initialized.
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 10324
        UTIL_ASSERT(FALSE);
    }

    int cnt = m_spriteEffectSequenceLength * (m_spriteEffectBaseIntensity + m_spriteEffectRandomIntensity - 1);
    if (m_spriteEffectSequenceLength * (m_spriteEffectBaseIntensity + m_spriteEffectRandomIntensity - 1) > 0) {
        m_pSpriteEffectArray = new USHORT[cnt];
        if (m_pSpriteEffectArray == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 10332
            UTIL_ASSERT(FALSE);
            return;
        }

        m_pSpriteEffectArrayPosition = new POINT[cnt];
        if (m_pSpriteEffectArrayPosition == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 10338
            UTIL_ASSERT(FALSE);
            delete m_pSpriteEffectArray;
            m_pSpriteEffectArray = NULL;
            return;
        }

        memset(m_pSpriteEffectArray, 0xFF, sizeof(USHORT) * cnt);
    }

    m_spriteEffectDuration = m_spriteEffectSequenceLength + effectDuration + 9;
    m_animation.CalculateFxRect(m_rSpriteEffectFX, m_ptSpriteEffectReference, m_posZ);
}

// 0x708D50
void CGameSprite::RenderSpriteCover(CVidMode* pVidMode, INT nSurface, CVidCell* pVidCell, const IcewindCVisualEffect& vfx)
{
    CRect rFX;
    CSize objectSize;
    CRect rGCBounds;
    CPoint newPos;
    CPoint ptReference;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 10436
    UTIL_ASSERT(pVidMode != NULL);

    DWORD dwRenderFlags = vfx.m_dwFlags;

    BOOLEAN bDithered = m_id == m_pArea->m_iPicked
        || g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bAlwaysDither
        || g_pBaldurChitin->GetObjectGame()->m_bForceDither;

    pVidCell->GetCurrentCenterPoint(ptReference, FALSE);
    pVidCell->GetCurrentFrameSize(objectSize, FALSE);

    rFX.SetRect(0, 0, objectSize.cx, objectSize.cy);

    newPos.x = m_pos.x;
    newPos.y = m_pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y + 1;

    if (vfx.m_bCopyFromBack == TRUE || vfx.m_bTransparent == TRUE) {
        dwRenderFlags |= CInfinity::FXPREP_COPYFROMBACK | 0x2;
    } else {
        dwRenderFlags |= CInfinity::FXPREP_CLEARFILL | 0x1;
    }

    m_pArea->GetInfinity()->FXPrep(rFX,
        dwRenderFlags,
        nSurface,
        newPos,
        ptReference);

    if (m_pArea->GetInfinity()->FXLock(rFX, dwRenderFlags)) {
        m_pArea->GetInfinity()->FXRender(pVidCell,
            ptReference.x,
            ptReference.y,
            vfx.m_dwFlags,
            vfx.m_nTransValue);

        rGCBounds.left = newPos.x - ptReference.x;
        rGCBounds.top = newPos.y + m_posZ - ptReference.y;
        rGCBounds.right = rGCBounds.left + rFX.Width();
        rGCBounds.bottom = rGCBounds.top + rFX.Height() - m_posZ;
        m_pArea->GetInfinity()->FXRenderClippingPolys(newPos.x,
            newPos.y - m_posZ,
            m_posZ,
            CPoint(ptReference.x, ptReference.y + m_posZ),
            rGCBounds,
            bDithered,
            FALSE);

        m_pArea->GetInfinity()->FXUnlock(dwRenderFlags, NULL, CPoint(0, 0));
        m_pArea->GetInfinity()->FXBltFrom(nSurface,
            rFX,
            newPos.x,
            newPos.y + m_posZ,
            ptReference.x,
            ptReference.y,
            vfx.m_dwFlags | 0x1);
    }
}

// 0x708FC0
void CGameSprite::RenderDamageArrow(CGameArea* pArea, CVidMode* pVidMode, INT nSurface)
{
    // TODO: Incomplete.
}

// 0x7093E0
//
// Draws whatever StartSpriteEffect armed: while the blood splash cell still
// has frames left it renders as a single cell pinned to the sprite; after
// that the elemental overlay renders as a grid of particle cells (one row
// per animation frame, base+random-1 particles per row, 0xFFFF = end of
// row), clipped to the sprite's FX rect.
void CGameSprite::RenderSpriteEffect(CVidMode* pVidMode, INT nSurface)
{
    CPoint ptCenter;
    CSize frameSize;
    CRect rFXRect;
    CRect rGCBounds;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 10675
    UTIL_ASSERT(pVidMode != NULL);

    DWORD dwFlags = m_spriteEffectFlags;
    if (m_animation.IsMirroring()) {
        dwFlags |= CInfinity::MIRROR_FX;
    }

    if (m_spriteEffectDuration == 0) {
        return;
    }

    BOOL bDithered = m_id == m_pArea->m_iPicked
        || g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bAlwaysDither
        || g_pBaldurChitin->GetObjectGame()->m_bForceDither;

    if (!m_spriteSplashVidCell.IsEndOfSequence(FALSE)) {
        m_spriteSplashVidCell.GetCurrentCenterPoint(ptCenter, FALSE);
        m_spriteSplashVidCell.GetCurrentFrameSize(frameSize, FALSE);
        SetRect(&rFXRect, 0, 0, frameSize.cx, frameSize.cy);

        INT x = m_pos.x;
        INT nY = m_pArea->GetHeightOffset(m_pos, m_listType) + m_pos.y;

        rGCBounds.left = x - ptCenter.x;
        rGCBounds.right = rFXRect.Width() + rGCBounds.left;

        dwFlags |= CInfinity::FXPREP_COPYFROMBACK;

        CPoint ptPos(x, (m_posZ - m_ptSpriteEffectReference.y / 2) + nY);
        INT nRenderY = (m_posZ - ptCenter.y) + nY;

        m_pArea->GetInfinity()->FXPrep(rFXRect, dwFlags, nSurface, ptPos, ptCenter);
        if (!m_pArea->GetInfinity()->FXLock(rFXRect, dwFlags)) {
            return;
        }

        m_pArea->GetInfinity()->FXRender(&m_spriteSplashVidCell,
            ptCenter.x,
            ptCenter.y,
            dwFlags,
            0);

        INT nHalfRefY = m_ptSpriteEffectReference.y / 2;
        rGCBounds.top = (nRenderY - nHalfRefY) - m_posZ;
        rGCBounds.bottom = ((rFXRect.Height() + nRenderY) - nHalfRefY) - m_posZ;
        m_pArea->GetInfinity()->FXRenderClippingPolys(x,
            (nY - nHalfRefY) - m_posZ,
            m_posZ,
            CPoint(ptCenter.x, (m_posZ - nHalfRefY) + ptCenter.y),
            rGCBounds,
            bDithered,
            dwFlags);

        m_pArea->GetInfinity()->FXUnlock(dwFlags, NULL, CPoint(0, 0));

        m_pArea->GetInfinity()->FXBltFrom(nSurface,
            rFXRect,
            x,
            (m_posZ - nHalfRefY) + nY,
            ptCenter.x,
            ptCenter.y,
            dwFlags);
        return;
    }

    if (m_pSpriteEffectArray == NULL) {
        return;
    }

    BOOL bRendered = FALSE;
    CPoint ptOffset;
    CSize fxSize;

    pVidMode->GetFXSize(fxSize);

    switch (m_effectExtendDirection) {
    case 0:
        ptOffset.x = 0;
        ptOffset.y = 0;
        SetRect(&rFXRect, 0, 0, m_rSpriteEffectFX.right, m_rSpriteEffectFX.bottom);
        break;
    case 1:
        ptOffset.x = 0;
        ptOffset.y = (m_rSpriteEffectFX.bottom - fxSize.cy) + 1;
        SetRect(&rFXRect, 0, 0, m_rSpriteEffectFX.right, fxSize.cy - 1);
        break;
    case 2:
        ptOffset.x = 0;
        ptOffset.y = 0;
        SetRect(&rFXRect, 0, 0, m_rSpriteEffectFX.right, fxSize.cy - 1);
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 10799
        UTIL_ASSERT(FALSE);
    }

    if (fxSize.cx < rFXRect.right) {
        rFXRect.right = fxSize.cx;
    }
    if (fxSize.cy < rFXRect.bottom) {
        rFXRect.bottom = fxSize.cy;
    }

    INT nHeightOffset = m_pArea->GetHeightOffset(m_pos, m_listType);
    INT nLeft = (m_pos.x - m_ptSpriteEffectReference.x) + ptOffset.x;
    INT nTop = (m_posZ - m_ptSpriteEffectReference.y) + m_pos.y + nHeightOffset + ptOffset.y;
    INT nRight = rFXRect.Width() + nLeft;
    INT nBottom = rFXRect.Height() + nTop;

    if ((m_spriteEffectFlags & 0x208) == 0) {
        dwFlags |= CInfinity::FXPREP_CLEARFILL | 0x1;
    } else {
        dwFlags |= CInfinity::FXPREP_COPYFROMBACK;
    }

    CPoint ptPos(m_pos.x + ptOffset.x, m_posZ + m_pos.y + nHeightOffset + ptOffset.y);

    m_pArea->GetInfinity()->FXPrep(rFXRect, dwFlags, nSurface, ptPos, m_ptSpriteEffectReference);
    if (!m_pArea->GetInfinity()->FXLock(rFXRect, dwFlags)) {
        return;
    }

    for (BYTE nFrame = 0; nFrame < m_spriteEffectSequenceLength; nFrame++) {
        // The base intensity is stored negative for the weakest levels.
        INT nParticles = static_cast<char>(m_spriteEffectBaseIntensity) - 1 + m_spriteEffectRandomIntensity;
        for (BYTE nParticle = 0; static_cast<INT>(nParticle) < nParticles; nParticle++) {
            INT nIndex = nParticles * nFrame + nParticle;

            USHORT entry = m_pSpriteEffectArray[nIndex];
            if (entry == 0xFFFF) {
                break;
            }

            m_spriteEffectVidCell.SequenceSet(static_cast<SHORT>(entry >> 8));
            m_spriteEffectVidCell.FrameSet(static_cast<SHORT>(entry & 0xFF));
            m_spriteEffectVidCell.GetCurrentCenterPoint(ptCenter, FALSE);
            m_spriteEffectVidCell.GetCurrentFrameSize(frameSize, FALSE);

            POINT* pt = &m_pSpriteEffectArrayPosition[nIndex];

            INT x;
            if ((dwFlags & CInfinity::MIRROR_FX) == 0) {
                x = ((pt->x - ptOffset.x) - ptCenter.x) + m_ptSpriteEffectReference.x;
            } else {
                x = ((rFXRect.right - pt->x) - m_ptSpriteEffectReference.x) + ptOffset.x + ptCenter.x;
            }
            INT y = ((pt->y - ptOffset.y) - ptCenter.y) + m_ptSpriteEffectReference.y;

            if (frameSize.cx / 4 <= x && x <= rFXRect.right - frameSize.cx / 4
                && frameSize.cy / 4 <= y && y <= rFXRect.bottom - frameSize.cy / 4
                && x - ptCenter.x > -1
                && (x - ptCenter.x) + frameSize.cx < rFXRect.Width()
                && y - ptCenter.y > -1
                && (y - ptCenter.y) + frameSize.cy < rFXRect.Height()) {
                m_pArea->GetInfinity()->FXRender(&m_spriteEffectVidCell, x, y, dwFlags, 0);
                bRendered = TRUE;
            }
        }
    }

    if (bRendered) {
        rGCBounds.left = nLeft;
        rGCBounds.top = nTop;
        rGCBounds.right = nRight;
        rGCBounds.bottom = nBottom - m_posZ;
        m_pArea->GetInfinity()->FXRenderClippingPolys(m_pos.x,
            m_pos.y,
            0,
            m_ptSpriteEffectReference,
            rGCBounds,
            bDithered,
            dwFlags);
    }

    m_pArea->GetInfinity()->FXUnlock(dwFlags, NULL, CPoint(0, 0));

    if (!bRendered) {
        return;
    }

    m_pArea->GetInfinity()->FXBltFrom(nSurface,
        rFXRect,
        m_pos.x + ptOffset.x,
        m_posZ + m_pos.y + nHeightOffset + ptOffset.y,
        m_ptSpriteEffectReference.x,
        m_ptSpriteEffectReference.y,
        dwFlags);
}

// 0x709B60
void CGameSprite::UpdateSpriteEffect()
{
    SHORT nSequence;
    SHORT nNumberSequences;

    for (INT nIndex = 0; nIndex < 32; nIndex++) {
        if (!m_derivedStats.m_visualEffects[nIndex]) {
            if (nIndex == IWD_VFX_ENTANGLE
                || nIndex == IWD_VFX_WEB
                || nIndex == IWD_VFX_CLOAK_OF_FEAR) {
                m_aVfxCells[nIndex].SequenceSet(0);
                m_aVfxCells[nIndex].FrameSet(0);
            }

            if (nIndex == IWD_VFX_FIRE_SHIELD_RED
                || nIndex == IWD_VFX_FIRE_SHIELD_BLUE) {
                m_aVfxCells[nIndex].SequenceSet(0);
                m_aVfxCells[nIndex].FrameSet(0);
            }

            if (nIndex == IWD_VFX_28
                || nIndex == IWD_VFX_29) {
                m_aVfxCells[nIndex].SequenceSet(1);
                m_aVfxCells[nIndex].FrameSet(0);
            }
        }

        if (m_derivedStats.m_visualEffects[nIndex]
            && nIndex != IWD_VFX_INVISIBILITY
            && nIndex != IWD_VFX_BLUR) {
            switch (nIndex) {
            case IWD_VFX_SHROUD_OF_FLAME:
                if (m_aVfxCells[nIndex].IsEndOfSequence(FALSE)) {
                    m_sndSpriteEffect.Stop();
                    m_sndSpriteEffect.SetResRef(CResRef("MISC_01C"), TRUE, TRUE);
                    m_sndSpriteEffect.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                    m_sndSpriteEffect.Play(m_pos.x, m_pos.y, 0, FALSE);

                    nNumberSequences = m_aVfxCells[nIndex].GetNumberSequences(FALSE);
                    if (nNumberSequences != 0) {
                        m_aVfxCells[nIndex].SequenceSet(rand() % nNumberSequences);
                    } else {
                        m_aVfxCells[nIndex].SequenceSet(0);
                    }
                } else {
                    m_aVfxCells[nIndex].FrameAdvance();
                }
                break;
            case IWD_VFX_FIRE_SHIELD_RED:
            case IWD_VFX_FIRE_SHIELD_BLUE:
                if (m_aVfxCells[nIndex].IsEndOfSequence(FALSE)) {
                    m_aVfxCells[nIndex].SequenceSet(2);
                } else {
                    m_aVfxCells[nIndex].FrameAdvance();
                }
                break;
            case IWD_VFX_28:
            case IWD_VFX_29:
                if (m_aVfxCells[nIndex].IsEndOfSequence(FALSE)) {
                    m_aVfxCells[nIndex].SequenceSet(3);
                } else {
                    m_aVfxCells[nIndex].FrameAdvance();
                }
                break;
            case IWD_VFX_ENTANGLE:
            case IWD_VFX_WEB:
            case IWD_VFX_CLOAK_OF_FEAR:
                if (m_aVfxCells[nIndex].IsEndOfSequence(FALSE)) {
                    nSequence = m_aVfxCells[nIndex].GetCurrentSequenceId() + 1;
                    nNumberSequences = m_aVfxCells[nIndex].GetNumberSequences(FALSE);
                    if (nSequence >= nNumberSequences) {
                        nSequence--;
                    }
                    m_aVfxCells[nIndex].SequenceSet(nSequence);
                } else {
                    m_aVfxCells[nIndex].FrameAdvance();
                }
                break;
            case IWD_VFX_SEVEN_EYES:
                nSequence = 0;
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_THE_MIND]) {
                    nSequence++;
                }
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_THE_SWORD]) {
                    nSequence++;
                }
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_THE_MAGE]) {
                    nSequence++;
                }
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_VENOM]) {
                    nSequence++;
                }
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_THE_SPIRIT]) {
                    nSequence++;
                }
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_FORTITUDE]) {
                    nSequence++;
                }
                if (!m_derivedStats.m_spellStates[SPLSTATE_EYE_OF_STONE]) {
                    nSequence++;
                }
                if (!m_aVfxSounds[IWD_VFX_SEVEN_EYES].IsSoundPlaying()) {
                    if (nSequence == 7) {
                        m_aVfxSounds[IWD_VFX_SEVEN_EYES].Stop();
                    } else {
                        if (nSequence >= 5) {
                            m_aVfxSounds[IWD_VFX_SEVEN_EYES].SetResRef(CResRef("AFT_M17"), TRUE, TRUE);
                        } else if (nSequence >= 2) {
                            m_aVfxSounds[IWD_VFX_SEVEN_EYES].SetResRef(CResRef("AFT_M16"), TRUE, TRUE);
                        } else {
                            m_aVfxSounds[IWD_VFX_SEVEN_EYES].SetResRef(CResRef("AFT_M15"), TRUE, TRUE);
                        }
                        m_aVfxSounds[IWD_VFX_SEVEN_EYES].SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                        m_aVfxSounds[IWD_VFX_SEVEN_EYES].Play(m_pos.x, m_pos.y, 0, FALSE);
                    }
                }
                m_aVfxCells[IWD_VFX_SEVEN_EYES].SequenceSet(nSequence);
                m_aVfxCells[IWD_VFX_SEVEN_EYES_2].SequenceSet(nSequence);
                if (m_aVfxCells[IWD_VFX_SEVEN_EYES].IsEndOfSequence(FALSE)) {
                    m_aVfxCells[IWD_VFX_SEVEN_EYES].FrameSet(0);
                    m_aVfxCells[IWD_VFX_SEVEN_EYES_2].FrameSet(0);
                } else {
                    m_aVfxCells[IWD_VFX_SEVEN_EYES].FrameAdvance();
                    m_aVfxCells[IWD_VFX_SEVEN_EYES_2].FrameAdvance();
                }
                break;
            case IWD_VFX_SEVEN_EYES_2:
                break;
            default:
                if (!m_aVfxSounds[nIndex].IsSoundPlaying()) {
                    m_aVfxSounds[nIndex].SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                    m_aVfxSounds[nIndex].Play(m_pos.x, m_pos.y, 0, FALSE);
                }
                if (m_aVfxCells[nIndex].IsEndOfSequence(FALSE)) {
                    nNumberSequences = m_aVfxCells[nIndex].GetNumberSequences(FALSE);
                    if (nNumberSequences != 0) {
                        m_aVfxCells[nIndex].SequenceSet(rand() % nNumberSequences);
                    } else {
                        m_aVfxCells[nIndex].SequenceSet(0);
                    }
                } else {
                    m_aVfxCells[nIndex].FrameAdvance();
                }
            }
        }
    }

    if (m_spriteEffectDuration != 0) {
        if (m_spriteSplashVidCell.IsEndOfSequence(FALSE)) {
            if (m_pSpriteEffectArray != NULL) {
                CRect rFx;
                CPoint ptReference;
                m_animation.CalculateFxRect(rFx, ptReference, m_posZ);

                BOOL bFxRectSaved = FALSE;

                for (BYTE nRow = 0; nRow < m_spriteEffectSequenceLength; nRow++) {
                    // The base intensity is stored negative for the weakest levels.
                    INT nParticles = static_cast<char>(m_spriteEffectBaseIntensity) - 1 + m_spriteEffectRandomIntensity;

                    if ((g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime / 2) % m_spriteEffectSequenceLength == nRow) {
                        // This row's turn to respawn: wipe it, then (unless the
                        // effect is already fading out) seed a random batch of
                        // particles in the middle third of the FX rect.
                        memset(&m_pSpriteEffectArray[nParticles * nRow],
                            -1,
                            sizeof(USHORT) * nParticles);

                        if (m_spriteEffectDuration >= m_spriteEffectSequenceLength) {
                            INT nCount = static_cast<char>(m_spriteEffectBaseIntensity);
                            if (m_spriteEffectRandomIntensity != 0) {
                                nCount += rand() % m_spriteEffectRandomIntensity;
                            }

                            for (BYTE nParticle = 0; static_cast<INT>(nParticle) < nCount; nParticle++) {
                                BYTE nSequence = 0;
                                if (m_spriteEffectSequenceNumber != 0) {
                                    nSequence = static_cast<BYTE>(rand() % m_spriteEffectSequenceNumber);
                                }

                                INT nIndex = nParticles * nRow + nParticle;
                                m_pSpriteEffectArray[nIndex] = static_cast<USHORT>(nSequence) << 8;

                                INT nRange = rFx.right / 3 + 1;
                                m_pSpriteEffectArrayPosition[nIndex].x =
                                    (rFx.right / 3 - ptReference.x) + (nRange != 0 ? rand() % nRange : 0);

                                nRange = (rFx.bottom * 4) / 5 + 1;
                                m_pSpriteEffectArrayPosition[nIndex].y =
                                    (rFx.bottom - (nRange != 0 ? rand() % nRange : 0)) - ptReference.y;
                            }
                        }
                    } else {
                        // Other rows: advance each particle one frame and pull
                        // strays back inside the FX rect (which moves with the
                        // sprite's animation).
                        for (BYTE nParticle = 0; static_cast<INT>(nParticle) < nParticles; nParticle++) {
                            INT nIndex = nParticles * nRow + nParticle;
                            if (m_pSpriteEffectArray[nIndex] == 0xFFFF) {
                                break;
                            }

                            LONG x = m_pSpriteEffectArrayPosition[nIndex].x;
                            if (x < rFx.right / 3 - ptReference.x
                                || (rFx.right * 2) / 3 - ptReference.x < x) {
                                INT nRange = rFx.right / 3 + 1;
                                m_pSpriteEffectArrayPosition[nIndex].x =
                                    (rFx.right / 3 - ptReference.x) + (nRange != 0 ? rand() % nRange : 0);
                                if (!bFxRectSaved) {
                                    m_rSpriteEffectFX = rFx;
                                    m_ptSpriteEffectReference = ptReference;
                                    bFxRectSaved = TRUE;
                                }
                            }

                            LONG y = m_pSpriteEffectArrayPosition[nIndex].y;
                            if (rFx.bottom - ptReference.y < y
                                || y < ((rFx.bottom - (rFx.bottom * 4) / 5) - ptReference.y) - 1) {
                                INT nRange = (rFx.bottom * 4) / 5 + 1;
                                m_pSpriteEffectArrayPosition[nIndex].y =
                                    (rFx.bottom - (nRange != 0 ? rand() % nRange : 0)) - ptReference.y;
                                if (!bFxRectSaved) {
                                    m_rSpriteEffectFX = rFx;
                                    m_ptSpriteEffectReference = ptReference;
                                    bFxRectSaved = TRUE;
                                }
                            }

                            m_pSpriteEffectArray[nIndex]++;
                        }
                    }
                }
            }
        } else {
            m_spriteSplashVidCell.FrameAdvance();
        }
        m_spriteEffectDuration--;
    } else {
        if (m_pSpriteEffectArray != NULL) {
            delete m_pSpriteEffectArray;
            m_pSpriteEffectArray = NULL;
            delete m_pSpriteEffectArrayPosition;
            m_pSpriteEffectArrayPosition = NULL;
        }
    }
}

// 0x70AE20
void CGameSprite::ClearMarshal(BOOL unequip)
{
    UINT nClass;
    UINT nLevel;
    POSITION pos;
    CGameEffect* nodeEffect;

    for (nClass = 0; nClass < CSPELLLIST_NUM_CLASSES; nClass++) {
        for (nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            m_spells.m_spellsByClass[nClass].m_lists[nLevel].m_List.clear();
        }
        m_spells.m_spellsByClass[nClass].m_nHighestLevel = 0;
    }

    for (nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        if (m_domainSpells.m_lists[nLevel].m_List.size() > 0) {
            m_domainSpells.m_lists[nLevel].m_List.clear();
            m_domainSpells.m_lists[nLevel].m_nSharedMax = 0;
            m_domainSpells.m_lists[nLevel].m_nSharedTotal = 0;
        }
    }
    m_domainSpells.m_nHighestLevel = 0;

    if (m_innateSpells.m_List.size() > 0) {
        m_innateSpells.m_List.clear();
        m_innateSpells.m_nSharedMax = 0;
        m_innateSpells.m_nSharedTotal = 0;
    }

    if (m_songs.m_List.size() > 0) {
        m_songs.m_List.clear();
        m_songs.m_nSharedMax = 0;
        m_songs.m_nSharedTotal = 0;
    }

    if (m_shapeshifts.m_List.size() > 0) {
        m_shapeshifts.m_List.clear();
        m_shapeshifts.m_nSharedMax = 0;
        m_shapeshifts.m_nSharedTotal = 0;
    }

    // NOTE: Uninline.
    m_equipment.SetSpritePointer(this);

    m_equipment.ClearMarshal(unequip);

    memset(&m_baseStats, 0, sizeof(m_baseStats));

    pos = m_timedEffectList.GetHeadPosition();
    while (pos != NULL) {
        nodeEffect = m_timedEffectList.GetNext(pos);
        delete nodeEffect;
    }

    pos = m_equipedEffectList.GetHeadPosition();
    while (pos != NULL) {
        nodeEffect = m_equipedEffectList.GetNext(pos);
        delete nodeEffect;
    }

    if (m_pDialogData != NULL) {
        delete m_pDialogData;
        m_pDialogData = NULL;
        m_nDialogData = 0;
    }
}

// 0x70AFD0
void CGameSprite::Marshal(CAreaFileCreature** pCreature)
{
    BOOL areaMove = FALSE;

    if (m_pArea == NULL) {
        m_pArea = g_pBaldurChitin->GetObjectGame()->m_pGameAreaMaster;
        m_pos.x = 0;
        m_pos.y = 0;
        areaMove = TRUE;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 11809
    UTIL_ASSERT(pCreature != NULL);

    m_baseStats.m_flags &= ~0x80000000;

    *pCreature = new CAreaFileCreature();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 11817
    UTIL_ASSERT(*pCreature != NULL);

    // FIXME: Redundant, memset is a part of constructor.
    memset(*pCreature, 0, sizeof(CAreaFileCreature));

    if (m_pos.x < -100) {
        m_pos.x = -100;
        m_fCircleChange = 0.0;
        m_fCurrCircleChange = 1.0;
    } else if (m_pos.x > m_pArea->GetInfinity()->nAreaX + 100) {
        m_pos.x = m_pArea->GetInfinity()->nAreaX + 100;
        m_fCircleChange = 0.0;
        m_fCurrCircleChange = 1.0;
    }

    if (m_pos.y < -100) {
        m_pos.y = -100;
        m_fCircleChange = 0.0;
        m_fCurrCircleChange = 1.0;
    } else if (m_pos.y > m_pArea->GetInfinity()->nAreaY + 100) {
        m_pos.y = m_pArea->GetInfinity()->nAreaY + 100;
        m_fCircleChange = 0.0;
        m_fCurrCircleChange = 1.0;
    }

    (*pCreature)->m_posX = static_cast<WORD>(m_pos.x);
    (*pCreature)->m_posY = static_cast<WORD>(m_pos.y);
    (*pCreature)->m_startingPosX = static_cast<WORD>(m_posStart.x);
    (*pCreature)->m_startingPosY = static_cast<WORD>(m_posStart.y);
    (*pCreature)->m_type = m_type;
    (*pCreature)->m_expirationTime = m_expirationTime != -1
        ? m_expirationTime / CTimerWorld::TIMESCALE_MSEC_PER_SEC
        : -1;
    (*pCreature)->m_huntingRange = m_huntingRange;
    (*pCreature)->m_followRange = m_followRange;
    (*pCreature)->m_timeOfDayVisible = m_timeOfDayVisible;

    BYTE* creatureData;
    LONG creatureSize;
    Marshal(&creatureData, &creatureSize, &((*pCreature)->m_facing), TRUE, TRUE);

    // FIXME: Unsafe x64 conversion.
    (*pCreature)->m_creatureOffset = reinterpret_cast<DWORD>(creatureData);
    (*pCreature)->m_creatureSize = creatureSize;

    (*pCreature)->m_numberTimesTalkedTo = m_nNumberOfTimesTalkedTo;

    strncpy((*pCreature)->m_scriptName, m_scriptName, SCRIPTNAME_SIZE);

    if (m_overrideScript != NULL) {
        // NOTE: Uninline.
        m_overrideScript->m_cResRef.GetResRef((*pCreature)->m_overrideScriptOverride);
    }

    if (m_special3Script != NULL) {
        // NOTE: Uninline.
        m_special3Script->m_cResRef.GetResRef((*pCreature)->m_special3ScriptOverride);
    }

    if (m_special2Script != NULL) {
        // NOTE: Uninline.
        m_special2Script->m_cResRef.GetResRef((*pCreature)->m_special2ScriptOverride);
    }

    if (m_combatScript != NULL) {
        // NOTE: Uninline.
        m_combatScript->m_cResRef.GetResRef((*pCreature)->m_combatScriptOverride);
    }

    if (m_movementScript != NULL) {
        // NOTE: Uninline.
        m_movementScript->m_cResRef.GetResRef((*pCreature)->m_movementScriptOverride);
    }

    if (m_teamScript != NULL) {
        // NOTE: Uninline.
        m_teamScript->m_cResRef.GetResRef((*pCreature)->m_teamScriptOverride);
    }

    if (m_special1Script != NULL) {
        // NOTE: Uninline.
        m_special1Script->m_cResRef.GetResRef((*pCreature)->m_special1ScriptOverride);
    }

    if (areaMove) {
        m_pArea = NULL;
    }
}

// 0x70B2F0
void CGameSprite::Marshal(BYTE** pCreature, LONG* creatureSize, WORD* facing, BOOLEAN a4, BOOLEAN a5)
{
    UTIL_ASSERT(pCreature != NULL && creatureSize != NULL && facing != NULL);

    const DWORD CRE_V22_HEADER_SIZE = 0x37C;
    const DWORD CRE_V22_OFFSETS_OFFSET = 8 + CRE_V22_HEADER_SIZE;
    const DWORD CRE_V22_DATA_OFFSET = 0x62E;
    const DWORD CRE_EFFECT_SIZE = 0x108;

    DWORD nSize = CRE_V22_DATA_OFFSET;

    for (UINT nClass = 0; nClass < CSPELLLIST_NUM_CLASSES; nClass++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            nSize += m_spells.m_spellsByClass[nClass].m_lists[nLevel].m_List.size() * sizeof(CCreatureFileSpell)
                + 2 * sizeof(UINT);
        }
    }

    for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        nSize += m_domainSpells.m_lists[nLevel].m_List.size() * sizeof(CCreatureFileSpell)
            + 2 * sizeof(UINT);
    }

    nSize += m_innateSpells.m_List.size() * sizeof(CCreatureFileSpell) + 2 * sizeof(UINT);
    nSize += m_songs.m_List.size() * sizeof(CCreatureFileSpell) + 2 * sizeof(UINT);
    nSize += m_shapeshifts.m_List.size() * sizeof(CCreatureFileSpell) + 2 * sizeof(UINT);
    // Equipment header + inventory item records (CGameSpriteEquipment::Marshal
    // at 0x7124C0 builds both; nItemCount excludes the fist slot).
    CCreatureFileEquipment equipment;
    CCreatureFileItem* pItems = NULL;
    LONG nItemCount = 0;
    m_equipment.Marshal(&equipment, &pItems, &nItemCount, a5);
    nSize += sizeof(CCreatureFileEquipment) + nItemCount * sizeof(CCreatureFileItem);

    // Active (timed) effects, serialised as version-2 (0x108-byte) records.
    BYTE* pEffectData = NULL;
    ULONG nEffectBytes = m_timedEffectList.Marshal(&pEffectData, 1, a4 == FALSE);
    DWORD nEffectCount = nEffectBytes / CRE_EFFECT_SIZE;
    nSize += nEffectCount * CRE_EFFECT_SIZE;

    *pCreature = new BYTE[nSize];
    *creatureSize = nSize;
    memset(*pCreature, 0, nSize);

    memcpy(*pCreature, "CRE V2.2", 8);

    CCreatureFileHeader header = m_baseStats;
    header.m_subrace = m_startTypeAI.GetSubRace();
    header.m_bRemoveFromArea = m_removeFromArea;
    DWORD nHeaderCopySize = sizeof(header);
    if (nHeaderCopySize > CRE_V22_HEADER_SIZE) {
        nHeaderCopySize = CRE_V22_HEADER_SIZE;
    }
    memcpy(*pCreature + 8, &header, nHeaderCopySize);

    CCreatureFileOffsets* offsets = reinterpret_cast<CCreatureFileOffsets*>(*pCreature + CRE_V22_OFFSETS_OFFSET);
    offsets->m_enemyAlly = m_startTypeAI.m_nEnemyAlly;
    offsets->m_general = m_startTypeAI.m_nGeneral;
    offsets->m_race = m_startTypeAI.m_nRace;
    offsets->m_class = m_startTypeAI.m_nClass;
    offsets->m_specifics = m_startTypeAI.m_nSpecific;
    offsets->m_gender = m_startTypeAI.m_nGender;
    memcpy(offsets->m_specialCase, m_startTypeAI.m_SpecialCase, sizeof(offsets->m_specialCase));
    offsets->m_alignment = m_startTypeAI.m_nAlignment;
    offsets->m_instance = m_startTypeAI.m_nInstance;
    strncpy(offsets->m_name, m_scriptName, SCRIPTNAME_SIZE);
    offsets->m_avClass = m_startTypeAI.m_nAvClass;
    offsets->m_classMask = m_startTypeAI.m_nClassMask;
    m_dialog.GetResRef(offsets->m_dialog);

    *facing = m_nDirection;

    DWORD nOffset = CRE_V22_DATA_OFFSET;

    for (UINT nClass = 0; nClass < CSPELLLIST_NUM_CLASSES; nClass++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            CGameSpriteSpellList& list = m_spells.m_spellsByClass[nClass].m_lists[nLevel];
            offsets->m_spellListOffset[nClass][nLevel] = nOffset;
            offsets->m_spellListCount[nClass][nLevel] = list.m_List.size();

            for (UINT nIndex = 0; nIndex < list.m_List.size(); nIndex++) {
                CCreatureFileSpell* pSpell = reinterpret_cast<CCreatureFileSpell*>(*pCreature + nOffset);
                pSpell->field_0 = list.m_List[nIndex].m_nID;
                pSpell->m_nMax = list.m_List[nIndex].m_nMax;
                pSpell->m_nCurrent = list.m_List[nIndex].m_nCurrent;
                pSpell->field_C = list.m_List[nIndex].m_nShared;
                nOffset += sizeof(CCreatureFileSpell);
            }

            *reinterpret_cast<UINT*>(*pCreature + nOffset) = list.m_nSharedMax;
            nOffset += sizeof(UINT);
            *reinterpret_cast<UINT*>(*pCreature + nOffset) = list.m_nSharedTotal;
            nOffset += sizeof(UINT);
        }
    }

    for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        CGameSpriteSpellList& list = m_domainSpells.m_lists[nLevel];
        offsets->m_domainListOffset[nLevel] = nOffset;
        offsets->m_domainListCount[nLevel] = list.m_List.size();

        for (UINT nIndex = 0; nIndex < list.m_List.size(); nIndex++) {
            CCreatureFileSpell* pSpell = reinterpret_cast<CCreatureFileSpell*>(*pCreature + nOffset);
            pSpell->field_0 = list.m_List[nIndex].m_nID;
            pSpell->m_nMax = list.m_List[nIndex].m_nMax;
            pSpell->m_nCurrent = list.m_List[nIndex].m_nCurrent;
            pSpell->field_C = list.m_List[nIndex].m_nShared;
            nOffset += sizeof(CCreatureFileSpell);
        }

        *reinterpret_cast<UINT*>(*pCreature + nOffset) = list.m_nSharedMax;
        nOffset += sizeof(UINT);
        *reinterpret_cast<UINT*>(*pCreature + nOffset) = list.m_nSharedTotal;
        nOffset += sizeof(UINT);
    }

    CGameSpriteSpellList* extraLists[3] = { &m_innateSpells, &m_songs, &m_shapeshifts };
    DWORD* extraOffsets[3] = { &offsets->m_innateListOffset, &offsets->m_songListOffset, &offsets->m_shapeListOffset };
    DWORD* extraCounts[3] = { &offsets->m_innateListCount, &offsets->m_songListCount, &offsets->m_shapeListCount };

    for (INT nList = 0; nList < 3; nList++) {
        CGameSpriteSpellList& list = *extraLists[nList];
        *extraOffsets[nList] = nOffset;
        *extraCounts[nList] = list.m_List.size();

        for (UINT nIndex = 0; nIndex < list.m_List.size(); nIndex++) {
            CCreatureFileSpell* pSpell = reinterpret_cast<CCreatureFileSpell*>(*pCreature + nOffset);
            pSpell->field_0 = list.m_List[nIndex].m_nID;
            pSpell->m_nMax = list.m_List[nIndex].m_nMax;
            pSpell->m_nCurrent = list.m_List[nIndex].m_nCurrent;
            pSpell->field_C = list.m_List[nIndex].m_nShared;
            nOffset += sizeof(CCreatureFileSpell);
        }

        *reinterpret_cast<UINT*>(*pCreature + nOffset) = list.m_nSharedMax;
        nOffset += sizeof(UINT);
        *reinterpret_cast<UINT*>(*pCreature + nOffset) = list.m_nSharedTotal;
        nOffset += sizeof(UINT);
    }

    offsets->m_equipmentListOffset = nOffset;
    memcpy(*pCreature + nOffset, &equipment, sizeof(CCreatureFileEquipment));
    nOffset += sizeof(CCreatureFileEquipment);

    if (nItemCount != 0) {
        offsets->m_itemListOffset = nOffset;
        offsets->m_itemListCount = nItemCount;
        memcpy(*pCreature + nOffset, pItems, nItemCount * sizeof(CCreatureFileItem));
        nOffset += nItemCount * sizeof(CCreatureFileItem);
        delete[] pItems;
    }

    if (nEffectCount != 0) {
        offsets->m_effectListCount = nEffectCount;
        offsets->m_effectListOffset = nOffset;
        memcpy(*pCreature + nOffset, pEffectData, nEffectCount * CRE_EFFECT_SIZE);
        nOffset += nEffectCount * CRE_EFFECT_SIZE;
        delete[] reinterpret_cast<CGameEffectBase*>(pEffectData);
    }

    UTIL_ASSERT(nOffset == static_cast<DWORD>(*creatureSize));

    // TODO: 0x70B2F0 also temporarily shifts the sprite position fields around
    // the body and calls RemoveAllOfType(0xBA) on the equiped + timed effect
    // lists afterwards; not yet ported (does not affect the saved size).
}

// 0x70BEE0
void CGameSprite::Marshal(CSavedGamePartyCreature& partyCreature, BOOLEAN bNetworkMessage)
{
    DWORD nIndex;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 12226
    UTIL_ASSERT(m_bGlobal);

    partyCreature.m_wFlags = 0;

    if (m_bSelected) {
        partyCreature.m_wFlags |= 0x1;
    }

    if (bNetworkMessage == TRUE) {
        partyCreature.m_wFlags |= 0x8000;
    }

    partyCreature.m_portraitId = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id);

    memset(partyCreature.m_creatureResRef, 0, RESREF_SIZE);

    BYTE* pCreature;
    LONG nCreatureSize;
    WORD facing;
    Marshal(&pCreature, &nCreatureSize, &facing, TRUE, TRUE);

    // FIXME: Unsafe x64 conversion.
    partyCreature.m_creatureOffset = reinterpret_cast<DWORD>(pCreature);
    partyCreature.m_creatureSize = nCreatureSize;
    partyCreature.m_creatureFacing = facing;

    BOOLEAN v1 = FALSE;
    for (INT nCharacterSlot = 0; nCharacterSlot < 6; nCharacterSlot++) {
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(nCharacterSlot) == m_id) {
            v1 = TRUE;
            break;
        }
    }

    if (m_pArea != NULL) {
        m_pArea->m_resRef.GetResRef(partyCreature.m_areaName);

        INT x;
        INT y;
        m_pArea->GetInfinity()->GetViewPosition(x, y);
        partyCreature.m_posViewX = static_cast<WORD>(x);
        partyCreature.m_posViewY = static_cast<WORD>(y);
    } else {
        if (v1 == TRUE) {
            CResRef sBlankResRef;
            sBlankResRef = "NO_AREA";

            partyCreature.m_wFlags |= 0x8000;
            sBlankResRef.GetResRef(partyCreature.m_areaName);
        } else {
            m_currentArea.GetResRef(partyCreature.m_areaName);
        }

        partyCreature.m_posViewX = 0;
        partyCreature.m_posViewY = 0;
    }

    partyCreature.m_posX = static_cast<WORD>(m_pos.x);
    partyCreature.m_posY = static_cast<WORD>(m_pos.y);
    partyCreature.m_nModalState = m_nModalState;

    for (nIndex = 0; nIndex < 5; nIndex++) {
        partyCreature.m_nFeatRanks[nIndex] = m_nFeatRanks[nIndex];
    }

    partyCreature.m_nWeaponSet = m_nWeaponSet;

    for (nIndex = 0; nIndex < 8; nIndex++) {
        partyCreature.m_quickWeaponsItemNum[nIndex] = m_quickWeapons[nIndex].m_abilityId.m_itemNum;
        partyCreature.m_quickWeaponsAbilityNum[nIndex] = m_quickWeapons[nIndex].m_abilityId.m_abilityNum;
        partyCreature.field_2A7[nIndex] = field_3D3A[nIndex];
    }

    for (nIndex = 0; nIndex < 9; nIndex++) {
        m_quickSpells[nIndex].m_abilityId.m_res.GetResRef(partyCreature.m_quickSpellsSpellId[nIndex]);
        partyCreature.m_quickSpellsClass[nIndex] = m_quickSpells[nIndex].m_abilityId.m_nClass;
        partyCreature.field_280[nIndex] = m_quickSpells[nIndex].m_abilityId.m_bCanUse;
        partyCreature.field_29E[nIndex] = static_cast<unsigned char>(m_quickSpells[nIndex].m_abilityId.m_nTooltip);
    }

    for (nIndex = 0; nIndex < 3; nIndex++) {
        partyCreature.m_quickItemsItemNum[nIndex] = m_quickItems[nIndex].m_abilityId.m_itemNum;
        partyCreature.m_quickItemsAbilityNum[nIndex] = m_quickItems[nIndex].m_abilityId.m_abilityNum;
    }

    for (nIndex = 0; nIndex < 9; nIndex++) {
        m_quickInnates[nIndex].m_abilityId.m_res.GetResRef(partyCreature.m_quickInnatesSpellId[nIndex]);
    }

    for (nIndex = 0; nIndex < 9; nIndex++) {
        m_quickSongs[nIndex].m_abilityId.m_res.GetResRef(partyCreature.m_quickSongsSpellId[nIndex]);
    }

    for (nIndex = 0; nIndex < 9; nIndex++) {
        partyCreature.field_19A[nIndex] = field_3D14[nIndex];
    }

    partyCreature.m_nLastSpellbookClassIndex = m_nLastSpellbookClassIndex;
    partyCreature.m_nLastSpellbookSpellLevel = m_nLastSpellbookSpellLevel;
    partyCreature.m_nLastSong = m_nLastSong;
    partyCreature.m_strStrongestKillName = m_cGameStats.m_strStrongestKillName;
    partyCreature.m_nStrongestKillXPValue = m_cGameStats.m_nStrongestKillXPValue;
    partyCreature.m_nPreviousTimeWithParty = m_cGameStats.m_nPreviousTimeWithParty;
    partyCreature.m_nJoinPartyTime = m_cGameStats.m_nJoinPartyTime;
    partyCreature.m_bWithParty = m_cGameStats.m_bWithParty;
    partyCreature.m_nChapterKillsXPValue = m_cGameStats.m_nChapterKillsXPValue;
    partyCreature.m_nChapterKillsNumber = m_cGameStats.m_nChapterKillsNumber;
    partyCreature.m_nGameKillsXPValue = m_cGameStats.m_nGameKillsXPValue;
    partyCreature.m_nGameKillsNumber = m_cGameStats.m_nGameKillsNumber;

    for (nIndex = 0; nIndex < 4; nIndex++) {
        m_cGameStats.m_pSpellStats[nIndex].m_cResRef.GetResRef(partyCreature.m_lSpellStatsName[nIndex]);
        partyCreature.m_lSpellStatsCount[nIndex] = m_cGameStats.m_pSpellStats[nIndex].m_nTimesUsed;
    }

    for (nIndex = 0; nIndex < 4; nIndex++) {
        m_cGameStats.m_pWeaponStats[nIndex].m_cResRef.GetResRef(partyCreature.m_lWeaponStatsName[nIndex]);
        partyCreature.m_lWeaponStatsCount[nIndex] = m_cGameStats.m_pWeaponStats[nIndex].m_nTimesUsed;
    }

    partyCreature.m_nHappiness = m_nHappiness;

    for (nIndex = 0; nIndex < 24; nIndex++) {
        partyCreature.m_nNumberOfTimesInteractedWith[nIndex] = m_nNumberOfTimesInteractedWith[nIndex];
    }

    if (m_baseStats.m_name == -1) {
        strncpy(partyCreature.m_name, (LPCSTR)m_sName, SCRIPTNAME_SIZE);
    } else {
        partyCreature.m_name[0] = '\0';
    }

    partyCreature.m_nNumberOfTimesTalkedTo = m_nNumberOfTimesTalkedTo;
    m_secondarySounds.GetResRef(partyCreature.m_secondarySounds);
    memcpy(partyCreature.field_25E, field_725A, 32);
}

// 0x70C3F0
void CGameSprite::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    CResRef spriteResRef;
    CString sSpriteString;
    BYTE nSpriteStringLength;
    CResRef areaResRef;
    CString sAreaString;
    BYTE nAreaStringLength;
    CPoint pos;
    BYTE enemyAlly;
    DWORD cnt;

    spriteResRef = m_resRef;
    spriteResRef.CopyToString(sSpriteString);
    nSpriteStringLength = static_cast<BYTE>(sSpriteString.GetLength());

    areaResRef = m_pArea->m_resRef;
    areaResRef.CopyToString(sAreaString);
    nAreaStringLength = static_cast<BYTE>(sAreaString.GetLength());

    pos = GetPos();
    enemyAlly = m_startTypeAI.GetEnemyAlly();

    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 12428
    UTIL_ASSERT(nAreaStringLength != 0);

    *dwSize = sizeof(BYTE)
        + nSpriteStringLength
        + sizeof(BYTE)
        + nAreaStringLength
        + sizeof(LONG)
        + sizeof(LONG)
        + sizeof(BYTE);

    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 12440
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE_CGAMESPRITE);

    cnt = 0;

    *reinterpret_cast<BYTE*>(*pData + cnt) = nSpriteStringLength;
    cnt += sizeof(BYTE);

    memcpy(*pData + cnt, sSpriteString.GetBuffer(nSpriteStringLength), nSpriteStringLength);
    cnt += nSpriteStringLength;

    *reinterpret_cast<BYTE*>(*pData + cnt) = nAreaStringLength;
    cnt += sizeof(BYTE);

    memcpy(*pData + cnt, sSpriteString.GetBuffer(nAreaStringLength), nAreaStringLength);
    cnt += nAreaStringLength;

    *reinterpret_cast<LONG*>(*pData + cnt) = pos.x;
    cnt += sizeof(LONG);

    *reinterpret_cast<LONG*>(*pData + cnt) = pos.y;
    cnt += sizeof(LONG);

    *reinterpret_cast<BYTE*>(*pData + cnt) = enemyAlly;
    cnt += sizeof(BYTE);

    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 12481
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x70C600
void CGameSprite::Unmarshal(CSavedGamePartyCreature* pCreature, BOOLEAN bPartyMember, BOOLEAN bProgressBarInPlace)
{
    DWORD nIndex;

    m_bInUnmarshal = TRUE;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    m_bGlobal = TRUE;
    m_currentArea = pCreature->m_areaName;

    // NOTE: Uninline.
    SetFacing(static_cast<SHORT>(pCreature->m_creatureFacing));

    m_nModalState = static_cast<BYTE>(pCreature->m_nModalState);

    for (nIndex = 0; nIndex < 5; nIndex++) {
        m_nFeatRanks[nIndex] = pCreature->m_nFeatRanks[nIndex];
    }

    RefreshCombatFeats();

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22; nIndex++) {
        InitQuickItemData(pCreature->m_quickWeaponsItemNum[nIndex],
            pCreature->m_quickWeaponsAbilityNum[nIndex],
            pCreature->field_2A7[nIndex],
            m_quickWeapons[nIndex]);

        // NOTE: Uninline.
        SetQuickWeapon(static_cast<BYTE>(nIndex), pCreature->field_2A7[nIndex]);
    }

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_QUICK_SPELLS22; nIndex++) {
        if (pCreature->m_quickSpellsClass[nIndex] != 0) {
            InitQuickSpellData(CResRef(pCreature->m_quickSpellsSpellId[nIndex]),
                1,
                m_quickSpells[nIndex],
                pCreature->m_quickSpellsClass[nIndex],
                pCreature->field_280[nIndex],
                pCreature->field_29E[nIndex]);
        }
    }

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_QUICK_ITEMS22; nIndex++) {
        InitQuickItemData(pCreature->m_quickItemsItemNum[nIndex],
            pCreature->m_quickItemsAbilityNum[nIndex],
            0,
            m_quickItems[nIndex]);
    }

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_QUICK_ABILITIES22; nIndex++) {
        InitQuickSpellData(CResRef(pCreature->m_quickInnatesSpellId[nIndex]),
            2,
            m_quickInnates[nIndex],
            0,
            0,
            0);
    }

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_QUICK_SONGS22; nIndex++) {
        InitQuickSpellData(CResRef(pCreature->m_quickSongsSpellId[nIndex]),
            3,
            m_quickSongs[nIndex],
            0,
            0,
            0);
    }

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_CUSTOM_BUTTONS22; nIndex++) {
        field_3D14[nIndex] = pCreature->field_19A[nIndex];
    }

    m_nLastSpellbookClassIndex = pCreature->m_nLastSpellbookClassIndex;
    m_nLastSpellbookSpellLevel = pCreature->m_nLastSpellbookSpellLevel;
    m_nLastSong = pCreature->m_nLastSong;

    m_nNumberOfTimesTalkedTo = pCreature->m_nNumberOfTimesTalkedTo;
    m_nHappiness = pCreature->m_nHappiness;

    for (nIndex = 0; nIndex < 24; nIndex++) {
        m_nNumberOfTimesInteractedWith[nIndex] = pCreature->m_nNumberOfTimesInteractedWith[nIndex];
    }

    if (pCreature->m_strStrongestKillName == 0) {
        pCreature->m_strStrongestKillName = -1;
    }

    m_cGameStats.m_strStrongestKillName = pCreature->m_strStrongestKillName;
    m_cGameStats.m_nStrongestKillXPValue = pCreature->m_nStrongestKillXPValue;
    m_cGameStats.m_nPreviousTimeWithParty = pCreature->m_nPreviousTimeWithParty;
    m_cGameStats.m_nJoinPartyTime = pCreature->m_nJoinPartyTime;
    m_cGameStats.m_bWithParty = pCreature->m_bWithParty;
    m_cGameStats.m_nChapterKillsXPValue = pCreature->m_nChapterKillsXPValue;
    m_cGameStats.m_nChapterKillsNumber = pCreature->m_nChapterKillsNumber;
    m_cGameStats.m_nGameKillsXPValue = pCreature->m_nGameKillsXPValue;
    m_cGameStats.m_nGameKillsNumber = pCreature->m_nGameKillsNumber;

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_STATS_SPELLS; nIndex++) {
        m_cGameStats.SetSpellStats(static_cast<BYTE>(nIndex),
            pCreature->m_lSpellStatsName[nIndex],
            pCreature->m_lSpellStatsCount[nIndex]);
    }

    for (nIndex = 0; nIndex < CGAMESAVECHARACTER_NUM_STATS_WEAPONS; nIndex++) {
        m_cGameStats.SetWeaponStats(static_cast<BYTE>(nIndex),
            pCreature->m_lWeaponStatsName[nIndex],
            pCreature->m_lWeaponStatsCount[nIndex]);
    }

    m_secondarySounds = pCreature->m_secondarySounds;
    memcpy(field_725A, pCreature->field_25E, sizeof(field_725A));

    CString sPath = g_pBaldurChitin->GetObjectGame()->GetDirSounds() + field_725A + '\\';
    g_pBaldurChitin->cDimm.AddToDirectoryList(sPath, TRUE);

    if (bPartyMember) {
        CString areaName;
        m_currentArea.CopyToString(areaName);

        CGameArea* pArea;
        if ((pCreature->m_wFlags & 0x8000) == 0) {
            pArea = pGame->LoadArea(areaName,
                255,
                FALSE,
                bProgressBarInPlace);
            if (pArea == NULL) {
                return;
            }

            pArea->GetInfinity()->SetViewPosition(pCreature->m_posViewX,
                pCreature->m_posViewY,
                TRUE);
        } else {
            if (bProgressBarInPlace) {
                g_pChitin->cProgressBar.AddActionTarget(-static_cast<LONG>(CInfGame::PROGRESSBAR_CACHING_ADDITIONAL));
            }
        }

        if (pGame->m_bInLoadGame == TRUE) {
            pGame->AddCharacterToParty(m_id, pCreature->m_portraitId);
        }

        CPoint pos;
        pos.x = pCreature->m_posX != -1 ? pCreature->m_posX : -1;
        pos.y = pCreature->m_posY != -1 ? pCreature->m_posY : -1;

        if ((pCreature->m_wFlags & 0x8000) == 0) {
            if ((m_baseStats.m_generalState & STATE_DEAD) != 0
                && GetAnimation()->CanLieDown()) {
                AddToArea(pArea, pos, 0, LIST_BACK);
            } else {
                AddToArea(pArea, pos, 0, LIST_FRONT);
            }

            if ((pCreature->m_wFlags & 0x1) != 0) {
                pGame->SelectCharacter(m_id, FALSE);
            }
        }

        if (m_baseStats.m_name == -1) {
            m_sName = pCreature->m_name;
        }

        if (m_baseStats.m_resistMagicBase < 0) {
            m_baseStats.m_resistMagicBase = 0;
        } else if (m_baseStats.m_resistMagicBase > 50) {
            m_baseStats.m_resistMagicBase = 50;
        }

        sub_71E760(m_derivedStats, TRUE);

        for (UINT nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
            for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
                BYTE nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nClassIndex);

                INT nBonus;
                INT nMaxSpells = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetMaxKnownSpells(nClass,
                    m_startTypeAI,
                    m_derivedStats,
                    m_baseStats.m_specialization,
                    nLevel + 1,
                    nBonus);

                // __FILE__: .\Include\ObjCreature.h
                // __LINE__: 1751
                UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

                m_spells.m_spellsByClass[nClassIndex].m_lists[nLevel].m_nSharedMax = nMaxSpells + nBonus;
            }
        }
    } else {
        m_pos.x = pCreature->m_posX;
        m_pos.y = pCreature->m_posY;
        if (m_baseStats.m_name == -1) {
            m_sName = pCreature->m_name;
        }
    }

    SetWeaponSet(pCreature->m_nWeaponSet);

    INT nDruidLevel = m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_DRUID);
    if (nDruidLevel > 0) {
        INT nMaxShapeshifts = pGame->GetRuleTables().GetMaxDruidShapeshifts(m_baseStats, nDruidLevel) - m_shapeshifts.m_nSharedMax;
        if (nMaxShapeshifts) {
            AllocateShapeshiftSlots(nMaxShapeshifts);
            m_shapeshifts.AddToSharedCurrentCount(nMaxShapeshifts, FALSE);
        }
    }

    m_bInUnmarshal = FALSE;
}

// 0x70CF90
void CGameSprite::Unmarshal(BYTE* pCreature, LONG creatureSize, WORD facing, int a4)
{
    CCreatureFileOffsets* offsets;

    m_bInUnmarshal = TRUE;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 12738
    UTIL_ASSERT(pCreature != NULL);

    BOOL bNeedUpgrade = FALSE;
    BOOL bAlloced = FALSE;
    if (memcmp(pCreature, "CRE V2.2", 8) != 0) {
        bNeedUpgrade = TRUE;
    }

    if (bNeedUpgrade) {
        // TODO: Incomplete.
    }

    ClearMarshal(TRUE);
    creatureSize -= 900;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 12812
    UTIL_ASSERT(creatureSize > 0);

    memcpy(&m_baseStats, pCreature + 8, sizeof(m_baseStats));

    if (m_baseStats.m_scriptTeam[0] == 0xFF) {
        memcpy(m_baseStats.m_scriptTeam, "None", strlen("None") + 1);
    }

    if (m_baseStats.m_scriptSpecial1[0] == 0xFF) {
        memcpy(m_baseStats.m_scriptSpecial1, "None", strlen("None") + 1);
    }

    STR_RES nameRes;
    g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_name, nameRes);

    for (BYTE nRange = 0; nRange < 7; nRange++) {
        if (m_baseStats.m_colors[nRange] == 0xFF) {
            m_baseStats.m_colors[nRange] = 0;
        }
    }

    // NOTE: Uninline.
    GetAnimation()->SetAnimationType(m_baseStats.m_animationType, m_baseStats.m_colors, facing);

    if (GetAnimation()->GetListType() == LIST_FLIGHT) {
        // NOTE: Uninline.
        SetFacing(4 * ((facing + 2) / 4));
    }

    m_vbPortraitSmall.SetResRef(CResRef(m_baseStats.m_portraitSmall), TRUE, TRUE);
    m_vbPortraitSmall.m_bDoubleSize = FALSE;

    offsets = reinterpret_cast<CCreatureFileOffsets*>(pCreature + 900);
    creatureSize -= sizeof(CCreatureFileOffsets);

    if (m_baseStats.m_subrace == -1) {
        m_baseStats.m_subrace = 0;
    }

    if (offsets->m_class == 0 || offsets->m_class == -1) {
        offsets->m_class = CAIOBJECTTYPE_C_FIGHTER;
        offsets->m_avClass = CAIOBJECTTYPE_C_FIGHTER;
        offsets->m_classMask = CLASSMASK_FIGHTER;
    }

    m_derivedStats.m_classMask = 0;
    for (INT iClassType = 1; iClassType <= 11; iClassType++) {
        INT nLevel = GetClassLevel(iClassType);
        if (nLevel > 0) {
            m_derivedStats.SetClassLevel(iClassType, nLevel);
        }
    }

    SetAIType(CAIObjectType(offsets->m_enemyAlly,
                  offsets->m_general,
                  offsets->m_race,
                  m_baseStats.m_subrace,
                  offsets->m_class,
                  offsets->m_specifics,
                  offsets->m_gender,
                  offsets->m_alignment,
                  m_id,
                  offsets->m_specialCase,
                  CString(offsets->m_name),
                  offsets->m_avClass,
                  offsets->m_classMask),
        TRUE, TRUE);

    if ((m_baseStats.m_generalState & STATE_DEAD) != 0) {
        m_startTypeAI.m_nGeneral = CAIObjectType::G_DEAD;
        m_typeAI.m_nGeneral = CAIObjectType::G_DEAD;
    }

    strncpy(m_scriptName, offsets->m_name, SCRIPTNAME_SIZE);

    if (m_baseStats.m_attackBase == 0) {
        int v1;
        int v2;
        int v3;
        g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetBaseCombatValues(this,
            v1,
            v2,
            v3,
            FALSE);
        m_baseStats.m_attackBase = v1;
    }

    if (m_baseStats.m_numberOfAttacksBase == 0) {
        int v1;
        int v2;
        int v3;
        g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetBaseCombatValues(this,
            v1,
            v2,
            v3,
            FALSE);
        m_baseStats.m_numberOfAttacksBase = v2;
    }

    m_typeAI.m_nSubRace = m_baseStats.m_subrace;
    m_startTypeAI.m_nSubRace = m_baseStats.m_subrace;
    m_liveTypeAI.m_nSubRace = m_baseStats.m_subrace;

    BYTE* pEffectData = pCreature + offsets->m_effectListOffset;
    BOOL bEffectListIsBase = offsets->m_effectListCount != 0
        && (memcmp(pEffectData, "EFF V2.0", 8) == 0
            || memcmp(pEffectData, "EFF V2.1", 8) == 0
            || memcmp(pEffectData, "EFF V2.2", 8) == 0);

    if (bEffectListIsBase) {
        m_baseStats.m_effectVersion = 1;
        m_timedEffectList.Unmarshal(pEffectData,
            sizeof(CGameEffectBase) * offsets->m_effectListCount,
            this,
            1);
        creatureSize -= sizeof(CGameEffectBase) * offsets->m_effectListCount;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 12920
        UTIL_ASSERT(creatureSize > 0);
    } else {
        m_baseStats.m_effectVersion = 0;
        m_timedEffectList.Unmarshal(pEffectData,
            sizeof(ITEM_EFFECT) * offsets->m_effectListCount,
            this,
            0);
        creatureSize -= sizeof(ITEM_EFFECT) * offsets->m_effectListCount;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 12914
        UTIL_ASSERT(creatureSize > 0);
    }

    m_equipment.Unmarshal(reinterpret_cast<CCreatureFileEquipment*>(pCreature + offsets->m_equipmentListOffset),
        reinterpret_cast<CCreatureFileItem*>(pCreature + offsets->m_itemListOffset),
        offsets->m_itemListCount,
        this);
    creatureSize -= sizeof(CCreatureFileEquipment) + sizeof(CCreatureFileItem) * offsets->m_itemListCount;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 12925
    UTIL_ASSERT(creatureSize > 0);

    if (IcewindMisc::IsPC(this)) {
        if (m_equipment.m_selectedWeapon >= 43) {
            m_nWeaponSet = (m_equipment.m_selectedWeapon - 43) / 2;
        }
        SetWeaponSet(m_nWeaponSet);
    }

    for (INT buttonNum = 0; buttonNum < g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetNumQuickWeaponSlots(m_typeAI.m_nClass); buttonNum++) {
        INT abilityNum = buttonNum + 43 == m_equipment.m_selectedWeapon
            ? m_equipment.m_selectedWeaponAbility
            : 0;

        CGameButtonList* buttons = GetItemUsages(buttonNum + 43, TRUE, abilityNum);
        if (!buttons->IsEmpty()) {
            // NOTE: Uninline.
            SetQuickWeapon(static_cast<BYTE>(buttonNum), *buttons->GetHead());

            while (!buttons->IsEmpty()) {
                delete buttons->RemoveHead();
            }
        }
        delete buttons;
    }

    CCreatureFileSpell* pSpell = new CCreatureFileSpell();
    DWORD nOffset;
    DWORD nIndex;
    UINT nClass;
    UINT nLevel;

    for (nClass = 0; nClass < CSPELLLIST_NUM_CLASSES; nClass++) {
        for (nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            // Spell offsets start at struct+0x36 (index 0 = class mask, skip it)
            DWORD* pOffsets = &offsets->m_classMask;
            nOffset = pOffsets[nClass * 9 + nLevel + 1];

            // Spell counts at CRE+0x4B6 (Bard spells 1 count = 1st entry)
            DWORD* pCounts = reinterpret_cast<DWORD*>(pCreature + 0x4B6);
            DWORD nCount = pCounts[nClass * 9 + nLevel];


            for (nIndex = 0; nIndex < nCount; nIndex++) {
                *pSpell = *reinterpret_cast<CCreatureFileSpell*>(pCreature + nOffset);
                nOffset += sizeof(CCreatureFileSpell);

                const CResRef& resRef = g_pBaldurChitin->GetObjectGame()->m_spells.Get(pSpell->field_0);
                if (memcmp(resRef.GetResRef(), "**", strlen("**")) == 0) {
                    CString sError;
                    sError.Format("*** %s has OBSOLETE DOMAIN SPELL FILE: %s.\n",
                        m_scriptName,
                        (LPCSTR)resRef.GetResRefStr());
                } else {
                    // NOTE: Uninline.
                    m_spells.Get(nClass)->Add(pSpell->field_0,
                        nLevel,
                        pSpell->m_nMax,
                        pSpell->m_nCurrent,
                        pSpell->field_C);
                }
            }
            creatureSize -= sizeof(CCreatureFileSpell) * nCount;

            // NOTE: There are some inlining.

            m_spells.m_spellsByClass[nClass].m_lists[nLevel].m_nSharedMax = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
            nOffset += sizeof(unsigned int);
            creatureSize -= sizeof(unsigned int);

            m_spells.m_spellsByClass[nClass].m_lists[nLevel].m_nSharedTotal = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
            nOffset += sizeof(unsigned int);
            creatureSize -= sizeof(unsigned int);
        }
    }
    for (nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        nOffset = offsets->m_domainListOffset[nLevel];
        for (nIndex = 0; nIndex < offsets->m_domainListCount[nLevel]; nIndex++) {
            *pSpell = *reinterpret_cast<CCreatureFileSpell*>(pCreature + nOffset);
            nOffset += sizeof(CCreatureFileSpell);

            const CResRef& resRef = g_pBaldurChitin->GetObjectGame()->m_spells.Get(pSpell->field_0);
            if (memcmp(resRef.GetResRef(), "**", strlen("**")) == 0) {
                CString sError;
                sError.Format("*** %s has OBSOLETE DOMAIN SPELL FILE: %s.\n",
                    m_scriptName,
                    (LPCSTR)resRef.GetResRefStr());
            } else {
                // NOTE: Uninline.
                m_domainSpells.Add(pSpell->field_0,
                    nLevel,
                    pSpell->m_nMax,
                    pSpell->m_nCurrent,
                    pSpell->field_C);
            }
        }
        creatureSize -= sizeof(CCreatureFileSpell) * offsets->m_domainListCount[nLevel];

        m_domainSpells.m_lists[nLevel].m_nSharedMax = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
        nOffset += sizeof(unsigned int);
        creatureSize -= sizeof(unsigned int);

        m_domainSpells.m_lists[nLevel].m_nSharedTotal = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
        nOffset += sizeof(unsigned int);
        creatureSize -= sizeof(unsigned int);
    }
    nOffset = offsets->m_innateListOffset;
    for (nIndex = 0; nIndex < offsets->m_innateListCount; nIndex++) {
        *pSpell = *reinterpret_cast<CCreatureFileSpell*>(pCreature + nOffset);
        nOffset += sizeof(CCreatureFileSpell);

        const CResRef& resRef = g_pBaldurChitin->GetObjectGame()->GetInnateSpells()->Get(pSpell->field_0);
        if (memcmp(resRef.GetResRef(), "**", strlen("**")) == 0) {
            CString sError;
            sError.Format("*** %s has OBSOLETE INNATE SPELL FILE: %s.\n",
                m_scriptName,
                (LPCSTR)resRef.GetResRefStr());
        } else {
            m_innateSpells.Add(pSpell->field_0,
                pSpell->m_nMax,
                pSpell->m_nCurrent,
                pSpell->field_C);
        }
    }
    creatureSize -= sizeof(CCreatureFileSpell) * offsets->m_innateListCount;

    m_innateSpells.m_nSharedMax = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
    nOffset += sizeof(unsigned int);
    creatureSize -= sizeof(unsigned int);

    m_innateSpells.m_nSharedTotal = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
    nOffset += sizeof(unsigned int);
    creatureSize -= sizeof(unsigned int);

    nOffset = offsets->m_songListOffset;
    for (nIndex = 0; nIndex < offsets->m_songListCount; nIndex++) {
        *pSpell = *reinterpret_cast<CCreatureFileSpell*>(pCreature + nOffset);
        nOffset += sizeof(CCreatureFileSpell);

        const CResRef& resRef = g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().Get(pSpell->field_0);
        if (memcmp(resRef.GetResRef(), "**", strlen("**")) == 0) {
            CString sError;
            sError.Format("*** %s has OBSOLETE SONG FILE: %s.\n",
                m_scriptName,
                (LPCSTR)resRef.GetResRefStr());
        } else {
            m_songs.Add(pSpell->field_0,
                pSpell->m_nMax,
                pSpell->m_nCurrent,
                pSpell->field_C);
        }
    }
    creatureSize -= sizeof(CCreatureFileSpell) * offsets->m_songListCount;

    m_songs.m_nSharedMax = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
    nOffset += sizeof(unsigned int);
    creatureSize -= sizeof(unsigned int);

    m_songs.m_nSharedTotal = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
    nOffset += sizeof(unsigned int);
    creatureSize -= sizeof(unsigned int);

    nOffset = offsets->m_shapeListOffset;
    for (nIndex = 0; nIndex < offsets->m_shapeListCount; nIndex++) {
        *pSpell = *reinterpret_cast<CCreatureFileSpell*>(pCreature + nOffset);
        nOffset += sizeof(CCreatureFileSpell);

        const CResRef& resRef = g_pBaldurChitin->GetObjectGame()->GetShapeshifts()->Get(pSpell->field_0);
        if (memcmp(resRef.GetResRef(), "**", strlen("**")) == 0) {
            CString sError;
            sError.Format("*** %s has OBSOLETE SHAPESHIFT FILE: %s.\n",
                m_scriptName,
                (LPCSTR)resRef.GetResRefStr());
        } else {
            m_shapeshifts.Add(pSpell->field_0,
                pSpell->m_nMax,
                pSpell->m_nCurrent,
                pSpell->field_C);
        }
    }
    creatureSize -= sizeof(CCreatureFileSpell) * offsets->m_shapeListCount;

    m_shapeshifts.m_nSharedMax = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
    nOffset += sizeof(unsigned int);
    creatureSize -= sizeof(unsigned int);

    m_shapeshifts.m_nSharedTotal = *reinterpret_cast<unsigned int*>(pCreature + nOffset);
    nOffset += sizeof(unsigned int);
    creatureSize -= sizeof(unsigned int);

    delete pSpell;

    for (BYTE nHatedRaceIndex = 0; nHatedRaceIndex < 8; nHatedRaceIndex++) {
        if (m_baseStats.m_favoredEnemies[nHatedRaceIndex] == 0) {
            m_baseStats.m_favoredEnemies[nHatedRaceIndex] = CAIObjectType::R_NO_RACE;
        }
    }

    m_derivedStats.Reload(this, &m_baseStats, &m_spells, &m_domainSpells);
    m_bInUnmarshal = FALSE;

    if (g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nNightmareMode == TRUE
        && (m_baseStats.field_2FB & 0x1) == 0
        && a4 == NULL) {
        BOOL bIncreaseStats = FALSE;
        if (m_type == 2) {
            m_baseStats.m_hitPoints = 2 * (m_baseStats.m_hitPoints + 10);
            m_baseStats.m_maxHitPointsBase = 2 * (m_baseStats.m_maxHitPointsBase + 10);
            bIncreaseStats = TRUE;
        } else {
            if (offsets->m_enemyAlly > CAIObjectType::EA_GOODCUTOFF) {
                m_baseStats.m_hitPoints = 3 * (m_baseStats.m_hitPoints + 80);
                m_baseStats.m_maxHitPointsBase = 3 * (m_baseStats.m_maxHitPointsBase + 80);
                bIncreaseStats = TRUE;
            }
        }

        if (bIncreaseStats) {
            // NOTE: Uninline.
            BYTE moveScale = GetAnimation()->GetMoveScale();

            // NOTE: Uninline.
            GetAnimation()->SetMoveScale(static_cast<BYTE>(static_cast<float>(moveScale) * 1.3f + 1.0f));

            if (m_baseStats.m_gold != 0) {
                m_baseStats.m_gold += 75;
            }

            for (INT iClassType = 1; iClassType <= 11; iClassType++) {
                INT nLevel = GetClassLevel(iClassType);
                if (nLevel > 0) {
                    SetClassLevel(iClassType, min(nLevel + 12, 30));
                }
            }

            m_baseStats.m_STRBase += 10;
            m_baseStats.m_INTBase += 10;
            m_baseStats.m_WISBase += 10;
            m_baseStats.m_DEXBase += 10;
            m_baseStats.m_CONBase += 10;
            m_baseStats.m_CHRBase += 10;
            m_baseStats.field_252 += 10;
            m_baseStats.field_2FB |= 0x1;
        }
    }

    m_baseStats.m_critSectService &= ~0x1;
    m_dialog = offsets->m_dialog;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 13227
    UTIL_ASSERT(creatureSize == 0);

    m_derivedStats.Reload(this, &m_baseStats, &m_spells, &m_domainSpells);
    m_tempStats = m_derivedStats;
    m_bonusStats.BonusInit();

    if (bAlloced) {
        delete pCreature;
    }
}

// 0x70E750
void CGameSprite::UnmarshalScripts()
{
    CString sFileName;

    sFileName = m_baseStats.m_scriptOverRide;
    if (sFileName != "") {
        m_overrideScript = new CAIScript(CResRef(sFileName));
        if (m_overrideScript->IsEmpty()) {
            delete m_overrideScript;
            m_overrideScript = NULL;
        }
    }

    sFileName = m_baseStats.m_scriptSpecial1;
    if (sFileName != "") {
        m_special1Script = new CAIScript(CResRef(sFileName));
        if (m_special1Script->IsEmpty()) {
            delete m_special1Script;
            m_special1Script = NULL;
        }
    }

    sFileName = m_baseStats.m_scriptTeam;
    if (sFileName != "") {
        m_teamScript = new CAIScript(CResRef(sFileName));
        if (m_teamScript->IsEmpty()) {
            delete m_teamScript;
            m_teamScript = NULL;
        }
    }

    sFileName = m_baseStats.m_scriptSpecial2;
    if (sFileName != "") {
        m_special2Script = new CAIScript;
        m_special2Script->Read(CResRef(sFileName), g_pBaldurChitin->GetObjectGame()->m_bPlayerScriptStyle);
        if (m_special2Script->IsEmpty()) {
            delete m_special2Script;
            m_special2Script = NULL;
        }
    }

    sFileName = m_baseStats.m_scriptCombat;
    if (sFileName != "") {
        m_combatScript = new CAIScript(CResRef(sFileName));
        if (m_combatScript->IsEmpty()) {
            delete m_combatScript;
            m_combatScript = NULL;
        }
    }

    sFileName = m_baseStats.m_scriptSpecial3;
    if (sFileName != "") {
        m_special3Script = new CAIScript(CResRef(sFileName));
        if (m_special3Script->IsEmpty()) {
            delete m_special3Script;
            m_special3Script = NULL;
        }
    }

    sFileName = m_baseStats.m_scriptMovement;
    if (sFileName != "") {
        m_movementScript = new CAIScript(CResRef(sFileName));
        if (m_movementScript->IsEmpty()) {
            delete m_movementScript;
            m_movementScript = NULL;
        }
    }
}

// 0x70EC00
void CGameSprite::FetchCommonStrings()
{
    int index;

    for (index = 0; index < 3; index++) {
        g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_speech[33 + index],
            m_speech[33 + index]);
    }

    for (index = 0; index < 7; index++) {
        g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_speech[15 + index],
            m_speech[15 + index]);
    }
}

// 0x70EC70
void CGameSprite::LoadAreaInformation(CAreaFileCreature* pCreature)
{
    m_nNumberOfTimesTalkedTo = pCreature->m_numberTimesTalkedTo;

    CString tempRes;

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_dialogOverride), RESREF_SIZE);
    if (tempRes != "") {
        m_dialog = tempRes;
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_overrideScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptOverRide);
        SetScript(0, pScript);
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_special3ScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptSpecial3);
        SetScript(5, pScript);
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_special2ScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptSpecial2);
        SetScript(3, pScript);
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_combatScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptCombat);
        SetScript(4, pScript);
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_movementScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptMovement);
        SetScript(6, pScript);
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_teamScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptTeam);
        SetScript(2, pScript);
    }

    tempRes = CString(reinterpret_cast<char*>(pCreature->m_special1ScriptOverride), RESREF_SIZE);
    tempRes.TrimLeft();
    if (tempRes != "") {
        CAIScript* pScript = new CAIScript(CResRef(tempRes));

        CResRef(tempRes).GetResRef(m_baseStats.m_scriptSpecial1);
        SetScript(1, pScript);
    }

    memcpy(m_scriptName, pCreature->m_scriptName, SCRIPTNAME_SIZE);
}

// 0x70F270
BYTE CGameSprite::GetChannel()
{
    SHORT nPortraitNum = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id);
    if (nPortraitNum != -1) {
        return nPortraitNum + 7;
    }
    return 13;
}

// 0x70F2A0
void CGameSprite::RenderMirrorImage(INT placement, CRect& rFX, CRect& rGCBounds, CRect& rViewRect, CPoint& ptReference, CSearchBitmap* pSearch, CVisibilityMap* pVisibility, CVidMode* pVidMode, INT nSurface, COLORREF& rgbTint, BOOLEAN& bDithered, BOOLEAN& bFadeOut, DWORD& dwRenderFlags)
{
    // 0x8B7308
    static int mirrorImagePlacementX[] = {
        -25,
        25,
        0,
        0,
        -18,
        18,
        18,
        -18,
    };

    // 0x8B7328
    static int mirrorImagePlacementY[] = {
        0,
        0,
        -25,
        25,
        -18,
        18,
        -18,
        18,
    };

    CPoint mirrorPos;
    CPoint mirrorSquare;
    SHORT searchSquareCode;
    CPoint pos;

    pos.x = m_pos.x + mirrorImagePlacementX[placement];
    pos.y = m_pos.y += mirrorImagePlacementY[placement];
    pos.y += m_pArea->GetHeightOffset(pos, m_listType);

    m_animation.CalculateGCBoundsRect(rGCBounds,
        pos,
        ptReference,
        m_posZ,
        rFX.Width(),
        rFX.Height());

    if (!IsRectEmpty(rViewRect & rGCBounds)) {
        mirrorPos.x = max(pos.x, m_pArea->GetInfinity()->nAreaX - 1) / CPathSearch::GRID_SQUARE_SIZEX;
        mirrorPos.y = max(pos.y, m_pArea->GetInfinity()->nAreaY - 1) / CPathSearch::GRID_SQUARE_SIZEY;
        if ((pSearch->GetLOSCost(mirrorPos, m_terrainTable, searchSquareCode, FALSE) != CPathSearch::COST_IMPASSABLE
                || searchSquareCode == 14)
            && pVisibility->IsTileExplored(pVisibility->PointToTile(mirrorPos))) {
            m_animation.Render(m_pArea->GetInfinity(),
                pVidMode,
                nSurface,
                rFX,
                mirrorPos,
                ptReference,
                dwRenderFlags | 0x2,
                rgbTint,
                rGCBounds,
                bDithered,
                bFadeOut,
                m_posZ,
                96);
        }
    }
}

// 0x713FE0
void CGameSprite::GetSelectedWeaponButton(CButtonData& cButtonData)
{
    const CRuleTables& rule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    cButtonData.m_icon = "NOICON";
    cButtonData.m_count = 0;
    cButtonData.m_abilityId.m_itemType = 2;
    cButtonData.m_abilityId.m_itemNum = 10;
    cButtonData.m_abilityId.m_abilityNum = 1;
    cButtonData.m_abilityId.m_strDescription = -1;

    CItem* pItem = m_equipment.m_items[m_nTempSelectedWeapon];
    if (pItem != NULL) {
        pItem->Demand();
        ITEM_ABILITY* pAbility = pItem->GetAbility(m_nTempSelectedWeaponAbility);
        if (pAbility != NULL) {
            cButtonData.m_icon = CString(pAbility->quickSlotIcon);
            cButtonData.m_abilityId.m_itemNum = m_nTempSelectedWeapon;
            cButtonData.m_abilityId.m_itemType = 2;
            cButtonData.m_abilityId.m_abilityNum = m_nTempSelectedWeaponAbility;
            cButtonData.m_abilityId.m_strDescription = rule.GetItemAbilityDescription(pItem->cResRef,
                m_nTempSelectedWeaponAbility);
            if (cButtonData.m_abilityId.m_strDescription == -1) {
                cButtonData.m_abilityId.m_strDescription = pItem->GetGenericName();
            }

            cButtonData.m_count = 0;
            if (pItem->GetMaxStackable() > 1) {
                cButtonData.m_count = pItem->GetUsageCount(m_nTempSelectedWeaponAbility);
            }
        }
        pItem->Release();
    }
}

// 0x714130
void CGameSprite::GetWeaponButton(BYTE nButtonNum, CButtonData& cButtonData)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 15766
    UTIL_ASSERT_MSG(nButtonNum < CCREATUREFILEEQUIPMENT21_WEAPON_NUM, "Invalid button number.");

    const CRuleTables& rule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    cButtonData.m_icon = "";
    cButtonData.m_count = 0;
    cButtonData.m_abilityId.m_itemType = 2;
    cButtonData.m_abilityId.m_itemNum = 10;
    cButtonData.m_abilityId.m_abilityNum = 1;
    cButtonData.m_abilityId.m_strDescription = -1;

    CItem* pItem = m_equipment.m_items[nButtonNum + 43];
    if (pItem != NULL) {
        pItem->Demand();
        ITEM_ABILITY* pAbility = pItem->GetAbility(field_3D3A[nButtonNum]);
        if (pAbility != NULL) {
            cButtonData.m_icon = CString(pAbility->quickSlotIcon);
            cButtonData.m_abilityId.m_itemType = 2;
            cButtonData.m_abilityId.m_itemNum = nButtonNum + 43;
            cButtonData.m_abilityId.m_abilityNum = field_3D3A[nButtonNum];
            cButtonData.m_abilityId.m_strDescription = rule.GetItemAbilityDescription(pItem->cResRef,
                field_3D3A[nButtonNum]);
            if (cButtonData.m_abilityId.m_strDescription == -1) {
                cButtonData.m_abilityId.m_strDescription = pItem->GetGenericName();
            }

            cButtonData.m_count = 0;
            if (pItem->GetMaxStackable() > 1) {
                cButtonData.m_count = pItem->GetUsageCount(field_3D3A[nButtonNum]);
            }
        }
        pItem->Release();
    }
}

// 0x7142D0
void CGameSprite::SetSelectedWeaponButton(SHORT buttonNum)
{
    if (m_equipment.m_items[42] != NULL) {
        g_pBaldurChitin->GetObjectGame()->SetState(2);
        g_pBaldurChitin->GetObjectGame()->SetIconIndex(12);
        g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
        g_pBaldurChitin->GetObjectGame()->field_38A6 = 1;
    } else {
        g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        SHORT itemNum = m_quickWeapons[buttonNum].m_abilityId.m_itemNum;
        SHORT abilityNum = m_quickWeapons[buttonNum].m_abilityId.m_abilityNum;
        if (itemNum != -1 && abilityNum != -1) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE_: 15877
            m_nTempSelectedWeapon = static_cast<BYTE>(itemNum);
            m_nTempSelectedWeaponAbility = static_cast<BYTE>(abilityNum);
            SelectWeaponAbility(static_cast<BYTE>(itemNum),
                static_cast<BYTE>(abilityNum),
                0,
                1);
            m_interrupt = TRUE;
            if (m_equipment.m_selectedWeapon == itemNum) {
                g_pBaldurChitin->GetObjectGame()->SetState(2);
                g_pBaldurChitin->GetObjectGame()->SetIconIndex(12);
                g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
            }
            g_pBaldurChitin->GetObjectGame()->field_38A6 = 1;
        }
    }
}

// 0x714410
void CGameSprite::InitQuickSpellData(CResRef resRef, BYTE type, CButtonData& cButtonData, BYTE nClass, BYTE a5, BYTE nKitIndex)
{
    CSpell cSpell;
    cSpell.SetResRef(resRef, TRUE, TRUE);
    BYTE count = 0;

    if (!resRef.IsValid()) {
        return;
    }

    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->m_shapeshifts.Find(resRef, nID) == TRUE) {
        type = 4;
    }

    UINT v1 = 0;
    UINT v2 = 0;
    UINT nClassIndex;

    switch (type) {
    case 1:
        if (!g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, v1)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 15953
            UTIL_ASSERT(FALSE);
        }

        nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);

        if (nKitIndex != 0) {
            if (nClass == CAIOBJECTTYPE_C_CLERIC) {
                BOOLEAN bFound = FALSE;
                for (UINT nLevel = 0; nLevel < m_domainSpells.m_nHighestLevel; nLevel++) {
                    if (!bFound) {
                        break;
                    }

                    if (m_domainSpells.Find(v1, nLevel, v2) == TRUE) {
                        bFound = TRUE;
                        count += m_domainSpells.m_lists[nLevel].Get(v2)->m_nCurrent;
                    }
                }
            }
        } else {
            BOOLEAN bFound = FALSE;
            for (UINT nLevel = 0; nLevel < m_spells.m_spellsByClass[nClass].m_nHighestLevel; nLevel++) {
                if (!bFound) {
                    break;
                }

                if (m_spells.m_spellsByClass[nClass].Find(v1, nLevel, v2) == TRUE) {
                    bFound = TRUE;
                    count += m_spells.m_spellsByClass[nClass].m_lists[nLevel].Get(v2)->m_nCurrent;
                }
            }
        }
        break;
    case 2:
        if (!g_pBaldurChitin->GetObjectGame()->m_innateSpells.Find(resRef, v1)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 15994
            UTIL_ASSERT(FALSE);
        }

        if (m_innateSpells.Find(v1, v2) == TRUE) {
            count += m_innateSpells.Get(v2)->m_nCurrent;
        }
        break;
    case 3:
        if (!g_pBaldurChitin->GetObjectGame()->m_songs.Find(resRef, v1)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 16012
            UTIL_ASSERT(FALSE);
        }

        if (m_songs.Find(v1, v2) == TRUE) {
            count += m_songs.Get(v2)->m_nCurrent;
        }
        break;
    case 4:
        if (!g_pBaldurChitin->GetObjectGame()->m_shapeshifts.Find(resRef, v1)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 16030
            UTIL_ASSERT(FALSE);
        }

        if (m_shapeshifts.Find(v1, v2) == TRUE) {
            count += m_shapeshifts.Get(v2)->m_nCurrent;
        }
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 16044
        UTIL_ASSERT(FALSE);
    }

    DWORD nSpecialization = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpecializationMask(nClass, nKitIndex);
    cSpell.Demand();

    SHORT nCasterLevel = GetCasterLevel(&cSpell, nClass, nSpecialization);
    if (nCasterLevel <= 1) {
        nCasterLevel = 1;
    }

    SPELL_ABILITY* pBestAbility = NULL;
    for (INT abilityNum = 0; abilityNum < cSpell.GetAbilityCount(); abilityNum++) {
        if (cSpell.GetAbility(abilityNum)->minCasterLevel > nCasterLevel) {
            break;
        }

        // FIXME: Calls `GetAbility` one more time.
        pBestAbility = cSpell.GetAbility(abilityNum);
    }

    if (pBestAbility != NULL) {
        cButtonData.m_icon = CString(pBestAbility->quickSlotIcon);
        cButtonData.m_name = cSpell.GetGenericName();
        cButtonData.m_abilityId.m_itemType = 1;
        cButtonData.m_abilityId.m_targetType = pBestAbility->actionType;
        cButtonData.m_abilityId.m_bCanUse = a5;
        cButtonData.m_abilityId.m_nClass = nClass;
        cButtonData.m_abilityId.m_nTooltip = nKitIndex;
        cButtonData.m_abilityId.m_strDescription = cSpell.GetGenericName();

        if (type == 1) {
            cButtonData.m_abilityId.m_strTooltipDesc = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassSuffixStringRef(nClass);
            if (!CanCast(nClass, 0, &cSpell)) {
                cButtonData.m_bDisabled = TRUE;
                cButtonData.m_abilityId.m_strTooltipDesc = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassBeyondCastingAbilityStringRef(nClass);
            }
        }

        cButtonData.m_count = count;

        if (count == 0) {
            cButtonData.m_bDisabled = TRUE;
        }

        if (type == 2) {
            if (resRef == CGameSprite::SPIN277) {
                cButtonData.m_bDisplayCount = FALSE;
                cButtonData.m_count = 0;

                if (!HasFeat(CGAMESPRITE_FEAT_ARTERIAL_STRIKE)) {
                    cButtonData.m_bDisabled = TRUE;
                }
            } else if (resRef == CGameSprite::SPIN278) {
                cButtonData.m_bDisplayCount = FALSE;
                cButtonData.m_count = 0;

                if (!HasFeat(CGAMESPRITE_FEAT_HAMSTRING)) {
                    cButtonData.m_bDisabled = TRUE;
                }
            } else if (resRef == CGameSprite::SPIN279) {
                cButtonData.m_bDisplayCount = FALSE;
                cButtonData.m_count = 0;

                if (!HasFeat(CGAMESPRITE_FEAT_RAPID_SHOT)) {
                    cButtonData.m_bDisabled = TRUE;
                }
            }
        }
    }

    cSpell.Release();
}

// 0x714DB0
void CGameSprite::InitQuickItemData(SHORT itemNum, SHORT abilityNum, int a3, CButtonData& cButtonData)
{
    const CRuleTables& cRule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    if (itemNum != -1 && abilityNum != -1) {
        CItem* pItem = m_equipment.m_items[itemNum];
        if (pItem != NULL) {
            pItem->Demand();

            ITEM_ABILITY* ability = pItem->GetAbility(abilityNum);
            if (ability != NULL) {
                if (ability->type != 4
                    && (ability->type != 2 || CheckLauncherType(ability, NULL))) {
                    cButtonData.m_icon = CString(ability->quickSlotIcon);
                    cButtonData.m_name = pItem->GetGenericName();
                    cButtonData.m_abilityId.m_itemType = 2;
                    cButtonData.m_abilityId.m_itemNum = itemNum;
                    cButtonData.m_abilityId.m_abilityNum = abilityNum;
                    cButtonData.m_abilityId.m_targetType = ability->actionType;
                    cButtonData.m_abilityId.m_strDescription = cRule.GetItemAbilityDescription(pItem->GetResRef(), abilityNum);
                    if (cButtonData.m_abilityId.m_strDescription == -1) {
                        cButtonData.m_abilityId.m_strDescription = pItem->GetGenericName();
                    }
                    cButtonData.m_count = 0;
                    if (pItem->GetMaxStackable() > 1
                        || pItem->GetMaxUsageCount(abilityNum) > 0) {
                        cButtonData.m_count = pItem->GetUsageCount(abilityNum);
                    }

                    SHORT launcherSlot;
                    CItem* pLauncher = GetLauncher(ability, launcherSlot);
                    if (pLauncher != NULL) {
                        cButtonData.m_launcherIcon = pLauncher->GetItemIcon();
                        cButtonData.m_launcherName = pLauncher->GetGenericName();
                    }
                }
            }

            pItem->Release();
        }
    }
}

// 0x716540
CGameButtonList* CGameSprite::GetSongsButtonList()
{
    CGameButtonList* buttons = new CGameButtonList();
    RESREF resRef;

    for (size_t index = 0; index < m_songs.m_List.size(); index++) {
        // NOTE: Uninline.
        CGameSpriteSpellListEntry* entry = m_songs.Get(index);

        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().Get(entry->m_nID).GetResRef(resRef);

        buttons->AddTail(IcewindMisc::CreateButtonData(resRef));
    }

    return buttons;
}

// Build the cast-spell picker list across every memorised level of class
// nClass.  Used when SetState(0x67) is entered for a regular class spellbook.
//
// 0x714F70
CGameButtonList* CGameSprite::GetSpellsButtonList(const BYTE& nClass)
{
    CGameButtonList* pButtons = new CGameButtonList();

    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);
    SPELL_ABILITY* pBestAbility = NULL;

    CGameSpriteGroupedSpellList* pGrouped = m_spells.Get(nClassIndex);
    if (pGrouped->m_nHighestLevel == 0) {
        return pButtons;
    }

    for (UINT nLevel = 0; nLevel < pGrouped->m_nHighestLevel; nLevel++) {
        CGameSpriteSpellList* pLevelList = pGrouped->GetSpellsAtLevel(nLevel);

        // Spontaneous casters (bard/sorcerer) hide whole levels with no
        // shared casts left.
        if ((nClassIndex == 0 || nClassIndex == 5)
            && pLevelList->m_nSharedTotal == 0) {
            continue;
        }

        for (UINT nIndex = 0; nIndex < pLevelList->m_List.size(); nIndex++) {
            CGameSpriteSpellListEntry* pEntry = pLevelList->Get(nIndex);

            if ((pEntry->m_nShared & 1) != 0) {
                continue;
            }

            // Prepared casters skip exhausted entries entirely; spontaneous
            // casters keep them (the level-wide shared pool gates them).
            if (nClassIndex != 0 && nClassIndex != 5
                && (pEntry->m_nMax == 0 || pEntry->m_nCurrent == 0)) {
                continue;
            }

            UINT nSpellId = pEntry->m_nID;
            const CResRef& spellResRef = g_pBaldurChitin->GetObjectGame()->m_spells.Get(nSpellId);

            CSpell cSpell;
            cSpell.SetResRef(spellResRef, TRUE, TRUE);
            if (cSpell.Demand() != NULL) {
                // Identify is only castable from the inventory screen.
                if (spellResRef != SPWI110) {
                    SHORT nCasterLevel = GetCasterLevel(&cSpell, nClass, 0);
                    if (nCasterLevel < 1) {
                        nCasterLevel = 1;
                    }

                    for (INT nAbility = 0; nAbility < cSpell.GetAbilityCount(); nAbility++) {
                        if (cSpell.GetAbility(nAbility)->minCasterLevel > nCasterLevel) {
                            break;
                        }

                        // FIXME: Calls `GetAbility` one more time.
                        pBestAbility = cSpell.GetAbility(nAbility);
                    }

                    if (pBestAbility != NULL && pBestAbility->quickSlotType == 2) {
                        CButtonData* pButton = new CButtonData();

                        pButton->m_icon = CString(pBestAbility->quickSlotIcon);
                        pButton->m_abilityId.m_itemType = 1;
                        pButton->m_abilityId.m_res = spellResRef;
                        pButton->m_abilityId.m_targetType = pBestAbility->actionType;

                        SHORT nCasterType = cSpell.GetCasterType();
                        INT nDisabledType = 0;
                        if (nCasterType == 1) {
                            nDisabledType = 0;
                        } else if (nCasterType == 2) {
                            nDisabledType = 1;
                        } else if (nCasterType == 4) {
                            nDisabledType = 2;
                        }

                        CDerivedStats& stats = m_bAllowEffectListCall ? m_derivedStats : m_tempStats;
                        pButton->m_bDisabled = static_cast<BOOLEAN>(stats.m_disabledSpellTypes[nDisabledType]);

                        pButton->m_abilityId.m_strDescription = cSpell.GetGenericName();
                        pButton->m_name = cSpell.GetGenericName();
                        pButton->m_abilityId.m_strTooltipDesc = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassSuffixStringRef(nClass);
                        pButton->m_abilityId.m_nClass = nClass;
                        pButton->m_abilityId.m_bCanUse = static_cast<unsigned char>(nLevel + 1);
                        pButton->m_abilityId.m_nTooltip = 0;

                        if (!CanCast(nClass, 0, &cSpell)) {
                            pButton->m_bDisabled = TRUE;
                            pButton->m_abilityId.m_strTooltipDesc = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassBeyondCastingAbilityStringRef(nClass);
                        }

                        SHORT nCount;
                        if (nClassIndex == 0 || nClassIndex == 5) {
                            nCount = static_cast<SHORT>(pLevelList->m_nSharedTotal);
                        } else {
                            nCount = static_cast<SHORT>(pEntry->m_nCurrent);
                        }

                        pButton->m_count = nCount;
                        if (nCount < 1) {
                            pButton->m_bDisabled = TRUE;
                        }

                        pButtons->AddTail(pButton);
                    }
                }

                cSpell.Release();
            }
        }
    }

    return pButtons;
}

// Build the cast-spell picker list for the cleric domain pool across every
// memorised level.  Used when SetState(0x67) is entered with a cleric class
// and a non-zero domain specialization.
//
// 0x7155C0
CGameButtonList* CGameSprite::GetDomainSpellsButtonList(const BYTE& nClass, DWORD nSpecialization)
{
    CGameButtonList* pButtons = new CGameButtonList();

    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);
    SPELL_ABILITY* pBestAbility = NULL;

    if (m_domainSpells.m_nHighestLevel == 0) {
        return pButtons;
    }

    for (UINT nLevel = 0; nLevel < m_domainSpells.m_nHighestLevel; nLevel++) {
        CGameSpriteSpellList* pLevelList = m_domainSpells.GetSpellsAtLevel(nLevel);

        // Spontaneous casters (bard/sorcerer) hide whole levels with no
        // shared casts left.  Dead path for the cleric-only domain pool,
        // but present in the binary.
        if ((nClassIndex == 0 || nClassIndex == 5)
            && pLevelList->m_nSharedTotal == 0) {
            continue;
        }

        for (UINT nIndex = 0; nIndex < pLevelList->m_List.size(); nIndex++) {
            CGameSpriteSpellListEntry* pEntry = pLevelList->Get(nIndex);

            if ((pEntry->m_nShared & 1) != 0
                || pEntry->m_nMax == 0
                || pEntry->m_nCurrent == 0) {
                continue;
            }

            UINT nSpellId = pEntry->m_nID;
            const CResRef& spellResRef = g_pBaldurChitin->GetObjectGame()->m_spells.Get(nSpellId);

            CSpell cSpell;
            cSpell.SetResRef(spellResRef, TRUE, TRUE);
            if (cSpell.Demand() != NULL) {
                // Identify is only castable from the inventory screen.
                if (spellResRef != SPWI110) {
                    SHORT nCasterLevel = GetCasterLevel(&cSpell, nClass, nSpecialization);
                    if (nCasterLevel < 1) {
                        nCasterLevel = 1;
                    }

                    for (INT nAbility = 0; nAbility < cSpell.GetAbilityCount(); nAbility++) {
                        if (cSpell.GetAbility(nAbility)->minCasterLevel > nCasterLevel) {
                            break;
                        }

                        // FIXME: Calls `GetAbility` one more time.
                        pBestAbility = cSpell.GetAbility(nAbility);
                    }

                    if (pBestAbility != NULL && pBestAbility->quickSlotType == 2) {
                        CButtonData* pButton = new CButtonData();

                        pButton->m_icon = CString(pBestAbility->quickSlotIcon);
                        pButton->m_abilityId.m_itemType = 1;
                        pButton->m_abilityId.m_res = spellResRef;
                        pButton->m_abilityId.m_targetType = pBestAbility->actionType;

                        SHORT nCasterType = cSpell.GetCasterType();
                        INT nDisabledType = 0;
                        if (nCasterType == 1) {
                            nDisabledType = 0;
                        } else if (nCasterType == 2) {
                            nDisabledType = 1;
                        } else if (nCasterType == 4) {
                            nDisabledType = 2;
                        }

                        CDerivedStats& stats = m_bAllowEffectListCall ? m_derivedStats : m_tempStats;
                        pButton->m_bDisabled = static_cast<BOOLEAN>(stats.m_disabledSpellTypes[nDisabledType]);

                        pButton->m_abilityId.m_strDescription = cSpell.GetGenericName();
                        pButton->m_name = cSpell.GetGenericName();
                        pButton->m_abilityId.m_strTooltipDesc = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassSuffixStringRef(nClass);
                        pButton->m_abilityId.m_bCanUse = static_cast<unsigned char>(nLevel + 1);
                        pButton->m_abilityId.m_nClass = nClass;
                        pButton->m_abilityId.m_nTooltip = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetSpecializationIndex(nClass, nSpecialization);

                        if (!CanCast(nClass, nSpecialization, &cSpell)) {
                            pButton->m_bDisabled = TRUE;
                            pButton->m_abilityId.m_strTooltipDesc = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetClassBeyondCastingAbilityStringRef(nClass);
                        }

                        SHORT nCount = static_cast<SHORT>(pEntry->m_nCurrent);
                        pButton->m_count = nCount;
                        if (nCount < 1) {
                            pButton->m_bDisabled = TRUE;
                        }

                        pButtons->AddTail(pButton);
                    }
                }

                cSpell.Release();
            }
        }
    }

    return pButtons;
}

// Count classes that have any memorised spells.  Used by the Cast Spell
// click handler to decide whether to show the class picker (state 0x76)
// or jump straight to the spellbook (state 0x67) when only one class has
// spells.  Matches Ghidra FUN_00594280 case 3.
// NOTE: Convenience.
INT CGameSprite::CountClassesWithSpells()
{
    INT count = 0;
    // Ghidra checks classes 2/3/4/7/8/10/11 (the canonical spellcaster set).
    static const BYTE classes[] = { 2, 3, 4, 7, 8, 10, 11 };
    for (size_t i = 0; i < sizeof(classes) / sizeof(classes[0]); i++) {
        CGameSpriteGroupedSpellList* grouped = GetSpells(classes[i]);
        if (grouped != NULL && grouped->m_nHighestLevel != 0) {
            count++;
        }
    }
    // Domain counts as its own class for the picker.
    if (m_domainSpells.m_nHighestLevel != 0) {
        count++;
    }
    return count;
}

// NOTE: This function correctly accepts `nClass` by value (as opposed by to
// many others which passes `nClass` as a reference for unknown reason).
//
// 0x7166D0
BOOLEAN CGameSprite::CanCast(BYTE nClass, DWORD nSpecialization, CSpell* pSpell)
{
    INT nValue = 0;
    switch (nClass) {
    case CAIOBJECTTYPE_C_BARBARIAN:
    case CAIOBJECTTYPE_C_FIGHTER:
    case CAIOBJECTTYPE_C_MONK:
    case CAIOBJECTTYPE_C_ROGUE:
        return FALSE;
    case CAIOBJECTTYPE_C_BARD:
    case CAIOBJECTTYPE_C_SORCERER:
        nValue = m_derivedStats.m_nCHR;
        break;
    case CAIOBJECTTYPE_C_CLERIC:
    case CAIOBJECTTYPE_C_DRUID:
    case CAIOBJECTTYPE_C_PALADIN:
    case CAIOBJECTTYPE_C_RANGER:
        nValue = m_derivedStats.m_nWIS;
        break;
    case CAIOBJECTTYPE_C_WIZARD:
        nValue = m_derivedStats.m_nINT;
        break;
    }

    INT nLevel = g_pBaldurChitin->GetObjectGame()->GetSpellLevel(pSpell->GetResRef(), nClass, nSpecialization);
    return nLevel < CSPELLLIST_MAX_LEVELS && nValue >= nLevel + 11;
}

// 0x717620
CGameButtonList* CGameSprite::GetInternalButtonList()
{
    CGameButtonList* buttons = new CGameButtonList();

    if (m_internalButtonList != NULL) {
        POSITION pos = m_internalButtonList->GetHeadPosition();
        while (pos != NULL) {
            CButtonData* node = m_internalButtonList->GetNext(pos);
            CButtonData* copy = new CButtonData();
            *copy = *node;
            buttons->AddTail(copy);
        }

        // FIXME: Meaninless memory leak.
        new CButtonData();
    }

    return buttons;
}

// 0x717850
CGameButtonList* CGameSprite::GetItemUsages(SHORT slotNum, WORD buttonType, SHORT abilityNum)
{
    const CRuleTables& cRule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();
    CGameButtonList* buttons = new CGameButtonList();

    // FIXME: Unused.
    STR_RES strRes;

    if (slotNum >= 51) {
        return buttons;
    }

    if (buttonType == 1 && m_equipment.m_items[42] != NULL) {
        if (slotNum == 43) {
            delete buttons;
            return GetItemUsages(42, 1, 0);
        }

        if (slotNum > 43) {
            return buttons;
        }
    }

    CItem* pItem = m_equipment.m_items[slotNum];
    if (pItem == NULL) {
        return buttons;
    }

    if (slotNum == 43
        || slotNum == 45
        || slotNum == 47
        || slotNum == 49) {
        if ((pItem->GetFlagsFile() & 0x2) != 0) {
            if (m_equipment.m_items[slotNum + 1] != NULL) {
                return buttons;
            }
        }
    }

    pItem->Demand();

    INT nStart = abilityNum;
    INT nEnd;
    if (abilityNum == -1) {
        nStart = 0;
        if (pItem->GetItemType() == 11) {
            nEnd = 1;
        } else {
            nEnd = pItem->GetAbilityCount();
        }
    } else {
        nEnd = abilityNum + 1;
    }

    for (INT nAbility = nStart; nAbility < nEnd; nAbility++) {
        const ITEM_ABILITY* curAbility = pItem->GetAbility(nAbility);
        if (curAbility == NULL) {
            continue;
        }

        if ((curAbility->type & 0x100) != 0 && (pItem->m_flags & 0x1) == 0) {
            continue;
        }

        if ((curAbility->type & 0x200) != 0 && (pItem->m_flags & 0x1) != 0) {
            continue;
        }

        if (curAbility->quickSlotType == buttonType && (curAbility->type & 0xFF) != 4) {
            if ((curAbility->type & 0xFF) != 2 || CheckLauncherType(curAbility, NULL)) {
                CButtonData* pButtonData = new CButtonData();
                pButtonData->m_icon = CString(curAbility->quickSlotIcon);
                pButtonData->m_name = pItem->GetGenericName();
                pButtonData->m_abilityId.m_itemType = 2;
                pButtonData->m_abilityId.m_itemNum = slotNum;
                pButtonData->m_abilityId.m_abilityNum = nAbility;
                pButtonData->m_abilityId.m_targetType = curAbility->actionType;
                pButtonData->m_abilityId.m_strDescription = cRule.GetItemAbilityDescription(pItem->GetResRef(), nAbility);
                if (pButtonData->m_abilityId.m_strDescription == -1) {
                    pButtonData->m_abilityId.m_strDescription = pItem->GetGenericName();
                }
                pButtonData->m_count = 0;
                if (pItem->GetMaxStackable() > 1 || pItem->GetMaxUsageCount(nAbility) > 0) {
                    pButtonData->m_count = pItem->GetUsageCount(nAbility);
                }

                SHORT launcherSlot;
                CItem* pLauncher = GetLauncher(curAbility, launcherSlot);
                if (pLauncher != NULL) {
                    pButtonData->m_launcherIcon = pLauncher->GetItemIcon();
                    pButtonData->m_launcherName = pLauncher->GetGenericName();
                }

                buttons->AddTail(pButtonData);
            }
        } else {
            if ((curAbility->type & 0xFF) == 4) {
                for (INT ammoSlotNum = 11; ammoSlotNum < 15; ammoSlotNum++) {
                    CItem* pAmmo = m_equipment.m_items[ammoSlotNum];
                    if (pAmmo == NULL) {
                        continue;
                    }

                    pAmmo->Demand();

                    for (INT nAmmoAbility = nStart; nAmmoAbility < nEnd; nAmmoAbility++) {
                        const ITEM_ABILITY* curAmmoAbility = pAmmo->GetAbility(nAmmoAbility);
                        if (curAmmoAbility != NULL
                            && curAmmoAbility->quickSlotType == buttonType
                            && (curAmmoAbility->type & 0xFF) != 4
                            && CheckLauncherType(curAmmoAbility, pItem)) {
                            CButtonData* pButtonData = new CButtonData();
                            pButtonData->m_icon = CString(curAmmoAbility->quickSlotIcon);
                            pButtonData->m_name = pAmmo->GetGenericName();
                            pButtonData->m_count = pAmmo->GetUsageCount(nAmmoAbility);
                            pButtonData->m_abilityId.m_itemType = 2;
                            pButtonData->m_abilityId.m_itemNum = ammoSlotNum;
                            pButtonData->m_abilityId.m_abilityNum = nAmmoAbility;
                            pButtonData->m_abilityId.m_targetType = curAmmoAbility->actionType;
                            if (curAmmoAbility->maxUsageCount == 0) {
                                pButtonData->m_bDisplayCount = FALSE;
                            }
                            pButtonData->m_abilityId.m_strDescription = cRule.GetItemAbilityDescription(pAmmo->GetResRef(), nAmmoAbility);
                            if (pButtonData->m_abilityId.m_strDescription == -1) {
                                pButtonData->m_abilityId.m_strDescription = pAmmo->GetGenericName();
                            }
                            pButtonData->m_launcherIcon = pItem->GetItemIcon();
                            pButtonData->m_launcherName = pItem->GetGenericName();

                            buttons->AddTail(pButtonData);
                        }
                    }

                    pAmmo->Release();
                }
            }
        }
    }

    pItem->Release();

    return buttons;
}

// Populate a single CButtonData from a spell/ability resref: demand the spell,
// pick the highest ability whose minimum caster level is within reach, then copy
// its icon, name and ability identity into the button.  Returns FALSE when the
// spell cannot be loaded or has no usable ability.
//
// 0x718390
BOOL CGameSprite::BuildAbilityButtonData(const CResRef& res, BYTE nClass, DWORD nSpecialization, CButtonData& cButtonData)
{
    CSpell cSpell;
    cSpell.SetResRef(res, TRUE, TRUE);

    if (cSpell.Demand() == NULL) {
        return FALSE;
    }

    if (cSpell.GetRes() == NULL) {
        cSpell.Release();
        return FALSE;
    }

    SHORT nCasterLevel = GetCasterLevel(&cSpell, nClass, nSpecialization);
    if (nCasterLevel <= 1) {
        nCasterLevel = 1;
    }

    SPELL_ABILITY* pBestAbility = NULL;
    for (INT nAbility = 0; nAbility < cSpell.GetAbilityCount(); nAbility++) {
        if (cSpell.GetAbility(nAbility)->minCasterLevel > nCasterLevel) {
            break;
        }

        // FIXME: Calls `GetAbility` one more time.
        pBestAbility = cSpell.GetAbility(nAbility);
    }

    BOOL bResult = FALSE;
    if (pBestAbility != NULL) {
        cButtonData.m_icon = CString(pBestAbility->quickSlotIcon);
        cButtonData.m_name = cSpell.GetGenericName();
        cButtonData.m_abilityId.m_itemType = 1;
        cButtonData.m_abilityId.m_res = res;
        cButtonData.m_abilityId.m_targetType = pBestAbility->actionType;
        cButtonData.m_abilityId.m_strDescription = cSpell.GetGenericName();
        cButtonData.m_bDisabled = FALSE;
        cButtonData.m_count = 0;
        bResult = TRUE;
    }

    cSpell.Release();
    return bResult;
}

// 0x718650
CItem* CGameSprite::GetLauncher(const ITEM_ABILITY* ability, SHORT& launcherSlot)
{
    int index;
    launcherSlot = 0;

    if (ability == NULL || ability->type != 2) {
        return NULL;
    }

    switch (ability->launcherType) {
    case 1:
        launcherSlot = 2 * m_nWeaponSet + 43;

        if (m_equipment.m_items[launcherSlot] != NULL
            && m_equipment.m_items[launcherSlot]->GetItemType() == 15) {
            return m_equipment.m_items[launcherSlot];
        }

        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 15) {
                return m_equipment.m_items[43 + index];
            }
        }

        break;
    case 2:
        launcherSlot = 2 * m_nWeaponSet + 43;

        if (m_equipment.m_items[launcherSlot] != NULL
            && m_equipment.m_items[launcherSlot]->GetItemType() == 27) {
            return m_equipment.m_items[launcherSlot];
        }

        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 27) {
                return m_equipment.m_items[43 + index];
            }
        }

        break;
    case 3:
        launcherSlot = 2 * m_nWeaponSet + 43;

        if (m_equipment.m_items[launcherSlot] != NULL
            && m_equipment.m_items[launcherSlot]->GetItemType() == 18) {
            return m_equipment.m_items[launcherSlot];
        }

        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 18) {
                return m_equipment.m_items[43 + index];
            }
        }

        break;
    }

    return NULL;
}

// 0x7187E0
SHORT CGameSprite::GetLauncherSlot(SHORT slotNum, SHORT abilityNum)
{
    int index;

    CItem* pItem = m_equipment.m_items[slotNum];
    if (pItem == NULL) {
        return -1;
    }

    pItem->Demand();

    ITEM_ABILITY* ability = pItem->GetAbility(abilityNum);
    if (ability == NULL || ability->type != 2) {
        pItem->Release();
        return -1;
    }

    switch (ability->launcherType) {
    case 1:
        if (m_equipment.m_items[2 * m_nWeaponSet + 43] != NULL
            && m_equipment.m_items[2 * m_nWeaponSet + 43]->GetItemType() == 15) {
            pItem->Release();
            return 2 * m_nWeaponSet + 43;
        }
        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 15) {
                pItem->Release();
                return 43 + index;
            }
        }
        break;
    case 2:
        if (m_equipment.m_items[2 * m_nWeaponSet + 43] != NULL
            && m_equipment.m_items[2 * m_nWeaponSet + 43]->GetItemType() == 27) {
            pItem->Release();
            return 2 * m_nWeaponSet + 43;
        }
        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 27) {
                pItem->Release();
                return 43 + index;
            }
        }
        break;
    case 3:
        if (m_equipment.m_items[2 * m_nWeaponSet + 43] != NULL
            && m_equipment.m_items[2 * m_nWeaponSet + 43]->GetItemType() == 18) {
            pItem->Release();
            return 2 * m_nWeaponSet + 43;
        }
        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 18) {
                pItem->Release();
                return 43 + index;
            }
        }
        break;
    }

    pItem->Release();
    return -1;
}

// 0x718980
BOOL CGameSprite::CheckLauncherType(const ITEM_ABILITY* ability, CItem* pLauncher)
{
    int index;

    if (pLauncher != NULL) {
        switch (ability->launcherType) {
        case 1:
            return pLauncher->GetItemType() == 15;
        case 2:
            return pLauncher->GetItemType() == 27;
        case 3:
            return pLauncher->GetItemType() == 18;
        default:
            return FALSE;
        }
    }

    switch (ability->launcherType) {
    case 0:
        return TRUE;
    case 1:
        if (m_equipment.m_items[2 * m_nWeaponSet + 43] != NULL
            && m_equipment.m_items[2 * m_nWeaponSet + 43]->GetItemType() == 15) {
            return TRUE;
        }
        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 15) {
                return TRUE;
            }
        }
        break;
    case 2:
        if (m_equipment.m_items[2 * m_nWeaponSet + 43] != NULL
            && m_equipment.m_items[2 * m_nWeaponSet + 43]->GetItemType() == 27) {
            return TRUE;
        }
        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 27) {
                return TRUE;
            }
        }
        break;
    case 3:
        if (m_equipment.m_items[2 * m_nWeaponSet + 43] != NULL
            && m_equipment.m_items[2 * m_nWeaponSet + 43]->GetItemType() == 18) {
            return TRUE;
        }
        for (index = 0; index < 8; index++) {
            if (m_equipment.m_items[43 + index] != NULL
                && m_equipment.m_items[43 + index]->GetItemType() == 18) {
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}

// 0x5B9BA0
static BOOL IsItemUsableByClass(DWORD nClassMask, DWORD nKitMask, CItem* pItem)
{
    SHORT nItemType = pItem->GetItemType();
    if (nItemType != 0x23 && nItemType != 0xB) {
        DWORD nNotUsableBy = pItem->GetNotUsableBy();
        DWORD nNotUsableBy2 = pItem->GetNotUsableBy2();

        if ((nNotUsableBy & nClassMask) == nClassMask) {
            return (nNotUsableBy2 & nKitMask) != nKitMask;
        }

        DWORD nKitHits = nNotUsableBy2 & nKitMask;
        if (nKitHits != 0
            && (~(nNotUsableBy & nClassMask) & nClassMask & 0x39B) == 0
            && ((nNotUsableBy & 4) != 0 || (nClassMask & 4) == 0 || (nKitHits & 0xFF8000) != 0)
            && ((nNotUsableBy & 0x20) != 0 || (nClassMask & 0x20) == 0 || (nKitHits & 0x38) != 0)
            && ((nNotUsableBy & 0x40) != 0 || (nClassMask & 0x40) == 0 || (nKitHits & 7) != 0)
            && ((nNotUsableBy & 0x400) != 0 || (nClassMask & 0x400) == 0 || (nKitHits & 0x7FC0) != 0)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 0x5B9D20
// pUser = the wearer (first stack arg). this(ecx) and a4 are supplied by callers but
// never read by the body -- faithful to the binary's __thiscall + 4-stack-arg ABI
// (ret 0x10); verified via Frida trace + disassembly.
INT CGameSprite::CanUseItem(CGameSprite* pUser, CItem* pItem, STRREF& errorCode, BOOL a4)
{
    if (pItem != NULL) {
        pItem->GetItemType();
    }

    errorCode = -1;

    if (pItem == NULL) {
        return 1;
    }

    DWORD nNotUsableBy = pItem->GetNotUsableBy();

    CAIObjectType cType;
    cType.Set(pUser->GetAIType());

    BOOL bUsable = g_pBaldurChitin->GetObjectGame()->GetRuleTables().IsUsableByAlignment(nNotUsableBy, cType.m_nAlignment);
    if (!bUsable) {
        errorCode = 0x24A6;
    }

    if ((nNotUsableBy & 0x800000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_ELF) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }
    if ((nNotUsableBy & 0x1000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_DWARF) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }
    if ((nNotUsableBy & 0x2000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HALF_ELF) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }
    if ((nNotUsableBy & 0x4000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HALFLING) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }
    if ((nNotUsableBy & 0x8000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HUMAN) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }
    if ((nNotUsableBy & 0x10000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_GNOME) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }
    if ((nNotUsableBy & 0x20000000) != 0 && cType.m_nRace == CAIOBJECTTYPE_R_HALF_ORC) {
        bUsable = FALSE;
        errorCode = 0x24A6;
    }

    if (!bUsable) {
        return 0;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetRuleTables().ShouldCheckItemRequirements(pItem)) {
        CDerivedStats* pStats = pUser->GetActiveStats();
        if (pStats->m_nLevel < pItem->GetMinLevelRequired()) {
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if (pStats->m_nSTR < static_cast<SHORT>(pItem->GetMinSTRRequired())) {
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if (pStats->m_nINT < static_cast<SHORT>(pItem->GetMinINTRequired())) {
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if (pStats->m_nDEX < static_cast<SHORT>(pItem->GetMinDEXRequired())) {
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if (pStats->m_nWIS < static_cast<SHORT>(pItem->GetMinWISRequired())) {
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if (pStats->m_nCON < static_cast<SHORT>(pItem->GetMinCONRequired())) {
            bUsable = FALSE;
            errorCode = 0x24A6;
        }
        if (static_cast<SHORT>(pItem->GetMinCHRRequired()) > pStats->m_nCHR) {
            errorCode = 0x24A6;
            return 0;
        }
    }

    if (!bUsable) {
        return 0;
    }

    CDerivedStats* pStats = pUser->GetActiveStats();
    INT nResult = IsItemUsableByClass(pStats->m_classMask, pUser->GetActiveStats()->m_nSpecialization, pItem);
    if (nResult != 0) {
        return nResult;
    }

    errorCode = 0x24A6;
    return nResult;
}

// 0x5BA2E0
INT CGameSprite::CanEquipItemInSlot(INT nSlotNum, CItem*& pItem, STRREF& errorCode)
{
    BOOL bQuickItemSlot = FALSE;

    WORD nNewItemType = static_cast<WORD>(nSlotNum);
    if (pItem != NULL) {
        nNewItemType = pItem->GetItemType();
    }

    errorCode = -1;

    if (CGameSpriteEquipment::NUM_SLOT <= nSlotNum) {
        errorCode = 0x249E;
        return 0;
    }

    INT nResult = CanUseItem(this, pItem, errorCode, TRUE);
    if (nResult == 0) {
        return 0;
    }

    CGameSpriteEquipment* pEquip = GetEquipment();

    switch (nSlotNum) {
    case 0: // amulet
        if (pItem != NULL && nNewItemType != 1 && nNewItemType != 0x46) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 1: // armor
        if (pItem != NULL
            && nNewItemType != 0x3C && nNewItemType != 0x3D && nNewItemType != 0x42
            && nNewItemType != 0x3E && nNewItemType != 0x3F && nNewItemType != 0x40
            && nNewItemType != 0x41 && nNewItemType != 0x43 && nNewItemType != 0x44) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 2: // belt
        if (pItem != NULL && nNewItemType != 3) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 3: // boots
        if (pItem != NULL && nNewItemType != 4) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 4: // cloak
        if (pItem != NULL && nNewItemType != 0x20) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 5: // gauntlets
        if (pItem != NULL && nNewItemType != 6 && nNewItemType != 0x49) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 6: // helmet
        if (pItem != NULL && nNewItemType != 7 && nNewItemType != 0x48) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 7: // ring
        if (pItem != NULL && nNewItemType != 10) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 8: // ring
        if (pItem != NULL && nNewItemType != 10) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 10: // fist (cannot hold an item)
        errorCode = 0x249F;
        return 0;
    case 0xB: // ammo quiver
    case 0xC:
    case 0xD:
    case 0xE:
        if (pItem != NULL && nNewItemType != 5 && nNewItemType != 0x1F && nNewItemType != 0xE) {
            errorCode = 0x249F;
            return 0;
        }
        break;
    case 0xF: // quick item slots
    case 0x10:
    case 0x11:
        bQuickItemSlot = TRUE;
        if (pItem != NULL) {
            if (nNewItemType != 0 && nNewItemType != 8 && nNewItemType != 0x23
                && nNewItemType != 9 && nNewItemType != 0xB && nNewItemType != 0xD
                && nNewItemType != 0x47) {
                errorCode = 0x249F;
                return 0;
            }
            if (pItem->GetResRef() == "SCRL75") {
                errorCode = 0x249F;
                return 0;
            }
            if (pItem->GetResRef() == "MISC3P") {
                errorCode = 0x249F;
                return 0;
            }
            if (pItem->GetResRef() == "MISC86") {
                errorCode = 0x249F;
                return 0;
            }
            if (pItem->GetAbilityCount() == 0) {
                errorCode = 0x249F;
                return 0;
            }
            if (g_pBaldurChitin->GetObjectGame()->CheckItemUsable(this, pItem) == 0) {
                errorCode = 0x24A6;
                return 0;
            }
        }
        break;
    case 0x2B: // main-hand weapon, sets 0-3
    case 0x2D:
    case 0x2F:
    case 0x31:
        if (pItem != NULL) {
            if (nNewItemType != 0xF) {
                if (nNewItemType != 0x10 && nNewItemType != 0x11 && nNewItemType != 0x12
                    && nNewItemType != 0x13 && nNewItemType != 0x14 && nNewItemType != 0x45
                    && nNewItemType != 0x39 && nNewItemType != 0x15 && nNewItemType != 0x16
                    && nNewItemType != 0x17 && nNewItemType != 0x18 && nNewItemType != 0x19
                    && nNewItemType != 0x1A && nNewItemType != 0x1B && nNewItemType != 0x1D
                    && nNewItemType != 0x1E && nNewItemType != 0x2C) {
                    errorCode = 0x249F;
                    nResult = 0;
                }
                goto weaponMainOffhand;
            }
            // Two-handed weapon (type 0xF).
            if (pEquip->m_items[nSlotNum + 1] != NULL) {
                SHORT nOffhandType = pEquip->m_items[nSlotNum + 1]->GetItemType();
                if (nOffhandType != 0x2F && nOffhandType != 0x35
                    && nOffhandType != 0x31 && nOffhandType != 0x29) {
                    errorCode = 0x521;
                    nResult = 0;
                }
            }
            goto weaponMainCursed;
        }
    weaponMainOffhand:
        if (nNewItemType == 0xF || nNewItemType == 0x1B || nNewItemType == 0x12) {
            if (pItem != NULL && pEquip->m_items[nSlotNum + 1] != NULL) {
                SHORT nOffhandType = pEquip->m_items[nSlotNum + 1]->GetItemType();
                if (nOffhandType != 0x2F && nOffhandType != 0x35
                    && nOffhandType != 0x31 && nOffhandType != 0x29) {
                    errorCode = 0x521;
                    nResult = 0;
                }
            }
        } else if (pItem != NULL && pEquip->m_items[nSlotNum + 1] != NULL) {
            SHORT nOffhandType = pEquip->m_items[nSlotNum + 1]->GetItemType();
            if (nOffhandType != 0x2F && nOffhandType != 0x35
                && nOffhandType != 0x31 && nOffhandType != 0x29) {
                pItem->Demand();
                INT nAbilities = pItem->GetAbilityCount();
                for (INT nAbility = 0; nAbility < nAbilities && nResult != 0; nAbility++) {
                    ITEM_ABILITY* pAbility = pItem->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        if ((pAbility->type & 0xFF) == 2) {
                            errorCode = 0x520;
                            nResult = 0;
                        } else if ((pAbility->type & 0xFF) == 4) {
                            errorCode = 0x521;
                            nResult = 0;
                        }
                    }
                    nAbilities = pItem->GetAbilityCount();
                }
                pItem->Release();
                if (nResult == 0) {
                    return 0;
                }
            }
        }
    weaponMainCursed:
        if (pItem != NULL && (pItem->GetFlagsFile() & 2) != 0 && pEquip->m_items[nSlotNum + 1] != NULL) {
            SHORT nOffhandType = pEquip->m_items[nSlotNum + 1]->GetItemType();
            if (nOffhandType != 0x2F && nOffhandType != 0x35
                && nOffhandType != 0x31 && nOffhandType != 0x29) {
                errorCode = 0x7056;
                return 0;
            }
            errorCode = 0x24A4;
            return 0;
        }
        break;
    case 0x2C: // off-hand / shield, sets 0-3
    case 0x2E:
    case 0x30:
    case 0x32:
        if (pItem != NULL) {
            if (nNewItemType == 0x10 || nNewItemType == 0x11 || nNewItemType == 0x2C
                || nNewItemType == 0x12 || nNewItemType == 0x13 || nNewItemType == 0x14
                || nNewItemType == 0x45 || nNewItemType == 0x39 || nNewItemType == 0x15
                || nNewItemType == 0x16 || nNewItemType == 0x17 || nNewItemType == 0x18
                || nNewItemType == 0x19 || nNewItemType == 0x1A || nNewItemType == 0x1B
                || nNewItemType == 0x1D || nNewItemType == 0x1E) {
                if ((pItem->GetFlagsFile() & 2) != 0) {
                    errorCode = 0x66E6;
                    return 0;
                }
                pItem->Demand();
                INT nAbilities = pItem->GetAbilityCount();
                for (INT nAbility = 0; nAbility < nAbilities && nResult != 0; nAbility++) {
                    ITEM_ABILITY* pAbility = pItem->GetAbility(nAbility);
                    if (pAbility != NULL
                        && ((pAbility->type & 0xFF) == 2 || (pAbility->type & 0xFF) == 4)) {
                        errorCode = 0x522;
                        nResult = 0;
                    }
                    nAbilities = pItem->GetAbilityCount();
                }
                pItem->Release();
            } else if (nNewItemType != 0x2F && nNewItemType != 0x35
                       && nNewItemType != 0x31 && nNewItemType != 0x29) {
                errorCode = 0x249F;
                return 0;
            }
        }
        if (pEquip->m_items[nSlotNum - 1] != NULL && pItem != NULL) {
            CItem* pMainhand = pEquip->m_items[nSlotNum - 1];
            if ((pItem->GetFlagsFile() & 2) != 0 && pMainhand != pEquip->m_items[10]) {
                errorCode = 0x24A5;
                return 0;
            }
            SHORT nMainType = pMainhand->GetItemType();
            if (nMainType != 0x2F && nMainType != 0x35 && nMainType != 0x31 && nMainType != 0x29) {
                pMainhand->Demand();
                INT nAbilities = pMainhand->GetAbilityCount();
                for (INT nAbility = 0; nAbility < nAbilities && nResult != 0; nAbility++) {
                    ITEM_ABILITY* pAbility = pMainhand->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        if ((pAbility->type & 0xFF) == 2) {
                            errorCode = 0x51E;
                            nResult = 0;
                        } else if ((pAbility->type & 0xFF) == 4) {
                            errorCode = 0x51F;
                            nResult = 0;
                        }
                    }
                    nAbilities = pMainhand->GetAbilityCount();
                }
                pMainhand->Release();
            }
        }
        break;
    }

    if (nResult != 0) {
        STRREF nImmunityError;
        CGameEffect* pImmunityEffect;
        if (pItem == NULL
            || m_derivedStats.m_cImmunitiesItemEquip.OnList(pItem->GetResRef(), nImmunityError, pImmunityEffect) == 0) {
            if (pItem != NULL
                && m_derivedStats.m_cImmunitiesItemTypeEquip.OnList(pItem->GetItemType(), nImmunityError, pImmunityEffect)) {
                errorCode = nImmunityError;
                if (pImmunityEffect != NULL) {
                    delete pImmunityEffect;
                }
                return 0;
            }
            if (bQuickItemSlot) {
                pItem->Demand();
                if (pItem->GetAbility(0) == NULL) {
                    errorCode = 0x43A5;
                    pItem->Release();
                    return 0;
                }
                if ((pItem->m_flags & 1) == 0 && (pItem->GetFlagsFile() & 0x100) != 0) {
                    errorCode = 0x43A4;
                    pItem->Release();
                    return 0;
                }
                if ((pItem->m_flags & 1) != 0 && (pItem->GetFlagsFile() & 0x200) != 0) {
                    errorCode = 0x43A4;
                    nResult = 0;
                }
                pItem->Release();
            }
        } else {
            errorCode = nImmunityError;
            nResult = 0;
            if (pImmunityEffect != NULL) {
                delete pImmunityEffect;
                return 0;
            }
        }
    }

    return nResult;
}

// 0x718B30
BOOL CGameSprite::ReadyCursor()
{
    switch (m_currentUseButton.m_abilityId.m_targetType) {
    case 1:
        g_pBaldurChitin->GetObjectGame()->SetState(2);
        g_pBaldurChitin->GetObjectGame()->SetIconIndex(20);
        g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
        return TRUE;
    case 2:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 18306
        UTIL_ASSERT(FALSE);
    case 3:
        g_pBaldurChitin->GetObjectGame()->SetState(2);
        g_pBaldurChitin->GetObjectGame()->SetIconIndex(20);
        g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
        return TRUE;
    case 4:
        g_pBaldurChitin->GetObjectGame()->SetState(1);
        g_pBaldurChitin->GetObjectGame()->SetIconIndex(20);
        g_pBaldurChitin->GetObjectGame()->m_iconResRef = "";
        return TRUE;
    case 5:
    case 6:
        g_pBaldurChitin->GetObjectGame()->SetState(0);
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->SetSelectedButton(100);
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->UpdateState();
        return FALSE;
    default:
        return TRUE;
    }
}

// 0x718CE0
void CGameSprite::ReadySpell(SHORT buttonNum, INT nType, BOOLEAN firstCall)
{
    switch (nType) {
    case 2:
        m_currentUseButton = m_quickSpells[buttonNum];
        break;
    case 4:
        m_currentUseButton = m_quickInnates[buttonNum];
        break;
    case 6:
        m_currentUseButton = m_quickSongs[buttonNum];
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 18354
        UTIL_ASSERT(FALSE);
    }

    CAIAction action;
    CString string;
    BOOL cursor = ReadyCursor();

    if (!firstCall && !cursor) {
        switch (m_currentUseButton.m_abilityId.m_targetType) {
        case 5:
            // FIXME: One time is usually enough.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            m_currentUseButton.m_abilityId.m_res.CopyToString(string);
            action = CAIAction(CAIAction::SPELL,
                m_typeAI,
                string,
                0,
                m_currentUseButton.m_abilityId.m_nClass | (m_currentUseButton.m_abilityId.m_nTooltip << 8));
            action.m_specificID3 = m_currentUseButton.m_abilityId.m_bCanUse;
            ClearActions(FALSE);
            m_userCommandPause = 75;
            m_triggerId = CGameObjectArray::INVALID_INDEX;
            AddAction(action);
            m_interrupt = TRUE;
            break;
        case 6:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 18385
            UTIL_ASSERT(FALSE);
            break;
        }
    }
}

// 0x719860
void CGameSprite::ReadyItem(SHORT buttonNum, BOOLEAN firstCall)
{
    m_currentUseButton = m_quickItems[buttonNum];

    CAIAction action;
    CString string;
    BOOL cursor = ReadyCursor();

    if (!firstCall && !cursor) {
        switch (m_currentUseButton.m_abilityId.m_targetType) {
        case 5:
            // FIXME: One time is usually enough.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            // FIXME: Unused.
            m_currentUseButton.m_abilityId.m_res.CopyToString(string);

            // NOTE: Uninline.
            action = CAIAction(CAIAction::USEITEM,
                m_typeAI,
                m_currentUseButton.m_abilityId.m_itemNum,
                m_currentUseButton.m_abilityId.m_abilityNum,
                0);

            ClearActions(FALSE);
            m_userCommandPause = 75;
            m_triggerId = CGameObjectArray::INVALID_INDEX;
            AddAction(action);
            m_interrupt = TRUE;
            break;
        case 6:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 18538
            UTIL_ASSERT(FALSE);
            break;
        }
    }
}

// FIXME: `buttonData` should be reference.
//
// 0x719CA0
void CGameSprite::ReadyOffInternalList(CButtonData buttonData, BOOLEAN firstCall)
{
    m_currentUseButton = buttonData;

    CAIAction action;
    CString string;
    BOOL cursor = ReadyCursor();

    if (!firstCall && !cursor) {
        switch (m_currentUseButton.m_abilityId.m_targetType) {
        case 5:
            // FIXME: One time is usually enough.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            // FIXME: Unused.
            m_currentUseButton.m_abilityId.m_res.CopyToString(string);

            // NOTE: Uninline.
            action = CAIAction(CAIAction::USEITEM,
                m_typeAI,
                m_currentUseButton.m_abilityId.m_itemNum,
                m_currentUseButton.m_abilityId.m_abilityNum,
                0);

            ClearActions(FALSE);
            m_userCommandPause = 75;
            m_triggerId = CGameObjectArray::INVALID_INDEX;
            AddAction(action);
            m_interrupt = TRUE;
            break;
        case 6:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 18593
            UTIL_ASSERT(FALSE);
            break;
        }
    }
}

// FIXME: `buttonData` should be reference.
//
// 0x71A0E0
void CGameSprite::UseButtonAction(CButtonData buttonData, BOOLEAN firstCall)
{
    m_currentUseButton = buttonData;

    CAIAction action;
    CString string;
    BOOL cursor = ReadyCursor();

    if (!firstCall && !cursor) {
        switch (m_currentUseButton.m_abilityId.m_targetType) {
        case 5:
            // FIXME: One time is usually enough.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            m_currentUseButton.m_abilityId.m_res.CopyToString(string);

            // NOTE: Uninline.
            action = CAIAction(CAIAction::SPELL,
                m_typeAI,
                string,
                0,
                m_currentUseButton.m_abilityId.m_nClass | (m_currentUseButton.m_abilityId.m_nTooltip << 8));

            action.m_specificID3 = m_currentUseButton.m_abilityId.m_bCanUse;
            ClearActions(FALSE);
            m_userCommandPause = 75;
            m_triggerId = CGameObjectArray::INVALID_INDEX;
            AddAction(action);
            m_interrupt = TRUE;
            break;
        case 6:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 16649
            UTIL_ASSERT(FALSE);
            break;
        }
    }
}

// 0x5886A0
BOOLEAN CGameSprite::UseSpellAction(const CButtonData* pButtonData, BOOLEAN bUseNow)
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

// 0x588760
BOOLEAN CGameSprite::UseInnateAction(const CButtonData* pButtonData, BOOLEAN bUseNow)
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

// FIXME: `buttonData` should be reference.
//
// 0x71A550
void CGameSprite::UseButtonItem(CButtonData buttonData, BOOLEAN firstCall)
{
    m_currentUseButton = buttonData;

    CAIAction action;
    CString string;
    BOOL cursor = ReadyCursor();

    if (!firstCall && !cursor) {
        switch (m_currentUseButton.m_abilityId.m_itemType) {
        case 3:
            m_castCounter = -1;

            // FIXME: One time is usually enough.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            m_currentUseButton.m_abilityId.m_res.CopyToString(string);

            // NOTE: Uninline.
            action = CAIAction(CAIAction::SPELL,
                m_typeAI,
                string,
                0,
                m_currentUseButton.m_abilityId.m_nClass | (m_currentUseButton.m_abilityId.m_nTooltip << 8));

            ClearActions(FALSE);
            m_userCommandPause = 75;
            m_triggerId = CGameObjectArray::INVALID_INDEX;
            AddAction(action);
            m_interrupt = TRUE;
            break;
        case 4:
            // FIXME: One time is usually enough.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

            m_currentUseButton.m_abilityId.m_res.CopyToString(string);

            // NOTE: This call is not inlined (see above).
            action = CAIAction(CAIAction::SPELL,
                m_typeAI,
                string,
                0,
                m_currentUseButton.m_abilityId.m_nClass | (m_currentUseButton.m_abilityId.m_nTooltip << 8));
            action.m_specificID3 = m_currentUseButton.m_abilityId.m_bCanUse;
            ClearActions(FALSE);
            m_userCommandPause = 75;
            m_triggerId = CGameObjectArray::INVALID_INDEX;
            AddAction(action);
            m_interrupt = TRUE;
            break;
        case 5:
            // NOTE: For unknown reason the first call of `SetLastTarget` is
            // inlined, but the second is not.
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);
            FireSpell(m_currentUseButton.m_abilityId.m_res, this);
            break;
        }
    }
}

// 0x71B3B0
void CGameSprite::CheckToolTipItem(BYTE buttonNum)
{
    SHORT nItemNum = m_quickItems[buttonNum].m_abilityId.m_itemNum;
    if (nItemNum != -1) {
        CItem* pItem = m_equipment.m_items[nItemNum];
        if (pItem != NULL) {
            // FIXME: Calls `GetGenericName` twice.
            m_quickItems[nItemNum].m_name = pItem->GetGenericName();
            m_quickItems[nItemNum].m_abilityId.m_strDescription = pItem->GetGenericName();
        }
    }
}

// 0x71B400
CItem* CGameSprite::GetQuickItem(BYTE buttonNum)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 18909
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_ITEMS21);

    SHORT nItemNum = m_quickItems[buttonNum].m_abilityId.m_itemNum;
    if (nItemNum != -1) {
        return m_equipment.m_items[nItemNum];
    } else {
        return NULL;
    }
}

// 0x71B460
void CGameSprite::UnequipAll(BOOL animationOnly)
{
    if (m_equipment.m_items[6] != NULL) {
        m_equipment.m_items[6]->Unequip(this, 6, TRUE, animationOnly);
    }

    if (m_equipment.m_items[1] != NULL) {
        m_equipment.m_items[1]->Unequip(this, 1, TRUE, animationOnly);
    }

    if (m_equipment.m_items[5] != NULL) {
        m_equipment.m_items[5]->Unequip(this, 5, TRUE, animationOnly);
    }

    if (m_equipment.m_items[7] != NULL) {
        m_equipment.m_items[7]->Unequip(this, 7, TRUE, animationOnly);
    }

    if (m_equipment.m_items[8] != NULL) {
        m_equipment.m_items[8]->Unequip(this, 8, TRUE, animationOnly);
    }

    if (m_equipment.m_items[0] != NULL) {
        m_equipment.m_items[0]->Unequip(this, 0, TRUE, animationOnly);
    }

    if (m_equipment.m_items[2] != NULL) {
        m_equipment.m_items[2]->Unequip(this, 2, TRUE, animationOnly);
    }

    if (m_equipment.m_items[3] != NULL) {
        m_equipment.m_items[3]->Unequip(this, 3, TRUE, animationOnly);
    }

    if (m_equipment.m_items[4] != NULL) {
        m_equipment.m_items[4]->Unequip(this, 4, TRUE, animationOnly);
    }

    BYTE nIndex = 2 * (m_nWeaponSet + 22);
    if (m_equipment.m_items[nIndex] != NULL) {
        m_equipment.m_items[nIndex]->Unequip(this, nIndex, TRUE, animationOnly);
    }

    if (m_equipment.m_items[m_equipment.m_selectedWeapon] != NULL) {
        m_equipment.m_items[m_equipment.m_selectedWeapon]->Unequip(this, m_equipment.m_selectedWeapon, TRUE, animationOnly);

        m_equipment.m_items[m_equipment.m_selectedWeapon]->Demand();
        ITEM_ABILITY* pAbility = m_equipment.m_items[m_equipment.m_selectedWeapon]->GetAbility(m_equipment.m_selectedWeaponAbility);

        SHORT lSlot;
        CItem* pLauncher = GetLauncher(pAbility, lSlot);
        if (pLauncher != NULL) {
            pLauncher->Unequip(this, lSlot, TRUE, animationOnly);
        }

        m_equipment.m_items[m_equipment.m_selectedWeapon]->Release();
    }
}

// 0x71B5E0
void CGameSprite::EquipAll(BOOL animationOnly)
{
    if (m_equipment.m_items[6] != NULL) {
        m_equipment.m_items[6]->Equip(this, 6, animationOnly);
    }

    if (m_equipment.m_items[1] != NULL) {
        m_equipment.m_items[1]->Equip(this, 1, animationOnly);
    }

    if (m_equipment.m_items[5] != NULL) {
        m_equipment.m_items[5]->Equip(this, 5, animationOnly);
    }

    if (m_equipment.m_items[7] != NULL) {
        m_equipment.m_items[7]->Equip(this, 7, animationOnly);
    }

    if (m_equipment.m_items[8] != NULL) {
        m_equipment.m_items[8]->Equip(this, 8, animationOnly);
    }

    if (m_equipment.m_items[0] != NULL) {
        m_equipment.m_items[0]->Equip(this, 0, animationOnly);
    }

    if (m_equipment.m_items[2] != NULL) {
        m_equipment.m_items[2]->Equip(this, 2, animationOnly);
    }

    if (m_equipment.m_items[3] != NULL) {
        m_equipment.m_items[3]->Equip(this, 3, animationOnly);
    }

    if (m_equipment.m_items[4] != NULL) {
        m_equipment.m_items[4]->Equip(this, 4, animationOnly);
    }

    BOOL v1 = FALSE;
    BYTE v2 = 2 * m_nWeaponSet + 43;
    if (m_equipment.m_selectedWeapon == v2) {
        v1 = TRUE;
    }

    if (m_equipment.m_items[m_equipment.m_selectedWeapon] != NULL) {
        m_equipment.m_items[m_equipment.m_selectedWeapon]->Equip(this, m_equipment.m_selectedWeapon, animationOnly);

        m_equipment.m_items[m_equipment.m_selectedWeapon]->Demand();

        ITEM_ABILITY* pAbility = m_equipment.m_items[m_equipment.m_selectedWeapon]->GetAbility(m_equipment.m_selectedWeaponAbility);

        SHORT lSlot;
        CItem* pLauncher = GetLauncher(pAbility, lSlot);
        if (pLauncher != NULL) {
            pLauncher->Equip(this, lSlot, animationOnly);

            if (lSlot == v2) {
                v1 = TRUE;
            }
        }
    }

    BYTE v3 = 2 * (m_nWeaponSet + 22);
    if (m_equipment.m_items[v3] != NULL && v1 == TRUE) {
        m_equipment.m_items[v3]->Equip(this, v3, animationOnly);
    }
}

// 0x71B770
void CGameSprite::Equip(SHORT slotNum)
{
    if (slotNum <= 51) {
        if (m_equipment.m_items[slotNum] != NULL) {
            if (slotNum >= 0 && slotNum <= 8) {
                m_equipment.m_items[slotNum]->Equip(this, slotNum, FALSE);
            }
        }
    }
}

// 0x71B7A0
void CGameSprite::Unequip(SHORT slotNum)
{
    if (slotNum <= 51) {
        if (m_equipment.m_items[slotNum] != NULL) {
            switch (slotNum) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                m_equipment.m_items[slotNum]->Unequip(this, slotNum, TRUE, FALSE);
                break;
            case 11:
            case 12:
            case 13:
            case 14:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
                if (slotNum == m_equipment.m_selectedWeapon) {
                    m_equipment.m_items[slotNum]->Unequip(this, slotNum, TRUE, FALSE);

                    m_equipment.m_items[slotNum]->Demand();

                    ITEM_ABILITY* pAbility = m_equipment.m_items[slotNum]->GetAbility(m_equipment.m_selectedWeaponAbility);

                    SHORT lSlot;
                    CItem* pLauncher = GetLauncher(pAbility, lSlot);
                    if (pLauncher != NULL) {
                        pLauncher->Unequip(this, lSlot, TRUE, FALSE);
                    }

                    m_equipment.m_items[slotNum]->Release();

                    SelectWeaponAbility(10, 0, 0, 1);
                } else {
                    m_equipment.m_items[slotNum]->Unequip(this, slotNum, TRUE, FALSE);
                }
                break;
            }
        }
    }
}

// 0x71B8D0
SHORT CGameSprite::GetTurnUndeadLevel()
{
    DWORD nLevel = m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_CLERIC);

    // FIXME: Calculates paladin class twice.
    if (m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_PALADIN) > 2) {
        nLevel += m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_PALADIN) - 2;
    }

    return static_cast<SHORT>(nLevel);
}

// 0x71B910
SHORT CGameSprite::GetBackstabDamageMultiplier()
{
    if (!m_typeAI.IsClassValid(CAIObjectType::C_ROGUE)) {
        return 0;
    }

    // NOTE: Uninline.
    CDerivedStats* DStats = GetActiveStats();

    INT nLevel = DStats->GetBardMonkRogueLevel();
    return static_cast<SHORT>(atol(g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_tBackstabMultiplier.GetAt(CPoint(nLevel, 0))));
}

// 0x71B9A0
SHORT CGameSprite::GetLayOnHandsAmount()
{
    return g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetLayOnHandsAmount(m_typeAI, m_derivedStats);
}

// 0x71B9C0
void CGameSprite::AddSpecialAbility(const CResRef& cResInnateSpell, BOOL feedBack)
{
    // TODO: Incomplete.
}

// 0x71BC60
void CGameSprite::RemoveSpecialAbility(const CResRef& cResInnateSpell)
{
    // TODO: Incomplete.
}

// 0x71BE80
void CGameSprite::CheckLoadState()
{
    if (m_baseStats.m_generalState == 0) {
        return;
    }

    if ((m_baseStats.m_generalState & STATE_DEAD) != 0) {
        SetSequence(SEQ_TWITCH);
    }

    if ((m_baseStats.m_generalState & STATE_FLAME_DEATH) != 0) {
        GetAnimation()->SetColorEffectAll(0, RGB(75, 75, 75), 1);
    }

    if ((m_baseStats.m_generalState & STATE_STONE_DEATH) != 0) {
        m_animationRunning = FALSE;
        if (GetAnimation()->IsFalseColor()) {
            m_hasColorRangeEffects = TRUE;

            for (BYTE range = 0; range < 7; range++) {
                CColorRange* pColorRange = new CColorRange();
                pColorRange->m_range = range;
                pColorRange->m_color = CVidPalette::STONE;
                m_derivedStats.m_appliedColorRanges.AddTail(pColorRange);

                GetAnimation()->SetColorRange(range, CVidPalette::STONE);
            }
        }
    }

    if ((m_baseStats.m_generalState & STATE_FROZEN_DEATH) != 0) {
        m_animationRunning = FALSE;
        if (GetAnimation()->IsFalseColor()) {
            m_hasColorRangeEffects = TRUE;

            for (BYTE range = 0; range < 7; range++) {
                CColorRange* pColorRange = new CColorRange();
                pColorRange->m_range = range;
                pColorRange->m_color = CVidPalette::ICE;
                m_derivedStats.m_appliedColorRanges.AddTail(pColorRange);

                GetAnimation()->SetColorRange(range, CVidPalette::ICE);
            }
        }
    }
}

// 0x71C0A0
INT CGameSprite::GetNumQuickWeaponSlots()
{
    return g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetNumQuickWeaponSlots(m_typeAI.m_nClass);
}

// 0x71D310
SHORT CGameSprite::FindItemPersonal(const CString& sName, int number, BOOL restrictToEquiped)
{
    CString sMutableName(sName);
    sMutableName.MakeUpper();

    SHORT end = 51;
    if (restrictToEquiped) {
        end = 18;
    }

    for (SHORT slot = 0; slot < end; slot++) {
        if (m_equipment.m_items[slot] != NULL
            && m_equipment.m_items[slot]->cResRef == sName) {
            if (number > 0) {
                number--;
            } else {
                return slot;
            }
        }
    }

    return 0;
}

// 0x71D3D0
SHORT CGameSprite::CountItemPersonal(const CString& sName)
{
    SHORT number = 0;

    CString sMutableName(sName);
    sMutableName.MakeUpper();

    for (SHORT slot = 0; slot < 51; slot++) {
        if (m_equipment.m_items[slot] != NULL
            && m_equipment.m_items[slot]->cResRef == sName) {
            if (m_equipment.m_items[slot]->GetMaxStackable() > 1) {
                number += m_equipment.m_items[slot]->GetUsageCount(0);
            } else {
                number++;
            }
        }
    }

    return number;
}

// 0x71D480
DWORD CGameSprite::GetCarriedWeight()
{
    DWORD weight = 0;

    for (int index = 0; index < 51; index++) {
        if (m_equipment.m_items[index] != NULL) {
            weight += m_equipment.m_items[index]->GetWeight();
        }
    }

    return weight;
}

// 0x71D4B0
void CGameSprite::GetQuickWeapon(BYTE buttonNum, CButtonData& buttonData)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 20520
    UTIL_ASSERT_MSG(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22, "Trying to access illegal quick weapon slot");

    if (m_equipment.m_items[42] != NULL) {
        CGameButtonList* buttons = GetItemUsages(42, 1, 0);
        if (!buttons->IsEmpty()) {
            CButtonData* node = buttons->RemoveHead();
            if (node != NULL) {
                buttonData = *node;
                delete node;
            }
            while (!buttons->IsEmpty()) {
                delete buttons->RemoveHead();
            }
        }
        delete buttons;
    }

    buttonData = m_quickWeapons[buttonNum];

    if (buttonNum == 0
        || buttonNum == 2
        || buttonNum == 4
        || buttonNum == 6) {
        CItem* pItem = m_equipment.m_items[buttonNum + 44];
        if (pItem != NULL) {
            // FIXME: Calls `GetItemType` four times.
            if (pItem->GetItemType() == 47
                || pItem->GetItemType() == 53
                || pItem->GetItemType() == 49
                || pItem->GetItemType() == 41) {
                if (buttonData.m_abilityId.m_itemNum == -1
                    || buttonData.m_abilityId.m_itemNum == 10) {
                    CGameButtonList* buttons = GetItemUsages(10, 1, 0);
                    // Original 0x71D4B0 calls RemoveHead unconditionally here,
                    // assuming the fist slot always yields a usage.  In the
                    // reconstructed debug build MFC asserts when that list is
                    // empty, so keep original behavior when present but guard
                    // the empty-list case.
                    if (!buttons->IsEmpty()) {
                        CButtonData* node = buttons->RemoveHead();
                        if (node != NULL) {
                            buttonData = *node;
                            m_quickWeapons[buttonNum] = *node;
                            delete node;
                        }
                    }
                    while (!buttons->IsEmpty()) {
                        delete buttons->RemoveHead();
                    }
                    delete buttons;
                }
            }
        } else if (buttonData.m_abilityId.m_itemNum == -1) {
            if (buttonData.m_abilityId.m_itemNum == -1
                || buttonData.m_abilityId.m_itemNum == 10) {
                CGameButtonList* buttons = GetItemUsages(10, 1, 0);
                // Original 0x71D4B0 calls RemoveHead unconditionally here,
                // assuming the fist slot always yields a usage.  In the
                // reconstructed debug build MFC asserts when that list is
                // empty, so keep original behavior when present but guard
                // the empty-list case.
                if (!buttons->IsEmpty()) {
                    CButtonData* node = buttons->RemoveHead();
                    if (node != NULL) {
                        buttonData = *node;
                        m_quickWeapons[buttonNum] = *node;
                        delete node;
                    }
                }
                while (!buttons->IsEmpty()) {
                    delete buttons->RemoveHead();
                }
                delete buttons;
            }
        }
    } else {
        CItem* pItem = m_equipment.m_items[buttonNum + 43];
        if (pItem != NULL) {
            // FIXME: Calls `GetItemType` four times.
            if (pItem->GetItemType() == 47
                || pItem->GetItemType() == 53
                || pItem->GetItemType() == 49
                || pItem->GetItemType() == 41) {
                buttonData.m_icon = pItem->GetItemIcon();
                buttonData.m_name = pItem->GetGenericName();
                buttonData.m_abilityId.m_itemType = 2;
                buttonData.m_abilityId.m_itemNum = buttonNum + 43;
                buttonData.m_abilityId.m_targetType = -1;
                buttonData.m_abilityId.m_strDescription = pItem->GetGenericName();
            }
        }
    }
}

// 0x71DAB0
void CGameSprite::SetScript(SHORT level, CAIScript* script)
{
    // FIXME: Unused.
    CString v1;

    switch (level) {
    case 0:
        if (m_overrideScript != NULL) {
            delete m_overrideScript;
            m_overrideScript = NULL;
        }

        m_overrideScript = script;
        m_overrideScript->m_cResRef.GetResRef(m_baseStats.m_scriptOverRide);
        break;
    case 1:
        if (m_special1Script != NULL) {
            delete m_special1Script;
            m_special1Script = NULL;
        }
        m_special1Script = script;
        // FIXME: Probably missing copying resref to `m_baseStats`.
        break;
    case 2:
        if (m_teamScript != NULL) {
            delete m_teamScript;
            m_teamScript = NULL;
        }

        m_teamScript = script;
        m_teamScript->m_cResRef.GetResRef(m_baseStats.m_scriptTeam);
        break;
    case 3:
        if (m_special2Script != NULL) {
            delete m_special2Script;
            m_special2Script = NULL;
        }
        m_special2Script = script;
        m_special2Script->m_cResRef.GetResRef(m_baseStats.m_scriptSpecial2);
        break;
    case 4:
        if (m_combatScript != NULL) {
            delete m_combatScript;
            m_combatScript = NULL;
        }
        m_combatScript = script;
        m_combatScript->m_cResRef.GetResRef(m_baseStats.m_scriptCombat);
        break;
    case 5:
        if (m_special3Script != NULL) {
            delete m_special3Script;
            m_special3Script = NULL;
        }
        m_special3Script = script;
        m_special3Script->m_cResRef.GetResRef(m_baseStats.m_scriptSpecial3);
        break;
    case 6:
        if (m_movementScript != NULL) {
            delete m_movementScript;
            m_movementScript = NULL;
        }
        m_movementScript = script;
        m_movementScript->m_cResRef.GetResRef(m_baseStats.m_scriptMovement);
        break;
    }
}

// 0x71DD20
void CGameSprite::InitializeWalkingSound()
{
    m_nSndWalk = 0;
    m_currSndWalk = 0;

    // NOTE: Uninline.
    char* pSndWalk = m_animation.GetSndWalk(m_pArea->m_search.GetTableIndex(m_pos));
    if (pSndWalk != NULL) {
        m_sndWalk[m_currSndWalk].SetResRef(CResRef(pSndWalk), TRUE, TRUE);
        if (pSndWalk[0] != '\0') {
            delete pSndWalk;
        }
    } else {
        m_sndWalk[m_currSndWalk].SetResRef(CResRef(""), TRUE, TRUE);
    }
}

// 0x71DFB0
void CGameSprite::AutoPause(DWORD type)
{
    COLORREF nameColor;
    STRREF strPause;

    if (type == 256
        || (type == 128 && g_pBaldurChitin->GetObjectGame()->GetOptions()->m_bAutoPauseOnTrap)
        || (type & g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nAutoPauseState) != 0) {
        if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1
            || g_pBaldurChitin->GetObjectGame()->IsAlly(m_id)
            || g_pBaldurChitin->GetObjectGame()->IsFamiliar(m_id)) {
            nameColor = CVidPalette::RANGE_COLORS[m_baseStats.m_colors[CVIDPALETTE_RANGE_MAIN_CLOTH]];
            if (g_pChitin->cNetwork.GetSessionOpen()
                || g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->GetScreenWorld()) {
                switch (type) {
                case 0x1:
                    strPause = 17113; // "Auto-Paused: Weapon Unusable"
                    break;
                case 0x2:
                    strPause = 17114; // "Auto-Paused: Attacked"
                    break;
                case 0x4:
                    strPause = 17115; // "Auto-Paused: Hit"
                    break;
                case 0x8:
                    strPause = 17116; // "Auto-Paused: Badly Wounded"
                    break;
                case 0x10:
                    strPause = 17117; // "Auto-Paused: Dead"
                    break;
                case 0x20:
                    strPause = 17118; // "Auto-Paused: Target Gone"
                    break;
                case 0x40:
                    strPause = 10014; // "Auto-Paused: Round End"
                    break;
                case 0x80:
                    strPause = 18559; // "Trap Detected"
                    break;
                case 0x100:
                    strPause = 7666; // "Auto-Paused: Scripted"
                    break;
                case 0x200:
                    strPause = 23516; // "Enemy Sighted"
                    break;
                case 0x400:
                    strPause = 26310; // "Spell Cast"
                    break;
                default:
                    strPause = 17324; // "Auto-Paused: Unknown Reason"
                    break;
                }
            } else {
                strPause = 17324; // "Auto-Paused: Unknown Reason"
            }

            CScreenWorld* pWorld = g_pBaldurChitin->GetScreenWorld();
            pWorld->m_autoPauseColor = nameColor;
            pWorld->m_autoPauseId = m_id;
            pWorld->m_autoPauseRef = strPause;
            pWorld->m_autoPauseName = GetNameRef();
        }
    }
}

// 0x71E1B0
void CGameSprite::PlayDialogSound(CGameSprite* pNPC)
{
    if (g_pChitin->cNetwork.GetSessionOpen()) {
        CMessage* message = new CMessagePlaySound(SOUND_SELECT,
            TRUE,
            TRUE,
            m_id,
            pNPC->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    } else {
        PlaySound(SOUND_SELECT, TRUE, TRUE, FALSE);
    }
}

// 0x71E9F0
void CGameSprite::AddKnownDivineSpells(const BYTE& nClass)
{
    switch (nClass) {
    case CAIOBJECTTYPE_C_CLERIC:
    case CAIOBJECTTYPE_C_DRUID:
    case CAIOBJECTTYPE_C_PALADIN:
    case CAIOBJECTTYPE_C_RANGER:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 21545
        UTIL_ASSERT(FALSE);
    }

    if ((nClass == CAIOBJECTTYPE_C_CLERIC && m_startTypeAI.IsClassValid(CAIOBJECTTYPE_C_CLERIC) != 0)
        || (nClass == CAIOBJECTTYPE_C_DRUID && m_startTypeAI.IsClassValid(CAIOBJECTTYPE_C_DRUID) != 0)
        || (nClass == CAIOBJECTTYPE_C_PALADIN && m_startTypeAI.IsClassValid(CAIOBJECTTYPE_C_PALADIN) != 0)
        || (nClass == CAIOBJECTTYPE_C_RANGER && m_startTypeAI.IsClassValid(CAIOBJECTTYPE_C_RANGER) != 0)) {
        // NOTE: Original code is slightly different (full of inlined
        // functions).
        UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);
        CGroupedSpellList* pGameSpells = &(g_pBaldurChitin->GetObjectGame()->m_spellsByClass[nClassIndex]);

        // NOTE: Uninline.
        CGameSpriteGroupedSpellList* pSpriteSpells = GetSpells(nClass);

        CResRef resRef;

        UINT nClassLevel = m_derivedStats.GetClassLevel(nClass);
        UINT nMaxSpellLevel = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetMaxSpellLevel(nClass, nClassLevel);

        for (UINT nLevel = 0; nLevel < nMaxSpellLevel; nLevel++) {
            for (UINT nIndex = 0; nIndex < pGameSpells->m_lists[nLevel].m_nCount; nIndex++) {
                // NOTE: Uninline.
                UINT nID = pGameSpells->m_lists[nLevel].Get(nIndex);

                UINT temp;
                if (!pSpriteSpells->m_lists[nLevel].Find(nID, temp)) {
                    // NOTE: Uninline.
                    resRef = g_pBaldurChitin->GetObjectGame()->m_spells.Get(nID);

                    CSpell cSpell;
                    cSpell.SetResRef(resRef, TRUE, TRUE);

                    DWORD dwClassFlags;
                    switch (nClass) {
                    case CAIOBJECTTYPE_C_CLERIC:
                    case CAIOBJECTTYPE_C_PALADIN:
                        dwClassFlags = 0x40000000;
                        break;
                    case CAIOBJECTTYPE_C_DRUID:
                    case CAIOBJECTTYPE_C_RANGER:
                        dwClassFlags = 0x80000000;
                        break;
                    default:
                        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                        // __LINE__: 21606
                        UTIL_ASSERT(FALSE);
                    }

                    if (cSpell.pRes != NULL) {
                        // FIXME: Calls `GetNotUsableBy` many times.
                        if ((cSpell.GetNotUsableBy() & dwClassFlags) != 0) {
                            continue;
                        }

                        if ((cSpell.GetNotUsableBy() & 0x4) != 0
                            && IcewindMisc::IsGood(this)) {
                            continue;
                        }

                        if ((cSpell.GetNotUsableBy() & 0x2) != 0
                            && IcewindMisc::IsEvil(this)) {
                            continue;
                        }

                        if ((cSpell.GetNotUsableBy() & 0x8) != 0
                            && !IcewindMisc::IsGood(this)
                            && !IcewindMisc::IsEvil(this)) {
                            continue;
                        }

                        if ((cSpell.GetNotUsableBy() & 0x10) != 0
                            && IcewindMisc::IsLawful(this)) {
                            continue;
                        }

                        if ((cSpell.GetNotUsableBy() & 0x1) != 0
                            && IcewindMisc::IsChaotic(this)) {
                            continue;
                        }

                        if ((cSpell.GetNotUsableBy() & 0x20) != 0
                            && !IcewindMisc::IsLawful(this)
                            && !IcewindMisc::IsChaotic(this)) {
                            continue;
                        }

                        // NOTE: Uninline.
                        bool bResult = AddKnownSpell(nClass, nLevel, resRef, 0, 0, 0);

                        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                        // __LINE__: 21648
                        UTIL_ASSERT(bResult == true);
                    }
                }
            }
        }
    }
}

// 0x71F170
void CGameSprite::DisplayTextRef(STRREF nameRef, STRREF textRef, COLORREF nameColor, COLORREF textColor)
{
    STR_RES strResName;
    STR_RES strResText;
    CString name;

    if (nameRef >= -7 && nameRef != -1) {
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(-2 - nameRef);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            name = pSprite->GetName();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(nameRef, strResName);
        name = strResName.szText;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(textRef, strResText);
    if (strResText.cSound.GetRes() != NULL) {
        if (!strResText.cSound.GetLooping()) {
            strResText.cSound.SetFireForget(TRUE);
        }
        strResText.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
        strResText.cSound.Play(GetPos().x, GetPos().y, 0, FALSE);
    }

    strResText.szText.TrimLeft();
    strResText.szText.TrimRight();
    if (strResText.szText != "") {
        g_pBaldurChitin->GetScreenWorld()->DisplayText(name,
            strResText.szText,
            nameColor,
            textColor,
            -1,
            FALSE);
    }
}

// 0x71FBA0
void CGameSprite::GetNumInventoryPersonalSlots(INT& nUsedSlots, INT& nTotalSlots)
{
    nTotalSlots = CScreenInventory::PERSONAL_INVENTORY_SIZE;
    nUsedSlots = 0;

    for (int index = 0; index < nTotalSlots; index++) {
        if (m_equipment.m_items[index + 18] != NULL) {
            nUsedSlots++;
        }
    }
}

// 0x71FC00
void CGameSprite::SetModalState(BYTE modalState, BOOL bUpdateToolbar)
{
    if (modalState == m_nModalState) {
        return;
    }

    // Stop the current modal mode before switching.  Modal states (MODAL.IDS):
    // 1 = bard song, 2 = detect traps / search, 3 = stealth, 4 = turn undead.
    WORD nFeedback;
    switch (m_nModalState) {
    case 1:
        // Bard song.  With the Lingering Song feat the song lingers one more
        // round: re-apply its ability effects to nearby allies (two rounds of
        // duration) and show the lingering-song feedback instead of "song
        // ended".  Recovered from 0x71FC41-0x720245 -- the same per-cycle
        // re-application as CheckModal case 0, but ally-filtered and longer.
        if (HasFeat(CGAMESPRITE_FEAT_LINGERING_SONG) == 1) {
            CInfGame* pLingerGame = g_pBaldurChitin->GetObjectGame();
            // A silenced singer (derived or base state) cannot sing; the game
            // flag at +0x43E6 (store/dialog/cutscene gate) also suppresses it.
            if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
                && (m_baseStats.m_generalState & STATE_SILENCED) == 0
                && *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pLingerGame) + 0x43E6) != 1) {
                UTIL_ASSERT(m_nLastSong < pLingerGame->m_songs.m_nCount);
                CResRef songRes = pLingerGame->m_songs.Get636(m_nLastSong);

                CSpell spell(songRes);
                spell.Demand();
                if (spell.GetCasterType() == 5) {
                    FeedBack(FEEDBACK_LINGERINGSONG, 0, 0, 0, spell.GetGenericName(), 0, 0);
                    SPELL_ABILITY* pAbility = spell.GetAbility(0);

                    // Gather everyone within 400 of the singer, plus the singer.
                    CTypedPtrList<CPtrList, LONG*> targets;
                    m_pArea->GetCloseObjects(GetVertListPos(), GetPos(), CAIObjectType::ANYONE,
                        400, GetTerrainTable(), targets, TRUE, FALSE);
                    m_pArea->GetAllInRangeBack(GetPos(), CAIObjectType::ANYONE, 400,
                        GetTerrainTable(), targets, TRUE, FALSE, FALSE);
                    targets.AddTail(reinterpret_cast<LONG*>(m_id));

                    SHORT nEffects = *reinterpret_cast<SHORT*>(reinterpret_cast<BYTE*>(pAbility) + 0x1E);

                    POSITION pos = targets.GetHeadPosition();
                    while (pos != NULL) {
                        LONG allyId = reinterpret_cast<LONG>(targets.GetNext(pos));

                        CGameObject* pAlly;
                        BYTE rc = pLingerGame->GetObjectArray()->GetShare(allyId,
                            CGameObjectArray::THREAD_ASYNCH, &pAlly, INFINITE);
                        if (rc != CGameObjectArray::SUCCESS) {
                            continue;
                        }

                        if (pAlly->GetObjectType() == CGameObject::TYPE_SPRITE) {
                            CGameSprite* pAllySprite = static_cast<CGameSprite*>(pAlly);

                            // SPIN147 (the hostile Siren's Yearning) is meant to
                            // target enemies, so it gets its own pre-filter that
                            // skips allies, charm-immune sprites, and sprites
                            // flagged at +0xA10 & 0x40.  But this branch and the
                            // every-other-song branch both fall into the same final
                            // AreAllies gate below (0x71FFCB), which only lets allies
                            // through -- so the enemy pre-filter is dead code and
                            // SPIN147 never lingers on anyone.  Latent bug in
                            // IWD2.exe, reproduced faithfully.  (Its per-round
                            // application in CheckModal case 0 has no ally gate, so
                            // it does reach enemies while the song is being sung.)
                            BOOL skip = FALSE;
                            if (songRes == "SPIN147"
                                && (IcewindMisc::AreAllies(this, pAllySprite)
                                    || IcewindMisc::sud_585070(pAllySprite)
                                    || (*(reinterpret_cast<BYTE*>(pAllySprite) + 0xA10) & 0x40))) {
                                skip = TRUE;
                            }

                            // Final ally gate (0x71FFCB) -- both branches funnel here.
                            if (!skip && IcewindMisc::AreAllies(this, pAllySprite)) {
                                CMessage105::Node localHead;
                                localHead.m_pNext = &localHead;
                                localHead.m_pPrev = &localHead;

                                for (SHORT i = 0; i < nEffects; i++) {
                                    CGameEffect* pEff = spell.BuildAbilityEffect(0, i, this, 0, 0, 0);
                                    *reinterpret_cast<LONG*>(reinterpret_cast<BYTE*>(pEff) + 0x7C) = m_pos.x;
                                    *reinterpret_cast<LONG*>(reinterpret_cast<BYTE*>(pEff) + 0x80) = m_pos.y;
                                    pEff->m_sourceID = m_id;
                                    IcewindMisc::ApplyDamageModifiers(this, pEff);
                                    // Re-tag a "Duration"-timed song effect to expire
                                    // two rounds out (game time + 200) so the lingering
                                    // buff survives instead of reading as already past.
                                    if (*reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pEff) + 0x20) == 0
                                        && *reinterpret_cast<LONG*>(reinterpret_cast<BYTE*>(pEff) + 0x24) == 7) {
                                        pEff->m_durationType = 0x1000;
                                        pEff->m_duration =
                                            g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + 200;
                                    }

                                    CMessageAddEffect* pSub = new CMessageAddEffect(pEff, m_id, allyId);
                                    CMessage105::Node* pNode = new CMessage105::Node;
                                    pNode->m_pMessage = pSub;
                                    pNode->m_pNext = &localHead;
                                    pNode->m_pPrev = localHead.m_pPrev;
                                    localHead.m_pPrev->m_pNext = pNode;
                                    localHead.m_pPrev = pNode;
                                }

                                CMessage* pMsg = new CMessage105(&localHead, m_id, allyId);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);

                                CMessage105::Node* p = localHead.m_pNext;
                                while (p != &localHead) {
                                    CMessage105::Node* pNext = p->m_pNext;
                                    delete p;
                                    p = pNext;
                                }
                            }
                        }

                        pLingerGame->GetObjectArray()->ReleaseShare(allyId,
                            CGameObjectArray::THREAD_ASYNCH, INFINITE);
                    }
                }
                spell.Release();
            }
            goto updateState;
        }
        nFeedback = FEEDBACK_BATTLESONGEND;
        break;
    case 2:
        nFeedback = FEEDBACK_SEARCHEND;
        break;
    case 3:
        if (m_bHiding) {
            FeedBack(FEEDBACK_LEAVING_SHADOWS, 0, 0, 0, -1, 0, 0);
            m_bHiding = FALSE;
        }
        goto updateState;
    case 4:
        nFeedback = FEEDBACK_TURNUNDEADEND;
        break;
    default:
        goto updateState;
    }
    FeedBack(nFeedback, 0, 0, 0, -1, 0, 0);

updateState:
    m_nModalState = modalState;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (bUpdateToolbar && pGame->m_group.GetGroupLeader() == m_id) {
        pGame->m_cButtonArray.UpdateButtons();
    }
}

// 0x7202E0
BOOL CGameSprite::CanAct()
{
    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene) {
        return FALSE;
    } else {
        return (m_curAction.m_internalFlags & 0x1) == 0;
    }
}

// 0x720310
void CGameSprite::CheckSequence(BYTE& sequence)
{
    // TODO: Incomplete.
}

// 0x7204C0
void CGameSprite::LoadSoundEntries()
{
    CString animationResRef;
    CMemINI* pINI = &(g_pBaldurChitin->GetObjectGame()->m_INISounds);
    CMemINIValue* pValue;

    // NOTE: Uninline.
    m_animation.GetAnimationResRef(animationResRef, CGameAnimationType::RANGE_BODY);

    field_70FB = FALSE;
    field_70FC = FALSE;
    field_70FD = FALSE;
    field_70FE = FALSE;
    field_70FF = FALSE;
    field_7100 = FALSE;
    field_7101 = FALSE;

    pValue = pINI->GetFast(animationResRef, CString("att1"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_70FB = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("att1frame")));

        pValue = pINI->GetFast(animationResRef, CString("att2"));
        if (pValue != NULL && pValue->GetValue() != "") {
            ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("att2frame")));
        }

        pValue = pINI->GetFast(animationResRef, CString("att3"));
        if (pValue != NULL && pValue->GetValue() != "") {
            ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("att3frame")));
        }

        pValue = pINI->GetFast(animationResRef, CString("att4"));
        if (pValue != NULL && pValue->GetValue() != "") {
            ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("att4frame")));
        }
    }

    pValue = pINI->GetFast(animationResRef, CString("btlcry"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_70FC = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("btlcryframe")));
    }

    pValue = pINI->GetFast(animationResRef, CString("damage"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_70FD = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("damageframe")));
    }

    pValue = pINI->GetFast(animationResRef, CString("death"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_70FE = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("deathframe")));
    }

    pValue = pINI->GetFast(animationResRef, CString("fidget"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_70FF = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("fidgetframe")));
    }

    pValue = pINI->GetFast(animationResRef, CString("selected"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_7100 = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("selectedframe")));
    }

    pValue = pINI->GetFast(animationResRef, CString("fall"));
    if (pValue != NULL && pValue->GetValue() != "") {
        field_7101 = TRUE;
        ParseSoundEntry(pValue, pINI->GetFast(animationResRef, CString("fallframe")));
    }
}

// 0x720B50
void CGameSprite::ParseSoundEntry(CMemINIValue* a1, CMemINIValue* a2)
{
    CString v1;
    CString v2;
    CString v3(a1->GetValue());
    CString v4;
    if (a2 != NULL) {
        v4 = a2->GetValue();
    }
    CGameSpriteSoundEntry entry;
    INT pos1;
    INT pos2;

    if (v3[v3.GetLength() - 1] != ',') {
        v3 += ',';
    }

    if (v4 != "" && v4[v4.GetLength() - 1] != ',') {
        v4 += ',';
    }

    if (a1->GetName() == "att1") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_72F0.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "att2") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_730C.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "att3") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_7328.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "att4") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_7344.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "btlcry") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_7360.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "damage") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_737C.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "death") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_7398.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "fidget") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_73B4.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "selected") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_73D0.AddTail(entry);
            }
        }
    }

    if (a1->GetName() == "fall") {
        while (1) {
            v1 = "";
            v2 = "";
            entry.field_0 = "";
            entry.field_4 = 0;

            if (v3.IsEmpty()) {
                break;
            }

            pos1 = v3.FindOneOf(",");
            pos2 = -1;
            if (!v4.IsEmpty()) {
                pos2 = v4.FindOneOf(",");
            }

            if (pos1 >= 0) {
                v1 = v3.Left(pos1);
                v3 = v3.Right(v3.GetLength() - pos1 - 1);
                v3.TrimRight();
                v3.TrimLeft();
                v1.TrimRight();
                v1.TrimLeft();
            }

            if (pos2 >= 0) {
                v2 = v4.Left(pos2);
                v4 = v4.Right(v4.GetLength() - pos2 - 1);
                v4.TrimRight();
                v4.TrimLeft();
                v2.TrimRight();
                v2.TrimLeft();
            }

            if (!v1.IsEmpty()) {
                entry.field_0 = v1;
                if (!v2.IsEmpty()) {
                    entry.field_4 = atoi(v2);
                } else {
                    entry.field_4 = 0;
                }
                field_73EC.AddTail(entry);
            }
        }
    }
}

// 0x722530
SHORT CGameSprite::FindItemBags(const CString& sName, LONG number, BOOL checkForIdentified)
{
    CString sMutableName(sName);
    sMutableName.MakeUpper();

    if (number <= 1) {
        number = 1;
    }

    for (SHORT slot = 0; slot < 51; slot++) {
        CItem* pItem = m_equipment.m_items[slot];
        if (pItem != NULL && pItem->GetItemType() == 58) {
            number -= g_pBaldurChitin->GetObjectGame()->FindItemInStore(pItem->GetResRef(),
                CResRef(sMutableName),
                checkForIdentified);
            if (number <= 0) {
                return slot;
            }
        }
    }

    return -1;
}

// 0x722610
SHORT CGameSprite::TakeItemBags(const CString& sName, LONG number, SHORT slotNum)
{
    CString sMutableName(sName);
    sMutableName.MakeUpper();

    SHORT numTaken = 0;
    if (slotNum != -1) {
        CItem* pItem = m_equipment.m_items[slotNum];
        if (pItem != NULL
            && pItem->GetItemType() == 58) {
            numTaken = g_pBaldurChitin->GetObjectGame()->TakeItemFromStore(pItem->GetResRef(),
                CResRef(sName),
                number);
        }
    } else {
        for (SHORT slot = 0; slot < 51; slot++) {
            CItem* pItem = m_equipment.m_items[slot];
            if (pItem != NULL && pItem->GetItemType() == 58) {
                numTaken += g_pBaldurChitin->GetObjectGame()->TakeItemFromStore(pItem->GetResRef(),
                    CResRef(sName),
                    number);
                if (number - numTaken <= 0) {
                    break;
                }
            }
        }
    }
    return numTaken;
}

// Returns `TRUE` if object contains quest items or containers (such as potion
// bags). See STREF #26585.
//
// 0x723350
BOOL CGameSprite::HaveUnexportableItems()
{
    for (INT nSlot = 0; nSlot < 51; nSlot++) {
        CItem* pItem = m_equipment.m_items[nSlot];
        if (pItem != NULL) {
            if (pItem->GetItemType() == 58
                || (pItem->GetFlagsFile() & 0x800) != 0) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

// 0x723390
void CGameSprite::SetMonkAbilities()
{
    if (GetAIType().IsClassValid(CAIObjectType::C_MONK)) {
        if (m_equipment.m_selectedWeapon == 10) {
            if (m_equipment.m_items[10] != NULL) {
                m_equipment.m_items[10]->Unequip(this, 10, FALSE, FALSE);
            }
        }

        if (m_equipment.m_items[10] != NULL) {
            delete m_equipment.m_items[10];
        }

        // NOTE: Uninline.
        CDerivedStats* pStats = GetActiveStats();

        CItem* pItem;
        switch (pStats->GetClassLevel(CAIOBJECTTYPE_C_MONK)) {
        case 1:
        case 2:
        case 3:
            pItem = new CItem(CResRef("00MFIST1"), 0, 0, 0, 0, 0);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            pItem = new CItem(CResRef("00MFIST2"), 0, 0, 0, 0, 0);
            break;
        case 8:
        case 9:
            pItem = new CItem(CResRef("00MFIST3"), 0, 0, 0, 0, 0);
            break;
        case 10:
        case 11:
            pItem = new CItem(CResRef("00MFIST4"), 0, 0, 0, 0, 0);
            break;
        case 12:
            pItem = new CItem(CResRef("00MFIST5"), 0, 0, 0, 0, 0);
            break;
        case 13:
        case 14:
        case 15:
            pItem = new CItem(CResRef("00MFIST6"), 0, 0, 0, 0, 0);
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            pItem = new CItem(CResRef("00MFIST7"), 0, 0, 0, 0, 0);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            pItem = new CItem(CResRef("00MFIST8"), 0, 0, 0, 0, 0);
            break;
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
            pItem = new CItem(CResRef("00MFIST9"), 0, 0, 0, 0, 0);
            break;
        default:
            pItem = new CItem(CResRef("00MFIST1"), 0, 0, 0, 0, 0);
            break;
        }

        m_equipment.m_items[10] = pItem;

        if (m_equipment.m_selectedWeapon == 10) {
            m_equipment.m_items[10]->Equip(this, 10, FALSE);
        }
    }
}

// 0x7238A0
LONG CGameSprite::GetLevel()
{
    return m_derivedStats.GetAtOffset(STAT_CLASSLEVELSUM);
}

// 0x723B20
void CGameSpriteLastUpdate::Initialize(BOOL bFullUpdateRequired)
{
    // TODO: Incomplete.
}

// 0x723BF0
void CGameSprite::SetHideState(BOOLEAN a1, BOOLEAN a2)
{
    // TODO: Incomplete.
}

// 0x723CC0
INT CGameSprite::GetAC()
{
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    INT nDexBonus = GetMaxDexterityBonus(ruleTables.GetAbilityScoreModifier(m_derivedStats.m_nDEX));
    if ((m_derivedStats.m_generalState & STATE_BLIND) != 0
        && nDexBonus > 0
        && !HasFeat(6)) {
        nDexBonus = 0;
    }

    INT nArmorBonus = m_derivedStats.m_nACArmorBonus;
    if (nArmorBonus == 0 && m_equipment.m_items[CGameSpriteEquipment::SLOT_ARMOR] != NULL) {
        nArmorBonus += m_equipment.m_items[CGameSpriteEquipment::SLOT_ARMOR]->GetEquippedACBonus();
    }

    INT nShieldBonus = m_derivedStats.m_nACDeflectionBonus;
    INT nOffhandSlot = CGameSpriteEquipment::SLOT_WEAPON + 2 * m_nWeaponSet + 1;
    CItem* pShield = NULL;
    if (nOffhandSlot >= 0 && nOffhandSlot < CGameSpriteEquipment::NUM_SLOT) {
        CItem* pItem = m_equipment.m_items[nOffhandSlot];
        if (pItem != NULL) {
            WORD nItemType = pItem->GetItemType();
            if (nItemType == 41 || nItemType == 47 || nItemType == 49 || nItemType == 53) {
                pShield = pItem;
            }
        }
    }

    if (pShield == NULL && m_equipment.m_items[CGameSpriteEquipment::SLOT_SHIELD] != NULL) {
        pShield = m_equipment.m_items[CGameSpriteEquipment::SLOT_SHIELD];
    }

    if (pShield != NULL) {
        nShieldBonus = max(nShieldBonus, pShield->GetEquippedACBonus());
    }

    return 10
        + nArmorBonus
        + m_derivedStats.m_nACNaturalBonus
        + nShieldBonus
        + m_derivedStats.m_nACDodgeBonus
        + nDexBonus
        + GetAttacksPerRound();
}

// 0x723F60
INT CGameSprite::GetAttacksPerRound()
{
    if (GetClassLevel(CAIOBJECTTYPE_C_MONK) == 0) {
        return 0;
    }

    if (m_equipment.m_items[CGameSpriteEquipment::SLOT_ARMOR] != NULL) {
        return 0;
    }

    BYTE nWeaponSlot = static_cast<BYTE>((m_nWeaponSet + 0x16) * 2);
    if (m_equipment.m_items[nWeaponSlot] != NULL) {
        WORD nItemType = m_equipment.m_items[nWeaponSlot]->GetItemType();
        if (nItemType == 47 || nItemType == 53 || nItemType == 49 || nItemType == 41) {
            return 0;
        }
    }

    INT nWisBonus = g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetAbilityScoreModifier(m_derivedStats.m_nWIS);
    if (nWisBonus < 1) {
        return 0;
    }

    return nWisBonus;
}

// 0x72DE60
void CGameSprite::sub_72DE60()
{
    ProcessEffectList();
}

// 0x72B9A0 (virtual)
void CGameSprite::ProcessAI()
{
    if (!ProcessEffectList()) {
        return;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    if (pWorld->m_bPaused == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE
        && pGame->GetGameSave()->m_cutScene == TRUE) {
        pWorld->TogglePauseGame(0, 1, 0);
    }

    if (m_userCommandPause > 0) {
        m_baseStats.m_flags &= 0x7FFFFFFF;
    }
    if (m_nCommandPause > 0) {
        m_nCommandPause--;
    }

    m_randValue = rand() % 0x7FFF;

    if ((m_derivedStats.m_generalState & STATE_DEAD) != 0
        && m_nSequence != SEQ_AWAKE
        && m_nSequence != SEQ_DIE
        && m_nSequence != SEQ_TWITCH) {
        SetSequence(SEQ_TWITCH);
    }

    if (m_pPath == NULL && m_nSequence == SEQ_WALK) {
        if (m_animation.m_animation != NULL
            && m_animation.m_nSequence != SEQ_WALK) {
            SetIdleSequence();
        }
    }

    if (m_bSelected && !Orderable(FALSE)) {
        Unselect();
        pGame->SelectToolbar();
    }

    SHORT curActionId = m_curAction.GetActionID();
    if (m_bEscapingArea
        || curActionId == CAIAction::LEAVEAREALUA
        || curActionId == 183
        || m_currentActionId == CAIAction::LEAVEAREALUA
        || m_currentActionId == 183
        || curActionId == CAIAction::STARTDIALOG
        || curActionId == CAIAction::DIALOGUE
        || curActionId == CAIAction::STARTDIALOGUENOSET
        || m_currentActionId == CAIAction::STARTDIALOG
        || m_currentActionId == CAIAction::DIALOGUE
        || m_currentActionId == CAIAction::STARTDIALOGUENOSET
        || curActionId == 207
        || curActionId == 208
        || m_currentActionId == 207
        || m_currentActionId == 208
        || !CanAct()) {
        m_dialogWait = 0;
    }

    if (m_curAction.GetActionID() == CAIAction::NO_ACTION
        && !m_queuedActions.IsEmpty()) {
        SetCurrAction(GetNextAction(m_aiDoAction));
        m_interrupt = FALSE;
        m_actionCount = 0;
        m_nCommandPause = 0;
    }

    BOOL bWorldTimerActive = pGame->GetWorldTimer()->m_active;
    if (!bWorldTimerActive || m_inCutScene) {
        if (m_curAction.GetActionID() == CAIAction::NO_ACTION
            && !m_queuedActions.IsEmpty()) {
            SetCurrAction(GetNextAction(m_aiDoAction));
            m_interrupt = FALSE;
        }

        ResolvePausedAction(&m_curAction, m_queuedActions.GetHeadPosition());

        if (m_interrupt && (m_baseStats.m_flags & 0x80000000) == 0) {
            SetCurrAction(GetNextAction(m_aiDoAction));
            m_interrupt = FALSE;
        }

        BOOL bResolveAction = bWorldTimerActive;
        if (!bWorldTimerActive) {
            bResolveAction = g_pBaldurChitin->GetObjectGame()
                                 ->GetRuleTables()
                                 .m_lInstantActions.Find(m_curAction.GetActionID())
                != NULL;
        }

        if (bResolveAction
            && !(g_pChitin->cNetwork.GetSessionOpen() == TRUE
                && m_curAction.GetActionID() == CAIAction::LEAVEAREALUA)) {
            // TODO INCOMPLETE: this cutscene fast path stands in for the
            // unrecovered active branch at 0x72DAC5. Runtime traces show active
            // cutscene sprites use dropNonInstants=TRUE; paused/timer-stopped
            // instant-only resolution still uses FALSE.
            ResolveInstants(bWorldTimerActive);
        }

        if (!m_groupMove) {
            ResolveTargetPoint(&m_curAction, m_queuedActions.GetHeadPosition());
        }
        return;
    }

    if (m_nUnselectableCounter > 0) {
        m_nUnselectableCounter--;
    }

    m_userCommandPause--;
    m_nCommandPause--;
    field_44A++;

    // TODO INCOMPLETE: 0x72B9A0 still has unrecovered active-combat branches
    // for morale failure, modal actions, berserk, panic and invisibility. The
    // recovered path below keeps the exact paused action loop used by chapter
    // text before delegating active script selection to the recovered
    // CGameAIBase implementation until those branches are ported.
    CGameAIBase::ProcessAI();

    // 0x72DA32: script timers tick every 16th AI frame, staggered per id, so
    // TimerActive() expires and the 00AMVW wander loop repaces (2-5s).
    if (((m_id ^ field_44A) & 0xF) == 0) {
        CheckTimers(1);
    }

    // 0x72D816: per-round modal / passive-ability update (bard song, search,
    // stealth, turn undead).  The binary also runs a nearest-enemy acquisition
    // pass (0x72FBF0) immediately before this; that remains unrecovered.
    CheckModal();

    if (ProcessEffectList()
        && m_pArea != NULL
        && (m_dialogWait < 1
            || m_typeAI.m_nEnemyAlly >= CAIObjectType::EA_GOODCUTOFF
            || m_bAllowDialogInterrupt == FALSE)) {
        BOOL bCanResolveInstants = (m_derivedStats.m_generalState & STATE_DEAD) == 0;
        bCanResolveInstants = bCanResolveInstants
            && IsHelpless() == FALSE
            && (m_derivedStats.m_generalState & (STATE_STONE_DEATH | STATE_FROZEN_DEATH)) == 0;

        BOOL bCurrentInstant = g_pBaldurChitin->GetObjectGame()
                                   ->GetRuleTables()
                                   .m_lInstantActions.Find(m_curAction.GetActionID())
            != NULL;
        if (bCanResolveInstants) {
            do {
                ResolveInstants(TRUE);
                bCurrentInstant = g_pBaldurChitin->GetObjectGame()
                                      ->GetRuleTables()
                                      .m_lInstantActions.Find(m_curAction.GetActionID())
                    != NULL;
            } while (bCurrentInstant
                && !(g_pChitin->cNetwork.GetSessionOpen() == TRUE
                    && m_curAction.GetActionID() == CAIAction::LEAVEAREALUA));
        } else {
            while (bCurrentInstant
                && !(g_pChitin->cNetwork.GetSessionOpen() == TRUE
                    && m_curAction.GetActionID() == CAIAction::LEAVEAREALUA)) {
                ResolveInstants(FALSE);
                bCurrentInstant = g_pBaldurChitin->GetObjectGame()
                                      ->GetRuleTables()
                                      .m_lInstantActions.Find(m_curAction.GetActionID())
                    != NULL;
            }
        }
    }

    // Keep the destination ground marker in sync with the current action. The
    // binary's ProcessAI (0x72b9a0) resolves m_targetPoint each tick (via
    // ResolvePausedAction, 0x72dd64); the recovered stub omitted it, so once a
    // member stopped (action advanced to FACE/idle) m_targetPoint kept the old
    // move destination and RenderMarkers (m_targetPoint.x != -1) drew the marker
    // forever. ResolveTargetPoint is the side-effect-free core of that resolve:
    // it sets the marker to the pending move's destination, or clears it (-1)
    // for FACE/idle, so the marker disappears on arrival.
    if (!m_groupMove) {
        ResolveTargetPoint(&m_curAction, m_queuedActions.GetHeadPosition());
    }
}

// 0x71E760
void CGameSprite::sub_71E760(CDerivedStats& DStats, int a2)
{
    // TODO: Incomplete.
}

// 0x71F480
//
// Ambient light level at the sprite (0..50), used as the light penalty in the
// stealth detection check.  Samples the area tint at the sprite's height-
// projected position, takes its NTSC luminance relative to the night-colour
// baseline, then halves it.  When the area applies a day/night global tint (and
// is not flagged to skip it) the sampled colour is first modulated by the video
// mode's global tint -- this is the branch the inlined infravision copy in the
// asynchronous update leaves as a no-op.
char CGameSprite::GetLightLevel()
{
    CPoint pt;
    pt.x = m_pos.x;
    pt.y = m_pos.y + m_posZ + m_pArea->GetHeightOffset(m_pos, m_listType);
    COLORREF rgbTintColor = m_pArea->GetTintColor(pt, m_listType);

    BYTE nBaseline = static_cast<BYTE>((299 * GetRValue(CInfinity::RGB_NIGHT_COLOR)
        + 587 * GetGValue(CInfinity::RGB_NIGHT_COLOR)
        + 114 * GetBValue(CInfinity::RGB_NIGHT_COLOR)) / 2550);

    BYTE nLevel;
    if ((m_pArea->GetInfinity()->m_areaType & 0x2) == 0
        || ((m_pArea->GetInfinity()->m_areaType & 0x40) != 0
            && (m_pArea->GetInfinity()->m_renderDayNightCode & 0x2) != 0)) {
        nLevel = static_cast<BYTE>((299 * GetRValue(rgbTintColor)
            + 587 * GetGValue(rgbTintColor)
            + 114 * GetBValue(rgbTintColor)) / 2550);
    } else {
        COLORREF rgbGlobal = g_pBaldurChitin->GetCurrentVideoMode()->GetGlobalTintColor();
        COLORREF rgbModulated = RGB(
            GetRValue(rgbGlobal) * GetRValue(rgbTintColor) / 255,
            GetGValue(rgbGlobal) * GetGValue(rgbTintColor) / 255,
            GetBValue(rgbGlobal) * GetBValue(rgbTintColor) / 255);
        nLevel = static_cast<BYTE>((299 * GetRValue(rgbModulated)
            + 587 * GetGValue(rgbModulated)
            + 114 * GetBValue(rgbModulated)) / 2550);
    }

    int nLight = (nLevel - nBaseline) * 100 / (100 - nBaseline);
    return static_cast<char>(static_cast<char>(nLight) >> 1);
}

// 0x722090
//
// Map a race id (RACE.IDS) to its upper-case label.  The label keys the racial
// stealth-modifier 2DA (LIGHTMOD / QUIETMOD / HIDEMOD columns) in the stealth
// detection check, and also names race-keyed script variables.  An unrecognised
// id yields an empty string.  (The binary passes `this` but does not use it.)
CString CGameSprite::GetRaceLabel(BYTE nRace)
{
    CString sRace;
    switch (nRace) {
    case 1:    sRace = "HUMAN"; break;
    case 2:    sRace = "ELF"; break;
    case 3:    sRace = "HALF_ELF"; break;
    case 4:    sRace = "DWARF"; break;
    case 5:    sRace = "HALFLING"; break;
    case 6:    sRace = "GNOME"; break;
    case 7:    sRace = "HALF_ORC"; break;
    case 0x65: sRace = "ANKHEG"; break;
    case 0x66: sRace = "BASILISK"; break;
    case 0x67: sRace = "BEAR"; break;
    case 0x68: sRace = "CARRIONCRAWLER"; break;
    case 0x69: sRace = "DOG"; break;
    case 0x6A: sRace = "DOPPLEGANGER"; break;
    case 0x6B: sRace = "ETTERCAP"; break;
    case 0x6C: sRace = "GHOUL"; break;
    case 0x6D: sRace = "GIBBERLING"; break;
    case 0x6E: sRace = "GNOLL"; break;
    case 0x6F: sRace = "HOBGOBLIN"; break;
    case 0x70: sRace = "KOBOLD"; break;
    case 0x71: sRace = "OGRE"; break;
    case 0x73: sRace = "SKELETON"; break;
    case 0x74: sRace = "SPIDER"; break;
    case 0x75: sRace = "WOLF"; break;
    case 0x76: sRace = "WYVERN"; break;
    case 0x77: sRace = "SLIME"; break;
    case 0x78: sRace = "FAIRY"; break;
    case 0x96: sRace = "BEETLE"; break;
    case 0x97: sRace = "CARRION_CRAWLER"; break;
    case 0x98: sRace = "ELEMENTAL"; break;
    case 0x99: sRace = "GIANT"; break;
    case 0x9A: sRace = "DRAKE"; break;
    case 0x9B: sRace = "GOBLIN"; break;
    case 0x9C: sRace = "GOLEM"; break;
    case 0x9D: sRace = "LIZARDMAN"; break;
    case 0x9E: sRace = "MANTICORE"; break;
    case 0x9F: sRace = "MYCONID"; break;
    case 0xA0: sRace = "ORC"; break;
    case 0xA1: sRace = "SALAMANDER"; break;
    case 0xA2: sRace = "SHRIEKER"; break;
    case 0xA3: sRace = "SVIRFNEBLIN"; break;
    case 0xA4: sRace = "TANARI"; break;
    case 0xA5: sRace = "TROLL"; break;
    case 0xA6: sRace = "UMBERHULK"; break;
    case 0xA7: sRace = "UNDEAD"; break;
    case 0xA8: sRace = "YUANTI"; break;
    case 0xAE: sRace = "HARPY"; break;
    case 0xB1: sRace = "BEHOLDER"; break;
    case 0xB3: sRace = "MINOTAUR"; break;
    case 0xB4: sRace = "BUGBEAR"; break;
    case 0xB5: sRace = "HOOKHORROR"; break;
    case 0xB6: sRace = "DRIDER"; break;
    case 0xB7: sRace = "DROW"; break;
    case 0xB8: sRace = "MIND_FLAYER"; break;
    case 0xBF: sRace = "SHAPESHIFTER"; break;
    case 0xC0: sRace = "WILLOWISP"; break;
    case 0xC1: sRace = "OTYUGH"; break;
    case 0xFF: sRace = "NO_RACE"; break;
    }
    return sRace;
}

// 0x72FD20
//
// Per-round modal / passive-ability update, called once per active AI tick from
// Called once per AI tick from CGameSprite::ProcessAI.  Self-staggered: a sprite
// processes on the tick where the shared AI frame counter aligns with its id
// (once per 100 ticks).  For a party member it first runs the passive
// secret-door sweep, then dispatches the active modal ability by m_nModalState:
// 1 = bard song, 2 = detect traps / search, 3 = stealth, 4 = turn undead.
//
// Recovered: the stagger, the passive secret-door sweep, the bard-song cycle
// (re-applies the song's ability effects to nearby allies via a CMessage105
// effect-list message), the detect-traps sweep (door / trigger / container) and
// the stealth re-check (every third cycle -> sub_757B40).
// Still unrecovered: the turn-undead case (case 3) and the singer's own
// song-marker effect (case 0).
// 0x72FD20
void CGameSprite::CheckModal()
{
    // Process this sprite's modal abilities only on the AI tick where the frame
    // counter aligns with its id (a once-per-100-ticks stagger).
    if (static_cast<DWORD>(field_44A) % 100 != static_cast<DWORD>(m_id) % 100) {
        return;
    }
    m_modalCounter++;

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) != -1) {
        // A party member passively searches for nearby secret doors each cycle.
        // Roll the searcher's luck against every still-hidden secret door within
        // half its visual range; reveal the ones whose detection difficulty is
        // beaten, cueing ACT_09 and broadcasting a CMessageDoorStatus.
        CAITrigger searchTrigger(CAITRIGGER_NO_TRIGGER, 0);
        CTypedPtrList<CPtrList, LONG*> objects;
        int searchRate = GetBaseMovementRate();
        int luckRoll = CUtil::UtilRandInt(100, -m_derivedStats.m_nLuck);
        m_pArea->GetAllInRange(m_pos, CAIObjectType::ANYONE, GetVisualRange() >> 1,
            GetVisibleTerrainTable(), objects, FALSE, TRUE);

        POSITION pos = objects.GetHeadPosition();
        while (pos != NULL) {
            LONG objId = reinterpret_cast<LONG>(objects.GetNext(pos));

            CGameObject* pObject;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(objId,
                    CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if (pObject->GetObjectType() == CGameObject::TYPE_DOOR) {
                    CGameDoor* pDoor = static_cast<CGameDoor*>(pObject);
                    if (pDoor->m_dwFlags & 0x80) {  // secret door
                        if ((pDoor->m_dwFlags & 0x100) == 0) {  // not yet found
                            int difficulty = pDoor->m_detectionDifficulty;
                            if (difficulty == 0) {
                                difficulty = 100;
                            }
                            if (difficulty + luckRoll - 101 <= searchRate) {
                                CResRef sound("ACT_09");
                                PlaySound(sound);
                                pDoor->SetDrawPoly(400);
                                pDoor->m_dwFlags |= 0x100;  // mark found
                                CMessage* message = new CMessageDoorStatus(pDoor, m_id, pDoor->m_id);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                            }
                        } else {
                            pDoor->SetDrawPoly(400);
                        }
                    }
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(objId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                } else {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(objId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }
            }
        }
    }

    switch (m_nModalState - 1) {
    case 0: {  // bard song -- re-apply the song to nearby allies this cycle
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        // A silenced singer (derived or base general state) cannot sing; the
        // game flag at +0x43E6 (store/dialog/cutscene gate) also suppresses it.
        if ((m_derivedStats.m_generalState & STATE_SILENCED) == 0
            && (m_baseStats.m_generalState & STATE_SILENCED) == 0
            && *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pGame) + 0x43E6) != 1) {
            if (!CheckInvisibility(FALSE)) {
                // Singing is a visible action: a non-invisible singer re-asserts
                // visibility this cycle by delivering a FORCEVISIBLE effect (0x88)
                // to itself, mirroring the spell-cast path.
                ITEM_EFFECT effect;
                CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);
                effect.durationType = 1;
                CGameEffect* visibleEffect = CGameEffect::DecodeEffect(
                    &effect,
                    m_pos,
                    m_id,
                    CPoint(-1, -1));
                CMessage* message = new CMessageAddEffect(visibleEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            UTIL_ASSERT(m_nLastSong < pGame->m_songs.m_nCount);
            // (The binary also asserts the resolved song ResRef is non-empty.)
            CResRef songRes = pGame->m_songs.Get636(m_nLastSong);

            CSpell spell(songRes);
            spell.Demand();
            if (spell.GetCasterType() == 5) {
                FeedBack(FEEDBACK_BATTLESONGSTART, 0, 0, 0, spell.GetGenericName(), 0, 0);
                SPELL_ABILITY* pAbility = spell.GetAbility(0);

                // Gather everyone within 400 of the singer, plus the singer.
                CTypedPtrList<CPtrList, LONG*> targets;
                m_pArea->GetCloseObjects(GetVertListPos(), GetPos(), CAIObjectType::ANYONE,
                    400, GetTerrainTable(), targets, TRUE, FALSE);
                m_pArea->GetAllInRangeBack(GetPos(), CAIObjectType::ANYONE, 400,
                    GetTerrainTable(), targets, TRUE, FALSE, FALSE);
                targets.AddTail(reinterpret_cast<LONG*>(m_id));

                // SPELL_ABILITY effect count.
                SHORT nEffects = *reinterpret_cast<SHORT*>(reinterpret_cast<BYTE*>(pAbility) + 0x1E);

                POSITION pos = targets.GetHeadPosition();
                while (pos != NULL) {
                    LONG allyId = reinterpret_cast<LONG>(targets.GetNext(pos));

                    CGameObject* pAlly;
                    BYTE rc = pGame->GetObjectArray()->GetShare(allyId,
                        CGameObjectArray::THREAD_ASYNCH, &pAlly, INFINITE);
                    if (rc != CGameObjectArray::SUCCESS) {
                        continue;
                    }

                    if (pAlly->GetObjectType() == CGameObject::TYPE_SPRITE) {
                        // Build one CMessageAddEffect per song effect, addressed
                        // singer -> ally, into a local list, then deliver the
                        // whole list to the ally through a CMessage105.
                        CMessage105::Node localHead;
                        localHead.m_pNext = &localHead;
                        localHead.m_pPrev = &localHead;

                        for (SHORT i = 0; i < nEffects; i++) {
                            CGameEffect* pEff = spell.BuildAbilityEffect(0, i, this, 0, 0, 0);
                            // Source the effect at the singer's position and id.
                            *reinterpret_cast<LONG*>(reinterpret_cast<BYTE*>(pEff) + 0x7C) = m_pos.x;
                            *reinterpret_cast<LONG*>(reinterpret_cast<BYTE*>(pEff) + 0x80) = m_pos.y;
                            pEff->m_sourceID = m_id;
                            IcewindMisc::ApplyDamageModifiers(this, pEff);
                            // Re-tag a "Duration"-timed song effect so it expires a
                            // round later (game time + 100), keeping the per-cycle
                            // buff alive instead of letting ResolveEffect treat it
                            // as already past.  The binary writes the raw effect
                            // bytes at +0x20/+0x24, but those map to m_duration /
                            // m_probabilityUpper in our field layout; write through
                            // the typed members so ResolveEffect / CheckExpiration
                            // (which read m_durationType / m_duration) see a future
                            // expiry.
                            if (*reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(pEff) + 0x20) == 0
                                && *reinterpret_cast<LONG*>(reinterpret_cast<BYTE*>(pEff) + 0x24) == 7) {
                                pEff->m_durationType = 0x1000;
                                pEff->m_duration =
                                    g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + 100;
                            }

                            CMessageAddEffect* pSub = new CMessageAddEffect(pEff, m_id, allyId);
                            CMessage105::Node* pNode = new CMessage105::Node;
                            pNode->m_pMessage = pSub;
                            pNode->m_pNext = &localHead;
                            pNode->m_pPrev = localHead.m_pPrev;
                            localHead.m_pPrev->m_pNext = pNode;
                            localHead.m_pPrev = pNode;
                        }

                        CMessage* pMsg = new CMessage105(&localHead, m_id, allyId);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMsg, FALSE);

                        // Free the local list nodes; the sub-messages are now
                        // owned (and freed) by the CMessage105.
                        CMessage105::Node* p = localHead.m_pNext;
                        while (p != &localHead) {
                            CMessage105::Node* pNext = p->m_pNext;
                            delete p;
                            p = pNext;
                        }
                    }

                    pGame->GetObjectArray()->ReleaseShare(allyId,
                        CGameObjectArray::THREAD_ASYNCH, INFINITE);
                }
            }
            spell.Release();
        }
        break;
    }
    case 1: {  // Detect Traps -- the searcher actively sweeps everything within
               // half its visual range for armed-but-undetected traps on doors,
               // triggers and containers, revealing each one whose Search skill
               // beats the trap's detection difficulty.
        CAITrigger detectTrigger(CAITRIGGER_NO_TRIGGER, 0);
        CTypedPtrList<CPtrList, LONG*> objects;
        m_pArea->GetAllInRange(m_pos, CAIObjectType::ANYONE, GetVisualRange() >> 1,
            GetVisibleTerrainTable(), objects, TRUE, TRUE);
        m_pArea->GetAllInRangeBack(m_pos, CAIObjectType::ANYONE, GetVisualRange() >> 1,
            GetVisibleTerrainTable(), objects, TRUE, TRUE, TRUE);

        // Search skill rank, read signed (penalties can drive it negative).
        int searchSkill = static_cast<char>(m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_SEARCH]);

        POSITION pos = objects.GetHeadPosition();
        while (pos != NULL) {
            LONG objId = reinterpret_cast<LONG>(objects.GetNext(pos));

            CGameObject* pObject;
            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(objId,
                    CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                BYTE type = pObject->GetObjectType();

                if (type == CGameObject::TYPE_DOOR) {
                    CGameDoor* pDoor = static_cast<CGameDoor*>(pObject);
                    BOOL bDetected = FALSE;
                    if (pDoor->m_trapActivated != 0) {
                        if (pDoor->m_trapDetected == 0) {
                            int difficulty = pDoor->m_trapDetectionDifficulty;
                            // "Search (detect traps) check. Search skill %d vs.
                            // trap's difficulty %d (searcher's %d INT bonus)."
                            FeedBack(FEEDBACK_ROLL, searchSkill, difficulty / 5,
                                GetSkillModifier(CGAMESPRITE_SKILL_SEARCH), 39303, 0, 0);
                            if (difficulty <= searchSkill * 7 && difficulty != 100
                                && (pDoor->m_dwFlags & 0x8)) {  // TRAPDETECTABLE
                                CAITrigger trapFound(CAITRIGGER_DETECTED, m_typeAI, 0);
                                CMessage* message = new CMessageSetTrigger(trapFound,
                                    pDoor->m_id, m_id);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                                pDoor->m_trapDetected = 1;
                                bDetected = TRUE;
                                AutoPause(0x80);
                            }
                        }
                        if (pDoor->m_trapDetected != 0) {
                            pDoor->SetDrawPoly(400);
                        }
                        if (bDetected) {
                            CMessage* message = new CMessageDoorStatus(pDoor, m_id, pDoor->m_id);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                    }
                } else if (type == CGameObject::TYPE_TRIGGER) {
                    CGameTrigger* pTrigger = static_cast<CGameTrigger*>(pObject);
                    BOOL bDetected = FALSE;
                    if (pTrigger->m_trapActivated != 0 && (pTrigger->m_dwFlags & 0x100) == 0) {
                        if (pTrigger->m_trapDetected == 0) {
                            int difficulty = pTrigger->m_trapDetectionDifficulty;
                            if (difficulty <= searchSkill * 7 && difficulty != 100
                                && (pTrigger->m_dwFlags & 0x8)) {  // TRAPDETECTABLE
                                CAITrigger trapFound(CAITRIGGER_DETECTED, m_typeAI, 0);
                                CMessage* message = new CMessageSetTrigger(trapFound,
                                    pTrigger->m_id, m_id);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                                pTrigger->m_trapDetected = 1;
                                bDetected = TRUE;
                                AutoPause(0x80);
                            }
                        }
                        if (pTrigger->m_trapDetected != 0) {
                            pTrigger->SetDrawPoly(400);
                        }
                        if (bDetected) {
                            CMessage* message = new CMessageTriggerStatus(pTrigger->m_dwFlags,
                                pTrigger->m_trapActivated, pTrigger->m_trapDetected,
                                pTrigger->m_id, m_id);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                    }
                } else if (type == CGameObject::TYPE_CONTAINER) {
                    CGameContainer* pContainer = static_cast<CGameContainer*>(pObject);
                    BOOL bDetected = FALSE;
                    if (pContainer->m_trapActivated != 0) {
                        if (pContainer->m_trapDetected == 0) {
                            int difficulty = pContainer->m_trapDetectionDifficulty;
                            // Containers carry no TRAPDETECTABLE flag gate.
                            if (difficulty <= searchSkill * 7 && difficulty != 100) {
                                CAITrigger trapFound(CAITRIGGER_DETECTED, m_typeAI, 0);
                                CMessage* message = new CMessageSetTrigger(trapFound,
                                    pContainer->m_id, m_id);
                                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                                pContainer->SetTrapDetected(1);
                                pContainer->SetDrawPoly(400);
                                bDetected = TRUE;
                                AutoPause(0x80);
                            }
                        }
                        if (pContainer->m_trapDetected != 0) {
                            pContainer->SetDrawPoly(400);
                        }
                        if (bDetected) {
                            CMessage* message = new CMessageContainerStatus(pContainer->m_dwFlags,
                                pContainer->m_trapActivated, pContainer->m_trapDetected,
                                pContainer->m_id, pContainer->m_id);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(objId,
                    CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }
        break;
    }
    case 2:
        // Hide in Shadows is re-checked once every three modal cycles.
        if (m_modalCounter % 3 == 0) {
            sub_757B40();
        }
        break;
    case 3:
        // Unrecovered: per-round turn-undead sweep.
        break;
    default:
        break;
    }
}

// 0x757B40
SHORT CGameSprite::sub_757B40()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    // D20 stealth roll scaled to 5..100 (a 1..20 roll times five, plus five).
    int nRoll = (rand() % 20) * 5 + 5;
    // The Stealth slot of the disabled-buttons array (m_disabledButtons[0], at
    // +0x16CC) -- a DisableButton effect sets it to force the creature visible.
    BOOL nForceReveal = GetDerivedStats()->m_disabledButtons[0];

    // A natural 20 (a roll of 100), or a pending forced-reveal request, breaks
    // stealth outright -- no per-creature comparison is made.
    if (nRoll == 100 || nForceReveal != 0) {
        m_bHiding = FALSE;
        FeedBack(0xD, 0, 0, 0, -1, 0, 0);
        m_nStealthGreyOut = 90;
        *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pGame) + 0x35F6) = 100;
        SetModalState(0, TRUE);
        // If this sprite owns the selected portrait and the mouse is not in a
        // special targeting cursor mode, refresh the action toolbar.
        if (g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter()
                == pGame->GetCharacterPortraitNum(m_id)) {
            int nCursorState = *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pGame) + 0x35FA);
            if (nCursorState != 0x66 && nCursorState != 0x65 && nCursorState != 0x68
                && nCursorState != 0x67 && nCursorState != 0x69 && nCursorState != 0x70
                && nCursorState != 0x73 && nCursorState != 0x74 && nCursorState != 0x75
                && nCursorState != 0x76 && nCursorState != 0x77 && nCursorState != 0x78
                && nCursorState != 0x79 && nCursorState != 0x7A && nCursorState != 0x7B
                && nCursorState != 0x6A) {
                pGame->GetButtonArray()->ResetState();
            }
        }
        return ACTION_DONE;
    }

    const CRuleTables& ruleTables = pGame->GetRuleTables();

    // The nearest creature that counts us as an enemy is the potential witness.
    LONG nEnemyId = m_pArea->GetNearest(m_id, GetAIType().GetEnemyOf(), GetVisualRange(),
        GetVisibleTerrainTable(), TRUE, TRUE, TRUE, 0, FALSE);

    // Stealth value = mean of Hide and Move Silently, scaled down by ambient
    // light: a bright tile erodes the skill, full dark leaves it untouched.
    int nMoveSilently = static_cast<char>(m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_MOVE_SILENTLY]);
    int nHide = static_cast<char>(m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_HIDE]);
    int nStealth = (nHide + nMoveSilently) / 2;
    INT nHideSkillMod = GetSkillModifier(CGAMESPRITE_SKILL_HIDE);
    {
        CString sRace = GetRaceLabel(GetAIType().m_nRace);
        int nLightMod = atol(ruleTables.m_tCRELIGHT.GetAt(CString("LIGHTMOD"), sRace));
        char nLight = GetLightLevel();
        nStealth = ((nStealth - (nLight * nLightMod * nStealth) / 10000) + 10) * 5;
    }

    BOOL bDetected = FALSE;
    if (nEnemyId == CGameObjectArray::INVALID_INDEX) {
        // No witness in range: contest the light-adjusted stealth against the roll.
        if (nStealth <= nRoll) {
            FeedBack(0x53, nStealth / 5, nRoll / 5, nHideSkillMod, 0x9984, 0, 0);
            bDetected = TRUE;
        } else {
            FeedBack(0x53, nStealth / 5, nRoll / 5, nHideSkillMod, 0x9983, 0, 0);
        }
    } else {
        CGameObject* pObject;
        BYTE rc;
        do {
            do {
                rc = pGame->GetObjectArray()->GetShare(nEnemyId, CGameObjectArray::THREAD_ASYNCH,
                    &pObject, INFINITE);
            } while (rc == CGameObjectArray::SHARED);
        } while (rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            CGameSprite* pEnemy = static_cast<CGameSprite*>(pObject);
            BYTE nEnemyLevel = pEnemy->m_derivedStats.m_nLevel;
            INT nWisMod = ruleTables.GetAbilityScoreModifier(pEnemy->m_derivedStats.m_nWIS);
            CString sEnemyRace = GetRaceLabel(pEnemy->GetAIType().m_nRace);

            // A deafened enemy cannot hear us, so the Move Silently contest is skipped.
            if (!pEnemy->m_derivedStats.m_spellStates[SPLSTATE_DEAFENED]) {
                LONG nHearing = (atol(ruleTables.m_tCREHIDEM.GetAt(CString("QUIETMOD"), sEnemyRace))
                    + nWisMod + nEnemyLevel) * 5;
                if (nHearing < 0) {
                    nHearing = 0;
                }
                LONG nResult;
                if (nHearing + nRoll < nMoveSilently) {
                    bDetected = TRUE;
                    nResult = 0x9981;
                } else {
                    nResult = 0x70;
                }
                FeedBack(0x53, nMoveSilently / 5, nHearing / 5, nRoll / 5, nResult, 0, 0);
            }

            // A blinded enemy cannot see us, so the Hide in Shadows contest is skipped.
            if ((pEnemy->m_derivedStats.m_generalState & STATE_BLIND) == 0) {
                BYTE nLevel = pEnemy->m_derivedStats.m_nLevel;
                nWisMod = ruleTables.GetAbilityScoreModifier(pEnemy->m_derivedStats.m_nWIS);
                LONG nSight = (atol(ruleTables.m_tCREHIDEM.GetAt(CString("HIDEMOD"), sEnemyRace))
                    + nWisMod + nLevel) * 5;
                if (nSight < 0) {
                    nSight = 0;
                }
                pGame->GetObjectArray()->ReleaseShare(nEnemyId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
                LONG nResult;
                if (nSight + nRoll < nHide) {
                    nResult = 0x6EDB;
                } else {
                    nResult = 0x9982;
                    bDetected = TRUE;
                }
                FeedBack(0x53, nStealth / 5, nSight / 5, nRoll / 5, nResult, 0, 0);
            }

            // A creature that can neither hear nor see contests a plain stealth roll.
            if (pEnemy->m_derivedStats.m_spellStates[SPLSTATE_DEAFENED]
                && (pEnemy->m_derivedStats.m_generalState & STATE_BLIND)) {
                LONG nResult;
                if (nRoll < nStealth) {
                    nResult = 0x9983;
                } else {
                    nResult = 0x9984;
                    bDetected = TRUE;
                }
                FeedBack(0x53, nStealth / 5, nRoll / 5, nHideSkillMod, nResult, 0, 0);
            }

            pGame->GetObjectArray()->ReleaseShare(nEnemyId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    }

    if (bDetected) {
        // Stealth broken: clear the hiding flag and modal state, grey the stealth
        // button, and (when this sprite owns the toolbar) refresh it.
        m_bHiding = FALSE;
        m_nModalState = 0;
        FeedBack(0xD, 0, 0, 0, -1, 0, 0);
        m_nStealthGreyOut = 90;
        *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pGame) + 0x35F6) = 100;
        SetModalState(0, TRUE);
        if (g_pBaldurChitin->GetActiveEngine()->GetSelectedCharacter()
                == pGame->GetCharacterPortraitNum(m_id)) {
            int nCursorState = *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(pGame) + 0x35FA);
            if (nCursorState != 0x66 && nCursorState != 0x65 && nCursorState != 0x68
                && nCursorState != 0x67 && nCursorState != 0x69 && nCursorState != 0x70
                && nCursorState != 0x73 && nCursorState != 0x74 && nCursorState != 0x75
                && nCursorState != 0x76 && nCursorState != 0x77 && nCursorState != 0x78
                && nCursorState != 0x79 && nCursorState != 0x7A && nCursorState != 0x7B
                && nCursorState != 0x6A) {
                pGame->GetButtonArray()->ResetState();
            }
        }
    } else {
        // Still hidden: play the stealth cue on the cycle we first hide, then
        // renew a three-round self-invisibility so the sprite stays unseen.
        if (!m_bHiding) {
            CResRef sound("ACT_07");
            PlaySound(sound);
        }
        FeedBack(0x11, 0, 0, 0, -1, 0, 0);

        ITEM_EFFECT* pEffect = new ITEM_EFFECT;
        CGameEffect::ClearItemEffect(pEffect, CGAMEEFFECT_INVISIBLE);
        pEffect->dwFlags = 0;
        pEffect->effectAmount = 1;
        pEffect->durationType = 0x1000;
        pEffect->duration = pGame->GetWorldTimer()->m_gameTime + 3 * 100;
        CGameEffect* pDecoded = CGameEffect::DecodeEffect(pEffect, m_pos, m_id, CPoint(-1, -1));
        CMessage* pMessage = new CMessageAddEffect(pDecoded, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        delete pEffect;

        m_bHiding = TRUE;
        m_nModalState = 3;
    }

    return ACTION_DONE;
}

// 0x71F6E0
int CGameSprite::GetBaseMovementRate()
{
    int v1;

    switch (m_typeAI.m_nClass) {
    case CAIOBJECTTYPE_C_BARBARIAN:
    case CAIOBJECTTYPE_C_CLERIC:
    case CAIOBJECTTYPE_C_DRUID:
    case CAIOBJECTTYPE_C_FIGHTER:
    case CAIOBJECTTYPE_C_PALADIN:
    case CAIOBJECTTYPE_C_RANGER:
        v1 = 10;
        break;
    case CAIOBJECTTYPE_C_BARD:
    case CAIOBJECTTYPE_C_MONK:
    case CAIOBJECTTYPE_C_ROGUE:
        v1 = 15;
        break;
    default:
        v1 = 5;
        break;
    }

    switch (m_typeAI.m_nRace) {
    case CAIOBJECTTYPE_R_ELF:
        v1 += 20;
        break;
    case CAIOBJECTTYPE_R_DWARF:
        v1 += 10;
        break;
    case CAIOBJECTTYPE_R_HALFLING:
        v1 += 5;
        break;
    }

    if (m_nModalState == 2) {
        v1 = 100;
    }

    return v1;
}

// 0x71F760
const CString& CGameSprite::GetName()
{
    if (m_baseStats.m_name != -1) {
        STR_RES strRes;
        g_pBaldurChitin->GetTlkTable().Fetch(m_baseStats.m_name, strRes);
        m_sName = strRes.szText;
    }

    return m_sName;
}

// 0x71F820
STRREF CGameSprite::GetNameRef()
{
    STRREF strName = m_baseStats.m_name;

    if (strName == -1) {
        SHORT nSlot = g_pBaldurChitin->GetObjectGame()->GetCharacterSlotFromId(m_id);
        if (nSlot < 0) {
            nSlot = 0;
        }

        strName = -2 - nSlot;
    }

    return strName;
}

// 0x727B80
//
// Visibility filter used by every targeting / "find-nearest" path. Returns
// FALSE if this sprite is hidden from an observer described by
// `bSeesInvisible`, TRUE otherwise.
//
// The sprite is hidden iff the observer cannot see invisible
// (`bSeesInvisible == FALSE`), the sprite carries the regular invisibility
// state (`m_generalState & STATE_INVISIBLE`), and the sprite has not been
// re-exposed (`m_baseStats` byte at `+0x2FC` bit 0 clear, i.e. not currently
// rendered visible to the local player).
BOOL CGameSprite::CheckInvisibility(BOOL bSeesInvisible)
{
    if (bSeesInvisible == FALSE
        && (m_derivedStats.m_generalState & STATE_INVISIBLE) != 0
        && (*reinterpret_cast<BYTE*>(reinterpret_cast<BYTE*>(&m_baseStats) + 0x2FC) & 0x1) == 0) {
        return FALSE;
    }
    return TRUE;
}

// Followup half of `CGameSprite::ApplyCastingEffect`.  Called when the cast
// enters the burst visual stage (DAT_0085BBB2 marker) and on a couple of
// projectile-completion paths.  Builds the casting-feedback sound resref
// ("CAS_M0N" for arcane casters, "CAS_P0N" for divine, where N selects the
// caster's animation variant), parks it in `m_sndMagic` so the sprite can
// replay it without re-loading, plays the 3D-positioned local sound, and
// queues a CMessagePlaySoundRef on channel 4 so remote clients hear the
// same cue.
namespace {

const char* GetCastingSoundSuffix(WORD animationType, BOOL useDefault)
{
    switch (animationType) {
    case 9: return "07";
    case 10: return "08";
    case 11: return "05";
    case 12: return "02";
    case 13: return "01";
    case 14: return "03";
    case 15: return "06";
    case 16: return "04";
    default: return useDefault ? "01" : NULL;
    }
}

BOOL GetCastingChantResRef(WORD animationType, char gender, char casterType, CString& resName)
{
    if (gender == 'N') {
        return FALSE;
    }

    CString base("CHA_");
    base += gender;
    base += casterType;

    // The original advances RNG for these chant variants even though the
    // recovered branch uses fixed suffixes.
    switch (animationType) {
    case 9:
        rand();
        resName.Format("%s1%c", (LPCSTR)base, '3');
        break;
    case 10:
        rand();
        resName.Format("%s1%c", (LPCSTR)base, '5');
        break;
    case 11:
        rand();
        resName = base;
        resName += "09";
        break;
    case 12:
        rand();
        resName.Format("%s0%c", (LPCSTR)base, '3');
        break;
    case 13:
        rand();
        resName.Format("%s0%c", (LPCSTR)base, '1');
        break;
    case 14:
        rand();
        resName.Format("%s0%c", (LPCSTR)base, '5');
        break;
    case 15:
        rand();
        resName.Format("%s1%c", (LPCSTR)base, '1');
        break;
    case 16:
        rand();
        resName.Format("%s0%c", (LPCSTR)base, '7');
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

void PlayCastingSound(CSound& sound, const CResRef& resRef, BYTE channel, CGameArea* pArea, const CPoint& pos, LONG posZ, LONG id)
{
    sound.Stop();
    sound.SetChannel(channel, reinterpret_cast<DWORD>(pArea));
    sound.SetResRef(resRef, TRUE, TRUE);
    sound.Play(pos.x, pos.y, posZ, FALSE);

    CMessagePlaySoundRef* msg = new CMessagePlaySoundRef(resRef, id, id);
    msg->m_nChannel = channel;
    g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
}

BOOL GetCastingVisualEffect(WORD animationType, WORD& effectID, LONG& effectAmount, DWORD& dwFlags)
{
    effectAmount = 0;

    switch (animationType) {
    case 9:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 7;
        break;
    case 10:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 8;
        break;
    case 11:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 4;
        break;
    case 12:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 1;
        break;
    case 13:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 5;
        break;
    case 14:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 2;
        break;
    case 15:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 6;
        break;
    case 16:
        effectID = ICEWIND_CGAMEEFFECT_CASTINGGLOW;
        dwFlags = 3;
        break;
    case 17:
    case 26:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 2;
        dwFlags = 3;
        break;
    case 18:
    case 27:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 1;
        dwFlags = 3;
        break;
    case 19:
    case 28:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 2;
        dwFlags = 3;
        break;
    case 20:
    case 29:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 4;
        dwFlags = 3;
        break;
    case 21:
    case 30:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 5;
        dwFlags = 3;
        break;
    case 22:
    case 31:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 11;
        dwFlags = 3;
        break;
    case 23:
    case 32:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 6;
        dwFlags = 3;
        break;
    case 24:
    case 33:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 7;
        dwFlags = 3;
        break;
    case 25:
    case 34:
        effectID = CGAMEEFFECT_SPARKLE;
        effectAmount = 8;
        dwFlags = 3;
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

}

// 0x7567F0
//
// Range gate for the targeted casts (Spell / SpellPoint).  Demands the SPL,
// picks the highest ability whose minCasterLevel the caster meets, then
// tests the target point against that ability's range in path-grid squares
// (+2 squares of slack).  range == 0xFFFF is unbounded.  No ability
// qualifying (or none at all) fails the gate.  Line of sight is the
// caller's problem.
BOOL CGameSprite::CheckCastingRange(CSpell* pSpell, const CPoint& targetPos, BYTE nClass, DWORD nSpecialization)
{
    pSpell->Demand();

    SHORT casterLevel = GetCasterLevel(pSpell, nClass, nSpecialization);

    Spell_ability_st* ability = NULL;
    if (pSpell->GetAbilityCount() > 0) {
        INT index = 0;
        do {
            if (casterLevel < pSpell->GetAbility(index)->minCasterLevel) {
                break;
            }
            ability = pSpell->GetAbility(index);
            ++index;
        } while (index < pSpell->GetAbilityCount());

        if (ability != NULL) {
            if (ability->range == 0xFFFF) {
                pSpell->Release();
                return TRUE;
            }

            const CPoint& pos = GetPos();
            LONG dx = pos.x / CPathSearch::GRID_SQUARE_SIZEX
                - targetPos.x / CPathSearch::GRID_SQUARE_SIZEX;
            LONG dy = pos.y / CPathSearch::GRID_SQUARE_SIZEY
                - targetPos.y / CPathSearch::GRID_SQUARE_SIZEY;
            LONG range = ability->range + 2;
            if (dx * dx + dy * dy <= range * range) {
                pSpell->Release();
                return TRUE;
            }
        }
    }

    pSpell->Release();
    return FALSE;
}

// 0x740270
SHORT CGameSprite::Spell(CGameAIBase* target)
{
    SHORT initialSequence = m_nSequence;
    SHORT actionReturn = ACTION_DONE;

    if (m_typeAI.GetGeneral() == CAIObjectType::G_DEAD) {
        return actionReturn;
    }

    if (m_actionCount == 0) {
        if (m_curProjectile != NULL) {
            delete m_curProjectile;
            m_curProjectile = NULL;
        }

        if (m_derivedStats.m_bAuraCleansing && m_castCounter != -1) {
            FeedBack(FEEDBACK_AURA_CLEANSED, 0, 0, 0, -1, 0, 0);
            m_castCounter = -1;
        }
    }

    actionReturn = ACTION_ERROR;
    if (target == NULL) {
        return actionReturn;
    }

    if (target->GetObjectType() == CGameObject::TYPE_SPRITE && target != this) {
        CGameSprite* targetSprite = static_cast<CGameSprite*>(target);
        if (!targetSprite->CheckInvisibility(GetCanSeeInvisible())
            || targetSprite->m_derivedStats.m_spellStates[SPLSTATE_SANCTUARY]) {
            AutoPause(0x20);

            CAITrigger targetUnreachable(
                CAITRIGGER_TARGETUNREACHABLE,
                m_curAction.m_acteeID,
                0);
            CMessage* message = new CMessageSetTrigger(targetUnreachable, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                target->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return ACTION_ERROR;
        }
    }

    CPoint targetPos = target->GetPos();
    if (target != this && m_nDirection != GetDirection(targetPos)) {
        CMessage* message = new CMessageSetDirection(targetPos, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        return ACTION_NORMAL;
    }

    CString resName = m_curAction.GetString1();
    if (resName.IsEmpty()) {
        SpellIdToResRef(m_curAction.m_specificID, resName);
    }
    if (resName.IsEmpty()) {
        return ACTION_ERROR;
    }

    CResRef spellResRef(resName);
    CInfGame* game = g_pBaldurChitin->GetObjectGame();

    if (!m_derivedStats.m_spellStates[SPLSTATE_VOCALIZE]
        && (m_derivedStats.m_generalState & STATE_SILENCED) != 0
        && !HasFeat(CGAMESPRITE_FEAT_SUBVOCAL_CASTING)) {
        BYTE spellType = game->GetSpellType(spellResRef);
        if ((spellType == 1 || spellType == 3) && spellResRef != "SPWI219") {
            return ACTION_ERROR;
        }
    }

    BYTE nClass = static_cast<BYTE>(m_curAction.m_specificID2);
    BYTE nSpecializationIndex = static_cast<BYTE>(m_curAction.m_specificID2 >> 8);
    DWORD nSpecialization = game->GetRuleTables().GetSpecializationMask(
        nClass,
        nSpecializationIndex);
    BYTE nSpellLevel = static_cast<BYTE>(m_curAction.m_specificID3);

    if (!m_bStartedCasting) {
        CSpell* targetCheckSpell = m_curSpell;
        BOOL deleteTargetCheckSpell = FALSE;
        if (targetCheckSpell == NULL) {
            targetCheckSpell = new CSpell(spellResRef);
            deleteTargetCheckSpell = TRUE;
        }
        if (targetCheckSpell == NULL) {
            return ACTION_ERROR;
        }

        if (!CheckAppropriateTarget(targetCheckSpell, target)) {
            if (deleteTargetCheckSpell) {
                delete targetCheckSpell;
            }
            return ACTION_ERROR;
        }

        STRREF genericName = targetCheckSpell->GetGenericName();
        if (genericName != 0x64A5 && genericName != 0x7E89 && genericName != 0x2F55) {
            // 0x740891: itemFlags are read before the range gate; flag 0x800
            // (the asm tests the NOTed bit) waives the line-of-sight half.
            DWORD itemFlags = targetCheckSpell->GetItemFlags();
            BOOL canCast = CheckCastingRange(targetCheckSpell, targetPos, nClass, nSpecialization)
                && (m_pArea->CheckLOS(m_pos, targetPos, GetVisibleTerrainTable(), FALSE)
                    || (itemFlags & 0x800) != 0);

            if (!canCast) {
                if (deleteTargetCheckSpell) {
                    delete targetCheckSpell;
                }
                SHORT moveResult = MoveToObject(target);
                if (moveResult != ACTION_DONE) {
                    return moveResult;
                }
            } else {
                CMessage* message = new CMessageDropPath(m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        if (deleteTargetCheckSpell) {
            delete targetCheckSpell;
        }

        if (m_castCounter == -1) {
            m_castCounter = 0;
            m_bInCasting = TRUE;
        }
    }

    if (m_castCounter == 0) {
        ITEM_EFFECT effect;
        if (!CheckInvisibility(FALSE)) {
            CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);
            effect.durationType = 1;
            CGameEffect* visibleEffect = CGameEffect::DecodeEffect(
                &effect,
                m_pos,
                m_id,
                CPoint(-1, -1));
            CMessage* message = new CMessageAddEffect(visibleEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (m_derivedStats.m_spellStates[SPLSTATE_SANCTUARY]) {
            CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_DISPELSANCTUARY);
            effect.durationType = 1;
            CGameEffect* sanctuaryEffect = CGameEffect::DecodeEffect(
                &effect,
                m_pos,
                m_id,
                CPoint(-1, -1));
            CMessage* message = new CMessageAddEffect(sanctuaryEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (m_curSpell != NULL) {
            delete m_curSpell;
            m_curSpell = NULL;
        }

        if (resName.Left(2) == "**") {
            return ACTION_ERROR;
        }

        m_curSpell = new CSpell(spellResRef);
        if (m_curSpell == NULL) {
            return ACTION_ERROR;
        }

        m_cGameStats.RecordSpellUse(spellResRef);

        SHORT casterType = m_curSpell->GetCasterType();
        INT disabledType = -1;
        if (casterType == 1) {
            disabledType = 0;
        } else if (casterType == 2) {
            disabledType = 1;
        } else if (casterType == 4) {
            disabledType = 2;
        } else {
            UTIL_ASSERT_MSG(FALSE, "Invalid spell type");
        }

        if (m_curAction.m_actionID != CAIAction::SPELLNODEC) {
            BYTE spellType = game->GetSpellType(spellResRef);
            UINT spellID = 0;
            UINT spellIndex = 0;
            UINT levelIndex = static_cast<UINT>(nSpellLevel) - 1;

            if (spellType == 1) {
                if (!game->m_spells.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                if (nSpecializationIndex == 0) {
                    if (nClass == CAIOBJECTTYPE_C_BARD
                        || nClass == CAIOBJECTTYPE_C_SORCERER) {
                        UINT classIndex = game->GetSpellcasterIndex(nClass);
                        m_spells.Get(classIndex)->SubtractFromSharedCurrentCountAtLevel(
                            levelIndex,
                            1,
                            FALSE);

                        // Spontaneous casters burn shared slots: every known
                        // spell of this level loses one displayed use on the
                        // quick bar.
                        CAbilityId buttonAbility;
                        buttonAbility.m_itemType = 1;
                        buttonAbility.m_nClass = nClass;
                        buttonAbility.m_bCanUse = nSpellLevel;

                        CGameSpriteSpellList* pLevelList =
                            m_spells.GetSpellsAtLevel(classIndex, levelIndex);
                        for (UINT spellIndex = 0;
                             spellIndex < pLevelList->m_List.size();
                             ++spellIndex) {
                            UINT knownSpellID = pLevelList->Get(spellIndex)->m_nID;
                            buttonAbility.m_res = game->m_spells.Get(knownSpellID);
                            UpdateQuickButtons(buttonAbility, -1, FALSE, FALSE);
                        }
                    } else {
                        SubtractFromSpellCount(nClass, levelIndex, spellResRef, 1, FALSE);
                    }
                } else {
                    SubtractFromDomainSpellCount(levelIndex, spellResRef, 1, FALSE);
                }
            } else if (spellType == 2) {
                if (!game->m_innateSpells.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                if (m_innateSpells.Find(spellID, spellIndex)
                    && spellResRef != SPIN275
                    && spellResRef != SPIN276
                    && spellResRef != SPIN277
                    && spellResRef != SPIN278
                    && spellResRef != SPIN279) {
                    SubtractFromInnateSpellCount(spellResRef, 1, FALSE);
                }
            } else if (spellType == 3) {
                if (!game->m_songs.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                m_songs.Find(spellID, spellIndex);
            } else if (spellType == 4) {
                if (!game->m_shapeshifts.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                if (m_shapeshifts.Find(spellID, spellIndex)) {
                    if (spellResRef == SPIN122) {
                        m_shapeshifts.SubtractFromCurrentCount(spellID, 1, FALSE);
                        m_shapeshifts.Remove(spellID, TRUE, 0, 0);
                    } else {
                        m_shapeshifts.SubtractFromSharedCurrentCount(1, FALSE);
                    }
                }
            }
        }

        if (disabledType >= 0 && m_derivedStats.m_disabledSpellTypes[disabledType]) {
            delete m_curSpell;
            m_curSpell = NULL;
            return ACTION_ERROR;
        }
    }

    if (m_curSpell == NULL) {
        return ACTION_NORMAL;
    }
    if (!m_curSpell->Demand()) {
        return ACTION_ERROR;
    }

    SHORT casterLevel = GetCasterLevel(m_curSpell, nClass, nSpecialization);
    LONG abilityIndex = -1;
    Spell_ability_st* ability = NULL;
    INT abilityCount = m_curSpell->GetAbilityCount();
    for (INT index = 0; index < abilityCount; ++index) {
        Spell_ability_st* candidate = m_curSpell->GetAbility(index);
        if (candidate == NULL
            || candidate->minCasterLevel > static_cast<WORD>(casterLevel)) {
            break;
        }
        ability = candidate;
        abilityIndex = index;
    }

    if (ability == NULL) {
        m_curSpell->Release();
        return ACTION_ERROR;
    }

    SHORT casterType = m_curSpell->GetCasterType();
    if (!m_bStartedCasting) {
        ApplyCastingEffect(m_curSpell, ability, targetPos);
        m_bStartedCasting = TRUE;

        // Spell-capture marker (no-op unless .\iwd2-re-debug.enabled exists): the
        // spell_capture rig tails this line to auto-clip the cast. Fires once per
        // cast at the visual cast-start (ApplyCastingEffect), keyed by the spell
        // resref so clips are named per spell. Not present in IWD2.exe (debug-only);
        // the original is traced by the Frida hook on this same function.
        Iwd2DebugLog("CAST spell=%s", (const char*)resName);

        CAIObjectType enemy = GetAIType().GetEnemyOf();
        LONG nearestEnemy = m_pArea->GetNearest(
            m_id,
            enemy,
            24,
            GetTerrainTable(),
            FALSE,
            GetCanSeeInvisible(),
            FALSE,
            0,
            FALSE);
        if (nearestEnemy != CGameObjectArray::INVALID_INDEX && !field_9D15) {
            if (!field_9D14) {
                field_9D14 = TRUE;
                INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
                INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
                INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
                if ((m_baseStats.field_2FB & 1) != 0) {
                    skill += 10;
                }
                FeedBack(
                    FEEDBACK_ROLL,
                    roll,
                    skill - featBonus,
                    featBonus,
                    0x9BA2,
                    0,
                    m_curSpell->GetLevel());
                field_9D15 = skill + roll < m_curSpell->GetLevel() + 10;
            }
            if (field_9D15) {
                FeedBack(FEEDBACK_SPELLFAILURE_CONCENTRATION, 0, 0, 0, -1, 0, 0);
                m_curSpell->Release();
                return ACTION_DONE;
            }
        }

        // 0x741C8B: when the caster is not in the party, each party member in
        // visual range may identify the spell with Spellcraft (DC 15 + spell
        // level).  The first success names the cast in the combat log and
        // floats the spell name over the caster.
        if (game->GetCharacterPortraitNum(m_id) == -1) {
            SHORT characterCount = game->m_nCharacters;
            for (INT characterIndex = 0; characterIndex < characterCount; ++characterIndex) {
                LONG memberId = characterIndex < game->m_nCharacters
                    ? game->m_characterPortraits[characterIndex]
                    : CGameObjectArray::INVALID_INDEX;

                CGameObject* pMember;
                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(
                        memberId,
                        CGameObjectArray::THREAD_ASYNCH,
                        &pMember,
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    continue;
                }

                CGameSprite* pMemberSprite = static_cast<CGameSprite*>(pMember);
                INT memberSpellcraft =
                    pMemberSprite->m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_SPELLCRAFT];
                INT spellcraftModifier =
                    pMemberSprite->GetSkillModifier(CGAMESPRITE_SKILL_SPELLCRAFT);
                DWORD memberState = pMemberSprite->m_derivedStats.m_generalState;
                CPoint memberPos = pMemberSprite->GetPos();

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                    memberId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                LONG visualRange = GetVisualRange();
                LONG visualRangeSquared = visualRange * GetVisualRange();
                CPoint casterPos = GetPos();
                LONG dx = memberPos.x - casterPos.x;
                LONG dy = memberPos.y - casterPos.y;

                if ((memberState & 0x821) != 0
                    || memberSpellcraft <= 0
                    || dx * dx + dy * dy > visualRangeSquared) {
                    continue;
                }

                INT roll = rand() % 20;
                FeedBack(
                    FEEDBACK_ROLL,
                    memberSpellcraft + roll + 1,
                    m_curSpell->GetLevel() + 15,
                    spellcraftModifier,
                    0x998A,
                    0,
                    0);
                if (memberSpellcraft + roll + 1 >= m_curSpell->GetLevel() + 15) {
                    FeedBack(
                        FEEDBACK_SPELL,
                        casterType != 4,
                        0,
                        0,
                        m_curSpell->GetGenericName(),
                        0,
                        0);
                    CMessage* message = new CMessageFloatText(
                        m_id,
                        m_id,
                        m_curSpell->GetGenericName(),
                        TRUE);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    break;
                }
            }
        }
    }

    INT adjustedSpeed = static_cast<SHORT>(ability->speedFactor)
        - m_derivedStats.m_nMentalSpeed;
    if (adjustedSpeed < 0) {
        adjustedSpeed = 0;
    }
    m_speedFactor -= m_derivedStats.m_nMentalSpeed;
    if (m_speedFactor < 0) {
        m_speedFactor = 0;
    }
    INT castTime = adjustedSpeed * 10;

    if (m_castCounter < castTime) {
        if (!field_9D15
            && (m_nDamageTaken == 0 || m_curSpell->GetCasterType() == 4)) {
            if (initialSequence != SEQ_CONJURE && m_castCounter < castTime - 4) {
                CMessage* message = new CMessageSetSequence(SEQ_CONJURE, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            } else if (initialSequence != SEQ_CAST && m_castCounter >= castTime - 4) {
                CMessage* message = new CMessageSetSequence(SEQ_CAST, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                ApplyCastingEffectPost(m_curSpell, ability);
            }
            m_curSpell->Release();
            return ACTION_NORMAL;
        }

        // Damage taken mid-cast: this tick goes to the concentration roll --
        // no sequence transition happens even when the roll succeeds
        // (0x742117 falls straight into the release/return).
        if (!field_9D15) {
            if (!field_9D14) {
                field_9D14 = TRUE;
                INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
                INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
                INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
                if ((m_baseStats.field_2FB & 1) != 0) {
                    skill += 10;
                }
                FeedBack(
                    FEEDBACK_ROLL,
                    roll,
                    skill - featBonus,
                    featBonus,
                    0x9BA2,
                    m_nDamageTaken,
                    m_curSpell->GetLevel());
                field_9D15 = skill + roll < m_nDamageTaken + m_curSpell->GetLevel() + 10;
            }
            if (!field_9D15) {
                m_curSpell->Release();
                return ACTION_NORMAL;
            }
        }

        FeedBack(FEEDBACK_SPELLFAILED_DISRUPTED, 0, 0, 0, -1, 0, 0);
        m_curSpell->Release();
        return ACTION_DONE;
    }

    BOOL spellFailed = FALSE;
    INT failureRoll = rand() % 100;
    if (casterType != 4) {
        switch (nClass) {
        case CAIOBJECTTYPE_C_CLERIC:
        case CAIOBJECTTYPE_C_DRUID:
        case CAIOBJECTTYPE_C_PALADIN:
        case CAIOBJECTTYPE_C_RANGER:
            if (CheckDivineFailure(failureRoll)) {
                FeedBack(FEEDBACK_SPELLFAILED_CASTFAILURE, 0, 0, 0, -1, 0, 0);
                spellFailed = TRUE;
            }
            break;
        default:
            if (CheckAranceFailure(failureRoll)) {
                FeedBack(FEEDBACK_SPELLFAILED_CASTFAILURE, 0, 0, 0, -1, 0, 0);
                spellFailed = TRUE;
            }
            break;
        }
    }

    if ((m_curSpell->GetItemFlags() & 0x2000) != 0
        && (m_pArea->m_header.m_areaType & 1) == 0) {
        FeedBack(FEEDBACK_SPELLFAILED_INDOORS, 0, 0, 0, -1, 0, 0);
        spellFailed = TRUE;
    }

    if (field_9D15 || (m_nDamageTaken != 0 && m_curSpell->GetCasterType() != 4)) {
        if (!field_9D15 && !field_9D14) {
            field_9D14 = TRUE;
            INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
            INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
            INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
            if ((m_baseStats.field_2FB & 1) != 0) {
                skill += 10;
            }
            FeedBack(
                FEEDBACK_ROLL,
                roll,
                skill - featBonus,
                featBonus,
                0x9BA2,
                m_nDamageTaken,
                m_curSpell->GetLevel());
            field_9D15 = skill + roll < m_nDamageTaken + m_curSpell->GetLevel() + 10;
        }
        if (field_9D15) {
            FeedBack(FEEDBACK_SPELLFAILED_DISRUPTED, 0, 0, 0, -1, 0, 0);
            m_curSpell->Release();
            return ACTION_DONE;
        }
    }

    if (spellFailed) {
        m_curSpell->Release();
        return ACTION_DONE;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_WEB] && casterType != 4) {
        INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
        INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
        INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
        if ((m_baseStats.field_2FB & 1) != 0) {
            skill += 10;
        }
        FeedBack(
            FEEDBACK_ROLL,
            roll,
            skill - featBonus,
            featBonus,
            0x9BA2,
            m_nDamageTaken,
            m_curSpell->GetLevel());
        // Original bug preserved: 0x7423C7 `CMP EBX,EAX; JL 0x7423F0` jumps
        // PAST the disrupted exit when skill+roll < DC, so PASSING the
        // webbed-concentration check disrupts the spell and failing it lets
        // the cast continue.
        if (skill + roll >= m_nDamageTaken + m_curSpell->GetLevel() + 10) {
            FeedBack(FEEDBACK_SPELLFAILED_DISRUPTED, 0, 0, 0, -1, 0, 0);
            m_curSpell->Release();
            return ACTION_DONE;
        }
    }

    FeedBack(
        FEEDBACK_SPELL,
        casterType != 4,
        0,
        0,
        m_curSpell->GetGenericName(),
        0,
        0);

    if (initialSequence != SEQ_CAST) {
        CMessage* message = new CMessageSetSequence(SEQ_CAST, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    m_curProjectile = CProjectile::DecodeProjectile(ability->missileType, this, 0);
    if (m_curProjectile != NULL) {
        m_curProjectile->m_casterResRef = spellResRef;
        // The binary store (0x7424F4) reads a stack slot no instruction on
        // the completion path writes (stale frame data; in the
        // SpellPointSequence twin the same store provably reads the
        // always-zero spell-failure flag). Store the defined value of that
        // set -- the real spell level is only filled by the AI-side
        // FireSpell paths.
        m_curProjectile->m_nSpellLevel = 0;
    }

    for (LONG effectIndex = 0; effectIndex < ability->effectCount; ++effectIndex) {
        // 0x742514: the binary hardcodes nLevel to 0 here (unlike ForceSpell).
        CGameEffect* effect = m_curSpell->BuildAbilityEffect(
            abilityIndex,
            effectIndex,
            this,
            nClass,
            nSpecialization,
            0);
        if (effect == NULL) {
            continue;
        }

        effect->m_source = m_pos;
        effect->m_sourceID = m_id;
        effect->m_target = targetPos;
        // 0x742514: the binary writes the effect's spell level (+0x14) with the
        // same zero local as the m_curProjectile->m_nSpellLevel store above; the
        // player Spell path leaves it 0.  (Previously mis-recovered onto
        // m_effectAmount (+0x18), which wrongly zeroed opcode-449's Static
        // Charge count -- the Call Lightning bolt count.)
        if (effect->m_spellLevel != 0) {
            effect->m_spellLevel = 0;
        }

        IcewindMisc::ApplyDamageModifiers(this, effect);
        if (effect->m_durationType == 0) {
            if (casterType == 1) {
                effect->m_duration = m_derivedStats.m_nSpellDurationModMage
                    * effect->m_duration / 100;
            } else if (casterType == 2) {
                effect->m_duration = m_derivedStats.m_nSpellDurationModPriest
                    * effect->m_duration / 100;
            }
        }

        switch (effect->m_targetType) {
        case 1: {
            effect->m_flags |= 2;
            CMessage* message = new CMessageAddEffect(effect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            continue;
        }
        case 2:
            if (m_curProjectile != NULL) {
                m_curProjectile->AddEffect(effect);
                continue;
            }
            break;
        case 3:
            ApplyEffectToParty(effect);
            break;
        case 4:
            m_pArea->ApplyEffect(effect, FALSE, FALSE, 0, NULL);
            break;
        case 5:
            m_pArea->ApplyEffect(effect, TRUE, FALSE, 0, NULL);
            break;
        case 6:
            m_pArea->ApplyEffect(effect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
            break;
        case 7:
            m_pArea->ApplyEffect(
                effect,
                FALSE,
                TRUE,
                target->GetAIType().m_nSpecific,
                NULL);
            break;
        case 8:
            m_pArea->ApplyEffect(effect, FALSE, FALSE, 0, this);
            break;
        default:
            break;
        }
        delete effect;
    }

    if (m_curProjectile != NULL) {
        LONG height = m_curProjectile->DetermineHeight(this);
        CMessageFireProjectile* message = new CMessageFireProjectile(
            m_curProjectile->m_projectileType,
            target->m_id,
            targetPos,
            height,
            m_id,
            m_id,
            0);
        if (m_curProjectile->m_projectileType == 0x130) {
            // 0x74272C: seed the whirlwind's deterministic wander chain, and
            // replicate it through the message for multiplayer.
            LONG wanderSeed = rand() % 1000000;
            static_cast<CProjectileWhirlwind*>(m_curProjectile)->m_wanderSeed = wanderSeed;
            message->field_20 = wanderSeed;
        }
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        m_curProjectile->Fire(
            m_pArea,
            m_id,
            target->m_id,
            targetPos,
            height,
            0);
        m_curProjectile = NULL;
    }

    m_curSpell->Release();
    AutoPause(0x40);
    AutoPause(0x400);
    return ACTION_DONE;
}

// 0x742840
//
// Sprite executor for the SpellPoint (95) and SpellPointNoDec (192) actions --
// the point-target sibling of CGameSprite::Spell (0x740270).  Same cast
// lifecycle with these differences: the caster orients toward the cast point
// and approaches with MoveToPointRange; there is no appropriate-target check,
// no spellcraft identify and no generic-name skip; the NoDec variant (0xC0)
// creates and records the spell without any count decrement; shapeshift casts
// (spell type 4) burn the shared count and refresh every known shapeshift
// button (no SPIN122 special case); a single quick-button decrement covers the
// other decremented types; the projectile fires at the point with an invalid
// target id and the direct Fire re-derives its height; the disrupted/failed
// completion exits return ACTION_ERROR (Spell returns ACTION_DONE); and the
// second outdoors-only check is live -- it gates the fire block (in Spell it
// is dead).  Wired from ExecuteAction (95/192); the binary reaches it through
// the sprite sequence dispatcher (FUN_00728F80, vtable +0x84, case 0x2c).
SHORT CGameSprite::SpellPointSequence()
{
    SHORT initialSequence = m_nSequence;
    SHORT actionReturn = ACTION_DONE;

    CPoint castPoint = m_curAction.m_dest;

    if (m_typeAI.GetGeneral() == CAIObjectType::G_DEAD) {
        return actionReturn;
    }

    if (m_actionCount == 0) {
        if (m_curProjectile != NULL) {
            delete m_curProjectile;
            m_curProjectile = NULL;
        }

        if (m_derivedStats.m_bAuraCleansing && m_castCounter != -1) {
            FeedBack(FEEDBACK_AURA_CLEANSED, 0, 0, 0, -1, 0, 0);
            m_castCounter = -1;
        }
    }

    if (m_nDirection != GetDirection(castPoint)) {
        CMessage* message = new CMessageSetDirection(castPoint, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        return ACTION_NORMAL;
    }

    CString resName = m_curAction.GetString1();
    if (resName.IsEmpty()) {
        SpellIdToResRef(m_curAction.m_specificID, resName);
    }
    if (resName.IsEmpty()) {
        return ACTION_ERROR;
    }

    CResRef spellResRef(resName);
    CInfGame* game = g_pBaldurChitin->GetObjectGame();

    if (!m_derivedStats.m_spellStates[SPLSTATE_VOCALIZE]
        && (m_derivedStats.m_generalState & STATE_SILENCED) != 0
        && !HasFeat(CGAMESPRITE_FEAT_SUBVOCAL_CASTING)) {
        BYTE spellType = game->GetSpellType(spellResRef);
        if ((spellType == 1 || spellType == 3) && spellResRef != "SPWI219") {
            return ACTION_ERROR;
        }
    }

    BYTE nClass = static_cast<BYTE>(m_curAction.m_specificID2);
    BYTE nSpecializationIndex = static_cast<BYTE>(m_curAction.m_specificID2 >> 8);
    DWORD nSpecialization = game->GetRuleTables().GetSpecializationMask(
        nClass,
        nSpecializationIndex);
    BYTE nSpellLevel = static_cast<BYTE>(m_curAction.m_specificID3);

    if (!m_bStartedCasting) {
        CSpell* targetCheckSpell = m_curSpell;
        BOOL deleteTargetCheckSpell = FALSE;
        if (targetCheckSpell == NULL) {
            targetCheckSpell = new CSpell(spellResRef);
            deleteTargetCheckSpell = TRUE;
        }
        if (targetCheckSpell == NULL) {
            return ACTION_ERROR;
        }

        // itemFlags are read before the range gate; flag 0x800 (the asm tests
        // the NOTed bit) waives the line-of-sight half.
        DWORD itemFlags = targetCheckSpell->GetItemFlags();
        BOOL canCast = CheckCastingRange(targetCheckSpell, castPoint, nClass, nSpecialization)
            && (m_pArea->CheckLOS(m_pos, castPoint, GetVisibleTerrainTable(), FALSE)
                || (itemFlags & 0x800) != 0);

        if (!canCast) {
            SHORT moveResult = MoveToPointRange(m_curAction.m_dest, 0);
            if (moveResult != ACTION_DONE) {
                if (deleteTargetCheckSpell) {
                    delete targetCheckSpell;
                }
                return moveResult;
            }
        } else {
            CMessage* message = new CMessageDropPath(m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (deleteTargetCheckSpell) {
            delete targetCheckSpell;
        }

        if (m_castCounter == -1) {
            m_castCounter = 0;
            m_bInCasting = TRUE;
        }
    }

    if (m_castCounter == 0) {
        ITEM_EFFECT effect;
        if (!CheckInvisibility(FALSE)) {
            CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);
            effect.durationType = 1;
            CGameEffect* visibleEffect = CGameEffect::DecodeEffect(
                &effect,
                m_pos,
                m_id,
                CPoint(-1, -1));
            CMessage* message = new CMessageAddEffect(visibleEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (m_derivedStats.m_spellStates[SPLSTATE_SANCTUARY]) {
            CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_DISPELSANCTUARY);
            effect.durationType = 1;
            CGameEffect* sanctuaryEffect = CGameEffect::DecodeEffect(
                &effect,
                m_pos,
                m_id,
                CPoint(-1, -1));
            CMessage* message = new CMessageAddEffect(sanctuaryEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (m_curSpell != NULL) {
            delete m_curSpell;
            m_curSpell = NULL;
        }

        if (m_curAction.m_actionID == CAIAction::SPELLPOINTNODEC) {
            m_curSpell = new CSpell(spellResRef);
            m_cGameStats.RecordSpellUse(spellResRef);
            if (m_curSpell == NULL) {
                return ACTION_ERROR;
            }
            // The binary fetches the caster type here and discards the result.
            m_curSpell->GetCasterType();
        } else {
            if (resName.Left(2) == "**") {
                CString obsoleteMsg;
                obsoleteMsg.Format(
                    "*** %s is trying to cast an OBSOLETE SPELL: %s.\n",
                    static_cast<LPCTSTR>(m_sName),
                    spellResRef);
                return ACTION_ERROR;
            }

            m_cGameStats.RecordSpellUse(spellResRef);
            m_curSpell = new CSpell(spellResRef);
            if (m_curSpell == NULL) {
                return ACTION_ERROR;
            }

            SHORT casterType = m_curSpell->GetCasterType();
            INT disabledType = -1;
            if (casterType == 1) {
                disabledType = 0;
            } else if (casterType == 2) {
                disabledType = 1;
            } else if (casterType == 4) {
                disabledType = 2;
            } else {
                UTIL_ASSERT_MSG(FALSE, "Invalid spell type");
            }

            BOOL spellDisabled = disabledType >= 0
                && m_derivedStats.m_disabledSpellTypes[disabledType];

            BYTE spellType = game->GetSpellType(spellResRef);
            UINT spellID = 0;
            UINT spellIndex = 0;
            UINT levelIndex = static_cast<UINT>(nSpellLevel) - 1;
            BOOL updateCastButton = TRUE;

            if (spellType == 1) {
                if (!game->m_spells.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                UINT classIndex = game->GetSpellcasterIndex(nClass);
                if (nSpecializationIndex == 0) {
                    if (nClass == CAIOBJECTTYPE_C_BARD
                        || nClass == CAIOBJECTTYPE_C_SORCERER) {
                        m_spells.Get(classIndex)->SubtractFromSharedCurrentCountAtLevel(
                            levelIndex,
                            1,
                            FALSE);
                        updateCastButton = FALSE;

                        // Spontaneous casters burn shared slots: every known
                        // spell of this level loses one displayed use on the
                        // quick bar.
                        CAbilityId buttonAbility;
                        buttonAbility.m_itemType = 1;
                        buttonAbility.m_nClass = nClass;
                        buttonAbility.m_bCanUse = nSpellLevel;

                        CGameSpriteSpellList* pLevelList =
                            m_spells.GetSpellsAtLevel(classIndex, levelIndex);
                        for (UINT knownIndex = 0;
                             knownIndex < pLevelList->m_List.size();
                             ++knownIndex) {
                            UINT knownSpellID = pLevelList->Get(knownIndex)->m_nID;
                            buttonAbility.m_res = game->m_spells.Get(knownSpellID);
                            UpdateQuickButtons(buttonAbility, -1, FALSE, FALSE);
                        }
                    } else {
                        SubtractFromSpellCount(nClass, levelIndex, spellResRef, 1, FALSE);
                    }
                } else {
                    SubtractFromDomainSpellCount(levelIndex, spellResRef, 1, FALSE);
                }
            } else if (spellType == 2) {
                if (!game->m_innateSpells.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                if (m_innateSpells.Find(spellID, spellIndex)) {
                    if (spellResRef == SPIN275
                        || spellResRef == SPIN276
                        || spellResRef == SPIN277
                        || spellResRef == SPIN278
                        || spellResRef == SPIN279) {
                        updateCastButton = FALSE;
                    } else {
                        SubtractFromInnateSpellCount(spellResRef, 1, FALSE);
                    }
                }
            } else if (spellType == 3) {
                if (!game->m_songs.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                m_songs.Find(spellID, spellIndex);
            } else if (spellType == 4) {
                if (!game->m_shapeshifts.Find(spellResRef, spellID)) {
                    return ACTION_ERROR;
                }
                if (m_shapeshifts.Find(spellID, spellIndex)) {
                    m_shapeshifts.SubtractFromSharedCurrentCount(1, FALSE);
                    updateCastButton = FALSE;

                    // Shapeshifts share one pool: refresh every known
                    // shapeshift button.
                    CAbilityId buttonAbility;
                    buttonAbility.m_itemType = 1;
                    buttonAbility.m_nClass = nClass;
                    buttonAbility.m_bCanUse = nSpellLevel;
                    for (UINT knownIndex = 0;
                         knownIndex < m_shapeshifts.m_List.size();
                         ++knownIndex) {
                        UINT knownSpellID = m_shapeshifts.Get(knownIndex)->m_nID;
                        buttonAbility.m_res = game->m_shapeshifts.Get(knownSpellID);
                        UpdateQuickButtons(buttonAbility, -1, FALSE, FALSE);
                    }
                }
            }

            if (spellDisabled) {
                delete m_curSpell;
                m_curSpell = NULL;
                return ACTION_ERROR;
            }

            if (updateCastButton) {
                CAbilityId buttonAbility;
                buttonAbility.m_itemType = 1;
                buttonAbility.m_res = spellResRef;
                if (spellType == 1) {
                    buttonAbility.m_nClass = nClass;
                    buttonAbility.m_bCanUse = nSpellLevel;
                    buttonAbility.m_nTooltip = nSpecializationIndex;
                }
                UpdateQuickButtons(buttonAbility, -1, FALSE, FALSE);
            }
        }
    }

    if (m_curSpell == NULL) {
        if (m_typeAI.m_nEnemyAlly <= CAIObjectType::EA_GOODCUTOFF) {
            return ACTION_INTERRUPTABLE;
        }
        return ACTION_NORMAL;
    }

    // Unlike Spell, Demand's result is not checked: a failed demand yields no
    // abilities and exits through the ability == NULL path below.
    m_curSpell->Demand();

    SHORT casterLevel = GetCasterLevel(m_curSpell, nClass, nSpecialization);
    Spell_ability_st* ability = NULL;
    INT acceptedCount = 0;
    if (m_curSpell->GetAbilityCount() > 0) {
        do {
            if (casterLevel < m_curSpell->GetAbility(acceptedCount)->minCasterLevel) {
                break;
            }
            ability = m_curSpell->GetAbility(acceptedCount);
            ++acceptedCount;
        } while (acceptedCount < m_curSpell->GetAbilityCount());
    }
    LONG abilityIndex = acceptedCount - 1;

    if (ability == NULL) {
        m_curSpell->Release();
        return ACTION_ERROR;
    }

    if (m_castCounter == 0) {
        ApplyCastingEffect(m_curSpell, ability, castPoint);

        // Spell-capture marker (no-op unless .\iwd2-re-debug.enabled exists): the
        // spell_capture rig tails this line to auto-clip the cast. Fires once per
        // cast at the visual cast-start, keyed by the spell resref. Point-target
        // sibling of the CGameSprite::Spell marker. Not present in IWD2.exe.
        Iwd2DebugLog("CAST spell=%s", (const char*)resName);
    }

    INT adjustedSpeed = static_cast<SHORT>(ability->speedFactor)
        - m_derivedStats.m_nMentalSpeed;
    if (adjustedSpeed < 0) {
        adjustedSpeed = 0;
    }
    m_speedFactor -= m_derivedStats.m_nMentalSpeed;
    if (m_speedFactor < 0) {
        m_speedFactor = 0;
    }
    INT castTime = adjustedSpeed * 10;

    if (m_castCounter < castTime) {
        if (field_9D15
            || (m_nDamageTaken != 0 && m_curSpell->GetCasterType() != 4)) {
            // Damage taken mid-cast: this tick goes to the concentration
            // roll -- no sequence transition happens even when the roll
            // succeeds (same shape as Spell).
            if (!field_9D15) {
                if (!field_9D14) {
                    field_9D14 = TRUE;
                    INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
                    INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
                    INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
                    if ((m_baseStats.field_2FB & 1) != 0) {
                        skill += 10;
                    }
                    FeedBack(
                        FEEDBACK_ROLL,
                        roll,
                        skill - featBonus,
                        featBonus,
                        0x9BA2,
                        m_nDamageTaken,
                        m_curSpell->GetLevel());
                    field_9D15 = skill + roll < m_nDamageTaken + m_curSpell->GetLevel() + 10;
                }
                if (!field_9D15) {
                    m_curSpell->Release();
                    return ACTION_NORMAL;
                }
            }

            FeedBack(FEEDBACK_SPELLFAILED_DISRUPTED, 0, 0, 0, -1, 0, 0);
            m_curSpell->Release();
            return ACTION_DONE;
        }

        if (initialSequence != SEQ_CONJURE && m_castCounter < castTime - 4) {
            CMessage* message = new CMessageSetSequence(SEQ_CONJURE, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else if (initialSequence != SEQ_CAST && m_castCounter >= castTime - 4) {
            CMessage* message = new CMessageSetSequence(SEQ_CAST, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            ApplyCastingEffectPost(m_curSpell, ability);
        }
        m_curSpell->Release();
        return ACTION_NORMAL;
    }

    BOOL spellFailed = FALSE;
    INT failureRoll = rand() % 100;
    SHORT casterType = m_curSpell->GetCasterType();
    if (casterType != 4) {
        switch (nClass) {
        case CAIOBJECTTYPE_C_CLERIC:
        case CAIOBJECTTYPE_C_DRUID:
        case CAIOBJECTTYPE_C_PALADIN:
        case CAIOBJECTTYPE_C_RANGER:
            if (CheckDivineFailure(failureRoll)) {
                FeedBack(FEEDBACK_SPELLFAILED_CASTFAILURE, 0, 0, 0, -1, 0, 0);
                spellFailed = TRUE;
            }
            break;
        default:
            if (CheckAranceFailure(failureRoll)) {
                FeedBack(FEEDBACK_SPELLFAILED_CASTFAILURE, 0, 0, 0, -1, 0, 0);
                spellFailed = TRUE;
            }
            break;
        }
    }

    if ((m_curSpell->GetItemFlags() & 0x2000) != 0
        && (m_pArea->m_header.m_areaType & 1) == 0) {
        FeedBack(FEEDBACK_SPELLFAILED_INDOORS, 0, 0, 0, -1, 0, 0);
        spellFailed = TRUE;
    }

    if (field_9D15 || (m_nDamageTaken != 0 && m_curSpell->GetCasterType() != 4)) {
        if (!field_9D15 && !field_9D14) {
            field_9D14 = TRUE;
            INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
            INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
            INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
            if ((m_baseStats.field_2FB & 1) != 0) {
                skill += 10;
            }
            FeedBack(
                FEEDBACK_ROLL,
                roll,
                skill - featBonus,
                featBonus,
                0x9BA2,
                m_nDamageTaken,
                m_curSpell->GetLevel());
            field_9D15 = skill + roll < m_nDamageTaken + m_curSpell->GetLevel() + 10;
        }
        if (field_9D15) {
            FeedBack(FEEDBACK_SPELLFAILED_DISRUPTED, 0, 0, 0, -1, 0, 0);
            m_curSpell->Release();
            return ACTION_ERROR;
        }
    }

    if (spellFailed) {
        m_curSpell->Release();
        return ACTION_ERROR;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_WEB] && casterType != 4) {
        INT featBonus = GetFeatValue(CGAMESPRITE_FEAT_COMBAT_CASTING) != 0 ? 4 : 0;
        INT roll = CUtil::UtilRandInt(20, m_derivedStats.m_nLuck) + 1;
        INT skill = m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_CONCENTRATION];
        if ((m_baseStats.field_2FB & 1) != 0) {
            skill += 10;
        }
        FeedBack(
            FEEDBACK_ROLL,
            roll,
            skill - featBonus,
            featBonus,
            0x9BA2,
            m_nDamageTaken,
            m_curSpell->GetLevel());
        // Original bug preserved (same inverted comparison as Spell's
        // 0x7423C7): PASSING the webbed-concentration check disrupts the
        // spell and failing it lets the cast continue.
        if (skill + roll >= m_nDamageTaken + m_curSpell->GetLevel() + 10) {
            FeedBack(FEEDBACK_SPELLFAILED_DISRUPTED, 0, 0, 0, -1, 0, 0);
            m_curSpell->Release();
            return ACTION_ERROR;
        }
    }

    if ((m_curSpell->GetItemFlags() & 0x2000) != 0
        && (m_pArea->m_header.m_areaType & 1) == 0) {
        // Live second outdoors-only check: at completion it withholds the
        // fire block entirely (in Spell the equivalent test is dead).
        FeedBack(FEEDBACK_SPELLFAILED_INDOORS, 0, 0, 0, -1, 0, 0);
    } else {
        FeedBack(
            FEEDBACK_SPELL,
            casterType != 4,
            0,
            0,
            m_curSpell->GetGenericName(),
            0,
            0);

        if (initialSequence != SEQ_CAST) {
            CMessage* message = new CMessageSetSequence(SEQ_CAST, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        m_curProjectile = CProjectile::DecodeProjectile(ability->missileType, this, 0);
        if (m_curProjectile != NULL) {
            m_curProjectile->m_casterResRef = spellResRef;
            // The binary store (0x7443DC) reads the spell-failure flag,
            // which is provably zero here: the whole fire block is guarded
            // by `flag == 0` and nothing rewrites it in between.
            m_curProjectile->m_nSpellLevel = 0;
        }

        for (LONG effectIndex = 0; effectIndex < ability->effectCount; ++effectIndex) {
            CGameEffect* effect = m_curSpell->BuildAbilityEffect(
                abilityIndex,
                effectIndex,
                this,
                nClass,
                nSpecialization,
                0);
            if (effect == NULL) {
                continue;
            }

            effect->m_source = m_pos;
            effect->m_sourceID = m_id;
            effect->m_target = castPoint;
            // 0x742XXX: writes the effect's spell level (+0x14) with the same
            // zero flag as the m_nSpellLevel store above.  (Previously
            // mis-recovered onto m_effectAmount (+0x18), zeroing opcode-449's
            // Static Charge count.)
            if (effect->m_spellLevel != 0) {
                effect->m_spellLevel = 0;
            }

            IcewindMisc::ApplyDamageModifiers(this, effect);
            if (effect->m_durationType == 0) {
                if (casterType == 1) {
                    effect->m_duration = m_derivedStats.m_nSpellDurationModMage
                        * effect->m_duration / 100;
                } else if (casterType == 2) {
                    effect->m_duration = m_derivedStats.m_nSpellDurationModPriest
                        * effect->m_duration / 100;
                }
            }

            switch (effect->m_targetType) {
            case 1: {
                effect->m_flags |= 2;
                CMessage* message = new CMessageAddEffect(effect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                continue;
            }
            case 2:
                if (m_curProjectile != NULL) {
                    m_curProjectile->AddEffect(effect);
                    continue;
                }
                break;
            case 3:
                ApplyEffectToParty(effect);
                break;
            case 4:
                m_pArea->ApplyEffect(effect, FALSE, FALSE, 0, NULL);
                break;
            case 5:
                m_pArea->ApplyEffect(effect, TRUE, FALSE, 0, NULL);
                break;
            case 6:
                m_pArea->ApplyEffect(effect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
                break;
            case 7:
                // No target object on a point cast: the 7th target type just
                // discards the effect.
                break;
            case 8:
                m_pArea->ApplyEffect(effect, FALSE, FALSE, 0, this);
                break;
            default:
                break;
            }
            delete effect;
        }

        if (m_curProjectile != NULL) {
            LONG height = m_curProjectile->DetermineHeight(this);
            CMessageFireProjectile* message = new CMessageFireProjectile(
                m_curProjectile->m_projectileType,
                CGameObjectArray::INVALID_INDEX,
                castPoint,
                height,
                m_id,
                m_id,
                0);
            if (m_curProjectile->m_projectileType == 0x130) {
                // Seed the whirlwind's deterministic wander chain, and
                // replicate it through the message for multiplayer.
                LONG wanderSeed = rand() % 1000000;
                static_cast<CProjectileWhirlwind*>(m_curProjectile)->m_wanderSeed = wanderSeed;
                message->field_20 = wanderSeed;
            }
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            // The binary re-derives the height for the direct fire.
            m_curProjectile->Fire(
                m_pArea,
                m_id,
                CGameObjectArray::INVALID_INDEX,
                castPoint,
                m_curProjectile->DetermineHeight(this),
                0);
            m_curProjectile = NULL;
        }
    }

    m_curSpell->Release();
    AutoPause(0x40);
    AutoPause(0x400);
    return ACTION_DONE;
}

// 0x7473E0
//
// Sprite executor for the UseItemPoint (97) action: resolve the item (slot
// index from the action, or an equipment scan when string1 names a resref),
// then run the item-use lifecycle at the target point -- depleted-charge
// Unusable trigger, range/LOS approach with MoveToPointRange, orient, a
// 20-tick use time, invisibility/sanctuary dispel, then fire the ability
// projectile with its effects and consume one charge.  Wired from
// ExecuteAction (jumptable case 0x2e).
SHORT CGameSprite::UseItemPoint()
{
    field_55A0 = 0;
    CPoint castPoint = m_curAction.m_dest;
    ITEM_ABILITY* ability = NULL;

    if (m_actionCount == 0) {
        m_curItem = NULL;
    }

    if (m_curItem == NULL) {
        CString resName = m_curAction.GetString1();
        if (resName.CompareNoCase("") == 0) {
            if (m_curAction.m_specificID == -1) {
                return ACTION_ERROR;
            }

            m_curItem = m_equipment.m_items[m_curAction.m_specificID];
            if (m_curItem == NULL) {
                return ACTION_ERROR;
            }
            field_559E = static_cast<short>(m_curAction.m_specificID);

            m_curItem->Demand();
            ability = m_curItem->GetAbility(m_curAction.m_specificID2);
            field_55A0 = static_cast<short>(m_curAction.m_specificID2);
            if (ability == NULL) {
                m_curItem->Release();
                return ACTION_ERROR;
            }
        } else {
            for (INT nSlot = 0; nSlot < CGameSpriteEquipment::NUM_SLOT; ++nSlot) {
                if (m_equipment.m_items[nSlot] == NULL) {
                    continue;
                }

                if (m_equipment.m_items[nSlot]->cResRef == m_curAction.GetString1()) {
                    m_curItem = m_equipment.m_items[nSlot];
                    field_559E = static_cast<short>(nSlot);
                    if (m_curItem == NULL) {
                        return ACTION_ERROR;
                    }

                    m_curItem->Demand();
                    ability = m_curItem->GetAbility(m_curAction.m_specificID2);
                    field_55A0 = static_cast<short>(m_curAction.m_specificID2);
                    if (ability == NULL) {
                        m_curItem->Release();
                        return ACTION_ERROR;
                    }
                    break;
                }
            }
        }
    } else {
        m_curItem->Demand();
    }

    if (m_curItem == NULL) {
        return ACTION_ERROR;
    }

    if (ability == NULL) {
        ability = m_curItem->GetAbility(m_curAction.m_specificID2);
        field_55A0 = static_cast<short>(m_curAction.m_specificID2);
        if (ability == NULL) {
            m_curItem->Release();
            return ACTION_ERROR;
        }
    }

    if (ability->usageFlags == 3
        && m_curItem->GetUsageCount(field_55A0) == 0
        && ability->maxUsageCount > 0) {
        CString sItemRes;
        m_curItem->cResRef.CopyToString(sItemRes);

        CAITrigger trigger(CAITrigger::UNUSABLE, 0);
        trigger.m_string1 = sItemRes;

        CMessage* message = new CMessageSetTrigger(trigger, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        m_curItem->Release();
        return ACTION_ERROR;
    }

    LONG gridTargetX = castPoint.x / CPathSearch::GRID_SQUARE_SIZEX;
    LONG gridTargetY = castPoint.y / CPathSearch::GRID_SQUARE_SIZEY;
    const CPoint& pos = GetPos();
    LONG dx = pos.x / CPathSearch::GRID_SQUARE_SIZEX - gridTargetX;
    LONG dy = pos.y / CPathSearch::GRID_SQUARE_SIZEY - gridTargetY;
    LONG distance = dx * dx + dy * dy;
    LONG range = ability->range + 1;
    BYTE personalHalf = static_cast<BYTE>((m_animation.GetPersonalSpace() - 1) >> 1);

    LONG nearLimit = personalHalf + range;
    // SHORT @ 0x85BC86 (= 1): extra approach slack once the use has started.
    LONG farLimit = range + 1 + personalHalf;
    if ((nearLimit * nearLimit < distance && !field_7118)
        || farLimit * farLimit < distance
        || !m_pArea->CheckLOS(castPoint, GetPos(), GetVisibleTerrainTable(), FALSE)) {
        SHORT moveResult = MoveToPointRange(castPoint, 0);
        if (moveResult == ACTION_DONE) {
            moveResult = ACTION_INTERRUPTABLE;
        }
        m_curItem->Release();
        return moveResult;
    }

    if (m_pPath != NULL) {
        CMessage* message = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    SHORT oldDirection = m_nDirection;
    field_7118 = 1;
    if (oldDirection != GetDirection(castPoint)) {
        CMessage* message = new CMessageSetDirection(castPoint, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        m_curItem->Release();
        return ACTION_NORMAL;
    }

    if (m_pPath != NULL) {
        CMessage* message = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    if (m_actionCount == 0) {
        if (m_castCounter != -1) {
            m_bStartedCasting = FALSE;
        } else {
            m_castCounter = 0;
            m_bStartedCasting = TRUE;
        }
    } else if (m_castCounter == -1) {
        m_castCounter = 0;
        m_bStartedCasting = TRUE;
    }

    if (m_actionCount < 20 || !m_bStartedCasting) {
        m_curItem->Release();
        return ACTION_INTERRUPTABLE;
    }

    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    INT usable = game->CheckItemUsable(this, m_curItem);
    if (usable == 2) {
        if (CRuleTables::CheckUseMagicDevice(this, m_curItem)) {
            FeedBack(0x42, 0, 0, 0, -1, 0, 0);
        } else {
            FeedBack(0x41, 0, 0, 0, -1, 0, 0);

            INT backfireLevel = m_curItem->GetMaxEffectSpellLevel();
            DWORD backfireDice = 2 * (backfireLevel < 1 ? 1 : backfireLevel);
            CGameEffect* backfireEffect = IcewindMisc::CreatePiercingDamageEffect(
                this, backfireDice, 6, 0, static_cast<BYTE>(backfireLevel), 0);
            backfireEffect->m_flags |= 2;

            CMessage* backfireMessage = new CMessageAddEffect(backfireEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(backfireMessage, FALSE);
            goto consumeCharge;
        }
    }

    ITEM_EFFECT effect;
    if (!CheckInvisibility(FALSE)) {
        CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_FORCEVISIBLE);
        effect.durationType = 1;
        CGameEffect* visibleEffect = CGameEffect::DecodeEffect(
            &effect,
            m_pos,
            m_id,
            CPoint(-1, -1));
        CMessage* message = new CMessageAddEffect(visibleEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_SANCTUARY]) {
        CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_DISPELSANCTUARY);
        effect.durationType = 1;
        CGameEffect* sanctuaryEffect = CGameEffect::DecodeEffect(
            &effect,
            m_pos,
            m_id,
            CPoint(-1, -1));
        CMessage* message = new CMessageAddEffect(sanctuaryEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    m_curProjectile = CProjectile::DecodeProjectile(ability->missileType, this, 0);
    m_curProjectile->m_casterResRef = m_curItem->cResRef;
    m_curProjectile->m_nSpellLevel = 0;

    for (INT effectIndex = 0; effectIndex < ability->effectCount; ++effectIndex) {
        CGameEffect* itemEffect = m_curItem->GetAbilityEffect(field_55A0, effectIndex, this);
        itemEffect->m_source = m_pos;
        itemEffect->m_sourceID = m_id;
        itemEffect->m_target = castPoint;

        IcewindMisc::ApplyDamageModifiers(this, itemEffect);

        switch (itemEffect->m_targetType) {
        case 1: {
            itemEffect->m_flags |= 2;
            CMessage* message = new CMessageAddEffect(itemEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            continue;
        }
        case 2:
            m_curProjectile->AddEffect(itemEffect);
            continue;
        case 3:
            ApplyEffectToParty(itemEffect);
            break;
        case 4:
            m_pArea->ApplyEffect(itemEffect, FALSE, FALSE, 0, NULL);
            break;
        case 5:
            m_pArea->ApplyEffect(itemEffect, TRUE, FALSE, 0, NULL);
            break;
        case 6:
            m_pArea->ApplyEffect(itemEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
            break;
        case 7:
            break;
        case 8:
            m_pArea->ApplyEffect(itemEffect, FALSE, FALSE, 0, this);
            break;
        default:
            break;
        }
        delete itemEffect;
    }

    {
        CMessageFireProjectile* message = new CMessageFireProjectile(
            m_curProjectile->m_projectileType,
            CGameObjectArray::INVALID_INDEX,
            castPoint,
            m_curProjectile->DetermineHeight(this),
            m_id,
            m_id,
            0);
        if (m_curProjectile->m_projectileType == 0x130) {
            // Seed the whirlwind's deterministic wander chain, and replicate it
            // through the message for multiplayer.
            LONG wanderSeed = rand() % 1000000;
            static_cast<CProjectileWhirlwind*>(m_curProjectile)->m_wanderSeed = wanderSeed;
            message->field_20 = wanderSeed;
        }
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    // The binary re-derives the height for the direct fire.
    m_curProjectile->Fire(
        m_pArea,
        m_id,
        CGameObjectArray::INVALID_INDEX,
        castPoint,
        m_curProjectile->DetermineHeight(this),
        0);
    m_curProjectile = NULL;

consumeCharge:
    WORD usageCount = m_curItem->GetUsageCount(field_55A0);
    m_curItem->SetUsageCount(field_55A0, usageCount - 1);
    m_curItem->Release();

    if (ability->maxUsageCount > 0) {
        // UNIMPLEMENTED: the binary removes the button when
        // ability->usageFlags == 1 and FUN_0075D450(field_559E, field_55A0)
        // (the depleted-item check/cleanup helper, unrecovered) reports the
        // item spent; the refresh runs without removal instead.
        CAbilityId buttonAbility;
        buttonAbility.m_itemType = 2;
        buttonAbility.m_itemNum = field_559E;
        buttonAbility.m_abilityNum = field_55A0;
        UpdateQuickButtons(buttonAbility, -1, FALSE, FALSE);

        if (game->GetCharacterPortraitNum(m_id)
            == g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter()) {
            game->m_cButtonArray.UpdateButtons();
        }
    }

    AutoPause(0x40);
    return ACTION_DONE;
}

// 0x7564E0
void CGameSprite::ApplyCastingEffectPost(CSpell* pSpell, const Spell_ability_st* pAbility)
{
    if (pSpell == NULL || pAbility == NULL) {
        return;
    }
    SHORT casterType = pSpell->GetCasterType();
    const char* suffix = GetCastingSoundSuffix(pSpell->GetAnimationType(), FALSE);
    if (suffix == NULL) {
        return;
    }

    CString resName("CAS_");
    resName += (casterType == 2) ? 'P' : 'M';
    resName += suffix;

    CResRef resRef = static_cast<LPCSTR>(resName);
    PlayCastingSound(m_sndMagic, resRef, 4, m_pArea, m_pos, m_posZ, m_id);
}

// Main cast-completion handler, called by ForceSpellAction (and its
// ForceSpellPoint / scripted-projectile siblings) when m_actionCount
// crosses the cast-time threshold.  Three layers of work in the binary:
//   1. Chant voice cue ("CHA_" + gender + casterType + randomised
//      variant tag, with a HARPY06 special case for harpy callers)
//      loaded into m_sndVoice on channel 2 -- gated on the cast being
//      slow enough (speedFactor - mentalSpeed >= 3) AND the caster being
//      a standard race.
//   2. Pre-cast audio cue ("PRE_" + casterType + "0N") loaded into
//      m_sndMagic on channel 4 and broadcast via CMessagePlaySoundRef.
//   3. Projectile-spawn visual: builds an ITEM_EFFECT template (opcode
//      0x29 SpawnCreature or 0xEB VisualSpellHit depending on animation
//      type), runs CGameEffect::DecodeEffect against the target's CPoint
//      with the Empower-feat / mage-bonus-level-adjusted duration, then
//      queues a CMessageAddEffect on self.
//   4. The 8-case casting-effect dispatch loop -- iterates CSpell's
//      casting-effect list (GetCastingEffectNo / GetCastingEffect) and
//      forwards each effect by m_targetType into either a per-target
//      CMessageAddEffect (case 1), ApplyEffectToParty (case 3), or one
//      of five CGameArea::ApplyEffect filters (cases 4-8).
//
// Layers 1-3 still TODO (sound + projectile-visual only -- no gameplay
// impact when missing).  Layer 4 is the gameplay-critical bit that
// actually applies damage / status / buffs and is fully wired here so
// ForceSpell stops dropping spell effects on the floor.
// 0x755A70
void CGameSprite::ApplyCastingEffect(CSpell* pSpell,
    const Spell_ability_st* pAbility,
    const CPoint& targetPos)
{
    if (pSpell == NULL || pAbility == NULL) {
        return;
    }

    SHORT casterType = pSpell->GetCasterType();
    WORD animationType = pSpell->GetAnimationType();
    char casterTypeChar = (casterType == 2) ? 'P' : 'M';

    SHORT chantDelay = static_cast<SHORT>(pAbility->speedFactor) - m_derivedStats.m_nMentalSpeed;
    char genderChar = 'N';
    if (chantDelay >= 3) {
        genderChar = (m_typeAI.m_nGender == CAIObjectType::SEX_FEMALE) ? 'F' : 'M';
    }

    CString chantResName;
    BOOL playChant = GetCastingChantResRef(animationType, genderChar, casterTypeChar, chantResName);
    if (m_typeAI.m_nRace == CAIObjectType::R_HARPY) {
        chantResName = "HARPY06";
        playChant = TRUE;
    }
    if (playChant
        && (m_typeAI.m_nRace == CAIObjectType::R_HARPY
            || (genderChar != 'N' && IcewindMisc::IsStandardRace(this)))) {
        CResRef chantResRef = static_cast<LPCSTR>(chantResName);
        PlayCastingSound(m_sndVoice, chantResRef, 2, m_pArea, m_pos, m_posZ, m_id);
    }

    CString preResName("PRE_");
    preResName += casterTypeChar;
    preResName += GetCastingSoundSuffix(animationType, TRUE);

    CResRef preResRef = static_cast<LPCSTR>(preResName);
    PlayCastingSound(m_sndMagic, preResRef, 4, m_pArea, m_pos, m_posZ, m_id);

    WORD visualEffectID;
    LONG visualEffectAmount;
    DWORD visualDwFlags;
    if (GetCastingVisualEffect(animationType, visualEffectID, visualEffectAmount, visualDwFlags)) {
        WORD adjustedSpeed = pAbility->speedFactor;
        if (HasFeat(32)) {
            adjustedSpeed = adjustedSpeed > 0 ? adjustedSpeed - 1 : 0;
        }

        LONG durationDelta = adjustedSpeed - m_derivedStats.m_nMentalSpeed;
        if (durationDelta < 0) {
            durationDelta = 0;
        }
        durationDelta = durationDelta * 100 / 10;

        ITEM_EFFECT effect;
        CGameEffect::ClearItemEffect(&effect, visualEffectID);
        effect.effectAmount = visualEffectAmount;
        effect.dwFlags = visualDwFlags;
        effect.durationType = 0x1000;
        effect.duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime
            + durationDelta
            + 4;

        CGameEffect* pVisualEffect = CGameEffect::DecodeEffect(&effect,
            targetPos,
            m_id,
            CPoint(-1, -1));
        if (pVisualEffect != NULL) {
            CMessage* msg = new CMessageAddEffect(pVisualEffect, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
        }
    }

    // Casting-effect dispatch -- iterate CSpell::GetCastingEffect and
    // forward each effect by m_targetType.
    SHORT nCount = pSpell->GetCastingEffectNo();
    for (LONG i = 0; i < nCount; ++i) {
        CGameEffect* pEffect = pSpell->GetCastingEffect(i);
        if (pEffect == NULL) {
            continue;
        }

        pEffect->m_source = m_pos;
        pEffect->m_sourceID = m_id;
        pEffect->m_target = m_pos;

        switch (pEffect->m_targetType) {
        case 1:
            // Self-targeted -- queue add-effect on caster.  Ownership
            // transfers to CMessageAddEffect.
            {
                CMessage* msg = new CMessageAddEffect(pEffect, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(msg, FALSE);
            }
            continue;
        case 2:
            // Reserved for in-flight projectile attach (CProjectile::AddEffect).
            // Binary asserts here -- unreachable at cast completion.
            break;
        case 3:
            ApplyEffectToParty(pEffect);
            break;
        case 4:
            m_pArea->ApplyEffect(pEffect, FALSE, FALSE, 0, NULL);
            break;
        case 5:
            m_pArea->ApplyEffect(pEffect, TRUE, FALSE, 0, NULL);
            break;
        case 6:
            m_pArea->ApplyEffect(pEffect, FALSE, TRUE, m_typeAI.m_nSpecific, NULL);
            break;
        case 7:
            // Binary reads m_typeAI.m_nSpecific via the GetAIType() vtable
            // entry rather than the direct +0x33 offset of case 6 -- same
            // value, same caster, different access path.  Preserve the
            // GetAIType() reading for binary fidelity.
            m_pArea->ApplyEffect(pEffect, FALSE, TRUE, GetAIType().m_nSpecific, NULL);
            break;
        case 8:
            m_pArea->ApplyEffect(pEffect, FALSE, FALSE, 0, this);
            break;
        default:
            break;
        }

        // Cases 3-8 (and default) get a deleting destructor invocation in
        // the binary -- the dispatch helpers all copy the effect before
        // queueing, so the locally-decoded copy must be released.
        delete pEffect;
    }
}

// 0x728270
void CGameSprite::PlaySound(const CResRef& res)
{
    CSound cSound;

    CMessage* message = new CMessagePlaySoundRef(res, m_id, m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    cSound.SetResRef(res, TRUE, TRUE);

    if (cSound.m_nLooping == 0) {
        cSound.SetFireForget(TRUE);
    }

    cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
    cSound.Play(GetPos().x, GetPos().y, 0, FALSE);
}

// 0x728460
BOOL CGameSprite::Orderable(BOOL bIgnoreControl)
{
    if (!bIgnoreControl && !InControl()) {
        return FALSE;
    }

    if ((m_derivedStats.m_generalState & STATE_DEAD) != 0) {
        return FALSE;
    }

    if (m_nUnselectableCounter > 0) {
        return FALSE;
    }

    if (m_typeAI.GetEnemyAlly() > CAIObjectType::EA_CONTROLCUTOFF) {
        return FALSE;
    }

    if (m_moraleFailure) {
        return FALSE;
    }

    if ((m_derivedStats.m_generalState & STATE_STONE_DEATH) != 0
        || (m_derivedStats.m_generalState & STATE_FROZEN_DEATH) != 0
        || (m_derivedStats.m_generalState & STATE_HELPLESS) != 0
        || (m_derivedStats.m_generalState & STATE_STUNNED) != 0
        || (m_derivedStats.m_generalState & STATE_PANIC) != 0
        || ((m_derivedStats.m_generalState & STATE_BERSERK) != 0
            && m_berserkActive)
        || m_derivedStats.m_spellStates[SPLSTATE_ANIMAL_RAGE]
        || (m_derivedStats.m_generalState & STATE_SLEEPING) != 0
        || ((m_derivedStats.m_generalState & STATE_CHARMED) != 0
            && m_typeAI.GetEnemyAlly() != CAIObjectType::EA_0x847C3A
            && m_typeAI.GetEnemyAlly() != CAIObjectType::EA_CONTROLLED)
        || (m_derivedStats.m_generalState & STATE_FEEBLEMINDED) != 0
        || (m_derivedStats.m_generalState & STATE_CONFUSED) != 0) {
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->m_nTimeStop != 0
        && g_pBaldurChitin->GetObjectGame()->m_nTimeStopCaster != m_id) {
        return FALSE;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_179]) {
        return FALSE;
    }

    return TRUE;
}

// 0x728560
BOOL CGameSprite::Animate()
{
    return (~m_derivedStats.m_generalState >> 11) & 0x1;
}

// 0x728570
DWORD CGameSprite::GetSpecialization()
{
    return m_baseStats.m_specialization;
}

// 0x728BC0
void CGameSprite::ResolveInstants(BOOL dropNonInstants)
{
    if (m_moveToFrontQueue > 0) {
        m_moveToFrontQueue--;
        MoveToFront();
    }

    if (m_moveToBackQueue > 0) {
        m_moveToBackQueue--;
        MoveToBack();
    }

    if (m_castCounter > -1) {
        m_castCounter++;
        if (m_castCounter >= 100 && !m_bInCasting) {
            m_castCounter = -1;
        }
    }

    if (m_attackFrame > -2) {
        m_attackFrame++;
        if (m_attackFrame >= 100) {
            m_attackFrame = -2;
        }
    }

    if (((m_derivedStats.m_generalState & STATE_DEAD) == 0
            && (m_derivedStats.m_generalState & STATE_SLEEPING) == 0)
        || ((g_pChitin->cNetwork.GetSessionOpen() != TRUE || m_curAction.GetActionID() != CAIACTION_LEAVEAREALUA)
            && g_pBaldurChitin->GetObjectGame()->GetRuleTables().m_lInstantActions.Find(m_curAction.GetActionID()) != NULL)) {
        m_typeAI.SetInstance(m_id);
        m_liveTypeAI.SetInstance(m_id);
        m_startTypeAI.SetInstance(m_id);

        if (Orderable(FALSE) && m_curAction.GetActionID() != 0) {
            g_pBaldurChitin->GetScreenWorld()->m_bored = FALSE;
            g_pBaldurChitin->GetScreenWorld()->m_boredCount = 0;
        }

        SHORT actionReturn = ExecuteAction();
        if (m_curAction.GetActionID() != CAIACTION_ATTACK) {
            m_lastActionID = m_curAction.GetActionID();
        }

        if (actionReturn == ACTION_DONE
            || actionReturn == ACTION_ERROR
            || actionReturn == ACTION_STOPPED) {
            SetCurrAction(GetNextAction(m_aiDoAction));
            if (m_curAction.GetActionID() == CAIAction::NO_ACTION) {
                m_curResponseNum = -1;
                m_curResponseSetNum = -1;
                m_curScriptNum = -1;
            }
        } else if (m_interrupt
            && actionReturn == ACTION_INTERRUPTABLE
            && (m_baseStats.m_flags & 0x80000000) == 0) {
            if (m_pPath != NULL) {
                CMessage* message = new CMessageDropPath(m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
            m_actionCount++;
            SetCurrAction(GetNextAction(m_aiDoAction));
            m_interrupt = FALSE;
        } else {
            m_actionCount++;
        }

        if (dropNonInstants
            && !m_sequenceTest
            && m_noActionCount > READY_COUNT
            && m_nSequence != SEQ_TWITCH
            && m_nSequence != SEQ_DIE
            && m_nSequence != SEQ_SLEEP
            && m_nSequence != SEQ_DAMAGE) {
            if (m_pPath != NULL || GetVertListType() == LIST_FLIGHT) {
                if (m_nSequence != SEQ_WALK) {
                    CMessage* message = new CMessageSetSequence(SEQ_WALK, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            } else {
                if (m_nSequence != GetIdleSequence()) {
                    CMessage* message = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()), m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }
        }

        m_nDamageTaken = 0;
        field_9D14 = 0;
        field_9D15 = 0;
    }
}

// 0x72B670
void CGameSprite::ResolvePausedAction(const CAIAction* curAction, POSITION pos)
{
    CGameObject* pObject;
    BYTE rc;

    if (m_groupMove) {
        return;
    }

    ResolveTargetPoint(curAction, pos);

    if (curAction->GetActionID() == 23
        && m_targetId != CGameObjectArray::INVALID_INDEX) {
        if (m_bSelected && Orderable(FALSE)) {
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pObject,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            if (pObject->GetObjectType() == TYPE_SPRITE) {
                static_cast<CGameSprite*>(pObject)->m_marker.SetType(CMarker::ELLIPSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        m_targetId = CGameObjectArray::INVALID_INDEX;
    } else if (curAction->GetActionID() == 27
        && m_targetId != CGameObjectArray::INVALID_INDEX) {
        if (m_bSelected && Orderable(FALSE)) {
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(m_targetId,
                    CGameObjectArray::THREAD_ASYNCH,
                    &pObject,
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            if (pObject->GetObjectType() == TYPE_SPRITE) {
                static_cast<CGameSprite*>(pObject)->m_marker.SetType(CMarker::ELLIPSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        m_targetId = CGameObjectArray::INVALID_INDEX;
    }
}

// NOTE: Assembly is tail-call optimized.
//
// 0x72B870
void CGameSprite::ResolveTargetPoint(const CAIAction* curAction, POSITION pos)
{
    CAIAction* workAction;

    switch (curAction->GetActionID()) {
    case 23:
    case 88:
        if (pos != NULL) {
            workAction = m_queuedActions.GetNext(pos);
            switch (workAction->GetActionID()) {
            case 23:
            case 88:
                ResolveTargetPoint(workAction, pos);
                break;
            default:
                if (!m_groupMove) {
                    m_targetPoint = curAction->m_dest;
                }
                break;
            }
        } else {
            m_targetPoint = curAction->m_dest;
        }
        break;
    case 27:
        m_targetPoint = curAction->m_dest;
        break;
    case 83:
    case 84:
        if (pos != NULL) {
            workAction = m_queuedActions.GetNext(pos);
            ResolveTargetPoint(workAction, pos);
        } else {
            m_targetPoint.x = -1;
            m_targetPoint.y = -1;
        }
        break;
    default:
        m_targetPoint.x = -1;
        m_targetPoint.y = -1;
        break;
    }
}

// 0x733200
//
// Walk the sprite's persistant effects (colour glows, burning death, ...):
// purge entries already marked deleted, tick the rest, purge the ones that
// finish, then hand the regenerated list its tick.
BOOL CGameSprite::ProcessPersistantEffects(LONG deltaT)
{
    POSITION pos = m_persistantEffects.GetHeadPosition();
    while (pos != NULL) {
        POSITION posCurrent = pos;
        CPersistantEffect* pEffect = m_persistantEffects.GetNext(pos);

        if (pEffect->m_deleted) {
            m_persistantEffects.RemoveAt(posCurrent);
            delete pEffect;
            continue;
        }

        pEffect->AIUpdate(this, deltaT);

        if (pEffect->m_deleted || pEffect->m_done) {
            m_persistantEffects.RemoveAt(posCurrent);
            delete pEffect;
        }
    }

    m_derivedStats.m_cRegeneratedPersistantEffectList.AIUpdate(this, deltaT);
    return TRUE;
}

// 0x71C0C0
//
// Per-tick re-derivation of the spellcasting limits, hit points and saving
// throws.  In the original this is the bulk of the work called by
// CGameSprite::ProcessEffectList (0x72DE60); ProcessEffectList is currently a
// partial reconstruction that inlines part of this (the saving-throw block), so
// wiring this in -- and removing that duplicated inline work -- is left as a
// follow-up to avoid applying the derivation twice.
void CGameSprite::sub_71C0C0()
{
    sub_737990();

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    const CRuleTables& ruleTables = pGame->GetRuleTables();

    // Recompute the per-class, per-level known-spell limits.  Each slot's
    // maximum becomes (max + bonus); the currently memorised total is clamped
    // down if it now exceeds that maximum.
    for (UINT nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            BYTE nClass = pGame->GetSpellcasterClass(nClassIndex);
            INT nBonus = 0;
            INT nMaxKnown = ruleTables.GetMaxKnownSpells(nClass, m_startTypeAI, m_derivedStats,
                m_baseStats.m_specialization, nLevel + 1, nBonus);

            CGameSpriteSpellList& list = m_spells.m_spellsByClass[nClassIndex].m_lists[nLevel];
            list.m_nSharedMax = nBonus + nMaxKnown;
            if (list.m_nSharedMax < list.m_nSharedTotal) {
                list.m_nSharedTotal = list.m_nSharedMax;
            }
        }
    }

    // Turn Undead (SPIN970) innate attempts scale with the Charisma modifier
    // (+3 base, floored at 1) plus the Extra Turning feat.  Grow or shrink the
    // granted count to match the recomputed target.
    const CResRef resRefTurnUndead("SPIN970");
    UINT nTurnUndeadID = 0;
    if (pGame->m_innateSpells.Find(resRefTurnUndead, nTurnUndeadID)) {
        for (UINT i = 0; i < m_innateSpells.m_List.size(); i++) {
            if (m_innateSpells.m_List[i].m_nID != nTurnUndeadID) {
                continue;
            }

            INT nTarget = ruleTables.GetAbilityScoreModifier(m_derivedStats.m_nCHR) + 3;
            if (nTarget < 1) {
                nTarget = 1;
            }
            if (HasFeat(CGAMESPRITE_FEAT_EXTRA_TURNING) == 1) {
                nTarget += GetFeatValue(CGAMESPRITE_FEAT_EXTRA_TURNING);
            }

            INT nDelta = nTarget - static_cast<INT>(m_innateSpells.m_List[i].m_nMax);
            if (nDelta >= 1) {
                UINT nID = 0;
                if (pGame->m_innateSpells.Find(resRefTurnUndead, nID)) {
                    m_innateSpells.Add(nID, nDelta, 0, 0);
                }
                if (pGame->m_innateSpells.Find(resRefTurnUndead, nID)) {
                    m_innateSpells.AddToCurrentCount(nID, nDelta, FALSE);
                }
            } else if (nDelta < 0) {
                UINT nID = 0;
                if (pGame->m_innateSpells.Find(resRefTurnUndead, nID)) {
                    UINT nIndex = 0;
                    if (m_innateSpells.Find(nID, nIndex)) {
                        INT nNewCurrent = static_cast<INT>(m_innateSpells.m_List[nIndex].m_nCurrent) + nDelta;
                        if (nNewCurrent < 0) {
                            nNewCurrent = 0;
                        }
                        m_innateSpells.m_nSharedCurrent -= m_innateSpells.m_List[nIndex].m_nCurrent;
                        m_innateSpells.m_List[nIndex].m_nCurrent = nNewCurrent;
                        m_innateSpells.m_nSharedCurrent += nNewCurrent;
                    }
                }
                if (pGame->m_innateSpells.Find(resRefTurnUndead, nID)) {
                    m_innateSpells.Remove(nID, FALSE, -nDelta, 0);
                }
            }
            break;
        }
    }

    // Hit points: re-derive the Constitution bonus and apply the change to the
    // current HP, the derived maximum and the creature-file base maximum.  A
    // creature in a death state has its current HP zeroed when the bonus rises
    // and floored at 1 when it falls.
    const DWORD nDeathStates = STATE_DEAD | STATE_STONE_DEATH | STATE_FROZEN_DEATH;
    INT nHPCONBonus = ruleTables.GetHPCONBonusTotal(m_typeAI, m_derivedStats,
        static_cast<BYTE>(m_derivedStats.m_nCON));
    SHORT nHPDelta = static_cast<SHORT>(nHPCONBonus) - static_cast<SHORT>(m_nHPCONBonusTotalOld);
    if (m_nHPCONBonusTotalOld < nHPCONBonus) {
        if ((m_derivedStats.m_generalState & nDeathStates) == 0) {
            m_baseStats.m_hitPoints += nHPDelta;
        } else {
            m_baseStats.m_hitPoints = 0;
        }
        m_derivedStats.m_nMaxHitPoints += nHPDelta;
        m_baseStats.m_maxHitPointsBase += nHPDelta;
    } else if (nHPCONBonus < m_nHPCONBonusTotalOld) {
        if ((m_derivedStats.m_generalState & nDeathStates) == 0) {
            m_baseStats.m_hitPoints += nHPDelta;
            if (m_baseStats.m_hitPoints < 1) {
                m_baseStats.m_hitPoints = 1;
            }
        }
        m_derivedStats.m_nMaxHitPoints += nHPDelta;
        m_baseStats.m_maxHitPointsBase += nHPDelta;
    }

    // The maximum HP never drops below the character level.
    WORD wMaxHP = static_cast<BYTE>(m_derivedStats.m_nLevel);
    if (static_cast<SHORT>(static_cast<BYTE>(m_derivedStats.m_nLevel)) < m_derivedStats.m_nMaxHitPoints) {
        wMaxHP = m_derivedStats.m_nMaxHitPoints;
    }
    m_derivedStats.m_nMaxHitPoints = wMaxHP;

    // Saving throws gain the relevant ability modifier; a paladin's positive
    // Charisma modifier adds to all three.
    m_derivedStats.m_nSaveVSFortitude += ruleTables.GetAbilityScoreModifier(m_derivedStats.m_nCON);
    m_derivedStats.m_nSaveVSReflex += ruleTables.GetAbilityScoreModifier(m_derivedStats.m_nDEX);
    m_derivedStats.m_nSaveVSWill += ruleTables.GetAbilityScoreModifier(m_derivedStats.m_nWIS);
    if (m_typeAI.IsClassValid(CAIOBJECTTYPE_C_PALADIN)) {
        INT nCharismaBonus = ruleTables.GetAbilityScoreModifier(m_derivedStats.m_nCHR);
        if (nCharismaBonus > 0) {
            m_derivedStats.m_nSaveVSFortitude += static_cast<SHORT>(nCharismaBonus);
            m_derivedStats.m_nSaveVSReflex += static_cast<SHORT>(nCharismaBonus);
            m_derivedStats.m_nSaveVSWill += static_cast<SHORT>(nCharismaBonus);
        }
    }

    m_nHPCONBonusTotalOld = nHPCONBonus;
    m_bHPCONBonusTotalUpdate = FALSE;

    if (m_derivedStats.m_nMaxHitPoints < m_baseStats.m_hitPoints) {
        m_baseStats.m_hitPoints = m_derivedStats.m_nMaxHitPoints;
    }
    if (field_7110 < static_cast<INT>(static_cast<SHORT>(m_baseStats.m_hitPoints))) {
        field_7110 = static_cast<SHORT>(m_baseStats.m_hitPoints);
    }

    // Party members: clamp experience to the cap and announce a level-up.
    if (pGame->GetCharacterPortraitNum(m_id) != -1) {
        CDerivedStats nextLevelStats;

        DWORD nXPCap = ruleTables.GetXPCap();
        if (nXPCap < m_derivedStats.m_nXP) {
            m_derivedStats.m_nXP = nXPCap;
        }

        nextLevelStats = m_derivedStats;
        ruleTables.GetNextLevel(m_startTypeAI.m_nClass, nextLevelStats, this);

        BOOL bLevelChanged = nextLevelStats.m_nLevel != m_derivedStats.m_nLevel;
        // The original additionally requires single-player, or that this sprite
        // belongs to the local player (the g_pBaldurChitin multiplayer flag /
        // local-player id compared against m_remotePlayerID); that sub-clause is
        // deferred with the rest of the multiplayer path.
        if (bLevelChanged && !m_bLevelUp) {
            FeedBack(FEEDBACK_LEVELUP, 0, 0, 0, -1, 0, 0);
        }
        m_bLevelUp = bLevelChanged;
    }

    sub_71CC90();
}

// 0x737990
//
// Reputation-driven loss of paladin or ranger status.  When a creature that can
// fall from grace (creature flag 0x800) drops below the reputation threshold
// (70), its class collapses into Fighter: the granted innate abilities are
// stripped, the class levels are folded into Fighter, the experience is reset
// to the start of the current level and the improved saving throws are added to
// the base saves.  Shared with CInfGame::ReputationAdjustment.
void CGameSprite::sub_737990()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    UTIL_ASSERT(pGame != NULL);

    const CRuleTables& ruleTables = pGame->GetRuleTables();

    if ((m_baseStats.m_flags & 0x800) == 0) {
        return;
    }

    INT nOldFortitude = ruleTables.GetSavingThrow(m_startTypeAI, m_derivedStats, m_baseStats.m_CONBase, CRuleTables::FORTITUDE);
    INT nOldReflex = ruleTables.GetSavingThrow(m_startTypeAI, m_derivedStats, m_baseStats.m_DEXBase, CRuleTables::REFLEX);
    INT nOldWill = ruleTables.GetSavingThrow(m_startTypeAI, m_derivedStats, m_baseStats.m_WISBase, CRuleTables::WILL);

    BOOL bFell = FALSE;

    // Paladin (class-mask bit 0x40) falls when reputation drops below 70.
    if ((m_typeAI.m_nClassMask & 0x40) != 0 && m_derivedStats.m_nReputation < 70) {
        DWORD nXP = ruleTables.GetNextLevelXP(static_cast<BYTE>(GetEffectiveCharacterLevel()));
        m_baseStats.m_xp = nXP;
        m_derivedStats.m_nXP = nXP;

        CResRef resRef;
        resRef = "SPIN109"; RemoveInnateSpell(resRef, 1, 0, 0); // Lay on Hands
        resRef = "SPIN120"; RemoveInnateSpell(resRef, 1, 0, 0); // Detect Evil
        resRef = "SPIN121"; RemoveInnateSpell(resRef, 1, 0, 0); // Protection From Evil
        resRef = "SPIN125"; RemoveInnateSpell(resRef, 1, 0, 0); // Remove Disease
        resRef = "SPIN152"; RemoveInnateSpell(resRef, 1, 0, 0); // Smite Evil

        INT nPaladinLevel = m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_PALADIN);
        m_derivedStats.SetClassLevel(CAIOBJECTTYPE_C_PALADIN, 0);
        m_derivedStats.SetClassLevel(CAIOBJECTTYPE_C_FIGHTER,
            nPaladinLevel + m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_FIGHTER));

        m_typeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
        m_liveTypeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
        m_startTypeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
        m_typeAI.m_nClassMask = m_derivedStats.m_classMask;
        m_liveTypeAI.m_nClassMask = m_derivedStats.m_classMask;
        m_startTypeAI.m_nClassMask = m_derivedStats.m_classMask;

        FeedBack(FEEDBACK_16, 0, 0, 0, 0x4CA4, 0, 0); // "fallen paladin" message
        m_baseStats.m_flags |= 0x200;
        bFell = TRUE;
    }

    // Ranger (class-mask bit 0x80) collapses the same way; its levels are folded
    // through the creature-file level bytes rather than the derived class table.
    if (m_derivedStats.HasClassMask(0x80) && m_derivedStats.m_nReputation < 70) {
        DWORD nXP = ruleTables.GetNextLevelXP(static_cast<BYTE>(GetEffectiveCharacterLevel()));
        m_baseStats.m_xp = nXP;
        m_derivedStats.m_nXP = nXP;

        CResRef resRef;
        resRef = "SPIN108"; RemoveInnateSpell(resRef, 1, 0, 0); // Charm Animal
        resRef = "SPIN139"; RemoveInnateSpell(resRef, 1, 0, 0); // Tracking

        BYTE nRangerLevel = m_baseStats.m_rangerLevel;
        m_baseStats.m_characterLevel -= nRangerLevel;
        m_baseStats.m_rangerLevel = 0;
        BYTE nFighterLevel = nRangerLevel + m_baseStats.m_fighterLevel;
        m_baseStats.m_characterLevel += (nFighterLevel - m_baseStats.m_fighterLevel);
        m_baseStats.m_fighterLevel = nFighterLevel;

        m_typeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
        m_liveTypeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
        m_startTypeAI.m_nClass = CAIOBJECTTYPE_C_FIGHTER;
        m_typeAI.m_nClassMask = m_derivedStats.m_classMask;
        m_liveTypeAI.m_nClassMask = m_derivedStats.m_classMask;
        m_startTypeAI.m_nClassMask = m_derivedStats.m_classMask;

        FeedBack(FEEDBACK_16, 0, 0, 0, 0x4CA5, 0, 0); // "fallen ranger" message
        m_baseStats.m_flags |= 0x400;
    } else if (!bFell) {
        return;
    }

    // Re-derive the saving throws as a Fighter and credit any improvement to the
    // base saves (a drop is ignored).
    INT nNewFortitude = ruleTables.GetSavingThrow(m_startTypeAI, m_derivedStats, m_baseStats.m_CONBase, CRuleTables::FORTITUDE);
    INT nNewReflex = ruleTables.GetSavingThrow(m_startTypeAI, m_derivedStats, m_baseStats.m_DEXBase, CRuleTables::REFLEX);
    INT nNewWill = ruleTables.GetSavingThrow(m_startTypeAI, m_derivedStats, m_baseStats.m_WISBase, CRuleTables::WILL);
    m_baseStats.m_saveVSFortitudeBase += static_cast<BYTE>(nNewFortitude - nOldFortitude < 0 ? 0 : nNewFortitude - nOldFortitude);
    m_baseStats.m_saveVSReflexBase += static_cast<BYTE>(nNewReflex - nOldReflex < 0 ? 0 : nNewReflex - nOldReflex);
    m_baseStats.m_saveVSWillBase += static_cast<BYTE>(nNewWill - nOldWill < 0 ? 0 : nNewWill - nOldWill);

    field_562C = 1;
    ProcessEffectList();

    // The original then notifies the engine of the class change and, in a
    // multiplayer session, broadcasts the updated character slot to the other
    // clients (a g_pBaldurChitin sub-object virtual call followed by an
    // m_characters[] scan and CBaldurMessage::BroadcastUpdateCharacterSlot).
    // That engine/multiplayer tail is deferred with the rest of the MP path.
}

// 0x71CC90
//
// Refresh the weapon-proficiency contribution after the equipped weapons may
// have changed: re-apply the proficiency bonus for the main hand and off hand,
// then (only when the main weapon just changed) add the fighting-style feat
// bonuses.  The style-feat query (sub_75E930) is stubbed to zero in the shipped
// binary, so those bonus branches never actually fire.
void CGameSprite::sub_71CC90()
{
    INT nOffhandSlot = CGameSpriteEquipment::SLOT_WEAPON + 2 * m_nWeaponSet + 1;
    CItem* pMainWeapon = m_equipment.m_items[m_equipment.m_selectedWeapon];
    CItem* pOffhand = m_equipment.m_items[nOffhandSlot];

    // A shield or armour in the off hand is not a second weapon.
    if (m_equipment.m_selectedWeapon == 0x2A
        || (pOffhand != NULL
            && (pOffhand->GetItemType() == 0x2F || pOffhand->GetItemType() == 0x35
                || pOffhand->GetItemType() == 0x31 || pOffhand->GetItemType() == 0x29))) {
        pOffhand = NULL;
    }

    if (pMainWeapon == NULL) {
        SelectWeaponAbility(CGameSpriteEquipment::SLOT_FIST, 0, 0, 1);
        pMainWeapon = m_equipment.m_items[m_equipment.m_selectedWeapon];
    }

    sub_71CEA0(pMainWeapon, 0);
    sub_71CEA0(pOffhand, 1);

    BOOL bWeaponChanged = pMainWeapon != m_equipment.m_items[CGameSpriteEquipment::SLOT_FIST];

    pMainWeapon->Demand();
    ITEM_ABILITY* pAbility = pMainWeapon->GetAbility(m_equipment.m_selectedWeaponAbility);
    if (pAbility != NULL && static_cast<BYTE>(pAbility->type) != 2) {
        if ((pMainWeapon->GetFlagsFile() & 2) == 0) {
            if (pOffhand == NULL) {
                if (m_equipment.m_items[nOffhandSlot] == NULL) {
                    if (bWeaponChanged) {
                        INT nRank = sub_75E930(CGAMESPRITE_FEAT_DODGE);
                        if (nRank == 1) {
                            m_derivedStats.m_nACArmorBonus += 1;
                            m_derivedStats.m_nCriticalHitBonus += 1;
                        } else if (nRank == 2) {
                            m_derivedStats.m_nACArmorBonus += 2;
                            m_derivedStats.m_nCriticalHitBonus += 1;
                        }
                    }
                } else if (bWeaponChanged) {
                    INT nRank = sub_75E930(CGAMESPRITE_FEAT_DISCIPLINE);
                    if (nRank == 1) {
                        m_derivedStats.m_nACMissileMod -= 2;
                    } else if (nRank == 2) {
                        m_derivedStats.m_nACMissileMod -= 4;
                    }
                }
            }
        } else if (bWeaponChanged) {
            INT nRank = sub_75E930(CGAMESPRITE_FEAT_DIRTY_FIGHTING);
            if (nRank == 1) {
                m_derivedStats.m_nPhysicalSpeed += 2;
                m_derivedStats.m_nCriticalHitBonus += 1;
                m_derivedStats.m_DamageBonusRight += 1;
            } else if (nRank == 2) {
                m_derivedStats.m_nPhysicalSpeed += 4;
                m_derivedStats.m_nCriticalHitBonus += 1;
                m_derivedStats.m_DamageBonusRight += 1;
            }
        }
    }
    pMainWeapon->Release();
}

// 0x71CEA0
//
// Apply the weapon-specialization damage bonus for one hand.  The proficiency
// level of the wielded weapon (taking the better of the weapon and its launcher
// for ammunition) indexes the WEAPSPEC table; the resulting bonus is added to
// the main-hand (a2 == 0) or off-hand (a2 != 0) damage bonus.
void CGameSprite::sub_71CEA0(CItem* pItem, int a2)
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    SHORT nDamageBonus = 0;

    if (pItem == NULL) {
        return;
    }

    pItem->Demand();
    ITEM_ABILITY* pAbility = pItem->GetAbility(m_equipment.m_selectedWeaponAbility);
    if (pAbility != NULL) {
        SHORT nLauncherSlot;
        CItem* pLauncher = GetLauncher(pAbility, nLauncherSlot);
        if (pLauncher != NULL) {
            pLauncher->Demand();
            pLauncher->GetAbility(0);
        }

        SHORT nProficiency = GetProficiencyTHAC0Bonus(pItem);
        if (pLauncher != NULL) {
            SHORT nLauncherProficiency = GetProficiencyTHAC0Bonus(pLauncher);
            if (nProficiency < nLauncherProficiency) {
                nProficiency = GetProficiencyTHAC0Bonus(pLauncher);
            }
            pLauncher->Release();
        }

        if (pItem->GetItemType() == 0x1C && nProficiency < 2) {
            nProficiency = 1;
        }
        if (nProficiency >= 1) {
            nDamageBonus = static_cast<SHORT>(
                atol(pGame->GetRuleTables().m_tWeaponSpecialization.GetAt(CPoint(1, nProficiency))));
        }
    }

    pItem->Release();
    if (a2 != 0) {
        m_derivedStats.m_DamageBonusLeft += nDamageBonus;
    } else {
        m_derivedStats.m_DamageBonusRight += nDamageBonus;
    }
}

// 0x75E930
int CGameSprite::sub_75E930(int a1)
{
    // The shipped binary stubs this fighting-style feat query to zero
    // (xor eax, eax; ret 4), which disables the style-bonus branches above.
    return 0;
}

// 0x72DE60
BOOL CGameSprite::ProcessEffectList()
{
    // This function is huge.  Recovered: the multiplayer-ownership head gate
    // (0x72DE7C, below), the re-entrancy / deferred-tick gate, the staggered head
    // pre-pass (regeneration timestamp, per-tick random scratch and the
    // encumbrance-change detection that feeds the body guard), effect handling,
    // the per-tick derived-stat re-derivation (0x71C0C0), the body-entry
    // move-scale re-derivation (0x72E221 -- barbarian/monk/Dash class movement
    // bonuses), the encumbrance check (0x72F2B3), the animation colour/palette
    // refresh (0x72E3A8), the shapeshift / animation-rebuild block (0x72E437), the
    // persistant effect tick and the movement-interpolation re-pace (0x72F476),
    // the post-HandleEffects visual-refresh flag (0x72E770), the forced-action
    // override (berserk 0x72E759, panic 0x72EC32, null-action clear 0x72EC25,
    // panic-exit idle 0x72EA81), the party-ally membership re-derivation
    // (0x72EC3A), the helpless animation gate (0x72EFF2), the per-tick
    // intoxication-decay and fatigue/morale luck penalties (0x72F07A), the
    // death-state colour re-push (0x72F72B), the multiplayer-peer state broadcast
    // (0x72F8EC) and the end-of-tick UI/state snapshot (0x72FB24).

    // 0x72DE7C: multiplayer-ownership head gate.  In a network session a client
    // only ticks the effect list of the sprites it owns; one owned by a different
    // player is left to its owning client.  With no service provider (single
    // player) the test is transparent -- the sprite is always processed.
    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != m_remotePlayerID) {
        return FALSE;
    }

    BOOL bResult = TRUE;

    // 0x72DEC4: re-entrancy / deferred-tick guard.  m_bAllowEffectListCall is set
    // TRUE once at construction (0x6F06D3) and re-armed at the tail of every pass
    // (0x72FB9D), so in steady state it is TRUE on entry and the body below runs
    // exactly once per tick (field_72A2 stays 0).  It is cleared only for the
    // duration of a pass; a call that re-enters while a pass is in flight is
    // deferred -- it bumps the catch-up counter field_72A2 and returns, and the
    // in-flight pass drains those deferred ticks (re-processing the body) before
    // it re-arms.  While the flag is clear, external stat readers (GetActiveStats
    // and the m_bAllowEffectListCall ? m_derivedStats : m_tempStats sites) fall
    // back to m_tempStats, the stable base, rather than the mid-rebuild
    // m_derivedStats.
    if (!m_bAllowEffectListCall) {
        field_72A2++;
        return TRUE;
    }

    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    do {
        // 0x72DEF7: enter the pass -- suppress re-entrant processing and clamp
        // the deferred-tick counter to 5.
        m_bAllowEffectListCall = FALSE;
        if (field_72A2 > 4) {
            field_72A2 = 5;
        }

        // 0x72DF1D: head pre-pass.  On every 16th AI frame -- staggered per
        // sprite id off field_44A, the same phase the caller (0x72DA32) uses to
        // tick CheckTimers -- or whenever a derived-stat (field_562C) or
        // HP/Constitution recompute is already pending, refresh the regeneration
        // timestamp and re-roll the per-tick random scratch bytes
        // (field_70F6..field_70FA, which effect application consumes), then, for a
        // party PC, detect whether the carried weight implies a walk-scale change.
        // That detection feeds the guard below so a pure carried-weight change
        // (no pending effect) still refreshes the walk scale on its next staggered
        // frame rather than waiting for the next body run.
        BOOL bEncumbranceChanged = FALSE;
        if (((m_id ^ field_44A) & 0xF) == 0 || field_562C != 0 || m_bHPCONBonusTotalUpdate != 0) {
            m_lastRegenerationTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
            field_70F6 = rand() % 20 + 1;
            field_70F7 = rand() % 20 + 1;
            field_70F8 = rand() % 20 + 1;
            field_70F9 = rand() % 20 + 1;
            field_70FA = rand() % 100;

            if (pGame->GetCharacterPortraitNum(m_id) != -1) {
                INT nWeight = static_cast<INT>(GetCarriedWeight());
                INT nMaxWeight = static_cast<INT>(
                    static_cast<float>(ruleTables.GetEncumbranceMod(this)) / 100.0f
                    * static_cast<float>(atol(ruleTables.m_tStrengthMod.GetAt(CPoint(3, m_derivedStats.m_nSTR)))));

                // The body's encumbrance block (0x72F2B3) sets the move scale to 0
                // over 120% of capacity, to half the default over 100%, and to the
                // default otherwise; here detect whether that target differs from
                // the animation's current scale.
                if (nWeight > nMaxWeight * 120 / 100) {
                    bEncumbranceChanged = (m_animation.m_animation->GetMoveScale() != 0);
                } else if (nWeight > nMaxWeight) {
                    bEncumbranceChanged = (m_animation.m_animation->GetMoveScale()
                        != (m_animation.m_animation->GetMoveScaleDefault() >> 1));
                } else {
                    bEncumbranceChanged = (m_animation.m_animation->GetMoveScale()
                        < m_animation.m_animation->GetMoveScaleDefault());
                }
            }
        }

        // 0x72E1AE: the pre-pass guard.  The binary runs the whole re-derivation
        // body only when there is work to do; otherwise it skips straight to the
        // tail (0x72FB24).  Without this guard the body -- HandleEffects and the
        // heavy per-tick re-derivation sub_71C0C0 -- ran on every sprite every
        // tick (~52 sprites in a populated area), which dropped the frame rate to
        // a crawl.  The original enters the body when either effect list has a
        // pending effect, the field_562C dirty flag is set, the carried weight
        // implies a walk-scale change, or the HP/Constitution-bonus recompute is
        // pending.
        BOOL bEffectsPending = m_timedEffectList.CheckEffects()
            || m_equipedEffectList.CheckEffects()
            || field_562C != 0
            || bEncumbranceChanged
            || m_bHPCONBonusTotalUpdate != 0;
        if (bEffectsPending) {
            // 0x72E1E2: snapshot the live derived stats into the base copy and
            // clear the dirty flag.  While m_bAllowEffectListCall is clear (set
            // above) external readers see this stable m_tempStats snapshot rather
            // than the m_derivedStats the body is about to rebuild.
            m_tempStats = m_derivedStats;
            field_562C = 0;

            // 0x72E221: cache the animation's current move scale and the live
            // general state, then reset the move scale to the animation's default
            // before re-deriving the per-tick class movement bonuses below.
            field_72EA = static_cast<WORD>(m_animation.m_animation->GetMoveScale());
            field_72EC = m_derivedStats.m_generalState;
            m_animation.m_animation->ResetMoveScale();

            // 0x72E29C: while the sprite is moving (non-zero scale), re-derive the
            // move scale from class movement bonuses -- a barbarian out of heavy
            // armour moves at 1.2x (0x84EB58), a monk out of body armour adds its
            // monk level / 3, and the Dash feat adds one.
            if (m_animation.m_animation->GetMoveScale() != 0) {
                INT nScale = m_animation.m_animation->GetMoveScale();
                if (m_typeAI.IsClassValid(CAIOBJECTTYPE_C_BARBARIAN) && !HasArmorType(3)) {
                    nScale = static_cast<INT>(static_cast<float>(nScale) * 1.2f);
                }
                if (GetAIType().IsClassValid(CAIOBJECTTYPE_C_MONK)
                    && (m_equipment.m_items[CGameSpriteEquipment::SLOT_ARMOR] == NULL || HasArmorType(0))) {
                    nScale += m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_MONK) / 3;
                }
                if (HasFeat(12)) {   // FEAT_DASH
                    nScale += 1;
                }
                m_animation.m_animation->SetMoveScale(nScale);
            }

            // 0x72E3A8: re-apply the creature's base colour ranges and clear any
            // per-frame colour effects so HandleEffects below rebuilds the tint
            // from the current effect list.  m_hasColorRangeEffects re-pushes the
            // seven CRE colour indices; m_hasColorEffects clears the colour effects.
            if (m_hasColorRangeEffects) {
                for (INT i = 0; i < 7; i++) {
                    m_animation.m_animation->SetColorRange(static_cast<BYTE>(i), m_baseStats.m_colors[i]);
                }
            }
            if (m_hasColorEffects) {
                m_animation.m_animation->ClearColorEffectsAll();
            }

            // 0x72E437: an effect requested an animation change (a polymorph /
            // shape-change).  Re-build the animation object in place, but only
            // while the sprite is live on the search map -- a stored or dead
            // sprite has no footprint to migrate.  The whole shape swap is one
            // unit: un-equip first so the old item-based appearance is gone,
            // de-register the OLD footprint, rebuild the animation, re-register
            // the NEW footprint, then re-equip and re-issue the sequence.
            if (m_bOnSearchMap && field_5640) {
                WORD savedSequence = static_cast<WORD>(m_nSequence);
                UnequipAll(1);

                // De-register the old footprint on the area's search bitmap
                // before the animation (and thus its personal-space size)
                // changes.  List type 2 animations carry no search footprint,
                // so they are left alone.
                if (m_pArea != NULL && m_animation.GetListType() != 2) {
                    m_pArea->m_search.RemoveObject(
                        CPoint(GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                               GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        GetAIType().GetEnemyAlly(),
                        m_animation.GetPersonalSpace(),
                        m_bBumpable,
                        m_bOnSearchMap);
                }

                m_animation.SetAnimationType(m_baseStats.m_animationType, m_baseStats.m_colors, m_nDirection);

                // Re-register at the new footprint.
                if (m_pArea != NULL && m_animation.GetListType() != 2) {
                    m_pArea->m_search.AddObject(
                        CPoint(GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                               GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        GetAIType().GetEnemyAlly(),
                        m_animation.GetPersonalSpace(),
                        m_bBumpable,
                        m_bOnSearchMap);
                }

                // Bounce the sequence through READY so the freshly built
                // animation re-initialises, then restore the sprite's prior one.
                SetSequence(CGAMESPRITE_SEQ_READY);
                SetSequence(static_cast<BYTE>(savedSequence));
                EquipAll(1);

                CMessage* message = new CMessageAnimationChange(m_baseStats.m_animationType, m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            // 0x72E6C6: tell the renderer to drop any per-frame colour effects
            // when the colour or animation state changed this tick.
            if (m_hasColorEffects || field_5640) {
                CMessage* message = new CMessageColorReset(m_id, m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            // 0x72E70E: consume the colour / animation change flags now that the
            // refresh has been applied, drop the portrait-icon cache and refresh
            // the portrait so the new appearance shows.
            m_hasColorRangeEffects = FALSE;
            m_hasColorEffects = FALSE;
            field_5640 = 0;
            m_portraitIcons.RemoveAll();
            pGame->UpdatePortrait(static_cast<SHORT>(pGame->GetCharacterPortraitNum(m_id)), 1);

            bResult = HandleEffects();

            // 0x72E770: force the visual-effect system to re-push this tick.
            m_bForceVisualEffects = TRUE;

            // 0x72E759: while berserk, force the Berserk() action.  When the
            // sprite is in STATE_BERSERK with the berserk timer live (m_berserkActive)
            // and it is neither attacking nor already berserking, drop its current
            // actions and path and push an empty-target Berserk() (ACTION.IDS 124).
            if ((m_derivedStats.m_generalState & STATE_BERSERK)
                && m_berserkActive
                && m_curAction.m_actionID != CAIAction::ATTACK
                && m_curAction.m_actionID != 124 /* Berserk() */) {
                CAIAction action(124 /* Berserk() */,
                    CAIObjectType(0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0),
                    CGameObjectArray::INVALID_INDEX, 0, 0);
                ClearActions(FALSE);
                DropPath();
                m_interrupt = TRUE;
                AddAction(action);
            }

            // 0x72EC32: while panicking, force the Panic() action -- clear the
            // sprite's actions and push an empty-target Panic() (ACTION.IDS 24).
            if (m_derivedStats.m_generalState & STATE_PANIC) {
                CAIAction action(24 /* Panic() */,
                    CAIObjectType(0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0), 0, 0, 0);
                ClearActions(FALSE);
                m_interrupt = TRUE;
                AddAction(action);
            } else {
                // 0x72EA81: panic-exit idle.  When the sprite was panicking last
                // tick (the cached general state field_72EC & STATE_PANIC) and its
                // morale has not failed, it has just calmed down: drop its path and
                // fall back to the next queued action.  A network owner also flags
                // a sprite-state broadcast.
                if ((field_72EC & STATE_PANIC) && !m_moraleFailure) {
                    if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                        || g_pChitin->cNetwork.m_idLocalPlayer == m_remotePlayerID) {
                        m_bSendSpriteUpdate = TRUE;
                    }
                    ClearActions(FALSE);
                    CMessage* message = new CMessageDropPath(m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    CAIAction action;
                    SetCurrAction(GetNextAction(action));
                }

                // 0x72EC25: once a sprite is no longer panicking, clear a lingering
                // Berserk() / Panic() forced action back to the null action (the
                // global CAIAction::NULL_ACTION at 0x8C6118).
                if (m_curAction.m_actionID == 124 /* Berserk() */
                    || m_curAction.m_actionID == 24 /* Panic() */) {
                    CAIAction action(CAIAction::NULL_ACTION);
                    ClearActions(FALSE);
                    m_interrupt = TRUE;
                    AddAction(action);
                }
            }

            // 0x72EC3A: re-derive party-ally membership from the allegiance.  A
            // controlled sprite (or one at EA 7) that the game is not already
            // tracking as an ally or a familiar, and that holds no portrait slot,
            // is enrolled in the ally list; a sprite at or past GOODCUTOFF is
            // dropped from it.
            BYTE nEnemyAlly = GetAIType().GetEnemyAlly();
            if (nEnemyAlly == CAIObjectType::EA_0x847C3A
                || nEnemyAlly == CAIObjectType::EA_CONTROLLED) {
                if (pGame->m_allies.Find(reinterpret_cast<int*>(m_id), NULL) == NULL
                    && pGame->m_familiars.Find(reinterpret_cast<int*>(m_id), NULL) == NULL
                    && pGame->GetCharacterPortraitNum(m_id) == -1) {
                    pGame->AddCharacterToAllies(m_id);
                }
            } else if (nEnemyAlly >= CAIObjectType::EA_GOODCUTOFF) {
                pGame->RemoveCharacterFromAllies(m_id);
            }

            // 0x72EFF2: animation-active gate.  An alive, un-held sprite that a
            // time stop does not freeze out keeps its animation marked live; a
            // helpless, caster-held or timed-out sprite has its movement animation
            // frozen instead.
            if (!IsHelpless()
                && !m_derivedStats.m_bCasterHold
                && (pGame->m_nTimeStop == 0 || pGame->m_nTimeStopCaster == m_id)) {
                // 0x72F020: keep the animation live unless the sprite is a corpse
                // (dead / petrified / frozen) that is no longer mid-death -- the
                // DIE sequence still plays the death animation out.
                if ((m_derivedStats.m_generalState
                        & (STATE_DEAD | STATE_STONE_DEATH | STATE_FROZEN_DEATH)) == 0
                    || m_nSequence == 5) {   // 5 = SEQ_DIE (SEQUENCE.IDS)
                    m_animationRunning = TRUE;
                }
            } else {
                // 0x72F048: freeze the movement animation and drop the live flag.
                UTIL_ASSERT(m_animation.m_animation != NULL);
                m_animation.m_animation->SetMoveScale(0);
                m_animationRunning = FALSE;
            }

            // 0x72F07A: per-tick intoxication and fatigue penalties, applied just
            // before the derived-stat re-derivation.  While the derived
            // intoxication level is positive the stored intoxication decays toward
            // sobriety on a staggered schedule; intoxication and fatigue then each
            // index a rule table to apply a luck penalty and raise the matching
            // status portrait icon, and intoxication additionally shifts the base
            // morale.
            if (m_derivedStats.m_nIntoxication > 0) {
                // 0x72F087: once every (0x84EC0C * 0x84EC0D * 0x84EC0E) = 15*60*5
                // AI frames, staggered per sprite by field_44A, recover the stored
                // intoxication toward zero by the Constitution-based recovery rate
                // from GetIntoxicationInfo.
                if (field_44A % (15 * 60 * 5) == 0 && m_baseStats.m_intoxication != 0) {
                    INT nIntoxRate, nRecoveryRate, nIntoxCap;
                    ruleTables.GetIntoxicationInfo(static_cast<BYTE>(m_derivedStats.m_nCON),
                                                   nIntoxRate, nRecoveryRate, nIntoxCap);
                    INT nNewIntox = m_baseStats.m_intoxication - nRecoveryRate;
                    m_baseStats.m_intoxication = static_cast<BYTE>(nNewIntox < 1 ? 0 : nNewIntox);
                }

                // 0x72F102: INTOXICATION_MODIFIERS luck penalty, indexed by the
                // derived intoxication level; a non-zero penalty also raises the
                // intoxicated status portrait icon (5).
                char cIntoxLuck = static_cast<char>(
                    atol(ruleTables.m_tIntoxicationMod.GetAt(CPoint(0, m_derivedStats.m_nIntoxication))));
                m_derivedStats.m_nLuck += static_cast<SHORT>(cIntoxLuck);
                if (cIntoxLuck != 0) {
                    AddPortraitIcon(5);
                }

                // 0x72F179: the same INTOXICATION_MODIFIERS entry shifts the base
                // morale, clamped to the 0..20 morale range.
                char cIntoxMorale = static_cast<char>(
                    atol(ruleTables.m_tIntoxicationMod.GetAt(CPoint(0, m_derivedStats.m_nIntoxication))));
                BYTE nNewMorale = static_cast<BYTE>(m_baseStats.m_morale + cIntoxMorale);
                if (nNewMorale > 0x13) {
                    nNewMorale = 0x14;
                }
                m_baseStats.m_morale = nNewMorale;
            }

            // 0x72F1FF: FATIGUE_MODIFIERS luck penalty, indexed by the derived
            // fatigue level; a non-zero penalty raises the fatigued status portrait
            // icon (0x27).  Applied even when the intoxication level is zero.
            char cFatigueLuck = static_cast<char>(
                atol(ruleTables.m_tFatigueMod.GetAt(CPoint(0, m_derivedStats.m_nFatigue))));
            m_derivedStats.m_nLuck += static_cast<SHORT>(cFatigueLuck);
            if (cFatigueLuck != 0) {
                AddPortraitIcon(0x27);
            }

            // 0x71C0C0: the per-tick re-derivation the binary performs here -- the
            // saving throws, hit-point/Constitution bonus and per-class spellcasting
            // limits.  This supersedes the saving-throw block that was previously
            // inlined above (the binary applies it only inside this call).  It reads
            // m_derivedStats directly (it does not consult m_bAllowEffectListCall),
            // so the cleared flag above does not perturb the re-derivation itself.
            sub_71C0C0();

            m_derivedStats.CheckLimits();

            // 0x72F2B3: per-tick encumbrance for party PCs.  Compare the carried
            // weight to the STR-based maximum load (STRENGTH_MODIFIERS column 3,
            // WEIGHT_ALLOWANCE, scaled by the encumbrance modifier percent).  Over
            // 120% of capacity the sprite cannot move (m_nEncumberance = 2, walk
            // scale 0); over capacity it is slowed to half (m_nEncumberance = 1, walk
            // scale >> 1); otherwise it walks normally.  The 0x142 game mode leaves
            // the walk scale untouched.
            if (pGame->GetCharacterPortraitNum(m_id) != -1) {
                INT nWeight = static_cast<INT>(GetCarriedWeight());
                INT nMaxWeight = static_cast<INT>(
                    static_cast<float>(ruleTables.GetEncumbranceMod(this)) / 100.0f
                    * static_cast<float>(atol(ruleTables.m_tStrengthMod.GetAt(CPoint(3, m_derivedStats.m_nSTR)))));

                // The walk scale is recomputed from the animation's default each
                // tick.  In the original this is split across the (here deferred)
                // per-tick refresh -- which calls ResetMoveScale (m_moveScaleCurrent
                // = base) when the encumbrance state changes -- and this block, which
                // then halves the freshly-reset scale.  Collapsed here to an
                // idempotent set off the default so the half-speed slow does not
                // compound to zero every tick.
                if (nWeight > nMaxWeight * 120 / 100) {
                    if (pGame->m_gameSave.m_mode != 0x142) {
                        m_animation.m_animation->SetMoveScale(0);
                        m_derivedStats.m_nEncumberance = 2;
                    }
                } else if (nWeight > nMaxWeight) {
                    if (pGame->m_gameSave.m_mode != 0x142) {
                        m_animation.m_animation->SetMoveScale(m_animation.m_animation->GetMoveScaleDefault() >> 1);
                        m_derivedStats.m_nEncumberance = 1;
                    }
                } else {
                    if (m_animation.m_animation->GetMoveScale() < m_animation.m_animation->GetMoveScaleDefault()) {
                        m_animation.m_animation->ResetMoveScale();
                    }
                    m_derivedStats.m_nEncumberance = 0;
                }
            }

            // 0x72F476: re-pace the in-flight walk.  When this tick changed the
            // animation's move scale (the encumbrance / class-movement logic
            // above) while the sprite is still following a path, rebuild
            // m_posDelta -- the fixed-point (<< EXACT_SCALE) sub-pixel step
            // m_posExact accrues each frame as the sprite slides from m_pos
            // toward m_posDest -- so the remainder of the slide runs at the new
            // speed.  This mirrors the per-segment delta setup in Move
            // (0x6F9040) but works in raw screen space: the y axis carries the
            // 4/3 isometric stretch inline (16/9 under the distance root) rather
            // than pre-converting m_pos.y, and the distance is rounded to
            // nearest (+0.5).  m_pPath != NULL is the "still walking" gate; with
            // the move scale unchanged (== field_72EA, cached above) the delta
            // is already correct and the rebuild is skipped.
            if (m_animation.m_animation->GetMoveScale() != field_72EA && m_pPath != NULL) {
                INT scale = static_cast<INT>(sqrt((double)(
                    (m_posDest.y - m_pos.y) * (m_posDest.y - m_pos.y) * 16 / 9
                    + (m_posDest.x - m_pos.x) * (m_posDest.x - m_pos.x))) + 0.5);
                if (scale == 0) {
                    m_posDelta.x = 0;
                    m_posDelta.y = 0;
                } else if (m_animation.m_animation->GetMoveScale() > 8) {
                    // Fast movement: split the remaining distance into whole
                    // frames and step one frame's worth toward the destination.
                    if (scale >= m_animation.m_animation->GetMoveScale()) {
                        scale = (scale + m_animation.m_animation->GetMoveScale() / 2)
                            / m_animation.m_animation->GetMoveScale();
                    } else {
                        scale = 1;
                    }
                    m_posDelta.x = ((m_posDest.x - m_pos.x) << EXACT_SCALE) / scale;
                    m_posDelta.y = (((m_posDest.y - m_pos.y) * 4 / 3) << EXACT_SCALE) / scale;
                } else {
                    // Slow movement: scale the unit displacement by the move
                    // scale, then spread it over the remaining distance.
                    m_posDelta.x = (((m_posDest.x - m_pos.x) << EXACT_SCALE)
                        * m_animation.m_animation->GetMoveScale()) / scale;
                    m_posDelta.y = ((((m_posDest.y - m_pos.y) * 4 / 3) << EXACT_SCALE)
                        * m_animation.m_animation->GetMoveScale()) / scale;
                }
            }

            // 0x72F72B: death-state colour-range re-push.  A creature that died
            // by petrification or freezing keeps its body recoloured to the
            // stone / ice palette.  The derived colour-range list is wiped each
            // tick by the derived-stats refresh (0x71C0C0), so the seven ranges
            // are re-pushed here -- both into m_appliedColorRanges (for the
            // colour broadcast in block E) and straight onto a false-colour
            // animation via SetColorRange.  m_hasColorRangeEffects flags the
            // override.  Frozen death additionally forces every range at once.
            if ((m_derivedStats.m_generalState & STATE_STONE_DEATH) != 0) {
                if (GetAnimation()->IsFalseColor()) {
                    m_hasColorRangeEffects = TRUE;

                    for (BYTE range = 0; range < 7; range++) {
                        CColorRange* pColorRange = new CColorRange();
                        pColorRange->m_range = range;
                        pColorRange->m_color = CVidPalette::STONE;
                        m_derivedStats.m_appliedColorRanges.AddTail(pColorRange);

                        GetAnimation()->SetColorRange(range, CVidPalette::STONE);
                    }
                }
            } else if ((m_derivedStats.m_generalState & STATE_FROZEN_DEATH) != 0) {
                if (GetAnimation()->IsFalseColor()) {
                    m_hasColorRangeEffects = TRUE;

                    for (BYTE range = 0; range < 7; range++) {
                        CColorRange* pColorRange = new CColorRange();
                        pColorRange->m_range = range;
                        pColorRange->m_color = CVidPalette::ICE;
                        m_derivedStats.m_appliedColorRanges.AddTail(pColorRange);

                        GetAnimation()->SetColorRange(range, CVidPalette::ICE);
                    }

                    GetAnimation()->SetColorRangeAll(CVidPalette::ICE);
                }
            }

            // 0x72F8EC: multiplayer-peer state broadcast.  When the sprite is
            // in an area and this host owns it -- single-player (no service
            // provider) or the local player matches m_remotePlayerID -- flag a
            // sprite-update and push a fresh CMessage for each derived-stat
            // group whose contents changed since the last tick.  Each message
            // is dirty-checked against a per-group cache so an unchanged group
            // sends nothing.  AddMessage(_, FALSE) queues without forcing a
            // flush.
            if (m_pArea != NULL
                && (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == m_remotePlayerID)) {
                m_bSendSpriteUpdate = TRUE;

                // Colour: keyed on the applied colour-effect count; the cache
                // also stores the colour-range count.
                if (m_derivedStats.m_appliedColorEffects.GetCount() != field_7536
                    || m_derivedStats.m_appliedColorEffects.GetCount() != field_7532) {
                    CMessage* message = new CMessageColorUpdate(this, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    field_7536 = m_derivedStats.m_appliedColorEffects.GetCount();
                    field_7532 = m_derivedStats.m_appliedColorRanges.GetCount();
                }

                // Weapon immunities.
                if (m_derivedStats.m_cImmunitiesWeapon.GetCount() != field_9D08) {
                    CMessage* message = new CMessageWeaponImmumityUpdate(this, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    field_9D08 = m_derivedStats.m_cImmunitiesWeapon.GetCount();
                }

                // Projectile immunities.
                if (m_derivedStats.m_cImmunitiesProjectile.GetCount() != field_9D0C) {
                    CMessage* message = new CMessageProjectileImmumityUpdate(this, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    field_9D0C = m_derivedStats.m_cImmunitiesProjectile.GetCount();
                }

                // Spell-level immunities (keyed on the packed level mask).
                if (m_derivedStats.m_cImmunitiesSpellLevel.GetMask() != field_9D10) {
                    CMessage* message = new CMessageSpellLevelImmumityUpdate(this, m_id, m_id);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    field_9D10 = m_derivedStats.m_cImmunitiesSpellLevel.GetMask();
                }
            }

            // 0x72FB24: end-of-tick UI refresh + interpolation/state snapshot.
            // Push the freshly recomputed stats to the active engine's on-screen
            // character status, then cache this tick's move scale and general
            // state so the next tick's interpolation re-pace (0x72F476) and
            // state-change detection can see what changed.
            static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine)->UpdateCharacterStatus(m_id);
            field_72EA = GetAnimation()->GetMoveScale();
            field_72EC = m_derivedStats.m_generalState;
        }

        // 0x72FB85: tail.  When no deferred ticks remain, re-arm the gate
        // (0x72FB9D) and run the persistant-effect tick -- which only advances
        // while world time does (not on pause) -- then return.  Otherwise drain
        // one deferred tick and re-process the body.
        if (field_72A2 < 1) {
            m_bAllowEffectListCall = TRUE;
            if (g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_active) {
                ProcessPersistantEffects(1);
            }
            return bResult;
        }
        field_72A2--;
    } while (TRUE);
}

// 0x733290
void CGameSprite::ClearStoredPaths()
{
    POSITION pos = m_pPathTemp.GetHeadPosition();
    while (pos != NULL) {
        int* node = m_pPathTemp.GetNext(pos);
        delete node;
    }

    m_pPathTemp.RemoveAll();
    m_nPathTemp.RemoveAll();
}

// 0x7332D0
void CGameSprite::ClearDialogActions()
{
    SHORT actionID = m_curAction.m_actionID;
    if (actionID == 8
        || actionID == 137
        || actionID == 139
        || actionID == 198) {
        SetCurrAction(CAIAction::NULL_ACTION);

        CMessageDropPath* pMessage = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }
}

// 0x7337D0
void CGameSprite::ClearAI(BOOLEAN bSetSequence)
{
    ClearActions(FALSE);
    ClearTriggers();
    SetCurrAction(CAIAction::NULL_ACTION);

    if (bSetSequence) {
        if (m_nSequence != GetIdleSequence()
            && m_nSequence != SEQ_TWITCH
            && m_nSequence != SEQ_SLEEP
            && m_nSequence != SEQ_DIE) {
            if (GetVertListType() != LIST_FLIGHT) {
                CMessage* pMessage = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()),
                    m_id,
                    m_id);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }
        }
    }
}

// 0x7338E0
//
// CGameSprite override of CGameAIBase::SetCurrAction (0x45D190): in addition to
// the base copy/decode, the binary resets the sprite's per-action execution
// state. The decisive field is m_curDest = (-1, -1): MoveToPoint (0x73F560),
// MoveToObject (0x73EDD0) and MoveToPointRange (0x73FEC0) gate their re-search
// on `dest != m_curDest`, so without this reset a retried move toward the same
// destination never re-issues a CSearchRequest and the sprite stalls at a
// chokepoint. A symmetric original/ours dock differential confirmed it: the
// original clears m_curDest here on every action change; our build, missing this
// override, fell through to the base (which never touches m_curDest), so blocked
// members kept m_curDest == dest and stopped re-pathing.
//
// DEFERRED: for non-NULL actions the binary also closes any open container,
// updates modal-state/quickbar, broadcasts CMessage action-change notifications
// (0x84D424 / 0x84D22C), tears down the target marker + deny-list reservation
// (m_targetId), releases the portrait resource request, emits encumbrance
// feedback, randomizes the move-throttle phase (field_5616 = UtilRandInt) and
// preloads the spell-action ResRef. Those are orthogonal to the movement
// re-search and depend on still-unrecovered spell/resource machinery; the
// recovered behavior is the action-state reset (below) plus the base copy/decode.
// 0x7338E0
void CGameSprite::SetCurrAction(const CAIAction& action)
{
    m_curDest.x = -1;
    m_curDest.y = -1;
    field_7118 = 0;
    field_5616 = 0;
    field_711C = 0;
    m_actionCount = 0;
    m_bStartedCasting = FALSE;
    m_interrupt = FALSE;
    field_5630 = 0;
    field_5632 = 0;
    field_5636 = 0;
    m_moveCount = 0;
    m_bInCasting = FALSE;

    // 0x733D6B / 0x734056: a new action clears the encumbrance feedback latch,
    // then -- for a movement action -- re-announces the sprite's encumbrance so
    // "Encumbered: Slowed/Can not move" prints once per move command (the only
    // emitter of the SLOWED line; STOPPED is also emitted by MoveTo*).
    m_bPlayedEncumberedStopped = 0;
    m_bPlayedEncumberedSlowed = 0;

    SHORT nActionID = action.GetActionID();
    if (nActionID == CAIAction::MOVETOPOINT
        || nActionID == CAIAction::MOVETOOBJECT
        || nActionID == CAIAction::FOLLOW
        || nActionID == CAIAction::MOVETOPOINTNORECTICLE
        || nActionID == CAIAction::PROTECTPOINT
        || nActionID == 215) { // engine-internal move action (0x847920, not in ACTION.IDS)
        if (m_derivedStats.m_nEncumberance == 2) {
            FeedBack(FEEDBACK_TOOHEAVY_STOPPED, 0, 0, 0, -1, 0, 0);
        } else if (m_derivedStats.m_nEncumberance == 1) {
            FeedBack(FEEDBACK_TOOHEAVY_SLOWED, 0, 0, 0, -1, 0, 0);
        }
    }

    CGameAIBase::SetCurrAction(action);
}

// 0x7238C0
// Per-frame AI gate for sprites.  Extends the bare CGameObject predicate
// (active && (counter & m_AISpeed) == (m_AISpeed & m_id)) with the 3-tick
// throttle counter (m_AIInhibitor) and off-screen AI culling: a neutral
// creature (EA in 16..199) that is hidden or scrolled off-screen leaves its
// throttle set and skips full AI this tick; party (EA <= 15) and enemies
// (EA >= 200) always run.  The override ignores the `active` argument.
BOOLEAN CGameSprite::DoAIUpdate(BOOLEAN active, LONG counter)
{
    if ((m_AISpeed & counter) != (m_AISpeed & m_id)) {
        return FALSE;
    }

    if ((m_baseStats.m_flags & 0x200000) == 0) {
        m_AIInhibitor = (BYTE)((m_AIInhibitor + 1) % 3);
    } else {
        if (m_AIInhibitor == 0) {
            return TRUE;
        }
        m_AIInhibitor = 0;
    }

    if (m_AIInhibitor != 0) {
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        if (m_pArea != NULL && pGame->m_gameSave.m_mode != 0x142) {
            CDerivedStats& stats = m_bAllowEffectListCall ? m_derivedStats : m_tempStats;
            if (!stats.m_bPreventAISlowDown) {
                // EA cutoffs DAT_00847c3c (0x0F) / DAT_00847c42 (0xC8): only neutral
                // creatures are culled; party stays below, enemies above.
                BYTE nEnemyAlly = GetAIType().m_nEnemyAlly;
                if (nEnemyAlly < 0xC8 && nEnemyAlly > 0x0F
                    && pGame->GetCharacterPortraitNum(m_id) == -1) {
                    BOOL bVisible = TRUE;
                    // 0x8CF6D8 = g_pChitin; the byte at +0x1032 is the dev "draw-all
                    // / disable-cull" toggle (0 in normal play).  CChitin's layout
                    // there is #guess, so read the offset directly instead of
                    // modelling a member at an uncertain location.
                    if (*(reinterpret_cast<const BYTE*>(g_pChitin) + 0x1032) != 1) {
                        LONG heightOffset = m_pArea->GetHeightOffset(m_pos, m_listType);
                        CRect rFx;
                        CPoint ptReference;
                        m_animation.m_animation->CalculateFxRectMax(rFx, ptReference, heightOffset);

                        LONG nBaseY = heightOffset + m_pos.y;
                        CRect rSprite;
                        rSprite.left = m_pos.x - ptReference.x;
                        rSprite.top = nBaseY - ptReference.y;
                        rSprite.right = (rFx.right - rFx.left - ptReference.x) + m_pos.x;
                        rSprite.bottom = (rFx.bottom - rFx.top - ptReference.y) + nBaseY;

                        CInfinity* pInfinity = m_pArea->GetInfinity();
                        CRect rViewport;
                        rViewport.left = pInfinity->nCurrentX;
                        rViewport.top = pInfinity->nCurrentY;
                        rViewport.right = pInfinity->nCurrentX + pInfinity->rViewPort.Width();
                        rViewport.bottom = pInfinity->nCurrentY + pInfinity->rViewPort.Height();

                        CRect rIntersect;
                        IntersectRect(&rIntersect, &rSprite, &rViewport);
                        bVisible = !IsRectEmpty(&rIntersect);
                    }

                    if (m_canBeSeen != 0 && bVisible) {
                        m_AIInhibitor = 0;
                    }
                    return TRUE;
                }
            }
        }
        m_AIInhibitor = 0;
    }

    return TRUE;
}

// The CGameSprite vtable overrides below are present in IWD2.exe but their
// sprite-specific behavior is not yet recovered.  Each delegates to the base so
// the slot keeps its current fall-through behavior (an empty body would instead
// drop the base work and regress).  Replace each body with the recovered impl.
// Found via scripts/vtable_audit.py; grep TODO(vtable-stub) for the remaining set.

// 0x6FC260 (vtable 0x2C)
// TODO(vtable-stub): recover CGameSprite::CompressTime.
BOOLEAN CGameSprite::CompressTime(DWORD deltaTime)
{
    return CGameAIBase::CompressTime(deltaTime);
}

// 0x6FD9F0 (vtable 0x30)
// TODO(vtable-stub): recover CGameSprite::DebugDump.
void CGameSprite::DebugDump(const CString& a1, BOOLEAN a2)
{
    CGameAIBase::DebugDump(a1, a2);
}

// 0x731B30 (vtable 0x64)
BOOL CGameSprite::EvaluateStatusTrigger(const CAITrigger& trigger)
{
    // TODO INCOMPLETE: original 0x731B30 is a large status-trigger dispatcher.
    // This recovers the NumTimesTalkedTo block at 0x731F9F..0x731FCD, needed by
    // the prologue 10HEDRON dialog entry condition, and the TimerActive case
    // (gates the ambient RandomWalk loop in 00AMVW*.BCS).
    switch (trigger.m_triggerID) {
    case CAITRIGGER_TIMERACTIVE: {
        BOOL bActive = FALSE;
        POSITION pos = m_timers.GetHeadPosition();
        while (pos != NULL) {
            CGameTimer* pTimer = m_timers.GetNext(pos);
            if (pTimer != NULL && pTimer->m_id == trigger.m_specificID) {
                bActive = TRUE;
            }
        }
        return bActive;
    }

    case CAITRIGGER_NEARSAVEDLOCATION: {
        // 0x4099: within Range search-grid cells of the location stored by
        // SaveObjectLocation (the 00AMVW wander scripts' home anchor).
        BOOL bNear = FALSE;
        if (m_baseStats.m_savedLocationX != 0 && m_baseStats.m_savedLocationY != 0) {
            CPoint pos = GetPos();
            INT dx = (pos.x - m_baseStats.m_savedLocationX) / CPathSearch::GRID_SQUARE_SIZEX;
            INT dy = (pos.y - m_baseStats.m_savedLocationY) / CPathSearch::GRID_SQUARE_SIZEY;
            if (dx * dx + dy * dy <= trigger.m_specificID * trigger.m_specificID) {
                bNear = TRUE;
            }
        }
        return bNear;
    }

    case CAITRIGGER_NUMTIMESTALKEDTO:
        return m_nNumberOfTimesTalkedTo == trigger.m_specificID;
    case CAITRIGGER_NUMTIMESTALKEDTOGT:
        return m_nNumberOfTimesTalkedTo > trigger.m_specificID;
    case CAITRIGGER_NUMTIMESTALKEDTOLT:
        return m_nNumberOfTimesTalkedTo < trigger.m_specificID;
    }

    return CGameAIBase::EvaluateStatusTrigger(trigger);
}

// 0x733660 (vtable 0x70)
// TODO(vtable-stub): recover CGameSprite::UpdateTarget -- sets m_targetId and
// toggles the CMarker on the old/new target (gated on m_bSelected + Orderable).
void CGameSprite::UpdateTarget(CGameObject* pObject)
{
    CGameAIBase::UpdateTarget(pObject);
}

// 0x733050 (vtable 0x78)
void CGameSprite::AddEffect(CGameEffect* pEffect, BYTE list, BOOL noSave, BOOL immediateApply)
{
    if (pEffect->m_effectID == CGAMEEFFECT_DETECTTRAPS) {
        delete pEffect;
        return;
    }

    // A target inside Otiluke's Resilient Sphere rejects every incoming effect
    // except dispel, the effects-list carrier, and the cast-spell contingencies.
    if (m_derivedStats.m_spellStates.test(SPLSTATE_OTILUKES_RESILIENT_SPHERE)
        && pEffect->m_effectID != CGAMEEFFECT_DISPELEFFECTS
        && pEffect->m_effectID != ICEWIND_CGAMEEFFECT_APPLYEFFECTSLIST
        && pEffect->m_effectID != CGAMEEFFECT_CASTSPELL
        && pEffect->m_effectID != CGAMEEFFECT_CASTSPELLPOINT) {
        delete pEffect;
        return;
    }

    if (GetAIType().Equal(CAIObjectType::NOT_SPRITE)) {
        return;
    }

    if (!pEffect->CheckAdd(this, &field_70F6, &field_70F7, &field_70F8, &field_70F9, &field_70FA)
        && !noSave) {
        delete pEffect;
    } else {
        field_562C = 1;

        if (immediateApply) {
            m_tempStats = m_derivedStats;
            pEffect->ResolveEffect(this);
            if (pEffect->m_done) {
                delete pEffect;
                return;
            }
        }

        if (list != 1 || pEffect->m_durationType == 2) {
            m_equipedEffectList.AddTail(pEffect);
        } else {
            m_timedEffectList.AddTail(pEffect);
        }
    }

    if (!g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_active && immediateApply) {
        ProcessEffectList();
    }
}

// 0x728580 (vtable 0xAC)
// TODO(vtable-stub): recover CGameSprite::ApplyTriggers -- per-trigger-opcode
// switch; the binary calls CGameAIBase::ApplyTriggers first.
void CGameSprite::ApplyTriggers()
{
    CGameAIBase::ApplyTriggers();
}

// 0x734550
BOOL CGameSprite::HandleEffects()
{
    BOOL bRetry;
    BOOL v1;
    BOOL v2;

    do {
        bRetry = FALSE;

        m_derivedStats.Reload(this, &m_baseStats, &m_spells, &m_domainSpells);
        m_derivedStats.m_nTurnUndeadLevel = GetTurnUndeadLevel();
        m_derivedStats.m_nBackstabDamageMultiplier = GetBackstabDamageMultiplier();
        m_derivedStats.m_nLayOnHandsAmount = GetLayOnHandsAmount();

        ResetAIType();

        m_activeImprisonment = TRUE;

        // TODO: Incomplete.  Original handles many more passes and status
        // side-effects (e.g. the portrait/button refresh at 0x7346c5); this
        // restores the core equipped/timed list pass and the skill-modifier fold.
        m_bonusStats.BonusInit();

        v1 = m_equipedEffectList.HandleList(this);
        v2 = m_timedEffectList.HandleList(this);

        m_derivedStats += m_bonusStats;

        // 0x73469e: fold each situational skill modifier -- the governing
        // ability modifier plus the race/feat bonuses computed by
        // GetSkillModifier -- into the derived skill ranks (first 16 skills),
        // clamped to a floor of 0.  The add is byte-wide, matching the binary's
        // `add cl, al` (only the low byte of the modifier is used) and the signed
        // clamp keeps a net-negative skill from underflowing.  Without this pass
        // the detect-traps / stealth / lore checks run on raw ranks and fall
        // short -- e.g. an elf rogue's Search 4 never reaches the 8 (4 ranks +
        // 2 INT + 2 racial) the trap-detection gate expects.
        for (int index = 0; index < 16; index++) {
            signed char nSkill = static_cast<signed char>(
                static_cast<BYTE>(m_derivedStats.m_nSkills[index])
                + static_cast<BYTE>(GetSkillModifier(index)));
            m_derivedStats.m_nSkills[index] = (nSkill > 0) ? static_cast<BYTE>(nSkill) : 0;
        }

        bRetry = m_equipedEffectList.m_retry || m_timedEffectList.m_retry;
    } while (bRetry);

    return v1 && v2;
}

// 0x7349A0
void CGameSprite::FeedBack(WORD nFeedBackId, LONG a3, LONG a4, LONG a5, LONG a6, LONG a7, LONG a8)
{
    COLORREF rgbNameColor = CVidPalette::RANGE_COLORS[m_baseStats.m_colors[CVIDPALETTE_RANGE_MAIN_CLOTH]];
    CString v1;
    CString v2;

    switch (nFeedBackId) {
    case FEEDBACK_BACKSTAB:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            STRREF strBackstabDamage;
            switch (m_derivedStats.m_nBackstabDamageMultiplier) {
            case 1:
                strBackstabDamage = 32110; // "Backstab Normal Damage"
                break;
            case 2:
                strBackstabDamage = 16471; // "Backstab Double Damage"
                break;
            case 3:
                strBackstabDamage = 16472; // "Backstab Triple Damage"
                break;
            case 4:
                strBackstabDamage = 16473; // "Backstab Quadruple Damage"
                break;
            case 5:
                strBackstabDamage = 16474; // "Backstab Quintuple Damage"
                break;
            case 6:
                strBackstabDamage = 16475; // "Backstab Sextuple Damage"
                break;
            default:
                strBackstabDamage = -1;
                break;
            }

            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                strBackstabDamage,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_CRITICALHIT:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            if (a3 != 0) {
                STR_RES strRes;
                CString sText;
                g_pBaldurChitin->GetTlkTable().Fetch(16462, strRes); // "Critical Hit!"
                sText.Format("%s (%+d)", strRes.szText, a3);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            } else {
                g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                    16462, // "Critical Hit!"
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_CRITICALMISS:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                16463, // "Critical Miss!"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_TOHIT:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x1) != 0) {
            CString sText1;
            CString sText2;
            sText2.Format("%d ", a8);

            if (a7 > 0) {
                STR_RES strRes;
                g_pBaldurChitin->GetTlkTable().Fetch(733, strRes); // "Off Hand"
                sText2 += "(" + strRes.szText + ")";
            }

            if (a5 > 0) {
                sText1.Format(": %d + %d = %d", a3, a5, a3 + a5);
            } else if (a5 < 0) {
                sText1.Format(": %d - %d = %d", a3, -a5, a3 + a5);
            } else {
                sText1.Format(": %d", a3);
            }

            STR_RES strRes;
            g_pBaldurChitin->GetTlkTable().Fetch(14643, strRes); // "Attack Roll "

            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }

            if (strRes.szText != "") {
                sText1 = strRes.szText + " " + sText2 + sText1;
            }

            if (a3 == 1) {
                g_pBaldurChitin->GetTlkTable().Fetch(16463, strRes); // "Critical Miss!"
            } else {
                if (a4 != 0) {
                    g_pBaldurChitin->GetTlkTable().Fetch(16460, strRes); // "Hit"
                } else {
                    g_pBaldurChitin->GetTlkTable().Fetch(16461, strRes); // "Miss"
                }
            }

            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }

            if (strRes.szText != "") {
                sText1 += " : " + strRes.szText;
            }

            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                sText1,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_SPELL:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x4) != 0) {
            STR_RES strRes;
            g_pBaldurChitin->GetTlkTable().Fetch(16464, strRes); // "Casts"

            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }

            if (strRes.szText != "") {
                v1 = strRes.szText;
            }

            g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);

            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }
        }
        break;
    case FEEDBACK_ATTACKS:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x4) != 0) {
            STR_RES strRes1;
            g_pBaldurChitin->GetTlkTable().Fetch(16465, strRes1); // "Attacks"

            if (strRes1.cSound.GetRes() != NULL) {
                if (!strRes1.cSound.GetLooping()) {
                    strRes1.cSound.SetFireForget(TRUE);
                }
                strRes1.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes1.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }

            if (strRes1.szText != "") {
                v1 = strRes1.szText;
            }

            CString sName;
            STR_RES strRes2;
            if (a6 >= -7 && a6 != -1) {
                LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetCharacterSlot(-2 - a6);

                CGameSprite* pSprite;

                BYTE rc;
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
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes2);
                sName = strRes2.szText;
            }

            if (sName != "") {
                v1 += " " + sName;
            }

            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                v1,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_MODAL:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x4) != 0) {
            STR_RES strRes;
            g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);

            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }

            if (strRes.szText != "") {
                v1 = strRes.szText;
            }

            if (a3 != 0) {
                g_pBaldurChitin->GetTlkTable().Fetch(16469, strRes); // "On"
            } else {
                g_pBaldurChitin->GetTlkTable().Fetch(16470, strRes); // "Off"
            }

            if (strRes.cSound.GetRes() != NULL) {
                if (!strRes.cSound.GetLooping()) {
                    strRes.cSound.SetFireForget(TRUE);
                }
                strRes.cSound.SetChannel(14, reinterpret_cast<DWORD>(m_pArea));
                strRes.cSound.Play(m_pos.x, m_pos.y, 0, FALSE);
            }

            if (strRes.szText != "") {
                v1 += strRes.szText;
            }

            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                v1,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_LOCKPICKSUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                16517, // "Lock Pick Succeeded"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_LOCKPICKFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                16518, // "Lock Pick Failed"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_TRAPDISARMED:
        PlaySound(CResRef("ACT_09"));
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                16520, // "Trap Disarmed"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_TRAPDETECTED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                16519, // "Trap Detected"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_ITEMCURSED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                16304, // "Item Cursed!"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_HIDEFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                17120, // "Hide Failed"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_TOOHEAVY_SLOWED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // Debounced once per action: shown only while the flag is clear (or
            // forced with a3 == 1), then latched.  SetCurrAction clears it.
            if (m_bPlayedEncumberedSlowed == 0 || a3 == 1) {
                m_bPlayedEncumberedSlowed = 1;
                g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                    19503, // "Encumbered: Slowed"
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_TOOHEAVY_STOPPED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            if (m_bPlayedEncumberedStopped == 0 || a3 == 1) {
                m_bPlayedEncumberedStopped = 1;
                g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                    19504, // "Encumbered: Can not move"
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_16:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                a6,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_HIDESUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                19944, // "Hide Succeeded"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_MORALEFAILURE_PANIC:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                20568, // "Morale Failure: Panic"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_MORALEFAILURE_RUN:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                20569, // "Morale Failure: Running"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_MORALEFAILURE_BERSERK:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                20570, // "Morale Failure: Berserk"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_LEVELUP:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            17119, //  "Level Up"
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    case FEEDBACK_SPELLFAILED_INDOORS:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            9744, // "Spell Failed: Indoors"
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    case FEEDBACK_LEAVING_SHADOWS:
        PlaySound(CResRef("ACT_07"));
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                4188, // "Leaving Shadows"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_BASHDOORSUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                9915, // "You have successfully forced the door open!"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_BASHDOORFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                9913, // "You have failed to force the door."
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_BASHCHESTSUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                9916, // "You have successfully forced the lock open!"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_BASHCHESTFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                9914, // "You have failed to force the lock."
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_BACKSTAB_UNUSABLE_WEAPON:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            DisplayTextRef(GetNameRef(),
                10013, // "Your weapon is unsuitable for backstab."
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_PICKPOCKETFAILED_ARMOR:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                10067, // "Pick Pocket Disabled Due To Armor"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_PICKPOCKETFAILED_HOSTILE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                10068, // "Cannot pick pocket hostile creatures"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_PICKPOCKETFAILED_SKILL:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                10069, // "Pick Pocket Failed"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_PICKPOCKETFAILED_NOITEMS:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                18297, // "Pick Pocket Succeeded -- Target Had Nothing to Steal"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_PICKPOCKETFAILED_INVENTORYFULL:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                10071, // "Inventory Full"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_PICKPOCKETSUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                10072, // "Pick Pocket Succeeded"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_GAINEDSPECIALABILITY:
        if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCreateChar) {
            STR_RES strRes;
            g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
            g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_SPECIALABILITY, strRes.szText);
            g_pBaldurChitin->GetTlkTable().Fetch(10514, strRes); // "Gained Special Ability: <SPECIALABILITYNAME>"
            g_pBaldurChitin->m_pEngineWorld->DisplayText(GetName(),
                strRes.szText,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                FALSE);
            if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCharacter) {
                g_pBaldurChitin->GetActiveEngine()->PlayGUISound(CResRef("GAM_11"));
            }
        }
        break;
    case FEEDBACK_IMMUNE_TO_WEAPON:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                11025, // "Weapon Ineffective."
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_CHARACTER_CONTROL_SEIZURE:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            20679, // "Timeout: Server Assumed Control Of Character"
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    case FEEDBACK_TRAPDISARMEDFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                1608, // "Trap Not Disarmed"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_CRITICALHITAVOIDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x1) != 0) {
            CString sText1;
            CString sText2;
            STR_RES strRes;
            sText2.Format("%d ", a8);

            if (a7 > 0) {
                g_pBaldurChitin->GetTlkTable().Fetch(733, strRes); // "Off Hand"
                sText2 += "(" + strRes.szText + ")";
            }

            if (a5 > 0) {
                sText1.Format(": %d + %d = %d", a3, a5, a3 + a5);
            } else if (a5 < 0) {
                sText1.Format(": %d - %d = %d", a3, -a5, a3 + a5);
            } else {
                sText1.Format(": %d", a3);
            }

            g_pBaldurChitin->GetTlkTable().Fetch(39874, strRes); // "Critical Threat Roll"

            if (strRes.szText != "") {
                sText1 = strRes.szText + " " + sText2 + sText1;
            }

            if (a4 != 0) {
                g_pBaldurChitin->GetTlkTable().Fetch(16462, strRes); // "Critical Hit!"
            } else {
                g_pBaldurChitin->GetTlkTable().Fetch(33752, strRes); // "Failed!"
            }

            if (strRes.szText != "") {
                sText1 += " : " + strRes.szText;
            }

            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                sText1,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);

            if (a4 != 0) {
                PlaySound(16, TRUE, FALSE, FALSE);
            }
        }
        break;
    case FEEDBACK_AURA_CLEANSED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                23797, // "Aura Cleansed"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_ITEM_DROPPED_FULL:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                10959, // "Inventory Full: The item has been dropped."
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_LOCK_UNPICKABLE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                23169, // "The mechanism that operates this does not have a conventional lock, and may be warded against simple spells."
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_TOOMANYSUMMONED:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            18955, // "Summoning Limit Reached"
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    case FEEDBACK_SILENCED:
        DisplayTextRef(GetNameRef(),
            21298, // "Bard Song Silenced"
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_CRIPPLINGSTRIKESUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            STR_RES strRes;
            CString sText;
            g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
            sText.Format(strRes.szText, a3);
            if (HasFeat(CGAMESPRITE_FEAT_CRIPPLING_STRIKE)) {
                g_pBaldurChitin->GetTlkTable().Fetch(25070, strRes); // " with Crippling Strike (-1 Str)"
                sText.Format(strRes.szText, a3);
            }
            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                sText,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_SNEAK_UNUSABLE_WEAPON:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                24899, // "Weapon is unsuitable for sneak attack"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_EFFECTSEVADED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            STRREF strEvasion;
            if (a6 != -1) {
                strEvasion = 25368; // "Evades effects from <RESOURCE>"

                STR_RES strRes;
                CString sText;
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
                g_pBaldurChitin->GetTlkTable().SetToken(CString("RESOURCE"), strRes.szText);
            } else {
                strEvasion = 25369; // "Evades effects"
            }

            STR_RES strRes;
            CString sText;
            g_pBaldurChitin->GetTlkTable().Fetch(strEvasion, strRes);
            sText = strRes.szText;
            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                sText,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_SPELLFAILED_DISRUPTED:
        DisplayTextRef(GetNameRef(),
            14796, // "Spell Disrupted"
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_GAINEDBARDSONG:
        if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCreateChar) {
            STR_RES strRes;
            CString sText;
            g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
            g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_SPECIALABILITY, strRes.szText);
            g_pBaldurChitin->GetTlkTable().Fetch(26320, strRes); // "Gained Bard Song: <SPECIALABILITYNAME>"
            g_pBaldurChitin->m_pEngineWorld->DisplayText(GetName(),
                sText,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                FALSE);
        }
        if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCharacter) {
            g_pBaldurChitin->GetActiveEngine()->PlayGUISound(CResRef("GAM_11"));
        }
        break;
    case FEEDBACK_STATICCHARGEEND:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                26518, // "Static charge buildup dissipates harmlessly."
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_IMMUNE_TO_RESOURCE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            if (a6 != -1) {
                STR_RES strRes;
                CString sText;
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
                g_pBaldurChitin->GetTlkTable().SetToken(CString("RESOURCE"), strRes.szText);
                g_pBaldurChitin->GetTlkTable().Fetch(26818, strRes); // "Unaffected by effects from <RESOURCE>"
                sText.Format("%s (%+d)", strRes.szText, a3);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_LEAVEFAILED_INVENTORY:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            26826, // "Cannot leave area while remote player is in inventory screen."
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    case FEEDBACK_BACKSTAB_AVOIDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                121, // "The backstab seems to have failed."
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_SPELLFAILED_CASTFAILURE:
        DisplayTextRef(GetNameRef(),
            10453, // "Casting Failure"
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_62:
        DisplayTextRef(GetNameRef(),
            18352, // "Blinks out of existence and attack fails"
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_63:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                18353, // "Blinks out of existence and avoids an attack"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_CRIPPLINGSTRIKEFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                18759, // "The crippling strike seems to have failed."
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_USEMAGICDEVICEFAILED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                24197, // "Use Magic Device Failed"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_USEMAGICDEVICESUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                24198, // "Use Magic Device Succeeded"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_SEARCHSTART:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                25085, // "Searching..."
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_SEARCHEND:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                25090, // "Stopped Searching"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_TURNUNDEADEND:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                25136, // "Stopped Turning Undead"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_BATTLESONGSTART:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            if (a6 != 0) {
                STR_RES strRes1;
                STR_RES strRes2;
                CString sText;

                g_pBaldurChitin->GetTlkTable().Fetch(25138, strRes1); // "Singing Battlesong"
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes2);
                sText.Format("%s: %s", strRes1.szText, strRes2.szText);

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_BATTLESONGEND:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                25141, // "Stopped Singing Battlesong"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_ARTERIALSTRIKESUCCEEDED:
        DisplayTextRef(GetNameRef(),
            37673, // "Arterial Strike feat was used successfully to cause a bleeding wound."
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_HAMSTRINGSUCCEEDED:
        DisplayTextRef(GetNameRef(),
            37675, // "Hamstring feat was used successfully to hinder your opponent's movement by 50%."
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_HEROICINSPIRATION:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            DisplayTextRef(GetNameRef(),
                24196, // "Heroic Inspiration"
                rgbNameColor,
                RGB(215, 215, 190));
        }
        break;
    case FEEDBACK_TARGETCONCEALED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x1) != 0) {
            STR_RES strRes;
            CString sText;
            g_pBaldurChitin->GetTlkTable().Fetch(37668, strRes); // "Missed <TARGETNAME> due to concealment."
            sText.Format("%s (%+d)", strRes.szText, a3);
            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                sText,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_UNCANNYDODGESUCCEEDED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                39308, // "Uncanny Dodge prevented flank attack."
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_ROLL:
        if (m_bRollFeedbackEnabled == -1) {
            if (GetPrivateProfileIntA("Program Options", "EnableRollFeedback", 0, g_pBaldurChitin->GetIniFileName()) != 0) {
                m_bRollFeedbackEnabled = 1;
            } else {
                m_bRollFeedbackEnabled = 0;
            }
        }

        if (m_bRollFeedbackEnabled != 0) {
            if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
                STR_RES strRes;
                CString sText;
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
                sText.Format(strRes.szText, a3, a4, a5, a7, a8);
                g_pBaldurChitin->m_pEngineWorld->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    FALSE);
                if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineCharacter) {
                    g_pBaldurChitin->GetActiveEngine()->PlayGUISound(CResRef("GAM_11"));
                }
            }
        }
        break;
    case FEEDBACK_SPELLFAILURE_CONCENTRATION:
        DisplayTextRef(GetNameRef(),
            39265, // Spell disrupted because caster failed concentration check required due to of nearness of enemies."
            rgbNameColor,
            RGB(215, 215, 190));
        break;
    case FEEDBACK_SPONTANEOUSCASTING:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            if (a6 != 0) {
                STR_RES strRes1;
                STR_RES strRes2;
                CString sText;

                g_pBaldurChitin->GetTlkTable().Fetch(39742, strRes1); // "Spontaneous Casting"
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes2);
                sText.Format("%s: %s", strRes1.szText, strRes2.szText);

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_TOGGLEFEAT:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            if (a6 != 0) {
                STR_RES strRes1;
                STR_RES strRes2;
                CString sText;

                // 39853: "Stopped Using Feat"
                // 39823: "Using Feat"
                g_pBaldurChitin->GetTlkTable().Fetch(a3 == 1 ? 39853 : 39823, strRes1);
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes2);

                if (a4 > 0) {
                    sText.Format("%s: %s %d", strRes1.szText, strRes2.szText, a4);
                } else {
                    sText.Format("%s: %s", strRes1.szText, strRes2.szText);
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_CLEAVE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x4) != 0) {
            STR_RES strRes;
            CString sText;
            g_pBaldurChitin->GetTlkTable().Fetch(39846, strRes); // "Cleave feat adds another level %d attack."
            sText.Format("%s (%+d)", strRes.szText, a3);
            g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                sText,
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_LINGERINGSONG:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            if (a6 != 0) {
                STR_RES strRes1;
                STR_RES strRes2;
                CString sText;

                g_pBaldurChitin->GetTlkTable().Fetch(39881, strRes1); // "Using Feat: Lingering Song"
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes2);
                sText.Format("%s: %s", strRes1.szText, strRes2.szText);

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_89:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            if (a6 != 0) {
                STR_RES strRes;
                CString sText(" ");
                g_pBaldurChitin->GetTlkTable().Fetch(a6, strRes);
                sText.Format("%s", strRes.szText);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(GetName(),
                    sText,
                    rgbNameColor,
                    RGB(215, 215, 190),
                    -1,
                    m_id,
                    m_id);
            }
        }
        break;
    case FEEDBACK_CALLLIGHTNINGEND:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x2) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                41008, // "Call Lightning buildup dissipates harmlessly"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_MAGICALWEAPONINUSE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
                10141, // "Magical weapon in use"
                rgbNameColor,
                RGB(215, 215, 190),
                -1,
                m_id,
                m_id);
        }
        break;
    case FEEDBACK_LEAVEFAILED_LEVELUP:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            41398, // "Cannot leave area while remote player is in the level up screen."
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    case FEEDBACK_LEAVEFAILED_STORE:
        g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(GetNameRef(),
            41400, // "Cannot leave area while remote player is in the store screen."
            rgbNameColor,
            RGB(215, 215, 190),
            -1,
            m_id,
            m_id);
        break;
    }
}

// 0x737910
BOOLEAN CGameSprite::CheckWeaponUsability(BOOL a1)
{
    if (!a1
        && m_equipment.m_items[m_equipment.m_selectedWeapon] != NULL
        && (m_equipment.m_items[m_equipment.m_selectedWeapon]->GetFlagsFile() & 0x10) != 0) {
        m_equipment.m_items[m_equipment.m_selectedWeapon]->m_flags |= 0x1;
        return FALSE;
    }

    if ((m_equipment.m_selectedWeapon == 43
            || m_equipment.m_selectedWeapon == 45
            || m_equipment.m_selectedWeapon == 47
            || m_equipment.m_selectedWeapon == 49)
        && m_equipment.m_items[m_equipment.m_selectedWeapon + 1] != NULL
        && (m_equipment.m_items[m_equipment.m_selectedWeapon + 1]->GetFlagsFile() & 0x10) != 0) {
        m_equipment.m_items[m_equipment.m_selectedWeapon + 1]->m_flags |= 0x1;
        return FALSE;
    }

    return TRUE;
}

// 0x73B740
SHORT CGameSprite::GetCriticalHitBonus()
{
    SHORT nCriticalHitBonus = m_derivedStats.m_nCriticalHitBonus;
    if (HasFeat(CGAMESPRITE_FEAT_IMPROVED_CRITICAL)) {
        nCriticalHitBonus++;
    }
    return nCriticalHitBonus;
}

// 0x73C6A0
SHORT CGameSprite::GetDamageTypeACMod(CGameSprite* target, CItem* curWeapon, const ITEM_ABILITY* curAttack)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 9840
    UTIL_ASSERT(target != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 9841
    UTIL_ASSERT(curWeapon != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 9842
    UTIL_ASSERT(curAttack != NULL);

    SHORT mod = 0;

    switch (curAttack->damageType) {
    case 1:
        mod += target->GetDerivedStats()->m_nACPiercingMod;
        break;
    case 2:
        mod += target->GetDerivedStats()->m_nACCrushingMod;
        break;
    case 3:
        mod += target->GetDerivedStats()->m_nACSlashingMod;
        break;
    case 4:
        mod += target->GetDerivedStats()->m_nACMissileMod;
        mod += target->GetDerivedStats()->m_nACPiercingMod;
        break;
    case 5:
        mod += target->GetDerivedStats()->m_nACCrushingMod;
        break;
    case 6:
        mod += max(target->GetDerivedStats()->m_nACPiercingMod, target->GetDerivedStats()->m_nACCrushingMod);
        break;
    case 7:
        mod += max(target->GetDerivedStats()->m_nACPiercingMod, target->GetDerivedStats()->m_nACSlashingMod);
        break;
    case 8:
        mod += max(target->GetDerivedStats()->m_nACCrushingMod, target->GetDerivedStats()->m_nACSlashingMod);
        break;
    case 9:
        mod += target->GetDerivedStats()->m_nACMissileMod;
        mod += target->GetDerivedStats()->m_nACCrushingMod;
        break;
    }

    return mod;
}

// 0x73C7E0
SHORT CGameSprite::GetRacialAttackPenalty()
{
    SHORT mod = 0;

    if (m_typeAI.GetRace() == CAIOBJECTTYPE_R_DWARF
        && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_DWARF_GRAY) {
        if (m_pArea != NULL
            && (m_pArea->GetHeader()->m_areaType & 0x1) != 0
            && (m_pArea->GetHeader()->m_areaType & 0x2) != 0
            && (g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDay()
                || g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDawn())) {
            mod -= 2;
        }
    }

    if (m_typeAI.GetRace() == CAIOBJECTTYPE_R_ELF
        && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_ELF_DROW) {
        if (m_pArea != NULL
            && (m_pArea->GetHeader()->m_areaType & 0x1) != 0
            && (m_pArea->GetHeader()->m_areaType & 0x2) != 0
            && (g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDay()
                || g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->IsDawn())) {
            mod -= 1;
        }
    }

    return mod;
}

// 0x73C8C0
SHORT CGameSprite::GetRacialTargetBonus(CGameSprite* target)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 9925
    UTIL_ASSERT(target != NULL);

    SHORT mod = 0;

    if (m_typeAI.GetRace() == CAIOBJECTTYPE_R_DWARF) {
        if (m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_DWARF_GOLD
            && IcewindMisc::IsAberration(target) == TRUE) {
            mod++;
        } else {
            switch (target->m_typeAI.GetRace()) {
            case CAIOBJECTTYPE_R_HOBGOBLIN:
            case CAIOBJECTTYPE_R_GOBLIN:
            case CAIOBJECTTYPE_R_ORC:
            case CAIOBJECTTYPE_R_BUGBEAR:
                mod++;
                break;
            }
        }
    }

    if (m_typeAI.GetRace() == CAIOBJECTTYPE_R_GNOME) {
        switch (target->m_typeAI.GetRace()) {
        case CAIOBJECTTYPE_R_HOBGOBLIN:
        case CAIOBJECTTYPE_R_KOBOLD:
        case CAIOBJECTTYPE_R_GOBLIN:
        case CAIOBJECTTYPE_R_BUGBEAR:
            mod++;
            break;
        }
    }

    if (m_typeAI.GetRace() == CAIOBJECTTYPE_R_GIANT
        || m_typeAI.GetRace() == CAIOBJECTTYPE_R_OGRE) {
        if ((target->m_typeAI.GetRace() == CAIOBJECTTYPE_R_GNOME
                && target->m_typeAI.m_nSubRace != CAIOBJECTTYPE_SUBRACE_DWARF_GOLD)
            || target->m_typeAI.GetRace() == CAIOBJECTTYPE_R_DWARF) {
            mod -= 4;
        }
    }

    return mod;
}

// 0x73CA20
SHORT CGameSprite::GetHalflingSlingBonus(CItem* curWeapon, const ITEM_ABILITY* curAttack)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 9988
    UTIL_ASSERT(curWeapon != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 9989
    UTIL_ASSERT(curAttack != NULL);

    SHORT mod = 0;

    if (m_typeAI.GetRace() == CAIOBJECTTYPE_R_HALFLING
        && curAttack->type == 2) {
        switch (curWeapon->GetItemType()) {
        case 14:
        case 16:
        case 21:
        case 24:
        case 25:
        case 29:
            mod++;
            break;
        }
    }

    return mod;
}

// 0x73CAD0
SHORT CGameSprite::GetFlankingBonus()
{
    return 0;
}

// 0x73CAE0
SHORT CGameSprite::GetWeaponStyleBonus(CItem* curWeapon, const ITEM_ABILITY* curAttack)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 10073
    UTIL_ASSERT(curAttack != NULL);

    return 0;
}

// 0x73CB10
SHORT CGameSprite::GetAbilityHitBonus(CItem* curWeapon, const ITEM_ABILITY* curAttack)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 10083
    UTIL_ASSERT(curWeapon != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 10084
    UTIL_ASSERT(curAttack != NULL);

    const CRuleTables& cRule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    if (curAttack->type == 2 || curAttack->type == 4) {
        return cRule.GetAbilityScoreModifier(m_derivedStats.m_nDEX);
    }

    WORD itemType = curWeapon->GetItemType();
    INT mod = cRule.GetAbilityScoreModifier(m_derivedStats.m_nSTR);

    if (HasFeat(CGAMESPRITE_FEAT_WEAPON_FINESSE)
        && (itemType == 19 || itemType == 16)) {
        INT nDEXMod = cRule.GetAbilityScoreModifier(m_derivedStats.m_nDEX);
        if (mod < nDEXMod) {
            mod = nDEXMod;
        }
    }

    if (HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK)
        && GetFeatRank(CGAMESPRITE_FEAT_POWER_ATTACK) > 0) {
        mod -= GetFeatRank(CGAMESPRITE_FEAT_POWER_ATTACK);
    }

    if (HasFeat(CGAMESPRITE_FEAT_EXPERTISE)
        && GetFeatRank(CGAMESPRITE_FEAT_EXPERTISE) > 0) {
        mod -= GetFeatRank(CGAMESPRITE_FEAT_EXPERTISE);
    }

    return static_cast<SHORT>(mod);
}

// 0x73CC40
SHORT CGameSprite::GetTHAC0Bonuses(CItem* curWeapon, const ITEM_ABILITY* curAttack)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 10144
    UTIL_ASSERT(curWeapon != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 10145
    UTIL_ASSERT(curAttack != NULL);

    SHORT mod = 0;

    if (curAttack->thac0Bonus != SHORT_MAX) {
        mod += curAttack->thac0Bonus;
    }

    if (curAttack->type == 2 || curAttack->type == 4) {
        mod += m_derivedStats.m_nMissileTHAC0Bonus;
    }

    SHORT lSlot;
    CItem* pLauncher = GetLauncher(curAttack, lSlot);
    if (pLauncher != NULL) {
        pLauncher->Demand();
        const ITEM_ABILITY* ability = pLauncher->GetAbility(0);
        if (ability != NULL) {
            mod += ability->thac0Bonus;
        }
        pLauncher->Release();
    }

    return mod;
}

// 0x73D420
SHORT CGameSprite::GetTHAC0()
{
    SHORT mod = 0;

    if ((m_derivedStats.m_generalState & STATE_BERSERK) != 0) {
        mod += 2;
    }

    mod += m_derivedStats.m_nTHAC0;

    return mod;
}

// 0x73D440
SHORT CGameSprite::GetProficiencyTHAC0Bonus(CItem* curWeapon)
{
    SHORT mod = 0;
    switch (curWeapon->GetItemType()) {
    case 0:
    case 28:
        mod++;
        break;
    case 15:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_BOW));
        break;
    case 16:
    case 19:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE));
        break;
    case 17:
    case 22:
    case 44:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_SIMPLE_MACE));
        break;
    case 18:
    case 24:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_SIMPLE_MISSILE));
        break;
    case 20:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_LARGESWORD));
        break;
    case 21:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_HAMMER));
        break;
    case 23:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_FLAIL));
        break;
    case 25:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_AXE));
        break;
    case 26:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF));
        break;
    case 27:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_SIMPLE_CROSSBOW));
        break;
    case 29:
    case 30:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_POLEARM));
        break;
    case 57:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_GREATSWORD));
        break;
    case 69:
        mod = static_cast<SHORT>(GetFeatValue(CGAMESPRITE_FEAT_EXOTIC_BASTARD));
        break;
    }

    if (mod < 0) {
        mod = 0;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_TENSERS_TRANSFORMATION] && mod == 0) {
        mod = 1;
    }

    return mod;
}

// 0x73EDD0
SHORT CGameSprite::MoveToObject(CGameObject* pTarget)
{
    if (m_derivedStats.m_nEncumberance == 2) {
        FeedBack(FEEDBACK_TOOHEAVY_STOPPED, 0, 0, 0, -1, 0, 0);
        return ACTION_ERROR;
    }

    if (pTarget == NULL) {
        return ACTION_ERROR;
    }

    if (pTarget->m_pArea == NULL) {
        return ACTION_DONE;
    }

    // Action 0x107 (MoveToObjectUntilSee) stops early once the target is within
    // half the visual range and in line of sight, posting a follow message
    // (vtable 0x848998 @ binary 0x73EFD3).  DEFERRED -- needs that CMessage
    // subclass plus the GetAIType()/m_liveTypeAI field comparison; until then
    // the action falls through to the generic approach below.

    CPoint targetPt;
    pTarget->GetNextWaypoint(&targetPt);

    if (m_curAction.m_actionID == 0x69) {
        // AttackOneRound: bail out if the target square itself is impassable.
        SHORT searchSquareCode;
        if (m_pArea->m_search.GetLOSCost(
                CPoint(targetPt.x / CPathSearch::GRID_SQUARE_SIZEX,
                    targetPt.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_terrainTable, searchSquareCode, FALSE) == CPathSearch::COST_IMPASSABLE) {
            return ACTION_ERROR;
        }
    }

    CPoint selfPos = GetPos();
    CPoint targetCell(targetPt.x / CPathSearch::GRID_SQUARE_SIZEX,
        targetPt.y / CPathSearch::GRID_SQUARE_SIZEY);
    CPoint selfCell(selfPos.x / CPathSearch::GRID_SQUARE_SIZEX,
        selfPos.y / CPathSearch::GRID_SQUARE_SIZEY);
    LONG distSquares = CAIUtil::CountSquares(selfCell, targetCell);
    distSquares = distSquares * distSquares;

    // The combined personal space is truncated to a byte before the radius
    // math (binary masks with 0xFF), so keep it in a BYTE here.
    BYTE range = m_animation.GetPersonalSpace();
    if (pTarget->GetObjectType() == CGameObject::TYPE_SPRITE) {
        BYTE targetSpace = static_cast<CGameSprite*>(pTarget)->m_animation.GetPersonalSpace();
        range = static_cast<BYTE>(((targetSpace - 1) >> 1) + ((range - 1) >> 1) - 1);
    }

    // 0xD0 keeps moving with ACTION_NORMAL; every other variant uses
    // ACTION_INTERRUPTABLE while still approaching.
    SHORT actionID = m_curAction.m_actionID;

    if (actionID == 0xB4 && distSquares <= (range + 4) * (range + 4)) {
        // MoveToObjectFollow: close enough on the loose follow distance.
        if (m_pPath != NULL) {
            // DEFERRED: post the clear-search message (vtable 0x84C44C @ binary
            // 0x73F1D4) so the follower halts its current path.
        }
        return actionID == 0xD0 ? ACTION_NORMAL : ACTION_INTERRUPTABLE;
    }

    if (distSquares <= (range + 1) * (range + 1)) {
        return actionID == 0xB4 ? ACTION_INTERRUPTABLE : ACTION_DONE;
    }

    // --- Out of range: (re)issue a path search toward the target object. ---
    m_followRange = 0;

    if (targetPt.x / CPathSearch::GRID_SQUARE_SIZEX == m_curDest.x / CPathSearch::GRID_SQUARE_SIZEX
        && targetPt.y / CPathSearch::GRID_SQUARE_SIZEY == m_curDest.y / CPathSearch::GRID_SQUARE_SIZEY
        && field_5618 != 0
        && (m_pPath != NULL || m_currentSearchRequest != NULL)) {
        // Already pathing toward the same destination cell -- keep moving.
        return actionID == 0xD0 ? ACTION_NORMAL : ACTION_INTERRUPTABLE;
    }
    field_5618 = 0;

    // The binary forks on a "<16 cells" vs ">=16 cells" distance test (binary
    // 0x73F2C0 / 0x73F3F0); both arms run the identical request build and differ
    // only in SEH unwind state, so they collapse to one throttled block here.
    if (field_5616 % 8 == 0) {
        m_curDest.x = targetPt.x;
        m_curDest.y = targetPt.y;

        CSearchRequest* pSearchRequest = new CSearchRequest();
        if (pSearchRequest == NULL) {
            return ACTION_ERROR;
        }

        pSearchRequest->m_searchBitmap = &(m_pArea->m_search);
        memcpy(pSearchRequest->m_terrainTable, m_terrainTable, sizeof(m_terrainTable));
        pSearchRequest->m_pathSmooth = m_animation.GetPathSmooth();
        pSearchRequest->m_sourceId = m_id;
        pSearchRequest->m_nTargetIds = 1;

        pSearchRequest->m_targetIds = new LONG[pSearchRequest->m_nTargetIds];
        if (pSearchRequest->m_targetIds == NULL) {
            delete pSearchRequest;
            return ACTION_ERROR;
        }

        pSearchRequest->m_targetIds[0] = pTarget->m_id;
        pSearchRequest->m_sourceSide = m_typeAI.GetEnemyAlly();
        field_5618 = 1;
        SetTarget(pSearchRequest, FALSE, LIST_FRONT);
    }
    field_5616++;

    return actionID == 0xD0 ? ACTION_NORMAL : ACTION_INTERRUPTABLE;
}

// 0x73F560
SHORT CGameSprite::MoveToPoint()
{
    LONG x = m_curAction.m_dest.x;
    LONG y = m_curAction.m_dest.y;
    if (m_curAction.GetActionID() == CAIAction::TIMEDMOVETOPOINT) {
        LONG n = m_curAction.m_specificID - 1;
        if (n < 1) {
            m_curAction.m_actionID = CAIAction::JUMPTOPOINT;
            return JumpToPoint(m_curAction.m_dest, TRUE);
        }
        m_curAction.m_specificID = n;
    }

    if (m_derivedStats.m_nEncumberance == 2) {
        FeedBack(FEEDBACK_TOOHEAVY_STOPPED, 0, 0, 0, -1, 0, 0);
        return ACTION_ERROR;
    }

    if (x == -1) {
        x = m_posStart.x;
    } else if (x == -2) {
        x = m_baseStats.m_savedLocationX;
    }

    if (y == -1) {
        y = m_posStart.y;
    } else if (y == -2) {
        y = m_baseStats.m_savedLocationY;
    }

    if (x / CPathSearch::GRID_SQUARE_SIZEX == m_pos.x / CPathSearch::GRID_SQUARE_SIZEX
        && y / CPathSearch::GRID_SQUARE_SIZEY == m_pos.y / CPathSearch::GRID_SQUARE_SIZEY) {
        return ACTION_DONE;
    }

    if (x < 0
        || x >= m_pArea->GetInfinity()->nAreaX
        || y < 0
        || y >= m_pArea->GetInfinity()->nAreaY) {
        return ACTION_ERROR;
    }

    if (m_pPath == NULL
        && m_currentSearchRequest == NULL
        && m_actionCount > 0
        && m_curAction.m_specificID != CAIAction::BACKGROUND) {
        m_curAction.m_specificID2++;
        if (m_curAction.m_specificID2 > 4) {
            return ACTION_ERROR;
        }

        SHORT searchSquareCode;
        if (m_pArea->m_search.GetLOSCost(CPoint(x / CPathSearch::GRID_SQUARE_SIZEX, y / CPathSearch::GRID_SQUARE_SIZEY), m_terrainTable, searchSquareCode, FALSE) == CPathSearch::COST_IMPASSABLE) {
            return ACTION_ERROR;
        }

        m_curDest.x = x;
        m_curDest.y = y;

        if (!m_interrupt) {
            if (m_curAction.GetActionID() == CAIAction::MOVETOPOINTNORECTICLE) {
                CAIAction action(CAIAction::MOVETOPOINTNORECTICLE,
                    CPoint(x, y),
                    m_curAction.m_specificID,
                    m_curAction.m_specificID2);
                AddAction(action);
            } else {
                CAIAction action(CAIAction::MOVETOPOINT,
                    CPoint(x, y),
                    m_curAction.m_specificID,
                    m_curAction.m_specificID2);
                AddAction(action);
            }

            CAIAction action(CAIAction::SMALLWAIT,
                CPoint(-1, -1),
                rand() % 8 + 1,
                -1);
            AddAction(action);
        }

        return ACTION_DONE;
    }

    if (m_pPath == NULL
        && m_currentSearchRequest == NULL
        && m_actionCount > 0
        && m_curAction.m_specificID == CAIAction::BACKGROUND) {
        return ACTION_ERROR;
    }

    if (x != m_curDest.x || y != m_curDest.y) {
        m_curDest.x = x;
        m_curDest.y = y;

        CSearchRequest* pSearchRequest = new CSearchRequest();
        if (pSearchRequest == NULL) {
            return ACTION_ERROR;
        }

        pSearchRequest->m_searchBitmap = &(m_pArea->m_search);
        if (m_animation.GetListType() == LIST_FLIGHT) {
            memcpy(pSearchRequest->m_terrainTable, m_flightTerrainTable, sizeof(m_flightTerrainTable));
        } else {
            memcpy(pSearchRequest->m_terrainTable, m_terrainTable, sizeof(m_terrainTable));
        }

        pSearchRequest->m_removeSelf = m_animation.GetListType() != LIST_FLIGHT;
        pSearchRequest->m_pathSmooth = m_animation.GetPathSmooth();
        pSearchRequest->m_sourceId = m_id;
        pSearchRequest->m_nTargetPoints = 1;
        pSearchRequest->m_exclusiveTargetPoints = TRUE;

        pSearchRequest->m_targetPoints = new POINT[pSearchRequest->m_nTargetPoints];
        if (pSearchRequest->m_targetPoints == NULL) {
            delete pSearchRequest;
            return ACTION_ERROR;
        }

        if (m_pArea->m_pGame->GetGroup()->InList(m_id)) {
            pSearchRequest->m_nPartyIds = m_pArea->m_pGame->GetGroup()->GetCount();
            pSearchRequest->m_partyIds = m_pArea->m_pGame->GetGroup()->GetGroupList();
        }

        pSearchRequest->m_targetPoints[0] = m_curDest;
        pSearchRequest->m_sourceSide = m_typeAI.GetEnemyAlly();

        if (m_curAction.m_specificID == CAIAction::BACKGROUND) {
            pSearchRequest->m_frontList = 2;
            pSearchRequest->m_minNodesBack = 110;
            pSearchRequest->m_sourcePt.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
            pSearchRequest->m_sourcePt.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
            pSearchRequest->m_maxNodesBack = CSearchRequest::MINNODESBACK;
            SetTarget(pSearchRequest, FALSE, LIST_FLIGHT);
        } else {
            SetTarget(pSearchRequest, FALSE, LIST_FRONT);
        }
    }

    if (m_curAction.GetActionID() == 207) {
        return ACTION_NORMAL;
    }

    return ACTION_INTERRUPTABLE;
}

// 0x734720
void CGameSprite::ComputeWaypoint(CPoint& result, LONG** ppPath, SHORT* pnPath, SHORT delay, BYTE* pReachedEnd)
{
    LONG* pPath = *ppPath;
    SHORT nPath = *pnPath;

    LONG lastPos = pPath[nPath - 1];
    LONG targetSquares = delay * 3;

    LONG lastX = lastPos % CPathSearch::GRID_ACTUALX;
    LONG lastY = CPathSearch::GRID_ACTUALY - lastPos / CPathSearch::GRID_ACTUALX - 1;

    LONG prevPos = pPath[nPath - 2];
    LONG prevX = prevPos % CPathSearch::GRID_ACTUALX;
    LONG prevY = CPathSearch::GRID_ACTUALY - prevPos / CPathSearch::GRID_ACTUALX - 1;

    *pReachedEnd = 0;

    CPoint ptLast(lastX, lastY);
    CPoint ptPrev(prevX, prevY);
    LONG segSquares = CAIUtil::CountSquares(ptLast, ptPrev);

    BOOL exact = (segSquares == targetSquares);

    while (segSquares < targetSquares) {
        SHORT newN = nPath - 1;
        *pnPath = newN;

        if (newN < 3) {
            *pnPath = nPath;
            LONG dx = lastX - prevX;
            LONG dy = lastY - prevY;
            LONG segLen = CAIUtil::CountSquares(ptLast, ptPrev);
            if (segLen > 0) {
                LONG steps = targetSquares / segLen + 1;
                *pReachedEnd = 1;
                result.x = CPathSearch::GRID_SQUARE_SIZEX * (lastX - steps * dx);
                result.y = CPathSearch::GRID_SQUARE_SIZEY * (lastY - steps * dy);
            } else {
                result.x = CPathSearch::GRID_SQUARE_SIZEX * lastX;
                result.y = CPathSearch::GRID_SQUARE_SIZEY * lastY;
            }
            return;
        }

        nPath = newN;
        targetSquares -= segSquares;

        lastPos = pPath[nPath - 1];
        lastX = lastPos % CPathSearch::GRID_ACTUALX;
        lastY = CPathSearch::GRID_ACTUALY - lastPos / CPathSearch::GRID_ACTUALX - 1;

        prevPos = pPath[nPath - 2];
        prevX = prevPos % CPathSearch::GRID_ACTUALX;
        prevY = CPathSearch::GRID_ACTUALY - prevPos / CPathSearch::GRID_ACTUALX - 1;

        ptLast = CPoint(lastX, lastY);
        ptPrev = CPoint(prevX, prevY);
        segSquares = CAIUtil::CountSquares(ptLast, ptPrev);
    }

    exact = (segSquares == targetSquares);

    if (exact) {
        if (*pnPath > 3) {
            *pnPath = *pnPath - 1;
        }
    } else {
        LONG dx = lastX - prevX;
        LONG dy = lastY - prevY;
        double dist = sqrt((double)(dx * dx + dy * dy));
        if (dist > 0.0) {
            LONG iDist = (LONG)dist;
            lastX = lastX + ((prevX - lastX) * targetSquares) / iDist;
            lastY = lastY + ((prevY - lastY) * targetSquares) / iDist;
        }
        pPath[nPath - 1] = (CPathSearch::GRID_ACTUALY - lastY - 1) * CPathSearch::GRID_ACTUALX + lastX;
    }

    result.x = CPathSearch::GRID_SQUARE_SIZEX * lastX;
    result.y = CPathSearch::GRID_SQUARE_SIZEY * lastY;
}

// 0x73FEC0
SHORT CGameSprite::MoveToPointRange(const CPoint& dest, LONG range)
{
    if (range == 0) {
        if (dest.x / CPathSearch::GRID_SQUARE_SIZEX == m_pos.x / CPathSearch::GRID_SQUARE_SIZEX
            && dest.y / CPathSearch::GRID_SQUARE_SIZEY == m_pos.y / CPathSearch::GRID_SQUARE_SIZEY) {
            return ACTION_DONE;
        }
    } else {
        LONG dx = dest.x / CPathSearch::GRID_SQUARE_SIZEX - m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
        LONG dy = (dest.y / CPathSearch::GRID_SQUARE_SIZEY == m_pos.y / CPathSearch::GRID_SQUARE_SIZEY) ? 0 : 1;
        if (dx * dx + dy * dy <= range * range) {
            return ACTION_DONE;
        }
    }

    LONG x = dest.x;
    LONG y = dest.y;

    if (x < 0 || x >= m_pArea->GetInfinity()->nAreaX
        || y < 0 || y >= m_pArea->GetInfinity()->nAreaY) {
        return ACTION_ERROR;
    }

    if ((m_pPath != NULL || m_currentSearchRequest != NULL)
        || (m_actionCount < 1 || m_curAction.m_specificID == CAIAction::BACKGROUND)) {
        if (x == m_curDest.x && y == m_curDest.y) {
            return ACTION_INTERRUPTABLE;
        }
    }

    m_curDest.x = x;
    m_curDest.y = y;

    CSearchRequest* pSearchRequest = new CSearchRequest();
    if (pSearchRequest == NULL) {
        return ACTION_ERROR;
    }

    pSearchRequest->m_searchBitmap = &(m_pArea->m_search);
    if (m_animation.GetListType() == LIST_FLIGHT) {
        memcpy(pSearchRequest->m_terrainTable, m_flightTerrainTable, sizeof(m_flightTerrainTable));
    } else {
        memcpy(pSearchRequest->m_terrainTable, m_terrainTable, sizeof(m_terrainTable));
    }

    pSearchRequest->m_removeSelf = m_animation.GetListType() != LIST_FLIGHT;
    pSearchRequest->m_pathSmooth = m_animation.GetPathSmooth();
    pSearchRequest->m_sourceId = m_id;
    pSearchRequest->m_nTargetPoints = 1;
    pSearchRequest->m_exclusiveTargetPoints = TRUE;

    pSearchRequest->m_targetPoints = new POINT[pSearchRequest->m_nTargetPoints];
    if (pSearchRequest->m_targetPoints == NULL) {
        delete pSearchRequest;
        return ACTION_ERROR;
    }

    if (m_pArea->m_pGame->GetGroup()->InList(m_id)) {
        pSearchRequest->m_nPartyIds = m_pArea->m_pGame->GetGroup()->GetCount();
        pSearchRequest->m_partyIds = m_pArea->m_pGame->GetGroup()->GetGroupList();
    }

    pSearchRequest->m_targetPoints[0].x = m_curDest.x;
    pSearchRequest->m_targetPoints[0].y = m_curDest.y;
    pSearchRequest->m_sourceSide = m_typeAI.GetEnemyAlly();

    if (m_curAction.m_specificID == CAIAction::BACKGROUND) {
        pSearchRequest->m_frontList = 2;
        pSearchRequest->m_minNodesBack = 110;
        pSearchRequest->m_sourcePt.x = m_pos.x / CPathSearch::GRID_SQUARE_SIZEX;
        pSearchRequest->m_sourcePt.y = m_pos.y / CPathSearch::GRID_SQUARE_SIZEY;
        pSearchRequest->m_maxNodesBack = CSearchRequest::MINNODESBACK;
        SetTarget(pSearchRequest, FALSE, LIST_FLIGHT);
    } else {
        SetTarget(pSearchRequest, FALSE, LIST_FRONT);
    }

    return ACTION_INTERRUPTABLE;
}

// 0x74A7C0
SHORT CGameSprite::Follow()
{
    CPoint actionDest = m_curAction.m_dest;
    CPoint waypoint = CPoint(m_curAction.m_dest.x, m_curAction.m_dest.y);
    LONG savedSpecID2 = m_curAction.m_specificID2;
    LONG savedSpecID3 = m_curAction.m_specificID3;

    if (m_actionCount == 0) {
        if (m_pPathTemp.IsEmpty()) {
            return ACTION_ERROR;
        }

        LONG* pPath = reinterpret_cast<LONG*>(m_pPathTemp.RemoveHead());
        SHORT nPath = (SHORT)reinterpret_cast<int>(m_nPathTemp.RemoveHead());

        BYTE reachedEnd = 0;
        CPoint computedWaypoint;
        ComputeWaypoint(computedWaypoint, &pPath, &nPath, (SHORT)m_curAction.m_specificID, &reachedEnd);

        waypoint = computedWaypoint;
        m_curAction.m_dest = waypoint;

        LONG distToAction = (m_pos.x - savedSpecID2) * (m_pos.x - savedSpecID2)
            + (m_pos.y - savedSpecID3) * (m_pos.y - savedSpecID3);

        if (distToAction > 0x40000) {
            delete[] pPath;
            CAIAction moveAction(CAIAction::MOVETOPOINTNORECTICLE, waypoint, 0, -1);
            AddAction(moveAction);
            return ACTION_DONE;
        }

        LONG distToWaypoint = (m_pos.x - waypoint.x) * (m_pos.x - waypoint.x)
            + (m_pos.y - waypoint.y) * (m_pos.y - waypoint.y);

        if (distToWaypoint < distToAction) {
            delete[] pPath;
            CAIAction moveAction(CAIAction::MOVETOPOINTNORECTICLE, waypoint, 0, -1);
            AddAction(moveAction);
            return ACTION_DONE;
        }

        if (reachedEnd) {
            delete[] pPath;
            CAIAction moveAction(CAIAction::MOVETOPOINTNORECTICLE, waypoint, 0, -1);
            AddAction(moveAction);
            return ACTION_DONE;
        }

        CPoint ptActionDest(savedSpecID2, savedSpecID3);
        SHORT moveResult = MoveToPointRange(ptActionDest, 0);
        if (moveResult == ACTION_DONE) {
            m_curAction.m_internalFlags |= 0x10000000;
            SetPath(pPath, nPath);
        } else if (moveResult == ACTION_ERROR) {
            delete[] pPath;
            CAIAction moveAction(CAIAction::MOVETOPOINTNORECTICLE, waypoint, 0, -1);
            AddAction(moveAction);
            return ACTION_DONE;
        } else {
            m_pPathTemp.AddHead(reinterpret_cast<int*>(pPath));
            m_nPathTemp.AddHead(reinterpret_cast<int*>(nPath));
            return ACTION_INTERRUPTABLE;
        }
    } else {
        if ((m_curAction.m_internalFlags & 0x10000000) != 0) {
            goto movementCheck;
        }

        LONG* pPath = reinterpret_cast<LONG*>(m_pPathTemp.RemoveHead());
        SHORT nPath = (SHORT)reinterpret_cast<int>(m_nPathTemp.RemoveHead());

        CPoint ptActionDest(savedSpecID2, savedSpecID3);
        SHORT moveResult = MoveToPointRange(ptActionDest, 0);
        if (moveResult == ACTION_DONE) {
            m_curAction.m_internalFlags |= 0x10000000;
            SetPath(pPath, nPath);
        } else if (moveResult == ACTION_ERROR) {
            delete[] pPath;
            CAIAction moveAction(CAIAction::MOVETOPOINTNORECTICLE, waypoint, 0, -1);
            AddAction(moveAction);
            return ACTION_DONE;
        } else {
            m_pPathTemp.AddHead(reinterpret_cast<int*>(pPath));
            m_nPathTemp.AddHead(reinterpret_cast<int*>(nPath));
            return ACTION_INTERRUPTABLE;
        }
    }

movementCheck:
    if ((waypoint.x / CPathSearch::GRID_SQUARE_SIZEX != m_pos.x / CPathSearch::GRID_SQUARE_SIZEX
        || waypoint.y / CPathSearch::GRID_SQUARE_SIZEY != m_pos.y / CPathSearch::GRID_SQUARE_SIZEY)
        && m_pPath == NULL
        && m_actionCount > 0) {
        CAIAction moveAction(CAIAction::MOVETOPOINTNORECTICLE, waypoint, 0, -1);
        AddAction(moveAction);
        return ACTION_DONE;
    }

    return ACTION_INTERRUPTABLE;
}

// 0x728F80 (partial)
SHORT CGameSprite::ExecuteAction()
{
    // ActionOverride (id 1) is a queue marker -- dequeue and dispatch the real
    // action this tick, same as the base dispatcher.
    if (m_curAction.m_actionID == 1) {
        SetCurrAction(GetNextAction(m_aiAction));
    }

    // 0x729A84 (jumptable case 0x19). Spell(31) / SpellNoDec(191).  Spell()
    // is called unconditionally -- a NULL target still runs its prologue
    // (projectile cleanup, aura-cleansing feedback) before its own NULL
    // exit, like the binary.
    if (m_curAction.m_actionID == CAIAction::SPELL
        || m_curAction.m_actionID == CAIAction::SPELLNODEC) {
        CGameObject* object = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        SHORT actionReturn = Spell(static_cast<CGameAIBase*>(object));
        if (object != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                object->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        return actionReturn;
    }

    // 0x729E8F (jumptable case 0x2c). SpellPoint(95) / SpellPointNoDec(192):
    // straight to the point-cast executor, no target resolution -- the cast
    // point travels in the action.
    if (m_curAction.m_actionID == CAIAction::SPELLPOINT
        || m_curAction.m_actionID == CAIAction::SPELLPOINTNODEC) {
        return SpellPointSequence();
    }

    // 0x729AE4 (jumptable case 0x2e). UseItemPoint(97): straight to the
    // point-use executor, no target resolution.
    if (m_curAction.m_actionID == 97) {
        return UseItemPoint();
    }

    if (m_curAction.m_actionID == 8) {
        // Dialogue(O:Object*): approach the target sprite and start talking.
        SHORT actionReturn = ACTION_DONE;
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if (pObj->GetObjectType() == CGameObject::TYPE_SPRITE) {
                CGameSprite* pTarget = static_cast<CGameSprite*>(pObj);
                // Skip targets whose base or derived state blocks conversation
                // (STATE bit 0x800).
                if (!(pTarget->m_baseStats.m_generalState & 0x800)
                    && !(pTarget->m_derivedStats.m_generalState & 0x800)) {
                    actionReturn = Dialogue(pTarget);
                }
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        // DEFERRED: when no target resolves, the binary (0x729296..0x729378)
        // walks the party list for an eligible talker.
        return actionReturn;
    }

    if (m_curAction.m_actionID == 18) {
        // Hide() (ACTION.IDS 18, jumptable case at 0x72956F).  The instant the
        // queued Hide() action runs -- before the modal upkeep's next three-cycle
        // tick -- run one stealth detection pass so the sprite hides (or is
        // spotted) right away.  Skipped once already hidden; the per-cycle
        // re-check then lives in CheckModal case 2.
        if (!m_bHiding) {
            return sub_757B40();
        }
        return ACTION_DONE;
    }

    if (m_curAction.m_actionID == 0x8B) {
        // PlayerDialog(O:Target*): player-initiated approach + talk (the action
        // queued when the player clicks an NPC with the talk cursor). Binary
        // dispatch block 0x72A3AB.
        SHORT actionReturn = ACTION_DONE;
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_SPRITE);
        if (pObj != NULL) {
            if (pObj->GetObjectType() == CGameObject::TYPE_SPRITE) {
                actionReturn = PlayerDialog(static_cast<CGameSprite*>(pObj));
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
        return actionReturn;
    }

    if (m_curAction.m_actionID == 0xE5) {
        // FaceObject(O:Object*) used by cutscenes.
        SHORT actionReturn = ACTION_ERROR;
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        if (pObj != NULL) {
            if ((pObj->GetObjectType() & CGameObject::TYPE_AIBASE) != 0) {
                actionReturn = FaceObject(static_cast<CGameAIBase*>(pObj));
            }
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        return actionReturn;
    }

    if (m_curAction.m_actionID == 0x100) {
        // SetDialogueRange(I:Range*) sets the explicit squared-range override
        // consumed by Dialogue().
        field_7106 = m_curAction.GetSpecifics();
        return ACTION_DONE;
    }

    // 0x729032
    if (m_curAction.m_actionID == CAIAction::MOVETOPOINT
        || m_curAction.m_actionID == CAIAction::MOVETOPOINTNORECTICLE
        || m_curAction.m_actionID == CAIAction::TIMEDMOVETOPOINT) {
        POSITION pos = m_queuedActions.GetHeadPosition();
        ResolveTargetPoint(&m_curAction, pos);
        return MoveToPoint();
    }

    // 0x729C2C
    if (m_curAction.m_actionID == CAIAction::LEADER) {
        m_followLeader = TRUE;
        m_followLeaderAdditive = (BOOL)m_curAction.m_specificID;

        CAIAction moveAction(CAIAction::MOVETOPOINT, m_curAction.m_dest, -1, -1);
        AddAction(moveAction);

        return ACTION_DONE;
    }

    // 0x729CE2
    if (m_curAction.m_actionID == CAIAction::FOLLOW) {
        return Follow();
    }

    // Face(I:Direction): the formation move (CAIGroup::GroupSetTarget) queues a
    // FACE after the MOVETOPOINT so members turn to the drag/formation direction
    // on arrival. Binary 0x729032 dispatches it to Face(); without this the
    // action falls through to the base no-op and members keep their travel facing.
    if (m_curAction.m_actionID == CAIAction::FACE) {
        return Face();
    }

    // 0x72A19E (ExecuteAction jumptable case 0x3A). UseContainer(O:Object*): the
    // leader, having walked to the pile via the queued MoveToPoint, opens it.
    if (m_curAction.m_actionID == CAIAction::USECONTAINER) {
        return UseContainer();
    }

    // 0x7290D2 (ExecuteAction jumptable case 0x55). RandomWalk(): ambient
    // wandering for area creatures (Targos villagers etc.).
    if (m_curAction.m_actionID == CAIAction::RANDOMWALK) {
        return RandomWalk();
    }

    // 0x72AA61 (jumptable cases 0xED/0x106). ReturnToSavedLocation(I:Tolerance)
    // and the Delete variant: walk back to the saved location, face the saved
    // direction on arrival.
    if (m_curAction.m_actionID == 0xED || m_curAction.m_actionID == 0x106) {
        return ReturnToSavedLocation();
    }

    // 0x729A3C (jumptable case 0x136). SetTeamBit(I:TeamFlag*TeamBit,I:Value*BOOLEAN):
    // sets/clears the team-allegiance bit in field_58C; IsTeamBitOn (0x40E4) reads
    // it back. 10ONBOAT re-runs its SetTeamBit block every AI round until the bit
    // sticks, and each re-run's script-instant restore resets m_curDest, which
    // kept re-issuing the wander path search forever.
    if (m_curAction.m_actionID == 0x136) {
        if (m_curAction.m_specificID2 != 0) {
            field_58C |= m_curAction.m_specificID;
        } else {
            field_58C &= ~m_curAction.m_specificID;
        }
        return ACTION_DONE;
    }

    // 0x72AD26 (jumptable case 0x10D). SetStartPos(P:Point*): re-anchor
    // m_posStart; (-1,-1) means "here".
    if (m_curAction.m_actionID == 0x10D) {
        CPoint pt = m_curAction.m_dest;
        if (pt.x == -1 && pt.y == -1) {
            pt = m_pos;
        }
        m_posStart = pt;
        return ACTION_DONE;
    }

    // 0x7296AE (ExecuteAction jumptable case 0x17). RemoveTraps(O:Trap*): resolve
    // the targeted trap object and attempt the disarm.  RemoveTraps() is called
    // unconditionally -- it handles a NULL target -- and the shared lock is
    // released only when a target actually resolved.
    if (m_curAction.m_actionID == CAIAction::REMOVETRAPS) {
        CGameObject* pObj = ResolveActionTarget(CGameObject::TYPE_AIBASE);
        SHORT actionReturn = RemoveTraps(static_cast<CGameAIBase*>(pObj));
        if (pObj != NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(
                pObj->m_id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
        return actionReturn;
    }

    // 0x7295D5 (jumptable case 0x28; the index byte at 0x72B520+0x28 routes
    // action 91 here). LeaveArea(): the map-edge trigger's party-gather +
    // world-map handoff.
    if (m_curAction.m_actionID == CAIAction::LEAVEAREA) {
        return LeaveArea();
    }

    return CGameAIBase::ExecuteAction();
}

// 0x7555F0
SHORT CGameSprite::UseContainer()
{
    SHORT actionReturn = ACTION_INTERRUPTABLE;

    if (m_actionCount >= 1) {
        return actionReturn;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    BOOL bEligible = pGame->GetCharacterPortraitNum(m_id) != -1;
    if (!bEligible) {
        actionReturn = ACTION_DONE;
        bEligible = pGame->IsFamiliar(m_id);
    }
    if (!bEligible || !Orderable(FALSE) || !m_bSelected) {
        return actionReturn;
    }

    actionReturn = ACTION_ERROR;

    // The container is the current action's actee (set when the click queued the
    // UseContainer action). Share it by id directly -- ResolveActionTarget() runs
    // type/immunity matching that is not valid for a container and faults.
    LONG nContainerId = m_curAction.m_acteeID.GetInstance();
    CGameObjectArray* pArray = pGame->GetObjectArray();

    CGameObject* pContainer;
    if (pArray->GetShare(nContainerId, CGameObjectArray::THREAD_ASYNCH, &pContainer, INFINITE)
        != CGameObjectArray::SUCCESS) {
        return actionReturn;
    }

    // NOTE: the original broadcasts two MP-sync messages here (the container-touch
    // trigger + a marker message); SP-inert, deferred.

    CPoint posContainer = pContainer->GetPos();
    CPoint posSelf = GetPos();
    INT dx = posContainer.x - posSelf.x;
    INT dy = posContainer.y - posSelf.y;
    INT nRange = CPathSearch::GRID_SQUARE_SIZEX * CPathSearch::GRID_SQUARE_SIZEX * 16;

    if (nRange < (dy * dy * 16) / 9 + dx * dx) {
        // Not adjacent yet -- the queued MoveToPoint is walking us in.
        pArray->ReleaseShare(nContainerId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        return ACTION_DONE;
    }

    // NOTE: the original drops the actor's invisibility (CGameEffect opcode 0x88)
    // here before opening; deferred.

    if (pContainer->GetObjectType() == CGameObject::TYPE_CONTAINER) {
        CGameContainer* pCont = static_cast<CGameContainer*>(pContainer);
        if ((pCont->m_dwFlags & 0x1) != 0
            && (pCont->m_keyType == "" || !PartyHasItem(pCont->m_keyType))) {
            // Locked, and the party is not carrying its key.
            DisplayTextRef(-1, 0x4D96, 0, RGB(0xD7, 0xD7, 0xBE));
            pArray->ReleaseShare(nContainerId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            return ACTION_DONE;
        }
        pCont->OpenContainer(m_typeAI);
    }

    pArray->ReleaseShare(nContainerId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

    if (pGame->GetCharacterPortraitNum(m_id) == -1) {
        DisplayTextRef(-1, 0x1D7A, 0xFF, 0xFF);
    } else {
        pGame->m_iContainer = nContainerId;
        pGame->m_iContainerSprite = m_id;
        g_pBaldurChitin->GetScreenWorld()->ConfigureContainer();
    }

    ClearActions(FALSE);

    return ACTION_INTERRUPTABLE;
}

static void SetDialogueTalkedTo(CGameSprite* pSource, CGameSprite* pTarget)
{
    pSource->m_lTalkedTo.Set(pTarget->GetAIType());
    pTarget->m_lTalkedTo.Set(pSource->GetAIType());
}

static void QueueSetNumTimesTalkedTo(CGameSprite* pSource, CGameSprite* pSprite)
{
    if ((pSource->m_derivedStats.m_generalState & 0x2000) == 0
        && pSprite->GetAIType().m_nEnemyAlly != CAIObjectType::EA_PC) {
        CMessage* pMessage = new CMessageSetNumTimesTalkedTo(
            pSprite->m_nNumberOfTimesTalkedTo + 1,
            pSource->GetId(),
            pSprite->GetId());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }
}

static void QueueClearDialogActions(CGameSprite* pTarget)
{
    CMessage* pMessage = new CMessageClearDialogActions(
        pTarget->GetId(),
        pTarget->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
}

static void QueueDialogueAddAction(CGameSprite* pSource, CGameSprite* pTarget, const CAIObjectType& actee)
{
    static const SHORT DIALOGUE_ADD_ACTION_ID = 7;

    CAIAction action(DIALOGUE_ADD_ACTION_ID, actee, 0, 0, 0);
    CMessage* pMessage = new CMessageAddAction(
        action,
        pSource->GetId(),
        pTarget->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
}

static void QueueDialogueBreakEffect(CGameSprite* pSprite, WORD effectID)
{
    ITEM_EFFECT effect;
    CGameEffect::ClearItemEffect(&effect, effectID);
    effect.durationType = 1;

    CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
        pSprite->GetPos(),
        pSprite->GetId(),
        CPoint(-1, -1));

    CMessage* pMessage = new CMessageAddEffect(
        pEffect,
        pSprite->GetId(),
        pSprite->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
}

static void QueuePlayerDialogBreakEffects(CGameSprite* pSource)
{
    if (!pSource->CheckInvisibility(FALSE)) {
        QueueDialogueBreakEffect(pSource, CGAMEEFFECT_FORCEVISIBLE);
    }

    if (pSource->m_derivedStats.m_spellStates[SPLSTATE_SANCTUARY]) {
        QueueDialogueBreakEffect(pSource, CGAMEEFFECT_DISPELSANCTUARY);
    }
}

static void QueueDialogueStartedMessages(CGameSprite* pSource, CGameSprite* pTarget)
{
    QueueClearDialogActions(pTarget);
    QueueSetNumTimesTalkedTo(pSource, pSource);
    QueueSetNumTimesTalkedTo(pSource, pTarget);
    QueueDialogueAddAction(pSource, pSource, pTarget->GetAIType());
    QueueDialogueAddAction(pSource, pTarget, pSource->GetAIType());
}

// 0x752DD0
SHORT CGameSprite::Dialogue(CGameSprite* pTarget)
{
    if (pTarget == NULL) {
        return ACTION_ERROR;
    }

    if (!CanSpeak(FALSE, FALSE)) {
        return ACTION_ERROR;
    }

    if (!pTarget->CanSpeak(FALSE, TRUE)) {
        return ACTION_ERROR;
    }

    if (m_derivedStats.m_generalState & 0x2000) {
        // Caller is in a state that forbids initiating conversation.
        return ACTION_ERROR;
    }

    // DEFERRED (multiplayer): the "multig" portrait-assignment block at binary
    // 0x752E33..0x752E7E hands an unportraited multiplayer character a default
    // portrait before talking.  Single-player never enters it.

    if (m_actionCount == 0) {
        PlayDialogSound(this);
    }

    CPoint targetPos = pTarget->GetPos();
    CPoint selfPos = GetPos();
    CPoint targetCell(targetPos.x / CPathSearch::GRID_SQUARE_SIZEX,
        targetPos.y / CPathSearch::GRID_SQUARE_SIZEY);
    CPoint selfCell(selfPos.x / CPathSearch::GRID_SQUARE_SIZEX,
        selfPos.y / CPathSearch::GRID_SQUARE_SIZEY);
    LONG distSquares = CAIUtil::CountSquares(selfCell, targetCell);
    distSquares = distSquares * distSquares;

    // field_7106 is the dialogue-range override set by SetDialogueRange (action
    // 0x100); 0 means "derive the range from the combined personal space".
    LONG range = field_7106;
    if (range == 0) {
        BYTE selfSpace = m_animation.GetPersonalSpace();
        BYTE targetSpace = pTarget->m_animation.GetPersonalSpace();
        range = ((targetSpace - 1) >> 1) - 1 + ((selfSpace - 1) >> 1);
    } else {
        range = range * range;
    }

    BOOL bEnter = FALSE;
    if (field_7106 != 0) {
        // Explicit range: enter on distance alone, otherwise keep moving.
        bEnter = distSquares <= range;
    } else {
        range = (range + 2) * (range + 2);
        if (distSquares <= range
            && m_pArea->CheckLOS(targetPos, selfPos, GetVisibleTerrainTable(), Orderable(FALSE))) {
            bEnter = TRUE;
        }
    }

    if (bEnter) {
        // In range (binary LAB_00752FF0).
        SetDialogueTalkedTo(this, pTarget);
        if (!g_pBaldurChitin->m_pEngineWorld->StartDialog(pTarget, this, 0, 0)) {
            return ACTION_ERROR;
        }
        QueueDialogueStartedMessages(this, pTarget);
        return ACTION_DONE;
    }

    // Out of range: step toward the target and re-evaluate next tick.
    SHORT result = MoveToObject(pTarget);
    if (result == ACTION_DONE) {
        result = ACTION_INTERRUPTABLE;
    }
    if (result == ACTION_ERROR
        && m_pArea->CheckLOS(targetPos, selfPos, GetVisibleTerrainTable(), Orderable(FALSE))) {
        // Pathing failed but the target is visible -- talk from here anyway.
        SetDialogueTalkedTo(this, pTarget);
        if (g_pBaldurChitin->m_pEngineWorld->StartDialog(pTarget, this, 0, 0)) {
            QueueDialogueStartedMessages(this, pTarget);
            result = ACTION_DONE;
        } else {
            result = ACTION_ERROR;
        }
    }
    return result;
}

// 0x7537A0
SHORT CGameSprite::PlayerDialog(CGameSprite* pTarget)
{
    if (pTarget == NULL) {
        return ACTION_ERROR;
    }
    if (pTarget->m_bEscapingArea) {
        return ACTION_ERROR;
    }
    if (pTarget->m_curAction.m_actionID == 0x6C || pTarget->m_curAction.m_actionID == 0xB0) {
        return ACTION_ERROR;
    }
    if (m_currentActionId == 0x6C || m_currentActionId == 0xB0) {
        return ACTION_ERROR;
    }
    if (!CanSpeak(FALSE, FALSE)) {
        return ACTION_ERROR;
    }
    if (!pTarget->CanSpeak(FALSE, TRUE)) {
        return ACTION_ERROR;
    }
    if (pTarget->m_moraleFailure) {
        return ACTION_ERROR;
    }

    DWORD targetState = pTarget->m_derivedStats.m_generalState;
    if (targetState & 0xC) {
        return ACTION_ERROR;
    }
    if ((targetState & 2) && m_berserkActive) {
        return ACTION_ERROR;
    }
    if (targetState & 0x80100000) {
        return ACTION_ERROR;
    }

    if (m_actionCount == 0) {
        PlayDialogSound(pTarget);
    }

    CPoint targetPos = pTarget->GetPos();
    CPoint selfPos = GetPos();
    CPoint targetCell(targetPos.x / CPathSearch::GRID_SQUARE_SIZEX,
        targetPos.y / CPathSearch::GRID_SQUARE_SIZEY);
    CPoint selfCell(selfPos.x / CPathSearch::GRID_SQUARE_SIZEX,
        selfPos.y / CPathSearch::GRID_SQUARE_SIZEY);
    LONG distSquares = CAIUtil::CountSquares(selfCell, targetCell);
    distSquares = distSquares * distSquares;

    BYTE selfSpace = m_animation.GetPersonalSpace();
    BYTE targetSpace = pTarget->m_animation.GetPersonalSpace();
    BYTE range = static_cast<BYTE>(((targetSpace - 1) >> 1) + ((selfSpace - 1) >> 1) - 1);

    BOOL inRange = distSquares <= (range + 2) * (range + 2);
    BOOL hasLOS = m_pArea->CheckLOS(targetPos, selfPos, GetVisibleTerrainTable(), Orderable(FALSE));

    if (inRange && hasLOS) {
        QueuePlayerDialogBreakEffects(this);
        SetDialogueTalkedTo(this, pTarget);
        BOOL started = g_pBaldurChitin->m_pEngineWorld->StartDialog(this, pTarget, 1, 0);
        if (!started) {
            return ACTION_ERROR;
        }
        QueueDialogueStartedMessages(this, pTarget);
        return ACTION_DONE;
    }

    // Out of range: step toward the target and re-evaluate next tick.
    SHORT result = MoveToObject(pTarget);
    if (result == ACTION_DONE) {
        result = ACTION_INTERRUPTABLE;
    }
    if (result == ACTION_ERROR
        && m_pArea->CheckLOS(targetPos, selfPos, GetVisibleTerrainTable(), Orderable(FALSE))) {
        QueuePlayerDialogBreakEffects(this);
        SetDialogueTalkedTo(this, pTarget);
        BOOL started = g_pBaldurChitin->m_pEngineWorld->StartDialog(this, pTarget, 1, 0);
        if (started) {
            QueueDialogueStartedMessages(this, pTarget);
            result = ACTION_DONE;
        } else {
            result = ACTION_ERROR;
        }
    }
    return result;
}

// 0x7446F0
SHORT CGameSprite::OneSwing()
{
    SHORT returnValue = ACTION_NORMAL;
    CMessage* message;

    if (m_attackFrame == 0) {
        m_speedFactor = 5;
        m_speedFactor += rand() % 6;
        m_speedFactor -= 3;
        if (m_speedFactor < 0) {
            m_speedFactor = 0;
        } else if (m_speedFactor > 10) {
            m_speedFactor = 10;
        }
    }

    BYTE frameType;
    if (m_derivedStats.m_nNumberOfAttacks > 0) {
        frameType = m_animation.GetAttackFrameType(static_cast<BYTE>(m_derivedStats.m_nNumberOfAttacks),
            static_cast<BYTE>(m_speedFactor),
            static_cast<BYTE>(m_attackFrame));
    } else {
        frameType = 0;
    }

    switch (frameType) {
    case 0:
        if (m_nSequence == GetIdleSequence()) {
            message = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()), m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        returnValue = ACTION_INTERRUPTABLE;
        break;
    case 6:
        if (m_nSequence != SEQ_ATTACK) {
            message = new CMessageSetSequence(SEQ_ATTACK, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        break;
    case 7:
        if (m_nSequence != SEQ_ATTACK) {
            message = new CMessageSetSequence(SEQ_ATTACK, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        returnValue = ACTION_NORMAL;
        break;
    case 9:
        if (m_nSequence != SEQ_ATTACK) {
            message = new CMessageSetSequence(SEQ_ATTACK, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
        returnValue = ACTION_NORMAL;
        break;
    case 15:
        returnValue = ACTION_DONE;
        break;
    }

    return returnValue;
}

// 0x7449D0
SHORT CGameSprite::Recoil()
{
    CMessage* message;

    if (m_recoilFrame == 0 && m_baseStats.m_hitPoints == 0) {
        m_endOfDamageSeq = FALSE;
        m_recoilFrame = 0;
        return ACTION_DONE;
    }

    if (m_nSequence != SEQ_DAMAGE) {
        message = new CMessageSetSequence(SEQ_DAMAGE, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    m_recoilFrame++;
    if (!m_endOfDamageSeq) {
        return ACTION_NORMAL;
    }

    m_endOfDamageSeq = FALSE;
    m_recoilFrame = 0;

    message = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()), m_id, m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    return ACTION_DONE;
}

// 0x744B20
SHORT CGameSprite::PlayDead()
{
    CMessage* message;

    if (m_nSequence != SEQ_DIE || m_nSequence != SEQ_TWITCH) {
        message = new CMessageSetSequence(SEQ_DIE, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    if (m_actionCount == 0) {
        PlaySound(SOUND_DYING, TRUE, FALSE, FALSE);
    }

    if (m_curAction.m_specificID > 0) {
        m_curAction.m_specificID--;
        return ACTION_NORMAL;
    }

    message = new CMessageSetSequence(SEQ_AWAKE, m_id, m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    return ACTION_DONE;
}

// 0x745950
SHORT CGameSprite::JumpToPoint(CPoint dest, BOOL spriteUpdate)
{
    if (m_pArea == NULL) {
        return ACTION_DONE;
    }

    CPoint gridDest(dest.x / CPathSearch::GRID_SQUARE_SIZEX,
        dest.y / CPathSearch::GRID_SQUARE_SIZEY);

    if (m_pPath != NULL) {
        CMessage* pDropPath = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pDropPath, FALSE);
    }

    switch (m_listType) {
    case CGAMEOBJECT_LIST_FRONT:
        if (m_active && m_activeAI && m_activeImprisonment) {
            m_pArea->m_search.RemoveObject(
                CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                    m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
        }
        break;
    case CGAMEOBJECT_LIST_BACK:
        if (m_active && m_activeAI && m_activeImprisonment
            && (m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
            m_pArea->m_search.RemoveObject(
                CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                    m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
        }
        break;
    case CGAMEOBJECT_LIST_FLIGHT:
        break;
    default:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 0x39E6
        UTIL_ASSERT(FALSE);
        break;
    }

    BYTE personalSpace = m_animation.GetPersonalSpace();
    POINT adjustedGoal = { gridDest.x, gridDest.y };
    m_pArea->AdjustTarget(gridDest, &adjustedGoal, personalSpace, -1);
    CPoint adjustedGrid(adjustedGoal.x, adjustedGoal.y);

    if (adjustedGrid.x == dest.x / CPathSearch::GRID_SQUARE_SIZEX
        && adjustedGrid.y == dest.y / CPathSearch::GRID_SQUARE_SIZEY) {
        m_pos = dest;
    } else {
        m_pos.x = adjustedGrid.x * CPathSearch::GRID_SQUARE_SIZEX;
        m_pos.y = adjustedGrid.y * CPathSearch::GRID_SQUARE_SIZEY;
    }

    m_posExact.x = m_pos.x << EXACT_SCALE;
    m_posExact.y = (m_pos.y << (EXACT_SCALE + 2)) / 3;

    InitBlur();

    switch (m_listType) {
    case CGAMEOBJECT_LIST_FRONT:
        if (m_active && m_activeAI && m_activeImprisonment) {
            m_pArea->m_search.AddObject(
                CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                    m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
        }
        break;
    case CGAMEOBJECT_LIST_BACK:
        if (m_active && m_activeAI && m_activeImprisonment
            && (m_derivedStats.m_generalState & STATE_SLEEPING) != 0) {
            m_pArea->m_search.AddObject(
                CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                    m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                m_typeAI.GetEnemyAlly(),
                m_animation.GetPersonalSpace(),
                m_bBumpable,
                m_bOnSearchMap);
        }
        break;
    }

    SHORT nPortrait = g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id);
    if (nPortrait != -1
        && (!g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_bArenaMode || InControl())) {
        m_pArea->m_visibility.UpDate(m_posLastVisMapEntry, m_pos, m_id, m_visibleTerrainTable);
        m_posLastVisMapEntry = m_pos;
    }

    if (spriteUpdate
        && (!g_pChitin->cNetwork.GetSessionOpen() || InControl())) {
        CMessage* pUpdate = new CMessageSpriteUpdate(this, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pUpdate, FALSE);
    }

    return ACTION_DONE;
}

// 0x7462D0
SHORT CGameSprite::Face()
{
    SetDirection(static_cast<SHORT>(m_curAction.m_specificID));

    if (m_pos.x != -1 || m_pos.y != -1 || m_pArea != NULL) {
        if (InControl()) {
            CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x746390
SHORT CGameSprite::FaceObject(CGameAIBase* pObject)
{
    if (pObject == NULL) {
        return ACTION_ERROR;
    }

    SetDirection(GetDirection(pObject->GetPos()));

    if (m_pos.x != -1 || m_pos.y != -1 || m_pArea != NULL) {
        if (InControl()) {
            CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x748CA0
SHORT CGameSprite::RandomWalk()
{
    CAIObjectType cType(0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0);
    cType.Set(CAIObjectType::ANYONE);

    DWORD nCreFlags = m_baseStats.m_flags;

    if (m_interrupt) {
        return ACTION_INTERRUPTABLE;
    }

    // Re-queue the walk itself so the wandering repeats indefinitely.
    CAIAction repeatAction(m_curAction.m_actionID, CPoint(-1, -1), 0, -1);
    AddAction(repeatAction);

    CRect rView;
    CopyRect(&rView, &m_pArea->m_cInfinity.rViewPort);

    UINT nWidth = rView.right - rView.left;
    if (m_huntingRange != 0 && 2 * m_huntingRange < static_cast<INT>(nWidth)) {
        nWidth = m_huntingRange;
    }

    UINT nHeight = rView.bottom - rView.top;
    if (m_huntingRange != 0 && 2 * m_huntingRange < static_cast<INT>(nHeight)) {
        nHeight = m_huntingRange;
    }

    // CRE flags bits 24-30 select which of our own identity components define
    // the herd this walk drifts towards.
    BOOL bHerd = FALSE;
    if ((nCreFlags & 0x1000000) != 0) {
        cType.m_nEnemyAlly = m_typeAI.m_nEnemyAlly;
        bHerd = TRUE;
    }
    if ((nCreFlags & 0x2000000) != 0) {
        cType.m_nGeneral = m_typeAI.m_nGeneral;
        bHerd = TRUE;
    }
    if ((nCreFlags & 0x4000000) != 0) {
        cType.m_nRace = m_typeAI.m_nRace;
        bHerd = TRUE;
    }
    if ((nCreFlags & 0x8000000) != 0) {
        cType.m_nClass = m_typeAI.m_nClass;
        bHerd = TRUE;
    }
    if ((nCreFlags & 0x10000000) != 0) {
        cType.m_nSpecific = m_typeAI.m_nSpecific;
        bHerd = TRUE;
    }
    if ((nCreFlags & 0x20000000) != 0) {
        cType.m_nGender = m_typeAI.m_nGender;
        bHerd = TRUE;
    }
    if ((nCreFlags & 0x40000000) != 0) {
        cType.m_nAlignment = m_typeAI.m_nAlignment;
        bHerd = TRUE;
    }

    INT nHalfWidth = static_cast<INT>(nWidth) >> 1;
    INT nHalfHeight = static_cast<INT>(nHeight) >> 1;

    INT nDriftX = 0;
    INT nDriftY = 0;

    if (bHerd) {
        LONG nNearestId = m_pArea->GetNearest(m_id,
            cType,
            GetVisualRange(),
            GetVisibleTerrainTable(),
            TRUE,
            GetCanSeeInvisible(),
            FALSE,
            0,
            FALSE);

        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nNearestId,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS && pObject != NULL) {
            LONG dx = pObject->GetPos().x - m_pos.x;
            nDriftX = dx < 0 ? -dx : dx;
            if (nHalfWidth <= nDriftX) {
                nDriftX = nHalfWidth;
            }
            if (dx < 0) {
                nDriftX = -nDriftX;
            }

            LONG dy = pObject->GetPos().y - m_pos.y;
            nDriftY = dy < 0 ? -dy : dy;
            if (nHalfHeight <= nDriftY) {
                nDriftY = nHalfHeight;
            }
            if (dy < 0) {
                nDriftY = -nDriftY;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nNearestId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    BYTE nRounds = 0;
    BYTE nAttempts = 0;
    INT x = 0;
    INT y = 0;

    while (TRUE) {
        BYTE nCost;
        LONG nOffsetDistSq;

        do {
            INT nOffsetX = (nWidth != 0 ? rand() % static_cast<INT>(nWidth) : 0) - nHalfWidth;
            INT nOffsetY = (nHeight != 0 ? rand() % static_cast<INT>(nHeight) : 0) - nHalfHeight;
            nOffsetDistSq = nOffsetX * nOffsetX + nOffsetY * nOffsetY;

            // Anchor on the spawn point once we have wandered out of range of it.
            INT nAnchorX;
            INT nAnchorY;
            if (m_huntingRange == 0
                || (m_pos.x - m_posStart.x) * (m_pos.x - m_posStart.x)
                        + (m_pos.y - m_posStart.y) * (m_pos.y - m_posStart.y)
                    < m_huntingRange) {
                nAnchorX = m_pos.x;
                nAnchorY = m_pos.y;
            } else {
                nAnchorX = m_posStart.x;
                nAnchorY = m_posStart.y;
            }

            x = nOffsetX + nAnchorX + nDriftX;
            y = nOffsetY + nAnchorY + nDriftY;
            if (x < 0) {
                x = 0;
            }
            if (y < 0) {
                y = 0;
            }
            if (m_pArea->m_cInfinity.nAreaX <= x) {
                x = m_pArea->m_cInfinity.nAreaX - 1;
            }
            if (m_pArea->m_cInfinity.nAreaY <= y) {
                y = m_pArea->m_cInfinity.nAreaY - 1;
            }

            CPoint ptSearch(x / CPathSearch::GRID_SQUARE_SIZEX,
                y / CPathSearch::GRID_SQUARE_SIZEY);

            SHORT nTableIndex;
            nCost = m_pArea->m_search.GetCost(ptSearch,
                GetTerrainTable(),
                m_animation.GetPersonalSpace(),
                nTableIndex,
                TRUE);

            nAttempts++;
        } while (nCost == CPathSearch::COST_IMPASSABLE && nAttempts < 100);

        if (m_posStart.x == -1 && m_posStart.y == -1) {
            nOffsetDistSq = -1;
        }

        nRounds++;
        if (m_huntingRange == 0
            || nOffsetDistSq <= static_cast<LONG>(static_cast<DWORD>(m_huntingRange) * m_huntingRange)
            || nRounds > 19) {
            break;
        }

        nAttempts = 0;
    }

    CAIAction moveAction(CAIAction::MOVETOPOINT, CPoint(x, y), 0, -1);
    AddAction(moveAction);

    return ACTION_DONE;
}

// 0x75F270
SHORT CGameSprite::ReturnToSavedLocation()
{
    if (m_baseStats.m_savedLocationX != 0 && m_baseStats.m_savedLocationY != 0) {
        CPoint dest;
        if (m_curAction.m_actionID == 0x12E) {
            dest = m_pos;
        } else {
            dest.x = m_baseStats.m_savedLocationX;
            dest.y = m_baseStats.m_savedLocationY;
        }

        SHORT actionReturn = MoveToPointRange(dest, m_curAction.m_specificID);
        if (actionReturn != ACTION_DONE && actionReturn != ACTION_ERROR) {
            m_lastActionID = CAIAction::MOVETOPOINT;
            return actionReturn;
        }

        SetDirection(m_baseStats.m_savedLocationFacing);

        if (m_pos.x != -1 || m_pos.y != -1 || m_pArea != NULL) {
            CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        if (m_curAction.m_actionID == 0x106) {
            // ReturnToSavedLocationDelete(): leave the area on arrival.
            m_removeFromArea = TRUE;
        }

        return actionReturn;
    }

    return ACTION_DONE;
}

// 0x751CD0
SHORT CGameSprite::LeaveParty()
{
    // TODO: Incomplete.

    return ACTION_DONE;
}

// 0x74F6D0
void CGameSprite::MoveGlobal(const CString& sArea, const CPoint& ptStart)
{
    // TODO: Incomplete.
}

// 0x74FC50
SHORT CGameSprite::GroupAttack(CGameSprite* pTarget)
{
    if (pTarget == NULL
        || !pTarget->m_active
        || !pTarget->m_activeAI
        || !pTarget->m_activeImprisonment) {
        return ACTION_DONE;
    }

    if (m_interrupt) {
        return ACTION_INTERRUPTABLE;
    }

    CAIObjectType typeAI(GetAIType());
    typeAI.SetName(CString(""));
    typeAI.SetInstance(-1);
    m_curAction.m_acteeID.Set(typeAI);
    AddAction(m_curAction);
    AddAction(CAIAction(CAIAction::ATTACK, m_curAction.m_acteeID, 0, 0, 0));

    return ACTION_DONE;
}

// 0x750DA0
SHORT CGameSprite::Enemy()
{
    m_typeAI.SetEnemyAlly(CAIObjectType::EA_ENEMY);
    m_liveTypeAI.SetEnemyAlly(CAIObjectType::EA_ENEMY);
    m_startTypeAI.SetEnemyAlly(CAIObjectType::EA_ENEMY);

    if (InControl()) {
        CMessage* message = new CMessageSpriteUpdate(this, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    m_equipedEffectList.RemoveAllOfType(this, CGAMEEFFECT_CHARM, m_equipedEffectList.m_posCurrent, -1);
    m_equipedEffectList.RemoveAllOfType(this, ICEWIND_CGAMEEFFECT_CHARM, m_equipedEffectList.m_posCurrent, -1);
    m_timedEffectList.RemoveAllOfType(this, CGAMEEFFECT_CHARM, m_timedEffectList.m_posCurrent, -1);
    m_timedEffectList.RemoveAllOfType(this, ICEWIND_CGAMEEFFECT_CHARM, m_timedEffectList.m_posCurrent, -1);
    m_timedEffectList.RemoveAllOfType(this, CGAMEEFFECT_SETAISCRIPT, m_timedEffectList.m_posCurrent, -1);
    m_equipedEffectList.RemoveAllOfType(this, CGAMEEFFECT_SETAISCRIPT, m_equipedEffectList.m_posCurrent, -1);
    g_pBaldurChitin->GetObjectGame()->RemoveCharacterFromAllies(m_id);
    g_pBaldurChitin->GetObjectGame()->RemoveCharacterFromFamiliars(m_id);

    return ACTION_DONE;
}

// 0x754870
SHORT CGameSprite::SetDialog()
{
    m_dialog = m_curAction.GetString1();

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        CMessage* message = new CMessageSetDialogResRef(CResRef(m_curAction.GetString1()), m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    return ACTION_DONE;
}

// 0x74F830
void CGameSprite::SelectWeaponAbility(unsigned char slotNum, unsigned char abilityNum, unsigned char checkCursed, unsigned char retry)
{
    BYTE oldSlot = m_equipment.m_selectedWeapon;
    BYTE oldAbility = static_cast<BYTE>(m_equipment.m_selectedWeaponAbility);

    if (oldSlot < CGameSpriteEquipment::NUM_SLOT && m_equipment.m_items[oldSlot] != NULL) {
        CItem* pOldItem = m_equipment.m_items[oldSlot];
        if ((pOldItem->GetFlagsFile() & 0x10) != 0 && checkCursed != 0) {
            // Original displays feedback 0xC here.
            return;
        }

        pOldItem->Demand();
        ITEM_ABILITY* pOldAbility = pOldItem->pRes != NULL ? pOldItem->pRes->GetAbility(m_equipment.m_selectedWeaponAbility) : NULL;
        SHORT launcherSlot;
        CItem* pLauncher = GetLauncher(pOldAbility, launcherSlot);
        if (pLauncher != NULL) {
            pLauncher->Unequip(this, launcherSlot, TRUE, FALSE);
        }
        pOldItem->Unequip(this, oldSlot, TRUE, FALSE);
        pOldItem->Release();
    }

    m_equipment.m_selectedWeapon = slotNum;
    m_nTempSelectedWeapon = slotNum;
    m_nTempSelectedWeaponAbility = abilityNum;
    m_equipment.m_selectedWeaponAbility = abilityNum;

    BOOL bEquipped = FALSE;
    CItem* pItem = slotNum < CGameSpriteEquipment::NUM_SLOT ? m_equipment.m_items[slotNum] : NULL;
    if (pItem != NULL) {
        pItem->Demand();
        ITEM_ABILITY* pAbility = pItem->pRes != NULL ? pItem->pRes->GetAbility(m_equipment.m_selectedWeaponAbility) : NULL;
        if (pAbility != NULL) {
            if ((pAbility->type & 0xFF) == 4) {
                for (BYTE ammoSlot = CGameSpriteEquipment::SLOT_AMMO; ammoSlot < CGameSpriteEquipment::SLOT_AMMO + 4; ammoSlot++) {
                    CItem* pAmmo = m_equipment.m_items[ammoSlot];
                    if (pAmmo == NULL) {
                        continue;
                    }

                    pAmmo->Demand();
                    for (BYTE ammoAbility = 0; ammoAbility < 3; ammoAbility++) {
                        ITEM_ABILITY* pAmmoAbility = pAmmo->pRes != NULL ? pAmmo->pRes->GetAbility(ammoAbility) : NULL;
                        if (pAmmoAbility != NULL
                            && (pAmmoAbility->type & 0xFF) != 4
                            && CheckLauncherType(pAmmoAbility, pItem)) {
                            pAmmo->Release();
                            pItem->Release();
                            SelectWeaponAbility(ammoSlot, ammoAbility, checkCursed, TRUE);
                            return;
                        }
                    }
                    pAmmo->Release();
                }
            } else if (CheckLauncherType(pAbility, NULL)) {
                SHORT launcherSlot;
                CItem* pLauncher = GetLauncher(pAbility, launcherSlot);
                if (pLauncher != NULL) {
                    pLauncher->Equip(this, launcherSlot, FALSE);
                }
                pItem->Equip(this, m_equipment.m_selectedWeapon, FALSE);
                bEquipped = TRUE;
            }
        }
        pItem->Release();
    }

    if (!bEquipped) {
        if (retry != 0) {
            SelectWeaponAbility(oldSlot, oldAbility, checkCursed, FALSE);
            return;
        }

        if (oldSlot == CGameSpriteEquipment::SLOT_FIST) {
            UTIL_ASSERT_MSG(FALSE, "Failed equiping Weapon: Fist");
        } else {
            SelectWeaponAbility(CGameSpriteEquipment::SLOT_FIST, 0, checkCursed, TRUE);
            return;
        }
    }

    CMessage* pMessage = new CMessageSpriteEquipment(this, m_id, m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    if ((GetActiveStats()->m_generalState & STATE_DEAD) == 0) {
        CMessage* pSequenceMessage = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()), m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pSequenceMessage, FALSE);
    }
}

// 0x756930
void CGameSprite::sub_756930(CItem* pItem, CItem* pLauncher)
{
    // TODO: Incomplete.
}

// 0x7579C0
SHORT CGameSprite::EquipItem()
{
    SHORT slot = FindItemPersonal(m_curAction.GetString1(), 0, FALSE);
    if (slot == -1) {
        return ACTION_ERROR;
    }

    if (m_curAction.GetSpecifics()) {
        m_equipment.m_items[slot]->Equip(this, slot, FALSE);
    } else {
        m_equipment.m_items[slot]->Unequip(this, slot, TRUE, FALSE);
    }

    return ACTION_DONE;
}

// 0x757A80
SHORT CGameSprite::FindTraps()
{
    if (!m_typeAI.IsClassValid(CAIObjectType::C_ROGUE)
        && !m_typeAI.IsClassValid(CAIObjectType::C_MONK)) {
        return ACTION_ERROR;
    }

    SetModalState(2, TRUE);
    ClearActions(FALSE);

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter()) {
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->m_nSelectedButton = 100;
        g_pBaldurChitin->GetObjectGame()->SetState(0);
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->UpdateButtons();
    }

    return ACTION_DONE;
}

// 0x759240
SHORT CGameSprite::RemoveTraps(CGameAIBase* pTarget)
{
    if (pTarget == NULL) {
        return ACTION_ERROR;
    }

    if (pTarget->GetObjectType() != TYPE_DOOR
        && pTarget->GetObjectType() != TYPE_CONTAINER
        && pTarget->GetObjectType() != TYPE_TRIGGER) {
        return ACTION_ERROR;
    }

    CMessage* message = new CMessageSetForceActionPick(TRUE, m_id, pTarget->GetId());
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    // 0x8CF6D8 = g_pChitin; the bytes at +0x1032/+0x1033 are dev toggles (off in
    // normal play).  When both are set the trap may be reached from farther away.
    int searchRange = 0x20;
    if (*(reinterpret_cast<const BYTE*>(g_pChitin) + 0x1032) != 1
        || *(reinterpret_cast<const BYTE*>(g_pChitin) + 0x1033) != 0) {
        searchRange = 0x10;
    }

    CPoint targetPos = pTarget->GetPos();
    int dx = targetPos.x / 16 - m_pos.x / 16;
    int dy = targetPos.y / 12 - m_pos.y / 12;

    SHORT result = ACTION_DONE;
    if (searchRange < dx * dx + dy * dy) {
        m_curAction.m_dest = pTarget->GetPos();
        result = MoveToPointRange(pTarget->GetPos(), 0);
    }

    if (result != ACTION_DONE) {
        return result;
    }

    if (m_pPath != NULL) {
        message = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    int skill = static_cast<char>(m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_DISABLE_DEVICE]);
    int roll = rand() % 20 + 1;
    int skillModifier = GetSkillModifier(CGAMESPRITE_SKILL_DISABLE_DEVICE);
    int check = roll + skill;

    if (pTarget->GetObjectType() == TYPE_DOOR
        && static_cast<CGameDoor*>(pTarget)->m_trapActivated != 0) {
        CGameDoor* pDoor = static_cast<CGameDoor*>(pTarget);
        BOOL bDisarmed = FALSE;
        int dc = pDoor->m_trapDisarmingDifficulty / 7 + 10;

        if (dc < check && pDoor->m_trapDisarmingDifficulty != 100) {
            bDisarmed = TRUE;

            CAITrigger trigger(CAITrigger::DISARMED, m_typeAI, 0);
            message = new CMessageSetTrigger(trigger, m_id, pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            pDoor->m_trapActivated = 0;
            pDoor->SetDrawPoly(0);
            FeedBack(FEEDBACK_ROLL, roll, skill - skillModifier, skillModifier, 0x9962, dc, 0);
            FeedBack(FEEDBACK_TRAPDISARMED, 0, 0, 0, -1, 0, 0);
        } else {
            FeedBack(FEEDBACK_ROLL, roll, skill - skillModifier, skillModifier, 0x9963, dc, 0);

            if (25 < pDoor->m_trapDisarmingDifficulty - dc) {
                CAITrigger trigger(CAITrigger::OPENED, m_typeAI, 0);
                message = new CMessageSetTrigger(trigger, m_id, pDoor->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                if ((pDoor->m_dwFlags & 4) == 0) {
                    pDoor->m_trapActivated = 0;
                }
            }

            CAITrigger trigger(CAITrigger::DISARMFAILED, m_typeAI, 0);
            message = new CMessageSetTrigger(trigger, m_id, pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            FeedBack(FEEDBACK_TRAPDISARMEDFAILED, 0, 0, 0, -1, 0, 0);
        }

        if (bDisarmed) {
            message = new CMessageDoorStatus(pDoor, m_id, pDoor->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    if (pTarget->GetObjectType() == TYPE_TRIGGER
        && static_cast<CGameTrigger*>(pTarget)->m_trapActivated != 0
        && (static_cast<CGameTrigger*>(pTarget)->m_dwFlags & 0x100) == 0) {
        CGameTrigger* pTrigger = static_cast<CGameTrigger*>(pTarget);
        int dc = pTrigger->m_trapDisarmingDifficulty / 7 + 10;

        if (dc < check && pTrigger->m_trapDisarmingDifficulty != 100) {
            CAITrigger trigger(CAITrigger::DISARMED, m_typeAI, 0);
            message = new CMessageSetTrigger(trigger, m_id, pTrigger->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            pTrigger->m_trapActivated = 0;
            pTrigger->SetDrawPoly(0);
            FeedBack(FEEDBACK_ROLL, roll, skill - skillModifier, skillModifier, 0x9962, dc, 0);
            FeedBack(FEEDBACK_TRAPDISARMED, 0, 0, 0, -1, 0, 0);

            message = new CMessageTriggerStatus(pTrigger->m_dwFlags,
                pTrigger->m_trapActivated,
                pTrigger->m_trapDetected,
                m_id,
                pTrigger->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        } else {
            FeedBack(FEEDBACK_ROLL, roll, skill - skillModifier, skillModifier, 0x9963, dc, 0);

            if (25 < pTrigger->m_trapDisarmingDifficulty - dc) {
                CAITrigger trigger(CAITrigger::ENTERED, m_typeAI, 0);
                message = new CMessageSetTrigger(trigger, m_id, pTrigger->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                if ((pTrigger->m_dwFlags & 2) == 0) {
                    pTrigger->m_trapActivated = 0;
                }
            }

            CAITrigger trigger(CAITrigger::DISARMFAILED, m_typeAI, 0);
            message = new CMessageSetTrigger(trigger, m_id, pTrigger->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            FeedBack(FEEDBACK_TRAPDISARMEDFAILED, 0, 0, 0, -1, 0, 0);
        }
    }

    if (pTarget->GetObjectType() == TYPE_CONTAINER
        && static_cast<CGameContainer*>(pTarget)->m_trapActivated != 0) {
        CGameContainer* pContainer = static_cast<CGameContainer*>(pTarget);
        BOOL bDisarmed = FALSE;
        int dc = pContainer->m_trapRemovalDifficulty / 7 + 10;

        if (dc < check && pContainer->m_trapRemovalDifficulty != 100) {
            bDisarmed = TRUE;

            CAITrigger trigger(CAITrigger::DISARMED, m_typeAI, 0);
            message = new CMessageSetTrigger(trigger, m_id, pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            pContainer->SetDrawPoly(0);
            pContainer->SetTrapActivated(0);
            pContainer->SetTrapDetected(0);
            FeedBack(FEEDBACK_ROLL, roll, skill - skillModifier, skillModifier, 0x9962, dc, 0);
            FeedBack(FEEDBACK_TRAPDISARMED, 0, 0, 0, -1, 0, 0);
        } else {
            FeedBack(FEEDBACK_ROLL, roll, skill - skillModifier, skillModifier, 0x9963, dc, 0);

            if (25 < pContainer->m_trapRemovalDifficulty - dc) {
                CAITrigger trigger(CAITrigger::OPENED, m_typeAI, 0);
                message = new CMessageSetTrigger(trigger, m_id, pContainer->GetId());
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                if ((pContainer->m_dwFlags & 8) == 0) {
                    pContainer->SetTrapActivated(0);
                }
            }

            CAITrigger trigger(CAITrigger::DISARMFAILED, m_typeAI, 0);
            message = new CMessageSetTrigger(trigger, m_id, pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

            FeedBack(FEEDBACK_TRAPDISARMEDFAILED, 0, 0, 0, -1, 0, 0);
        }

        if (bDisarmed) {
            message = new CMessageContainerStatus(pContainer->m_dwFlags,
                pContainer->m_trapActivated,
                pContainer->m_trapDetected,
                m_id,
                pContainer->GetId());
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }
    }

    return ACTION_DONE;
}

// 0x758440
SHORT CGameSprite::Panic()
{
    CMessage* message;

    if (m_actionCount == 0) {
        message = new CMessageDropPath(m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        message = new CMessageSetSequence(static_cast<BYTE>(GetIdleSequence()),
            m_id,
            m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    CAIAction randomWalk(CAIAction::RANDOMWALK, CPoint(-1, -1), 0, -1);
    AddAction(randomWalk);

    return ACTION_DONE;
}

// 0x75A3F0
SHORT CGameSprite::Turn()
{
    if (!m_typeAI.IsClassValid(CAIObjectType::C_CLERIC)
        && !m_typeAI.IsClassValid(CAIObjectType::C_PALADIN)) {
        return ACTION_ERROR;
    }

    SetModalState(4, TRUE);
    ClearActions(FALSE);

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == g_pBaldurChitin->m_pEngineWorld->GetSelectedCharacter()) {
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->m_nSelectedButton = 100;
        g_pBaldurChitin->GetObjectGame()->SetState(0);
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->UpdateButtons();
    }

    return ACTION_DONE;
}

// 0x75DFD0
SHORT CGameSprite::EquipMostDamagingMelee()
{
    return ACTION_ERROR;
}

// 0x75E880
const CAIObjectType& CGameSprite::GetLiveAIType()
{
    return m_liveTypeAI;
}

// 0x75E890 (virtual)
BOOLEAN CGameSprite::CanSaveGame(STRREF& strError)
{
    if (!m_baseStats.m_bStealthMode
        && m_typeAI.GetEnemyAlly() >= CAIObjectType::EA_EVILCUTOFF
        && m_canBeSeen > 0
        && (m_derivedStats.m_generalState & STATE_DEAD) == 0
        && (m_baseStats.m_flags & 0x8000) == 0) {
        strError = 16501;
        return FALSE;
    }

    if (CanAct()) {
        if (m_curAction.m_actionID != CAIAction::DIALOGUE
            && m_curAction.m_actionID != CAIAction::STARTDIALOG
            && m_currentActionId != CAIAction::DIALOGUE
            && m_currentActionId != CAIAction::STARTDIALOG) {
            strError = -1;
            return TRUE;
        }
    }

    strError = 16502;
    return FALSE;
}

// 0x75E940
SHORT CGameSprite::GetCasterLevel(CSpell* pSpell, BYTE nClass, DWORD nSpecialization)
{
    // NOTE: Unsigned compare below in the loop.
    UINT nLevel = 1;
    UINT nBestCasterLevel = 0;

    if (pSpell == NULL) {
        if (nClass != 0 || nSpecialization != 0) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26119
            UTIL_ASSERT(pSpell != NULL);
        }
        return 1;
    }

    if (pSpell->GetCasterType() == 4) {
        if (pSpell->GetResRef() == "SPIN263") {
            nClass = CAIOBJECTTYPE_C_CLERIC;
            nSpecialization = SPECMASK_CLERIC_TALOS;
        } else if (pSpell->GetResRef() == "SPIN264") {
            nClass = CAIOBJECTTYPE_C_CLERIC;
            nSpecialization = SPECMASK_CLERIC_TEMPUS;
        } else if (pSpell->GetResRef() == "SPIN265"
            || pSpell->GetResRef() == "SPIN266") {
            nClass = CAIOBJECTTYPE_C_CLERIC;
            nSpecialization = SPECMASK_CLERIC_HELM;
        } else if (pSpell->GetResRef() == "SPIN267"
            || pSpell->GetResRef() == "SPIN270") {
            nClass = CAIOBJECTTYPE_C_CLERIC;
            nSpecialization = SPECMASK_CLERIC_ILMATER;
        } else if (pSpell->GetResRef() == "SPIN268"
            || pSpell->GetResRef() == "SPIN269") {
            nClass = CAIOBJECTTYPE_C_CLERIC;
            nSpecialization = SPECMASK_CLERIC_SELUNE;
        } else if (pSpell->GetResRef() == "SPIN271") {
            nClass = CAIOBJECTTYPE_C_CLERIC;
            nSpecialization = SPECMASK_CLERIC_LATHANDER;
        } else {
            nBestCasterLevel = m_derivedStats.GetLevel();
            if (nBestCasterLevel == 0) {
                nBestCasterLevel = 1;
            }
            return static_cast<SHORT>(nBestCasterLevel);
        }
    } else {
        switch (nClass) {
        case CAIOBJECTTYPE_C_BARBARIAN:
        case CAIOBJECTTYPE_C_FIGHTER:
        case CAIOBJECTTYPE_C_MONK:
        case CAIOBJECTTYPE_C_ROGUE:
            return static_cast<SHORT>(m_derivedStats.GetClassLevel(GetAIType().m_nClass));
        }
    }

    UINT nStart = 0;
    UINT nEnd = CSPELLLIST_NUM_CLASSES;

    if (nClass != 0) {
        nStart = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);
        nEnd = nStart + 1;
    }

    for (UINT nClassIndex = nStart; nClassIndex < nEnd; nClassIndex++) {
        nClass = g_pBaldurChitin->GetObjectGame()->GetSpellcasterClass(nStart);
        switch (nClass) {
        case CAIOBJECTTYPE_C_BARD:
        case CAIOBJECTTYPE_C_DRUID:
        case CAIOBJECTTYPE_C_SORCERER:
        case CAIOBJECTTYPE_C_WIZARD:
            nLevel = m_derivedStats.GetClassLevel(nClass);
            break;
        case CAIOBJECTTYPE_C_CLERIC:
            nLevel = m_derivedStats.GetClassLevel(nClass);
            if (nSpecialization != 0) {
                nLevel++;
            }
            break;
        case CAIOBJECTTYPE_C_PALADIN:
        case CAIOBJECTTYPE_C_RANGER:
            nLevel = m_derivedStats.GetClassLevel(nClass);
            if (nLevel >= 4) {
                nLevel /= 2;
            } else {
                nLevel = 0;
            }
            break;
        }

        // NOTE: Unsigned compare.
        if (nLevel > nBestCasterLevel) {
            nBestCasterLevel = nLevel;
        }
    }

    if (nBestCasterLevel == 0) {
        nBestCasterLevel = m_derivedStats.GetClassLevel(GetAIType().m_nClass);
    }
    if (nBestCasterLevel == 0) {
        nBestCasterLevel = 1;
    }
    return static_cast<SHORT>(nBestCasterLevel);
}

// 0x75F240
SHORT CGameSprite::SavePositionToBaseStats()
{
    m_baseStats.m_savedLocationX = static_cast<SHORT>(m_pos.x);
    m_baseStats.m_savedLocationY = static_cast<SHORT>(m_pos.y);
    m_baseStats.m_savedLocationFacing = static_cast<BYTE>(m_nDirection);
    return ACTION_DONE;
}

// 0x75F3D0
SHORT CGameSprite::SetStealthState(int a1)
{
    if (m_id == 0 || m_pArea == NULL) {
        return ACTION_ERROR;
    }

    if (a1 == 1) {
        if (m_baseStats.m_bStealthMode != TRUE) {
            if (m_typeAI.GetObject(this, TRUE) != NULL) {
                // NOTE: Uninline.
                m_pArea->RemoveFromMarkers(m_id);
            }

            if (m_bOnSearchMap == TRUE) {
                m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                                   m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
                    m_typeAI.GetEnemyAlly(),
                    m_animation.GetPersonalSpace(),
                    m_bBumpable,
                    m_bOnSearchMap);
            }

            m_baseStats.m_bStealthMode = TRUE;
        }
        return ACTION_DONE;
    }

    if (a1 != 0) {
        return ACTION_ERROR;
    }

    if (m_baseStats.m_bStealthMode == FALSE) {
        return ACTION_DONE;
    }

    const BYTE* terrainTable;
    if ((m_animation.GetAnimationId() & 0xF000) == 0x4000
        && m_animation.GetAnimationId() >= 0x4400) {
        terrainTable = m_visibleTerrainTable;
    } else {
        terrainTable = m_terrainTable;
    }

    CPoint ptPassable;
    m_pArea->m_search.FindNearbyPassablePoint(&ptPassable,
        m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
        m_pos.y / CPathSearch::GRID_SQUARE_SIZEY,
        terrainTable,
        m_animation.GetPersonalSpace(),
        -1);

    if (m_typeAI.GetObject(this, TRUE) != NULL) {
        // NOTE: Uninline.
        m_pArea->RemoveFromMarkers(m_id);
    }

    if (m_bOnSearchMap == TRUE) {
        m_pArea->m_search.RemoveObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                           m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
            m_typeAI.GetEnemyAlly(),
            m_animation.GetPersonalSpace(),
            m_bBumpable,
            m_bOnSearchMap);
    }

    // NOTE: Uninline.
    m_pArea->AddToMarkers(m_id);

    m_pArea->m_search.AddObject(CPoint(m_pos.x / CPathSearch::GRID_SQUARE_SIZEX,
                                    m_pos.y / CPathSearch::GRID_SQUARE_SIZEY),
        m_typeAI.GetEnemyAlly(),
        m_animation.GetPersonalSpace(),
        m_bBumpable,
        m_bOnSearchMap);

    m_baseStats.m_bStealthMode = FALSE;
    return ACTION_DONE;
}

// 0x75F800
SHORT CGameSprite::SetAtOffset(DWORD stat, DWORD value, BOOL modify)
{
    switch (stat) {
    case STAT_MAXHITPOINTS:
        if (modify == TRUE) {
            m_baseStats.m_maxHitPointsBase += static_cast<SHORT>(value);
            m_derivedStats.m_nMaxHitPoints += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_maxHitPointsBase = static_cast<SHORT>(value);
            m_derivedStats.m_nMaxHitPoints = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_ARMORCLASS:
        if (modify == TRUE) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26506
            UTIL_ASSERT_MSG(FALSE, "this option is no longer available -rjf");
        } else {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26513
            UTIL_ASSERT_MSG(FALSE, "this option is no longer available -rjf");
        }
    case STAT_ACBLUDGEONINGMOD:
        if (modify == TRUE) {
            m_baseStats.m_armorClassCrushingAdjustment += static_cast<SHORT>(value);
            m_derivedStats.m_nACCrushingMod += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_armorClassCrushingAdjustment = static_cast<SHORT>(value);
            m_derivedStats.m_nACCrushingMod = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_ACMISSILEMOD:
        if (modify == TRUE) {
            m_baseStats.m_armorClassMissileAdjustment += static_cast<SHORT>(value);
            m_derivedStats.m_nACMissileMod += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_armorClassMissileAdjustment = static_cast<SHORT>(value);
            m_derivedStats.m_nACMissileMod = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_ACPIERCINGMOD:
        if (modify == TRUE) {
            m_baseStats.m_armorClassPiercingAdjustment += static_cast<SHORT>(value);
            m_derivedStats.m_nACPiercingMod += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_armorClassPiercingAdjustment = static_cast<SHORT>(value);
            m_derivedStats.m_nACPiercingMod = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_ACSLASHINGMOD:
        if (modify == TRUE) {
            m_baseStats.m_armorClassSlashingAdjustment += static_cast<SHORT>(value);
            m_derivedStats.m_nACSlashingMod += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_armorClassSlashingAdjustment = static_cast<SHORT>(value);
            m_derivedStats.m_nACSlashingMod = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_THAC0:
        if (modify == TRUE) {
            m_baseStats.m_attackBase += static_cast<BYTE>(value);
            m_derivedStats.m_nTHAC0 += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_attackBase = static_cast<BYTE>(value);
            m_derivedStats.m_nTHAC0 = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_NUMBEROFATTACKS:
        if (modify == TRUE) {
            m_baseStats.m_numberOfAttacksBase += static_cast<BYTE>(value);
            m_derivedStats.m_nNumberOfAttacks += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_numberOfAttacksBase = static_cast<BYTE>(value);
            m_derivedStats.m_nNumberOfAttacks = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SAVEFORTITUDE:
        if (modify == TRUE) {
            m_baseStats.m_saveVSFortitudeBase += static_cast<BYTE>(value);
            m_derivedStats.m_nSaveVSFortitude += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_saveVSFortitudeBase = static_cast<BYTE>(value);
            m_derivedStats.m_nSaveVSFortitude = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SAVEREFLEX:
        if (modify == TRUE) {
            m_baseStats.m_saveVSReflexBase += static_cast<BYTE>(value);
            m_derivedStats.m_nSaveVSReflex += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_saveVSReflexBase = static_cast<BYTE>(value);
            m_derivedStats.m_nSaveVSReflex = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SAVEWILL:
        if (modify == TRUE) {
            m_baseStats.m_saveVSWillBase += static_cast<BYTE>(value);
            m_derivedStats.m_nSaveVSWill += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_saveVSWillBase = static_cast<BYTE>(value);
            m_derivedStats.m_nSaveVSWill = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_KNOWLEDGEARCANA:
    case STAT_13:
        return 1;
    case STAT_RESISTFIRE:
        if (modify == TRUE) {
            m_baseStats.m_resistFireBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistFire += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistFireBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistFire = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTCOLD:
        if (modify == TRUE) {
            m_baseStats.m_resistColdBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistCold += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistColdBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistCold = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTELECTRICITY:
        if (modify == TRUE) {
            m_baseStats.m_resistElectricityBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistElectricity += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistElectricityBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistElectricity = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTACID:
        if (modify == TRUE) {
            m_baseStats.m_resistAcidBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistAcid += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistAcidBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistAcid = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTMAGIC:
        if (modify == TRUE) {
            m_baseStats.m_resistMagicBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistMagic += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistMagicBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistMagic = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTMAGICFIRE:
        if (modify == TRUE) {
            m_baseStats.m_resistMagicFireBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistMagicFire += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistMagicFireBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistMagicFire = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTMAGICCOLD:
        if (modify == TRUE) {
            m_baseStats.m_resistMagicColdBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistMagicCold += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistMagicColdBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistMagicCold = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTSLASHING:
        if (modify == TRUE) {
            m_baseStats.m_resistSlashingBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistSlashing += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistSlashingBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistSlashing = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTBLUDGEONING:
        if (modify == TRUE) {
            m_baseStats.m_resistCrushingBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistCrushing += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistCrushingBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistCrushing = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTPIERCING:
        if (modify == TRUE) {
            m_baseStats.m_resistPiercingBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistPiercing += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistPiercingBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistPiercing = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTMISSILE:
        if (modify == TRUE) {
            m_baseStats.m_resistMissileBase += static_cast<signed char>(value);
            m_derivedStats.m_nResistMissile += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_resistMissileBase = static_cast<signed char>(value);
            m_derivedStats.m_nResistMissile = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_ALCHEMY:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_ALCHEMY] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_ALCHEMY] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_ALCHEMY] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_ALCHEMY] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_LOCKPICKING:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_OPEN_LOCK] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_OPEN_LOCK] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_OPEN_LOCK] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_MOVESILENTLY:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_MOVE_SILENTLY] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_MOVE_SILENTLY] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_MOVE_SILENTLY] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_MOVE_SILENTLY] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_TRAPS:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_SEARCH] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_SEARCH] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_SEARCH] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_SEARCH] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_PICKPOCKET:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_PICK_POCKET] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_PICK_POCKET] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_PICK_POCKET] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_PICK_POCKET] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_FATIGUE:
        if (modify == TRUE) {
            m_baseStats.m_fatigue += static_cast<BYTE>(value);
            m_derivedStats.m_nFatigue += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_fatigue = static_cast<BYTE>(value);
            m_derivedStats.m_nFatigue = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_INTOXICATION:
        if (modify == TRUE) {
            m_baseStats.m_intoxication += static_cast<BYTE>(value);
            // FIXME: Looks like a bug - should be `m_nIntoxication`.
            m_derivedStats.m_nFatigue += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_intoxication = static_cast<BYTE>(value);
            // FIXME: Looks like a bug - should be `m_nIntoxication`.
            m_derivedStats.m_nFatigue = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_LUCK:
        if (modify == TRUE) {
            m_baseStats.m_luckBase += static_cast<BYTE>(value);
            m_derivedStats.m_nLuck += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_luckBase = static_cast<BYTE>(value);
            m_derivedStats.m_nLuck = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_TRACKING:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_WILDERNESS_LORE] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_WILDERNESS_LORE] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_LEVEL:
        if (modify == TRUE) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26891
            UTIL_ASSERT_MSG(FALSE, "Scripts and dialogs may no longer set Class Levels");
        } else {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26897
            UTIL_ASSERT_MSG(FALSE, "Scripts and dialogs may no longer set Class Levels");
        }
    case STAT_SEX:
        if (modify == TRUE) {
            m_baseStats.m_sex += static_cast<BYTE>(value);
            m_derivedStats.m_nSex += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_sex = static_cast<BYTE>(value);
            m_derivedStats.m_nSex = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_STR:
        if (modify == TRUE) {
            m_baseStats.m_STRBase += static_cast<BYTE>(value);
            m_derivedStats.m_nSTR += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_STRBase = static_cast<BYTE>(value);
            m_derivedStats.m_nSTR = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_STREXTRA:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 26969
        UTIL_ASSERT(FALSE);
    case STAT_INT:
        if (modify == TRUE) {
            m_baseStats.m_INTBase += static_cast<BYTE>(value);
            m_derivedStats.m_nINT += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_INTBase = static_cast<BYTE>(value);
            m_derivedStats.m_nINT = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_WIS:
        if (modify == TRUE) {
            m_baseStats.m_WISBase += static_cast<BYTE>(value);
            m_derivedStats.m_nWIS += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_WISBase = static_cast<BYTE>(value);
            m_derivedStats.m_nWIS = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_DEX:
        if (modify == TRUE) {
            m_baseStats.m_DEXBase += static_cast<BYTE>(value);
            m_derivedStats.m_nDEX += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_DEXBase = static_cast<BYTE>(value);
            m_derivedStats.m_nDEX = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_CON:
        if (modify == TRUE) {
            m_baseStats.m_CONBase += static_cast<BYTE>(value);
            m_derivedStats.m_nCON += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_CONBase = static_cast<BYTE>(value);
            m_derivedStats.m_nCON = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_CHR:
        if (modify == TRUE) {
            m_baseStats.m_CHRBase += static_cast<BYTE>(value);
            m_derivedStats.m_nCHR += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_CHRBase = static_cast<BYTE>(value);
            m_derivedStats.m_nCHR = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_XPVALUE:
        if (modify == TRUE) {
            m_baseStats.m_xpValue += value;
            m_derivedStats.m_nXPValue += value;
        } else {
            m_baseStats.m_xpValue = value;
            m_derivedStats.m_nXPValue = value;
        }
        return 1;
    case STAT_XP:
        if (modify == TRUE) {
            m_baseStats.m_xp += value;
            m_derivedStats.m_nXP += value;
        } else {
            m_baseStats.m_xp = value;
            m_derivedStats.m_nXP = value;
        }
        return 1;
    case STAT_GOLD:
        if (modify == TRUE) {
            m_baseStats.m_gold += value;
            m_derivedStats.m_nGold += value;
        } else {
            m_baseStats.m_gold = value;
            m_derivedStats.m_nGold = value;
        }
        return 1;
    case STAT_MORALEBREAK:
        if (modify == TRUE) {
            m_baseStats.m_moraleBreak += static_cast<BYTE>(value);
            m_derivedStats.m_nMoraleBreak += value;
        } else {
            m_baseStats.m_moraleBreak = static_cast<BYTE>(value);
            m_derivedStats.m_nMoraleBreak = value;
        }
        return 1;
    case STAT_MORALERECOVERYTIME:
        if (modify == TRUE) {
            m_baseStats.m_moraleRecoveryTime += static_cast<WORD>(value);
            m_derivedStats.m_nMoraleRecoveryTime += value;
        } else {
            m_baseStats.m_moraleRecoveryTime = static_cast<WORD>(value);
            m_derivedStats.m_nMoraleRecoveryTime = value;
        }
        return 1;
    case STAT_REPUTATION:
        if (modify == TRUE) {
            m_baseStats.m_reputation += static_cast<BYTE>(value);
            m_derivedStats.m_nReputation += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_reputation = static_cast<BYTE>(value);
            m_derivedStats.m_nReputation = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_HATEDRACE:
        if (modify == TRUE) {
            m_baseStats.m_favoredEnemies[0] += static_cast<BYTE>(value);
            m_derivedStats.m_favoredEnemies[0] += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_favoredEnemies[0] = static_cast<BYTE>(value);
            m_derivedStats.m_favoredEnemies[0] = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_DAMAGEBONUS:
        if (modify == TRUE) {
            m_derivedStats.m_nDamageBonus += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nDamageBonus = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SPELLFAILUREMAGE:
        if (modify == TRUE) {
            m_derivedStats.m_nSpellFailureArcane += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nSpellFailureArcane = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SPELLFAILUREPRIEST:
        if (modify == TRUE) {
            m_derivedStats.m_nSpellFailureDivine += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nSpellFailureDivine = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SPELLDURATIONMODMAGE:
        if (modify == TRUE) {
            m_derivedStats.m_nSpellDurationModMage += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nSpellDurationModMage = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SPELLDURATIONMODPRIEST:
        if (modify == TRUE) {
            m_derivedStats.m_nSpellDurationModPriest += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nSpellDurationModPriest = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_TURNUNDEADLEVEL:
        if (modify == TRUE) {
            m_baseStats.m_turnUndeadLevel += static_cast<BYTE>(value);
            m_derivedStats.m_nTurnUndeadLevel += static_cast<SHORT>(value);
        } else {
            m_baseStats.m_turnUndeadLevel = static_cast<BYTE>(value);
            m_derivedStats.m_nTurnUndeadLevel = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_BACKSTABDAMAGEMULTIPLIER:
        if (modify == TRUE) {
            m_derivedStats.m_nBackstabDamageMultiplier += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nBackstabDamageMultiplier = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_LAYONHANDSAMOUNT:
        if (modify == TRUE) {
            m_derivedStats.m_nLayOnHandsAmount += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nLayOnHandsAmount = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_TRANSLUCENT:
        if (modify == TRUE) {
            m_derivedStats.m_nTranslucent += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nTranslucent = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_LEVEL2:
        if (modify == TRUE) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26905
            UTIL_ASSERT_MSG(FALSE, "Scripts and dialogs may no longer set Class Levels");
        } else {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26911
            UTIL_ASSERT_MSG(FALSE, "Scripts and dialogs may no longer set Class Levels");
        }
    case STAT_LEVEL3:
        if (modify == TRUE) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 29919
            UTIL_ASSERT_MSG(FALSE, "Scripts and dialogs may no longer set Class Levels");
        } else {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 26925
            UTIL_ASSERT_MSG(FALSE, "Scripts and dialogs may no longer set Class Levels");
        }
    case STAT_ENCUMBERANCE:
        if (modify == TRUE) {
            m_derivedStats.m_nEncumberance += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nEncumberance = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_MISSILETHAC0BONUS:
        if (modify == TRUE) {
            m_derivedStats.m_nMissileTHAC0Bonus += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nMissileTHAC0Bonus = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_MAGICDAMAGERESISTANCE:
        if (modify == TRUE) {
            m_derivedStats.m_nMagicDamageResistance += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nMagicDamageResistance = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_RESISTPOISON:
        if (modify == TRUE) {
            m_derivedStats.m_nResistPoison += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nResistPoison = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_MENTALSPEED:
        if (modify == TRUE) {
            m_derivedStats.m_nMentalSpeed += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nMentalSpeed = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_PHYSICALSPEED:
        if (modify == TRUE) {
            m_derivedStats.m_nPhysicalSpeed += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nPhysicalSpeed = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_CASTINGLEVELBONUSMAGE:
        if (modify == TRUE) {
            m_derivedStats.m_nCastingLevelBonusMage += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nCastingLevelBonusMage = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_CASTINGLEVELBONUSCLERIC:
        if (modify == TRUE) {
            m_derivedStats.m_nCastingLevelBonusCleric += static_cast<SHORT>(value);
        } else {
            m_derivedStats.m_nCastingLevelBonusCleric = static_cast<SHORT>(value);
        }
        return 1;
    case STAT_SEEINVISIBLE:
        if (modify == TRUE) {
            m_derivedStats.m_bSeeInvisible += value;
        } else {
            m_derivedStats.m_bSeeInvisible = value;
        }
        return 1;
    case STAT_IGNOREDIALOGPAUSE:
        if (modify == TRUE) {
            m_derivedStats.m_bIgnoreDialogPause += value;
        } else {
            m_derivedStats.m_bIgnoreDialogPause = value;
        }
        return 1;
    case STAT_MINHITPOINTS:
        if (modify == TRUE) {
            m_derivedStats.m_nMinHitPoints += value;
        } else {
            m_derivedStats.m_nMinHitPoints = value;
        }
        return 1;
    case STAT_THAC0BONUSRIGHT:
        if (modify == TRUE) {
            m_derivedStats.m_THAC0BonusRight += value;
        } else {
            m_derivedStats.m_THAC0BonusRight = value;
        }
        return 1;
    case STAT_THAC0BONUSLEFT:
        if (modify == TRUE) {
            m_derivedStats.m_THAC0BonusLeft += value;
        } else {
            m_derivedStats.m_THAC0BonusLeft = value;
        }
        return 1;
    case STAT_DAMAGEBONUSRIGHT:
        if (modify == TRUE) {
            m_derivedStats.m_DamageBonusRight += value;
        } else {
            m_derivedStats.m_DamageBonusRight = value;
        }
        return 1;
    case STAT_DAMAGEBONUSLEFT:
        if (modify == TRUE) {
            m_derivedStats.m_DamageBonusLeft += value;
        } else {
            m_derivedStats.m_DamageBonusLeft = value;
        }
        return 1;
    case STAT_HIDEINSHADOWS:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_HIDE] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_HIDE] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_HIDE] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_HIDE] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_MOVESILENTLYMTPBONUS:
        if (modify == TRUE) {
            m_derivedStats.m_nMoveSilentlyMTPBonus += value;
        } else {
            m_derivedStats.m_nMoveSilentlyMTPBonus = value;
        }
        return 1;
    case STAT_HIDEINSHADOWSMTPBONUS:
        if (modify == TRUE) {
            m_derivedStats.m_nHideInShadowsMTPBonus += value;
        } else {
            m_derivedStats.m_nHideInShadowsMTPBonus = value;
        }
        return 1;
    case STAT_SUBRACE:
        if (modify == TRUE) {
            m_baseStats.m_subrace += static_cast<BYTE>(value);
            m_derivedStats.m_nSubRace += value;
        } else {
            m_baseStats.m_subrace = static_cast<BYTE>(value);
            m_derivedStats.m_nSubRace = value;
        }
        return 1;
    case STAT_USEMAGICDEVICE:
        if (modify == TRUE) {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] += static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] += static_cast<BYTE>(value);
        } else {
            m_baseStats.m_skills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = static_cast<BYTE>(value);
            m_derivedStats.m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] = static_cast<BYTE>(value);
        }
        return 1;
    case STAT_CLASSLEVELBARBARIAN:
    case STAT_CLASSLEVELBARD:
    case STAT_CLASSLEVELCLERIC:
    case STAT_CLASSLEVELDRUID:
    case STAT_CLASSLEVELFIGHTER:
    case STAT_CLASSLEVELMONK:
    case STAT_CLASSLEVELPALADIN:
    case STAT_CLASSLEVELRANGER:
    case STAT_CLASSLEVELROGUE:
    case STAT_CLASSLEVELSORCERER:
    case STAT_CLASSLEVELWIZARD:
        if (modify == TRUE) {
            m_derivedStats.SetClassLevel(stat - STAT_CLASSLEVELSUM,
                m_derivedStats.GetClassLevel(stat - STAT_CLASSLEVELSUM) + value);
            SetClassLevel(stat - STAT_CLASSLEVELSUM,
                GetClassLevel(stat - STAT_CLASSLEVELSUM) + value);
        } else {
            m_derivedStats.SetClassLevel(stat - STAT_CLASSLEVELSUM, value);
            SetClassLevel(stat - STAT_CLASSLEVELSUM, value);
        }
        return 1;
    case STAT_PREVENTAISLOWDOWN:
        if (modify == TRUE) {
            m_derivedStats.m_bPreventAISlowDown += value;
        } else {
            m_derivedStats.m_bPreventAISlowDown = value;
        }
        return 1;
    }

    return 0;
}

// 0x761470
SHORT CGameSprite::ForceHide(CGameSprite* pSprite)
{
    if (m_bHiding) {
        return ACTION_DONE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetState() != 0) {
        return ACTION_DONE;
    }

    SetModalState(3, TRUE);
    g_pBaldurChitin->GetObjectGame()->GetButtonArray()->SetSelectedButton(5);

    if (!m_bHiding && !m_baseStats.m_bStealthMode) {
        PlaySound(CResRef("ACT_07"));
    }

    if (!m_baseStats.m_bStealthMode) {
        FeedBack(FEEDBACK_HIDESUCCEEDED,
            0,
            0,
            0,
            -1,
            0,
            0);
    }

    ITEM_EFFECT effect;
    CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_INVISIBLE);
    effect.durationType = 0x100;
    effect.dwFlags = 0;
    effect.effectAmount = 1;
    effect.duration = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime + 300;

    CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
        m_pos,
        m_id,
        CPoint(-1, -1));

    CMessage* message = new CMessageAddEffect(pEffect, m_id, m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

    m_modalCounter = 0;
    m_bHiding = TRUE;

    return ACTION_DONE;
}

// 0x7615F0
SHORT CGameSprite::SetExplorationRange(int a1)
{
    if (a1 == 0) {
        m_pArea->m_visibility.SetAreaVisible(FALSE);
        a1 = CVisibilityMap::EXPLORED_RANGE_3;
    }

    m_pArea->m_visibility.m_nSearchRangeH = static_cast<short>(a1);
    m_pArea->m_visibility.m_nSearchRangeV = static_cast<short>(3 * a1 / 4);
    m_pArea->m_visibility.UpdateVisibilityEllipses();

    return ACTION_DONE;
}

// 0x761650
void CGameSprite::sub_761650()
{
    // TODO: Incomplete.
}

// 0x7618E0
SHORT CGameSprite::DropItem(CItem* pItem)
{
    if (pItem == NULL) {
        return ACTION_ERROR;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    LONG iContainer = pGame->GetGroundPile(m_id);
    if (iContainer == CGameObjectArray::INVALID_INDEX) {
        return ACTION_ERROR;
    }

    CGameContainer* pContainer;

    BYTE rc = pGame->GetObjectArray()->GetDeny(iContainer,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return ACTION_ERROR;
    }

    if ((pItem->GetFlagsFile() & 0x4) != 0) {
        pContainer->PlaceItemInBlankSlot(pItem, 1, SHORT_MAX);
    }

    pGame->GetObjectArray()->ReleaseDeny(iContainer,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return ACTION_DONE;
}

// 0x761990
void CGameSprite::ReapplyEquipmentEffects()
{
    m_timedEffectList.RemoveAllEffectsIgnoreMoreThenPermanent(NULL, FALSE, FALSE, 0, 0);
    UnequipAll(FALSE);
    EquipAll(FALSE);
}

// 0x7619C0
SHORT CGameSprite::PlayBardSong()
{
    if ((GetAIType().m_nClassMask & CLASSMASK_BARD) == 0) {
        return ACTION_ERROR;
    }

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    if (pGame->GetButtonArray()->m_nState == 113) {
        return ACTION_DONE;
    }

    BYTE nSong = static_cast<BYTE>(m_curAction.m_specificID);
    if (nSong >= pGame->GetMasterSongLookup().m_nCount) {
        return ACTION_ERROR;
    }

    m_nLastSong = nSong;
    SetModalState(1, TRUE);
    ClearActions(FALSE);

    if (g_pBaldurChitin->GetObjectGame()->GetCharacterPortraitNum(m_id) == g_pBaldurChitin->GetScreenWorld()->GetSelectedCharacter()) {
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->SetSelectedButton(100);
        g_pBaldurChitin->GetObjectGame()->SetState(0);
        g_pBaldurChitin->GetObjectGame()->GetButtonArray()->UpdateButtons();
    }

    return ACTION_DONE;
}

// 0x762740
BOOL CGameSprite::HasClassMask(DWORD dwMask)
{
    return (dwMask & GetAIType().m_nClassMask) != 0;
}

// 0x762760
INT CGameSprite::GetClassLevel(INT iClassType)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28701
    UTIL_ASSERT(iClassType > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28702
    UTIL_ASSERT(iClassType <= CAIOBJECT_CLASS_MAX);

    return m_baseStats.m_classLevels[iClassType - 1];
}

// 0x7627C0
void CGameSprite::SetClassLevel(INT iClassType, INT nLevel)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28718
    UTIL_ASSERT(iClassType > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28719
    UTIL_ASSERT(iClassType <= CAIOBJECT_CLASS_MAX);

    m_baseStats.m_characterLevel += static_cast<BYTE>(nLevel) - m_baseStats.m_classLevels[iClassType];
    m_baseStats.m_classLevels[iClassType - 1] = static_cast<BYTE>(nLevel);
}

// 0x762830
BOOL CGameSprite::HasClassLevel(INT iClassType)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28739
    UTIL_ASSERT(iClassType > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28740
    UTIL_ASSERT(iClassType <= CAIOBJECT_CLASS_MAX);

    return m_baseStats.m_classLevels[iClassType - 1] != 0;
}

// 0x762890
void CGameSprite::SetFeatValue(UINT nFeatNumber, INT iFeatValue)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28756
    UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

    if (iFeatValue > 0) {
        m_baseStats.m_feats[nFeatNumber >> 5] |= 1 << (nFeatNumber & 0x1F);
    } else {
        m_baseStats.m_feats[nFeatNumber >> 5] &= ~(1 << (nFeatNumber & 0x1F));
    }

    switch (nFeatNumber) {
    case CGAMESPRITE_FEAT_ARMOR_PROF:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28774
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_ARMOR_PROFICIENCY_UPGRADE);

        m_baseStats.m_featArmorProficiency = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_ARMORED_ARCANA:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28781
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_ARMORED_ARCANA_UPGRADE);

        m_baseStats.m_featArmoredArcana = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_CLEAVE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28781
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_CLEAVE_UPGRADE);

        m_baseStats.m_featCleave = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_EXOTIC_BASTARD:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28795
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featBastardSword = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_EXTRA_RAGE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28802
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_EXTRA_RAGE_UPGRADE);

        m_baseStats.m_featExtraRage = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_EXTRA_SHAPESHIFTING:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28809
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_EXTRA_SHAPESHIFTING_UPGRADE);

        m_baseStats.m_featExtraShapeshifting = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_EXTRA_SMITING:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28816
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_EXTRA_SMITING_UPGRADE);

        m_baseStats.m_featExtraSmiting = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_EXTRA_TURNING:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28823
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_EXTRA_TURNING_UPGRADE);

        m_baseStats.m_featExtraTurning = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_AXE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28830
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featAxe = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_BOW:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28837
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featBow = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_FLAIL:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28844
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featFlail = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_GREATSWORD:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28851
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featGreatSword = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_HAMMER:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28858
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featHammer = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_LARGESWORD:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28865
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featLargeSword = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_MARTIAL_POLEARM:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28872
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featPolearm = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SIMPLE_CROSSBOW:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28879
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featCrossbow = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SIMPLE_MACE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28886
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featMace = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SIMPLE_MISSILE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28893
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featMissile = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28890
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featQuarterStaff = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28897
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE);

        m_baseStats.m_featSmallBlade = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_ENCHANTMENT:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28914
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_ENCHANTMENT_UPGRADE);

        m_baseStats.m_featFocusEnchantment = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_EVOCATION:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28921
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_EVOCATION_UPGRADE);

        m_baseStats.m_featFocusEvocation = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_NECROMANCY:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28928
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_NECROMANCY_UPGRADE);

        m_baseStats.m_featFocusNecromancy = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_TRANSMUTE:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28935
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_TRANSMUTATION_UPGRADE);

        m_baseStats.m_featFocusTransmutation = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_SPELL_PENETRATION:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28942
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_MAX_SPELL_PENETRATION_UPGRADE);

        m_baseStats.m_featSpellPenetration = iFeatValue;
        break;
    case CGAMESPRITE_FEAT_TOUGHNESS:
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
        // __LINE__: 28949
        UTIL_ASSERT(iFeatValue <= CGAMESPRITE_FEAT_TOUGHNESS_MAX_LEVEL);

        m_baseStats.m_featToughness = iFeatValue;
        break;
    }
}

// 0x762E20
INT CGameSprite::GetFeatValue(UINT nFeatNumber)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 28968
    UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

    if ((m_baseStats.m_feats[nFeatNumber >> 5] & (1 << (nFeatNumber & 0x1F))) == 0) {
        return 0;
    }

    switch (nFeatNumber) {
    case CGAMESPRITE_FEAT_ARMOR_PROF:
        return m_baseStats.m_featArmorProficiency;
    case CGAMESPRITE_FEAT_ARMORED_ARCANA:
        return m_baseStats.m_featArmoredArcana;
    case CGAMESPRITE_FEAT_CLEAVE:
        return m_baseStats.m_featCleave;
    case CGAMESPRITE_FEAT_EXOTIC_BASTARD:
        return m_baseStats.m_featBastardSword;
    case CGAMESPRITE_FEAT_EXTRA_RAGE:
        return m_baseStats.m_featExtraRage;
    case CGAMESPRITE_FEAT_EXTRA_SHAPESHIFTING:
        return m_baseStats.m_featExtraShapeshifting;
    case CGAMESPRITE_FEAT_EXTRA_SMITING:
        return m_baseStats.m_featExtraSmiting;
    case CGAMESPRITE_FEAT_EXTRA_TURNING:
        return m_baseStats.m_featExtraTurning;
    case CGAMESPRITE_FEAT_MARTIAL_AXE:
        return m_baseStats.m_featAxe;
    case CGAMESPRITE_FEAT_MARTIAL_BOW:
        return m_baseStats.m_featBow;
    case CGAMESPRITE_FEAT_MARTIAL_FLAIL:
        return m_baseStats.m_featFlail;
    case CGAMESPRITE_FEAT_MARTIAL_GREATSWORD:
        return m_baseStats.m_featGreatSword;
    case CGAMESPRITE_FEAT_MARTIAL_HAMMER:
        return m_baseStats.m_featHammer;
    case CGAMESPRITE_FEAT_MARTIAL_LARGESWORD:
        return m_baseStats.m_featLargeSword;
    case CGAMESPRITE_FEAT_MARTIAL_POLEARM:
        return m_baseStats.m_featPolearm;
    case CGAMESPRITE_FEAT_SIMPLE_CROSSBOW:
        return m_baseStats.m_featCrossbow;
    case CGAMESPRITE_FEAT_SIMPLE_MACE:
        return m_baseStats.m_featMace;
    case CGAMESPRITE_FEAT_SIMPLE_MISSILE:
        return m_baseStats.m_featMissile;
    case CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF:
        return m_baseStats.m_featQuarterStaff;
    case CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE:
        return m_baseStats.m_featSmallBlade;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_ENCHANTMENT:
        return m_baseStats.m_featFocusEnchantment;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_EVOCATION:
        return m_baseStats.m_featFocusEvocation;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_NECROMANCY:
        return m_baseStats.m_featFocusNecromancy;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_TRANSMUTE:
        return m_baseStats.m_featFocusTransmutation;
    case CGAMESPRITE_FEAT_SPELL_PENETRATION:
        return m_baseStats.m_featSpellPenetration;
    case CGAMESPRITE_FEAT_TOUGHNESS:
        return m_baseStats.m_featToughness;
    }
    return 1;
}

// 0x7630A0
INT CGameSprite::GetMaxFeatValue(UINT nFeatNumber)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 29126
    UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

    switch (nFeatNumber) {
    case CGAMESPRITE_FEAT_ARMOR_PROF:
        return CGAMESPRITE_FEAT_MAX_ARMOR_PROFICIENCY_UPGRADE;
    case CGAMESPRITE_FEAT_ARMORED_ARCANA:
        return CGAMESPRITE_FEAT_MAX_ARMORED_ARCANA_UPGRADE;
    case CGAMESPRITE_FEAT_CLEAVE:
        return CGAMESPRITE_FEAT_MAX_CLEAVE_UPGRADE;
    case CGAMESPRITE_FEAT_EXOTIC_BASTARD:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_EXTRA_RAGE:
        return CGAMESPRITE_FEAT_MAX_EXTRA_RAGE_UPGRADE;
    case CGAMESPRITE_FEAT_EXTRA_SHAPESHIFTING:
        return CGAMESPRITE_FEAT_MAX_EXTRA_SHAPESHIFTING_UPGRADE;
    case CGAMESPRITE_FEAT_EXTRA_SMITING:
        return CGAMESPRITE_FEAT_MAX_EXTRA_SMITING_UPGRADE;
    case CGAMESPRITE_FEAT_EXTRA_TURNING:
        return CGAMESPRITE_FEAT_MAX_EXTRA_TURNING_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_AXE:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_BOW:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_FLAIL:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_GREATSWORD:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_HAMMER:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_LARGESWORD:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_MARTIAL_POLEARM:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_SIMPLE_CROSSBOW:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_SIMPLE_MACE:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_SIMPLE_MISSILE:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE:
        return CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_ENCHANTMENT:
        return CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_ENCHANTMENT_UPGRADE;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_EVOCATION:
        return CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_EVOCATION_UPGRADE;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_NECROMANCY:
        return CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_NECROMANCY_UPGRADE;
    case CGAMESPRITE_FEAT_SPELL_FOCUS_TRANSMUTE:
        return CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_TRANSMUTATION_UPGRADE;
    case CGAMESPRITE_FEAT_SPELL_PENETRATION:
        return CGAMESPRITE_FEAT_MAX_SPELL_PENETRATION_UPGRADE;
    case CGAMESPRITE_FEAT_TOUGHNESS:
        return CGAMESPRITE_FEAT_TOUGHNESS_MAX_LEVEL;
    }

    return 1;
}

// 0x763150
BOOL CGameSprite::HasFeat(UINT nFeatNumber)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 29278
    UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

    if ((m_baseStats.m_feats[nFeatNumber >> 5] & (1 << (nFeatNumber & 0x1F))) == 0) {
        return 0;
    }

    if (nFeatNumber == CGAMESPRITE_FEAT_CLEAVE) {
        if (m_derivedStats.m_nSTR >= 13) {
            if (HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK)) {
                INT nValue = GetFeatValue(CGAMESPRITE_FEAT_CLEAVE);
                if (nValue == 1 || nValue == 2) {
                    if (m_baseStats.m_attackBase >= 4) {
                        return TRUE;
                    }
                }
            }
        }
        return FALSE;
    }

    return CheckFeatPrerequisites(nFeatNumber, 0);
}

// 0x763200
BOOL CGameSprite::CheckFeatPrerequisites(UINT nFeatNumber, INT a2)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 29333
    UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

    SHORT nSTR = m_derivedStats.m_nSTR;
    SHORT nDEX = m_derivedStats.m_nDEX;
    SHORT nINT = m_derivedStats.m_nINT;
    SHORT nWIS = m_derivedStats.m_nWIS;

    if (a2 == 1) {
        nSTR = m_baseStats.m_STRBase;
        nDEX = m_baseStats.m_DEXBase;
        nINT = m_baseStats.m_INTBase;
        nWIS = m_baseStats.m_WISBase;
    }

    switch (nFeatNumber) {
    case CGAMESPRITE_FEAT_AEGIS_OF_RIME:
    case CGAMESPRITE_FEAT_AQUA_MORTIS:
    case CGAMESPRITE_FEAT_SCION_OF_STORMS:
    case CGAMESPRITE_FEAT_SPIRIT_OF_FLAME:
        return (m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_BARD) >= 11
                   || m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_CLERIC) >= 7
                   || m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_DRUID) >= 7
                   || m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_WIZARD) >= 7
                   || m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_PALADIN) >= 15
                   || m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_RANGER) >= 15
                   || m_derivedStats.GetClassLevel(CAIOBJECTTYPE_C_RANGER) >= 8)
            && m_derivedStats.m_nSkills[13] >= 10;
    case CGAMESPRITE_FEAT_AMBIDEXTERITY:
        return nDEX >= 15;
    case CGAMESPRITE_FEAT_ARMOR_PROF:
    case CGAMESPRITE_FEAT_BLIND_FIGHT:
    case CGAMESPRITE_FEAT_COURTEOUS_MAGOCRACY:
    case CGAMESPRITE_FEAT_DASH:
    case CGAMESPRITE_FEAT_DISCIPLINE:
    case CGAMESPRITE_FEAT_FORESTER:
    case CGAMESPRITE_FEAT_GREAT_FORTITUDE:
    case CGAMESPRITE_FEAT_IMPROVED_INITIATIVE:
    case CGAMESPRITE_FEAT_IRON_WILL:
    case CGAMESPRITE_FEAT_LIGHTNING_REFLEXES:
    case CGAMESPRITE_FEAT_PRECISE_SHOT:
    case CGAMESPRITE_FEAT_SHIELD_PROF:
    case CGAMESPRITE_FEAT_SPELL_FOCUS_ENCHANTMENT:
    case CGAMESPRITE_FEAT_SPELL_FOCUS_EVOCATION:
    case CGAMESPRITE_FEAT_SPELL_FOCUS_NECROMANCY:
    case CGAMESPRITE_FEAT_SPELL_FOCUS_TRANSMUTE:
    case CGAMESPRITE_FEAT_SPELL_PENETRATION:
    case CGAMESPRITE_FEAT_STRONG_BACK:
    case CGAMESPRITE_FEAT_TOUGHNESS:
    case CGAMESPRITE_FEAT_TWO_WEAPON_FIGHTING:
        return TRUE;
    case CGAMESPRITE_FEAT_ARMORED_ARCANA:
        return m_derivedStats.HasClassMask(CLASSMASK_WIZARD | CLASSMASK_SORCERER | CLASSMASK_BARD);
    case CGAMESPRITE_FEAT_ARTERIAL_STRIKE:
        return m_baseStats.m_rogueLevel > 0 && m_baseStats.m_attackBase >= 4;
    case CGAMESPRITE_FEAT_BULLHEADED:
        return m_typeAI.m_nRace == CAIOBJECTTYPE_R_HUMAN
            || m_typeAI.m_nRace == CAIOBJECTTYPE_R_DWARF;
    case CGAMESPRITE_FEAT_CLEAVE:
        if (!(nSTR >= 13 && HasFeat(CGAMESPRITE_FEAT_POWER_ATTACK))) {
            return FALSE;
        }

        if (GetFeatValue(CGAMESPRITE_FEAT_CLEAVE) >= 1) {
            return m_baseStats.m_attackBase >= 4;
        }

        return TRUE;
    case CGAMESPRITE_FEAT_COMBAT_CASTING:
        return m_derivedStats.HasClassMask(CLASSMASK_WIZARD | CLASSMASK_SORCERER | CLASSMASK_BARD)
            || m_derivedStats.HasClassMask(CLASSMASK_RANGER | CLASSMASK_PALADIN | CLASSMASK_DRUID | CLASSMASK_CLERIC);
    case CGAMESPRITE_FEAT_CRIPPLING_STRIKE:
        return m_baseStats.m_rogueLevel >= 10;
    case CGAMESPRITE_FEAT_DEFLECT_ARROWS:
    case CGAMESPRITE_FEAT_DODGE:
    case CGAMESPRITE_FEAT_RAPID_SHOT:
        return nDEX >= 13;
    case CGAMESPRITE_FEAT_DIRTY_FIGHTING:
        return nDEX >= 13 && m_baseStats.m_attackBase >= 2;
    case CGAMESPRITE_FEAT_ENVENOM_WEAPON:
        return m_baseStats.m_rogueLevel > 0
            && m_baseStats.m_skills[CGAMESPRITE_SKILL_ALCHEMY] >= 8;
    case CGAMESPRITE_FEAT_EXOTIC_BASTARD:
        return GetFeatValue(CGAMESPRITE_FEAT_EXOTIC_BASTARD) != 2
            || m_baseStats.m_fighterLevel >= 4;
    case CGAMESPRITE_FEAT_EXPERTISE:
        return nINT >= 13;
    case CGAMESPRITE_FEAT_EXTRA_RAGE:
        return m_baseStats.m_barbarianLevel > 0;
    case CGAMESPRITE_FEAT_EXTRA_SHAPESHIFTING:
        return m_baseStats.m_druidLevel >= 3;
    case CGAMESPRITE_FEAT_EXTRA_SMITING:
        return m_baseStats.m_paladinLevel >= 2;
    case CGAMESPRITE_FEAT_EXTRA_TURNING:
        return m_baseStats.m_paladinLevel >= 3
            || m_baseStats.m_clericLevel > 0;
    case CGAMESPRITE_FEAT_FIENDSLAYER:
        return (GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_LARGESWORD) >= 2
                   || GetFeatValue(CGAMESPRITE_FEAT_MARTIAL_GREATSWORD) >= 2)
            && m_baseStats.m_paladinLevel >= 3;
    case CGAMESPRITE_FEAT_HAMSTRING:
        return m_baseStats.m_rogueLevel >= 3;
    case CGAMESPRITE_FEAT_HERETICS_BANE:
        return m_baseStats.m_attackBase >= 4
            && m_derivedStats.HasClassMask(CLASSMASK_RANGER | CLASSMASK_PALADIN | CLASSMASK_DRUID | CLASSMASK_CLERIC);
    case CGAMESPRITE_FEAT_HEROIC_INSPIRATION:
        return m_baseStats.m_barbarianLevel > 0
            || m_baseStats.m_barbarianLevel > 0
            || m_baseStats.m_paladinLevel > 0;
    case CGAMESPRITE_FEAT_IMPROVED_CRITICAL:
        return m_baseStats.m_attackBase >= 8;
    case CGAMESPRITE_FEAT_IMPROVED_EVASION:
        return m_baseStats.m_rogueLevel >= 10
            || m_baseStats.m_monkLevel >= 9;
    case CGAMESPRITE_FEAT_IMPROVED_TURNING:
        return m_baseStats.m_clericLevel > 0
            || m_baseStats.m_paladinLevel >= 3;
    case CGAMESPRITE_FEAT_LINGERING_SONG:
        return m_baseStats.m_bardLevel > 0;
    case CGAMESPRITE_FEAT_LUCK_OF_HEROES:
        return m_typeAI.m_nRace == CAIOBJECTTYPE_R_HUMAN;
    case CGAMESPRITE_FEAT_MARTIAL_AXE:
    case CGAMESPRITE_FEAT_MARTIAL_BOW:
    case CGAMESPRITE_FEAT_MARTIAL_FLAIL:
    case CGAMESPRITE_FEAT_MARTIAL_GREATSWORD:
    case CGAMESPRITE_FEAT_MARTIAL_HAMMER:
    case CGAMESPRITE_FEAT_MARTIAL_LARGESWORD:
    case CGAMESPRITE_FEAT_MARTIAL_POLEARM:
        return GetFeatValue(nFeatNumber) != 3 || m_baseStats.m_fighterLevel >= 4;
    case CGAMESPRITE_FEAT_MAXIMIZED_ATTACKS:
        if (1) {
            INT nWeaponSpecializations = 0;
            for (INT nFeatNumber = CGAMESPRITE_FEAT_MARTIAL_AXE; nFeatNumber <= CGAMESPRITE_FEAT_MARTIAL_POLEARM; nFeatNumber++) {
                if (GetFeatValue(nFeatNumber) == 3) {
                    nWeaponSpecializations++;
                }
            }

            if (GetFeatValue(CGAMESPRITE_FEAT_EXOTIC_BASTARD) == 3) {
                nWeaponSpecializations++;
            }

            if (nWeaponSpecializations < 2) {
                for (INT nFeatNumber = CGAMESPRITE_FEAT_SIMPLE_CROSSBOW; nFeatNumber <= CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE; nFeatNumber++) {
                    if (GetFeatValue(nFeatNumber) == 3) {
                        nWeaponSpecializations++;
                    }
                }
            }

            return nWeaponSpecializations >= 2
                && m_baseStats.m_skills[CGAMESPRITE_SKILL_CONCENTRATION] >= 4;
        }
    case CGAMESPRITE_FEAT_MERCANTILE_BACKGROUND:
        // TODO: Incomplete.
        return FALSE;
    case CGAMESPRITE_FEAT_SIMPLE_CROSSBOW:
    case CGAMESPRITE_FEAT_SIMPLE_MACE:
    case CGAMESPRITE_FEAT_SIMPLE_MISSILE:
    case CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF:
    case CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE:
        return GetFeatValue(nFeatNumber) != 3 || m_baseStats.m_fighterLevel >= 4;
    case CGAMESPRITE_FEAT_SNAKE_BLOOD:
        // TODO: Incomplete.
        return FALSE;
    case CGAMESPRITE_FEAT_STUNNING_FIST:
        return (nDEX >= 13 && nWIS >= 13 && m_baseStats.m_attackBase >= 8)
            || m_baseStats.m_monkLevel > 0;
    case CGAMESPRITE_FEAT_SUBVOCAL_CASTING:
        return m_derivedStats.HasClassMask(CLASSMASK_WIZARD | CLASSMASK_SORCERER | CLASSMASK_BARD)
            || m_derivedStats.HasClassMask(CLASSMASK_RANGER | CLASSMASK_PALADIN | CLASSMASK_DRUID | CLASSMASK_CLERIC);
    case CGAMESPRITE_FEAT_WEAPON_FINESSE:
        return GetFeatValue(CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE)
            && m_baseStats.m_attackBase >= 1;
    case CGAMESPRITE_FEAT_WILDSHAPE_BOAR:
    case CGAMESPRITE_FEAT_WILDSHAPE_PANTHER:
        return m_baseStats.m_druidLevel >= 5;
    case CGAMESPRITE_FEAT_WILDSHAPE_SHAMBLER:
        return m_baseStats.m_druidLevel >= 8;
    }

    return FALSE;
}

// 0x763A40
BOOL CGameSprite::CanSelectFeat(UINT nFeatNumber, INT a2)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 29873
    UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

    switch (nFeatNumber) {
    case CGAMESPRITE_FEAT_ARMOR_PROF:
        return m_baseStats.m_featArmorProficiency < CGAMESPRITE_FEAT_MAX_ARMOR_PROFICIENCY_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_ARMORED_ARCANA:
        return m_baseStats.m_featArmoredArcana < CGAMESPRITE_FEAT_MAX_ARMORED_ARCANA_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_CLEAVE:
        return m_baseStats.m_featCleave < CGAMESPRITE_FEAT_MAX_CLEAVE_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_EXOTIC_BASTARD:
        return m_baseStats.m_featBastardSword < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_EXTRA_RAGE:
        return m_baseStats.m_featExtraRage < CGAMESPRITE_FEAT_MAX_EXTRA_RAGE_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_EXTRA_SHAPESHIFTING:
        return m_baseStats.m_featExtraShapeshifting < CGAMESPRITE_FEAT_MAX_EXTRA_SHAPESHIFTING_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_EXTRA_SMITING:
        return m_baseStats.m_featExtraSmiting < CGAMESPRITE_FEAT_MAX_EXTRA_SMITING_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_EXTRA_TURNING:
        return m_baseStats.m_featExtraTurning < CGAMESPRITE_FEAT_MAX_EXTRA_TURNING_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_AXE:
        return (m_baseStats.m_featAxe != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featAxe < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_BOW:
        return (m_baseStats.m_featBow != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featBow < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_FLAIL:
        return (m_baseStats.m_featFlail != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featFlail < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_GREATSWORD:
        return (m_baseStats.m_featGreatSword != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featGreatSword < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_HAMMER:
        return (m_baseStats.m_featHammer != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featHammer < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_LARGESWORD:
        return (m_baseStats.m_featLargeSword != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featLargeSword < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_MARTIAL_POLEARM:
        return (m_baseStats.m_featPolearm != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featPolearm < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SIMPLE_CROSSBOW:
        return (m_baseStats.m_featCrossbow != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featCrossbow < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SIMPLE_MACE:
        return (m_baseStats.m_featMace != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featMace < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SIMPLE_MISSILE:
        return (m_baseStats.m_featMissile != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featMissile < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF:
        return (m_baseStats.m_featQuarterStaff != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featQuarterStaff < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE:
        return (m_baseStats.m_featSmallBlade != CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE - 1
                   || m_baseStats.m_attackBase >= 4)
            && m_baseStats.m_featSmallBlade < CGAMESPRITE_FEAT_MAX_WEAPON_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SPELL_FOCUS_ENCHANTMENT:
        return m_baseStats.m_featFocusEnchantment < CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_ENCHANTMENT_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SPELL_FOCUS_EVOCATION:
        return m_baseStats.m_featFocusEvocation < CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_EVOCATION_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SPELL_FOCUS_NECROMANCY:
        return m_baseStats.m_featFocusNecromancy < CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_NECROMANCY_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SPELL_FOCUS_TRANSMUTE:
        return m_baseStats.m_featFocusTransmutation < CGAMESPRITE_FEAT_MAX_SPELL_FOCUS_TRANSMUTATION_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_SPELL_PENETRATION:
        return m_baseStats.m_featSpellPenetration < CGAMESPRITE_FEAT_MAX_SPELL_PENETRATION_UPGRADE
            && CheckFeatPrerequisites(nFeatNumber, 1);
    case CGAMESPRITE_FEAT_TOUGHNESS:
        return m_baseStats.m_featToughness < CGAMESPRITE_FEAT_TOUGHNESS_MAX_LEVEL
            && CheckFeatPrerequisites(nFeatNumber, 1);
    }

    if ((m_baseStats.m_feats[nFeatNumber >> 5] & (1 << (nFeatNumber & 0x1F))) != 0) {
        return FALSE;
    }

    if (a2) {
        return CheckFeatPrerequisites(nFeatNumber, 1);
    }

    return TRUE;
}

// 0x763CB0
int CGameSprite::GetExtraFeats(BYTE nClass)
{
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    int classLevel = m_derivedStats.GetClassLevel(nClass);

    int feats = atol(ruleTables.m_tFeatLevel.GetAt(CPoint(0, m_derivedStats.m_nLevel - 1)));
    feats += atol(ruleTables.m_tFeatClass.GetAt(CPoint(nClass - 1, classLevel - 1)));

    if (m_derivedStats.m_nLevel == 1) {
        if (m_typeAI.m_nRace == CAIOBJECTTYPE_R_HUMAN
            && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_PURERACE) {
            feats++;
        } else if (m_typeAI.m_nRace == CAIOBJECTTYPE_R_HALFLING
            && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_HALFLING_STRONGHEART) {
            feats++;
        }
    }

    return feats;
}

// 0x763DA0
int CGameSprite::GetExtraSkillPoints(BYTE nClass)
{
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    int sp = atol(ruleTables.m_tSkillPoints.GetAt(CPoint(nClass - 1, 0)));

    sp += ruleTables.GetAbilityScoreModifier(m_baseStats.m_INTBase);

    if (sp < 1) {
        sp = 1;
    }

    if (m_derivedStats.m_nLevel == 1) {
        sp *= 4;
    }

    if (m_typeAI.m_nRace == CAIOBJECTTYPE_R_HUMAN
        && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_PURERACE) {
        if (m_derivedStats.m_nLevel == 1) {
            sp++;
        }
        sp++;
    }

    return sp;
}

// 0x764270
void CGameSprite::SetSkillValue(UINT iSkillNumber, INT iSkillValue)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 30424
    UTIL_ASSERT(iSkillValue >= 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 30425
    UTIL_ASSERT(iSkillNumber < CGAMESPRITE_SKILL_NUMSKILLS);

    m_baseStats.m_skills[iSkillNumber] = static_cast<BYTE>(iSkillValue);
}

// 0x7642D0
INT CGameSprite::GetSkillValue(UINT iSkillNumber)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 30444
    UTIL_ASSERT(iSkillNumber < CGAMESPRITE_SKILL_NUMSKILLS);

    return m_baseStats.m_skills[iSkillNumber];
}

// 0x764310
INT CGameSprite::GetSkillCost(UINT iSkillNumber, BYTE nClass)
{
    INT iClassType;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 30463
    UTIL_ASSERT(iSkillNumber < CGAMESPRITE_SKILL_NUMSKILLS);

    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    int v1 = 0;
    int v2 = 0;
    int v3 = 0;

    // A way to bypass condition below without `goto`. Need a better
    // understanding of the calculations to get rid of it.
    // TODO: Review calculations.
    bool check = true;

    for (iClassType = 1; iClassType <= CAIOBJECT_CLASS_MAX; iClassType++) {
        if (m_derivedStats.HasClass(iClassType)) {
            INT nCost = atol(ruleTables.m_tSkillCosts.GetAt(CPoint(iClassType - 1, iSkillNumber)));
            if (nCost == 1) {
                v3 = 1;
                if (iClassType == nClass) {
                    v2 = 1;
                    check = false;
                    break;
                }

                if (v1 > 0) {
                    check = false;
                    break;
                }
            } else if (nCost == 2) {
                if (v3 == 0) {
                    v3 = 2;
                }

                if (iClassType == nClass) {
                    v2 = 2;
                    v1 = 2;
                }
            } else if (nCost > 2) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
                // __LINE__: 30508
                UTIL_ASSERT_MSG(FALSE, "Invalid Skill Cost (not 0, 1 or 2) in SKILCOST.2DA");
            }
        }
    }

    if (check) {
        if (v1 <= 0) {
            return v1;
        }
    }

    INT nLevels = 0;
    for (iClassType = 1; iClassType <= CAIOBJECT_CLASS_MAX; iClassType++) {
        INT nLevel = m_derivedStats.GetClassLevel(iClassType);
        if (nLevel > 0) {
            INT nCost = atol(ruleTables.m_tSkillCosts.GetAt(CPoint(iClassType - 1, iSkillNumber)));
            if (nCost > 0) {
                nLevels += nLevel;
            }
        }
    }

    if (m_baseStats.m_skills[iSkillNumber] < (nLevels + 3) / v3) {
        return v2;
    }

    return 0;
}

// 0x764520
INT CGameSprite::GetEffectiveCharacterLevel()
{
    INT nLevel = m_derivedStats.m_nLevel;

    switch (m_typeAI.m_nRace) {
    case CAIOBJECTTYPE_R_HUMAN:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_HUMAN_AASIMAR:
        case CAIOBJECTTYPE_SUBRACE_HUMAN_TIEFLING:
            nLevel += 1;
            break;
        }
        break;
    case CAIOBJECTTYPE_R_ELF:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_ELF_DROW:
            nLevel += 2;
            break;
        }
        break;
    case CAIOBJECTTYPE_R_DWARF:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_DWARF_GRAY:
            nLevel += 2;
            break;
        }
        break;
    case CAIOBJECTTYPE_R_GNOME:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_GNOME_DEEP:
            nLevel += 3;
            break;
        }
        break;
    }

    return nLevel;
}

// 0x764590
INT CGameSprite::GetRacialFavoredClass(DWORD& nSpecialization)
{
    INT iClass;

    nSpecialization = 0;

    switch (m_typeAI.m_nRace) {
    case CAIOBJECTTYPE_R_HUMAN:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_PURERACE:
            iClass = m_derivedStats.GetBestClass();
            if (m_derivedStats.m_nLevel == 0) {
                iClass = CAIOBJECTTYPE_C_FIGHTER;
            }
            break;
        case CAIOBJECTTYPE_SUBRACE_HUMAN_AASIMAR:
            iClass = CAIOBJECTTYPE_C_PALADIN;
            break;
        case CAIOBJECTTYPE_SUBRACE_HUMAN_TIEFLING:
            iClass = CAIOBJECTTYPE_C_ROGUE;
            break;
        }
        break;
    case CAIOBJECTTYPE_R_ELF:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_PURERACE:
            iClass = CAIOBJECTTYPE_C_WIZARD;
            break;
        case CAIOBJECTTYPE_SUBRACE_ELF_DROW:
            switch (m_typeAI.m_nGender) {
            case CAIOBJECTTYPE_SEX_MALE:
                iClass = CAIOBJECTTYPE_C_WIZARD;
                break;
            case CAIOBJECTTYPE_SEX_FEMALE:
                iClass = CAIOBJECTTYPE_C_CLERIC;
                break;
            default:
                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
                // __LINE__: 30684
                UTIL_ASSERT_MSG(FALSE, "Character has no gender");
            }
            break;
        case CAIOBJECTTYPE_SUBRACE_ELF_WILD:
            iClass = CAIOBJECTTYPE_C_SORCERER;
            break;
        }
        break;
    case CAIOBJECTTYPE_R_HALF_ELF:
        iClass = m_derivedStats.GetBestClass();
        if (m_derivedStats.m_nLevel == 0) {
            iClass = CAIOBJECTTYPE_C_FIGHTER;
        }
        break;
    case CAIOBJECTTYPE_R_DWARF:
        iClass = CAIOBJECTTYPE_C_FIGHTER;
        break;
    case CAIOBJECTTYPE_R_HALFLING:
        switch (m_typeAI.m_nSubRace) {
        case CAIOBJECTTYPE_SUBRACE_PURERACE:
        case CAIOBJECTTYPE_SUBRACE_HALFLING_STRONGHEART:
            iClass = CAIOBJECTTYPE_C_ROGUE;
            break;
        case CAIOBJECTTYPE_SUBRACE_HALFLING_GHOSTWISE:
            // TODO: This does not look right.
            iClass = CAIOBJECTTYPE_C_BARBARIAN;
            break;
        }
        break;
    case CAIOBJECTTYPE_R_GNOME:
        iClass = CAIOBJECTTYPE_C_WIZARD;
        nSpecialization = 0x400; // WIZARD_ILLUSIONIST
        break;
    case CAIOBJECTTYPE_R_HALF_ORC:
        iClass = CAIOBJECTTYPE_C_BARBARIAN;
        break;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 30733
    UTIL_ASSERT_MSG(iClass != 0, "GetRacialFavoredClass() returned 0!");

    return iClass;
}

// 0x7646E0
INT CGameSprite::GetMulticlassingPenalty()
{
    if (m_derivedStats.m_nLevel == 0) {
        return 0;
    }

    DWORD nSpecialization = 0;
    INT iExcludedRacialClass = 0;
    INT iRacialFavoredClass = GetRacialFavoredClass(nSpecialization);
    INT iBestClass = m_derivedStats.GetBestClass();

    if (iRacialFavoredClass == iBestClass
        && (nSpecialization == 0
            || (m_baseStats.m_specialization & nSpecialization) != 0)) {
        if (m_derivedStats.GetClassLevel(iBestClass) == m_derivedStats.m_nLevel) {
            // Means there is no multiclassing.
            return 0;
        }

        iExcludedRacialClass = iRacialFavoredClass;
        iBestClass = 0;
        for (INT iClass = 1; iClass <= CAIOBJECT_CLASS_MAX; iClass++) {
            if (iClass != iRacialFavoredClass) {
                if (m_derivedStats.GetClassLevel(iClass) != 0) {
                    if (iBestClass == 0
                        || m_derivedStats.GetClassLevel(iClass) > m_derivedStats.GetClassLevel(iBestClass)) {
                        iBestClass = iClass;
                    }
                }
            }
        }
    }

    INT nBestClassLevel = m_derivedStats.GetClassLevel(iBestClass);
    if (nBestClassLevel <= 2) {
        return 0;
    }

    INT nPenalty = 0;
    for (INT iClass = 1; iClass <= CAIOBJECT_CLASS_MAX; iClass++) {
        if (iClass != iExcludedRacialClass && iClass != iBestClass) {
            INT nClassLevel = m_derivedStats.GetClassLevel(iClass);
            if (nClassLevel != 0) {
                if (nBestClassLevel - nClassLevel > 1
                    && iClass != iRacialFavoredClass) {
                    nPenalty += 20;
                }
            }
        }
    }

    // 100% penalty, really?
    if (nPenalty > 100) {
        nPenalty = 100;
    }

    return nPenalty;
}

// 0x765BD0
void CGameSprite::SetColorRange(BYTE rangeValue)
{
    m_hasColorRangeEffects = TRUE;

    // NOTE: Uninline.
    m_animation.SetColorRangeAll(rangeValue);

    CColorRange* pColorRange = new CColorRange();
    pColorRange->m_range = -1;
    pColorRange->m_color = rangeValue;
    m_derivedStats.m_appliedColorRanges.AddTail(pColorRange);
}

// 0x765C50
void CGameSprite::DisplayFeats(CUIControlTextDisplay* pText)
{
    CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 31489
    UTIL_ASSERT(pText != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 31490
    UTIL_ASSERT(pEngine != NULL);

    for (int index = 0; index < CGAMESPRITE_FEAT_NUMFEATS; index++) {
        UINT nFeatNumber = ruleTables.GetFeatId(index);
        INT nFeatRank = GetFeatValue(nFeatNumber);
        if (nFeatRank > 0) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 29233
            UTIL_ASSERT(nFeatNumber < CGAMESPRITE_FEAT_NUMFEATS);

            if ((m_baseStats.m_feats[nFeatNumber >> 5] & (1 << (nFeatNumber & 0x1F))) != 0) {
                switch (nFeatNumber) {
                case CGAMESPRITE_FEAT_ARMOR_PROF:
                case CGAMESPRITE_FEAT_ARMORED_ARCANA:
                case CGAMESPRITE_FEAT_CLEAVE:
                case CGAMESPRITE_FEAT_EXOTIC_BASTARD:
                case CGAMESPRITE_FEAT_EXTRA_RAGE:
                case CGAMESPRITE_FEAT_EXTRA_SHAPESHIFTING:
                case CGAMESPRITE_FEAT_EXTRA_SMITING:
                case CGAMESPRITE_FEAT_EXTRA_TURNING:
                case CGAMESPRITE_FEAT_MARTIAL_AXE:
                case CGAMESPRITE_FEAT_MARTIAL_BOW:
                case CGAMESPRITE_FEAT_MARTIAL_FLAIL:
                case CGAMESPRITE_FEAT_MARTIAL_GREATSWORD:
                case CGAMESPRITE_FEAT_MARTIAL_HAMMER:
                case CGAMESPRITE_FEAT_MARTIAL_LARGESWORD:
                case CGAMESPRITE_FEAT_MARTIAL_POLEARM:
                case CGAMESPRITE_FEAT_SIMPLE_CROSSBOW:
                case CGAMESPRITE_FEAT_SIMPLE_MACE:
                case CGAMESPRITE_FEAT_SIMPLE_MISSILE:
                case CGAMESPRITE_FEAT_SIMPLE_QUARTERSTAFF:
                case CGAMESPRITE_FEAT_SIMPLE_SMALLBLADE:
                case CGAMESPRITE_FEAT_SPELL_FOCUS_ENCHANTMENT:
                case CGAMESPRITE_FEAT_SPELL_FOCUS_EVOCATION:
                case CGAMESPRITE_FEAT_SPELL_FOCUS_NECROMANCY:
                case CGAMESPRITE_FEAT_SPELL_FOCUS_TRANSMUTE:
                case CGAMESPRITE_FEAT_SPELL_PENETRATION:
                case CGAMESPRITE_FEAT_TOUGHNESS:
                    CBaldurEngine::UpdateText(pText,
                        "%s: %d",
                        (LPCSTR)CBaldurEngine::FetchString(ruleTables.GetFeatName(nFeatNumber)),
                        nFeatRank);
                    break;
                default:
                    CBaldurEngine::UpdateText(pText,
                        "%s",
                        (LPCSTR)CBaldurEngine::FetchString(ruleTables.GetFeatName(nFeatNumber)));
                    break;
                }
            } else {
                CBaldurEngine::UpdateText(pText,
                    "%s",
                    (LPCSTR)CBaldurEngine::FetchString(ruleTables.GetFeatName(nFeatNumber)));
            }
        }
    }
}

// 0x765E40
void CGameSprite::DisplaySkills(CUIControlTextDisplay* pText)
{
    CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 31521
    UTIL_ASSERT(pText != NULL);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 31522
    UTIL_ASSERT(pEngine != NULL);

    for (int index = 0; index < CGAMESPRITE_SKILL_NUMSKILLS; index++) {
        INT nSkillId = ruleTables.GetSkillId(index);
        if (m_baseStats.m_skills[nSkillId] == m_derivedStats.m_nSkills[nSkillId]) {
            if (m_derivedStats.m_nSkills[nSkillId] != 0) {
                CBaldurEngine::UpdateText(pText,
                    "%s: %d",
                    (LPCSTR)CBaldurEngine::FetchString(ruleTables.GetSkillName(nSkillId)),
                    m_derivedStats.m_nSkills[nSkillId]);
            }
        } else {
            CBaldurEngine::UpdateText(pText,
                "%s: %d (%d)",
                (LPCSTR)CBaldurEngine::FetchString(ruleTables.GetSkillName(nSkillId)),
                m_derivedStats.m_nSkills[nSkillId],
                m_baseStats.m_skills[nSkillId]);
        }
    }
}

// 0x766140
SHORT CGameSprite::JumpToArea(CString areaName, const CPoint& dest, SHORT facingDirection, SHORT delay)
{
    // TODO: Incomplete.

    return ACTION_ERROR;
}

// 0x766380
void CGameSprite::MoveOntoArea(CGameArea* pArea, const CPoint& dest, SHORT facingDirection)
{
    // TODO: Incomplete.
}

// 0x74C0D0
SHORT CGameSprite::LeaveArea()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    SHORT nMyPortrait = pGame->GetCharacterPortraitNum(m_id);
    LONG nDestX = m_curAction.m_dest.x;
    LONG nDestY = m_curAction.m_dest.y;
    LONG nLeavingEdge = m_curAction.m_specificID;
    BOOL bAllAtEdge = TRUE;

    if (m_pArea != NULL) {
        m_pArea->SaveMusicPosition();
    }

    // Not a viewed character and not a tracked familiar either -- nothing to
    // gather for this sprite.
    if (nMyPortrait == -1 && pGame->m_familiars.Find(reinterpret_cast<int*>(m_id)) == NULL) {
        return ACTION_DONE;
    }

    if (!Orderable(FALSE)) {
        return ACTION_DONE;
    }

    // Am I myself close enough to the leaving edge?
    {
        LONG dy = nDestY - m_pos.y;
        LONG dx = nDestX - m_pos.x;
        if ((dy * dy * 16) / 9 + dx * dx > LEAVEAREA_GATHER_RANGE) {
            return ACTION_DONE;
        }
    }

    if (!pGame->m_singlePlayerPermissions.GetSinglePermission(CGamePermission::AREA_TRANSITION)) {
        if (!pGame->m_soundNeedParty.cSound.IsSoundPlaying()) {
            pGame->m_soundNeedParty.cSound.SetChannel(0, 0);
            pGame->m_soundNeedParty.cSound.Play(FALSE);
            g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                pGame->m_soundAreaTransitionRefused.szText,
                -1,
                FALSE);
        }
        return ACTION_DONE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        // Multiplayer readiness vote: unless every viewer's screen is clear, a
        // majority of ready players blocks the request; failing that, at
        // least one live NPC in the party still lets the travel through.
        if (pGame->m_multiplayerSettings.CountViewedCharacters() < 1) {
            if (pGame->m_multiplayerSettings.CountReadyPlayers() > 0) {
                FeedBack(0x5c, 0, 0, 0, -1, 0, 0);
                return ACTION_DONE;
            }
            if (pGame->CountNPCs() < 1) {
                goto gatherParty;
            }
        }
        FeedBack(0x3b, 0, 0, 0, -1, 0, 0);
        return ACTION_DONE;
    }

gatherParty:
    // Whether *I* am within the tighter RANGE_EDGE trigger radius of the
    // destination (vs. the broader LEAVEAREA_GATHER_RANGE checked above).
    {
        LONG dy = nDestY - m_pos.y;
        LONG dx = nDestX - m_pos.x;
        if ((dy * dy * 16) / 9 + dx * dx > CGameTrigger::RANGE_EDGE) {
            bAllAtEdge = FALSE;
        }
    }

    // Sweep the rest of the viewed party: everyone must be in this area,
    // orderable (or owned by me over the network), and within
    // LEAVEAREA_GATHER_RANGE of the leaving edge, before the party can travel
    // together. A member who fails re-queues my own LeaveArea for a retry.
    for (SHORT nPortrait = 0; nPortrait < pGame->m_nCharacters; nPortrait++) {
        if (nPortrait == nMyPortrait) {
            continue;
        }

        LONG nCharacterId = (nPortrait < pGame->m_nCharacters)
            ? pGame->m_characterPortraits[nPortrait]
            : CGameObjectArray::INVALID_INDEX;

        CGameObject* pObject = NULL;
        BYTE rc = pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 17313
            UTIL_ASSERT_MSG(FALSE, "FALSE");
            return ACTION_ERROR;
        }
        CGameSprite* pMember = static_cast<CGameSprite*>(pObject);

        if (pMember->m_bInvisible != 0) {
            pGame->GetObjectArray()->ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            STR_RES strRes;
            g_pBaldurChitin->GetTlkTable().Fetch(0x66ae, strRes);
            g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""), strRes.szText, -1, FALSE);
            return ACTION_ERROR;
        }

        CPoint& memberPos = pMember->GetPos();
        if ((pMember->m_derivedStats.m_generalState & 0x800) == 0) {
            BOOL bOwnedHere = !Orderable(FALSE)
                && (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pMember->m_remotePlayerID);
            LONG dy = nDestY - memberPos.y;
            LONG dx = nDestX - memberPos.x;
            LONG distSq = (dy * dy * 16) / 9 + dx * dx;

            if (bOwnedHere || pMember->m_pArea != m_pArea || distSq > LEAVEAREA_GATHER_RANGE) {
                pGame->GetObjectArray()->ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

                BOOL bStillOwnedHere = !Orderable(FALSE)
                    && (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                        || g_pChitin->cNetwork.m_idLocalPlayer == pMember->m_remotePlayerID);
                if ((bStillOwnedHere || pMember->m_pArea != m_pArea || !pMember->m_bSelected)
                    && !pGame->m_soundNeedParty.cSound.IsSoundPlaying()
                    && !s_bLeaveAreaMessageShown) {
                    pGame->m_soundNeedParty.cSound.SetChannel(0, 0);
                    pGame->m_soundNeedParty.cSound.Play(FALSE);
                    g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                        pGame->m_soundNeedParty.szText,
                        -1,
                        FALSE);
                    s_bLeaveAreaMessageShown = TRUE;
                }

                AddAction(CAIAction(CAIAction::LEAVEAREA, CPoint(-1, -1), 0, -1));
                return ACTION_ERROR;
            }

            if (!bAllAtEdge && distSq <= CGameTrigger::RANGE_EDGE) {
                bAllAtEdge = TRUE;
            }

            // 0x5872C0: Icewind586B70's per-summon dismiss/message cleanup for
            // this now-gathered member -- unrecovered (CGameObjectArray deny-lock
            // + CMessageHandler flow), not reproduced. Missing better than wrong.
        }

        pGame->GetObjectArray()->ReleaseShare(nCharacterId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    s_bLeaveAreaMessageShown = FALSE;

    // Third sweep: the familiars list (CInfGame::m_familiars stores character
    // ids as ints disguised as pointers). Same gather check, but a familiar
    // never blocks on invisibility and never runs the Icewind586B70 cleanup
    // done for viewed party members above.
    for (POSITION pos = pGame->m_familiars.GetHeadPosition(); pos != NULL;) {
        LONG nFamiliarId = reinterpret_cast<LONG>(pGame->m_familiars.GetNext(pos));
        if (nFamiliarId == m_id) {
            continue;
        }

        CGameObject* pObject = NULL;
        BYTE rc = pGame->GetObjectArray()->GetShare(nFamiliarId,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
            // __LINE__: 17394
            UTIL_ASSERT_MSG(FALSE, "FALSE");
            return ACTION_ERROR;
        }
        CGameSprite* pFamiliar = static_cast<CGameSprite*>(pObject);

        CPoint& familiarPos = pFamiliar->GetPos();
        if ((pFamiliar->m_derivedStats.m_generalState & 0x800) == 0) {
            BOOL bOwnedHere = !Orderable(FALSE)
                && (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pFamiliar->m_remotePlayerID);
            LONG dy = nDestY - familiarPos.y;
            LONG dx = nDestX - familiarPos.x;
            LONG distSq = (dy * dy * 16) / 9 + dx * dx;

            if (bOwnedHere || pFamiliar->m_pArea != m_pArea || distSq > LEAVEAREA_GATHER_RANGE) {
                pGame->GetObjectArray()->ReleaseShare(nFamiliarId, CGameObjectArray::THREAD_ASYNCH, INFINITE);

                BOOL bStillOwnedHere = !Orderable(FALSE)
                    && (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                        || g_pChitin->cNetwork.m_idLocalPlayer == pFamiliar->m_remotePlayerID);
                if ((bStillOwnedHere || pFamiliar->m_pArea != m_pArea || !pFamiliar->m_bSelected)
                    && !pGame->m_soundNeedParty.cSound.IsSoundPlaying()
                    && !s_bLeaveAreaMessageShown) {
                    pGame->m_soundNeedParty.cSound.SetChannel(0, 0);
                    pGame->m_soundNeedParty.cSound.Play(FALSE);
                    g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                        pGame->m_soundNeedParty.szText,
                        -1,
                        FALSE);
                    s_bLeaveAreaMessageShown = TRUE;
                }

                AddAction(CAIAction(CAIAction::LEAVEAREA, CPoint(-1, -1), 0, -1));
                return ACTION_ERROR;
            }

            if (!bAllAtEdge && distSq <= CGameTrigger::RANGE_EDGE) {
                bAllAtEdge = TRUE;
            }
        }

        pGame->GetObjectArray()->ReleaseShare(nFamiliarId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }

    s_bLeaveAreaMessageShown = FALSE;

    if (!bAllAtEdge) {
        // Everyone gathered, but nobody (including me) is within the tighter
        // RANGE_EDGE trigger radius yet -- keep waiting at the edge.
        if (!pGame->m_soundNeedParty.cSound.IsSoundPlaying() && !s_bLeaveAreaMessageShown) {
            pGame->m_soundNeedParty.cSound.SetChannel(0, 0);
            pGame->m_soundNeedParty.cSound.Play(FALSE);
            g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                pGame->m_soundNeedParty.szText,
                -1,
                FALSE);
            s_bLeaveAreaMessageShown = TRUE;
        }
        return ACTION_ERROR;
    }

    // Whole party gathered and at the edge -- commit to the transition.
    g_pBaldurChitin->cSoundMixer.StopMusic(FALSE);
    pGame->UnselectAll();

    if (g_pChitin->cNetwork.m_bConnectionEstablished) {
        if (!g_pChitin->cNetwork.m_bIsHost) {
            // 0x435150: MP client -> host area-transition request over
            // CNetwork::SendSpecificMessage -- unrecovered (single caller,
            // MP-only). Matches the CGameAIBase::ExecuteAction
            // MultiPlayerSync precedent (0x466750) of deferring the MP
            // handshake and using the binary's own SP-observable outcome.
            return ACTION_DONE;
        }
        g_pBaldurChitin->GetBaldurMessage()->SendMapWorldAnnounceStatus(TRUE,
            g_pChitin->cNetwork.m_idLocalPlayer,
            static_cast<SHORT>(nLeavingEdge));
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreatureAI.cpp
    // __LINE__: 17525
    UTIL_ASSERT_MSG(g_pBaldurChitin->m_pEngineWorldMap != NULL, "pWorldMap != NULL");
    g_pBaldurChitin->m_pEngineWorldMap->StartWorldMap(TRUE, static_cast<SHORT>(nLeavingEdge), TRUE);
    g_pChitin->pActiveEngine->SelectEngine(g_pBaldurChitin->m_pEngineWorldMap);
    return ACTION_DONE;
}

// 0x42FDC0
void CGameSprite::SetResRef(const CResRef& resRef)
{
    m_resRef = resRef;
}

// 0x453160
void CGameSprite::SetHiding(int a1)
{
    m_bHiding = a1;
}

// 0x453170
int CGameSprite::GetHiding()
{
    return m_bHiding;
}

// 0x453180
SHORT CGameSprite::GetSequence()
{
    return m_nSequence;
}

// 0x453190
CCreatureFileHeader* CGameSprite::GetBaseStats()
{
    return &m_baseStats;
}

// 0x4531A0
CDerivedStats* CGameSprite::GetDerivedStats()
{
    return &m_derivedStats;
}

// 0x4531B0
void CGameSprite::MarkRenderDirty()
{
    field_562C = 1;
}

// 0x4531C0
CGameEffectList* CGameSprite::GetEquipedEffectList()
{
    return &m_equipedEffectList;
}

// 0x4531D0
CGameEffectList* CGameSprite::GetTimedEffectList()
{
    return &m_timedEffectList;
}

// 0x4531E0
void CGameSprite::SetStealthGreyOut(LONG greyOut)
{
    m_nStealthGreyOut = greyOut;
}

// 0x4AEF20
void CGameSprite::AddPortraitIcon(int icon)
{
    if (m_portraitIcons.Find(reinterpret_cast<int*>(icon), 0) == NULL) {
        m_portraitIcons.AddTail(reinterpret_cast<int*>(icon));
    }
}

// NOTE: Inlined.
void CGameSprite::RemovePortraitIcon(int icon)
{
    POSITION pos = m_portraitIcons.Find(reinterpret_cast<int*>(icon), 0);
    if (pos != NULL) {
        m_portraitIcons.RemoveAt(pos);
    }
}

// 0x5F6810
CGameSpriteSpellList* CGameSprite::GetInnateSpells()
{
    return &m_innateSpells;
}

// 0x5F6820
CGameSpriteSpellList* CGameSprite::GetSongs()
{
    return &m_songs;
}

// 0x58FED0
CGameSpriteSpellList* CGameSprite::GetShapeshifts()
{
    return &m_shapeshifts;
}

// 0x5940E0
INT CGameSprite::GetCustomButtonValue(BYTE buttonNum)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2028
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_CUSTOM_BUTTONS22);

    return field_3D14[buttonNum];
}

// 0x594120
void CGameSprite::SetCustomButtonValue(BYTE buttonNum, int a2)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2036
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_CUSTOM_BUTTONS22);

    field_3D14[buttonNum] = a2;
}

// 0x594160
BYTE CGameSprite::GetModalState()
{
    return m_nModalState;
}

// 0x45B710
int CGameSprite::GetHideState()
{
    return m_bInvisible;
}

// 0x706D50
SHORT CGameSprite::GetDirection(const CPoint& target)
{
    if (m_pos == target) {
        return m_nNewDirection;
    }

    CPoint ptStart(m_pos.x, 4 * m_pos.y / 3);
    CPoint ptTarget(target.x, 4 * target.y / 3);
    return GetDirection(ptStart, ptTarget);
}

// 0x45B720
SHORT CGameSprite::GetDirection()
{
    return m_nDirection;
}

// 0x706DD0
SHORT CGameSprite::GetDirection(const CPoint& ptStart, const CPoint& ptTarget)
{
    LONG dx;
    LONG dy;

    if (ptStart == ptTarget) {
        return 0;
    }

    if (ptTarget.y < ptStart.y) {
        if (ptTarget.x < ptStart.x) {
            dx = ptStart.x - ptTarget.x;
            dy = ptStart.y - ptTarget.y;

            if (27 * dy <= 6 * dx) {
                return 4;
            } else if (27 * dy <= 16 * dx) {
                return 5;
            } else if (24 * dy <= 38 * dx) {
                return 6;
            } else if (7 * dy <= 38 * dx) {
                return 7;
            } else {
                return 8;
            }
        } else {
            dx = ptTarget.x - ptStart.x;
            dy = ptStart.y - ptTarget.y;

            if (27 * dy <= 6 * dx) {
                return 12;
            } else if (27 * dy <= 16 * dx) {
                return 11;
            } else if (24 * dy <= 38 * dx) {
                return 10;
            } else if (7 * dy <= 38 * dx) {
                return 9;
            } else {
                return 8;
            }
        }
    } else {
        if (ptTarget.x < ptStart.x) {
            dx = ptStart.x - ptTarget.x;
            dy = ptTarget.y - ptStart.y;

            if (27 * dy <= 6 * dx) {
                return 4;
            } else if (27 * dy <= 16 * dx) {
                return 3;
            } else if (24 * dy <= 38 * dx) {
                return 2;
            } else if (7 * dy <= 38 * dx) {
                return 1;
            } else {
                return 0;
            }
        } else {
            dx = ptTarget.x - ptStart.x;
            dy = ptTarget.y - ptStart.y;

            if (27 * dy <= 6 * dx) {
                return 12;
            } else if (27 * dy <= 16 * dx) {
                return 13;
            } else if (24 * dy <= 38 * dx) {
                return 14;
            } else if (7 * dy <= 38 * dx) {
                return 15;
            } else {
                return 0;
            }
        }
    }
}

// 0x45B730
CGameAnimation* CGameSprite::GetAnimation()
{
    return &m_animation;
}

// 0x45B740
CGameSpriteEquipment* CGameSprite::GetEquipment()
{
    return &m_equipment;
}

// 0x45B750
SHORT CGameSprite::GetHappiness()
{
    return m_nHappiness;
}

// 0x45B760
BOOL CGameSprite::GetActive()
{
    return m_active;
}

// 0x45B770
CVariableHash* CGameSprite::GetLocalVariables()
{
    return m_pLocalVariables;
}

// 0x724010
INT CGameSprite::GetMaxDexterityBonus(INT a1)
{
    CItem* pItem = m_equipment.m_items[1];
    if (pItem == NULL) {
        return a1;
    }

    switch (pItem->GetItemType()) {
    case 60:
        return min(6, a1);
    case 61:
        return min(5, a1);
    case 62:
        return min(2, a1);
    case 63:
    case 64:
        return min(0, a1);
    case 65:
        return min(1, a1);
    case 66:
        return min(4, a1);
    case 68:
        return min(3, a1);
    default:
        return a1;
    }
}

// 0x763E40
//
// Racial / class / feat / ability-score modifier for one skill, on top of the
// base ranks.  Column 3 of SKILLS.2DA is the usable-untrained flag; columns
// 15+ hold the per-deity-order bonuses applied to cleric levels.
INT CGameSprite::GetSkillModifier(INT iSkillNumber)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();
    const C2DArray& tSkills = game->GetRuleTables().m_tSkills;

    LONG nUntrained = tSkills.GetAtLong(CPoint(3, iSkillNumber));

    // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameSprite.cpp
    // __LINE__: 30444
    UTIL_ASSERT(iSkillNumber < CGAMESPRITE_SKILL_NUMSKILLS);

    if (m_baseStats.m_skills[iSkillNumber] == 0 && nUntrained == 0) {
        return 0;
    }

    const CRuleTables& rule = game->GetRuleTables();

    INT nBonus = 0;
    if (m_derivedStats.HasClass(CAIOBJECTTYPE_C_CLERIC)) {
        DWORD nOrder;
        switch (m_baseStats.m_specialization & 0xFF8000) {
        case SPECMASK_CLERIC_ILMATER:
            nOrder = 0;
            break;
        case SPECMASK_CLERIC_LATHANDER:
            nOrder = 1;
            break;
        case SPECMASK_CLERIC_SELUNE:
            nOrder = 2;
            break;
        case SPECMASK_CLERIC_HELM:
            nOrder = 3;
            break;
        case SPECMASK_CLERIC_OGHMA:
            nOrder = 4;
            break;
        case SPECMASK_CLERIC_TEMPUS:
            nOrder = 5;
            break;
        case SPECMASK_CLERIC_BANE:
            nOrder = 6;
            break;
        case SPECMASK_CLERIC_MASK:
            nOrder = 7;
            break;
        case SPECMASK_CLERIC_TALOS:
            nOrder = 8;
            break;
        default:
            // Unknown deity mask: the original indexes the table with the
            // skill number itself.
            nOrder = iSkillNumber;
            break;
        }
        nBonus = tSkills.GetAtLong(CPoint(nOrder + 15, iSkillNumber));
    }

    switch (iSkillNumber) {
    case CGAMESPRITE_SKILL_ALCHEMY:
        if (m_typeAI.m_nRace == CAIOBJECTTYPE_R_GNOME
            && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_PURERACE) {
            nBonus += 2;
        }
        if (m_derivedStats.HasClass(CAIOBJECTTYPE_C_BARD)) {
            nBonus += 1;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT) + nBonus;
    case CGAMESPRITE_SKILL_ANIMAL_EMPATHY:
    case CGAMESPRITE_SKILL_BLUFF:
    case CGAMESPRITE_SKILL_USE_MAGIC_DEVICE:
        if (m_typeAI.m_nRace == CAIOBJECTTYPE_R_HUMAN
            && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_HUMAN_TIEFLING
            && iSkillNumber == CGAMESPRITE_SKILL_BLUFF) {
            nBonus += 2;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nCHR) + nBonus;
    case CGAMESPRITE_SKILL_CONCENTRATION: {
        INT nModifier = rule.GetAbilityScoreModifier(m_derivedStats.m_nCON);
        if (HasFeat(CGAMESPRITE_FEAT_DISCIPLINE)) {
            nModifier += 2;
        }
        if (HasFeat(CGAMESPRITE_FEAT_COMBAT_CASTING)) {
            nModifier += 4;
        }
        return nModifier;
    }
    case CGAMESPRITE_SKILL_DIPLOMACY:
        if (HasFeat(CGAMESPRITE_FEAT_COURTEOUS_MAGOCRACY)) {
            nBonus += 2;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nCHR) + nBonus;
    case CGAMESPRITE_SKILL_DISABLE_DEVICE:
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT);
    case CGAMESPRITE_SKILL_HIDE:
        if ((m_typeAI.m_nRace == CAIOBJECTTYPE_R_HUMAN
                && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_HUMAN_TIEFLING)
            || (m_typeAI.m_nRace == CAIOBJECTTYPE_R_GNOME
                && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_GNOME_DEEP)) {
            nBonus += 2;
        }
        return GetMaxDexterityBonus(rule.GetAbilityScoreModifier(m_derivedStats.m_nDEX)) + nBonus;
    case CGAMESPRITE_SKILL_INTIMIDATE:
        if (HasFeat(CGAMESPRITE_FEAT_BULLHEADED)) {
            nBonus += 2;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nCHR) + nBonus;
    case CGAMESPRITE_SKILL_KNOWLEDGE_ARCANA:
        if (m_derivedStats.HasClass(CAIOBJECTTYPE_C_BARD)) {
            nBonus += 1;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT) + nBonus;
    case CGAMESPRITE_SKILL_MOVE_SILENTLY:
        if (m_typeAI.m_nRace == CAIOBJECTTYPE_R_HALFLING
            || (m_typeAI.m_nRace == CAIOBJECTTYPE_R_DWARF
                && m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_DWARF_GRAY)) {
            nBonus += 4;
        }
        return GetMaxDexterityBonus(rule.GetAbilityScoreModifier(m_derivedStats.m_nDEX)) + nBonus;
    case CGAMESPRITE_SKILL_OPEN_LOCK:
    case CGAMESPRITE_SKILL_PICK_POCKET:
        return GetMaxDexterityBonus(rule.GetAbilityScoreModifier(m_derivedStats.m_nDEX)) + nBonus;
    case CGAMESPRITE_SKILL_SEARCH:
        switch (m_typeAI.m_nRace) {
        case CAIOBJECTTYPE_R_ELF:
        case CAIOBJECTTYPE_R_DWARF:
            return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT) + nBonus + 2;
        case CAIOBJECTTYPE_R_HALF_ELF:
            nBonus += 1;
            break;
        case CAIOBJECTTYPE_R_GNOME:
            if (m_typeAI.m_nSubRace == CAIOBJECTTYPE_SUBRACE_GNOME_DEEP) {
                return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT) + nBonus + 2;
            }
            break;
        default:
            break;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT) + nBonus;
    case CGAMESPRITE_SKILL_SPELLCRAFT:
        if (HasFeat(CGAMESPRITE_FEAT_COURTEOUS_MAGOCRACY)) {
            nBonus += 2;
        }
        if (m_derivedStats.HasClass(CAIOBJECTTYPE_C_BARD)) {
            nBonus += 1;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nINT) + nBonus;
    case CGAMESPRITE_SKILL_WILDERNESS_LORE:
        if (HasFeat(CGAMESPRITE_FEAT_FORESTER)) {
            nBonus += 2;
        }
        return rule.GetAbilityScoreModifier(m_derivedStats.m_nWIS) + nBonus;
    default:
        return 0;
    }
}

// 0x71ABA0
//
// Adjusts the use count shown on every quick-bar slot matching cAbility and
// clears (or disables) the slot when it runs out.  Quick items stop at the
// first match; quick spells / innates only toggle the disabled state unless
// bRemove / bRemoveOnEmpty force a full clear.
void CGameSprite::UpdateQuickButtons(const CAbilityId& cAbility, SHORT nDelta, BOOL bRemove, BOOL bRemoveOnEmpty)
{
    CInfGame* game = g_pBaldurChitin->GetObjectGame();

    if (!m_bGlobal && game->m_allies.Find(reinterpret_cast<int*>(m_id), NULL) == NULL) {
        return;
    }

    for (INT index = 0; index < 8; ++index) {
        CButtonData& button = m_quickWeapons[index];
        if (button.m_abilityId.m_itemType == cAbility.m_itemType
            && button.m_abilityId.m_itemNum == cAbility.m_itemNum
            && button.m_abilityId.m_abilityNum == cAbility.m_abilityNum
            && button.m_abilityId.m_res == cAbility.m_res
            && button.m_abilityId.m_nClass == cAbility.m_nClass
            && button.m_abilityId.m_bCanUse == cAbility.m_bCanUse
            && button.m_abilityId.m_nTooltip == cAbility.m_nTooltip) {
            SHORT nCount = button.m_count + nDelta;
            button.m_count = nCount;
            if (nCount < 0) {
                nCount = 0;
            }
            button.m_count = nCount;
            if (nCount < 1 || bRemove) {
                button = CButtonData();
                field_3D3A[index] = 0;
            }
        }
    }

    for (INT index = 0; index < 9; ++index) {
        CButtonData& button = m_quickSpells[index];
        if (button.m_abilityId.m_itemType == cAbility.m_itemType
            && button.m_abilityId.m_itemNum == cAbility.m_itemNum
            && button.m_abilityId.m_abilityNum == cAbility.m_abilityNum
            && button.m_abilityId.m_res == cAbility.m_res
            && button.m_abilityId.m_nClass == cAbility.m_nClass
            && button.m_abilityId.m_bCanUse == cAbility.m_bCanUse
            && button.m_abilityId.m_nTooltip == cAbility.m_nTooltip) {
            SHORT nCount = button.m_count + nDelta;
            button.m_count = nCount;
            if (nCount < 0) {
                nCount = 0;
            }
            button.m_count = nCount;
            if (!bRemove && !(nCount < 1 && bRemoveOnEmpty)) {
                button.m_bDisabled = nCount < 1;
            } else {
                button = CButtonData();
            }
        }
    }

    for (INT index = 0; index < 3; ++index) {
        CButtonData& button = m_quickItems[index];
        if (button.m_abilityId.m_itemType == cAbility.m_itemType
            && button.m_abilityId.m_itemNum == cAbility.m_itemNum
            && button.m_abilityId.m_abilityNum == cAbility.m_abilityNum
            && button.m_abilityId.m_res == cAbility.m_res
            && button.m_abilityId.m_nClass == cAbility.m_nClass
            && button.m_abilityId.m_bCanUse == cAbility.m_bCanUse
            && button.m_abilityId.m_nTooltip == cAbility.m_nTooltip) {
            SHORT nCount = button.m_count + nDelta;
            button.m_count = nCount;
            if (nCount < 0) {
                nCount = 0;
            }
            button.m_count = nCount;
            if (bRemove || (nCount < 1 && bRemoveOnEmpty)) {
                button = CButtonData();
            }
            break;
        }
    }

    for (INT index = 0; index < 9; ++index) {
        CButtonData& button = m_quickInnates[index];
        if (button.m_abilityId.m_itemType == cAbility.m_itemType
            && button.m_abilityId.m_itemNum == cAbility.m_itemNum
            && button.m_abilityId.m_abilityNum == cAbility.m_abilityNum
            && button.m_abilityId.m_res == cAbility.m_res
            && button.m_abilityId.m_nClass == cAbility.m_nClass
            && button.m_abilityId.m_bCanUse == cAbility.m_bCanUse
            && button.m_abilityId.m_nTooltip == cAbility.m_nTooltip) {
            SHORT nCount = button.m_count + nDelta;
            button.m_count = nCount;
            if (nCount < 0) {
                nCount = 0;
            }
            button.m_count = nCount;
            if (!bRemove && !(nCount < 1 && bRemoveOnEmpty)) {
                button.m_bDisabled = nCount < 1;
            } else {
                button = CButtonData();
            }
        }
    }

    for (INT index = 0; index < 6; ++index) {
        CButtonData& button = m_quickSongs[index];
        if (button.m_abilityId.m_itemType == cAbility.m_itemType
            && button.m_abilityId.m_itemNum == cAbility.m_itemNum
            && button.m_abilityId.m_abilityNum == cAbility.m_abilityNum
            && button.m_abilityId.m_res == cAbility.m_res
            && button.m_abilityId.m_nClass == cAbility.m_nClass
            && button.m_abilityId.m_bCanUse == cAbility.m_bCanUse
            && button.m_abilityId.m_nTooltip == cAbility.m_nTooltip) {
            SHORT nCount = button.m_count + nDelta;
            button.m_count = nCount;
            if (nCount < 0) {
                nCount = 0;
            }
            button.m_count = nCount;
            if (bRemove || (nCount < 1 && bRemoveOnEmpty)) {
                button = CButtonData();
            }
        }
    }

    CBaldurEngine* pEngine = static_cast<CBaldurEngine*>(g_pBaldurChitin->pActiveEngine);
    if (pEngine->GetSelectedCharacter() == game->GetCharacterPortraitNum(m_id)) {
        game->GetButtonArray()->UpdateState();
    }

    if (game->m_group.InList(m_id)) {
        CUIManager* pManager = g_pBaldurChitin->m_pEngineWorld->GetManager();
        CUIPanel* pPanel = pManager->GetPanel(3);
        if (pPanel != NULL) {
            pPanel->InvalidateRect(NULL);
        }
    }
}

// 0x7240A0
INT CGameSprite::GetArmorCheckPenalty()
{
    CItem* pItem = m_equipment.m_items[1];
    if (pItem == NULL) {
        return 0;
    }

    INT v1 = 0;

    INT nItemType = pItem->GetItemType();
    INT nFeatValue = GetFeatValue(CGAMESPRITE_FEAT_ARMOR_PROF);
    switch (nItemType) {
    case 60:
        if (nFeatValue >= 1) {
            return v1;
        }
        return 0;
    case 61:
        if (nFeatValue >= 1) {
            return v1;
        }
        v1 = -1;
        break;
    case 62:
        if (nFeatValue >= 2) {
            return v1;
        }
        v1 = -5;
        break;
    case 63:
    case 64:
        if (nFeatValue >= 3) {
            return v1;
        }
        v1 = -7;
        break;
    case 65:
        if (nFeatValue >= 3) {
            return v1;
        }
        v1 = -6;
        break;
    case 66:
        if (nFeatValue >= 2) {
            return v1;
        }
        v1 = -3;
        break;
    case 68:
        if (nFeatValue >= 3) {
            return v1;
        }
        v1 = -4;
        break;
    default:
        return v1;
    }

    if ((pItem->m_flags & 0x8) != 0) {
        v1++;
    }

    return v1;
}

// 0x724170
INT CGameSprite::GetShieldCheckPenalty()
{
    INT nSlot = 2 * (m_nWeaponSet + 22);
    CItem* pItem = m_equipment.m_items[nSlot];
    if (pItem == NULL) {
        return 0;
    }

    INT v1 = 0;

    INT nItemType = pItem->GetItemType();
    if (GetFeatValue(CGAMESPRITE_FEAT_SHIELD_PROF) < 1) {
        switch (nItemType) {
        case 41:
            if (pItem->GetResRef() != CResRef("00BARD04")
                && pItem->GetResRef() != CResRef("00HFBD04")) {
                v1 = -1;
                if ((pItem->m_flags & 0x8) != 0) {
                    v1++;
                }
            }
            break;
        case 47:
        case 49:
            v1 = -2;
            if ((pItem->m_flags & 0x8) != 0) {
                v1++;
            }
            break;
        case 53:
            v1 = -1;
            if ((pItem->m_flags & 0x8) != 0) {
                v1++;
            }
            break;
        }
    }

    return v1;
}

// 0x724270
INT CGameSprite::GetShieldSpellFailure()
{
    INT nSlot = 2 * (m_nWeaponSet + 22);
    CItem* pItem = m_equipment.m_items[nSlot];
    if (pItem == NULL) {
        return 0;
    }

    switch (pItem->GetItemType()) {
    case 41:
        if (pItem->GetResRef() != CResRef("00BARD04")
            && pItem->GetResRef() != CResRef("00HFBD04")) {
            return 5;
        }
        return 0;
    case 47:
        return 50;
    case 49:
        return 15;
    case 53:
        return 5;
    default:
        return 0;
    }
}

// 0x724360
INT CGameSprite::GetArmorSpellFailure()
{
    CItem* pItem = m_equipment.m_items[1];
    if (pItem == NULL) {
        return 0;
    }

    switch (pItem->GetItemType()) {
    case 60:
        return 10;
    case 61:
        return 15;
    case 62:
        return 30;
    case 63:
    case 64:
        return 40;
    case 65:
        return 35;
    case 66:
        return 20;
    case 68:
        return 25;
    default:
        return 0;
    }
}

// 0x7243F0
BOOL CGameSprite::CheckAranceFailure(INT nRoll)
{
    INT nFailureChance = GetArcaneSpellFailure();

    if (nRoll >= nFailureChance) {
        return FALSE;
    }

    // "Spell Failure check: Roll d100 %d vs. Spell failure chance %d"
    FeedBack(83, nRoll, nFailureChance, 0, 40955, 0, 0);

    return TRUE;
}

// 0x724430
int CGameSprite::GetArcaneSpellFailure()
{
    INT nMod = 0;

    if (m_derivedStats.m_spellStates[SPLSTATE_MENTAL_DOMINATION]) {
        nMod = 100;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_BLINK]) {
        nMod += 20;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_DEAFENED]) {
        nMod += 20;
    }

    INT nFailureChance = m_derivedStats.m_nSpellFailureArcane
        + nMod
        + GetArmorSpellFailure()
        + GetShieldSpellFailure();

    if (HasFeat(CGAMESPRITE_FEAT_ARMORED_ARCANA)) {
        nFailureChance -= 5 * GetFeatValue(CGAMESPRITE_FEAT_ARMORED_ARCANA);
    }

    if (nFailureChance > 100) {
        nFailureChance = 100;
    } else if (nFailureChance < 0) {
        nFailureChance = 0;
    }

    return nFailureChance;
}

// 0x724540
BOOL CGameSprite::CheckDivineFailure(INT nRoll)
{
    INT nMod = 0;

    if (m_derivedStats.m_spellStates[SPLSTATE_MENTAL_DOMINATION]) {
        nMod = 100;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_BLINK]) {
        nMod += 20;
    }

    if (m_derivedStats.m_spellStates[SPLSTATE_DEAFENED]) {
        nMod += 20;
    }

    INT nFailureChance = m_derivedStats.m_nSpellFailureDivine + nMod;
    if (nFailureChance > 100) {
        nFailureChance = 100;
    } else if (nFailureChance < 0) {
        nFailureChance = 0;
    }

    if (nRoll >= nFailureChance) {
        return FALSE;
    }

    // "Spell Failure check: Roll d100 %d vs. Spell failure chance %d"
    FeedBack(FEEDBACK_ROLL, nRoll, nFailureChance, 0, 40955, 0, 0);

    return TRUE;
}

// 0x7245D0
BOOLEAN CGameSprite::IsHelpless()
{
    return m_derivedStats.m_spellStates[SPLSTATE_HELD]
        || m_derivedStats.m_spellStates[SPLSTATE_HOPELESSNESS]
        || m_derivedStats.m_spellStates[SPLSTATE_OTILUKES_RESILIENT_SPHERE];
}

// 0x7245F0
INT CGameSprite::GetNextHatedRacesSlot()
{
    INT nIndex;

    for (nIndex = 0; nIndex < MAX_HATED_RACES; nIndex++) {
        if (m_baseStats.m_favoredEnemies[nIndex] == CAIObjectType::R_NO_RACE) {
            break;
        }
    }

    return nIndex;
}

// 0x724610
void CGameSprite::ResetQuickSlots()
{
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();
    INT nClass = m_derivedStats.GetBestClass() - 1;

    for (int nSlot = 0; nSlot < CGAMESAVECHARACTER_NUM_CUSTOM_BUTTONS22; nSlot++) {
        if (field_3D14[nSlot] == 0) {
            field_3D14[nSlot] = atol(ruleTables.m_tQuickSlots.GetAt(CPoint(nSlot, nClass)));
        }
    }
}

// 0x724690
BOOL CGameSprite::HasArmorType(SHORT a1)
{
    CItem* pItem = m_equipment.m_items[1];
    if (pItem == NULL) {
        return FALSE;
    }

    WORD nItemType = pItem->GetItemType();
    switch (a1) {
    case 0:
        switch (nItemType) {
        case 67:
            return TRUE;
        }
        break;
    case 1:
        switch (nItemType) {
        case 60:
        case 61:
            return TRUE;
        }
        break;
    case 2:
        switch (nItemType) {
        case 62:
        case 66:
        case 68:
            return TRUE;
        }
        break;
    case 3:
        switch (nItemType) {
        case 63:
        case 64:
        case 65:
            return TRUE;
        }
        break;
    }

    return FALSE;
}

// FIXME: `nClass` should not be reference.
//
// 0x724730
CGameSpriteGroupedSpellList* CGameSprite::GetSpells(const BYTE& nClass)
{
    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 25826
    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

    return &(m_spells.m_spellsByClass[nClassIndex]);
}

// FIXME: `nClass` should not be reference.
// FIXME: `nLevel` should not be reference.
//
// 0x724790
CGameSpriteSpellList* CGameSprite::GetSpellsAtLevel(const BYTE& nClass, const UINT& nLevel)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 25835
    UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 25838
    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 2521
    UTIL_ASSERT(nLevel < CSPELLLIST_MAX_LEVELS);

    return &(m_spells.m_spellsByClass[nClassIndex].m_lists[nLevel]);
}

// 0x724840
UINT CGameSprite::GetNumSpells()
{
    UINT nCount = 0;

    for (int index = 0; index < CSPELLLIST_NUM_CLASSES; index++) {
        // NOTE: Uninline.
        nCount += m_spells.m_spellsByClass[index].GetNumSpells();
    }

    // NOTE: Uninline.
    nCount += m_domainSpells.GetNumSpells();

    return nCount;
}

// 0x724900
BOOLEAN CGameSprite::IsSpellcaster()
{
    DWORD dwClassMask = GetAIType().m_nClassMask;

    return (dwClassMask & (CLASSMASK_BARD | CLASSMASK_CLERIC | CLASSMASK_DRUID | CLASSMASK_PALADIN | CLASSMASK_RANGER)) != 0
        || (dwClassMask & (CLASSMASK_SORCERER | CLASSMASK_WIZARD)) != 0;
}

// 0x724920
BOOLEAN CGameSprite::IsBard()
{
    return (GetAIType().m_nClassMask & CLASSMASK_BARD) != 0;
}

// FIXME: Review references (latter params unclear).
//
// 0x724930
BOOLEAN CGameSprite::AddKnownSpell(const BYTE& nClass, const UINT& nSpellLevel, const CResRef& resRef, const unsigned int& a4, const unsigned int& a5, const unsigned int& a6)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 25936
    UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);

    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    // NOTE: Uninline.
    CGameSpriteGroupedSpellList* pSpellsByLevel = m_spells.Get(nClassIndex);

    // NOTE: Uninline.
    return pSpellsByLevel->Add(nID, nSpellLevel, a4, a5, a6);
}

// 0x724A40
BOOLEAN CGameSprite::AddDomainSpell(const UINT& nSpellLevel, const CResRef& resRef, const unsigned int& a3, const unsigned int& a4, const unsigned int& a5)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 25954
    UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    // NOTE: Uninline.
    m_domainSpells.Add(nID, nSpellLevel, a3, a4, a5);

    return TRUE;
}

// 0x724B10
BOOLEAN CGameSprite::AddInnateSpell(const CResRef& resRef, const unsigned int& a2, const unsigned int& a3, const unsigned int& a4)
{
    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->GetInnateSpells()->Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    m_innateSpells.Add(nID, a2, a3, a4);

    return TRUE;
}

// 0x724B70
BOOLEAN CGameSprite::AddSong(const CResRef& resRef, const unsigned int& a2, const unsigned int& a3, const unsigned int& a4)
{
    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    m_songs.Add(nID, a2, a3, a4);

    return TRUE;
}

// 0x724BD0
BOOLEAN CGameSprite::AddShapeshift(const CResRef& resRef, const unsigned int& a2)
{
    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->GetShapeshifts()->Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    m_shapeshifts.Add(nID, 0, 0, a2);

    return TRUE;
}

// 0x724C40
BOOLEAN CGameSprite::AllocateShapeshiftSlots(const unsigned int& a1)
{
    m_shapeshifts.m_nSharedMax += a1;

    for (UINT nIndex = 0; nIndex < m_shapeshifts.m_List.size(); nIndex++) {
        CGameSpriteSpellListEntry* pEntry = m_shapeshifts.Get(nIndex);

        m_shapeshifts.Add(pEntry->m_nID,
            m_shapeshifts.m_nSharedMax - pEntry->m_nMax,
            0,
            0);
    }

    return FALSE;
}

// 0x724D30
BOOLEAN CGameSprite::RemoveKnownSpell(const BYTE& nClass, const UINT& nSpellLevel, const CResRef& resRef, const unsigned int& a4, const unsigned int& a5, const unsigned int& a6)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 25954
    UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);

    UINT nID;
    if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    // NOTE: Uninline.
    CGameSpriteGroupedSpellList* pSpellsByLevel = m_spells.Get(nClassIndex);

    return pSpellsByLevel->Remove(nID, nSpellLevel, a4, a5, a6);
}

// 0x724E00
BOOLEAN CGameSprite::RemoveDomainSpell(const UINT& nSpellLevel, const CResRef& resRef, const unsigned int& a3, const unsigned int& a4, const unsigned int& a5)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 26073
    UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

    if (!m_derivedStats.HasClass(CAIOBJECTTYPE_C_CLERIC)) {
        return FALSE;
    }

    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    return m_domainSpells.Remove(nID, nSpellLevel, a3, a4, a5);
}

// 0x724EA0
BOOLEAN CGameSprite::RemoveInnateSpell(const CResRef& resRef, const unsigned int& a2, const unsigned int& a3, const unsigned int& a4)
{
    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->GetInnateSpells()->Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    return m_innateSpells.Remove(nID, a2, a3, a4);
}

// 0x724F00
BOOLEAN CGameSprite::RemoveSong(const CResRef& resRef, const unsigned int& a2, const unsigned int& a3, const unsigned int& a4)
{
    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->GetMasterSongLookup().Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    return m_songs.Remove(nID, a2, a3, a4);
}

// 0x724F60
BOOLEAN CGameSprite::RemoveShapeshift(const CResRef& resRef)
{
    UINT nID = 0;
    if (g_pBaldurChitin->GetObjectGame()->GetShapeshifts()->Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    return m_shapeshifts.Remove(nID, 1, 0, 0);
}

// 0x724FD0
BOOLEAN CGameSprite::SubtractFromSpellCount(const BYTE& nClass, const UINT& nSpellLevel, const CResRef& resRef, const unsigned int& a4, const unsigned int& a5)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 26177
    UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

    UINT nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);

    UINT nID;
    if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    // NOTE: Uninline.
    CGameSpriteGroupedSpellList* pSpellsByLevel = m_spells.Get(nClassIndex);

    // NOTE: Uninline.
    return pSpellsByLevel->SubtractFromCurrentCountAtLevel(nID, nSpellLevel, a4, a5);
}

// 0x725110
BOOLEAN CGameSprite::SubtractFromDomainSpellCount(const UINT& nSpellLevel, const CResRef& resRef, const unsigned int& a4, const unsigned int& a5)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 26211
    UTIL_ASSERT(nSpellLevel < CSPELLLIST_MAX_LEVELS);

    UINT nID;
    if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    // NOTE: Uninline.
    CGameSpriteSpellList* pList = m_domainSpells.GetSpellsAtLevel(nSpellLevel);

    // NOTE: Uninline.
    return pList->SubtractFromCurrentCount(nID, a4, a5);
}

// 0x725210
BOOLEAN CGameSprite::AddToInnateSpellCount(const CResRef& resRef, const unsigned int& a2, const unsigned int& a3)
{
    UINT nID;
    if (g_pBaldurChitin->GetObjectGame()->GetInnateSpells()->Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    return m_innateSpells.AddToCurrentCount(nID, a2, a3);
}

// 0x725270
BOOLEAN CGameSprite::SubtractFromInnateSpellCount(const CResRef& resRef, const unsigned int& a2, const unsigned int& a3)
{
    UINT nID;
    if (g_pBaldurChitin->GetObjectGame()->GetInnateSpells()->Find(resRef, nID) != TRUE) {
        return FALSE;
    }

    // NOTE: Uninline.
    return m_innateSpells.SubtractFromCurrentCount(nID, a2, a3);
}

// FIXME: `dwClassMask` should not be reference.
//
// 0x725330
BOOLEAN CGameSprite::HasSpell(const CResRef& resRef, const DWORD& dwClassMask, UINT nLevel, BOOLEAN a4)
{
    BOOLEAN v1 = FALSE;
    UINT nStartLevel = 0;
    UINT nEndLevel = 9;

    if (nLevel < CSPELLLIST_MAX_LEVELS) {
        nStartLevel = nLevel;
        nStartLevel = nLevel + 1;
    }

    UINT nID;
    if (!g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID)) {
        return FALSE;
    }

    for (UINT nClassIndex = 0; nClassIndex < CSPELLLIST_NUM_CLASSES; nClassIndex++) {
        if (v1) {
            break;
        }

        if ((g_pBaldurChitin->GetObjectGame()->GetSpellcasterClassMask(nClassIndex) & dwClassMask) != 0) {
            for (UINT nCurrLevel = nStartLevel; nCurrLevel < nEndLevel; nCurrLevel++) {
                if (v1) {
                    break;
                }

                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                // __LINE__: 26902
                UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
                // __LINE__: 26903
                UTIL_ASSERT(nCurrLevel < CSPELLLIST_MAX_LEVELS);

                UINT nIndex = 0;
                v1 = m_spells.m_spellsByClass[nClassIndex].m_lists[nCurrLevel].Find(nID, nIndex);
                if (v1 == TRUE && a4 == TRUE) {
                    // __FILE__: .\Include\FileFormat.h
                    // __LINE__: 2565
                    UTIL_ASSERT(nClassIndex < CSPELLLIST_NUM_CLASSES);

                    // NOTE: Uninline.
                    v1 = m_spells.m_spellsByClass[nClassIndex].m_lists[nCurrLevel].CheckF8(nIndex);
                }
            }
        }
    }

    if (!v1) {
        if ((dwClassMask & CLASSMASK_CLERIC) == CLASSMASK_CLERIC) {
            for (UINT nCurrLevel = nStartLevel; nCurrLevel < nEndLevel; nCurrLevel++) {
                if (v1) {
                    break;
                }

                UINT nIndex = 0;

                // NOTE: Uninline.
                v1 = m_domainSpells.Find(nID, nStartLevel, nIndex);
                if (v1 == TRUE && a4 == TRUE) {
                    // NOTE: Uninline.
                    v1 = m_innateSpells.CheckF8(nIndex);
                }
            }
        }
    }

    return v1;
}

// 0x7256B0
BOOLEAN CGameSprite::HasDomainSpell(const CResRef& resRef, const UINT& nLevel, BOOLEAN a3)
{
    BOOLEAN v1 = FALSE;
    UINT nStartLevel = 0;
    UINT nEndLevel = 9;

    if (nLevel < CSPELLLIST_MAX_LEVELS) {
        nStartLevel = nLevel;
        // TODO: Looks odd, should it be `nLevel + 1` as in the function above
        // (0x725330) to check only one specific level?
        nEndLevel = nLevel - 1;
    }

    UINT nID;
    if (!g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 26395
        UTIL_ASSERT(FALSE);
    }

    for (UINT nCurrLevel = nStartLevel; nCurrLevel < nEndLevel; nCurrLevel++) {
        if (v1) {
            break;
        }

        UINT nIndex;

        // NOTE: Uninline.
        v1 = m_domainSpells.Find(nID, nStartLevel, nIndex);
        if (v1 == TRUE && a3 == TRUE) {
            // NOTE: Uninline.
            v1 = m_innateSpells.CheckF8(nIndex);
        }
    }

    return v1;
}

// 0x725840
BOOLEAN CGameSprite::HasInnateSpell(const CResRef& resRef, BOOLEAN a2)
{
    UINT nID;
    if (!g_pBaldurChitin->GetObjectGame()->GetInnateSpells()->Find(resRef, nID)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
        // __LINE__: 26420
        UTIL_ASSERT(FALSE);
    }

    UINT nIndex;
    BOOLEAN v1 = m_innateSpells.Find(nID, nIndex);

    if (v1 && a2 == TRUE) {
        v1 = m_innateSpells.CheckF8(nIndex);
    }

    return v1;
}

// 0x726270
INT CGameSprite::GetFeatRank(UINT nFeatNumber)
{
    INT v1 = 0;
    if (HasFeat(nFeatNumber)) {
        switch (nFeatNumber) {
        case CGAMESPRITE_FEAT_ARTERIAL_STRIKE:
            v1 = m_nFeatRanks[2];
            break;
        case CGAMESPRITE_FEAT_EXPERTISE:
            v1 = m_nFeatRanks[0];
            break;
        case CGAMESPRITE_FEAT_HAMSTRING:
            v1 = m_nFeatRanks[3];
            break;
        case CGAMESPRITE_FEAT_POWER_ATTACK:
            v1 = m_nFeatRanks[1];
            break;
        case CGAMESPRITE_FEAT_RAPID_SHOT:
            v1 = m_nFeatRanks[4];
            break;
        }
    }
    return v1;
}

// 0x726330
void CGameSprite::SetFeatRank(UINT nFeatNumber, INT nValue)
{
    if (HasFeat(nFeatNumber)) {
        switch (nFeatNumber) {
        case CGAMESPRITE_FEAT_ARTERIAL_STRIKE:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26976
            UTIL_ASSERT_MSG(nValue >= 0, "Invalid feat level.");

            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26977
            UTIL_ASSERT_MSG(nValue <= 1, "Invalid feat level.");

            m_nFeatRanks[2] = nValue;
            m_nFeatRanks[3] = 0;
            break;
        case CGAMESPRITE_FEAT_EXPERTISE:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26962
            UTIL_ASSERT_MSG(nValue >= 0, "Invalid feat level.");

            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26963
            UTIL_ASSERT_MSG(nValue <= MAX_SELECTABLE_FEAT_USE_LEVELS, "Invalid feat level.");

            m_nFeatRanks[0] = nValue;
            m_nFeatRanks[1] = 0;
            break;
        case CGAMESPRITE_FEAT_HAMSTRING:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26983
            UTIL_ASSERT_MSG(nValue >= 0, "Invalid feat level.");

            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26984
            UTIL_ASSERT_MSG(nValue <= 1, "Invalid feat level.");

            m_nFeatRanks[3] = nValue;
            m_nFeatRanks[2] = 0;
            break;
        case CGAMESPRITE_FEAT_POWER_ATTACK:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26969
            UTIL_ASSERT_MSG(nValue >= 0, "Invalid feat level.");

            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26970
            UTIL_ASSERT_MSG(nValue <= MAX_SELECTABLE_FEAT_USE_LEVELS, "Invalid feat level.");

            m_nFeatRanks[1] = nValue;
            m_nFeatRanks[0] = 0;
            break;
        case CGAMESPRITE_FEAT_RAPID_SHOT:
            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 26990
            UTIL_ASSERT_MSG(nValue >= 0, "Invalid feat level.");

            // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
            // __LINE__: 269991
            UTIL_ASSERT_MSG(nValue <= 1, "Invalid feat level.");

            m_nFeatRanks[4] = nValue;
            break;
        }
    }
}

// 0x726570
void CGameSprite::RefreshCombatFeats()
{
    ITEM_EFFECT effect;
    CGameEffect* pEffect;
    CMessageAddEffect* pMessage;

    if (m_nFeatRanks[1] > 0) {
        CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_FEATPOWERATTACK);
        pEffect = CGameEffect::DecodeEffect(&effect, m_pos, m_id, CPoint(-1, -1));
        pMessage = new CMessageAddEffect(pEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    if (m_nFeatRanks[0] > 0) {
        CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_FEATEXPERTISE);
        pEffect = CGameEffect::DecodeEffect(&effect, m_pos, m_id, CPoint(-1, -1));
        pMessage = new CMessageAddEffect(pEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    if (m_nFeatRanks[2] > 0) {
        CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_FEATARTERIALSTRIKE);
        pEffect = CGameEffect::DecodeEffect(&effect, m_pos, m_id, CPoint(-1, -1));
        pMessage = new CMessageAddEffect(pEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    if (m_nFeatRanks[3] > 0) {
        CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_FEATHAMSTRING);
        pEffect = CGameEffect::DecodeEffect(&effect, m_pos, m_id, CPoint(-1, -1));
        pMessage = new CMessageAddEffect(pEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }

    if (m_nFeatRanks[4] > 0) {
        CGameEffect::ClearItemEffect(&effect, ICEWIND_CGAMEEFFECT_FEATRAPIDSHOT);
        pEffect = CGameEffect::DecodeEffect(&effect, m_pos, m_id, CPoint(-1, -1));
        pMessage = new CMessageAddEffect(pEffect, m_id, m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    }
}

// 0x726800
INT CGameSprite::GetWeaponSlot()
{
    return 2 * m_nWeaponSet + 44;
}

namespace {

BYTE g_emptySpriteColorRangeValues[7] = { 0, 0, 0, 0, 0, 0, 0 };
const WORD g_defaultSpriteAttackProbability[6] = { 0x22, 0x21, 0x21, 0, 0, 0 };

BOOL IsShieldItemType(WORD nItemType)
{
    return nItemType == 47 || nItemType == 53 || nItemType == 49 || nItemType == 41;
}

void DeleteButtonList(CGameButtonList* pButtons)
{
    if (pButtons == NULL) {
        return;
    }

    while (!pButtons->IsEmpty()) {
        delete pButtons->RemoveHead();
    }
    delete pButtons;
}

} // namespace

// Has external linkage: also called from CInfGame::SwapItemPersonal.
void RefreshWeaponSetButtons(CGameSprite* pSprite)
{
    for (BYTE nWeaponSet = 0; nWeaponSet < 4; nWeaponSet++) {
        BYTE nWeaponSlot = static_cast<BYTE>(CGameSpriteEquipment::SLOT_WEAPON + 2 * nWeaponSet);
        BYTE nWeaponButton = static_cast<BYTE>(nWeaponSlot - CGameSpriteEquipment::SLOT_WEAPON);

        CButtonData cWeaponButton;
        pSprite->GetQuickWeapon(nWeaponButton, cWeaponButton);

        SHORT nAbility = 0;
        SHORT nDesiredItemNum = 0;
        CItem* pWeapon = pSprite->m_equipment.m_items[nWeaponSlot];
        if (pWeapon != NULL) {
            BYTE nAbilityType = 0;
            pWeapon->Demand();
            ITEM_ABILITY* pAbility = pWeapon->pRes != NULL ? pWeapon->pRes->GetAbility(0) : NULL;
            if (pAbility != NULL) {
                nAbilityType = static_cast<BYTE>(pAbility->type & 0xFF);
            }
            pWeapon->Release();

            if (nAbilityType == 4) {
                BYTE nRememberedSlot = pSprite->field_3D3A[nWeaponButton];
                if (nRememberedSlot >= CGameSpriteEquipment::SLOT_AMMO
                    && nRememberedSlot < CGameSpriteEquipment::SLOT_AMMO + 4
                    && pSprite->m_equipment.m_items[nRememberedSlot] != NULL) {
                    nDesiredItemNum = nRememberedSlot;
                    nAbility = cWeaponButton.m_abilityId.m_abilityNum;
                }
            } else {
                nAbility = cWeaponButton.m_abilityId.m_abilityNum;
            }
        }

        CGameButtonList* pButtons = pSprite->GetItemUsages(nWeaponSlot, 1, nAbility);
        if (pButtons->IsEmpty()) {
            CButtonData cDefault;
            pSprite->SetQuickWeapon(nWeaponButton, cDefault);
            pSprite->SetQuickWeapon(nWeaponButton, static_cast<BYTE>(0));
        } else {
            POSITION pos = pButtons->GetHeadPosition();
            while (pos != NULL) {
                CButtonData* pButton = pButtons->GetNext(pos);
                if (pButton != NULL && pButton->m_abilityId.m_itemNum == nDesiredItemNum) {
                    pSprite->SetQuickWeapon(nWeaponButton, *pButton);
                    pSprite->SetQuickWeapon(nWeaponButton, static_cast<BYTE>(nDesiredItemNum));
                    break;
                }
            }
        }
        DeleteButtonList(pButtons);

        BYTE nOffhandSlot = static_cast<BYTE>(nWeaponSlot + 1);
        BYTE nOffhandButton = static_cast<BYTE>(nOffhandSlot - CGameSpriteEquipment::SLOT_WEAPON);
        pButtons = pSprite->GetItemUsages(nOffhandSlot, 1, 0);
        if (pButtons->IsEmpty()) {
            CButtonData cDefault;
            pSprite->SetQuickWeapon(nOffhandButton, cDefault);
            pSprite->SetQuickWeapon(nOffhandButton, static_cast<BYTE>(0));
        } else {
            CButtonData* pButton = pButtons->GetHead();
            if (pButton != NULL) {
                pSprite->SetQuickWeapon(nOffhandButton, *pButton);
                SHORT nItemNum = pButton->m_abilityId.m_itemNum;
                if (nItemNum >= CGameSpriteEquipment::SLOT_AMMO
                    && nItemNum < CGameSpriteEquipment::SLOT_AMMO + 4) {
                    pSprite->SetQuickWeapon(nOffhandButton, static_cast<BYTE>(nItemNum));
                } else {
                    pSprite->SetQuickWeapon(nOffhandButton, static_cast<BYTE>(0));
                }
            }
        }
        DeleteButtonList(pButtons);
    }

    CMessage* pMessage = new CMessageSpriteEquipment(pSprite, pSprite->m_id, pSprite->m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
}

// 0x726810
void CGameSprite::SetWeaponSet(BYTE nWeaponSet)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\ObjCreature.cpp
    // __LINE__: 27066
    UTIL_ASSERT(nWeaponSet < 4);

    if (m_equipment.m_items[42] != NULL) {
        SelectWeaponAbility(42,
            static_cast<BYTE>(m_equipment.m_selectedWeaponAbility),
            TRUE,
            TRUE);
        return;
    }

    BYTE nOldWeaponSlot = static_cast<BYTE>(2 * m_nWeaponSet + CGameSpriteEquipment::SLOT_WEAPON);
    BYTE nOldOffhandSlot = static_cast<BYTE>(nOldWeaponSlot + 1);

    if (m_equipment.m_selectedWeapon < CGameSpriteEquipment::NUM_SLOT
        && m_equipment.m_items[m_equipment.m_selectedWeapon] != NULL) {
        CItem* pSelected = m_equipment.m_items[m_equipment.m_selectedWeapon];
        pSelected->Demand();
        ITEM_ABILITY* pAbility = pSelected->pRes != NULL ? pSelected->pRes->GetAbility(m_equipment.m_selectedWeaponAbility) : NULL;
        SHORT launcherSlot;
        CItem* pLauncher = GetLauncher(pAbility, launcherSlot);
        if (pLauncher != NULL) {
            pSelected->Unequip(this, m_equipment.m_selectedWeapon, TRUE, FALSE);
        }
        pSelected->Release();
    }

    if (m_equipment.m_items[nOldWeaponSlot] == NULL) {
        if (m_animation.m_animation != NULL) {
            CString sEmpty("");
            m_animation.m_animation->EquipWeapon(sEmpty,
                g_emptySpriteColorRangeValues,
                0,
                g_defaultSpriteAttackProbability);
        }
    } else {
        m_equipment.m_items[nOldWeaponSlot]->Unequip(this, nOldWeaponSlot, TRUE, FALSE);
    }

    if (m_equipment.m_items[nOldOffhandSlot] != NULL) {
        m_equipment.m_items[nOldOffhandSlot]->Unequip(this, nOldOffhandSlot, TRUE, FALSE);
    }

    if (m_animation.m_animation != NULL) {
        CString sEmpty("");
        m_animation.m_animation->EquipShield(sEmpty, g_emptySpriteColorRangeValues);
    }

    m_nWeaponSet = nWeaponSet;

    BYTE nWeaponSlot = static_cast<BYTE>(2 * m_nWeaponSet + CGameSpriteEquipment::SLOT_WEAPON);
    BYTE nOffhandSlot = static_cast<BYTE>(nWeaponSlot + 1);
    BYTE nWeaponButton = static_cast<BYTE>(nWeaponSlot - CGameSpriteEquipment::SLOT_WEAPON);
    BYTE nOffhandButton = static_cast<BYTE>(nOffhandSlot - CGameSpriteEquipment::SLOT_WEAPON);

    CButtonData cWeaponButton;
    GetQuickWeapon(nWeaponButton, cWeaponButton);

    CButtonData cOffhandButton;
    GetQuickWeapon(nOffhandButton, cOffhandButton);

    CItem* pWeapon = m_equipment.m_items[nWeaponSlot];
    BYTE nSelectedSlot = 0;
    BYTE nSelectedAbility = 0;
    BYTE nRememberedQuickSlot = 0;
    BOOL bEquipOffhandAfterSelect = FALSE;

    if (pWeapon == NULL) {
        CItem* pOffhand = m_equipment.m_items[nOffhandSlot];
        if (pOffhand == NULL) {
            nSelectedSlot = CGameSpriteEquipment::SLOT_FIST;
            nSelectedAbility = 0;
        } else if (IsShieldItemType(pOffhand->GetItemType())) {
            nSelectedSlot = CGameSpriteEquipment::SLOT_FIST;
            nSelectedAbility = 0;
            pOffhand->Equip(this, nOffhandSlot, FALSE);
        } else {
            nSelectedSlot = nOffhandSlot;
            nSelectedAbility = static_cast<BYTE>(cOffhandButton.m_abilityId.m_abilityNum);
        }
    } else {
        BYTE nAbilityType = 0;
        pWeapon->Demand();
        ITEM_ABILITY* pAbility = pWeapon->pRes != NULL ? pWeapon->pRes->GetAbility(0) : NULL;
        if (pAbility != NULL) {
            nAbilityType = static_cast<BYTE>(pAbility->type & 0xFF);
        }
        pWeapon->Release();

        if (nAbilityType == 4) {
            // __FILE__: .\Include\ObjCreature.h
            // __LINE__: 2021
            UTIL_ASSERT(nWeaponButton < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22);

            BYTE nQuickAmmoSlot = field_3D3A[nWeaponButton];
            if (nQuickAmmoSlot >= CGameSpriteEquipment::SLOT_AMMO
                && nQuickAmmoSlot < CGameSpriteEquipment::SLOT_AMMO + 4
                && m_equipment.m_items[nQuickAmmoSlot] != NULL) {
                CItem* pAmmo = m_equipment.m_items[nQuickAmmoSlot];
                pAmmo->Demand();
                ITEM_ABILITY* pAmmoAbility = pAmmo->pRes != NULL ? pAmmo->pRes->GetAbility(0) : NULL;
                SHORT launcherSlot;
                CItem* pLauncher = GetLauncher(pAmmoAbility, launcherSlot);
                pAmmo->Release();
                if (pLauncher == pWeapon) {
                    nSelectedSlot = nQuickAmmoSlot;
                    nSelectedAbility = static_cast<BYTE>(cWeaponButton.m_abilityId.m_abilityNum);
                    nRememberedQuickSlot = nQuickAmmoSlot;
                }
            }

            if (nSelectedSlot == 0) {
                BYTE nBestAmmoSlot = 0;
                WORD nBestAmmoCount = 0;
                for (BYTE nAmmoSlot = CGameSpriteEquipment::SLOT_AMMO; nAmmoSlot < CGameSpriteEquipment::SLOT_AMMO + 4; nAmmoSlot++) {
                    CItem* pAmmo = m_equipment.m_items[nAmmoSlot];
                    if (pAmmo == NULL) {
                        continue;
                    }

                    pAmmo->Demand();
                    ITEM_ABILITY* pAmmoAbility = pAmmo->pRes != NULL ? pAmmo->pRes->GetAbility(0) : NULL;
                    SHORT launcherSlot;
                    CItem* pLauncher = GetLauncher(pAmmoAbility, launcherSlot);
                    if (pAmmoAbility != NULL
                        && (pAmmoAbility->type & 0xFF) == 2
                        && pLauncher == pWeapon) {
                        WORD nAmmoCount = pAmmo->GetUsageCount(0);
                        if (nBestAmmoSlot == 0 || nAmmoCount > nBestAmmoCount) {
                            nBestAmmoSlot = nAmmoSlot;
                            nBestAmmoCount = nAmmoCount;
                        }
                    }
                    pAmmo->Release();
                }

                if (nBestAmmoSlot == 0) {
                    nSelectedSlot = CGameSpriteEquipment::SLOT_FIST;
                    nSelectedAbility = 0;
                } else {
                    nSelectedSlot = nBestAmmoSlot;
                    nSelectedAbility = 0;
                    nRememberedQuickSlot = nBestAmmoSlot;
                }
            }
        } else {
            nSelectedSlot = nWeaponSlot;
            nSelectedAbility = static_cast<BYTE>(cWeaponButton.m_abilityId.m_abilityNum);
        }

        if (m_equipment.m_items[nOffhandSlot] != NULL) {
            bEquipOffhandAfterSelect = TRUE;
        }
    }

    SelectWeaponAbility(nSelectedSlot, nSelectedAbility, FALSE, TRUE);

    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2031
    UTIL_ASSERT(nWeaponButton < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22);
    field_3D3A[nWeaponButton] = nRememberedQuickSlot;

    if (bEquipOffhandAfterSelect && m_equipment.m_items[nOffhandSlot] != NULL) {
        m_equipment.m_items[nOffhandSlot]->Equip(this, nOffhandSlot, FALSE);
    }

    RefreshWeaponSetButtons(this);

    MarkRenderDirty();

    int nBaseAttack;
    int nAttacks;
    int nFeat;
    g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetBaseCombatValues(this,
        nBaseAttack,
        nAttacks,
        nFeat,
        FALSE);
    m_baseStats.m_numberOfAttacksBase = static_cast<BYTE>(nAttacks);
    if (m_baseStats.m_numberOfAttacksBase > 5) {
        m_baseStats.m_numberOfAttacksBase = 5;
    }

    CMessage* pMessage = new CMessageSpriteEquipment(this, m_id, m_id);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
}

// 0x58FEE0
BYTE CGameSprite::GetLastSong()
{
    return m_nLastSong;
}

// NOTE: Inlined.
void CGameSprite::GetQuickItem(BYTE buttonNum, CButtonData& buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2022
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_ITEMS22);

    CheckToolTipItem(buttonNum);

    buttonData = m_quickItems[buttonNum];
}

// NOTE: Inlined.
void CGameSprite::GetQuickSpell(BYTE buttonNum, CButtonData& buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2023
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_SPELLS22);

    buttonData = m_quickSpells[buttonNum];
}

// NOTE: Inlined.
void CGameSprite::GetQuickAbility(BYTE buttonNum, CButtonData& buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2024
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_ABILITIES22);

    buttonData = m_quickInnates[buttonNum];
}

// NOTE: Inlined.
void CGameSprite::GetQuickSong(BYTE buttonNum, CButtonData& buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2025
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_SONGS22);

    buttonData = m_quickSongs[buttonNum];
}

// FIXME: `buttonData` should be reference.
//
// NOTE: Inlined.
void CGameSprite::SetQuickWeapon(BYTE buttonNum, CButtonData buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2030
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22);

    m_quickWeapons[buttonNum] = buttonData;
}

// NOTE: Inlined.
void CGameSprite::SetQuickWeapon(BYTE buttonNum, BYTE index)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2031
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_WEAPONS22);

    field_3D3A[buttonNum] = index;
}

// FIXME: `buttonData` should be reference.
//
// NOTE: Inlined.
void CGameSprite::SetQuickItem(BYTE buttonNum, CButtonData buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2032
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_ITEMS22);

    m_quickItems[buttonNum] = buttonData;
}

// FIXME: `buttonData` should be reference.
//
// NOTE: Inlined.
void CGameSprite::SetQuickSpell(BYTE buttonNum, CButtonData buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2033
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_SPELLS22);

    m_quickSpells[buttonNum] = buttonData;
}

// FIXME: `buttonData` should be reference.
//
// NOTE: Inlined.
void CGameSprite::SetQuickAbility(BYTE buttonNum, CButtonData buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2034
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_ABILITIES22);

    m_quickInnates[buttonNum] = buttonData;
}

// FIXME: `buttonData` should be reference.
//
// NOTE: Inlined.
void CGameSprite::SetQuickSong(BYTE buttonNum, CButtonData buttonData)
{
    // __FILE__: .\Include\ObjCreature.h
    // __LINE__: 2035
    UTIL_ASSERT(buttonNum < CGAMESAVECHARACTER_NUM_QUICK_SONGS22);

    m_quickSongs[buttonNum] = buttonData;
}

// NOTE: Inlined.
CDerivedStats* CGameSprite::GetActiveStats()
{
    if (m_bAllowEffectListCall) {
        return &m_derivedStats;
    } else {
        return &m_tempStats;
    }
}
