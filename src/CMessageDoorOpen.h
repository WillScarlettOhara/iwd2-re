#ifndef CMESSAGEDOOROPEN_H_
#define CMESSAGEDOOROPEN_H_

#include "CMessage.h"

#pragma pack(push, 2)
class CMessageDoorOpen : public CMessage {
public:
    CMessageDoorOpen(BOOLEAN open, LONG caller, LONG target)
        : CMessage(caller, target), m_bOpenDoor(open) {}
    SHORT GetCommType() override;
    BYTE GetMsgType() override;
    BYTE GetMsgSubType() override;
    void MarshalMessage(BYTE** pData, DWORD* dwSize) override;
    BOOL UnmarshalMessage(BYTE* pData, DWORD dwSize) override;
    void Run() override;
    /* 000C */ BOOLEAN m_bOpenDoor;
};
#pragma pack(pop)

#endif
