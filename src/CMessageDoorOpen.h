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
    // Slot 0x18 / 0x4FD1D0 remains inherited until ToggleDoor is recovered.
    /* 000C */ BOOLEAN m_bOpenDoor;
};
#pragma pack(pop)

#endif
