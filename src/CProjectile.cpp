#include "CProjectile.h"

#include "CBaldurChitin.h"
#include "CGameEffect.h"
#include "CGameSprite.h"
#include "CInfGame.h"

// 0x6A3130
BOOLEAN CProjectile::IsProjectile()
{
    return TRUE;
}

// 0x5551B0
void CProjectile::RemoveSelf()
{
    RemoveFromArea();

    BYTE rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x51EA90
void CProjectile::AddEffect(CGameEffect* pEffect)
{
    pEffect->m_projectileType = m_projectileType;
    m_effectList.AddTail(pEffect);
}

// 0x529F10
void CProjectile::ClearEffects()
{
    POSITION pos = m_effectList.GetHeadPosition();
    while (pos != NULL) {
        CGameEffect* node = m_effectList.GetNext(pos);
        delete node;
    }
    m_effectList.RemoveAll();
}

// 0x529F40
LONG CProjectile::DetermineHeight(CGameSprite* pSprite)
{
    if (!m_bHasHeight) {
        return 0;
    }

    if (pSprite->GetObjectType() != TYPE_SPRITE) {
        return 32;
    }

    return pSprite->GetAnimation()->GetCastHeight();
}

// 0x78E740
void CProjectile::Fire(CGameArea* pArea, LONG source, LONG target, CPoint targetPos, LONG nHeight, SHORT nType)
{
}

// 0x529FB0
void CProjectile::OnArrival()
{
    CProjectile* pProjectile;
    BYTE rc;

    if (m_callBackProjectile != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_callBackProjectile,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        pProjectile->CallBack();

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_callBackProjectile,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    // NOTE: Uninline.
    PlaySound(m_arrivalSoundRef, m_loopArrivalSound, TRUE);

    if (m_secondaryProjectileId != CGameObjectArray::INVALID_INDEX) {
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_secondaryProjectileId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pProjectile),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pProjectile->RemoveSelf();

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_secondaryProjectileId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    DeliverEffects();
    RemoveFromArea();

    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->Delete(m_id,
        CGameObjectArray::THREAD_ASYNCH,
        NULL,
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        delete this;
    }
}

// 0x52A1A0
void CProjectile::DeliverEffects()
{
    // TODO: Incomplete.
}

// 0x52A480
SHORT CProjectile::GetDirection(CPoint target)
{
    CPoint ptStart;
    ptStart.x = m_pos.x;
    ptStart.y = 4 * m_pos.y / 3;

    CPoint ptTarget;
    ptTarget.x = target.x;
    ptTarget.y = 4 * target.y / 3;

    return CGameSprite::GetDirection(ptStart, ptTarget);
}

// 0x52A4E0
void CProjectile::PlaySound(CResRef resRef, BOOL loop, BOOL fireAndForget)
{
    m_sound.Stop();
    if (resRef != "") {
        m_sound.SetResRef(resRef, TRUE, TRUE);
        if (loop) {
            m_sound.SetLoopingFlag(TRUE);
        }
        if (fireAndForget) {
            m_sound.SetFireForget(TRUE);
        }
        m_sound.SetChannel(15, reinterpret_cast<DWORD>(m_pArea));
        m_sound.Play(m_pos.x, m_pos.y, 0, FALSE);
    }
}

// 0x78E730
void CProjectile::CallBack()
{
}

// Phase 1-2: Scaffold functions
// 0x51E9B0
void FUN_0051e9b0() {
    // TODO: Incomplete.
}

// 0x51EA70
void FUN_0051ea70() {
    // TODO: Incomplete.
}

// 0x51EAB0
void FUN_0051eab0() {
    // TODO: Incomplete.
}

// 0x51EAF0
void FUN_0051eaf0() {
    // TODO: Incomplete.
}

// 0x529490
void FUN_00529490() {
    // TODO: Incomplete.
}

// 0x5295D0
void FUN_005295d0() {
    // TODO: Incomplete.
}

// 0x529660
void FUN_00529660() {
    // TODO: Incomplete.
}

// 0x5297B0
void FUN_005297b0() {
    // TODO: Incomplete.
}

// 0x5297E0
void FUN_005297e0() {
    // TODO: Incomplete.
}

// 0x529800
void FUN_00529800() {
    // TODO: Incomplete.
}

// 0x529990
void FUN_00529990() {
    // TODO: Incomplete.
}

// 0x5299B0
void FUN_005299b0() {
    // TODO: Incomplete.
}

// 0x529A90
void FUN_00529a90() {
    // TODO: Incomplete.
}

// 0x529AB0
void FUN_00529ab0() {
    // TODO: Incomplete.
}

// 0x529AD0
void FUN_00529ad0() {
    // TODO: Incomplete.
}

// 0x529BF0
void FUN_00529bf0() {
    // TODO: Incomplete.
}

// 0x529CE0
void FUN_00529ce0() {
    // TODO: Incomplete.
}

// 0x529E30
void FUN_00529e30() {
    // TODO: Incomplete.
}

// 0x529E50
void FUN_00529e50() {
    // TODO: Incomplete.
}

// 0x52A640
void FUN_0052a640() {
    // TODO: Incomplete.
}

// 0x52AD60
void FUN_0052ad60() {
    // TODO: Incomplete.
}

// 0x52B010
void FUN_0052b010() {
    // TODO: Incomplete.
}

// 0x52B030
void FUN_0052b030() {
    // TODO: Incomplete.
}

// 0x52B150
void FUN_0052b150() {
    // TODO: Incomplete.
}

// 0x52B190
void FUN_0052b190() {
    // TODO: Incomplete.
}

// 0x52B6B0
void FUN_0052b6b0() {
    // TODO: Incomplete.
}

// 0x52B900
void FUN_0052b900() {
    // TODO: Incomplete.
}

// 0x52C8C0
void FUN_0052c8c0() {
    // TODO: Incomplete.
}

// 0x52CA10
void FUN_0052ca10() {
    // TODO: Incomplete.
}

// 0x52CBA0
void FUN_0052cba0() {
    // TODO: Incomplete.
}

// 0x52CBC0
void FUN_0052cbc0() {
    // TODO: Incomplete.
}

// 0x52CCE0
void FUN_0052cce0() {
    // TODO: Incomplete.
}

// 0x52CE10
void FUN_0052ce10() {
    // TODO: Incomplete.
}

// 0x52CE30
void FUN_0052ce30() {
    // TODO: Incomplete.
}

// 0x52D430
void FUN_0052d430() {
    // TODO: Incomplete.
}

// 0x52E160
void FUN_0052e160() {
    // TODO: Incomplete.
}

// 0x52E230
void FUN_0052e230() {
    // TODO: Incomplete.
}

// 0x52E360
void FUN_0052e360() {
    // TODO: Incomplete.
}

// 0x52E380
void FUN_0052e380() {
    // TODO: Incomplete.
}

// 0x52E4D0
void FUN_0052e4d0() {
    // TODO: Incomplete.
}

// 0x5549B0
void FUN_005549b0() {
    // TODO: Incomplete.
}

// 0x5549D0
void FUN_005549d0() {
    // TODO: Incomplete.
}

// 0x554D20
void FUN_00554d20() {
    // TODO: Incomplete.
}

// 0x554F00
void FUN_00554f00() {
    // TODO: Incomplete.
}

// 0x555200
void FUN_00555200() {
    // TODO: Incomplete.
}

