#include "Icewind586B70.h"

#include "CBaldurChitin.h"
#include "CGameSprite.h"
#include "CInfGame.h"
#include "CMessage.h"
#include "CProjectile.h"
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
bool Icewind586B70::CanJoinParty(CGameSprite* sprite)
{
    if (!IcewindMisc::IsPC(sprite)) {
        return true;
    }

    // NOTE: Uninline.
    return GetCount() + 1 <= 6;
}

// 0x586CF0
bool Icewind586B70::CanAddSummoned(CGameSprite* sprite1, CGameSprite* sprite2)
{
    if (!IcewindMisc::IsPC(sprite1)) {
        return true;
    }

    // NOTE: Uninline.
    if (GetCount() + 1 > 6) {
        return false;
    }

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].field_0 != 0
            && mEntries[index].field_4 != 0
            && sprite2->GetId() == mEntries[index].field_0) {
            return false;
        }
    }

    return true;
}

// 0x586D60
void Icewind586B70::AddSummoned(CGameSprite* sprite1, CGameSprite* sprite2)
{
    if (!IcewindMisc::IsPC(sprite1)) {
        return;
    }

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].field_0 == 0 || mEntries[index].field_4 == 0) {
            // NOTE: Odd check, looks like inlining.
            if (index != -1) {
                mEntries[index].field_0 = sprite2->GetId();
                mEntries[index].field_4 = sprite1->GetId();
                mEntries[index].field_8 = false;
                return;
            }
        }
    }
}

// 0x586DC0
void Icewind586B70::Unsummon(CGameSprite* sprite)
{
    std::vector<Entry>::iterator it;
    for (it = mEntries.begin(); it != mEntries.end(); it++) {
        if (sprite->GetId() == it->field_0) {
            break;
        }
    }

    // Dismissed summons leave without the poof; their link stays for the caller.
    if (it == mEntries.end() || it->field_8 == true) {
        return;
    }

    it->field_0 = 0;
    it->field_4 = 0;
    it->field_8 = false;

    CProjectile* projectile = CProjectileSummonVFX::DecodeSpellHitProjectile(0x15, NULL, FALSE);
    CMessage* message = new CMessageFireProjectile(0x1015,
        sprite->GetId(), sprite->GetPos(), 0, sprite->GetId(), sprite->GetId(), 0);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    projectile->Fire(sprite->GetArea(), sprite->GetId(), sprite->GetId(), sprite->GetPos(), 0, 0);

    sprite->SetStealthState(1);

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
        g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessage90(sprite->GetId(), sprite->GetId(), 1), FALSE);
    }

    sprite->m_removeFromArea = TRUE;
}

// 0x586F20
void Icewind586B70::AddDismissedSummoned(CGameSprite* sprite1, CGameSprite* sprite2)
{
    if (!IcewindMisc::IsPC(sprite1)) {
        return;
    }

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].field_0 == 0 || mEntries[index].field_4 == 0) {
            // NOTE: Odd check, looks like inlining.
            if (index != -1) {
                mEntries[index].field_0 = sprite2->GetId();
                mEntries[index].field_4 = sprite1->GetId();
                mEntries[index].field_8 = true;
                return;
            }
        }
    }
}

// 0x586F80
void Icewind586B70::Remove(CGameSprite* sprite)
{
    for (std::vector<Entry>::iterator it = mEntries.begin(); it < mEntries.end(); it++) {
        if (sprite->GetId() == it->field_0) {
            it->field_0 = 0;
            it->field_4 = 0;
            it->field_8 = false;
        }
    }
}

