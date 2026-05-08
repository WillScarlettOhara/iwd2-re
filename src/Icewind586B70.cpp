#include "Icewind586B70.h"

#include "CBaldurChitin.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "IcewindMisc.h"

// 0x8E6400
Icewind586B70* Icewind586B70::mInstance;

// 0x586B70
Icewind586B70::Icewind586B70()
    : mEntries(6)
{
}

// 0x586BD0
Icewind586B70* Icewind586B70::Instance()
{
    if (mInstance == NULL) {
        mInstance = new Icewind586B70();
    }
    return mInstance;
}

// 0x586C80
void Icewind586B70::Destroy()
{
    if (mInstance != NULL) {
        delete mInstance;
    }
}

// 0x586CA0
bool Icewind586B70::IsPartyNotFull(CGameSprite* sprite)
{
    if (!IcewindMisc::IsPC(sprite)) {
        return true;
    }

    // NOTE: Uninline.
    return GetCount() + 1 <= 6;
}

// 0x586CF0
bool Icewind586B70::CanAddSprite(CGameSprite* sprite1, CGameSprite* sprite2)
{
    if (!IcewindMisc::IsPC(sprite1)) {
        return true;
    }

    // NOTE: Uninline.
    if (GetCount() + 1 > 6) {
        return false;
    }

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].m_targetId != 0
            && mEntries[index].m_controllerId != 0
            && sprite2->GetId() == mEntries[index].m_targetId) {
            return false;
        }
    }

    return true;
}

// 0x586D60
void Icewind586B70::AddTemporary(CGameSprite* sprite1, CGameSprite* sprite2)
{
    if (!IcewindMisc::IsPC(sprite1)) {
        return;
    }

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].m_targetId == 0 || mEntries[index].m_controllerId == 0) {
            // NOTE: Odd check, looks like inlining.
            if (index != -1) {
                mEntries[index].m_targetId = sprite2->GetId();
                mEntries[index].m_controllerId = sprite1->GetId();
                mEntries[index].m_bPermanent = false;
            }
        }
    }
}

// 0x586F20
void Icewind586B70::AddPermanently(CGameSprite* sprite1, CGameSprite* sprite2)
{
    if (!IcewindMisc::IsPC(sprite1)) {
        return;
    }

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].m_targetId == 0 || mEntries[index].m_controllerId == 0) {
            // NOTE: Odd check, looks like inlining.
            if (index != -1) {
                mEntries[index].m_targetId = sprite2->GetId();
                mEntries[index].m_controllerId = sprite1->GetId();
                mEntries[index].m_bPermanent = true;
            }
        }
    }
}

// 0x586F80
void Icewind586B70::Remove(CGameSprite* sprite)
{
    for (std::vector<Entry>::iterator it = mEntries.begin(); it < mEntries.end(); it++) {
        if (sprite->GetId() == it->m_targetId) {
            it->m_targetId = 0;
            it->m_controllerId = 0;
            it->m_bPermanent = false;
        }
    }
}

// 0x586FC0
void Icewind586B70::Reinstate(CGameSprite* sprite)
{
    if (sprite->GetBaseStats()->m_field_2E9) {
        mEntries[sprite->GetBaseStats()->m_field_2E9].m_targetId = sprite->GetId();
        mEntries[sprite->GetBaseStats()->m_field_2E9].m_bPermanent = sprite->GetBaseStats()->m_field_2F7 != 0;
        sprite->GetBaseStats()->m_field_2E9 = 0;
        sprite->GetBaseStats()->m_field_2F7 = 0;

        CAIObjectType typeAI(sprite->GetAIType());
        CAIObjectType liveTypeAI(sprite->GetLiveAIType());
        CAIObjectType startTypeAI(sprite->GetLiveAIType());

        g_pBaldurChitin->GetObjectGame()->AddCharacterToAllies(sprite->GetId());

        if (!mEntries[sprite->GetBaseStats()->m_field_2E9].m_bPermanent) {
            startTypeAI.SetEnemyAlly(CAIObjectType::EA_ALL);
            sprite->m_startTypeAI.Set(startTypeAI);
        }

        typeAI.SetEnemyAlly(CAIObjectType::EA_ALLY);
        sprite->SetAIType(typeAI, FALSE, FALSE);

        liveTypeAI.SetEnemyAlly(CAIObjectType::EA_ALLY);
        sprite->m_liveTypeAI.Set(liveTypeAI);
    }

    for (int index = 0; index < 6; index++) {
        if (sprite->GetBaseStats()->m_field_2EA[index]) {
            mEntries[index].m_controllerId = sprite->GetId();
            sprite->GetBaseStats()->m_field_2EA[index] = FALSE;
        }
    }
}

// 0x587190
void Icewind586B70::SyncToSprites()
{
    CGameSprite* sprite;
    BYTE rc;

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].m_targetId != 0) {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(mEntries[index].m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&sprite),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            sprite->GetBaseStats()->m_field_2E9 = index + 1;
            sprite->GetBaseStats()->m_field_2F7 = mEntries[index].m_bPermanent;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(mEntries[index].m_targetId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (mEntries[index].m_controllerId != 0) {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(mEntries[index].m_controllerId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&sprite),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            sprite->GetBaseStats()->m_field_2EA[index] = 1;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(mEntries[index].m_controllerId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5872A0
void Icewind586B70::Clear()
{
    for (int index = 0; index < 6; index++) {
        mEntries[index].m_targetId = 0;
        mEntries[index].m_controllerId = 0;
        mEntries[index].m_bPermanent = false;
    }
}

// NOTE: Inlined.
Icewind586B70::Entry::Entry()
{
    m_targetId = 0;
    m_controllerId = 0;
    m_bPermanent = false;
}

// 0x587500
void Icewind586B70::Entry::Clear()
{
    m_targetId = 0;
    m_controllerId = 0;
    m_bPermanent = false;
}

// 0x587610
int Icewind586B70::GetCount()
{
    int count = 0;
    for (int index = 0; index < 6; index++) {
        if (mEntries[index].m_targetId != 0 && mEntries[index].m_controllerId != 0) {
            count++;
        }
    }
    return count;
}

// 0x587630
Icewind586B70::Entry::Entry(int a1, int a2)
{
    m_targetId = a1;
    m_controllerId = a2;
    m_bPermanent = false;
}

// Phase 1-2: Scaffold functions
// 0x586DC0
void FUN_00586dc0() {
    // TODO: Incomplete.
}

// 0x5872C0
void FUN_005872c0() {
    // TODO: Incomplete.
}

// 0x587510
void FUN_00587510() {
    // TODO: Incomplete.
}

// 0x587650
void FUN_00587650() {
    // TODO: Incomplete.
}

// 0x587660
void FUN_00587660() {
    // TODO: Incomplete.
}

// 0x5876D0
void FUN_005876d0() {
    // TODO: Incomplete.
}

