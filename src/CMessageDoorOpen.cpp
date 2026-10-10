#include "CMessageDoorOpen.h"
#include "CBaldurChitin.h"
#include "CInfGame.h"
#include "CGameObjectArray.h"
#include "CGameObject.h"
#include "CGameDoor.h"
#include "CGameRemoteObjectArray.h"
#include "CNetwork.h"
#include "CUtil.h"

static const DWORD STATICBUFFERSIZE = 0x4000;

// 0x40A0D0
SHORT CMessageDoorOpen::GetCommType()
{
    return SEND;
}
// 0x40A0E0
BYTE CMessageDoorOpen::GetMsgType()
{
    return CBaldurMessage::MSG_TYPE_CMESSAGE;
}
// 0x466470
BYTE CMessageDoorOpen::GetMsgSubType()
{
    return CBaldurMessage::MSG_SUBTYPE_CMESSAGE_DOOR_OPEN;
}

// 0x4FCF00
void CMessageDoorOpen::MarshalMessage(BYTE** pData, DWORD* dwSize)
{
    UTIL_ASSERT(pData != NULL && dwSize != NULL);
    PLAYER_ID targetPlayer = 0, sourcePlayer = 0;
    LONG targetObject = CGameObjectArray::INVALID_INDEX, sourceObject = CGameObjectArray::INVALID_INDEX;
    auto remote = [&](LONG id, PLAYER_ID& player, LONG& object) {
        CGameObject* pObject;
        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(id,
                CGameObjectArray::THREAD_ASYNCH, &pObject, INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
        if (rc == CGameObjectArray::SUCCESS) {
            player = pObject->m_remotePlayerID;
            object = pObject->m_remoteObjectID;
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);
        }
    };
    remote(m_targetId, targetPlayer, targetObject);
    remote(m_sourceId, sourcePlayer, sourceObject);
    *dwSize = 2 * (sizeof(PLAYER_ID) + sizeof(LONG)) + sizeof(BOOLEAN);
    UTIL_ASSERT(*dwSize <= STATICBUFFERSIZE);
    DWORD cnt = 0;
    memcpy(*pData + cnt, &targetPlayer, sizeof(targetPlayer)); cnt += sizeof(targetPlayer);
    memcpy(*pData + cnt, &targetObject, sizeof(targetObject)); cnt += sizeof(targetObject);
    memcpy(*pData + cnt, &sourcePlayer, sizeof(sourcePlayer)); cnt += sizeof(sourcePlayer);
    memcpy(*pData + cnt, &sourceObject, sizeof(sourceObject)); cnt += sizeof(sourceObject);
    (*pData)[cnt++] = m_bOpenDoor;
    UTIL_ASSERT(cnt == *dwSize);
}

// 0x4FD0E0
BOOL CMessageDoorOpen::UnmarshalMessage(BYTE* pData, DWORD dwSize)
{
    UTIL_ASSERT(pData != NULL);
    DWORD cnt = CNetwork::SPEC_MSG_HEADER_LENGTH;
    PLAYER_ID player;
    LONG object, local;
    memcpy(&player, pData + cnt, sizeof(player)); cnt += sizeof(player);
    memcpy(&object, pData + cnt, sizeof(object)); cnt += sizeof(object);
    if (g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(player, object, local) != TRUE) {
        m_targetId = CGameObjectArray::INVALID_INDEX;
        return FALSE;
    }
    m_targetId = local;
    memcpy(&player, pData + cnt, sizeof(player)); cnt += sizeof(player);
    memcpy(&object, pData + cnt, sizeof(object)); cnt += sizeof(object);
    m_sourceId = g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->Find(player, object, local) == TRUE
        ? local : CGameObjectArray::INVALID_INDEX;
    m_bOpenDoor = pData[cnt++];
    UTIL_ASSERT(cnt == dwSize);
    return TRUE;
}

// 0x4FD1D0
void CMessageDoorOpen::Run()
{
    CAIObjectType type;
    CGameObjectArray* array = g_pBaldurChitin->GetObjectGame()->GetObjectArray();
    CGameObject* target;
    BYTE rc;
    do {
        rc = array->GetDeny(m_targetId, CGameObjectArray::THREAD_ASYNCH, &target, INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
    if (rc == CGameObjectArray::SUCCESS) {
        CGameDoor* door = static_cast<CGameDoor*>(target);
        if ((door->m_dwFlags & 1) != m_bOpenDoor) {
            CGameObject* source;
            do {
                rc = array->GetShare(m_sourceId, CGameObjectArray::THREAD_ASYNCH, &source, INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);
            if (rc == CGameObjectArray::SUCCESS) {
                type.Set(source->GetAIType());
                door->ToggleDoor(type, FALSE);
                array->ReleaseShare(m_sourceId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
            }
        }
        array->ReleaseDeny(m_targetId, CGameObjectArray::THREAD_ASYNCH, INFINITE);
    }
}