// 0x586FC0
void Icewind586B70::RestoreSummonLinks(CGameSprite* sprite)
{
    if (sprite->GetBaseStats()->field_2E9) {
        mEntries[sprite->GetBaseStats()->field_2E9].field_0 = sprite->GetId();
        mEntries[sprite->GetBaseStats()->field_2E9].field_8 = sprite->GetBaseStats()->field_2F7 != 0;
        sprite->GetBaseStats()->field_2E9 = 0;
        sprite->GetBaseStats()->field_2F7 = 0;

        CAIObjectType typeAI(sprite->GetAIType());
        CAIObjectType liveTypeAI(sprite->GetLiveAIType());
        CAIObjectType startTypeAI(sprite->GetLiveAIType());

        g_pBaldurChitin->GetObjectGame()->AddCharacterToAllies(sprite->GetId());

        if (!mEntries[sprite->GetBaseStats()->field_2E9].field_8) {
            startTypeAI.SetEnemyAlly(CAIObjectType::EA_ALL);
            sprite->m_startTypeAI.Set(startTypeAI);
        }

        typeAI.SetEnemyAlly(CAIObjectType::EA_ALLY);
        sprite->SetAIType(typeAI, FALSE, FALSE);

        liveTypeAI.SetEnemyAlly(CAIObjectType::EA_ALLY);
        sprite->m_liveTypeAI.Set(liveTypeAI);
    }

    for (int index = 0; index < 6; index++) {
        if (sprite->GetBaseStats()->field_2EA[index]) {
            mEntries[index].field_4 = sprite->GetId();
            sprite->GetBaseStats()->field_2EA[index] = FALSE;
        }
    }
}

// 0x587190
void Icewind586B70::SaveSummonLinks()
{
    CGameSprite* sprite;
    BYTE rc;

    for (int index = 0; index < 6; index++) {
        if (mEntries[index].field_0 != 0) {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(mEntries[index].field_0,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&sprite),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            sprite->GetBaseStats()->field_2E9 = index + 1;
            sprite->GetBaseStats()->field_2F7 = mEntries[index].field_8;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(mEntries[index].field_0,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        if (mEntries[index].field_4 != 0) {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(mEntries[index].field_4,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&sprite),
                INFINITE);
            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            sprite->GetBaseStats()->field_2EA[index] = 1;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(mEntries[index].field_4,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5872A0
void Icewind586B70::Clear()
{
    for (int index = 0; index < 6; index++) {
        mEntries[index].field_0 = 0;
        mEntries[index].field_4 = 0;
        mEntries[index].field_8 = false;
    }
}

// 0x5872C0
void Icewind586B70::UnsummonAllOf(CGameSprite* sprite)
{
    for (std::vector<Entry>::iterator it = mEntries.begin(); it != mEntries.end(); it++) {
        while (it != mEntries.end() && it->field_4 != sprite->GetId()) {
            it++;
        }
        if (it == mEntries.end()) {
            break;
        }

        LONG summonedId = it->field_0;
        if (summonedId == 0) {
            continue;
        }

        // A single attempt: a busy summon keeps its link.
        CGameSprite* summoned;
        if (g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(summonedId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&summoned),
                INFINITE) != CGameObjectArray::SUCCESS) {
            continue;
        }

        Instance()->Unsummon(summoned);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessageUnsummon(summoned->GetId(), summoned->GetId(), summoned->GetId()), FALSE);

        Icewind586B70* instance = Instance();
        LONG id = summoned->GetId();
        std::vector<Entry>::iterator link;
        for (link = instance->mEntries.begin(); link != instance->mEntries.end(); link++) {
            if (id == link->field_0) {
                break;
            }
        }
        if (link != instance->mEntries.end() && link->field_8) {
            link->Clear();
        }

        g_pBaldurChitin->GetMessageHandler()->AddMessage(new CMessageRemoveSummonLink(summoned->GetId(), summoned->GetId(), summoned->GetId()), FALSE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(summonedId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// NOTE: Inlined.
Icewind586B70::Entry::Entry()
{
    field_0 = 0;
    field_4 = 0;
    field_8 = false;
}

// 0x587500
void Icewind586B70::Entry::Clear()
{
    field_0 = 0;
    field_4 = 0;
    field_8 = false;
}

// 0x587610
int Icewind586B70::GetCount()
{
    int count = 0;
    for (int index = 0; index < 6; index++) {
        if (mEntries[index].field_0 != 0 && mEntries[index].field_4 != 0) {
            count++;
        }
    }
    return count;
}

// 0x587630
Icewind586B70::Entry::Entry(int a1, int a2)
{
    field_0 = a1;
    field_4 = a2;
    field_8 = false;
}
