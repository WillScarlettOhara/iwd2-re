#ifndef CAIACTION_H_
#define CAIACTION_H_

#include "mfc.h"

#include "CAIObjectType.h"

#define CAIACTION_ATTACK 3
#define CAIACTION_LEAVEAREALUA 110
#define CAIACTION_212 212
#define CAIACTION_MOVEVIEWPOINTUNTILDONE 264

class CGameAIBase;

// Binary-mirror layout (IE packs to 2): size is 0xD6 (CGameAIBase m_curAction @0x476 ->
// field_54C @0x54C). Without pack(2) MSVC 4-aligns the LONG/CString/CPoint/DWORD tail after
// the three CAIObjectType members, inflating it to 0xD8 and shifting the CGameAIBase members
// after m_curAction.
#pragma pack(push)
#pragma pack(2)
class CAIAction {
public:
    static const SHORT NO_ACTION;
    static const SHORT ATTACK;
    static const SHORT BERSERK;
    static const SHORT DIALOGUE;
    static const SHORT MOVETOOBJECT;
    static const SHORT MOVETOPOINT;
    static const SHORT PROTECTPOINT;
    static const SHORT REMOVETRAPS;
    static const SHORT PICKPOCKETS;
    static const SHORT SPELL;
    static const SHORT USEITEM;
    static const SHORT CONTINUE;
    static const SHORT JUMPTOPOINT;
    static const SHORT MOVEVIEWPOINT;
    static const SHORT MOVEVIEWOBJECT;
    static const SHORT CLICKLBUTTONPOINT;
    static const SHORT CLICKLBUTTONOBJECT;
    static const SHORT CLICKRBUTTONPOINT;
    static const SHORT CLICKRBUTTONOBJECT;
    static const SHORT DOUBLECLICKLBUTTONPOINT;
    static const SHORT DOUBLECLICKLBUTTONOBJECT;
    static const SHORT DOUBLECLICKRBUTTONPOINT;
    static const SHORT DOUBLECLICKRBUTTONOBJECT;
    static const SHORT MOVECURSORPOINT;
    static const SHORT CHANGEAISCRIPT;
    static const SHORT STARTTIMER;
    static const SHORT SENDTRIGGER;
    static const SHORT WAIT;
    static const SHORT SMALLWAIT;
    static const SHORT FACE;
    static const SHORT RANDOMWALK;
    static const SHORT RANDOMWALKCONTINUOUS;
    static const SHORT LEADER;
    static const SHORT FOLLOW;
    static const SHORT MOVETOPOINTNORECTICLE;
    static const SHORT LEAVEAREA;
    static const SHORT GROUPATTACK;
    static const SHORT SPELLPOINT;
    static const SHORT USEITEMPOINT;
    static const SHORT ATTACKNOSOUND;
    static const SHORT ATTACKONEROUND;
    static const SHORT SHOUT;
    static const SHORT LEAVEAREALUA;
    static const SHORT USECONTAINER;
    static const SHORT FORCESPELL;
    static const SHORT FORCESPELLPOINT;
    static const SHORT TAKEPARTYGOLD;
    static const SHORT STARTDIALOG;
    static const SHORT PLAYERDIALOG;
    static const SHORT PICKLOCK;
    static const SHORT BASHDOOR;
    static const SHORT APPLYSPELL;
    static const SHORT GIVEGOLDFORCE;
    static const SHORT MOVETOOBJECTFOLLOW;
    static const SHORT REALLYFORCESPELL;
    static const SHORT SPELLNODEC;
    static const SHORT SPELLPOINTNODEC;
    static const SHORT GIVEPARTYGOLD;
    static const SHORT ATTACKREEVALUATE;
    static const SHORT RUNAWAYFROM;
    static const SHORT PANIC;
    static const SHORT RUNAWAYFROMNOINTERRUPT;
    static const SHORT STARTDIALOGUENOSET;
    static const SHORT TIMEDMOVETOPOINT;
    static const SHORT BACKGROUND;

    static const CAIAction NULL_ACTION;

    CAIAction();
    CAIAction(SHORT actionID, LONG specificID);
    CAIAction(SHORT actionID, const CAIObjectType& actee, LONG specificID, LONG specificID2, LONG specificID3);
    CAIAction(SHORT actionID, const CPoint& dest, LONG specificID, LONG specificID2);
    CAIAction(SHORT actionID, const CString& name, CPoint pt, LONG specificID);
    CAIAction(SHORT actionID, const CAIObjectType& actee, const CString& name, LONG specificID, LONG specificID2);
    CAIAction(SHORT actionID, const CString& string1, CPoint pt, LONG specificID, LONG specificID2);
    CAIAction(const CAIAction& action);
    ~CAIAction();
    BOOL OfType(SHORT type) const;
    void Read(CString sData);
    void Decode(CGameAIBase* caller);
    CAIAction& operator=(const CAIAction& other);

    const CPoint& GetDest() const;
    SHORT GetActionID() const;
    CString GetString2() const;
    DWORD GetFlags() const;
    const CAIObjectType& GetActee2() const;
    LONG GetSpecifics() const;
    LONG GetSpecifics2() const;
    LONG GetSpecifics3() const;
    CString GetString1() const;

    void SetString2(CString sText);
    void SetString1(CString sText);

    SHORT m_actionID;
    CAIObjectType m_actorID;
    CAIObjectType m_acteeID;
    CAIObjectType m_acteeID2;
    LONG m_specificID;
    LONG m_specificID2;
    LONG m_specificID3;
    CString m_string1;
    CString m_string2;
    CPoint m_dest;
    DWORD m_internalFlags;
};

#pragma pack(pop)

#endif /* CAIACTION_H_ */
